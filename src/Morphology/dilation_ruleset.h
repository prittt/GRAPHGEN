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

#ifndef GRAPHGEN_DILATION_RULESET_H_
#define GRAPHGEN_DILATION_RULESET_H_

#include <string>

#include "graphgen.h"

class DilationRS : public BaseRuleSet {

public:

    using BaseRuleSet::BaseRuleSet;

    rule_set GenerateRuleSet()
    {
        pixel_set kernel_3x3{
            { "a", {-1, -1} }, { "b", {0, -1} }, { "c", {+1, -1} },
            { "d", {-1, +0} }, { "x", {0, +0} }, { "e", {+1, +0} }, 
            { "f", {-1, +1} }, { "g", {0, +1} }, { "h", {+1, +1} },
        };

        rule_set morphology;
        morphology.InitConditions(kernel_3x3);
        morphology.InitActions({ "nothing", "dilate" });
        
        morphology.generate_rules([&](rule_set& rs, uint i) {
            rule_wrapper r(rs, i);

            //if (r["x"]) {
            //    r << "nothing";
            //    return;
            //}

            if (r["x"] || r["a"] || r["b"] || r["c"] || r["d"] || r["e"] || r["f"] || r["g"] || r["h"]) {
                r << "dilate";
            }
            else {
                r << "nothing";
            }

   
        });

        return morphology;
    }
};

#endif // GRAPHGEN_DILATION_RULESET_H_