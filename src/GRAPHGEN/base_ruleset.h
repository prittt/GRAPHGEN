// Copyright(c) 2019
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
//
// *Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and / or other materials provided with the distribution.
//
// * Neither the name of GRAPHGEN nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef GRAPHGEN_BASE_RULESET_H_
#define GRAPHGEN_BASE_RULESET_H_

#include <fstream>
#include <filesystem>
#include <string>
#include <cmath>
#include <utility>
#include <filesystem>

#include "zstdstream.h"

#include "yaml-cpp/yaml.h"
#include "rule_set.h"
#include <zstd.h>

#include "constants.h"
#include "utilities.h"
#include <omp.h>


/** @brief Is the base class for the RuleSet from which every user-defined RuleSet should inherit

It contains member functions (LoadRuleSet and SaveRuleSet) to load and store a .yaml file which
define the RuleSet.

*/
class BaseRuleSet {
    std::filesystem::path p_;
    bool force_generation_;
	bool disable_generation_;
    rule_set rs_;

	std::vector<action_bitset> currently_loaded_rules = std::vector<action_bitset>();
	int currently_open_partition = -1;

    /** @brief Load the RuleSet from a (.yaml) file. The name of the file is defined in 
    the conf global variable. 
    */
    bool LoadRuleSet() {

        YAML::Node rs_node;
        try {
            rs_node = YAML::LoadFile(p_.string());
        }
        catch (...){
            return false;
        }

        rs_ = rule_set(rs_node);
        return true;
    }

    /** @brief Store the RuleSet into a (.yaml) file. The name of the file is defined in 
    the conf global variable. 
    */
    void SaveRuleSet() {
        std::ofstream os(p_.string());
        if (os) {
            YAML::Node n = rs_.Serialize();
            YAML::Emitter emitter(os);
            emitter.SetSeqFormat(YAML::EMITTER_MANIP::Flow);
            emitter << n;
        }
    }

public:

    BaseRuleSet(bool force_generation = false) : force_generation_{ force_generation }, disable_generation_{ false }
    {
        p_ = conf.rstable_path_;
    }

	BaseRuleSet(std::filesystem::path custom_path) : force_generation_{ false }, disable_generation_{ true }, p_{ custom_path } 
	{
	}

    rule_set GetRuleSet() {
		if (force_generation_ || !LoadRuleSet()) {
			if (disable_generation_) {
				auto msg = "Could not load rule set " + p_.string() + " from file (generation disabled).\n";
				std::cerr << msg;
				throw std::runtime_error((msg).c_str());
			}

            rs_ = GenerateRuleSet();
			TLOG("Writing rules to disk",
				//SaveAllRulesBinary();
			);
			//std::cout << "** DONE" << std::endl;
			//exit(EXIT_SUCCESS);
            SaveRuleSet();
        }
        return rs_;
    }

