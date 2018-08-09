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

#ifndef GRAPHSGEN_TREE_H_
#define GRAPHSGEN_TREE_H_

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>

// BDRAG - This data structure allows to manage a Binary Directed Rooted Acyclic Graph,
// that can be both a standard tree or a DAG, with the sole limitation that every node
// shall have two children. In our case exactly two or zero.
template<typename T>
struct tree { 
    struct node {
        T data;
        node *left = nullptr, *right = nullptr;

        node() {}
        node(T d) : data(move(d)) {}
        node(T d, node* l, node* r) : data(move(d)), left(l), right(r) {}

        bool isleaf() const {
            return left == nullptr && right == nullptr;
        }
    };

    // Vector of unique pointers to tree nodes. This is useful to free memory when the tree is converted to DAG 
    std::vector<std::unique_ptr<node>> nodes;

    // Creates and returns new node updating the nodes vector
    template<typename... Args>
    node* make_node(Args&&... args) {
        nodes.emplace_back(std::make_unique<node>(std::forward<Args>(args)...));
        return nodes.back().get();
    }

    node *root = nullptr;

    friend void swap(tree& t1, tree& t2) {
        using std::swap;
        swap(t1.root, t2.root);
        swap(t1.nodes, t2.nodes);
    }

    // Recursive function to copy a tree. This is required by the copy constructor
    // It works for both trees and dags
    node *MakeCopyRecursive(node *n, std::unordered_map<node*, node*> &copies) {
        if (n == nullptr)
            return nullptr;

        if (!copies[n]) {
            node *nn = make_node();
            nn->data = n->data;
            nn->left = MakeCopyRecursive(n->left, copies);
            nn->right = MakeCopyRecursive(n->right, copies);
            copies[n] = nn;
        }
        return copies[n];
    }

    tree() {}

    // Copy constructor
    tree(const tree& t) {
        std::unordered_map<node*, node*> copies;
        root = MakeCopyRecursive(t.root, copies);
    }
    tree(tree&& t) {
        swap(*this, t);
    }
    // Copy assignment
    tree& operator=(tree t) {
        swap(*this, t);
        return *this;
    }

    node* make_root() { return root = make_node(); }

    template<typename fn>
    void preorder_rec(node *n, fn& f, int i) {
        if (n == nullptr)
            return;
        node *left = n->left, *right = n->right;
        f(n, i);
        preorder_rec(left, f, i + 1);
        preorder_rec(right, f, i + 1);
    }

    template<typename fn>
    void preorder(fn& f) {
        preorder_rec(root, f, 0);
    }

    void clear() {
        nodes.clear();
        root = nullptr;
    }
};

#endif // !GRAPHSGEN_TREE_H_

