#pragma once
#include "agent.h"
#include "kppt_learn.h"
#include "kkppt_learn.h"
#include "kpp_kkpt_learn.h"
#include "kpp_learn.h"

#ifdef USE_KPPT
using LearnVec = kppt::kppt_paramVector;
using LearnOpt = kppt::Adam;
#endif

#ifdef USE_KKPPT
using LearnVec = kkppt::kkppt_paramVector;
#endif

#ifdef USE_KPP_KKPT
using LearnVec = kpp_kkpt::kpp_kkpt_paramVector;
using LearnOpt = kpp_kkpt::Adam;
#endif

#ifdef USE_KPP
using LearnVec = kpp::kpp_paramVector;
using LearnOpt = kpp::Adam;
#endif

enum class GameResult {
	SenteWin, GoteWin, Draw
};

enum class MyGameResult {
	PlayerWin, PlayerLose, Draw
};

class LearnUtil {
public:
	static double getChildrenZ(const SearchNode* const parent, const double T, double& CE);
	static SearchNode* choicePolicyRandomChild(const SearchNode* const root, const double T, double pip);
	static SearchNode* choiceRandomChild(const SearchNode* const root, double pip);
	static SearchNode* choiceBestChild(const SearchNode* const root);
	static SearchNode* getPrincipalLeaf(const SearchNode* const root);
	static SearchPlayer getQSBest(const SearchNode* const root, SearchPlayer& player, const int depthlimit);
	static SearchPlayer getQSBest(const SearchNode* const root, SearchPlayer& player);
	static LearnVec getGrad(const SearchNode* const root, const SearchPlayer& player, bool teban, unsigned long long samplingnum, const int qsdepth=8);
	static LearnVec getSamplingGrad(const SearchNode* root, const SearchPlayer& player, bool teban, unsigned long long samplingnum, const int qsdepth = 8);
	static LearnVec getSamplingGradV(const SearchNode* root, const SearchPlayer& player,unsigned samplingnum);
	static LearnVec getSamplingGradQ(const SearchNode* root, const SearchPlayer& player, unsigned samplingnum);
	static double EvalToProb(const double eval);
	static double EvalToSignProb(const double eval);
	static double BackProb(const SearchNode& parent, const SearchNode& child, const double T);
	static double ResultToProb(GameResult result, bool teban);
	static double ResultToReward(GameResult result, bool teban, double r_win, double r_draw, double r_lose);
	static double ResultToProb(MyGameResult result);
	static double ResultToReward(MyGameResult result, double r_win, double r_draw, double r_lose);
	static std::string ResultToString(const GameResult& result);
	static std::string ResultToString(const MyGameResult& result);
	static constexpr double probT = 600.0;
	static double pTb;
	static double change_evalTs_to_probTs(const double evalT);
	static std::string getDateString();
};
