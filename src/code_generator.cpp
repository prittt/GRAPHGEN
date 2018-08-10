// Copyright(c) 2018 Costantino Grana, Federico Bolelli 
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
//
// *Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and / or other materials provided with the distribution.
//
// * Neither the name of GRAPHSGEN nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "code_generator.h"

#include <map>

#include "utilities.h"

using namespace std;

// If leaves have multiple actions only the first one will be considered
void GenerateCodeRec(std::ostream& os, ltree::node *n, std::map<ltree::node*, int>& visited_nodes, std::map<ltree::node*, pair<int, bool>>& nodes_requiring_labels, nodeid &id, int tab = 1) {
    auto& m = visited_nodes;
    auto& ml = nodes_requiring_labels;
    if (n->isleaf()) {
        vector<uint> actions = n->data.actions();
        os << string(tab, '\t') << "ACTION_" << actions[0] << "\n";
    }
    else {
        if (ml[n].second) {
            os << string(tab, '\t') << "NODE_" << id.get() << ":\n";
        }
        string condition = n->data.condition;
        transform(condition.begin(), condition.end(), condition.begin(), ::toupper);
        os << string(tab, '\t') << "if (CONDITION_" << condition << ")";
        if (m.find(n->right) == end(m)) {
            // not found
            if (!n->right->isleaf()) {
                m[n->right] = id.next();
            }
            os << " {\n";
            GenerateCodeRec(os, n->right, m, ml, id, tab + 1);
            os << string(tab, '\t') << "}\n";
        }
        else {
            // code already exists
            os << "{\n" << string(tab + 1, '\t') << "goto NODE_" << m[n->right] << ";\n";
            os << string(tab, '\t') << "}\n";
        }

        os << string(tab, '\t') << "else";

        if (m.find(n->left) == end(m)) {
            // not found
            if (!n->left->isleaf()) {
                m[n->left] = id.next();
            }
            os << " {\n";
            GenerateCodeRec(os, n->left, m, ml, id, tab + 1);
            os << string(tab, '\t') << "}\n";
        }
        else {
            // code already exists
            os << "{\n" << string(tab + 1, '\t') << "goto NODE_" << m[n->left] << ";\n";
            os << string(tab, '\t') << "}\n";
        }
    }
}

// Check the nodes which will require labels for the following accesses (m)
void CheckNodesTraversalRec(ltree::node *n, std::map<ltree::node*, pair<int, bool>>& m, nodeid &id) {
    if (n->isleaf()) {
        //vector<uint> actions = n->data.actions();
        //os << string(tab, '\t') << "goto ACTION_" << actions[0] << ";\n";
        return;
    }
    else {
        //os << string(tab, '\t') << "NODE_" << id.get() << ":\n";
        //os << string(tab, '\t') << "if (CONDITION_" << n->data.condition << ")";
        if (m.find(n->right) == end(m)) {
            // not found
            if (!n->right->isleaf()) {
                m[n->right] = make_pair(id.next(), false);
            }
            //os << " {\n";
            CheckNodesTraversalRec(n->right, m, id);
            //os << string(tab, '\t') << "}\n";
        }
        else {
            // code already exists
            m[n->right].second = true;
            //os << "{\n" << string(tab + 1, '\t') << "goto NODE_" << m[n->right] << ";\n";
            //os << string(tab, '\t') << "}\n";
        }

        //os << string(tab, '\t') << "else";

        if (m.find(n->left) == end(m)) {
            // not found
            if (!n->left->isleaf()) {
                m[n->left] = make_pair(id.next(), false);
            }
            //os << " {\n";
            CheckNodesTraversalRec(n->left, m, id);
            //os << string(tab, '\t') << "}\n";
        }
        else {
            // code already exists
            m[n->left].second = true;
            //os << "{\n" << string(tab + 1, '\t') << "goto NODE_" << m[n->left] << ";\n";
            //os << string(tab, '\t') << "}\n";
        }
    }
}

// All nodes must have both sons! 
void GenerateCode(std::ostream& os, ltree& t) {
    std::map<ltree::node*, int> printed_node = { { t.root, 0 } };;

    std::map<ltree::node*, pair<int, bool>> nodes_requiring_labels;
    CheckNodesTraversalRec(t.root, nodes_requiring_labels, nodeid());

    GenerateCodeRec(os, t.root, printed_node, nodes_requiring_labels, nodeid(), 2);
}
