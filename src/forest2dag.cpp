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

#include "forest2dag.h"

#include <sstream>
#include <iomanip>

using namespace std;

// Converts a tree into unique string exploiting memoization 
string Forest2Dag::Tree2String(ltree::node* n) {
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
void Forest2Dag::FindAndLink(ltree::node* n) {
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
Forest2Dag::Forest2Dag(Forest& f) : f_(f) {
	// Conversion to DAG for the main forest
	for (auto& t : f_.trees_) {
		FindAndLink(t.root);
	}

	// Clean memoizing data structures
	ps_.clear();
	sp_.clear();

	// Conversion to DAG for the end forest
	for (auto& etg : f_.end_trees_) { // etg -> end trees groups

		if (f.separately) {
			// Clean memoizing data structures
			ps_.clear();
			sp_.clear();
		}

		for (auto& et : etg) {
			FindAndLink(et.root);
		}
	}
}