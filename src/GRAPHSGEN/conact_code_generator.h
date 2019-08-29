// Copyright(c) 2018 - 2019 Costantino Grana, Federico Bolelli 
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
// * Neither the name of GRAPHSGEN nor the names of its
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

#ifndef GRAPHSGEN_CONACT_CODE_GENERATOR_H_
#define GRAPHSGEN_CONACT_CODE_GENERATOR_H_

#include <ostream>
#include <optional>

#include "conact_tree.h"
#include "forest.h"
#include "rule_set.h"

// This function generates code for conditions and actions' macros and rows' pointers
//void GenerateConditionsActionsCode(std::ofstream& os, const rule_set& rs);
//// Overloading function
// names contains the position in the labels image corresponding to the names used in labeling actions. 
// It is necessary to handle blocks names and defaults to mask pixel set if not provided. 
bool GeneratePointersConditionsActionsCode(const std::string& algorithm_name, 
                                           const rule_set& rs, 
                                           std::optional<pixel_set> names = std::nullopt);

//bool GenerateActionsForCtbe(const std::string& filename, const rule_set& rs);

#endif // GRAPHSGEN_CONACT_CODE_GENERATOR_H_