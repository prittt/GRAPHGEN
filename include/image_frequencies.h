#pragma once

#include <opencv2\core.hpp>
#include <opencv2\imgcodecs.hpp>
#include <opencv2\imgproc.hpp>

#include "rule_set.h"

struct mask {
    cv::Mat1b mask_;
    int top_ = 0, bottom_ = 0, left_ = 0, right_ = 0;
    int border_ = 0;
    int exp_;
    int increment_ = 0;

    mask(const pixel_set &ps) {
        increment_ = ps.shift_;
        exp_ = ps.pixels.size();
        for (int i = 0; i < exp_; ++i) {
            border_ = std::max(border_, std::max(std::abs(ps.pixels[i].dx), std::abs(ps.pixels[i].dy)));
            top_ = std::min(top_, ps.pixels[i].dy);
            right_ = std::max(right_, ps.pixels[i].dx);
            left_ = std::min(left_, ps.pixels[i].dx);
            bottom_ = std::max(bottom_, ps.pixels[i].dy);
        }

        left_ = std::abs(left_);
        top_ = std::abs(top_);

        mask_ = cv::Mat1b(top_ + bottom_ + 1, left_ + right_ + 1, uchar(0));
        for (int i = 0; i < exp_; ++i) {
            mask_(ps.pixels[i].dy + top_, ps.pixels[i].dx + left_) = 1;
        }
    }

    size_t MaskToLinearMask(const cv::Mat1b r_img) const {
        size_t linearMask = 0;

        for (int r = 0; r < r_img.rows; ++r) {
            for (int c = 0; c < r_img.cols; ++c) {
                linearMask <<= (1 & mask_(r, c));
                linearMask |= (r_img(r, c) & mask_(r, c));
            }
        }
        return linearMask;
    }
};

void CalculateConfigurationsFrequencyOnImage(const cv::Mat1b& img, const mask &msk, rule_set &rs);
bool GetBinaryImage(const std::string &FileName, cv::Mat1b& binary);
bool LoadFileList(std::vector<std::pair<std::string, bool>>& filenames, const std::string& files_path);
void CalculateRulesFrequencies(const pixel_set &ps, const std::vector<std::string> &paths, rule_set &rs);
