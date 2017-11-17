#ifndef TREESGENERATOR_DOT_GENERATOR_H
#define TREESGENERATOR_DOT_GENERATOR_H

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "condition_action.h"
#include "tree.h"

std::string output_path = "..\\output\\";

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

#endif // !TREESGENERATOR_DOT_GENERATOR_H