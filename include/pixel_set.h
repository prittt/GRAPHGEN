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

#ifndef GRAPHSGEN_PIXEL_SET_H_
#define GRAPHSGEN_PIXEL_SET_H_

#include <algorithm>
#include <cassert>
#include <fstream>
#include <string>
#include <vector>

struct pixel {
    std::vector<int> coords_;
    std::string name_;

    pixel(std::string name, std::vector<int> coords) : name_{ std::move(name) }, coords_{ std::move(coords) } {}

    auto size() const { return coords_.size(); }
    auto& operator[](size_t i) { return coords_[i]; }
    auto& operator[](size_t i) const { return coords_[i]; }

	// Metodi provvisori per adattare il pixel al vecchio codice specifico per il 2D
	int GetDx() const { return coords_[0]; }
	int GetDy() const { return coords_[1]; }
};

// This function return the ChebyshevDistance between the two pixels p1 and p2. 
// The Chebyshev distance between two vectors is the greatest of their differences
// along any coordinate dimension.
static int ChebyshevDistance(const pixel& p1, const pixel& p2)
{
    assert(p1.size() == p2.size());
    int max = 0;
    for (size_t i = 0; i < p1.size(); ++i) {
        max = std::max(max, abs(p1[i] - p2[i]));
    }
    return max;
}

struct pixel_set {
    std::vector<pixel> pixels_;
    std::vector<uint8_t> shifts_;

    pixel_set() {}
    pixel_set(std::initializer_list<pixel> il) : pixels_{ il } {
        shifts_.resize(pixels_.front().size());
        for (size_t i = 0; i < pixels_.front().size(); ++i)
            shifts_[i] = 1;
    }

    void SetShifts(std::vector<uint8_t> shifts) {
		assert(shifts.size() == shifts_.size() && "'shifts' vector size cannot be changed"); 
		shifts_ = shifts;
    };

    auto& operator[](size_t i) { return pixels_[i]; }
    auto& operator[](size_t i) const { return pixels_[i]; }

    auto size() const { return pixels_.size(); }

    auto begin() { return std::begin(pixels_); }
    auto end() { return std::end(pixels_); }
    auto begin() const { return std::begin(pixels_); }
    auto end() const { return std::end(pixels_); }

	// Metodi provvisori per adattare il pixel_set al vecchio codice specifico per il 2D
	int GetShift2D() const { return shifts_[0]; }
};

#endif //GRAPHSGEN_PIXEL_SET_H_
