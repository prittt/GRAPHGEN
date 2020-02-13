// Copyright(c) 2019 Maximilian Söchting
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
// * Neither the name of GRAPHGEN nor the names of its
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

#define _CRT_SECURE_NO_WARNINGS

#include <random>
#include <ctime>
#include <algorithm>
#include <thread>

#include "heuristics.h"
#include "utilities.h"
#include "drag_statistics.h"

#include "constants.h"

#define HDT_USE_FINISHED_TREE_ONLY false && HDT_DEPTH_PROGRESS_ENABLED

constexpr std::array<const char*, 4> HDT_ACTION_SOURCE_STRINGS = { "**** !! ZERO ACTION = GARBAGE DATA !! ****", "Memory (pre-generated or read from rule file)", "Generation during run-time", "Binary rule files" };
#define HDT_ACTION_SOURCE 3 /* Source of the actions. 4/Binary Rule Files is the most common option. */
#define HDT_COMBINED_CLASSIFIER true /* Count popularity based on all the actions, not just on the single-action tables. Makes everything much faster. */
#define HDT_INFORMATION_GAIN_METHOD_VERSION 2 /* Select a different formula on how information gain is calculated */

#define HDT_GLOBAL_EQUIVALENT_ACTIONS_COUNTING_PASS false /* Count equivalent actions globally in a separate pass. May improve accuracy? */

#define HDT_BENCHMARK_READAPPLY_ENABLED false /* Benchmark the performance of the ReadApply function */
#define HDT_BENCHMARK_READAPPLY_DEPTH 12 /* Choose from: 12, 14, 16. Higher depth means more but smaller recursion instances */
#define HDT_BENCHMARK_READAPPLY_PAUSE_FOR_MEMORY_MEASUREMENTS false && HDT_BENCHMARK_READAPPLY_ENABLED /* Pause the program at different points to allow for measuring memory usage manually */
#define HDT_BENCHMARK_READAPPLY_CORRECTNESS_TEST_GENERATE_FILE false /* Generate the ground truth files for the current configuration */
#define HDT_BENCHMARK_READAPPLY_CORRECTNESS_TEST_ENABLED true /* Whether a correctness test should be executed after the benchmark */

#define HDT_DEPTH_PROGRESS_ENABLED true /* Saves and loads depth progress files after every completed depth */
#define HDT_TABLE_PROGRESS_ENABLED false && !HDT_BENCHMARK_READAPPLY_ENABLED /* Saves and loads table progress files before processing, requires benchmark to be turned off */

#define HDT_READAPPLY_VERBOSE_TIMINGS false /* Display separate timings of loading the partitions and processing the rules in memory */

#define HDT_PARALLEL_INNERLOOP_ENABLED false /* Old solution for parallelization. If partition-based is off, this can be used to scale up without increasing memory. */
#define HDT_PARALLEL_INNERLOOP_NUMTHREADS 2

#define HDT_PARALLEL_PARTITIONBASED_ENABLED true /* Best approach at the moment. Allows for great parallelization and is generally faster for BBDT3D-36. For smaller problems like BBDT2D, turning this off may be faster (rule-based approach). */
constexpr auto HDT_PARALLEL_PARTITIONBASED_PRELOADED_PARTITIONS = std::min(PARTITIONS, 16);
constexpr auto HDT_PARALLEL_PARTITIONBASED_IDLE_MS = 1;
#define HDT_PARALLEL_PARTITIONBASED_NUMTHREADS_TOTAL 8
#define HDT_PARALLEL_PARTITIONBASED_NUMTHREADS_PARTITION_READING_INITIAL 1
#define HDT_PARALLEL_PARTITIONBASED_NUMTHREADS_PROCESSING_INITIAL (HDT_PARALLEL_PARTITIONBASED_NUMTHREADS_TOTAL - HDT_PARALLEL_PARTITIONBASED_NUMTHREADS_PARTITION_READING_INITIAL)

constexpr auto HDT_RECURSION_INSTANCE_GROUP_SIZE = 600000;

#define HDT_PROCESS_NODE_LOGGING_ENABLED true /* Write log files for all the recursion instances as they are processed. Very helpful for debugging. */

// How many log outputs will be done for each pass, i.e. the higher this number, the more and regular output there will be
constexpr int HDT_GENERATION_LOG_FREQUENCY = std::min(1, PARTITIONS);

// BBDT3D-36: from depth 5 on INT can be used, before that ULLONG needs to be used to prevent overflow.
using ActionCounter = int; // int, ullong


uint64_t rule_accesses = 0;
uint64_t necessary_rule_accesses = 0;

constexpr int32_t ceiling(float num)
{
	return (static_cast<float>(static_cast<int32_t>(num)) == num)
		? static_cast<int32_t>(num)
		: static_cast<int32_t>(num) + ((num > 0) ? 1 : 0);
}

constexpr int LAZY_COUNTING_VECTOR_PARTITIONS_INTERVAL = 256;
constexpr int LAZY_COUNTING_VECTOR_PARTITIONS_COUNT = ceiling(ACTION_COUNT * 1.f / LAZY_COUNTING_VECTOR_PARTITIONS_INTERVAL);
constexpr static ActionCounter ZERO = 0;



#pragma region Data Structures

struct LazyCountingVector {
	std::vector<std::vector<ActionCounter>> data_;

	LazyCountingVector(bool allocate = true) {
		if (allocate) {
			allocateTables();
		}
	}

	void allocateTables() {
		data_.resize(LAZY_COUNTING_VECTOR_PARTITIONS_COUNT);
	}

	void deallocateTables() {
		data_.clear();
	}

	const size_t size() const {
		return ACTION_COUNT;
	}

	const size_t sizeInMemory() const {
		size_t result = 0;
		for (const auto& t : data_) {
			result += t.size();
		}
		return result;
	}

	const void print(std::ostream& oss) const {
		if (data_.size() == 0) {
			return;
		}
		ActionCounter x;
		for (size_t i = 0; i < size(); i++) {
			if ((x = operator[](i)) != 0) {
				oss << i << ": " << x << "\n";
			}
		}
	}

	// writable non-const return type, creates new partition.
	ActionCounter& operator[](const size_t t) {
		auto& s = data_[t / LAZY_COUNTING_VECTOR_PARTITIONS_INTERVAL];
		if (s.empty()) {
			if (t / LAZY_COUNTING_VECTOR_PARTITIONS_INTERVAL == LAZY_COUNTING_VECTOR_PARTITIONS_COUNT - 1) {
				s.resize(ACTION_COUNT % LAZY_COUNTING_VECTOR_PARTITIONS_INTERVAL, 0);
			} else {
				s.resize(LAZY_COUNTING_VECTOR_PARTITIONS_INTERVAL, 0);
			}
		}
		return s[t % LAZY_COUNTING_VECTOR_PARTITIONS_INTERVAL];
	}

	// read only return type, do not create new partitions, instead return 0.
	const ActionCounter& operator[](const size_t t) const {
		auto& s = data_[t / LAZY_COUNTING_VECTOR_PARTITIONS_INTERVAL];
		if (s.empty()) {
			return ZERO;
		}
		return s[t % LAZY_COUNTING_VECTOR_PARTITIONS_INTERVAL];
	}
};

struct RuleCodeShift {
	ullong mask;
	int shift;
	RuleCodeShift(ullong mask, int shift) : mask(mask), shift(shift) {};
};

struct RecursionInstance {
	// parameters
	std::vector<int> conditions;
	ullong set_conditions0; // assume: condition count < 64
	ullong set_conditions1;
	BinaryDrag<conact>::node* parent;

	// local vars
	std::vector<LazyCountingVector> single_actions;
	LazyCountingVector all_single_actions = LazyCountingVector(false);
	LazyCountingVector all_single_equivalent_actions = LazyCountingVector(false);
#if HDT_COMBINED_CLASSIFIER == false
	std::vector<std::array<int, 2>> most_probable_action_index_;
	std::vector<std::array<int, 2>> most_probable_action_occurences_;
#endif
	// state
	bool processed = false;
	bool loaded_from_file = false;
	int counted_partitions = 0;

	void initialize(std::vector<int> c,
		ullong sc0,
		ullong sc1,
		BinaryDrag<conact>::node* p) {
		conditions = c;
		set_conditions0 = sc0;
		set_conditions1 = sc1;
		parent = p;
		prepareRuleCodeShifts();
		findNextRuleCode();
#if HDT_COMBINED_CLASSIFIER == false
		most_probable_action_index_.resize(CONDITION_COUNT, std::array<int, 2>());
		most_probable_action_occurences_.resize(CONDITION_COUNT, std::array<int, 2>());
#endif
	}

	void allocateTables() {
#if HDT_GLOBAL_EQUIVALENT_ACTIONS_COUNTING_PASS == true
		all_single_equivalent_actions.allocateTables();
#endif
		all_single_actions.allocateTables();
		single_actions.resize(CONDITION_COUNT * 2, LazyCountingVector(false));
		for (const auto& c : conditions) {
			single_actions[2 * c].allocateTables();
			single_actions[2 * c + 1].allocateTables();
		}
	}

	void deallocateTables() {
#if HDT_GLOBAL_EQUIVALENT_ACTIONS_COUNTING_PASS == true
		all_single_equivalent_actions.deallocateTables();
#endif
		all_single_actions.deallocateTables();
		single_actions.clear();
	}

	RecursionInstance(std::vector<int> conditions,
		ullong set_conditions0,
		ullong set_conditions1,
		BinaryDrag<conact>::node* parent)
	{
		initialize(conditions, set_conditions0, set_conditions1, parent);
	}

