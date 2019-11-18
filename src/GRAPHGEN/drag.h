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

#ifndef GRAPHGEN_DRAG_H_
#define GRAPHGEN_DRAG_H_

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <istream>
#include <vector>

template<typename T>
class BinaryDrag {
public:
    struct node
    {
        T data;
        node *left = nullptr, *right = nullptr;
        //std::vector<node *> parents_;

        node() {}
        node(T d) : data(std::move(d)) {}
        node(T d, node* l, node* r) : data(std::move(d)), left(l), right(r) {}
        //node(T d, node* l, node* r, std::vector<node *> parents) : data(std::move(d)), left(l), right(r), parents_(parents) {}

        bool isleaf() const {
            return left == nullptr && right == nullptr;
        }
    };

    // Vector of unique pointers to tree nodes. This is useful to free memory when the tree is converted to DAG 
    std::vector<std::unique_ptr<node>> nodes_;

    // Creates and returns new node updating the nodes vector
    template<typename... Args>
    node* make_node(Args&&... args) {
        nodes_.emplace_back(std::make_unique<node>(std::forward<Args>(args)...));
        return nodes_.back().get();
    }

    std::vector<node *> roots_;

    // Adds a root to the vector of roots and returns its index
    int AddRoot(node* r) {
        roots_.push_back(r);
        return roots_.size() - 1;
    }

    node* GetRoot() const {
        return roots_.front();
    }

    void SetRoot(node *root) {
        roots_.push_back(root);
    }

    friend void swap(BinaryDrag& bd1, BinaryDrag& bd2) {
        using std::swap;
        swap(bd1.roots_, bd2.roots_);
        swap(bd1.nodes_, bd2.nodes_);
    }

    // Recursive function to copy a tree. This is required by the copy constructor
    // copies maps every node to be copied to its copy
    node *MakeCopyRecursive(node *n, std::unordered_map<node*, node*> &copies) {
        if (n == nullptr)
            return nullptr;

        if (!copies[n]) {
            node *nn = make_node();
            nn->data = n->data;
            nn->left = MakeCopyRecursive(n->left, copies);
            nn->right = MakeCopyRecursive(n->right, copies);
            return copies[n] = nn;
        }
        return copies[n];
    }

    BinaryDrag() {}

    // Copy constructor
    BinaryDrag(const BinaryDrag& bd) {
        std::unordered_map<node*, node*> copies;
        for (const auto& x : bd.roots_) {
            roots_.push_back(MakeCopyRecursive(x, copies));
        }
        /*for (const auto& n : bd.nodes_) {
            auto& nn = copies[n.get()];
            for (const auto& p : n->parents_) {
                nn->parents_.push_back(copies[p]);
            }
        }*/
    }

    BinaryDrag(const BinaryDrag& bd, std::vector<node*>& tracked_nodes) { // Allows to track where the nodes in a tree have been copied to
        std::unordered_map<node*, node*> copies;
        for (const auto& x : bd.roots_) {
            roots_.push_back(MakeCopyRecursive(x, copies));
        }
        /*for (const auto& n : bd.nodes_) {
            auto& nn = copies[n.get()];
            for (const auto& p : n->parents_) {
                nn->parents_.push_back(copies[p]);
            }
        }*/
        for (auto& n : tracked_nodes) {
            n = copies[n];
        }
    }

    BinaryDrag(BinaryDrag&& t) {
        swap(*this, t);
    }
    // Copy assignment
    BinaryDrag& operator=(BinaryDrag t) {
        swap(*this, t);
        return *this;
    }

    node* make_root() {
        roots_.push_back(make_node());
        return roots_.back();
    }
};

