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

#ifndef GRAPHSGEN_OUTPUT_GENERATOR_H_
#define GRAPHSGEN_OUTPUT_GENERATOR_H_

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "condition_action.h"
#include "forest.h"
#include "tree.h"
#include "utilities.h"

/** @brief Flags for the DrawDagOnFile function
*/
enum DrawDagFlags {
    WITH_NEXT      = 1, /**< @brief Whether to print next tree indexes inside leaves or not */
    VERBOSE        = 2, /**< @brief Whether to display output messages or not */
    DELETE_DOTCODE = 4, /**< @brief Whether to delete or not the dot code used to draw the drag */
    WITH_ROOT_ID   = 8, /**< @brief Whether to print root id or not */
};

/** @brief Generate a file displaying the specified DRAG. The output format will be the 
one set in the general yaml configuration file

@param[in] output_file Name that the output file should have (without extension).
@param[in] dt DRAG (BinaryDrag<conact>) to be drawn.
@param[in] flags Function flags, see DrawDagFlags.

@return Whether the operation ended correctly (true) or not (false).
*/
bool DrawDagOnFile(const std::string& output_file,
                   const BinaryDrag<conact> &dt,
                   int flags = DrawDagFlags::DELETE_DOTCODE);


bool DrawForestOnFile(const std::string& output_file, const LineForestHandler& lfh, int flags);

#endif // !GRAPHSGEN_OUTPUT_GENERATOR_H_