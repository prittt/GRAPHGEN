// Copyright(c) 2018 - 2019 Costantino Grana, Federico Bolelli 
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

#ifndef GRAPHGEN_RULE_SET_H_
#define GRAPHGEN_RULE_SET_H_

#include <bitset>
#include <fstream>
#include <iterator>
#include <ostream>
#include <unordered_map>

#include "pixel_set.h"
#include "utilities.h"

struct rule {
    unsigned long long frequency = 1;
    action_bitset actions; // bitmapped
};

// Accrocchio per capire come mai pred va meglio di spaghetti su granularity a frequenze tra 9 e 19
struct rule2 {
    rule r_;
    size_t condition_outcome_;
};

struct rule_set {
    std::vector<std::string> conditions;
    std::unordered_map<std::string, uint> conditions_pos;
    std::vector<std::string> actions;
    std::unordered_map<std::string, uint> actions_pos;
    std::vector<rule> rules;
    pixel_set ps_;

    //std::vector<rule2> GetMaxFreqRules(int N) {
    //	std::bitset<128> no_action = 1;
    //	std::bitset<128> new_label = 2;

    //	std::vector<rule2> new_rules;
    //	for (size_t i = 0; i < rules.size(); ++i) {
    //		if (rules[i].actions != no_action && rules[i].actions != new_label) {
    //			new_rules.push_back({ rules[i], i });
    //		}
    //	}

    //	std::sort(new_rules.begin(), new_rules.end(), [](const rule2& a, const rule2& b) -> bool { return a.r_.frequency > b.r_.frequency; });
    //	new_rules.resize(N);
    //	return new_rules;

    //}

    rule_set() {}
    rule_set(YAML::Node& node) {
        Deserialize(node);
    }

    static std::string binary(uint u, uint nbits, const std::string& separator = "") {
        std::string s;
        while (nbits-- > 0) {
            s += ((u >> nbits) & 1) + 48;
            s += separator;
        }
        return s;
    }

    void AddCondition(const std::string& name) {
        conditions.emplace_back(name);
        conditions_pos[name] = conditions.size() - 1;
    }

    void ClearConditions() {
        conditions.clear();
        conditions_pos.clear();
    }

    void InitConditions(const pixel_set& ps) {
        ps_ = ps;
        ClearConditions();
        for (uint i = 0; i < ps.pixels_.size(); ++i) {
            AddCondition(ps.pixels_[i].name_);
        }
    }

    void AddAction(const std::string& action) {
        actions.emplace_back(action);
        actions_pos[action] = actions.size(); // Action 0 doesn't exist
    }

    void ClearActions() {
        actions.clear();
        actions_pos.clear();
    }

    void InitActions(std::vector<std::string> actions_) {
        actions = std::move(actions_);
        actions_pos.clear();
        for (uint i = 0; i < actions.size(); ++i)
            actions_pos[actions[i]] = i + 1; // Action 0 doesn't exist
    }

    uint GetNumberOfRules() const {
        return 1 << conditions.size();
    };

    template<typename T>
    void generate_rules(T fn) {
        uint nrules = 1 << conditions.size();
        rules.resize(nrules);
        for (uint i = 0; i < nrules; ++i) {
            fn(*this, i);
        }
    }

    void print_rules(std::ostream& os) const {
        copy(std::rbegin(conditions), std::rend(conditions), std::ostream_iterator<std::string>(os, "\t"));
        os << "\n";
        for (size_t i = 0; i < rules.size(); ++i) {
            os << binary(i, conditions.size(), "\t") << ": ";
            for (uint j = 0; j < actions.size(); ++j)
                if (rules[i].actions[j])
                    os << actions[j] << "(" << actions_pos.at(actions[j]) << ")" << ", ";
            os << "\n";
        }
    }

    uint get_condition(const std::string& s, ulong rule) const {
        return (rule >> conditions_pos.at(s)) & 1;
    }

    void set_action(const std::string& s, uint rule) {
        rules[rule].actions.set(actions_pos.at(s) - 1);
    }
    void SetFrequency(uint rule, uint frequency) {
        // Da migliorare: chi assicura che vi sia corrispondenza nella rappresentazione
        // delle regole usata all'esterno e quelle implementata dal rule_set?
        rules[rule].frequency = frequency;
    }

    void StoreFrequenciesOnFile(const std::string &output_file) const {
        std::ofstream os(output_file);
        if (!os.is_open()) {
            return;
        }

        for (size_t i = 0; i < rules.size(); ++i) {
            os << rules[i].frequency << "\n";
        }
    }

    bool LoadFrequenciesFromFile(const std::string &file_path) {
        std::ifstream is(file_path);
        if (!is.is_open()) {
            return false;
        }

        int i = 0;
        unsigned long long freq;
        while (is >> freq) {
            rules[i].frequency = freq;
            ++i;
        }

        return true;
    }

    YAML::Node Serialize() const {
        YAML::Node rs_node;
        rs_node["pixel_set"] = ps_.Serialize();

        for (const auto& c : conditions) {
            rs_node["conditions"].push_back(c);
        }

        for (const auto& a : actions) {
            rs_node["actions"].push_back(a);
        }

        bool with_freq = false;
        for (unsigned i = 0; i < rules.size(); ++i) {
            for (uint j = 0; j < actions.size(); ++j) {
                if (rules[i].actions[j]) {
                    rs_node["rules"][i].push_back(actions_pos.at(actions[j]));
                }
            }
            rs_node["frequencies"].push_back(rules[i].frequency);
            if (rules[i].frequency != 1) {
                with_freq = true;
            }
        }

        if (!with_freq) {
            rs_node.remove("frequencies");
        }

        return rs_node;
    }

    void Deserialize(const YAML::Node& rs_node) {
        /*std::vector<rule> rules;*/

        ps_ = pixel_set(rs_node["pixel_set"]);

        for (unsigned i = 0; i < rs_node["conditions"].size(); ++i) {
            AddCondition(rs_node["conditions"][i].as<std::string>());
        }

        for (unsigned i = 0; i < rs_node["actions"].size(); ++i) {
            AddAction(rs_node["actions"][i].as<std::string>());
        }

        rules.resize(rs_node["rules"].size());
        for (unsigned i = 0; i < rs_node["rules"].size(); ++i) {
            for (unsigned j = 0; j < rs_node["rules"][i].size(); ++j) {
                rules[i].actions.set(rs_node["rules"][i][j].as<int>() - 1);
                if (auto& frequencies = rs_node["frequencies"]) {
                    rules[i].frequency = frequencies[i].as<unsigned long long>();
                }
            }
        }

    }

};

struct rule_wrapper {
	const rule_set& rs_;
	ulong i_;
    rule_wrapper(const rule_set& rs, ulong i) : rs_{ rs }, i_{ i } {}

    bool operator[](const std::string& s) const {
        return rs_.get_condition(s, i_) != 0;
    }
    void operator<<(const std::string& s) {
        const_cast<rule_set&>(rs_).set_action(s, i_);
    }
    /*bool has_actions() {
        return rs_.rules[i_].actions != 0;
    }*/
};

#endif // !GRAPHGEN_RULE_SET_H_