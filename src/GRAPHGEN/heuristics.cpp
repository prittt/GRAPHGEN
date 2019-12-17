// Copyright(c) 2019 Maximilian Söchting
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

#include <random>
#include <ctime>
#include <algorithm>

#include "heuristics.h"
#include "utilities.h"
#include "drag_statistics.h"

using namespace std;

#define HDT_INFORMATION_GAIN_METHOD_VERSION 2
constexpr auto CONDITION_COUNT = 16;
constexpr auto ACTION_COUNT = 16;

double entropy(std::vector<int>& vector) {
	double s = 0, h = 0;
	for (const auto& x : vector) {
		if (x == 0) {
			continue;
		}
		s += x;
		h += x * log2(x);
	}
	return log2(s) - h / s;
}


//double entropy(std::unordered_map<int, int>& map) {
//	double s = 0, h = 0;
//	for (const auto& x : map) {
//		s += x.second;
//		h += x.second * log2(x.second);
//	}
//	return log2(s) - h / s;
//}
//
//enum Classifier {
//	Popularity,
//	GreedyAscending,
//	Random,
//};
//
//Classifier currentClassifier = Classifier::Popularity;
//
//template <int maxActions>
//std::unordered_map<int, int> FindBestSingleActionCombination(std::vector<action_bitset>& combined_actions) {
//	std::unordered_map<int, int> single_actions;
//	std::vector<std::pair<int, int>> singleActionCount;
//
//	for (int i = 0; i < maxActions; i++) {
//		singleActionCount.push_back(std::pair(i, 0));
//	}
//
//	if (currentClassifier == Classifier::Popularity) {
//		for (size_t i = 0; i < combined_actions.size(); i++) {
//			for (int bit_index = 0; bit_index < maxActions; bit_index++) {
//				if (combined_actions[i].test(bit_index)) {
//					singleActionCount[bit_index].second++;
//				}
//			}
//		}
//		sort(singleActionCount.begin(), singleActionCount.end(),
//			[](const std::pair<int, int> & a, const std::pair<int, int> & b) -> bool
//		{
//			return a.second > b.second;
//		});
//	}
//
//	if (currentClassifier == Classifier::Random) {
//		auto rng = std::mt19937(static_cast<unsigned int>(std::time(nullptr)));
//		std::shuffle(singleActionCount.begin(), singleActionCount.end(), rng);
//	}
//
//	if (currentClassifier == Classifier::GreedyAscending) {
//		// do nothing
//	}
//	
//	for (auto& r : combined_actions) {
//		for (auto& x : singleActionCount) {
//			if (r.test(x.first)) {
//				single_actions[x.first]++;
//				break;
//			}
//		}
//	}
//
//
//	return single_actions;
//}

template <ushort maxActions>
int FindBestSingleActionCombinationRunning(std::vector<int>& single_actions, action_bitset& combined_action, int previous_most_probable_action_occurences = -1) {
	int most_popular_single_action_occurences = -1;
	int most_popular_single_action_index = -1;

	for (ushort i = 0; i < combined_action.size(); i++) {
		auto s = combined_action.getActionByDataIndex(i);
		if (single_actions[s] > most_popular_single_action_occurences) {
			most_popular_single_action_index = s;
			most_popular_single_action_occurences = single_actions[s];
		}
	}

	single_actions[most_popular_single_action_index]++;
	
	if (previous_most_probable_action_occurences < 0) {
		return 0;
	}

	if (single_actions[most_popular_single_action_index] > previous_most_probable_action_occurences) {
		return most_popular_single_action_index;
	}
	return -1;
}

uint64_t total_rule_accesses = 0;
uint64_t necessary_rule_accesses = 0;

struct RecursionInstance {
	// parameters
	std::vector<int> conditions;
	std::bitset<CONDITION_COUNT> set_conditions0;
	std::bitset<CONDITION_COUNT> set_conditions1;
	BinaryDrag<conact>::node* parent;

	// local vars
	std::vector<std::array<std::vector<int>, 2>> single_actions;
	std::vector<int> all_single_actions = std::vector<int>(ACTION_COUNT);
	std::vector<std::array<int, 2>> most_probable_action_index_;
	std::vector<std::array<int, 2>> most_probable_action_occurences_;

	// state
	bool processed = false;

