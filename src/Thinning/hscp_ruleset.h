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

#include <string>

#include "graphgen.h"

class HscpRS : public BaseRuleSet {

public:

    using BaseRuleSet::BaseRuleSet;

struct edge {
    bool t00 = 0, t01 = 0, t01s = 0, t11 = 0;

    void check(bool v1, bool v2, bool v3) {
        if (!v2 && (!v1 || !v3))
            t00 = true;
        if (v2 && (v1 || v3))
            t11 = true;
        if ((!v1 && v2) || (!v2 && v3)) {
            t01s = t01;
            t01 = true;
        }
    }

    bool operator()(uint16_t block)
    {
        const bool vNW = (block >> 0x0) & 1;
        const bool vN = (block >> 0x1) & 1;
        const bool vNE = (block >> 0x2) & 1;
        const bool vW = (block >> 0x4) & 1;
        const bool vC = (block >> 0x5) & 1;
        const bool vE = (block >> 0x6) & 1;
        const bool vSW = (block >> 0x8) & 1;
        const bool vS = (block >> 0x9) & 1;
        const bool vSE = (block >> 0xa) & 1;

        check(vNW, vN, vNE);
        check(vNE, vE, vSE);
        check(vSE, vS, vSW);
        check(vSW, vW, vNW);

        return vC && t00 && t11 && !t01s;
    }
};
static bool survives_HSCP(uint16_t block)
{
    const bool vNW = (block >> 0x0) & 1;
    const bool vN = (block >> 0x1) & 1;
    const bool vNE = (block >> 0x2) & 1;
    const bool vW = (block >> 0x4) & 1;
    const bool vC = (block >> 0x5) & 1;
    const bool vE = (block >> 0x6) & 1;
    const bool vSW = (block >> 0x8) & 1;
    const bool vS = (block >> 0x9) & 1;
    const bool vSE = (block >> 0xa) & 1;

    const bool edgeC = edge()(block);
    const bool edgeE = edge()(block >> 1);
    const bool edgeS = edge()(block >> 4);
    const bool edgeSE = edge()(block >> 5);

    return !edgeC ||
        (edgeE && vN && vS) ||
        (edgeS && vW && vE) ||
        (edgeE && edgeSE && edgeS);
}

    rule_set GenerateRuleSet()
{
    /*
        abcd
        efgh
        ijkl
        mnop
    */
    pixel_set hscp_mask {
        { "a", {-1, -1} }, { "b", {0, -1} }, { "c", {+1, -1} }, { "d", {+2, -1} },
        { "e", {-1,  0} }, { "f", {0,  0} }, { "g", {+1,  0} }, { "h", {+2,  0} },
        { "i", {-1, +1} }, { "j", {0, +1} }, { "k", {+1, +1} }, { "l", {+2, +1} },
        { "m", {-1, +2} }, { "n", {0, +2} }, { "o", {+1, +2} }, { "p", {+2, +2} },
    };

    rule_set thinning;
    thinning.InitConditions(hscp_mask);
    thinning.InitActions({
        "keep0",
        "keep1",
        "change0",
        });


    thinning.generate_rules([](rule_set& rs, uint i) {
        rule_wrapper r(rs, i);

        if (!r["f"]) {
            r << "keep0";
            return;
        }

        
        if (survives_HSCP(i))
            r << "keep1";
        else
            r << "change0";
    });

    return thinning;
}
};