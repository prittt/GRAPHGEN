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

#ifndef GRAPHSGEN_CULO_H_
#define GRAPHSGEN_CULO_H_

#include <algorithm>
#include <iterator>
#include <set>
#include <unordered_set>

#include "conact_tree.h"
#include "drag_statistics.h"
#include "magic_optimizer.h"
#include "output_generator.h"
#include "remove_equal_subtrees.h"
#include "forest2dag.h"
#include "forest_statistics.h"
#include "graph_code_generator.h"

/*ltree::node* MergeEquivalentTreesRec(ltree::node* a, ltree::node* b, std::unordered_map<ltree::node*, ltree::node*> &merged)
    {
        auto it = merged.find(a);
        if (it != end(merged))
            return it->second;

        auto n = new ltree::node(*a);
        if (a->isleaf()) {
            n->data.action &= b->data.action;
        }
        else {
            n->left = MergeEquivalentTreesRec(a->left, b->left, merged);
            n->right = MergeEquivalentTreesRec(a->right, b->right, merged);
        }
        merged[a] = n;
        return n;
    }

    void DeleteTreeRec(ltree::node* n, std::unordered_map<ltree::node*, bool> &deleted)
    {
        auto it = deleted.find(n);
        if (it != end(deleted))
            return;
        deleted[n] = true;

        if (!n->isleaf()) {
            DeleteTreeRec(n->left, deleted);
            DeleteTreeRec(n->right, deleted);
        }
        delete n;
    }*/

    /*void PrintTreeRec(std::ostream& os, ltree::node* n, std::set<ltree::node*>& visited, int tab = 0) {
            os << std::string(tab, '\t');

            auto it = visited.find(n);
            if (it != end(visited)) {
                os << " -> " << n << "\n";
                return;
            }
            visited.insert(n);

            if (n->isleaf()) {
                os << ". ";
                auto a = n->data.actions();
                copy(begin(a), end(a), std::ostream_iterator<int>(os, ", "));
                os << "\n";
            }
            else {
                os << n->data.condition << "\n";
                PrintTreeRec(os, n->left, visited, tab + 1);
                PrintTreeRec(os, n->right, visited, tab + 1);
            }

        }*/

        // Compress a tree / forest into a DRAG solving equivalences
struct FastDragOptimizer {

    // This class perform the merge of equivalent trees updating links
    struct MergeEquivalentTreesAndUpdate {
        std::unordered_set<ltree::node*> visited_;
        std::unordered_map<ltree::node*, std::vector<ltree::node*>>& parents_;

        MergeEquivalentTreesAndUpdate(ltree::node* a, ltree::node* b, std::unordered_map<ltree::node*, std::vector<ltree::node*>>& parents) :
            parents_{ parents }
        {
            MergeEquivalentTreesAndUpdateRec(a, b);
        }

        void MergeEquivalentTreesAndUpdateRec(ltree::node* a, ltree::node* b)
        {
            auto it = visited_.find(a);
            if (it != end(visited_))
                return;
            visited_.insert(a);

            for (auto& x : parents_[b]) {
                if (x->left == b)
                    x->left = a;
                else
                    x->right = a;
            }

            if (a->isleaf()) {
                a->data.action &= b->data.action;
            }
            else {
                MergeEquivalentTreesAndUpdateRec(a->left, b->left);
                MergeEquivalentTreesAndUpdateRec(a->right, b->right);
            }
        }
    };

    // "Constructor" for tree optimization
    FastDragOptimizer(ltree& t) {
        FastDragOptimizerRec(t);
    }

    // "Constructor" for forest optimization
    FastDragOptimizer(Forest& f) {
        FastDragOptimizerRec(f);
    }

