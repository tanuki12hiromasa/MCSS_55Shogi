#include "learn_util.h"
#include <iostream>

double LearnUtil::pTb = 0.1;//評価関数をシグモイド関数に写像した場合の方策の温度は、(元の温度)*(写像先の標準的な評価値差)/(元の標準的な評価値差)で求められる。

double LearnUtil::getChildrenZ(const SearchNode* const parent, const double T, double& CE) {
	if (parent->children.empty()) return 1;
	CE = std::numeric_limits<double>::max();
	for (const auto& child : parent->children) {
		CE = std::min(CE, child.eval.load());
	}
	double Z = 0;
	for (const auto& child : parent->children) {
		Z += std::exp(-(child.eval - CE) / T);
	}
	return Z;
}

SearchNode* LearnUtil::choicePolicyRandomChild(const SearchNode* const root, const double T, double pip) {
	using dn = std::pair<double, SearchNode*>;
	double CE = std::numeric_limits<double>::max();
	if (root->isLeaf()) return nullptr;
	std::vector<dn> evals; evals.reserve(root->children.size());
	for (auto& child : root->children) {
		const double eval = child.eval;
		evals.push_back(std::make_pair(eval, &child));
		if (eval < CE) {
			CE = eval;
		}
	}
	if (evals.empty()) {
		return nullptr;
	}
	double Z = 0;
	for (const auto& eval : evals) {
		Z += std::exp(-(eval.first - CE) / T);
	}
	pip *= Z;
	for (const auto& eval : evals) {
		pip -= std::exp(-(eval.first - CE) / T);
		if (pip <= 0) {
			return eval.second;
		}
	}
	return evals.front().second;
}

SearchNode* LearnUtil::choiceRandomChild(const SearchNode* const root, const double pip) {
	const int index = std::clamp(static_cast<int>(root->children.size() * pip), 0, (int)root->children.size() - 1);
	return root->children.begin() + index;
}

SearchNode* LearnUtil::choiceBestChild(const SearchNode* const root) {
	double min = std::numeric_limits<double>::max();
	SearchNode* best = nullptr;
	for (auto& child : root->children) {
		double eval = child.getEs();
		if (eval < min) {
			min = eval;
			best = &child;
		}
	}
	return best;
}

SearchNode* LearnUtil::getPrincipalLeaf(const SearchNode* const root) {
	const auto child = choiceBestChild(root);
	if (child==nullptr || child->isTerminal() || child->isLeaf())
		return child;
	else 
		return getPrincipalLeaf(child);
}

double alphabeta(Move& pmove, SearchPlayer& player, int depth, double alpha, double beta, SearchPlayer& bestplayer) {
	const auto eval = Evaluator::evaluate(player);
	if (depth <= 0) {
		return eval;
	}
	if (eval > alpha) {
		alpha = eval;
	}
	if (alpha >= beta) {
		return alpha;
	}
	auto moves = MoveGenerator::genCapMove(pmove, player.kyokumen);
	if (moves.empty() || pmove.isOute()) {
		return eval;
	}
	for (auto& m : moves) {
		const FeatureCache cache = player.feature.getCache();
		const koma::Koma captured = player.proceed(m);
		SearchPlayer cbestplayer = player;
		const double ceval = -alphabeta(m, player, depth - 1, -beta, -alpha, cbestplayer);
		if (ceval > alpha) {
			alpha = ceval;
			bestplayer = cbestplayer;
		}
		player.recede(m, captured, cache);
		if (alpha >= beta) {
			return alpha;
		}
	}
	return alpha;
}

SearchPlayer LearnUtil::getQSBest(const SearchNode* const root, SearchPlayer& player, const int depthlimit) {
	SearchPlayer bestplayer = player;
	if (depthlimit <= 0) {
		return player;
	}
	Move m(root->move);
	auto moves = MoveGenerator::genCapMove(m, player.kyokumen);
	if (moves.empty()) {
		return player;
	}
	double max = root->eval;
	for (auto m : moves) {
		const FeatureCache cache = player.feature.getCache();
		const koma::Koma captured = player.proceed(m);
		SearchPlayer cbestplayer = player;
		const double eval = -alphabeta(m, player, depthlimit - 1, std::numeric_limits<double>::lowest(), -max, cbestplayer);
		if (eval > max) {
			max = eval;
			bestplayer = cbestplayer;
		}
		player.recede(m, captured, cache);
	}
	return bestplayer;
}

SearchPlayer LearnUtil::getQSBest(const SearchNode* const root, SearchPlayer& player) {
	return getQSBest(root, player, SearchNode::getQSdepth());
}

