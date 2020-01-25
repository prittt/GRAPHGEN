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

#include "forest.h"

#include <optional>

#include <cassert>

#include "drag_compressor.h"
#include "forest_optimizer.h"

using namespace std;

LineForestHandler::LineForestHandler(const BinaryDrag<conact>& bd,
    const pixel_set& ps,
    const constraints& initial_constraints) {
    // The input BinaryDrag should have just one root!
    if (bd.roots_.size() > 1) {
        throw;
    }

    // Setup next_tree_ for holding a reference to the start tree in first position
    next_tree_.push_back(0);

    // Apply predefined constraints
    BinaryDrag<conact> tmp;
    tmp.AddRoot(Reduce(bd.roots_[0], tmp, initial_constraints));

    // Initialize next trees indexes in each leaf with sequential values.
    InitNextRec(tmp.roots_[0]);

    /*********************
     *  START LINE TREE  *
     *********************/

     // Create start tree constraints and add it to the drag f_. 
     // The root of the start tree will be the first one in the 
     // BinaryDrag vector of roots.
    {
        constraints start_constr;
        using namespace std;
        for (const auto& p : ps) {
            if (p.GetDx() < 0)
                start_constr[p.name_] = 0;
        }
        f_.AddRoot(Reduce(tmp.GetRoot(), f_, start_constr));
    }
    // Note that the start line tree is generated in a different
    // way but it is part of the main forest. 

    /********************
     *    MAIN TREES    *
     ********************/

     // Create all the possible reduced trees storing them into f_
    CreateReducedDrag(tmp, f_, ps);

    // Remove duplicate trees and then useless conditions until convergence
    while (RemoveEqualTrees()) {
        RemoveUselessConditions();
    }

    /********************
     *  END LINE TREES  *
     ********************/

     // For each tree we need to create at least one end tree with end line constraints.
     // With BBDT for example we need two end trees for each tree for example. This is 
     // explained below representing borderline cases:
     //
     //             -4  -3  -2  -1 | w
     //                            |
     //     +-------+-------+-------+
     //	   | a   b | c   d | e   f |
     //	   | g   h | i   j | k   l |
     // A:  +-------+-------+-------+               c = w - 4 (No problem)
     //	   | m   n | o   p |       |
     //	   | q   r | s   t |       |
     //	   +-------+-------+       |
     //                            |
     //        +-------+-------+---|---+
     //	       | a   b | c   d | e | f |
     //	       | g   h | i   j | k | l |
     // B:     +-------+-------+---|---+           c = w - 3 (No problem)
     //	       | m   n | o   p |   |
     //	       | q   r | s   t |   |
     //	       +-------+-------+   |
     //                            |
     //            +-------+-------+-------+
     //	           | a   b | c   d | e   f |
     //	           | g   h | i   j | k   l |
     // C:         +-------+-------+-------+       c = w - 2 (This requires one group of end-trees)
     //	           | m   n | o   p |   
     //	           | q   r | s   t |   
     //	           +-------+-------+   
     //                            |
     //                +-------+---|---+-------+
     //	               | a   b | c | d | e   f |
     //	               | g   h | i | j | k   l |
     // D:             +-------+---|---+-------+   c = w - 1 (This requires one group of end-trees)
     //	               | m   n | o | p |   
     //	               | q   r | s | t |   
     //	               +-------+---|---+   
     //                            |
    {
        for (int out_offset = 1;; ++out_offset) {
            // Create constraints for the last n-th column (n = out_offset)
            constraints end_constr;
            for (const auto& p : ps) {
                if (p.GetDx() >= out_offset)
                    end_constr[p.name_] = 0;
            }
            if (end_constr.empty()) {
                break;
            }

            end_forests_.push_back(BinaryDrag<conact>());

            for (const auto& t : f_.roots_) {
                end_forests_.back().AddRoot(Reduce(t, end_forests_.back(), end_constr));
            }

            assert(f_.roots_.size() == end_forests_.back().roots_.size());
        }

        // Initialize the mapping between main trees (including the start line tree) and end trees 
        main_end_tree_mapping_ = vector<vector<int>>(end_forests_.size(), vector<int>(f_.roots_.size()));
        for (auto& etm : main_end_tree_mapping_) {
            iota(etm.begin(), etm.end(), 0);
        }
        end_next_tree_ = main_end_tree_mapping_;

        // For each tree in the end forests the value of the next tree is set to uint32_t max value 
        // max value can be replaced with any value, but all end trees must share the same fake next
        // to be able to delete equal (useless) end trees.
        for (auto& f : end_forests_) { // For each end forest
            for (auto& n : f.nodes_) { // For each node of the forest
                if (n->isleaf()) {
                    n->data.next = numeric_limits<uint32_t>::max();
                }
            }
        }
    }

    // Removes duplicate end-trees and then useless conditions until convergence
    while (RemoveEqualEndTrees()) {
        RemoveEndTreesUselessConditions();
    }
}

