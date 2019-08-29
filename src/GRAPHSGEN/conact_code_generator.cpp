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

#include "conact_code_generator.h"

#include <map>


#include "utilities.h"

using namespace std;

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

string CreateAssignmentCodeRec(const std::vector<std::string>& pixels_names, const pixel_set& names) {
    if (pixels_names.size() == 1) {
        pixel p = names[pixels_names.front()];
        return GenerateAccessPixelCode("img_labels_", p);
    }

    std::vector<std::string> pixels_names_seta(pixels_names.size() / 2), pixels_names_setb(pixels_names.size() - (pixels_names.size() / 2));

    std::copy_n(pixels_names.begin(), pixels_names_seta.size(), pixels_names_seta.begin());
    std::copy_n(pixels_names.begin() + pixels_names_seta.size(), pixels_names_setb.size(), pixels_names_setb.begin());

    return "LabelsSolver::Merge(" + CreateAssignmentCodeRec(pixels_names_setb, names) + ", " + CreateAssignmentCodeRec(pixels_names_seta, names) + ")";
}

string CreateAssignmentCode(const string& action, const pixel_set& names) {
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

    return CreateAssignmentCodeRec(pixels_names, names);
}

// Scritta per ctbe
string CreateActionCodeCtbe(const string& action, const pixel_set& names, const string& assignment_variable) {
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

    return assignment_variable + " " + CreateAssignmentCodeRec(pixels_names, names);
}

void GeneratePointersCode(ofstream& os, const rule_set& rs) {
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
    os << global_ss.str();
}

void GenerateConditionsCode(ofstream& os, const rule_set& rs) 
{
    auto& shifts = rs.ps_.shifts_; // Shifts on each dim -> [x, y] or [x, y, z]
    unsigned n_dims = shifts.size(); // Here we get how many dims image has
    // Conditions:
    os << "//Conditions:\n";

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
}


// This function .. it works only for 2d and 3d images
void GenerateActionsCode(ofstream& os, const rule_set& rs, const pixel_set& names) 
{
    auto& shifts = rs.ps_.shifts_; // Shifts on each dim -> [x, y] or [x, y, z]
    unsigned n_dims = shifts.size(); // Here we get how many dims image has

    // Actions:
    os << "\n\n//Actions:\n";
    for (size_t a = 0; a < rs.actions.size(); ++a) {

        string cur_action = rs.actions[a];

        os << "// Action " << a + 1 << ": " << cur_action << "\n";
        os << "#define ACTION_" << a + 1 << " ";

        string where_to_write = "img_labels_" + string(n_dims > 2 ? "slice00_" : "") + "row00[c] = ";

        os << where_to_write << CreateAssignmentCode(cur_action, names) << "; continue; \n";
    }
}

// Overloading function
bool GeneratePointersConditionsActionsCode(const string& algorithm_name, const rule_set& rs, std::optional<pixel_set> names) {
    filesystem::path code_path = global_output_path / filesystem::path(algorithm_name + "_code.cpp");

    ofstream os(code_path);
    if (!os) {
        return false;
    }

    if (!names) {
        names = rs.ps_;
    }

    GeneratePointersCode(os, rs);
    GenerateConditionsCode(os, rs);
    GenerateActionsCode(os, rs, names.value());

    return true;
}

//bool GenerateActionsForCtbe(const string& filename, const rule_set& rs) {
//    ofstream os(filename);
//
//    if (!os) {
//        return false;
//    }
//
//    // Actions:
//    os << "\n\n//Actions:\n";
//    os << "#define NOTHING \n";
//    for (size_t a = 0; a < rs.actions.size(); ++a) {
//
//        string cur_action = rs.actions[a];
//
//        os << "// Action " << a + 1 << ": " << cur_action << "\n";
//        os << "#define ACTION_" << a + 1 << " ";
//
//        vector<string> pixels_actions;
//        StringSplit(cur_action, pixels_actions, ',');
//
//        if (pixels_actions[1].substr(3) == "e" && (pixels_actions[2].substr(3) == "e" || pixels_actions[2].substr(3) == "g")) {
//            os << CreateActionCodeCtbe(pixels_actions[0], rs, "img_labels_row00[c] = img_labels_row01[c] = img_labels_row02[c] = ") << "; continue; \n";
//        }
//        else {
//            if (pixels_actions[1].substr(3) == "e") {
//                os << CreateActionCodeCtbe(pixels_actions[0], rs, "img_labels_row00[c] = img_labels_row01[c] = ") << "; ";
//                os << CreateActionCodeCtbe(pixels_actions[2], rs, "img_labels_row02[c] = ") << "; continue;\n";
//            }
//            else {
//                if (pixels_actions[2].substr(3) == "e") {
//                    os << CreateActionCodeCtbe(pixels_actions[0], rs, "img_labels_row00[c] = img_labels_row02[c] = ") << "; ";
//                    os << CreateActionCodeCtbe(pixels_actions[1], rs, "img_labels_row01[c] = ") << "; continue;\n";
//                }
//                else {
//                    // Il pixel "e" ha azione unica
//
//                    os << CreateActionCodeCtbe(pixels_actions[0], rs, "img_labels_row00[c] = ") << "; ";
//                    if (pixels_actions[2].substr(3) == "e") {
//                        os << CreateActionCodeCtbe(pixels_actions[1], rs, "img_labels_row01[c] = img_labels_row02[c] = ") << "; continue;\n";
//                    }
//                    else {
//                        // Ogni pixel ha la sua azione
//                        os << CreateActionCodeCtbe(pixels_actions[1], rs, "img_labels_row01[c] = ") << "; ";
//                        os << CreateActionCodeCtbe(pixels_actions[2], rs, "img_labels_row02[c] = ") << "; continue;\n";
//                    }
//                }
//            }
//        }
//    }
//
//    return true;
//}
