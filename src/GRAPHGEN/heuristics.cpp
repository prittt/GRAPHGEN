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

constexpr std::array<const char*, 3> HDT_ACTION_SOURCE_STRINGS = { "Memory (pre-generated or read from rule file)", "Generation during run-time", "Binary rule files" };

#define HDT_INFORMATION_GAIN_METHOD_VERSION 2
#define HDT_COMBINED_CLASSIFIER true
#define HDT_ACTION_SOURCE 2

constexpr auto CONDITION_COUNT = 16; // 8, 14, 16, 36
constexpr auto ACTION_COUNT = 16; // 5, 77, 16, 5813

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

void FindBestSingleActionCombinationRunningCombined(
	std::vector<int>& all_single_actions,
	std::vector<std::vector<int>>& single_actions,
	action_bitset* combined_action,
	const ullong& rule_code) {

	int most_popular_single_action_occurences = -1;
	int most_popular_single_action_index = -1;

	for (const auto& s : combined_action->getSingleActions()) { 
		if (all_single_actions[s] > most_popular_single_action_occurences) {
			most_popular_single_action_index = s;
			most_popular_single_action_occurences = all_single_actions[s];
		}
	}
	all_single_actions[most_popular_single_action_index]++;

	int i = 0;
	for (auto& s : single_actions) {
		if (((rule_code >> (i / 2)) & 1) == (i % 2)) {
			s[most_popular_single_action_index]++;
		}
		i++;
	}
}

uint64_t total_rule_accesses = 0;
uint64_t necessary_rule_accesses = 0;

struct RecursionInstance {
	// parameters
	std::vector<int> conditions;
	ullong set_conditions0;
	ullong set_conditions1;
	BinaryDrag<conact>::node* parent;

	// local vars
	std::vector<std::vector<int>> single_actions;
	std::vector<int> all_single_actions = std::vector<int>(ACTION_COUNT); // TODO: Optimize this (also for single_actions), since not all actions are ever used -> lots of unused space and allocations
#if HDT_COMBINED_CLASSIFIER == false
	std::vector<std::array<int, 2>> most_probable_action_index_;
	std::vector<std::array<int, 2>> most_probable_action_occurences_;
#endif
	// state
	bool processed = false;

	RecursionInstance(std::vector<int> conditions,
		ullong set_conditions0,
		ullong set_conditions1,
		BinaryDrag<conact>::node* parent) : 
			conditions(conditions),
			set_conditions0(set_conditions0),
			set_conditions1(set_conditions1),
			parent(parent)
	{
		single_actions.resize(CONDITION_COUNT * 2, std::vector<int>(ACTION_COUNT));
#if HDT_COMBINED_CLASSIFIER == false
		most_probable_action_index_.resize(CONDITION_COUNT, std::array<int, 2>());
		most_probable_action_occurences_.resize(CONDITION_COUNT, std::array<int, 2>());
#endif
	}
};

