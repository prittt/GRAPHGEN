#include "connectivity_graph.h"

#include <set>
//#include <unordered_set>
#include <iterator>
#include <bitset>

// This function creates and initializes an adjacencies graph with name and values starting from a pixel_set
graph MakeAdjacencies(const pixel_set& ps) {
    graph g(ps.size());

    for (size_t i = 0; i < g.size(); ++i)
        g.set_name(i, ps[i].name_);

    for (size_t i = 0; i < g.size(); ++i) {
        for (size_t j = i; j < g.size(); ++j) {
            g[i][j] = g[j][i] = ChebyshevDistance(ps[i], ps[j]) <= 1;
        }
    }

    return g;
}

static void MakeConnectivitiesRec(size_t i, size_t j, graph& g, const graph& ag, std::set<size_t>& visited) {
    for (size_t k = 0; k < g.size(); ++k) {
        if (visited.find(k) == visited.end() && ag[j][k]) {
            g[i][k] = 1;
            visited.insert(k);
            if (g.nodes_[k] != "x")
                MakeConnectivitiesRec(i, k, g, ag, visited);
        }
    }
}
graph MakeConnectivities(const graph& ag) {
    graph g(ag.size());
    g.nodes_ = ag.nodes_;
    g.rnodes_ = ag.rnodes_;

    for (size_t i = 0; i < g.size(); ++i) {
        if (ag[i][i]) {
            std::set<size_t> visited;
            MakeConnectivitiesRec(i, i, g, ag, visited);
        }
    }

    return g;
}

static void MakeConnectivitiesRecSpecial(size_t i, size_t j, graph& g, const graph& ag, std::set<size_t>& visited, const std::vector<std::string>& pixel_list) {
    for (size_t k = 0; k < g.size(); ++k) {
        if (visited.find(k) == visited.end() && ag[j][k]) {
            g[i][k] = 1;
            visited.insert(k);
            if (std::count(pixel_list.begin(), pixel_list.end(), g.nodes_[k]) == 0)
                MakeConnectivitiesRecSpecial(i, k, g, ag, visited, pixel_list);
        }
    }
}
graph MakeConnectivitiesSpecial(const graph& ag, const std::vector<std::string>& pixel_list) {
    graph g(ag.size());
    g.nodes_ = ag.nodes_;
    g.rnodes_ = ag.rnodes_;

    for (size_t i = 0; i < g.size(); ++i) {
        if (ag[i][i]) {
            std::set<size_t> visited;
            MakeConnectivitiesRecSpecial(i, i, g, ag, visited, pixel_list);
        }
    }

    return g;
}

std::ostream& operator<<(std::ostream& os, const graph& g) {
    using std::copy; using std::begin; using std::end; using std::string;
    os << "  ";
    copy(begin(g.nodes_), end(g.nodes_), std::ostream_iterator<string>(os, " "));
    os << "\n";

    for (size_t i = 0; i < g.arcs_.size(); ++i) {
        os << g.nodes_[i] << " ";
        copy(begin(g.arcs_[i]), end(g.arcs_[i]), std::ostream_iterator<int>(os, " "));
        os << "\n";
    }

    return os;
}

bool graph::Write(const std::string& filename)
{
    std::ofstream os(filename);
    if (os) {
        os << *this;
        return true;
    }
    else {
        return false;
    }
}

// Less operator for bitset
template<size_t N>
struct less {
    bool operator()(const std::bitset<N>& lhs, const std::bitset<N>& rhs) const {
        return lhs.to_string() < rhs.to_string();
    }
};

