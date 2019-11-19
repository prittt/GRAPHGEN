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

#ifndef GRAPHGEN_CONDITION_ACTION_H_
#define GRAPHGEN_CONDITION_ACTION_H_

#include <string>
#include <vector>
#include <bitset>

#include "utilities.h"

using uint = uint32_t;

// Condition or action
struct conact {
    enum class type { CONDITION, ACTION };

    type t;
    // CONDITION
    std::string condition;
    // ACTION
    action_bitset action = 0; // List of actions (bitmapped)
    uint next = 0;

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

