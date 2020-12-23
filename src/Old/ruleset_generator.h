// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_RULESET_GENERATOR_H_
#define GRAPHGEN_RULESET_GENERATOR_H_

#include "rule_set.h"

#define RSG_ALGO RSG(rosenfeld) RSG(rosenfeld_3d) RSG(bbdt) RSG(chen) RSG(ctbe) RSG(thin_zs) RSG(thin_gh) RSG(thin_ch) RSG(thin_hscp)

#define RSG(a) rule_set generate_##a();
RSG_ALGO
#undef RSG

#define RSG(a) a, 
enum class ruleset_generator_type { RSG_ALGO };
#undef RSG

#define RSG(a) #a, 
static std::string ruleset_generator_names[] = { RSG_ALGO };
#undef RSG

#define RSG(a) generate_##a, 
static rule_set(*ruleset_generator_functions[])(void) = { RSG_ALGO };
#undef RSG

#endif // !GRAPHGEN_RULESET_GENERATOR_H_
