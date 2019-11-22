// Copyright(c) 2018 - 2019 
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
// * Neither the name of GRAPHGEN nor the names of its
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

#include "output_generator.h"

#include <optional>

#include "system_info.h"

using namespace std;

//void print_node(BinaryDrag<conact>::node *n, int i)
//{
//    std::cout << std::string(i, '\t');
//    if (n->data.t == conact::type::CONDITION) {
//        std::cout << n->data.condition;
//    }
//    else {
//        std::cout << ". ";
//        auto v = n->data.actions();
//
//        std::cout << v[0];
//        for (size_t i = 1; i < v.size(); ++i) {
//            std::cout << "," << v[i];
//        }
//    }
//    std::cout << "\n";
//}

string GetShape(char c) {
    switch (c) {
    case '!': return ", shape = doubleoctagon";
    case '?': return ", shape = doublecircle";
    default: return "";
    }
}

string GetNodeCode(const string& condition, bool with_root_id) {

    string real_condition = condition;
    if (condition[0] == '!' || condition[0] == '?') {
        real_condition = condition.substr(1);
    }

    if (!with_root_id) {
        size_t pos = real_condition.find("- ");
        if (pos != string::npos) {
            real_condition = real_condition.substr(pos + 2); // This supposes that there is a space between dash and condition name
        }
    }

    return " [label = \"" + real_condition + "\"" + GetShape(condition[0]) + "];\n";
}

void GenerateDotCodeForDagRec(std::ostream& os, 
                              BinaryDrag<conact>::node *n, 
                              std::map<BinaryDrag<conact>::node*, size_t>& printed_node,
                              std::vector<std::string>& links, 
                              nodeid &id, 
                              bool with_next,
                              bool with_root_id,
                              int tab) {
    os << std::string(tab, '\t') << "node" << id.get();
    if (n->isleaf()) {
        // print leaf
        os << " [label = \"";
        vector<uint> actions = n->data.actions();
        os << actions[0];
        for (size_t i = 1; i < actions.size(); ++i) {
            os << "," << actions[i];
        }
        if (with_next) {
            os << " - " << n->data.next;
        }
        os << "\", shape = box];\n";
    }
    else {
        os << GetNodeCode(n->data.condition, with_root_id);
        tab++;

        if (printed_node.find(n->left) == printed_node.end()) {
            // node not already printed
            printed_node[n->left] = id.next();
            os << string(tab, '\t') << "node" << printed_node[n] << " -> node" << printed_node[n->left] << " [label=\"0\"];\n";
            GenerateDotCodeForDagRec(os, n->left, printed_node, links, id, with_next, with_root_id, tab);
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
            GenerateDotCodeForDagRec(os, n->right, printed_node, links, id, with_next, with_root_id, tab);
        }
        else {
            std::stringstream ss;
            ss << "node" << printed_node[n] << " -> node" << printed_node[n->right] << " [label=\"1\", style=dotted];\n";
            links.push_back(ss.str());
        }
    }
}

// All nodes must have both sons! 
void GenerateDotCodeForDag(std::ostream& os, const BinaryDrag<conact>& bd, bool with_next, bool with_root_id) {
    os << "digraph dag{\n"
          "ranksep=" + conf.dot_ranksep_ + "\n" +
          "bgcolor=" + conf.dot_background_color_ + "\n" +
          "\tsubgraph tree{\n";
    nodeid id;
    
    std::map<BinaryDrag<conact>::node*, size_t> printed_node; // = { { bd.roots_[0] } , 0 };
    std::vector<std::string> links;
    
    for (size_t i = 0; i < bd.roots_.size(); ++i) {
        string tmp = bd.roots_[i]->data.condition;
        if (i == 0) {
            bd.roots_[i]->data.condition = "!" + tmp; // "! -> identifies the root of the start tree"
        }
        else {
            bd.roots_[i]->data.condition = "?" + to_string(i) + " - " + tmp; // "?" -> identifies tree's root
        }
        printed_node[bd.roots_[i]] = id.next();
        GenerateDotCodeForDagRec(os, bd.roots_[i], printed_node, links, id, with_next, with_root_id, 2);
        bd.roots_[i]->data.condition = tmp; // Turn the condition back to its original value
    }

    os << "\t}\n";
    for (size_t i = 0; i < links.size(); ++i) {
        os << links[i];
    }

    os << "}\n";
}

string GetDotCallString(const std::string& code_path, const std::string& output_path = "")
{
    string dot_call;

#ifdef GRAPHGEN_WINDOWS
    dot_call = "..\\tools\\dot\\dot -T" + conf.dot_output_format_.substr(1) + " \"" + code_path + "\"";
#endif

#ifdef GRAPHGEN_LINUX
    dot_call = "dot -T" + conf.dot_output_format_.substr(1) + " \"" + code_path + "\"";
#endif
    
    if (dot_call.length() == 0) {
        throw std::runtime_error("Error in generating graph output: Unsupported operating system for using dot module of graphviz (Linux and Windows only).");
    }

    if(output_path != ""){
        dot_call += " -o \"" + output_path + "\"";
    }

    return dot_call;
}

string GetDotCallString(const filesystem::path& code_path, filesystem::path output_path = "") {
    return GetDotCallString(code_path.string(), output_path.string());
}

bool DrawDagOnFile(const string& base_filename, const BinaryDrag<conact> &t, DrawDagFlags flags) {

    bool with_next = flags & DrawDagFlags::WITH_NEXT;
    bool verbose = flags & DrawDagFlags::VERBOSE;
    bool delete_dotcode = flags & DrawDagFlags::DELETE_DOTCODE;
    bool with_root_id = flags & DrawDagFlags::WITH_ROOT_ID;
    
    if (verbose) {
        std::cout << "Drawing DAG: " << base_filename << ".. ";
    }
    filesystem::path code_path = conf.GetDotCodePath(base_filename);
    filesystem::path pdf_path = conf.GetDotOutPath(base_filename);
    ofstream os(code_path);
    if (!os) {
        if (verbose) {
            std::cout << "Unable to generate " << code_path << ", stopped\n";
        }
        return false;
    }
    GenerateDotCodeForDag(os, t, with_next, with_root_id);
    os.close();
    if (0 != system(GetDotCallString(code_path, pdf_path).c_str())) {
        if (verbose) {
            std::cout << "Unable to generate " + pdf_path.string() + ", stopped\n";
        }
        return false;
    }

    if (verbose) {
        std::cout << "done\n";
    }

    if (delete_dotcode) {
        remove(code_path.string().c_str());
    }
    return true;
}

bool DrawForestOnFile(const string& output_file, const LineForestHandler& lfh, DrawDagFlags flags)
{
    bool result = DrawDagOnFile(output_file + "_main_forest", lfh.f_, flags | DrawDagFlags::WITH_NEXT | DrawDagFlags::WITH_ROOT_ID);
    for (size_t i = 0; i < lfh.end_forests_.size(); ++i) {
        result &= DrawDagOnFile(output_file + "_end_forest_" + to_string(i), lfh.end_forests_[i], flags | DrawDagFlags::WITH_ROOT_ID);
    }

    return result;
}
