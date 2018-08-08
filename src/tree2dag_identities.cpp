#include "tree2dag_identities.h"

void FindAndLinkIdentiesRec(ltree::node* n1, ltree::node* n2) {
    if (n2->isleaf() || n1 == n2)
        return;

    if (n1 != n2->left && EqualTrees(n1, n2->left)) {
        n2->left = n1;
    }

    if (n1 != n2->right && EqualTrees(n1, n2->right)) {
        n2->right = n1;
    }

    FindAndLinkIdentiesRec(n1, n2->left);
    FindAndLinkIdentiesRec(n1, n2->right);
}

void Tree2DagUsingIdentitiesRec(ltree::node *n, ltree& t) {
    FindAndLinkIdentiesRec(n, t.root);

    if (!n->isleaf()) {
        Tree2DagUsingIdentitiesRec(n->left, t);
        Tree2DagUsingIdentitiesRec(n->right, t);
    }
}

// Converts a tree into dag considering only equal subtrees
void Tree2DagUsingIdentities(ltree& t) {
    Tree2DagUsingIdentitiesRec(t.root, t);
}