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

#ifndef GRAPHGEN_DRAG_COMPRESSOR_H_
#define GRAPHGEN_DRAG_COMPRESSOR_H_

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

/*BinaryDrag<conact>::node* MergeEquivalentTreesRec(BinaryDrag<conact>::node* a, BinaryDrag<conact>::node* b, std::unordered_map<BinaryDrag<conact>::node*, BinaryDrag<conact>::node*> &merged)
    {
        auto it = merged.find(a);
        if (it != end(merged))
            return it->second;

        auto n = new BinaryDrag<conact>::node(*a);
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

    void DeleteTreeRec(BinaryDrag<conact>::node* n, std::unordered_map<BinaryDrag<conact>::node*, bool> &deleted)
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

    /*void PrintTreeRec(std::ostream& os, BinaryDrag<conact>::node* n, std::set<BinaryDrag<conact>::node*>& visited, int tab = 0) {
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
    std::unordered_set<const BinaryDrag<conact>::node*> visited_;
    bool removed_;
public:
    MergeSpecialLeaves(BinaryDrag<conact>& bd) {
        do {
            removed_ = false;
            for (auto& t : bd.roots_) {
                MergeSpecialLeavesRec(t);
            }
        } while (removed_);
    }

    void MergeSpecialLeavesRec(BinaryDrag<conact>::node* n) {

        if (n->isleaf()) {
            return;
        }

        auto& left = n->left;
        auto& right = n->right;
        if (left->isleaf() && right->isleaf()) {
            if (left->data.eq(right->data)) {
                // In this case the current node becomes a leaf. The action associated to this 
                // leaf will be the intersection of the actions of its children.
                n->data = left->data;
                n->data.action &= right->data.action;
                left = right = nullptr;
                removed_ = true;
                return;
            }
        }

        if (visited_.insert(n).second) {
            MergeSpecialLeavesRec(n->left);
            MergeSpecialLeavesRec(n->right);
        }
    }
};

class MergeLeaves {
private:
    std::unordered_set<BinaryDrag<conact>::node*> visited_nodes_;
    std::unordered_set<BinaryDrag<conact>::node*> visited_leaves_;
    std::unordered_map<BinaryDrag<conact>::node*, std::vector<BinaryDrag<conact>::node*>> parents_;
    BinaryDrag<conact>& bd_;
public:
    MergeLeaves(BinaryDrag<conact>& bd) : bd_{bd} {
        for (auto& t : bd.roots_) {
            CalculateStatsRec(t);
        }
        CompressLeaves();
    }

    // Yet another function to populate visited nodes/leaves and parents node data structure
    void CalculateStatsRec(BinaryDrag<conact>::node*& n) {

        if (n->isleaf()) {
            visited_leaves_.insert(n);
            return;
        }

        if (visited_nodes_.insert(n).second) {
            parents_[n->left].push_back(n);
            parents_[n->right].push_back(n);

            CalculateStatsRec(n->left);
            CalculateStatsRec(n->right);
        }
    }
    
    void SerializeVisitedLeaves(std::ostream& os = std::cout) {
        for (const auto& l : visited_leaves_) {
            for (const auto& a : l->data.actions()) {
                os << a;
            }
            os << "- " << l->data.next << "\n";
        }
    }

    void CompressLeaves() {

        std::unordered_set<BinaryDrag<conact>::node*> already_updated; // Store the leaves for which the parents have already been updated
        // For each actually used leaf
        for(auto& i : visited_leaves_){
            // Skip multiple actions leaves 
            if (i->data.actions().size() != 1 || already_updated.find(i) != end(already_updated)){
                continue;
            }

            // Compare the current leaf's action with the actions of all the others leaves 
            // and merge them (updating parents) if there is a non-empty intersection and
            // the next tree ids are the same
            for (auto& j : visited_leaves_){
                if(i == j){
                    continue; // We don't want to compare a leaf with itself
                }

                if (already_updated.find(j) != end(already_updated)) {
                    continue; // We don't want to consider the same leaf multiple times
                }
                
                if ((i->data.action & j->data.action).size() != 0 && i->data.next == j->data.next) {
                    // Merge required!
                    
                    // Add them to the already_updated
                    already_updated.insert(j);

                    // Update j's parents so that they will point to i. 
                    for (auto& x : parents_[j]) {
                        if (x->left == j) {
                            x->left = i;
                        }
                        else if (x->right == j) { // This should be always true, but...
                            x->right = i;
                        }
                        else {
                            throw; // Bug catcher
                        }
                    }
                }
                //DrawDagOnFile("current_dag", bd_, DrawDagFlags::WITH_NEXT | DrawDagFlags::WITH_ROOT_ID);
            }
        }
    }
};


/** @brief This enum class defines the available flags for the DragCompression class. */
DEFINE_ENUM_CLASS_FLAGS(DragCompressorFlags, 
    NONE                      = 0, /**< @brief No flags */
    PRINT_STATUS_BAR          = 1, /**< @brief Whether to print a sort of progress bar or not */
    IGNORE_LEAVES             = 2, /**< @brief Whether to ignore leaves or not during the compression.
                                               Please note that compressing the leaves will significantly
                                               increase the total execution time without improving the
                                               final compression result in anyway. */
    SAVE_INTERMEDIATE_RESULTS = 4, /**< @brief Whether to delete or not the dot code used to draw the drag */
)

//DEFINE_ENUM_CLASS_OR_OPERATOR(DragCompressorFlags)
//DEFINE_ENUM_CLASS_AND_OPERATOR(DragCompressorFlags)

// Compress a tree / forest into a DRAG solving equivalences
class DragCompressor {
private:
    bool changes_;

    bool early_stopping_active_;
    bool early_stopping_reached_ = false;
    int iterations_max_;
    int iterations_left_;

