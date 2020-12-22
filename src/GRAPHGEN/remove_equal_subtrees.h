// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_REMOVE_EQUAL_SUBTREES_H_
#define GRAPHGEN_REMOVE_EQUAL_SUBTREES_H_

#include <unordered_set>

#include "conact_tree.h"

/** @brief This class allows to "remove" equal subtrees from a BinaryDrag.

The class updates the input BinaryDrag itself so there is no need to build
a non temporary object. Thus you can do: RemoveEqualSubtrees{bd}.
The recursive procedure to find equal subtrees exploit memoization so it is
quite efficient. Please note that the removal of equal subtrees is performed 
updating the links but nodes are not actually deleted.
*/
struct RemoveEqualSubtrees {
    std::unordered_map<std::string, BinaryDrag<conact>::node*> sp_; // string -> pointer
    std::unordered_map<BinaryDrag<conact>::node*, std::string> ps_; // pointer -> string
    uint nodes_ = 0, leaves_ = 0;

    RemoveEqualSubtrees(BinaryDrag<conact>& bd) {
        for (auto& t : bd.roots_) {
            RemoveEqualSubtreesRec(t);
        }
    }

    std::string RemoveEqualSubtreesRec(BinaryDrag<conact>::node*& n)
    {
        // Did we already find this node?
        auto itps = ps_.find(n);
        if (itps != end(ps_)) {
            // Yes, return the string
            return itps->second;
        }

        std::string s;
        if (n->isleaf()) {
            ++leaves_;
            //ss << setfill('0') << setw(3) << n->data.next;
            //s = n->data.action.to_string() + ss.str();
            const std::vector<uint>& a = n->data.actions();
            s = '.' + std::to_string(a[0]);
            for (std::size_t i = 1; i < a.size(); ++i) {
                s += ',' + std::to_string(a[i]);
            }
            s += '-' + std::to_string(n->data.next);
        }
        else {
            ++nodes_;
            auto sl = RemoveEqualSubtreesRec(n->left);
            auto sr = RemoveEqualSubtreesRec(n->right);
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

#endif // GRAPHGEN_REMOVE_EQUAL_SUBTREES_H_