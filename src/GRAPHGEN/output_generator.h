// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_OUTPUT_GENERATOR_H_
#define GRAPHGEN_OUTPUT_GENERATOR_H_

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
DEFINE_ENUM_CLASS_FLAGS(DrawDagFlags, 
    NONE           = 0,        /**< @brief No flags */
    WITH_NEXT      = (1 << 0), /**< @brief Whether to print next tree indexes inside leaves or not */
    VERBOSE        = (1 << 1), /**< @brief Whether to display output messages or not */
    DELETE_DOTCODE = (1 << 2), /**< @brief Whether to delete or not the dot code used to draw the drag */
    WITH_ROOT_ID   = (1 << 3), /**< @brief Whether to print root id or not */
)

/** @brief Generate a file displaying the specified DRAG. The output format will be the 
one set in the general yaml configuration file

@param[in] base_filename Name that the output file should have (without extension).
@param[in] dt DRAG (BinaryDrag<conact>) to be drawn.
@param[in] flags Function flags, see DrawDagFlags.

@return Whether the operation ended correctly (true) or not (false).
*/
bool DrawDagOnFile(const std::string& base_filename,
                   const BinaryDrag<conact> &dt,
                   DrawDagFlags flags = DrawDagFlags::DELETE_DOTCODE);


bool DrawForestOnFile(const std::string& output_file, const LineForestHandler& lfh, DrawDagFlags flags);

#endif // !GRAPHGEN_OUTPUT_GENERATOR_H_