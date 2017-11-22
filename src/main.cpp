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
#include "code_generator.h"
#include "hypercube.h"
#include "output_generator.h"
#include "tree.h"
#include "utilities.h"

using namespace std;

#define LOG(message, instructions) cout << (message) << "... "; instructions cout << "done.\n"

using ltree = tree<conact>;

void CreateTree_rec(tree<conact>& t, tree<conact>::node *n, const rule_set& rs, const VHyperCube &hcube, const VIndex &idx) {
    VNode node = hcube[idx];
    if (node.uiAction == 0) {
        n->data.t = conact::type::CONDITION;
        n->data.condition = rs.conditions[node.uiMaxGainIndex];

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


// Checks if two subtrees 'n1' and 'n2' are equivalent or not 
bool equivalent_trees(const tree<conact>::node* n1, const tree<conact>::node* n2) {
    if (n1->data.neq(n2->data))
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

void FindAndLinkEquivalencesRec(tree<conact>::node* n1, tree<conact>::node* n2) {
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

    FindAndLinkEquivalencesRec(n1, n2->left);
    FindAndLinkEquivalencesRec(n1, n2->right);
}


// Recursive auxiliary function for the conversion of a tree into DAG
void Tree2DagUsingEquivalencesRec(tree<conact>::node *n, tree<conact>& t) {
    FindAndLinkEquivalencesRec(n, t.root);

    if (!n->isleaf()) {
        Tree2DagUsingEquivalencesRec(n->left, t);
        Tree2DagUsingEquivalencesRec(n->right, t);
    }
}

// Converts a tree into dag considering equivalences between subtrees
void Tree2DagUsingEquivalences(ltree& t) {
    Tree2DagUsingEquivalencesRec(t.root, t);
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

void FindAndLinkIdentiesRec(tree<conact>::node* n1, tree<conact>::node* n2) {
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

// Give a tree with multiple actions on leaves generate all possible subtree with only one action per leaf
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
            Tree2DagUsingEquivalences(trees[i]);
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

// "output_file": output file name without extension 
// "t": tree<conact> to draw
// "verbose": to print messages on the standard output
// return true if the process ends correctly, false otherwise
bool DrawDagOnFile(const string& output_file, ltree &t, bool verbose = false) {

    if (verbose) {
        std::cout << "Drawing DAG: " << output_file << ".. ";
    }
    string output_path_lowercase = output_file;
    std::transform(output_path_lowercase.begin(), output_path_lowercase.end(), output_path_lowercase.begin(), ::tolower);
    output_path_lowercase = global_output_path + output_path_lowercase;
    string code_path = output_path_lowercase + "_dotcode.txt";
    string pdf_path = output_path_lowercase + ".pdf";
    ofstream os(code_path);
    if (!os) {
        if (verbose) {
            std::cout << "Unable to generate " << code_path << ", stopped\n";
        }
        return false;
    }
    DrawDag(os, t);
    os.close();
    if (0 != system(string("..\\tools\\dot\\dot -Tpdf " + code_path + " -o " + pdf_path).c_str())) {
        if (verbose) {
            std::cout << "Unable to generate " + pdf_path + ", stopped\n";
        }
        return false;
        _unlink(code_path.c_str());
    }
    if (verbose) {
        std::cout << "done\n";
    }
    return true;
}

int main()
{
    //tree_loader tl;
    //tl.load_tree(ifstream("../doc/t1.txt"));
    //ltree t1 = tl.t;

    //{
    //    ofstream os("original_tree.txt");
    //    DrawDag(os, t1);
    //    os.close();
    //    system("..\\tools\\dot\\dot -Tpdf original_tree.txt -o original_tree.pdf");
    //}

    //Tree2OptimalDag(t1);
    //{
    //    ofstream os("optimal_dag.txt");
    //    DrawDag(os, t1);
    //    os.close();
    //    system("..\\tools\\dot\\dot -Tpdf optimal_dag.txt -o optimal_dag.pdf");
    //}

    //ofstream os("prova_codice.txt");
    //GenerateCode(os, t1);

    ////t1.preorder(print_node);
    //return 0;

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
                for (size_t i = 1; i < s.size(); ++i)
                    action += "+" + s[i];
            }
            r << action;
        }
    });

    /****************    BBDT    ****************/
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
        con.set("Q", "S", (r["i"] && r["n"]) || (con("P", "Q") && con("P", "S")));

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
                for (size_t i = 1; i < s.size(); ++i)
                    action += "+" + s[i];
            }
            r << action;
        }
    });

    auto& rs = labeling_bbdt;
    auto nvars = rs.conditions.size();
    auto nrules = rs.rules.size();

    LOG("Allocating hypercube",
        VHyperCube hcube(nvars);
    );

    ifstream is("hypercube.bin", ios::binary);
    if (!is) {
        LOG("Initializing rules",
            hcube.initialize_rules(rs);
        );

        LOG("Optimizing rules",
            hcube.optimize(false);
        );

        ofstream os("hypercube.bin", ios::binary);
        LOG("Writing to file",
            hcube.write(os);
        );
    }
    else {
        LOG("Reading from file",
            hcube.read(is);
        );
    }

    LOG("Creating tree",
        auto t = CreateTree(rs, hcube);
    );

    vector<ltree::node*> visited_nodes;
    CountDagNodes(t.root, visited_nodes);
    cout << "Nodes = " << visited_nodes.size() << "\n";

    DrawDagOnFile("bbdt_tree", t, true);

    LOG("Creating DRAG using identites",
        Tree2DagUsingIdentities(t);
    );

    DrawDagOnFile("bbdt_dag_identites", t, true);

    visited_nodes.clear();
    CountDagNodes(t.root, visited_nodes);
    cout << "Nodes = " << visited_nodes.size() << "\n";

    /****************    SAUF    ****************/
    /*
    auto& rs = labeling;
    auto nvars = rs.conditions.size();
    auto nrules = rs.rules.size();

    LOG("Allocating hypercube",
        VHyperCube hcube(nvars);
    );

    ifstream is("hypercube.bin", ios::binary);
    if (!is) {
        LOG("Initializing rules",
            hcube.initialize_rules(rs);
        );

        LOG("Optimizing rules",
            hcube.optimize(false);
        );

        ofstream os("hypercube.bin", ios::binary);
        LOG("Writing to file",
            hcube.write(os);
        );
    }
    else {
        LOG("Reading from file",
            hcube.read(is);
        );
    }

    LOG("Creating tree",
        auto t = CreateTree(rs, hcube);
    );

    vector<ltree::node*> visited_nodes;
    CountDagNodes(t.root, visited_nodes);
    cout << "Nodes = " << visited_nodes.size() << "\n";


    LOG("Saving tree",
    {
    ofstream os("sauf_tree.txt");
    DrawDag(os, t);
    os.close();
    system("..\\tools\\dot\\dot -Tpdf sauf_tree.txt -o sauf_tree.pdf");
    _unlink("sauf_tree.txt");
    }
    );

    LOG("Creating DRAG",
        convert_tree_to_dag(t);
    );
    LOG("Saving DRAG",
    {
    ofstream os("sauf_dag.txt");
    DrawDag(os, t);
    os.close();
    system("..\\tools\\dot\\dot -Tpdf sauf_dag.txt -o sauf_dag.pdf");
    _unlink("sauf_dag.txt");
    }
    );

    visited_nodes.clear();
    CountDagNodes(t.root, visited_nodes);
    cout << "Nodes = " << visited_nodes.size() << "\n";
    */

    LOG("Writing DRAG code",
    {
        ofstream os("bbdt_drag_code.txt");
        GenerateCode(os, t);
    }
    );
}