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

#ifndef GRAPHGEN_DRAG_STATISTICS_H_
#define GRAPHGEN_DRAG_STATISTICS_H_

#include <iostream>
#include <set>

#include "conact_tree.h"

/** @brief Calculates the statistics of a binary drag with one or multiple roots.

The statistics are basically the number of unique nodes an unique leaves inside the
binary drag. Nodes() and Leaves() member functions allows to access the calculated
statistics.

*/
class BinaryDragStatistics {
    std::set<const BinaryDrag<conact>::node*> visited_nodes;
    std::set<const BinaryDrag<conact>::node*> visited_leaves;

    void PerformStatistics(const BinaryDrag<conact>::node *n) {
        if (n->isleaf()) {
            visited_leaves.insert(n);
            return;
        }

        if (visited_nodes.insert(n).second) {
            PerformStatistics(n->left);
            PerformStatistics(n->right);
        }
    }

public:
    /** @brief The constructor creates the object and directly calculates the statistics.

    @param [in] bd BinaryDrag on which calculate the statistics.
    */
    BinaryDragStatistics(const BinaryDrag<conact>& bd) {
        for (const auto& t : bd.roots_) {
            PerformStatistics(t);
        }
    }

    /** @brief Returns the number of unique nodes inside the DRAG.

    @return number of unique nodes
    */
    auto Nodes() const { return visited_nodes.size(); }

    /** @brief Returns the number of unique leaves inside the DRAG.

    @return number of unique leaves
    */
    auto Leaves() const { return visited_leaves.size(); }

    /** @brief Reverse into the specified output stream the unique leaves of a BinaryDrag.

    @param[in] os Output stream where to write the leaves. Default value is std::cout.

    @return 
    */
    void PrintLeaves(std::ostream& os = std::cout) {
        for (const auto& l : visited_leaves) {
            if (l->isleaf()) {
                for (const auto& a : l->data.actions()) {
                    os << a << " ";
                }
                os << "- " << l->data.next << "\n";
            }
        }
        os << "----------------------------------\n";
    }
};

/** @brief Displays on the specified output stream  the statistics of a DRAG (number of unique nodes an unique leaves).

@param [in] bd BinaryDrag<conact> on which calculate statistics.
@param [in] os Output stream on which reverse the statistics of bd.
*/
void PrintStats(const BinaryDrag<conact>& bd, std::ostream& os = std::cout);

#endif // !GRAPHGEN_DRAG_STATISTICS_H_