	void SaveAllRulesBinary() {
		std::cout << "[Rule Files] Partitions: " << PARTITIONS << " Rulecodes for each partition: " << RULES_PER_PARTITION << std::endl;
		
		#pragma omp parallel
		{			
			#pragma omp for
			for (int p = 0; p < PARTITIONS; p++) {
				const ullong begin_rule_code = p * RULES_PER_PARTITION;
				const ullong end_rule_code = (p + 1) * RULES_PER_PARTITION;

				auto final_path = conf.binary_rule_file_path_partitioned(std::to_string(begin_rule_code) + "-" + std::to_string(end_rule_code));
				auto tmp_path = final_path + ".tmp";

				// check status of existing files
				if (std::filesystem::exists(final_path)) {
					std::cout << "[Partition " << p << "] Final partition exists: skipping.\n";
					continue;
				}
				if (std::filesystem::exists(tmp_path)) {
					try {
						auto p2 = tmp_path + ".test";
						std::filesystem::rename(tmp_path, p2);
						std::filesystem::remove(p2);
						std::cout << "[Partition " << p << "] Partition .tmp-file exists and is not locked: overwriting.\n";
					}
					catch (std::filesystem::filesystem_error e) {
						std::cerr << "[Partition " << p << "] Partition .tmp-file exists, but is locked: skipping.\n";                    
						continue;
					}
				}
				if (!std::filesystem::is_directory(std::filesystem::path(tmp_path).remove_filename())) {
					if (!std::filesystem::create_directories(std::filesystem::path(tmp_path).remove_filename())) {
						std::cerr << "[Partition " << p << "] Could not create rules directory, aborting.\n";
						exit(EXIT_FAILURE);
					}
				}
				
				zstd::ofstream os(tmp_path, std::ios::binary);
				if (!os) {
					std::cerr << "[Partition " << p << "] Partition could not be saved to " << tmp_path << ": skipping.\n";
					continue;
				}

				std::cout << "[Partition " << p << "] Begin processing. First Rule Code: " << begin_rule_code << " Last (exclusive) Rule Code: " << end_rule_code << std::endl;

				if (rs_.rulesStatus == IN_MEMORY) {
					// rules are in memory
					throw std::runtime_error("writing rules from memory not yet implemented");
					//std::for_each(rs_.rules.begin(), rs_.rules.end(), [&os, stream_size](rule r) { os.write(reinterpret_cast<const char*>(&r.actions), stream_size); });
				}
				else {
					// rules are generated during the writing
					std::vector<action_bitset> actions;
					if (RULES_PER_BATCH > actions.max_size()) {
						std::cerr << "ERROR: Batches are too big to be stored in a vector. (batch size: " << RULES_PER_BATCH << " vector.max_size: " << actions.max_size() << ")" << std::endl;
						exit(EXIT_FAILURE);
					}
					actions.resize(RULES_PER_BATCH);

					//TLOG("rules batched", 

					for (int b = 0; b < BATCHES; b++) {
						const ullong batch_begin_rule_code = (begin_rule_code + b * RULES_PER_BATCH);
						const ullong batch_end_rule_code = (begin_rule_code + (b + 1) * RULES_PER_BATCH);

						std::cout << "[Partition " << p << "] Processing rule batch " << (b + 1) << " of " << BATCHES << ", rules from " << batch_begin_rule_code << " to " << batch_end_rule_code << ".\n";
						size_t i = 0;
						for (ullong rule_code = batch_begin_rule_code; rule_code < batch_end_rule_code; rule_code++, i++) {
							actions[i] = GetActionFromRuleIndex(rs_, rule_code);
						}
						for (const action_bitset& a : actions) {
							auto s = a.size();
							os.write(reinterpret_cast<const char*>(&s), 1);
							for (const ushort& b : a.getSingleActions()) {
								os.write(reinterpret_cast<const char*>(&b), 2);
							}
						}
					}
					//);
				}
				os.close();
				std::filesystem::rename(tmp_path, final_path);
			}
		}		
	}

	void OpenRuleFiles() {
		std::cout << "Opening rule files...";

		if (RULES_PER_PARTITION > currently_loaded_rules.max_size()) {
			std::cerr << "Cannot store 1 partition in memory, aborting. (" << RULES_PER_PARTITION << " / " << currently_loaded_rules.max_size() << ")" << std::endl;
			throw std::runtime_error("Cannot store 1 partition in memory, aborting.");
		}
		//decompression.allocateResources();
		currently_loaded_rules.resize(RULES_PER_PARTITION);
		std::cout << "done." << std::endl;
	}

	void VerifyRuleFiles() {
		std::cout << "** Verifying rule files. (Partitions: " << PARTITIONS << " Rulecodes for each partition: " << RULES_PER_PARTITION << ")" << std::endl;

#pragma omp parallel for
		for (int p = 0; p < PARTITIONS; p++) {
			//std::cout << "Verifying partition " << p << "...\n";
			const ullong begin_rule_code = p * RULES_PER_PARTITION;
			const ullong end_rule_code = (p + 1) * RULES_PER_PARTITION;

			auto path = conf.binary_rule_file_path_partitioned(std::to_string(begin_rule_code) + "-" + std::to_string(end_rule_code));

			// check status of existing files
			if (!std::filesystem::exists(path)) {
				std::cerr << "Partition " << p << "(" << begin_rule_code << " - " << end_rule_code << ", " << path << ") does not exist: aborting.\n";
				exit(EXIT_FAILURE);
			}

			zstd::ifstream is(path, std::ios::binary);
			if (!is) {
				std::cout << "Could not open partition " << p << " at " << path << " \n";
				exit(EXIT_FAILURE);
			}

			constexpr int actions_verified = 64000;

			std::vector<action_bitset> actions_correct(actions_verified);
			std::vector<action_bitset> actions_read(actions_verified);

			bool correct = true;
			int wrong_index;
			uchar size;

			for (int i = 0; i < actions_verified; i++) {
				actions_correct[i] = GetActionFromRuleIndex(rs_, begin_rule_code + i);

				is.read(reinterpret_cast<char*>(&size), 1);
				actions_read[i].resize(size);
				for (ushort& x : actions_read[i].getSingleActions()) {
					is.read(reinterpret_cast<char*>(&x), 2);
				}
				if (actions_correct[i] != actions_read[i]) {
					correct = false;
					wrong_index = i;
				}
			}

			if (!correct) {
				std::cout << "\n************************************************************" << std::endl;
				std::cout << "Incorrect rule found in rule partition " << p << " at " << wrong_index << " at path: "<< path << std::endl;
				for (int i = wrong_index; i <= wrong_index; i++) {
					std::cout << "Rule Code: " << (begin_rule_code + i) << "\tCorrect action: ";
					for (const auto& x : actions_correct[i].getSingleActions()) {
						std::cout << x << ",";
					}
					std::cout << "\tRead action: ";
					for (const auto& x : actions_read[i].getSingleActions()) {
						std::cout << x << ",";
					}
					std::cout << std::endl;
				}
				std::cout << "************************************************************" << std::endl;
				exit(EXIT_FAILURE);
			}
			if (p % std::max(1, (PARTITIONS / 64)) == 0) {
				std::cout << "P" << p << " correct. ";
			}

		}
		
		std::cout << "** All rule files verified." << std::endl;		
	}

#include "bbtd3d-36-reductiontable-5813-2829.inc"

