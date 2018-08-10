// Copyright(c) 2018 Costantino Grana, Federico Bolelli 
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

#include "ruleset_generator.h"

#include <bitset>
#include <string>

#include "merge_set.h"

using namespace std;

static bool g_debug_log = false;

rule_set generate_rosenfeld()
{
    pixel_set rosenfeld_mask {
        { "p", -1, -1 }, { "q", 0, -1 }, { "r", +1, -1 },
        { "s", -1,  0 }, { "x", 0, 0 },
    };

    rule_set labeling;
    labeling.init_conditions(rosenfeld_mask);
    labeling.init_actions({
        "nothing",
        "x<-newlabel",
        "x<-p", "x<-q", "x<-r", "x<-s",
        "x<-p+q", "x<-p+r", "x<-p+s", "x<-q+r", "x<-q+s", "x<-r+s",
        "x<-p+q+r", "x<-p+q+s", "x<-p+r+s", "x<-q+r+s",
        "x<-p+q+r+s",
        });


    labeling.generate_rules([](rule_set& rs, uint i) {
        rule_wrapper r(rs, i);

        bool X = r["x"];
        if (!X) {
            r << "nothing";
            return;
        }

        connectivity_mat<5> con({ "p", "q", "r", "s", "x" });

        con.set("x", "p", r["p"]);
        con.set("x", "q", r["q"]);
        con.set("x", "r", r["r"]);
        con.set("x", "s", r["s"]);

        con.set("p", "q", r["p"] && r["q"]);
        con.set("p", "s", r["p"] && r["s"]);
        con.set("q", "r", r["q"] && r["r"]);
        con.set("q", "s", r["q"] && r["s"]);

        con.set("p", "r", con("p", "q") && con("q", "r"));
        con.set("s", "r", (con("p", "r") && con("p", "s")) || (con("s", "q") && con("q", "r")));

        if (g_debug_log) {
            con.DisplayCondNames();
            string bits = bitset<5>(i).to_string();
            reverse(begin(bits), end(bits));
            cout << bits << "\n";
            con.DisplayMap();
            cout << endl;
        }

        MergeSet<5> ms(con);
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