    size_t progress_counter_ = 0;
    size_t best_nodes_ = std::numeric_limits<size_t >::max();
    size_t best_leaves_ = std::numeric_limits<size_t >::max();
public:

    void UpdateProgress(DragCompressorFlags flags) {
        bool print_status_bar = flags & DragCompressorFlags::PRINT_STATUS_BAR;
        if (print_status_bar) {
            std::cout << "\r" << progress_counter_ << "\n";
        }
    }

    // BinaryDrag 
    // Iteration is the early stopping criteria and represents the number of "iteration" 
    // after a new optimal is found. -1 means no early stopping and it's the dafult
    DragCompressor(BinaryDrag<conact>& bd, int iterations = -1, DragCompressorFlags flags = DragCompressorFlags::PRINT_STATUS_BAR | DragCompressorFlags::IGNORE_LEAVES) {
        early_stopping_active_ = iterations != -1;
        iterations_left_ = iterations_max_ = iterations;
        TLOG("Compressing BinaryDrag",
            RemoveEqualSubtrees{ bd };
            do {
                changes_ = false;
                FastDragOptimizerRec(bd, flags);
                bd = best_bd_;
                ResetIterations();
            } while (changes_);
        
            UpdateProgress(flags);
        )
    }

    void ResetIterations() {
        iterations_left_ = iterations_max_;
        early_stopping_reached_ = false;
    }

    // LineForestHandler
    DragCompressor(LineForestHandler& lfh, int iterations = -1, DragCompressorFlags flags = DragCompressorFlags::PRINT_STATUS_BAR | DragCompressorFlags::IGNORE_LEAVES) {
        early_stopping_active_ = iterations != -1;
        iterations_left_ = iterations_max_ = iterations;
        std::string msg = conf.algorithm_name_ + " - compressing main forest\n";
        TLOG(msg,
            RemoveEqualSubtrees{ lfh.f_ };
            do {
                changes_ = false;
                FastDragOptimizerRec(lfh.f_, flags);
                lfh.f_ = best_bd_;
                ResetIterations();
            }while(changes_);
            UpdateProgress(flags);
        )

        int fn = 0;
        for (auto& f : lfh.end_forests_) {
            progress_counter_ = 0;
            ResetIterations();
            msg = conf.algorithm_name_ + " - compressing end forest #" + std::to_string(fn++) + "\n";
            TLOG(msg,
                RemoveEqualSubtrees{ f };
                ResetBest();
                do {
                    changes_ = false;
                    FastDragOptimizerRec(f, flags);
                    f = best_bd_;
                    ResetIterations();
                } while (changes_);
                UpdateProgress(flags);)
        }
    }

private:

    void ResetBest() {
        best_nodes_ = std::numeric_limits<size_t >::max();
        best_leaves_ = std::numeric_limits<size_t >::max();
    }

    // This class perform the merge of equivalent trees updating links
    struct MergeEquivalentTreesAndUpdate {
        std::unordered_set<BinaryDrag<conact>::node*> visited_;
        std::unordered_map<BinaryDrag<conact>::node*, std::vector<BinaryDrag<conact>::node*>>& parents_;

        MergeEquivalentTreesAndUpdate(BinaryDrag<conact>::node* a, 
                                      BinaryDrag<conact>::node* b, 
                                      std::unordered_map<BinaryDrag<conact>::node*, std::vector<BinaryDrag<conact>::node*>>& parents) :
            parents_{ parents }
        {
            MergeEquivalentTreesAndUpdateRec(a, b);
        }

        void MergeEquivalentTreesAndUpdateRec(BinaryDrag<conact>::node* a, BinaryDrag<conact>::node* b)
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

    BinaryDrag<conact> best_bd_;

    void FastDragOptimizerRec(BinaryDrag<conact>& bd, DragCompressorFlags flags)
    {
        if (early_stopping_reached_ && early_stopping_active_) {
            return;
        }

        bool print_status_bar = flags & DragCompressorFlags::PRINT_STATUS_BAR;
        bool ignore_leaves = flags & DragCompressorFlags::IGNORE_LEAVES;
        bool save_intermediate_results = flags & DragCompressorFlags::SAVE_INTERMEDIATE_RESULTS;

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
                    RemoveEqualSubtrees{ bd_copy };

                    // Recursively call the compression function on the current
                    // resulting tree
                    FastDragOptimizerRec(bd_copy, flags);
                }
            }
        }

        if (no_eq) {
            // Display a raw progress status if needed
            ++progress_counter_;
            if (print_status_bar) {
                if (progress_counter_ % 1000 == 0) {
                    std::cout << "\r" << progress_counter_ << std::flush;
                }
            }

            if (--iterations_left_ <= 0) {
                early_stopping_reached_ = true;
            }

            BinaryDragStatistics bds(bd);
            if (bds.Nodes() < best_nodes_) {
                // New better binary drag found ...
                changes_ = true;
                iterations_left_ = iterations_max_;

                // ... compress the leaves of the current optimal binary drag
                MergeSpecialLeaves{ bd };
                MergeLeaves{ bd };

                // ... and finally update class attributes accordingly
                BinaryDragStatistics bds(bd);
                best_nodes_  = bds.Nodes();
                best_leaves_ = bds.UniqueLeaves();
                best_bd_ = bd;

                // ... save the current tree if needed
                if (save_intermediate_results) {
                    DrawDagOnFile("BestDrag" + zerostr(progress_counter_, 10), bd);
                }

                // ... print status if needed
                if (print_status_bar) {
                    std::cout << "\r" << progress_counter_ << " - nodes: " << bds.Nodes() << "; leaves: " << bds.UniqueLeaves() << "\n";
                }

            }
        }
    }
};

#endif // GRAPHGEN_DRAG_COMPRESSOR_H_