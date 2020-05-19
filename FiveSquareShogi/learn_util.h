#pragma once
#include "agent.h"

class LearnUtil {
public:
	static SearchNode* choiceChildRandom(const SearchNode* const root, const double T, double pip);
	static SearchPlayer getQSBest(const SearchNode* const root, const SearchPlayer& player, const int depthlimit);
};