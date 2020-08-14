// Copyright(c) 2020
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

#ifndef GRAPHGEN_HYPERCUBEPP_H_
#define GRAPHGEN_HYPERCUBEPP_H_

#include <algorithm>
#include <cassert>
#include <iostream>

#include "conact_tree.h"
#include "rule_set.h"

namespace hyper {

template <typename T>
std::istream& rawread(std::istream& is, T& val, size_t n) {
    return is.read(reinterpret_cast<char*>(&val), n);
}
template <typename T>
std::ostream& rawwrite(std::ostream& os, const T& val, size_t n) {
    return os.write(reinterpret_cast<const char*>(&val), n);
}

class HyperCube {
    size_t GetIndexWithIndifference(size_t value, size_t indif) {
        size_t index = 0;
        int vbits = 0;
        for (size_t pos = 0; pos < nbits_; ++pos) {
            if ((indif >> pos) & 1) {
                index += 2 * pow3_[pos];
            }
            else {
                index += ((value >> vbits) & 1)*pow3_[pos];
                ++vbits;
            }
        }
        return index;
    }

    size_t GetIndex(size_t value) {
        size_t index = 0;
        for (int pos = 0; pos < nbits_; ++pos) {
            index += ((value >> pos) & 1)*pow3_[pos];
        }
        return index;
    }

    void CreateTreeRec(BinaryDrag<conact>& t, BinaryDrag<conact>::node *n, size_t idx) const;

public:

#pragma pack(push)
#pragma pack(1)
    struct Node {
        std::bitset</*11881*/128> actions_ = 0;
        unsigned long long frequency_ = 1;
        unsigned long long gain_ = 0;
        uint8_t max_gain_index_ = 0;
        unsigned int num_equiv_ = 0;
    };
#pragma pack(pop)

    size_t nbits_;
    std::vector<Node> data_;
    const rule_set& rs_;
    std::vector<size_t> pow3_;

    HyperCube(const rule_set& rs) 
        : rs_(rs), nbits_(rs.conditions.size()), 
        data_(size_t(pow(3.0, rs.conditions.size()))), pow3_(rs.conditions.size())
    {
        // Initialize vector of powers of 3
        pow3_[0] = 1;
        for (size_t i = 1; i < nbits_; ++i) {
            pow3_[i] = pow3_[i - 1] * 3;
        }

        // Initialize hypercube nodes using the rules defined in the ruleset
        auto nrules = rs.rules.size();
        for (size_t i = 0; i < nrules; ++i) {
            // for each rule generate the hypercube index
            size_t idx = GetIndex(i);
            // and set its values
            data_[idx].frequency_ = rs.rules[i].frequency;
            data_[idx].actions_ = rs.rules[i].actions;
        }
    }

    std::istream& read(std::istream& is) {
        return rawread(is, data_[0], data_.size() * sizeof(Node));
    }
    std::ostream& write(std::ostream& os) {
        return rawwrite(os, data_[0], data_.size() * sizeof(Node));
    }

    Node& operator[](size_t idx) { return data_[idx]; }
    const Node& operator[](size_t idx) const { return data_[idx]; }

    BinaryDrag<conact> Optimize();
};

// Generates an Optimal Decision Tree from the given rule_set,
// and store it in the filename when specified.
BinaryDrag<conact> GenerateOdt(const rule_set& rs);
BinaryDrag<conact> GenerateOdt(const rule_set& rs, const std::string& filename);

/** @brief Returns the optimal (or pseudo optimal) decision tree generated from the given rule set

This function generates the optimal decision tree from the given rule set. When the number
of rules is too high, a pseudo optimal tree is generated. If the tree has already been generated, it
is loaded from file, unless the "force_generation" parameter is set to true. In this case the tree
is always regenerated. The loaded/generated tree is then returned from the function.

@param[in] rs Rule set from which generate the decision tree.
@param[in] force_generation Whether the tree must be generated or can be loaded from file.

@return The optimal decision tree associated to the specified rule set.
*/
BinaryDrag<conact> GetOdt(const rule_set& rs, bool force_generation = false);


/** @brief Returns the optimal (or pseudo optimal) decision tree generated from the given rule set

This function generates the optimal decision tree from the given rule set. When the number
of rules is too high, a pseudo optimal tree is generated. If the tree has already been generated, it
is loaded from file, unless the "force_generation" parameter is set to true. In this case the tree
is always regenerated. The loaded/generated tree is then returned from the function.

@param[in] rs Rule set from which generate the decision tree.
@param[in] file_suffix Suffix that is appended to the file name of the decision tree file.
@param[in] force_generation Whether the tree must be generated or can be loaded from file.

@return The optimal decision tree associated to the specified rule set.
*/
BinaryDrag<conact> GetOdtWithFileSuffix(const rule_set& rs, const std::string& file_suffix, bool force_generation = false);

}

#endif // !GRAPHGEN_HYPERCUBEPP_H_