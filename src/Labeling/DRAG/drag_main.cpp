// Copyright(c) 2018 - 2019 Costantino Grana, Federico Bolelli 
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

#include "yaml-cpp/yaml.h"

#include "graphsgen.h"

#include "grana_ruleset.h"

#include <unordered_set>

using namespace std;

int main()
{
    string algorithm_name = "DRAG";
    conf = ConfigData(algorithm_name);

    auto rs = GenerateGrana();

    // Call GRAPHSGEN:
    // 1) Load or generate Optimal Decision Tree based on Grana mask
    ltree t = GetOdt(rs, algorithm_name);

    // 2) Draw the generated tree to pdf
    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, t, false, true, false);

    auto t2 = t;
    RemoveEqualSubtrees sc;
    sc.T2D(t2.root);

    MagicOptimizer mo;
    mo.CollectStatsRec(t2.root);
    vector<MagicOptimizer::STreeProp> trees;
    for (const auto& x : mo.np_)
        trees.push_back(x.second);

    struct Culo {
        ltree::node* MergeEquivalentTreesRec(ltree::node* a, ltree::node* b, unordered_map<ltree::node*, ltree::node*> &merged)
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

        void DeleteTreeRec(ltree::node* n, unordered_map<ltree::node*, bool> &deleted)
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

        void PrintTreeRec(ostream& os, ltree::node* n, set<ltree::node*>& visited, int tab = 0) {
            os << string(tab, '\t');

            auto it = visited.find(n);
            if (it != end(visited)) {
                os << " -> " << n << "\n";
                return;
            }
            visited.insert(n);

            if (n->isleaf()) {
                os << ". ";
                auto a = n->data.actions();
                copy(begin(a), end(a), ostream_iterator<int>(os,", "));
                os << "\n";
            }
            else {
                os << n->data.condition << "\n";
                PrintTreeRec(os, n->left, visited, tab + 1);
                PrintTreeRec(os, n->right, visited, tab + 1);
            }

        }

        void MergeEquivalentTreesAndUpdateRec(ltree::node* a, ltree::node* b, unordered_map<ltree::node*, vector<ltree::node*>>& parents, unordered_set<ltree::node*>& visited)
        {
            auto it = visited.find(a);
            if (it != end(visited))
                return;
            visited.insert(a);

            for (auto& x : parents[b]) {
                if (x->left == b)
                    x->left = a;
                else
                    x->right = a;
            }

            if (a->isleaf()) {
                a->data.action &= b->data.action;
            }
            else {
                MergeEquivalentTreesAndUpdateRec(a->left, b->left, parents, visited);
                MergeEquivalentTreesAndUpdateRec(a->right, b->right, parents, visited);
            }
        }

        int count = 0;
        int best_nodes = numeric_limits<int>::max();
        int best_leaves = numeric_limits<int>::max();
        void FaiTutto(ltree& t)
        {
            MagicOptimizer mo;
            mo.CollectStatsRec(t.root);
            vector<MagicOptimizer::STreeProp> trees;
            for (const auto& x : mo.np_)
                trees.push_back(x.second);

            for (size_t i = 0; i < trees.size(); ) {
                if (trees[i].conditions_==".") {
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

                        vector<ltree::node*> tracked_nodes{ trees[i].n_, trees[j].n_ };
                        ltree t_copy(t, tracked_nodes);
                        MagicOptimizer mo;
                        mo.CollectStatsRec(t_copy.root);

                        unordered_set<ltree::node*> visited;
                        MergeEquivalentTreesAndUpdateRec(tracked_nodes[0], tracked_nodes[1], mo.parents_, visited);
                        RemoveEqualSubtrees sc;
                        sc.T2D(t_copy.root);

                        //{ ofstream os("after.txt"); set<ltree::node*> visited; PrintTreeRec(os, t_copy.root, visited); }
                        //DrawDagOnFile("After", t_copy, true);
                        FaiTutto(t_copy);
                    }
                }
            }
            if (no_eq) {
                ++count;
                if (count % 1000 == 0)
                    cout << "\r" << count;
                DragStatistics ds(t);
                if (ds.Nodes() < best_nodes || (ds.Nodes() == best_nodes) && ds.Leaves() < best_leaves) {
                    best_nodes = ds.Nodes();
                    best_leaves = ds.Leaves();
                    DrawDagOnFile("Culo" + zerostr(count, 4), t, true);
                    cout << count << " - nodes: " << ds.Nodes() << " - leaves: " << ds.Leaves() << "\n";
                }
            }
        }
    };

    Culo c;
    c.FaiTutto(t2);
    cout << "Done\n";

    //for (size_t i = 0; i < trees.size(); ) {
    //    bool eq = false;
    //    for (size_t j = 0; j < trees.size(); ++j) {
    //        if (i != j && trees[i].equivalent(trees[j])) {
    //            eq = true;
    //            break;
    //        }
    //    }
    //    if (!eq) {
    //        trees.erase(begin(trees) + i);
    //    }
    //    else {
    //        ++i;
    //    }
    //}


    {
        TLOG("Creating DRAG using equivalences",
            std::cout << "\n";
        auto t2 = t;
        RemoveEqualSubtrees sc;
        sc.T2D(t2.root);
        DrawDagOnFile("RemoveEqualSubtrees", t2, true);
        std::cout << "After equal subtrees removal: nodes = " << sc.nodes_ << " - leaves = " << sc.leaves_ << "\n";

        FindOptimalDrag c(t2);
        c.GenerateAllTrees();
        DrawDagOnFile("FindOptimalDrag", c.best_tree_, true);
        std::cout << "\n";
        );
    }
    return 0;
    /*
        // 2a) Convert Optimal Decision Tree into Directed Rooted Acyclic Graph
        //     using a exhaustive strategy
        TLOG("Creating DRAG using identites",
            Tree2DagUsingIdentities(t);
        );
        string drag_filename = algorithm_name + "_drag_identities";
        DrawDagOnFile(drag_filename, t, true);
        PrintStats(t);

        //string odrag_filename = algorithm_name + "_optimal_drag.txt";
        //if (!LoadConactDrag(t, odrag_filename)) {
        //    TLOG("Computing optimal DRAG\n",
        //        Dag2OptimalDag(t);
        //    );
        //    WriteConactDrag(t, odrag_filename);
        //}
        //string optimal_drag_filename = algorithm_name + "_optimal_drag";
        //DrawDagOnFile(optimal_drag_filename, t, true);
        //PrintStats(t);

        // 2b) Convert Optimal Decision Tree into Directed Rooted Acyclic Graph
        //     using a heuristic
        // TODO

        // 3) Generate the C++ source code for the ODT
        GenerateDragCode(algorithm_name, t);

        // 4) Generate the C++ source code for pointers,
        // conditions to check and actions to perform
        pixel_set block_positions{
              { "P", {-2, -2} },{ "Q", {+0, -2} },{ "R", {+2, -2} },
              { "S", {-2, +0} },{ "x", {+0, +0} }
        };
        GeneratePointersConditionsActionsCode(algorithm_name, rs, block_positions);
    */
    return EXIT_SUCCESS;
}