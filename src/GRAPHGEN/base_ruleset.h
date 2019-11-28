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
	int binary_rule_file_stream_size = ceil(action_bitset().size() / 8);

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
				throw std::exception((msg).c_str());
			}

            rs_ = GenerateRuleSet();
            SaveRuleSet();
			SaveAllRulesBinary();
        }
        return rs_;
    }

	void SaveAllRulesBinary() {
		std::ofstream os(conf.binary_rule_file_path_, std::ios::binary);
		if (!os) {
			std::cerr << "Could not save binary rule file to " << conf.binary_rule_file_path_ << ".\n";
		}
		else {
			int stream_size = binary_rule_file_stream_size;
			std::for_each(rs_.rules.begin(), rs_.rules.end(), [&os, stream_size](rule r) { os.write(reinterpret_cast<const char*>(&r.actions), stream_size); });
			os.close();
		}
	}

	void OpenBinaryRuleFile() {
		if (binary_rule_file.is_open()) {
			std::cout << "Binary rule file already opened\n";
			return;
		}
		binary_rule_file = std::ifstream(conf.binary_rule_file_path_, std::ios::binary);
		if (!binary_rule_file.is_open()) {
			std::cout << "Could not open binary rule file\n";
			return;
		}
		binary_rule_file.exceptions(std::fstream::badbit | std::fstream::failbit | std::fstream::eofbit);
	}

	action_bitset LoadRuleFromBinaryRuleFile(ulong& rule_code) {
		if (!binary_rule_file.is_open()) {
			std::cout << "Binary rule file not yet opened, no rule reading possible\n";
			return action_bitset();
		}
		try {
			action_bitset action;
			//binary_rule_file.seekg(binary_rule_file_stream_size * rule_code); // 1) absolute seekg
			binary_rule_file.seekg(256 * binary_rule_file_stream_size, std::ios_base::cur); // 2) relative seekg
			//binary_rule_file.ignore(binary_rule_file_stream_size * 256); // 3) ignore "seekg"
			binary_rule_file.read(reinterpret_cast<char*>(&action), binary_rule_file_stream_size);
			return action;
		}
		catch (const std::ifstream::failure&) {
			std::cout << "Binary rule " << rule_code << " couldn't be loaded from binary rule file (stream failure).\n";
		}
		catch (const std::runtime_error&) {
			std::cout << "Binary rule " << rule_code << " couldn't be loaded from binary rule file (runtime error).\n";
		}

	}

    virtual rule_set GenerateRuleSet() = 0;

	virtual action_bitset GetActionFromRuleIndex(const rule_set& rs, uint64_t rule_index) const = 0;

};

#endif //GRAPHGEN_BASE_RULESET_H_