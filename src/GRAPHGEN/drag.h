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
#include <memory>
#include <unordered_map>
#include <vector>

template<typename T>
struct BinaryDrag {
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

#endif // !GRAPHGEN_DRAG_H_
