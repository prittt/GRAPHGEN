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

#ifndef GRAPHGEN_FOREST_STATISTICS_H_
#define GRAPHGEN_FOREST_STATISTICS_H_

#include <set>

#include "forest.h"

class ForestStatistics {
	std::set<const BinaryDrag<conact>::node*> visited_nodes;
	std::set<const BinaryDrag<conact>::node*> visited_leaves;

	std::set<const BinaryDrag<conact>::node*> visited_end_nodes;
	std::set<const BinaryDrag<conact>::node*> visited_end_leaves;

	void PerformStatistics(const BinaryDrag<conact>::node *n) {
		if (n->isleaf()) {
			visited_leaves.insert(n);
			return;
		}

		if (visited_nodes.insert(n).second) {
			PerformStatistics(n->left);
			PerformStatistics(n->right);
		}
	}

	void PerformEndStatistics(const BinaryDrag<conact>::node *n) {
		if (n->isleaf()) {
			visited_end_leaves.insert(n);
			return;
		}

		if (visited_end_nodes.insert(n).second) {
			PerformEndStatistics(n->left);
			PerformEndStatistics(n->right);
		}
	}

public:
	ForestStatistics(const Forest& f) {
		// Internal trees statistics
		for (const auto& t : f.trees_) {
			PerformStatistics(t.GetRoot());
		}

		// End trees statistics
		for (const auto& g : f.end_trees_) {
			for (const auto& t : g) {
				PerformEndStatistics(t.GetRoot());
			}
		}
	}

	auto nodes() const { return visited_nodes.size(); }
	auto leaves() const { return visited_leaves.size(); }
	auto end_nodes() const { return visited_end_nodes.size(); }
	auto end_leaves() const { return visited_end_leaves.size(); }
};

void PrintStats(const Forest& f);


#endif // !GRAPHGEN_FOREST_STATISTICS_H_