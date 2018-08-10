#pragma once

#include <algorithm>
#include <numeric>
#include <map>

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

struct Forest {
    ltree t_;
    Equivalences eq_;

    std::vector<int> next_tree_;
    std::vector<ltree> trees_;

    Forest(ltree t, Equivalences eq);

    void RemoveUselessConditions();

    void UpdateNext(ltree::node* n);
    bool RemoveEqualTrees();

    void InitNextRec(ltree::node* n);
    void InitNext(ltree& t);

    static ltree::node* Reduce(const ltree::node* n, ltree& t, const constraints& constr);

    void CreateReducedTreesRec(const ltree::node* n, const constraints& constr = {});
    void CreateReducedTrees(const ltree& t);
};