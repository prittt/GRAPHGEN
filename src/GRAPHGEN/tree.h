// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_TREE_H_
#define GRAPHGEN_TREE_H_

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>

// BDRAG - This data structure allows to manage a Binary Directed Rooted Acyclic Graph,
// that can be both a standard tree or a DAG, with the sole limitation that every node
// shall have two children. In our case exactly two or zero.
//template<typename T>
//struct tree { 
//    struct node
//	{
//        T data;
//        node *left = nullptr, *right = nullptr;
//
//        node() {}
//        node(T d) : data(move(d)) {}
//        node(T d, node* l, node* r) : data(move(d)), left(l), right(r) {}
//
//        bool isleaf() const {
//            return left == nullptr && right == nullptr;
//        }
//    };
//
//    // Vector of unique pointers to tree nodes. This is useful to free memory when the tree is converted to DAG 
//    std::vector<std::unique_ptr<node>> nodes;
//
//    // Creates and returns new node updating the nodes vector
//    template<typename... Args>
//    node* make_node(Args&&... args) {
//        nodes.emplace_back(std::make_unique<node>(std::forward<Args>(args)...));
//        return nodes.back().get();
//    }
//
//private:
//    node *root_ = nullptr;
//public:
//
//    node* GetRoot() const {
//        return root_;
//    }
//
//    node* GetRoot() {
//        return root_;
//    }
//
//    void SetRoot(node *root) {
//        root_ = root;
//    }
//
//    friend void swap(tree& t1, tree& t2) {
//        using std::swap;
//        swap(t1.root_, t2.root_);
//        swap(t1.nodes, t2.nodes);
//    }
//
//    // Recursive function to copy a tree. This is required by the copy constructor
//    // It works for both trees and dags
//    node *MakeCopyRecursive(node *n, std::unordered_map<node*, node*> &copies, std::vector<node*>& tracked_nodes) {
//        if (n == nullptr)
//            return nullptr;
//
//        if (!copies[n]) {
//            node *nn = make_node();
//            nn->data = n->data;
//            nn->left = MakeCopyRecursive(n->left, copies, tracked_nodes);
//            nn->right = MakeCopyRecursive(n->right, copies, tracked_nodes);
//            copies[n] = nn;
//            auto it = find(begin(tracked_nodes), end(tracked_nodes), n);
//            if (it != end(tracked_nodes)) {
//                *it = nn;
//            }
//        }
//        return copies[n];
//    }
//
//    tree() {}
//
//    // Copy constructor
//    tree(const tree& t) {
//        std::unordered_map<node*, node*> copies;
//        std::vector<node*> tracked_nodes;
//        root_ = MakeCopyRecursive(t.root_, copies, tracked_nodes);
//    }
//    tree(const tree& t, std::vector<node*>& tracked_nodes) { // Allows to track where the nodes in a tree have been copied to
//        std::unordered_map<node*, node*> copies;
//        root_ = MakeCopyRecursive(t.root_, copies, tracked_nodes);
//    }
//    tree(tree&& t) {
//        swap(*this, t);
//    }
//    // Copy assignment
//    tree& operator=(tree t) {
//        swap(*this, t);
//        return *this;
//    }
//
//    node* make_root() { return root_ = make_node(); }
//};

#endif // !GRAPHGEN_TREE_H_

