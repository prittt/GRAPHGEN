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

#ifndef GRAPHSGEN_CONACT_TREE_H_
#define GRAPHSGEN_CONACT_TREE_H_

#include <fstream>

#include "condition_action.h"
#include "tree.h"

using ltree = tree<conact>;

// Example of conact tree
//  x
//  	a (left son of x) 
//  		c 
//  			. 2
//  			. 3
//  		. 1,3,4 (list of actions)
//  	b (right son of x)
//  		c
//  			. 2
//  			. 3
//  		. 4


// Loads a tree from a txt file structured as in the example above
ltree LoadConactTree(const char *filename);

// Auxiliary recursive function of the LoadConactTree
ltree::node* LoadConactTreeRec(std::ifstream& is);

// Write a tree into a txt file structured as in the example above
bool WriteConactTree(const ltree& t, const char *filename);

// Auxiliary recursive function of the WriteConactTree
bool WriteConactTreeRec(std::ofstream& os);

#endif // !GRAPHSGEN_CONACT_TREE_H_



