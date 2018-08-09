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
#include "drag_statistics.h"
#include "drag2optimal.h"
#include "hypercube.h"
#include "output_generator.h"
#include "ruleset_generator.h"
#include "tree.h"
#include "tree2dag_identities.h"
#include "utilities.h"

using namespace std;

void print_stats(const ltree& t) {
    DragStatistics ds(t);
    cout << "Nodes = " << ds.nodes() << "\n";
    cout << "Leaves = " << ds.leaves() << "\n";
}

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
    string tree_filename = global_output_path + algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, t, true);
    print_stats(t);

    LOG("Creating DRAG using identites",
        Tree2DagUsingIdentities(t);
    );
    string drag_filename = global_output_path + algorithm_name + "_drag_identities";
    DrawDagOnFile(drag_filename, t, true);
    print_stats(t);

    TLOG("Computing optimal DRAG\n",
        Tree2OptimalDag(t);
    );
    string optimal_drag_filename = global_output_path + algorithm_name + "_optimal_drag";
    DrawDagOnFile(optimal_drag_filename, t, true);
    print_stats(t);

    LOG("Writing DRAG code",
        {
            ofstream os("bbdt_drag_code.txt");
            GenerateCode(os, t);
        }
    );
}