// This function generates all possible actions, avoiding useless ones such as merges between adjacent pixels
std::vector<std::string> GenerateAllPossibleLabelingActions(const graph& ag)
{
    auto nconds = ag.size();
    auto nrules = 1u << nconds;
    std::set<std::bitset<128>, less<128>> actions_set; // Set of actions described as list of pixels to be merged
    std::vector<std::string> actions = { "nothing" };

    auto posx = ag.rnodes_.at("x");
    for (size_t rule = 0; rule < nrules; ++rule) {
        if ((rule >> posx) & 1) {
            std::bitset<128> cur_action = 0;
            std::vector<size_t> cur_conds;
            for (size_t j = 0; j < nconds; ++j) {
                if (j != posx && ((rule >> j) & 1) == 1) {
                    bool adj = false;
                    for (size_t k = 0; k < cur_conds.size(); ++k) {
                        if (ag[j][cur_conds[k]] == 1) {
                            adj = true;
                            break;
                        }
                    }
                    cur_conds.push_back(j);
                    if (!adj) {
                        cur_action.set(j);
                    }
                }
            }
            actions_set.insert(cur_action);
        }
    }

    for (const auto& a : actions_set) {
        std::string action = "x<-";
        for (size_t j = 0; j < nconds; ++j) {
            if (a[j]) {
                action += ag.nodes_[j] + "+";
            }
        }
        if (action == "x<-")
            action += "newlabel";
        else
            action.resize(action.size() - 1); // remove last + sign

        actions.push_back(action);
    }

    return actions;
}

// This function generates all possible actions, avoiding useless ones such as merges between adjacent pixels. This version allows
// to consider reference pixel with a name different from x
std::vector<std::string> GenerateAllPossibleLabelingActions(const graph& ag, const std::string& ref_pixel_name)
{
    auto nconds = ag.size();
    auto nrules = 1u << nconds;
    std::set<std::bitset<128>, less<128>> actions_set; // Set of actions described as list of pixels to be merged
    std::vector<std::string> actions = { "nothing" };

    auto posx = ag.rnodes_.at(ref_pixel_name);
    for (size_t rule = 0; rule < nrules; ++rule) {
        if ((rule >> posx) & 1) {
            std::bitset<128> cur_action = 0;
            std::vector<size_t> cur_conds;
            for (size_t j = 0; j < nconds; ++j) {
                if (j != posx && ((rule >> j) & 1) == 1) {
                    bool adj = false;
                    for (size_t k = 0; k < cur_conds.size(); ++k) {
                        if (ag[j][cur_conds[k]] == 1) {
                            adj = true;
                            break;
                        }
                    }
                    cur_conds.push_back(j);
                    if (!adj) {
                        cur_action.set(j);
                    }
                }
            }
            actions_set.insert(cur_action);
        }
    }

    for (const auto& a : actions_set) {
        std::string action = ref_pixel_name + "<-";
        for (size_t j = 0; j < nconds; ++j) {
            if (a[j]) {
                action += ag.nodes_[j] + "+";
            }
        }
        if (action == ref_pixel_name + "<-")
            action += "newlabel";
        else
            action.resize(action.size() - 1); // remove last + sign

        actions.push_back(action);
    }

    return actions;
}

void PrintActionsSet(std::set<std::bitset<128>, less<128>>& to_print, std::ostream& os) {
    for (const auto& x : to_print) {
        os << x.to_string().substr(119) << "\n";
    }
}

