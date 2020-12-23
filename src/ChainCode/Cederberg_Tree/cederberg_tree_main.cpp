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
    string algorithm_name = "Cederberg_Tree";
    string mask_name = "Cederberg";

    conf = ConfigData(algorithm_name, mask_name);

	ChainCodeRS cc_rs;
	auto rs = cc_rs.GetRuleSet();

    // Call GRAPHGEN:
    // 1) Load or generate Optimal Decision Tree based on Cederberg mask
    BinaryDrag<conact> bd = GetOdt(rs);

    // 2) Draw the generated tree to pdf
    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, bd);

    // 3) Generate the C++ source code for the ODT
    ofstream os(conf.treecode_path_);
    if (os) {
        GenerateDragCode(os, bd);
    }

    // 4) Generate the C++ source code for pointers,
    // conditions to check and actions to perform
    GeneratePointersConditionsActionsCode(rs,
		GenerateConditionActionCodeFlags::CONDITIONS_WITH_IFS | 
		GenerateConditionActionCodeFlags::ACTIONS_WITH_CONTINUE,
		GenerateActionCodeTypes::CHAIN_CODE);

    return EXIT_SUCCESS;
}