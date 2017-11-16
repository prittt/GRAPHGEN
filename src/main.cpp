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

#include "condition_action.h"
#include "hypercube.h"
#include "output_generator.h"
#include "tree.h"

using namespace std;

#define LOG(message, instructions) cout << (message) << "... "; instructions cout << "done.\n"

using ltree = tree<conact>;

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
bool equivalent_trees(const tree<conact>::node* n1, const tree<conact>::node* n2) {
    if (n1->data != n2->data)
        return false;

    if (n1->isleaf())
        return true;
    else
        return equivalent_trees(n1->left, n2->left) && equivalent_trees(n1->right, n2->right);
}

void intersect_leaves(tree<conact>::node* n1, tree<conact>::node* n2) {
    if (n1->isleaf()) {
        n2->data.action = n1->data.action &= n2->data.action;
    }
    else {
        intersect_leaves(n1->left, n2->left);
        intersect_leaves(n1->right, n2->right);
    }
}

void find_and_link(tree<conact>::node* n1, tree<conact>::node* n2) {
    if (n2->isleaf() || n1 == n2)
        return;

    if (n1 != n2->left && equivalent_trees(n1, n2->left)) {
        intersect_leaves(n1, n2->left);
        n2->left = n1;
    }

    if (n1 != n2->right && equivalent_trees(n1, n2->right)) {
        intersect_leaves(n1, n2->right);
        n2->right = n1;
    }

    find_and_link(n1, n2->left);
    find_and_link(n1, n2->right);
}


// Recursive auxiliary function for the conversion of a tree into DAG
void convert_tree_to_dag_rec(tree<conact>::node *n, tree<conact>& t) {
    find_and_link(n, t.root);

    if (!n->isleaf()) {
        convert_tree_to_dag_rec(n->left, t);
        convert_tree_to_dag_rec(n->right, t);
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
                is >> action >> ws;
                n->data.action |= 1 << (action - 1);
            } while (is.peek() == ',' && is.get());
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

void Tree2OptimalDagRec(ltree& t, ltree::node* n, vector<ltree>& trees) {
    ltree nt;
    if (n->isleaf()) { 
        // leaf with multiple action
        vector<uint> actions_list = n->data.actions();
        if (actions_list.size() > 1) {
            for (size_t i = 0; i < actions_list.size() - 1; ++i) {
                n->data.action = 1 << (actions_list[i] - 1);
                t.CopyTo(nt);
                Tree2OptimalDagRec(nt, nt.root, trees);
            }
            n->data.action = 1 << (actions_list[actions_list.size() - 1] - 1);
            Tree2OptimalDagRec(t, t.root, trees);
        }
        return;
    }


    Tree2OptimalDagRec(t, n->left, trees);
    Tree2OptimalDagRec(t, n->right, trees);

    if (t.root == n) {
        trees.push_back(t);
    }

    return;
}

// Converts a tree into dag minimizing the number of nodes (Note: this is "necessary" when the leaves of a tree contain multiple actions)
void Tree2OptimalDag(ltree& t) {
    vector<ltree> trees;
    Tree2OptimalDagRec(t, t.root, trees);

    for(size_t i = 0; i<trees.size(); ++i)
    {
        string s_txt = "tree_" + to_string(i) + ".txt";
        string s_pdf = "tree_" + to_string(i) + ".pdf";
        ofstream os(s_txt);
        DrawDag(os, trees[i]);
        os.close();
        string cmd = "..\\tools\\dot\\dot -Tpdf " + s_txt + " -o " + s_pdf;
        system(cmd.c_str());
    }

}

int main()
{
    tree_loader tl;
    tl.load_tree(ifstream("../doc/t1.txt"));
    ltree t1 = tl.t;

    Tree2OptimalDag(t1);


    {
        ofstream os("tree.txt");
        DrawDag(os, t1);
        os.close();
        system("..\\tools\\dot\\dot -Tpdf tree.txt -o tree.pdf");
    }

    t1.preorder(print_node);

    convert_tree_to_dag(t1);
    {
        ofstream os("dag.txt");
        DrawDag(os, t1);
        os.close();
        system("..\\tools\\dot\\dot -Tpdf dag.txt -o dag.pdf");
    }

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