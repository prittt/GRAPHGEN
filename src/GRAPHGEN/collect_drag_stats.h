// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_COLLECT_DRAG_STATISTICS_H_
#define GRAPHGEN_COLLECT_DRAG_STATISTICS_H_

#include <algorithm>
#include <unordered_set>

#include "conact_tree.h"

// This class serves to efficiently collect information about each node/subtree of a tree starting from a node
// it both stores properties of each subtree (as string) and all the parents of each node.
struct CollectDragStatistics {

    // Utility class to calculate and store (sub)trees properties
    struct STreeProp {
        std::string conditions_;
        std::vector<BinaryDrag<conact>::node*> leaves_;
        BinaryDrag<conact>::node* n_;

        STreeProp& operator+=(const STreeProp& rhs) {
            conditions_ += rhs.conditions_;
            copy(begin(rhs.leaves_), end(rhs.leaves_), back_inserter(leaves_));
            return *this;
        }

        bool equivalent(const STreeProp& rhs) {
            if (conditions_ != rhs.conditions_)
                return false;
            for (size_t i = 0; i < leaves_.size(); ++i)
                if (leaves_[i]->data.next != rhs.leaves_[i]->data.next
                    ||
                    (leaves_[i]->data.action & rhs.leaves_[i]->data.action) == 0)
                    return false;
            return true;
        }
    };
    std::unordered_map<BinaryDrag<conact>::node*, STreeProp> np_; // Associate to each tree node its properties (STreeProp)
    std::unordered_map<BinaryDrag<conact>::node*, std::vector<BinaryDrag<conact>::node*>> parents_; // Associate to each tree node its parents (vector of nodes)

    CollectDragStatistics() {}
    CollectDragStatistics(BinaryDrag<conact>& bd) {
        for (const auto& t : bd.roots_) {
            CollectStatsRec(t);
            //CollectStatsRec(t->left);
            //CollectStatsRec(t->right);
        }
    }

    // Collect information about each node/subtree of a tree starting from a node
    STreeProp CollectStatsRec(BinaryDrag<conact>::node * n) {
        auto it = np_.find(n);
        if (it != end(np_))
            return it->second;

        STreeProp sp;
        sp.n_ = n;
        if (n->isleaf()) {
            sp.conditions_ = ".";
            sp.leaves_.push_back(n);
        }
        else {
            parents_[n->left].push_back(n);
            parents_[n->right].push_back(n);
            sp.conditions_ = n->data.condition;
            sp += CollectStatsRec(n->left);
            sp += CollectStatsRec(n->right);
        }

        np_[n] = sp;
        return sp;
    }
};

#endif // GRAPHGEN_COLLECT_DRAG_STATISTICS_H_
