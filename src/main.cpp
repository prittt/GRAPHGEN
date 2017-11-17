#include <algorithm>
#include <set>
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
#include <cassert>

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
                nt = t;
                Tree2OptimalDagRec(nt, nt.root, trees);
            }
            n->data.action = 1 << (actions_list[actions_list.size() - 1] - 1);
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

void CountDagNodes(const ltree::node *n, vector<ltree::node*> &visited_nodes) {

    if (n->isleaf()) {
        return;
    }

    CountDagNodes(n->left, visited_nodes);
    CountDagNodes(n->right, visited_nodes);

    if (std::find(visited_nodes.begin(), visited_nodes.end(), n) == visited_nodes.end()) {
        visited_nodes.push_back(const_cast<ltree::node*>(n));
    }
}

// Returns the "best" (with minimal number of nodes) DAG of a list of DAGs
void BestDagFromList(const vector<ltree>& dags, ltree &t)
{
    uint best;
    {
        vector<ltree::node*> visited_node;
        CountDagNodes(dags[0].root, visited_node);
        best = visited_node.size();
        t = dags[0];
    }
    for (size_t i = 1; i < dags.size(); ++i) {
        vector<ltree::node*> visited_node;
        CountDagNodes(dags[i].root, visited_node);
        if (visited_node.size() < best) {
            best = visited_node.size();
            t = dags[i];
        }
    }
}

// Converts a tree into dag minimizing the number of nodes (Note: this is "necessary" when the leaves of a tree contain multiple actions)
void Tree2OptimalDag(ltree& t) {
    vector<ltree> trees;
    Tree2OptimalDagRec(t, t.root, trees);

    for (size_t i = 0; i < trees.size(); ++i) {
        {
            string s_txt = "tree_" + to_string(i) + ".txt";
            string s_pdf = "tree_" + to_string(i) + ".pdf";
            ofstream os(s_txt);
            DrawDag(os, trees[i]);
            os.close();
            string cmd = "..\\tools\\dot\\dot -Tpdf " + s_txt + " -o " + s_pdf;
            system(cmd.c_str());
        }

        {
            convert_tree_to_dag(trees[i]);
            string s_txt = "dag_" + to_string(i) + ".txt";
            string s_pdf = "dag_" + to_string(i) + ".pdf";
            ofstream os(s_txt);
            DrawDag(os, trees[i]);
            os.close();
            string cmd = "..\\tools\\dot\\dot -Tpdf " + s_txt + " -o " + s_pdf;
            system(cmd.c_str());
        }
    }

    BestDagFromList(trees, t);
}


struct rule_wrapper {
    rule_set& rs_;
    uint i_;
    rule_wrapper(rule_set& rs, uint i) : rs_{ rs }, i_{ i } {}

    bool operator[](const std::string& s) const {
        return rs_.get_condition(s, i_) != 0;
    }
    void operator<<(const std::string& s) {
        rs_.set_action(s, i_);
    }
    bool has_actions() {
        return rs_.rules[i_].actions != 0;
    }
};

template<uint N>
struct connectivity_mat {
    bool data_[N][N] = { 0 };
    map<string, uint> pos_;
    vector<string> names_;

    connectivity_mat(const vector<string> &names) : names_{ names } {
        assert(names.size() == N);
        for (uint i = 0; i < N; ++i) {
            data_[i][i] = 1;
            pos_[names[i]] = i;
        }
    }

    bool operator()(const string& row, const string& col) const {
        uint r = pos_.at(row);
        uint c = pos_.at(col);
        return data_[r][c];
    }

    //bool operator()(const string& row, const uint col) const {
    //    uint r = pos_.at(row);
    //    assert(col < N);
    //    return data_[r][col];
    //}

    void set(const string& row, const string& col, bool b) {
        uint r = pos_.at(row);
        uint c = pos_.at(col);
        data_[r][c] = data_[c][r] = b;
    }

    const string& GetHeader(uint i) {
        assert(i < N);
        return names_[i];
    }
};

template <uint N>
struct MergeSet {
    set<vector<string>> mergesets_;
    connectivity_mat<N> &con_;

    MergeSet(connectivity_mat<N> &con) : con_{ con } {}

    void ReduceMergeSet(vector<string>& ms) {
        for (size_t i = 0; i < ms.size(); ++i) {
            for (size_t j = i + 1; j < ms.size(); ) {
                if (con_(ms[i], ms[j])) {
                    // remove j-th element
                    ms.erase(begin(ms) + j);
                }
                else {
                    // move next
                    ++j;
                }
            }
        }
    }

    void ExpandAllEquivalences(vector<string> ms, size_t pos) {
        if (pos >= ms.size()) {
            sort(begin(ms), end(ms));
            mergesets_.emplace(ms);
        }
        else {
            string cur = ms[pos];
            for (size_t i = 0; i < N; ++i) {
                string h = con_.GetHeader(i);
                if (h != "x" && con_(cur, h)) {
                    ms[pos] = h;
                    ExpandAllEquivalences(ms, pos + 1);
                }
            }
        }
    }

    void BuildMergeSet() {
        vector<string> ms;
        // Create initial merge set
        for (size_t i = 0; i < N; ++i) {
            string h = con_.GetHeader(i);
            if (h != "x" && con_("x", h)) {
                ms.push_back(h);
            }
        }
        ReduceMergeSet(ms);
        ExpandAllEquivalences(ms, 0);
    }

};