	RecursionInstance(std::istringstream& iss) {
		// source: "RecursionInstance 1,2,3,4,5,6,7,8,9, 2048 0"
		std::vector<int> conditions;
		ullong set_conditions0;
		ullong set_conditions1;

		int counter = 1;
		do
		{
			std::string subs;
			iss >> subs;
			if (counter == 1) {
				// parse conditions
				std::istringstream iss(subs);
				std::string delimiter = ",";
				size_t last = 0;
				size_t next = 0;
				while ((next = subs.find(delimiter, last)) != std::string::npos) {
					conditions.push_back(std::stoi(subs.substr(last, next - last)));
					last = next + 1;
				}
				//conditions.push_back(std::stoi(subs.substr(last)));
			}
			else if (counter == 2) {
				set_conditions0 = std::stoull(subs);
			}
			else if (counter == 3) {
				set_conditions1 = std::stoull(subs);
			}
			counter++;
		} while (iss);
		initialize(conditions, set_conditions0, set_conditions1, nullptr);
	}

	void setParent(BinaryDrag<conact>::node* p) {
		parent = p;
	}

	const size_t tableSizeInMemory() const {
		size_t result = 0;
		result += all_single_actions.sizeInMemory();
		for (const auto& s : single_actions) {
			result += s.sizeInMemory();
		}
		return result;
	}

	std::string to_string() const {
		std::stringstream ss;
		ss << "RecursionInstance ";
		for (const auto& c : conditions) {
			ss << c << ",";
		}
		ss << " " << std::to_string(set_conditions0) << " " << std::to_string(set_conditions1);
		return ss.str();
	}

	ullong nextRuleCode;
	ullong ruleCodeBitMask = 0;

	void forwardToRuleCode(const ullong& rule_code, const rule_set& rs) {
		ruleCodeBitMask = 0;
		int i = 0;
		for (const auto& c : conditions) {
			ruleCodeBitMask |= ((rule_code >> c) & 1ULL) << i;
			i++;
		}
		findNextRuleCode();
	}

	std::vector<RuleCodeShift> rule_code_shifts;

	void prepareRuleCodeShifts() {
		RuleCodeShift* current_shift = nullptr;
		int non_mask_conditions = 0;
		for (int i = 0; i < CONDITION_COUNT; i++) {
			if (std::find(conditions.begin(), conditions.end(), i) != conditions.end()) { // condition, add to bitmask				
				if (current_shift == nullptr) { // new mask
					rule_code_shifts.push_back(RuleCodeShift(1ULL << (i - non_mask_conditions), non_mask_conditions));
					current_shift = &rule_code_shifts[rule_code_shifts.size() - 1];
				} else {
					current_shift->mask |= 1ULL << (i - non_mask_conditions);
				}
			} else {
				current_shift = nullptr;
				non_mask_conditions++;
			}
		}
	}

	bool findNextRuleCode() {
		if (ruleCodeBitMask >= (1ULL << conditions.size())) {
			return false;
		}
		nextRuleCode = set_conditions1;
		for (const auto& s : rule_code_shifts) {
			nextRuleCode |= ((ruleCodeBitMask & s.mask) << s.shift);
		}
		ruleCodeBitMask++;
		return true;
	}

	const std::string printTables() const {
		std::ostringstream oss;
		oss << "RecursionInstance Tables\nAll Single Actions (Size in memory: " << all_single_actions.sizeInMemory() << ")\n";
		all_single_actions.print(oss);
		int i = 0;
		for (const auto& s : single_actions) {
			if (s.sizeInMemory() == 0) {
				i++;
				continue;
			}
			oss << "Single Actions, Condition " << (i / 2) << ", Bit: " << i % 2 << " (Size in memory: " << std::to_string(s.sizeInMemory()) << ")\n";
			s.print(oss);
			i++;
		}
		return oss.str();
	}
};

struct LeafInfo {
	BinaryDrag<conact>::node* ptr;
	LazyCountingVector counts;
	ullong set_conditions0 = 0;
	ullong set_conditions1 = 0;
	std::vector<int> conditions;

	LeafInfo(BinaryDrag<conact>::node* ptr, ullong& set_conditions0, ullong& set_conditions1, std::vector<int>& conditions) : ptr(ptr), set_conditions0(set_conditions0), set_conditions1(set_conditions1), conditions(conditions)
	{
		prepareRuleCodeShifts();
	}

	ullong nextRuleCode;
	ullong ruleCodeBitMask = 0;

	void forwardToRuleCode(const ullong& rule_code, const rule_set& rs) {
		ruleCodeBitMask = 0;
		int i = 0;
		for (const auto& c : conditions) {
			ruleCodeBitMask |= ((rule_code >> c) & 1ULL) << i;
			i++;
		}
		findNextRuleCode();
	}

	std::vector<RuleCodeShift> rule_code_shifts;

	void prepareRuleCodeShifts() {
		RuleCodeShift* current_shift = nullptr;
		int non_mask_conditions = 0;
		for (int i = 0; i < CONDITION_COUNT; i++) {
			if (std::find(conditions.begin(), conditions.end(), i) != conditions.end()) { // condition, add to bitmask				
				if (current_shift == nullptr) { // new mask
					rule_code_shifts.push_back(RuleCodeShift(1ULL << (i - non_mask_conditions), non_mask_conditions));
					current_shift = &rule_code_shifts[rule_code_shifts.size() - 1];
				}
				else {
					current_shift->mask |= 1ULL << (i - non_mask_conditions);
				}
			}
			else {
				current_shift = nullptr;
				non_mask_conditions++;
			}
		}
	}

	bool findNextRuleCode() {
		if (ruleCodeBitMask >= (1ULL << conditions.size())) {
			return false;
		}
		nextRuleCode = set_conditions1;
		for (const auto& s : rule_code_shifts) {
			nextRuleCode |= ((ruleCodeBitMask & s.mask) << s.shift);
		}
		ruleCodeBitMask++;
		return true;
	}
};

struct RecursionInstanceGroup {
	size_t begin_index;
	size_t end_index;

	const size_t size() const {
		return end_index - begin_index;
	}

	RecursionInstanceGroup(size_t begin_index, size_t end_index) : begin_index(begin_index), end_index(end_index) {}
};

struct PartitionState {
	bool processed = true;
	int partition_id = -1;
};

struct ProgressMetaData {
	int start_depth = 0;
	int start_leaves = 0;
	int start_path_length_sum = 0;
	ullong start_rule_accesses = 0;

	ProgressMetaData(int start_depth = 0, int start_leaves = 0, int start_path_length_sum = 0, ullong start_rule_accesses = 0) : start_depth(start_depth), start_leaves(start_leaves), start_path_length_sum(start_path_length_sum), start_rule_accesses(start_rule_accesses) {}
};

struct Log {
#if HDT_PROCESS_NODE_LOGGING_ENABLED == true
	std::ostream& os_;
	Log(std::ofstream& os) : os_(os) { }
#else
	Log() {}
#endif

	const Log& operator<<(const std::string s) const {
#if HDT_PROCESS_NODE_LOGGING_ENABLED == true
		os_ << s;
#endif
		return *this;
	}
};

#pragma endregion

#pragma region Progress Management

void NodeToStringRec(BinaryDrag<conact>::node* n, std::stringstream& ss) {
	if (n == nullptr || (n->data.action.size() == 0 && n->data.condition.size() == 0)) {
		ss << "*";
	}
	else if (n->data.t == conact::type::ACTION) {
		ss << "[";
		for (const auto& a : n->data.action.getSingleActions()) {
			ss << a << ",";
		}
		ss << "]";
	}
	else if (n->data.t == conact::type::CONDITION) {
		ss << n->data.condition << "(";
		NodeToStringRec(n->left, ss);
		ss << ".";
		NodeToStringRec(n->right, ss);
		ss << ")";
	}
}

std::string TreeToString(BinaryDrag<conact>& tree) {
	std::stringstream ss;
	ss << "Tree ";
	NodeToStringRec(tree.roots_[0], ss);
	return ss.str();
}

int found_wildcards = 0;

void StringToTreeRec(
	std::string s,
	BinaryDrag<conact>& tree,
	BinaryDrag<conact>::node* node,
	std::vector<RecursionInstance>& r_insts,
	int& next_recursion_index) {
	auto bracket_pos = s.find('(');

	if (bracket_pos == 1 || bracket_pos == 2) { // support 1 or 2 character pixel/voxel names
		std::string condition = s.substr(0, bracket_pos);
		node->data.t = conact::type::CONDITION;
		node->data.condition = condition;

		auto left = tree.make_node();
		auto right = tree.make_node();

		node->left = left;
		node->right = right;

		std::string remaining = s.substr(bracket_pos + 1, s.size() - (2 + bracket_pos));
		size_t separator_pos, open = 0, closed = 0;
		for (size_t i = 0; i < remaining.size(); i++) {
			if (remaining[i] == '(') {
				open++;
			}
			else if (remaining[i] == ')') {
				closed++;
			}
			else if (remaining[i] == '.' && open == closed) {
				separator_pos = i;
				break;
			}
		}
		auto lsubstr = remaining.substr(0, separator_pos);
		auto rsubstr = remaining.substr(separator_pos + 1, remaining.size() - separator_pos);
		StringToTreeRec(lsubstr, tree, left, r_insts, next_recursion_index);
		StringToTreeRec(rsubstr, tree, right, r_insts, next_recursion_index);
	}
	else {
		if (s[0] == '*' && s.size() == 1) {
			// wildcard for pending recursion instances
			found_wildcards++;
			r_insts[next_recursion_index++].setParent(node);
		}
		else if (s[0] == '[' && s[s.size() - 1] == ']') {
			// no brackets and no wild card = action node
			std::istringstream iss(s.substr(1, s.size() - 2));
			std::string str;
			node->data.t = conact::type::ACTION;
			action_bitset action;
			while (std::getline(iss, str, ',')) {
				action.set(stoi(str));
			}
			node->data.action = action;
		}
	}
}

