// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_CONNECTIVITY_GRAPH_H_
#define GRAPHGEN_CONNECTIVITY_GRAPH_H_


#include <map>
#include <string>
#include <vector>

#include "pixel_set.h"

#include "rule_set.h" // Da rimuove, provvisorio

struct graph {
    std::vector<std::string> nodes_;        // Index -> Name
    std::map<std::string, size_t> rnodes_;  // Name  -> Index
    std::vector<std::vector<int>> arcs_;

    graph(size_t n) : nodes_(n), arcs_(n, std::vector<int>(n)) {}

    auto size() const { return nodes_.size(); }

    auto& operator[](size_t i) { return arcs_[i]; }
    auto& operator[](size_t i) const { return arcs_[i]; }

    void set_name(size_t i, const std::string& name) {
        nodes_[i] = name;
        rnodes_[name] = i;
    }

    // This functions remove the connections between a node and all others
    void DetachNode(size_t i) {
        for (size_t j = 0; j < size(); ++j) {
            arcs_[i][j] = arcs_[j][i] = 0;
        }
    }
    void DetachNode(const std::string& name) { DetachNode(rnodes_[name]); }

    bool Write(const std::string& filename);
};

graph MakeAdjacencies(const pixel_set& ps);

graph MakeConnectivities(const graph& ag);

graph MakeConnectivitiesSpecial(const graph& ag, const std::vector<std::string>& pixel_list);

std::ostream& operator<<(std::ostream& os, const graph& g);



std::vector<std::string> GenerateAllPossibleLabelingActions(const graph& ag);
std::vector<std::string> GenerateAllPossibleLabelingActionsGivenTheSetOfPixelToBeLabeled(const graph& ag, const std::vector<std::string>& to_be_labeled_pixels, rule_set& rs);

std::vector<std::string> GenerateAllPossibleLabelingActions(const graph& ag, const std::string& ref_pixel_name);

#endif // !GRAPHGEN_CONNECTIVITY_GRAPH_H_