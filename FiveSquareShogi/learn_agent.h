#pragma once
#include "tree.h"
#include "learn_method.h"
#include "random.h"
#include <condition_variable>
#include <chrono>

using Method = SamplingBTS;


class LearnAgent {
public:
	static int drawmovenum;
public:
	LearnAgent(const Random::xoshiro256p&);

	void setTree(SearchTree* const tree) { this->tree = tree; }
	void setTs(const double T) { Ts = T; }
	void start();
	void stop();
	void deleteTree();
	void join();

private:
	enum class State {
		pause, search, del, terminate
	};
	
	void loop();
	void search();
	size_t qsimulate(SearchNode* const root, SearchPlayer& player, const int hislength);
	bool checkRepetitiveCheck(const Kyokumen& k, const std::vector<SearchNode*>& searchhis, const SearchNode* const latestRepnode)const;
	void del();

	std::atomic<State> status;
	std::atomic_bool inSearch;
	std::mutex mtx;
	std::condition_variable cv;
	SearchTree* tree;
	double Ts = 90;
	std::thread th;
	Random::xoshiro256p random;
};

class LearnAgentPool {
public:
	void setAgentNum(int num) { agentNum = num; }

	void deleteTree(SearchTree& tree);
	void search(SearchTree& tree, std::chrono::milliseconds ms);
	//void search(SearchTree& tree, unsigned long long num);
	//void learn(SearchTree& tree, Method& method);
	
private:
	int agentNum = 8;
	std::vector<std::unique_ptr<LearnAgent>> agents;

};