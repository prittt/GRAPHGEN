
#pragma once

constexpr int PARTITIONS = 1;
constexpr int BATCHES = 1;

constexpr int CONDITION_COUNT = 16; // 8, 14, 16, 36
constexpr int ACTION_COUNT = 16; // 5, 77, 16, 5813

constexpr ullong RULES_PER_PARTITION = (1ULL << CONDITION_COUNT) / PARTITIONS;
