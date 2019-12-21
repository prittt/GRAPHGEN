
#pragma once

// The rules are written into separate files (partitions).
// To balance file IO and memory consumption, the rules of each partition are generated and written to file in batches. 
// Once all rules of one batch have been generated and saved in memory, all these rules are written to the partition.

constexpr int PARTITIONS = 1024;
constexpr int BATCHES = 32;

constexpr int CONDITION_COUNT = 36; // 8, 14, 16, 36
constexpr int ACTION_COUNT = 5813; // 5, 77, 16, 5813

constexpr uint64_t TOTAL_RULES = (1ULL << CONDITION_COUNT);
constexpr size_t RULES_PER_PARTITION = TOTAL_RULES / PARTITIONS;
constexpr size_t RULES_PER_BATCH = RULES_PER_PARTITION / BATCHES;