void HdtReadAndApplyRulesOnePass(BaseRuleSet& brs, rule_set& rs, std::vector<RecursionInstance>& r_insts) {
	bool first_match;
	action_bitset* action;

	for (ullong rule_code = 0; rule_code < (1ULL << CONDITION_COUNT); rule_code += 1) {
		//if (rule_code % (1ULL << 12) == 0) {
		//	std::cout << "Rule " << rule_code << " of " << (1ULL << condition_count) << " (" << 100 * rule_code / (1ULL << condition_count) << "%)." << std::endl;
		//}
		first_match = true;
		for (auto& r : r_insts) {
			if (((rule_code & r.set_conditions0) == 0ULL) && ((rule_code & r.set_conditions1) == r.set_conditions1)) {
				if (first_match) {
#if (HDT_ACTION_SOURCE == 0)
					action = &rs.rules[rule_code].actions;				// 0) load from rule table
#elif (HDT_ACTION_SOURCE == 1)
					action = &brs.GetActionFromRuleIndex(rs, rule_code);	// 1) generate during run-time
#elif (HDT_ACTION_SOURCE == 2)
					action = brs.LoadRuleFromBinaryRuleFiles(rule_code);	// 2) read from file
#endif
					total_rule_accesses++;
					necessary_rule_accesses++;
					first_match = false;
				}
#if HDT_COMBINED_CLASSIFIER == true
				FindBestSingleActionCombinationRunningCombined(
					r.all_single_actions,
					r.single_actions,
					action,
					rule_code);
#else
				FindBestSingleActionCombinationRunning(r.all_single_actions, action);

				for (auto& c : r.conditions) {
					int bit_value = (rule_code >> c) & 1;

					int return_code = FindBestSingleActionCombinationRunning(r.single_actions[c][bit_value], action, r.most_probable_action_occurences_[c][bit_value]);

					if (return_code >= 0) {
						{
							r.most_probable_action_index_[c][bit_value] = return_code;
							r.most_probable_action_occurences_[c][bit_value] = r.single_actions[c][bit_value][return_code];
						}
					}
				}
#endif
			}
		}
	}
}

//void HdtReadAndApplyRulesSingle(BaseRuleSet& brs, const rule_set& rs, RecursionInstance& r) {
//	//const int64_t iteration_step = (1ULL << std::max(0, static_cast<int>(conditions.size() - 3)));
//	const int iteration_step = 1;
//
//	//action_bitset action = action_bitset().set(0); // 1) Use Zero Action (performance baseline)
//
//	for (ullong rule_code = 0; rule_code < (1ULL << CONDITION_COUNT); rule_code += iteration_step) {
//		/*if (rule_code % (1ULL << 24) == 0 && rule_code > 0) {
//			std::cout << "Rule " << rule_code << " of " << (1ULL << rs.conditions.size()) << " (" << 100 * rule_code / (1ULL << rs.conditions.size()) << "%)." << std::endl;
//		}*/
//
//		if ((rule_code & r.set_conditions1.to_ullong()) != r.set_conditions1.to_ullong()) {
//			continue;
//		}
//		if ((rule_code & r.set_conditions0.to_ullong()) != 0u) {
//			continue;
//		}
//
//		total_rule_accesses++;
//		necessary_rule_accesses++;
//
//		//action_bitset action = rs.rules[rule_code].actions;				// 2) load from rule table
//		//action_bitset action = brs.GetActionFromRuleIndex(rs, rule_code);	// 3) generate during run-time
//		action_bitset action = brs.LoadRuleFromBinaryRuleFiles(rule_code);	// 4) read from file
//
//		FindBestSingleActionCombinationRunning(r.all_single_actions, action);
//
//		for (auto& c : r.conditions) {
//			int bit_value = (rule_code >> c) & 1;
//
//			int return_code = FindBestSingleActionCombinationRunning(r.single_actions[c][bit_value], action, r.most_probable_action_occurences_[c][bit_value]);
//
//			if (return_code >= 0) {
//				{
//					r.most_probable_action_index_[c][bit_value] = return_code;
//					r.most_probable_action_occurences_[c][bit_value] = r.single_actions[c][bit_value][return_code];
//				}
//			}
//		}
//	}
//
//}

