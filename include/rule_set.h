#pragma once

#include "pixel_set.h"
#include <unordered_map>
#include <ostream>

using uint = uint32_t;

std::string binary(uint u, uint nbits);

struct rule {
	unsigned long long frequency = 1;
	uint actions; // bitmapped
};

struct rule_set {
	std::vector<std::string> conditions;
	std::unordered_map<std::string, uint> conditions_pos;
	std::vector<std::string> actions;
	std::unordered_map<std::string, uint> actions_pos;
	std::vector<rule> rules;

	void init_conditions(const pixel_set& ps) {
		conditions.clear();
		conditions_pos.clear();
		for (uint i = 0; i < ps.pixels.size(); ++i) {
			conditions.push_back(ps.pixels[i].name);
			conditions_pos[ps.pixels[i].name] = i;
		}
	}

	void init_actions(std::vector<std::string> actions_) {
		actions = std::move(actions_);
		actions_pos.clear();
		for (uint i = 0; i < actions.size(); ++i)
			actions_pos[actions[i]] = i + 1; // Action 0 doesn't exist
	}

    uint GetNumberOfRules() {
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

	void print_rules(std::ostream& os) const;

	uint get_condition(const std::string& s, uint rule) const {
		return (rule >> conditions_pos.at(s)) & 1;
	}

	void set_action(const std::string& s, uint rule) {
		rules[rule].actions |= 1 << (actions_pos.at(s) - 1);
	}
    void SetFrequency(uint rule, uint frequency) { 
        // Da migliorare: chi assicura che vi sia corrispondenza nella rappresentazione
        // delle regole usata all'esterno e quelle implementata dal rule_set?
        rules[rule].frequency = frequency;
    }
};

struct rule_wrapper {
    rule_set& rs_;
    uint i_;
    rule_wrapper(rule_set& rs, uint i) : rs_{ rs }, i_{ i } {}

    bool operator[](const std::string& s) const {
        return rs_.get_condition(s, i_) != 0;
    }
    void operator<<(const std::string& s) {
        rs_.set_action(s, i_);
    }
    bool has_actions() {
        return rs_.rules[i_].actions != 0;
    }
};