	RecursionInstance(std::vector<int> conditions,
		std::bitset<CONDITION_COUNT> set_conditions0,
		std::bitset<CONDITION_COUNT> set_conditions1,
		BinaryDrag<conact>::node* parent) : 
			conditions(conditions),
			set_conditions0(set_conditions0),
			set_conditions1(set_conditions1),
			parent(parent)
	{
		for (int i = 0; i < CONDITION_COUNT; i++) {
			single_actions.push_back(std::array<std::vector<int>, 2>({
				std::vector<int>(ACTION_COUNT),
				std::vector<int>(ACTION_COUNT)
				}));
			most_probable_action_index_.push_back(std::array<int, 2>());
			most_probable_action_occurences_.push_back(std::array<int, 2>());
		}
	}

};

void HdtReadAndApplyRulesOnePass(BaseRuleSet& brs, const rule_set& rs, std::vector<RecursionInstance>& r_insts) {
	std::vector<bool> rule_matches;
	rule_matches.resize(r_insts.size());
	bool rule_match_with_any;

	for (ullong rule_code = 0; rule_code < (1ULL << CONDITION_COUNT); rule_code += 1) {
		//if (rule_code % (1ULL << 12) == 0) {
		//	std::cout << "Rule " << rule_code << " of " << (1ULL << condition_count) << " (" << 100 * rule_code / (1ULL << condition_count) << "%)." << std::endl;
		//}
		rule_match_with_any = false;
		for (size_t i = 0; i < r_insts.size(); i++) {
			if ((rule_code & r_insts[i].set_conditions1.to_ullong()) != r_insts[i].set_conditions1.to_ullong()) {
				rule_matches[i] = false;
			} else if ((rule_code & r_insts[i].set_conditions0.to_ullong()) != 0u) {
				rule_matches[i] = false;
			} else {
				rule_matches[i] = true;
				rule_match_with_any = true;
			}
		}

		if (!rule_match_with_any) {
			continue;
		}

		//action_bitset action = rs.rules[rule_code].actions;				// 2) load from rule table
		//action_bitset action = brs.GetActionFromRuleIndex(rs, rule_code);	// 3) generate during run-time
		action_bitset action = brs.LoadRuleFromBinaryRuleFiles(rule_code);	// 4) read from file

		total_rule_accesses++;
		necessary_rule_accesses++;

		for (size_t i = 0; i < r_insts.size(); i++) {
			if (!rule_matches[i]) {
				continue;
			}

			FindBestSingleActionCombinationRunning<ACTION_COUNT>(r_insts[i].all_single_actions, action);

			for (auto& c : r_insts[i].conditions) {
				int bit_value = (rule_code >> c) & 1;

				int return_code = FindBestSingleActionCombinationRunning<ACTION_COUNT>(r_insts[i].single_actions[c][bit_value], action, r_insts[i].most_probable_action_occurences_[c][bit_value]);

				if (return_code >= 0) {
					{
						r_insts[i].most_probable_action_index_[c][bit_value] = return_code;
						r_insts[i].most_probable_action_occurences_[c][bit_value] = r_insts[i].single_actions[c][bit_value][return_code];
					}
				}
			}
		}		
	}
}

void HdtReadAndApplyRulesSingle(BaseRuleSet& brs, const rule_set& rs, RecursionInstance& r) {
	//const int64_t iteration_step = (1ULL << std::max(0, static_cast<int>(conditions.size() - 3)));
	const int iteration_step = 1;

	//action_bitset action = action_bitset().set(0); // 1) Use Zero Action (performance baseline)

	for (ullong rule_code = 0; rule_code < (1ULL << CONDITION_COUNT); rule_code += iteration_step) {
		/*if (rule_code % (1ULL << 24) == 0 && rule_code > 0) {
			std::cout << "Rule " << rule_code << " of " << (1ULL << rs.conditions.size()) << " (" << 100 * rule_code / (1ULL << rs.conditions.size()) << "%)." << std::endl;
		}*/

		if ((rule_code & r.set_conditions1.to_ullong()) != r.set_conditions1.to_ullong()) {
			continue;
		}
		if ((rule_code & r.set_conditions0.to_ullong()) != 0u) {
			continue;
		}

		total_rule_accesses++;
		necessary_rule_accesses++;

		//action_bitset action = rs.rules[rule_code].actions;				// 2) load from rule table
		//action_bitset action = brs.GetActionFromRuleIndex(rs, rule_code);	// 3) generate during run-time
		action_bitset action = brs.LoadRuleFromBinaryRuleFiles(rule_code);	// 4) read from file

		FindBestSingleActionCombinationRunning<ACTION_COUNT>(r.all_single_actions, action);

		for (auto& c : r.conditions) {
			int bit_value = (rule_code >> c) & 1;

			int return_code = FindBestSingleActionCombinationRunning<ACTION_COUNT>(r.single_actions[c][bit_value], action, r.most_probable_action_occurences_[c][bit_value]);

			if (return_code >= 0) {
				{
					r.most_probable_action_index_[c][bit_value] = return_code;
					r.most_probable_action_occurences_[c][bit_value] = r.single_actions[c][bit_value][return_code];
				}
			}
		}
	}

}


