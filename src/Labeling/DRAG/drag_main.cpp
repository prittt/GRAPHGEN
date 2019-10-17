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
// * Neither the name of GRAPHSGEN nor the names of its
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

#include "graphsgen.h"

#include "grana_ruleset.h"

#include <unordered_set>

using namespace std;

int main()
{
    string algorithm_name = "DRAG";
    conf = ConfigData(algorithm_name);

    GranaRS g_rs;
    auto rs = g_rs.GetRuleSet();

    // Call GRAPHSGEN:
    // 1) Load or generate Optimal Decision Tree based on Grana mask
    BinaryDrag<conact> bd = GetOdt(rs, algorithm_name);

    // 2) Draw the generated tree to pdf
    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, bd);

    ofstream os("foglie.txt");
    BinaryDragStatistics a(bd);
    a.PrintLeaves(os);

    RemoveEqualSubtrees{bd};
    DrawDagOnFile("RemoveEqualSubtrees", bd);

    BinaryDragStatistics b(bd);
    b.PrintLeaves(os);

    DragCompressor{ bd };
    
    BinaryDragStatistics c(bd);
    c.PrintLeaves(os);

    DrawDagOnFile("DragCompressor", bd, true);
    GenerateDragCode(algorithm_name, bd);
    pixel_set block_positions{
           { "P", {-2, -2} },{ "Q", {+0, -2} },{ "R", {+2, -2} },
           { "S", {-2, +0} },{ "x", {+0, +0} }
    };
    GeneratePointersConditionsActionsCode(rs, false, block_positions);
        
    /*{
        TLOG("Creating DRAG using equivalences",
            std::cout << "\n";
            auto t2 = t;
            RemoveEqualSubtrees sc(t2.GetRoot());
            DrawDagOnFile("RemoveEqualSubtrees", t2, true);
            std::cout << "After equal subtrees removal: nodes = " << sc.nodes_ << " - leaves = " << sc.leaves_ << "\n";

            FindOptimalDrag c(t2);
            c.GenerateAllTrees();
            DrawDagOnFile("FindOptimalDrag", c.best_tree_, true);
            std::cout << "\n";
            );
    }*/
    return 0;
    /*
        // 2a) Convert Optimal Decision Tree into Directed Rooted Acyclic Graph
        //     using a exhaustive strategy
        TLOG("Creating DRAG using identites",
            Tree2DagUsingIdentities(t);
        );
        string drag_filename = algorithm_name + "_drag_identities";
        DrawDagOnFile(drag_filename, t, true);
        PrintStats(t);

        //string odrag_filename = algorithm_name + "_optimal_drag.txt";
        //if (!LoadConactDrag(t, odrag_filename)) {
        //    TLOG("Computing optimal DRAG\n",
        //        Dag2OptimalDag(t);
        //    );
        //    WriteConactDrag(t, odrag_filename);
        //}
        //string optimal_drag_filename = algorithm_name + "_optimal_drag";
        //DrawDagOnFile(optimal_drag_filename, t, true);
        //PrintStats(t);

        // 2b) Convert Optimal Decision Tree into Directed Rooted Acyclic Graph
        //     using a heuristic
        // TODO

        // 3) Generate the C++ source code for the ODT
        GenerateDragCode(algorithm_name, t);

        // 4) Generate the C++ source code for pointers,
        // conditions to check and actions to perform
        pixel_set block_positions{
              { "P", {-2, -2} },{ "Q", {+0, -2} },{ "R", {+2, -2} },
              { "S", {-2, +0} },{ "x", {+0, +0} }
        };
        GeneratePointersConditionsActionsCode(algorithm_name, rs, block_positions);
    */
    return EXIT_SUCCESS;
}