std::string GetLatestDepthProgressFilePath() {
	return (conf.progress_file_path_ / std::filesystem::path("progress-latest.txt")).string();
}

std::string GetFinishedDepthProgressFilePath() {
	return (conf.progress_file_path_ / std::filesystem::path("progress-finished.txt")).string();
}


std::string GetBenchmarkDepthProgressFilePath() {
	return (conf.progress_file_path_ / std::filesystem::path("progress-BBDT3D-benchmark-depth" + std::to_string(HDT_BENCHMARK_READAPPLY_DEPTH) + ".txt")).string();
}

void SaveDepthProgressToFile(std::vector<RecursionInstance>& r_insts, BinaryDrag<conact>& tree, int depth, int nodes, int path_length_sum, uint64_t rule_accesses) {
	std::stringstream ss;
	ss << "progress-" << conf.algorithm_name_ << "-";
	char mbstr[100];
	std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	if (std::strftime(mbstr, sizeof(mbstr), "%Y-%m-%d_%H-%M-%S", std::localtime(&time))) {
		ss << mbstr;
	}
	ss << "-depth" << depth;
	ss << ".txt";
	std::string filename = ss.str();
	std::string path = (conf.progress_file_path_ / std::filesystem::path(filename)).string();

	if (!std::filesystem::exists(conf.progress_file_path_)) {
		std::filesystem::create_directory(conf.progress_file_path_);
	}

	std::stringstream oss;
	bool retry = false;
	try {
		std::ofstream os(path, std::ios::binary);
		if (!os) {
			std::cout << "Could not save progress to file: " << path << std::endl;
			return;
		}
		for (const auto& r : r_insts) {
			auto r_string = r.to_string();
			oss << r_string << "\n";
			os << r_string << "\n";
		}
		auto tree_string = TreeToString(tree);
		oss << tree_string << "\n";
		os << tree_string << "\n";
		os << "Meta(Depth/Nodes/Pathlengthsum/Ruleaccesses) " << depth << " " << nodes << " " << path_length_sum << " " << rule_accesses << "\n";
		os.close();
	}
	catch (const std::exception &e) {
		std::cout << "exception: " << e.what() << std::endl;
		std::cout << "The following output has been generated before the program crashed:" << std::endl;
		std::cout << oss.str() << std::endl;
		std::cout << "--- end of output" << std::endl;
		retry = true;
	}
	catch (...) {
		std::cout << "Miscellenanous exception" << std::endl;
		std::cout << "The following output has been generated before the program crashed:" << std::endl;
		std::cout << oss.str() << std::endl;
		std::cout << "--- end of output" << std::endl;
		retry = true;
	}

	if (retry) {
		try {
			std::cout << "Since there was an exception, I will now try to generate the complete output again and output it to stdout." << std::endl;
			for (const auto& r : r_insts) {
				auto r_string = r.to_string();
				std::cout << r_string << "\n";
			}
			auto tree_string = TreeToString(tree);
			std::cout << tree_string << "\n";
			std::cout << "Meta(Depth/Nodes/Pathlengthsum/Ruleaccesses) " << depth << " " << nodes << " " << path_length_sum << " " << rule_accesses << "\n";
		}
		catch (const std::exception &e) {
			std::cout << "exception: " << e.what() << std::endl;
		}
		catch (...) {
			std::cout << "Miscellenanous exception" << std::endl;
		}
	}

	std::cout << "Saved depth progress to file: " << path << std::endl;
	std::string latest_path = GetLatestDepthProgressFilePath();
	try {
		if (std::filesystem::exists(latest_path)) {
			std::filesystem::remove(latest_path);
		}
		std::filesystem::copy(path, latest_path);
	}
	catch (std::filesystem::filesystem_error e) {
		std::cerr << "Could not save progress to 'latest file': " << latest_path << std::endl;
	}
}

std::string GetTableProgressPath(std::string table_hash) {
	std::string path = "table-" + table_hash + ".txt";
	return conf.GetCountingTablesFilePath(path);
}

void SaveTableProgressToFile(std::vector<RecursionInstance>& recursion_instances, int depth, const ullong& next_rule_code) {
	int counter = 0;
#pragma omp parallel for
	for (int i = 0; i < recursion_instances.size(); i++) {
		const auto& r = recursion_instances[i];
		if (r.loaded_from_file) {
			continue;
		}
		std::string path = GetTableProgressPath(r.to_string());
		std::ofstream os(path);
		os << r.printTables();
		os.close();
#pragma omp atomic
		counter++;
	}
	std::cout << "Saved " << counter << " tables to file. " << std::endl;
}

void LoadTableProgressFromFile(std::vector<RecursionInstance>& recursion_instances) {
	int not_found_tables = 0, loaded_tables = 0;
	for (auto& r : recursion_instances) {
		std::string file_path = GetTableProgressPath(r.to_string());
		if (!std::filesystem::exists(file_path)) {
			not_found_tables++;
			continue;
		}
		std::ifstream is(file_path);
		std::string line;
		int current_condition = -1;
		int current_bit = -1;
		bool all_actions_table = false;
		while (std::getline(is, line))
		{
			if (line.size() == 0) {
				continue;
			}

			std::istringstream iss(line);
			std::string keyword;
			iss >> keyword;

			if (line.compare("RecursionInstance Tables") == 0) {
				continue;
			}
			if (keyword.compare("All") == 0) { // "All Single Actions (...)"
				all_actions_table = true;
				continue;
			}
			if (keyword.compare("Single") == 0) { // "Single Actions (...)"
				all_actions_table = false;
				iss >> keyword; // "Actions,"
				iss >> keyword; // "Condition"
				iss >> current_condition;
				iss >> keyword; // ","
				iss >> keyword; // "Bit:"
				iss >> current_bit;
				continue;
			}

			int index = std::stoi(keyword);
			ActionCounter count;
			iss >> count;
			if (all_actions_table) {
				r.all_single_actions[index] = count;
			}
			else {
				r.single_actions[current_condition * 2 + current_bit][index] = count;
			}
		}
		is.close();
		loaded_tables++;
		r.counted_partitions = PARTITIONS;
		r.loaded_from_file = true;
	}
	std::cout << "Loaded " << loaded_tables << " tables from files, " << not_found_tables << " not found." << std::endl;
}


uint64_t GetFileLineCount(std::string path) {
	std::ifstream file(path);
	file.unsetf(std::ios_base::skipws);
	uint64_t line_count = std::count(std::istream_iterator<char>(file), std::istream_iterator<char>(), '\n');
	return line_count;
}

ProgressMetaData GetInitialProgress(std::vector<RecursionInstance>& recursion_instances, BinaryDrag<conact>::node* root, rule_set& rs, BaseRuleSet& brs, BinaryDrag<conact>& tree) {
#if HDT_BENCHMARK_READAPPLY_ENABLED == true
	std::string path = GetBenchmarkDepthProgressFilePath();
	if (!std::filesystem::exists(path)) {
		std::cerr << "Benchmark progress file not found at " << path << std::endl;
		throw std::runtime_error("Benchmark progress file not found at " + path);
	}
	bool load = true;
#if HDT_BENCHMARK_READAPPLY_PAUSE_FOR_MEMORY_MEASUREMENTS == true
	std::cout << "Pre-load memory - press enter to continue" << std::endl;
	getchar();
#endif
#else
#if HDT_DEPTH_PROGRESS_ENABLED == true
#if HDT_USE_FINISHED_TREE_ONLY == true
	std::string path = GetFinishedDepthProgressFilePath();
#else
	std::string path = GetLatestDepthProgressFilePath();
#endif
	bool load = std::filesystem::exists(path);
#else
	bool load = false;
#endif
#endif
	if (load) {
		// continue existing run
		std::cout << "Depth progress file found, loading progress: " << std::flush;

		recursion_instances.reserve(GetFileLineCount(path));

		std::ifstream is(path, std::ios::binary);
		std::string line;
		int depth, leaves, path_length_sum;
		uint64_t rule_accesses;
		bool tree_found = false, meta_found = false;
		while (std::getline(is, line))
		{
			std::istringstream iss(line);
			std::string keyword;
			iss >> keyword;

			if (keyword.compare("RecursionInstance") == 0) {
				recursion_instances.push_back(RecursionInstance(iss));
			}
			else if (keyword.compare("Tree") == 0) {
				std::string s;
				iss >> s;
				int val = 0;
				StringToTreeRec(s, tree, root, recursion_instances, val);
				tree_found = true;
			}
			else if (keyword.compare("Meta(Depth/Nodes/Pathlengthsum/Ruleaccesses)") == 0) {
				iss >> depth;
				iss >> leaves;
				iss >> path_length_sum;
				iss >> rule_accesses;
				meta_found = true;
			}
		}
		is.close();

		// checks
#if HDT_USE_FINISHED_TREE_ONLY == false
		if (recursion_instances.size() == 0) {
			std::cerr << "No RecursionInstances found in loaded progress file. Aborting." << std::endl;
			exit(EXIT_FAILURE);
		}
#endif
		if (!meta_found) {
			std::cerr << "No Meta section found in loaded progress file. Aborting." << std::endl;
			exit(EXIT_FAILURE);
		}
		if (!tree_found) {
			std::cerr << "No Tree found in loaded progress file. Aborting." << std::endl;
			exit(EXIT_FAILURE);
		}
		if (found_wildcards != recursion_instances.size()) {
			std::cout << "Found " << found_wildcards << " wildcards ('*' nodes), but have " << recursion_instances.size() << " RecursionInstances." << std::endl;
			exit(EXIT_FAILURE);
		}
		for (const auto& r : recursion_instances) {
			if (r.conditions.size() != CONDITION_COUNT - (std::bitset<64>(r.set_conditions0).count() + std::bitset<64>(r.set_conditions1).count())) {
				std::cerr << "RecursionInstance with wrongly sized condition array found. Aborting." << std::endl;
				exit(EXIT_FAILURE);
			}
			if (r.parent == nullptr) {
				std::cerr << "RecursionInstance with NULL parent found after loading progress file. Aborting." << std::endl;
				exit(EXIT_FAILURE);
			}
		}
		std::cout << "Depth: " << depth << ", RecursionInstances: " << recursion_instances.size() << ", Leaves: " << leaves << std::endl;
		return ProgressMetaData(depth, leaves, path_length_sum, rule_accesses);
	}
	else {
		// start new run
#if HDT_USE_FINISHED_TREE_ONLY == true
		std::cout << "No finished tree found. Will abort because of the HDT_USE_FINISHED_TREE_ONLY parameter being true." << std::endl;
		getchar();
		exit(EXIT_FAILURE);
#endif
#if HDT_DEPTH_PROGRESS_ENABLED
		std::cout << "No progress file found, starting new run from depth 0." << std::endl;
#endif
		std::vector<int> conditions;
		conditions.reserve(rs.conditions.size());
		for (auto &c : rs.conditions) {
			conditions.push_back(rs.conditions_pos.at(c));
		}
		auto r = RecursionInstance(conditions, 0, 0, root);
		recursion_instances.push_back(r);
		return ProgressMetaData();
	}
}
#pragma endregion


