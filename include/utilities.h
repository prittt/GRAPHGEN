#ifndef TREESGENERATOR_UTILITIES_H
#define TREESGENERATOR_UTILITIES_H

#include <string.h>

std::string global_output_path = "..\\outputs\\";

struct nodeid {
    int _id = 0;
    int next() { return ++_id; }
    int get() { return _id; }
};

#endif // !TREESGENERATOR_UTILITIES_H

