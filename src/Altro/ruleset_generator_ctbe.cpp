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

#include "ruleset_generator.h"

#include <bitset>
#include <string>

#include "merge_set.h"
#include "connectivity_graph.h"

using namespace std;

rule_set generate_ctbe()
{
    // +-+-+-+
    // |a|b|c|
    // +-+-+-+
    // |d|e|
    // +-+-+
    // |f|g|
    // +-+-+
    // |h|i|
    // +-+-+
    // 

    pixel_set ctbe_mask{
            { "a", {-1, -1} }, { "b", {+0, -1} },{ "c", {+1, -1} },
            { "d", {-1, +0} }, { "e", {+0, +0} },
            { "f", {-1, +1} }, { "g", {+0, +1} },
            { "h", {-1, +2} }, { "i", {+0, +2} }
    };
    ctbe_mask.SetShifts({ 1, 3 });

    /*pixel_set ctbe_e_mask{
        { "a",{ -1, -1 } },{ "b",{ +0, -1 } },{ "c",{ +1, -1 } },
        { "d",{ -1, +0 } },{ "e",{ +0, +0 } },
        { "f",{ -1, +1 } }
    };

    pixel_set ctbe_g_mask{
            { "d", {-1, +0} }, { "e", {+0, +0} },
            { "f", {-1, +1} }, { "g", {+0, +1} },
            { "h", {-1, +2} }
    };

    pixel_set ctbe_i_mask{
            { "f", {-1, +1} }, { "g", {+0, +1} },
            { "h", {-1, +2} }, { "i", {+0, +2} }
    };*/


    rule_set labeling;
    labeling.InitConditions(ctbe_mask);

    // General adjacencies graph
    graph ag = MakeAdjacencies(ctbe_mask);
    {
        ofstream os("ctb_adjacencies.txt");
        if (os) {
            os << ag;
        }
    }

    //// "e" adjacencies graph
    //graph ag_e = MakeAdjacencies(ctbe_e_mask);
    //{
    //    ofstream os("ctb_e_adjacencies.txt");
    //    if (os) {
    //        os << ag_e;
    //    }
    //}
    //auto actions_e = GenerateAllPossibleLabelingActions(ag_e, "e");

    //// "g" adjacencies graph
    //graph ag_g = MakeAdjacencies(ctbe_g_mask);
    //{
    //    ofstream os("ctb_g_adjacencies.txt");
    //    if (os) {
    //        os << ag_g;
    //    }
    //}
    //auto actions_g = GenerateAllPossibleLabelingActions(ag_g, "g");

    //// "i" adjacencies graph
    //graph ag_i = MakeAdjacencies(ctbe_i_mask);
    //{
    //    ofstream os("ctb_i_adjacencies.txt");
    //    if (os) {
    //        os << ag_i;
    //    }
    //}
    //auto actions_i = GenerateAllPossibleLabelingActions(ag_i, "i");
    auto actions = GenerateAllPossibleLabelingActionsGivenTheSetOfPixelToBeLabeled(ag, { "e", "g", "i" }, labeling);

    //// Costruisco il set finale di azioni dove ogni azioni sar� del tipo e<-..,g<-..,i<-..
    //vector<string> actions;
    //for (const auto& ae : actions_e) {
    //    for (const auto& ag : actions_g) {
    //        for (const auto& ai : actions_i) {
    //            actions.push_back(ae + "," + ag + "," + ai);
    //        }
    //    }
    //}

    if (true) {
        ofstream os("actions_ctbe.txt");
        if (os) {
            for (const auto& s : actions) {
                os << s << "\n";
            }
        }
    }

    labeling.InitActions(actions);

    labeling.generate_rules([&](rule_set& rs, uint i) {
        rule_wrapper r(rs, i);

        auto lag = ag;
        for (size_t j = 0; j < lag.size(); ++j) {
            if (((i >> j) & 1) == 0)
                lag.DetachNode(j);
        }

        vector<string> e_actions;
        if (r["e"]) {
            graph cg = MakeConnectivitiesSpecial(lag, vector<string>{"e", "g", "i"});
            connectivity_mat con(rs.conditions);
            con.data_ = cg.arcs_;

            MultiMergeSet mse(con, vector<string>({ "e", "g", "i" }), string("e"));
            mse.BuildMergeSet();
            for (const auto& s : mse.mergesets_) {
                string action = "e<-";
                if (s.empty())
                    action += "newlabel";
                else {
                    action += s[0];
                    for (size_t i = 1; i < s.size(); ++i)
                        action += "+" + s[i];
                }
                e_actions.push_back(action);
            }
        }
        else {
            e_actions.push_back("nothing");
        }

        vector<string> g_actions;
        if (r["g"]) {
            graph cg = MakeConnectivitiesSpecial(lag, vector<string>{"g", "i"});
            connectivity_mat con(rs.conditions);
            con.data_ = cg.arcs_;
            MultiMergeSet msg(con, vector<string>({ "g", "i" }), string("g"));
            msg.BuildMergeSet();

            for (const auto& s : msg.mergesets_) {
                string action = "g<-";
                if (s.empty())
                    action += "newlabel";
                else {
                    action += s[0];
                    for (size_t i = 1; i < s.size(); ++i)
                        action += "+" + s[i];
                }
                g_actions.push_back(action);
            }
        }
        else {
            g_actions.push_back("nothing");
        }

        vector<string> i_actions;
        if (r["i"]) {
            graph cg = MakeConnectivitiesSpecial(lag, vector<string>{"i"});
            connectivity_mat con(rs.conditions);
            con.data_ = cg.arcs_;
            MultiMergeSet msi(con, vector<string>({ "i" }), string("i"));
            msi.BuildMergeSet();

            for (const auto& s : msi.mergesets_) {
                string action = "i<-";
                if (s.empty())
                    action += "newlabel";
                else {
                    action += s[0];
                    for (size_t i = 1; i < s.size(); ++i)
                        action += "+" + s[i];
                }
                i_actions.push_back(action);
            }
        }
        else {
            i_actions.push_back("nothing");
        }

        // Costruisco il set finale di azioni dove ogni azioni sar� del tipo e<-..,g<-..,i<-..
        vector<string> actions;
        for (const auto& ae : e_actions) {
            for (const auto& ag : g_actions) {
                for (const auto& ai : i_actions) {
                    std::string es = ae, gs = ag, is = ai;
                    size_t found = es.find(gs.substr(3));
                    if (found != std::string::npos && es != "nothing" && es != "e<-newlabel") {
                        gs = "g<-e";
                    }
                    found = es.find(is.substr(3));
                    if (found != std::string::npos && es != "nothing" && es != "e<-newlabel") {
                        is = "i<-e";
                    }
                    found = gs.find(is.substr(3));
                    if (found != std::string::npos && gs != "nothing" && gs != "g<-newlabel") {
                        is = "i<-g";
                    }
                    r << es + "," + gs + "," + is;
                }
            }
        }

    });

    return labeling;
}