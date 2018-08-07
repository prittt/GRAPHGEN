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

/*
* Mask configuration file
The mask configuration file is line oriented. Every token is separated by spaces or tabs.
A line starting with # is a comment and is skipped.

*Line commands
Line commands have a verb followed by parameters. 
The "origin <name>" command sets the pixel name which will be at 0,0
The "hshift <value>" command sets the number of pixels the mask will move horizontally
The "vshift <value>" command sets the number of pixels the mask will move vertically

*Mask information
Mask information is enclosed within a "begin_mask" and "end_mask" commands.
Every pixel is denoted by a name token. A 0 is used to ignore pixels.

*Examples:

# Rosenfeld mask
origin  x
hshift  1
vshift  1
begin_mask
p   q   r
s   x
end_mask

# BBDT mask
origin  o
hshift  2
vshift  2
begin_mask 
0   b   c   d   e   0
g   h   i   j   k   0
m   n   o   p
0   r   s   t
end_mask

# Alternative equivalent BBDT mask
origin  o
hshift  2
vshift  2
begin_mask
0   b   c   d   e   
g   h   i   j   k   
m   n   o   p
0   r   s   t
end_mask

# CTB mask
origin  o
hshift  1
vshift  2
begin_mask
n1  n2  n3
n4  a
n5  b
end_mask
*/
struct pixel_set {
	std::vector<pixel> pixels;
    uint8_t shift_ = 1;

    pixel_set(std::initializer_list<pixel> il) : pixels{ il }{}

	// Creates a pixel set reading the mask from file
	//pixel_set(const char* filename) {
	//	std::ifstream is(filename);
	//	if (!is.is_open()) {
	//		// TODO, error handling?
	//		return;
	//	}


	//}

    void SetShift(uint8_t shift) {
        shift_ = shift;
    };

	void add(std::string name, int dx, int dy) { pixels.emplace_back(name, dx, dy); }
};

#endif //GRAPHSGEN_PIXEL_SET_H_
