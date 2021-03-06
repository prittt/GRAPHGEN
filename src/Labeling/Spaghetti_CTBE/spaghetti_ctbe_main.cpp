// Copyright(c) 2019
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

// This target generates the optimal decision tree for the Rosenfeld mask

#include "graphgen.h"

#include "ctbe_ruleset.h"

using namespace std;

int main()
{
    string algorithm_name = "Spaghetti_CTBE";
    string mask_name = "CTBE";

    conf = ConfigData(algorithm_name, mask_name);

    CTBE_RS c_rs;
    auto rs = c_rs.GetRuleSet();

    // Call GRAPHGEN:
    // 1) Load or generate Optimal Decision Tree based on CTBE mask
    BinaryDrag<conact> bd = GetOdt(rs);

    // 2) Draw the generated tree to pdf
    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, bd);

     // 3) Generate forests of trees
    LOG(algorithm_name + " - making forests",
        ForestHandler fh(bd, rs.ps_,
            ForestHandlerFlags::CENTER_LINES |
            ForestHandlerFlags::FIRST_LINE |
            ForestHandlerFlags::LAST_LINE |
            ForestHandlerFlags::SINGLE_LINE);
    );

    // 4) Draw the generated forests on file
    fh.DrawOnFile(algorithm_name, DrawDagFlags::DELETE_DOTCODE);

    // 5) Compress the forests
    fh.Compress(DragCompressorFlags::PRINT_STATUS_BAR | DragCompressorFlags::IGNORE_LEAVES);

    // 6) Draw the compressed forests on file
    fh.DrawOnFile(algorithm_name, DrawDagFlags::DELETE_DOTCODE);

    // 7) Generate the DRAG C/C++ code: this (Pointers/Action/Code) needs to be
    // rethought to work with the CTBE mask!!!
    fh.GenerateCode();
    // GeneratePointersConditionsActionsCode(rs, GenerateConditionActionCodeFlags::NONE);

    return EXIT_SUCCESS;
}