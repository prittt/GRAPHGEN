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

// This function generates forest code using numerical labels starting from start_id and returns the last used_id
int GenerateForestCode(std::ostream& os, const Forest& f, int start_id) {

	std::map<ltree::node*, int> printed_node;
	std::map<ltree::node*, pair<int, bool>> nodes_requiring_labels;
	nodeid id;
	for (size_t i = 0; i < f.trees_.size(); ++i) {
		const auto& t = f.trees_[i];
		// printed_node[t.root] = id.next();
		CheckNodesTraversalRec(t.root, nodes_requiring_labels, id);
	}

	// Initial and internal trees (tree 0 is always the start tree)
	//for (size_t i = 0; i < f.trees_.size(); ++i) {
	//	const auto& t = f.trees_[i];
	//	os << "tree_" << i << ": " << "finish_condition(" << i << "); \n"; // "finish_condition(n)" sarà da sostituire con "if (++c >= COLS - 1) goto break_n" quando inseriremo gli alberi di fine riga;
	//	GenerateCodeRec(os, t.root, printed_node, nodes_requiring_labels, id2, 2, true);
	//}

	//#define finish_condition(n) if ((c+=2) >= w - 2) { if (c > w - 2) { goto break_0_##n; } else { goto break_1_##n; } }
	// TODO questa versione è specifica per BBDT, bisogna trovare un modo per generalizzare rispetto allo shift della maschera!!
	id.Clear();
	id.SetId(start_id);
	for (size_t i = 0; i < f.trees_.size(); ++i) {
		const auto& t = f.trees_[i];
		// BBDT os << "tree_" << i << ": if ((c+=2) >= w - 2) { if (c > w - 2) { goto break_0_" << f.main_trees_end_trees_mapping_[0][i] << "; } else { goto break_1_" << f.main_trees_end_trees_mapping_[1][i] << "; } } \n";
        /* Thinning */ os << "tree_" << i << ": if ((c+=1) >= w - 1) goto break_0_" << f.main_trees_end_trees_mapping_[0][i] << ";\n";
		GenerateCodeRec(os, t.root, printed_node, nodes_requiring_labels, id, 2, true);
	}
	int last_id_main_forest_node = id.get();

	// Final trees
	std::map<ltree::node*, int> printed_node_end_trees;
	std::map<ltree::node*, pair<int, bool>> nodes_requiring_labels_end_trees;
	id.Clear();
	for (size_t tg = 0; tg < f.end_trees_.size(); ++tg) {
		const auto& cur_trees = f.end_trees_[tg];
		for (size_t i = 0; i < cur_trees.size(); ++i) {
			const auto& t = cur_trees[i];
			// printed_node_end_trees[t.root] = id.next();
			CheckNodesTraversalRec(t.root, nodes_requiring_labels_end_trees, id);
		}
	}

	id.SetId(last_id_main_forest_node);
	for (size_t tg = 0; tg < f.end_trees_.size(); ++tg) {
		const auto& cur_trees = f.end_trees_[tg];
		for (size_t i = 0; i < cur_trees.size(); ++i) {
			//os << "break_" << zerostr(tg, 2) << "_" << zerostr(i, 2) << ":\n";
			os << "break_" << tg << "_" << i << ":\n";
			GenerateCodeRec(os, cur_trees[i].root, printed_node_end_trees, nodes_requiring_labels_end_trees, id, 2, false);
			os << "\tcontinue;\n";
		}
	}

	return id.get();
}

// Generate string to access a pixel value using pointers
string GenerateAccessPixelCode(const string& img_name, const pixel& p) {
	string slice_id = "";
	if (p.coords_.size() > 2) {
		slice_id = "slice" + string(p.coords_[2] < 0 ? "1" : "0") + to_string(abs(p.coords_[2])) + "_";
	}

	string row_id = "row" + string(p.coords_[1] < 0 ? "1" : "0") + to_string(abs(p.coords_[1]));
	string col = "";
	if (p.coords_[0] > 0) {
		col += " + " + to_string(abs(p.coords_[0]));
	}
	else if (p.coords_[0] < 0) {
		col += " - " + to_string(abs(p.coords_[0]));
	}

	return img_name + slice_id + row_id + "[c" + col + "]";
}

