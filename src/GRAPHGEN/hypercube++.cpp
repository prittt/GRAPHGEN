// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "hypercube++.h"

#include "utilities.h"

using namespace std;

namespace hyper {

void HyperCube::CreateTreeRec(BinaryDrag<conact>& t, BinaryDrag<conact>::node *n, size_t idx) const {
    const Node& node = data_[idx];
    if (node.actions_ == 0) {
        n->data.t = conact::type::CONDITION;
        n->data.condition = rs_.conditions[node.max_gain_index_];

        size_t pow3 = pow3_[node.max_gain_index_];
        size_t idx1 = idx - pow3;
        size_t idx0 = idx1 - pow3;

        CreateTreeRec(t, n->left = t.make_node(), idx0);
        CreateTreeRec(t, n->right = t.make_node(), idx1);
    }
    else {
        n->data.t = conact::type::ACTION;
        n->data.action = node.actions_;
    }
}

std::string BinaryWithIndifference(size_t value, size_t indif, size_t nbits) {
    std::string s(nbits, '0');
    size_t vbits = 0;
    for (size_t pos = 0; pos < nbits; ++pos) {
        if ((indif >> pos) & 1) {
            s[nbits - 1 - pos] = '-';
        }
        else {
            s[nbits - 1 - pos] = ((value >> vbits) & 1) + 48;
            ++vbits;
        }
    }
    return s;
}

#ifdef _MSC_VER
static inline int __builtin_ctz(unsigned x) {
    unsigned long ret;
    _BitScanForward(&ret, x);
    return (int)ret;
}
#endif

//#define HYPERCUBE_VERBOSE
BinaryDrag<conact> HyperCube::Optimize()
{
#ifdef HYPERCUBE_VERBOSE
    // Print the table
    for (size_t i = 0; i < 1ull << nbits_; ++i) {
        size_t idx = GetIndex(i);
        std::cout << BinaryWithIndifference(i, 0, nbits_) << "\t" << data_[idx].frequency_ << "\t";
        if (data_[idx].actions_ == 0) {
            std::cout << "0";
        }
        else {
            for (size_t i = 1; i < 128; i++) {
                if (data_[idx].actions_[i - 1]) {
                    std::cout << i << ",";
                }
            }
        }
        std::cout << "\n";
    }
    std::cout << "------------------------\n";
#endif

    for (size_t num_indif = 1; num_indif <= nbits_; num_indif++) {
        #ifndef HYPERCUBE_VERBOSE
            std::cout << num_indif << " " << std::flush;
        #endif
        // Initialize the Indifferences
        int indif = (1 << num_indif) - 1;
        int last = indif << (nbits_ - num_indif);

        // Do all permutations of indifferences
        while (true) {
            // use permutation

            int max_value = 1 << (nbits_ - num_indif);
            for (int i = 0; i < max_value; ++i) {
                #ifdef HYPERCUBE_VERBOSE
                vector<Node> nodes_;
                #endif

                size_t idx = GetIndexWithIndifference(i, indif);
                int tmp_indif = indif;
                int pos_indif = 0;
                Node& max_gain_node = data_[idx];
                while (tmp_indif>0) { // there are more indifferences to check
                    if (tmp_indif & 1) { // this is and indifference
                        size_t pow3 = pow3_[pos_indif];
                        size_t idx1 = idx - pow3;
                        size_t idx0 = idx1 - pow3;

                        Node& node0 = data_[idx0], node1 = data_[idx1];

                        Node cur_node;
                        cur_node.actions_ = node0.actions_ & node1.actions_;
                        cur_node.frequency_ = node0.frequency_ + node1.frequency_;
                        cur_node.gain_ = node0.gain_ + node1.gain_;
                        cur_node.max_gain_index_ = pos_indif;
                        if (cur_node.actions_ != 0) {
                            cur_node.gain_ += cur_node.frequency_;
                            cur_node.num_equiv_ = 0;
                        }
                        else {
                            cur_node.num_equiv_ = node0.num_equiv_ * node1.num_equiv_;
                        }

                        #ifdef HYPERCUBE_VERBOSE
                        nodes_.push_back(cur_node);
                        #endif

                        if (max_gain_node.gain_ <= cur_node.gain_) {
                            if (max_gain_node.gain_ == cur_node.gain_) {
                                cur_node.num_equiv_ += max_gain_node.num_equiv_;
                            }
                            max_gain_node = cur_node;
                        }
                    }

                    ++pos_indif;
                    tmp_indif >>= 1;
                }
                max_gain_node.num_equiv_ = std::max(max_gain_node.num_equiv_, 1u);

                #ifdef HYPERCUBE_VERBOSE
                std::cout << BinaryWithIndifference(i, indif, nbits_) << "\t" << data_[idx].frequency_ << "\t";
                if (data_[idx].actions_ == 0) {
                    std::cout << "0";
                }
                else {
                    for (size_t i = 1; i < 128; i++) {
                        if (data_[idx].actions_[i - 1]) {
                            std::cout << i << ",";
                        }
                    }
                }
                std::cout << "\t";
                for (const auto& x : nodes_) {
                    std::cout << x.gain_;
                    if (x.max_gain_index_ == max_gain_node.max_gain_index_)
                        std::cout << "*";
                    else if (x.gain_ == max_gain_node.gain_)
                        std::cout << "#";
                    std::cout << "\t";
                }
                std::cout << max_gain_node.num_equiv_ << "\n";
                #endif
            }
            #ifdef HYPERCUBE_VERBOSE
            std::cout << "\n";
            #endif

            // check if last
            if (indif == last)
                break;

            // next permutation (https://graphics.stanford.edu/~seander/bithacks.html#NextBitPermutation)
            int t = indif | (indif - 1);
            indif = (t + 1) | (((~t & -~t) - 1) >> (__builtin_ctz(indif) + 1));
        } 
        #ifdef HYPERCUBE_VERBOSE
            std::cout << "------------------------\n";
        #endif
    }

    BinaryDrag<conact> t;
    CreateTreeRec(t, t.make_root(), data_.size() - 1);
    return t;
}

BinaryDrag<conact> GenerateOdt(const rule_set& rs) {
    TLOG("Allocating hypercube",
        HyperCube hcube(rs);
    );

    TLOG("Optimizing rules",
        auto t = hcube.Optimize();
    );

    return t;
}

BinaryDrag<conact> GenerateOdt(const rule_set& rs, const string& filename)
{
    auto t = GenerateOdt(rs);
    WriteConactTree(t, filename);
    return t;
}

BinaryDrag<conact> GetOdt(const rule_set& rs, bool force_generation) {
    string odt_filename = conf.odt_path_.string();
    BinaryDrag<conact> t;
    if (conf.force_odt_generation_ || force_generation || !LoadConactTree(t, odt_filename)) {
        t = GenerateOdt(rs, odt_filename);
    }
    return t;
}

BinaryDrag<conact> GetOdtWithFileSuffix(const rule_set& rs, const string& file_suffix, bool force_generation) {
    string odt_filename = conf.GetCustomOdtPath(file_suffix).string();
    BinaryDrag<conact> t;
    if (conf.force_odt_generation_ || force_generation || !LoadConactTree(t, odt_filename)) {
        t = GenerateOdt(rs, odt_filename);
    }
    return t;
}

}