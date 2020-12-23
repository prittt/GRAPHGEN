// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

//#include <set>
#include <algorithm>
#include <fstream>
//#include <unordered_map>
//#include <iterator>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdint>
//#include <map>
//#include <unordered_map>
//#include <unordered_set>

#include <intrin.h>

#include "condition_action.h"
#include "code_generator.h"
#include "file_manager.h"
#include "forest2dag.h"
#include "forest_optimizer.h"
#include "forest_statistics.h"
#include "drag_statistics.h"
#include "drag2optimal.h"
#include "hypercube.h"
#include "output_generator.h"
#include "ruleset_generator.h"
#include "tree2dag_identities.h"
//#include "utilities.h"

#include "image_frequencies.h"

using namespace std;
namespace fs = filesystem;

void PerformOptimalDragGeneration(ltree& t, const string& algorithm_name)
{
	LOG("Creating DRAG using identites",
		Tree2DagUsingIdentities(t);
	);
	string drag_filename = global_output_path + algorithm_name + "_drag_identities";
	DrawDagOnFile(drag_filename, t, true);
	PrintStats(t);

	string odrag_filename = global_output_path + algorithm_name + "_optimal_drag.txt";
	if (!LoadConactDrag(t, odrag_filename)) {
		TLOG("Computing optimal DRAG\n",
			Dag2OptimalDag(t);
		);
		WriteConactDrag(t, odrag_filename);
	}
	string optimal_drag_filename = global_output_path + algorithm_name + "_optimal_drag";
	DrawDagOnFile(optimal_drag_filename, t, true);
	PrintStats(t);

	LOG("Writing DRAG code",
		{
			string code_filename = global_output_path + algorithm_name + "_drag_code.txt";
			ofstream os(code_filename);
			GenerateCode(os, t);
		}
	);
}

// This function calls DrawDagOnFile to generate pdf for all the trees (maintrees and endtrees) of a given forest 
void DrawForestTrees(const Forest& f, const fs::path& base_path, const string& algorithm_name) {
    for (size_t i = 0; i < f.trees_.size(); ++i) {
        fs::path current_path = base_path / fs::path(algorithm_name + "_tree" + zerostr(i, 4));
        DrawDagOnFile(current_path.string(), f.trees_[i], true);
    }
    for (size_t i = 0; i < f.end_trees_.size(); ++i) {
        for (size_t j = 0; j < f.end_trees_[i].size(); ++j) {
            fs::path current_path = base_path / fs::path(algorithm_name + "_endtree" + zerostr(i, 2) + "_" + zerostr(j, 2));
            DrawDagOnFile(current_path.string(), f.end_trees_[i][j], false);
        }
    }
}

