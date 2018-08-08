#ifndef TREESGENERATOR_UTILITIES_H
#define TREESGENERATOR_UTILITIES_H

#include <algorithm>
#include <string>

extern std::string global_output_path;

struct nodeid {
    int _id = 0;
    int next() { return ++_id; }
    int get() { return _id; }
};

static inline void RemoveCharacter(std::string& s, const char c) {
    s.erase(std::remove(s.begin(), s.end(), c), s.end());
}

// Function to automatically print a message before and after each operation
// No braces around instruction, so you can log also variable definitions without scoping them
#define LOG(message, instructions) std::cout << (message) << "... "; instructions std::cout << "done.\n"

#endif // !TREESGENERATOR_UTILITIES_H

