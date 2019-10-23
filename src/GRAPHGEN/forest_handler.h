#pragma once

#include "output_generator.h"

/** @brief This class allows to generate the forests associated to an algorithm when the pixel prediction is applied.

When applying pixel prediction optimization many different forest should be generated. The number of forests depends
on the mask size and on vertical shift size. This class considers only Rosenfeld and Grana masks like situations. In 
the former case the vertical shift is unitary and only first line forests and central lines forest are required. In 
the latter case the mask has a vertical shift of 2 pixels. In this case four different forest are required:
    - first line
    - center lines
    - last line
    - single line

The forests generation requires the original decision tree associated to the algorithm on which apply the prediction
optimization and the pixel set associated to the mask.
*/
class ForestHandler {
public:
    std::map<int, LineForestHandler> f_;
    std::map<int, std::string> names = { 
                                         {CENTER_LINES, "center"}, 
                                         {FIRST_LINE  , "first"}, 
                                         {LAST_LINE   , "last"}, 
                                         {SINGLE_LINE , "single"} 
                                       };
    std::map<int, std::string> prefixs = {
                                           {CENTER_LINES, "cl"},
                                           {FIRST_LINE  , "fl"},
                                           {LAST_LINE   , "ll"},
                                           {SINGLE_LINE , "sl"}
                                         };
public:
    static const int CENTER_LINES;
    static const int FIRST_LINE;
    static const int LAST_LINE;
    static const int SINGLE_LINE;

    ForestHandler(const BinaryDrag<conact>& bd,
        const pixel_set& ps,
        int flag = ForestHandler::CENTER_LINES | ForestHandler::FIRST_LINE)
    {
        // Center lines forest generation
        if (CENTER_LINES & flag) {
            f_[CENTER_LINES] = LineForestHandler(bd, ps);
        }

        // First line forest generation
        if (FIRST_LINE & flag) {
            constraints first_line_constr;
            for (const auto& p : ps) {
                if (p.GetDy() < 0)
                    first_line_constr[p.name_] = 0;
            }
            f_[FIRST_LINE] = LineForestHandler(bd, ps, first_line_constr);
        }
        // Last line forest generation
        if (LAST_LINE & flag) {
            constraints last_line_constr;
            for (const auto& p : ps) {
                if (p.GetDy() > 0)
                    last_line_constr[p.name_] = 0;
            }
            f_[LAST_LINE] = LineForestHandler(bd, ps, last_line_constr);
        }
        // Single line forest generation
        if (SINGLE_LINE & flag) {
            constraints single_line_constr;
            for (const auto& p : ps) {
                if (p.GetDy() != 0)
                    single_line_constr[p.name_] = 0;
            }
            f_[SINGLE_LINE] = LineForestHandler(bd, ps, single_line_constr);
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
        for (const auto& i : f_) {
            std::filesystem::path filepath = conf.GetForestCodePath(names[i.first] + "_line");
            std::ofstream os(filepath);
            if (!os) {
                std::cout << "Something went wrong during the generation of " << conf.algorithm_name_ << "forest code\n";
                return;
            }
            if (i.first != CENTER_LINES) {
                GenerateLineForestCode(os, i.second, prefixs[i.first] + "_", 0, before_main, after_main, before_end, after_end_no_loop);
                os << prefixs[i.first] + "_" << ":;\n";
            }
            else {
                GenerateLineForestCode(os, i.second, prefixs[i.first] + "_", 0, before_main, after_main, before_end, after_end);
            }
        }
    }

    void Compress(DragCompressorFlags flags = DragCompressorFlags::PRINT_STATUS_BAR | DragCompressorFlags::IGNORE_LEAVES) {
       for (auto& x : f_) {
           DragCompressor(x.second, 5000, flags);
       }
        
    }

    LineForestHandler& GetLineForestHandler(int forest_id) {
        return f_.at(forest_id);
    }
};

// out of class static const initializer for strict compilers (e.g. gcc)
// see: https://stackoverflow.com/questions/5391973/undefined-reference-to-static-const-int

const int ForestHandler::CENTER_LINES = 1;
const int ForestHandler::FIRST_LINE   = 2;
const int ForestHandler::LAST_LINE    = 4;
const int ForestHandler::SINGLE_LINE  = 8;