// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_BASE_RULESET_H_
#define GRAPHGEN_BASE_RULESET_H_

#include <fstream>
#include <filesystem>
#include <string>
#include <utility>

#include "yaml-cpp/yaml.h"

#include "rule_set.h"


/** @brief Is the base class for the RuleSet from which every user-defined RuleSet should inherit

It contains member functions (LoadRuleSet and SaveRuleSet) to load and store a .yaml file which
define the RuleSet.

*/
class BaseRuleSet {
    std::filesystem::path p_;
    bool force_generation_;
	bool disable_generation_;
    rule_set rs_;

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
            SaveRuleSet();
        }
        return rs_;
    }

    virtual rule_set GenerateRuleSet() = 0;

};

#endif //GRAPHGEN_BASE_RULESET_H_