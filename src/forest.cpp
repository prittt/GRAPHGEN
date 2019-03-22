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

#include "forest.h"

#include <cassert>

#include "Forest2dag.h"
#include "forest_optimizer.h"

using namespace std;

Forest::Forest(ltree t, const pixel_set& ps, const constraints& initial_constraints) : /*t_(std::move(t)),*/ eq_(ps) {
	next_tree_.push_back(0); // Setup next_tree_ for holding a reference to the start tree in first position
	t_.root = Reduce(t.root, t_, initial_constraints);
	InitNext(t_);

	// Create start tree constraints and add start tree in position 0 of the tree_ array
	{
		constraints start_constr;
		using namespace std;
		for (const auto& p : ps) {
			if (p.GetDx() < 0)
				start_constr[p.name_] = 0;
		}
		ltree t;
		t.root = Reduce(t_.root, t, start_constr);
		trees_.emplace_back(t);
	}

	CreateReducedTrees(t_, {});
	
	// Vecchio metodo
	while (RemoveEqualTrees()) {
		RemoveUselessConditions();
	}

	////Nuovo Metodo
	//do {
	//	while (RemoveEquivalentTrees()) {
	//		RemoveUselessConditions();
	//	}
	//	Forest2Dag f2d(*this);
	//	STree st(*this);
	//	RemoveUselessConditions();		
	//	RebuildDisjointTrees();
	//} while (RemoveEquivalentTrees());
	 

	// For each tree creates end trees with end line constraints
	// BBDT example (borderline cases): 
	//              -4  -3  -2  -1 | w
	//				    		   |
	//     +-------+-------+-------+
	//	   | a   b | c   d | e   f |
	//	   | g   h | i   j | k   l |
	// A:  +-------+-------+-------+				c = w - 4 (No problem)
	//	   | m   n | o   p |       |
	//	   | q   r | s   t |       |
	//	   +-------+-------+       |
	//                             |
	//         +-------+-------+---|---+
	//	       | a   b | c   d | e | f |
	//	       | g   h | i   j | k | l |
	// B:      +-------+-------+---|---+			c = w - 3 (No problem)
	//	       | m   n | o   p |   |
	//	       | q   r | s   t |   |
	//	       +-------+-------+   |
	//                             |
	//             +-------+-------+-------+
	//	           | a   b | c   d | e   f |
	//	           | g   h | i   j | k   l |
	// C:          +-------+-------+-------+		c = w - 2 (This requires one group of end-trees)
	//	           | m   n | o   p |   
	//	           | q   r | s   t |   
	//	           +-------+-------+   
	//                             |
	//                 +-------+---|---+-------+
	//	               | a   b | c | d | e   f |
	//	               | g   h | i | j | k   l |
	// D:              +-------+---|---+-------+	c = w - 1 (This requires one group of end-trees)
	//	               | m   n | o | p |   
	//	               | q   r | s | t |   
	//	               +-------+---|---+   
	//                             |
	{
		for (int out_offset = 1;; ++out_offset) {
			constraints end_constr;
			for (const auto& p : ps) {
				if (p.GetDx() >= out_offset)
					end_constr[p.name_] = 0;
			}
			if (end_constr.empty())
				break;

			end_trees_.emplace_back(vector<ltree>());

			for (const auto& t : trees_) {
				ltree tr;
				tr.root = Reduce(t.root, tr, end_constr);
				end_trees_.back().emplace_back(tr);
			}

			assert(trees_.size() == end_trees_[end_trees_.size() - 1].size());
		}

		// Init trees_ - end_trees_ mapping
		main_trees_end_trees_mapping_ = vector<vector<int>>(end_trees_.size(), vector<int>(trees_.size()));
		for (auto& etm : main_trees_end_trees_mapping_) {
			iota(etm.begin(), etm.end(), 0);
		}
		end_next_trees_ = main_trees_end_trees_mapping_;


		// Set end_trees_'s next_trees to uint32_t max value (max value can be replaced with any value, but all end trees must share the same fake next)
		for (auto& t_a : end_trees_) { // foreach group of end trees
			for (auto& t_b : t_a) {	// foreach tree in a group
				for (auto& n : t_b.nodes) { // foreach node in a tree
					if (n->isleaf()) {
						n->data.next = numeric_limits<uint32_t>::max();
					}
				}
			}
		}
	}

	// Removes useless conditions and then possible duplicate end-trees until convergence
	// Vecchio metodo ( se non lo metto non va un cazzo, perchè? )
	while (RemoveEqualEndTrees()) {
		RemoveEndTreesUselessConditions();
	}

	//// Nuovo metodo
	//do {
	//	while (RemoveEquivalentEndTrees()) {
	//		RemoveEndTreesUselessConditions();
	//	}
	//	Forest2Dag f2d(*this);
	//	STree st(*this);
	//	RemoveEndTreesUselessConditions();
	//	RebuildDisjointEndTrees();
	//} while (RemoveEquivalentEndTrees());
}

