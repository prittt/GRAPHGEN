// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_CONDITION_ACTION_H_
#define GRAPHGEN_CONDITION_ACTION_H_

#include <string>
#include <vector>
#include <bitset>


using uint = uint32_t;

// Condition or action
struct conact {
    enum class type { CONDITION, ACTION };

    type t;
    // CONDITION
    std::string condition;
    // ACTION
    std::bitset</*11881*/128> action = 0; // List of actions (bitmapped)
    size_t next = 0;

    conact() {}
    conact(std::string c) : t(type::CONDITION), condition(std::move(c)) {}
    conact(uint a, uint n) : t(type::ACTION), action(a), next(n) {}

    std::vector<uint> actions() const {
        std::vector<uint> a;
        auto uAction = action;
        uint nAction = 1;
        while (uAction != 0) {
            if (uAction[0])
                a.push_back(nAction);
            uAction >>= 1;
            nAction++;
        }
        return a;
    }

    // To check if two conact are equal (exactly the same)
    bool operator==(const conact& other) const {
        if (t != other.t)
            return false;
        if (t == type::CONDITION)
            return condition == other.condition;
        else
            return ((action == other.action) && (next == other.next));
    }
    // To check if two conact are not equal
    bool operator!=(const conact& other) const {
        return !(*this == other);
    }

    // To check if two conact are equivalent (the leaves actions has non empty intersection)
    bool eq(const conact& other) const {
        if (t != other.t)
            return false;
        if (t == type::CONDITION)
            return condition == other.condition;
        else
            return (action & other.action) != 0 && next == other.next;
    }
    // To check if two conact are not equivalent (the leaves actions has empty intersection)
    bool neq(const conact& other) const {
        return !((*this).eq(other));
    }
};

#endif // !GRAPHGEN_CONDITION_ACTION_H_

