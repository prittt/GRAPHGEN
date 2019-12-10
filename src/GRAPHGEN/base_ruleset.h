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

#include "yaml-cpp/yaml.h"

#include "rule_set.h"

class BaseRuleSet {
    std::filesystem::path p_;
    bool force_generation_;
	bool disable_generation_;
    rule_set rs_;
	std::ifstream binary_rule_file;
	int binary_rule_file_stream_size = static_cast<int>(ceil(action_bitset::max_size_in_bits() / 8.0));

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
		const ullong partitions = 1024;
		const ullong partition_steps = (1ULL << rs_.conditions.size()) / partitions;
		const ullong partition_size = binary_rule_file_stream_size * partition_steps / (1024 * 1024);

		std::cout << "[Rule Files] Partitions: " << partitions << " Rulecodes for each partition: " << partition_steps << " Estimated partition size (Megabyte): " << partition_size << std::endl;

		#pragma omp parallel for
		for (int p = 0; p < partitions; p++) {
			const ullong begin_rule_code = p * partition_steps;
			const ullong end_rule_code = (p + 1) * partition_steps;
			std::cout << "[Partition " << p << "] First Rule Code: " << begin_rule_code << " Last (exclusive) Rule Code: " << end_rule_code << std::endl;
			
			auto path = conf.binary_rule_file_path_partitioned("BBDT3D", std::to_string(begin_rule_code) + "-" + std::to_string(end_rule_code));
			std::ofstream os(path, std::ios::binary);
			if (!os) {
				std::cerr << "Could not save binary rule file to " << path << ".\n";
				//return; //forbidden by OpenMP lol
			}

			int stream_size = binary_rule_file_stream_size;
			if (rs_.rulesStatus == IN_MEMORY) {
				// rules are in memory
				std::for_each(rs_.rules.begin(), rs_.rules.end(), [&os, stream_size](rule r) { os.write(reinterpret_cast<const char*>(&r.actions), stream_size); });
			}
			else {
				// rules are generated during the writing
				const ullong batches = 8;
				const ullong batches_steps = partition_steps / batches;

				std::vector<action_bitset> actions;
				actions.resize(batches_steps);

				//TLOG("rules batched", 
					
				for (int b = 0; b < batches; b++) {
					const ullong batch_begin_rule_code = (begin_rule_code + b * batches_steps);
					const ullong batch_end_rule_code = (begin_rule_code + (b + 1) * batches_steps);

					//std::cout << "[Partition  " << p << "] Starting rule batch " << (b + 1) << " of " << batches << ", rules from " << batch_begin_rule_code << " to " << batch_end_rule_code << ".\n";
					for (ullong rule_code = batch_begin_rule_code, i = 0; rule_code < batch_end_rule_code; rule_code++, i++) {
						actions[i] = GetActionFromRuleIndex(rs_, rule_code);
					}
					for (const action_bitset& a : actions) {
						os.write(reinterpret_cast<const char*>(&a), stream_size);
					}
				}
				//);
			}
			os.close();			
		}
	}

	void OpenBinaryRuleFile() {
		/*if (binary_rule_file.is_open()) {
			std::cout << "Binary rule file already opened\n";
			return;
		}
		binary_rule_file = std::ifstream(conf.binary_rule_file_path_, std::ios::binary);
		if (!binary_rule_file.is_open()) {
			std::cout << "Could not open binary rule file\n";
			return;
		}
		binary_rule_file.exceptions(std::fstream::badbit | std::fstream::failbit | std::fstream::eofbit);*/
	}

	ullong previous_rule_code = UINT64_MAX;

	action_bitset LoadRuleFromBinaryRuleFile(ullong& rule_code) {
		if (!binary_rule_file.is_open()) {
			std::cout << "Binary rule file not yet opened, no rule reading possible\n";
			return action_bitset();
		}
		try {
			action_bitset action;

			// 2nd condition prevents potential overflow error since first condition would be true for 
			// rule_code = 0 and previous_rule_code = UINT64_MAX since -UINT64_MAX == 1
			if ((rule_code - previous_rule_code) != 1 || previous_rule_code == UINT64_MAX) { 
				binary_rule_file.seekg(binary_rule_file_stream_size * rule_code); // absolute seekg
			}
			
			previous_rule_code = rule_code;

			binary_rule_file.read(reinterpret_cast<char*>(&action), binary_rule_file_stream_size);
			return action;
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