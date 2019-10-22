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

#ifndef GRAPHGEN_CONACT_TREE_H_
#define GRAPHGEN_CONACT_TREE_H_

#include <fstream>

#include "condition_action.h"
#include "tree.h"

#include "drag.h"
//using ltree = tree<conact>;
using ltree = BinaryDrag<conact>;


/**
* Load tree from file. The tree must be saved in a file with the following structure:
*	
* x
*   	a (left son of x)
*   		c
*   			. 2
*   			. 3
*   		. 1,3,4 (list of actions)
*   	b (right son of x)
*   		c
*   			. 2
*   			. 3
*   		. 4
* @param [out] t: Loaded tree
* @param [in] filename: Name of the file (path) from which load the tree
*
* @return whether the tree has been loaded or not
*/
bool LoadConactTree(ltree& t, const std::string& filename);

// Write a tree into a txt file structured as in the example above
bool WriteConactTree(const ltree& t, const std::string& filename);

bool equivalent_trees(const ltree::node* n1, const ltree::node* n2);
void intersect_leaves(ltree::node* n1, ltree::node* n2);

// Checks if two (sub)trees 'n1' and 'n2' are equal
bool EqualTrees(const ltree::node* n1, const ltree::node* n2);

void IntersectTrees(ltree::node* n1, ltree::node* n2);

// Instead of defining a novel format to save DRAGS, we save them as trees, then link
// identical sub-trees. Since the order of traversal is the same, the result should be the same.
// Should...
bool LoadConactDrag(ltree& t, const std::string& filename);

// Instead of defining a novel format to save DRAGS, we save them as trees, then link
// identical sub-trees. Since the order of traversal is the same, the result should be the same.
// Should...
bool WriteConactDrag(ltree& t, const std::string& filename);

#endif // !GRAPHGEN_CONACT_TREE_H_



