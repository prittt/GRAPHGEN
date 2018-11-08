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

//#include <set>
#include <algorithm>
#include <fstream>
//#include <unordered_map>
//#include <iterator>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdint>
//#include <map>
//#include <unordered_map>
//#include <unordered_set>
#include <intrin.h>

#include "condition_action.h"
#include "code_generator.h"
#include "forest2dag.h"
#include "forest_statistics.h"
#include "drag_statistics.h"
#include "drag2optimal.h"
#include "hypercube.h"
#include "output_generator.h"
#include "ruleset_generator.h"
#include "tree2dag_identities.h"
//#include "utilities.h"

using namespace std;

// Instead of defining a novel format to save DRAGS, we save them as trees, then link
// identical sub-trees. Since the order of traversal is the same, the result should be the same.
// Should...
bool LoadConactDrag(ltree& t, const string& filename)
{
	if (!LoadConactTree(t, filename))
		return false;

	Dag2DagUsingIdenties(t);

	return true;
}
bool WriteConactDrag(ltree& t, const string& filename)
{
	return WriteConactTree(t, filename);
}

void PerformOptimalDragGeneration(ltree& t, const string& algorithm_name)
{
	LOG("Creating DRAG using identites",
		Tree2DagUsingIdentities(t);
	);
	string drag_filename = global_output_path + algorithm_name + "_drag_identities";
	DrawDagOnFile(drag_filename, t, true);
	PrintStats(t);

	string odrag_filename = global_output_path + algorithm_name + "_optimal_drag.txt";
	if (!LoadConactDrag(t, odrag_filename)) {
		TLOG("Computing optimal DRAG\n",
			Dag2OptimalDag(t);
		);
		WriteConactDrag(t, odrag_filename);
	}
	string optimal_drag_filename = global_output_path + algorithm_name + "_optimal_drag";
	DrawDagOnFile(optimal_drag_filename, t, true);
	PrintStats(t);

	LOG("Writing DRAG code",
		{
			string code_filename = global_output_path + algorithm_name + "_drag_code.txt";
			ofstream os(code_filename);
			GenerateCode(os, t);
		}
	);
}

