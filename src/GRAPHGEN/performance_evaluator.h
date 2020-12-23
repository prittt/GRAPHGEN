// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

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