// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_CONNECTIVITY_MAT_H_
#define GRAPHGEN_CONNECTIVITY_MAT_H_

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

/*
connectivity_mat stores a matrix which tells if two pixels/blocks are connected, as an intermediate
step to choose which actions should be performed during connected components labeling.
*/
struct connectivity_mat {
    std::vector<std::vector<int>> data_; // connectivity matrix
    std::map<std::string, size_t> pos_;  // inverse lookup table (from pixel names to matrix indexes)
    std::vector<std::string> names_;     // list of pixel names (ordered as in the connectivity matrix)

    // A connectivity matrix is constructed from a list of pixel names
    connectivity_mat(const std::vector<std::string> &names) : names_{ names }, data_{ names.size(), std::vector<int>(names.size(), 0) } {
        auto N = data_.size();
        for (size_t i = 0; i < N; ++i) {
            data_[i][i] = 1; // by definition every pixel is connected to itself
            pos_[names[i]] = i; // initialize the inverse look up table
        }
    }

    // tells if two pixels (specified by names) are connected 
    bool operator()(const std::string& row, const std::string& col) const {
        size_t r = pos_.at(row);
        size_t c = pos_.at(col);
        return data_[r][c];
    }

    // sets the connection of two pixels (specified by names)
    void set(const std::string& row, const std::string& col, bool b) {
        size_t r = pos_.at(row);
        size_t c = pos_.at(col);
        data_[r][c] = data_[c][r] = b;
    }

    // gives back the name of a row/column
    const std::string& GetHeader(size_t i) {
        auto N = data_.size();
        assert(i < N);
        return names_[i];
    }

    // prints an undelimited list of pixel names
    void DisplayCondNames(std::ostream &os = std::cout) {
        std::copy(std::begin(names_), std::end(names_), std::ostream_iterator<std::string>(os));
        os << "\n";
    }

    // prints a visual representation of the matrix
    void DisplayMap(std::ostream & os = std::cout) {
        auto N = data_.size();
        for (size_t c = 0; c < N; ++c) {
            os << "\t" << names_[c];
        }
        os << "\n";

        for (size_t r = 0; r < N; ++r) {
            os << names_[r];
            for (size_t c = 0; c < N; ++c) {
                os << "\t" << data_[pos_.at(GetHeader(r))][pos_.at(GetHeader(c))];
            }
            os << "\n";
        }
    }
};

#endif // !GRAPHGEN_CONNECTIVITY_MAT_H_