    int count = 0;
    size_t best_nodes = std::numeric_limits<size_t >::max();
    size_t best_leaves = std::numeric_limits<size_t >::max();
    ltree best_tree;
    void FastDragOptimizerRec(ltree& t, bool ignore_leaves = true)
    {
        // Collect information of tree's subtrees (as strings)
        MagicOptimizer mo(t.GetRoot());

        // Push previously collected information into a (simpler to use) vector
        std::vector<MagicOptimizer::STreeProp> trees;
        for (const auto& x : mo.np_)
            trees.push_back(x.second);

        // For each subtree (with or without considering leaves) ...
        for (size_t i = 0; i < trees.size(); ) {
            if (ignore_leaves && trees[i].conditions_ == ".") {
                trees.erase(begin(trees) + i);
                continue;
            }
            bool eq = false;
            // ... check all the other subtrees to find equivalences
            for (size_t j = 0; j < trees.size(); ++j) {
                if (i != j && trees[i].equivalent(trees[j])) {
                    eq = true;
                    break;
                }
            }
            // If there the current subtree has no equivalent subtrees
            // remove it from the list.
            if (!eq) {
                trees.erase(begin(trees) + i);
            }
            else {
                ++i;
            }
        }

        // Order subtrees which have equivalent from the deepest to the least deep
        // this step is somehow useless since later all the possible merges are 
        // tested. Ordering the tree is useful to find, in some cases, the best or
        // pseudo optimal solution earlier, so that when the process does not end 
        // in "good time" we still have a good solution/good compression.
        sort(begin(trees), end(trees), [](const MagicOptimizer::STreeProp& a, const MagicOptimizer::STreeProp& b) {
            return a.conditions_.size() > b.conditions_.size();
        });

        bool no_eq = true;
        // Compare each subtree with has equivalent subtrees with all the other
        for (size_t i = 0; i < trees.size(); ++i) {
            bool eq = false;
            size_t j;
            for (j = i + 1; j < trees.size(); ++j) {
                if (trees[i].equivalent(trees[j])) {
                    // Here an equivalence is found!
                    no_eq = false;

                    // Then we need to copy the original (entire) tree 
                    // keeping track of some nodes. Indeed, to perform
                    // the link, we need to know the pointers to the 
                    // roots of the equivalent subtrees after the copy.
                    std::vector<ltree::node*> tracked_nodes{ trees[i].n_, trees[j].n_ };
                    ltree t_copy(t, tracked_nodes);

                    // Re-calculate subtrees/node statistics over the tree 
                    // copy since pointers to nodes are changed.
                    MagicOptimizer mo(t_copy.GetRoot());

                    // Perform the merge of equivalent trees updating links
                    MergeEquivalentTreesAndUpdate(tracked_nodes[0], tracked_nodes[1], mo.parents_);

                    // Remove equal subtrees inside a tree. Is this really
                    // necessary here? 
                    RemoveEqualSubtrees(t_copy.GetRoot());

                    //{ ofstream os("after.txt"); set<ltree::node*> visited; PrintTreeRec(os, t_copy.GetRoot(), visited); }
                    //DrawDagOnFile("After", t_copy, true);

                    // Recursively call the compression function on the current
                    // resulting tree
                    FastDragOptimizerRec(t_copy);
                }
            }
        }
        if (no_eq) {
            ++count;
            if (count % 1000 == 0)
                std::cout << "\r" << count % 1000;
            DragStatistics ds(t);
            if (ds.Nodes() < best_nodes || (ds.Nodes() == best_nodes) && ds.Leaves() < best_leaves) {
                best_nodes = ds.Nodes();
                best_leaves = ds.Leaves();
                best_tree = t;
                DrawDagOnFile("BestDrag" + zerostr(count, 4), t, true);
                std::cout << count << " - nodes: " << ds.Nodes() << " - leaves: " << ds.Leaves() << "\n";
                FastDragOptimizerRec(t, false);
            }
        }
    }

    void FastDragOptimizerRec(Forest& f, bool ignore_leaves = true)
    {
        std::vector<MagicOptimizer::STreeProp> trees;
        MagicOptimizer mo;
        for (const auto& t : f.trees_) {
            mo.CollectStatsRec(t.GetRoot());
        }
        for (const auto& x : mo.np_) {
            trees.push_back(x.second);
        }

        for (size_t i = 0; i < trees.size(); ) {
            if (ignore_leaves && trees[i].conditions_ == ".") {
                trees.erase(begin(trees) + i);
                continue;
            }
            bool eq = false;
            for (size_t j = 0; j < trees.size(); ++j) {
                if (i != j && trees[i].equivalent(trees[j])) {
                    eq = true;
                    break;
                }
            }
            if (!eq) {
                trees.erase(begin(trees) + i);
            }
            else {
                ++i;
            }
        }

        sort(begin(trees), end(trees), [](const MagicOptimizer::STreeProp& a, const MagicOptimizer::STreeProp& b) {
            return a.conditions_.size() > b.conditions_.size();
        });

        bool no_eq = true;
        for (size_t i = 0; i < trees.size(); ++i) {
            bool eq = false;
            size_t j;
            for (j = i + 1; j < trees.size(); ++j) {
                if (trees[i].equivalent(trees[j])) {
                    no_eq = false;

                    std::vector<ltree::node*> tracked_nodes{ trees[i].n_, trees[j].n_ };
                    Forest f_copy(f, tracked_nodes);

                    MagicOptimizer mo;
                    for (const auto& t : f_copy.trees_) {
                        mo.CollectStatsRec(t.GetRoot());
                    }

                    if (false) {
                        ForestStatistics fs(f_copy);
                        std::cout << "ForestBeforeMerge" << count << " - nodes: " << fs.nodes() << " - leaves: " << fs.leaves() << "\n";
                        //DrawForestOnFile("ForestBeforeMerge" + zerostr(count, 4), f_copy);
                    }
                    MergeEquivalentTreesAndUpdate(tracked_nodes[0], tracked_nodes[1], mo.parents_);
                    if (false) {
                        ForestStatistics fs(f_copy);
                        std::cout << "ForestAfterMerge" << count << " - nodes: " << fs.nodes() << " - leaves: " << fs.leaves() << "\n";
                        //DrawForestOnFile("ForestAfterMerge" + zerostr(count, 4), f_copy);
                    }

                    Forest2Dag f2d(f_copy);

                    //{ ofstream os("after.txt"); set<ltree::node*> visited; PrintTreeRec(os, t_copy.GetRoot(), visited); }
                    //DrawDagOnFile("After", t_copy, true);
                    FastDragOptimizerRec(f_copy);
                }
            }
        }
        if (no_eq) {
            ++count;
            if (count % 1000 == 0)
                std::cout << "\r" << count;
            ForestStatistics fs(f);
            if (fs.nodes() < best_nodes || (fs.nodes() == best_nodes) && fs.leaves() < best_leaves) {
                best_nodes = fs.nodes();
                best_leaves = fs.leaves();
                //DrawForestOnFile("BestForest" + zerostr(count, 4), f);
                std::cout << count << " - nodes: " << fs.nodes() << " - leaves: " << fs.leaves() << "\n";

                {
                    std::ofstream os("stocazzo.txt");
                    // GenerateForestCode(os, f, "", 0, 2);
                }

                FastDragOptimizerRec(f, false);
            }
        }
    }
};

#endif // GRAPHSGEN_CULO_H_