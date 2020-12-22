// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "graphgen.h"

#include "guohall_ruleset.h"

using namespace std;

int main()
{
    string algo_name = "GH_Tree";
    string mask_name = "3x3";
    conf = ConfigData(algo_name, mask_name);

    GuoHallRS gh_rs;
    auto rs = gh_rs.GetRuleSet();

    // Call GRAPHGEN:
    // 1) Load or generate Optimal Decision Tree based on Guo-Hall algorithm
    BinaryDrag<conact> bd = GetOdt(rs);

    // 2) Draw the generated tree on file
    string tree_filename = algo_name + "_tree";
    DrawDagOnFile(tree_filename, bd);

    // 3) Generate the C/C++ source code
    GenerateDragCode(bd);
    GeneratePointersConditionsActionsCode(rs, 
                                          GenerateConditionActionCodeFlags::NONE, 
                                          GenerateActionCodeTypes::THINNING);

    return EXIT_SUCCESS;
}