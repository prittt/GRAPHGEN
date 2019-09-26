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

#ifndef GRAPHSGEN_REMOVE_EQUAL_SUBTREES_H_
#define GRAPHSGEN_REMOVE_EQUAL_SUBTREES_H_

#include <unordered_set>

#include "conact_tree.h"


struct RemoveEqualSubtrees {
    std::unordered_map<std::string, ltree::node*> sp_; // string -> pointer
    std::unordered_map<ltree::node*, std::string> ps_; // pointer -> string
    uint nodes_ = 0, leaves_ = 0;

    RemoveEqualSubtrees(ltree::node*& n) {
        T2D(n);
    }

    std::string T2D(ltree::node*& n)
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
            auto& a = n->data.actions();
            s = '.' + std::to_string(a[0]);
            for (std::size_t i = 1; i < a.size(); ++i) {
                s += ',' + std::to_string(a[i]);
            }
            s += '-' + std::to_string(n->data.next);
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

#endif // GRAPHSGEN_REMOVE_EQUAL_SUBTREES_H_