#pragma region Read and Classify Rules
void FindBestSingleActionCombinationRunningCombined(
#if HDT_GLOBAL_EQUIVALENT_ACTIONS_COUNTING_PASS == true
	const LazyCountingVector& all_single_equivalent_actions,
#endif
	LazyCountingVector& all_single_actions,
	std::vector<LazyCountingVector>& single_actions,
	const std::vector<int>& conditions,
	const action_bitset& combined_action,
	const ullong& rule_code) {

	const auto& actions = combined_action.getSingleActions();

	Action most_popular_single_action_index = actions[0];
#if HDT_GLOBAL_EQUIVALENT_ACTIONS_COUNTING_PASS == true
	ActionCounter most_popular_single_action_occurences = all_single_equivalent_actions[actions[0]];

	if (actions.size() > 1) {
		for (const auto& a : actions) { // TODO: optimize by skipping first element
			if (all_single_equivalent_actions[a] > most_popular_single_action_occurences) {
				most_popular_single_action_index = a;
				most_popular_single_action_occurences = all_single_equivalent_actions[a];
			}
		}
	}
#else
	ActionCounter most_popular_single_action_occurences = all_single_actions[actions[0]];

	if (actions.size() > 1) {
		for (const auto& a : actions) { // TODO: optimize by skipping first element
			if (all_single_actions[a] > most_popular_single_action_occurences) {
				most_popular_single_action_index = a;
				most_popular_single_action_occurences = all_single_actions[a];
			}
		}
	}
#endif

	all_single_actions[most_popular_single_action_index]++;

	for (const auto& c : conditions) {
		single_actions[2 * c + ((rule_code >> c) & 1)][most_popular_single_action_index]++;
	}
}

void FindBestSingleActionCombinationRunningCombinedPtr(
	LazyCountingVector& all_single_actions,
	std::vector<LazyCountingVector>& single_actions,
	const std::vector<int>& conditions,
	const action_bitset* combined_action,
	const ullong& rule_code) {

	int most_popular_single_action_index = -1;
	ActionCounter most_popular_single_action_occurences = -1;

	for (const auto& a : combined_action->getSingleActions()) {
		if (all_single_actions[a] > most_popular_single_action_occurences) {
			most_popular_single_action_index = a;
			most_popular_single_action_occurences = all_single_actions[a];
		}
	}
	all_single_actions[most_popular_single_action_index]++;

	for (const auto& c : conditions) {
		single_actions[2 * c + ((rule_code >> c) & 1)][most_popular_single_action_index]++;
	}
}

#pragma endregion

#pragma region Parallel Partition Processing

int threads_reading = HDT_PARALLEL_PARTITIONBASED_NUMTHREADS_PARTITION_READING_INITIAL;
int threads_processing = HDT_PARALLEL_PARTITIONBASED_NUMTHREADS_PROCESSING_INITIAL;

enum PartitionProcessingTaskType {
	ReadAndApply,
	RegenerateEquivalentActions,
	EquivalentCountingPass
};

struct PartitionProcessingTaskData {
	std::vector<LeafInfo>* leaves;

	std::vector<RecursionInstance>* r_insts;
	const RecursionInstanceGroup* rig;

	PartitionProcessingTaskData(std::vector<RecursionInstance>* r_insts, const RecursionInstanceGroup* rig) : r_insts(r_insts), rig(rig) {};
	PartitionProcessingTaskData(std::vector<LeafInfo>* leaves) : leaves(leaves) {};
};

