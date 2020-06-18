#include "learner.h"
#include <iostream>
#include <fstream>
#include "usi.h"

void Learner::execute() {
	Learner learner;
	LearnVec dw;
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
			Evaluator::genFirstEvalFile(usiin.substr(9));
		}
		else if (tokens[0] == "rlearn") {
			//強化学習で勾配ベクトルを求める
			//rlearn sfen startpos moves ... 
			dw += learner.reinforcement_learn(tokens, true);
			dw += learner.reinforcement_learn(tokens, false);
			dw.clamp(1000);
			LearnVec::EvalClamp(30000);
			dw.updateEval();
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
		else if (tokens[0] == "selfplaylearn") {
			//自己対局を行い、その棋譜データで学習を行うことを繰り返す
			//selfplaylearn 回数
			learner.selfplay_learn(tokens);
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
	search(tree, searchtime);
}

void Learner::search(SearchTree& tree, const std::chrono::milliseconds time) {
	using namespace std::chrono_literals;
	constexpr auto checkflame = 50ms;
	std::vector<std::unique_ptr<SearchAgent>> agents;
	for (int i = 0; i < agentnum; i++) {
		agents.push_back(std::unique_ptr<SearchAgent>(new SearchAgent(tree, T_search, i)));
	}
	const auto starttime = std::chrono::system_clock::now();
	const SearchNode* const root = tree.getRoot();
	while (std::abs(root->eval) < SearchNode::getMateScoreBound() && std::chrono::system_clock::now() - starttime < time) {
		std::this_thread::sleep_for(checkflame);
	}
	for (auto& ag : agents) {
		ag->stop();
	}
	for (auto& ag : agents) {
		ag->terminate();
	}
	agents.clear();
}

//どちらが勝ったかを返す関数 1:先手勝ち -1:後手勝ち 0:引き分け
int Learner::getWinner(std::vector<std::string>& sfen) {
	int startnum = 0;
	for (int t = 0; t < sfen.size(); t++) {
		if (sfen[t] == "moves") {
			startnum = t;
			break;
		}
	}
	if (sfen.back() == "resign") {
		sfen.pop_back();
		return ((sfen.size() - startnum) % 2 != 0) ? 1 : -1;
	}
	else return 0;
}

LearnVec Learner::reinforcement_learn(std::vector<std::string> cmdtokens, const bool learnteban) {
	const auto winner = getWinner(cmdtokens);
	cmdtokens[0] = "position";
	const Kyokumen startKyokumen(cmdtokens);
	const auto kifu = Move::usiToMoves(cmdtokens);
	return reinforcement_learn(startKyokumen, kifu, winner, learnteban);
}

LearnVec Learner::reinforcement_learn(const Kyokumen startKyokumen, const std::vector<Move>& kifu, const int winner, const bool learnteban) {
	double Pwin_result = (1.0 + winner) / 2.0;
	std::vector<Move> history;
	if (!learnteban)history.push_back(kifu[0]);
	int kifuLength = kifu.size();
	std::cout << "L=" << (kifuLength - 1) << std::endl;
	const double Tb = SearchNode::getTeval();
	//tree初期化
	SearchTree tree;
	tree.makeNewTree(startKyokumen, {});
	LearnVec dw;
	LearnVec td_e;
	double Pwin_prev = 0.5f;
	std::cout << "reinforcement learning " << std::endl;
	std::cout << "t=" << ((learnteban) ? 0 : 1) << ":";
	search(tree);
	std::cout << " searched.";
	for (int t = (learnteban) ? 0 : 1; t < kifuLength - 1; t = t + 2) {
		const auto root = tree.getRoot();
		double Pwin = LearnUtil::EvalToProb(root->eval);
		//TD-清算
		if (t > 1) {
			dw += learning_rate_td * (td_gamma * Pwin - Pwin_prev) * td_e;
		}
		LearnVec rootVec;
		if (root->children.empty()) {
			rootVec.addGrad(1, tree.getRootPlayer(), true);
		}
		else {
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
				const double Pwin_child = 1 - LearnUtil::EvalToProb(child->eval);
				rootVec += pi * -((Pwin_child - Pwin) / LearnUtil::pTb + 1) * childVec;
				//bts-pp
				dw += learning_rate_pp * (LearnUtil::EvalToProb(child->getOriginEval()) - Pwin_child) * childVec;
				//pg-leaf
				if (child->move == kifu[t + 1]) {
					dw += -learning_rate_pge * childVec;
				}
				dw += learning_rate_pge * pi * childVec;
			}
		}
		//bts
		dw += -learning_rate_bts * (LearnUtil::EvalToProb(root->eval) - Pwin) * rootVec;
		//reg
		dw += learning_rate_reg * (Pwin_result - Pwin) / LearnUtil::probT * (1 - Pwin) * Pwin * rootVec;
		std::cout << " calculated." << std::endl;
		//TD-準備
		td_e *= td_gamma * td_lambda;
		td_e += rootVec;
		Pwin_prev = Pwin;
		//proceed
		history.push_back(kifu[t]);
		history.push_back(kifu[t + 1]);
		tree.clear();
		tree.makeNewTree(startKyokumen, history);
		std::cout << "t=" << (t+1)<<":";
		search(tree);
		std::cout << " searched.";
	}
	//TD-清算
	double Pwin = LearnUtil::EvalToProb(tree.getRoot()->eval);
	dw += learning_rate_td * ((learnteban ? winner : -winner) + td_gamma * Pwin - Pwin_prev) * td_e;
	//ノード消去
	tree.clear();
	std::cout << "reinforcement learning finished." << std::endl;
	return dw;
}

