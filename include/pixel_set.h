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
	std::vector<pixel> pixels;
    uint8_t shift_ = 1;

    pixel_set(std::initializer_list<pixel> il) : pixels{ il }{}

	// Creates a pixel set reading the mask from file
	pixel_set(const char* filename) {
		std::ifstream is(filename);
		if (!is.is_open()) {
			// TODO, error handling?
			return;
		}


	}

    void SetShift(uint8_t shift) {
        shift_ = shift;
    };

	void add(std::string name, int dx, int dy) { pixels.emplace_back(name, dx, dy); }
};

#endif //GRAPHSGEN_PIXEL_SET_H_