// Starting from a tree produce a compressed forest applying the following steps:
//      - Convert Tree to Forest (with Prediction)
//          - <path>/forest_trees/<algorithm_name>_tree<number>.pdf files will contain a representation of forest trees (still no DAG)
//          - <path>/forest_trees/<algorithm_name>_end_tree<number>.pdf files will contain a representation of forest end_trees (still no DAG)
//          - <path>/<algorithm_name>_forest_nodag_code.txt will contain the forest code without DAG (still no DAG here)
//      - Convert Forest to DAG (using identities)
//          - <path>/<algorithm_name>_mainforest.pdf file will contain non compressed DAG forest for the main part of a line (still no equivalences)
//          - <path>/<algorithm_name>_endforest.pdf file will contain non compressed DAG forest for the end part of a line (still no equivalences)
//          - <path>/<algorithm_name>_forest_identities_code.txt will contain the non compressed DAG forest code, both main and end DAG here (still no equivalences)
//      - Reduce DAG forest (using equivalences)
//          - <path>/<algorithm_name>_reduced_mainforest.pdf file will contain compressed DAG forest for the main part of a line
//          - <path>/<algorithm_name>_reduced_endforest.pdf file will contain compressed DAG forest for the end part of a line
//          - <path>/<algorithm_name>_forest_reduced_code.txt will contain the compressed DAG forest code, both main and end DAG here
int PerformPseudoOptimalDragGeneration(const ltree& t, const pixel_set& ps, const string& algorithm_name, const fs::path& path = fs::path(""), int nodeid = 0)
{
    fs::path output_path = fs::path(global_output_path) / path;
    fs::create_directories(output_path);

    // Convert Tree to Forest
    LOG("Making " + algorithm_name + " forest",
        Forest f(t, ps);
        // f.separately = true;

        fs::path treefolder_path = output_path / fs::path("forest_trees");
        fs::create_directories(treefolder_path);

        DrawForestTrees(f, treefolder_path, algorithm_name);
    );
    PrintStats(f);

    LOG("Generating " + algorithm_name + " forest (no DAG) code",
        fs::path forest_code_nodag = output_path / fs::path(algorithm_name + "_forest_nodag_code.txt");
        {
            ofstream os(forest_code_nodag.string());
            GenerateForestCode(os, f);
        }
    );
    
    // Convert Forest to DAG
    LOG("Converting "+ algorithm_name + " forest into DAG",
        Forest2Dag x(f);
    );
    PrintStats(f);

    LOG("Drawing " + algorithm_name + " forest (DAG)",
        fs::path forest_path = output_path / fs::path(algorithm_name);
        DrawForestOnFile(forest_path.string(), f, true);
    );

    LOG("Generating " + algorithm_name + " forest (DAG) code",
        fs::path forest_code = output_path / fs::path(algorithm_name + "_forest_identities_code.txt");
        {
            ofstream os(forest_code.string());
            GenerateForestCode(os, f);
        }
    );

    LOG("Reducing " + algorithm_name + " forest (DAG) using equivalences",
        STree st(f);
    );
    PrintStats(f);

    LOG("Drawing " + algorithm_name + " reduced forest (DAG)",
        fs::path reduced_forest_path = output_path / fs::path(algorithm_name + "_reduced");
        DrawForestOnFile(reduced_forest_path.string(), f, true);
    );

    int last_nodeid;
    LOG("Generating " + algorithm_name + " reduced forest (DAG) code",
        fs::path reduced_forest_code = output_path / fs::path(algorithm_name + "_forest_reduced_code.txt");
        {
            ofstream os(reduced_forest_code.string());
            last_nodeid = GenerateForestCode(os, f, nodeid);
        }
    );

    return last_nodeid;
}

