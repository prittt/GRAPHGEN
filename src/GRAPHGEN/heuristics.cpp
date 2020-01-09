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

#include <random>
#include <ctime>
#include <algorithm>

#include "heuristics.h"
#include "utilities.h"
#include "drag_statistics.h"

#include "constants.h"

constexpr std::array<const char*, 4> HDT_ACTION_SOURCE_STRINGS = { "**** !! ZERO ACTION = GARBAGE DATA !! ****", "Memory (pre-generated or read from rule file)", "Generation during run-time", "Binary rule files" };

#define HDT_INFORMATION_GAIN_METHOD_VERSION 2
#define HDT_COMBINED_CLASSIFIER true
#define HDT_ACTION_SOURCE 3
#define HDT_PROGRESS_ENABLED true
#define HDT_BENCHMARK_READAPPLY false

#define HDT_PARALLEL_INNERLOOP_ENABLED false
#define HDT_PARALLEL_INNERLOOP_NUMTHREADS 2

#define HDT_PARALLEL_PARTITIONBASED true

// How many log outputs will be done for each pass, i.e. the higher this number, the more and regular output there will be
constexpr int HDT_GENERATION_LOG_FREQUENCY = 32;


using namespace std;

uint64_t rule_accesses = 0;
uint64_t necessary_rule_accesses = 0;

struct RecursionInstance {
	// parameters
	std::vector<int> conditions;
	ullong set_conditions0; // assume: condition count < 64
	ullong set_conditions1;
	BinaryDrag<conact>::node* parent;

	// local vars
	std::vector<std::vector<int>> single_actions;
	std::vector<int> all_single_actions; // TODO: Optimize this (also for single_actions), since not all actions are ever used -> lots of unused space and allocations
#if HDT_COMBINED_CLASSIFIER == false
	std::vector<std::array<int, 2>> most_probable_action_index_;
	std::vector<std::array<int, 2>> most_probable_action_occurences_;
#endif
	// state
	bool processed = false;

	void initialize(std::vector<int> c,
		ullong sc0,
		ullong sc1,
		BinaryDrag<conact>::node* p) {
		conditions = c;
		set_conditions0 = sc0;
		set_conditions1 = sc1;
		parent = p;
		single_actions.resize(CONDITION_COUNT * 2, std::vector<int>(ACTION_COUNT, 0));
		all_single_actions.resize(ACTION_COUNT, 0);
#if HDT_COMBINED_CLASSIFIER == false
		most_probable_action_index_.resize(CONDITION_COUNT, std::array<int, 2>());
		most_probable_action_occurences_.resize(CONDITION_COUNT, std::array<int, 2>());
#endif
	}

	RecursionInstance(std::vector<int> conditions,
		ullong set_conditions0,
		ullong set_conditions1,
		BinaryDrag<conact>::node* parent)
	{
		initialize(conditions, set_conditions0, set_conditions1, parent);
	}