int HdtProcessNode(RecursionInstance& r, BinaryDrag<conact>& tree, const rule_set& rs, std::vector<RecursionInstance>& upcoming_recursion_instances) {
	int amount_of_action_children = 0;
	
	// Case 3: Both childs are leafs, end of recursion
	if (r.conditions.size() == 1) {
		r.parent->data.t = conact::type::CONDITION;
		r.parent->data.condition = rs.conditions[r.conditions[0]];
		auto leftNode = tree.make_node();
		auto rightNode = tree.make_node();
		r.parent->left = leftNode;
		r.parent->right = rightNode;
		auto leftAction = action_bitset().set(r.most_probable_action_index_[r.conditions[0]][0]);
		leftNode->data.t = conact::type::ACTION;
		leftNode->data.action = leftAction;
		auto rightAction = action_bitset().set(r.most_probable_action_index_[r.conditions[0]][1]);
		rightNode->data.t = conact::type::ACTION;
		rightNode->data.action = rightAction;
		//std::cout << "Case 3: Both childs are leafs. Condition: " << c << " Left Action: " << leftAction.to_ulong() << " Right Action: " << rightAction.to_ulong() << std::endl;
		return 2;
	}

	// Case 2: Take best guess (highest p/total occurences), both children are conditions/nodes 
	int splitCandidate = r.conditions[0];
	double maximum_information_gain = 0;

	double baseEntropy = entropy(r.all_single_actions);
	//std::cout << "Base Entropy: " << baseEntropy << std::endl;

	for (auto& c : r.conditions) {
		double leftEntropy = entropy(r.single_actions[c][0]);
		double rightEntropy = entropy(r.single_actions[c][1]);

#if HDT_INFORMATION_GAIN_METHOD_VERSION == 1
		// 1) Simple Information Gain
		double informationGain = std::max((baseEntropy - leftEntropy), (baseEntropy - rightEntropy));
#elif HDT_INFORMATION_GAIN_METHOD_VERSION == 2
		// 2) Information Gain Sum
		double informationGain = (baseEntropy - leftEntropy) + (baseEntropy - rightEntropy);
#elif HDT_INFORMATION_GAIN_METHOD_VERSION == 3
		// 3) Weighted Information Gain Sum
		double LigTimesRig = (baseEntropy - leftEntropy) + (baseEntropy - rightEntropy); 
		double difference = (std::max(leftEntropy, rightEntropy) - std::min(leftEntropy, rightEntropy)) + 0.001;
		double informationGain = LigTimesRig / std::sqrt(difference);
#endif

		if (informationGain > maximum_information_gain) {
			maximum_information_gain = informationGain;
			splitCandidate = c;
		}
		//std::cout << "Condition: " << c << " Max information gain: " << informationGain << " Ratio: " << difference << " LIG*RIG: " << LigTimesRig << "\tEntropy Left (0): " << leftEntropy << "\tEntropy Right (1): " << rightEntropy << std::endl;
	}
	//std::cout << "Split candidate chosen: " << splitCandidate << std::endl;

	bool LeftIsAction = r.most_probable_action_occurences_[splitCandidate][0] == (1 << (r.conditions.size() - 1));
	bool RightIsAction = r.most_probable_action_occurences_[splitCandidate][1] == (1 << (r.conditions.size() - 1));

	r.conditions.erase(std::remove(r.conditions.begin(), r.conditions.end(), splitCandidate), r.conditions.end());

	r.parent->data.t = conact::type::CONDITION;
	r.parent->data.condition = rs.conditions[splitCandidate];

	if (LeftIsAction) {
		r.parent->left = tree.make_node();
		r.parent->left->data.t = conact::type::ACTION;
		r.parent->left->data.action = action_bitset().set(r.most_probable_action_index_[splitCandidate][0]);
		amount_of_action_children++;
	}
	else {
		r.parent->left = tree.make_node();
		auto newConditions0 = r.set_conditions0;
		newConditions0[splitCandidate] = 1;
		upcoming_recursion_instances.push_back(RecursionInstance(r.conditions, newConditions0, r.set_conditions1, r.parent->left));
	}

	if (RightIsAction) {
		r.parent->right = tree.make_node();
		r.parent->right->data.t = conact::type::ACTION;
		r.parent->right->data.action = action_bitset().set(r.most_probable_action_index_[splitCandidate][1]);
		amount_of_action_children++;
	}
	else {
		r.parent->right = tree.make_node();
		auto newConditions1 = r.set_conditions1;
		newConditions1[splitCandidate] = 1;
		upcoming_recursion_instances.push_back(RecursionInstance(r.conditions, r.set_conditions0, newConditions1, r.parent->right));
	}
	r.processed = true;
	return amount_of_action_children;
}

