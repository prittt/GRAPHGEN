// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "graphgen.h"

#include "zangsuen_ruleset.h"

using namespace std;

int main()
{
    string algo_name = "ZS_Spaghetti_FREQ";
    string mask_name = "3x3";
    conf = ConfigData(algo_name, mask_name, true);

    ZangSuenRS zs_rs;
    auto rs = zs_rs.GetRuleSet();

    // Call GRAPHGEN:
    // 1) Count frequencies
	AddFrequenciesToRuleset(rs, false, true);

    // 2) Load or generate Optimal Decision Tree based on Zang-Suen algorithm
    BinaryDrag<conact> bd = GetOdt(rs);

    // 3) Draw the generated tree on file
    string tree_filename = conf.algorithm_name_ + "_tree";
    DrawDagOnFile(tree_filename, bd);

    // 4) Generate forests of trees
    LOG(algo_name + " - making forests",
        ForestHandler fh(bd, rs.ps_, 
                         ForestHandlerFlags::FIRST_LINE  |
                         ForestHandlerFlags::LAST_LINE   |
                         ForestHandlerFlags::SINGLE_LINE |
                         ForestHandlerFlags::CENTER_LINES);
    );

    // 5) Compress the forest
    fh.Compress();

    // 6) Draw the compressed forests on file
    fh.DrawOnFile(algo_name, DrawDagFlags::DELETE_DOTCODE);

    // 7) Generate the C/C++ source code
    fh.GenerateCode();
    GeneratePointersConditionsActionsCode(rs, 
                                          GenerateConditionActionCodeFlags::NONE, 
                                          GenerateActionCodeTypes::THINNING);

    return EXIT_SUCCESS;
}