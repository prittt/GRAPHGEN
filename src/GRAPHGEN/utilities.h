// Copyright(c) 2019
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

using uchar = unsigned char;
using ushort = uint16_t;
using uint = uint32_t;
using llong = int64_t;
using ullong = uint64_t;

using Action = ushort;


class action_bitset {
private:
	std::vector<Action> data_;

public:
	// CONSTRUCTORS
	action_bitset() {
	}
	action_bitset(size_t i) {
		data_.reserve(i);
	}

	// OPERATORS
	const int operator[] (const Action a) const {
		return test(a) ? 1 : 0;
	}

	const Action getActionByDataIndex(const int& index) const {
		return data_[index];
	}

	Action& getActionByDataIndex(const int& index) {
		return data_[index];
	}

	bool operator==(const action_bitset& rhs) const {
		size_t my_size = size();
		size_t their_size = rhs.size();
		if (my_size != their_size) {
			return false;
		}

		size_t intersection_size = operator&(rhs).size();
		if (intersection_size != my_size) {
			return false;
		}

		return true;
	}

	bool operator!=(const action_bitset& rhs) const {
		return !(operator==(rhs));
	}

	// METHODS

	action_bitset& set(const Action a) {
		data_.push_back(a);
		return *this;
	}

	void clear() {
		data_.clear();
	}

	void resize(size_t i) {
		data_.resize(i);
	}

	const std::vector<Action>& getSingleActions() const {
		return data_;
	}

	std::vector<Action>& getSingleActions() {
		return data_;
	}

	const bool test(const Action action) const {
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

	// overflows as soon as there is an action >64 in there
	const ullong to_ullong() const {
		ullong output = 0;
		for (size_t i = 0; i < size(); i++) {
			output |= (1ULL << (data_[i]));
		}
		return output;
	}
};


std::string binary(size_t u, size_t nbits);

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
    size_t _id = 0;
    size_t next() { return ++_id; }
    size_t get() { return _id; }

    void Clear() {
        _id = 0;
    }

    void SetId(size_t new_id) {
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

#define TLOG(message, instructions) std::cout << (message) << "... " << std::flush; PerformanceEvaluator my_tlog; my_tlog.start(); instructions; std::cout << "done. " << my_tlog.stop() << " ms.\n";
#define TLOG2(message, instructions) std::cout << (message) << "... " << std::flush; PerformanceEvaluator my_tlog2; my_tlog2.start(); instructions; std::cout << "done. " << my_tlog2.stop() << " ms.\n";
#define TLOG3_START(message) std::cout << (message) << "... " << std::flush; PerformanceEvaluator my_tlog3; my_tlog3.start();
#define TLOG3_STOP std::cout << "done. " << my_tlog3.stop() << " ms.\n";
#define TLOG4_DEF PerformanceEvaluator my_tlog4;
#define TLOG4_START(message) std::cout << (message) << "... " << std::flush; my_tlog4.start();
#define TLOG4_STOP std::cout << "done. " << my_tlog4.stop() << " ms.\n";
#define TLOG5(message, instructions) std::cout << (message) << "... " << std::flush; PerformanceEvaluator my_tlog5; my_tlog5.start(); instructions; std::cout << "done. " << my_tlog5.stop() << " ms.\n";
#define TLOG6(message, instructions) std::cout << (message) << "... " << std::flush; PerformanceEvaluator my_tlog6; my_tlog6.start(); instructions; std::cout << "done. " << my_tlog6.stop() << " ms.\n";

#endif // !GRAPHGEN_UTILITIES_H_