//void LineForestHandler::RebuildDisjointTrees() {
//
//    vector<BinaryDrag<conact>> new_trees;
//
//    for (auto& t : trees_) {
//        // Here Reduce() is used just to recreate trees 
//        BinaryDrag<conact> new_t;
//        new_t.SetRoot(Reduce(t.GetRoot(), new_t, {}));
//        new_trees.push_back(move(new_t));
//    }
//
//    trees_ = move(new_trees);
//}

//void LineForestHandler::RebuildDisjointEndTrees() {
//
//    vector<vector<BinaryDrag<conact>>> new_trees;
//    for (auto& tg : end_trees_) {
//        new_trees.emplace_back();
//        for (auto& t : tg) {
//            // Here Reduce() is used just to recreate trees 
//            BinaryDrag<conact> new_t;
//            new_t.SetRoot(Reduce(t.GetRoot(), new_t, {}));
//            new_trees.back().push_back(move(new_t));
//        }
//    }
//    end_trees_ = new_trees;
//
//}

// See RemoveUselessConditions
void RemoveUselessConditionsRec(BinaryDrag<conact>::node* n, bool& changed) {
    if (!n->isleaf()) {
        if (EqualTrees(n->left, n->right)) {
            changed = true;
            *n = *n->left;
            RemoveUselessConditionsRec(n, changed);
        }
        else {
            RemoveUselessConditionsRec(n->left, changed);
            RemoveUselessConditionsRec(n->right, changed);
        }
    }
}

// Removes useless conditions inside the forest.
// That means that the subtree:
//			a
//       0/  \1
//       c    ...
//    0/   \1
//    5     5
// will be transformed into:
//			a
//       0/  \1
//       5    ...
// these useless condition may appears after the execution of CreateReducedTrees.
void LineForestHandler::RemoveUselessConditions() {
    bool changed;
    do {
        changed = false;
        for (auto& r : f_.roots_) {
            RemoveUselessConditionsRec(r, changed);
        }
    } while (changed);
}
void LineForestHandler::RemoveEndTreesUselessConditions() {
    bool changed;
    do {
        changed = false;
        for (auto& f : end_forests_) {
            for (auto& t : f.roots_) {
                RemoveUselessConditionsRec(t, changed);
            }
        }
    } while (changed);
}

bool LineForestHandler::RemoveEqualEndTrees() {
    bool changed = false;
    for (size_t i = 0; i < end_forests_.size(); ++i) {
        changed |= RemoveTrees(EqualTrees, end_next_tree_[i], end_forests_[i], true, main_end_tree_mapping_[i]);
    }
    return changed;
}

bool LineForestHandler::RemoveEquivalentEndTrees() {
    // TODO 
    return false;
    //return RemoveEndTrees(equivalent_trees);
}

