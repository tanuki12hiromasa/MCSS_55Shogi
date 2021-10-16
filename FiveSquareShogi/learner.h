#pragma once
#include "learn_util.h"
#include "learn_agent.h"
#include "learn_method.h"

class Kifu {
public:
	Kyokumen startpos;
	std::vector<Move> moves;
	bool teban;
	GameResult result;

	void read_from_command();
};

class Learner {
public:
	static void execute();

private:
	bool enable = true;

	void init(const std::vector<std::string>& cmdtokens);
	static void coutOption();
	void setOption(const std::vector<std::string>& token);

	void search(SearchTree& tree);
	void search(SearchTree& tree, const std::chrono::milliseconds time);
	void search(SearchTree& tree, const long long simulate_num);
	static int getWinner(std::vector<std::string>& sfen);

	void learn_start_by_randompos(int batch,int itr);
	
	void lbk_options();
	void learn_by_kifu();

	void test();

	static bool LoadTempInfo(int& itr, int& batch, int& itr_max, int& bat_max, std::string& datestr, const std::string& dir = "./learning");
	static bool SaveTempInfo(int itr, int batch, int itr_max, int bat_max,const std::string& datestr, const std::string& dir = "./learning");
	static bool LoadTempVec(LearnVec& vec, const int vecnum = 1, const std::string& dir = "./learning/");
	static bool SaveTempVec(LearnVec& vec, const int vecnum = 1, const std::string& dir = "./learning/");

	double T_search = 200;
	double T_selfplay = 120;
	std::chrono::milliseconds searchtime{ 1000 };
	int agentnum = 8;
	LearnAgentPool agents;

	double child_pi_limit = 0.00005;
	double samplingrate = 0.1;

	long long int batch_size = 1;

	long long unsigned samplingnum = 10000;

	double learning_rate_td = 0.01;
	double learning_rate_pp = 0.1;
	double learning_rate_bts = 0.1;
	double learning_rate_reg = 0.1;
	double learning_rate_pg = 0.1;
	double learning_rate_bts_sampling = 0.1;

	double td_r_win = 1;
	double td_r_draw = 0;
	double td_r_lose = -1;
	
	double td_gamma = 0.95;
	double td_lambda = 0.9;

	double pg_r_win = 1;
	double pg_r_draw = 0;
	double pg_r_lose = -1;

	friend class ShogiTest;
};