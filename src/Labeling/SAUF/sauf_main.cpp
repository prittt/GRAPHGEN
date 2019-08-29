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

#include "yaml-cpp/yaml.h"

#include "graphsgen.h"

#include "rosenfeld_ruleset.h"

using namespace std;

int main()
{
    // Read yaml configuration file
    string config_file = "config.yaml";
    YAML::Node config;
    try {
        config = YAML::LoadFile(config_file);
    }
    catch (...) {
        cout << "ERROR: Unable to read configuration file '" << config_file << "'\n";
        exit(EXIT_FAILURE);
    }

    string algorithm_name = "SAUF";
    global_output_path = filesystem::path(config["paths"]["output"].as<string>()) / filesystem::path(algorithm_name);
    filesystem::create_directories(global_output_path);

    auto rs = GenerateRosenfeld();

    // Call GRAPHSGEN:
    // 1) Load or generate Optimal Decision Tree based on Rosenfeld mask
    ltree t = GetOdt(rs, algorithm_name);

    // 2) Draw the generated tree to pdf
    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, t, false, true, false);

    // 3) Generate the C++ source code for the ODT
    GenerateDragCode(algorithm_name, t);

    // 4) Generate the C++ source code for pointers, 
    // conditions to check and actions to perform
    GenerateConditionsActionsCode(algorithm_name, rs);

    return EXIT_SUCCESS;
}