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
    // with_gotos_ is used to add gotos to next DRAGs on leaves.
    bool with_gotos_;

    std::string prefix_;

    // printed_node keeps track of the nodes that have already been written in the C++ source code and allows to avoid 
    // duplicated nodes in the final result. Indeed, the same node can be pointed by multiple nodes in the DAG but it 
    // will have to appear only once in the final code.
    std::map<ltree::node*, int> printed_nodes_;

    // nodes_requring_labels keeps track of the DAG nodes that are pointed by other nodes and thus need to have a label.
    // We need this in order to know if we have to create a label for this node or not. This map is populated by the 
    // ChekNodesTraversalRec procedure. 
    std::map<ltree::node*, bool> nodes_requiring_labels_;

    nodeid id_;

public:

    GenerateCodeClass(bool with_gotos, std::string prefix, map<ltree::node*, int> printed_nodes) :
        with_gotos_(with_gotos),
        prefix_(prefix),
        printed_nodes_(printed_nodes)
    {}

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

    // This procedure write to the output stream the C++ source exploring recursively the specified DRAG. 
    // When a leaf with multiple actions is found only the first action is considered and write in the
    // output file.
    void GenerateCodeRec(std::ostream& os, ltree::node *n, int tab) 
    {
        // Extract needed data from the GenerateCodeClass
        auto& m = printed_nodes_;
        auto& ml = nodes_requiring_labels_;
        
        if (n->isleaf()) {
            vector<uint> actions = n->data.actions();
            os << string(tab, '\t') << "ACTION_" << actions[0] << "\n";
            if (with_gotos_) {
                os << string(tab, '\t') << "goto " << prefix_ << "tree_" << n->data.next << ";\n";
            }
            return;
        }

        if (m.find(n) == end(m)) {
            // code not generated yet
            if (ml[n]) {
                // The node will be accessed more than once, so we store its Id in a map to remember that
                // we already generated the corresponding code
                m[n] = NextId();
                os << string(tab, '\t') << "NODE_" << m[n] << ":\n";
            }
            string condition = n->data.condition;
            transform(condition.begin(), condition.end(), condition.begin(), ::toupper);
            os << string(tab, '\t') << "if (CONDITION_" << condition << ") {\n";
            GenerateCodeRec(os, n->right, tab + 1);
            os << string(tab, '\t') << "}\n";
            os << string(tab, '\t') << "else {\n";
            GenerateCodeRec(os, n->left, tab + 1);
            os << string(tab, '\t') << "}\n";
        }
        else {
            // code already exists
            os << string(tab, '\t') << "goto NODE_" << m[n] << ";\n";
        }
    }

    // This procedure checks and stores (inside the nodes_requiring_labels_ map) the nodes which will require labels, 
    // that are nodes pointed by other nodes inside the DAG. This allows to handle labels/gotos during the generation 
    // of the C++ source code.
    void CheckNodesTraversalRec(ltree::node *n) 
    {
        auto& ml = nodes_requiring_labels_;

        if (n->isleaf())
            return;

        if (ml.find(n) != end(ml)) {
            ml[n] = true;
        }
        else {
            ml[n] = false;
            CheckNodesTraversalRec(n->left);
            CheckNodesTraversalRec(n->right);
        }
    }

};

// Actual implementation of the GenerateDragCode func. This allows to hide useless 
// parameters (like prefix string) from the public interface. The prefix string is
// used to generate specific labels for the start/end trees during the forest code
// generation, so it is useless during the code generation of a tree.
bool GenerateDragCode(const string& algorithm_name, ltree& t, std::string prefix)
{
    filesystem::path code_path = conf.treecode_path_;

    ofstream os(code_path);
    if (!os) {
        return false;
    }

    // This object wraps all the variables needed by the recursive function GenerateCodeRec and allows to simplify its
    // following call.
    GenerateCodeClass gcc(false, prefix, /*{ { t.root, 0 } }*/{});

    // Populates the nodes_requring_labels to keep tracks of the DAG nodes that are pointed by other nodes and thus need
    // to have a label
    gcc.CheckNodesTraversalRec(t.root);

    // This function actually generates and writes in the output stream the C++ source code using pre-calculated data.
    gcc.GenerateCodeRec(os, t.root, 2);

    return true;
}

// GenerateDragCode public interface.
bool GenerateDragCode(const string& algorithm_name, ltree& t) 
{
    return GenerateDragCode(algorithm_name, t, "");
}

// This function generates forest code using numerical labels starting from start_id and returns the last used_id
int GenerateForestCode(std::ostream& os, const Forest& f, std::string prefix, int start_id, int mask_shift) 
{
    GenerateCodeClass gcc_main(true, prefix, {{}});
    
    for (size_t i = 0; i < f.trees_.size(); ++i) {
        const auto& t = f.trees_[i];
        gcc_main.CheckNodesTraversalRec(t.root);
    }

    // TODO questa versione è specifica per BBDT, bisogna trovare un modo per generalizzare rispetto allo shift della maschera!!
    gcc_main.SetId(start_id);
    for (size_t i = 0; i < f.trees_.size(); ++i) {
        const auto& t = f.trees_[i];
        if (mask_shift == 1) {
            // Thinning, PRED, SAUF, CTB 
            os << prefix << "tree_" << i << ": if ((c+=1) >= w - 1) goto " << prefix << 
                "break_0_" << f.main_trees_end_trees_mapping_[0][i] << ";\n";
        }
        else if (mask_shift == 2) {
            // BBDT, DRAG, SPAGHETTI
            os << prefix << "tree_" << i << ": if ((c+=2) >= w - 2) { if (c > w - 2) { goto " << prefix << 
                "break_0_" << f.main_trees_end_trees_mapping_[0][i] << "; } else { goto " << prefix << 
                "break_1_" << f.main_trees_end_trees_mapping_[1][i] << "; } } \n";
        }
        gcc_main.GenerateCodeRec(os, t.root, 2);
    }

    // End trees
    GenerateCodeClass gcc_end(false, prefix, { {} });

    for (size_t tg = 0; tg < f.end_trees_.size(); ++tg) {
        const auto& cur_trees = f.end_trees_[tg];
        for (size_t i = 0; i < cur_trees.size(); ++i) {
            const auto& t = cur_trees[i];
            gcc_end.CheckNodesTraversalRec(t.root);
        }
    }

    gcc_end.SetId(gcc_main.GetId());
    for (size_t tg = 0; tg < f.end_trees_.size(); ++tg) {
        const auto& cur_trees = f.end_trees_[tg];
        for (size_t i = 0; i < cur_trees.size(); ++i) {
            os << prefix << "break_" << tg << "_" << i << ":\n";
            gcc_end.GenerateCodeRec(os, cur_trees[i].root, 2);
            os << string(2, '\t') << "continue;\n";
        }
    }

    return gcc_end.GetId();
}
