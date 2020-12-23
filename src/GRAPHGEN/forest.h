// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_FOREST_H_
#define GRAPHGEN_FOREST_H_

#include <algorithm>
#include <map>
#include <numeric>

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
static std::vector<size_t> DEFAULT_VECTOR; // Dummy vector for the default value of DeleteTree member function

/** @brief Generates all the forests needed to handle one line of the image.
*/
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
        void CreateReducedTreesRec(const BinaryDrag<conact>::node* n, const constraints& constr) {
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

    
    BinaryDrag<conact> f_;
    std::vector<BinaryDrag<conact>> end_forests_;

    std::vector<size_t> next_tree_; // This vector contains the equivalences between main trees
    
    std::vector<std::vector<size_t>> end_next_tree_; // This vectors contain the equivalences between end trees
    std::vector<std::vector<size_t>> main_end_tree_mapping_; // This is the mapping between main trees and end trees

    LineForestHandler() {}
    LineForestHandler(const BinaryDrag<conact>& t, const pixel_set& ps, const constraints& initial_constraints = {}); // Initial_constraints are useful to create particular forests such as the first line forest

    void RemoveUselessConditions();
    void RemoveEndTreesUselessConditions();

    void UpdateNext(BinaryDrag<conact>::node* n);

    // Removes duplicate trees
    bool RemoveEqualTrees();
    bool RemoveEquivalentTrees();

    // Removes duplicate end trees. This action is performed in each end forest separately, different
    // groups cannot contain equal end trees. Moreover, experimental result have proven that merging
    // end forest together is useless since they will never be used together when working on an image.
    bool RemoveEqualEndTrees();
    bool RemoveEquivalentEndTrees();

    void InitNextRec(BinaryDrag<conact>::node* n);

    static BinaryDrag<conact>::node* Reduce(const BinaryDrag<conact>::node* n, BinaryDrag<conact>& t, const constraints& constr);

    void CreateReducedTreesRec(const BinaryDrag<conact>::node* n, const constraints& constr = {});
    void CreateReducedTrees(const BinaryDrag<conact>& t, const constraints& initial_constr);

private:
    bool RemoveEqualEndTreesSeparately();
    bool RemoveEqualEndTreesJointly();

    void RebuildDisjointTrees();
    void RebuildDisjointEndTrees();

    // Actual implementation of RemoveEqualTrees, RemoveEquivalentTrees, RemoveEqualEndTrees, RemoveEquivalentEndTrees
    bool RemoveTrees(bool(*FunctionPrt)(const BinaryDrag<conact>::node* n1, const BinaryDrag<conact>::node* n2),
        std::vector<size_t>& next_tree,
        BinaryDrag<conact>& f,
        bool are_end_trees = false,
        std::vector<size_t>& mapping = DEFAULT_VECTOR);
};

#endif // !GRAPHGEN_FOREST_H_