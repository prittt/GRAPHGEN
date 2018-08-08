#include <algorithm>
#include <bitset>
#include <set>
#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdint>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <cassert>

//#include <opencv2\core.hpp>
//#include <opencv2\imgcodecs.hpp>
//#include <opencv2\imgproc.hpp>

#include "condition_action.h"
#include "code_generator.h"
#include "hypercube.h"
#include "output_generator.h"
#include "ruleset_generator.h"
#include "tree.h"
#include "tree2dag_identities.h"
#include "utilities.h"
#include "drag_statistics.h"

using namespace std;

//#include "main.h"

int main()
{
    enum class algorithm_type { rosenfeld, bbdt };
    string algorithm_names[] = { "rosenfeld", "bbdt" };
    rule_set (*algorithm_functions[])(void) = { GenerateRosenfeld, GenerateBBDT };

    auto at = algorithm_type::bbdt;

    auto algorithm_name = algorithm_names[static_cast<int>(at)];
    auto algorithm_function = algorithm_functions[static_cast<int>(at)];

    auto rs = algorithm_function();

    string odt_filename = global_output_path + algorithm_name + "_odt.txt";
    ltree t;
    if (!LoadConactTree(t, odt_filename)) {
        t = GenerateOdt(rs, odt_filename);
    }

    DragStatistics ds(t);
    cout << "Nodes = " << ds.nodes() << "\n";
    cout << "Leaves = " << ds.leaves() << "\n";

    string tree_filename = global_output_path + algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, t, true);

    LOG("Creating DRAG using identites",
        Tree2DagUsingIdentities(t);
    );

    string drag_filename = global_output_path + algorithm_name + "_drag";
    DrawDagOnFile(drag_filename, t, true);

    ds = DragStatistics(t);
    cout << "Nodes = " << ds.nodes() << "\n";
    cout << "Leaves = " << ds.leaves() << "\n";
}