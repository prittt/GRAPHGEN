#include "drag2optimal.h"

#include <map>
#include <vector>
#include <iostream>

#include "drag_statistics.h"

void FindAndLinkIdentiesDagRec(ltree::node* n1, ltree::node* n2, std::map<ltree::node*, bool> &visited_fl) {
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
void Dag2DagUsingIdentiesRec(ltree::node *n, ltree& t, std::map<ltree::node*, bool> &visited_n) {
    std::map<ltree::node*, bool> visited_fl;
    FindAndLinkIdentiesDagRec(n, t.root, visited_fl);
    visited_n[n] = true;

    if (!n->isleaf()) {
        if (!visited_n[n->left])
            Dag2DagUsingIdentiesRec(n->left, t, visited_n);
        if (!visited_n[n->right])
            Dag2DagUsingIdentiesRec(n->right, t, visited_n);
    }
}

// Converts dag to dag using equivalences between subtrees
void Dag2DagUsingIdenties(ltree& t) {
    std::map<ltree::node*, bool> visited_n;
    Dag2DagUsingIdentiesRec(t.root, t, visited_n);
}

// Given a dag with multiple actions on leaves this function generate all possible dags with only one action per leaf
// VERSIONE CHE CONTA I NODI E LE FOGLIE PER DECIDERE QUAL E' L'ALBERO MIGLIORE
void Tree2OptimalDagRec(ltree& t, ltree::node* n, ltree &best_tree, uint &best_nodes, uint &best_leaves, std::map<const ltree::node*, bool> &visited_n, uint &counter) {
    ltree nt;
    if (n->isleaf()) {
        // leaf with multiple action
        std::vector<uint> actions_list = n->data.actions();
        if (actions_list.size() > 1) {
            for (size_t i = 0; i < actions_list.size() - 1; ++i) {
                std::map<const ltree::node*, bool> visited_node_cur;
                n->data.action = 1 << (actions_list[i] - 1);
                nt = t;
                Tree2OptimalDagRec(nt, nt.root, best_tree, best_nodes, best_leaves, visited_node_cur, counter);
            }
            n->data.action = 1 << (actions_list[actions_list.size() - 1] - 1);
        }
        return;
    }

    if (!visited_n[n->left]) {
        // left node not already visited
        visited_n[n->left] = true;
        Tree2OptimalDagRec(t, n->left, best_tree, best_nodes, best_leaves, visited_n, counter);
    }

    if (!visited_n[n->right]) {
        // right node not already visited
        visited_n[n->right] = true;
        Tree2OptimalDagRec(t, n->right, best_tree, best_nodes, best_leaves, visited_n, counter);
    }

    if (t.root == n) {
        counter++;
        ltree dag = t;
        Dag2DagUsingIdenties(dag);

        DragStatistics ds(dag);

        if (best_nodes > ds.nodes()) {
            best_nodes = ds.nodes();
            best_leaves = ds.leaves();
            best_tree = dag;
        }
        else if (best_nodes == ds.nodes() && best_leaves > ds.leaves()) {
            best_leaves = ds.leaves();
            best_tree = dag;
        }

        if (counter % 1000 == 0) {
            std::cout << counter / 1000 << "\r";
        }
    }
}


// Converts a tree into dag minimizing the number of nodes (Note: this is "necessary" when the leaves of a tree contain multiple actions)
// UTILIZZA IL NUMERO DI NODI PER SCEGLIERE IL DAG OTTIMO
void Tree2OptimalDag(ltree& t) {
    std::vector<ltree> trees;
    ltree best_tree;
    std::map<const ltree::node*, bool> visited_nodes;
    uint counter = 0;
    uint best_nodes = std::numeric_limits<uint>::max();
    uint best_leaves = std::numeric_limits<uint>::max();
    Tree2OptimalDagRec(t, t.root, best_tree, best_nodes, best_leaves, visited_nodes, counter);
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
