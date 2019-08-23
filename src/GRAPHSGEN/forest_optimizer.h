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

#ifndef GRAPHSGEN_FOREST_OPTIMIZER_H_
#define GRAPHSGEN_FOREST_OPTIMIZER_H_

#include <algorithm>
#include <bitset>
#include <string>
#include <vector>

#include "forest.h"

using namespace std;

struct STree {
	struct STreeProp {
		string conditions;
		vector<std::bitset<11881>> actions;
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
	map<ltree::node*, STreeProp> np_;
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
			// To avoid finding of equal trees that optimizer cannot compress
			CollectStatsRec(t.root->right);
			CollectStatsRec(t.root->left);
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
		if (j >= vec.size()) {
			throw;
		}

		Intersect(vec[0].n_, vec[j].n_);
		for (size_t k = 0; k < f_.trees_.size(); ++k) {
			auto& t = f_.trees_[k];
			// Era un viaggio che poi è deviato verso un altro viaggio
			//// Whole trees can be equivalent too
			//if (t.root == vec[0].n_) {
			//	t.root = vec[j].n_;
			//}
			/////////////////////////////////////
			FindAndReplace(t.root, vec[0].n_, vec[j].n_);
		}
		return true;
	}

	bool LetsDoEndTrees(const vector<vector<ltree*>> trees) {
		np_.clear();

		for (size_t tg = 0; tg < trees.size(); ++tg) {
			const auto& cur_trees = trees[tg];
			for (size_t i = 0; i < cur_trees.size(); ++i) {
				const auto t = cur_trees[i];
				// To avoid finding of equal trees that optimizer cannot compress
				CollectStatsRec(t->root->right);
				CollectStatsRec(t->root->left);
			}
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
		if (j >= vec.size()) {
			throw;
		}

		Intersect(vec[0].n_, vec[j].n_);
		for (size_t tg = 0; tg < trees.size(); ++tg) {
			const auto& cur_trees = trees[tg];
			for (size_t k = 0; k < cur_trees.size(); ++k) {
				const auto& t = cur_trees[k];
				// Era un viaggio che poi è deviato verso un altro viaggio
				// Whole trees can be equivalent too
				/*if (t->root == vec[0].n_) {
					t->root = vec[j].n_;
				}*/
				///////////////////////////////////
				FindAndReplace(t->root, vec[0].n_, vec[j].n_);
			}
		}

		return true;
	}

	STree(Forest& f) : f_(f) {
		while (LetsDoIt()) {
			f_.RemoveUselessConditions();
			//f_.RemoveEqualTrees();
		}

		if (f_.separately) {
			for (auto& tg : f.end_trees_) {
				vector<vector<ltree*>> cur_tree_ptrs = { vector<ltree*>() };
				for (ltree &t : tg) {
					cur_tree_ptrs.front().push_back(&t);
				}
				while (LetsDoEndTrees(cur_tree_ptrs)) {
					f_.RemoveEndTreesUselessConditions();
					//f_.RemoveEqualEndTrees();
				}
			}
		}
		else {
			vector<vector<ltree*>> tree_ptrs;
			for (vector<ltree> &tg : f_.end_trees_) {
				tree_ptrs.emplace_back();
				for (ltree &t : tg) {
					tree_ptrs.back().push_back(&t);
				}
			}
			while (LetsDoEndTrees(tree_ptrs)) {
				f_.RemoveEndTreesUselessConditions();
				//f_.RemoveEqualEndTrees();
			}
		}
	}
};

#endif // !GRAPHSGEN_FOREST_OPTIMIZER_H_