#pragma once

#include <vector>
#include <map>
#include <string>

#include "pixel_set.h"

struct graph {
    std::vector<std::string> nodes_;        // Index -> Name
    std::map<std::string, size_t> rnodes_;  // Name -> Index
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
    void detach_node(size_t i) {
        for (size_t j = 0; j < size(); ++j) {
            arcs_[i][j] = arcs_[j][i] = 0;
        }
    }
    void detach_node(const std::string& name) { detach_node(rnodes_[name]); }
};

graph make_adjacencies(const pixel_set& ps);

graph make_connectivities(const graph& ag);

std::ostream& operator<<(std::ostream& os, const graph& g);

std::vector<std::string> generate_all_possible_labeling_actions(const graph& ag);
