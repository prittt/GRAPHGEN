// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_TREE2DAG_IDENTITIES_H_
#define GRAPHGEN_TREE2DAG_IDENTITIES_H_

#include "conact_tree.h"

void FindAndLinkIdentiesRec(BinaryDrag<conact>::node* n1, BinaryDrag<conact>::node* n2);
//void Tree2DagUsingIdentitiesRec(BinaryDrag<conact>::node *n, BinaryDrag<conact>& t);

// Converts a tree into dag considering only equal subtrees
void Tree2DagUsingIdentities(BinaryDrag<conact>& t);

#endif //!GRAPHGEN_TREE2DAG_IDENTITIES_H_