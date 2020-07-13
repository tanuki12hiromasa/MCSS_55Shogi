#pragma once
#include "learn_util.h"
#include "agent.h"

class Learner {
public:
	static void execute();

private:
	void init(const std::vector<std::string>& cmdtokens);
	void search(SearchTree& tree);
	void search(SearchTree& tree, const std::chrono::milliseconds time);
	static int getWinner(std::vector<std::string>& sfen);
	LearnVec reinforcement_learn(const Kyokumen startpos, const std::vector<Move>& kifu, const int winner, const bool learnteban);
	LearnVec reinforcement_learn(std::vector<std::string> cmdtokens,const bool learnteban);
	LearnVec simple_bootstrap(const Kyokumen startpos, const std::vector<Move>& kifu, const int winner, const bool learnteban);
	LearnVec sampling_bootstrap(const Kyokumen startpos, const std::vector<Move>& kifu, const int winner, const bool learnteban);

	void consecutive_rl(const std::string& sfenfile);
	void selfplay_learn(const std::vector<std::string>& comdtokens);
	void selfplay_simple_bootstrap();
	void selfplay_child_bootstrap();
	void selfplay_sampling_regression();
	void selfplay_sampling_pge();
	void selfplay_sampling_td();

	double T_search = 120;
	double T_selfplay = 100;
	std::chrono::milliseconds searchtime{ 500 };
	int agentnum = 8;

	double child_pi_limit = 0.00005;
	double samplingrate = 0.1;

	double learning_rate_td = 0.1;
	double learning_rate_pp = 0.1;
	double learning_rate_bts = 0.1;
	double learning_rate_reg = 0.1;
	double learning_rate_pge = 0.1;

	double td_gamma = 0.95;
	double td_lambda = 0.9;

	friend class ShogiTest;
};