template<PartitionProcessingTaskType task>
void ParallelPartitionProcessing(BaseRuleSet& brs, rule_set& rs, PartitionProcessingTaskData data) {
	auto start = std::chrono::system_clock::now();

#if HDT_BENCHMARK_READAPPLY_ENABLED == true
	int indicated_progress = -1;
	bool benchmark_started = false;
	std::chrono::system_clock::time_point benchmark_start_point;
	std::cout << "warm up for benchmark - " << std::flush;

	constexpr llong benchmark_sample_point = TOTAL_RULES / 2;

	constexpr llong begin_rule_code = benchmark_sample_point - (1ULL << 21);
	constexpr llong benchmark_start_rule_code = benchmark_sample_point;
	constexpr llong end_rule_code = benchmark_sample_point + (1ULL << 27);

	constexpr int begin_partition = begin_rule_code / RULES_PER_PARTITION;
	constexpr int benchmark_start_partition = benchmark_start_rule_code / RULES_PER_PARTITION;
	constexpr int end_partition = std::max(static_cast<int>(end_rule_code / RULES_PER_PARTITION), benchmark_start_partition + 1);

#if HDT_BENCHMARK_READAPPLY_CORRECTNESS_TEST_ENABLED == true
#if HDT_PARALLEL_PARTITIONBASED_ENABLED == true
	std::string correctness_file_path = "benchmark-p" + std::to_string(PARTITIONS) + "-a" + std::to_string(ACTION_COUNT) + "-depth" + std::to_string(HDT_BENCHMARK_READAPPLY_DEPTH) + "_P" + std::to_string(begin_partition) + "-" + std::to_string(end_partition) + ".txt";
#else
	std::string correctness_file_path = "benchmark-p" + std::to_string(PARTITIONS) + "-a" + std::to_string(ACTION_COUNT) + "-depth" + std::to_string(HDT_BENCHMARK_READAPPLY_DEPTH) + "_R" + std::to_string(begin_rule_code) + "-" + std::to_string(end_rule_code) + ".txt";
#endif
#if HDT_BENCHMARK_READAPPLY_CORRECTNESS_TEST_GENERATE_FILE == false
	auto benchmark_correctness_file_path = conf.GetCountingTablesFilePath(correctness_file_path);
	if (!std::filesystem::exists(benchmark_correctness_file_path)) {
		std::cout << "\n\nBenchmark correctness file does not exist: " << benchmark_correctness_file_path << std::endl;
		getchar();
		exit(EXIT_FAILURE);
	}
#endif
#endif

#else
	constexpr llong begin_rule_code = 0;
	constexpr llong end_rule_code = TOTAL_RULES;

	constexpr int begin_partition = 0;
	constexpr int end_partition = PARTITIONS;
#endif

#if HDT_BENCHMARK_READAPPLY_PAUSE_FOR_MEMORY_MEASUREMENTS == true
	std::cout << "Pre-partition memory - press enter to continue" << std::endl;
	getchar();
#endif

	std::vector<std::vector<action_bitset>> action_data(HDT_PARALLEL_PARTITIONBASED_PRELOADED_PARTITIONS, std::vector<action_bitset>(RULES_PER_PARTITION));
	std::vector<PartitionState> action_data_state(HDT_PARALLEL_PARTITIONBASED_PRELOADED_PARTITIONS, PartitionState());

	for (size_t i = 0; i < action_data_state.size(); i++) {
		action_data_state[i].partition_id = static_cast<int>(i);
	}

	bool clamped_processing_thread_count = false;

	if (data.rig != nullptr && data.rig->size() < threads_processing) {
		threads_processing = static_cast<int>(data.rig->size());
		threads_reading = HDT_PARALLEL_PARTITIONBASED_NUMTHREADS_TOTAL - threads_processing;
		clamped_processing_thread_count = true;
	}
	std::cout << "Thread Distribution: [Reading: " << threads_reading << "] [Processing: " << threads_processing << (clamped_processing_thread_count ? " (clamped to recursion instance size)" : "") << "]" << std::endl;

	int partition_reader_idle = 0;
	int partition_processor_idle = 0;

#if HDT_BENCHMARK_READAPPLY_ENABLED == true
	int partition_reader_idle_benchmark = 0;
	int partition_processor_idle_benchmark = 0;
#endif
#pragma omp parallel num_threads(2)
	{
		// Task: Read Partitions
		if (omp_get_thread_num() == 0)
		{
#pragma omp parallel num_threads(threads_reading)
			{
#pragma omp for schedule(dynamic, 1)
				for (int next_loaded_partition = begin_partition; next_loaded_partition < end_partition; next_loaded_partition++) {
					//std::ofstream os(GetTableProgressPath("Read" + std::to_string(next_loaded_partition) + ".txt"));
					int index = next_loaded_partition % HDT_PARALLEL_PARTITIONBASED_PRELOADED_PARTITIONS;
					//os << "Thread Number: " << omp_get_thread_num() << " PartitionIndex: " << index << std::endl;
					while (action_data_state[index].processed == false || action_data_state[index].partition_id != next_loaded_partition) {
						//os << "partition reader idle\n";
#if HDT_BENCHMARK_READAPPLY_ENABLED == true
						if (benchmark_started) {
#pragma omp atomic
							partition_reader_idle_benchmark++;
						}
						else
#endif
						{
#pragma omp atomic
							partition_reader_idle++;
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(HDT_PARALLEL_PARTITIONBASED_IDLE_MS));
					}
					//os << "start\n";
					brs.LoadPartition(next_loaded_partition, action_data[index]);
					action_data_state[index].processed = false;
					rule_accesses += RULES_PER_PARTITION;
					//os << "end\n";
					//os.close();
				}
			}
		}

		// Task: Process Partitions
		if (omp_get_thread_num() == 1)
		{
#pragma omp parallel num_threads(threads_processing)
			for (int p = begin_partition; p < end_partition; p++) {
				int index = p % HDT_PARALLEL_PARTITIONBASED_PRELOADED_PARTITIONS;
#pragma omp single
				{
					//std::ofstream os(GetTableProgressPath("Process" + std::to_string(p) + ".txt"));
					//os << "(Waiting) Thread Number: " << omp_get_thread_num() << " PartitionIndex: " << index << std::endl;

					while (action_data_state[index].processed == true || action_data_state[index].partition_id != p) {
						//os << " processing idle (action data state: processed: " << action_data_state[index].processed << " partition id: " << action_data_state[index].partition_id << ")\n";
#if HDT_BENCHMARK_READAPPLY_ENABLED == true
						if (benchmark_started) {
#pragma omp atomic
							partition_processor_idle_benchmark++;
						}
						else
#endif
						{
#pragma omp atomic
							partition_processor_idle += threads_processing;
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(HDT_PARALLEL_PARTITIONBASED_IDLE_MS));
					}
					//os << "start processing\n";
					//os.close();

#if HDT_BENCHMARK_READAPPLY_ENABLED == true
					if (!benchmark_started && p >= benchmark_start_partition) {
						benchmark_started = true;
						benchmark_start_point = std::chrono::system_clock::now();
						std::cout << "start - ";
					}
#else
					if (p % (PARTITIONS / HDT_GENERATION_LOG_FREQUENCY) == 0) {
						auto end = std::chrono::system_clock::now();
						std::chrono::duration<double> elapsed_seconds = end - start;
						std::time_t end_time = std::chrono::system_clock::to_time_t(end);
						double progress = p * 1.f / PARTITIONS;
						double projected_mins = ((elapsed_seconds.count() / progress) - elapsed_seconds.count()) / 60;
						char mbstr[100];
						if (p == 0) {
							std::cout << std::endl;
						}
						if (std::strftime(mbstr, sizeof(mbstr), "%Y-%m-%d %H:%M:%S", std::localtime(&end_time))) {
							std::cout << "[" << mbstr << "] Partition " << p << " of " << PARTITIONS << " (" << progress * 100 << "%, ca. " << projected_mins << " minutes remaining)." << std::endl;
						}
						else {
							std::cout << "[" << std::ctime(&end_time) << "] Rule " << p << " of " << PARTITIONS << " (" << progress * 100 << "%, ca. " << projected_mins << " minutes remaining)." << std::endl;
						}
					}
#endif
				}
				const ullong first_rule_code = p * RULES_PER_PARTITION;
				const ullong last_rule_code = (p + 1) * RULES_PER_PARTITION;
				std::vector<action_bitset>& actions = action_data[index];
#if HDT_READAPPLY_VERBOSE_TIMINGS == true
				TLOG4_DEF;
				if (omp_get_thread_num() == 0) {
					TLOG4_START("Processing");
				}
#endif

				if (task == PartitionProcessingTaskType::ReadAndApply) {
					const int begin_r_inst = static_cast<int>(data.rig->begin_index);
					const int end_r_inst = static_cast<int>(data.rig->end_index);
					auto& r_insts = *data.r_insts;
#pragma omp for schedule(dynamic, 1)
					for (int i = begin_r_inst; i < end_r_inst; i++) {
						auto& r = r_insts[i];
						if (r.counted_partitions == PARTITIONS) {
							continue;
						}
						size_t n;
						if (r.nextRuleCode < first_rule_code || r.nextRuleCode >= last_rule_code) {
							r.forwardToRuleCode(first_rule_code, rs);
						}
						while (true) {
							n = static_cast<size_t>(r.nextRuleCode - first_rule_code);
							if (n >= RULES_PER_PARTITION) { // delegate this rule to next partition
								break;
							}
#if HDT_GLOBAL_EQUIVALENT_ACTIONS_COUNTING_PASS == true
							FindBestSingleActionCombinationRunningCombined(r.all_single_equivalent_actions, r.all_single_actions, r.single_actions, r.conditions, (actions)[n], first_rule_code + n);
#else
							FindBestSingleActionCombinationRunningCombined(r.all_single_actions, r.single_actions, r.conditions, (actions)[n], first_rule_code + n);
#endif
							if (!r.findNextRuleCode()) {
								r.counted_partitions = PARTITIONS;
								break;
							}
						}
						r.counted_partitions++;
					}
				}
				else if (task == PartitionProcessingTaskType::RegenerateEquivalentActions) {
					auto& leaves = *data.leaves;
					#pragma omp for schedule(dynamic, 1)
					for (int i = 0; i < leaves.size(); i++) {
						auto& leaf = leaves[i];
						size_t n;
						if (leaf.nextRuleCode < first_rule_code || leaf.nextRuleCode >= last_rule_code) {
							leaf.forwardToRuleCode(first_rule_code, rs);
						}
						while (true) {
							n = static_cast<size_t>(leaf.nextRuleCode - first_rule_code);
							if (n >= RULES_PER_PARTITION) { // delegate this rule to next partition
								break;
							}
							for (const auto& a : actions[n].getSingleActions()) {
								leaf.counts[a]++;
							}
							if (!leaf.findNextRuleCode()) {
								break;
							}
						}
					}
				}
				else if (task == PartitionProcessingTaskType::EquivalentCountingPass) {
					const int begin_r_inst = static_cast<int>(data.rig->begin_index);
					const int end_r_inst = static_cast<int>(data.rig->end_index);
					auto& r_insts = *data.r_insts;

					#pragma omp critical 
					{
						for (int i = begin_r_inst; i < end_r_inst; i++) {
							auto& r = r_insts[i];

							size_t n;
							if (r.nextRuleCode < first_rule_code || r.nextRuleCode >= last_rule_code) {
								r.forwardToRuleCode(first_rule_code, rs);
							}
							while (true) {
								n = static_cast<size_t>(r.nextRuleCode - first_rule_code);
								if (n >= RULES_PER_PARTITION) { // delegate this rule to next partition
									break;
								}
								for (auto& b : actions[n].getSingleActions()) {
									r.all_single_equivalent_actions[b]++;
								}
								if (!r.findNextRuleCode()) {
									break;
								}
							}
						}
					}
					
				}
				else {
					std::cerr << "Task type" + std::to_string(task) + " not yet implemented." << std::endl;;
					throw std::runtime_error("Task type" + std::to_string(task) + " not yet implemented.");
				}

#pragma omp single
				{
					//std::ofstream os(GetTableProgressPath("Process" + std::to_string(p) + ".txt"), std::ios::app);
					//os << "finished\n";
					//os.close();
					action_data_state[index].processed = true;
					action_data_state[index].partition_id = p + HDT_PARALLEL_PARTITIONBASED_PRELOADED_PARTITIONS;

					//std::cout << ("processed p" + std::to_string(p) + "\n");
				}
#if HDT_READAPPLY_VERBOSE_TIMINGS == true
				if (omp_get_thread_num() == 0) {
					TLOG4_STOP;
				}
#endif
			}
		}
	}
#if HDT_BENCHMARK_READAPPLY_ENABLED == false
	std::cout << "\n\nPartition Reader Idle: " << (partition_reader_idle * HDT_PARALLEL_PARTITIONBASED_IDLE_MS / 1000.) << " seconds. Partition Processor Idle: " << (partition_processor_idle * HDT_PARALLEL_PARTITIONBASED_IDLE_MS / 1000.) << " seconds." << std::endl;
	
	if (clamped_processing_thread_count) {
		threads_reading = HDT_PARALLEL_PARTITIONBASED_NUMTHREADS_PARTITION_READING_INITIAL;
		threads_processing = HDT_PARALLEL_PARTITIONBASED_NUMTHREADS_PROCESSING_INITIAL;
	} else {
		if ((partition_reader_idle / 100.) > partition_processor_idle && threads_reading > 1) {
			threads_processing++;
			threads_reading--;
			std::cout << "In next level, one thread re-assigned to processing." << std::endl;
		}
		if ((partition_processor_idle / 100.) > partition_reader_idle && threads_processing > 1) {
			threads_processing--;
			threads_reading++;
			std::cout << "In next level, one thread re-assigned to reading." << std::endl;
		}	
	}
#endif
#if HDT_BENCHMARK_READAPPLY_PAUSE_FOR_MEMORY_MEASUREMENTS == true
	std::cout << "Pre-finish memory - press enter to continue" << std::endl;
	getchar();
#endif


#if HDT_BENCHMARK_READAPPLY_ENABLED == true
#if HDT_BENCHMARK_READAPPLY_PAUSE_FOR_MEMORY_MEASUREMENTS == true
	std::cout << "Before finish memory - press enter to continue" << std::endl;
	getchar();
#endif
	std::cout << "\n\n[Before Benchmark Period] Partition Reader Idle: " << (partition_reader_idle * HDT_PARALLEL_PARTITIONBASED_IDLE_MS / 1000.) << " seconds. Partition Processor Idle: " << (partition_processor_idle * HDT_PARALLEL_PARTITIONBASED_IDLE_MS / 1000.) << " seconds." << std::endl;
	std::cout << "[During Benchmark Period] Partition Reader Idle: " << (partition_reader_idle_benchmark * HDT_PARALLEL_PARTITIONBASED_IDLE_MS / 1000.) << " seconds. Partition Processor Idle: " << (partition_processor_idle_benchmark * HDT_PARALLEL_PARTITIONBASED_IDLE_MS / 1000.) << " seconds." << std::endl;

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - benchmark_start_point;

#if HDT_PARALLEL_PARTITIONBASED_ENABLED
	ullong processed_rules = (end_partition - benchmark_start_partition) * RULES_PER_PARTITION;
	double progress = processed_rules * 1. / TOTAL_RULES;
#else
	ullong processed_rules = (end_rule_code - benchmark_start_rule_code);
	double progress = (end_rule_code - benchmark_start_rule_code) * 1. / TOTAL_RULES;
#endif

	double projected_mins = ((elapsed_seconds.count() / progress) - elapsed_seconds.count()) / 60;
	std::cout << "\n\n*******************************\nBenchmark Result:\n" << elapsed_seconds.count() << " seconds for " << processed_rules << " of " << TOTAL_RULES << " rules\n" << progress * 100 << "%, ca. " << projected_mins << " minutes for total benchmarked depth." << std::endl;

#if HDT_PARALLEL_PARTITIONBASED_ENABLED == true
	std::cout << "begin_partition : " << begin_partition << " benchmark_start_partition : " << benchmark_start_partition << " end_partition : " << end_partition << std::endl;
#else
	std::cout << "begin_rule_code : " << begin_rule_code << " benchmark_start_rule_code : " << benchmark_start_rule_code << " end_rule_code : " << end_rule_code << std::endl;
#endif

#if HDT_BENCHMARK_READAPPLY_CORRECTNESS_TEST_GENERATE_FILE == true
	auto path = conf.GetCountingTablesFilePath(correctness_file_path);
	std::filesystem::create_directories(conf.GetCountingTablesFilePath(""));
	std::ofstream os(path);
	for (const auto& r : r_insts) {
		os << r.printTables();
	}
	os.close();
	std::cout << "\nWrote correctness benchmark file to " << path << std::endl;
#elif HDT_BENCHMARK_READAPPLY_CORRECTNESS_TEST_ENABLED == true
	std::cout << "\nChecking correctness of counting tables..." << std::endl;

	std::ifstream correct_stream(benchmark_correctness_file_path);
	auto test_stream = stringstream();
	for (const auto& r : r_insts) {
		test_stream << r.printTables();
	}

	std::string correct_line, test_line;
	int i = 0;
	bool correct = true;
	while (correct_stream.good()) {
		std::getline(test_stream, test_line);
		std::getline(correct_stream, correct_line);
		if (test_line.compare(correct_line) != 0) {
			correct = false;
			std::cout << "Correctness test failed (Line " << i << ").\nLine generated from benchmark:\n" << test_line << "\nCorrect line:\n" << correct_line << std::endl;
			break;
		}
		i++;
	}
	if (correct) {
		std::cout << "Counting tables are correct." << std::endl;
	}
#endif

	getchar();
	exit(EXIT_SUCCESS);
#endif
}
#pragma endregion