void LineForestHandler::UpdateNext(BinaryDrag<conact>::node* n) {
    if (n->isleaf()) {
        n->data.next = next_tree_[n->data.next];
    }
    else {
        UpdateNext(n->left);
        UpdateNext(n->right);
    }
}

bool LineForestHandler::RemoveEquivalentTrees() {
    return RemoveTrees(equivalent_trees, next_tree_, f_);
}

bool LineForestHandler::RemoveEqualTrees() {
    return RemoveTrees(EqualTrees, next_tree_, f_);
}

// Removes duplicate trees inside the forest
bool LineForestHandler::RemoveTrees(bool(*FunctionPtr)(const BinaryDrag<conact>::node* n1, const BinaryDrag<conact>::node* n2), 
                         vector<int>& next_tree, 
                         BinaryDrag<conact>& f, 
                         bool are_end_trees, 
                         vector<int>& mapping) {
    // Find which trees are identical and mark them in next_tree
    bool found = false;
    for (size_t i = 0; i < next_tree.size() - 1; ++i) {
        if (next_tree[i] == i) {
            for (size_t j = i + 1; j < next_tree.size(); ++j) {
                if (next_tree[j] == j) {
                    if (FunctionPtr(f.roots_[i], f.roots_[j])) {
                        next_tree[j] = static_cast<int>(i);
                        found = true;
                        if (FunctionPtr == equivalent_trees) {
                            IntersectTrees(f.roots_[i], f.roots_[j]);
                        }
                    }
                }
            }
        }
    }
    if (!found)
        return false;

    // Flatten the trees indexes
    size_t new_index = 0;
    for (size_t i = 0; i < next_tree.size(); ++i) {
        if (next_tree[i] == i) {
            next_tree[i] = static_cast<int>(new_index);
            ++new_index;
        }
        else {
            next_tree[i] = next_tree[next_tree[i]];
        }
    }

    // Remove roots that are now useless
    new_index = 0;
    for (size_t i = 0; i < next_tree.size(); ++i) {
        if (next_tree[i] != new_index) {
            f.roots_[i] = nullptr;
        }
        else {
            ++new_index;
        }
    }
    f.roots_.erase(remove(begin(f.roots_), end(f.roots_), nullptr), end(f.roots_));

    if (are_end_trees) {
        // If we are dealing with end trees then we need to update the 
        // mapping main-end trees because it changed
        for (size_t i = 0; i < mapping.size(); ++i) {
            mapping[i] = next_tree[mapping[i]];
        }
    }else{
        // If we are dealing with main trees then we need to update the 
        // id of the next tree because they changed
        for (auto& r : f.roots_) {
            UpdateNext(r);
        }
    }

    next_tree.resize(f.roots_.size());
    iota(begin(next_tree), end(next_tree), 0);
    return true;
}

// Initializes leave's next trees (drag) of a tree (drag) with sequential values.
void LineForestHandler::InitNextRec(BinaryDrag<conact>::node* n) {
    if (n->isleaf()) {
        // Set the next tree to be used for each leaf
        n->data.next = static_cast<uint>(next_tree_.size());
        // Setup a structure for managing equal trees
        next_tree_.push_back(static_cast<int>(next_tree_.size()));
    }
    else {
        InitNextRec(n->left);
        InitNextRec(n->right);
    }
}

// Perform tree pruning by removing useless nodes. Useless nodes are identified looking at given constraints 
BinaryDrag<conact>::node* LineForestHandler::Reduce(const BinaryDrag<conact>::node* n, BinaryDrag<conact>& t, const constraints& constr) {
    if (n->isleaf()) {
        return t.make_node(n->data);
    }
    else {
        auto it = constr.find(n->data.condition);
        if (it != end(constr)) {
            if (it->second == 0)
                return Reduce(n->left, t, constr);
            else
                return Reduce(n->right, t, constr);
        }
        else {
            return t.make_node(n->data, Reduce(n->left, t, constr), Reduce(n->right, t, constr));
        }
    }
}