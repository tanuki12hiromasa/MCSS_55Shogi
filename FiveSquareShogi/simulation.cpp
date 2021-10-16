#include "stdafx.h"
#include "simulation.h"
#include "leaf_guard.h"

int Simulation::drawmovenum = 320;
Random::xoshiro256p Simulation::rand_parent{ 0x1234567ULL,0x928AC12ULL,0x8FE4561199ULL,0xC123ABDDD1ULL };

Simulation::Simulation(SearchTree& tree, const double T_s) 
	:Simulation(tree, T_s, rand_parent)
{
	rand_parent.jump();
}

Simulation::Simulation(SearchTree& tree, const double T_s, Random::xoshiro256p rand)
	: tree(tree), player(tree.getRootPlayer()), T_s(T_s), rand(rand)
{
	search_enable = true;
}

void Simulation::init() {
	history.clear();
	history.push_back(tree.getRoot());
	k_history.clear();
	player = tree.getRootPlayer();
}


void Simulation::simulate() {
	init();
	if (!search_enable) return;
	select();
	if (!search_enable) return;
	expand();
	backup();
}

void Simulation::select() {
	SearchNode* node = current_node();
	while (!node->isLeaf()) {
		if (!search_enable) return;
		node = select_child();
		proceed_to_child(node);
	}
}

SearchNode* Simulation::select_child() {
	using dn = std::pair<double, SearchNode*>;

	SearchNode* const node = current_node();
	double CE = std::numeric_limits<double>::max();
	std::vector<dn> evals; evals.reserve(node->children.size());
	for (auto& child : node->children) {
		if (child.isSearchable()) {
			double eval = child.getEs();
			evals.push_back(std::make_pair(eval, &child));
			if (eval < CE) {
				CE = eval;
			}
		}
	}
	if (evals.empty()) {
		//node->status = SearchNode::State::Terminal;
		if (history.size() == 1) {
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(200us);
		}
		return nullptr;
	}
	const double T_c = SearchTemperature::getTs_atNode(T_s, *node);
	double Z = 0;
	for (const auto& eval : evals) {
		Z += std::exp(-(eval.first - CE) / T_c);
	}
	double pip = Z * rand.rand01();
	SearchNode* next = evals.front().second;
	for (const auto& eval : evals) {
		pip -= std::exp(-(eval.first - CE) / T_c);
		if (pip <= 0) {
			next = eval.second;
			break;
		}
	}
	return next;
}

SearchNode* Simulation::select_bestchild() const {
	const SearchNode* const node = current_node();
	const auto& children = node->children;
	if (children.empty()) return nullptr;
	SearchNode* bestchild = children.begin();
	for (auto child = children.begin(); child != children.end(); child++) {
		if (child->eval < bestchild->eval) {
			bestchild = child;
		}
	}
	return bestchild;
}

void Simulation::proceed_to_child(SearchNode* const child) {
	//局面を進める
	k_history.push_back(std::make_pair(player.kyokumen.getHash(), player.kyokumen.getBammen()));
	player.proceed(child->move);
	history.push_back(child);
}

void Simulation::expand() {
	const double T_e = SearchTemperature::Te;
	SearchNode* const node = current_node();
	LeafGuard dredear(node);
	if (!dredear.Result()) {
		return;
	}

	{//子ノード生成
		const auto moves = MoveGenerator::genMove(node->move, player.kyokumen);
		if (moves.empty()) {
			node->setMate();
			return;
		}
		else if (history.size() - 1 + tree.getMoveNum() >= drawmovenum) {
			node->setDraw(player.kyokumen.teban());
			return;
		}
		node->addChildren(moves);
	}

	uint64_t evalcount = 0ull;
	for (auto& child : node->children) {
		const auto cache = player.proceedC(child.move);
		if (!checkRepetiton(&child, player.kyokumen, history, k_history)) {
			evalcount += qsearch(&child, player, history.size());
		}
		player.recede(child.move, cache);
	}
	tree.addEvaluationCount(evalcount);
	//sortは静止探索後の方が評価値順の並びが維持されやすい　親スタートの静止探索ならその前後共にsortしてもいいかもしれない
	node->children.sort();
	//sortしたので一番上が最小値になっているはず
	double emin = node->children.begin()->eval;
	double Z_e = 0;
	for (const auto& child : node->children) {
		Z_e += std::exp(-(child.eval - emin) / T_e);
	}
	double E = 0;
	for (const auto& child : node->children) {
		E -= child.eval * std::exp(-(child.eval - emin) / T_e) / Z_e;
	}
	node->setEvaluation(E);
	node->setMass(1);
}

std::size_t Simulation::qsearch(SearchNode* const root, SearchPlayer& player, const int kifulength) {
	const int& depth = SearchNode::getQSdepth();
	if (depth <= 0) {
		const double eval = Evaluator::evaluate(player);
		root->setEvaluation(eval);
		root->setOriginEval(eval);
		return 1ull;
	}
	auto moves = MoveGenerator::genCapMove(root->move, player.kyokumen);
	if (moves.empty()) {
		if (root->move.isOute()) {
			root->setMate();
			return 1ull;
		}
		else {
			const double eval = Evaluator::evaluate(player);
			root->setEvaluation(eval);
			root->setOriginEval(eval);
			return 1ull;
		}
	}
	else if (kifulength - 1 + tree.getMoveNum() >= drawmovenum) {
		root->setDraw(player.kyokumen.teban());
		return 1ull;
	}
	double max = Evaluator::evaluate(player);
	evalcount = 1;
	for (auto& m : moves) {
		const FeatureCache cache = player.feature.getCache();
		const koma::Koma captured = player.proceed(m);
		const double eval = -alphabeta(m, player, depth - 1, std::numeric_limits<double>::lowest(), -max);
		if (eval > max) {
			max = eval;
		}
		player.recede(m, captured, cache);
	}
	root->setEvaluation(max);
	root->setOriginEval(max);
	return evalcount;
}

