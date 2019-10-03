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
    struct node
	{
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

private:
    node *root_ = nullptr;
public:

    node* GetRoot() const {
        return root_;
    }

    node* GetRoot() {
        return root_;
    }

    void SetRoot(node *root) {
        root_ = root;
    }

    friend void swap(tree& t1, tree& t2) {
        using std::swap;
        swap(t1.root_, t2.root_);
        swap(t1.nodes, t2.nodes);
    }

    // Recursive function to copy a tree. This is required by the copy constructor
    // It works for both trees and dags
    node *MakeCopyRecursive(node *n, std::unordered_map<node*, node*> &copies, std::vector<node*>& tracked_nodes) {
        if (n == nullptr)
            return nullptr;

        if (!copies[n]) {
            node *nn = make_node();
            nn->data = n->data;
            nn->left = MakeCopyRecursive(n->left, copies, tracked_nodes);
            nn->right = MakeCopyRecursive(n->right, copies, tracked_nodes);
            copies[n] = nn;
            auto it = find(begin(tracked_nodes), end(tracked_nodes), n);
            if (it != end(tracked_nodes)) {
                *it = nn;
            }
        }
        return copies[n];
    }

    tree() {}

    // Copy constructor
    tree(const tree& t) {
        std::unordered_map<node*, node*> copies;
        std::vector<node*> tracked_nodes;
        root_ = MakeCopyRecursive(t.root_, copies, tracked_nodes);
    }
    tree(const tree& t, std::vector<node*>& tracked_nodes) { // Allows to track where the nodes in a tree have been copied to
        std::unordered_map<node*, node*> copies;
        root_ = MakeCopyRecursive(t.root_, copies, tracked_nodes);
    }
    tree(tree&& t) {
        swap(*this, t);
    }
    // Copy assignment
    tree& operator=(tree t) {
        swap(*this, t);
        return *this;
    }

    node* make_root() { return root_ = make_node(); }
};

#endif // !GRAPHSGEN_TREE_H_

