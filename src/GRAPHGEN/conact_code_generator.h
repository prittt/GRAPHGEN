// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_CONACT_CODE_GENERATOR_H_
#define GRAPHGEN_CONACT_CODE_GENERATOR_H_

#include <ostream>
#include <optional>

#include "conact_tree.h"
#include "forest.h"
#include "rule_set.h"


/** @brief This is the enum class that defines the available flags for the 
GeneratePointersConditionsActionsCode function.
*/
DEFINE_ENUM_CLASS_FLAGS(GenerateConditionActionCodeFlags,
    NONE                    = 0,          /**< @brief No flags */
    CONDITIONS_WITH_IFS     = (1 << 0),   /**< @brief Whether to add if statements or not when generating conditions code. 
                                                      They serve to check if the pixel we want to check is inside the image 
                                                      or not. All the algorithms that make use of prediction can avoid these
                                                      checks because they are inbuilt in the forest. */
    ACTIONS_WITH_CONTINUE   = (1 << 1),   /**< @brief Whether to add continues at the end of each action or not */
)

/** @brief This is the enum class that defines the available algorithms types for
the GenerateActionsCode function.
*/
DEFINE_ENUM_CLASS_FLAGS(GenerateActionCodeTypes,
    LABELING   = (1 << 0), /**< @brief Connected Components Labeling */
    THINNING   = (1 << 1), /**< @brief Thinning */
    CHAIN_CODE = (1 << 2), /**< @brief Chain Code */
)

/** @brief

TODO fix documentation
names contains the position in the labels image corresponding to the names used in labeling actions. 
It is necessary to handle blocks names and defaults to mask pixel set if not provided. 

*/
bool GeneratePointersConditionsActionsCode(const rule_set& rs,
                                           GenerateConditionActionCodeFlags flag = GenerateConditionActionCodeFlags::CONDITIONS_WITH_IFS | GenerateConditionActionCodeFlags::ACTIONS_WITH_CONTINUE,
                                           GenerateActionCodeTypes type = GenerateActionCodeTypes::LABELING,
                                           std::optional<pixel_set> names = std::nullopt);

// This is the version for CTBE algorithm. It is very raw and it is based on the previous versione of graphgen
//bool GenerateActionsForCtbe(const std::string& filename, const rule_set& rs);


#endif // GRAPHGEN_CONACT_CODE_GENERATOR_H_