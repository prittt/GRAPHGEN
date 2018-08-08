#include "conact_tree.h"

using namespace std;

ltree LoadConactTree(const char *filename) {
	ltree n;

	ifstream is(filename);
	if (!is.is_open()) {
		return n;
	}

	n.root = LoadConactTreeRec(is);
	return n;
}

ltree::node* LoadConactTreeRec(ifstream& is)
{
	string s;
	while (is >> s) {
		if (s[0] == '#')
			getline(is, s);
		else
			break;
	}

	ltree::node* n = new ltree::node;
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

		n->left = LoadConactTreeRec(is);
		n->right = LoadConactTreeRec(is);
	}

	return n;
}

bool WriteConactTree(const ltree& t, const char *filename) {

	return true;
}

bool WriteConactTreeRec(ofstream& os) {

	return true;
}