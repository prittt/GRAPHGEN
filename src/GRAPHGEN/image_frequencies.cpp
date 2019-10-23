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
#include <filesystem>
#include <algorithm>
#include <iterator>

#include "utilities.h"
#include "performance_evaluator.h"

using namespace std;
using namespace filesystem;
using namespace cv;

mask::mask(const pixel_set& ps) {
    increment_ = ps.GetShiftX();
    exp_ = ps.pixels_.size();
    for (int i = 0; i < exp_; ++i) {
        border_ = max(border_, max(abs(ps.pixels_[i].GetDx()), abs(ps.pixels_[i].GetDy())));
        top_ = min(top_, ps.pixels_[i].GetDy());
        right_ = max(right_, ps.pixels_[i].GetDx());
        left_ = min(left_, ps.pixels_[i].GetDx());
        bottom_ = max(bottom_, ps.pixels_[i].GetDy());
    }

    left_ = abs(left_);
    top_ = abs(top_);

    mask_ = Mat1b(top_ + bottom_ + 1, left_ + right_ + 1, uchar(0));
    for (int i = 0; i < exp_; ++i) {
        mask_(ps.pixels_[i].GetDy() + top_, ps.pixels_[i].GetDx() + left_) = 1;
    }
}

size_t mask::MaskToLinearMask(const cv::Mat1b r_img) const {
    size_t linearMask = 0;

    for (int r = 0; r < r_img.rows; ++r) {
        for (int c = 0; c < r_img.cols; ++c) {
            linearMask <<= (1 & mask_(r, c));
            linearMask |= (r_img(r, c) & mask_(r, c));
        }
    }
    return linearMask;
}

// This function extracts all the configurations of a given mask (mask) in a given image (img) and stores the occurrences (frequencies) in the rRules vector
void CalculateConfigurationsFrequencyOnImage(const cv::Mat1b& img, const mask& msk, rule_set& rs) {

    cv::Mat1b clone;
    copyMakeBorder(img, clone, msk.border_, msk.border_, msk.border_, msk.border_, cv::BORDER_CONSTANT, 0);
    const int h = clone.rows, w = clone.cols;

    for (int r = msk.border_; r < h - msk.border_; r += msk.increment_) {
        for (int c = msk.border_; c < w - msk.border_; c += msk.increment_) {
            cv::Mat1b read_pixels = clone(cv::Rect(cv::Point(c - msk.left_, r - msk.top_), cv::Point(c + 1 + msk.right_, r + 1 + msk.bottom_))).clone();
            bitwise_and(msk.mask_, read_pixels, read_pixels);
            size_t rule = msk.MaskToLinearMask(read_pixels);
            rs.rules[rule].frequency++;
            if (rs.rules[rule].frequency == numeric_limits<unsigned long long>::max()) {
                cout << "OVERFLOW freq\n";
            }
        }
    }
}

bool GetBinaryImage(const string& FileName, cv::Mat1b& binary) {
    // Image load
    cv::Mat image;
    image = cv::imread(FileName, cv::IMREAD_GRAYSCALE);   // Read the file

    if (image.empty()) // Check if image exists
        return false;

    //// Convert the image to grayscale
    //Mat grayscaleMat;
    //cvtColor(image, grayscaleMat, CV_RGB2GRAY);

    // Adjust the threshold to actually make it binary
    cv::threshold(image, binary, 100, 1, cv::THRESH_BINARY);

    return true;
}

bool LoadFileList(vector<pair<string, bool>>& filenames, const string& files_path)
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

bool CalculateRulesFrequencies(const pixel_set& ps, vector<pair<path, bool>>& paths, rule_set& rs) {
    mask msk(ps);

    cv::Mat1b img;

    cout << "Counting frequencies of patterns in datasets . . . \n";

    PerformanceEvaluator perf;
    perf.start();

    unsigned int existing_datasets = 0;
    for (uint i = 0; i < paths.size(); ++i) {
        path dataset_path = paths[i].first;
        vector<pair<string, bool>> files_list;
        if (!LoadFileList(files_list, (dataset_path / path("files.txt")).string())) {
            cout << "Unable to find 'files.txt' of " << dataset_path << ", dataset skipped.\n";
            paths[i].second = false;
            continue;
        }
        cout << dataset_path.filename().string() << ":\n";

        unsigned int files_list_size = files_list.size();
        for (uint d = 0; d < files_list_size; ++d) {
            cout << '\r' << d << '/' << files_list_size;
            path file_name = files_list[d].first;
            GetBinaryImage((dataset_path / file_name).string(), img);
            if (img.empty()) {
                cout << "Unable to find '" << file_name << "' image in '" << dataset_path << "' dataset, image skipped\n";
                continue;
            }
            CalculateConfigurationsFrequencyOnImage(img, msk, rs);
        }
        cout << '\r' << files_list_size << '/' << files_list_size << '\n';
        existing_datasets++;
    }

    cout << "done. " << perf.stop() << " ms.\n";

    return existing_datasets > 0;
}

