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

#include "image_frequencies.h"

#include <iostream>
#include <limits>

#include "utilities.h"

using namespace std;

// This function extracts all the configurations of a given mask (msk) in a give image (img) and store the occourences (frequencies) in the rRules vector
void CalculateConfigurationsFrequencyOnImage(const cv::Mat1b& img, const mask &msk, rule_set &rs) {

    cv::Mat1b clone;
    copyMakeBorder(img, clone, msk.border_, msk.border_, msk.border_, msk.border_, cv::BORDER_CONSTANT, 0);
    const int h = clone.rows, w = clone.cols;

    for (int r = msk.border_; r < h - msk.border_; r += msk.increment_) {
        for (int c = msk.border_; c < w - msk.border_; c += msk.increment_) {
            cv::Mat1b read_pixels = clone(cv::Rect(cv::Point(c - msk.left_, r - msk.top_), cv::Point(c + 1 + msk.right_, r + 1 + msk.bottom_))).clone();
            bitwise_and(msk.mask_, read_pixels, read_pixels);
            size_t rule = msk.MaskToLinearMask(read_pixels);
            rs.rules[rule].frequency++;
            if (rs.rules[rule].frequency == std::numeric_limits<unsigned long long>::max()) {
                std::cout << "OVERFLOW freq\n";
            }
        }
    }
}

bool GetBinaryImage(const std::string &FileName, cv::Mat1b& binary) {
    // Image load
    cv::Mat image;
    image = cv::imread(FileName, cv::IMREAD_GRAYSCALE);   // Read the file

    if (image.empty()) // Check if image exist
        return false;

    //// Convert the image to grayscale
    //Mat grayscaleMat;
    //cvtColor(image, grayscaleMat, CV_RGB2GRAY);

    // Adjust the threshold to actually make it binary
    cv::threshold(image, binary, 100, 1, cv::THRESH_BINARY);

    return true;
}

bool LoadFileList(std::vector<std::pair<std::string, bool>>& filenames, const std::string& files_path)
{
    // Open files_path (files.txt)
    ifstream is(files_path);
    if (!is.is_open()) {
        return false;
    }

    string cur_filename;
    while (getline(is, cur_filename)) {
        // To delete possible carriage return in the file name
        // (especially designed for windows file newline format)
        RemoveCharacter(cur_filename, '\r');
        filenames.push_back(make_pair(cur_filename, true));
    }

    is.close();
    return true;
}

void CalculateRulesFrequencies(const pixel_set &ps, const vector<string> &paths, rule_set &rs) {
    mask msk(ps);

    cv::Mat1b img;

    for (uint i = 0; i < paths.size(); ++i) {
        vector<pair<string, bool>> files_list;
        if (!LoadFileList(files_list, paths[i] + "//files.txt")) {
            cout << "Unable to find 'files.txt' of " << paths[i] << ", dataset skipped";
            continue;
        }

        for (uint d = 0; d < files_list.size(); ++d) {
            GetBinaryImage(paths[i] + "//" + files_list[d].first, img);
            if (img.empty()) {
                cout << "Unable to find '" << files_list[d].first << "' image in '" << paths[i] << "' dataset, image skipped\n";
                continue;
            }
            CalculateConfigurationsFrequencyOnImage(img, msk, rs);
        }
    }
}
