// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "drag_statistics.h"

#include <iostream>

void PrintStats(const BinaryDrag<conact>& bd, std::ostream& os) {
    BinaryDragStatistics bds(bd);
    os << "nodes:" << bds.Nodes() << "; leaves = " << bds.Leaves() << "\n";
}