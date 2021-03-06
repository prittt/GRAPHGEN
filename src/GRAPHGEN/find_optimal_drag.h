// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_FIND_OPTIMAL_DRAG_H_
#define GRAPHGEN_FIND_OPTIMAL_DRAG_H_

#include <bitset>
#include <iostream>
#include <iomanip>
#include <unordered_set>
#include <mutex>
#include <thread>

#include "pool.h"

#include "conact_tree.h"
#include "remove_equal_subtrees.h"

struct FindOptimalDrag {
    std::vector<BinaryDrag<conact>::node*> lma_; // leaves with multiple actions
    std::unordered_set<BinaryDrag<conact>::node*> visited_; // utility set to remember already visited nodes
    BinaryDrag<conact> t_;

    BinaryDrag<conact> best_tree_;
    uint counter_ = 0;
    uint best_nodes_ = std::numeric_limits<uint>::max();
    uint best_leaves_ = std::numeric_limits<uint>::max();

    std::mutex best_tree_mutex_;
    thread_pool *pool_ = nullptr;

    FindOptimalDrag(BinaryDrag<conact> t) : t_{ std::move(t) } {
        GetLeavesWithMultipleActionsRec(t_.GetRoot());
    }

    // This method fill the lma_ variable with all the leaves that have multiple actions. This
    // vector (lma_) will be used by the backtrack algorithm to generate all the possible trees 
    // (i.e all the possible trees with one action per leaf).
    void GetLeavesWithMultipleActionsRec(BinaryDrag<conact>::node* n) {
        if (visited_.count(n) > 0) {
            return;
        }
        visited_.insert(n);
        if (n->isleaf()) {
            if (n->data.action.count() > 1) {
                lma_.push_back(n);
            }
            return;
        }
        GetLeavesWithMultipleActionsRec(n->left);
        GetLeavesWithMultipleActionsRec(n->right);
    }

    void ReduceAndUpdateBest(BinaryDrag<conact> t)
    {
        //RemoveEqualSubtrees sc(t.GetRoot()); TODO originale
        RemoveEqualSubtrees sc(t);

        std::lock_guard<std::mutex> lock(best_tree_mutex_);
        if (best_nodes_ > sc.nodes_ || (best_nodes_ == sc.nodes_ && best_leaves_ > sc.leaves_)) {
            best_nodes_ = sc.nodes_;
            best_leaves_ = sc.leaves_;
            best_tree_ = std::move(t);
            std::cout << "\rbest_nodes_ = " << best_nodes_ << " - best_leaves_ = " << best_leaves_ << "\n";
        }

        if (counter_ == 0) {
            std::cout << "  0%";
        }

        if (++counter_ % 79626 == 0) {
            std::cout << "\r" << std::setfill(' ') << std::setw(3) << counter_ / 79626 << "%";
        }
    }

    void GenerateAllTreesRec(int cur_leaf)
    {
        if (cur_leaf == lma_.size()) {
            // We have a tree without multiple actions

            pool_->enqueue_work(&FindOptimalDrag::ReduceAndUpdateBest, this, t_);

            return;
        }

        auto action_bs = lma_[cur_leaf]->data.action;
        auto actions = lma_[cur_leaf]->data.actions(); // vector of actions ("uint")

        for (size_t i = 0; i < actions.size(); ++i) {
            std::bitset<131/*CTBE needs 131 bits*/> bs;
            bs.set(actions[i] - 1);
            lma_[cur_leaf]->data.action = bs;
            GenerateAllTreesRec(cur_leaf + 1);
        }
        lma_[cur_leaf]->data.action = action_bs;
    }

    void GenerateAllTrees()
    {
        pool_ = new thread_pool(8, 8);
        GenerateAllTreesRec(0);
        delete pool_;
    }
};

#endif // GRAPHGEN_FIND_OPTIMAL_DRAG_H_