LearnVec LearnUtil::getGrad(const SearchNode* const root, const SearchPlayer& rootplayer, bool teban, unsigned long long samplingnum, const int qsdepth) {
	const double T = 120;
	LearnVec vec;
	if (root == nullptr) return vec;
	if (root->children.empty()) {
		auto player = rootplayer;
		double c = 1;
		const auto qsbest = getQSBest(root, player, qsdepth);
		if (qsbest.kyokumen.teban() != player.kyokumen.teban()) c = -c;
		vec.addGrad(c, qsbest);
		return vec;
	}
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine{ std::random_device()() };
	for (unsigned long long i = 0; i < samplingnum; i++) {
		const SearchNode* node = root;
		double c = 1;
		double Peval_prev = (teban == rootplayer.kyokumen.teban()) ? EvalToProb(node->eval) : (1 - EvalToProb(node->eval));
		SearchPlayer player = rootplayer;
		while (!node->isLeaf()) {
			auto next = choicePolicyRandomChild(node, T, random(engine));
			if (next == nullptr)break;
			node = next;
			player.proceed(node->move);
			const double Peval = (teban == player.kyokumen.teban()) ? EvalToProb(node->eval) : (1 - EvalToProb(node->eval));
			c *= -((Peval - Peval_prev) / pTb + 1);
			Peval_prev = Peval;
		}
		/*const auto qsbest = getQSBest(node, player, qsdepth);
		const double Peval = EvalToProb(Evaluator::evaluate(qsbest));
		c *= probT * Peval * (1 - Peval);
		if (qsbest.kyokumen.teban() != player.kyokumen.teban()) c = -c;
		vec.addGrad(c, qsbest, teban);*/
		const double Peval = EvalToProb(Evaluator::evaluate(player));
		c *= probT * Peval * (1 - Peval);
		vec.addGrad(c, player);
	}
	vec *= (1.0 / samplingnum);
	return vec;
}

LearnVec LearnUtil::getSamplingGrad(const SearchNode* const root, const SearchPlayer& rootplayer, const bool teban, unsigned long long samplingnum, const int qsdepth) {
	const double T = 100;
	LearnVec vec;
	if (root == nullptr) return vec;
	if (root->children.empty()) {
		auto player = rootplayer;
		double c = 1;
		if (teban == player.kyokumen.teban()) {
			const double sigH = EvalToProb(Evaluator::evaluate(player));
			c = probT * sigH * (1 - sigH);
			vec.addGrad(c, player);
		}
		else {
			const double sigH = EvalToProb(Evaluator::evaluate(player));
			c = -probT * sigH * (1 - sigH);
			vec.addGrad(c, player);
		}
		return vec;
	}
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine{ std::random_device()() };
	for (unsigned long long i = 0; i < samplingnum; i++) {
		const SearchNode* node = root;
		double c = 1;
		SearchPlayer player = rootplayer;
		while (!node->isLeaf()) {
			if (player.kyokumen.teban() == teban) {
				const double sigV = EvalToProb(node->eval);
				node = choicePolicyRandomChild(node, T, random(engine));
				player.proceed(node->move);
				const double sigQ = 1 - EvalToProb(node->eval);
				c *= (sigQ - sigV) / pTb + 1;
			}
			else {
				node = choicePolicyRandomChild(node, T, random(engine));
				player.proceed(node->move);
			}
		}
		if (player.kyokumen.teban() == teban) {
			const double sigH = EvalToProb(Evaluator::evaluate(player));
			c *= probT * sigH * (1 - sigH);
			vec.addGrad(c, player);
		}
		else {
			const double sigH = EvalToProb(Evaluator::evaluate(player, false));
			c *= -probT * sigH * (1 - sigH);
			vec.addGrad(c, player);
		}
	}
	vec *= (1.0 / samplingnum);
	return vec;
}

LearnVec LearnUtil::getSamplingGradV(const SearchNode* const root, const SearchPlayer& rootplayer, const unsigned samplingnum) {
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine{ std::random_device()() };
	const double T = SearchTemperature::Te;
	LearnVec vec;
	if (!root)return vec;
	if (root->children.empty() || root->isLeaf()) {
		vec.addGrad(1, rootplayer);
		return vec;
	}
	for (unsigned count = 0; count < samplingnum; count++) {
		const SearchNode* node = root;
		SearchPlayer player = rootplayer;
		double c = 1;
		while (true) {
			if (node->children.empty() || node->isLeaf()) {
				const double sigH = EvalToProb(node->eval);
				c *= probT * sigH * (1 - sigH);
				vec.addGrad(c, player);
				break;
			}
			SearchNode* enode = choicePolicyRandomChild(node, T, random(engine));
			if (!enode || enode->children.empty() || enode->isLeaf()) {
				const double sigH = EvalToProb(node->eval);
				c *= probT * sigH * (1 - sigH);
				vec.addGrad(c, player);
				break;
			}
			const double V = EvalToProb(node->eval);
			const double Q = 1 - EvalToProb(enode->eval);
			c *= (Q - V) / pTb + 1;
			const double fnodeeval = node->eval;
			node = choicePolicyRandomChild(enode, T, random(engine));
			if (!node) {
				const double sigH = EvalToProb(fnodeeval);
				c *= probT * sigH * (1 - sigH);
				vec.addGrad(c, player);
				break;
			}
			player.proceed(enode->move);
			player.proceed(node->move);
		}
	}
	vec *= 1.0 / samplingnum;
	return vec;
}

