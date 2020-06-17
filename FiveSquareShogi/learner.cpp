#include "learner.h"
#include <iostream>
#include <fstream>
#include "usi.h"

void Learner::execute() {
	Learner learner;
	while (true) {
		std::string usiin;
		std::getline(std::cin, usiin);
		auto tokens = usi::split(usiin, ' ');
		if (tokens.empty()) {
			std::cout << "command ready" << std::endl;
		}
		else if (tokens[0] == "init") {
			//設定ファイルを読み込む
			//init 設定ファイル.txt
			learner.init(tokens);
			std::cout << "init done." << std::endl;
		}
		else if (tokens[0] == "genparam") {
			//学習前の初期パラメータを生成する
			//genparam 出力先フォルダ

		}
		else if (tokens[0] == "rlearn") {
			//強化学習で勾配ベクトルを求める
			//rlearn sfen startpos moves ... 
			const auto grad = learner.reinforcement_learn(tokens);
			grad.updateEval();
			std::cout << "rlearn done." << std::endl;
		}
		else if (tokens[0] == "saveparam") {
			//更新したパラメータを保存
			//saveparam [保存先フォルダ]
			if (tokens.size() > 2) Evaluator::setpath_input(usiin.substr(10));
			Evaluator::save();
			std::cout << "saveparam done." << std::endl;
		}
		else if (tokens[0] == "crl") {
			//rlearnを連続で行う 
			//crl 棋譜.sfen
			learner.consecutive_rl(usiin.substr(4));
		}
		else if (tokens[0] == "quit") {
			break;
		}
	}
}

void Learner::init(const std::vector<std::string>& cmdtokens) {
	BBkiki::init();
	Evaluator::init();
}

void Learner::search(SearchTree& tree) {
	std::vector<std::unique_ptr<SearchAgent>> agents;
	for (int i = 0; i < agentnum; i++) {
		agents.push_back(std::unique_ptr<SearchAgent>(new SearchAgent(tree, T_search, i)));
	}
	std::this_thread::sleep_for(searchtime);
	for (auto& ag : agents) {
		ag->stop();
	}
	for (auto& ag : agents) {
		ag->terminate();
	}
	agents.clear();
}

//どちらが勝ったかを返す関数 1:先手勝ち -1:後手勝ち 0:引き分け
double getWinner(std::vector<std::string>& sfen) {
	int startnum = 0;
	for (int t = 0; t < sfen.size(); t++) {
		if (sfen[t] == "moves") {
			startnum = t;
			break;
		}
	}
	if (sfen.back() == "resign") {
		sfen.pop_back();
		return ((sfen.size() - startnum) % 2 != 0) ? 1 : 0;
	}
	else return 0.5;
}

LearnVec Learner::reinforcement_learn(const std::vector<std::string>& cmdtokens) {
	auto sfen = cmdtokens;
	sfen[0] = "position";
	const Kyokumen startKyokumen(sfen);
	double Pwin_result = getWinner(sfen);
	const auto kifu = Move::usiToMoves(sfen);
	std::vector<Move> history;
	int kifuLength = kifu.size();
	std::cout << "L=" << kifuLength << std::endl;
	const double Tb = SearchNode::getTeval();
	//tree初期化
	SearchTree tree;
	tree.makeNewTree(startKyokumen, {});
	LearnVec dw;
	LearnVec td_e;
	double Pwin_prev = 0.5f;
	std::cout << "reinforcement learning " << std::endl;
	search(tree);
	for (int t = 0; t < kifuLength - 1; t++) {
		const auto root = tree.getRoot();
		double Pwin = LearnUtil::EvalToProb(root->eval);
		//TD-清算
		if (t > 0) {
			dw += learning_rate_td * (td_gamma * (1 - Pwin) - Pwin_prev) * td_e;
		}
		LearnVec rootVec;
		double Emin = std::numeric_limits<double>::max();
		for (const auto child : root->children) if (child->eval < Emin) Emin = child->eval;
		double Z = 0;
		for (const auto child : root->children)	Z += std::exp(-(child->eval - Emin) / Tb);
		for (const auto child : root->children) {
			SearchPlayer player = tree.getRootPlayer();
			player.proceed(child->move);
			double pi = std::exp(-(child->eval - Emin) / Tb) / Z;
			if (pi < child_pi_limit) continue;
			const auto childVec = LearnUtil::getGrad(child, player, tree.getRootPlayer().kyokumen.teban(), pi * 1000 + 1);
			const double Pwin_child = LearnUtil::EvalToProb(child->eval);
			rootVec += pi * ((Pwin - Pwin_child) / LearnUtil::pTb + 1) * childVec;
			//bts-pp
			dw += learning_rate_pp * (LearnUtil::EvalToProb(child->getOriginEval()) - Pwin_child) * childVec;
			//pg-leaf
			if (child->move == kifu[t + 1]) {
				dw += -learning_rate_pge * childVec;
			}
			dw += learning_rate_pge * pi * childVec;
		}
		//bts
		dw += -learning_rate_bts * (LearnUtil::EvalToProb(root->eval) - Pwin) * rootVec;
		//reg
		dw += learning_rate_reg * (Pwin_result - Pwin) / LearnUtil::probT * (1 - Pwin) * Pwin * rootVec;
		//TD-準備
		td_e *= td_gamma * td_lambda;
		td_e += rootVec;
		Pwin_prev = Pwin;
		//proceed
		history.push_back(kifu[t]);
		tree.clear();
		tree.makeNewTree(startKyokumen, history);
		std::cout << "t=" << t << std::endl;
		search(tree);
	}
	//TD-清算
	double Pwin = LearnUtil::EvalToProb(tree.getRoot()->eval);
	dw += learning_rate_td * ((Pwin_result * 2 - 1) + td_gamma * (1 - Pwin) - Pwin_prev) * td_e;
	//ノード消去
	tree.clear();
	std::cout << "reinforcement learning finished." << std::endl;
	return dw;
}

void Learner::consecutive_rl(const std::string& sfenfile) {
	std::ifstream ifs(sfenfile);
	while (!ifs.eof()) {
		std::string line;
		std::getline(ifs, line);
		std::cout << line << std::endl;
		line = "rlearn " + line;
		const auto tokens = usi::split(line, ' ');
		if (tokens.size() < 3)continue;
		const auto dw = reinforcement_learn(tokens);
		dw.updateEval();
	}
	Evaluator::save();
	std::cout << "learning finished." << std::endl;
}