string CreateAssignmentCodeRec(const std::vector<std::string>& pixels_names, const rule_set& rs) {
	if (pixels_names.size() == 1) {
		pixel p = rs.ps_[pixels_names.front()];
		return GenerateAccessPixelCode("img_labels_", p);
	}

	std::vector<std::string> pixels_names_seta(pixels_names.size() / 2), pixels_names_setb(pixels_names.size() - (pixels_names.size() / 2));

	std::copy_n(pixels_names.begin(), pixels_names_seta.size(), pixels_names_seta.begin());
	std::copy_n(pixels_names.begin() + pixels_names_seta.size(), pixels_names_setb.size(), pixels_names_setb.begin());

	return "LabelsSolver::Merge(" + CreateAssignmentCodeRec(pixels_names_setb, rs) + ", " + CreateAssignmentCodeRec(pixels_names_seta, rs) + ")";
}

string CreateAssignmentCode(const string& action, const rule_set& rs) {
	if (action == "nothing") {
		return "0";
	}

	string action_ = action.substr(3);
	if (action_ == "newlabel") {
		return "LabelsSolver::NewLabel()";
	}

	std::vector<std::string> pixels_names;
	StringSplit(action_, pixels_names);

	assert(pixels_names.size() > 0 && "Something wrong with actions");

	return CreateAssignmentCodeRec(pixels_names, rs);
}

// Scritta per ctbe
string CreateActionCodeCtbe(const string& action, const rule_set& rs, const string& assignment_variable) {
    if (action == "nothing") {
        return "NOTHING";
    }

    string action_ = action.substr(3);
    if (action_ == "newlabel") {
        return assignment_variable + " LabelsSolver::NewLabel()";
    }

    std::vector<std::string> pixels_names;
    StringSplit(action_, pixels_names);

    assert(pixels_names.size() > 0 && "Something wrong with actions");

    return assignment_variable + " " + CreateAssignmentCodeRec(pixels_names, rs);
}