void CreateSpaghettiLabeling() {
    
    auto at = ruleset_generator_type::bbdt;

	auto algorithm_name = ruleset_generator_names[static_cast<int>(at)];
	auto ruleset_generator = ruleset_generator_functions[static_cast<int>(at)];

	auto rs = ruleset_generator();

	//GenerateConditionsActionsCode("condition_action.txt", rs);

	string odt_filename = global_output_path + algorithm_name + "_odt.txt";
	ltree t;
	if (!LoadConactTree(t, odt_filename)) {
		t = GenerateOdt(rs, odt_filename);
	}
	string tree_filename = algorithm_name + "_tree";
	DrawDagOnFile(tree_filename, t, false, true);
	PrintStats(t);

    PerformPseudoOptimalDragGeneration(t, rs.ps_, algorithm_name, fs::path("Spaghetti_BBDT"));

    return;

	string tree_code_filename = global_output_path + algorithm_name + "_code.txt";
	GenerateCode(tree_code_filename, t);

	//PerformOptimalDragGeneration(t, algorithm_name);

	LOG("Making forest",
		Forest f(t, rs.ps_);
	for (size_t i = 0; i < f.trees_.size(); ++i) {
		DrawDagOnFile(algorithm_name + "tree" + zerostr(i, 4), f.trees_[i], true);
	}
	);
	PrintStats(f);

	string forest_code_nodag = global_output_path + algorithm_name + "_forest_nodag_code.txt";
	{
		ofstream os(forest_code_nodag);
		GenerateForestCode(os, f);
	}

	for (size_t i = 0; i < f.end_trees_.size(); ++i) {
		for (size_t j = 0; j < f.end_trees_[i].size(); ++j) {
			DrawDagOnFile(algorithm_name + "_end_tree_" + zerostr(i, 2) + "_" + zerostr(j, 2), f.end_trees_[i][j], false);
		}
	}

	LOG("Converting forest to dag",
		Forest2Dag x(f);
		for (size_t i = 0; i < f.trees_.size(); ++i) {
			DrawDagOnFile(algorithm_name + "drag" + zerostr(i, 4), f.trees_[i], true);
		}
	);
	PrintStats(f);
	
	DrawForestOnFile(algorithm_name + "forest", f, true);

	string forest_code = global_output_path + algorithm_name + "_forest_identities_code.txt";
	{
		ofstream os(forest_code);
		GenerateForestCode(os, f);
	}

	LOG("Reducing forest",
		STree st(f);
	);
	PrintStats(f);

	int last_nodeid_main_forest_group;
	string forest_reduced_code = global_output_path + algorithm_name + "_forest_reduced_code.txt";
	{
		ofstream os(forest_reduced_code);
		last_nodeid_main_forest_group = GenerateForestCode(os, f);
	}
	//DrawForestOnFile(algorithm_name + "_forest_reduced", f, true, true);

	// Create first line constraints
	constraints first_line_constr;
	using namespace std;
	for (const auto& p : rs.ps_) {
		if (p.GetDy() < 0)
			first_line_constr[p.name_] = 0;
	}

	Forest flf(t, rs.ps_, first_line_constr);
	DrawForestOnFile(algorithm_name + "_first_line_original", flf, true, true);

	LOG("Converting first line forest to dag",
	Forest2Dag y(flf);
	);
	PrintStats(flf);

	LOG("Reducing first line forest",
	STree st_fl(flf);
	);
	PrintStats(flf);

	DrawForestOnFile(algorithm_name + "_first_line_reduced", flf, true, true);

	string first_line_forest_reduced_code = global_output_path + algorithm_name + "_first_line_forest_reduced_code.txt";
	{
		ofstream os(first_line_forest_reduced_code);
		last_nodeid_main_forest_group = GenerateForestCode(os, flf, last_nodeid_main_forest_group);
	}

	// Create last line constraints
	constraints last_line_constr;
	using namespace std;
	for (const auto& p : rs.ps_) {
		if (p.GetDy() > 0)
			last_line_constr[p.name_] = 0;
	}

	Forest llf(t, rs.ps_, last_line_constr);
	DrawForestOnFile(algorithm_name + "_last_line_original", llf, true, true);

	/*LOG("Converting first line forest to dag",
	Forest2Dag z(llf);
	);
	PrintStats(llf);

	LOG("Reducing last line forest",
	STree st_ll(llf);
	);
	PrintStats(llf);*/

	DrawForestOnFile(algorithm_name + "_last_line_reduced", llf, true, true);

	string last_line_forest_reduced_code = global_output_path + algorithm_name + "_last_line_forest_reduced_code.txt";
	{
		ofstream os(last_line_forest_reduced_code);
		last_nodeid_main_forest_group = GenerateForestCode(os, llf, last_nodeid_main_forest_group);
	}

	// Create last line constraints
	constraints single_line_constr;
	using namespace std;
	for (const auto& p : rs.ps_) {
		if (p.GetDy() != 0)
			single_line_constr[p.name_] = 0;
	}

	Forest slf(t, rs.ps_, single_line_constr);
	DrawForestOnFile(algorithm_name + "_single_line_original", slf, true, true);

	/*LOG("Converting single line forest to dag",
	Forest2Dag w(slf);
	);
	PrintStats(slf);

	LOG("Reducing single line forest",
	STree st_sl(slf);
	);
	PrintStats(slf);*/

	DrawForestOnFile(algorithm_name + "_single_line_reduced", slf, true, true);

	string single_line_forest_reduced_code = global_output_path + algorithm_name + "_single_line_forest_reduced_code.txt";
	{
		ofstream os(single_line_forest_reduced_code);
		GenerateForestCode(os, slf, last_nodeid_main_forest_group);
	}
}

