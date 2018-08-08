#include "ruleset_generator.h"

#include <bitset>
#include <string>

#include "merge_set.h"

using namespace std;

static bool g_debug_log = false;

rule_set GenerateRosenfeld() 
{
    pixel_set rosenfeld_mask {
        { "p", -1, -1 }, { "q", 0, -1 }, { "r", +1, -1 },
        { "s", -1,  0 }, { "x", 0, 0 },
    };

    rule_set labeling;
    labeling.init_conditions(rosenfeld_mask);
    labeling.init_actions({
        "nothing",
        "x<-newlabel",
        "x<-p", "x<-q", "x<-r", "x<-s",
        "x<-p+q", "x<-p+r", "x<-p+s", "x<-q+r", "x<-q+s", "x<-r+s",
        "x<-p+q+r", "x<-p+q+s", "x<-p+r+s", "x<-q+r+s",
        "x<-p+q+r+s",
        });


    labeling.generate_rules([](rule_set& rs, uint i) {
        rule_wrapper r(rs, i);

        bool X = r["x"];
        if (!X) {
            r << "nothing";
            return;
        }

        connectivity_mat<5> con({ "p", "q", "r", "s", "x" });

        con.set("x", "p", r["p"]);
        con.set("x", "q", r["q"]);
        con.set("x", "r", r["r"]);
        con.set("x", "s", r["s"]);

        con.set("p", "q", r["p"] && r["q"]);
        con.set("p", "s", r["p"] && r["s"]);
        con.set("q", "r", r["q"] && r["r"]);
        con.set("q", "s", r["q"] && r["s"]);

        con.set("p", "r", con("p", "q") && con("q", "r"));
        con.set("s", "r", (con("p", "r") && con("p", "s")) || (con("s", "q") && con("q", "r")));

        if (g_debug_log) {
            con.DisplayCondNames();
            string bits = bitset<5>(i).to_string();
            reverse(begin(bits), end(bits));
            cout << bits << "\n";
            con.DisplayMap();
            cout << endl;
        }

        MergeSet<5> ms(con);
        ms.BuildMergeSet();

        for (const auto& s : ms.mergesets_) {
            string action = "x<-";
            if (s.empty())
                action += "newlabel";
            else {
                action += s[0];
                for (size_t i = 1; i < s.size(); ++i)
                    action += "+" + s[i];
            }
            r << action;
        }
    });

    return labeling;
}