	action_bitset* LoadRuleFromBinaryRuleFiles(const ullong& rule_code) {
		try {
			int p = static_cast<int>(rule_code / RULES_PER_PARTITION);

			if (p != currently_open_partition) {
				// rule is in a different partition, open it
				const ullong begin_rule_code = p * RULES_PER_PARTITION;
				const ullong end_rule_code = (p + 1) * RULES_PER_PARTITION;

				auto path = conf.binary_rule_file_path_partitioned(std::to_string(begin_rule_code) + "-" + std::to_string(end_rule_code));

				// check status of existing files
				if (!std::filesystem::exists(path)) {
					std::cerr << "Partition " << p << " does not exist: aborting.\n";
					exit(EXIT_FAILURE);
				}

				zstd::ifstream is(path, std::ios::binary);
				uchar size;
				for (int i = 0; i < RULES_PER_PARTITION; i++) {
					is.read(reinterpret_cast<char*>(&size), 1);
					currently_loaded_rules[i].resize(size);
					for (ushort& x : currently_loaded_rules[i].getSingleActions()) {
						is.read(reinterpret_cast<char*>(&x), 2);
					}
				}

				currently_open_partition = p;
			}

			return &currently_loaded_rules[static_cast<size_t>(rule_code % RULES_PER_PARTITION)];
		}
		catch (const std::ifstream::failure&) {
			std::cerr << "Binary rule " << rule_code << " couldn't be loaded from binary rule file (stream failure).\n";
		}
		catch (const std::runtime_error&) {
			std::cerr << "Binary rule " << rule_code << " couldn't be loaded from binary rule file (runtime error).\n";
		}
		std::cerr << "Rule lookup from binary rule file failed. Rule code: " << rule_code << std::endl;
		throw std::runtime_error("Rule lookup from binary rule file failed.");
	}

	void LoadPartition(const int p, std::vector<action_bitset>& actions) {
		try {
			const ullong begin_rule_code = p * RULES_PER_PARTITION;
			const ullong end_rule_code = (p + 1) * RULES_PER_PARTITION;

			auto path = conf.binary_rule_file_path_partitioned(std::to_string(begin_rule_code) + "-" + std::to_string(end_rule_code));

			// check status of existing files
			if (!std::filesystem::exists(path)) {
				std::cerr << "Partition " << p << " does not exist: aborting.\n";
				exit(EXIT_FAILURE);
			}

			zstd::ifstream is(path, std::ios::binary);
			uchar size;
			for (int i = 0; i < RULES_PER_PARTITION; i++) {
				auto& a = actions[i];
				is.read(reinterpret_cast<char*>(&size), 1);
				a.resize(size);
				for (ushort& x : a.getSingleActions()) {
					is.read(reinterpret_cast<char*>(&x), 2);
				}
			}
			return;
		}
		catch (const std::ifstream::failure&) {
			std::cerr << "Partition " << p << " couldn't be loaded from binary rule file (stream failure).\n";
		}
		catch (const std::runtime_error&) {
			std::cerr << "Partition " << p << " couldn't be loaded from binary rule file (runtime error).\n";
		}
		std::cerr << "Rule lookup from binary rule file failed. Partition: " << p << std::endl;
		throw std::runtime_error("Rule lookup from binary rule file failed.");
	}


