// Copyright(c) 2019
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

#ifndef GRAPHGEN_OPENING_RULESET_H_
#define GRAPHGEN_OPENING_RULESET_H_

#include <string>

#include "graphgen.h"

class OpeningRS : public BaseRuleSet {

public:

    using BaseRuleSet::BaseRuleSet;

    rule_set GenerateRuleSet()
    {
        // Blocks (kernel 3x3) are named as -> center pixel name uppercase:
        //  G H I
        //  L X M
        //  P Q R

        pixel_set kernel_5x5{
            { "a", {-2, -2} }, { "b", {-1, -2} }, { "c", {0, -2} }, { "d", {+1, -2} }, { "e", {+2, -2} },
            { "f", {-2, -1} }, { "g", {-1, -1} }, { "h", {0, -1} }, { "i", {+1, -1} }, { "j", {+2, -1} },
            { "k", {-2, +0} }, { "l", {-1, +0} }, { "x", {0, +0} }, { "m", {+1, +0} }, { "n", {+2, +0} },
            { "o", {-2, +1} }, { "p", {-1, +1} }, { "q", {0, +1} }, { "r", {+1, +1} }, { "s", {+2, +1} },
            { "t", {-2, +2} }, { "u", {-1, +2} }, { "v", {0, +2} }, { "w", {+1, +2} }, { "y", {+2, +2} },
        };

        rule_set morphology;
        morphology.InitConditions(kernel_5x5);
        morphology.InitActions({ "nothing", "erode" });
        
        morphology.generate_rules([&](rule_set& rs, uint i) {
            rule_wrapper r(rs, i);

            if (!r["x"]) {
                r << "nothing";
                return;
            }

            bool G = r["a"] && r["b"] && r["c"] && r["f"] && r["g"] && r["h"] && r["k"] && r["l"] && r["x"];
            bool H = r["d"] && r["b"] && r["c"] && r["i"] && r["g"] && r["h"] && r["m"] && r["l"] && r["x"];
            bool I = r["d"] && r["e"] && r["c"] && r["i"] && r["j"] && r["h"] && r["m"] && r["n"] && r["x"];

            bool L = r["f"] && r["g"] && r["h"] && r["k"] && r["l"] && r["x"] && r["o"] && r["p"] && r["q"];
            bool X = r["i"] && r["g"] && r["h"] && r["m"] && r["l"] && r["x"] && r["r"] && r["p"] && r["q"];
            bool M = r["i"] && r["j"] && r["h"] && r["m"] && r["n"] && r["x"] && r["r"] && r["s"] && r["q"];
            
            bool P = r["k"] && r["l"] && r["x"] && r["o"] && r["p"] && r["q"] && r["t"] && r["u"] && r["v"];
            bool Q = r["m"] && r["l"] && r["x"] && r["r"] && r["p"] && r["q"] && r["w"] && r["u"] && r["v"];
            bool R = r["m"] && r["n"] && r["x"] && r["r"] && r["s"] && r["q"] && r["w"] && r["y"] && r["v"];

            if (!G && !H && !I && !L && !X && !M && !P && !Q && !R) {
                r << "erode";
            }
            else {
                r << "nothing";
            }
   
        });

        return morphology;
    }
};

#endif // GRAPHGEN_OPENING_RULESET_H_