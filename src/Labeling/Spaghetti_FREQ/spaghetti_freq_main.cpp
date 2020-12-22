// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "graphgen.h"

#include "grana_ruleset.h"

using namespace std;

string Description() {
    return "Spaghetti_FREQ (ODT with frequencies + prediction + compression).\n  \
            Frequencies are calculated over the following datasets: " +  
            conf.GetDatasetsString(", ") + "\n";
}

int main()
{
    string algo_name = "Spaghetti_FREQ";
    string mask_name = "Grana";
    conf = ConfigData(algo_name, mask_name, true);
    conf.SetDescription(Description());

    GranaRS g_rs;
    auto rs = g_rs.GetRuleSet();

    // Call GRAPHGEN:
    // 1) Count frequencies
    AddFrequenciesToRuleset(rs);

    // 2) Load or generate Optimal Decision Tree based on Grana mask
    BinaryDrag<conact> bd = GetOdt(rs);

    // 3) Draw the generated tree to pdf
    string tree_filename = algo_name + "_tree";
    DrawDagOnFile(tree_filename, bd);

    // 4) Generate forests of trees
    LOG(algo_name + " - making forests",
        ForestHandler fh(bd, rs.ps_, 
                         ForestHandlerFlags::CENTER_LINES |
                         ForestHandlerFlags::FIRST_LINE   |
                         ForestHandlerFlags::LAST_LINE    |
                         ForestHandlerFlags::SINGLE_LINE);
    );

    // 5) Draw the generated forests on file
    fh.DrawOnFile(algo_name, DrawDagFlags::DELETE_DOTCODE);

    // 6) Compress the forests
    fh.Compress(DragCompressorFlags::PRINT_STATUS_BAR | DragCompressorFlags::IGNORE_LEAVES, 10);

    // 7) Draw the compressed forests on file
    fh.DrawOnFile(algo_name, DrawDagFlags::DELETE_DOTCODE);

    // 8) Generate the C/C++ code taking care of the names used
    //    in the Grana's rule set GranaRS
    fh.GenerateCode(BeforeMainShiftTwo);
    pixel_set block_positions{
           { "P", {-2, -2} },{ "Q", {+0, -2} },{ "R", {+2, -2} },
           { "S", {-2, +0} },{ "x", {+0, +0} }
    };
    GeneratePointersConditionsActionsCode(rs, 
                                          GenerateConditionActionCodeFlags::NONE, 
                                          GenerateActionCodeTypes::LABELING,
                                          block_positions);

    return EXIT_SUCCESS;
}