#pragma region Process Nodes

ushort GetFirstCountedAction(const LazyCountingVector& b) {
	for (size_t i = 0; i < b.size(); i++) {
		if (b[i] > 0) {
			return static_cast<ushort>(i);
		}
	}
	std::cerr << "GetFirstCountedAction called with empty vector" << std::endl;
	throw std::runtime_error("GetFirstCountedAction called with empty vector");
}

double entropy(const LazyCountingVector& vector) {
	double s = 0, h = 0;
	for (size_t i = 0; i < vector.size(); i++) {
		const ActionCounter x = vector[i];
		if (x == 0) {
			continue;
		}
		s += x;
		h += x * log2(x);
	}
	if (s == 0) {
		std::cerr << "VERY BAD: Entropy function passed 0-array, therefore creating NaN value!" << std::endl;
	}
	return log2(s) - h / s;
}

int HdtProcessNode(
	RecursionInstance& r,
	BinaryDrag<conact>& tree,
	const rule_set& rs,
	std::vector<RecursionInstance>& upcoming_recursion_instances,
	const Log& log) {
	int amount_of_action_children = 0;
	log << "HdtProcessNode start with RecInst: " << r.to_string() << "\n";

	std::vector<int> uselessConditions;

	// Case 2: Take best guess (highest p/total occurences), both children are conditions/nodes 
	int splitCandidate = r.conditions[0];
	double maximum_information_gain = 0;

	double baseEntropy = entropy(r.all_single_actions);
	log << "Base Entropy: " << std::to_string(baseEntropy) << "\n";

	if (baseEntropy == 0.) {
		log << "Base Entropy is zero, therefore this node is a leaf.\n";
		r.parent->data.t = conact::type::ACTION;
		r.parent->data.action = action_bitset().set(GetFirstCountedAction(r.all_single_actions));

		amount_of_action_children++;
		r.processed = true;

		log << "Processed instance. (Action Children = 1; due to Zero Entropy case) \n";
		log << r.printTables();

		return amount_of_action_children;
	}

	bool leftIsAction = false;
	bool rightIsAction = false;

	for (auto& c : r.conditions) {
		double leftEntropy = entropy(r.single_actions[c * 2]);
		double rightEntropy = entropy(r.single_actions[c * 2 + 1]);

#if HDT_INFORMATION_GAIN_METHOD_VERSION == 1
		// 1) Simple Information Gain
		double informationGain = std::max((baseEntropy - leftEntropy), (baseEntropy - rightEntropy));
#elif HDT_INFORMATION_GAIN_METHOD_VERSION == 2
		// 2) Information Gain Sum
		double informationGain = (baseEntropy - leftEntropy) + (baseEntropy - rightEntropy);
#elif HDT_INFORMATION_GAIN_METHOD_VERSION == 3
		// 3) Weighted Information Gain Sum
		double LigTimesRig = (baseEntropy - leftEntropy) + (baseEntropy - rightEntropy);
		double difference = (std::max(leftEntropy, rightEntropy) - std::min(leftEntropy, rightEntropy)) + 0.001;
		double informationGain = LigTimesRig / std::sqrt(difference);
#elif HDT_INFORMATION_GAIN_METHOD_VERSION == 4
		// 4) Information Gain Sum; Punish Negative
		double leftGain = (baseEntropy - leftEntropy);
		double rightGain = (baseEntropy - rightEntropy);
		if (leftGain < 0) {
			leftGain *= 0;
		}
		if (rightGain < 0) {
			rightGain *= 0;
		}
		double informationGain = rightGain + leftGain;
#endif

		if (std::abs(baseEntropy - leftEntropy) < 0.00001 && std::abs(baseEntropy - rightEntropy) < 0.00001) {
			uselessConditions.push_back(c);
		}

		if (informationGain > maximum_information_gain) {
			maximum_information_gain = informationGain;
			splitCandidate = c;
			leftIsAction = (leftEntropy == 0);
			rightIsAction = (rightEntropy == 0);
		}
		log << "Condition: " << rs.conditions[c] << " Information gain: " << std::to_string(informationGain) << "\tEntropy Left (0): " << std::to_string(leftEntropy) << "\tEntropy Right (1): " << std::to_string(rightEntropy) << "\n";
	}
	log << "------\nSplit candidate chosen: " << rs.conditions[splitCandidate] << "\n";

	//log << "Deleting " << std::to_string(uselessConditions.size()) << " useless conditions: ";
	//for (const auto& s : uselessConditions) {
	//	log << rs.conditions[s] << " ";
	//	r.set_conditions0 |= (1ULL << s);
	//	r.conditions.erase(std::remove(r.conditions.begin(), r.conditions.end(), s), r.conditions.end());
	//}
	//log << "\n";

	r.conditions.erase(std::remove(r.conditions.begin(), r.conditions.end(), splitCandidate), r.conditions.end());

	r.parent->data.t = conact::type::CONDITION;
	r.parent->data.condition = rs.conditions[splitCandidate];

	if (leftIsAction) {
		r.parent->left = tree.make_node();
		r.parent->left->data.t = conact::type::ACTION;
		r.parent->left->data.action = action_bitset().set(GetFirstCountedAction(r.single_actions[splitCandidate * 2]));
		amount_of_action_children++;
	}
	else {
		r.parent->left = tree.make_node();
		auto newConditions0 = r.set_conditions0 | (1ULL << splitCandidate);
		upcoming_recursion_instances.push_back(RecursionInstance(r.conditions, newConditions0, r.set_conditions1, r.parent->left));
	}

	if (rightIsAction) {
		r.parent->right = tree.make_node();
		r.parent->right->data.t = conact::type::ACTION;
		r.parent->right->data.action = action_bitset().set(GetFirstCountedAction(r.single_actions[splitCandidate * 2 + 1]));
		amount_of_action_children++;
	}
	else {
		r.parent->right = tree.make_node();
		auto newConditions1 = r.set_conditions1 | (1ULL << splitCandidate);
		upcoming_recursion_instances.push_back(RecursionInstance(r.conditions, r.set_conditions0, newConditions1, r.parent->right));
	}

	r.processed = true;

	log << "Processed instance. Split Candidate: " << rs.conditions[splitCandidate] << " Action Children: " << std::to_string(amount_of_action_children) << "\n";
	log << r.printTables();

	return amount_of_action_children;
}
#pragma endregion