///VERSION WITH MANY MORE ACTIONS THAN NECESSARY
//// This function generates all possible actions, avoiding useless ones such as merges between adjacent pixels and considering multiple pixels inside the mask
//std::vector<std::string> GenerateAllPossibleLabelingActionsGivenTheSetOfPixelToBeLabeled(const graph& ag, const std::vector<std::string>& to_be_labeled_pixels)
//{
//    auto nconds = ag.size();
//    auto nrules = 1u << nconds;
//
//    // Vector of set of actions described as pixels to be merged (bitmapped). The vector contains as many sets as the number of
//    // pixels to be labeled
//    std::vector<std::set<std::bitset<128>, less<128>>> actions_set(to_be_labeled_pixels.size());
//    std::vector<std::vector<std::string>> actions(to_be_labeled_pixels.size());
//
//    //auto posx = ag.rnodes_.at("x");
//    std::vector<size_t> posp(to_be_labeled_pixels.size());
//    for (size_t i = 0; i < to_be_labeled_pixels.size(); ++i) {
//        posp[i] = ag.rnodes_.at(to_be_labeled_pixels[i]);
//    }
//
//    for (size_t i = 0; i < to_be_labeled_pixels.size(); ++i) {
//        auto posx = posp[i];
//        for (size_t rule = 0; rule < nrules; ++rule) {
//            if ((rule >> posx) & 1) {
//                std::bitset<128> cur_action = 0;
//                std::vector<int> cur_conds;
//                for (size_t j = 0; j < nconds; ++j) {
//
//                    bool is_in_the_labe_set = false;
//                    for (size_t lss = 0 + i; lss < to_be_labeled_pixels.size(); ++lss) {
//                        if (j == posp[lss]) {
//                            is_in_the_labe_set = true;
//                            break;
//                        }
//                    }
//
//                    if (!is_in_the_labe_set && ((rule >> j) & 1) == 1) {
//                        bool adj = false;
//                        for (size_t k = 0; k < cur_conds.size(); ++k) {
//                            if (ag[j][cur_conds[k]] == 1) {
//                                adj = true;
//                                break;
//                            }
//                        }
//                        cur_conds.push_back(j);
//                        if (!adj) {
//                            cur_action.set(j);
//                        }
//                    }
//                }
//                actions_set[i].insert(cur_action);
//            }
//        }
//    }
//
//
//    {
//        std::ofstream os("actions_set.txt");
//        for (int i = ag.nodes_.size() - 1; i >= 0; --i) {
//            os << ag.nodes_[i];
//        }
//        os << "\n";
//        PrintActionsSet(actions_set[0], os);
//        os << "\n";
//        PrintActionsSet(actions_set[1], os);
//        os << "\n";
//        PrintActionsSet(actions_set[2], os);
//
//    }
//
//    for (size_t i = 0; i < actions_set.size(); ++i) {
//        std::vector<std::string> cur_actions = { "nothing" };
//        auto cur_actions_set = actions_set[i];
//        for (const auto& a : cur_actions_set) {
//            std::string action = to_be_labeled_pixels[i] + "<-";
//            for (size_t j = 0; j < nconds; ++j) {
//                if (a[j]) {
//                    action += ag.nodes_[j] + "+";
//                }
//            }
//            if (action == to_be_labeled_pixels[i] + "<-")
//                action += "newlabel";
//            else
//                action.resize(action.size() - 1); // remove last + sign
//
//            cur_actions.push_back(action);
//        }
//        actions[i] = cur_actions;
//    }
//
//    //std::set<std::string> final_actions;
//    std::vector<std::string> final_actions;
//
//    for (size_t e = 0; e < actions[0].size(); ++e) {
//        for (size_t g = 0; g < actions[1].size(); ++g) {
//            for (size_t i = 0; i < actions[2].size(); ++i) {
//                std::string es = actions[0][e], gs = actions[1][g], is = actions[2][i];
//                /*if (es.substr(3) == gs.substr(3) && es != "nothing") {
//                    gs = "g<-e";
//                }
//
//                if (es.substr(3) == is.substr(3) && es != "nothing") {
//                    is = "i<-e";
//                }
//
//                if (gs.substr(3) == is.substr(3) && gs != "nothing") {
//                    is = "i<-g";
//                }*/
//                
//                //final_actions.insert(es + "," + gs + "," + is);
//                final_actions.push_back(es + "," + gs + "," + is);
//            }
//        }
//    }
//
//    /*std::vector<std::string> return_val(final_actions.size());
//    std::copy(final_actions.begin(), final_actions.end(), return_val.begin());
//
//    return return_val;*/
//    return final_actions;
//}
///VERSIONE CON MOLTE PIU' AZIONI DEL NECESSARIO

