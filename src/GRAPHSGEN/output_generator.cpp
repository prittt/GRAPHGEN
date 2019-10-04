// Copyright(c) 2018 - 2019 Costantino Grana, Federico Bolelli 
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

#include "output_generator.h"

#include <optional>

using namespace std;

void print_node(BinaryDrag<conact>::node *n, int i)
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

string GetShape(char c) {
    switch (c) {
    case '!': return ", shape = doubleoctagon";
    case '?': return ", shape = doublecircle";
    default: return "";
    }
}

string GetNodeCode(const string& condition) {

    string real_condition = condition;
    if (condition[0] == '!' || condition[0] == '?') {
        real_condition = condition.substr(1);
    }

    return " [label = \"" + real_condition + "\"" + GetShape(condition[0]) + "];\n";
}

void GenerateDotCodeForDagRec(std::ostream& os, BinaryDrag<conact>::node *n, std::map<BinaryDrag<conact>::node*, int>& printed_node, std::vector<std::string>& links, nodeid &id, bool with_next, int tab) {
    os << std::string(tab, '\t') << "node" << id.get();
    if (n->isleaf()) {
        // print leaf
        os << " [label = \"";
        vector<uint> actions = n->data.actions();
        os << actions[0];
        for (size_t i = 1; i < actions.size(); ++i) {
            os << "," << actions[i];
        }
        if (with_next)
            os << " - " << n->data.next;
        os << "\", shape = box];\n";
    }
    else {
        os << GetNodeCode(n->data.condition);
        tab++;

        if (printed_node.find(n->left) == printed_node.end()) {
            // node not already printed
            printed_node[n->left] = id.next();
            os << string(tab, '\t') << "node" << printed_node[n] << " -> node" << printed_node[n->left] << " [label=\"0\"];\n";
            GenerateDotCodeForDagRec(os, n->left, printed_node, links, id, with_next, tab);
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
            GenerateDotCodeForDagRec(os, n->right, printed_node, links, id, with_next, tab);
        }
        else {
            std::stringstream ss;
            ss << "node" << printed_node[n] << " -> node" << printed_node[n->right] << " [label=\"1\", style=dotted];\n";
            links.push_back(ss.str());
        }
    }
}

// All nodes must have both sons! 
void GenerateDotCodeForDag(std::ostream& os, const BinaryDrag<conact>& t, bool with_next = false) {
    os << "digraph dag{\n"
          "bgcolor=\"transparent\""
          "\tsubgraph tree{\n";
    nodeid id;
    std::map<BinaryDrag<conact>::node*, int> printed_node = { { t.GetRoot(), 0 } };
    std::vector<std::string> links;
    for (size_t i = 0; i < t.roots_.size(); ++i) {
        string tmp = t.GetRoot()->data.condition;
        if (i == 0)
            t.roots_[i]->data.condition = "!" + tmp; // "! -> identifies the root of the start tree"
        else
            t.roots_[i]->data.condition = "?" + to_string(i) + " - " + tmp; // "?" -> identifies tree's root
        printed_node[t.roots_[i]] = id.next();
        GenerateDotCodeForDagRec(os, t.roots_[i], printed_node, links, id, with_next, 2);
        // Turn the condition back to its original value
        t.roots_[i]->data.condition = tmp;
    }

    os << "\t}\n";
    for (size_t i = 0; i < links.size(); ++i) {
        os << links[i];
    }

    os << "}\n";
}

string GetDotCallString(const std::string& code_path, const std::string& output_path = "")
{
    string dot_call = "..\\tools\\dot\\dot -T" + conf.dot_output_format_.substr(1) + " \"" + code_path + "\"";
    if(output_path != ""){
        dot_call += " -o \"" + output_path + "\"";
    }
    //std::cout << dot_call;
    return dot_call;
}

string GetDotCallString(const filesystem::path& code_path, filesystem::path output_path = "") {
    return GetDotCallString(code_path.string(), output_path.string());
}

