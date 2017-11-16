#ifndef TREESGENERATOR_DOT_GENERATOR_H
#define TREESGENERATOR_DOT_GENERATOR_H

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "condition_action.h"
#include "tree.h"

void print_node(tree<conact>::node *n, int i)
{
    std::cout << std::string(i, '\t');
    if (n->data.t == conact::type::CONDITION) {
        std::cout << n->data.condition;
    }
    else {
        std::cout << ". ";
        auto v = n->data.actions();

        std::cout << v[0];
        for (size_t i = 1; i < v.size(); ++i) {
            std::cout << "," << v[i];
        }
    }
    std::cout << "\n";
}

struct nodeid {
    int _id = 0;
    int next() { return ++_id; }
    int get() { return _id; }
};

using namespace std;

void DrawDagRec(std::ostream& os, tree<conact>::node *n, std::map<tree<conact>::node*, int>& printed_node, std::vector<std::string>& links, nodeid &id, int tab=1) {
    os << std::string(tab, '\t') << "node" << id.get();
    if (n->isleaf()) {
        // print leaf
        os << " [label = \"";
        vector<uint> actions = n->data.actions();
        os << actions[0];
        for (size_t i = 1; i < actions.size(); ++i) {
            os << "," << actions[i];
        }
        os << "\", shape = box];\n";
    }
    else {
        os << " [label = \"" << n->data.condition << "\"];\n";
        tab++;

        if (printed_node.find(n->left) == printed_node.end()) {
            // node not already printed
            printed_node[n->left] = id.next();
            os << string(tab, '\t') << "node" << printed_node[n] << " -> node" << printed_node[n->left] << " [label=\"0\"];\n";
            DrawDagRec(os, n->left, printed_node, links, id, tab);
        }
        else {
            std::stringstream ss;
            ss << "node" << printed_node[n] << " -> node" << printed_node[n->left] << " [label=\"0\", style=dotted];\n";
            links.push_back(ss.str());
        }

        if (printed_node.find(n->right) == printed_node.end()) {
            // node not already printed
            printed_node[n->right] = id.next();
            os << string(tab, '\t') << "node" << printed_node[n] << " -> node" << printed_node[n->right] << " [label=\"1\"];\n";
            DrawDagRec(os, n->right, printed_node, links, id, tab);
        }
        else {
            std::stringstream ss;
            ss << "node" << printed_node[n] << " -> node" << printed_node[n->right] << " [label=\"1\", style=dotted];\n";
            links.push_back(ss.str());
        }
    }
}

// All nodes must have both sons! 
void DrawDag(std::ostream& os, tree<conact>& t) {
    os << "digraph dag{\n";
    os << "\tsubgraph tree{\n";
    
    std::map<tree<conact>::node*, int> printed_node = { { t.root, 0 } };;
    std::vector<std::string> links;
    DrawDagRec(os, t.root, printed_node, links, nodeid(), 2);
    
    os << "\t}\n";
    for (size_t i = 0; i < links.size(); ++i) {
        os << links[i];
    }
    
    os << "}\n";
}

//void print_forest_rec(ostream& os, int treeid, const node* n, map<const node*, int>& m, nodeid &id, int tab = 1) {
//    os << string(tab, '\t') << "tree" << treeid << "_node" << (m[n] & 0xffff);
//    if (n->l == nullptr) {
//        os << " [label = \"" << n->a << "," << next_tree[n->n] << "\", shape = box];\n";
//
//        stringstream ss;
//        ss << "tree" << treeid << "_node" << (m[n] & 0xffff) << " -> tree" << next_tree[n->n] << "_node" << (m[trees[next_tree[n->n]]] & 0xffff) << " [style=dotted];\n";
//        links.push_back(ss.str());
//    }
//    else {
//        os << " [label = \"" << n->c << "\"];\n";
//        tab++;
//
//        if (m.find(n->l) == m.end()) {
//            // not found
//            m[n->l] = (treeid << 16) | id.next();
//            os << string(tab, '\t') << "tree" << treeid << "_node" << (m[n] & 0xffff) << " -> tree" << (m[n->l] >> 16) << "_node" << (m[n->l] & 0xffff) << " [label=\"0\"];\n";
//            print_forest_rec(os, treeid, n->l, m, id, tab);
//        }
//        else {
//            os << string(tab, '\t') << "tree" << treeid << "_node" << (m[n] & 0xffff) << " -> tree" << (m[n->l] >> 16) << "_node" << (m[n->l] & 0xffff) << " [label=\"0\"];\n";
//        }
//
//        if (m.find(n->r) == m.end()) {
//            // not found
//            m[n->r] = (treeid << 16) | id.next();
//            os << string(tab, '\t') << "tree" << treeid << "_node" << (m[n] & 0xffff) << " -> tree" << (m[n->r] >> 16) << "_node" << (m[n->r] & 0xffff) << " [label=\"1\"];\n";
//            print_forest_rec(os, treeid, n->r, m, id, tab);
//        }
//        else {
//            os << string(tab, '\t') << "tree" << treeid << "_node" << (m[n] & 0xffff) << " -> tree" << (m[n->r] >> 16) << "_node" << (m[n->r] & 0xffff) << " [label=\"1\"];\n";
//        }
//    }
//}
//void print_forest(ostream& os) {
//    links.clear();
//    os << "digraph forest {\n";
//    os << "\tsubgraph cluster_start {\n";
//
//    nodeid id;
//
//    map<const node*, int> m;
//    m.insert({ start_tree, ((last_next + 1) << 16) | id.next() });
//    for (int i = 0; i <= last_next; ++i) {
//        if (next_tree[i] == i) {
//            m.insert({ trees[i], (i << 16) | id.next() });
//        }
//    }
//
//    print_forest_rec(os, last_next + 1, start_tree, m, id, 2);
//    os << "\t}\n";
//    for (int i = 0; i <= last_next; ++i) {
//        if (next_tree[i] == i) {
//            os << "\tsubgraph cluster" << i << " {\n";
//            print_forest_rec(os, i, trees[i], m, id, 2);
//            os << "\t}\n";
//        }
//    }
//    for (const auto& x : links)
//        os << "\t" << x;
//    os << "}\n";
//}

#endif // !TREESGENERATOR_DOT_GENERATOR_H