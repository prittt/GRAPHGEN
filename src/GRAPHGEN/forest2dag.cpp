// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "forest2dag.h"

#include <sstream>
#include <iomanip>

using namespace std;

// Converts a tree into unique string exploiting memoization 
string Forest2Dag::Tree2String(BinaryDrag<conact>::node* n) {
	auto it = ps_.find(n);
	if (it != end(ps_))
		return it->second;

	string s;
	if (n->isleaf()) {
		//char a_action[sizeof(n->data.action) + 1] = { 0 };
		//memcpy(a_action, reinterpret_cast<char*>(&n->data.action), sizeof(n->data.action));
		stringstream ss;
		ss << setfill('0') << setw(3) << n->data.next;
		s = n->data.action.to_string() + ss.str();
	}
	else
		s = n->data.condition + Tree2String(n->left) + Tree2String(n->right);

	ps_[n] = s;
	return s;
}

// Recursively searches for equal subtrees inside the forest. It starts from a root of the forest
// and explore its subtrees. For each subtree, if there is an equal subtree (i.e. sp_ hash table
// contains the tree identifier string) the function updates the link, otherwise it updates the 
// hash table with the "new" subtree. This must be repeated for every root of the forest (see Forest2Dag) 
void Forest2Dag::FindAndLink(BinaryDrag<conact>::node* n) {
	if (!n->isleaf()) {
		auto s = Tree2String(n->left);

		auto it = sp_.find(s);
		if (it == end(sp_)) {
			sp_[s] = n->left;
			FindAndLink(n->left);
		}
		else {
			n->left = it->second;
		}

		s = Tree2String(n->right);

		it = sp_.find(s);
		if (it == end(sp_)) {
			sp_[s] = n->right;
			FindAndLink(n->right);
		}
		else {
			n->right = it->second;
		}
	}
}

// Calls FindAndLink for each root of the forest
//Forest2Dag::Forest2Dag(LineForestHandler& f) : f_(f) {
//	// Conversion to DAG for the main forest
//	for (auto& t : f_.trees_) {
//		FindAndLink(t.GetRoot());
//	}
//
//	// Clean memoizing data structures
//	ps_.clear();
//	sp_.clear();
//
//	// Conversion to DAG for the end forest
//	for (auto& etg : f_.end_trees_) { // etg -> end trees groups
//
//		if (f.separately) {
//			// Clean memoizing data structures
//			ps_.clear();
//			sp_.clear();
//		}
//
//		for (auto& et : etg) {
//			FindAndLink(et.GetRoot());
//		}
//	}
//}