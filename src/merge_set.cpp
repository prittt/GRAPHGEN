#include "merge_set.h"

static connectivity_mat<5> test_con({ "c1", "c2", "c3", "c4", "c5" });
static MergeSet<5> test_ms(test_con);