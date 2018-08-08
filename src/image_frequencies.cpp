#include "image_frequencies.h"

#include <iostream>
#include <limits>

#include "utilities.h"

using namespace std;

// This function extracts all the configurations of a given mask (msk) in a give image (img) and store the occourences (frequencies) in the rRules vector
void CalculateConfigurationsFrequencyOnImage(const cv::Mat1b& img, const mask &msk, rule_set &rs) {

    cv::Mat1b clone;
    copyMakeBorder(img, clone, msk.border_, msk.border_, msk.border_, msk.border_, cv::BORDER_CONSTANT, 0);
    const int h = clone.rows, w = clone.cols;

    for (int r = msk.border_; r < h - msk.border_; r += msk.increment_) {
        for (int c = msk.border_; c < w - msk.border_; c += msk.increment_) {
            cv::Mat1b read_pixels = clone(cv::Rect(cv::Point(c - msk.left_, r - msk.top_), cv::Point(c + 1 + msk.right_, r + 1 + msk.bottom_))).clone();
            bitwise_and(msk.mask_, read_pixels, read_pixels);
            size_t rule = msk.MaskToLinearMask(read_pixels);
            rs.rules[rule].frequency++;
            if (rs.rules[rule].frequency == std::numeric_limits<unsigned long long>::max()) {
                std::cout << "OVERFLOW freq\n";
            }
        }
    }
}

bool GetBinaryImage(const std::string &FileName, cv::Mat1b& binary) {
    // Image load
    cv::Mat image;
    image = cv::imread(FileName, cv::IMREAD_GRAYSCALE);   // Read the file

    if (image.empty()) // Check if image exist
        return false;

    //// Convert the image to grayscale
    //Mat grayscaleMat;
    //cvtColor(image, grayscaleMat, CV_RGB2GRAY);

    // Adjust the threshold to actually make it binary
    cv::threshold(image, binary, 100, 1, cv::THRESH_BINARY);

    return true;
}

bool LoadFileList(std::vector<std::pair<std::string, bool>>& filenames, const std::string& files_path)
{
    // Open files_path (files.txt)
    ifstream is(files_path);
    if (!is.is_open()) {
        return false;
    }

    string cur_filename;
    while (getline(is, cur_filename)) {
        // To delete possible carriage return in the file name
        // (especially designed for windows file newline format)
        RemoveCharacter(cur_filename, '\r');
        filenames.push_back(make_pair(cur_filename, true));
    }

    is.close();
    return true;
}

void CalculateRulesFrequencies(const pixel_set &ps, const vector<string> &paths, rule_set &rs) {
    mask msk(ps);

    cv::Mat1b img;

    for (uint i = 0; i < paths.size(); ++i) {
        vector<pair<string, bool>> files_list;
        if (!LoadFileList(files_list, paths[i] + "//files.txt")) {
            cout << "Unable to find 'files.txt' of " << paths[i] << ", dataset skipped";
            continue;
        }

        for (uint d = 0; d < files_list.size(); ++d) {
            GetBinaryImage(paths[i] + "//" + files_list[d].first, img);
            if (img.empty()) {
                cout << "Unable to find '" << files_list[d].first << "' image in '" << paths[i] << "' dataset, image skipped\n";
                continue;
            }
            CalculateConfigurationsFrequencyOnImage(img, msk, rs);
        }
    }
}
