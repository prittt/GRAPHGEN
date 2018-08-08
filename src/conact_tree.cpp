#include "conact_tree.h"

#include <cassert>

using namespace std;

static ltree::node* LoadConactTreeRec(ltree& t, ifstream& is)
{
	string s;
	while (is >> s) {
		if (s[0] == '#')
			getline(is, s);
		else
			break;
	}

	ltree::node* n = t.make_node();
	if (s == ".") {
		// leaf
		n->data.t = conact::type::ACTION;
		do {
			int action;
			is >> action >> ws;
			n->data.action |= 1 << (action - 1);
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

bool LoadConactTree(ltree& t, const string& filename)
{
    ifstream is(filename);
    if (!is) {
        return false;
    }

    t.root = LoadConactTreeRec(t, is);
    return true;
}

static void WriteConactTreeRec(const ltree::node* n, ofstream& os, size_t tab = 0)
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

bool WriteConactTree(const ltree& t, const string& filename)
{
    ofstream os(filename);
    if (!os)
        return false;
    WriteConactTreeRec(t.root, os);
    return true;
}

// Checks if two subtrees 'n1' and 'n2' are equivalent or not 
bool equivalent_trees(const ltree::node* n1, const ltree::node* n2) {
    if (n1->data.neq(n2->data))
        return false;

    if (n1->isleaf())
        return true;
    else
        return equivalent_trees(n1->left, n2->left) && equivalent_trees(n1->right, n2->right);
}

void intersect_leaves(ltree::node* n1, ltree::node* n2) {
    if (n1->isleaf()) {
        n2->data.action = n1->data.action &= n2->data.action;
    }
    else {
        intersect_leaves(n1->left, n2->left);
        intersect_leaves(n1->right, n2->right);
    }
}

// Checks if two (sub)trees 'n1' and 'n2' are equal
bool EqualTrees(const ltree::node* n1, const ltree::node* n2) {
    if (n1->data != n2->data)
        return false;

    if (n1->isleaf())
        return true;
    else
        return EqualTrees(n1->left, n2->left) && EqualTrees(n1->right, n2->right);
}

