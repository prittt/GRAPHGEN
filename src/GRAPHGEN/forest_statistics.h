// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_FOREST_STATISTICS_H_
#define GRAPHGEN_FOREST_STATISTICS_H_

#include <set>

#include "forest.h"

//class ForestStatistics {
//    std::set<const BinaryDrag<conact>::node*> visited_nodes;
//    std::set<const BinaryDrag<conact>::node*> visited_leaves;
//
//    std::set<const BinaryDrag<conact>::node*> visited_end_nodes;
//    std::set<const BinaryDrag<conact>::node*> visited_end_leaves;
//
//    void PerformStatistics(const BinaryDrag<conact>::node *n) {
//        if (n->isleaf()) {
//            visited_leaves.insert(n);
//            return;
//        }
//
//        if (visited_nodes.insert(n).second) {
//            PerformStatistics(n->left);
//            PerformStatistics(n->right);
//        }
//    }
//
//    void PerformEndStatistics(const BinaryDrag<conact>::node *n) {
//        if (n->isleaf()) {
//            visited_end_leaves.insert(n);
//            return;
//        }
//
//        if (visited_end_nodes.insert(n).second) {
//            PerformEndStatistics(n->left);
//            PerformEndStatistics(n->right);
//        }
//    }
//
//public:
//    ForestStatistics(const LineForestHandler& f) {
//        // Internal trees statistics
//        for (const auto& t : f.trees_) {
//            PerformStatistics(t.GetRoot());
//        }
//
//        // End trees statistics
//        for (const auto& g : f.end_trees_) {
//            for (const auto& t : g) {
//                PerformEndStatistics(t.GetRoot());
//            }
//        }
//    }
//
//    auto nodes() const { return visited_nodes.size(); }
//    auto leaves() const { return visited_leaves.size(); }
//    auto end_nodes() const { return visited_end_nodes.size(); }
//    auto end_leaves() const { return visited_end_leaves.size(); }
//};
//
//void PrintStats(const LineForestHandler& f);


#endif // !GRAPHGEN_FOREST_STATISTICS_H_