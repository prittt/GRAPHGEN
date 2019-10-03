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

#ifndef GRAPHSGEN_DRAG_STATISTICS_H_
#define GRAPHSGEN_DRAG_STATISTICS_H_

#include <set>

#include "conact_tree.h"

/** 
* Calculates the statisics of a DRAG (the number of unique nodes an unique leaves).
*
*/
class DragStatistics {
    std::set<const ltree::node*> visited_nodes;
    std::set<const ltree::node*> visited_leaves;

    void PerformStatistics(const ltree::node *n) {
        if (n->isleaf()) {
            visited_leaves.insert(n);
            return;
        }

        if (visited_nodes.insert(n).second) {
            PerformStatistics(n->left);
            PerformStatistics(n->right);
        }
    }

public:
	/** 
	* The constructor creates a DragStatistics object and calculates the statistics of a DRAG
	* 
	* @param [in] t: DRAG on which calculate statistics. Note that simple tree are also DRAG.
	*/
    DragStatistics(const ltree& t) {
        PerformStatistics(t.GetRoot());
    }

	/**
	* Returns the number of unique nodes inside the DRAG.
	* 
	* @return number of unique nodes
	*/
    auto Nodes() const { return visited_nodes.size(); }

	/**
	* Returns the number of unique leaves inside the DRAG.
	*
	* @return number of unique leaves
	*/
	auto Leaves() const { return visited_leaves.size(); }
};

/**
* Displays on stdout the statisics of a DRAG (number of unique nodes an unique leaves).
*
* @param [in] t: DRAG on which calculate statistics. Note that simple tree are also DRAG.
*/
void PrintStats(const ltree& t);

#endif // !GRAPHSGEN_DRAG_STATISTICS_H_