#include "rule_set.h"
// This function .. it works only for 2d and 3d images
void GenerateConditionsActionsCode(ofstream& os, const rule_set& rs) {
	// The names of pointers are identified by the following string
	//	<image_name>_<slice_identifier>_<row_identifier>
	//
	// slice identifiers can be (first number is the sign):
	//	- 'slice00' for the current slice (z)
	//	- 'slice11' for the slice z - 1
	//	- 'slice12' for the slice z - 2
	//  - 'slice01' for the slice z + 1
	//  - 'slice02' for the slice z + 2
	//  - .. and so on
	//
	// row identifiers can be (first number is the sign):
	//	- 'row00' for the current row (y)
	//	- 'row11' for the row y - 1
	//	- 'row12' for the row y - 2
	//  - 'row01' for the row y + 1
	//  - 'row02' for the row y + 2
	//  - .. and so on

	auto& shifts = rs.ps_.shifts_; // Shifts on each dim -> [x, y] or [x, y, z]
	unsigned n_dims = shifts.size(); // Here we get how many dims image has

	stringstream global_ss, in_ss, out_ss;
	string type_in_prefix_string = "const unsigned char* const ";
	string type_out_prefix_string = "unsigned* const ";

	// x is always ignored because we always create row pointers
	switch (n_dims) {
	case 2:
	{
		string base_row_in_name = "img_row00";
		string base_row_out_name = "img_labels_row00";
		string base_row_in = type_in_prefix_string + base_row_in_name + " = img_.ptr<unsigned char>(r);";
		string base_row_out = type_out_prefix_string + base_row_out_name + " = img_labels_.ptr<unsigned>(r);";

		in_ss << "// Row pointers for the input image \n";
		in_ss << base_row_in + "\n";

		out_ss << "// Row pointers for the output image \n";
		out_ss << base_row_out + "\n";

		for (int j = -shifts[1]; j < shifts[1]; ++j) {

			if (j == 0) {
				continue;
			}

			string complete_string_in =
				type_in_prefix_string +
				"img_row" + to_string(((j >> (sizeof(int) - 1)) & 1)) + to_string(abs(j)) +
				" = (unsigned char *)(((char *)" + base_row_in_name + ") + img_.step.p[0] * " + to_string(j) + ");";
			in_ss << complete_string_in + "\n";


			string complete_string_out =
				type_out_prefix_string +
				"img_labels_row" + to_string(((j >> (sizeof(int) - 1)) & 1)) + to_string(abs(j)) +
				" = (unsigned *)(((char *)" + base_row_out_name + ") + img_labels_.step.p[0] * " + to_string(j) + ");";
			out_ss << complete_string_out + "\n";

		}
		break;
	}
	case 3:
	{
		// TODO: this generation only works with unitary shift

		// Current slice
		string base_row_in_name = "img_slice00_row00";
		string base_row_out_name = "img_labels_slice00_row00";
		string base_row_in = type_in_prefix_string + base_row_in_name + " = img_.ptr<unsigned char>(s, r);";
		string base_row_out = type_out_prefix_string + base_row_out_name + " = img_labels_.ptr<unsigned>(s, r);";

		in_ss << "// Row pointers for the input image (current slice) \n";
		in_ss << base_row_in + "\n";

		out_ss << "// Row pointers for the output image (current slice)\n";
		out_ss << base_row_out + "\n";

		for (int j = -shifts[1]; j < shifts[1]; ++j) {

			if (j == 0) {
				continue;
			}

			string complete_string_in =
				type_in_prefix_string +
				"img_slice00_row" + to_string(((j >> (sizeof(int) - 1)) & 1)) + to_string(abs(j)) +
				" = (unsigned char *)(((char *)" + base_row_in_name + ") + img_.step.p[1] * " + to_string(j) + ");";
			in_ss << complete_string_in + "\n";

			string complete_string_out =
				type_out_prefix_string +
				"img_labels_slice00_row" + to_string(((j >> (sizeof(int) - 1)) & 1)) + to_string(abs(j)) +
				" = (unsigned *)(((char *)" + base_row_out_name + ") + img_labels_.step.p[1] * " + to_string(j) + ");";
			out_ss << complete_string_out + "\n";

		}

		in_ss << "\n// Row pointers for the input image (previous slice) \n";
		out_ss << "\n// Row pointers for the output image (previous slice)\n";

		// Previous slice
		base_row_in = type_in_prefix_string + base_row_in_name + " = img_.ptr<unsigned char>(s, r);";
		base_row_out = type_out_prefix_string + base_row_out_name + " = img_labels_.ptr<unsigned>(s, r);";

		for (int j = -shifts[1]; j <= shifts[1]; ++j) {

			string complete_string_in =
				type_in_prefix_string +
				"img_slice11_row" + to_string(((j >> (sizeof(int) - 1)) & 1)) + to_string(abs(j)) +
				" = (unsigned char *)(((char *)" + base_row_in_name + ") - img_.step.p[0] + img_.step.p[1] * " + to_string(j) + ");";
			in_ss << complete_string_in + "\n";

			string complete_string_out =
				type_out_prefix_string +
				"img_labels_slice11_row" + to_string(((j >> (sizeof(int) - 1)) & 1)) + to_string(abs(j)) +
				" = (unsigned *)(((char *)" + base_row_out_name + ") - img_labels_.step.p[0] + img_labels_.step.p[1] * " + to_string(j) + ");";
			out_ss << complete_string_in + "\n";

		}
	}
	break;
	}

	global_ss << in_ss.str() + "\n" + out_ss.str();
	os << global_ss.str() << "\n\n\nConditions:\n";

	// Conditions
	vector<string> counters_names = { "c", "r", "s" };
	vector<string> sizes_names = { "w", "h", "d" };

	for (const auto& p : rs.ps_) {
		string uppercase_name(p.name_);
		transform(p.name_.begin(), p.name_.end(), uppercase_name.begin(), ::toupper);
		os << "#define CONDITION_" + uppercase_name + " ";
		stringstream col;
		for (size_t i = 0; i < n_dims; ++i) {
			if (p.coords_[i] < 0) {
				if (!i) {
					col << "-" << abs(p.coords_[i]);
				}
				os << counters_names[i] << "-" << abs(p.coords_[i]) << ">=0 && ";
			}
			else if (p.coords_[i] > 0) {
				if (!i) {
					col << "+" << p.coords_[i];
				}
				os << counters_names[i] << "+" << p.coords_[i] << "<" + sizes_names[i] + " && ";
			}
		}

		string slice_id = "";
		if (n_dims > 2) {
			slice_id = "slice" + string(p.coords_[2] < 0 ? "1" : "0") + to_string(abs(p.coords_[2])) + "_";
		}

		string row_id = "row" + string(p.coords_[1] < 0 ? "1" : "0") + to_string(abs(p.coords_[1]));

		os << "img_" + slice_id + row_id + "[c" + col.str() + "]>0\n";
	}

	//#define CONDITION_B c-1>=0 && r-2>=0 && img_row_prev_prev[c-1]>0
	//#define CONDITION_C r-2>=0 && img_row_prev_prev[c]>0
	//#define CONDITION_D c+1<w && r-2>=0 && img_row_prev_prev[c+1]>0
	//#define CONDITION_E c+2<w && r-2>=0 && img_row_prev_prev[c+2]>0
	//
	//#define CONDITION_G c-2>=0 && r-1>=0 && img_row_prev[c-2]>0
	//#define CONDITION_H c-1>=0 && r-1>=0 && img_row_prev[c-1]>0
	//#define CONDITION_I r-1>=0 && img_row_prev[c]>0
	//#define CONDITION_J c+1<w && r-1>=0 && img_row_prev[c+1]>0
	//#define CONDITION_K c+2<w && r-1>=0 && img_row_prev[c+2]>0
	//
	//#define CONDITION_M c-2>=0 && img_row[c-2]>0
	//#define CONDITION_N c-1>=0 && img_row[c-1]>0
	//#define CONDITION_O img_row[c]>0
	//#define CONDITION_P c+1<w && img_row[c+1]>0
	//
	//#define CONDITION_R c-1>=0 && r+1<h && img_row_fol[c-1]>0
	//#define CONDITION_S r+1<h && img_row_fol[c]>0
	//#define CONDITION_T c+1<w && r+1<h && img_row_fol[c+1]>0
	//
	//	// Action 1: No action
	//#define ACTION_1 img_labels_row[c] = 0; continue; 
	//	// Action 2: New label (the block has foreground pixels and is not connected to anything else)
	//#define ACTION_2 img_labels_row[c] = LabelsSolver::NewLabel(); continue; 
	//	//Action 3: Assign label of block P
	//#define ACTION_3 img_labels_row[c] = img_labels_row_prev_prev[c - 2]; continue;
	//	// Action 4: Assign label of block Q 
	//#define ACTION_4 img_labels_row[c] = img_labels_row_prev_prev[c]; continue;
	//	// Action 5: Assign label of block R
	//#define ACTION_5 img_labels_row[c] = img_labels_row_prev_prev[c + 2]; continue;
	//	// Action 6: Assign label of block S
	//#define ACTION_6 img_labels_row[c] = img_labels_row[c - 2]; continue; 
	//	// Action 7: Merge labels of block P and Q
	//#define ACTION_7 img_labels_row[c] = LabelsSolver::Merge(img_labels_row_prev_prev[c - 2], img_labels_row_prev_prev[c]); continue;
	//	//Action 8: Merge labels of block P and R
	//#define ACTION_8 img_labels_row[c] = LabelsSolver::Merge(img_labels_row_prev_prev[c - 2], img_labels_row_prev_prev[c + 2]); continue;
	//	// Action 9 Merge labels of block P and S
	//#define ACTION_9 img_labels_row[c] = LabelsSolver::Merge(img_labels_row_prev_prev[c - 2], img_labels_row[c - 2]); continue;
	//	// Action 10 Merge labels of block Q and R
	//#define ACTION_10 img_labels_row[c] = LabelsSolver::Merge(img_labels_row_prev_prev[c], img_labels_row_prev_prev[c + 2]); continue;
	//	// Action 11: Merge labels of block Q and S
	//#define ACTION_11 img_labels_row[c] = LabelsSolver::Merge(img_labels_row_prev_prev[c], img_labels_row[c - 2]); continue;
	//	// Action 12: Merge labels of block R and S
	//#define ACTION_12 img_labels_row[c] = LabelsSolver::Merge(img_labels_row_prev_prev[c + 2], img_labels_row[c - 2]); continue;
	//	// Action 13: not used
	//#define ACTION_13 
	//	// Action 14: Merge labels of block P, Q and S
	//#define ACTION_14 img_labels_row[c] = LabelsSolver::Merge(LabelsSolver::Merge(img_labels_row_prev_prev[c - 2], img_labels_row_prev_prev[c]), img_labels_row[c - 2]); continue;
	//	//Action 15: Merge labels of block P, R and S
	//#define ACTION_15 img_labels_row[c] = LabelsSolver::Merge(LabelsSolver::Merge(img_labels_row_prev_prev[c - 2], img_labels_row_prev_prev[c + 2]), img_labels_row[c - 2]); continue;
	//	//Action 16: labels of block Q, R and S
	//#define ACTION_16 img_labels_row[c] = LabelsSolver::Merge(LabelsSolver::Merge(img_labels_row_prev_prev[c], img_labels_row_prev_prev[c + 2]), img_labels_row[c - 2]); continue;
	//

}

