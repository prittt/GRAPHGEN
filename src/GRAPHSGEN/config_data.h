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

#ifndef GRAPGHSGEN_CONFIG_DATA_H_
#define GRAPGHSGEN_CONFIG_DATA_H_

#include <iostream>

#include "yaml-cpp/yaml.h"

#include "utilities.h"

struct ConfigData {

    // Configuration file
    std::string config_file_ = "config.yaml";

    // Global names/paths
    std::filesystem::path global_output_path_;
    std::string algorithm_name_;
    
    // Dot configurations
    std::string dot_background_color_ = "\"transparent\"";
    std::string dot_output_format_ = ".pdf";

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

    ConfigData() {}
    
    ConfigData(std::string algorithm_name) : algorithm_name_{ algorithm_name } {
        YAML::Node config;
        try {
            config = YAML::LoadFile(config_file_);
        }
        catch (...) {
            std::cout << "ERROR: Unable to read configuration file '" << config_file_ << "'.\n";
            exit(EXIT_FAILURE);
        }

        if (config["paths"]["output"]) {
            global_output_path_ = std::filesystem::path(config["paths"]["output"].as<std::string>()) / std::filesystem::path(algorithm_name);
            std::filesystem::create_directories(global_output_path_);

            // ODT
            odt_path_ = global_output_path_ / std::filesystem::path(algorithm_name + odt_suffix_);

            // Code
            code_path_ = global_output_path_ / std::filesystem::path(algorithm_name + code_suffix_);

            // Rule Set / Decision Table
            rstable_path_ = global_output_path_ / std::filesystem::path(algorithm_name + rstable_suffix_);
            
            // Tree / Forest / Dag (.inc)
            treecode_path_ = global_output_path_ / std::filesystem::path(algorithm_name + treecode_suffix_);
            forestcode_path_ = global_output_path_ / std::filesystem::path(algorithm_name + forestcode_suffix_);
            treedagcode_path_ = global_output_path_ / std::filesystem::path(algorithm_name + treedagcode_suffix_);
            forestdagcode_path_ = global_output_path_ / std::filesystem::path(algorithm_name + forestdagcode_suffix_);
        }
        else {
            std::cout << "ERROR: missing output path in configuration file.\n";
            exit(EXIT_FAILURE);
        }

        if (config["dot"]["background"]) {
            dot_background_color_ = "\"" + config["dot"]["background"].as<std::string>() + "\"";
        }
        else {
            std::cout << "WARNING: missing dot background color, 'transparent' will be used.\n";
        }

        if (config["dot"]["out_format"]) {
            dot_output_format_ = "." + config["dot"]["background"].as<std::string>();
        }
        else {
            std::cout << "WARNING: missing output file format, 'pdf' will be used.\n";
        }
    }

    // DotCode
    std::filesystem::path GetDotCodePath(const std::string& out_base_name) {
        return global_output_path_ / std::filesystem::path(out_base_name + dotcode_suffix_);
    }

    // Output dot
    std::filesystem::path GetDotOutPath(const std::string& out_base_name) {
        return global_output_path_ / std::filesystem::path(out_base_name + dot_output_format_);
    }
};

#endif // GRAPGHSGEN_CONFIG_DATA_H_
