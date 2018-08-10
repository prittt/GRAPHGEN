// Copyright(c) 2018 Costantino Grana, Federico Bolelli 
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

#ifndef GRAPHSGEN_OUTPUT_GENERATOR_H_
#define GRAPHSGEN_OUTPUT_GENERATOR_H_

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <sstream>

#include "condition_action.h"
#include "tree.h"
#include "utilities.h"
#include "forest.h"

void print_node(tree<conact>::node *n, int i);

// All nodes must have both sons! 
void GenerateDotCodeForDag(std::ostream& os, tree<conact>& t, bool with_next = false);

// "output_file": output file name without extension 
// "t": tree<conact> to draw
// "verbose": to print messages on standard output
// return true if the process ends correctly, false otherwise
bool DrawDagOnFile(const std::string& output_file, tree<conact> &t, bool with_next = false, bool verbose = false);
bool DrawForestOnFile(const std::string& output_file, Forest& f, bool verbose = false);

#endif // !GRAPHSGEN_OUTPUT_GENERATOR_H_