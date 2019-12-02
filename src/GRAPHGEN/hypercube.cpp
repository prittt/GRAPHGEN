// Copyright(c) 2018 Costantino Grana, Federico Bolelli 
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

#include <math.h>
#include <random>
#include <ctime>
#include <algorithm>

#include "hypercube.h"
#include "utilities.h"
#include "drag_statistics.h"

using namespace std;

#pragma region ODT

void CreateTree_rec(BinaryDrag<conact>& t, BinaryDrag<conact>::node *n, const rule_set& rs, const VHyperCube &hcube, const VIndex &idx) {
	VNode node = hcube[idx];
	if (node.uiAction == 0) {
		n->data.t = conact::type::CONDITION;
		n->data.condition = rs.conditions[node.uiMaxGainIndex];

		// Estraggo i due (n-1)-cubi
		string sChild0(idx.GetIndexString());
		string sChild1(sChild0);
		sChild0[node.uiMaxGainIndex] = '0';
		sChild1[node.uiMaxGainIndex] = '1';
		VIndex idx0(sChild0), idx1(sChild1);

		CreateTree_rec(t, n->left = t.make_node(), rs, hcube, idx0);
		CreateTree_rec(t, n->right = t.make_node(), rs, hcube, idx1);
	}
	else {
		n->data.t = conact::type::ACTION;
		n->data.action = node.uiAction;
	}
}

BinaryDrag<conact> VHyperCube::optimize(bool bVerbose)
{
	std::string s;
	s.resize(m_iDim, '0');
	VIndex idx(s);
	if (bVerbose) {
		// Stampo la tabella
		do {
			std::cout << idx.GetIndexString() << "\t" << m_arrIndex[idx.GetIndex()].uiProb << "\t";
			if (m_arrIndex[idx.GetIndex()].uiAction == 0)
				std::cout << "0";
			else
				for (unsigned i = 1; i < 32; i++)
					if (m_arrIndex[idx.GetIndex()].uiAction[1 << (i - 1)])
						std::cout << i << ",";
			std::cout << "\n";
		} while (idx.MoveNext());
		std::cout << "------------------------\n";
	}

	for (size_t iNumIndifference = 1; iNumIndifference <= m_iDim; iNumIndifference++) {
		if (!bVerbose)
			std::cout << iNumIndifference << " " << std::flush;
		// Inizializzo le indifferenze
		bool bFine;
		std::vector<size_t> arrPosIndifference(iNumIndifference);
		int iPos = 0;
		for (size_t i = 0; i < iNumIndifference; i++) {
			arrPosIndifference[i] = iPos;
			iPos++;
		}

		std::vector</*unsigned*/unsigned long long> arrProb(iNumIndifference);
		std::vector</*unsigned*/unsigned long long> arrGain(iNumIndifference);
		std::vector<unsigned> arrNEq(iNumIndifference);

		// Faccio tutte le combinazioni
		do {
			// Genero la maschera delle indifferenze
			std::string s;
			s.resize(m_iDim, '0');
			for (size_t i = 0; i < iNumIndifference; i++)
				s[arrPosIndifference[i]] = '-';

			// Stampo tutte le combinazioni
			if (!idx.SetIndex(s))
				throw;
			do {
				for (size_t i = 0; i < iNumIndifference; i++) {
					std::string sChild0(idx.GetIndexString());
					std::string sChild1(sChild0);
					sChild0[arrPosIndifference[i]] = '0';
					sChild1[arrPosIndifference[i]] = '1';
					VIndex idx0(sChild0), idx1(sChild1);
					VNode node0(m_arrIndex[idx0.GetIndex()]), node1(m_arrIndex[idx1.GetIndex()]);

					// Faccio l'intersezione delle possibili azioni
					auto uiIntersezione = node0.uiAction & node1.uiAction;

					m_arrIndex[idx.GetIndex()].uiAction = uiIntersezione;
					arrProb[i] = node0.uiProb + node1.uiProb;
					arrGain[i] = node0.uiGain + node1.uiGain;
					arrNEq[i] = node0.neq * node1.neq;

					if (uiIntersezione != 0) {
						arrGain[i] += arrProb[i];
						arrNEq[i] = 0;
					}
				}
				/*unsigned*/unsigned long long uiMaxGain(0);
				/*unsigned*/unsigned long long uiMaxGainProb(0);
				unsigned uiMaxGainIndex(0);
				unsigned uiNEq(0);
				for (size_t i = 0; i < iNumIndifference; i++) {
					if (uiMaxGain <= arrGain[i]) {
						if (uiMaxGain < arrGain[i])
							uiNEq = arrNEq[i];
						else
							uiNEq += arrNEq[i];
						uiMaxGain = arrGain[i];
						uiMaxGainProb = arrProb[i];
						uiMaxGainIndex = arrPosIndifference[i];
					}
				}
				m_arrIndex[idx.GetIndex()].uiGain = uiMaxGain;
				m_arrIndex[idx.GetIndex()].uiProb = uiMaxGainProb;
				m_arrIndex[idx.GetIndex()].uiMaxGainIndex = uiMaxGainIndex;
				m_arrIndex[idx.GetIndex()].neq = std::max(uiNEq, 1u);

				if (bVerbose) {
					std::cout << idx.GetIndexString() << "\t" << m_arrIndex[idx.GetIndex()].uiProb << "\t";
					if (m_arrIndex[idx.GetIndex()].uiAction == 0)
						std::cout << "0";
					else
						for (unsigned j = 1; j < 32; j++)
							if (m_arrIndex[idx.GetIndex()].uiAction[1 << (j - 1)])
								std::cout << j << ",";
					std::cout << "\t";

					for (size_t i = 0; i < iNumIndifference; i++) {
						std::cout << arrGain[i];
						if (arrPosIndifference[i] == uiMaxGainIndex)
							std::cout << "*";
						else if (arrGain[i] == uiMaxGain)
							std::cout << "#";
						std::cout << "\t";
					}

					std::cout << m_arrIndex[idx.GetIndex()].neq << "\n";
				}
			} while (idx.MoveNext());
			if (bVerbose)
				std::cout << "\n";

			// Passo alla permutazione di indifferenze successiva
			bFine = true;
			for (int i = iNumIndifference - 1; i >= 0; i--) {
				arrPosIndifference[i]++;
				// Ho una posizione valida?
				if (arrPosIndifference[i] < m_iDim) {
					// Ci stanno le altre indifferenze?
					if (m_iDim - 1 - arrPosIndifference[i] >= iNumIndifference - 1 - i) {
						// La posizione � valida, ci stanno le altre, allora sistemo 
						// le indifferenze successive
						iPos = arrPosIndifference[i] + 1;
						for (size_t j = i + 1; j < iNumIndifference; j++) {
							arrPosIndifference[j] = iPos;
							iPos++;
						}
						bFine = false;
						break;
					}
				}
			}
		} while (!bFine);
		if (bVerbose)
			std::cout << "------------------------\n";
	}

	BinaryDrag<conact> t;
	CreateTree_rec(t, t.make_root(), m_rs, *this, string(m_iDim, '-'));
	return t;
}