void Learner::consecutive_rl(const std::string& sfenfile) {
	std::ifstream ifs(sfenfile);
	LearnVec dw;
	while (!ifs.eof()) {
		std::string line;
		std::getline(ifs, line);
		std::cout << line << std::endl;
		line = "rlearn " + line;
		auto tokens = usi::split(line, ' ');
		if (tokens.size() < 3)continue;
		dw += reinforcement_learn(tokens, true);
		dw += reinforcement_learn(tokens, false);
		dw.clamp(1000);
		LearnVec::EvalClamp(30000);
		dw.updateEval();
	}
	Evaluator::save();
	std::cout << "learning finished." << std::endl;
}

void Learner::selfplay_learn(const std::vector<std::string>& comdtokens) {
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine{ std::random_device()() };

	const int playnum = std::stoi(comdtokens[1]);
	LearnVec dw;
	for (int pl = 0; pl < playnum; pl++) {
		std::cout << "self-play learning: " << pl << "\n";
		std::vector<Move> history;
		const Kyokumen startpos;
		int winner = 0;
		{
			SearchTree tree;
			tree.makeNewTree(startpos, {});
			while (true) {
				search(tree, searchtime * 2);
				const auto root = tree.getRoot();
				if (root->eval >= SearchNode::getMateScoreBound()) {
					winner = tree.getRootPlayer().kyokumen.teban() ? 1 : -1;
					history.push_back(LearnUtil::choiceBestChild(root)->move);
					break;
				}
				else if (root->eval <= -SearchNode::getMateScoreBound()) {
					winner = tree.getRootPlayer().kyokumen.teban() ? -1 : 1;
					break;
				}
				const auto next = LearnUtil::choiceChildRandom(root, T_selfplay, random(engine));
				tree.proceed(next);
				history.push_back(next->move);
				std::cout << next->move.toUSI() << " ";
			}
			std::cout << "gameend.\n";
		}
		dw += reinforcement_learn(startpos, history, winner, true);
		dw += reinforcement_learn(startpos, history, winner, false);
		dw.clamp(1000);
		LearnVec::EvalClamp(30000);
		dw.updateEval();
		Evaluator::save();
	}
	std::cout << "self-play learning finished" << std::endl;
}
