// Copyright(c) 2018
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