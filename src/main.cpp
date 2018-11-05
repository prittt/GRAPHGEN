#include <array>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "ruleset_generator.h"

using namespace std;

int main()
{
    auto at = ruleset_generator_type::rosenfeld_3d;
    auto algorithm_name = ruleset_generator_names[static_cast<int>(at)];
    auto ruleset_generator = ruleset_generator_functions[static_cast<int>(at)];

    auto rs = ruleset_generator();
    ofstream os("rules.txt");
    rs.print_rules(os);
}