LearnVec LearnUtil::getSamplingGradQ(const SearchNode* root, const SearchPlayer& rootplayer, const unsigned samplingnum) {
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine{ std::random_device()() };
	const double T = SearchTemperature::Te;
	LearnVec vec;
	if (!root)return vec;
	if (root->children.empty() || root->isLeaf()) {
		vec.addGrad(-1, rootplayer);
		return vec;
	}
	for (unsigned count = 0; count < samplingnum; count++) {
		const SearchNode* node = root;
		SearchPlayer player = rootplayer;
		double c = 1;
		while (true) {
			SearchNode* fnode = choicePolicyRandomChild(node, T, random(engine));
			if (!fnode) break;
			player.proceed(fnode->move);
			if (fnode->children.empty() || fnode->isLeaf()) {
				const double sigH = EvalToProb(fnode->eval);
				c *= probT * sigH * (1 - sigH);
				vec.addGrad(c, player);
				break;
			}
			node = choicePolicyRandomChild(fnode, T, random(engine));
			if (!node || node->children.empty() || node->isLeaf()) {
				const double sigH = EvalToProb(fnode->eval);
				c *= probT * sigH * (1 - sigH);
				vec.addGrad(c, player);
				break;
			}
			const double V = EvalToProb(fnode->eval);
			const double Q = 1 - EvalToProb(node->eval);
			c *= (Q - V) / pTb + 1;
			player.proceed(node->move);
		}
	}
	vec *= 1.0 / samplingnum;
	return vec;
}

double LearnUtil::EvalToProb(const double eval) {
	return 1.0 / (1.0 + std::exp(-eval / probT));
}

double LearnUtil::EvalToSignProb(const double eval) {
	return 2.0 / (1.0 + std::exp(-eval / probT)) - 1.0;
}

double LearnUtil::BackProb(const SearchNode& parent, const SearchNode& child, const double T) {
	double Z = 0;
	for (const auto& c : parent.children) {
		Z += std::exp(-(c.eval + parent.eval) / T);
	}
	return std::exp(-(child.eval + parent.eval) / T) / Z;
}

double LearnUtil::ResultToProb(GameResult result, bool teban) {
	switch (result)
	{
		case GameResult::SenteWin:
			return teban ? 1.0 : 0.0;
		case GameResult::GoteWin:
			return teban ? 0.0 : 1.0;
		case GameResult::Draw:
		default:
			return 0.5;
	}
}

double LearnUtil::ResultToReward(const GameResult result, const bool teban, const double win, const double draw, const double lose) {
	switch (result)
	{
		case GameResult::SenteWin:
			return teban ? win : lose;
		case GameResult::GoteWin:
			return teban ? lose : win;
		case GameResult::Draw:
		default:
			return draw;
	}
}

double LearnUtil::ResultToProb(MyGameResult result) {
	switch (result)
	{
		case MyGameResult::PlayerWin:
			return 1.0;
		case MyGameResult::PlayerLose:
			return 0;
		case MyGameResult::Draw: default:
			return 0.5;
	}
}

double LearnUtil::ResultToReward(const MyGameResult result, const double win, const double draw, const double lose) {
	switch (result)
	{
		case MyGameResult::PlayerWin:
			return win;
		case MyGameResult::PlayerLose:
			return lose;
		case MyGameResult::Draw: default:
			return draw;
	}
}

std::string LearnUtil::ResultToString(const GameResult& result) {
		switch (result)
	{
		case GameResult::SenteWin:
			return "sw";
		case GameResult::GoteWin:
			return "gw";
		case GameResult::Draw:
		default:
			return "draw";
	}
}

std::string LearnUtil::ResultToString(const MyGameResult& result) {
	switch (result)
	{
		case MyGameResult::PlayerWin:
			return "win";
		case MyGameResult::PlayerLose:
			return "lose";
		case MyGameResult::Draw: default:
			return "draw";
	}
}

double LearnUtil::change_evalTs_to_probTs(const double T) {
	return T * 0.2 / 500;
}

std::string LearnUtil::getDateString() {
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::string s(30, '\0');
	std::tm ima;
	localtime_s(&ima, &now);
	std::strftime(&s[0], s.size(), "%Y%m%d-%H%M", &ima);
	s.resize(s.find('\0'));
	return s;
}