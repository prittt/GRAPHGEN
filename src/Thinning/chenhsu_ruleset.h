// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <string>

#include "graphgen.h"

class ChenHsuRS : public BaseRuleSet {

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

        int A = (P2 == 0 && P3 == 1) + (P3 == 0 && P4 == 1) +
                (P4 == 0 && P5 == 1) + (P5 == 0 && P6 == 1) +
                (P6 == 0 && P7 == 1) + (P7 == 0 && P8 == 1) +
                (P8 == 0 && P9 == 1) + (P9 == 0 && P2 == 1);
        int B = P2 + P3 + P4 + P5 + P6 + P7 + P8 + P9;
        
        int c, d, f, g;
        if (r["iter"] == 0) {
            c = (P2 * P4 * P6 == 0);
            d = (P4 * P6 * P8 == 0);
            f = (P2 * P4 == 1 && P6 + P7 + P8 == 0);
            g = (P4 * P6 == 1 && P2 + P8 + P9 == 0);
        }
        else {
            c = (P2 * P4 * P8 == 0);
            d = (P2 * P6 * P8 == 0);
            f = (P2 * P8 == 1 && P4 + P5 + P6 == 0);
            g = (P6 * P8 == 1 && P2 + P3 + P4 == 0);
        }
        
        if (
            /*(a)*/ (2 <= B && B <= 7) && ((
            /*(b)*/ (A == 1) &&
            /*(c)*/ c        &&
            /*(d)*/ d )      || (
            /*(e)*/ (A == 2) && (
            /*(f)*/ f        ||
            /*(g)*/ g )))
            )
            r << "change0";
        else
            r << "keep1";
    });

    return thinning;
}
};
