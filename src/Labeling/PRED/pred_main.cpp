// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "graphgen.h"

#include "rosenfeld_ruleset.h"

using namespace std;

int main()
{
    // Setup configuration
    string algorithm_name = "PRED";
    conf = ConfigData(algorithm_name, "Rosenfeld");

    // Load or generate rules
    RosenfeldRS r_rs;
    auto rs = r_rs.GetRuleSet();

    // Call GRAPHGEN:
    // 1) Load or generate Optimal Decision Tree based on Rosenfeld mask
    BinaryDrag<conact> bd = GetOdt(rs);
    
    // 2) Draw the generated tree on file
    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, bd);
    
    // 3) Generate forests of trees
    LOG(algorithm_name + " - making forests",
        ForestHandler fh(bd, rs.ps_);
    );
    
    // 4) Draw the generated forests on file
    fh.DrawOnFile(algorithm_name, DrawDagFlags::DELETE_DOTCODE);
    
    // 5) Generate the C/C++ source code
    fh.GenerateCode();
    GeneratePointersConditionsActionsCode(rs, GenerateConditionActionCodeFlags::NONE);

	return EXIT_SUCCESS;
}