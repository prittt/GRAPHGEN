// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_UTILITIES_H_
#define GRAPHGEN_UTILITIES_H_

#include <algorithm>
#include <iomanip>
#include <string>
#include <sstream>
#include <filesystem>

#include "config_data.h"
#include "performance_evaluator.h"

extern ConfigData conf;

using uint = uint32_t;

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

static PerformanceEvaluator tlog_pe;
#define TLOG(message, instructions) std::cout << (message) << "... "; tlog_pe.start(); instructions std::cout << "done. " << tlog_pe.stop() << " ms.\n";

#endif // !GRAPHGEN_UTILITIES_H_

