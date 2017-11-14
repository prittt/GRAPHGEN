#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdint>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <memory>

#include "hypercube.h"

using namespace std;

#define LOG(message, instructions) cout << (message) << "... "; instructions cout << "done.\n"

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
    vector<unique_ptr<node>> nodes;
    
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

// Condition or action
struct conact {
    enum class type { CONDITION, ACTION };

    type t;
    // CONDITION
    string condition;
    // ACTION
    uint action = 0; // List of actions (bitmapped)
    uint next = 0;

    vector<uint> actions() const {
        vector<uint> a;
        uint uAction = action;
        uint nAction = 1;
        while (uAction != 0) {
            if (uAction & 1)
                a.push_back(nAction);
            uAction >>= 1;
            nAction++;
        }
        return a;
    }

    bool operator==(const conact& other) const {
        if (t != other.t)
            return false;
        if (t == type::CONDITION)
            return condition == other.condition;
        else
            return (action & other.action) && next == other.next;
    }
    bool operator!=(const conact& other) const {
        return !(*this == other);
    }
};

void CreateTree_rec(tree<conact>& t, tree<conact>::node *n, const rule_set& rs, const VHyperCube &hcube, const VIndex &idx) {
    VNode node = hcube[idx];
    if (node.uiAction == 0) {
        n->data.t = conact::type::CONDITION;
        n->data.condition = rs.conditions[rs.conditions.size() - 1 - node.uiMaxGainIndex];

        // Estraggo i due (n-1)-cubi
        string sChild0(idx.GetIndexString());
        string sChild1(sChild0);
        sChild0[node.uiMaxGainIndex] = '0';
        sChild1[node.uiMaxGainIndex] = '1';
        VIndex idx0(sChild0), idx1(sChild1);

        CreateTree_rec(t, n->left = t.make_node(), rs, hcube, idx0);
        CreateTree_rec(t, n->right = t.make_node(), rs, hcube, idx1);
    }
    else {
        n->data.t = conact::type::ACTION;
        n->data.action = node.uiAction;
    }
}

// Creates a tree from the optimized HyperCube rules
tree<conact> CreateTree(const rule_set& rs, const VHyperCube &hcube) {
    tree<conact> t;
    CreateTree_rec(t, t.make_root(), rs, hcube, string(hcube.m_iDim, '-'));
    return t;
}


// Checks if two subtrees 'n1' and 'n2' are equal or not 
template <typename T>
bool equal_trees(typename const tree<T>::node* n1, typename const tree<T>::node* n2) {
    if (n1->data != n2->data)
        return false;

    if (n1->isleaf())
        return true;
    else
        return equal_trees<T>(n1->left, n2->left) && equal_trees<T>(n1->right, n2->right);
}

template <typename T>
void find_and_link(typename tree<T>::node* n1, typename tree<T>::node* n2) {
    if (n2->isleaf() || n1 == n2)
        return;

    if (n1 != n2->left && equal_trees<T>(n1, n2->left))
        n2->left = n1;
    if (n1 != n2->right && equal_trees<T>(n1, n2->right))
        n2->right = n1;

    find_and_link<T>(n1, n2->left);
    find_and_link<T>(n1, n2->right);
}


// Recursive auxiliary function for the conversion of a tree into DAG
template <typename T>
void convert_tree_to_dag_rec(typename tree<T>::node *n, tree<T>& t) {
    find_and_link<T>(n, t.root);

    if (!n->isleaf()) {
        if (!n->left->isleaf())
            convert_tree_to_dag_rec<T>(n->left, t);
        if (!n->right->isleaf())
            convert_tree_to_dag_rec<T>(n->right, t);
    }
}

// Transform a tree 't' into a DAG
template <typename T>
void convert_tree_to_dag(tree<T>& t) {
    convert_tree_to_dag_rec(t.root, t);
}

// To load a tree from a txt file with the following structure:
//  x
//  	a (left son of x) 
//  		c 
//  			. 2
//  			. 3
//  		. 1,3,4 (list of actions)
//  	b (right son of x)
//  		c
//  			. 2
//  			. 3
//  		. 4
struct tree_loader {
    tree<conact> t;

    void load_tree(istream& is)
    {
        t.clear();
        t.root = load_tree_rec(is);
    }

    tree<conact>::node* load_tree_rec(istream& is)
    {
        string s;
        while (is >> s) {
            if (s[0] == '#')
                getline(is, s);
            else
                break;
        }

        tree<conact>::node* n = new tree<conact>::node;
        if (s == ".") {
            // leaf
            n->data.t = conact::type::ACTION;
            do {
                int action;
                is >> action;
                n->data.action |= 1 << (action - 1);
            } while (is.peek() == ',');
        }
        else {
            // real node with branches
            n->data.t = conact::type::CONDITION;
            n->data.condition = s;

            n->left = load_tree_rec(is);
            n->right = load_tree_rec(is);
        }

        return n;
    }
};

void print_node(tree<conact>::node *n, int i) 
{
    cout << string(i, '\t');
    if (n->data.t == conact::type::CONDITION) {
        cout << n->data.condition;
    }
    else {
        cout << ". ";
        auto v = n->data.actions();

        cout << v[0];
        for (size_t i = 1; i < v.size(); ++i) {
            cout << "," << v[i];
        }
    }
    cout << "\n";
}

int main()
{
    tree_loader tl;
    tl.load_tree(ifstream("../doc/t1.txt"));
    auto t1 = tl.t;
    t1.preorder(print_node);
    tl.load_tree(ifstream("../doc/t2.txt"));
    auto t2 = tl.t;
    t2.preorder(print_node);

    bool b1 = equal_trees<conact>(t1.root, t2.root);
    bool b2 = equal_trees<conact>(t1.root, t1.root);
    bool b3 = equal_trees<conact>(t2.root, t2.root);
    bool b4 = equal_trees<conact>(t2.root, t1.root);

    convert_tree_to_dag(t1);

    t1.preorder(print_node);

    return 0;

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


    auto t = CreateTree(rs, hcube);

    t.preorder(print_node);
}