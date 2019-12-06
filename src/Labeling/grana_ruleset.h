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

#ifndef GRAPHGEN_GRANA_RULESET_H_
#define GRAPHGEN_GRANA_RULESET_H_

#include <string>

#include "graphgen.h"

class GranaRS : public BaseRuleSet {

public:

    using BaseRuleSet::BaseRuleSet;

    rule_set GenerateRuleSet()
    {
        pixel_set grana_mask{
            /*{ "a", {-2, -2} },*/ { "b", {-1, -2} },{ "c", {+0, -2} },{ "d", {+1, -2} },{ "e", {+2, -2} }, /*{ "f", {+3, -2} },*/
              { "g", {-2, -1} },   { "h", {-1, -1} },{ "i", {+0, -1} },{ "j", {+1, -1} },{ "k", {+2, -1} }, /*{ "l", {+3, -1} },*/
              { "m", {-2, +0} },   { "n", {-1, +0} },{ "o", {+0, +0} },{ "p", {+1, +0} },
              /*{ "q", {-2, +1} },*/ { "r", {-1, +1} },{ "s", {+0, +1} },{ "t", {+1, +1} },
        };
        grana_mask.SetShifts({ 2, 2 });

        rule_set labeling;
        labeling.InitConditions(grana_mask);
        labeling.InitActions({ "nothing", "x<-newlabel",
            "x<-P", "x<-Q", "x<-R", "x<-S",
            "x<-P+Q", "x<-P+R", "x<-P+S", "x<-Q+R", "x<-Q+S", "x<-R+S",
            "x<-P+Q+R", "x<-P+Q+S", "x<-P+R+S", "x<-Q+R+S", });

		/*labeling.generate_rules([](rule_set& rs, uint i) {
			rule_wrapper r(rs, i);

			bool X = r["o"] || r["p"] || r["s"] || r["t"];
			if (!X) {
				r << "nothing";
				return;
			}

			connectivity_mat con({ "P", "Q", "R", "S", "x" });

			con.set("x", "P", r["h"] && r["o"]);
			con.set("x", "Q", (r["i"] || r["j"]) && (r["o"] || r["p"]));
			con.set("x", "R", r["k"] && r["p"]);
			con.set("x", "S", (r["n"] || r["r"]) && (r["o"] || r["s"]));

			con.set("P", "Q", (r["b"] || r["h"]) && (r["c"] || r["i"]));
			con.set("P", "S", (r["g"] || r["h"]) && (r["m"] || r["n"]));
			con.set("Q", "R", (r["d"] || r["j"]) && (r["e"] || r["k"]));
			con.set("Q", "S", (r["i"] && r["n"]) || (con("P", "Q") && con("P", "S")));

			con.set("P", "R", con("P", "Q") && con("Q", "R"));
			con.set("S", "R", (con("P", "R") && con("P", "S")) || (con("S", "Q") && con("Q", "R")));

			MergeSet ms(con);
			ms.BuildMergeSet();

			for (const auto& s : ms.mergesets_) {
				std::string action = "x<-";
				if (s.empty())
					action += "newlabel";
				else {
					action += s[0];
					for (size_t i = 1; i < s.size(); ++i)
						action += "+" + s[i];
				}
				r << action;
			}
		});*/

        return labeling;
    }


	action_bitset GetActionFromRuleIndex(const rule_set& rs, uint64_t rule_index) const override {
		rule_wrapper r(rs, rule_index);

		bool X = r["o"] || r["p"] || r["s"] || r["t"];
		if (!X) {
			//r << "nothing";
			return action_bitset().addAction(0);
		}

		connectivity_mat con({ "P", "Q", "R", "S", "x" });

		con.set("x", "P", r["h"] && r["o"]);
		con.set("x", "Q", (r["i"] || r["j"]) && (r["o"] || r["p"]));
		con.set("x", "R", r["k"] && r["p"]);
		con.set("x", "S", (r["n"] || r["r"]) && (r["o"] || r["s"]));

		con.set("P", "Q", (r["b"] || r["h"]) && (r["c"] || r["i"]));
		con.set("P", "S", (r["g"] || r["h"]) && (r["m"] || r["n"]));
		con.set("Q", "R", (r["d"] || r["j"]) && (r["e"] || r["k"]));
		con.set("Q", "S", (r["i"] && r["n"]) || (con("P", "Q") && con("P", "S")));

		con.set("P", "R", con("P", "Q") && con("Q", "R"));
		con.set("S", "R", (con("P", "R") && con("P", "S")) || (con("S", "Q") && con("Q", "R")));

		MergeSet ms(con);
		ms.BuildMergeSet();

		action_bitset combined_actions;
		for (const auto& s : ms.mergesets_) {
			std::string action = "x<-";
			if (s.empty())
				action += "newlabel";
			else {
				action += s[0];
				for (size_t i = 1; i < s.size(); ++i)
					action += "+" + s[i];
			}
			combined_actions.addAction(rs.actions_pos.at(action) - 1);
		}
		return combined_actions;
	}

};

#endif // GRAPHGEN_GRANA_RULESET_H_
