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

#include "output_generator.h"

using namespace std;

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

void GenerateDotCodeForDagRec(std::ostream& os, tree<conact>::node *n, std::map<tree<conact>::node*, int>& printed_node, std::vector<std::string>& links, nodeid &id, bool with_next, int tab) {
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
        os << " [label = \"" << n->data.condition << "\"];\n";
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
void GenerateDotCodeForDag(std::ostream& os, tree<conact>& t, bool with_next) {
    os << "digraph dag{\n";
    os << "\tsubgraph tree{\n";

    std::map<tree<conact>::node*, int> printed_node = { { t.root, 0 } };;
    std::vector<std::string> links;
    GenerateDotCodeForDagRec(os, t.root, printed_node, links, nodeid(), with_next, 2);

    os << "\t}\n";
    for (size_t i = 0; i < links.size(); ++i) {
        os << links[i];
    }

    os << "}\n";
}

// "output_file": output file name without extension 
// "t": tree<conact> to draw
// "verbose": to print messages on standard output
// return true if the process ends correctly, false otherwise
bool DrawDagOnFile(const string& output_file, tree<conact> &t, bool with_next, bool verbose) {

    if (verbose) {
        std::cout << "Drawing DAG: " << output_file << ".. ";
    }
    string output_path_lowercase = output_file;
    std::transform(output_path_lowercase.begin(), output_path_lowercase.end(), output_path_lowercase.begin(), ::tolower);
    output_path_lowercase = global_output_path + output_path_lowercase;
    string code_path = output_path_lowercase + "_dotcode.txt";
    string pdf_path = output_path_lowercase + ".pdf";
    ofstream os(code_path);
    if (!os) {
        if (verbose) {
            std::cout << "Unable to generate " << code_path << ", stopped\n";
        }
        return false;
    }
    GenerateDotCodeForDag(os, t, with_next);
    os.close();
    if (0 != system(string("..\\tools\\dot\\dot -Tpdf " + code_path + " -o " + pdf_path).c_str())) {
        if (verbose) {
            std::cout << "Unable to generate " + pdf_path + ", stopped\n";
        }
        return false;
    }
    if (verbose) {
        std::cout << "done\n";
    }
    remove(code_path.c_str());
    return true;
}

bool DrawForestOnFile(const string& output_file, Forest& f, bool verbose) 
{
    if (verbose) {
        std::cout << "Drawing Forest: " << output_file << ".. ";
    }
    string output_path_lowercase = output_file;
    std::transform(output_path_lowercase.begin(), output_path_lowercase.end(), output_path_lowercase.begin(), ::tolower);
    output_path_lowercase = global_output_path + output_path_lowercase;
    string code_path = output_path_lowercase + "_dotcode.txt";
    string pdf_path = output_path_lowercase + ".pdf";
    ofstream os(code_path);
    if (!os) {
        if (verbose) {
            std::cout << "Unable to generate " << code_path << ", stopped\n";
        }
        return false;
    }
    os <<   "digraph dag{\n"
            "\tranksep = 1.5;\n"
            "\tsubgraph tree{\n";

    nodeid id;
    std::map<ltree::node*, int> printed_node;
    std::vector<std::string> links;
    for (size_t i = 0; i < f.trees_.size(); ++i) {
        const auto& t = f.trees_[i];
        t.root->data.condition = "ROOT " + to_string(i) + ": " + t.root->data.condition;
        printed_node[t.root] = id.next();
        GenerateDotCodeForDagRec(os, t.root, printed_node, links, id, true, 2);
    }

    os << "\t}\n";
    for (size_t i = 0; i < links.size(); ++i) {
        os << links[i];
    }

    os << "}\n";

    os.close();
    if (0 != system(string("..\\tools\\dot\\dot -Tpdf " + code_path + " -o " + pdf_path).c_str())) {
        if (verbose) {
            std::cout << "Unable to generate " + pdf_path + ", stopped\n";
        }
        return false;
    }
    if (verbose) {
        std::cout << "done\n";
    }
    //remove(code_path.c_str());
    return true;
}
