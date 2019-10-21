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
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "yaml-cpp/yaml.h"

#include "utilities.h"

struct pixel {
	std::vector<int> coords_;
	std::string name_;

    pixel() {}
    pixel(YAML::Node& node) {
        Deserialize(node);
    }
	pixel(std::string name, std::vector<int> coords) : name_{ std::move(name) }, coords_{ std::move(coords) } {}

	auto size() const { return coords_.size(); }
	auto& operator[](size_t i) { return coords_[i]; }
	auto& operator[](size_t i) const { return coords_[i]; }

	// Metodi provvisori per adattare il pixel al vecchio codice specifico per il 2D
	int GetDx() const { return coords_[0]; }
	int GetDy() const { return coords_[1]; }

	// It works only on coords_
	bool operator==(const pixel& rhs) const {
		assert(rhs.size() == coords_.size() && "Something wrong with pixel's coordinates");
		for (size_t i = 0; i < coords_.size(); ++i) {
			if (coords_[i] != rhs[i]) {
				return false;
			}
		}
		return true;
	}

	void ShiftX(int s) {
		coords_[0] += s;
	}

    YAML::Node Serialize() const {
        YAML::Node pixel_node;
        pixel_node["name"] = name_;

        for (const auto& c : coords_) {
            pixel_node["coords"].push_back(std::to_string(c));
        }

        return pixel_node;
    }

    void Deserialize(YAML::Node& node) {
        name_ = node["name"].as<std::string>();
        for (unsigned i = 0; i < node["coords"].size(); ++i) {
            coords_.push_back(node["coords"][i].as<int>());
        }
    }
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

    pixel_set() 
    {}
    pixel_set(YAML::Node& ps_node) {
        Deserialize(ps_node);
    }
    pixel_set(std::initializer_list<pixel> il, std::vector<uint8_t> shifts) : pixels_{ il }, shifts_{std::move(shifts)}
    {}
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

	// Search predicate to apply find algorithm on vector of pixels 
	struct find_pixel
	{
		std::string name_;
		find_pixel(std::string name) : name_(name) {}
		bool operator () (const pixel& p) const
		{
			return p.name_ == name_;
		}
	};

	auto& operator[](const std::string& s) { return *std::find_if(pixels_.begin(), pixels_.end(), find_pixel(s)); }
	auto& operator[](const std::string& s) const { return *std::find_if(pixels_.begin(), pixels_.end(), find_pixel(s)); }

    auto size() const { return pixels_.size(); }

    auto begin() { return std::begin(pixels_); }
    auto end() { return std::end(pixels_); }
    auto begin() const { return std::begin(pixels_); }
    auto end() const { return std::end(pixels_); }

	// Metodi provvisori per adattare il pixel_set al vecchio codice specifico per il 2D
	int GetShiftX() const { return shifts_[0]; }

    YAML::Node Serialize() const {
        YAML::Node ps_node;

        for (const auto& p : pixels_) {
            ps_node["pixels"].push_back(p.Serialize());
        }

        for (const auto& s : shifts_) {
            ps_node["shifts"].push_back(static_cast<int>(s));
        }
        return ps_node;
    }

    void Deserialize(YAML::Node& node) {
        for (unsigned i = 0; i < node["shifts"].size(); ++i) {
            shifts_.push_back(node["shifts"][i].as<int>());
        }

        for (unsigned i = 0; i < node["pixels"].size(); ++i) {
            pixels_.push_back(pixel(node["pixels"][i]));
        }
    }

    //friend std::ostream& operator<<(std::ostream& os, const pixel_set& ps);
    //friend std::istream& operator>>(std::istream& is, pixel_set& ps);
};

//inline std::ostream& operator<<(std::ostream& os, const pixel_set& ps) {
//    os << "pixels: ";
//    for (unsigned i = 0; i < ps.pixels_.size(); ++i) {
//        os << ps.pixels_[i];
//        if (i < ps.pixels_.size() - 1) {
//            os << "; ";
//        }
//    }
//    os << "\nshifts: ";
//    for (unsigned i = 0; i < ps.shifts_.size(); ++i) {
//        os << (int)ps.shifts_[i];
//        if (i < ps.shifts_.size() - 1) {
//            os << ", ";
//        }
//    }
//    os << "\n";
//    return os;
//}
//
//inline std::istream& operator>>(std::istream& is, pixel_set& ps) {
//    std::string pixels_id = "pixels:";
//    std::string ps_line; 
//    getline(is, ps_line);
//    if (ps_line.substr(0, pixels_id.size()) != pixels_id) {
//        throw std::runtime_error("Bad Rule Set File Format!");
//    }
//
//    std::vector<std::string> pixels_str;
//    StringSplit(ps_line, pixels_str, ';');
//
//    ps.pixels_ = std::vector<pixel>(pixels_str.size());
//    for (unsigned i = 0; i < pixels_str.size(); ++i) {
//        
//        unsigned begin_n = pixels_str[i].find_first_of("\"");
//        unsigned end_n   = pixels_str[i].find_last_of("\"");
//
//        ps.pixels_[i].name_ = pixels_str[i].substr(begin_n + 1, end_n - begin_n - 1);
//        pixels_str[i] = pixels_str[i].substr(end_n);
//
//        std::vector<std::string> coords;
//        unsigned begin_p = pixels_str[i].find_first_of("{");
//        unsigned end_p   = pixels_str[i].find_first_of("}");
//        pixels_str[i] = pixels_str[i].substr(begin_p + 1, end_p - begin_p - 1);
//        StringSplit(pixels_str[i], coords, ',');
//        
//        for (unsigned j = 0; j < coords.size(); ++j) {
//            ps.pixels_[i].coords_.push_back(std::stoi(coords[j]));
//        }
//    }
//
//    return is;
//}

#endif //GRAPHSGEN_PIXEL_SET_H_
