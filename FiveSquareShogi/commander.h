#pragma once
#include "agent.h"
#include "time_property.h"
#include <mutex>

class Commander {
public:
	static void execute(const std::string&);

private:
	Commander();
	~Commander();
	static void coutOption();
	void setOption(const std::vector<std::string>& token);
	void paramInit();
	void gameInit();
	void setTsDistribution();

	void startAgent();
	void stopAgent();
	void go(const std::vector<std::string>& tokens);
	std::pair<std::chrono::milliseconds, std::chrono::milliseconds> decide_timelimit(const TimeProperty time)const;
	void info();
	void chakushu(SearchNode* const bestmove);
	void position(const std::vector<std::string>& tokens);
	void releaseAgentAndBranch(SearchNode* const prevRoot, std::vector<SearchNode*>&& newNodes);
	void releaseAgentAndTree(SearchNode* const root);

	SearchTree tree;
	std::vector<std::unique_ptr<SearchAgent>> agents;
	std::unique_ptr<std::thread> deleteThread;
	int agentNum = 6;
	bool permitPonder;
	bool continuousTree = true;
	double Ts_min = 40;
	double Ts_max = 200;
	int TsDistFuncNum = 0;
	std::vector<double> TsDistribution;
	int resign_border = 10;
	std::chrono::milliseconds time_quickbm_lower{ 4000 };
	std::chrono::milliseconds time_standard_upper{ 10000 };
	std::chrono::milliseconds time_overhead{ 150 };
	int estimate_movesnum = 120;

	std::thread go_thread;
	std::thread info_thread;
	std::atomic_bool go_alive;
	std::atomic_bool info_enable;
	std::atomic_bool info_alive;
	uint64_t info_prev_evcount;
	std::chrono::time_point<std::chrono::system_clock> info_prevtime;

	std::mutex coutmtx;
	std::mutex treemtx;

	//値域 [0,1.0) のランダムな値
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine{ std::random_device()() };

	friend class ShogiTest;
};