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

#include "graph_code_generator.h"

#include <map>

#include "utilities.h"

using namespace std;

// This class allows to sum-up all the data required by the recursive functions that generate
// the DRAG source code, thus simplifying its signature/call.
class GenerateCodeClass {
    bool with_gotos_;
    std::string prefix_;

    // printed_node keep tracks of the nodes that have already been written in the C++ source code and allows to avoid 
    // duplicated nodes in the final result. Indeed, the same node can be pointed by multiple nodes in the DAG but it 
    // will have to appear only once in the final code.
    std::map<ltree::node*, int> printed_nodes_;

    // nodes_requring_labels keep tracks of the DAG nodes that are pointed by other nodes and thus need to have a label.
    // This map is populated by the ChekNodesTraversalRec procedure.
    std::map<ltree::node*, bool> nodes_requiring_labels_;

    nodeid id_;

public:

    GenerateCodeClass(bool with_gotos, std::string prefix, map<ltree::node*, int> printed_nodes) :
        with_gotos_(with_gotos),
        prefix_(prefix),
        printed_nodes_(printed_nodes)
    {}

    //GenerateCodeClass(bool with_gotos, std::string prefix, map<ltree::node*, int> printed_nodes, std::map<ltree::node*, bool> nodes_requiring_labels) :
    //    with_gotos_(with_gotos),
    //    prefix_(prefix),
    //    printed_nodes_(printed_nodes),
    //    nodes_requiring_labels_(nodes_requiring_labels)
    //{}

    void Clear() {
        ClearPrintedNodes();
        ClearNodesRequiringLabels();
        ClearId();
    }

    void ClearPrintedNodes() {
        printed_nodes_.clear();
    }

    void ClearNodesRequiringLabels() {
        nodes_requiring_labels_.clear();
    }

    void ClearId() {
        id_.Clear();
    }

    void SetId(int id) {
        id_.SetId(id);
    }

    int NextId() {
        return id_.next();
    }

    int GetId() {
        return id_.get();
    }

    void SetPrefix(std::string prefix) {
        if (!prefix.empty() && prefix.back() != '_') {
            prefix += '_';
        }
        prefix_ = std::move(prefix);
    }

    void SetWithGotos(bool with_gotos) {
        with_gotos_ = with_gotos;
    }

    bool WithGotos() {
        return with_gotos_;
    }

    auto& GetNodeRequiringLabelsMap() {
        return nodes_requiring_labels_;
    }

    auto& GetPrintedNodeMap() {
        return printed_nodes_;
    }

    std::string& GetPrefix() {
        return prefix_;
    }
};

// This procedure write to the output stream the C++ source exploring recursively the specified DRAG. 
// When a leaf with multiple actions is found only the first action is considered and write in the
// output file.
void GenerateCodeRec(std::ostream& os, ltree::node *n, std::map<ltree::node*, int>& visited_nodes, std::map<ltree::node*, bool>& nodes_requiring_labels, nodeid &id, int tab, bool add_gotos, const std::string& prefix) {
    auto& m = visited_nodes;
    auto& ml = nodes_requiring_labels;
    if (n->isleaf()) {
        vector<uint> actions = n->data.actions();
        os << string(tab, '\t') << "ACTION_" << actions[0] << "\n";
        if (add_gotos)
            os << string(tab, '\t') << "goto " << prefix << "tree_" << n->data.next << ";\n";
    }
    else {
        if (ml[n]) {
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
            GenerateCodeRec(os, n->right, m, ml, id, tab + 1, add_gotos, prefix);
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
            GenerateCodeRec(os, n->left, m, ml, id, tab + 1, add_gotos, prefix);
            os << string(tab, '\t') << "}\n";
        }
        else {
            // code already exists
            os << "{\n" << string(tab + 1, '\t') << "goto NODE_" << m[n->left] << ";\n";
            os << string(tab, '\t') << "}\n";
        }
    }
}

// This procedure write to the output stream the C++ source exploring recursively the specified DRAG. 
// When a leaf with multiple actions is found only the first action is considered and write in the
// output file.
void GenerateCodeRec(std::ostream& os, ltree::node *n, int tab, GenerateCodeClass& gcc) {
    
    // Extract needed data from the GenerateCodeClass
    auto& m = gcc.GetPrintedNodeMap();
    auto& ml = gcc.GetNodeRequiringLabelsMap();
    std::string prefix = gcc.GetPrefix();
    bool add_gotos = gcc.WithGotos();

    if (n->isleaf()) {
        vector<uint> actions = n->data.actions();
        os << string(tab, '\t') << "ACTION_" << actions[0] << "\n";
        if (add_gotos)
            os << string(tab, '\t') << "goto " << prefix << "tree_" << n->data.next << ";\n";
    }
    else {
        if (ml[n]) {
            os << string(tab, '\t') << "NODE_" << gcc.GetId() << ":\n";
        }
        string condition = n->data.condition;
        transform(condition.begin(), condition.end(), condition.begin(), ::toupper);
        os << string(tab, '\t') << "if (CONDITION_" << condition << ")";
        if (m.find(n->right) == end(m)) {
            // not found
            if (!n->right->isleaf()) {
                m[n->right] = gcc.NextId();
            }
            os << " {\n";
            GenerateCodeRec(os, n->right, tab + 1, gcc);
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
                m[n->left] = gcc.NextId();
            }
            os << " {\n";
            GenerateCodeRec(os, n->left, tab + 1, gcc);
            os << string(tab, '\t') << "}\n";
        }
        else {
            // code already exists
            os << "{\n" << string(tab + 1, '\t') << "goto NODE_" << m[n->left] << ";\n";
            os << string(tab, '\t') << "}\n";
        }
    }
}

