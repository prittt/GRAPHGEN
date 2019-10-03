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
// * Neither the name of GRAPHSGEN nor the names of its
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

#include "hypercube.h"

#include "utilities.h"

using namespace std;

void CreateTree_rec(ltree& t, ltree::node *n, const rule_set& rs, const VHyperCube &hcube, const VIndex &idx) {
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

ltree VHyperCube::optimize(bool bVerbose)
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
			std::cout << iNumIndifference << " ";
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

	ltree t;
	CreateTree_rec(t, t.make_root(), m_rs, *this, string(m_iDim, '-'));
	return t;
}

ltree GenerateOdt(const rule_set& rs) {
    TLOG("Allocating hypercube",
        VHyperCube hcube(rs);
    );

    TLOG("Optimizing rules",
        auto t = hcube.optimize(false);
    );

    return t;
}

ltree GenerateOdt(const rule_set& rs, const string& filename) 
{
    auto t = GenerateOdt(rs);
	WriteConactTree(t, filename);
	return t;
}

ltree GetOdt(const rule_set& rs, const string& algorithm_name, bool force_generation) {
    //string odt_filename = global_output_path.string() + "/" + algorithm_name + "_odt.txt";
    string odt_filename = conf.odt_path_.string();
    ltree t;
    if (force_generation || !LoadConactTree(t, odt_filename)) {
        t = GenerateOdt(rs, odt_filename);
    }
    return t;
}

bool ViolatesSetConditions(int rule_index, std::map<std::string, int>& set_conditions, const rule_set& rs) {
	for (auto& f : set_conditions) {
		std::string tested_condition_char = f.first;
		int tested_condition_power = 1 << rs.conditions_pos.at(tested_condition_char);
		int tested_condition_index = ((rule_index / tested_condition_power) % 2) == 1;
		if (tested_condition_index == f.second) {
			return true;
		}
	}
	return false;
}

void PrintOccurenceMap(std::unordered_map<string, std::array<std::unordered_map<std::bitset<128>, int>, 2>>  &action_occurence_map) 
{
	for (auto& x : action_occurence_map) {
		for (int bit = 0; bit < 2; bit++) {
			for (auto& z : x.second[bit]) {
				std::cout << "" << x.first << bit << " " << z.first.to_ulong() << " " << z.second << std::endl;
			}
		}
	}
}