// This function .. it works only for 2d and 3d images
void GenerateConditionsActionsCode(ofstream& os, const rule_set& rs) {
	// The names of pointers are identified by the following string
	//	<image_name>_<slice_identifier>_<row_identifier>
	//
	// slice identifiers can be (first number is the sign):
	//	- 'slice00' for the current slice (z)
	//	- 'slice11' for the slice z - 1
	//	- 'slice12' for the slice z - 2
	//  - 'slice01' for the slice z + 1
	//  - 'slice02' for the slice z + 2
	//  - .. and so on
	//
	// row identifiers can be (first number is the sign):
	//	- 'row00' for the current row (y)
	//	- 'row11' for the row y - 1
	//	- 'row12' for the row y - 2
	//  - 'row01' for the row y + 1
	//  - 'row02' for the row y + 2
	//  - .. and so on

	// Pointers: 
	os << "//Pointers:\n";

	auto& shifts = rs.ps_.shifts_; // Shifts on each dim -> [x, y] or [x, y, z]
	unsigned n_dims = shifts.size(); // Here we get how many dims image has

	stringstream global_ss, in_ss, out_ss;
	string type_in_prefix_string = "const unsigned char* const ";
	string type_out_prefix_string = "unsigned* const ";

	// TODO: 3D generation only works with unitary shift
	// x is always ignored because we always create row pointers
	switch (n_dims) {
	case 2:
	{
		string base_row_in_name = "img_row00";
		string base_row_out_name = "img_labels_row00";
		string base_row_in = type_in_prefix_string + base_row_in_name + " = img_.ptr<unsigned char>(r);";
		string base_row_out = type_out_prefix_string + base_row_out_name + " = img_labels_.ptr<unsigned>(r);";

		in_ss << "// Row pointers for the input image \n";
		in_ss << base_row_in + "\n";

		out_ss << "// Row pointers for the output image \n";
		out_ss << base_row_out + "\n";

		for (int j = -shifts[1]; j < shifts[1]; ++j) { // TODO: should use min and max y in mask

			if (j == 0) {
				continue;
			}

			string complete_string_in =
				type_in_prefix_string +
				"img_row" + to_string(j < 0) + to_string(abs(j)) +
				" = (unsigned char *)(((char *)" + base_row_in_name + ") + img_.step.p[0] * " + to_string(j) + ");";
			in_ss << complete_string_in + "\n";


			string complete_string_out =
				type_out_prefix_string +
				"img_labels_row" + to_string(j < 0) + to_string(abs(j)) +
				" = (unsigned *)(((char *)" + base_row_out_name + ") + img_labels_.step.p[0] * " + to_string(j) + ");";
			out_ss << complete_string_out + "\n";

		}
		break;
	}
	case 3:
	{
		// TODO: this generation only works with unitary shift

		// Current slice
		string base_row_in_name = "img_slice00_row00";
		string base_row_out_name = "img_labels_slice00_row00";
		string base_row_in = type_in_prefix_string + base_row_in_name + " = img_.ptr<unsigned char>(s, r);";
		string base_row_out = type_out_prefix_string + base_row_out_name + " = img_labels_.ptr<unsigned>(s, r);";

		in_ss << "// Row pointers for the input image (current slice) \n";
		in_ss << base_row_in + "\n";

		out_ss << "// Row pointers for the output image (current slice)\n";
		out_ss << base_row_out + "\n";

		for (int j = -shifts[1]; j < shifts[1]; ++j) {

			if (j == 0) {
				continue;
			}

			string complete_string_in =
				type_in_prefix_string +
				"img_slice00_row" + to_string(j < 0) + to_string(abs(j)) +
				" = (unsigned char *)(((char *)" + base_row_in_name + ") + img_.step.p[1] * " + to_string(j) + ");";
			in_ss << complete_string_in + "\n";

			string complete_string_out =
				type_out_prefix_string +
				"img_labels_slice00_row" + to_string(j < 0) + to_string(abs(j)) +
				" = (unsigned *)(((char *)" + base_row_out_name + ") + img_labels_.step.p[1] * " + to_string(j) + ");";
			out_ss << complete_string_out + "\n";

		}

		in_ss << "\n// Row pointers for the input image (previous slice) \n";
		out_ss << "\n// Row pointers for the output image (previous slice)\n";

		// Previous slice
		base_row_in = type_in_prefix_string + base_row_in_name + " = img_.ptr<unsigned char>(s, r);";
		base_row_out = type_out_prefix_string + base_row_out_name + " = img_labels_.ptr<unsigned>(s, r);";

		for (int j = -shifts[1]; j <= shifts[1]; ++j) {

			string complete_string_in =
				type_in_prefix_string +
				"img_slice11_row" + to_string(j < 0) + to_string(abs(j)) +
				" = (unsigned char *)(((char *)" + base_row_in_name + ") - img_.step.p[0] + img_.step.p[1] * " + to_string(j) + ");";
			in_ss << complete_string_in + "\n";

			string complete_string_out =
				type_out_prefix_string +
				"img_labels_slice11_row" + to_string(j < 0) + to_string(abs(j)) +
				" = (unsigned *)(((char *)" + base_row_out_name + ") - img_labels_.step.p[0] + img_labels_.step.p[1] * " + to_string(j) + ");";
			out_ss << complete_string_out + "\n";

		}
	}
	break;
	}
	global_ss << in_ss.str() + "\n" + out_ss.str();

	// Conditions:
	os << global_ss.str() << "\n\n//Conditions:\n";

	vector<string> counters_names = { "c", "r", "s" };
	vector<string> sizes_names = { "w", "h", "d" };
	for (const auto& p : rs.ps_) {
		string uppercase_name(p.name_);
		transform(p.name_.begin(), p.name_.end(), uppercase_name.begin(), ::toupper);
		os << "#define CONDITION_" + uppercase_name + " ";
		stringstream col;
		for (size_t i = 0; i < n_dims; ++i) {
			if (p.coords_[i] < 0) {
				os << counters_names[i] << " > " << -p.coords_[i] - 1 << " && ";
			}
			else if (p.coords_[i] > 0) {
				os << counters_names[i] << " < " << sizes_names[i] << " - " << p.coords_[i] << " && ";
			}
		}

		os << GenerateAccessPixelCode("img_", p) << " > 0\n";
	}

	// Actions:
	os << "\n\n//Actions:\n";
	for (size_t a = 0; a < rs.actions.size(); ++a) {

		string cur_action = rs.actions[a];

		os << "// Action " << a + 1 << ": " << cur_action << "\n";
		os << "#define ACTION_" << a + 1 << " ";

		string where_to_write = "img_labels_" + string(n_dims > 2 ? "slice00_" : "") + "row00[c] = ";

		os << where_to_write << CreateAssignmentCode(cur_action, rs) << "; continue; \n";
	}
}

