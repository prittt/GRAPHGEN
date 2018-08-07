#pragma once

#include <set>

#include "connectivity_mat.h"

template <size_t N>
struct MergeSet {
    std::set<std::vector<std::string>> mergesets_;
    connectivity_mat<N> &con_;

    MergeSet(connectivity_mat<N> &con) : con_{ con } {}

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
            string cur = ms[pos];
            for (size_t i = 0; i < N; ++i) {
                string h = con_.GetHeader(i);
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
        for (size_t i = 0; i < N; ++i) {
            string h = con_.GetHeader(i);
            if (h != "x" && con_("x", h)) {
                ms.push_back(h);
            }
        }
        ReduceMergeSet(ms);
        ExpandAllEquivalences(ms, 0);
    }
};

