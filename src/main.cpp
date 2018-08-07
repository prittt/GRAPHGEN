#include <algorithm>
#include <bitset>
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

#include <opencv2\core.hpp>
#include <opencv2\imgcodecs.hpp>
#include <opencv2\imgproc.hpp>

#include "condition_action.h"
#include "code_generator.h"
#include "hypercube.h"
#include "output_generator.h"
#include "tree.h"
#include "utilities.h"

#include "merge_set.h"

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

void FindAndLinkIdentiesDagRec(tree<conact>::node* n1, tree<conact>::node* n2, std::map<ltree::node*, bool> &visited_fl) {
    if (n2->isleaf() || n1 == n2 || visited_fl[n2])
        return;
    visited_fl[n2] = true;

    if (n1 != n2->left && EqualTrees(n1, n2->left)) {
        n2->left = n1;
    }

    if (n1 != n2->right && EqualTrees(n1, n2->right)) {
        n2->right = n1;
    }

    FindAndLinkIdentiesDagRec(n1, n2->left, visited_fl);
    FindAndLinkIdentiesDagRec(n1, n2->right, visited_fl);
}

// Recursive auxiliary function for the conversion of a DAG into DAG with no equivalent subgraphs
void Dag2DagUsingIdentiesRec(tree<conact>::node *n, tree<conact>& t, std::map<ltree::node*, bool> &visited_n) {
    std::map<ltree::node*, bool> visited_fl;
    FindAndLinkIdentiesDagRec(n, t.root, visited_fl);
    visited_n[n] = true;

    if (!n->isleaf()) {
        if (!visited_n[n->left])
            Dag2DagUsingIdentiesRec(n->left, t, visited_n);
        if (!visited_n[n->right])
            Dag2DagUsingIdentiesRec(n->right, t, visited_n);
    }
}

