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

#ifndef GRAPHSGEN_DRAG_COMPRESSOR_H_
#define GRAPHSGEN_DRAG_COMPRESSOR_H_

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

/** This class merges special leaves. The following tree
       b
      /
     a
    / \
   2  2,3
becomes:
       b
      /
     2
*/
class MergeSpecialLeaves {
private:
    std::set<const BinaryDrag<conact>::node*> visited_nodes;
    //std::set<const BinaryDrag<conact>::node*> visited_leaves;
public:
    MergeSpecialLeaves(BinaryDrag<conact>& bd) {
        for (auto& t : bd.roots_) {
            MergeSpecialLeavesRec(t);
        }
    }

    void MergeSpecialLeavesRec(BinaryDrag<conact>::node*& n) {
        
        if (n->isleaf()) {
            return;
        }

        auto left  = n->left;
        auto right = n->right;
        if (left->isleaf() && right->isleaf() && ((left->data.action & right->data.action) != 0 && left->data.next == right->data.next)) {
            // In this case the current node becomes a leaf. The action associated to this 
            // leaf will be the intersection of the actions of its children.
            n = left;
            n->data.action = left->data.action & right->data.action;
            return;
        }

        if (visited_nodes.insert(n).second) {
            MergeSpecialLeavesRec(n->left);
            MergeSpecialLeavesRec(n->right);
        }
    }
};


// Compress a tree / forest into a DRAG solving equivalences
class DragCompressor {
public:
    static const int PRINT_STATUS_BAR = 1; /**< @brief Whether to print a sort of progress bar or not */
    static const int IGNORE_LEAVES    = 2; /**< @brief Whether to ignore leaves or not during the compression.
                                                   Please note that compressing the leaves will significantly
                                                   increase the total execution time without improving the
                                                   final compression result in anyway. */
    static const int SAVE_INTERMEDIATE_RESULTS = 4; /**< @brief Whether to delete or not the dot code used to draw the drag */

    // BinaryDrag 
    DragCompressor(BinaryDrag<conact>& bd, int flags = PRINT_STATUS_BAR | IGNORE_LEAVES) {
        FastDragOptimizerRec(bd, flags);
        bd = best_bd;
    }

    // LineForestHandler
    DragCompressor(LineForestHandler& lfh, int flags = PRINT_STATUS_BAR | IGNORE_LEAVES) {
        FastDragOptimizerRec(lfh.f_, flags);
        lfh.f_ = best_bd;

        for (auto& f : lfh.end_forests_) {
            FastDragOptimizerRec(f, flags);
            f = best_bd;
        }
    }

private:
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


    size_t progress_counter = 0;
    size_t best_nodes  = std::numeric_limits<size_t >::max();
    size_t best_leaves = std::numeric_limits<size_t >::max();
    BinaryDrag<conact> best_bd;

    void FastDragOptimizerRec(BinaryDrag<conact>& bd, int flags)
    {
        bool print_status_bar = flags & PRINT_STATUS_BAR;
        bool ignore_leaves = flags & IGNORE_LEAVES;
        bool save_intermediate_results = flags & SAVE_INTERMEDIATE_RESULTS;

        // Collect information of trees'/drags' subtrees (as strings) and push
        // them into a vector so that they are easier to use
        std::vector<CollectDragStatistics::STreeProp> trees;
        CollectDragStatistics cds(bd);
        for (const auto& x : cds.np_) {
            trees.push_back(x.second);
        }

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

        // Then we order subtrees which have equivalent from the deepest to the least 
        // deep. This step is somehow useless since later all the possible merges are 
        // tested. Ordering the tree is useful to find, in some cases, the best or
        // pseudo optimal solution earlier, so that when the process does not end 
        // in "good time" we still have a good solution/compression.
        sort(begin(trees), end(trees), [](const CollectDragStatistics::STreeProp& a, const CollectDragStatistics::STreeProp& b) {
            return a.conditions_.size() > b.conditions_.size();
        });

        // Compare each subtree which has equivalent subtrees with all the others
        bool no_eq = true;
        for (size_t i = 0; i < trees.size(); ++i) {
            bool eq = false;
            size_t j;
            for (j = i + 1; j < trees.size(); ++j) {
                if (trees[i].equivalent(trees[j])) {
                    // Here an equivalence is found!
                    no_eq = false;

                    // Then we need to copy the original (entire) tree 
                    // keeping track of some nodes. Indeed, to perform
                    // the link update, we need to know the pointers to
                    // the roots of the equivalent subtrees after the 
                    // copy.
                    std::vector<BinaryDrag<conact>::node*> tracked_nodes{ trees[i].n_, trees[j].n_ };
                    BinaryDrag<conact> bd_copy(bd, tracked_nodes);

                    // Re-calculate subtrees/node statistics over the tree 
                    // copy since pointers to nodes are changed.
                    CollectDragStatistics cds(bd_copy);

                    // Perform the merge of equivalent trees updating links
                    MergeEquivalentTreesAndUpdate(tracked_nodes[0], tracked_nodes[1], cds.parents_);

                    // Remove equal subtrees inside a BinaryDrag. Is this really
                    // necessary here? Maybe it isn't but performing this operation
                    // here can improve the efficiency of the compression in case 
                    // there are actually equal sub-trees.
                    RemoveEqualSubtrees{bd_copy};

                    // Recursively call the compression function on the current
                    // resulting tree
                    FastDragOptimizerRec(bd_copy, flags);
                }
            }
        }

        if (no_eq) {
            // Display a raw progress status if needed
            if (ignore_leaves) {
                ++progress_counter;
                if (print_status_bar) {
                    if (progress_counter % 1 == 0) {
                        std::cout << "\r" << std::setfill(' ') << std::setw(10) << progress_counter;
                    }
                }
            }
            
            BinaryDragStatistics bds(bd);
            if (bds.Nodes() < best_nodes || (bds.Nodes() == best_nodes && bds.Leaves() < best_leaves)) {
                // New better binary drag found. Update attributes accordingly ...
                best_nodes  = bds.Nodes();
                best_leaves = bds.Leaves();
                best_bd = bd;

                // ... save the current tree if needed
                if (save_intermediate_results) {
                    DrawDagOnFile("BestDrag" + zerostr(progress_counter, 10), bd);
                }

                if (print_status_bar) {
                    std::cout << progress_counter << " - nodes: " << bds.Nodes() << "; leaves: " << bds.Leaves() << "\n";
                }

                // ... and finally compress the leaves of the current optimal binary drag
                // FastDragOptimizerRec(bd, flags & ~IGNORE_LEAVES); // This is a bad idea!
                MergeSpecialLeaves{best_bd};
            }
        }
    }
};

#endif // GRAPHSGEN_DRAG_COMPRESSOR_H_