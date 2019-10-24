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

#include "grana_ruleset.h"

#include "image_frequencies.h"

using namespace std;

int main()
{
    string algo_name = "Spaghetti_FREQ";
    string mask_name = "Grana";
    conf = ConfigData(algo_name, mask_name, true);

    GranaRS g_rs;
    auto rs = g_rs.GetRuleSet();

    // Call GRAPHGEN:
    // 1) Count frequencies
    AddFrequenciesToRuleset(rs);

    // 2) Load or generate Optimal Decision Tree based on Grana mask
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
    fh.Compress(DragCompressorFlags::PRINT_STATUS_BAR | DragCompressorFlags::IGNORE_LEAVES, 1);

    // 7) Draw the compressed forests on file
    fh.DrawOnFile(algo_name, DrawDagFlags::DELETE_DOTCODE);

    // 8) Generate the C/C++ code taking care of the names used
    //    in the Grana's rule set GranaRS
    fh.GenerateCode(BeforeMainShiftTwo);
    pixel_set block_positions{
           { "P", {-2, -2} },{ "Q", {+0, -2} },{ "R", {+2, -2} },
           { "S", {-2, +0} },{ "x", {+0, +0} }
    };
    GeneratePointersConditionsActionsCode(rs, 
                                          GenerateConditionActionCodeFlags::NONE, 
                                          GenerateActionCodeTypes::LABELING,
                                          block_positions);

    return EXIT_SUCCESS;
}