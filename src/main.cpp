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

#include <algorithm>
#include <bitset>
#include <set>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <iterator>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdint>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <cassert>

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
    DrawDagOnFile(tree_filename, t, false, true);
    print_stats(t);

    //PerformOptimalDragGeneration(t, algorithm_name);

    Forest f(t, rs.ps_);
    /*
    for (size_t i = 0; i < f.trees_.size(); ++i) {
        DrawDagOnFile("tree" + zerostr(i, 4), f.trees_[i], true);
    }*/

    struct t2d {
        unordered_map<ltree::node*, string> ps_;
        unordered_map<string, ltree::node*> sp_;
        Forest& f_;

        string Tree2String(ltree::node* n) {
            auto it = ps_.find(n);
            if (it != end(ps_))
                return it->second;

            string s;
            if (n->isleaf()) 
                s = to_string(n->data.action) + to_string(n->data.next);
            else
                s = n->data.condition + Tree2String(n->left) + Tree2String(n->right);
            
            ps_[n] = s;
            return s;
        }

        void FindAndLink(ltree::node* n) {
            if (!n->isleaf()) {
                auto s = Tree2String(n->left);

                auto it = sp_.find(s);
                if (it == end(sp_)) {
                    sp_[s] = n->left;
                    FindAndLink(n->left);
                }
                else {
                    n->left = it->second;
                }

                s = Tree2String(n->right);

                it = sp_.find(s);
                if (it == end(sp_)) {
                    sp_[s] = n->right;
                    FindAndLink(n->right);
                }
                else {
                    n->right = it->second;
                }
            }
        }

        t2d(Forest& f) : f_(f) {
            for (auto& t : f_.trees_)
                FindAndLink(t.root);
        }
    };

    t2d x(f);

    /*
    for (size_t i = 0; i < f.trees_.size(); ++i) {
        DrawDagOnFile("drag" + zerostr(i, 4), f.trees_[i], true);
    }
    */
    DrawForestOnFile("forest", f, true);

        
    /*
    Tree2DagUsingIdentities(f.trees_[2]); 
    DrawDagOnFile("prova", f.trees_[2]);
    */
    /*
    for (size_t i = 0; i < f.trees_.size(); ++i) {
        Tree2DagUsingIdentities(f.trees_[i]);
        Dag2OptimalDag(f.trees_[i]);
        DrawDagOnFile("odrag" + zerostr(i, 4), f.trees_[i]);
    }
    */
}