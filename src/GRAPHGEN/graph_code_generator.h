// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_GRAPH_CODE_GENERATOR_H_
#define GRAPHGEN_GRAPH_CODE_GENERATOR_H_

#include <ostream>

#include "conact_tree.h"
#include "forest.h"
#include "rule_set.h"

/** @brief Macro used to define the base signature of before and after functions */
#define BEFORE_AFTER_FUNC(func_name) std::string func_name(size_t index,                                    \
                                                          const std::string& prefix,                        \
                                                          const std::vector<std::vector<size_t>>& mapping,  \
                                                          size_t end_group_id)

BEFORE_AFTER_FUNC(DefaultEmptyFunc);
BEFORE_AFTER_FUNC(BeforeMainShiftOne);
BEFORE_AFTER_FUNC(BeforeMainShiftTwo);
BEFORE_AFTER_FUNC(BeforeEnd);
BEFORE_AFTER_FUNC(AfterEnd);
BEFORE_AFTER_FUNC(AfterEndNoLoop);

/** @brief This function generates the code for the given drag reversing the output into the specified stream

Function pointer parameters are not well treated by doxygen. TODO fix it.

@param[in] os Where to write the code (output stream).
@param[in] bd BinaryDrag<conact> for which generating the code.
@param[in] with_gotos Whether to add gotos or not during code generation.
@param[in] before Pointer to the function which defines the string that should be put in the code before a tree.
                  When dealing with forests for example you will need pass to GenerateDragCode a "before" function 
                  like the following one (i identifies the tree): 
                  \code{.cpp}
                  [mapping](int i, const std::string& prefix) -> std::string { 
                       return prefix + "tree_" + string(i) + ": if ((c+=1) >= w - 1) goto " + prefix + "break_0_" + string(mapping[0][i]) + ";\n";
                  }
                  \endcode
                  this function is valid for example for thinning algorithms, PRED, SAUF, CTB and all the algorithms that have a
                  unitary horizontal shift.
                  When dealing with algorithm which have an horizontal shift of 2 and for this reason will need multiple end line 
                  forests a function like the following one is required:
                  \code{.cpp}
                  [mapping](int i, const std::string& prefix) -> std::string {
                       return prefix + "tree_" + string(i) + ": if ((c+=2) >= w - 2) { if (c > w - 2) { goto " + prefix + \
                       "break_0_" + string(mapping_[0][i]) + "; } else { goto " + prefix +                                \
                       "break_1_" + string(mapping_[1][i]) + "; } } \n";
                  }
                  \endcode
                  DefaultBefore function returns an empty string and is the default function passed to the 
                  GenerateDragCode function.
@param[in] after Pointer to the function which defines the string that should be put into the code after a tree.
                 When dealing with forest for example you will need pass to GenerateDragCode a "before" function 
                 like the following one:
                 \code{.cpp}
                 [mapping](int i, const std::string& prefix) -> std::string {
                    return string(2, '\t') + "continue;\n";
                 }
                 \endcode
@param[in] prefix String to add before tree names. This variable will be passed to the before and after functions
@param[in] start_id Is the id from which start node enumeration when dealing with drags. It is especially useful
                    to avoid multiple defined labels when dealing with multiple forests in the same code, like 
                    for example when having a special forest for the first line e for the last one.
@param[in] mapping Mapping between main and end-of-the line trees. Default value is an empty mapping.
@param[in] end_group_id It is the id of the end-line forest group. It is needed when an algorithm 
                        requires multiple end-line forest group, such as when dealing with block-based
                        approaches. Default value is zero.

@return The last id used during code generation. This is useful when multiple must be written in the same file.
*/
size_t GenerateDragCode(std::ostream& os, 
                        const BinaryDrag<conact>& bd, 
                        bool with_gotos = false,
                        BEFORE_AFTER_FUNC(before) = DefaultEmptyFunc,
                        BEFORE_AFTER_FUNC(after)  = DefaultEmptyFunc,
                        const std::string prefix = "",
                        size_t start_id = 0,
                        const std::vector<std::vector<size_t>> mapping = {}, 
                        size_t end_group_id = 0);

/** @brief Overload. No output stream required in this case. */
bool GenerateDragCode(const BinaryDrag<conact>& bd, 
                      bool with_gotos = false,
                      BEFORE_AFTER_FUNC(before) = DefaultEmptyFunc,
                      BEFORE_AFTER_FUNC(after)  = DefaultEmptyFunc,
                      const std::string prefix = "",
                      size_t start_id = 0,
                      const std::vector<std::vector<size_t>> mapping = {},
                      size_t end_group_id = 0);

/** @brief Generate the C++ code for the given Forest. 

Parameters description missing. (TODO)

*/
size_t GenerateLineForestCode(std::ostream& os,
                              const LineForestHandler& lfh,
                              std::string prefix,
                              size_t start_id,
                              BEFORE_AFTER_FUNC(before_main) = BeforeMainShiftOne,
                              BEFORE_AFTER_FUNC(after_main)  = DefaultEmptyFunc,
                              BEFORE_AFTER_FUNC(before_end)  = BeforeEnd,
                              BEFORE_AFTER_FUNC(after_end)   = AfterEnd);


#endif // GRAPHGEN_GRAPH_CODE_GENERATOR_H_