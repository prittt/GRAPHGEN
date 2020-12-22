// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPGHSGEN_CONFIG_DATA_H_
#define GRAPGHSGEN_CONFIG_DATA_H_

#include <filesystem>
#include <iostream>
#include <numeric>

#include "yaml-cpp/yaml.h"

/** @brief This class stores the configuration data loaded from file. All data are placed 
in the config global variable that can be accessed anywhere in the code.

*/
struct ConfigData { 

    std::string description_ = ""; /**< Algorithm's description */

    std::string config_file_ = "config.yaml"; /**< Name of the configuration file */

    // Global names/paths
    std::filesystem::path global_output_path_;
    std::filesystem::path algorithm_output_path_;
    std::filesystem::path global_input_path_;
    std::string algorithm_name_;
    std::string mask_name_;

    // Dot configurations
    std::string dot_background_color_ = "\"transparent\"";
    std::string dot_output_format_ = ".pdf";
    std::string dot_ranksep_ = "0.5";


    // Dot Code
    std::string dotcode_suffix_ = "_dotcode.txt";
    std::filesystem::path dotcode_path_;

    // ODT
    std::string odt_suffix_ = "_odt.txt";
    std::filesystem::path odt_path_;

    // Code
    std::string code_suffix_ = "_code.cpp";
    std::filesystem::path code_path_;

    // Tree / Forest / Dag (.inc)
    std::string treecode_suffix_ = "_tree_code.inc.h";
    std::filesystem::path treecode_path_;
    std::string forestcode_suffix_ = "_forest_code.inc.h";
    std::filesystem::path forestcode_path_;

    std::string treedagcode_suffix_ = "_tree_dag_code.inc.h";
    std::filesystem::path treedagcode_path_;
    std::string forestdagcode_suffix_ = "_forest_dag_code.inc.h";
    std::filesystem::path forestdagcode_path_;

    // Rule Set / Decision Table
    std::string rstable_suffix_ = "_rstable.yaml";
    std::filesystem::path rstable_path_;

    // Datasets
    std::vector<std::string> datasets_;
    std::vector<std::filesystem::path> datasets_path_;

    // Frequencies
    std::string frequencies_local_path_ = "frequencies";
    std::filesystem::path frequencies_path_;
    std::string frequencies_suffix_ = ".bin";

	// Chaincode Ruleset Path
	std::filesystem::path chaincode_rstable_path_;
	std::string chaincode_rstable_filename_ = "ChainCode_rstable.yaml";

	bool force_odt_generation_ = false;

    ConfigData() {}

    ConfigData(std::string& algorithm_name, const std::string& mask_name, bool use_frequencies = false);

    // Dot code
    std::filesystem::path GetDotCodePath(const std::string& out_base_name) {
        return algorithm_output_path_ / std::filesystem::path(out_base_name + dotcode_suffix_);
    }

    // Dot output
    std::filesystem::path GetDotOutPath(const std::string& out_base_name) {
        return algorithm_output_path_ / std::filesystem::path(out_base_name + dot_output_format_);
    }

    // Forest code
    std::filesystem::path GetForestCodePath(const std::string& out_base_name) {
        return algorithm_output_path_ / std::filesystem::path(algorithm_name_ + "_" + out_base_name + forestcode_suffix_);
    }

    // Frequencies cache
    std::filesystem::path GetFrequenciesPath(const std::string& datasets_name) const {
        return algorithm_output_path_ / std::filesystem::path(algorithm_name_ + "_" + datasets_name + frequencies_suffix_);
    }

    // Get ODT path with custom suffix (used in conjunction with frequencies)
    std::filesystem::path GetCustomOdtPath(const std::string& custom_suffix) const {
        return algorithm_output_path_ / std::filesystem::path(algorithm_name_ + "_" + custom_suffix + odt_suffix_);
    }

    std::string GetDatasetsString(const std::string& separator = ", ") {
        std::string dataset_names;
        bool first = true;
        for (const auto &d : datasets_) {
            if (!first) {
                dataset_names += separator;
            }
            else {
                first = false;
            }
            dataset_names += d;
        }
        return dataset_names;
    }

    // This serves to use a special algorithm name when using frequencies
    void UpdateAlgoNameWithDatasets() {
        algorithm_name_ += "_" + GetDatasetsString("-");
    }

    void SetDescription(std::string description) {
        description_ = description;
    }

};

#endif // GRAPGHSGEN_CONFIG_DATA_H_
