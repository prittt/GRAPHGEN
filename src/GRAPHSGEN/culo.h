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

struct Culo {
    ltree::node* MergeEquivalentTreesRec(ltree::node* a, ltree::node* b, std::unordered_map<ltree::node*, ltree::node*> &merged)
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
    }

    void PrintTreeRec(std::ostream& os, ltree::node* n, std::set<ltree::node*>& visited, int tab = 0) {
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

    }

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

    Culo(ltree& t) {
        FaiTuttoRec(t);
    }

    int count = 0;
    int best_nodes = std::numeric_limits<int>::max();
    int best_leaves = std::numeric_limits<int>::max();
    void FaiTuttoRec(ltree& t)
    {
        MagicOptimizer mo(t.root);
        std::vector<MagicOptimizer::STreeProp> trees;
        for (const auto& x : mo.np_)
            trees.push_back(x.second);

        for (size_t i = 0; i < trees.size(); ) {
            if (trees[i].conditions_ == ".") {
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

                    //DrawDagOnFile("Before", t, true);

                    std::vector<ltree::node*> tracked_nodes{ trees[i].n_, trees[j].n_ };
                    ltree t_copy(t, tracked_nodes);
                    
                    MagicOptimizer mo(t_copy.root);

                    MergeEquivalentTreesAndUpdate(tracked_nodes[0], tracked_nodes[1], mo.parents_);

                    RemoveEqualSubtrees(t_copy.root);

                    //{ ofstream os("after.txt"); set<ltree::node*> visited; PrintTreeRec(os, t_copy.root, visited); }
                    //DrawDagOnFile("After", t_copy, true);
                    FaiTuttoRec(t_copy);
                }
            }
        }
        if (no_eq) {
            ++count;
            if (count % 1000 == 0)
                std::cout << "\r" << count;
            DragStatistics ds(t);
            if (ds.Nodes() < best_nodes || (ds.Nodes() == best_nodes) && ds.Leaves() < best_leaves) {
                best_nodes = ds.Nodes();
                best_leaves = ds.Leaves();
                DrawDagOnFile("Culo" + zerostr(count, 4), t, true);
                std::cout << count << " - nodes: " << ds.Nodes() << " - leaves: " << ds.Leaves() << "\n";
            }
        }
    }
};

#endif // GRAPHSGEN_CULO_H_