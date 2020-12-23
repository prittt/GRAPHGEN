// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <string>

#include "graphgen.h"

class GuoHallRS : public BaseRuleSet {

public:

    using BaseRuleSet::BaseRuleSet;

    rule_set GenerateRuleSet()
{
    pixel_set gh_mask {
        { "P9", {-1, -1} }, { "P2", {0, -1} }, { "P3", {+1, -1} },
        { "P8", {-1,  0} }, { "P1", {0,  0} }, { "P4", {+1,  0} },
        { "P7", {-1, +1} }, { "P6", {0, +1} }, { "P5", {+1, +1} },
    };

    rule_set thinning;
    thinning.InitConditions(gh_mask);
    thinning.AddCondition("iter");
    thinning.InitActions({
        "keep0",
        "keep1",
        "change0",
        });


    thinning.generate_rules([](rule_set& rs, uint i) {
        rule_wrapper r(rs, i);

        int P1 = r["P1"];
        int P2 = r["P2"];
        int P3 = r["P3"];
        int P4 = r["P4"];
        int P5 = r["P5"];
        int P6 = r["P6"];
        int P7 = r["P7"];
        int P8 = r["P8"];
        int P9 = r["P9"];
        if (!P1) {
            r << "keep0";
            return;
        }

        int C = ((!P2) & (P3 | P4)) + ((!P4) & (P5 | P6)) +
                ((!P6) & (P7 | P8)) + ((!P8) & (P9 | P2));
        int N1 = (P9 | P2) + (P3 | P4) + (P5 | P6) + (P7 | P8);
        int N2 = (P2 | P3) + (P4 | P5) + (P6 | P7) + (P8 | P9);
        int N = N1 < N2 ? N1 : N2;
        
        int m;
        if (r["iter"] == 0) {
            m = (P6 | P7 | (!P9)) & P8;
        }
        else {
            m = (P2 | P3 | (!P5)) & P4;
        }
        
        if (
            /*(a)*/ (C == 1) && 
            /*(b)*/ (2 <= N && N <= 3) &&
            /*(c)*/ m == 0 
            )
            r << "change0";
        else
            r << "keep1";
    });

    return thinning;
}
};
