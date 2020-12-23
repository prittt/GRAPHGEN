// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "utilities.h"

ConfigData conf;

std::string binary(size_t u, size_t nbits) {
    std::string s;
    while (nbits-- > 0)
        s += ((u >> nbits) & 1) + 48;
    return s;
}
