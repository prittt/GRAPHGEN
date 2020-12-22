// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "graph_code_generator.h"

#include <map>

#include "utilities.h"

using namespace std;

BEFORE_AFTER_FUNC(DefaultEmptyFunc)
{ 
    return std::string(); 
}

// This function defines and returns the string that should be put before each (main) 
// tree when generating the code for a forest of decision trees. The string contains 
// an if to check whether the end of a line is reached and a goto to the corresponding 
// tree in the end line forest. The string is specific for algorithms which exploit a 
// mask with a horizontal shift of one pixel like most of the thinning algorithms, PRED, 
// SAUF, CTB and so on. When the end line forest is not generated a different string 
// should be used, for example replacing the goto with a continue and changing the if
// condition accordingly. 
BEFORE_AFTER_FUNC(BeforeMainShiftOne)
{
    return prefix + "tree_" + to_string(index) + ": if ((c+=1) >= w - 1) goto " +
           prefix + "break_0_" + to_string(mapping[0][index]) + ";\n";
}

// This function defines and returns the string that should be put before each (main) 
// tree when generating the code for a forest of decision trees. The string contains 
// an if to check whether the end of a line is reached and a goto to the corresponding 
// tree in the end line forest. The string is specific for algorithms which exploit a 
// mask with a horizontal shift of two pixels like BBDT, DRAG, Spaghetti and so on. 
// When the end line forest is not generated a different string should be used, for 
// example replacing the goto with a continue and changing the if condition accordingly.
BEFORE_AFTER_FUNC(BeforeMainShiftTwo)
{
    return prefix + "tree_" + to_string(index) + ": if ((c+=2) >= w - 2) { if (c > w - 2) { goto " +
           prefix + "break_0_" + to_string(mapping[0][index]) + "; } else { goto " +
           prefix + "break_1_" + to_string(mapping[1][index]) + "; } } \n";
}

BEFORE_AFTER_FUNC(BeforeEnd)
{
    return prefix + "break_" + to_string(end_group_id) + "_" + to_string(index) + ":\n";
}

BEFORE_AFTER_FUNC(AfterEnd)
{
    return std::string(2, '\t') + "continue;\n";
}

BEFORE_AFTER_FUNC(AfterEndNoLoop) 
{
    return std::string(2, '\t') + "goto " + prefix + ";\n";
}


// This class allows to sum-up all the data required by the recursive functions that generate
// the DRAG source code, thus simplifying its signature/call.
class GenerateCodeClass {
    // with_gotos_ is used to add gotos to next DRAGs on leaves.
    bool with_gotos_;

    std::string prefix_;

    // printed_node keeps track of the nodes that have already been written in the C++ source code and allows to avoid 
    // duplicated nodes in the final result. Indeed, the same node can be pointed by multiple nodes in the DAG but it 
    // will have to appear only once in the final code.
    std::map<BinaryDrag<conact>::node*, size_t> printed_nodes_;

    // nodes_requring_labels keeps track of the DAG nodes that are pointed by other nodes and thus need to have a label.
    // We need this in order to know if we have to create a label for this node or not. This map is populated by the 
    // CheckNodesTraversalRec procedure. 
    std::map<BinaryDrag<conact>::node*, bool> nodes_requiring_labels_;

    nodeid id_;

public:

    GenerateCodeClass(bool with_gotos, std::string prefix, map<BinaryDrag<conact>::node*, size_t> printed_nodes) :
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

    void SetId(size_t id) {
        id_.SetId(id);
    }

    size_t NextId() {
        return id_.next();
    }