// Overloading function
bool GenerateConditionsActionsCode(const string& filename, const rule_set& rs) {
	ofstream os(filename);

	if (!os) {
		return false;
	}

	GenerateConditionsActionsCode(os, rs);

	return true;
}

int main()
{
	auto at = ruleset_generator_type::rosenfeld_3d;

	auto algorithm_name = ruleset_generator_names[static_cast<int>(at)];
	auto ruleset_generator = ruleset_generator_functions[static_cast<int>(at)];

	auto rs = ruleset_generator();

	GenerateConditionsActionsCode("condition_action.txt", rs);
	return 0;

	string odt_filename = global_output_path + algorithm_name + "_odt.txt";
	ltree t;
	if (!LoadConactTree(t, odt_filename)) {
		t = GenerateOdt(rs, odt_filename);
	}
	string tree_filename = global_output_path + algorithm_name + "_tree";
	DrawDagOnFile(tree_filename, t, false, true);

	string tree_code_filename = global_output_path + algorithm_name + "_code.txt";
	GenerateCode(tree_code_filename, t);

	PrintStats(t);

	//PerformOptimalDragGeneration(t, algorithm_name);
	return 0;

	LOG("Making forest",
		Forest f(t, rs.ps_);
	for (size_t i = 0; i < f.trees_.size(); ++i) {
		DrawDagOnFile(algorithm_name + "tree" + zerostr(i, 4), f.trees_[i], true);
	}
	);
	PrintStats(f);

	for (size_t i = 0; i < f.end_trees_.size(); ++i) {
		for (size_t j = 0; j < f.end_trees_[i].size(); ++j) {
			DrawDagOnFile(algorithm_name + "end_tree_" + zerostr(i, 2) + "_" + zerostr(j, 2), f.end_trees_[i][j], true);
		}
	}

	LOG("Converting forest to dag",
		Forest2Dag x(f);
	for (size_t i = 0; i < f.trees_.size(); ++i) {
		DrawDagOnFile(algorithm_name + "drag" + zerostr(i, 4), f.trees_[i], true);
	}
	);
	PrintStats(f);
	DrawForestOnFile(algorithm_name + "forest", f, true);

	string forest_code = global_output_path + algorithm_name + "_forestidentities_code.txt";
	{
		ofstream os(forest_code);
		GenerateForestCode(os, f);
	}

	struct STree {
		struct STreeProp {
			string conditions;
			vector<std::bitset<128>> actions;
			vector<uint> nexts;
			ltree::node* n_;

			STreeProp& operator+=(const STreeProp& rhs) {
				conditions += rhs.conditions;
				copy(begin(rhs.actions), end(rhs.actions), back_inserter(actions));
				copy(begin(rhs.nexts), end(rhs.nexts), back_inserter(nexts));
				return *this;
			}

			bool operator<(const STreeProp& rhs) {
				if (conditions.size() > rhs.conditions.size())
					return true;
				else if (conditions.size() < rhs.conditions.size())
					return false;
				else if (conditions < rhs.conditions)
					return true;
				else if (conditions > rhs.conditions)
					return false;

				else if (nexts.size() > rhs.nexts.size())
					return true;
				else if (nexts.size() < rhs.nexts.size())
					return false;
				else {
					for (size_t i = 0; i < nexts.size(); ++i)
						if (nexts[i] < rhs.nexts[i])
							return true;
						else if (nexts[i] > rhs.nexts[i])
							return false;
				}

				if (actions.size() > rhs.actions.size())
					return true;
				else if (actions.size() < rhs.actions.size())
					return false;
				else {
					int diff = 0;
					for (size_t i = 0; i < actions.size(); ++i) {
						diff += actions[i].count();
						diff -= rhs.actions[i].count();
					}

					return diff > 0;
				}
			}

			bool equivalent(const STreeProp& rhs) {
				if (conditions != rhs.conditions)
					return false;
				for (size_t i = 0; i < nexts.size(); ++i)
					if (nexts[i] != rhs.nexts[i])
						return false;
				for (size_t i = 0; i < actions.size(); ++i)
					if ((actions[i] & rhs.actions[i]) == 0)
						return false;
				return true;
			}
		};
		unordered_map<ltree::node*, STreeProp> np_;
		Forest& f_;

		STreeProp CollectStatsRec(ltree::node * n) {
			auto it = np_.find(n);
			if (it != end(np_))
				return it->second;

			STreeProp sp;
			sp.n_ = n;
			if (n->isleaf()) {
				sp.conditions = ".";
				sp.actions.push_back(n->data.action);
				sp.nexts.push_back(n->data.next);
			}
			else {
				sp.conditions = n->data.condition;
				sp += CollectStatsRec(n->left);
				sp += CollectStatsRec(n->right);
			}

			np_[n] = sp;
			return sp;
		}

		// Works only with equivalent trees
		static void Intersect(ltree::node* n1, ltree::node* n2) {
			if (n1->isleaf()) {
				n1->data.action &= n2->data.action;
				n2->data.action = n1->data.action;
			}
			else {
				Intersect(n1->left, n2->left);
				Intersect(n1->right, n2->right);
			}
		}
		// tbr to be replaced
		// rw replace with
		static void FindAndReplace(ltree::node* n, ltree::node* tbr, ltree::node* rw) {
			if (!n->isleaf()) {
				if (n->left == tbr) {
					n->left = rw;
				}
				else if (n->right == tbr) {
					n->right = rw;
				}
				else {
					FindAndReplace(n->left, tbr, rw);
					FindAndReplace(n->right, tbr, rw);
				}
			}
		}

		bool LetsDoIt() {
			np_.clear();

			for (size_t i = 0; i < f_.trees_.size(); ++i) {
				const auto& t = f_.trees_[i];
				CollectStatsRec(t.root);
			}

			vector<STreeProp> vec;
			for (const auto& x : np_)
				vec.emplace_back(x.second);
			sort(begin(vec), end(vec));

			for (size_t i = 0; i < vec.size(); /*empty*/) {
				size_t j = i + 1;
				for (; j < vec.size(); ++j) {
					if (vec[i].conditions != vec[j].conditions)
						break;
				}
				if (j == i + 1) {
					vec.erase(begin(vec) + i);
				}
				else {
					// from i to j-1 the subtrees have the same conditions.
					// Let's check if they have any equivalent subtree
					map<int, bool> keep;
					for (size_t k = i; k < j; ++k)
						keep[k] = false;
					for (size_t k = i; k < j; ++k) {
						for (size_t h = k + 1; h < j; ++h) {
							if (vec[k].equivalent(vec[h])) {
								keep[k] = true;
								keep[h] = true;
							}
						}
						if (!keep[k])
							vec[k].conditions = "Mark for erase";
					}
					for (size_t k = i; k < j;) {
						if (vec[k].conditions == "Mark for erase") {
							vec.erase(begin(vec) + k);
							--j;
						}
						else
							++k;
					}

					i = j;
				}
			}

			// Accrocchio temporaneo
			if (vec.empty())
				return false;

			size_t j = 1;
			for (; j < vec.size(); ++j) {
				if (vec[0].equivalent(vec[j]))
					break;
			}
			assert(j < vec.size());

			Intersect(vec[0].n_, vec[j].n_);
			for (size_t k = 0; k < f_.trees_.size(); ++k) {
				const auto& t = f_.trees_[k];
				FindAndReplace(t.root, vec[0].n_, vec[j].n_);
			}
			return true;
		}

		STree(Forest& f) : f_(f) {
			while (LetsDoIt())
				f_.RemoveUselessConditions();
		}
	};

	LOG("Reducing forest",
		STree st(f);
	);
	PrintStats(f);

	DrawForestOnFile(algorithm_name + "forest_reduced", f, true);

	string forest_reduced_code = global_output_path + algorithm_name + "_forest_reduced_code.txt";
	{
		ofstream os(forest_reduced_code);
		GenerateForestCode(os, f);
	}
}