// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "graphgen.h"

#include "dilation_ruleset.h"

using namespace std;

int main()
{
    string algorithm_name = "Dilation3x3_Spaghetti_FREQ";
    string mask_name = "kernel3x3";

    conf = ConfigData(algorithm_name, mask_name, true);

    DilationRS d_rs;
    auto rs = d_rs.GetRuleSet();

    // Call GRAPHGEN:
    // 1) Count frequencies
    AddFrequenciesToRuleset(rs);

    // 2) Load or generate Optimal Decision Tree based on Erosion 3x3 mask
    BinaryDrag<conact> bd = GetOdt(rs);

    // 3) Draw the generated tree to pdf
    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, bd);

    ofstream os(conf.treecode_path_);
    if (os) {
        GenerateDragCode(os, bd);
    }

    // 4) Generate forests of trees
    LOG(algorithm_name + " - making forests",
        ForestHandler fh(bd, rs.ps_,
            ForestHandlerFlags::CENTER_LINES |
            ForestHandlerFlags::FIRST_LINE |
            ForestHandlerFlags::LAST_LINE |
            ForestHandlerFlags::SINGLE_LINE);
    );

    // 5) Compress the forest
    fh.Compress();

    // 6) Draw the compressed forests on file
    fh.DrawOnFile(algorithm_name, DrawDagFlags::DELETE_DOTCODE);

    // 7) Generate the C/C++ source code
    fh.GenerateCode();
    //GeneratePointersConditionsActionsCode(rs, GenerateConditionActionCodeFlags::NONE); -> Currently not implemented for morphology

    return EXIT_SUCCESS;
}