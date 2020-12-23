// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "graphgen.h"

#include "zangsuen_ruleset.h"

#include "drag.h"

using namespace std;

int main()
{
    string algo_name = "ZS_Tree";
    string mask_name = "3x3";
    conf = ConfigData(algo_name, mask_name);

    ZangSuenRS zs_rs;
    auto rs = zs_rs.GetRuleSet();

    // Call GRAPHGEN:
    // 1) Load or generate Optimal Decision Tree based on Zang-Suen algorithm
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