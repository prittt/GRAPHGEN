// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_DRAG2OPTIMAL_H_
#define GRAPHGEN_DRAG2OPTIMAL_H_

#include "conact_tree.h"

// Converts dag to dag using equivalences between subtrees
void Dag2DagUsingIdenties(BinaryDrag<conact>& t);

// Converts a dag (tree) into dag removing equivalent subtrees. This could prevent better optimizations and does not guarantee optimality
// When considering_leaves is true also equivalent leaves will be compressed. 
void Dag2DagUsingEquivalences(BinaryDrag<conact>& t, bool considering_leaves = true);

// Converts a tree into dag minimizing the number of nodes (Note: this is "necessary" when the leaves of a tree contain multiple actions)
// USES NUMBER OF NODES TO PICK THE OPTIMAL DAG
void Dag2OptimalDag(BinaryDrag<conact>& t);

#endif // !GRAPHGEN_DRAG2OPTIMAL_H_