// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "graphgen.h"

#include "rosenfeld3d_ruleset.h"

using namespace std;

int main()
{
    // Setup configuration
    string algorithm_name = "PRED++3D";
    conf = ConfigData(algorithm_name, "Rosenfeld");

    // Load or generate rules
    Rosenfeld3dRS r_rs;
    auto rs = r_rs.GetRuleSet();

    // Call GRAPHGEN:
    // 1) Load or generate Optimal Decision Tree based on Rosenfeld mask
    BinaryDrag<conact> bd = GetOdt(rs);
    
    // 2) Draw the generated tree on file
    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, bd);
    
    // 3) Generate forests of trees
    LOG(algorithm_name + " - making forests",
        ForestHandler fh(bd, rs.ps_, ForestHandlerFlags::FIRST_LINE |
			ForestHandlerFlags::LAST_LINE |
			ForestHandlerFlags::SINGLE_LINE |
			ForestHandlerFlags::CENTER_LINES);
    );
    
    // 4) Compress the forest
    fh.Compress(DragCompressorFlags::PRINT_STATUS_BAR | DragCompressorFlags::IGNORE_LEAVES);

    // 5) Draw the compressed forests on file
    fh.DrawOnFile(algorithm_name, DrawDagFlags::DELETE_DOTCODE);
    
    // 6) Generate the C/C++ source code
    fh.GenerateCode();
    GeneratePointersConditionsActionsCode(rs, GenerateConditionActionCodeFlags::NONE);

	return EXIT_SUCCESS;
}