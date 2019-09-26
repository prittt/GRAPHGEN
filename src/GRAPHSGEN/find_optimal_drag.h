// Copyright(c) 2018 - 2019 Federico Bolelli, Costantino Grana
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

#ifndef GRAPHSGEN_FIND_OPTIMAL_DRAG_H_
#define GRAPHSGEN_FIND_OPTIMAL_DRAG_H_

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
    std::vector<ltree::node*> lma_; // leaves with multiple actions
    std::unordered_set<ltree::node*> visited_; // utility set to remember already visited nodes
    ltree t_;

    ltree best_tree_;
    uint counter_ = 0;
    uint best_nodes_ = std::numeric_limits<uint>::max();
    uint best_leaves_ = std::numeric_limits<uint>::max();

    std::mutex best_tree_mutex_;
    thread_pool *pool_ = nullptr;

    FindOptimalDrag(ltree t) : t_{ std::move(t) } {
        GetLeavesWithMultipleActionsRec(t_.root);
    }

    // This method fill the lma_ variable with all the leaves that have multiple actions. This
    // vector (lma_) will be used by the backtrack algorithm to generate all the possible trees 
    // (i.e all the possible trees with one action per leaf).
    void GetLeavesWithMultipleActionsRec(ltree::node* n) {
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

    void ReduceAndUpdateBest(ltree t)
    {
        RemoveEqualSubtrees sc(t.root);

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
            std::bitset<128> bs;
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

#endif // GRAPHSGEN_FIND_OPTIMAL_DRAG_H_