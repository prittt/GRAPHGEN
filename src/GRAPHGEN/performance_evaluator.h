// Copyright(c) 2016 - 2018
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

#ifndef GRAPHGEN_PERFORMANCE_EVALUATOR_H_
#define GRAPHGEN_PERFORMANCE_EVALUATOR_H_

#include <map>
#include <chrono>

class PerformanceEvaluator {
    using hrc = std::chrono::high_resolution_clock;
    using tp = hrc::time_point;

    struct Elapsed {
        double last = 0, total = 0;
    };

    tp last_;

public:
    void start() {
        last_ = hrc::now();
    }
    double stop() {
        auto cur = hrc::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(cur - last_).count();
        double t = static_cast<double>(duration);
        counter_.last = t;
        counter_.total += t;
        return counter_.last;
    }

    void reset() { counter_.total = 0; }
    double last() const { return counter_.last; }
    double total() const { return counter_.total; }

    void store(const std::string& s, double time /*milliseconds*/) {
        counters_[s].last = time;
        counters_[s].total += time;
    }
    double get(const std::string& s) {
        return counters_.at(s).last;
    }
    bool find(const std::string& s) {
        return counters_.find(s) != counters_.end();
    }

private:
    Elapsed counter_;
    std::map<std::string, Elapsed> counters_;
};

#endif // !GRAPHGEN_PERFORMANCE_EVALUATOR_H_