// Copyright(c) 2018 Costantino Grana, Federico Bolelli 
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
//
// *Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and / or other materials provided with the distribution.
//
// * Neither the name of GRAPHSGEN nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

// DA RIGUARDARE COSA FACCIAMO NEL PAPER E SCRIVERE LA GENERAZIONE ALLO STESSO MODO
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

	// Inutile, non ci sono sottoalberi identici nel caso della maschera di resenfeld 3d
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

	//LOG("Converting forest to dag",
	//	Forest2Dag x(f);
	//	for (size_t i = 0; i < f.trees_.size(); ++i) {
	//		DrawDagOnFile(algorithm_name + "drag" + zerostr(i, 4), f.trees_[i], true);
	//	}
	//);
	//PrintStats(f);
	//
	//DrawForestOnFile(algorithm_name + "forest", f, true);

	string forest_code = global_output_path + algorithm_name + "_forest_identities_code.txt";
	{
		ofstream os(forest_code);
		GenerateForestCode(os, f);
	}

	//LOG("Reducing forest",
	//	STree st(f);
	//);
	//PrintStats(f);

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

	/*LOG("Converting first line forest to dag",
	Forest2Dag y(flf);
	);
	PrintStats(flf);

	LOG("Reducing first line forest",
	STree st_fl(flf);
	);
	PrintStats(flf);*/

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

int main()
{
	

	return EXIT_SUCCESS;
}