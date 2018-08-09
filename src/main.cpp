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

// Instead of defining a novel format to save DRAGS, we save them as trees, then link
// identical sub-trees. Since the order of traversal is the same, the result should be the same.
// Should...
bool LoadConactDrag(ltree& t, const string& filename)
{
    if (!LoadConactTree(t, filename))
        return false;

    Dag2DagUsingIdenties(t);
    
    return true;
}
bool WriteConactDrag(ltree& t, const string& filename)
{
    return WriteConactTree(t, filename);
}

void PerformOptimalDragGeneration(ltree& t, const string& algorithm_name)
{
    LOG("Creating DRAG using identites",
        Tree2DagUsingIdentities(t);
    );
    string drag_filename = global_output_path + algorithm_name + "_drag_identities";
    DrawDagOnFile(drag_filename, t, true);
    print_stats(t);

    string odrag_filename = global_output_path + algorithm_name + "_optimal_drag.txt";
    if (!LoadConactDrag(t, odrag_filename)) {
        TLOG("Computing optimal DRAG\n",
            Dag2OptimalDag(t);
        );
        WriteConactDrag(t, odrag_filename);
    }
    string optimal_drag_filename = global_output_path + algorithm_name + "_optimal_drag";
    DrawDagOnFile(optimal_drag_filename, t, true);
    print_stats(t);

    LOG("Writing DRAG code",
        {
            string code_filename = global_output_path + algorithm_name + "_drag_code.txt";
            ofstream os(code_filename);
            GenerateCode(os, t);
        }
    );
}

#include "forest.h"

template <typename T>
string zerostr(const T& val, size_t n) {
    stringstream ss;
    ss << setw(n) << setfill('0') << val;
    return ss.str();
}

int main()
{
    auto at = ruleset_generator_type::bbdt;

    auto algorithm_name = ruleset_generator_names[static_cast<int>(at)];
    auto ruleset_generator = ruleset_generator_functions[static_cast<int>(at)];

    auto rs = ruleset_generator();

    string odt_filename = global_output_path + algorithm_name + "_odt.txt";
    ltree t;
    if (!LoadConactTree(t, odt_filename)) {
        t = GenerateOdt(rs, odt_filename);
    }
    string tree_filename = global_output_path + algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, t, true);
    print_stats(t);

    //PerformOptimalDragGeneration(t, algorithm_name);

    Forest f(t, rs.ps_);

    for (size_t i = 0; i < f.trees_.size(); ++i) {
        DrawDagOnFile("tree" + zerostr(i, 4), f.trees_[i]);
    }
}