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

#include "tree2dag_identities.h"

// Search in tree n2 there is a subtree equal to n1, which is not n1
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