uint getFirstCountedAction(std::vector<int> b) {
	for (size_t i = 0; i < b.size(); i++) {
		if (b[i] > 0) {
			return i;
		}
	}
	std::cerr << "getFirstCountedAction called with empty vector" << std::endl;
	throw std::runtime_error("getFirstCountedAction called with empty vector");
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
		auto leftAction = action_bitset().set(getFirstCountedAction(r.single_actions[r.conditions[0] * 2]));
		leftNode->data.t = conact::type::ACTION;
		leftNode->data.action = leftAction;
		auto rightAction = action_bitset().set(getFirstCountedAction(r.single_actions[r.conditions[0] * 2]));
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

	bool leftIsAction = false;
	bool rightIsAction = false;

	for (auto& c : r.conditions) {
		double leftEntropy = entropy(r.single_actions[c * 2]);
		double rightEntropy = entropy(r.single_actions[c * 2 + 1]);

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
			leftIsAction = (leftEntropy == 0);
			rightIsAction = (rightEntropy == 0);
		}
		//std::cout << "Condition: " << c << " Max information gain: " << informationGain << " Ratio: " << difference << " LIG*RIG: " << LigTimesRig << "\tEntropy Left (0): " << leftEntropy << "\tEntropy Right (1): " << rightEntropy << std::endl;
	}
	//std::cout << "Split candidate chosen: " << splitCandidate << std::endl;


	r.conditions.erase(std::remove(r.conditions.begin(), r.conditions.end(), splitCandidate), r.conditions.end());

	r.parent->data.t = conact::type::CONDITION;
	r.parent->data.condition = rs.conditions[splitCandidate];

	if (leftIsAction) {
		r.parent->left = tree.make_node();
		r.parent->left->data.t = conact::type::ACTION;
		r.parent->left->data.action = action_bitset().set(getFirstCountedAction(r.single_actions[splitCandidate * 2]));
		amount_of_action_children++;
	}
	else {
		r.parent->left = tree.make_node();
		auto newConditions0 = r.set_conditions0 | (1ULL << splitCandidate);
		upcoming_recursion_instances.push_back(RecursionInstance(r.conditions, newConditions0, r.set_conditions1, r.parent->left));
	}

	if (rightIsAction) {
		r.parent->right = tree.make_node();
		r.parent->right->data.t = conact::type::ACTION;
		r.parent->right->data.action = action_bitset().set(getFirstCountedAction(r.single_actions[splitCandidate * 2 + 1]));
		amount_of_action_children++;
	}
	else {
		r.parent->right = tree.make_node();
		auto newConditions1 = r.set_conditions1 | (1ULL << splitCandidate);
		upcoming_recursion_instances.push_back(RecursionInstance(r.conditions, r.set_conditions0, newConditions1, r.parent->right));
	}
	r.processed = true;
	return amount_of_action_children;
}

void FindHdtIteratively(rule_set& rs, 
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

	bool b3 = (HDT_INFORMATION_GAIN_METHOD_VERSION >= 1 && HDT_INFORMATION_GAIN_METHOD_VERSION <= 3);
	bool b4 = (HDT_ACTION_SOURCE >= 0 && HDT_ACTION_SOURCE <= 2);
	
	if (!(b3 && b4)) {
		std::cerr << "Assert failed: check HDT_ACTION_SOURCE and HDT_INFORMATION_GAIN_METHOD_VERSION." << std::endl;
		throw std::runtime_error("Assert failed: check HDT_ACTION_SOURCE and HDT_INFORMATION_GAIN_METHOD_VERSION.");
	}

	std::cout << "Information gain method version: [" << HDT_INFORMATION_GAIN_METHOD_VERSION << "]" << std::endl;

	brs.OpenRuleFiles();
	brs.VerifyRuleFiles();

	auto r = RecursionInstance(conditions, 0, 0, parent);

	FindHdtIteratively(const_cast<rule_set&>(rs), brs, tree, r);

	std::cout << "Total rule accesses: " << total_rule_accesses << " - Necessary rule accesses : " << necessary_rule_accesses << "\n";
	std::cout << "Information gain method version: [" << HDT_INFORMATION_GAIN_METHOD_VERSION << "]" << std::endl;
	std::cout << "Combined classifier enabled: [" << (HDT_COMBINED_CLASSIFIER ? "Yes" : "No") << "]" << std::endl;
	std::cout << "Action source: [" << HDT_ACTION_SOURCE_STRINGS[HDT_ACTION_SOURCE] << "]" << std::endl;
#ifndef NDEBUG
	std::cout << "Build: [Debug]" << std::endl;
#else
	std::cout << "Build: [Release]" << std::endl;
#endif

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