//class ConactBinaryDrag : BinaryDrag<conact> {
//public:
//    ConactBinaryDrag(std::istream& is) {
//        Load(is, *this);
//    }
//
//    bool Serialize(const std::string& filename) {
//        std::ofstream os(filename);
//
//        if (!os) {
//            return false;
//        }
//
//        return Serialize(os);
//    }
//
//    bool Serialize(std::ostream& os = std::cout) {
//        if (!os) {
//            return false;
//        }
//
//        Save(os, *this);
//        return true;
//    }
//
//private:
//    struct Save {
//        std::ostream& os_;
//        std::unordered_set<ConactBinaryDrag::node*> visited_;
//        std::unordered_map<ConactBinaryDrag::node*, int> nodes_with_refs_;
//        int id_ = 0;
//
//        void CheckNodesTraversalRec(BinaryDrag<conact>::node *n)
//        {
//            if (nodes_with_refs_.find(n) != end(nodes_with_refs_)) {
//                if (!nodes_with_refs_[n])
//                    nodes_with_refs_[n] = ++id_;
//            }
//            else {
//                nodes_with_refs_[n] = 0;
//                if (!n->isleaf()) {
//                    CheckNodesTraversalRec(n->left);
//                    CheckNodesTraversalRec(n->right);
//                }
//            }
//        }
//
//        Save(std::ostream& os, ConactBinaryDrag& bd) : os_(os)
//        {
//            for (auto& x : bd.roots_) {
//                CheckNodesTraversalRec(x);
//            }
//            for (auto& x : bd.roots_) {
//                SaveRec(x);
//            }
//        }
//
//        void SaveRec(ConactBinaryDrag::node* n, int tab = 0)
//        {
//            os_ << std::string(tab, '\t');
//
//            if (!visited_.insert(n).second) {
//                os_ << "@ " << nodes_with_refs_[n] << "\n";
//                return;
//            }
//
//            if (nodes_with_refs_[n]) {
//                os_ << "^ " << nodes_with_refs_[n] << " ";
//            }
//
//            if (n->isleaf()) {
//                assert(n->data.t == conact::type::ACTION);
//                auto a = n->data.actions();
//                os_ << ". " << a[0];
//                for (size_t i = 1; i < a.size(); ++i) {
//                    os_ << "," << a[i];
//                }
//                os_ << " - " << n->data.next << "\n";
//            }
//            else {
//                assert(n->data.t == conact::type::CONDITION);
//                os_ << n->data.condition << "\n";
//                SaveRec(n->left, tab + 1);
//                SaveRec(n->right, tab + 1);
//            }
//        }
//    };
//
//    struct Load {
//        std::istream& is_;
//        ConactBinaryDrag& bd_;
//        std::vector<ConactBinaryDrag::node*> np_;
//
//        Load(std::istream& is, ConactBinaryDrag& bd) : is_(is), bd_(bd)
//        {
//            np_.push_back(nullptr);
//            while (true) {
//                ConactBinaryDrag::node* n = LoadConactTreeRec();
//                if (n == nullptr)
//                    break;
//                bd_.AddRoot(n);
//            }
//        }
//
//        ConactBinaryDrag::node* LoadConactTreeRec()
//        {
//            std::string s;
//            while (is_ >> s) {
//                if (s[0] == '#') {
//                    getline(is_, s);
//                }
//                else {
//                    break;
//                }
//            }
//
//            if (!is_) {
//                return nullptr;
//            }
//
//            if (s == "@") {
//                int pos;
//                is_ >> pos;
//                return np_[pos];
//            }
//
//            auto n = bd_.make_node();
//
//            if (s == "^") {
//                int pos;
//                is_ >> pos >> s;
//                if (pos != np_.size()) {
//                    throw;
//                }
//                np_.push_back(n);
//            }
//
//            if (s == ".") {
//                // leaf
//                n->data.t = conact::type::ACTION;
//                do {
//                    int action;
//                    is_ >> action >> std::ws;
//                    n->data.action.set(action - 1);
//                } while (is_.peek() == ',' && is_.get());
//
//                if (is_.get() != '-') {
//                    throw;
//                }
//                is_ >> n->data.next;
//            }
//            else {
//                // real node with branches
//                n->data.t = conact::type::CONDITION;
//                n->data.condition = s;
//
//                n->left = LoadConactTreeRec();
//                n->right = LoadConactTreeRec();
//            }
//
//            return n;
//        }
//    };
//};

#endif // !GRAPHGEN_DRAG_H_