void CreateChangLabeling() {
    auto at = ruleset_generator_type::chen;

    auto algorithm_name = ruleset_generator_names[static_cast<int>(at)];
    auto ruleset_generator = ruleset_generator_functions[static_cast<int>(at)];

    auto rs = ruleset_generator();

    //GenerateConditionsActionsCode("condition_action.txt", rs);

    string odt_filename = global_output_path + algorithm_name + "_odt.txt";
    ltree t;
    if (!LoadConactTree(t, odt_filename)) {
        t = GenerateOdt(rs, odt_filename);
    }
    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, t, false, true);
    PrintStats(t);

    //Dag2DagUsingIdenties(t);
    //DrawDagOnFile("dag_bbdt_wychang.txt", t, false, true);

    //Dag2DagUsingEquivalences(t, false);
    //Dag2DagUsingEquivalences(t, true);
    //DrawDagOnFile("dag_rosenfeld_3d_with_equivalences", t, false, true);

    //Dag2OptimalDag(t);
    //DrawDagOnFile("dag_rosenfeld_3d_optimal", t, false, true);

    string tree_code_filename = global_output_path + algorithm_name + "_code.txt";
    GenerateCode(tree_code_filename, t);

    //PerformOptimalDragGeneration(t, algorithm_name);

    LOG("Making forest",
        Forest f(t, rs.ps_);
    for (size_t i = 0; i < f.trees_.size(); ++i) {
        DrawDagOnFile(algorithm_name + "tree" + zerostr(i, 4), f.trees_[i], true);
    }
    );
    PrintStats(f);

    string forest_code_nodag = global_output_path + algorithm_name + "_forest_nodag_code.txt";
    {
        ofstream os(forest_code_nodag);
        GenerateForestCode(os, f);
    }

    for (size_t i = 0; i < f.end_trees_.size(); ++i) {
        for (size_t j = 0; j < f.end_trees_[i].size(); ++j) {
            DrawDagOnFile(algorithm_name + "_end_tree_" + zerostr(i, 2) + "_" + zerostr(j, 2), f.end_trees_[i][j], false);
        }
    }

    LOG("Converting forest to dag",
    	Forest2Dag x(f);
    	for (size_t i = 0; i < f.trees_.size(); ++i) {
    		DrawDagOnFile(algorithm_name + "drag" + zerostr(i, 4), f.trees_[i], true);
    	}
    );
    PrintStats(f);
    
    DrawForestOnFile(algorithm_name + "forest", f, true);

    string forest_code = global_output_path + algorithm_name + "_forest_identities_code.txt";
    {
        ofstream os(forest_code);
        GenerateForestCode(os, f);
    }

    LOG("Reducing forest",
    	STree st(f);
    );
    PrintStats(f);

    int last_nodeid_main_forest_group;
    string forest_reduced_code = global_output_path + algorithm_name + "_forest_reduced_code.txt";
    {
        ofstream os(forest_reduced_code);
        last_nodeid_main_forest_group = GenerateForestCode(os, f);
    }
    DrawForestOnFile(algorithm_name + "_forest_reduced", f, true, true);

    // Create first line constraints
    constraints first_line_constr;
    using namespace std;
    for (const auto& p : rs.ps_) {
        if (p.GetDy() < 0)
            first_line_constr[p.name_] = 0;
    }

    Forest flf(t, rs.ps_, first_line_constr);
    DrawForestOnFile(algorithm_name + "_first_line_original", flf, true, true);

    LOG("Converting first line forest to dag",
    Forest2Dag y(flf);
    );
    PrintStats(flf);

    LOG("Reducing first line forest",
    STree st_fl(flf);
    );
    PrintStats(flf);

    DrawForestOnFile(algorithm_name + "_first_line_reduced", flf, true, true);

    string first_line_forest_reduced_code = global_output_path + algorithm_name + "_first_line_forest_reduced_code.txt";
    {
        ofstream os(first_line_forest_reduced_code);
        last_nodeid_main_forest_group = GenerateForestCode(os, flf, last_nodeid_main_forest_group);
    }

    // Create last line constraints
    constraints last_line_constr;
    using namespace std;
    for (const auto& p : rs.ps_) {
        if (p.GetDy() > 0)
            last_line_constr[p.name_] = 0;
    }

    Forest llf(t, rs.ps_, last_line_constr);
    DrawForestOnFile(algorithm_name + "_last_line_original", llf, true, true);

    LOG("Converting first line forest to dag",
    Forest2Dag z(llf);
    );
    PrintStats(llf);

    LOG("Reducing last line forest",
    STree st_ll(llf);
    );
    PrintStats(llf);

    DrawForestOnFile(algorithm_name + "_last_line_reduced", llf, true, true);

    string last_line_forest_reduced_code = global_output_path + algorithm_name + "_last_line_forest_reduced_code.txt";
    {
        ofstream os(last_line_forest_reduced_code);
        last_nodeid_main_forest_group = GenerateForestCode(os, llf, last_nodeid_main_forest_group);
    }

    // Create last line constraints
    constraints single_line_constr;
    using namespace std;
    for (const auto& p : rs.ps_) {
        if (p.GetDy() != 0)
            single_line_constr[p.name_] = 0;
    }

    Forest slf(t, rs.ps_, single_line_constr);
    DrawForestOnFile(algorithm_name + "_single_line_original", slf, true, true);

    LOG("Converting single line forest to dag",
    Forest2Dag w(slf);
    );
    PrintStats(slf);

    LOG("Reducing single line forest",
    STree st_sl(slf);
    );
    PrintStats(slf);

    DrawForestOnFile(algorithm_name + "_single_line_reduced", slf, true, true);

    string single_line_forest_reduced_code = global_output_path + algorithm_name + "_single_line_forest_reduced_code.txt";
    {
        ofstream os(single_line_forest_reduced_code);
        GenerateForestCode(os, slf, last_nodeid_main_forest_group);
    }
}

