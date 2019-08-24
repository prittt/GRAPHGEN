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

//#include <set>
//#include <algorithm>
//#include <fstream>
//#include <unordered_map>
//#include <iterator>
//#include <iomanip>
//#include <string>
//#include <vector>
//#include <cstdint>
//#include <map>
//#include <unordered_map>
//#include <unordered_set>

//#include <intrin.h>

//#include "condition_action.h"
//#include "code_generator.h"
//#include "file_manager.h"
//#include "forest2dag.h"
//#include "forest_optimizer.h"
//#include "forest_statistics.h"
//#include "drag_statistics.h"
//#include "drag2optimal.h"
//#include "hypercube.h"
//#include "output_generator.h"
//#include "ruleset_generator.h"
//#include "tree2dag_identities.h"
//#include "utilities.h"

#include <opencv2/imgproc.hpp>

#include "graphsgen.h"
#include "yaml-cpp/yaml.h"

using namespace std;

rule_set generate_SAUF()
{
    pixel_set rosenfeld_mask{
        { "p", {-1, -1} }, { "q", {0, -1} }, { "r", {+1, -1} },
        { "s", {-1, +0} }, { "x", {0, +0} },
    };

    rule_set labeling;
    labeling.InitConditions(rosenfeld_mask);

    graph ag = MakeAdjacencies(rosenfeld_mask);
    ag.Write("rosenfeld_adjacencies_graph.txt");
    auto actions = GenerateAllPossibleLabelingActions(ag);

    //{
    //    ofstream os("actions_rosenfeld.txt");
    //    if (os) {
    //        for (const auto& s : actions) {
    //            os << s << "\n";
    //        }
    //    }
    //}

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

int main()
{
    // Read yaml configuration file
    const string config_file = "config.yaml";
    YAML::Node config;
    try {
         config = YAML::LoadFile("config.yaml");
    }
    catch (const cv::Exception&) {
        // TODO add log messages
        exit(EXIT_FAILURE);  
    }

    global_output_path = string(config["paths"]["output"].as<string>());

    string algorithm_name = "SAUF";

    auto rs = generate_SAUF();

    // Call GRAPHSGEN
    string odt_filename = global_output_path.string() + "/" + algorithm_name + "_odt.txt";
    ltree t;
    if (!LoadConactTree(t, odt_filename)) {
        t = GenerateOdt(rs, odt_filename);
    }

    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, t, false, true);

    GenerateCode(algorithm_name, t);

    GenerateConditionsActionsCode(algorithm_name, rs);

	return EXIT_SUCCESS;
}