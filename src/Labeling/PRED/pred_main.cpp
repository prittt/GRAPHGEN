// Copyright(c) 2018 - 2019 Costantino Grana, Federico Bolelli 
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

#include "yaml-cpp/yaml.h"

#include "graphsgen.h"

#include "rosenfeld_ruleset.h"

using namespace std;

struct Save {
    std::ostream& os_;
    std::unordered_set<BinaryDrag<conact>::node*> visited_;
    std::unordered_map<BinaryDrag<conact>::node*, int> nodes_with_refs_;
    int id_ = 0;

    void CheckNodesTraversalRec(BinaryDrag<conact>::node *n)
    {
        if (nodes_with_refs_.find(n) != end(nodes_with_refs_)) {
            if (!nodes_with_refs_[n])
                nodes_with_refs_[n] = ++id_;
        }
        else {
            nodes_with_refs_[n] = 0;
            if (!n->isleaf()) {
                CheckNodesTraversalRec(n->left);
                CheckNodesTraversalRec(n->right);
            }
        }
    }

    Save(std::ostream& os, BinaryDrag<conact>& bd) : os_(os)
    {
        for (auto& x : bd.roots_)
            CheckNodesTraversalRec(x);
        for (auto& x: bd.roots_)
            SaveRec(x);
    }

    void SaveRec(BinaryDrag<conact>::node* n, int tab = 0)
    {
        os_ << string(tab, '\t');

        if (!visited_.insert(n).second) {
            os_ << "@ " << nodes_with_refs_[n] << "\n";
            return;
        }

        if (nodes_with_refs_[n])
            os_ << "^ " << nodes_with_refs_[n] << " ";

        if (n->isleaf()) {
            assert(n->data.t == conact::type::ACTION);
            auto a = n->data.actions();
            os_ << ". " << a[0];
            for (size_t i = 1; i < a.size(); ++i)
                os_ << "," << a[i];
            os_ << " - " << n->data.next << "\n";
        }
        else {
            assert(n->data.t == conact::type::CONDITION);
            os_ << n->data.condition << "\n";
            SaveRec(n->left, tab + 1);
            SaveRec(n->right, tab + 1);
        }
    }
};

struct Load {
    std::istream& is_;
    BinaryDrag<conact>& bd_;
    std::vector<BinaryDrag<conact>::node*> np_;

    Load(std::istream& is, BinaryDrag<conact>& bd) : is_(is), bd_(bd)
    {
        np_.push_back(nullptr);
        while (true) {
            ltree::node* n = LoadConactTreeRec();
            if (n == nullptr)
                break;
            bd.AddRoot(n);
        }
    }

    BinaryDrag<conact>::node* LoadConactTreeRec()
    {
        string s;
        while (is_ >> s) {
            if (s[0] == '#')
                getline(is_, s);
            else
                break;
        }

        if (!is_)
            return nullptr;

        if (s == "@") {
            int pos;
            is_ >> pos;
            return np_[pos];
        }

        auto n = bd_.make_node();

        if (s == "^") {
            int pos;
            is_ >> pos >> s;
            if (pos != np_.size())
                throw;
            np_.push_back(n);
        }

        if (s == ".") {
            // leaf
            n->data.t = conact::type::ACTION;
            do {
                int action;
                is_ >> action >> ws;
                n->data.action.set(action - 1);
            } while (is_.peek() == ',' && is_.get());
            
            if (is_.get() != '-')
                throw;            
            is_ >> n->data.next;
        }
        else {
            // real node with branches
            n->data.t = conact::type::CONDITION;
            n->data.condition = s;

            n->left = LoadConactTreeRec();
            n->right = LoadConactTreeRec();
        }

        return n;
    }
};

int main()
{
    // Setup configuration
    string algorithm_name = "PRED";
    conf = ConfigData(algorithm_name);

    // Load or generate rules
    RosenfeldRS r_rs;
    auto rs = r_rs.GetRuleSet();

    // Call GRAPHSGEN:
    // 1) Load or generate Optimal Decision Tree based on Rosenfeld mask
    BinaryDrag<conact> bd = GetOdt(rs, algorithm_name);
    
    // 2) Draw the generated tree on file
    string tree_filename = algorithm_name + "_tree";
    DrawDagOnFile(tree_filename, bd);
    
    // 3) Generate forests of trees
    LOG(algorithm_name + " - making forests",
        ForestHandler fh(bd, rs.ps_);
    );

    fh.Compress(DragCompressor::PRINT_STATUS_BAR | DragCompressor::IGNORE_LEAVES);

    // 4) Draw the generated forests on file
    fh.DrawOnFile(algorithm_name, DELETE_DOTCODE);

    {
        ofstream os("prova.txt");
        Save(os, fh.GetLineForestHandler(ForestHandler::CENTER_LINES).f_);
    }
    {
        ifstream is("prova.txt");
        BinaryDrag<conact> bd;
        Load(is, bd);
        bd.roots_;
        fh.DrawOnFile("prova", DELETE_DOTCODE);
    }

    // 5) Generate the C/C++ source code
    fh.GenerateCode();
    GeneratePointersConditionsActionsCode(rs, true);

	return EXIT_SUCCESS;
}