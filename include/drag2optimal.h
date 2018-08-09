#pragma once

#include "conact_tree.h"

// Converts dag to dag using equivalences between subtrees
void Dag2DagUsingIdenties(ltree& t);

// Converts a tree into dag minimizing the number of nodes (Note: this is "necessary" when the leaves of a tree contain multiple actions)
// UTILIZZA IL NUMERO DI NODI PER SCEGLIERE IL DAG OTTIMO
void Dag2OptimalDag(ltree& t);