void Simulation::backup() {
	using dd = std::pair<double, double>;
	const double MateScoreBound = SearchNode::getMateScoreBound();
	const double T_e = SearchTemperature::Te;
	const double T_d = SearchTemperature::Td;

	for (int i = history.size() - 2; i >= 0; i--) {
		SearchNode* const node = history[i];
		double emin = std::numeric_limits<double>::max();
		std::vector<dd> emvec; emvec.reserve(node->children.size());
		for (const auto& child : node->children) {
			const double eval = child.getEvaluation();
			const double mass = child.mass;
			emvec.push_back(std::make_pair(eval, mass));
			if (eval < emin) {
				emin = eval;
			}
		}
		if (std::abs(emin) > MateScoreBound) {
			node->setMateVariation(emin);
		}
		else {
			double Z_e = 0;
			double Z_d = 0;
			for (const auto& em : emvec) {
				const double eval = em.first;
				Z_e += std::exp(-(eval - emin) / T_e);
				Z_d += std::exp(-(eval - emin) / T_d);
			}
			double E = 0;
			double M = 1;
			for (const auto& em : emvec) {
				const double eval = em.first;
				const double mass = em.second;
				E -= eval * std::exp(-(eval - emin) / T_e) / Z_e;
				M += mass * std::exp(-(eval - emin) / T_d) / Z_d;
			}
			node->setEvaluation(E);
			node->setMass(M);
		}
	}
}

double Simulation::alphabeta(Move& pmove, SearchPlayer& player, int depth, double alpha, double beta) {
	evalcount++;
	const auto eval = Evaluator::evaluate(player);
	if (depth <= 0) {
		return eval;
	}
	alpha = std::max(eval, alpha);
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
		alpha = std::max(-alphabeta(m, player, depth - 1, -beta, -alpha), alpha);
		player.recede(m, captured, cache);
		if (alpha >= beta) {
			return alpha;
		}
	}
	return alpha;
}


bool Simulation::checkRepetiton(SearchNode* const node, const Kyokumen& kyokumen, const std::vector<SearchNode*>& history, const std::vector<std::pair<std::uint64_t, Bammen>>& k_history) {
	unsigned repnum = 0;
	SearchNode* repnode = nullptr;
	SearchNode* latestRepnode = nullptr;


	auto p = tree.findRepetition(kyokumen);
	repnum += p.first;
	repnode = p.second;
	const auto hash = kyokumen.getHash();
	//先後一致している方のみを調べればよいので1つ飛ばしで調べる
	for (int t = k_history.size() - 2; t >= 0; t -= 2) {
		const auto& k = k_history[t];
		if (k.first == hash && k.second == kyokumen.getBammen()) {
			repnum++;
			repnode = history[t];
			if (latestRepnode == nullptr) {
				latestRepnode = repnode;
			}
		}
	}
	if (latestRepnode == nullptr) {
		latestRepnode = repnode;
	}

	if (repnum > 0/*千日手である*/) {
		if (repnum >= 3) {
			//同一局面が4回繰り返されているなら、ゲーム終了
			if (checkRepetitiveCheck(kyokumen, history, latestRepnode)) {
				node->setRepetitiveCheckmate();
			}
			else {
				node->setRepetitiveEnd(kyokumen.teban());
			}
		}
		else {
			//繰り返し回数が少ない場合は、評価値を千日手にする
			if (checkRepetitiveCheck(kyokumen, history, latestRepnode)) {
				node->setRepetitiveCheck();
			}
			else {
				node->setRepetition(kyokumen.teban());
			}
		}
		return true;
	}
	return false;
}

bool Simulation::checkRepetitiveCheck(const Kyokumen& kyokumen, const std::vector<SearchNode*>& searchhis, const SearchNode* const repnode)const {
	//対象ノードは未展開なので局面から王手かどうか判断する この関数は4回目の繰り返しの終端ノードで呼ばれているのでkusemonoを何回も生成するような無駄は発生していない
	if (!searchhis.back()->move.isOute()) {
		if (!kyokumen.isOute(searchhis.back()->move)) {
			return false;
		}
		searchhis.back()->move.setOute(true);
	}
	//過去ノードはmoveのouteフラグから王手だったか判定する
	int t;
	for (t = searchhis.size() - 3; t >= 0; t -= 2) {//historyの後端は末端ノードなのでその二つ前から調べていく
		if (!searchhis[t]->move.isOute()) {
			return false;
		}
		if (searchhis[t] == repnode) {
			return true;
		}
	}
	const auto& treehis = tree.getHistory();
	for (t += treehis.size() - 1; t >= 0; t -= 2) {
		if (!treehis[t]->move.isOute()) {
			return false;
		}
		if (treehis[t] == repnode) {
			return true;
		}
	}
	return false;
}