void FindHdtIteratively(const rule_set& rs, 
	BaseRuleSet& brs,
	BinaryDrag<conact>& tree,
	RecursionInstance& initial_recursion_instance)
{
	std::vector<RecursionInstance> pending_recursion_instances;
	std::vector<RecursionInstance> upcoming_recursion_instances;
	int depth = 0;
	int nodes = 0;
	int path_length_sum = 0;

	pending_recursion_instances.push_back(initial_recursion_instance);

	while (pending_recursion_instances.size() > 0) {
		std::cout << "Processing next batch of recursion instances (depth: " << depth << ", count: " << pending_recursion_instances.size() << ")" << std::endl;

		TLOG("Reading rules and classifying",
			HdtReadAndApplyRulesOnePass(brs, rs, pending_recursion_instances);
		);

		TLOG2("Processing instances", 
			for (auto& r : pending_recursion_instances) {
				int amount_of_action_children = HdtProcessNode(r, tree, rs, upcoming_recursion_instances);
				nodes += amount_of_action_children;
				path_length_sum += (depth + 1) * amount_of_action_children;
			}
		);		

		pending_recursion_instances = upcoming_recursion_instances;
		upcoming_recursion_instances.clear();
		depth++;
		//getchar();
	}
	float average_path_length = path_length_sum / static_cast<float>(nodes);
	std::cout << "HDT construction done. Nodes: " << nodes << " Average path length: " << average_path_length << std::endl;
}

BinaryDrag<conact> GenerateHdt(const rule_set& rs, BaseRuleSet& brs) {
	BinaryDrag<conact> tree;
	auto parent = tree.make_root();

	std::vector<int> conditions;
	conditions.reserve(rs.conditions.size());
	for (auto &c : rs.conditions) {
		conditions.push_back(rs.conditions_pos.at(c));
	}

	std::bitset<CONDITION_COUNT> set_conditions0, set_conditions1;

	bool b1 = set_conditions0.size() == rs.conditions.size();
	bool b2 = ACTION_COUNT == rs.actions.size();

	if (!(b1 && b2)) {
		std::cerr << "Assert failed: check ACTION_COUNT and CONDITION_COUNT." << std::endl;
		throw std::runtime_error("Assert failed: check ACTION_COUNT and CONDITION_COUNT.");
	}

	/*if (ACTION_COUNT < action_bitset::max_size_in_bits()) {
		std::cout << "\nWarning: Bitset containing actions is bigger than required. (" << ACTION_COUNT << " < " << action_bitset::max_size_in_bits() << ")" << std::endl;
	}*/

	std::cout << "Information gain method version: [" << HDT_INFORMATION_GAIN_METHOD_VERSION << "]" << std::endl;

	brs.OpenRuleFiles();
	//brs.VerifyRuleFiles();

	auto r = RecursionInstance(conditions, set_conditions0, set_conditions1, parent);

	FindHdtIteratively(rs, brs, tree, r);

	std::cout << "Total rule accesses: " << total_rule_accesses << "\n";
	std::cout << "Necessary rule accesses: " << necessary_rule_accesses << "\n";
	std::cout << "Information gain method version: [" << HDT_INFORMATION_GAIN_METHOD_VERSION << "]" << std::endl;

	return tree;
}

BinaryDrag<conact> GenerateHdt(const rule_set& rs, const BaseRuleSet& brs, const string& filename)
{
	TLOG("Generating HDT",
		auto t = GenerateHdt(rs, const_cast<BaseRuleSet&>(brs));
	);

	WriteConactTree(t, filename);
	return t;
}

BinaryDrag<conact> GetHdt(const rule_set& rs, const BaseRuleSet& brs, bool force_generation) {
	string hdt_filename = conf.hdt_path_.string();
	BinaryDrag<conact> t;
	if (conf.force_odt_generation_ || force_generation || !LoadConactTree(t, hdt_filename)) {
		t = GenerateHdt(rs, brs, hdt_filename);
	}
	return t;
}
