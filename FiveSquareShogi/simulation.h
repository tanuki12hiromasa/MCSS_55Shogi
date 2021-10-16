#pragma once
#include "tree.h"
#include "move_gen.h"
#include "kppt_evaluate.h"
#include "temperature.h"
#include "random.h"
#include <random>
#include <thread>
#include <functional>

class Simulation {
public:
	static void setDrawMoveNum(int n) { drawmovenum = n; }
private:
	static int drawmovenum;
	static Random::xoshiro256p rand_parent;

public:
	Simulation(SearchTree& tree):Simulation(tree, SearchTemperature::getTs(0)){}
	Simulation(SearchTree& tree, const double T_s);
	Simulation(SearchTree& tree, const double T_s, Random::xoshiro256p rand);

	void init();
	void simulate();
	void select();
	SearchNode* current_node()const { return history.back(); }
	SearchNode* select_child();
	SearchNode* select_bestchild() const;
	void proceed_to_child(SearchNode* const child);
	void expand();
	void backup();
	std::size_t qsearch(SearchNode* const root, SearchPlayer& player, int kifulength);
	double alphabeta(Move&, SearchPlayer&, int depth, double alpha, double beta);
	bool checkRepetiton(SearchNode* const node, const Kyokumen& kyokumen, const std::vector<SearchNode*>& history, const std::vector<std::pair<std::uint64_t, Bammen>>& k_history);
	bool checkRepetitiveCheck(const Kyokumen& k, const std::vector<SearchNode*>& searchhis, const SearchNode* const latestRepnode)const;
	
	SearchTree& tree;
	SearchPlayer player;
	double T_s;
	std::vector<SearchNode*> history;
	std::vector<std::pair<uint64_t, Bammen>> k_history;
	std::atomic_bool search_enable;
	std::uint64_t evalcount = 0;
	Random::xoshiro256p rand;

	/*
	温度：simulate毎に渡せばよろしい

	*/
};