void FindHdtRecursively(std::vector<std::string> conditions, std::map<std::string, int> set_conditions, const rule_set& rs, ltree& tree, ltree::node* parent) 
{
	std::unordered_map<string, std::array<std::unordered_map<std::bitset<128>, int>, 2>> action_occurence_map;
	std::map<std::string, int> total_probable_action_occurences;

	for (auto c : conditions) {
		int power = 1 << rs.conditions_pos.at(c);
		for (int i = 0; i < rs.rules.size(); ++i) {
			if (ViolatesSetConditions(i, set_conditions, rs)) {
				continue;
			}
			int bit_value = ((i / power) % 2);
			for (int b = 0; b < 128; b++) {
				if (rs.rules[i].actions.test(b)) {
					action_occurence_map[c][bit_value][std::bitset<128>().set(b)]++;
					std::cout << "Condition: " << c << " Bit Value: " << bit_value << " Bitmapped Action: " << rs.rules[i].actions.to_ulong() << " Natural Action: " << b+1 << std::endl;
				}
			}
		}

		// Case 3: Both childs are leafs, end of recursion
		if (conditions.size() == 1) {
			parent->data.t = conact::type::CONDITION;
			parent->data.condition = conditions[0];
			auto leftNode = tree.make_node();
			auto rightNode = tree.make_node();
			parent->left = leftNode;
			parent->right = rightNode;
			auto leftAction = action_occurence_map.at(conditions[0])[0].begin()->first;
			leftNode->data.t = conact::type::ACTION;
			leftNode->data.action = leftAction;
			auto rightAction = action_occurence_map.at(conditions[0])[1].begin()->first;
			rightNode->data.t = conact::type::ACTION;
			rightNode->data.action = rightAction;
			std::cout << "Case 3: Both childs are leafs. Condition: " << c << " Left Action: " << leftAction.to_ulong() << " Right Action: " << rightAction.to_ulong() << std::endl;
			return;
		}

		for (int bit_value = 0; bit_value < 2; bit_value++) {
			std::bitset<128> most_probable_action;
			int most_probable_action_occurences = 0;
			for (auto& x : action_occurence_map[c][bit_value]) {
				if (x.second > most_probable_action_occurences) {
					most_probable_action = x.first;
					most_probable_action_occurences = x.second;
				}
			}
			total_probable_action_occurences[c] += most_probable_action_occurences;

			// Case 1: Definitive Action in one child = 1 leaf/action; other one is condition
			if (most_probable_action_occurences >= (1 << (conditions.size() - 1))) {
				auto actionNode = tree.make_node();
				auto newParent = tree.make_node();
				actionNode->data.t = conact::type::ACTION;
				actionNode->data.action = most_probable_action;
				parent->data.t = conact::type::CONDITION;
				parent->data.condition = c;
				if (bit_value == 0) {
					parent->left = actionNode;
					parent->right = newParent;
				} else {
					parent->left = newParent;
					parent->right = actionNode;
				}

				conditions.erase(std::remove(conditions.begin(), conditions.end(), c), conditions.end());
				set_conditions[c] = bit_value;
				std::cout << "Case 1: Definitive Action in one child; 1 leaf/action, 1 node/condition as children. Condition: " << c << " Action: " << most_probable_action.to_ulong() << std::endl;

				return FindHdtRecursively(conditions, set_conditions, rs, tree, newParent);
			}
		}
	}
	PrintOccurenceMap(action_occurence_map);

	// Case 2: Take best guess (highest p/total occurences), both children are conditions/nodes 
	std::string splitCandidate;
	int max = 0;
	for (auto& x : conditions) {
		if (total_probable_action_occurences[x] > max) {
			max = total_probable_action_occurences[x];
			splitCandidate = x;
		}
	}
	conditions.erase(std::remove(conditions.begin(), conditions.end(), splitCandidate), conditions.end());

	parent->data.t = conact::type::CONDITION;
	parent->data.condition = splitCandidate;
	auto leftNode = tree.make_node();
	auto rightNode = tree.make_node();
	parent->left = leftNode;
	parent->right = rightNode;
	auto conditionsForLeft = set_conditions;
	conditionsForLeft[splitCandidate] = 1;
	auto conditionsForRight = set_conditions;
	conditionsForRight[splitCandidate] = 0;
	std::cout << "Case 2: Take best guess, both children are conditions/nodes. Split Condition: " << splitCandidate << std::endl;

	FindHdtRecursively(conditions, conditionsForLeft, rs, tree, leftNode);
	FindHdtRecursively(conditions, conditionsForRight, rs, tree, rightNode);
}

ltree GenerateHdt(const rule_set& rs) {
	ltree t;
	auto parent = t.make_root();

	std::vector<std::string> remaining_conditions = rs.conditions; // copy
	std::map<std::string, int> set_conditions = std::map<std::string, int>();

	FindHdtRecursively(remaining_conditions, set_conditions, rs, t, parent);
	return t;
}

ltree GenerateHdt(const rule_set& rs, const string& filename)
{
	auto t = GenerateHdt(rs);
	WriteConactTree(t, filename);
	return t;
}

ltree GetHdt(const rule_set& rs, const string& algorithm_name, bool force_generation) {
	//string hdt_filename = global_output_path.string() + "/" + algorithm_name + "_hdt.txt";
	string hdt_filename = conf.hdt_path_.string();
	ltree t;
	if (force_generation || !LoadConactTree(t, hdt_filename)) {
		t = GenerateHdt(rs, hdt_filename);
	}
	return t;
}
