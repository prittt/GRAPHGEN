#include "rule_set.h"

#include <iterator>

using namespace std;

string binary(uint u, uint nbits) {
	string s;
	while (nbits-->0)
		s += ((u >> nbits) & 1) + 48;
	return s;
}

void rule_set::print_rules(ostream& os) const {
	copy(rbegin(conditions), rend(conditions), ostream_iterator<string>(os, ","));
	os << "\n";
	for (size_t i = 0; i < rules.size(); ++i) {
		os << binary(i, conditions.size()) << " ";
		for (uint j = 0; j < actions.size(); ++j)
			if ((rules[i].actions >> j) & 1)
				os << actions[j] << "(" << actions_pos.at(actions[j]) << ")" << ", ";
		os << "\n";
	}
}