// This procedure check and store (inside the m map) the nodes which will require labels, that are nodes 
// pointed by other nodes inside the DAG. This allows to handle labels/gotos during the generation of the
// C++ source code.
void CheckNodesTraversalRec(ltree::node *n, std::map<ltree::node*, bool>& m) {
    if (n->isleaf()) 
        return;

    // Right children
    if (m.find(n->right) == end(m)) {
        // not found
        if (!n->right->isleaf()) {
            m[n->right] = false;
        }
        CheckNodesTraversalRec(n->right, m);
    }
    else {
        m[n->right] = true;
    }

    // Left children
    if (m.find(n->left) == end(m)) {
        // not found
        if (!n->left->isleaf()) {
            m[n->left] = false;
        }
        CheckNodesTraversalRec(n->left, m);
    }
    else {
        m[n->left] = true;
    }
}

// Actual implementation of the GenerateDragCode func. This allows to hide useless 
// parameters (like prefix string) from the public interface. The prefix string is
// used to generate specific labels for the start/end trees during the forest code
// generation, so it is useless during the code generation of a tree.
bool GenerateDragCode(const string& algorithm_name, ltree& t, std::string prefix)
{
    filesystem::path code_path = global_output_path / filesystem::path(algorithm_name + "_code.inc.h");

    ofstream os(code_path);
    if (!os) {
        return false;
    }

    // This object wraps all the variables needed by the recursive function GenerateCodeRec and allows to simplify its
    // following call.
    GenerateCodeClass gcc(false, prefix, { { t.root, 0 } });

    // Populates the nodes_requring_labels to keep tracks of the DAG nodes that are pointed by other nodes and thus need
    // to have a label
    CheckNodesTraversalRec(t.root, gcc.GetNodeRequiringLabelsMap());

    // This function actually generates and writes in the output stream the C++ source code using pre-calculated data.
    GenerateCodeRec(os, t.root, 2, gcc);

    return true;
}

// GenerateDragCode public interface.
bool GenerateDragCode(const string& algorithm_name, ltree& t) {
    return GenerateDragCode(algorithm_name, t, "");
}

// This function generates forest code using numerical labels starting from start_id and returns the last used_id
int GenerateForestCode(std::ostream& os, const Forest& f, std::string prefix, int start_id) {

    std::map<ltree::node*, int> printed_node;
    std::map<ltree::node*, bool> nodes_requiring_labels;
    nodeid id;
    for (size_t i = 0; i < f.trees_.size(); ++i) {
        const auto& t = f.trees_[i];
        // printed_node[t.root] = id.next();
        CheckNodesTraversalRec(t.root, nodes_requiring_labels);
    }

    //#define finish_condition(n) if ((c+=2) >= w - 2) { if (c > w - 2) { goto break_0_##n; } else { goto break_1_##n; } }
    // TODO questa versione è specifica per BBDT, bisogna trovare un modo per generalizzare rispetto allo shift della maschera!!
    id.Clear();
    id.SetId(start_id);
    for (size_t i = 0; i < f.trees_.size(); ++i) {
        const auto& t = f.trees_[i];
        // BBDT os << "tree_" << i << ": if ((c+=2) >= w - 2) { if (c > w - 2) { goto break_0_" << f.main_trees_end_trees_mapping_[0][i] << "; } else { goto break_1_" << f.main_trees_end_trees_mapping_[1][i] << "; } } \n";
        /* Thinning */ os << prefix << "tree_" << i << ": if ((c+=1) >= w - 1) goto " << prefix << "break_0_" << f.main_trees_end_trees_mapping_[0][i] << ";\n";
        GenerateCodeRec(os, t.root, printed_node, nodes_requiring_labels, id, 2, true, prefix);
    }
    int last_id_main_forest_node = id.get();

    // Final trees
    std::map<ltree::node*, int> printed_node_end_trees;
    std::map<ltree::node*, bool> nodes_requiring_labels_end_trees;
    id.Clear();
    for (size_t tg = 0; tg < f.end_trees_.size(); ++tg) {
        const auto& cur_trees = f.end_trees_[tg];
        for (size_t i = 0; i < cur_trees.size(); ++i) {
            const auto& t = cur_trees[i];
            // printed_node_end_trees[t.root] = id.next();
            CheckNodesTraversalRec(t.root, nodes_requiring_labels_end_trees);
        }
    }

    id.SetId(last_id_main_forest_node);
    for (size_t tg = 0; tg < f.end_trees_.size(); ++tg) {
        const auto& cur_trees = f.end_trees_[tg];
        for (size_t i = 0; i < cur_trees.size(); ++i) {
            //os << "break_" << zerostr(tg, 2) << "_" << zerostr(i, 2) << ":\n";
            os << prefix << "break_" << tg << "_" << i << ":\n";
            GenerateCodeRec(os, cur_trees[i].root, printed_node_end_trees, nodes_requiring_labels_end_trees, id, 2, false, prefix);
            os << "\tcontinue;\n";
        }
    }

    return id.get();
}
