#include "forest.h"

using namespace std;

Forest::Forest(ltree t, Equivalences eq) : t_(std::move(t)), eq_(std::move(eq)) {
    InitNext(t_);
    CreateReducedTrees(t_);
    while (RemoveEqualTrees()) {
        RemoveUselessConditions();
    }
}

void RemoveUselessConditionsRec(ltree::node* n) {
    if (!n->isleaf()) {
        if (EqualTrees(n->left, n->right)) {
            *n = *n->left;
            RemoveUselessConditionsRec(n);
        }
        else {
            RemoveUselessConditionsRec(n->left);
            RemoveUselessConditionsRec(n->right);
        }
    }
}
void Forest::RemoveUselessConditions() {
    for (auto& t : trees_) {
        RemoveUselessConditionsRec(t.root);
    }
}

void Forest::UpdateNext(ltree::node* n) {
    if (n->isleaf()) {
        n->data.next = next_tree_[n->data.next];
    }
    else {
        UpdateNext(n->left);
        UpdateNext(n->right);
    }
}
bool Forest::RemoveEqualTrees() {
    // Find which trees are identical and mark them in next_tree
    bool found = false;
    for (size_t i = 0; i < next_tree_.size() - 1; ++i) {
        if (next_tree_[i] == i) {
            for (size_t j = i + 1; j < next_tree_.size(); ++j) {
                if (next_tree_[j] == j) {
                    if (EqualTrees(trees_[i].root, trees_[j].root)) {
                        next_tree_[j] = i;
                        found = true;
                    }
                }
            }
        }
    }
    if (!found)
        return false;

    // Flatten the trees indexes
    size_t new_index = 0;
    for (size_t i = 0; i < next_tree_.size(); ++i) {
        if (next_tree_[i] == i) {
            next_tree_[i] = new_index;
            ++new_index;
        }
        else {
            next_tree_[i] = next_tree_[next_tree_[i]];
        }
    }

    // Remove trees which are identical to already inserted ones
    new_index = 0;
    vector<ltree> trees;
    for (size_t i = 0; i < next_tree_.size(); ++i) {
        if (next_tree_[i] == new_index) {
            trees.push_back(trees_[i]);
            ++new_index;
        }
    }
    trees_ = move(trees);

    for (auto& t : trees_) {
        UpdateNext(t.root);
    }

    next_tree_.resize(trees_.size());
    iota(begin(next_tree_), end(next_tree_), 0);
    return true;
}

void Forest::InitNextRec(ltree::node* n) {
    if (n->isleaf()) {
        // Set the next tree to be used for each leaf
        n->data.next = next_tree_.size();
        // Setup a structure for managing equal trees
        next_tree_.push_back(next_tree_.size());
    }
    else {
        InitNextRec(n->left);
        InitNextRec(n->right);
    }
}
void Forest::InitNext(ltree& t) {
    InitNextRec(t_.root);
}

ltree::node* Forest::Reduce(const ltree::node* n, ltree& t, const constraints& constr) {
    if (n->isleaf()) {
        return t.make_node(n->data);
    }
    else {
        auto it = constr.find(n->data.condition);
        if (it != end(constr)) {
            if (it->second == 0)
                return Reduce(n->left, t, constr);
            else
                return Reduce(n->right, t, constr);
        }
        else {
            return t.make_node(n->data, Reduce(n->left, t, constr), Reduce(n->right, t, constr));
        }
    }
}

void Forest::CreateReducedTreesRec(const ltree::node* n, const constraints& constr) {
    if (n->isleaf()) {
        // Create a reduced version of the tree based on what we learned on the path to this leaf        
        ltree t;
        t.root = Reduce(t_.root, t, constr);
        trees_.emplace_back(t);
    }
    else {
        constraints constrNew = constr;
        auto ft = eq_.Find(n->data.condition);
        if (ft)
            constrNew[ft] = 0;
        CreateReducedTreesRec(n->left, constrNew);
        if (ft)
            constrNew[ft] = 1;
        CreateReducedTreesRec(n->right, constrNew);
    }
}

void Forest::CreateReducedTrees(const ltree& t) {
    CreateReducedTreesRec(t_.root);
}