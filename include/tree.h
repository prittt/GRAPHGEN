#ifndef TREESGENERATOR_TREE_H
#define TREESGENERATOR_TREE_H

#include <vector>
#include <memory>

template<typename T>
struct tree {
    struct node {
        T data;
        node *left = nullptr, *right = nullptr;

        bool isleaf() const {
            return left == nullptr && right == nullptr;
        }
    };

    // List of unique pointers to tree nodes. This is useful to free memory when the tree is converted to DAG 
    std::vector<std::unique_ptr<node>> nodes;

    // Creates and returns new node updating the nodes vector
    node* make_node() {
        nodes.emplace_back(make_unique<node>());
        return nodes.back().get();
    }

    node *root = nullptr;

    friend void swap(tree& t1, tree& t2) {
        swap(t1.root, t2.root);
    }

    // Recursive function to copy a tree. This is required by the copy constructor
    node *make_copy_rec(node *n) {
        if (n == nullptr)
            return nullptr;
        node *nn = make_node();
        nn->data = n->data;
        nn->left = make_copy_rec(n->left);
        nn->right = make_copy_rec(n->right);
        return nn;
    }

    tree() {}

    // Copy constructor
    tree(const tree& t) {
        root = make_copy_rec(t.root);
    }
    tree(tree&& t) {
        swap(root, t.root);
    }
    tree& operator=(tree t) {
        swap(*this, t);
        return *this;
    }

    void CopyTo(tree &t) {
        t.root = make_copy_rec(this->root);
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

