// Copyright(c) 2018 - 2019 Federico Bolelli, Costantino Grana
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

#ifndef GRAPHGEN_FOREST_HANDLER_H_
#define GRAPHGEN_FOREST_HANDLER_H_

#include "output_generator.h"


DEFINE_ENUM_CLASS_FLAGS(ForestHandlerFlags,
    CENTER_LINES = 1, /**< @brief Whether to generate the forest for the center lines of the image */
    FIRST_LINE   = 2, /**< @brief Whether to generate the forest for the first line of the image */
    LAST_LINE    = 4, /**< @brief Whether to generate the forest for the last line of the image */
    SINGLE_LINE  = 8, /**< @brief Whether to generate the forest for an image composed by a single line */
)

/** @brief This class allows to generate the forests associated to an algorithm when the pixel prediction is applied.

When applying pixel prediction optimization many different forest should be generated. The number of forests depends
on the mask size and on vertical shift size. This class considers only Rosenfeld and Grana masks like situations. In 
the former case the vertical shift is unitary and only first line forests and central lines forest are required. In 
the latter case the mask has a vertical shift of 2 pixels. In this case four different group of forests are required:
    - first line
    - center lines
    - last line
    - single line

The forests generation requires the original decision tree associated to the algorithm on which apply the prediction
optimization and the pixel set associated to the mask. The ForestHandlerFlagg enum class allows to select which kind
of forests groups you want the ForestHandler generates for you.
*/
class ForestHandler {
public:
    std::map<ForestHandlerFlags, LineForestHandler> f_;
    std::map<ForestHandlerFlags, std::string> names = {
                                                         {ForestHandlerFlags::CENTER_LINES, "center"},
                                                         {ForestHandlerFlags::FIRST_LINE  , "first"},
                                                         {ForestHandlerFlags::LAST_LINE   , "last"},
                                                         {ForestHandlerFlags::SINGLE_LINE , "single"}
                                                       };
    std::map<ForestHandlerFlags, std::string> prefixs = {
                                                         {ForestHandlerFlags::CENTER_LINES, "cl"},
                                                         {ForestHandlerFlags::FIRST_LINE  , "fl"},
                                                         {ForestHandlerFlags::LAST_LINE   , "ll"},
                                                         {ForestHandlerFlags::SINGLE_LINE , "sl"}
                                                       };
public:

    ForestHandler(const BinaryDrag<conact>& bd,
        const pixel_set& ps,
        ForestHandlerFlags flag = ForestHandlerFlags::CENTER_LINES | ForestHandlerFlags::FIRST_LINE)
    {
        // Center lines forest generation
        if (ForestHandlerFlags::CENTER_LINES & flag) {
            f_[ForestHandlerFlags::CENTER_LINES] = LineForestHandler(bd, ps);
        }

        // First line forest generation
        if (ForestHandlerFlags::FIRST_LINE & flag) {
            constraints first_line_constr;
            for (const auto& p : ps) {
                if (p.GetDy() < 0)
                    first_line_constr[p.name_] = 0;
            }
            f_[ForestHandlerFlags::FIRST_LINE] = LineForestHandler(bd, ps, first_line_constr);
        }
        // Last line forest generation
        if (ForestHandlerFlags::LAST_LINE & flag) {
            constraints last_line_constr;
            for (const auto& p : ps) {
                if (p.GetDy() > 0)
                    last_line_constr[p.name_] = 0;
            }
            f_[ForestHandlerFlags::LAST_LINE] = LineForestHandler(bd, ps, last_line_constr);
        }
        // Single line forest generation
        if (ForestHandlerFlags::SINGLE_LINE & flag) {
            constraints single_line_constr;
            for (const auto& p : ps) {
                if (p.GetDy() != 0)
                    single_line_constr[p.name_] = 0;
            }
            f_[ForestHandlerFlags::SINGLE_LINE] = LineForestHandler(bd, ps, single_line_constr);
        }
    }

    void DrawOnFile(const std::string& prefix, int flags = 0)
    {
        for (auto& x : f_) {
            DrawForestOnFile(prefix + "_" + names[x.first], x.second, flags);
        }
    }

    // TODO add documentation here and move the implementation in the cpp file
    void GenerateCode(BEFORE_AFTER_FUN(before_main)       = BeforeMainShiftOne,
                      BEFORE_AFTER_FUN(after_main)        = DefaultEmptyFunc,
                      BEFORE_AFTER_FUN(before_end)        = BeforeEnd,
                      BEFORE_AFTER_FUN(after_end)         = AfterEnd,
                      BEFORE_AFTER_FUN(after_end_no_loop) = AfterEndNoLoop,
                      int flags = 0 /* no flags available right now */)
    {
        int last_id = 0;
        for (const auto& i : f_) {
            std::filesystem::path filepath = conf.GetForestCodePath(names[i.first] + "_line");
            std::ofstream os(filepath);
            if (!os) {
                std::cout << "Something went wrong during the generation of " << conf.algorithm_name_ << "forest code\n";
                return;
            }
            if (i.first != ForestHandlerFlags::CENTER_LINES) {
                last_id = GenerateLineForestCode(os, i.second, prefixs[i.first] + "_", last_id, before_main, after_main, before_end, after_end_no_loop);
                os << prefixs[i.first] + "_" << ":;\n";
            }
            else {
                last_id = GenerateLineForestCode(os, i.second, prefixs[i.first] + "_", last_id, before_main, after_main, before_end, after_end);
            }
        }
    }

    void Compress(DragCompressorFlags flags = DragCompressorFlags::PRINT_STATUS_BAR | DragCompressorFlags::IGNORE_LEAVES, int iterations = - 1) {
       for (auto& x : f_) {
           DragCompressor(x.second, iterations, flags);
       }
    }

    LineForestHandler& GetLineForestHandler(ForestHandlerFlags forest_id) {
        return f_.at(forest_id);
    }
};


#endif // FOREST_HANDLER_H_