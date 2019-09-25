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

#include "grana_ruleset.h"

using namespace std;

int main()
{
    string algorithm_name = "Tagliatelle";
    conf = ConfigData(algorithm_name);

    auto rs = GenerateGrana();

    // Call GRAPHSGEN:
    // 1) Load or generate Optimal Decision Tree based on Grana mask
    ltree t = GetOdt(rs, algorithm_name);

    // 2) Draw the generated tree to pdf
    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, t, false, true, false);

    // Optional) Display tree statistics
    // PrintStats(t);

    // 3) Generate forests of tree (one for the first row and one for all
    //    the other rows)
    LOG(algorithm_name + ": making main forest",
        Forest f(t, rs.ps_);
    );

    // 3) Generate the C++ source code for the ODT
    GenerateDragCode(algorithm_name, t);

    // 4) Generate the C++ source code for pointers, 
    // conditions to check and actions to perform
    pixel_set block_positions{
          { "P", {-2, -2} },{ "Q", {+0, -2} },{ "R", {+2, -2} },
          { "S", {-2, +0} },{ "x", {+0, +0} }
    };
    GeneratePointersConditionsActionsCode(algorithm_name, rs, block_positions);


    //void CreateSpaghettiLabeling() {

    //    PerformPseudoOptimalDragGeneration(t, rs.ps_, algorithm_name, fs::path("Spaghetti_BBDT"));

    //    return;

    //    string tree_code_filename = global_output_path + algorithm_name + "_code.txt";
    //    GenerateCode(tree_code_filename, t);

    //    //PerformOptimalDragGeneration(t, algorithm_name);

    //    LOG("Making forest",
    //        Forest f(t, rs.ps_);
    //    for (size_t i = 0; i < f.trees_.size(); ++i) {
    //        DrawDagOnFile(algorithm_name + "tree" + zerostr(i, 4), f.trees_[i], true);
    //    }
    //    );
    //    PrintStats(f);

    //    string forest_code_nodag = global_output_path + algorithm_name + "_forest_nodag_code.txt";
    //    {
    //        ofstream os(forest_code_nodag);
    //        GenerateForestCode(os, f);
    //    }

    //    for (size_t i = 0; i < f.end_trees_.size(); ++i) {
    //        for (size_t j = 0; j < f.end_trees_[i].size(); ++j) {
    //            DrawDagOnFile(algorithm_name + "_end_tree_" + zerostr(i, 2) + "_" + zerostr(j, 2), f.end_trees_[i][j], false);
    //        }
    //    }

    //    LOG("Converting forest to dag",
    //        Forest2Dag x(f);
    //    for (size_t i = 0; i < f.trees_.size(); ++i) {
    //        DrawDagOnFile(algorithm_name + "drag" + zerostr(i, 4), f.trees_[i], true);
    //    }
    //    );
    //    PrintStats(f);

    //    DrawForestOnFile(algorithm_name + "forest", f, true);

    //    string forest_code = global_output_path + algorithm_name + "_forest_identities_code.txt";
    //    {
    //        ofstream os(forest_code);
    //        GenerateForestCode(os, f);
    //    }

    //    LOG("Reducing forest",
    //        STree st(f);
    //    );
    //    PrintStats(f);

    //    int last_nodeid_main_forest_group;
    //    string forest_reduced_code = global_output_path + algorithm_name + "_forest_reduced_code.txt";
    //    {
    //        ofstream os(forest_reduced_code);
    //        last_nodeid_main_forest_group = GenerateForestCode(os, f);
    //    }
    //    //DrawForestOnFile(algorithm_name + "_forest_reduced", f, true, true);

    //    // Create first line constraints
    //    constraints first_line_constr;
    //    using namespace std;
    //    for (const auto& p : rs.ps_) {
    //        if (p.GetDy() < 0)
    //            first_line_constr[p.name_] = 0;
    //    }

    //    Forest flf(t, rs.ps_, first_line_constr);
    //    DrawForestOnFile(algorithm_name + "_first_line_original", flf, true, true);

    //    LOG("Converting first line forest to dag",
    //        Forest2Dag y(flf);
    //    );
    //    PrintStats(flf);

    //    LOG("Reducing first line forest",
    //        STree st_fl(flf);
    //    );
    //    PrintStats(flf);

    //    DrawForestOnFile(algorithm_name + "_first_line_reduced", flf, true, true);

    //    string first_line_forest_reduced_code = global_output_path + algorithm_name + "_first_line_forest_reduced_code.txt";
    //    {
    //        ofstream os(first_line_forest_reduced_code);
    //        last_nodeid_main_forest_group = GenerateForestCode(os, flf, last_nodeid_main_forest_group);
    //    }

    //    // Create last line constraints
    //    constraints last_line_constr;
    //    using namespace std;
    //    for (const auto& p : rs.ps_) {
    //        if (p.GetDy() > 0)
    //            last_line_constr[p.name_] = 0;
    //    }

    //    Forest llf(t, rs.ps_, last_line_constr);
    //    DrawForestOnFile(algorithm_name + "_last_line_original", llf, true, true);

    //    /*LOG("Converting first line forest to dag",
    //    Forest2Dag z(llf);
    //    );
    //    PrintStats(llf);

    //    LOG("Reducing last line forest",
    //    STree st_ll(llf);
    //    );
    //    PrintStats(llf);*/

    //    DrawForestOnFile(algorithm_name + "_last_line_reduced", llf, true, true);

    //    string last_line_forest_reduced_code = global_output_path + algorithm_name + "_last_line_forest_reduced_code.txt";
    //    {
    //        ofstream os(last_line_forest_reduced_code);
    //        last_nodeid_main_forest_group = GenerateForestCode(os, llf, last_nodeid_main_forest_group);
    //    }

    //    // Create last line constraints
    //    constraints single_line_constr;
    //    using namespace std;
    //    for (const auto& p : rs.ps_) {
    //        if (p.GetDy() != 0)
    //            single_line_constr[p.name_] = 0;
    //    }

    //    Forest slf(t, rs.ps_, single_line_constr);
    //    DrawForestOnFile(algorithm_name + "_single_line_original", slf, true, true);

    //    /*LOG("Converting single line forest to dag",
    //    Forest2Dag w(slf);
    //    );
    //    PrintStats(slf);

    //    LOG("Reducing single line forest",
    //    STree st_sl(slf);
    //    );
    //    PrintStats(slf);*/

    //    DrawForestOnFile(algorithm_name + "_single_line_reduced", slf, true, true);

    //    string single_line_forest_reduced_code = global_output_path + algorithm_name + "_single_line_forest_reduced_code.txt";
    //    {
    //        ofstream os(single_line_forest_reduced_code);
    //        GenerateForestCode(os, slf, last_nodeid_main_forest_group);
    //    }
    //}

    return EXIT_SUCCESS;
}