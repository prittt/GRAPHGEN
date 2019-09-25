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

#ifndef GRAPHSGEN_FOREST_H_
#define GRAPHSGEN_FOREST_H_

#include <algorithm>
#include <map>
#include <numeric>

#include "base_forest.h"
#include "conact_tree.h"
//#include "forest_optimizer.h"
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

struct Forest {
    ltree t_;
    Equivalences eq_;

    bool separately = false; // Specify if end trees from different groups should be treated separately during Tree2Dag conversion

    std::vector<int> next_tree_; // This vector contains the equivalences between main trees
    std::vector<ltree> trees_;

    std::vector<std::vector<ltree>> end_trees_;
    std::vector<std::vector<int>> end_next_trees_; // This vector contains the equivalences between end trees
    std::vector<std::vector<int>> main_trees_end_trees_mapping_; // This is the mapping between main trees and end trees

    Forest() {}
    Forest(ltree t, const pixel_set& ps, const constraints& initial_constraints = {}); // Initial_constraints are useful to create particular forests such as the first line forest

    void RemoveUselessConditions();
    void RemoveEndTreesUselessConditions();

    void UpdateNext(ltree::node* n);
    bool RemoveEqualTrees();
    bool RemoveEquivalentTrees();
    bool RemoveTrees(bool(*FunctionPrt)(const ltree::node* n1, const ltree::node* n2));

    bool RemoveEqualEndTrees();
    bool RemoveEquivalentEndTrees();
    bool RemoveEndTrees(bool(*FunctionPtr)(const ltree::node* n1, const ltree::node* n2));


    void InitNextRec(ltree::node* n);
    void InitNext(ltree& t);

    static ltree::node* Reduce(const ltree::node* n, ltree& t, const constraints& constr);

    void CreateReducedTreesRec(const ltree::node* n, const constraints& constr = {});
    void CreateReducedTrees(const ltree& t, const constraints& initial_constr);

private:
    bool RemoveEqualEndTreesSeparately();
    bool RemoveEqualEndTreesJointly();

    void RebuildDisjointTrees();
    void RebuildDisjointEndTrees();
};


/** @brief This class allows to generate the forests associated to an algorithm when the pixel prediction is applied.

When applying pixel prediction optimization many different forest should be generated. The number of forests depends
on the mask size and on vertical shift size. This class considers only Rosenfeld and Grana masks. In the former case
the vertical shift is unitary and only first line forest and "main" forest are required. In the latter case the mask
has a vertical shift of 2 pixels. In this case four different forest are required:
    - first line
    - main
    - last line
    - single line

The forests generation requires the original decision tree associated to the algorithm on which apply the prediction
optimization and the pixel set associated to the mask.
*/
//class GenerateForests {
//
//#define MAIN_FOREST 1
//#define FIRST_LINE 2
//#define LAST_LINE 4
//#define SINGLE_LINE 8
//
//    std::map<int, Forest> f_;
//
//    // t is the tree from which generate the forests
//    ltree& t_;
//
//    // ps is the pixel set (i.e. the mask) from which
//    // the tree t generates
//    const pixel_set& ps_;
//
//public:
//
//    GenerateForests(ltree t, const pixel_set& ps, int flag = MAIN_FOREST | FIRST_LINE) : t_(t), ps_(ps)
//    {
//        // main forest generation
//        if (MAIN_FOREST & flag) {
//            f_[MAIN_FOREST] = Forest(t_, ps_);
//        }
//
//        // firstline forest generation
//        if (FIRST_LINE & flag) {
//            constraints first_line_constr;
//            for (const auto& p : ps_) {
//                if (p.GetDy() < 0)
//                    first_line_constr[p.name_] = 0;
//            }
//            f_[FIRST_LINE] = Forest(t_, ps_, first_line_constr);
//        }
//        // lastline forest forest generation
//        if (LAST_LINE & flag) {
//            constraints last_line_constr;
//            for (const auto& p : ps_) {
//                if (p.GetDy() > 0)
//                    last_line_constr[p.name_] = 0;
//            }
//            f_[LAST_LINE] = Forest(t_, ps_, last_line_constr);
//        }
//        // singleline forest generation
//        if (SINGLE_LINE & flag) {
//            constraints single_line_constr;
//            for (const auto& p : ps_) {
//                if (p.GetDy() != 0)
//                    single_line_constr[p.name_] = 0;
//            }
//            f_[SINGLE_LINE] = Forest(t_, ps_, single_line_constr);
//        }
//    }
//
//    void CompressForests() {
//        LOG("Reducing Forests",
//            for (auto& x : f_) {
//                STree st(x.second);
//            }
//        );
//    }
//
//    Forest& GetForest(int forest_id) {
//        return f_.at(forest_id);
//    }
//};

#endif // !GRAPHSGEN_FOREST_H_