void CreateCtbeLabeling() {
    auto at = ruleset_generator_type::ctbe;

    auto algorithm_name = ruleset_generator_names[static_cast<int>(at)];
    auto ruleset_generator = ruleset_generator_functions[static_cast<int>(at)];

    auto rs = ruleset_generator();
    {
        ofstream os("regole_ctbe.txt");
        rs.print_rules(os);
    }

    vector<unsigned long long> cnt(rs.rules[0].actions.size(), 0);
    for (int i = 0; i < rs.rules.size(); ++i) {
        for (int j = 0; j < cnt.size(); ++j) {
            cnt[j] += rs.rules[i].actions[j];
        }
    }
    {
        ofstream os("conta_azioni.txt");
        for (int j = 0; j < cnt.size(); ++j) {
            os << j + 1 << " " << cnt[j] << "\n";
        }
    }
    int non_zero = count_if(cnt.begin(), cnt.end(), [](unsigned long long v) {return v > 0; });
    cout << "\nnumero di azioni: " << non_zero << "\n";

    GenerateActionsForCtbe("ctbe_actions.txt", rs);
    //GenerateConditionsActionsCode("condition_action.txt", rs);

    string odt_filename = global_output_path + algorithm_name + "_odt.txt";
    ltree t;
    if (!LoadConactTree(t, odt_filename)) {
        t = GenerateOdt(rs, odt_filename);
    }
    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, t, false, true);
    PrintStats(t);

    //Dag2DagUsingIdenties(t);
    //DrawDagOnFile("dag_rosenfeld_3d_with_identities", t, false, true);

    //Dag2DagUsingEquivalences(t, false);
    //Dag2DagUsingEquivalences(t, true);
    //DrawDagOnFile("dag_rosenfeld_3d_with_equivalences", t, false, true);

    //Dag2OptimalDag(t);
    //DrawDagOnFile("dag_rosenfeld_3d_optimal", t, false, true);

    string tree_code_filename = global_output_path + algorithm_name + "_code.txt";
    GenerateCode(tree_code_filename, t);

    //PerformOptimalDragGeneration(t, algorithm_name);

    LOG("Making forest",
        Forest f(t, rs.ps_);
    for (size_t i = 0; i < f.trees_.size(); ++i) {
        DrawDagOnFile(algorithm_name + "tree" + zerostr(i, 4), f.trees_[i], true);
    }
    );
    PrintStats(f);

    string forest_code_nodag = global_output_path + algorithm_name + "_forest_nodag_frequencies_code.txt";
    {
        ofstream os(forest_code_nodag);
        GenerateForestCode(os, f);
    }

    for (size_t i = 0; i < f.end_trees_.size(); ++i) {
        for (size_t j = 0; j < f.end_trees_[i].size(); ++j) {
            DrawDagOnFile(algorithm_name + "_end_tree_" + zerostr(i, 2) + "_" + zerostr(j, 2), f.end_trees_[i][j], false);
        }
    }

    LOG("Converting forest to dag",
        Forest2Dag x(f);
    for (size_t i = 0; i < f.trees_.size(); ++i) {
        DrawDagOnFile(algorithm_name + "drag" + zerostr(i, 4), f.trees_[i], true);
    }
    );
    PrintStats(f);

    DrawForestOnFile(algorithm_name + "forest", f, true);

    string forest_code = global_output_path + algorithm_name + "_forest_identities_code.txt";
    {
        ofstream os(forest_code);
        GenerateForestCode(os, f);
    }

    LOG("Reducing forest",
    	STree st(f);
    );
    PrintStats(f);

    int last_nodeid_main_forest_group;
    string forest_reduced_code = global_output_path + algorithm_name + "_forest_reduced_code.txt";
    {
        ofstream os(forest_reduced_code);
        last_nodeid_main_forest_group = GenerateForestCode(os, f);
    }
    DrawForestOnFile(algorithm_name + "_forest_reduced", f, true, true);

    // Create first line constraints
    constraints first_line_constr;
    using namespace std;
    for (const auto& p : rs.ps_) {
        if (p.GetDy() < 0)
            first_line_constr[p.name_] = 0;
    }

    Forest flf(t, rs.ps_, first_line_constr);
    DrawForestOnFile(algorithm_name + "_first_line_original", flf, true, true);

    LOG("Converting first line forest to dag",
    Forest2Dag y(flf);
    );
    PrintStats(flf);

    LOG("Reducing first line forest",
    STree st_fl(flf);
    );
    PrintStats(flf);

    DrawForestOnFile(algorithm_name + "_first_line_reduced", flf, true, true);

    string first_line_forest_reduced_code = global_output_path + algorithm_name + "_first_line_forest_reduced_code.txt";
    {
        ofstream os(first_line_forest_reduced_code);
        last_nodeid_main_forest_group = GenerateForestCode(os, flf, last_nodeid_main_forest_group);
    }

    // Create last line constraints
    constraints last_line_constr;
    using namespace std;
    for (const auto& p : rs.ps_) {
        if (p.GetDy() > 0)
            last_line_constr[p.name_] = 0;
    }

    Forest llf(t, rs.ps_, last_line_constr);
    DrawForestOnFile(algorithm_name + "_last_line_original", llf, true, true);

    LOG("Converting first line forest to dag",
    Forest2Dag z(llf);
    );
    PrintStats(llf);

    LOG("Reducing last line forest",
    STree st_ll(llf);
    );
    PrintStats(llf);

    DrawForestOnFile(algorithm_name + "_last_line_reduced", llf, true, true);

    string last_line_forest_reduced_code = global_output_path + algorithm_name + "_last_line_forest_reduced_code.txt";
    {
        ofstream os(last_line_forest_reduced_code);
        last_nodeid_main_forest_group = GenerateForestCode(os, llf, last_nodeid_main_forest_group);
    }

    // Create last two lines constraints
    constraints last_two_lines_constr;
    using namespace std;
    for (const auto& p : rs.ps_) {
        if (p.GetDy() > 1)
            last_two_lines_constr[p.name_] = 0;
    }

    Forest l2lf(t, rs.ps_, last_two_lines_constr);
    DrawForestOnFile(algorithm_name + "_last_two_lines_original", l2lf, true, true);

    LOG("Converting first line forest to dag",
        Forest2Dag w(l2lf);
    );
    PrintStats(l2lf);

    LOG("Reducing last line forest",
        STree st_l2l(l2lf);
    );
    PrintStats(l2lf);

    DrawForestOnFile(algorithm_name + "_last_two_lines_reduced", l2lf, true, true);

    string last_two_lines_forest_reduced_code = global_output_path + algorithm_name + "_last_two_lines_forest_reduced_code.txt";
    {
        ofstream os(last_two_lines_forest_reduced_code);
        last_nodeid_main_forest_group = GenerateForestCode(os, l2lf, last_nodeid_main_forest_group);
    }

    // Create single line constraints
    constraints single_line_constr;
    using namespace std;
    for (const auto& p : rs.ps_) {
        if (p.GetDy() != 0)
            single_line_constr[p.name_] = 0;
    }

    Forest slf(t, rs.ps_, single_line_constr);
    DrawForestOnFile(algorithm_name + "_single_line_original", slf, true, true);

    LOG("Converting single line forest to dag",
    Forest2Dag ww(slf);
    );
    PrintStats(slf);

    LOG("Reducing single line forest",
    STree st_sl(slf);
    );
    PrintStats(slf);

    DrawForestOnFile(algorithm_name + "_single_line_reduced", slf, true, true);

    string single_line_forest_reduced_code = global_output_path + algorithm_name + "_single_line_forest_reduced_code.txt";
    {
        ofstream os(single_line_forest_reduced_code);
        GenerateForestCode(os, slf, last_nodeid_main_forest_group);
    }

    // Create two lines constraints
    constraints two_lines_constr;
    using namespace std;
    for (const auto& p : rs.ps_) {
        if (p.GetDy() != 0 && p.GetDy() != 1)
            two_lines_constr[p.name_] = 0;
    }

    Forest dlf(t, rs.ps_, two_lines_constr);
    DrawForestOnFile(algorithm_name + "_double_lines_original", dlf, true, true);

    LOG("Converting double lines forest to dag",
        Forest2Dag www(dlf);
    );
    PrintStats(dlf);

    LOG("Reducing double lines forest",
        STree st_dll(dlf);
    );
    PrintStats(dlf);

    DrawForestOnFile(algorithm_name + "_double_lines_reduced", dlf, true, true);

    string double_lines_forest_reduced_code = global_output_path + algorithm_name + "_double_lines_forest_reduced_code.txt";
    {
        ofstream os(double_lines_forest_reduced_code);
        GenerateForestCode(os, dlf, last_nodeid_main_forest_group);
    }

}

