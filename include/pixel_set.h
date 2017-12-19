#pragma once

#include <string>
#include <vector>

struct pixel {
	std::string name;
	int dx, dy;

	pixel(std::string name_, int dx_, int dy_) : name{ std::move(name_) }, dx{ dx_ }, dy{ dy_ } {}
};
struct pixel_set {
	std::vector<pixel> pixels;
    uint8_t shift_ = 1;

    pixel_set(std::initializer_list<pixel> il) : pixels{ il }{}

    void SetShift(uint8_t shift) {
        shift_ = shift;
    };

	void add(std::string name, int dx, int dy) { pixels.emplace_back(name, dx, dy); }
};