	RecursionInstance(istringstream& iss) {
		// source: "RecursionInstance 1,2,3,4,5,6,7,8,9, 2048 0"
		std::vector<int> conditions;
		ullong set_conditions0;
		ullong set_conditions1;

		int counter = 1;
		do
		{
			string subs;
			iss >> subs;
			if (counter == 1) {
				// parse conditions
				istringstream iss(subs);
				std::string delimiter = ",";
				size_t last = 0;
				size_t next = 0;
				while ((next = subs.find(delimiter, last)) != string::npos) {
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

	std::string to_string() const {
		stringstream ss;
		ss << "RecursionInstance ";
		for (const auto& c : conditions) {
			ss << c << ",";
		}
		ss << " " << std::to_string(set_conditions0) << " " << std::to_string(set_conditions1);
		return ss.str();
	}
};

double entropy(std::vector<int>& vector) {
	double s = 0, h = 0;
	for (const auto& x : vector) {
		if (x == 0) {
			continue;
		}
		s += x;
		h += x * log2(x);
	}
	return log2(s) - h / s;
}

int FindBestSingleActionCombinationRunning(std::vector<int>& single_actions, action_bitset& combined_action, int previous_most_probable_action_occurences = -1) {
	int most_popular_single_action_occurences = -1;
	int most_popular_single_action_index = -1;

	for (ushort i = 0; i < combined_action.size(); i++) {
		auto s = combined_action.getActionByDataIndex(i);
		if (single_actions[s] > most_popular_single_action_occurences) {
			most_popular_single_action_index = s;
			most_popular_single_action_occurences = single_actions[s];
		}
	}

	single_actions[most_popular_single_action_index]++;
	
	if (previous_most_probable_action_occurences < 0) {
		return 0;
	}

	if (single_actions[most_popular_single_action_index] > previous_most_probable_action_occurences) {
		return most_popular_single_action_index;
	}
	return -1;
}

void FindBestSingleActionCombinationRunningCombined(
	std::vector<int>& all_single_actions,
	std::vector<std::vector<int>>& single_actions,
	const action_bitset& combined_action,
	const ullong& rule_code) {

	int most_popular_single_action_occurences = -1;
	int most_popular_single_action_index = -1;

	for (const auto& a : combined_action.getSingleActions()) {
		if (all_single_actions[a] > most_popular_single_action_occurences) {
			most_popular_single_action_index = a;
			most_popular_single_action_occurences = all_single_actions[a];
		}
	}
	all_single_actions[most_popular_single_action_index]++;

	for (int i = 0; i < CONDITION_COUNT; i++) {
		single_actions[2 * i + ((rule_code >> i) & 1)][most_popular_single_action_index]++;
	}
}


void FindBestSingleActionCombinationRunningCombinedPtr(
	std::vector<int>& all_single_actions,
	std::vector<std::vector<int>>& single_actions,
	const action_bitset* combined_action,
	const ullong& rule_code) {

	int most_popular_single_action_occurences = -1;
	int most_popular_single_action_index = -1;

	for (const auto& a : combined_action->getSingleActions()) {
		if (all_single_actions[a] > most_popular_single_action_occurences) {
			most_popular_single_action_index = a;
			most_popular_single_action_occurences = all_single_actions[a];
		}
	}
	all_single_actions[most_popular_single_action_index]++;

	for (int i = 0; i < CONDITION_COUNT; i++) {
		single_actions[2 * i + ((rule_code >> i) & 1)][most_popular_single_action_index]++;
	}
}

void HdtReadAndApplyRulesOnePass(BaseRuleSet& brs, rule_set& rs, std::vector<RecursionInstance>& r_insts) {
	bool first_match;
#if (HDT_ACTION_SOURCE == 0)
	action_bitset zero_action = action_bitset(1).set(0);
#endif
	action_bitset* action;
	auto start = std::chrono::system_clock::now();

#if HDT_BENCHMARK_READAPPLY == true
	int indicated_progress = -1;
	bool benchmark_started = false;
	std::chrono::system_clock::time_point benchmark_start_point;
	std::cout << "warm up for benchmark - ";
	
	constexpr llong benchmark_sample_point = TOTAL_RULES / 2;

	constexpr llong begin_rule_code = benchmark_sample_point - (1ULL << 20);
	constexpr llong benchmark_start_rule_code = benchmark_sample_point;
	constexpr llong end_rule_code = benchmark_sample_point + (1ULL << 25);

	constexpr int begin_partition = begin_rule_code / RULES_PER_PARTITION;
	constexpr int benchmark_start_partition = benchmark_start_rule_code / RULES_PER_PARTITION;
	constexpr int end_partition = std::max(end_rule_code / RULES_PER_PARTITION, benchmark_start_partition + 1ULL);
#else
	constexpr llong begin_rule_code = 0;
	constexpr llong end_rule_code = TOTAL_RULES;

	constexpr int begin_partition = 0;
	constexpr int end_partition = PARTITIONS;
#endif

#if HDT_PARALLEL_PARTITIONBASED == true 
	std::vector<action_bitset> seen_actions(RULES_PER_PARTITION);

	for (int p = begin_partition; p < end_partition; p++) {
		#if HDT_BENCHMARK_READAPPLY == true
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
		brs.LoadPartition(p, seen_actions);
		rule_accesses += RULES_PER_PARTITION;
		const ullong first_rule_code = p * RULES_PER_PARTITION;
		#pragma omp parallel for num_threads(8)
		for (int i = 0; i < r_insts.size(); i++) {
			auto& r = r_insts[i];
			for (int n = 0; n < RULES_PER_PARTITION; n++) {
				if ((((first_rule_code + n) & r.set_conditions0) == 0ULL) && (((first_rule_code + n) & r.set_conditions1) == r.set_conditions1)) {
					FindBestSingleActionCombinationRunningCombined(r.all_single_actions, r.single_actions, seen_actions[n], first_rule_code + n);
				}
			}
		}
	}
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
				std::cout << "[" << mbstr << "] Rule " << rule_code << " of " << TOTAL_RULES << " (" <<  progress * 100 << "%, ca. " << projected_mins << " minutes remaining)." << std::endl;
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
			std::cout << "start - ";
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

#if HDT_BENCHMARK_READAPPLY == true
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - benchmark_start_point;

	#if HDT_PARALLEL_PARTITIONBASED
		ullong processed_rules = (end_partition - benchmark_start_partition) * RULES_PER_PARTITION;
		double progress = processed_rules * 1. / TOTAL_RULES;
	#else
		ullong processed_rules = (end_rule_code - benchmark_start_rule_code);
		double progress = (end_rule_code - benchmark_start_rule_code) * 1. / TOTAL_RULES;
	#endif

	double projected_mins = ((elapsed_seconds.count() / progress) - elapsed_seconds.count()) / 60;
	double corrected_projected_mins = projected_mins / 1.15f;
	std::cout << "\n\n*******************************\nBenchmark Result:\n" << elapsed_seconds.count() << " seconds for " << processed_rules << " of " << TOTAL_RULES << " rules\n" << progress * 100 << "%, ca. " << projected_mins << " minutes for total benchmarked depth." << std::endl;
	
	#if HDT_PARALLEL_PARTITIONBASED
		std::cout << "begin_partition : " << begin_partition << " benchmark_start_partition : " << benchmark_start_partition << " end_partition : " << end_partition;
	#else
		std::cout << "begin_rule_code : " << begin_rule_code << " benchmark_start_rule_code : " << benchmark_start_rule_code << " end_rule_code : " << end_rule_code;
	#endif
	
	getchar();
	exit(EXIT_SUCCESS);
#endif
}

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
	stringstream ss;
	ss << "Tree ";
	NodeToStringRec(tree.roots_[0], ss);
	return ss.str();
}

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

		std::string remaining = s.substr(bracket_pos + 1, s.size() - (2+bracket_pos));
		int separator_pos, open = 0, closed = 0;
		for (int i = 0; i < remaining.size(); i++) {
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
			r_insts[next_recursion_index++].setParent(node);
		}
		else if (s[0] == '[' && s[s.size() - 1] == ']') {
			// no brackets and no wild card = action node
			istringstream iss(s.substr(1, s.size() - 2));
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

std::string GetLatestProgressFilePath() {
	return (conf.progress_file_path_ / std::filesystem::path("progress-latest.txt")).string();
}

std::string GetBenchmarkProgressFilePath() {
	return (conf.progress_file_path_ / std::filesystem::path("progress-BBDT3D-benchmark-depth12.txt")).string();
}

void SaveProgressToFiles(std::vector<RecursionInstance>& r_insts, BinaryDrag<conact>& tree, int depth, int nodes, int path_length_sum, uint64_t rule_accesses) {
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
		std::ofstream os(path, ios::binary);
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

	std::cout << "Saved progress to file: " << path << std::endl;
	std::string latest_path = GetLatestProgressFilePath();
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

uint GetFirstCountedAction(std::vector<int>& b) {
	for (size_t i = 0; i < b.size(); i++) {
		if (b[i] > 0) {
			return i;
		}
	}
	std::cerr << "GetFirstCountedAction called with empty vector" << std::endl;
	throw std::runtime_error("GetFirstCountedAction called with empty vector");
}

int HdtProcessNode(RecursionInstance& r, BinaryDrag<conact>& tree, const rule_set& rs, std::vector<RecursionInstance>& upcoming_recursion_instances) {
	int amount_of_action_children = 0;
	
	// Case 3: Both childs are leafs, end of recursion
	if (r.conditions.size() == 1) {
		r.parent->data.t = conact::type::CONDITION;
		r.parent->data.condition = rs.conditions[r.conditions[0]];
		auto leftNode = tree.make_node();
		auto rightNode = tree.make_node();
		r.parent->left = leftNode;
		r.parent->right = rightNode;
		auto leftAction = action_bitset().set(GetFirstCountedAction(r.single_actions[r.conditions[0] * 2]));
		leftNode->data.t = conact::type::ACTION;
		leftNode->data.action = leftAction;
		auto rightAction = action_bitset().set(GetFirstCountedAction(r.single_actions[r.conditions[0] * 2]));
		rightNode->data.t = conact::type::ACTION;
		rightNode->data.action = rightAction;
		//std::cout << "Case 3: Both childs are leafs. Condition: " << c << " Left Action: " << leftAction.to_ulong() << " Right Action: " << rightAction.to_ulong() << std::endl;
		return 2;
	}


	// Case 2: Take best guess (highest p/total occurences), both children are conditions/nodes 
	int splitCandidate = r.conditions[0];
	double maximum_information_gain = 0;

	double baseEntropy = entropy(r.all_single_actions);
	//std::cout << "Base Entropy: " << baseEntropy << std::endl;

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
#endif

		if (informationGain > maximum_information_gain) {
			maximum_information_gain = informationGain;
			splitCandidate = c;
			leftIsAction = (leftEntropy == 0);
			rightIsAction = (rightEntropy == 0);
		}
		//std::cout << "Condition: " << c << " Max information gain: " << informationGain << " Ratio: " << difference << " LIG*RIG: " << LigTimesRig << "\tEntropy Left (0): " << leftEntropy << "\tEntropy Right (1): " << rightEntropy << std::endl;
	}
	//std::cout << "Split candidate chosen: " << rs.conditions[splitCandidate] << std::endl;


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
	//std::cout << "Processed instance. Split Candidate: " << rs.conditions[splitCandidate] << " Action Children: " << amount_of_action_children << std::endl;
	return amount_of_action_children;
}

void FindHdtIteratively(rule_set& rs, 
	BaseRuleSet& brs,
	BinaryDrag<conact>& tree,
	std::vector<RecursionInstance>& initial_recursion_instances,
	int start_depth = 0,
	int start_leaves = 0,
	int start_path_length_sum = 0,
	ullong start_rule_accesses = 0)
{
	std::vector<RecursionInstance> pending_recursion_instances = std::move(initial_recursion_instances);
	std::vector<RecursionInstance> upcoming_recursion_instances;
	int depth = start_depth;
	int leaves = start_leaves;
	int path_length_sum = start_path_length_sum;
	rule_accesses = start_rule_accesses;

	while (pending_recursion_instances.size() > 0) {
		std::cout << "Processing next batch of recursion instances (depth: " << depth << ", count: " << pending_recursion_instances.size() << ")" << std::endl;

		TLOG("Reading rules and classifying",
			HdtReadAndApplyRulesOnePass(brs, rs, pending_recursion_instances);
		);

		TLOG2("Processing instances", 
			for (auto& r : pending_recursion_instances) {
				int amount_of_action_children = HdtProcessNode(r, tree, rs, upcoming_recursion_instances);
				leaves += amount_of_action_children;
				path_length_sum += (depth + 1) * amount_of_action_children;
			}
		);
		depth++;
#if HDT_PROGRESS_ENABLED == true
		SaveProgressToFiles(upcoming_recursion_instances, tree, depth, leaves, path_length_sum, rule_accesses);
#endif
		pending_recursion_instances = std::move(upcoming_recursion_instances);
		upcoming_recursion_instances.clear();
		//getchar();
	}
	float average_path_length = path_length_sum / static_cast<float>(leaves);
	std::cout << "HDT construction done. Nodes: " << leaves << " Average path length: " << average_path_length << std::endl;
#if HDT_PROGRESS_ENABLED == true
	try {
		std::filesystem::remove(GetLatestProgressFilePath());
	} 
	catch (std::filesystem::filesystem_error e) {
		std::cerr << "Could not delete 'latest progress file'." << std::endl;
	}
#endif
}

void FindHdt(BinaryDrag<conact>::node* root, rule_set& rs, BaseRuleSet& brs, BinaryDrag<conact>& tree) {
#if HDT_BENCHMARK_READAPPLY == true
	std::string path = GetBenchmarkProgressFilePath();
	if (!std::filesystem::exists(path)) {
		std::cerr << "Benchmark progress file not found at " << path << std::endl;
		exit(EXIT_FAILURE);
	}
	bool load = true;
#else
	#if HDT_PROGRESS_ENABLED == true
		std::string path = GetLatestProgressFilePath();
		bool load = std::filesystem::exists(path);
	#else
		bool load = false;
	#endif
#endif
	if (load) {
		// continue existing run
		std::cout << "Progress file found, loading progress: ";
		std::ifstream is(path, ios::binary);
		std::string line;
		std::vector<RecursionInstance> r_insts;
		int depth, leaves, path_length_sum;
		uint64_t rule_accesses;
		bool tree_found = false, meta_found = false;
		while (std::getline(is, line))
		{
			std::istringstream iss(line);
			std::string keyword;
			iss >> keyword;

			if (keyword.compare("RecursionInstance") == 0) {
				r_insts.push_back(RecursionInstance(iss));
			}
			else if (keyword.compare("Tree") == 0) {
				std::string s;
				iss >> s;
				int val = 0;
				StringToTreeRec(s, tree, root, r_insts, val);
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
		if (r_insts.size() == 0) {
			std::cerr << "No RecursionInstances found in loaded progress file. Aborting." << std::endl;
			exit(EXIT_FAILURE);
		}
		if (!meta_found) {
			std::cerr << "No Meta section found in loaded progress file. Aborting." << std::endl;
			exit(EXIT_FAILURE);
		}
		if (!tree_found) {
			std::cerr << "No Tree found in loaded progress file. Aborting." << std::endl;
			exit(EXIT_FAILURE);
		}
		for (const auto& r : r_insts) {
			if (r.conditions.size() != CONDITION_COUNT - depth) {
				std::cerr << "RecursionInstance with wrongly sized condition array found. Aborting." << std::endl;
				exit(EXIT_FAILURE);
			}
			if (r.parent == nullptr) {
				std::cerr << "RecursionInstance with NULL parent found after loading progress file. Aborting." << std::endl;
				exit(EXIT_FAILURE);
			}
		}
		std::cout << "Depth: " << depth << ", RecursionInstances: " << r_insts.size() << ", Leaves: " << leaves << std::endl;
		FindHdtIteratively(rs, brs, tree, r_insts, depth, leaves, path_length_sum, rule_accesses);
	} else {
		// start new run
		#if HDT_PROGRESS_ENABLED
			std::cout << "No progress file found, starting new run from depth 0." << std::endl;
		#endif
		std::vector<int> conditions;
		conditions.reserve(rs.conditions.size());
		for (auto &c : rs.conditions) {
			conditions.push_back(rs.conditions_pos.at(c));
		}
		auto r = RecursionInstance(conditions, 0, 0, root);
		auto r_insts = std::vector<RecursionInstance>(1, r);
		FindHdtIteratively(rs, brs, tree, r_insts);
	}
}

BinaryDrag<conact> GenerateHdt(const rule_set& rs, BaseRuleSet& brs) {
	BinaryDrag<conact> tree;
	auto parent = tree.make_root();

	std::bitset<CONDITION_COUNT> set_conditions0, set_conditions1;

	bool b1 = set_conditions0.size() == rs.conditions.size();
	bool b2 = ACTION_COUNT == rs.actions.size();

	if (!(b1 && b2)) {
		std::cerr << "Assert failed: check ACTION_COUNT and CONDITION_COUNT." << std::endl;
		throw std::runtime_error("Assert failed: check ACTION_COUNT and CONDITION_COUNT.");
	}

	bool b3 = (HDT_INFORMATION_GAIN_METHOD_VERSION >= 1 && HDT_INFORMATION_GAIN_METHOD_VERSION <= 3);
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

#if HDT_BENCHMARK_READAPPLY == true
	std::cout << "\n\n***************************************************" << std::endl;
	std::cout << "***************** Benchmark Mode ******************" << std::endl;
	std::cout << "***************************************************" << std::endl;
#endif
	std::cout << "\nInformation gain method version: [" << HDT_INFORMATION_GAIN_METHOD_VERSION << "]" << std::endl;
	std::cout << "Combined classifier enabled: [" << (HDT_COMBINED_CLASSIFIER ? "Yes" : "No") << "]" << std::endl;
	std::cout << "Action source: [" << HDT_ACTION_SOURCE_STRINGS[HDT_ACTION_SOURCE] << "]" << std::endl;

	
#if HDT_PARALLEL_PARTITIONBASED == false
	brs.OpenRuleFiles();
#endif
	//brs.VerifyRuleFiles();

	FindHdt(parent, const_cast<rule_set&>(rs), brs, tree);

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

BinaryDrag<conact> GenerateHdt(const rule_set& rs, const BaseRuleSet& brs, const string& filename)
{
	TLOG("Generating HDT",
		auto t = GenerateHdt(rs, const_cast<BaseRuleSet&>(brs));
	);

	WriteConactTree(t, filename);
	return t;
}

BinaryDrag<conact> GetHdt(const rule_set& rs, const BaseRuleSet& brs, bool force_generation) {
	string hdt_filename = conf.hdt_path_.string();
	BinaryDrag<conact> t;
	if (conf.force_odt_generation_ || force_generation || !LoadConactTree(t, hdt_filename)) {
		t = GenerateHdt(rs, brs, hdt_filename);
	}
	return t;
}