#include "merge_set.h"
// This function generates all possible actions, avoiding useless ones such as merges between adjacent pixels and considering multiple pixels inside the mask
std::vector<std::string> GenerateAllPossibleLabelingActionsGivenTheSetOfPixelToBeLabeled(const graph& ag, const std::vector<std::string>& to_be_labeled_pixels, rule_set& rs)
{
    std::set<std::string> actions;

    auto nconds = ag.size();
    auto nrules = 1u << nconds;

    for (int i = 0; i < static_cast<int>(nrules); ++i) {
        rule_wrapper r(rs, i);

        auto lag = ag;
        for (size_t j = 0; j < lag.size(); ++j) {
            if (((i >> j) & 1) == 0)
                lag.DetachNode(j);
        }

        std::vector<std::string> e_actions;
        if (r["e"]) {
            graph cg = MakeConnectivitiesSpecial(lag, std::vector<std::string>{"e", "g", "i"});
            connectivity_mat con(rs.conditions);
            con.data_ = cg.arcs_;

            MultiMergeSet mse(con, std::vector<std::string>({ "e", "g", "i" }), std::string("e"));
            mse.BuildMergeSet();
            for (const auto& s : mse.mergesets_) {
                std::string action = "e<-";
                if (s.empty())
                    action += "newlabel";
                else {
                    action += s[0];
                    for (size_t i = 1; i < s.size(); ++i)
                        action += "+" + s[i];
                }
                e_actions.push_back(action);
            }
        }
        else {
            e_actions.push_back("nothing");
        }

        std::vector<std::string> g_actions;
        if (r["g"]) {
            graph cg = MakeConnectivitiesSpecial(lag, std::vector<std::string>{"g", "i"});
            connectivity_mat con(rs.conditions);
            con.data_ = cg.arcs_;
            MultiMergeSet msg(con, std::vector<std::string>({ "g", "i" }), std::string("g"));
            msg.BuildMergeSet();

            for (const auto& s : msg.mergesets_) {
                std::string action = "g<-";
                if (s.empty())
                    action += "newlabel";
                else {
                    action += s[0];
                    for (size_t i = 1; i < s.size(); ++i)
                        action += "+" + s[i];
                }
                g_actions.push_back(action);
            }
        }
        else {
            g_actions.push_back("nothing");
        }

        std::vector<std::string> i_actions;
        if (r["i"]) {
            graph cg = MakeConnectivitiesSpecial(lag, std::vector<std::string>{"i"});
            connectivity_mat con(rs.conditions);
            con.data_ = cg.arcs_;
            MultiMergeSet msi(con, std::vector<std::string>({ "i" }), std::string("i"));
            msi.BuildMergeSet();

            for (const auto& s : msi.mergesets_) {
                std::string action = "i<-";
                if (s.empty())
                    action += "newlabel";
                else {
                    action += s[0];
                    for (size_t i = 1; i < s.size(); ++i)
                        action += "+" + s[i];
                }
                i_actions.push_back(action);
            }
        }
        else {
            i_actions.push_back("nothing");
        }

        for (const auto& ae : e_actions) {
            for (const auto& ag : g_actions) {
                for (const auto& ai : i_actions) {
                    std::string es = ae, gs = ag, is = ai;
                    size_t found = es.find(gs.substr(3));
                    if (found != std::string::npos && es != "nothing" && es != "e<-newlabel") {
                        gs = "g<-e";
                    }
                    found = es.find(is.substr(3));
                    if (found != std::string::npos && es != "nothing" && es != "e<-newlabel") {
                        is = "i<-e";
                    }
                    found = gs.find(is.substr(3));
                    if (found != std::string::npos && gs != "nothing" && gs != "g<-newlabel") {
                        is = "i<-g";
                    }
                    actions.insert(es + "," + gs + "," + is);
                }
            }
        }
    }

    std::vector<std::string> return_val(actions.size());
    std::copy(actions.begin(), actions.end(), return_val.begin());
    
    return return_val;

    //return actions;
}