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
