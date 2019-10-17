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

#ifndef GRAPHSGEN_MAGIC_OPTIMIZER_H_
#define GRAPHSGEN_MAGIC_OPTIMIZER_H_

#include <algorithm>
#include <unordered_set>

#include "conact_tree.h"

// This class serves to efficiently collect information about each node/subtree of a tree starting from a node
// it both stores properties of each subtree (as string) and all the parents of each node.
struct CollectDragStatistics {

    // Utility class to calculate and store (sub)trees properties
    struct STreeProp {
        std::string conditions_;
        std::vector<BinaryDrag<conact>::node*> leaves_;
        BinaryDrag<conact>::node* n_;

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
    std::unordered_map<BinaryDrag<conact>::node*, STreeProp> np_; // Associate to each tree node its properties (STreeProp)
    std::unordered_map<BinaryDrag<conact>::node*, std::vector<BinaryDrag<conact>::node*>> parents_; // Associate to each tree node its parents (vector of nodes)

    CollectDragStatistics() {}
    CollectDragStatistics(BinaryDrag<conact>& bd) {
        for (const auto& t : bd.roots_) {
            CollectStatsRec(t);
        }
    }

    // Collect information about each node/subtree of a tree starting from a node
    STreeProp CollectStatsRec(BinaryDrag<conact>::node * n) {
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
            parents_[n->left].push_back(n);
            parents_[n->right].push_back(n);
            sp.conditions_ = n->data.condition;
            sp += CollectStatsRec(n->left);
            sp += CollectStatsRec(n->right);
        }

        np_[n] = sp;
        return sp;
    }
};

#endif // GRAPHSGEN_MAGIC_OPTIMIZER_H_
