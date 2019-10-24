// Copyright(c) 2018 - 2019 Costantino Grana, Federico Bolelli 
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
//
// *Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and / or other materials provided with the distribution.
//
// * Neither the name of GRAPHGEN nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "yaml-cpp/yaml.h"

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
    AddFrequenciesToRuleset(rs);

    // 2) Load or generate Optimal Decision Tree based on Zang-Suen algorithm
    BinaryDrag<conact> bd = GetOdt(rs);

    // 3) Draw the generated tree on file
    string tree_filename = algo_name + "_tree";
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