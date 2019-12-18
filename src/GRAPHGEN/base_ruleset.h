// Copyright(c) 2018 - 2019 Federico Bolelli, Costantino Grana
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

#include "compression.h"
#include "zstdstream.h"

#include "yaml-cpp/yaml.h"
#include "rule_set.h"
#include <zstd.h>

constexpr int PARTITIONS = 1;
constexpr int BATCHES = 1;


class BaseRuleSet {
    std::filesystem::path p_;
    bool force_generation_;
	bool disable_generation_;
    rule_set rs_;

	int binary_rule_file_stream_size = static_cast<int>(ceil(action_bitset::max_size_in_bits() / 8.0));
	std::vector<action_bitset> currently_loaded_rules = std::vector<action_bitset>();
	ZstdDecompression decompression;
	int currently_open_partition = -1;

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
				SaveAllRulesBinary();
			);
			//std::cout << "** DONE" << std::endl;
			//exit(EXIT_SUCCESS);
            SaveRuleSet();
        }
        return rs_;
    }

	void SaveAllRulesBinary() {
		const ullong rules_per_partition = (1ULL << rs_.conditions.size()) / PARTITIONS;
		const ullong partition_size_bytes = binary_rule_file_stream_size * rules_per_partition;
		const ullong partition_size_megabytes = binary_rule_file_stream_size * rules_per_partition / (1024 * 1024);

		std::cout << "[Rule Files] Partitions: " << PARTITIONS << " Rulecodes for each partition: " << rules_per_partition << " Estimated partition size (Megabyte): " << partition_size_megabytes << std::endl;
		
		#pragma omp parallel
		{
			//ZstdStreamingCompression streamingCompression;
			//streamingCompression.allocateResources();
			
			#pragma omp for
			for (int p = 0; p < PARTITIONS; p++) {
				const ullong begin_rule_code = p * rules_per_partition;
				const ullong end_rule_code = (p + 1) * rules_per_partition;

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
					std::cerr << "[Partition " << p << "] Rules directory does not exist, aborting.\n";
					exit(EXIT_FAILURE);
				}
				
				zstd::ofstream os(tmp_path, std::ios::binary);
				if (!os) {
					std::cerr << "[Partition " << p << "] Partition could not be saved to " << tmp_path << ": skipping.\n";
					continue;
				}

				std::cout << "[Partition " << p << "] Begin processing. First Rule Code: " << begin_rule_code << " Last (exclusive) Rule Code: " << end_rule_code << std::endl;

				int stream_size = binary_rule_file_stream_size;
				if (rs_.rulesStatus == IN_MEMORY) {
					// rules are in memory
					throw std::runtime_error("writing rules from memory not yet implemented");
					//std::for_each(rs_.rules.begin(), rs_.rules.end(), [&os, stream_size](rule r) { os.write(reinterpret_cast<const char*>(&r.actions), stream_size); });
				}
				else {
					// rules are generated during the writing
					const ullong batches_steps = rules_per_partition / BATCHES;

					std::vector<action_bitset> actions;
					if (batches_steps > actions.max_size()) {
						std::cerr << "ERROR: Batches are too big to be stored in a vector. (batch size: " << batches_steps << " vector.max_size: " << actions.max_size() << ")" << std::endl;
						exit(EXIT_FAILURE);
					}
					actions.resize(static_cast<size_t>(batches_steps));

					//TLOG("rules batched", 

					for (int b = 0; b < BATCHES; b++) {
						const ullong batch_begin_rule_code = (begin_rule_code + b * batches_steps);
						const ullong batch_end_rule_code = (begin_rule_code + (b + 1) * batches_steps);

						std::cout << "[Partition " << p << "] Processing rule batch " << (b + 1) << " of " << BATCHES << ", rules from " << batch_begin_rule_code << " to " << batch_end_rule_code << ".\n";
						size_t i = 0;
						for (ullong rule_code = batch_begin_rule_code; rule_code < batch_end_rule_code; rule_code++, i++) {
							actions[i] = GetActionFromRuleIndex(rs_, rule_code);
						}
						//streamingCompression.compressDataChunk(&actions[0], static_cast<size_t>(batches_steps) * stream_size, b == BATCHES - 1);
						for (const action_bitset& a : actions) {
							auto s = a.size();
							os.write(reinterpret_cast<const char*>(&s), 1);
							for (const ushort& b : a.getSingleActions()) {
								os.write(reinterpret_cast<const char*>(&b), 2);
							}
							//os.write(reinterpret_cast<const char*>(&a), 2 * a.size());
						}
					}
					//);
				}
				//streamingCompression.endStreaming();
				os.close();
				std::filesystem::rename(tmp_path, final_path);
			}
			
			//streamingCompression.freeResources();
		}		
	}

	void OpenRuleFiles() {
		const ullong rules_per_partition = (1ULL << rs_.conditions.size()) / PARTITIONS;
		if (rules_per_partition > currently_loaded_rules.max_size()) {
			std::cerr << "Cannot store 1 partition in memory, aborting. (" << rules_per_partition << " / " << currently_loaded_rules.max_size() << ")" << std::endl;
			throw std::runtime_error("Cannot store 1 partition in memory, aborting.");
		}
		//decompression.allocateResources();
		currently_loaded_rules.resize(static_cast<size_t>(rules_per_partition));
		std::cout << "Rule files opened." << std::endl;
	}

	void VerifyRuleFiles() {
		const ullong rules_per_partition = (1ULL << rs_.conditions.size()) / PARTITIONS;
		const ullong partition_size_bytes = binary_rule_file_stream_size * rules_per_partition;
		const ullong partition_size_megabytes = binary_rule_file_stream_size * rules_per_partition / (1024 * 1024);
		
		if (rules_per_partition > currently_loaded_rules.max_size()) {
			std::cerr << "Cannot store 1 partition in memory, aborting. (" << rules_per_partition << " / " << currently_loaded_rules.max_size() << ")" << std::endl;
			throw std::runtime_error("Cannot store 1 partition in memory, aborting.");
		}

		currently_loaded_rules.resize(static_cast<size_t>(rules_per_partition));

		std::cout << "** Verifying rule files. (Partitions: " << PARTITIONS << " Rulecodes for each partition: " << rules_per_partition << ")" << std::endl;
		
		for (int p = 0; p < PARTITIONS; p++) {
			const ullong begin_rule_code = p * rules_per_partition;
			const ullong end_rule_code = (p + 1) * rules_per_partition;

			auto path = conf.binary_rule_file_path_partitioned(std::to_string(begin_rule_code) + "-" + std::to_string(end_rule_code));

			// check status of existing files
			if (!std::filesystem::exists(path)) {
				std::cerr << "Partition " << p << " does not exist: aborting.\n";
				exit(EXIT_FAILURE);
			}

			std::vector<action_bitset> actions;
			actions.resize(static_cast<size_t>(rules_per_partition));

			zstd::ifstream binary_rule_file(path, std::ios::binary);
			if (!binary_rule_file) {
				std::cout << "Could not open partition " << p << " at " << path << " \n";
				exit(EXIT_FAILURE);
			}
			const action_bitset first_action_correct = GetActionFromRuleIndex(rs_, begin_rule_code);
			const action_bitset last_action_correct = GetActionFromRuleIndex(rs_, end_rule_code - 1);

			action_bitset* first_action_read = LoadRuleFromBinaryRuleFiles(begin_rule_code);
			
			action_bitset* last_action_read = LoadRuleFromBinaryRuleFiles(end_rule_code - 1);

			if (first_action_correct != *first_action_read || last_action_correct != *last_action_read) {
				std::cout << "************************************************************" << std::endl;
				std::cout << "Incorrect rule found in rule partition " << p << " at " << path << std::endl;
				std::cout << "Rule Code: " << begin_rule_code << "\tCorrect action: " << first_action_correct.to_string() << "\tRead action: " << first_action_read->to_string() << std::endl;
				std::cout << "Rule Code: " << (end_rule_code - 1) << "\tCorrect action: " << last_action_correct.to_string() << "\tRead action: " << last_action_read->to_string() << std::endl;
				std::cout << "************************************************************" << std::endl;
				exit(EXIT_FAILURE);
			}
		}
		
		std::cout << "** All rule files verified." << std::endl;		
	}

	ullong previous_rule_code = UINT64_MAX;

	action_bitset* LoadRuleFromBinaryRuleFiles(const ullong& rule_code) {
		const ullong rules_per_partition = (1ULL << rs_.conditions.size()) / PARTITIONS;
		const ullong partition_size_bytes = binary_rule_file_stream_size * rules_per_partition;

		try {
			int p = static_cast<int>(rule_code / rules_per_partition);

			if (p != currently_open_partition) {
				// rule is in a different partition, open it
				const ullong begin_rule_code = p * rules_per_partition;
				const ullong end_rule_code = (p + 1) * rules_per_partition;

				auto path = conf.binary_rule_file_path_partitioned(std::to_string(begin_rule_code) + "-" + std::to_string(end_rule_code));

				// check status of existing files
				if (!std::filesystem::exists(path)) {
					std::cerr << "Partition " << p << " does not exist: aborting.\n";
					exit(EXIT_FAILURE);
				}

				zstd::ifstream is(path, std::ios::binary);
				for (int i = 0; i < rules_per_partition; i++) {
					uchar size;
					is.read(reinterpret_cast<char*>(&size), 1);
					currently_loaded_rules[i].resize(size);
					for (uchar x = 0; x < size; x++) {
						is.read(reinterpret_cast<char*>(&(currently_loaded_rules[i].getActionByDataIndex(x))), 2);
					}
				}

				currently_open_partition = p;
			}

			return &currently_loaded_rules[static_cast<size_t>(rule_code % rules_per_partition)];
		}
		catch (const std::ifstream::failure&) {
			std::cout << "Binary rule " << rule_code << " couldn't be loaded from binary rule file (stream failure).\n";
		}
		catch (const std::runtime_error&) {
			std::cout << "Binary rule " << rule_code << " couldn't be loaded from binary rule file (runtime error).\n";
		}
		std::cerr << "Rule lookup from binary rule file failed. Rule code: " << rule_code << std::endl;
		throw std::runtime_error("Rule lookup from binary rule file failed.");
	}

    virtual rule_set GenerateRuleSet() = 0;

	virtual action_bitset GetActionFromRuleIndex(const rule_set& rs, uint64_t rule_index) const = 0;

};

#endif //GRAPHGEN_BASE_RULESET_H_