// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "conact_tree.h"
#include "drag2optimal.h"

#include <cassert>

using namespace std;

static BinaryDrag<conact>::node* LoadConactTreeRec(BinaryDrag<conact>& t, ifstream& is)
{
	string s;
	while (is >> s) {
		if (s[0] == '#')
			getline(is, s);
		else
			break;
	}

	BinaryDrag<conact>::node* n = t.make_node();
	if (s == ".") {
		// leaf
		n->data.t = conact::type::ACTION;
		do {
			int action;
			is >> action >> ws;
			n->data.action.set(action - 1);
		} while (is.peek() == ',' && is.get());
	}
	else {
		// real node with branches
		n->data.t = conact::type::CONDITION;
		n->data.condition = s;

		n->left = LoadConactTreeRec(t, is);
		n->right = LoadConactTreeRec(t, is);
	}

	return n;
}

bool LoadConactTree(BinaryDrag<conact>& t, const string& filename)
{
    ifstream is(filename);
    if (!is) {
        return false;
    }

    t.AddRoot(LoadConactTreeRec(t, is));
    return true;
}

static void WriteConactTreeRec(const BinaryDrag<conact>::node* n, ofstream& os, size_t tab = 0)
{
    os << string(tab, '\t');
    if (n->isleaf()) {
        assert(n->data.t == conact::type::ACTION);
        auto a = n->data.actions();
        os << ". " << a[0];
        for (size_t i = 1; i < a.size(); ++i)
            os << "," << a[i];
        os << "\n";
    }
    else {
        assert(n->data.t == conact::type::CONDITION);
        os << n->data.condition << "\n";
        WriteConactTreeRec(n->left, os, tab + 1);
        WriteConactTreeRec(n->right, os, tab + 1);
    }
}

bool WriteConactTree(const BinaryDrag<conact>& t, const string& filename)
{
    ofstream os(filename);
    if (!os)
        return false;
    WriteConactTreeRec(t.GetRoot(), os);
    return true;
}

// Checks if two subtrees 'n1' and 'n2' are equivalent or not 
bool equivalent_trees(const BinaryDrag<conact>::node* n1, const BinaryDrag<conact>::node* n2) {
    if (n1->data.neq(n2->data))
        return false;

    if (n1->isleaf())
        return true;
    else
        return equivalent_trees(n1->left, n2->left) && equivalent_trees(n1->right, n2->right);
}

void intersect_leaves(BinaryDrag<conact>::node* n1, BinaryDrag<conact>::node* n2) {
    if (n1->isleaf()) {
        n2->data.action = n1->data.action &= n2->data.action;
    }
    else {
        intersect_leaves(n1->left, n2->left);
        intersect_leaves(n1->right, n2->right);
    }
}

// Checks if two (sub)trees 'n1' and 'n2' are equal
bool EqualTrees(const BinaryDrag<conact>::node* n1, const BinaryDrag<conact>::node* n2) {
    if (n1->data != n2->data)
        return false;

    if (n1->isleaf())
        return true;
    else
        return EqualTrees(n1->left, n2->left) && EqualTrees(n1->right, n2->right);
}

// Works only with equivalent trees
void IntersectTrees(BinaryDrag<conact>::node* n1, BinaryDrag<conact>::node* n2) {
	if (n1->isleaf()) {
		n1->data.action &= n2->data.action;
		n2->data.action = n1->data.action;
	}
	else {
		IntersectTrees(n1->left, n2->left);
		IntersectTrees(n1->right, n2->right);
	}
}

bool LoadConactDrag(BinaryDrag<conact>& t, const string& filename)
{
	if (!LoadConactTree(t, filename))
		return false;

	Dag2DagUsingIdenties(t);

	return true;
}

bool WriteConactDrag(BinaryDrag<conact>& t, const string& filename)
{
	return WriteConactTree(t, filename);
}

