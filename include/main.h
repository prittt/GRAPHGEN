void FindAndLinkEquivalencesRec(ltree::node* n1, ltree::node* n2) {
    if (n2->isleaf() || n1 == n2)
        return;

    if (n1 != n2->left && equivalent_trees(n1, n2->left)) {
        intersect_leaves(n1, n2->left);
        n2->left = n1;
    }

    if (n1 != n2->right && equivalent_trees(n1, n2->right)) {
        intersect_leaves(n1, n2->right);
        n2->right = n1;
    }

    FindAndLinkEquivalencesRec(n1, n2->left);
    FindAndLinkEquivalencesRec(n1, n2->right);
}


// Recursive auxiliary function for the conversion of a tree into DAG
void Tree2DagUsingEquivalencesRec(ltree::node *n, ltree& t) {
    FindAndLinkEquivalencesRec(n, t.root);

    if (!n->isleaf()) {
        Tree2DagUsingEquivalencesRec(n->left, t);
        Tree2DagUsingEquivalencesRec(n->right, t);
    }
}

// Converts a tree into dag considering equivalences between subtrees
void Tree2DagUsingEquivalences(ltree& t) {
    Tree2DagUsingEquivalencesRec(t.root, t);
}





//// Given a tree with multiple actions on leaves this function generate all possible subtree with only one action per leaf
//void Tree2OptimalDagRec(ltree& t, ltree::node* n, vector<ltree>& trees) {
//    ltree nt;
//    if (n->isleaf()) {
//        // leaf with multiple action
//        vector<uint> actions_list = n->data.actions();
//        if (actions_list.size() > 1) {
//            for (size_t i = 0; i < actions_list.size() - 1; ++i) {
//                n->data.action = 1 << (actions_list[i] - 1);
//                nt = t;
//                Tree2OptimalDagRec(nt, nt.root, trees);
//            }
//            n->data.action = 1 << (actions_list[actions_list.size() - 1] - 1);
//        }
//        return;
//    }
//
//    Tree2OptimalDagRec(t, n->left, trees);
//    Tree2OptimalDagRec(t, n->right, trees);
//
//    if (t.root == n) {
//        trees.push_back(t);
//    }
//
//    return;
//}

//// Given a dag with multiple actions on leaves this function generate all possible dags with only one action per leaf
//void Tree2OptimalDagRec(ltree& t, ltree::node* n, vector<ltree>& trees, std::map<const ltree::node*, bool> &visited_n, uint &counter) {
//    ltree nt;
//    if (n->isleaf()) {
//        // leaf with multiple action
//        vector<uint> actions_list = n->data.actions();
//        if (actions_list.size() > 1) {
//            for (size_t i = 0; i < actions_list.size() - 1; ++i) {
//                std::map<const ltree::node*, bool> visited_node_cur;
//                n->data.action = 1 << (actions_list[i] - 1);
//                nt = t;
//                Tree2OptimalDagRec(nt, nt.root, trees, visited_node_cur, counter);
//            }
//            n->data.action = 1 << (actions_list[actions_list.size() - 1] - 1);
//        }
//        return;
//    }
//
//    if (!visited_n[n->left]) {
//        // left node not already visited
//        visited_n[n->left] = true;
//        Tree2OptimalDagRec(t, n->left, trees, visited_n, counter);
//    }
//
//    if (!visited_n[n->right]) {
//        // right node not already visited
//        visited_n[n->right] = true;
//        Tree2OptimalDagRec(t, n->right, trees, visited_n, counter);
//    }
//
//    if (t.root == n) {
//        trees.push_back(t);
//    }
//}



// Specific for bbdt (TODO: do it better)
struct TreePathFreq{
    vector<unsigned long long> paths_freq;

    void Generate(const ltree& t, const rule_set& rs) {
        paths_freq = vector<unsigned long long>(65536, 0);
        vector<int> cur_path(16,-1);
        GenerateRec(t.root, 0, cur_path, 0, rs);
    }

    void GenerateRec(const ltree::node *n, uint height, vector<int> cur_rule, uint cur_path, const rule_set& rs) {
        if (n->isleaf()) {
            unsigned long long freq = 0; 
            for (uint i = 0; i < rs.rules.size(); ++i) {
                bool stop = false;
                for (uint j = 0; j < cur_rule.size() && !stop; ++j) {
                    if (cur_rule[j] > -1) {
                        uint rule_bit = (i >> j) & 1;
                        if (cur_rule[j] != rule_bit) {
                            stop = true;
                        }
                    }
                }
                if (!stop) {
                    if(freq > freq + (height * rs.rules[i].frequency)){
                        cout << "OVERFLOW ..";
                    }
                    freq += height * rs.rules[i].frequency;
                }
            }

            paths_freq[cur_path] = freq;
            return;
        }
        else {
            cur_path <<= 1;
            cur_rule[rs.conditions_pos.at(n->data.condition)] = 0;
            GenerateRec(n->left, height + 1, cur_rule, cur_path, rs);
            cur_path++;
            cur_rule[rs.conditions_pos.at(n->data.condition)] = 1;
            GenerateRec(n->right, height + 1, cur_rule, cur_path, rs);
        }
    }

};

// Returns the "best" (with minimal number of nodes) DAG of a list of DAGs
void BestDagFromList(const vector<ltree>& dags, ltree &t)
{
    uint best_nodes = numeric_limits<uint>::max();
    uint best_leaves = numeric_limits<uint>::max();
    for (size_t i = 0; i < dags.size(); ++i) {
        set<const ltree::node*> visited_node, visited_leaves;
        CountDagNodes(dags[i].root, visited_node, visited_leaves);
        if (best_nodes > visited_node.size()) {
            best_nodes = visited_node.size();
            best_leaves = visited_leaves.size();
            t = dags[i];
        }
        else if (best_nodes == visited_node.size() && best_leaves > visited_leaves.size()) {
            best_leaves = visited_leaves.size();
            t = dags[i];
        }
    }
}

