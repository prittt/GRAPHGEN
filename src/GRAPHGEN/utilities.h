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

#ifndef GRAPHGEN_UTILITIES_H_
#define GRAPHGEN_UTILITIES_H_

#include <algorithm>
#include <iomanip>
#include <string>
#include <sstream>
#include <filesystem>
#include <bitset>

#include "config_data.h"
#include "performance_evaluator.h"

extern ConfigData conf;

constexpr auto MAX_COMBINED_ACTIONS_COUNT = 0; // 56 is upper limit for block based 3D; all equivalent actions between a 7 and 8 block component: 7*8=56
constexpr auto MAX_ACTION_BITS = 13; // ceil(log2(5813)), with 5813 being the amount of action in block based 3D with 36 conditions

using uchar = unsigned char;
using ushort = uint16_t;
using uint = uint32_t;
using ullong = uint64_t;

class action_bitset {
public:
	static const size_t max_size_in_bits() {
		return 16 * MAX_COMBINED_ACTIONS_COUNT; // because ushort is 16 bit type
	}

private:
	//std::bitset<MAX_COMBINED_ACTIONS_COUNT * MAX_ACTION_BITS> data;
	std::vector<ushort> data_;

public:
	// CONSTRUCTORS
	action_bitset() {
	}
	action_bitset(size_t i) {
		data_.reserve(i);
	}

	// OPERATORS
	const ushort operator[] (const ushort a) const {
		return test(a) ? 1 : 0;
	}

	const ushort getActionByDataIndex(const ushort& index) const {
		return data_[index];
	}

	bool operator==(const action_bitset& rhs) const {
		int my_size = size();
		int their_size = rhs.size();
		if (my_size != their_size) {
			return false;
		}

		int intersection_size = operator&(rhs).size();
		if (intersection_size != my_size) {
			return false;
		}

		return true;
	}

	bool operator!=(const action_bitset& rhs) const {
		return !(operator==(rhs));
	}

	// METHODS

	action_bitset& set(const ushort& a) {
		data_.push_back(a);
		return *this;
	}

	void clear() {
		data_.clear();
	}

	void reserve(size_t i) {
		data_.reserve(i);
	}

	const std::vector<ushort>& getSingleActions() const {
		return data_;
	}

	const bool test(const ushort& action) const {
		for (size_t i = 0; i < size(); i++) {
			if (data_[i] == action) {
				return true;
			} 
			if (data_[i] > action) { // data array is always ordered
				return false;
			}
		}
		return false;
	}

	size_t size() const {
		return data_.size();
	}
	
	action_bitset operator&(const action_bitset& rhs) const {
		action_bitset output;

		for (size_t i = 0; i < size(); i++) {
			for (size_t j = 0; j < rhs.size(); j++) {
				if (data_[i] < rhs.data_[j]) {
					break;
				}
				if (data_[i] == rhs.data_[j]) {
					output.set(data_[i]);
				}
			}			
		}

		return output;
	}

	action_bitset& operator&=(const action_bitset& rhs) {
		action_bitset output = operator&(rhs);
		data_ = output.data_;
		return *this;
	}

	const std::string to_string() const {
		return std::to_string(to_ullong());
	}

	const ullong to_ullong() const {
		ullong output = 0;
		for (size_t i = 0; i < size(); i++) {
			output |= (1ULL << (data_[i]));
		}
		return output;
	}
};


std::string binary(uint u, uint nbits);

/** @brief This macro serves to simplify the definition of flags, it takes the name of an enum class as 
input and defines the operator| for that class.
*/
#define DEFINE_ENUM_CLASS_OR_OPERATOR(class_name)                                                              \
constexpr enum class_name operator|(const enum class_name self_value, const enum class_name in_value) {        \
    return static_cast<enum class_name>(static_cast<uint32_t>(self_value) | static_cast<uint32_t>(in_value));  \
}                                                                                                              \

/** @brief This macro serves to simplify the definition of flags, it takes the name of an enum class as
input and defines the operator& for that class.
*/
#define DEFINE_ENUM_CLASS_AND_OPERATOR(class_name)                                                             \
constexpr bool operator&(const enum class_name self_value, const enum class_name in_value) {                   \
    return static_cast<bool>(static_cast<uint32_t>(self_value) & static_cast<uint32_t>(in_value));             \
}                                                                                                              \

#define DEFINE_ENUM_CLASS_FLAGS(class_name, ...)  \
enum class class_name : uint32_t {                \
    __VA_ARGS__                                   \
};                                                \
DEFINE_ENUM_CLASS_OR_OPERATOR(class_name)         \
DEFINE_ENUM_CLASS_AND_OPERATOR(class_name)        \


struct nodeid {
    int _id = 0;
    int next() { return ++_id; }
    int get() { return _id; }

    void Clear() {
        _id = 0;
    }

    void SetId(int new_id) {
        _id = new_id;
    }
};

static inline void RemoveCharacter(std::string& s, const char c) {
    s.erase(std::remove(s.begin(), s.end(), c), s.end());
}

// Convert value into string filling the head of the string with zeros up to n characters
template <typename T>
std::string zerostr(const T& val, size_t n) {
    std::stringstream ss;
    ss << std::setw(n) << std::setfill('0') << val;
    return ss.str();
}

// This function tokenize an input string
template <typename T>
void StringSplit(const std::string& str, T& cont, char delim = '+')
{
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim)) {
        cont.push_back(token);
    }
}



// Function to automatically print a message before and after each operation
// No braces around instruction, so you can log also variable definitions without scoping them
#define LOG(message, instructions) std::cout << (message) << "... "; instructions std::cout << "done.\n"

//static PerformanceEvaluator tlog_pe;
#define TLOG(message, instructions) std::cout << (message) << "... "; PerformanceEvaluator my_tlog; my_tlog.start(); instructions std::cout << "done. " << my_tlog.stop() << " ms.\n";
#define TLOG2(message, instructions) std::cout << (message) << "... "; PerformanceEvaluator my_tlog2; my_tlog.start(); instructions std::cout << "done. " << my_tlog.stop() << " ms.\n";

#endif // !GRAPHGEN_UTILITIES_H_

