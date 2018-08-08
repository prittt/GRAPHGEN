#ifndef TREESGENERATOR_TREE_H
#define TREESGENERATOR_TREE_H

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

        bool isleaf() const {
            return left == nullptr && right == nullptr;
        }
    };

    // Vector of unique pointers to tree nodes. This is useful to free memory when the tree is converted to DAG 
    std::vector<std::unique_ptr<node>> nodes;

    // Creates and returns new node updating the nodes vector
    node* make_node() {
        nodes.emplace_back(make_unique<node>());
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

#endif // !TREESGENERATOR_TREE_H

