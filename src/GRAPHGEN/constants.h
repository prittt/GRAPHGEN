
#pragma once

constexpr int PARTITIONS = 1;
constexpr int BATCHES = 1;

constexpr auto CONDITION_COUNT = 16; // 8, 14, 16, 36
constexpr auto ACTION_COUNT = 16; // 5, 77, 16, 5813

constexpr ullong rules_per_partition = (1ULL << CONDITION_COUNT) / PARTITIONS;
