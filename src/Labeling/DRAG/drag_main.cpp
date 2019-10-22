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

#include <unordered_set>

using namespace std;

int main()
{
    string algorithm_name = "DRAG";
    conf = ConfigData(algorithm_name);

    GranaRS g_rs;
    auto rs = g_rs.GetRuleSet();

    // Call GRAPHGEN:
    // 1) Load or generate Optimal Decision Tree based on Grana mask
    BinaryDrag<conact> bd = GetOdt(rs, algorithm_name);

    // 2) Draw the generated tree
    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, bd);

    // 3) Compress the tree into a DRAG
    DragCompressor{ bd };

    // 4) Draw the generated DRAG
    string drag_filename = algorithm_name + "_drag";
    DrawDagOnFile(drag_filename, bd);
    
    // 5) Generate the DRAG C/C++ code taking care of the names used
    //    in the Grana's rule set GranaRS
    GenerateDragCode(algorithm_name, bd);
    pixel_set block_positions{
           { "P", {-2, -2} },{ "Q", {+0, -2} },{ "R", {+2, -2} },
           { "S", {-2, +0} },{ "x", {+0, +0} }
    };
    GeneratePointersConditionsActionsCode(rs, true, block_positions);
    
    return EXIT_SUCCESS;
}