BinaryDrag<conact> GenerateOdt(const rule_set& rs) {
	TLOG("Allocating hypercube",
		VHyperCube hcube(rs);
	);

	TLOG("Optimizing rules",
		auto t = hcube.optimize(false);
	);

	return t;
}

BinaryDrag<conact> GenerateOdt(const rule_set& rs, const string& filename)
{
	auto t = GenerateOdt(rs);
	WriteConactTree(t, filename);
	return t;
}

BinaryDrag<conact> GetOdt(const rule_set& rs, bool force_generation) {
	string odt_filename = conf.odt_path_.string();
	BinaryDrag<conact> t;
	if (conf.force_odt_generation_ || force_generation || !LoadConactTree(t, odt_filename)) {
		t = GenerateOdt(rs, odt_filename);
	}
	return t;
}

#pragma endregion


#define CONDITION_COUNT 16
#define ACTION_COUNT 16

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


double entropy(std::unordered_map<int, int>& map) {
	double s = 0, h = 0;
	for (const auto& x : map) {
		s += x.second;
		h += x.second * log2(x.second);
	}
	return log2(s) - h / s;
}

enum Classifier {
	Popularity,
	GreedyAscending,
	Random,
};

Classifier currentClassifier = Classifier::Popularity;

template <int maxActions>
std::unordered_map<int, int> FindBestSingleActionCombination(std::vector<action_bitset>& combined_actions) {
	std::unordered_map<int, int> single_actions;
	std::vector<std::pair<int, int>> singleActionCount;

	for (int i = 0; i < maxActions; i++) {
		singleActionCount.push_back(std::pair(i, 0));
	}

	if (currentClassifier == Classifier::Popularity) {
		for (size_t i = 0; i < combined_actions.size(); i++) {
			for (int bit_index = 0; bit_index < maxActions; bit_index++) {
				if (combined_actions[i].test(bit_index)) {
					singleActionCount[bit_index].second++;
				}
			}
		}
		sort(singleActionCount.begin(), singleActionCount.end(),
			[](const std::pair<int, int> & a, const std::pair<int, int> & b) -> bool
		{
			return a.second > b.second;
		});
	}

	if (currentClassifier == Classifier::Random) {
		auto rng = std::mt19937(static_cast<unsigned int>(std::time(nullptr)));
		std::shuffle(singleActionCount.begin(), singleActionCount.end(), rng);
	}

	if (currentClassifier == Classifier::GreedyAscending) {
		// do nothing
	}
	
	for (auto& r : combined_actions) {
		for (auto& x : singleActionCount) {
			if (r.test(x.first)) {
				single_actions[x.first]++;
				break;
			}
		}
	}


	return single_actions;
}