void Forest::RebuildDisjointTrees() {

	vector<ltree> new_trees;

	for (auto& t : trees_) {
		// Here Reduce() is used just to recreate trees 
		ltree new_t;
		new_t.root = Reduce(t.root, new_t, {});
		new_trees.push_back(move(new_t));
	}

	trees_ = move(new_trees);
}

void Forest::RebuildDisjointEndTrees() {
	
	vector<vector<ltree>> new_trees;
	for (auto& tg : end_trees_) {
		new_trees.emplace_back();
		for (auto& t : tg) {
			// Here Reduce() is used just to recreate trees 
			ltree new_t;
			new_t.root = Reduce(t.root, new_t, {});
			new_trees.back().push_back(move(new_t));
		}
	}
	end_trees_ = new_trees;

}


// See RemoveUselessConditions
void RemoveUselessConditionsRec(ltree::node* n, bool& changed) {
	if (!n->isleaf()) {
		if (EqualTrees(n->left, n->right)) {
			changed = true;
			*n = *n->left;
			RemoveUselessConditionsRec(n, changed);
		}
		else {
			RemoveUselessConditionsRec(n->left, changed);
			RemoveUselessConditionsRec(n->right, changed);
		}
	}
}

// Removes useless conditions inside the forest.
// That means that the subtree:
//			a
//       0/  \1
//       c    ...
//    0/   \1
//    5     5
// will be transformed into:
//			a
//       0/  \1
//       5    ...
// these useless condition may appears after the execution of CreateReducedTrees.
void Forest::RemoveUselessConditions() {
	bool changed;
	do {
		changed = false;
		for (auto& t : trees_) {
			RemoveUselessConditionsRec(t.root, changed);
		}
	} while (changed);
}
void Forest::RemoveEndTreesUselessConditions() {
	bool changed;
	do {
		changed = false;
		for (auto& tg : end_trees_) {
			for (auto& t : tg) {
				RemoveUselessConditionsRec(t.root, changed);
			}
		}
	} while (changed);
}

bool Forest::RemoveEqualEndTrees() {
	return RemoveEndTrees(EqualTrees);
}

bool Forest::RemoveEquivalentEndTrees() {
	return RemoveEndTrees(equivalent_trees);
}

// Removes duplicate end-trees (this is performed in each group separately, it can't happen that different groups contain equal trees)
bool Forest::RemoveEndTrees(bool(*FunctionPtr)(const ltree::node* n1, const ltree::node* n2)) {

	// Find which trees are identical and mark them in end_trees_mapping
	bool found = false;
	for (size_t tg = 0; tg < end_trees_.size(); ++tg) { // foreach group of end trees
		const auto& cur_trees = end_trees_[tg];
		auto& cur_equal = end_next_trees_[tg];
		// triangular matching
		for (size_t i = 0; i < cur_trees.size() - 1; ++i) {
			if (cur_equal[i] == i) {
				for (size_t j = i + 1; j < cur_trees.size(); ++j) {
					if (cur_equal[j] == j) {
						if (FunctionPtr(cur_trees[i].root, cur_trees[j].root)) {
							cur_equal[j] = i;
							found = true;
							if (FunctionPtr == equivalent_trees) {
								IntersectTrees(cur_trees[i].root, cur_trees[j].root);
							}
						}
					}
				}
			}
		}
	}

	if (!found) {
		return false;
	}

	// Flatten the end_next_trees indexes
	size_t new_index;
	for (size_t tg = 0; tg < end_trees_.size(); ++tg) { // foreach group of end trees
		const auto& cur_trees = end_trees_[tg];
		auto& cur_equal = end_next_trees_[tg];
		new_index = 0;
		for (size_t i = 0; i < cur_equal.size(); ++i) {
			if (cur_equal[i] == i) {
				cur_equal[i] = new_index;
				++new_index;
			}
			else {
				cur_equal[i] = cur_equal[cur_equal[i]];
			}
		}
	}

	// Update the main_trees_end_trees_mapping_ data structure whose size will never change
	for (size_t tg = 0; tg < end_trees_.size(); ++tg) { // foreach group of end trees
		auto& cur_equal = end_next_trees_[tg];
		auto& cur_mapping = main_trees_end_trees_mapping_[tg];
		for (size_t i = 0; i < cur_mapping.size(); ++i) {
			cur_mapping[i] = cur_equal[cur_mapping[i]];
		}
	}

	// Remove trees which are identical to already inserted ones
	for (size_t tg = 0; tg < end_trees_.size(); ++tg) { // foreach group of end trees
		auto& cur_trees = end_trees_[tg];
		auto& cur_equal = end_next_trees_[tg];
		new_index = 0;
		vector<ltree> trees;
		for (size_t i = 0; i < cur_equal.size(); ++i) {
			if (cur_equal[i] == new_index) {
				trees.push_back(cur_trees[i]);
				++new_index;
			}
		}
		cur_trees = move(trees);
	}

	// No next for end trees, "UpdateNext" is useless

	// Update of end_next_trees_
	for (size_t tg = 0; tg < end_trees_.size(); ++tg) { // foreach group of end trees
		auto& cur_trees = end_trees_[tg];
		auto& cur_equal = end_next_trees_[tg];
	
		cur_equal.resize(cur_trees.size());
		iota(begin(cur_equal), end(cur_equal), 0);
	}

	return true;
}