int main()
{
/*  tree_loader tl;
    tl.load_tree(ifstream("../doc/t1.txt"));
    ltree t1 = tl.t;

    Tree2OptimalDag(t1);
    {
        ofstream os("optimal_dag.txt");
        DrawDag(os, t1);
        os.close();
        system("..\\tools\\dot\\dot -Tpdf optimal_dag.txt -o optimal_dag.pdf");
    }

    t1.preorder(print_node);
    return 0;
    */
    pixel_set rosenfeld_mask{
        { "p", -1, -1 }, { "q", 0, -1 }, { "r", +1, -1 },
        { "s", -1,  0 }, { "x", 0, 0 },
    };

    rule_set labeling;
    labeling.init_conditions(rosenfeld_mask);
    labeling.init_actions({ 
        "nothing", 
        "x<-newlabel", 
        "x<-p", "x<-q", "x<-r", "x<-s", 
        "x<-p+q", "x<-p+r", "x<-p+s", "x<-q+r", "x<-q+s", "x<-r+s",
        "x<-p+q+r", "x<-p+q+s", "x<-p+r+s", "x<-q+r+s", 
        "x<-p+q+r+s", 
    });

    /*labeling.generate_rules([](rule_set& rs, uint i) {
        rule_wrapper r(rs, i);

        bool X = r["x"];

        if (!X) {
            r << "nothing";
            return;
        }

        if (r["p"] && !r["q"] && r["r"])
            r << "x<-p+r";
        if (r["s"] && !r["q"] && r["r"])
            r << "x<-s+r";
        if (r.has_actions())
            return;

        if (r["p"])
            r << "x<-p";
        if (r["q"])
            r << "x<-q";
        if (r["r"])
            r << "x<-r";
        if (r["s"])
            r << "x<-s";
        if (r.has_actions())
            return;

        r << "x<-newlabel";
    });*/

    labeling.generate_rules([](rule_set& rs, uint i) {
        rule_wrapper r(rs, i);

        bool X = r["x"];
        if (!X) {
            r << "nothing";
            return;
        }

        connectivity_mat<5> con({ "p", "q", "r", "s", "x" });

        con.set("x", "p", r["p"]);
        con.set("x", "q", r["q"]);
        con.set("x", "r", r["r"]);
        con.set("x", "s", r["s"]);

        con.set("p", "q", r["p"] && r["q"]);
        con.set("p", "s", r["p"] && r["s"]);
        con.set("q", "r", r["q"] && r["r"]);
        con.set("q", "s", r["q"] && r["s"]);

        con.set("p", "r", con("p", "q") && con("q", "r"));
        con.set("s", "r", (con("p", "r") && con("p", "s")) || (con("s", "q") && con("q", "r")));

        MergeSet<5> ms(con);
        ms.BuildMergeSet();

        for (const auto& s : ms.mergesets_) {
            string action = "x<-";
            if (s.empty())
                action += "newlabel";
            else {
                action += s[0];
                for (size_t i=1;i<s.size();++i)
                    action += "+" + s[i];
            }
            r << action;
        }
    });

    pixel_set grana_mask{
        /*{ "a", -2, -2 },*/ { "b", -1, -2 }, { "c", +0, -2 }, { "d", +1, -2 }, { "e", +2, -2 }, /*{ "f", +3, -2 },*/
          { "g", -2, -1 },   { "h", -1, -1 }, { "i", +0, -1 }, { "j", +1, -1 }, { "k", +2, -1 }, /*{ "l", +3, -1 },*/
          { "m", -2, +0 },   { "n", -1, +0 }, { "o", +0, +0 }, { "p", +1, +0 },
        /*{ "q", -2, +1 },*/ { "r", -1, +1 }, { "s", +0, +1 }, { "t", +1, +1 },
    };

    rule_set labeling_bbdt;
    labeling_bbdt.init_conditions(grana_mask);
    labeling_bbdt.init_actions({ "nothing", "x<-newlabel",
                                 "x<-P", "x<-Q", "x<-R", "x<-S",
                                 "x<-P+Q", "x<-P+R", "x<-P+S", "x<-Q+R", "x<-Q+S", "x<-R+S",
                                 "x<-P+Q+R", "x<-P+Q+S", "x<-P+R+S", "x<-Q+R+S", });

    labeling_bbdt.generate_rules([](rule_set& rs, uint i) {
        rule_wrapper r(rs, i);

        bool X = r["o"] || r["p"] || r["s"] || r["t"];
        if (!X) {
            r << "nothing";
            return;
        }

        connectivity_mat<5> con({ "P", "Q", "R", "S", "x" });

        con.set("x", "P", r["h"] && r["o"]);
        con.set("x", "Q", (r["i"] || r["j"]) && (r["o"] || r["p"]));
        con.set("x", "R", r["k"] && r["p"]);
        con.set("x", "S", (r["n"] || r["r"]) && (r["o"] || r["s"]));

        con.set("P", "Q", (r["b"] || r["h"]) && (r["c"] || r["i"]));
        con.set("P", "S", (r["g"] || r["h"]) && (r["m"] || r["n"]));
        con.set("Q", "R", (r["d"] || r["j"]) && (r["e"] || r["k"]));
        con.set("Q", "S", r["i"] && r["n"]);

        con.set("P", "R", con("P", "Q") && con("Q", "R"));
        con.set("S", "R", (con("P", "R") && con("P", "S")) || (con("S", "Q") && con("Q", "R")));

        MergeSet<5> ms(con);
        ms.BuildMergeSet();

        for (const auto& s : ms.mergesets_) {
            string action = "x<-";
            if (s.empty())
                action += "newlabel";
            else {
                action += s[0];
                for (size_t i = 1; i<s.size(); ++i)
                    action += "+" + s[i];
            }
            r << action;
        }
    });

    labeling_bbdt.print_rules(cout);


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