// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "chaincode_ruleset.h"

#include "graphgen.h"

using namespace std;

int main()
{
    string algo_name = "Cederberg_Spaghetti_FREQ";
    string mask_name = "Cederberg";

    conf = ConfigData(algo_name, mask_name, true);

	ChainCodeRS cc_rs;
	auto rs = cc_rs.GetRuleSet();

    // Call GRAPHGEN:
    // 1) Count frequencies
    AddFrequenciesToRuleset(rs);

    // 2) Load or generate Optimal Decision Tree based on Cederberg mask
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
    fh.Compress(DragCompressorFlags::PRINT_STATUS_BAR | DragCompressorFlags::IGNORE_LEAVES);

    // 7) Draw the compressed forests on file
    fh.DrawOnFile(algo_name, DrawDagFlags::DELETE_DOTCODE);

    // 8) Generate the C/C++ code
    fh.GenerateCode();
	GeneratePointersConditionsActionsCode(rs,
		GenerateConditionActionCodeFlags::NONE,
		GenerateActionCodeTypes::CHAIN_CODE);

    return EXIT_SUCCESS;
}