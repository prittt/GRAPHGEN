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

#include "yaml-cpp/yaml.h"

#include "graphsgen.h"

#include "rosenfeld_ruleset.h"

using namespace std;

int main()
{
    string algorithm_name = "PRED";
    conf = ConfigData(algorithm_name);

    RosenfeldRS r_rs;
    auto rs = r_rs.GetRuleSet();

    // Call GRAPHSGEN:
    // 1) Load or generate Optimal Decision Tree based on Rosenfeld mask
    ltree t = GetOdt(rs, algorithm_name);
    
    // 2) Draw the generated tree to pdf
    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, t, false, true);
    
    // 3) Generate forests of tree (one for the first row and one for all
    //    the other rows)
    LOG(algorithm_name + ": making main forest",
        Forest f(t, rs.ps_);
    );

    // 4) Draw the trees composing the forests into pdf. A single forest 
    //    contains trees for all the columns (including the start tree for
    //    the first column) and special trees for the last column
    for (size_t i = 0; i < f.trees_.size(); ++i) {
        DrawDagOnFile(algorithm_name + "tree" + zerostr(i, 4), f.trees_[i], true);
    }
    for (size_t i = 0; i < f.end_trees_.size(); ++i) {
        for (size_t j = 0; j < f.end_trees_[i].size(); ++j) {
            DrawDagOnFile(algorithm_name + "_end_tree_" + zerostr(i, 2) + "_" + zerostr(j, 2), f.end_trees_[i][j], false);
        }
    }

    // Optionally, print statistics about nodes and leaves of the generated
    // forests.
    PrintStats(f);

    DrawForestOnFile(algorithm_name + "forest", f, true);

    filesystem::path forest_code_nodag = conf.forestcode_path_;
    {
        ofstream os(forest_code_nodag);
        GenerateForestCode(os, f);
    }

    // TODO: Update GenerateConditionsActionsCode to generate also conditions without ifs
    GeneratePointersConditionsActionsCode(algorithm_name, rs);

    // Create first line constraints
    constraints first_line_constr;
    using namespace std;
    for (const auto& p : rs.ps_) {
        if (p.GetDy() < 0)
            first_line_constr[p.name_] = 0;
    }

    Forest flf(t, rs.ps_, first_line_constr);
    DrawForestOnFile(algorithm_name + "_first_line_original", flf, true, true);

    string first_line_forest_reduced_code = conf.global_output_path_.string() + "/" + algorithm_name + "_first_line_forest_reduced_code.txt";
    {
        ofstream os(first_line_forest_reduced_code);
        GenerateForestCode(os, flf);
    }

	return EXIT_SUCCESS;
}