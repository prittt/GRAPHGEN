// Copyright(c) 2018 Costantino Grana, Federico Bolelli 
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
#include "pixel_set.h"

struct Equivalences {
    using eq_type = std::map<std::string, std::string>;
    eq_type eq_;

    Equivalences(const pixel_set& ps) {
        using namespace std;
        for (const auto& p : ps) {
            auto x = p.dx + ps.shift_;
            auto it = find_if(begin(ps), end(ps), [&x, &p](const pixel& q) { return x == q.dx && p.dy == q.dy; });
            if (it != end(ps)) {
                eq_[it->name] = p.name;
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

struct Forest : BaseForest {
    ltree t_;
    Equivalences eq_;

    std::vector<int> next_tree_;
    std::vector<ltree> trees_;

    std::vector<std::vector<ltree>> end_trees_;
	std::vector<std::vector<int>> end_trees_mapping_;

    Forest(ltree t, const pixel_set& ps);

    void RemoveUselessConditions();

    void UpdateNext(ltree::node* n);
    bool RemoveEqualTrees();

	bool RemoveEqualEndTrees();

    void InitNextRec(ltree::node* n);
    void InitNext(ltree& t);

    static ltree::node* Reduce(const ltree::node* n, ltree& t, const constraints& constr);

    void CreateReducedTreesRec(const ltree::node* n, const constraints& constr = {});
    void CreateReducedTrees(const ltree& t);
};

#endif // !GRAPHSGEN_FOREST_H_