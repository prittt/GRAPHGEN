// Copyright(c) 2018
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

#include <fstream>

#include "connectivity_graph.h"
#include "merge_set.h"
#include "ruleset_generator.h"

using namespace std;

rule_set generate_rosenfeld_3d()
{
    pixel_set rosenfeld_mask = {
        { "a", {-1,-1,-1} }, { "b", {+0,-1,-1} }, { "c", {+1,-1,-1} },
        { "d", {-1,+0,-1} }, { "e", {+0,+0,-1} }, { "f", {+1,+0,-1} },
        { "g", {-1,+1,-1} }, { "h", {+0,+1,-1} }, { "i", {+1,+1,-1} },
        { "j", {-1,-1,+0} }, { "k", {+0,-1,+0} }, { "l", {+1,-1,+0} },
        { "m", {-1,+0,+0} }, { "x", {+0,+0,+0} },
    };

    rule_set labeling;
    labeling.InitConditions(rosenfeld_mask);

    graph ag = MakeAdjacencies(rosenfeld_mask);

    auto actions = GenerateAllPossibleLabelingActions(ag);
	
	/*{
		ofstream os("out.txt");
		os << ag;
	}*/

    labeling.InitActions(actions);

    labeling.generate_rules([&](rule_set& rs, uint i) {
        rule_wrapper r(rs, i);

        if (!r["x"]) {
            r << "nothing";
            return;
        }

        auto lag = ag;
        for (size_t j = 0; j < lag.size(); ++j) {
            if (((i >> j) & 1) == 0)
                lag.DetachNode(j);
        }
        graph cg = MakeConnectivities(lag);

        connectivity_mat con(rs.conditions);
        con.data_ = cg.arcs_;

        MergeSet ms(con);
        ms.BuildMergeSet();

        for (const auto& s : ms.mergesets_) {
            string action = "x<-";
            if (s.empty())
                action += "newlabel";
            else {
                action += s[0];
                for (size_t i = 1; i < s.size(); ++i)
                    action += "+" + s[i];
            }
            r << action;
        }
    });

    return labeling;
}