void CreateRonsefeldLabeling() {
    auto at = ruleset_generator_type::rosenfeld;

    auto algorithm_name = ruleset_generator_names[static_cast<int>(at)];
    auto ruleset_generator = ruleset_generator_functions[static_cast<int>(at)];

    auto rs = ruleset_generator();

    string odt_filename = global_output_path + algorithm_name + "_odt.txt";
    ltree t;
    if (!LoadConactTree(t, odt_filename)) {
        t = GenerateOdt(rs, odt_filename);
    }

    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, t, false, true);
}


void CreateThinning() {
    auto at = ruleset_generator_type::thin_ch;
    auto algorithm_name = ruleset_generator_names[static_cast<int>(at)];
    auto ruleset_generator = ruleset_generator_functions[static_cast<int>(at)];
    auto rs = ruleset_generator();

    //GenerateConditionsActionsCode("condition_action.txt", rs);
    
    string odt_filename = global_output_path + algorithm_name + "_odt.txt";
    ltree t;
    if (!LoadConactTree(t, odt_filename)) {
        t = GenerateOdt(rs, odt_filename);
    }
    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, t, false, true);
    PrintStats(t);

    string tree_code_filename = global_output_path + algorithm_name + "tree_code.txt";
    GenerateCode(tree_code_filename, t);

    LOG("Making forest",
        Forest f(t, rs.ps_);
    );
    for (size_t i = 0; i < f.trees_.size(); ++i) {
        DrawDagOnFile(algorithm_name + "tree" + zerostr(i, 4), f.trees_[i], true);
    }
    PrintStats(f);

    string forest_code_nodag = global_output_path + algorithm_name + "_forest_nodag_frequencies_code.txt";
    {
        ofstream os(forest_code_nodag);
        GenerateForestCode(os, f);
    }

    for (size_t i = 0; i < f.end_trees_.size(); ++i) {
        for (size_t j = 0; j < f.end_trees_[i].size(); ++j) {
            DrawDagOnFile(algorithm_name + "_end_tree_" + zerostr(i, 2) + "_" + zerostr(j, 2), f.end_trees_[i][j], false);
        }
    }

    LOG("Converting forest to dag",
    	Forest2Dag x(f);
    	for (size_t i = 0; i < f.trees_.size(); ++i) {
    		DrawDagOnFile(algorithm_name + "drag" + zerostr(i, 4), f.trees_[i], true);
    	}
    );
    PrintStats(f);
    
    DrawForestOnFile(algorithm_name + "forest", f, true);

    string forest_code = global_output_path + algorithm_name + "_forest_identities_code.txt";
    {
        ofstream os(forest_code);
        GenerateForestCode(os, f);
    }

    return;
}

int main()
{
    //CreateRonsefeldLabeling();
    //CreateSpaghettiLabeling();
    //CreateChangLabeling();
    //CreateCtbeLabeling();
    CreateRonsefeldLabeling();

	return EXIT_SUCCESS;
}