    size_t GetId() {
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

    // This procedure writes to the output stream the C++ source exploring recursively the specified DRAG. 
    // When a leaf with multiple actions is found only the first action is considered and written in the
    // output file.
    void GenerateCodeRec(std::ostream& os, BinaryDrag<conact>::node *n, int tab)
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
    void CheckNodesTraversalRec(BinaryDrag<conact>::node *n)
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
// generation, so it is useless during the code generation of a tree. TODO, remove it? 
//bool GenerateDragCode(const string& algorithm_name, BinaryDrag<conact>& bd, std::string prefix)
//{
//    filesystem::path code_path = conf.treecode_path_;
//
//    ofstream os(code_path);
//    if (!os) {
//        return false;
//    }
//
//    // This object wraps all the variables needed by the recursive function GenerateCodeRec and allows to simplify its
//    // following call.
//    GenerateCodeClass gcc(bd.roots_.size() > 1, prefix, { {} }); // t.roots_.size() > 1 serves to distinguish between
//                                                                 // "simple" and multi-rooted DRAGs. In the latter case
//                                                                 // gotos to the next tree will be added.
//
//    // Populates the nodes_requring_labels to keep tracks of the DAG nodes that are pointed by other nodes and thus need
//    // to have a label
//    for (auto& t : bd.roots_) {
//        gcc.CheckNodesTraversalRec(t);
//    }
//
//    // This function actually generates and writes into the output stream the C++ source code using pre-calculated data.
//    for (auto& t : bd.roots_) {
//        // TODO We actually need to call prefix and suffix functions here.
//        gcc.GenerateCodeRec(os, t, 2);
//    }
//
//    return true;
//}

bool GenerateDragCode(const BinaryDrag<conact>& bd, 
                      bool with_gotos,
                      BEFORE_AFTER_FUNC(before),
                      BEFORE_AFTER_FUNC(after),
                      const std::string prefix,
                      size_t start_id,
                      const std::vector<std::vector<size_t>> mapping, 
                      size_t end_group_id)
{
    filesystem::path code_path = conf.treecode_path_;
    
    ofstream os(code_path);
    if (!os) {
        return false;
    }

    return GenerateDragCode(os, bd, with_gotos, before, after, prefix, start_id, mapping, end_group_id);
}

size_t GenerateDragCode(std::ostream& os, 
                     const BinaryDrag<conact>& bd, 
                     bool with_gotos,
                     BEFORE_AFTER_FUNC(before),
                     BEFORE_AFTER_FUNC(after),
                     const std::string prefix,
                     size_t start_id,
                     const std::vector<std::vector<size_t>> mapping, 
                     size_t end_group_id)
{
    // This object wraps all the variables needed by the recursive function GenerateCodeRec and allows to simplify its
    // following call.
    GenerateCodeClass gcc(with_gotos, prefix, { {} });

    // Populates the nodes_requring_labels to keep tracks of the DAG nodes that are pointed by other nodes and thus need
    // to have a label
    for (auto& t : bd.roots_) {
        gcc.CheckNodesTraversalRec(t);
    }

    // And then we generate and write into the output stream the C++ source code using pre-calculated data.
    gcc.SetId(start_id);
    for (size_t i = 0; i < bd.roots_.size(); ++i) {
        os << before(i, prefix, mapping, end_group_id);
        gcc.GenerateCodeRec(os, bd.roots_[i], 2);
        os << after(i, prefix, mapping, end_group_id);
    }

    return gcc.GetId();
}

// This function generates forest code using numerical labels starting from start_id and returns 
// the last used_id.
size_t GenerateLineForestCode(std::ostream& os, 
                              const LineForestHandler& lfh,
                              std::string prefix,
                              size_t start_id,
                              BEFORE_AFTER_FUNC(before_main),
                              BEFORE_AFTER_FUNC(after_main),
                              BEFORE_AFTER_FUNC(before_end),
                              BEFORE_AFTER_FUNC(after_end))
{
    // Generate the code for the main forest
    size_t last_id = GenerateDragCode(os, lfh.f_, true, before_main, after_main, prefix, start_id, lfh.main_end_tree_mapping_);

    // Generate the code for the end of the line forests
    for (size_t i = 0; i < lfh.end_forests_.size(); ++i) {
        last_id = GenerateDragCode(os, lfh.end_forests_[i], false, before_end, after_end, prefix, last_id, lfh.main_end_tree_mapping_, i);
    }

    return last_id;
}