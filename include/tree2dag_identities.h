#pragma once

#include "conact_tree.h"

void FindAndLinkIdentiesRec(ltree::node* n1, ltree::node* n2);
void Tree2DagUsingIdentitiesRec(ltree::node *n, ltree& t);

// Converts a tree into dag considering only equal subtrees
void Tree2DagUsingIdentities(ltree& t);