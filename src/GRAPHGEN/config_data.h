// Copyright(c) 2018 - 2019 Federico Bolelli, Costantino Grana
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

#ifndef GRAPGHSGEN_CONFIG_DATA_H_
#define GRAPGHSGEN_CONFIG_DATA_H_

#include <iostream>
#include <filesystem>

#include "yaml-cpp/yaml.h"

struct ConfigData {

    // Configuration file
    std::string config_file_ = "config.yaml";

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

    ConfigData(std::string algorithm_name, std::string mask_name, bool use_frequencies = false);

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

    // This serves to use a special algorithm name when using frequencies
    void UpdateAlgoNameWithDatasets() {
        // Add names of used data sets to file names
        std::string dataset_names;
        bool first = true;
        for (const auto &d : datasets_) {
            if (!first) {
                dataset_names += '-';
            }
            else {
                first = false;
            }
            dataset_names += d;
        }
        algorithm_name_ += "_" + dataset_names;
    }

};

#endif // GRAPGHSGEN_CONFIG_DATA_H_