//void CalculateRulesFrequencies(const pixel_set& ps, const vector<string>& paths, rule_set& rs) {
//    vector<path> datasets_path(paths.size());
//    generate(datasets_path.begin(), datasets_path.end(), [paths_it = paths.begin()]() mutable { return path(*paths_it++); });
//    CalculateRulesFrequencies(ps, datasets_path, rs);
//}

bool AddFrequenciesToRuleset(const ConfigData& config, rule_set& rs, bool force) {
   
    std::string dataset_names;
    bool first = true;
    for (const auto &piece : conf.datasets_) {
        if (!first) {
            dataset_names += '-';
        }
        else {
            first = false;
        }
        dataset_names += piece;
    }

    if (!force) {
        ifstream is;
        is.exceptions(fstream::badbit | fstream::failbit | fstream::eofbit);

        try {
            is.open(config.GetFrequenciesPath(dataset_names), ios::binary);
            vector<rule> new_rules = rs.rules;
            unsigned int n;
            is.read(reinterpret_cast<char*>(&n), 4);
            if (config.datasets_.size() != n) {
                throw runtime_error("Number of datasets in stored file doesn't match that of datasets in config.\n");
            }
            set<string> stored_datasets;
            for (unsigned int i = 0; i < n; i++) {
                string buf;
                char c;
                while (true) {
                    is.get(c);
                    if (c == '\n') {
                        break;
                    }
                    else {
                        buf += c;
                    }
                }
                stored_datasets.insert(buf);
            }
            if (set<string>(config.datasets_.begin(), config.datasets_.end()) != stored_datasets) {
                throw runtime_error("Datasets in stored file and don't match those in config.\n");
            }
            for_each(new_rules.begin(), new_rules.end(), [&is](rule& r) { is.read(reinterpret_cast<char*>(&r.frequency), 8); });
            rs.rules = new_rules;
            cout << "Frequencies were loaded from file.\n";
            return true;
        }
        catch (const ifstream::failure&) {
            cout << "Frequencies couldn't be loaded from file.\n";
        }
        catch (const runtime_error&) {
            cout << "Frequencies couldn't be loaded from file.\n";
        }
    }

    if (config.datasets_path_.empty()) {
        cerr << "Frequencies couldn't be counted because no input dataset is specified.\n";
        return false;
    }

    vector<pair<path, bool>> existing_datasets(config.datasets_path_.size());
    generate(existing_datasets.begin(), existing_datasets.end(), [it = config.datasets_path_.begin()]() mutable {return make_pair(*it++, true); });

    if (!CalculateRulesFrequencies(rs.ps_, existing_datasets, rs)) {
        cerr << "Couldn't count frequencies.\n";
        return false;
    }

    // Store frequencies into file
    // Format is the following
    // - n:             little endian 4 bytes unsigned integer, stores the number of datasets considered
    // - datasets:      a sequence of n datasets names, each of them ending with a '\n'
    // - frequencies:   the array of frequencies stored as little endian 8 bytes unsigned integers
    ofstream os(config.GetFrequenciesPath(dataset_names), ios::binary);
    if (!os) {
        cerr << "Frequencies couldn't be stored into file.\n";
    }
    else {
        unsigned int n = 0;
        for_each(existing_datasets.begin(), existing_datasets.end(), [&n](const pair<path, bool>& dataset) {n += dataset.second; });
        os.write(reinterpret_cast<const char*>(&n), 4);
        for (const pair<path, bool>& dataset : existing_datasets) {
            if (dataset.second) {
                string dataset_name = dataset.first.filename().string();
                os.write(dataset_name.c_str(), dataset_name.size() * sizeof(char));
                os.put('\n');
            }
        }
        for_each(rs.rules.begin(), rs.rules.end(), [&os](const rule& r) { os.write(reinterpret_cast<const char*>(&r.frequency), 8); });
    }

    return true;
}