#include <iostream>
#include <iterator>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdint>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "hypercube.h"

using namespace std;

#define LOG(message, instructions) cout << (message) << "... "; instructions cout << "done.\n"

template<typename T>
struct tree {
	struct node {
		T data;
		node *left = nullptr, *right = nullptr;

		node* make_left() { return left = new node; }
		node* make_right() { return right = new node; }
	};

	node *root = nullptr;

	friend void swap(tree& t1, tree& t2) {
		swap(t1.root, t2.root);
	}

	tree() {}
	tree(const tree&) = delete;
	tree(tree&& t) {
		swap(root, t.root);
	}
	tree& operator=(tree t) {
		swap(*this, t);
		return *this;
	}

	node* make_root() { return root = new node; }

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

	~tree() {
		preorder([](node *n, int i) { delete n; });
	}
};

// Condition or action
struct conact {
	bool condition;
	uint val;
};

void CreateTree_rec(tree<conact>::node *n, const VHyperCube &hcube, const VIndex &idx) {
	VNode node = hcube[idx];
	if (node.uiAction == 0) {
		n->data.condition = true;
		n->data.val = node.uiMaxGainIndex;

		// Estraggo i due (n-1)-cubi
		string sChild0(idx.GetIndexString());
		string sChild1(sChild0);
		sChild0[node.uiMaxGainIndex] = '0';
		sChild1[node.uiMaxGainIndex] = '1';
		VIndex idx0(sChild0), idx1(sChild1);

		CreateTree_rec(n->make_left(), hcube, idx0);
		CreateTree_rec(n->make_right(), hcube, idx1);
	}
	else {
		n->data.condition = false;
		unsigned uAction = node.uiAction;
		n->data.val = 0;
		while (uAction != 0) {
			uAction >>= 1;
			n->data.val++;
		}
	}
}

tree<conact> CreateTree(const VHyperCube &hcube) {
	tree<conact> t;
	CreateTree_rec(t.make_root(), hcube, string(hcube.m_iDim, '-'));
	return t;
}

int main()
{
	pixel_set rosenfeld_mask{
		{ "p", -1, -1 }, { "q", 0, -1 }, { "r", +1, -1 },
		{ "s", -1,  0 }, { "x", 0, 0 },
	};

	rule_set labeling;
	labeling.init_conditions(rosenfeld_mask);
	labeling.init_actions({ "nothing", "x<-newlabel", "x<-p", "x<-q", "x<-r", "x<-s", "x<-p+r", "x<-s+r", });

	labeling.generate_rules([](rule_set& rs, uint i) {
		if (rs.get_condition("x", i) == 0) {
			rs.set_action("nothing", i);
			return;
		}

		if (rs.get_condition("p", i) == 1 && rs.get_condition("q", i) == 0 && rs.get_condition("r", i) == 1)
			rs.set_action("x<-p+r", i);
		if (rs.get_condition("s", i) == 1 && rs.get_condition("q", i) == 0 && rs.get_condition("r", i) == 1)
			rs.set_action("x<-s+r", i);
		if (rs.rules[i].actions != 0)
			return;

		if (rs.get_condition("p", i) == 1)
			rs.set_action("x<-p", i);
		if (rs.get_condition("q", i) == 1)
			rs.set_action("x<-q", i);
		if (rs.get_condition("r", i) == 1)
			rs.set_action("x<-r", i);
		if (rs.get_condition("s", i) == 1)
			rs.set_action("x<-s", i);
		if (rs.rules[i].actions != 0)
			return;

		rs.set_action("x<-newlabel", i);
	});

	labeling.print_rules(cout);


	auto& rs = labeling;
	auto nvars = rs.conditions.size();
	auto nrules = rs.rules.size();

	LOG("Allocating hypercube",
		VHyperCube hcube(nvars);
	);

	LOG("Initializing rules",
		hcube.initialize_rules(labeling);
	);

	LOG("Optimizing rules",
		hcube.optimize(true);
	);


	auto t = CreateTree(hcube);

	t.preorder([&](tree<conact>::node *n, int i) {
		cout << string(i, '\t');
		if (n->data.condition) {
			cout << rs.conditions[rs.conditions.size() - 1 - n->data.val];
		}
		else {
			cout << ". " << n->data.val;
		}
		cout << "\n";
	});
}