#pragma region Interface/High-Level Functions

void HdtReadAndApplyRulesOnePass(BaseRuleSet& brs, rule_set& rs, std::vector<RecursionInstance>& r_insts, const RecursionInstanceGroup& rig) {
#if HDT_PARALLEL_PARTITIONBASED_ENABLED == true
	ParallelPartitionProcessing<ReadAndApply>(brs, rs, PartitionProcessingTaskData(&r_insts, &rig));
#else
	for (llong rule_code = begin_rule_code; rule_code < end_rule_code; rule_code++) {
#if HDT_BENCHMARK_READAPPLY == false
		if (rule_code % (TOTAL_RULES / 64) == 0) { // TODO: optimize this?
			auto end = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = end - start;
			std::time_t end_time = std::chrono::system_clock::to_time_t(end);
			double progress = rule_code * 1.f / TOTAL_RULES;
			double projected_mins = ((elapsed_seconds.count() / progress) - elapsed_seconds.count()) / 60;
			char mbstr[100];
			if (rule_code == 0) {
				std::cout << std::endl;
			}
			if (std::strftime(mbstr, sizeof(mbstr), "%Y-%m-%d %H:%M:%S", std::localtime(&end_time))) {
				std::cout << "[" << mbstr << "] Rule " << rule_code << " of " << TOTAL_RULES << " (" << progress * 100 << "%, ca. " << projected_mins << " minutes remaining)." << std::endl;
			}
			else {
				std::cout << "[" << std::ctime(&end_time) << "] Rule " << rule_code << " of " << TOTAL_RULES << " (" << progress * 100 << "%, ca. " << projected_mins << " minutes remaining)." << std::endl;
			}
		}
#endif
#if HDT_BENCHMARK_READAPPLY == true
		if (!benchmark_started && rule_code > benchmark_start_rule_code) {
			benchmark_started = true;
			benchmark_start_point = std::chrono::system_clock::now();
			std::cout << "start - " << std::flush;
		}
#endif
		first_match = true;

#if HDT_PARALLEL_INNERLOOP_ENABLED == true
#pragma omp parallel for num_threads(HDT_PARALLEL_INNERLOOP_NUMTHREADS)
		for (int i = 0; i < r_insts.size(); i++) {
			auto& r = r_insts[i];
#else
		for (auto& r : r_insts) {
#endif
			if (((rule_code & r.set_conditions0) == 0ULL) && ((rule_code & r.set_conditions1) == r.set_conditions1)) {
				if (first_match) {
#if (HDT_ACTION_SOURCE == 0)
					action = &zero_action;									// 0) Zero action
#elif (HDT_ACTION_SOURCE == 1)
					action = &rs.rules[rule_code].actions;					// 1) load from rule table
#elif (HDT_ACTION_SOURCE == 2)
					action = &brs.GetActionFromRuleIndex(rs, rule_code);	// 2) generate during run-time
#elif (HDT_ACTION_SOURCE == 3)
					action = brs.LoadRuleFromBinaryRuleFiles(rule_code);	// 3) read from file
#endif
					rule_accesses++;
					first_match = false;
				}
#if HDT_COMBINED_CLASSIFIER == true
				FindBestSingleActionCombinationRunningCombinedPtr(
					r.all_single_actions,
					r.single_actions,
					r.conditions,
					action,
					rule_code);
#else
				FindBestSingleActionCombinationRunning(r.all_single_actions, action);

				for (auto& c : r.conditions) {
					int bit_value = (rule_code >> c) & 1;

					int return_code = FindBestSingleActionCombinationRunning(r.single_actions[c][bit_value], action, r.most_probable_action_occurences_[c][bit_value]);

					if (return_code >= 0) {
						{
							r.most_probable_action_index_[c][bit_value] = return_code;
							r.most_probable_action_occurences_[c][bit_value] = r.single_actions[c][bit_value][return_code];
						}
					}
				}
#endif
			}
		}
		}
#endif
	}

#if HDT_GLOBAL_EQUIVALENT_ACTIONS_COUNTING_PASS == true
void HdtCountEquivalentActions(BaseRuleSet& brs, rule_set& rs, std::vector<RecursionInstance>& r_insts, const RecursionInstanceGroup& rig) {

	ParallelPartitionProcessing<EquivalentCountingPass>(brs, rs, PartitionProcessingTaskData(&r_insts, &rig));

	// Reset RecursionInstances so their state is zero
	for (auto& r : r_insts) {
		r.forwardToRuleCode(0, rs);
	}
}
#endif

void FindHdtIteratively(rule_set& rs,
	BaseRuleSet& brs,
	BinaryDrag<conact>& tree)
{
#if HDT_PROCESS_NODE_LOGGING_ENABLED == true
	std::filesystem::create_directories(conf.GetProcessNodeLogFilePath(""));
#endif
	std::vector<RecursionInstance> recursion_instance_data1;
	std::vector<RecursionInstance> recursion_instance_data2;

	std::vector<RecursionInstance>* pending_recursion_instances = &recursion_instance_data1;
	std::vector<RecursionInstance>* upcoming_recursion_instances = &recursion_instance_data2;

	std::vector<RecursionInstanceGroup> recursion_instance_groups;

	ProgressMetaData pmd = GetInitialProgress(*pending_recursion_instances, tree.GetRoot(), rs, brs, tree);

	omp_set_nested(1);

	int depth = pmd.start_depth;
	int leaves = pmd.start_leaves;
	int path_length_sum = pmd.start_path_length_sum;
	rule_accesses = pmd.start_rule_accesses;

	bool detected_counting_data_format_is_ullong = (sizeof(ullong) == sizeof(ActionCounter));
	if (depth < (CONDITION_COUNT - 31) && !detected_counting_data_format_is_ullong) {
		std::cout << "\n####### [ActionCounter Data Format] Critical depth and NOT selected ullong - aborting. #######\n" << std::endl;
		getchar();
		exit(EXIT_FAILURE);
	}
	else if (depth >= (CONDITION_COUNT - 31) && detected_counting_data_format_is_ullong) {
		std::cout << "\n- [ActionCounter Data Format] Not in critical depth but still selected ullong - recommended to restart program with int datatype.\n" << std::endl;
	}
	else {
		std::cout << "\n+ [ActionCounter Data Format] Recommended data type (" << (detected_counting_data_format_is_ullong ? "ullong" : "int") << ") detected.\n" << std::endl;
	}

	while (pending_recursion_instances->size() > 0) {
		std::cout << "Processing next batch of recursion instances (depth: " << depth << ", count: " << pending_recursion_instances->size() << ")" << std::endl;

		{
			int groups = std::max(static_cast<int>(std::ceil(pending_recursion_instances->size() * 1. / HDT_RECURSION_INSTANCE_GROUP_SIZE)), 1);
			int previous_end = 0;
			for (int i = 0; i < groups; i++) {
				size_t group_size = (i == groups - 1) ? (pending_recursion_instances->size() - previous_end) : HDT_RECURSION_INSTANCE_GROUP_SIZE;
				size_t start_index = i * HDT_RECURSION_INSTANCE_GROUP_SIZE;
				size_t end_index = start_index + group_size;
				recursion_instance_groups.push_back(RecursionInstanceGroup(start_index, end_index));
				previous_end = (i + 1) * HDT_RECURSION_INSTANCE_GROUP_SIZE;
			}
		}

#if HDT_PROCESS_NODE_LOGGING_ENABLED == true
		auto process_node_log_os = std::ofstream(conf.GetProcessNodeLogFilePath("d" + std::to_string(depth) + "-all_recinsts.txt"));
#endif
		int recursion_instance_counter = 0;

		for (const auto& rig : recursion_instance_groups) {
			std::cout << "Begin processing of RecursionInstanceGroup from RecInst " << rig.begin_index << " to RecInst " << (rig.end_index - 1) << std::endl;
			TLOG("Allocating tables",
				for (size_t i = rig.begin_index; i < rig.end_index; i++) {
					(*pending_recursion_instances)[i].allocateTables();
				}
			);

#if HDT_TABLE_PROGRESS_ENABLED == true
			// look for counting table files and load them
			LoadTableProgressFromFile(*pending_recursion_instances);
#endif

#if HDT_GLOBAL_EQUIVALENT_ACTIONS_COUNTING_PASS == true
			TLOG6("Counting equivalent actions",
				HdtCountEquivalentActions(brs, rs, *pending_recursion_instances, rig);
			);
#endif

			TLOG2("Reading rules and classifying",
				HdtReadAndApplyRulesOnePass(brs, rs, *pending_recursion_instances, rig);
			);

#if HDT_TABLE_PROGRESS_ENABLED == true
			SaveTableProgressToFile(*pending_recursion_instances, depth, TOTAL_RULES);
#endif


			std::cout << "debug marker A" << std::endl;

			TLOG3_START("Processing instances");
			{
				for (size_t i = rig.begin_index; i < rig.end_index; i++) {
#if HDT_PROCESS_NODE_LOGGING_ENABLED == true
					process_node_log_os << "*********************************\n# RecursionInstance " << recursion_instance_counter++ << std::endl;
					int amount_of_action_children = HdtProcessNode((*pending_recursion_instances)[i], tree, rs, *upcoming_recursion_instances, Log(process_node_log_os));
#else
					int amount_of_action_children = HdtProcessNode((*pending_recursion_instances)[i], tree, rs, *upcoming_recursion_instances, Log());
#endif
					leaves += amount_of_action_children;
					path_length_sum += (depth + 1) * amount_of_action_children;
				}
			}
			TLOG3_STOP;

			TLOG5("Dellocating tables",
				for (size_t i = rig.begin_index; i < rig.end_index; i++) {
					(*pending_recursion_instances)[i].deallocateTables();
				}
			);
		}
#if HDT_PROCESS_NODE_LOGGING_ENABLED == true
		process_node_log_os.close();
#endif
		depth++;


#if HDT_DEPTH_PROGRESS_ENABLED == true
		SaveDepthProgressToFile(*upcoming_recursion_instances, tree, depth, leaves, path_length_sum, rule_accesses);
#endif
		std::cout << "debug marker B" << std::endl;

		pending_recursion_instances->clear();
		recursion_instance_groups.clear();

		// swap pointers
		std::swap(upcoming_recursion_instances, pending_recursion_instances);

		std::cout << "debug marker C" << std::endl;
		//getchar();
	}
	float average_path_length = path_length_sum / static_cast<float>(leaves);
	std::cout << "HDT construction done. Nodes: " << leaves << " Average path length: " << average_path_length << std::endl;
#if HDT_DEPTH_PROGRESS_ENABLED == true
	try {
		if (std::filesystem::exists(GetFinishedDepthProgressFilePath())) {
			std::filesystem::remove(GetFinishedDepthProgressFilePath());
		}
		std::filesystem::rename(GetLatestDepthProgressFilePath(), GetFinishedDepthProgressFilePath());
	}
	catch (std::filesystem::filesystem_error e) {
		std::cerr << "Could not delete 'latest depth progress file'." << std::endl;
	}
#endif
}

void AddLeavesToVectorRec(BinaryDrag<conact>::node* node, ullong set_conditions0, ullong set_conditions1, rule_set& rs, std::vector<LeafInfo>& leaves) {
	if (node->isleaf()) {
		std::vector<int> conditions;
		ullong set_conditions = set_conditions0 | set_conditions1;
		for (int i = 0; i < CONDITION_COUNT; i++) {
			if (((set_conditions >> i) & 1) == 0) {
				conditions.push_back(i);
			}
		}
		leaves.push_back(LeafInfo(node, set_conditions0, set_conditions1, conditions));
	}
	else {
		ullong added_bit = (1ULL << rs.conditions_pos[node->data.condition]);
		AddLeavesToVectorRec(node->left, set_conditions0 | added_bit, set_conditions1, rs, leaves);
		AddLeavesToVectorRec(node->right, set_conditions0, set_conditions1 | added_bit, rs, leaves);
	}
}

void RegenerateEquivalentActionsInLeaves(rule_set& rs, BaseRuleSet& brs, BinaryDrag<conact>& tree) {
	std::vector<LeafInfo> leaves;
	leaves.reserve(1683079);
	AddLeavesToVectorRec(tree.GetRoot(), 0, 0, rs, leaves);

	ParallelPartitionProcessing<RegenerateEquivalentActions>(brs, rs, PartitionProcessingTaskData(&leaves));

#pragma omp parallel for
	for (int i = 0; i < leaves.size(); i++) {
		//std::cout << "**** Leaf: " << i << std::endl;
		auto& leaf = leaves[i];
		action_bitset& action = leaf.ptr->data.action;
		const LazyCountingVector counts = const_cast<const LazyCountingVector&>(leaf.counts); // force read-only access on counting vector
		auto previous_single_action = action.getSingleActions()[0];
		ActionCounter expected_count = 1 << leaf.conditions.size();

		for (int a = 0; a < ACTION_COUNT; a++) {
			if (counts[a] == expected_count) {
				/*if (previous_single_action == a) {
					std::cout << "sanity check success, the already set action in this leaf has the expected count" << std::endl;
				}*/
				if (previous_single_action != a) {
					//std::cout << "New equivalent action discovered!" << std::endl;
					action.set(a);
				}
			}
		}
		//leaf.counts.print(std::cout);
	}
}

BinaryDrag<conact> GenerateHdt(rule_set& rs, BaseRuleSet& brs) {
	BinaryDrag<conact> tree;
	auto parent = tree.make_root();

	std::bitset<CONDITION_COUNT> set_conditions0, set_conditions1;

	bool b1 = set_conditions0.size() == rs.conditions.size();
	bool b2 = ACTION_COUNT == rs.actions.size();

	if (!(b1 && b2)) {
		std::cerr << "Assert failed: check ACTION_COUNT and CONDITION_COUNT." << std::endl;
		throw std::runtime_error("Assert failed: check ACTION_COUNT and CONDITION_COUNT.");
	}

	bool b3 = (HDT_INFORMATION_GAIN_METHOD_VERSION >= 1 && HDT_INFORMATION_GAIN_METHOD_VERSION <= 4);
	bool b4 = (HDT_ACTION_SOURCE >= 0 && HDT_ACTION_SOURCE <= 3);

	if (!(b3 && b4)) {
		std::cerr << "Assert failed: check HDT_ACTION_SOURCE and HDT_INFORMATION_GAIN_METHOD_VERSION." << std::endl;
		throw std::runtime_error("Assert failed: check HDT_ACTION_SOURCE and HDT_INFORMATION_GAIN_METHOD_VERSION.");
	}

	bool b5 = RULES_PER_PARTITION < INT_MAX;

	if (!(b5)) {
		std::cerr << "Assert failed: RULES_PER_PARTITION is not smaller than INT_MAX. This is assumed in the algorithm to increase performance." << std::endl;
		throw std::runtime_error("Assert failed: RULES_PER_PARTITION is not smaller than INT_MAX.");
	}

#if HDT_BENCHMARK_READAPPLY_ENABLED == true
	std::cout << "\n\n***************************************************" << std::endl;
	std::cout << "***************** Benchmark Mode ******************" << std::endl;
	std::cout << "***************************************************" << std::endl;
#if HDT_BENCHMARK_READAPPLY_CORRECTNESS_TEST_GENERATE_FILE == true
	std::cout << "\nGenerating correctness test file." << std::endl;
#elif HDT_BENCHMARK_READAPPLY_CORRECTNESS_TEST_ENABLED == true 
	std::cout << "\nExecuting correctness test." << std::endl;
#endif
#endif
	std::cout << "\nInformation gain method version: [" << HDT_INFORMATION_GAIN_METHOD_VERSION << "]" << std::endl;
	std::cout << "Combined classifier enabled: [" << (HDT_COMBINED_CLASSIFIER ? "Yes" : "No") << "]" << std::endl;
	std::cout << "Action source: [" << HDT_ACTION_SOURCE_STRINGS[HDT_ACTION_SOURCE] << "]" << std::endl;


#if HDT_PARALLEL_PARTITIONBASED_ENABLED == false
	brs.OpenRuleFiles();
#endif
	//brs.VerifyRuleFiles();
#if HDT_USE_FINISHED_TREE_ONLY == true
	if (std::filesystem::exists(GetFinishedDepthProgressFilePath())) {
		std::vector<RecursionInstance> dummy;
		std::cout << "Loading finished depth progress from file..." << std::endl;
		GetInitialProgress(dummy, tree.GetRoot(), const_cast<rule_set&>(rs), brs, tree);
		std::cout << "Successfully retrieved tree from finished progress file." << std::endl;
		return tree;
	}
	else {
		std::cout << "Finish depth progress file does not exist. Aborting due to HDT_USE_FINISHED_TREE_ONLY parameter set to true." << std::endl;
		getchar();
		exit(EXIT_FAILURE);
	}
#endif
	FindHdtIteratively(rs, brs, tree);


	std::cout << "Total rule accesses: " << rule_accesses << "\n";
	std::cout << "Information gain method version: [" << HDT_INFORMATION_GAIN_METHOD_VERSION << "]" << std::endl;
	std::cout << "Combined classifier enabled: [" << (HDT_COMBINED_CLASSIFIER ? "Yes" : "No") << "]" << std::endl;
	std::cout << "Action source: [" << HDT_ACTION_SOURCE_STRINGS[HDT_ACTION_SOURCE] << "]" << std::endl;
#ifndef NDEBUG
	std::cout << "Build: [Debug]" << std::endl;
#else
	std::cout << "Build: [Release]" << std::endl;
#endif

	return tree;
}

BinaryDrag<conact> GenerateHdt(rule_set& rs, BaseRuleSet& brs, const std::string& filename)
{
	TLOG("Generating HDT",
		auto t = GenerateHdt(rs, brs);
	);

	//TLOG2("Regenerate equivalent actions in leaves",
	//	RegenerateEquivalentActionsInLeaves(rs, brs, t);
	//);

	WriteConactTree(t, filename);
	return t;
}

BinaryDrag<conact> GetHdt(const rule_set& rs, const BaseRuleSet& brs, bool force_generation) {
	std::string hdt_filename = conf.hdt_path_.string();
	BinaryDrag<conact> t;
	if (conf.force_odt_generation_ || force_generation || !LoadConactTree(t, hdt_filename)) {
		t = GenerateHdt(const_cast<rule_set&>(rs), const_cast<BaseRuleSet&>(brs), hdt_filename);
	}
	return t;
}
#pragma endregion
