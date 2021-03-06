// Copyright(c) 2019 
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

#include "drag2optimal.h"

#include <iostream>
#include <map>
#include <vector>

#include "drag_statistics.h"

void FindAndLinkIdentiesDagRec(BinaryDrag<conact>::node* n1, BinaryDrag<conact>::node* n2, std::map<BinaryDrag<conact>::node*, bool> &visited_fl) {
    if (n2->isleaf() || n1 == n2 || visited_fl[n2])
        return;
    visited_fl[n2] = true;

    if (n1 != n2->left && EqualTrees(n1, n2->left)) {
        n2->left = n1;
    }

    if (n1 != n2->right && EqualTrees(n1, n2->right)) {
        n2->right = n1;
    }

    FindAndLinkIdentiesDagRec(n1, n2->left, visited_fl);
    FindAndLinkIdentiesDagRec(n1, n2->right, visited_fl);
}

// Recursive auxiliary function for the conversion of a DAG into DAG with no equivalent subgraphs
void Dag2DagUsingIdentiesRec(BinaryDrag<conact>::node *n, BinaryDrag<conact>& t, std::map<BinaryDrag<conact>::node*, bool> &visited_n) {
    std::map<BinaryDrag<conact>::node*, bool> visited_fl;
    FindAndLinkIdentiesDagRec(n, t.GetRoot(), visited_fl);
    visited_n[n] = true;

    if (!n->isleaf()) {
        if (!visited_n[n->left])
            Dag2DagUsingIdentiesRec(n->left, t, visited_n);
        if (!visited_n[n->right])
            Dag2DagUsingIdentiesRec(n->right, t, visited_n);
    }
}

// Converts dag to dag using identies between subtrees
void Dag2DagUsingIdenties(BinaryDrag<conact>& t) {
	std::map<BinaryDrag<conact>::node*, bool> visited_n;
	Dag2DagUsingIdentiesRec(t.GetRoot(), t, visited_n);
}

void FindAndLinkEquivalencesDagRec(BinaryDrag<conact>::node* n1, BinaryDrag<conact>::node* n2, std::map<BinaryDrag<conact>::node*, bool> &visited_fl) {
	if (n2->isleaf() || n1 == n2 || visited_fl[n2])
		return;
	visited_fl[n2] = true;

	if (n1 != n2->left && equivalent_trees(n1, n2->left)) {
		n2->left = n1;
	}

	if (n1 != n2->right && equivalent_trees(n1, n2->right)) {
		n2->right = n1;
	}

	FindAndLinkEquivalencesDagRec(n1, n2->left, visited_fl);
	FindAndLinkEquivalencesDagRec(n1, n2->right, visited_fl);
}

// Recursive auxiliary function for the conversion of a DAG into DAG with no equivalent subgraphs
void Dag2DagUsingEquivalencesRec(BinaryDrag<conact>::node *n, BinaryDrag<conact>& t, std::map<BinaryDrag<conact>::node*, bool> &visited_n, bool considering_leaves) {
	std::map<BinaryDrag<conact>::node*, bool> visited_fl;
	
	if (!n->isleaf() || considering_leaves) {
		FindAndLinkEquivalencesDagRec(n, t.GetRoot(), visited_fl);
	}
	visited_n[n] = true;

	if (!n->isleaf()) {
		if (!visited_n[n->left])
			Dag2DagUsingEquivalencesRec(n->left, t, visited_n, considering_leaves);
		if (!visited_n[n->right])
			Dag2DagUsingEquivalencesRec(n->right, t, visited_n, considering_leaves);
	}
}

// Converts dag to dag using equivalences between subtrees
void Dag2DagUsingEquivalences(BinaryDrag<conact>& t, bool considering_leaves) {
	std::map<BinaryDrag<conact>::node*, bool> visited_n;
	Dag2DagUsingEquivalencesRec(t.GetRoot(), t, visited_n, considering_leaves);
}

// Given a dag with multiple actions on leaves this function generate all possible dags with only one action per leaf
void Dag2OptimalDagRec(BinaryDrag<conact>& t, BinaryDrag<conact>::node* n, BinaryDrag<conact> &best_tree, size_t &best_nodes, size_t &best_leaves, std::map<const BinaryDrag<conact>::node*, bool> &visited_n, uint &counter) {
    BinaryDrag<conact> nt;
    if (n->isleaf()) {
        // leaf with multiple action
        std::vector<uint> actions_list = n->data.actions();
        if (actions_list.size() > 1) {
            for (size_t i = 0; i < actions_list.size() - 1; ++i) {
                std::map<const BinaryDrag<conact>::node*, bool> visited_node_cur;
                n->data.action = 0;
                n->data.action.set(actions_list[i] - 1);
                nt = t;
                Dag2OptimalDagRec(nt, nt.GetRoot(), best_tree, best_nodes, best_leaves, visited_node_cur, counter);
            }
            n->data.action = 0;
            n->data.action.set(actions_list[actions_list.size() - 1] - 1);
        }
        return;
    }

    if (!visited_n[n->left]) {
        // left node not already visited
        visited_n[n->left] = true;
        Dag2OptimalDagRec(t, n->left, best_tree, best_nodes, best_leaves, visited_n, counter);
    }

    if (!visited_n[n->right]) {
        // right node not already visited
        visited_n[n->right] = true;
        Dag2OptimalDagRec(t, n->right, best_tree, best_nodes, best_leaves, visited_n, counter);
    }

    if (t.GetRoot() == n) {
        counter++;
        BinaryDrag<conact> dag = t;
        Dag2DagUsingIdenties(dag);

        BinaryDragStatistics ds(dag);

        if (best_nodes > ds.Nodes()) {
            best_nodes = ds.Nodes();
            best_leaves = ds.Leaves();
            best_tree = dag;
        }
        else if (best_nodes == ds.Nodes() && best_leaves > ds.Leaves()) {
            best_leaves = ds.Leaves();
            best_tree = dag;
        }

        if (counter % 1000 == 0) {
            std::cout << counter / 1000 << "\r";
        }
    }
}


// Converts a tree into dag minimizing the number of nodes (Note: this is "necessary" when the leaves of a tree contain multiple actions)
// USES NUMBER OF NODES TO PICK THE OPTIMAL DAG
void Dag2OptimalDag(BinaryDrag<conact>& t) {
    std::vector<BinaryDrag<conact>> trees;
    BinaryDrag<conact> best_tree;
    std::map<const BinaryDrag<conact>::node*, bool> visited_nodes;
    uint counter = 0;
    size_t best_nodes = std::numeric_limits<size_t>::max();
    size_t best_leaves = std::numeric_limits<size_t>::max();
    Dag2OptimalDagRec(t, t.GetRoot(), best_tree, best_nodes, best_leaves, visited_nodes, counter);
    std::cout << "** Vector size:" << counter << " **\n";
    std::cout << "** Counter:" << counter << " **\n";

    t = best_tree;

    /*for (size_t i = 0; i < trees.size(); ++i) {
    DrawDagOnFile("tree_" + to_string(i), trees[i]);
    Dag2DagUsingIdenties(trees[i]);
    DrawDagOnFile("dag_" + to_string(i), trees[i]);
    }

    BestDagFromList(trees, t);*/
}
