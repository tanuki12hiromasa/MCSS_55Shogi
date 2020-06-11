#pragma once
#include "learn_util.h"
#include "agent.h"

class Learner {
public:
	static void execute();

private:
	void init(const std::vector<std::string>& cmdtokens);
	void search(SearchTree& tree);
	LearnVec reinforcement_learn(const std::vector<std::string>& cmdtokens);

	double T_search = 120;
	std::chrono::milliseconds searchtime{ 300 };
	int agentnum = 8;

	double child_pi_limit = 0.00005;
	double samplingrate = 0.1;

	double learning_rate_td = 0.01;
	double learning_rate_pp = 0.01;
	double learning_rate_bts = 0.01;
	double learning_rate_reg = 0.01;
	double learning_rate_pge = 0.01;

	double td_gamma = 0.9;
	double td_lambda = 0.8;
};