#include "connectivity_graph.h"

#include <set>
//#include <unordered_set>
#include <iterator>
#include <bitset>

graph make_adjacencies(const pixel_set& ps) {
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

static void make_connectivities_rec(size_t i, size_t j, graph& g, const graph& ag, std::set<size_t>& visited) {
    for (size_t k = 0; k < g.size(); ++k) {
        if (visited.find(k) == visited.end() && ag[j][k]) {
            g[i][k] = 1;
            visited.insert(k);
            if (g.nodes_[k] != "x")
                make_connectivities_rec(i, k, g, ag, visited);
        }
    }
}
graph make_connectivities(const graph& ag) {
    graph g(ag.size());
    g.nodes_ = ag.nodes_;
    g.rnodes_ = ag.rnodes_;

    for (size_t i = 0; i < g.size(); ++i) {
        if (ag[i][i]) {
            std::set<size_t> visited;
            make_connectivities_rec(i, i, g, ag, visited);
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

template<size_t N>
struct less {
    bool operator()(const std::bitset<N>& lhs, const std::bitset<N>& rhs) {
        return lhs.to_string() < rhs.to_string();
    }
};

std::vector<std::string> generate_all_possible_labeling_actions(const graph& ag)
{
    auto nconds = ag.size();
    auto nrules = 1u << nconds;
    std::set<std::bitset<128>, less<128>> actions_set; // Insieme di azioni descritte come elenco dei pixel di cui fare il merge
    std::vector<std::string> actions = { "nothing" };

    auto posx = ag.rnodes_.at("x");
    for (size_t rule = 0; rule < nrules; ++rule) {
        if ((rule >> posx) & 1) {
            std::bitset<128> cur_action = 0;
            std::vector<int> cur_conds;
            for (size_t j = 0; j < nconds; ++j) {
                if (j != posx && ((rule >> j) & 1) == 1) {
                    bool adj = false;
                    for (size_t k = 0; k < cur_conds.size(); ++k) {
                        if (ag[j][cur_conds[k]] == 1) {
                            adj = true;
                            break;
                        }
                    }
                    if (!adj) {
                        cur_conds.push_back(j);
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

