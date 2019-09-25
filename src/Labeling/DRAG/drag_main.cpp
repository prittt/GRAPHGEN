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
#include <mutex>
#include <thread>

#include "pool.h"

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

    struct MagicOptimizer {
        struct STreeProp {
            string conditions_;
            vector<ltree::node*> leaves_;
            ltree::node* n_;

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
        unordered_map<ltree::node*, STreeProp> np_;

        STreeProp CollectStatsRec(ltree::node * n) {
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
                sp.conditions_ = n->data.condition;
                sp += CollectStatsRec(n->left);
                sp += CollectStatsRec(n->right);
            }

            np_[n] = sp;
            return sp;
        }
    };

    struct RemoveEqualSubtrees {
        std::unordered_map<std::string, ltree::node*> sp_; // string -> pointer
        std::unordered_map<ltree::node*, std::string> ps_; // pointer -> string
        uint nodes_ = 0, leaves_ = 0;

        string T2D(ltree::node*& n)
        {
            // Did we already find this node?
            auto itps = ps_.find(n);
            if (itps != end(ps_)) {
                // Yes, return the string
                return itps->second;
            }

            string s;
            if (n->isleaf()) {
                ++leaves_;
                //ss << setfill('0') << setw(3) << n->data.next;
                //s = n->data.action.to_string() + ss.str();
                auto& a = n->data.actions();
                s = '.' + to_string(a[0]);
                for (std::size_t i = 1; i < a.size(); ++i) {
                    s += ',' + to_string(a[i]);
                }
                s += '-' + to_string(n->data.next);
            }
            else {
                ++nodes_;
                auto sl = T2D(n->left);
                auto sr = T2D(n->right);
                s = n->data.condition + sl + sr;
            }

            auto it = sp_.find(s);
            if (it == end(sp_)) {
                sp_.insert({ s, n });
                ps_.insert({ n, s });
            }
            else {
                n = it->second;
                if (n->isleaf()) {
                    --leaves_;
                }
                else {
                    --nodes_;
                }
            }
            return s;
        }
    };

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
            RemoveEqualSubtrees sc;
            sc.T2D(t.root);

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
                bitset<128> bs;
                bs.set(actions[i] - 1);
                lma_[cur_leaf]->data.action = bs;
                GenerateAllTreesRec(cur_leaf + 1);
            }
            lma_[cur_leaf]->data.action = action_bs;
        }

        void GenerateAllTrees()
        {
            pool_ = new thread_pool();
            GenerateAllTreesRec(0);
            delete pool_;
        }
    };

    // Union-find (UF)
    class UF {
        // Maximum number of labels (included background) = 2^(sizeof(unsigned) x 8)
    public:
        UF(unsigned length) : length_{ length } {
            P_ = new unsigned[length];
            iota(P_, P_ + length, 0);
        }
        ~UF() {
            delete[] P_;
        }
        unsigned GetSet(unsigned index) {
            return P_[index];
        }

        unsigned Merge(unsigned i, unsigned j)
        {
            // FindRoot(i)
            while (P_[i] < i) {
                i = P_[i];
            }

            // FindRoot(j)
            while (P_[j] < j) {
                j = P_[j];
            }

            if (i < j)
                return P_[j] = i;
            return P_[i] = j;
        }

        unsigned Flatten()
        {
            unsigned k = 1;
            for (unsigned i = 1; i < length_; ++i) {
                if (P_[i] < i) {
                    P_[i] = P_[P_[i]];
                }
                else {
                    P_[i] = k;
                    k = k + 1;
                }
            }
            return k;
        }

    private:
        unsigned *P_;
        unsigned length_;
    };

    /*TLOG("Creating DRAG using equivalences",*/
        std::cout << "\n";
        auto t2 = t;
        RemoveEqualSubtrees sc;
        sc.T2D(t2.root);
        DrawDagOnFile("RemoveEqualSubtrees", t2, false);
        std::cout << "After equal subtrees removal: nodes = " << sc.nodes_ << " - leaves = " << sc.leaves_ << "\n";

        FindOptimalDrag c(t2);
        c.GenerateAllTrees();
        DrawDagOnFile("FindOptimalDrag", c.best_tree_, false);
        std::cout << "\n";
    /*);*/
    return 0;

    //auto t2 = t;
    //RemoveEqualSubtrees sc;
    //sc.T2D(t2.root);

    MagicOptimizer mo;
    mo.CollectStatsRec(t2.root);
    vector<MagicOptimizer::STreeProp> trees;
    for (const auto& x : mo.np_)
        trees.push_back(x.second);

    UF uf(trees.size());
    for (uint i = 0; i < trees.size(); ++i) {
        for (uint j = i + 1; j < trees.size(); ++j) {
            if (trees[i].equivalent(trees[j])) {
                uf.Merge(i, j);
            }
        }
    }
    auto nsets = uf.Flatten();

    // This map stores for each leaf the set (integer number given by 
    // the previous UF) of trees from which it belongs to.
    map<ltree::node*, std::set<uint>> lt;

    // The following nested loops populate the lt map. 
    // For each tree
    for (uint i = 0; i < trees.size(); ++i) {
        // For each leaf 
        auto& t = trees[i];
        for (uint j = 0; j < t.leaves_.size(); ++j) {
            auto& l = t.leaves_[j];
            lt[l].insert(uf.GetSet(i));
        }
    }

    std::vector<pair<ltree::node*, std::set<uint>>> vlt;
    for (const auto& x : lt) {
        vlt.push_back(make_pair(x.first, x.second));
    }

    //// FindOptimalDrag here is exploited just to find the vector of leaves with
    //// multiple actions, no more. 
    //FindOptimalDrag fod(t2);
    //auto& leaves = fod.lma_; // vector of leaves with multiple actions

    UF ufl(vlt.size());
    // For each leaf look to all the other leaves and if they belong both
    // to at least one (same) set of trees they must be in the same uf class
    // all the leaves belonging to the same set will be used to run the 
    // backtracking approach of the FindOptimalDrag (once per set separately).
    for (uint i = 0; i < vlt.size(); ++i) {
        if (vlt[i].first->data.actions().size() > 1) {
            for (uint j = i + 1; j < vlt.size(); ++j) {
                if (vlt[j].first->data.actions().size() > 1) {
                    std::vector<uint> intersection(nsets);
                    // This can be done by hand breaking the loop after finding the first intersection
                    auto it = std::set_intersection(begin(vlt[i].second), end(vlt[i].second), begin(vlt[j].second), end(vlt[j].second), begin(intersection));
                    // std::cout << " ";
                    if (it != begin(intersection)) {
                        // Non empty intersection
                        ufl.Merge(i, j);
                    }
                }
            }
        }
    }
    auto nsets_two = ufl.Flatten();
    // Appartengono tutte alla stessa classe ? :(

    // Vectors of vector of leaves with multiple actions
    std::vector<std::vector<ltree::node*>> vlma(nsets_two);
    for (uint i = 0; i < vlt.size(); ++i) {
        if (vlt[i].first->data.actions().size() > 1) {
            int leaf_class = ufl.GetSet(i);
            vlma[leaf_class].push_back(vlt[i].first);
        }
    }

    for (uint i = 0; i < vlma.size(); ++i) {
        if (vlma[i].size() != 0) {
            FindOptimalDrag tmp(t2); // TODO, change it: Calling this we call useless code that populate the lma_ vector.
            tmp.lma_ = vlma[i];
            tmp.GenerateAllTrees();
            t2 = tmp.best_tree_;
            DrawDagOnFile("optimal" + to_string(i), t2, false);
            std::cout << "\n";
        }
    }

    DrawDagOnFile("final_simplified_backtrack", t2, false);
    std::cout << "\n";

    return 0;
    /*
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
        return 0;

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