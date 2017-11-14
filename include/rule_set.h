#pragma once

#include "pixel_set.h"
#include <unordered_map>
#include <ostream>

using uint = uint32_t;

std::string binary(uint u, uint nbits);

struct rule {
	uint frequency = 1;
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
			actions_pos[actions[i]] = i + 1;
	}

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
};

