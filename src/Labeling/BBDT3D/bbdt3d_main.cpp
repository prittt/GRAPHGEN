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

#include "graphgen.h"

//#include "grana3d_36c_222b_ruleset.h"
//#include "grana3d_26c_221b_ruleset.h"
#include "grana3d_19c_211b_ruleset.h"

using namespace std;

int main()
{
	//string algorithm_name = "BBDT3D-36c-222b";
	//string algorithm_name = "BBDT3D-26c-221b";
	string algorithm_name = "BBDT3D-19c-211b";
    conf = ConfigData(algorithm_name, "Grana3D");

    Grana3d_19c_211b_RS g_rs;
    auto rs = g_rs.GetRuleSet();

    // Call GRAPHGEN:
    // 1) Load or generate Optimal Decision Tree based on Grana mask

	//std::cout << " --> DrawDagOnFile()" << std::endl;

 //   // 2) Draw the generated tree
 //   string tree_filename = algorithm_name + "_tree";
 //   DrawDagOnFile(tree_filename, bd);

	std::cout << " --> GenerateDragCode()" << std::endl;

    // 3) Generate the tree C/C++ code taking care of the names used
    //    in the Grana's rule set GranaRS
    GenerateDragCode(bd);

	std::cout << " --> GeneratePointersConditionsActionsCode()" << std::endl;

    pixel_set block_positions{
			    {"K", {-2,-1,-1}},{"L", {+0,-1,-1}},{"M", {+2,-1,-1}},
				{"N", {-2,+0,-1}},{"O", {+0,+0,-1}},{"P", {+2,+0,-1}},
				{"Q", {-2,+1,-1}},{"R", {+0,+1,-1}},{"S", {+2,+1,-1}},

				{"T", {-2,-1,+0}},{"U", {+0,-1,+0}},{"V", {+2,-1,+0}},
				{"W", {-2,+0,+0}},{"X", {+0,+0,+0}} 
    };
    GeneratePointersConditionsActionsCode(rs, 
                                          GenerateConditionActionCodeFlags::CONDITIONS_WITH_IFS | GenerateConditionActionCodeFlags::ACTIONS_WITH_CONTINUE, 
                                          GenerateActionCodeTypes::LABELING,
                                          block_positions);

	std::cout << " ** DONE" << std::endl;
	getchar();
    return EXIT_SUCCESS;
}