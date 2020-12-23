// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_ROSENFELD3D_RULESET_H_
#define GRAPHGEN_ROSENFELD3D_RULESET_H_

#include <string>

#include "graphgen.h"

class Grana3dRS : public BaseRuleSet {

public:

    using BaseRuleSet::BaseRuleSet;

	rule_set GenerateRuleSet()
	{

        pixel_set grana3d_block_mask = {
            { "a", {-2,-2,-2} }, { "b", {+0,-2,-2} }, { "c", {+2,-2,-2} },
            { "d", {-2,+0,-2} }, { "e", {+0,+0,-2} }, { "f", {+2,+0,-2} },
            { "g", {-2,+2,-2} }, { "h", {+0,+2,-2} }, { "i", {+2,+1,-1} },
            { "j", {-2,-2,+0} }, { "k", {+0,-2,+0} }, { "l", {+2,-2,+0} },
            { "m", {-2,+0,+0} }, { "x", {+0,+0,+0} },
        };

        pixel_set grana3d_pixel_mask{

          /*{ "aa", {-2,-2,-2} }, { "ab", {-1,-2,-2} }, { "ba", {+0,-2,-2} }, { "bb", {+1,-2,-2} }, { "ca", {+2,-2,-2} }, { "cb", {+3,-2,-2} },
            { "ac", {-2,-1,-2} }, { "ad", {-1,-1,-2} }, { "bc", {+0,-1,-2} }, { "bd", {+1,-1,-2} }, { "cc", {+2,-1,-2} }, { "cd", {+3,-1,-2} },

            { "da", {-2,+0,-2} }, { "db", {-1,+0,-2} }, { "ea", {+0,+0,-2} }, { "eb", {+1,+0,-2} }, { "fa", {+2,+0,-2} }, { "fb", {+3,+0,-2} },
            { "dc", {-2,+1,-2} }, { "dd", {-1,+1,-2} }, { "ec", {+0,+1,-2} }, { "ed", {+1,+1,-2} }, { "fc", {+2,+1,-2} }, { "fd", {+3,+1,-2} },

            { "ga", {-2,+2,-2} }, { "gb", {-1,+2,-2} }, { "ha", {+0,+2,-2} }, { "hb", {+1,+2,-2} }, { "ia", {+2,+2,-2} }, { "ib", {+3,+2,-2} },
            { "gc", {-2,+3,-2} }, { "gd", {-1,+3,-2} }, { "hc", {+0,+3,-2} }, { "hd", {+1,+3,-2} }, { "ic", {+2,+3,-2} }, { "id", {+3,+3,-2} },*/

            /**********************************************************************************************************************************/

            { "ae", {-2,-2,-1} }, { "af", {-1,-2,-1} }, { "be", {+0,-2,-1} }, { "bf", {+1,-2,-1} }, { "ce", {+2,-2,-1} }, { "cf", {+3,-2,-1} },
            { "ag", {-2,-1,-1} }, { "ah", {-1,-1,-1} }, { "bg", {+0,-1,-1} }, { "bh", {+1,-1,-1} }, { "cg", {+2,-1,-1} }, { "ch", {+3,-1,-1} },

            { "de", {-2,+0,-1} }, { "df", {-1,+0,-1} }, { "ee", {+0,+0,-1} }, { "ef", {+1,+0,-1} }, { "fe", {+2,+0,-1} }, { "ff", {+3,+0,-1} },
            { "dg", {-2,+1,-1} }, { "dh", {-1,+1,-1} }, { "eg", {+0,+1,-1} }, { "eh", {+1,+1,-1} }, { "fg", {+2,+1,-1} }, { "fh", {+3,+1,-1} },

            { "ge", {-2,+2,-1} }, { "gf", {-1,+2,-1} }, { "he", {+0,+2,-1} }, { "hf", {+1,+2,-1} }, { "ie", {+2,+2,-1} }, { "if", {+3,+2,-1} },
            { "gg", {-2,+3,-1} }, { "gh", {-1,+3,-1} }, { "hg", {+0,+3,-1} }, { "hh", {+1,+3,-1} }, { "ig", {+2,+3,-1} }, { "ih", {+3,+3,-1} },

            /**********************************************************************************************************************************/

            { "ja", {-2,-2,+0} }, { "jb", {-1,-2,+0} }, { "ka", {+0,-2,+0} }, { "kb", {+1,-2,+0} }, { "la", {+2,-2,+0} }, { "lb", {+3,-2,+0} },
            { "jc", {-2,-1,+0} }, { "jd", {-1,-1,+0} }, { "kc", {+0,-1,+0} }, { "kd", {+1,-1,+0} }, { "lc", {+2,-1,+0} }, { "ld", {+3,-1,+0} },

            { "ma", {-2,+0,+0} }, { "mb", {-1,+0,+0} }, { "xa", {+0,+0,+0} }, { "xb", {+1,+0,+0} },
            { "mc", {-2,+1,+0} }, { "md", {-1,+1,+0} }, { "xc", {+0,+1,+0} }, { "xd", {+1,+1,+0} },

            /**********************************************************************************************************************************/

            { "je", {-2,-2,+0} }, { "jf", {-1,-2,+0} }, { "ke", {+0,-2,+0} }, { "kf", {+1,-2,+0} }, { "le", {+2,-2,+0} }, { "lf", {+3,-2,+0} },
            { "jg", {-2,-1,+0} }, { "jh", {-1,-1,+0} }, { "kg", {+0,-1,+0} }, { "kh", {+1,-1,+0} }, { "lg", {+2,-1,+0} }, { "lh", {+3,-1,+0} },

            { "me", {-2,+0,+1} }, { "mf", {-1,+0,+1} }, { "xe", {+0,+0,+1} }, { "xf", {+1,+0,+1} },
            { "mg", {-2,+1,+1} }, { "mh", {-1,+1,+1} }, { "xg", {+0,+1,+1} }, { "xh", {+1,+1,+1} },

        };

		rule_set labeling;
		labeling.InitConditions(grana3d_pixel_mask);

		graph ag = MakeAdjacencies(grana3d_pixel_mask);

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
		});

		return labeling;
	}

};

#endif // GRAPHGEN_ROSENFELD_RULESET_H_