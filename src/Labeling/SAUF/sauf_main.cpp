// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// This target generates the optimal decision tree for the Rosenfeld mask

#include "graphgen.h"

#include "rosenfeld_ruleset.h"

using namespace std;

int main()
{
    string algorithm_name = "SAUF";
    string mask_name = "Rosenfeld";

    conf = ConfigData(algorithm_name, mask_name);

    RosenfeldRS r_rs;
    auto rs = r_rs.GetRuleSet();

    // Call GRAPHGEN:
    // 1) Load or generate Optimal Decision Tree based on Rosenfeld mask
    BinaryDrag<conact> bd = GetOdt(rs);

    // 2) Draw the generated tree to pdf
    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, bd);

    // 3) Generate the C++ source code for the ODT
    ofstream os(conf.treecode_path_);
    if (os){
        GenerateDragCode(os, bd);
    }

    // 4) Generate the C++ source code for pointers,
    // conditions to check and actions to perform
    GeneratePointersConditionsActionsCode(rs, GenerateConditionActionCodeFlags::CONDITIONS_WITH_IFS | GenerateConditionActionCodeFlags::ACTIONS_WITH_CONTINUE);

    return EXIT_SUCCESS;
}