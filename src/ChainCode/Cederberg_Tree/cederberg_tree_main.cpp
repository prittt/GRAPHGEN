// Copyright(c) 2018 - 2019
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