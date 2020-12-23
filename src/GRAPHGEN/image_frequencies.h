// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_IMAGE_FREQUENCIES_H_
#define GRAPHGEN_IMAGE_FREQUENCIES_H_

#include <filesystem>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "rule_set.h"
#include "config_data.h"

struct mask {
    cv::Mat1b mask_;
    int top_ = 0, bottom_ = 0, left_ = 0, right_ = 0;
    int border_ = 0;
    int exp_;
    int increment_ = 0;
	const rule_set& rs_;

	mask(const rule_set& rs);
    size_t MaskToLinearMask(const cv::Mat1b& r_img) const;
};

//void CalculateConfigurationsFrequencyOnImage(const cv::Mat1b& img, const mask &msk, rule_set &rs);
bool GetBinaryImage(const std::string &FileName, cv::Mat1b& binary);
bool LoadFileList(std::vector<std::pair<std::string, bool>>& filenames, const std::string& files_path);
//bool CalculateRulesFrequencies(const pixel_set& ps, std::vector<std::pair<std::filesystem::path, bool>>& paths, rule_set& rs);
//void CalculateRulesFrequencies(const pixel_set &ps, const std::vector<std::string> &paths, rule_set &rs);
bool AddFrequenciesToRuleset(rule_set& rs, bool force = false, bool is_thinning = false);

#endif // !GRAPHGEN_IMAGE_FREQUENCIES_H_