	void ConvertPartitions(int old_partition_count = 65536, int new_partition_count = 1048576, bool reduce_actions = false) {
	    const auto old_basepath = "D:/rules-bkp/zst-variable-data-format-65536p-2829a";
	    const auto new_basepath = "D:/rules-bkp/zst-variable-data-format-1048576p-2829a";
	    uint64_t old_rules_per_partition = TOTAL_RULES / old_partition_count;
		uint64_t new_rules_per_partition = TOTAL_RULES / new_partition_count;
	
	    // Assume: old partition count < new partition count
	    // Assume: both partition counts can be divised through each other as a whole number

		{
			std::vector<action_bitset> actions;
			actions.resize(static_cast<size_t>(old_rules_per_partition));

			for (int old_p = 0; old_p < old_partition_count; old_p++) {
				const ullong old_begin_rule_code = old_p * old_rules_per_partition;
				const ullong old_end_rule_code = (old_p + 1) * old_rules_per_partition;

				auto old_path = conf.binary_rule_file_path_partitioned(std::to_string(old_begin_rule_code) + "-" + std::to_string(old_end_rule_code), old_basepath);
				zstd::ifstream is(old_path, std::ios::binary);
				if (!is) {
					std::cerr << "[Old Partition " << old_p << "] Partition could not be opened at " << old_path << ": skipping.\n";
					continue;
				}
				std::cout << "[Old Partition " << old_p << "]\n";


				uchar size;
				for (size_t i = 0; i < old_rules_per_partition; i++) {
					is.read(reinterpret_cast<char*>(&size), 1);
					actions[i].resize(size);
					for (ushort& x : actions[i].getSingleActions()) {
						is.read(reinterpret_cast<char*>(&x), 2);
					}
				}

				const int first_new_partition = static_cast<int>(old_begin_rule_code / new_rules_per_partition);
				const int last_new_partition = static_cast<int>(old_end_rule_code / new_rules_per_partition);

#pragma omp parallel for
				for (int new_p = first_new_partition; new_p < last_new_partition; new_p++) {
					const ullong new_begin_rule_code = new_p * new_rules_per_partition;
					const ullong new_end_rule_code = (new_p + 1) * new_rules_per_partition;

					auto new_path = conf.binary_rule_file_path_partitioned(std::to_string(new_begin_rule_code) + "-" + std::to_string(new_end_rule_code), new_basepath);
					zstd::ofstream os(new_path, std::ios::binary);
					if (!os) {
						std::cerr << "[New Partition " << new_p << "] Partition could not be saved at " << new_path << ": skipping.\n";
						continue;
					}
					std::cout << "[New Partition " << new_p << "] ";

					const ullong actions_offset = new_begin_rule_code - old_begin_rule_code;
					ushort converted;
					for (ullong i = actions_offset; i < actions_offset + new_rules_per_partition; i++) {
						const auto& a = actions[static_cast<size_t>(i)];
						auto s = a.size();
						os.write(reinterpret_cast<const char*>(&s), 1);
						for (const ushort& b : a.getSingleActions()) {
							if (b > 90) {
								converted = action_reduction_mapping[b];
								os.write(reinterpret_cast<const char*>(&converted), 2);
							}
							else {
								os.write(reinterpret_cast<const char*>(&b), 2);
							}
						}
					}
					os.close();
				}
			}
		}
	}


	void ReduceActionsInRuleFiles() {
		OpenRuleFiles();
		std::bitset<ACTION_COUNT> counts;
		for (int p = 0; p < PARTITIONS; p++) {
			TLOG("load partition",
				LoadRuleFromBinaryRuleFiles(p * RULES_PER_PARTITION);    // hacky way of loading the desired partition
			);

			for (ullong rule_code = p * RULES_PER_PARTITION; rule_code < (p + 1) * RULES_PER_PARTITION; rule_code++) {
				if (rule_code % (TOTAL_RULES >> 8) == 0) {
					std::cout << "rule " << rule_code << " out of " << TOTAL_RULES << std::endl;
				}
				for (const auto& s : LoadRuleFromBinaryRuleFiles(rule_code)->getSingleActions()) {
					if (!counts.test(s)) {
						counts.set(s);
					}
				}
			}
		}
		
		std::cout << "writing" << std::endl;
		std::ofstream os("actions5-8.csv", std::ios::binary);
		for (int n = 0; n < ACTION_COUNT; n++) {
			os << n << ',' << counts[n] << ",\n";
		}
		os.close();
		std::cout << "done reducing.";
	}

    virtual rule_set GenerateRuleSet() = 0;

	virtual action_bitset GetActionFromRuleIndex(const rule_set& rs, uint64_t rule_index) const = 0;

};

#endif //GRAPHGEN_BASE_RULESET_H_