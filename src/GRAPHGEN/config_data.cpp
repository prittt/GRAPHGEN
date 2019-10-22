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

#include "config_data.h"

#include <algorithm>

using namespace std;
using namespace filesystem;

ConfigData::ConfigData(string algorithm_name) : algorithm_name_{ algorithm_name } {
    YAML::Node config;
    try {
        config = YAML::LoadFile(config_file_);
    }
    catch (...) {
        cout << "ERROR: Unable to read configuration file '" << config_file_ << "'.\n";
        exit(EXIT_FAILURE);
    }

    if (config["paths"]["output"]) {
        global_output_path_ = path(config["paths"]["output"].as<string>()) / path(algorithm_name);
        create_directories(global_output_path_);

        // ODT
        odt_path_ = global_output_path_ / path(algorithm_name + odt_suffix_);

        // Code
        code_path_ = global_output_path_ / path(algorithm_name + code_suffix_);

        // Rule Set / Decision Table
        rstable_path_ = global_output_path_ / path(algorithm_name + rstable_suffix_);

        // Tree / Forest / Dag (.inc) // TODO the following variables are probably useless
        treecode_path_ = global_output_path_ / path(algorithm_name + treecode_suffix_);
        forestcode_path_ = global_output_path_ / path(algorithm_name + forestcode_suffix_);
        treedagcode_path_ = global_output_path_ / path(algorithm_name + treedagcode_suffix_);
        forestdagcode_path_ = global_output_path_ / path(algorithm_name + forestdagcode_suffix_);

        // Frequencies
        frequencies_path_ = global_output_path_ / path(algorithm_name + frequencies_suffix_);
    }
    else {
        cout << "ERROR: missing output path in configuration file.\n";
        exit(EXIT_FAILURE);
    }

    if (config["paths"]["input"]) {
        global_input_path_ = path(config["paths"]["input"].as<string>());
        if (config["datasets"]) {
            datasets_ = config["datasets"].as<vector<string>>();
            //for (const string& dataset : datasets_) {
            //    datasets_path_.emplace_back(global_input_path_ / path(dataset));
            //}
            datasets_path_.resize(datasets_.size());
            generate(datasets_path_.begin(), datasets_path_.end(), [this, datasets_it = datasets_.begin()]() mutable { return global_input_path_ / path(*datasets_it++); });
        }
    }

    if (config["dot"]["background"]) {
        dot_background_color_ = "\"" + config["dot"]["background"].as<string>() + "\"";
    }
    else {
        cout << "WARNING: missing dot background color, 'transparent' will be used.\n";
    }

    if (config["dot"]["out_format"]) {
        dot_output_format_ = "." + config["dot"]["out_format"].as<string>();
    }
    else {
        cout << "WARNING: missing output file format, 'pdf' will be used.\n";
    }

    if (config["use_frequencies"]) {
        use_frequencies_ = config["use_frequencies"].as<bool>();
    }

    if (config["force_odt_generation"]) {
        force_odt_generation_ = config["force_odt_generation"].as<bool>();
    }

    if (config["force_frequencies_count"]) {
        force_frequencies_count_ = config["force_frequencies_count"].as<bool>();
    }
    
}