// Overloading function
bool GenerateConditionsActionsCode(const string& filename, const rule_set& rs) {
	ofstream os(filename);

	if (!os) {
		return false;
	}

	GenerateConditionsActionsCode(os, rs);

	return true;
}

bool GenerateActionsForCtbe(const string& filename, const rule_set& rs) {
    ofstream os(filename);

    if (!os) {
        return false;
    }
    
    // Actions:
    os << "\n\n//Actions:\n";
    os << "#define NOTHING \n";
    for (size_t a = 0; a < rs.actions.size(); ++a) {

        string cur_action = rs.actions[a];

        os << "// Action " << a + 1 << ": " << cur_action << "\n";
        os << "#define ACTION_" << a + 1 << " ";
        
        vector<string> pixels_actions;
        StringSplit(cur_action, pixels_actions,',');

        if (pixels_actions[1].substr(3) == "e" && (pixels_actions[2].substr(3) == "e" || pixels_actions[2].substr(3) == "g")) {
            os << CreateActionCodeCtbe(pixels_actions[0], rs, "img_labels_row00[c] = img_labels_row01[c] = img_labels_row02[c] = ") << "; continue; \n";
        }
        else {
            if (pixels_actions[1].substr(3) == "e") {
                os << CreateActionCodeCtbe(pixels_actions[0], rs, "img_labels_row00[c] = img_labels_row01[c] = ") << "; ";
                os << CreateActionCodeCtbe(pixels_actions[2], rs, "img_labels_row02[c] = ") << "; continue;\n";
            }
            else {
                if (pixels_actions[2].substr(3) == "e") {
                    os << CreateActionCodeCtbe(pixels_actions[0], rs, "img_labels_row00[c] = img_labels_row02[c] = ") << "; ";
                    os << CreateActionCodeCtbe(pixels_actions[1], rs, "img_labels_row01[c] = ") << "; continue;\n";
                }
                else {
                    // Il pixel "e" ha azione unica
                    
                    os << CreateActionCodeCtbe(pixels_actions[0], rs, "img_labels_row00[c] = ") << "; ";
                    if (pixels_actions[2].substr(3) == "e") {
                        os << CreateActionCodeCtbe(pixels_actions[1], rs, "img_labels_row01[c] = img_labels_row02[c] = ") << "; continue;\n";
                    }
                    else {
                        // Ogni pixel ha la sua azione
                        os << CreateActionCodeCtbe(pixels_actions[1], rs, "img_labels_row01[c] = ") << "; ";
                        os << CreateActionCodeCtbe(pixels_actions[2], rs, "img_labels_row02[c] = ") << "; continue;\n";
                    }
                }
            }
        }
    }

    return true;
}
