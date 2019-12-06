// Copyright(c) 2019 Maximilian Söchting 
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

#ifndef GRAPHGEN_HEURISTICS_H_
#define GRAPHGEN_HEURISTICS_H_

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>

#include "conact_tree.h"
#include "rule_set.h"
#include "base_ruleset.h"

// Generates an Optimal Decision Tree from the given rule_set,
// and store it in the filename when specified.
BinaryDrag<conact> GenerateHdt(const rule_set& rs, const BaseRuleSet& brs);
BinaryDrag<conact> GenerateHdt(const rule_set& rs, const BaseRuleSet& brs, const std::string& filename);

/** @brief Returns a decision tree generated with heuristics from the given rule set

This function generates a decision tree using heuristics from the given rule set. If the tree has 
already been generated, it is loaded from file, unless the "force_generation" parameter is set to true. 
In this case the tree is always regenerated. The loaded/generated tree is then returned from the function.

@param[in] rs Rule set from which generate the decision tree.
@param[in] force_generation Whether the tree must be generated or can be loaded from file.

@return The optimal decision tree associated to the specified rule set.
*/
BinaryDrag<conact> GetHdt(const rule_set& rs, const BaseRuleSet& brs, bool force_generation = false);

#endif // !GRAPHSGEN_HEURISTICS_H_
