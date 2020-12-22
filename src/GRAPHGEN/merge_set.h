// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_MERGE_SET_H_
#define GRAPHGEN_MERGE_SET_H_

#include <set>

#include "connectivity_mat.h"

struct MergeSet {
    std::set<std::vector<std::string>> mergesets_;
    connectivity_mat &con_;

    MergeSet(connectivity_mat &con) : con_{ con } {}

    void ReduceMergeSet(std::vector<std::string>& ms) {
        for (size_t i = 0; i < ms.size(); ++i) {
            for (size_t j = i + 1; j < ms.size(); ) {
                if (con_(ms[i], ms[j])) {
                    // remove j-th element
                    ms.erase(begin(ms) + j);
                }
                else {
                    // move next
                    ++j;
                }
            }
        }
    }

    void ExpandAllEquivalences(std::vector<std::string> ms, size_t pos) {
        if (pos >= ms.size()) {
            sort(begin(ms), end(ms));
            mergesets_.emplace(ms);
        }
        else {
            std::string cur = ms[pos];
            auto N = con_.data_.size();
            for (size_t i = 0; i < N; ++i) {
                std::string h = con_.GetHeader(i);
                if (h != "x" && con_(cur, h)) {
                    ms[pos] = h;
                    ExpandAllEquivalences(ms, pos + 1);
                }
            }
        }
    }

    void BuildMergeSet() {
        std::vector<std::string> ms;
        // Create initial merge set
        auto N = con_.data_.size();
        for (size_t i = 0; i < N; ++i) {
            std::string h = con_.GetHeader(i);
            if (h != "x" && con_("x", h)) {
                ms.push_back(h);
            }
        }
        ReduceMergeSet(ms);
        ExpandAllEquivalences(ms, 0);
    }
};

struct MultiMergeSet {
    std::set<std::vector<std::string>> mergesets_;
    connectivity_mat &con_;
    std::vector<std::string> pixel_list_;
    std::string x_pixel_;

    MultiMergeSet(connectivity_mat &con, const std::vector<std::string> &pixel_list, const std::string &x_pixel) : con_{ con }, pixel_list_{ pixel_list }, x_pixel_{ x_pixel } {}

    void ReduceMergeSet(std::vector<std::string>& ms) {
        for (size_t i = 0; i < ms.size(); ++i) {
            for (size_t j = i + 1; j < ms.size(); ) {
                if (con_(ms[i], ms[j])) {
                    // remove j-th element
                    ms.erase(begin(ms) + j);
                }
                else {
                    // move next
                    ++j;
                }
            }
        }
    }

    // Check if the pixel j is part of the set of pixels to be labelled in the mask (to_be_labeled_pixels)
    bool IsInThePixelList(const std::string &j) {
        for (size_t i = 0; i < pixel_list_.size(); ++i) {
            if (j == pixel_list_[i]) {
                return true;
            }
        }
        return false;
    }

    void ExpandAllEquivalences(std::vector<std::string> ms, size_t pos) {
        if (pos >= ms.size()) {
            sort(begin(ms), end(ms));
            mergesets_.emplace(ms);
        }
        else {
            std::string cur = ms[pos];
            auto N = con_.data_.size();
            for (size_t i = 0; i < N; ++i) {
                std::string h = con_.GetHeader(i);
                if (!IsInThePixelList(h) && con_(cur, h)) {
                    ms[pos] = h;
                    ExpandAllEquivalences(ms, pos + 1);
                }
            }
        }
    }

    void BuildMergeSet() {
        std::vector<std::string> ms;
        // Create initial merge set
        auto N = con_.data_.size();
        for (size_t i = 0; i < N; ++i) {
            std::string h = con_.GetHeader(i);
            if (!IsInThePixelList(h) && con_(x_pixel_, h)) {
                ms.push_back(h);
            }
        }
        ReduceMergeSet(ms);
        ExpandAllEquivalences(ms, 0);
    }
};

#endif // !GRAPHGEN_MERGE_SET_H_

