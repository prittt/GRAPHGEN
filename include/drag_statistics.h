#pragma once

#include "conact_tree.h"
#include <set>

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
    DragStatistics(const ltree& t) {
        PerformStatistics(t.root);
    }

    auto nodes() const { return visited_nodes.size(); }
    auto leaves() const { return visited_leaves.size(); }
};

