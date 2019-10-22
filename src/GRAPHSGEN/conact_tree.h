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

#ifndef GRAPHSGEN_CONACT_TREE_H_
#define GRAPHSGEN_CONACT_TREE_H_

#include <fstream>

#include "condition_action.h"
#include "tree.h"

#include "drag.h"
//using ltree = tree<conact>;
using ltree = BinaryDrag<conact>;


/**
* Load tree from file. The tree must be saved in a file with the following structure:
*	
* x
*   	a (left son of x)
*   		c
*   			. 2
*   			. 3
*   		. 1,3,4 (list of actions)
*   	b (right son of x)
*   		c
*   			. 2
*   			. 3
*   		. 4
* @param [out] t: Loaded tree
* @param [in] filename: Name of the file (path) from which load the tree
*
* @return whether the tree has been loaded or not
*/
bool LoadConactTree(ltree& t, const std::string& filename);

// Write a tree into a txt file structured as in the example above
bool WriteConactTree(const ltree& t, const std::string& filename);

bool equivalent_trees(const ltree::node* n1, const ltree::node* n2);
void intersect_leaves(ltree::node* n1, ltree::node* n2);

// Checks if two (sub)trees 'n1' and 'n2' are equal
bool EqualTrees(const ltree::node* n1, const ltree::node* n2);

void IntersectTrees(ltree::node* n1, ltree::node* n2);

// Instead of defining a novel format to save DRAGS, we save them as trees, then link
// identical sub-trees. Since the order of traversal is the same, the result should be the same.
// Should...
bool LoadConactDrag(ltree& t, const std::string& filename);

// Instead of defining a novel format to save DRAGS, we save them as trees, then link
// identical sub-trees. Since the order of traversal is the same, the result should be the same.
// Should...
bool WriteConactDrag(ltree& t, const std::string& filename);

struct Save {
    std::ostream& os_;
    std::unordered_set<BinaryDrag<conact>::node*> visited_;
    std::unordered_map<BinaryDrag<conact>::node*, int> nodes_with_refs_;
    int id_ = 0;

    void CheckNodesTraversalRec(BinaryDrag<conact>::node *n)
    {
        if (nodes_with_refs_.find(n) != end(nodes_with_refs_)) {
            if (!nodes_with_refs_[n])
                nodes_with_refs_[n] = ++id_;
        }
        else {
            nodes_with_refs_[n] = 0;
            if (!n->isleaf()) {
                CheckNodesTraversalRec(n->left);
                CheckNodesTraversalRec(n->right);
            }
        }
    }

    Save(std::ostream& os, BinaryDrag<conact>& bd) : os_(os)
    {
        for (auto& x : bd.roots_) {
            CheckNodesTraversalRec(x);
        }
        for (auto& x : bd.roots_) {
            SaveRec(x);
        }
    }

    void SaveRec(BinaryDrag<conact>::node* n, int tab = 0)
    {
        os_ << std::string(tab, '\t');

        if (!visited_.insert(n).second) {
            os_ << "@ " << nodes_with_refs_[n] << "\n";
            return;
        }

        if (nodes_with_refs_[n]) {
            os_ << "^ " << nodes_with_refs_[n] << " ";
        }

        if (n->isleaf()) {
            assert(n->data.t == conact::type::ACTION);
            auto a = n->data.actions();
            os_ << ". " << a[0];
            for (size_t i = 1; i < a.size(); ++i) {
                os_ << "," << a[i];
            }
            os_ << " - " << n->data.next << "\n";
        }
        else {
            assert(n->data.t == conact::type::CONDITION);
            os_ << n->data.condition << "\n";
            SaveRec(n->left, tab + 1);
            SaveRec(n->right, tab + 1);
        }
    }
};

struct Load {
    std::istream& is_;
    BinaryDrag<conact>& bd_;
    std::unordered_map<int, BinaryDrag<conact>::node*> np_;

    Load(std::istream& is, BinaryDrag<conact>& bd) : is_(is), bd_(bd)
    {
        np_[0] = nullptr;
        while (true) {
            BinaryDrag<conact>::node* n = LoadConactTreeRec();
            if (n == nullptr)
                break;
            bd_.AddRoot(n);
        }
    }

    BinaryDrag<conact>::node* LoadConactTreeRec()
    {
        std::string s;
        while (is_ >> s) {
            if (s[0] == '#') {
                getline(is_, s);
            }
            else {
                break;
            }
        }

        if (!is_) {
            return nullptr;
        }

        if (s == "@") {
            int pos;
            is_ >> pos;
            return np_[pos];
        }

        auto n = bd_.make_node();

        if (s == "^") {
            int pos;
            is_ >> pos >> s;
            np_[pos] = n;
        }

        if (s == ".") {
            // leaf
            n->data.t = conact::type::ACTION;
            do {
                int action;
                is_ >> action >> std::ws;
                n->data.action.set(action - 1);
            } while (is_.peek() == ',' && is_.get());

            if (is_.get() != '-') {
                throw;
            }
            is_ >> n->data.next;
        }
        else {
            // real node with branches
            n->data.t = conact::type::CONDITION;
            n->data.condition = s;

            n->left = LoadConactTreeRec();
            n->right = LoadConactTreeRec();
        }

        return n;
    }
};

#endif // !GRAPHSGEN_CONACT_TREE_H_