void Forest::UpdateNext(ltree::node* n) {
	if (n->isleaf()) {
		n->data.next = next_tree_[n->data.next];
	}
	else {
		UpdateNext(n->left);
		UpdateNext(n->right);
	}
}

bool Forest::RemoveEquivalentTrees() {
	return RemoveTrees(equivalent_trees);
}

bool Forest::RemoveEqualTrees() {
	return RemoveTrees(EqualTrees);
}


// Removes duplicate trees inside the forest
bool Forest::RemoveTrees(bool(*FunctionPtr)(const ltree::node* n1, const ltree::node* n2)) {
	// Find which trees are identical and mark them in next_tree
	bool found = false;
	for (size_t i = 0; i < next_tree_.size() - 1; ++i) {
		if (next_tree_[i] == i) {
			for (size_t j = i + 1; j < next_tree_.size(); ++j) {
				if (next_tree_[j] == j) {
					if (FunctionPtr(trees_[i].root, trees_[j].root)) {
						next_tree_[j] = i;
						found = true;
						if (FunctionPtr == equivalent_trees) {
							IntersectTrees(trees_[i].root, trees_[j].root);
						}
					}
				}
			}
		}
	}
	if (!found)
		return false;

	// Flatten the trees indexes
	size_t new_index = 0;
	for (size_t i = 0; i < next_tree_.size(); ++i) {
		if (next_tree_[i] == i) {
			next_tree_[i] = new_index;
			++new_index;
		}
		else {
			next_tree_[i] = next_tree_[next_tree_[i]];
		}
	}

	// Remove trees which are identical to already inserted ones
	new_index = 0;
	vector<ltree> trees;
	for (size_t i = 0; i < next_tree_.size(); ++i) {
		if (next_tree_[i] == new_index) {
			trees.push_back(trees_[i]);
			++new_index;
		}
	}
	trees_ = move(trees);

	for (auto& t : trees_) {
		UpdateNext(t.root);
	}

	next_tree_.resize(trees_.size());
	iota(begin(next_tree_), end(next_tree_), 0);
	return true;
}

// See InitNext
void Forest::InitNextRec(ltree::node* n) {
	if (n->isleaf()) {
		// Set the next tree to be used for each leaf
		n->data.next = next_tree_.size();
		// Setup a structure for managing equal trees
		next_tree_.push_back(next_tree_.size());
	}
	else {
		InitNextRec(n->left);
		InitNextRec(n->right);
	}
}

// Initializes leave's next trees with sequential values
void Forest::InitNext(ltree& t) {
	InitNextRec(t_.root);
}

// Perform tree pruning by removing useless nodes. Useless nodes are identified looking at given constraints 
ltree::node* Forest::Reduce(const ltree::node* n, ltree& t, const constraints& constr) {
	if (n->isleaf()) {
		return t.make_node(n->data);
	}
	else {
		auto it = constr.find(n->data.condition);
		if (it != end(constr)) {
			if (it->second == 0)
				return Reduce(n->left, t, constr);
			else
				return Reduce(n->right, t, constr);
		}
		else {
			return t.make_node(n->data, Reduce(n->left, t, constr), Reduce(n->right, t, constr));
		}
	}
}

// See CreateReducedTrees 
void Forest::CreateReducedTreesRec(const ltree::node* n, const constraints& constr) {
	if (n->isleaf()) {
		// Create a reduced version of the tree based on what we learned on the path to this leaf        
		ltree t;
		t.root = Reduce(t_.root, t, constr);
		trees_.emplace_back(t);
	}
	else {
		constraints constrNew = constr;
		auto ft = eq_.Find(n->data.condition);
		if (ft)
			constrNew[ft] = 0;
		CreateReducedTreesRec(n->left, constrNew);
		if (ft)
			constrNew[ft] = 1;
		CreateReducedTreesRec(n->right, constrNew);
	}
}

// Creates forest of trees pruning original tree. The pruning is performed as follow: the original 
// tree is recursively explored and constraints (different on each branch) are defined using equivalences 
// between pixels (i.e. pixels which remain in the mask when it moves). When a leaf is reached the 
// original tree is reduced using current branch's constraints. 
void Forest::CreateReducedTrees(const ltree& t, const constraints& initial_constr) {
	CreateReducedTreesRec(t_.root, initial_constr);
}