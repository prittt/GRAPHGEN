// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "graphgen.h"

#include "erosion_ruleset.h"

using namespace std;

int main()
{
    string algorithm_name = "Erosion3x3_Tree";
    string mask_name = "kernel3x3";

    conf = ConfigData(algorithm_name, mask_name);

    ErosionRS e_rs;
    auto rs = e_rs.GetRuleSet();

    // Call GRAPHGEN:
    // 1) Load or generate Optimal Decision Tree based on Erosion 3x3 mask
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
    // Not implemented for morphology 

    return EXIT_SUCCESS;
}