template <int maxActions>
int FindBestSingleActionCombinationRunning(std::vector<int>& single_actions, action_bitset& combined_action, int previous_most_probable_action_occurences = -1) {
	int most_popular_single_action_occurences = -1;
	int most_popular_single_action_index = -1;

	for (int bit_index = 0; bit_index < maxActions; bit_index++) {
		if (combined_action.test(bit_index)) {
			if (single_actions[bit_index] > most_popular_single_action_occurences) {
				most_popular_single_action_index = bit_index;
				most_popular_single_action_occurences = single_actions[bit_index];
			}
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
uint64_t node_number = 0;

struct RecursionInstance {
	// parameters
	std::vector<std::string> conditions;
	std::bitset<CONDITION_COUNT> set_conditions0;
	std::bitset<CONDITION_COUNT> set_conditions1;
	BinaryDrag<conact>::node* parent;

	// local vars
	std::unordered_map<string, std::array<std::vector<int>, 2>> single_actions;
	std::vector<int> all_single_actions = std::vector<int>(ACTION_COUNT);
	std::map<std::string, std::array<int, 2>> most_probable_action_index;
	std::map<std::string, std::array<int, 2>> most_probable_action_occurences;

	// state
	bool processed = false;


	RecursionInstance(std::vector<std::string> conditions,
		std::bitset<CONDITION_COUNT> set_conditions0,
		std::bitset<CONDITION_COUNT> set_conditions1,
		BinaryDrag<conact>::node* parent) : 
			conditions(conditions),
			set_conditions0(set_conditions0),
			set_conditions1(set_conditions1),
			parent(parent)
	{
		for (auto& c : conditions) {
			single_actions.emplace(c, std::array<std::vector<int>, 2>({
				std::vector<int>(ACTION_COUNT),
				std::vector<int>(ACTION_COUNT)
				}));
		}
	}

};

void HdtReadAndApplyRulesOnePass(BaseRuleSet& brs, const rule_set& rs, std::vector<RecursionInstance>& recursion_instances) {
	//const int64_t iteration_step = (1ULL << std::max(0, static_cast<int>(conditions.size() - 3)));
	const int iteration_step = 1;

	//action_bitset action = action_bitset().set(0); // 1) Use Zero Action (performance baseline)

	for (ullong rule_code = 0; rule_code < (1ULL << CONDITION_COUNT); rule_code += iteration_step) {
		/*if (rule_code % (1ULL << 12) == 0) {
			std::cout << "[Node " << node_number << "] Rule " << rule_code << " of " << (1ULL << condition_count) << " (" << 100 * rule_code / (1ULL << condition_count) << "%)." << std::endl;
		}*/

		//action_bitset action = rs.rules[rule_code].actions;				// 2) load from rule table
		//action_bitset action = brs.GetActionFromRuleIndex(rs, rule_code);	// 3) generate during run-time
		action_bitset action = brs.LoadRuleFromBinaryRuleFile(rule_code);	// 4) read from file

		total_rule_accesses++;

		for (auto& r : recursion_instances) {
			if ((rule_code & r.set_conditions1.to_ullong()) != r.set_conditions1.to_ullong()) {
				continue;
			}
			if ((rule_code & r.set_conditions0.to_ullong()) != 0u) {
				continue;
			}

			necessary_rule_accesses++;

			FindBestSingleActionCombinationRunning<ACTION_COUNT>(r.all_single_actions, action);

			for (auto& c : r.conditions) {
				int bit_value = (rule_code >> rs.conditions_pos.at(c)) & 1;

				int return_code = FindBestSingleActionCombinationRunning<ACTION_COUNT>(r.single_actions[c][bit_value], action, r.most_probable_action_occurences[c][bit_value]);

				if (return_code >= 0) {
					{
						r.most_probable_action_index[c][bit_value] = return_code;
						r.most_probable_action_occurences[c][bit_value] = r.single_actions[c][bit_value][return_code];
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
		/*if (rule_code % (1ULL << 12) == 0) {
			std::cout << "[Node " << node_number << "] Rule " << rule_code << " of " << (1ULL << condition_count) << " (" << 100 * rule_code / (1ULL << condition_count) << "%)." << std::endl;
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
		action_bitset action = brs.LoadRuleFromBinaryRuleFile(rule_code);	// 4) read from file

		FindBestSingleActionCombinationRunning<ACTION_COUNT>(r.all_single_actions, action);

		for (auto& c : r.conditions) {
			int bit_value = (rule_code >> rs.conditions_pos.at(c)) & 1;

			int return_code = FindBestSingleActionCombinationRunning<ACTION_COUNT>(r.single_actions[c][bit_value], action, r.most_probable_action_occurences[c][bit_value]);

			if (return_code >= 0) {
				{
					r.most_probable_action_index[c][bit_value] = return_code;
					r.most_probable_action_occurences[c][bit_value] = r.single_actions[c][bit_value][return_code];
				}
			}
		}
	}

}

void HdtProcessNode(RecursionInstance& r, BinaryDrag<conact>& tree, const rule_set& rs, std::vector<RecursionInstance>& upcoming_recursion_instances) {
	for (auto& c : r.conditions) {
		// Case 3: Both childs are leafs, end of recursion
		if (r.conditions.size() == 1) {
			r.parent->data.t = conact::type::CONDITION;
			r.parent->data.condition = r.conditions[0];
			auto leftNode = tree.make_node();
			auto rightNode = tree.make_node();
			r.parent->left = leftNode;
			r.parent->right = rightNode;
			auto leftAction = action_bitset().set(r.most_probable_action_index[r.conditions[0]][0]);
			leftNode->data.t = conact::type::ACTION;
			leftNode->data.action = leftAction;
			auto rightAction = action_bitset().set(r.most_probable_action_index[r.conditions[0]][1]);
			rightNode->data.t = conact::type::ACTION;
			rightNode->data.action = rightAction;
			//std::cout << "Case 3: Both childs are leafs. Condition: " << c << " Left Action: " << leftAction.to_ulong() << " Right Action: " << rightAction.to_ulong() << std::endl;
			return;
		}
	}

	// Case 2: Take best guess (highest p/total occurences), both children are conditions/nodes 
	std::string splitCandidate = r.conditions[0];
	double maximum_information_gain = 0;

	double baseEntropy = entropy(r.all_single_actions);
	//std::cout << "Base Entropy: " << baseEntropy << std::endl;

	for (auto& c : r.conditions) {
		double leftEntropy = entropy(r.single_actions[c][0]);
		double rightEntropy = entropy(r.single_actions[c][1]);

		double informationGain = std::max((baseEntropy - leftEntropy), (baseEntropy - rightEntropy));

		if (informationGain > maximum_information_gain) {
			maximum_information_gain = informationGain;
			splitCandidate = c;
		}
		//std::cout << "Condition: " << c << " Max information gain: "<< informationGain << "\tEntropy Left (0): " << leftEntropy << "\tEntropy Right (1): " << rightEntropy << std::endl;
	}

	bool LeftIsAction = r.most_probable_action_occurences[splitCandidate][0] == (1 << (r.conditions.size() - 1));
	bool RightIsAction = r.most_probable_action_occurences[splitCandidate][1] == (1 << (r.conditions.size() - 1));

	r.conditions.erase(std::remove(r.conditions.begin(), r.conditions.end(), splitCandidate), r.conditions.end());

	r.parent->data.t = conact::type::CONDITION;
	r.parent->data.condition = splitCandidate;

	if (LeftIsAction) {
		r.parent->left = tree.make_node();
		r.parent->left->data.t = conact::type::ACTION;
		r.parent->left->data.action = action_bitset().set(r.most_probable_action_index[splitCandidate][0]);
	}
	else {
		r.parent->left = tree.make_node();
		auto newConditions0 = r.set_conditions0;
		newConditions0[rs.conditions_pos.at(splitCandidate)] = 1;
		upcoming_recursion_instances.push_back(RecursionInstance(r.conditions, newConditions0, r.set_conditions1, r.parent->left));
	}

	if (RightIsAction) {
		r.parent->right = tree.make_node();
		r.parent->right->data.t = conact::type::ACTION;
		r.parent->right->data.action = action_bitset().set(r.most_probable_action_index[splitCandidate][1]);
	}
	else {
		r.parent->right = tree.make_node();
		auto newConditions1 = r.set_conditions1;
		newConditions1[rs.conditions_pos.at(splitCandidate)] = 1;
		upcoming_recursion_instances.push_back(RecursionInstance(r.conditions, r.set_conditions0, newConditions1, r.parent->right));
	}
	r.processed = true;
}


void FindHdtIteratively(std::vector<std::string> conditions, 
	std::bitset<CONDITION_COUNT> set_conditions0, 
	std::bitset<CONDITION_COUNT> set_conditions1, 
	const rule_set& rs, 
	BaseRuleSet& brs,
	BinaryDrag<conact>& tree, 
	BinaryDrag<conact>::node* parent)
{
	std::vector<RecursionInstance> pending_recursion_instances;
	std::vector<RecursionInstance> upcoming_recursion_instances;
	int depth = 0;
	pending_recursion_instances.push_back(RecursionInstance(conditions, set_conditions0, set_conditions1, parent));

	while (pending_recursion_instances.size() > 0) {
		std::cout << "Next batch of recursion instances (count: " << pending_recursion_instances.size() << ", depth: " << depth << ")" << std::endl;

		const ullong estimated_rule_accesses_one_pass = (1ULL << CONDITION_COUNT);
		const ullong estimated_rule_accesses_single = pending_recursion_instances.size() * (1ULL << (CONDITION_COUNT - depth));
		
		std::cout << "Estimated rule accesses: One Pass (" << estimated_rule_accesses_one_pass << "), Single Passes (" << estimated_rule_accesses_single << ") -- ";

		if (estimated_rule_accesses_one_pass < estimated_rule_accesses_single) {
			std::cout << "Reading rules in one pass." << std::endl;
			HdtReadAndApplyRulesOnePass(brs, rs, pending_recursion_instances);
		}
		else {
			std::cout << "Reading rules in single passes." << std::endl;
			for (auto& r : pending_recursion_instances) {
				HdtReadAndApplyRulesSingle(brs, rs, r);
			}
		}
		
		
		for (auto& r : pending_recursion_instances) {
			HdtProcessNode(r, tree, rs, upcoming_recursion_instances);
		}
		pending_recursion_instances = upcoming_recursion_instances;
		upcoming_recursion_instances.clear();
		depth++;
	}
}

BinaryDrag<conact> GenerateHdt(const rule_set& rs, BaseRuleSet& brs) {
	BinaryDrag<conact> t;
	auto parent = t.make_root();

	std::vector<std::string> remaining_conditions = rs.conditions; // copy

	std::bitset<CONDITION_COUNT> set_conditions0, set_conditions1;

	bool b1 = set_conditions0.size() == rs.conditions.size();
	bool b2 = ACTION_COUNT == rs.actions.size();
	bool b3 = ACTION_COUNT <= ACTION_BITSET_SIZE;

	if (!(b1 && b2 && b3)) {
		std::cerr << "Assert failed: check ACTION_COUNT and CONDITION_COUNT." << std::endl;
		throw std::runtime_error("Assert failed: check ACTION_COUNT and CONDITION_COUNT.");
	}

	if (ACTION_COUNT < ACTION_BITSET_SIZE) {
		std::cout << "\nWarning: Bitset containing actions is bigger than required. (" << ACTION_COUNT << " < " << ACTION_BITSET_SIZE << ")" << std::endl;
	}

	brs.OpenBinaryRuleFile();

	FindHdtIteratively(remaining_conditions, set_conditions0, set_conditions1, rs, brs, t, parent);

	std::cout << "Total rule accesses: " << total_rule_accesses << "\n";
	std::cout << "Necessary rule accesses: " << necessary_rule_accesses << "\n";
	return t;
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

BinaryDrag<conact> GetOdtWithFileSuffix(const rule_set& rs, const string& file_suffix, bool force_generation) {
    string odt_filename = conf.GetCustomOdtPath(file_suffix).string();
    BinaryDrag<conact> t;
    if (conf.force_odt_generation_ || force_generation || !LoadConactTree(t, odt_filename)) {
        t = GenerateOdt(rs, odt_filename);
    }
    return t;
}
