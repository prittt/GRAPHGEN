#ifndef GRAPHSGEN_PIXEL_SET_H_
#define GRAPHSGEN_PIXEL_SET_H_

#include <string>
#include <vector>
#include <fstream>

struct pixel {
	std::string name;
	int dx, dy;

	pixel(std::string name_, int dx_, int dy_) : name{ std::move(name_) }, dx{ dx_ }, dy{ dy_ } {}
};

struct pixel_set {
	std::vector<pixel> pixels_;
    uint8_t shift_ = 1;

    pixel_set() {}
    pixel_set(std::initializer_list<pixel> il) : pixels_{ il }{}

    void SetShift(uint8_t shift) {
        shift_ = shift;
    };

	void add(std::string name, int dx, int dy) { pixels_.emplace_back(name, dx, dy); }

    auto begin() { return std::begin(pixels_); }
    auto end() { return std::end(pixels_); }
    auto begin() const { return std::begin(pixels_); }
    auto end() const { return std::end(pixels_); }
};

#endif //GRAPHSGEN_PIXEL_SET_H_
