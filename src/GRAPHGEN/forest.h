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

#ifndef GRAPHGEN_FOREST_H_
#define GRAPHGEN_FOREST_H_

#include <algorithm>
#include <map>
#include <numeric>

#include "base_forest.h"
#include "conact_tree.h"
#include "pixel_set.h"
#include "utilities.h"

struct Equivalences {
    using eq_type = std::map<std::string, std::string>;
    eq_type eq_;

    Equivalences() {}
    Equivalences(const pixel_set& ps) {
        using namespace std;
        for (auto p : ps) {
            p.ShiftX(ps.GetShiftX());
            auto it = find(begin(ps), end(ps), p);
            if (it != end(ps)) {
                eq_[it->name_] = p.name_;
            }
        }
    }

    struct FindType {
        const eq_type& eq_;
        eq_type::iterator it_;
        FindType(const eq_type& eq, eq_type::iterator it) : eq_{ eq }, it_{ it } {}
        operator bool() { return it_ != end(eq_); }
        operator std::string() { return it_->second; }
    };
    FindType Find(const std::string& s) {
        return { eq_, eq_.find(s) };
    }
};

using constraints = std::map<std::string, int>;
static std::vector<int> DEFAULT_VECTOR; // Dummy vector for the default value of DeleteTree member function

/** @brief Generates all the forests needed to handle one line of the image.
*/
using Forest = struct LineForestHandler; // TO BE REMOVED
struct LineForestHandler {

    /** @brief Creates forest of trees pruning original tree. 
    
    The pruning is performed as follows: the original tree is recursively explored and constraints (different on 
    each branch) are defined using equivalences between pixels (i.e. pixels which remain in the mask when it moves). 
    When a leaf is reached the original tree is reduced using current branch's constraints. All the attributes are
    temporary objects, so this class can be used as function.

    */
    struct CreateReducedDrag {
        const BinaryDrag<conact>& bd_;
        BinaryDrag<conact>& f_;
        Equivalences eq_;

        // bd is the original binary drag from which to generate the forest and f is where to write the trees
        CreateReducedDrag(const BinaryDrag<conact>& bd, BinaryDrag<conact>& f, const pixel_set& ps) : bd_{ bd }, f_{ f }, eq_{ps} {
            constraints constr;
            CreateReducedTreesRec(bd_.roots_[0], constr);
        }

        // See CreateReducedTrees 
        void CreateReducedTreesRec(const ltree::node* n, const constraints& constr) {
            if (n->isleaf()) {
                // Create a reduced version of the tree based on what we learned on the path to this leaf. The new tree is stored in t_ as well.
                f_.AddRoot(Reduce(bd_.roots_[0], f_, constr));
            }
            else {
                constraints constrNew = constr;
                auto ft = eq_.Find(n->data.condition);
                if (ft)
                    constrNew[ft] = 0;
                CreateReducedTreesRec(n->left, constrNew);
                if (ft)
                    constrNew[ft] = 1;
                CreateReducedTreesRec(n->right, constrNew);
            }
        }
    };

    bool separately = false; // TO BE REMOVED Specify if end trees from different groups should be treated separately during Tree2Dag conversion

    std::vector<int> next_tree_; // This vector contains the equivalences between main trees
    BinaryDrag<conact> f_;
    std::vector<ltree> trees_; // TO BE REMOVED

    std::vector<BinaryDrag<conact>> end_forests_;
    std::vector<std::vector<ltree>> end_trees_; // TO BE REMOVED

    std::vector<std::vector<int>> end_next_tree_; // This vectors contain the equivalences between end trees
    std::vector<std::vector<int>> main_end_tree_mapping_; // This is the mapping between main trees and end trees

    LineForestHandler() {}
    LineForestHandler(const BinaryDrag<conact>& t, const pixel_set& ps, const constraints& initial_constraints = {}); // Initial_constraints are useful to create particular forests such as the first line forest

    void RemoveUselessConditions();
    void RemoveEndTreesUselessConditions();

    void UpdateNext(ltree::node* n);

    // Removes duplicate trees
    bool RemoveEqualTrees();
    bool RemoveEquivalentTrees();

    // Removes duplicate end trees. This action is performed in each end forest separately, different
    // groups cannot contain equal end trees. Moreover, experimental result have proven that merging
    // end forest together is useless since they will never be used together when working on an image.
    bool RemoveEqualEndTrees();
    bool RemoveEquivalentEndTrees();

    void InitNextRec(ltree::node* n);

    static ltree::node* Reduce(const ltree::node* n, ltree& t, const constraints& constr);

    void CreateReducedTreesRec(const ltree::node* n, const constraints& constr = {});
    void CreateReducedTrees(const ltree& t, const constraints& initial_constr);

private:
    bool RemoveEqualEndTreesSeparately();
    bool RemoveEqualEndTreesJointly();

    void RebuildDisjointTrees();
    void RebuildDisjointEndTrees();

    // Actual implementation of RemoveEqualTrees, RemoveEquivalentTrees, RemoveEqualEndTrees, RemoveEquivalentEndTrees
    bool RemoveTrees(bool(*FunctionPrt)(const ltree::node* n1, const ltree::node* n2),
        std::vector<int>& next_tree,
        BinaryDrag<conact>& f,
        bool are_end_trees = false,
        std::vector<int>& mapping = DEFAULT_VECTOR);
};

#endif // !GRAPHGEN_FOREST_H_