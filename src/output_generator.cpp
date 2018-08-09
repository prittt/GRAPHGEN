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

void GenerateDotCodeForDagRec(std::ostream& os, tree<conact>::node *n, std::map<tree<conact>::node*, int>& printed_node, std::vector<std::string>& links, nodeid &id, int tab) {
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
            GenerateDotCodeForDagRec(os, n->left, printed_node, links, id, tab);
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
            GenerateDotCodeForDagRec(os, n->right, printed_node, links, id, tab);
        }
        else {
            std::stringstream ss;
            ss << "node" << printed_node[n] << " -> node" << printed_node[n->right] << " [label=\"1\", style=dotted];\n";
            links.push_back(ss.str());
        }
    }
}

// All nodes must have both sons! 
void GenerateDotCodeForDag(std::ostream& os, tree<conact>& t) {
    os << "digraph dag{\n";
    os << "\tsubgraph tree{\n";

    std::map<tree<conact>::node*, int> printed_node = { { t.root, 0 } };;
    std::vector<std::string> links;
    GenerateDotCodeForDagRec(os, t.root, printed_node, links, nodeid(), 2);

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
bool DrawDagOnFile(const string& output_file, tree<conact> &t, bool verbose) {

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
    GenerateDotCodeForDag(os, t);
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
