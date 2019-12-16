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

#include "yaml-cpp/yaml.h"
#include "rule_set.h"
#include <zstd.h>

constexpr int partitions = 2048;

class BaseRuleSet {
    std::filesystem::path p_;
    bool force_generation_;
	bool disable_generation_;
    rule_set rs_;

	int binary_rule_file_stream_size = static_cast<int>(ceil(action_bitset::max_size_in_bits() / 8.0));
	std::vector<action_bitset> currently_loaded_rules;
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
            SaveRuleSet();
			TLOG("Writing rules to disk",
				SaveAllRulesBinary();
			);
        }
        return rs_;
    }

	void SaveAllRulesBinary() {
		const ullong rules_per_partition = (1ULL << rs_.conditions.size()) / partitions;
		const ullong partition_size_bytes = binary_rule_file_stream_size * rules_per_partition;
		const ullong partition_size_megabytes = binary_rule_file_stream_size * rules_per_partition / (1024 * 1024);

		std::cout << "[Rule Files] Partitions: " << partitions << " Rulecodes for each partition: " << rules_per_partition << " Estimated partition size (Megabyte): " << partition_size_megabytes << std::endl;
		
		#pragma omp parallel
		{
			ZstdStreamingCompression streamingCompression;
			streamingCompression.allocateResources();
			
			#pragma omp for
			for (int p = 0; p < partitions; p++) {
				const ullong begin_rule_code = p * rules_per_partition;
				const ullong end_rule_code = (p + 1) * rules_per_partition;

				auto path = conf.binary_rule_file_path_partitioned(std::to_string(begin_rule_code) + "-" + std::to_string(end_rule_code));

				// check status of existing files
				if (std::filesystem::exists(path)) {
					std::cout << "[Partition " << p << "] Partition exists: skipping.\n";
					continue;

					/*if (std::filesystem::file_size(path) == partition_size_bytes) {
						std::cout << "[Partition " << p << "] Partition exists and is complete: skipping.\n";
						continue;
					}

					// check if file is locked
					try {
						auto p2 = path + ".test";
						std::filesystem::rename(path, p2);
						std::filesystem::remove(p2);
						std::cout << "[Partition " << p << "] Partition exists, is not complete and not locked: overwriting.\n";
					}
					catch (std::filesystem::filesystem_error e) {
						std::cout << "[Partition " << p << "] Partition exists, is not complete but locked: skipping.\n";
						continue;
					}*/
				}

				//std::ofstream os(path, std::ios::binary);
				if (!streamingCompression.beginStreaming(path)) {
					std::cerr << "[Partition " << p << "] Partition could not be saved to " << path << ": skipping.\n";
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
					const uint batches = 16;
					const ullong batches_steps = rules_per_partition / batches;

					std::vector<action_bitset> actions;
					if (batches_steps > actions.max_size()) {
						std::cerr << "ERROR: Batches are too big to be stored in a vector. (batch size: " << batches_steps << " vector.max_size: " << actions.max_size() << ")" << std::endl;
						exit(EXIT_FAILURE);
					}
					actions.resize(static_cast<size_t>(batches_steps));

					//TLOG("rules batched", 

					for (int b = 0; b < batches; b++) {
						const ullong batch_begin_rule_code = (begin_rule_code + b * batches_steps);
						const ullong batch_end_rule_code = (begin_rule_code + (b + 1) * batches_steps);

						std::cout << "[Partition " << p << "] Processing rule batch " << (b + 1) << " of " << batches << ", rules from " << batch_begin_rule_code << " to " << batch_end_rule_code << ".\n";
						size_t i = 0;
						for (ullong rule_code = batch_begin_rule_code; rule_code < batch_end_rule_code; rule_code++, i++) {
							actions[i] = GetActionFromRuleIndex(rs_, rule_code);
						}
						streamingCompression.compressDataChunk(&actions[0], static_cast<size_t>(batches_steps) * stream_size, b == batches - 1);
						/*for (const action_bitset& a : actions) {
							os.write(reinterpret_cast<const char*>(&a), stream_size);
						}*/
					}
					//);
				}
				streamingCompression.endStreaming();
				//os.close();
			}
			
			streamingCompression.freeResources();
		}		
	}

	void OpenRuleFiles() {
		const ullong rules_per_partition = (1ULL << rs_.conditions.size()) / partitions;
		if (rules_per_partition > currently_loaded_rules.max_size()) {
			std::cerr << "Cannot store 1 partition in memory, aborting. (" << rules_per_partition << " / " << currently_loaded_rules.max_size() << ")" << std::endl;
			throw std::runtime_error("Cannot store 1 partition in memory, aborting.");
		}
		decompression.allocateResources();
		currently_loaded_rules.resize(static_cast<size_t>(rules_per_partition));
		std::cout << "Rule files opened." << std::endl;
	}

	void VerifyRuleFiles() {
		const ullong rules_per_partition = (1ULL << rs_.conditions.size()) / partitions;
		const ullong partition_size_bytes = binary_rule_file_stream_size * rules_per_partition;
		const ullong partition_size_megabytes = binary_rule_file_stream_size * rules_per_partition / (1024 * 1024);
		
		if (rules_per_partition > currently_loaded_rules.max_size()) {
			std::cerr << "Cannot store 1 partition in memory, aborting. (" << rules_per_partition << " / " << currently_loaded_rules.max_size() << ")" << std::endl;
			throw std::runtime_error("Cannot store 1 partition in memory, aborting.");
		}

		std::cout << "** Verifying rule files. (Partitions: " << partitions << " Rulecodes for each partition: " << rules_per_partition << ")" << std::endl;
		
		ZstdDecompression decompression_;
		decompression_.allocateResources();

		for (int p = 0; p < partitions; p++) {
			const ullong begin_rule_code = p * rules_per_partition;
			const ullong end_rule_code = (p + 1) * rules_per_partition;

			auto path = conf.binary_rule_file_path_partitioned(std::to_string(begin_rule_code) + "-" + std::to_string(end_rule_code));

			// check status of existing files
			if (std::filesystem::exists(path)) {
				/*if (std::filesystem::file_size(path) != partition_size_bytes) {
					std::cerr << "Partition " << p << " exists but is not complete: aborting.\n";
					exit(EXIT_FAILURE);
				}*/
			}
			else {
				std::cerr << "Partition " << p << " does not exist: aborting.\n";
				exit(EXIT_FAILURE);
			}

			std::vector<action_bitset> actions;
			actions.resize(static_cast<size_t>(rules_per_partition));

			decompression_.decompressFileToMemory(path, actions);
			//std::ifstream binary_rule_file = std::ifstream(path, std::ios::binary);
			/*if (!binary_rule_file.is_open()) {
				std::cout << "Could not open partition " << p << " at " << path << " \n";
				exit(EXIT_FAILURE);
			}*/
			const action_bitset first_action_correct = GetActionFromRuleIndex(rs_, begin_rule_code);
			const action_bitset last_action_correct = GetActionFromRuleIndex(rs_, end_rule_code - 1);

			const action_bitset first_action_read = actions[0];
			const action_bitset last_action_read = actions[actions.size() - 1];

			if (first_action_correct != first_action_read || last_action_correct != last_action_read) {
				std::cout << "************************************************************" << std::endl;
				std::cout << "Incorrect rule found in rule partition " << p << " at " << path << std::endl;
				std::cout << "Rule Code: " << begin_rule_code << " Correct action: " << first_action_correct.to_string() << "\t Read action: " << first_action_read.to_string() << std::endl;
				std::cout << "Rule Code: " << (end_rule_code - 1) << " Correct action: " << last_action_correct.to_string() << "\t Read action: " << last_action_read.to_string() << std::endl;
				std::cout << "************************************************************" << std::endl;
				exit(EXIT_FAILURE);
			}
		}
		
		decompression_.freeResources();

		std::cout << "** All rule files verified." << std::endl;		
	}

	ullong previous_rule_code = UINT64_MAX;

	action_bitset LoadRuleFromBinaryRuleFiles(ullong& rule_code) {
		const ullong rules_per_partition = (1ULL << rs_.conditions.size()) / partitions;
		const ullong partition_size_bytes = binary_rule_file_stream_size * rules_per_partition;

		try {
			action_bitset action;

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

				//std::ifstream is = std::ifstream(path, std::ios::binary);
				//int stream_size = binary_rule_file_stream_size;
				/*std::for_each(currently_loaded_rules.begin(), currently_loaded_rules.end(), 
					[&is, stream_size](action_bitset& a) { 
					is.read(reinterpret_cast<char*>(&a), stream_size); 
				});*/

				decompression.decompressFileToMemory(path, currently_loaded_rules);

				currently_open_partition = p;
			}

			return currently_loaded_rules[static_cast<size_t>(rule_code % rules_per_partition)];
		}
		catch (const std::ifstream::failure&) {
			std::cout << "Binary rule " << rule_code << " couldn't be loaded from binary rule file (stream failure).\n";
		}
		catch (const std::runtime_error&) {
			std::cout << "Binary rule " << rule_code << " couldn't be loaded from binary rule file (runtime error).\n";
		}
		return action_bitset();
	}

    virtual rule_set GenerateRuleSet() = 0;

	virtual action_bitset GetActionFromRuleIndex(const rule_set& rs, uint64_t rule_index) const = 0;

};

#endif //GRAPHGEN_BASE_RULESET_H_