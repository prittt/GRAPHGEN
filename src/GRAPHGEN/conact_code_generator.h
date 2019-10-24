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