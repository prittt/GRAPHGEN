// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "config_data.h"

#include <algorithm>

using namespace std;
using namespace filesystem;

ConfigData::ConfigData(string& algorithm_name, const string& mask_name, bool use_frequencies) : algorithm_name_{ algorithm_name }, mask_name_{ mask_name } {
    YAML::Node config;
    try {
        config = YAML::LoadFile(config_file_);
    }
    catch (...) {
        cout << "ERROR: Unable to read configuration file '" << config_file_ << "'.\n";
        exit(EXIT_FAILURE);
    }

    if (config["paths"]["input"]) {
        global_input_path_ = path(config["paths"]["input"].as<string>());
        if (config["datasets"]) {
            datasets_ = config["datasets"].as<vector<string>>();
            datasets_path_.resize(datasets_.size());
            generate(datasets_path_.begin(), datasets_path_.end(), [this, datasets_it = datasets_.begin()]() mutable { return global_input_path_ / path(*datasets_it++); });
        }
    }

    if (use_frequencies) {
        UpdateAlgoNameWithDatasets();
        algorithm_name = algorithm_name_;
    }

    if (config["paths"]["output"]) {

        global_output_path_ = path(config["paths"]["output"].as<string>());
        algorithm_output_path_ = global_output_path_ / path(algorithm_name);
        create_directories(algorithm_output_path_);

        // ODT
        odt_path_ = algorithm_output_path_ / path(algorithm_name + odt_suffix_);

        // Code
        code_path_ = algorithm_output_path_ / path(algorithm_name + code_suffix_);

        // Rule Set / Decision Table
        rstable_path_ = algorithm_output_path_ / path(algorithm_name + rstable_suffix_);

        // Tree / Forest / Dag (.inc)
        treecode_path_ = algorithm_output_path_ / path(algorithm_name + treecode_suffix_);
        forestcode_path_ = algorithm_output_path_ / path(algorithm_name + forestcode_suffix_);
        treedagcode_path_ = algorithm_output_path_ / path(algorithm_name + treedagcode_suffix_);
        forestdagcode_path_ = algorithm_output_path_ / path(algorithm_name + forestdagcode_suffix_);

        // Frequencies
        frequencies_path_ = global_output_path_ / frequencies_local_path_;
        create_directories(frequencies_path_ / mask_name_);

		// ChainCode Ruleset Path
		chaincode_rstable_path_ = global_output_path_ / path(chaincode_rstable_filename_);
    }
    else {
        cout << "ERROR: missing output path in configuration file.\n";
        exit(EXIT_FAILURE);
    }

    if (config["dot"]["background"]) {
        dot_background_color_ = "\"" + config["dot"]["background"].as<string>() + "\"";
    }
    else {
        cout << "WARNING: missing dot background color,  " + dot_background_color_ + " will be used.\n";
    }

    if (config["dot"]["ranksep"]) {
        dot_ranksep_ = config["dot"]["ranksep"].as<std::string>();
    }
    else {
        std::cout << "WARNING: missing dot ranksep, " + dot_ranksep_ + " will be used.\n";
    }

    if (config["dot"]["out_format"]) {
        dot_output_format_ = "." + config["dot"]["out_format"].as<string>();
    }
    else {
        cout << "WARNING: missing output file format, 'pdf' will be used.\n";
    }

    if (config["force_odt_generation"]) {
        force_odt_generation_ = config["force_odt_generation"].as<bool>();
    }
}