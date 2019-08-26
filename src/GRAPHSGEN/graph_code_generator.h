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

#ifndef GRAPHSGEN_GRAPH_CODE_GENERATOR_H_
#define GRAPHSGEN_GRAPH_CODE_GENERATOR_H_

#include <ostream>

#include "conact_tree.h"
#include "forest.h"
#include "rule_set.h"

/** @brief Generate the C++ code for the given DRAG (Directed Rooted Acyclic Graph). 

This function works only when all nodes of the DRAG have both left and right child!

@param[in] algorithm_name Name of the algorithm for which the code must be generated, it is used to name the output file.
@param[in] t Tree for which to generate the C++ code.

@return Whether the operation ended correctly (true) or not (false).
*/
bool GenerateDragCode(const std::string& algorithm_name, ltree& t);

// This function generates forest code. TODO: check this!!
int GenerateForestCode(std::ostream& os, const Forest& f, std::string prefix = "", int start_id = 0);

#endif // GRAPHSGEN_GRAPH_CODE_GENERATOR_H_