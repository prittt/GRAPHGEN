// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

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