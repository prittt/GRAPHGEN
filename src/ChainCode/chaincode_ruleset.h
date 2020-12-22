// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_ROSENFELD_RULESET_H_
#define GRAPHGEN_ROSENFELD_RULESET_H_

#include <string>

#include "graphgen.h"

class ChainCodeRS : public BaseRuleSet {

public:

  using BaseRuleSet::BaseRuleSet;

	// ChainCode ruleset is always read from file and never generated

	ChainCodeRS() : BaseRuleSet(conf.chaincode_rstable_path_) {	}

    rule_set GenerateRuleSet()
    {
		return rule_set();
    }

};

#endif // GRAPHGEN_ROSENFELD_RULESET_H_