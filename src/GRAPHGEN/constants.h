
#pragma once

// The rules are written into separate files (partitions).
// To balance file IO and memory consumption, the rules of each partition are generated and written to file in batches. 
// Once all rules of one batch have been generated and saved in memory, all these rules are written to the partition.

#define BUILD 3

#if BUILD == 0 // BBDT
	constexpr int PARTITIONS = 1; // 1, 1024, 65536
	constexpr int BATCHES = 32;

	constexpr int CONDITION_COUNT = 16; // 8, 14, 16, 36
	constexpr int ACTION_COUNT = 16; // 5, 77, 16, 5813 (reduced 2829)

	const std::string GLOBAL_RULEFILES_BASEPATH_OVERRIDE = "";
#endif


#if BUILD == 1 // BBDT3D-36c-222b
	constexpr int PARTITIONS = 65536; // 1, 1024, 65536
	constexpr int BATCHES = 32;

	constexpr int CONDITION_COUNT = 36; // 8, 14, 16, 36
	constexpr int ACTION_COUNT = 2829; // 5, 77, 16, 5813 (reduced 2829)

	const std::string GLOBAL_RULEFILES_BASEPATH_OVERRIDE = "D:/rules/bbdt3d-36c-222b/zst-variable-data-format-" + std::to_string(PARTITIONS) + "p-" + std::to_string(ACTION_COUNT) + "a";
#endif

#if BUILD == 2 // BBDT3D-26c-221b
	constexpr int PARTITIONS = 1; // 1, 1024, 65536
	constexpr int BATCHES = 32;

	constexpr int CONDITION_COUNT = 26; // 8, 14, 16, 36
	constexpr int ACTION_COUNT = 2829; // 5, 77, 16, 5813 (reduced 2829)

	const std::string GLOBAL_RULEFILES_BASEPATH_OVERRIDE = "D:/rules/bbdt3d-26c-221b/zst-variable-data-format-" + std::to_string(PARTITIONS) + "p-" + std::to_string(ACTION_COUNT) + "a";
#endif

#if BUILD == 3 // BBDT3D-19c-211b
	constexpr int PARTITIONS = 1; // 1, 1024, 65536
	constexpr int BATCHES = 32;

	constexpr int CONDITION_COUNT = 19; // 8, 14, 16, 36
	constexpr int ACTION_COUNT = 2829; // 5, 77, 16, 5813 (reduced 2829)

	const std::string GLOBAL_RULEFILES_BASEPATH_OVERRIDE = "D:/rules/bbdt3d-19c-211b/zst-variable-data-format-" + std::to_string(PARTITIONS) + "p-" + std::to_string(ACTION_COUNT) + "a";
#endif

#if BUILD == 4 // SAUF
	constexpr int PARTITIONS = 1; 
	constexpr int BATCHES = 32;

	constexpr int CONDITION_COUNT = 5; 
	constexpr int ACTION_COUNT = 8; 

	const std::string GLOBAL_RULEFILES_BASEPATH_OVERRIDE = "";
#endif

#if BUILD == 5 // SAUF3D
	constexpr int PARTITIONS = 1;
	constexpr int BATCHES = 32;

	constexpr int CONDITION_COUNT = 14;
	constexpr int ACTION_COUNT = 77;

	const std::string GLOBAL_RULEFILES_BASEPATH_OVERRIDE = "";
#endif

constexpr uint64_t TOTAL_RULES = (1ULL << CONDITION_COUNT);
constexpr size_t RULES_PER_PARTITION = TOTAL_RULES / PARTITIONS;
constexpr size_t RULES_PER_BATCH = RULES_PER_PARTITION / BATCHES;