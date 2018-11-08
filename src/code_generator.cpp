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
void GenerateCodeRec(std::ostream& os, ltree::node *n, std::map<ltree::node*, int>& visited_nodes, std::map<ltree::node*, pair<int, bool>>& nodes_requiring_labels, nodeid &id, int tab, bool add_gotos) {
    auto& m = visited_nodes;
    auto& ml = nodes_requiring_labels;
    if (n->isleaf()) {
        vector<uint> actions = n->data.actions();
        os << string(tab, '\t') << "ACTION_" << actions[0] << "\n";
        if (add_gotos)
            os << string(tab, '\t') << "goto tree_" << n->data.next << ";\n";
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
            GenerateCodeRec(os, n->right, m, ml, id, tab + 1, add_gotos);
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
            GenerateCodeRec(os, n->left, m, ml, id, tab + 1, add_gotos);
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

    GenerateCodeRec(os, t.root, printed_node, nodes_requiring_labels, nodeid(), 2, false);
}

// Overloading function
bool GenerateCode(const string& filename, ltree& t) {
	ofstream os(filename);

	if (!os) {
		return false;
	}

	GenerateCode(os, t);

	return true;
}

// TODO: check
void GenerateForestCode(std::ostream& os, const Forest& f) {
	
	std::map<ltree::node*, int> printed_node;
	std::map<ltree::node*, pair<int, bool>> nodes_requiring_labels;
	nodeid id;
	for (size_t i = 0; i < f.trees_.size(); ++i) {
		const auto& t = f.trees_[i];
		printed_node[t.root] = id.next();
		CheckNodesTraversalRec(t.root, nodes_requiring_labels, id);
	}

	// Initial and internal trees (tree 0 is always the start tree)
    nodeid id2;
	//for (size_t i = 0; i < f.trees_.size(); ++i) {
	//	const auto& t = f.trees_[i];
	//	os << "tree_" << i << ": " << "finish_condition(" << i << "); \n"; // "finish_condition(n)" sarà da sostituire con "if (++c >= COLS - 1) goto break_n" quando inseriremo gli alberi di fine riga;
	//	GenerateCodeRec(os, t.root, printed_node, nodes_requiring_labels, id2, 2, true);
	//}

	//#define finish_condition(n) if ((c+=2) >= w - 2) { if (c > w - 2) { goto break_0_##n; } else { goto break_1_##n; } }
	// TODO questa versione è specifica per BBDT, bisogna trovare un modo per generalizzare rispetto allo shift della maschera!!
	for (size_t i = 0; i < f.trees_.size(); ++i) {
		const auto& t = f.trees_[i];
		os << "tree_" << i << ": if ((c+=2) >= w - 2) { if (c > w - 2) { goto break_0_" << f.end_trees_mapping_[0][i] <<"; } else { goto break_1_" << f.end_trees_mapping_[1][i] << "; } } \n";
		GenerateCodeRec(os, t.root, printed_node, nodes_requiring_labels, id2, 2, true);
	}

	// Final trees
	std::map<ltree::node*, int> printed_node_end_trees;
	std::map<ltree::node*, pair<int, bool>> nodes_requiring_labels_end_trees;
	nodeid id3;
	for (size_t tg = 0; tg < f.end_trees_.size(); ++tg) {
		const auto& cur_trees = f.end_trees_[tg];
		const auto& cur_mapping = f.end_trees_mapping_[tg];
		for (size_t i = 0; i < cur_trees.size(); ++i) {
			if (cur_mapping[i] == i) {
				//os << "break_" << zerostr(tg, 2) << "_" << zerostr(i, 2) << ":\n";
				os << "break_" << tg << "_" << i << ":\n";
				GenerateCodeRec(os, cur_trees[i].root, printed_node_end_trees, nodes_requiring_labels_end_trees, id3, 2, false);
				os << "\tcontinue;\n";
			}
		}
	}
}


