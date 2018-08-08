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

#ifndef GRAPHSGEN_CONNECTIVITY_MAT_H_
#define GRAPHSGEN_CONNECTIVITY_MAT_H_

#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

#include <cassert>

/*
connectivity_mat stores a matrix which tells if two pixels/blocks are connected, as an intermediate
step to choose which actions should be performed during connected components labeling.
*/
template<size_t N>
struct connectivity_mat {
    bool data_[N][N] = { 0 }; // connectivity matrix
    std::map<std::string, size_t> pos_; // inverse lookup table (from pixel names to matrix indexes)
    std::vector<std::string> names_; // list of pixel names (ordered as in the connectivity matrix)

    // a connectivity matrix is constructed from a list of pixel names
    connectivity_mat(const std::vector<std::string> &names) : names_{ names } {
        assert(names.size() == N);
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

#endif // !GRAPHSGEN_CONNECTIVITY_MAT_H_
