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

#ifndef GRAPHSGEN_UTILITIES_H_
#define GRAPHSGEN_UTILITIES_H_

#include <algorithm>
#include <iomanip>
#include <string>
#include <sstream>

#include "performance_evaluator.h"

extern std::string global_output_path;

using uint = uint32_t;

std::string binary(uint u, uint nbits);

struct nodeid {
    int _id = 0;
    int next() { return ++_id; }
    int get() { return _id; }
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

// Function to automatically print a message before and after each operation
// No braces around instruction, so you can log also variable definitions without scoping them
#define LOG(message, instructions) std::cout << (message) << "... "; instructions std::cout << "done.\n"

static PerformanceEvaluator tlog_pe;
#define TLOG(message, instructions) std::cout << (message) << "... "; tlog_pe.start(); instructions std::cout << "done. " << tlog_pe.stop() << " ms.\n";

#endif // !GRAPHSGEN_UTILITIES_H_