// Converts dag to dag using equivalences between subtrees
void Dag2DagUsingIdenties(ltree& t) {
    std::map<ltree::node*, bool> visited_n;
    Dag2DagUsingIdentiesRec(t.root, t, visited_n);
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

//// Given a tree with multiple actions on leaves this function generate all possible subtree with only one action per leaf
//void Tree2OptimalDagRec(ltree& t, ltree::node* n, vector<ltree>& trees) {
//    ltree nt;
//    if (n->isleaf()) {
//        // leaf with multiple action
//        vector<uint> actions_list = n->data.actions();
//        if (actions_list.size() > 1) {
//            for (size_t i = 0; i < actions_list.size() - 1; ++i) {
//                n->data.action = 1 << (actions_list[i] - 1);
//                nt = t;
//                Tree2OptimalDagRec(nt, nt.root, trees);
//            }
//            n->data.action = 1 << (actions_list[actions_list.size() - 1] - 1);
//        }
//        return;
//    }
//
//    Tree2OptimalDagRec(t, n->left, trees);
//    Tree2OptimalDagRec(t, n->right, trees);
//
//    if (t.root == n) {
//        trees.push_back(t);
//    }
//
//    return;
//}

//// Given a dag with multiple actions on leaves this function generate all possible dags with only one action per leaf
//void Tree2OptimalDagRec(ltree& t, ltree::node* n, vector<ltree>& trees, std::map<const ltree::node*, bool> &visited_n, uint &counter) {
//    ltree nt;
//    if (n->isleaf()) {
//        // leaf with multiple action
//        vector<uint> actions_list = n->data.actions();
//        if (actions_list.size() > 1) {
//            for (size_t i = 0; i < actions_list.size() - 1; ++i) {
//                std::map<const ltree::node*, bool> visited_node_cur;
//                n->data.action = 1 << (actions_list[i] - 1);
//                nt = t;
//                Tree2OptimalDagRec(nt, nt.root, trees, visited_node_cur, counter);
//            }
//            n->data.action = 1 << (actions_list[actions_list.size() - 1] - 1);
//        }
//        return;
//    }
//
//    if (!visited_n[n->left]) {
//        // left node not already visited
//        visited_n[n->left] = true;
//        Tree2OptimalDagRec(t, n->left, trees, visited_n, counter);
//    }
//
//    if (!visited_n[n->right]) {
//        // right node not already visited
//        visited_n[n->right] = true;
//        Tree2OptimalDagRec(t, n->right, trees, visited_n, counter);
//    }
//
//    if (t.root == n) {
//        trees.push_back(t);
//    }
//}



// This function counts the number of nodes in a dag
void CountDagNodes(const ltree::node *n, set<const ltree::node*> &visited_nodes, set<const ltree::node*> &visited_leaves)
{
    if (n->isleaf()) {
        visited_leaves.insert(n);
        return;
    }

    if (visited_nodes.insert(n).second) {
        CountDagNodes(n->left, visited_nodes, visited_leaves);
        CountDagNodes(n->right, visited_nodes, visited_leaves);
    }
}

// Specific for bbdt (TODO: do it better)
struct TreePathFreq{
    vector<unsigned long long> paths_freq;

    void Generate(const ltree& t, const rule_set& rs) {
        paths_freq = vector<unsigned long long>(65536, 0);
        vector<int> cur_path(16,-1);
        GenerateRec(t.root, 0, cur_path, 0, rs);
    }

    void GenerateRec(const ltree::node *n, uint height, vector<int> cur_rule, uint cur_path, const rule_set& rs) {
        if (n->isleaf()) {
            unsigned long long freq = 0; 
            for (uint i = 0; i < rs.rules.size(); ++i) {
                bool stop = false;
                for (uint j = 0; j < cur_rule.size() && !stop; ++j) {
                    if (cur_rule[j] > -1) {
                        uint rule_bit = (i >> j) & 1;
                        if (cur_rule[j] != rule_bit) {
                            stop = true;
                        }
                    }
                }
                if (!stop) {
                    if(freq > freq + (height * rs.rules[i].frequency)){
                        cout << "OVERFLOW ..";
                    }
                    freq += height * rs.rules[i].frequency;
                }
            }

            paths_freq[cur_path] = freq;
            return;
        }
        else {
            cur_path <<= 1;
            cur_rule[rs.conditions_pos.at(n->data.condition)] = 0;
            GenerateRec(n->left, height + 1, cur_rule, cur_path, rs);
            cur_path++;
            cur_rule[rs.conditions_pos.at(n->data.condition)] = 1;
            GenerateRec(n->right, height + 1, cur_rule, cur_path, rs);
        }
    }

};

// Given a dag with multiple actions on leaves this function generate all possible dags with only one action per leaf
// VERSIONE CHE CONTA I NODI E LE FOGLIE PER DECIDERE QUAL E' L'ALBERO MIGLIORE
void Tree2OptimalDagRec(ltree& t, ltree::node* n, ltree &best_tree, uint &best_nodes, uint &best_leaves, std::map<const ltree::node*, bool> &visited_n, uint &counter) {
    ltree nt;
    if (n->isleaf()) {
        // leaf with multiple action
        vector<uint> actions_list = n->data.actions();
        if (actions_list.size() > 1) {
            for (size_t i = 0; i < actions_list.size() - 1; ++i) {
                std::map<const ltree::node*, bool> visited_node_cur;
                n->data.action = 1 << (actions_list[i] - 1);
                nt = t;
                Tree2OptimalDagRec(nt, nt.root, best_tree, best_nodes, best_leaves, visited_node_cur, counter);
            }
            n->data.action = 1 << (actions_list[actions_list.size() - 1] - 1);
        }
        return;
    }

    if (!visited_n[n->left]) {
        // left node not already visited
        visited_n[n->left] = true;
        Tree2OptimalDagRec(t, n->left, best_tree, best_nodes, best_leaves, visited_n, counter);
    }

    if (!visited_n[n->right]) {
        // right node not already visited
        visited_n[n->right] = true;
        Tree2OptimalDagRec(t, n->right, best_tree, best_nodes, best_leaves, visited_n, counter);
    }

    if (t.root == n) {
        counter++;
        ltree dag = t;
        Dag2DagUsingIdenties(dag);
        set<const ltree::node*> visited_nodes, visited_leaves;
        CountDagNodes(dag.root, visited_nodes, visited_leaves);

        if (best_nodes > visited_nodes.size()) {
            best_nodes = visited_nodes.size();
            best_leaves = visited_leaves.size();
            best_tree = dag;
        }
        else if (best_nodes == visited_nodes.size() && best_leaves > visited_leaves.size()) {
            best_leaves = visited_leaves.size();
            best_tree = dag;
        }

        if (counter % 1000 == 0) {
            cout << counter / 1000 << "\r";
        }
    }
}

// Returns the "best" (with minimal number of nodes) DAG of a list of DAGs
void BestDagFromList(const vector<ltree>& dags, ltree &t)
{
    uint best_nodes = numeric_limits<uint>::max();
    uint best_leaves = numeric_limits<uint>::max();
    for (size_t i = 0; i < dags.size(); ++i) {
        set<const ltree::node*> visited_node, visited_leaves;
        CountDagNodes(dags[i].root, visited_node, visited_leaves);
        if (best_nodes > visited_node.size()) {
            best_nodes = visited_node.size();
            best_leaves = visited_leaves.size();
            t = dags[i];
        }
        else if (best_nodes == visited_node.size() && best_leaves > visited_leaves.size()) {
            best_leaves = visited_leaves.size();
            t = dags[i];
        }
    }
}

//// Converts a tree into dag minimizing the number of nodes (Note: this is "necessary" when the leaves of a tree contain multiple actions)
//void Tree2OptimalDag(ltree& t) {
//    vector<ltree> trees;
//    std::map<ltree::node*, bool> visited_nodes;
//    uint counter = 0; 
//    Tree2OptimalDagRec(t, t.root, trees, visited_nodes, counter);
//
//    for (size_t i = 0; i < trees.size(); ++i) {
//        DrawDagOnFile("tree_" + to_string(i), trees[i]);
//        Dag2DagUsingIdenties(trees[i]);
//        DrawDagOnFile("dag_" + to_string(i), trees[i]);
//    }
//
//    BestDagFromList(trees, t);
//}

// Converts a tree into dag minimizing the number of nodes (Note: this is "necessary" when the leaves of a tree contain multiple actions)
// UTILIZZA IL NUMERO DI NODI PER SCEGLIERE IL DAG OTTIMO
void Tree2OptimalDag(ltree& t) {
    vector<ltree> trees;
    ltree best_tree;
    std::map<const ltree::node*, bool> visited_nodes;
    uint counter = 0;
    uint best_nodes = numeric_limits<uint>::max();
    uint best_leaves = numeric_limits<uint>::max();
    //Tree2OptimalDagRec(t, t.root, best_tree, best_nodes, best_leaves, visited_nodes, counter);
    Tree2OptimalDagRec(t, t.root, best_tree, best_nodes, best_leaves, visited_nodes, counter);
    cout << "** Vector size:" << counter << " **\n";
    cout << "** Counter:" << counter << " **\n";

    t = best_tree;

    /*for (size_t i = 0; i < trees.size(); ++i) {
    DrawDagOnFile("tree_" + to_string(i), trees[i]);
    Dag2DagUsingIdenties(trees[i]);
    DrawDagOnFile("dag_" + to_string(i), trees[i]);
    }

    BestDagFromList(trees, t);*/
}

struct mask {
    cv::Mat1b mask_;
    int top_ = 0, bottom_ = 0, left_ = 0, right_ = 0;
    int border_ = 0;
    int exp_;
    int increment_ = 0;

    mask(const pixel_set &ps) {
        increment_ = ps.shift_;
        exp_ = ps.pixels.size();
        for (int i = 0; i < exp_; ++i) {
            border_ = std::max(border_, std::max(std::abs(ps.pixels[i].dx), std::abs(ps.pixels[i].dy)));
            top_ = std::min(top_, ps.pixels[i].dy);
            right_ = std::max(right_, ps.pixels[i].dx);
            left_ = std::min(left_, ps.pixels[i].dx);
            bottom_ = std::max(bottom_, ps.pixels[i].dy);
        }

        left_ = std::abs(left_);
        top_ = std::abs(top_);

        mask_ = cv::Mat1b(top_ + bottom_ + 1, left_ + right_ + 1, uchar(0));
        for (int i = 0; i < exp_; ++i) {
            mask_(ps.pixels[i].dy + top_, ps.pixels[i].dx + left_) = 1;
        }
    }

    size_t MaskToLinearMask(const cv::Mat1b r_img) const {
        size_t linearMask = 0;

        for (int r = 0; r < r_img.rows; ++r) {
            for (int c = 0; c < r_img.cols; ++c) {
                linearMask <<= (1 & mask_(r, c));
                linearMask |= (r_img(r, c) & mask_(r, c));
            }
        }
        return linearMask;
    }
};

// This function extracts all the configurations of a given mask (msk) in a give image (img) and store the occourences (frequencies) in the rRules vector
void CalculateConfigurationsFrequencyOnImage(const cv::Mat1b& img, const mask &msk, rule_set &rs) {

    cv::Mat1b clone;
    copyMakeBorder(img, clone, msk.border_, msk.border_, msk.border_, msk.border_, cv::BORDER_CONSTANT, 0);
    const int h = clone.rows, w = clone.cols;

    for (int r = msk.border_; r < h - msk.border_; r += msk.increment_) {
        for (int c = msk.border_; c < w - msk.border_; c += msk.increment_) {
            cv::Mat1b read_pixels = clone(cv::Rect(cv::Point(c - msk.left_, r - msk.top_), cv::Point(c + 1 + msk.right_, r + 1 + msk.bottom_))).clone();
            bitwise_and(msk.mask_, read_pixels, read_pixels);
            size_t rule = msk.MaskToLinearMask(read_pixels);
            rs.rules[rule].frequency++;
            if (rs.rules[rule].frequency == numeric_limits</*uint*/ unsigned long long>::max()) {
                cout << "OVERFLOW freq" << endl;
            }
        }
    }
}

bool GetBinaryImage(const string &FileName, cv::Mat1b& binary) {

    // Image load
    cv::Mat image;
    image = cv::imread(FileName, cv::IMREAD_GRAYSCALE);   // Read the file

                                                         // Check if image exist
    if (image.empty())
        return false;

    //// Convert the image to grayscale
    //Mat grayscaleMat;
    //cvtColor(image, grayscaleMat, CV_RGB2GRAY);

    // Adjust the threshold to actually make it binary
    cv::threshold(image, binary, 100, 1, cv::THRESH_BINARY);

    return true;
}


void RemoveCharacter(string& s, const char c)
{
    s.erase(std::remove(s.begin(), s.end(), c), s.end());
}

bool LoadFileList(vector<pair<string, bool>>& filenames, const string& files_path)
{
    // Open files_path (files.txt)
    ifstream is(files_path);
    if (!is.is_open()) {
        return false;
    }

    string cur_filename;
    while (getline(is, cur_filename)) {
        // To delete possible carriage return in the file name
        // (especially designed for windows file newline format)
        RemoveCharacter(cur_filename, '\r');
        filenames.push_back(make_pair(cur_filename, true));
    }

    is.close();
    return true;
}

void CalculateRulesFrequencies(const pixel_set &ps, const vector<string> &paths, rule_set &rs) {
    mask msk(ps);

    cv::Mat1b img;

    for (uint i = 0; i < paths.size(); ++i) {
        vector<pair<string, bool>> files_list;
        if (!LoadFileList(files_list, paths[i] + "//files.txt")) {
            cout << "Unable to find 'files.txt' of " << paths[i] << ", dataset skipped";
            continue;
        }

        for (uint d = 0; d < files_list.size(); ++d) {
            GetBinaryImage(paths[i] + "//" + files_list[d].first, img);
            if (img.empty()) {
                cout << "Unable to find '" << files_list[d].first << "' image in '" << paths[i] << "' dataset, image skipped\n";
                continue;
            }
            CalculateConfigurationsFrequencyOnImage(img, msk, rs);
        }
    }
}

void StoreFrequenciesOnFile(const string &output_file, const rule_set& rs) {
    ofstream os(output_file);
    if (!os.is_open()) {
        return;
    }

    for (size_t i = 0; i < rs.rules.size(); ++i) {
        os << rs.rules[i].frequency << endl;
    }
}

bool LoadFrequenciesFromFile(const string &file_path, rule_set& rs) {
    ifstream is(file_path);
    if (!is.is_open()) {
        return false;
    }

    int i = 0;
    unsigned long long freq;
    while (is >> freq) {
        rs.rules[i].frequency = freq;
        ++i;
    }

    return true;
}

// This function counts the number of nodes in a dag
void CalculateDagWeight(const ltree::node *n, unsigned long long& weight, set<const ltree::node*> &visited_nodes, const TreePathFreq& tpf, uint cur_path = 0)
{
    if (n->isleaf()) {
        //if (weight > weight + tpf.paths_freq[cur_path]) {
        //    cout << "OVERFLOW during tree weight calculation" << endl;
        //}
        weight += tpf.paths_freq[cur_path];
        return;
    }

    if (visited_nodes.insert(n).second) {
        cur_path <<= 1;
        CalculateDagWeight(n->left, weight, visited_nodes, tpf);
        cur_path++;
        CalculateDagWeight(n->right, weight, visited_nodes, tpf);
    }
}


// Given a dag with multiple actions on leaves this function generate all possible dags with only one action per leaf
// VERSIONE CHE USA LE FREQUENZE PER DECIDERE QUAL E' L'ALBERO MIGLIORE (
void Tree2OptimalDagFreqRec(ltree& t, ltree::node* n, ltree &best_tree, uint &best_nodes, uint &best_leaves, unsigned long long best_weight, std::map<const ltree::node*, bool> &visited_n, uint &counter, const TreePathFreq& tpf) {
    ltree nt;
    if (n->isleaf()) {
        // leaf with multiple action
        vector<uint> actions_list = n->data.actions();
        if (actions_list.size() > 1) {
            for (size_t i = 0; i < actions_list.size() - 1; ++i) {
                std::map<const ltree::node*, bool> visited_node_cur;
                n->data.action = 1 << (actions_list[i] - 1);
                nt = t;
                Tree2OptimalDagFreqRec(nt, nt.root, best_tree, best_nodes, best_leaves, best_weight, visited_node_cur, counter, tpf);
            }
            n->data.action = 1 << (actions_list[actions_list.size() - 1] - 1);
        }
        return;
    }

    if (!visited_n[n->left]) {
        // left node not already visited
        visited_n[n->left] = true;
        Tree2OptimalDagFreqRec(t, n->left, best_tree, best_nodes, best_leaves, best_weight, visited_n, counter, tpf);
    }

    if (!visited_n[n->right]) {
        // right node not already visited
        visited_n[n->right] = true;
        Tree2OptimalDagFreqRec(t, n->right, best_tree, best_nodes, best_leaves, best_weight, visited_n, counter, tpf);
    }

    if (t.root == n) {
        counter++;
        ltree dag = t;
        uint height = 0;
        Dag2DagUsingIdenties(dag);
        unsigned long long weight = 0; 
        {
            set<const ltree::node*> visited_nodes;
            CalculateDagWeight(dag.root, weight, visited_nodes, tpf);
        }
        set<const ltree::node*> visited_nodes, visited_leaves;
        CountDagNodes(dag.root, visited_nodes, visited_leaves);

        if (weight < best_weight) {
            best_weight = weight;
            best_nodes = visited_nodes.size();
            best_leaves = visited_leaves.size();
            best_tree = dag;
        }
        else if (weight == best_weight && visited_nodes.size() < best_nodes) {
            best_nodes = visited_nodes.size();
            best_leaves = visited_leaves.size();
            best_tree = dag;
        }
        else if (weight == best_weight && visited_nodes.size() == best_nodes && visited_leaves.size() < best_leaves) {
            best_leaves = visited_leaves.size();
            best_tree = dag;
        }

        if (counter % 1000 == 0) {
            cout << counter / 1000 << "\r";
        }
    }
}

// Converts a tree into dag minimizing the number of nodes (Note: this is "necessary" when the leaves of a tree contain multiple actions)
// UTILIZZA LE FREQUENZE PER SCEGLIERE IL DAG OTTIMO
void Tree2OptimalDagFreq(ltree& t, const TreePathFreq& tpf) {
    vector<ltree> trees;
    ltree best_tree;
    std::map<const ltree::node*, bool> visited_nodes;
    uint counter = 0;
    uint best_nodes = numeric_limits<uint>::max();
    uint best_leaves = numeric_limits<uint>::max();
    unsigned long long best_weight = numeric_limits<unsigned long long>::max();

    Tree2OptimalDagFreqRec(t, t.root, best_tree, best_nodes, best_leaves, best_weight, visited_nodes, counter, tpf);
    cout << "** Vector size:" << counter << " **\n";
    cout << "** Counter:" << counter << " **\n";

    t = best_tree;

    /*for (size_t i = 0; i < trees.size(); ++i) {
    DrawDagOnFile("tree_" + to_string(i), trees[i]);
    Dag2DagUsingIdenties(trees[i]);
    DrawDagOnFile("dag_" + to_string(i), trees[i]);
    }

    BestDagFromList(trees, t);*/
}

int main()
{
    //cv::Mat1b test(4, 4);
    //test << 255, 0, 255, 0,
    //        0, 255, 0, 255,
    //        255, 0, 255, 0,
    //        0, 255, 0, 255;

    //imwrite("test.png", test);
    //return 0;

    //tree_loader tl;
    //tl.load_tree(ifstream("../doc/t1.txt"));
    //ltree t1 = tl.t;

    //DrawDagOnFile("original_tree", t1, true);

    //Tree2DagUsingIdentities(t1);
    //DrawDagOnFile("dag_with_eq", t1, true);

    //Tree2OptimalDag(t1);
    //DrawDagOnFile("optimal_dag", t1, true);

    //
    ///*ofstream os("prova_codice.txt");
    //GenerateCode(os, t1);*/

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

		con.DisplayCondNames();
		std::cout << std::bitset<5>(i) << std::endl;
		con.DisplayMap();
		cout << endl;

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
    grana_mask.SetShift(2);

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
    vector<string> dataset_paths = { "." };
    //string base_path = "C://Users//Federico Bolelli//Desktop//YACCLAB//bin//input//";
    //vector<string> dataset_paths = { base_path + "3dpes", base_path + "fingerprints", base_path + "hamlet", base_path + "medical", base_path + "mirflickr", base_path + "random//classical", base_path + "tobacco800", base_path + "xdocs"};

    // Set to true to use freq file, otherwise all freq will be considered equal to 1
    if (false) {
        string freq_file = "freqs.txt";
        if (!LoadFrequenciesFromFile(freq_file, labeling_bbdt)) {
            cout << "Calculate frequencies..." << endl;
            CalculateRulesFrequencies(grana_mask, dataset_paths, labeling_bbdt);
            cout << "Calculate frequencies...DONE" << endl;
            StoreFrequenciesOnFile(freq_file, labeling_bbdt);
        }
    }

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

    std::string s(hcube.m_iDim, '-');
    cout << hcube[VIndex{ s }].neq << '\n';
    return 0;

    LOG("Creating tree",
        auto t = CreateTree(rs, hcube);
    );

    set<const ltree::node*> visited_nodes, visited_leaves;
    CountDagNodes(t.root, visited_nodes, visited_leaves);
    cout << "Nodes = " << visited_nodes.size() << "\n";
    cout << "Leaves = " << visited_leaves.size() << "\n";

    DrawDagOnFile("bbdt_tree", t, true);

    TreePathFreq tpf;
    tpf.Generate(t,labeling_bbdt);

    ofstream os("verifica.txt");
    for (uint i = 0; i < tpf.paths_freq.size(); ++i) {
        bitset<16> bs(i);
        os << i << ", " << bs << ", " << tpf.paths_freq[i] << "\n";
    }
    os.close();

    LOG("Creating DRAG using identites",
        Tree2DagUsingIdentities(t);
    );

    DrawDagOnFile("bbdt_dag_identites", t, true);

    visited_nodes.clear();
    visited_leaves.clear();
    CountDagNodes(t.root, visited_nodes, visited_leaves);
    cout << "Nodes = " << visited_nodes.size() << "\n";
    cout << "Leaves = " << visited_leaves.size() << "\n";

    //Tree2OptimalDag(t);
    Tree2OptimalDagFreq(t, tpf);
    DrawDagOnFile("optimal_dag", t, true);

    visited_nodes.clear();
    visited_leaves.clear();
    CountDagNodes(t.root, visited_nodes, visited_leaves);
    cout << "Nodes = " << visited_nodes.size() << "\n";
    cout << "Leaves = " << visited_leaves.size() << "\n";

    LOG("Writing DRAG code",
    {
        ofstream os("bbdt_drag_code.txt");
        GenerateCode(os, t);
    }
    );

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
    GenerateDotCodeForDag(os, t);
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
    GenerateDotCodeForDag(os, t);
    os.close();
    system("..\\tools\\dot\\dot -Tpdf sauf_dag.txt -o sauf_dag.pdf");
    _unlink("sauf_dag.txt");
    }
    );

    visited_nodes.clear();
    CountDagNodes(t.root, visited_nodes);
    cout << "Nodes = " << visited_nodes.size() << "\n";
    */
}