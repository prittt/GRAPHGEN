// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

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
