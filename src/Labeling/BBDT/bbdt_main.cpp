// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "graphgen.h"

#include "grana_ruleset.h"

using namespace std;

int main()
{
    string algorithm_name = "BBDT";
    conf = ConfigData(algorithm_name, "Grana");

    GranaRS g_rs;
    auto rs = g_rs.GetRuleSet();

    // Call GRAPHGEN:
    // 1) Load or generate Optimal Decision Tree based on Grana mask
    BinaryDrag<conact> bd = GetOdt(rs);

    // 2) Draw the generated tree
    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, bd);

    // 3) Generate the tree C/C++ code taking care of the names used
    //    in the Grana's rule set GranaRS
    GenerateDragCode(bd);
    pixel_set block_positions{
           { "P", {-2, -2} },{ "Q", {+0, -2} },{ "R", {+2, -2} },
           { "S", {-2, +0} },{ "x", {+0, +0} }
    };
    GeneratePointersConditionsActionsCode(rs, 
                                          GenerateConditionActionCodeFlags::CONDITIONS_WITH_IFS | GenerateConditionActionCodeFlags::ACTIONS_WITH_CONTINUE, 
                                          GenerateActionCodeTypes::LABELING,
                                          block_positions);

    return EXIT_SUCCESS;
}