bool DrawDagOnFile(const string& output_file, const BinaryDrag<conact> &t, int flags) {

    bool with_next = false;
    bool verbose = false; 
    bool delete_dotcode = false;

    if (flags & DrawDagFlags::WITH_NEXT) {
        bool with_next = true;
    }

    if (flags & DrawDagFlags::VERBOSE) {
        bool verbose = true;
    }

    if (flags & DrawDagFlags::DELETE_DOTCODE) {
         bool delete_dotcode = true;
    }
    
    if (verbose) {
        std::cout << "Drawing DAG: " << output_file << ".. ";
    }
    filesystem::path code_path = conf.GetDotCodePath(output_file);
    filesystem::path pdf_path = conf.GetDotOutPath(output_file);
    ofstream os(code_path);
    if (!os) {
        if (verbose) {
            std::cout << "Unable to generate " << code_path << ", stopped\n";
        }
        return false;
    }
    GenerateDotCodeForDag(os, t, with_next);
    os.close();
    if (0 != system(GetDotCallString(code_path, pdf_path).c_str())) {
        if (verbose) {
            std::cout << "Unable to generate " + pdf_path.string() + ", stopped\n";
        }
        return false;
    }
    if (delete_dotcode) {
        remove(code_path.string().c_str());
    }
    if (verbose) {
        std::cout << "done\n";
    }
    return true;
}

bool DrawMainForestOnFile(const string& output_file, const Forest& f, bool save_dotcode, bool verbose) {
    if (verbose) {
        std::cout << "Drawing Main Forest: " << output_file << ".. ";
    }
    filesystem::path code_path = conf.GetDotCodePath(output_file);
    filesystem::path pdf_path = conf.GetDotOutPath(output_file);
    ofstream os(code_path);
    if (!os) {
        if (verbose) {
            std::cout << "Unable to generate " << code_path << ", stopped\n";
        }
        return false;
    }
    os << "digraph dag{\n"
          "bgcolor=\"transparent\""
          "\tranksep = 1.5;\n"
          "\tsubgraph tree{\n";

    nodeid id;
    std::map<ltree::node*, int> printed_node;
    std::vector<std::string> links;
    for (size_t i = 0; i < f.trees_.size(); ++i) {
        const auto& t = f.trees_[i];
        string tmp = t.GetRoot()->data.condition;
        if (i == 0)
            t.GetRoot()->data.condition = "!" + tmp; // "! -> identifies the root of the start tree"
        else
            t.GetRoot()->data.condition = "?" + to_string(i) + " - " + tmp; // "?" -> identifies tree's root
        printed_node[t.GetRoot()] = id.next();
        GenerateDotCodeForDagRec(os, t.GetRoot(), printed_node, links, id, true, 2);
        // Turn the condition back to its original value
        t.GetRoot()->data.condition = tmp;
    }

    os << "\t}\n";
    for (size_t i = 0; i < links.size(); ++i) {
        os << links[i];
    }

    os << "}\n";

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

    if (!save_dotcode) {
        remove(code_path.string().c_str());
    }
    return true;
}

// TODO try to write a single main common function for DrawEndForestOnFile and DrawMainForestOnFile
bool DrawEndForestOnFile(const string& output_file, const Forest& f, bool save_dotcode, bool verbose) {
    if (verbose) {
        std::cout << "Drawing End Forest: " << output_file << ".. ";
    }
    filesystem::path code_path = conf.GetDotCodePath(output_file);
    filesystem::path pdf_path = conf.GetDotOutPath(output_file);
    ofstream os(code_path);
    if (!os) {
        if (verbose) {
            std::cout << "Unable to generate " << code_path << ", stopped\n";
        }
        return false;
    }
    os << "digraph dag{\n"
        "\tranksep = 1.5;\n"
        "\tsubgraph tree{\n";

    nodeid id;
    std::map<ltree::node*, int> printed_node;
    std::vector<std::string> links;
    for (size_t tg = 0; tg < f.end_trees_.size(); ++tg) {
        const auto& cur_trees = f.end_trees_[tg];
        for (size_t i = 0; i < cur_trees.size(); ++i) {
            const auto& t = cur_trees[i];
            // In order to easily distinguish roots we change the condition string
            string tmp = t.GetRoot()->data.condition;
            t.GetRoot()->data.condition = "?" + to_string(tg) + "," + to_string(i) + " - " + tmp; // "?" -> identifies tree's root
            printed_node[t.GetRoot()] = id.next();
            GenerateDotCodeForDagRec(os, t.GetRoot(), printed_node, links, id, false, 2);
            // Turn the condition back to its original value
            t.GetRoot()->data.condition = tmp;
        }
    }

    os << "\t}\n";
    for (size_t i = 0; i < links.size(); ++i) {
        os << links[i];
    }

    os << "}\n";

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

    if (!save_dotcode) {
        remove(code_path.string().c_str());
    }
    return true;
}

bool DrawForestOnFile(const string& output_file, const Forest& f, bool save_dotcode, bool verbose)
{
    bool m = DrawMainForestOnFile(output_file + "_mainforest", f, save_dotcode, verbose);
    bool e = DrawEndForestOnFile(output_file + "_endforest", f, save_dotcode, verbose);

    return m && e;
}