//// Converts a tree into dag minimizing the number of nodes (Note: this is "necessary" when the leaves of a tree contain multiple actions)
//void Tree2OptimalDag(ltree& t) {
//    vector<ltree> trees;
//    std::map<ltree::node*, bool> visited_nodes;
//    uint counter = 0; 
//    Tree2OptimalDagRec(t, t.root, trees, visited_nodes, counter);
//
//    for (size_t i = 0; i < trees.size(); ++i) {
//        DrawDagOnFile("tree_" + to_string(i), trees[i]);
//        Dag2DagUsingIdenties(trees[i]);
//        DrawDagOnFile("dag_" + to_string(i), trees[i]);
//    }
//
//    BestDagFromList(trees, t);
//}


// This function counts the number of nodes in a dag
void CalculateDagWeight(const ltree::node *n, unsigned long long& weight, set<const ltree::node*> &visited_nodes, const TreePathFreq& tpf, uint cur_path = 0)
{
    if (n->isleaf()) {
        //if (weight > weight + tpf.paths_freq[cur_path]) {
        //    cout << "OVERFLOW during tree weight calculation" << endl;
        //}
        weight += tpf.paths_freq[cur_path];
        return;
    }

    if (visited_nodes.insert(n).second) {
        cur_path <<= 1;
        CalculateDagWeight(n->left, weight, visited_nodes, tpf);
        cur_path++;
        CalculateDagWeight(n->right, weight, visited_nodes, tpf);
    }
}


// Given a dag with multiple actions on leaves this function generate all possible dags with only one action per leaf
// VERSIONE CHE USA LE FREQUENZE PER DECIDERE QUAL E' L'ALBERO MIGLIORE (
void Tree2OptimalDagFreqRec(ltree& t, ltree::node* n, ltree &best_tree, uint &best_nodes, uint &best_leaves, unsigned long long best_weight, std::map<const ltree::node*, bool> &visited_n, uint &counter, const TreePathFreq& tpf) {
    ltree nt;
    if (n->isleaf()) {
        // leaf with multiple action
        vector<uint> actions_list = n->data.actions();
        if (actions_list.size() > 1) {
            for (size_t i = 0; i < actions_list.size() - 1; ++i) {
                std::map<const ltree::node*, bool> visited_node_cur;
                n->data.action = 1 << (actions_list[i] - 1);
                nt = t;
                Tree2OptimalDagFreqRec(nt, nt.root, best_tree, best_nodes, best_leaves, best_weight, visited_node_cur, counter, tpf);
            }
            n->data.action = 1 << (actions_list[actions_list.size() - 1] - 1);
        }
        return;
    }

    if (!visited_n[n->left]) {
        // left node not already visited
        visited_n[n->left] = true;
        Tree2OptimalDagFreqRec(t, n->left, best_tree, best_nodes, best_leaves, best_weight, visited_n, counter, tpf);
    }

    if (!visited_n[n->right]) {
        // right node not already visited
        visited_n[n->right] = true;
        Tree2OptimalDagFreqRec(t, n->right, best_tree, best_nodes, best_leaves, best_weight, visited_n, counter, tpf);
    }

    if (t.root == n) {
        counter++;
        ltree dag = t;
        uint height = 0;
        Dag2DagUsingIdenties(dag);
        unsigned long long weight = 0; 
        {
            set<const ltree::node*> visited_nodes;
            CalculateDagWeight(dag.root, weight, visited_nodes, tpf);
        }
        set<const ltree::node*> visited_nodes, visited_leaves;
        CountDagNodes(dag.root, visited_nodes, visited_leaves);

        if (weight < best_weight) {
            best_weight = weight;
            best_nodes = visited_nodes.size();
            best_leaves = visited_leaves.size();
            best_tree = dag;
        }
        else if (weight == best_weight && visited_nodes.size() < best_nodes) {
            best_nodes = visited_nodes.size();
            best_leaves = visited_leaves.size();
            best_tree = dag;
        }
        else if (weight == best_weight && visited_nodes.size() == best_nodes && visited_leaves.size() < best_leaves) {
            best_leaves = visited_leaves.size();
            best_tree = dag;
        }

        if (counter % 1000 == 0) {
            cout << counter / 1000 << "\r";
        }
    }
}

// Converts a tree into dag minimizing the number of nodes (Note: this is "necessary" when the leaves of a tree contain multiple actions)
// UTILIZZA LE FREQUENZE PER SCEGLIERE IL DAG OTTIMO
void Tree2OptimalDagFreq(ltree& t, const TreePathFreq& tpf) {
    vector<ltree> trees;
    ltree best_tree;
    std::map<const ltree::node*, bool> visited_nodes;
    uint counter = 0;
    uint best_nodes = numeric_limits<uint>::max();
    uint best_leaves = numeric_limits<uint>::max();
    unsigned long long best_weight = numeric_limits<unsigned long long>::max();

    Tree2OptimalDagFreqRec(t, t.root, best_tree, best_nodes, best_leaves, best_weight, visited_nodes, counter, tpf);
    cout << "** Vector size:" << counter << " **\n";
    cout << "** Counter:" << counter << " **\n";

    t = best_tree;

    /*for (size_t i = 0; i < trees.size(); ++i) {
    DrawDagOnFile("tree_" + to_string(i), trees[i]);
    Dag2DagUsingIdenties(trees[i]);
    DrawDagOnFile("dag_" + to_string(i), trees[i]);
    }

    BestDagFromList(trees, t);*/
}