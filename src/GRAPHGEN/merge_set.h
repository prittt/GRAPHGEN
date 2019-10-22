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

    // Controllo se il pixel j fa parte del set di pixel da etichettare nella maschera (to_be_labeled_pixels)
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

