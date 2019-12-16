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
#include "compression.h"
#include "grana_ruleset.h"

using namespace std;

int main()
{
	//ZstdCompression compr;
	//compr.allocateResources(128 * 1024 * 1024);
	//compr.compressFile("D:/code/graphsgen/bin/outputs/BBDT3D/rules/BBDT3D_0-67108864_rules.bin", "D:/code/graphsgen/bin/outputs/BBDT3D/rules/BBDT3D_0-67108864_rules.zst");
	//compr.freeResources();

	//ZstdDecompression decompr;
	//decompr.allocateResources(128 * 1024 * 1024);
	//decompr.decompressFile("D:/code/graphsgen/bin/outputs/BBDT3D/rules/BBDT3D_0-67108864_rules.zst", "D:/code/graphsgen/bin/outputs/BBDT3D/rules/BBDT3D_0-67108864_rules_new.bin");
	//decompr.freeResources();

	ZstdStreamingCompression streamcompr;
	streamcompr.beginStreaming("D:/code/graphsgen/bin/outputs/BBDT3D/rules/0test.zst");
	for (int i = 0; i < 100000000; i++) {
		streamcompr.compressDataChunk(reinterpret_cast<const void*>(&i), sizeof(i), i == 99999999);
	}
	streamcompr.endStreaming();

	ZstdDecompression decompr;
	decompr.allocateResources();
	TLOG("Decompress", 
	decompr.decompressFile("D:/code/graphsgen/bin/outputs/BBDT3D/rules/0test.zst", "D:/code/graphsgen/bin/outputs/BBDT3D/rules/0test.bin");	
	);
	decompr.freeResources();

	exit(EXIT_SUCCESS);

    string algorithm_name = "BBDT";
    conf = ConfigData(algorithm_name, "Grana");

    GranaRS g_rs;
    auto rs = g_rs.GetRuleSet();

    // Call GRAPHGEN:
    // 1) Load or generate Optimal Decision Tree based on Grana mask
	BinaryDrag<conact> bd = GetHdt(rs, g_rs, true);

	//BinaryDragStatistics stats(bd);
	//std::cout << "Generated Tree has " << stats.Leaves() << " leaves and an average path length of " << stats.AveragePathLength() << "." << std::endl;

    // 2) Draw the generated tree
    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, bd);

    // 3) Generate the tree C/C++ code taking care of the names used
    //    in the Grana's rule set GranaRS
    GenerateDragCode(bd);
    pixel_set block_positions{
           { "P", {-2, -2} },{ "Q", {+0, -2} },{ "R", {+2, -2} },
           { "S", {-2, +0} },{ "x", {+0, +0} }
    };
    GeneratePointersConditionsActionsCode(rs, 
                                          GenerateConditionActionCodeFlags::CONDITIONS_WITH_IFS | GenerateConditionActionCodeFlags::ACTIONS_WITH_CONTINUE, 
                                          GenerateActionCodeTypes::LABELING,
                                          block_positions);

	getchar();
    return EXIT_SUCCESS;
}