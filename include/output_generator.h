#ifndef TREESGENERATOR_OUTPUT_GENERATOR_H
#define TREESGENERATOR_OUTPUT_GENERATOR_H

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <sstream>

#include "condition_action.h"
#include "tree.h"
#include "utilities.h"

void print_node(tree<conact>::node *n, int i);
void GenerateDotCodeForDagRec(std::ostream& os, tree<conact>::node *n, std::map<tree<conact>::node*, int>& printed_node, std::vector<std::string>& links, nodeid &id, int tab = 1);

// All nodes must have both sons! 
void GenerateDotCodeForDag(std::ostream& os, tree<conact>& t);

// "output_file": output file name without extension 
// "t": tree<conact> to draw
// "verbose": to print messages on standard output
// return true if the process ends correctly, false otherwise
bool DrawDagOnFile(const std::string& output_file, tree<conact> &t, bool verbose = false);

#endif // !TREESGENERATOR_OUTPUT_GENERATOR_H