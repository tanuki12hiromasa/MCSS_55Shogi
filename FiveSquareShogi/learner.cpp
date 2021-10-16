#include "learner.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <ctime>
#include "usi.h"
#include "simulation.h"

void Kifu::read_from_command() {
	std::string line;
	std::cout << "kifu sfen line > ";
	std::getline(std::cin, line);
	auto tokens = usi::split(line, ' ');
	startpos = Kyokumen(tokens);
	moves = Move::usiToMoves(tokens);
	
	teban:
	std::cout << "learner teban (s/g) > ";
	std::cin >> line;
	if (line == "g") teban = false;
	else if (line == "s") teban = true;
	else goto teban;

	winner:
	std::cout << "game winner (s/g/d) > ";
	std::cin >> line;
	if (line == "d") result = GameResult::Draw;
	else if (line == "g") result = GameResult::GoteWin;
	else if (line == "s") result = GameResult::SenteWin;
	else goto winner;
}

void Learner::execute() {
	Learner learner;
	//LearnVec dw;
	while (learner.enable) {
		std::string usiin;
		std::getline(std::cin, usiin);
		auto tokens = usi::split(usiin, ' ');
		if (tokens.empty()) {
			std::cout << "command ready" << std::endl;
		}
		else if (tokens[0] == "usi") {
			std::cout << "id name TD(l)_pvleaf_Learn" << std::endl;
			coutOption();
			std::cout << "usiok" << std::endl;
		}
		else if (tokens[0] == "setoption") {
			if (tokens[1] == "evalinput") {
				Evaluator::setpath_input(tokens[2]);
				std::cout << "set evalinput" << std::endl;
			}
			else if (tokens[1] == "evaloutput") {
				Evaluator::setpath_output(tokens[2]);
				std::cout << "set evaloutput" << std::endl;
			}
			learner.setOption(tokens);
		}
		else if (tokens[0] == "init") {
			//設定ファイルを読み込む
			//init 設定ファイル.txt
			learner.init(tokens);
			std::cout << "readyok" << std::endl;
		}
		else if (tokens[0] == "genparam") {
			//学習前の初期パラメータを生成する
			//genparam 出力先フォルダ
			Evaluator::genFirstEvalFile(usiin.substr(9));
		}
		else if (tokens[0] == "updateparam") {
			//dw.updateEval();
		}
		else if (tokens[0] == "saveparam") {
			//更新したパラメータを保存
			//saveparam [保存先フォルダ]
			if (tokens.size() >= 2) Evaluator::save(usiin.substr(10));
			else Evaluator::save();
			std::cout << "saveparam done." << std::endl;
		}
		else if (tokens[0] == "printparam") {
			Evaluator::print((tokens.size() > 1) ? std::stoi(tokens[1]) : 0);
		}
		else if (tokens[0] == "loadgrad") {
			//dw.load(tokens.size() > 1 ? tokens[1] : "learninggrad.bin");
		}
		else if (tokens[0] == "savegrad") {
			//dw.save(tokens.size() > 1 ? tokens[1] : "learninggrad.bin");
		}
		else if (tokens[0] == "randomposlearn") {
			if (tokens.size() < 2) { std::cout << "randomposlearn <batchsize> <itrsize>" << std::endl; continue; }
			learner.learn_start_by_randompos(std::stoi(tokens[1]), std::stoi(tokens[2]));
		}
		else if (tokens[0] == "learnbykifu") {
			if (tokens.size() >= 2 && tokens[1] == "options") {
				learner.lbk_options();
				continue;
			}
			learner.learn_by_kifu();
		}
		else if (tokens[0] == "test") {
			learner.test();
		}
		else if (tokens[0] == "quit") {
			break;
		}
	}
}

void Learner::init(const std::vector<std::string>& cmdtokens) {
	BBkiki::init();
	Evaluator::init();
	SearchTemperature::TsDistFuncCode = 0;
	SearchTemperature::TsNodeFuncCode = 0;
}

void Learner::coutOption() {
	using namespace std;
	cout << "option name eval_folderpath type string default ./data/kppt" << endl;
	cout << "option name use_dynamicPieceScore type check default false" << endl;
	//cout << "option name NumOfAgent type spin default 12 min 1 max 128" << endl;
	cout << "option name QSearch_depth type string default 8" << endl;
	cout << "option name T_search type string default 180" << endl;
	cout << "option name T_eval type string default 40" << endl;
	cout << "option name T_depth type string default 100" << endl;
	cout << "option name Es_functionCode type spin default 18 min 0 max 20" << endl;
	cout << "option name Es_funcParam type string default 0.5" << endl;
	//cout << "option name NodeMaxNum type string default 100000000" << endl;
	cout << "option name DrawMoveNum type spin default 320 min 0 max 1000000" << endl;
	
	cout << "option name Sampling_Num type spin default 10000 min 1 max 99999999999" << endl;

	cout << "option name TD_lambda type string default 0.9" << endl;
	cout << "option name TD_gamma type string default 0.95" << endl;
	cout << "option name TD_rate type string default 0.1" << endl;

	cout << "option name Regression_rate type string default 0.1" << endl;
}

void Learner::setOption(const std::vector<std::string>& token) {
	if (token.size() > 4) {
		if (token[2] == "eval_folderpath") {
			//aperyのパラメータファイルの位置を指定する 空白文字がパスにあると駄目なのを何とかしたい?
			Evaluator::setpath_input(token[4]);
			Evaluator::setpath_output(token[4]);
		}
#if defined(USE_KPPT) || defined(USE_KKPPT)
		else if (token[2] == "use_dynamicPieceScore") {
			Evaluator::use_dynamicPieceScore(token[4] == "true");
		}
#endif
		else if (token[2] == "NumOfAgent") {
			agents.setAgentNum(std::stoi(token[4]));
		}
		else if (token[2] == "QSearch_depth") {
			SearchNode::setQSearchDepth(std::stod(token[4]));
		}
		else if (token[2] == "T_search") {
			T_search = std::stod(token[4]);
			SearchTemperature::Ts_max = std::stod(token[4]);
			SearchTemperature::Ts_min = std::stod(token[4]);
		}
		else if (token[2] == "T_eval") {
			SearchTemperature::Te = (std::stod(token[4]));
		}
		else if (token[2] == "T_depth") {
			SearchTemperature::Td = (std::stod(token[4]));
		}
		else if (token[2] == "Es_functionCode") {
			SearchNode::setEsFuncCode(std::stoi(token[4]));
		}
		else if (token[2] == "Es_funcParam") {
			SearchNode::setEsFuncParam(std::stod(token[4]));
		}
		else if (token[2] == "DrawMoveNum") {
			SearchAgent::setDrawMoveNum(std::stoi(token[4]));
		}
		else if (token[2] == "Sampling_Num") {
			samplingnum = std::stoull(token[4]);
		}
		else if (token[2] == "TD_lambda") {
			td_lambda = std::stod(token[4]);
		}
		else if (token[2] == "TD_gamma") {
			td_gamma = std::stod(token[4]);
		}
		else if (token[2] == "TD_rate") {
			learning_rate_td = std::stod(token[4]);
		}
		else if (token[2] == "Regression_rate") {
			learning_rate_reg = std::stod(token[4]);
		}
	}
}

void Learner::search(SearchTree& tree) {
	search(tree, searchtime);
}

void Learner::search(SearchTree& tree, const std::chrono::milliseconds time) {
	agents.setAgentNum(agentnum);
	agents.search(tree, time);
}

void Learner::search(SearchTree& tree, const long long simulate_num) {
	for (int t = 0; t < simulate_num; t++) {
		Simulation s(tree, T_search);
		s.simulate();
	}
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



void Learner::learn_start_by_randompos(const int batch,const int itr) {
	std::cout << "start randompos rootstarp learn\n";
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine{ std::random_device()() };
	const std::string tempinfo = "./learning/.learninfo";
	const std::string tempgrad = "./learning/.learngrad";
	const std::string learningdata_path = "./learning/";
	std::filesystem::create_directories(learningdata_path);
	std::string datestring = LearnUtil::getDateString();
	int counter_itr = 0, counter_batch = 0;
	unsigned long long rootnum = 0, learnedposnum = 0;
	const double learn_rate_0 = 0.01;
	const double learn_rate_r = 0.5;
	LearnVec dw;
	LearnOpt adam;
	//保険セーブが残っているなら再開
	{
		std::ifstream fs(tempinfo);  
		if (fs) {
			std::getline(fs, datestring);
			std::string buff;
			std::getline(fs, buff);
			counter_itr = std::stoi(buff);
			std::getline(fs, buff);
			counter_batch = std::stoi(buff) + 1;
			if (counter_batch >= batch) { counter_itr++; counter_batch = 0; }
			std::getline(fs, buff);
			rootnum = std::stoull(buff);
			std::getline(fs, buff);
			learnedposnum = std::stoull(buff);
		}
	}
	const std::string learnlogdir = "./data/learnlog/" + datestring;
	std::filesystem::create_directories(learnlogdir);
	SearchTree tree;
	for (; counter_itr < itr; counter_itr++) {
		for (; counter_batch < batch; counter_batch++) {
			std::cout << "(" << counter_itr << "," << counter_batch << ")\n";
			LearnMethod* method = new SamplingPGLeaf(dw, 1, 10000, 120);
			
			tree.makeNewTree(usi::split("position startpos", ' '));
			const int movesnum = 6 + 10 * random(engine);
			//ランダム局面を生成
			for (int i = 0; i < movesnum; i++) {
				const auto root = tree.getRoot();
				const auto& player = tree.getRootPlayer();
				const auto moves = MoveGenerator::genMove(root->move, player.kyokumen);
				if (moves.empty()) { counter_batch--; goto delTree; }
				root->addChildren(moves);
				const int randomchild = moves.size() * random(engine);
				tree.proceed(root->children.begin() + randomchild);
			}
			//rootが詰みの場合やり直し
			{
				const auto root = tree.getRoot();
				const auto& player = tree.getRootPlayer();
				const auto moves = MoveGenerator::genMove(root->move, player.kyokumen);
				if (moves.empty()) { counter_batch--; goto delTree; }
			}
			while (tree.getRoot()!=nullptr) {
				//探索
				const auto root = tree.getRoot();
				const auto& rootplayer = tree.getRootPlayer();
				std::cout << root->move.toUSI() << " [" << rootplayer.kyokumen.toSfen() << "] ";
				search(tree);
				rootnum++;

				//ゲームが終了しているか調べる
				const auto pl = LearnUtil::getPrincipalLeaf(root);
				if (pl==nullptr) {
					break;
				}
				if (pl->isRepetition()) {
					method->fin(root, rootplayer, GameResult::Draw);
					break;
				}
				if (pl->isTerminal()) {
					method->fin(root, rootplayer, GameResult::SenteWin);//resultには結果を入れる (要実装)
					break;
				}

				//学習
				method->update(root, rootplayer);
				
				const auto bestchild = LearnUtil::choiceBestChild(root);
				tree.proceed(bestchild);
			}
			learnedposnum += method->getSamplingPosNum();
			//保険セーブ
			delTree:
			{
				dw.save(tempgrad);
				std::ofstream fs(tempinfo);
				if (fs) {
					fs << datestring << "\n" << counter_itr << "\n" << counter_batch << "\n" << rootnum << "\n" << learnedposnum << "\n";
				}
			}
			agents.deleteTree(tree);
			delete method;
			std::cout << "\n";
		}
		//評価関数に勾配を反映
		adam.updateEval(dw);
		Evaluator::save();
		if ((((counter_itr + 1) % std::max(itr / 10, 1)) == 0)) {
			const auto logpath = learnlogdir + "/" + std::to_string(counter_itr);
			std::filesystem::create_directories(logpath);
			Evaluator::save(logpath);
			std::ofstream fs(logpath + "/log.txt");
			if (fs) {
				fs << rootnum << "\n" << learnedposnum << "\n";
			}
		}
		adam.save(learningdata_path);
		dw.save(tempgrad);
		counter_batch = 0;
	}
	std::ofstream fs("./log.txt", std::ios_base::app);
	if (fs) {
		fs << datestring<< " batch:" << batch << " iterate:" << itr <<
			" roots:" << rootnum << " learnedpos:" << learnedposnum << "\n";
	}
	//保険セーブ消去
	std::filesystem::remove(tempinfo);
	std::filesystem::remove(tempgrad);
	std::cout << "learning end." << std::endl;
}

void Learner::lbk_options() {
	std::cout << "learning_name : TD_learn\n";
	std::cout << "batch_size > 10\n";
}

//#define SAMPLING_GRAD
#define PVLEAF_GRAD

#define LEARN_TD_LAMBDA
//#define LEARN_PG
//#define LEARN_REGRESSION

void Learner::learn_by_kifu() {
	int itr = 0, batch = 0, itr_max = 1, bat_max = 1;
	SearchTemperature::TsNodeFuncCode = 0;
	const double T = SearchTemperature::Te;

	LearnVec dw;
	std::string datestr = LearnUtil::getDateString();
	if (!LoadTempInfo(itr, batch, itr_max, bat_max, datestr)) {
		std::cout << "new data\n";
	}
	else {
		std::cout << "restart from temp data\n";
		LoadTempVec(dw);
	}
	while (true) {
		// 棋譜を読み込む
		// 自分の手番と、勝ち負けを確認
		Kifu kifu;
		kifu.read_from_command();
		std::cout << "learn start movenum? > \n";
		long long startnum = 0;
		std::cin >> startnum;
		std::vector<Move> history;
		// 学習の準備
		SearchTree tree;
		tree.makeNewTree(kifu.startpos, {});

		std::cout << "learn start " << (kifu.teban ? "s " : "g ") << (kifu.result == GameResult::SenteWin ? "sw " : "gw ") << startnum << std::endl;

#ifdef LEARN_TD_LAMBDA
		//TD
		double Vt_1 = 0.5; 
		LearnVec e_vec, dw_td;
		const double lambda = td_lambda, gamma = td_gamma, alpha = learning_rate_td;
		bool td_first = true;
#endif
#ifdef LEARN_PG
		LearnVec dw_pg;
		const double pg_r = LearnUtil::ResultToReward(kifu.result, kifu.teban, pg_r_win, pg_r_draw, pg_r_lose);
		const double pg_epsilon = learning_rate_pg;
#endif
#ifdef LEARN_REGRESSION
		//回帰
		LearnVec dw_reg;
		const double reg_r = LearnUtil::ResultToProb(kifu.result, kifu.teban);
		const double reg_epsilon = learning_rate_reg;
#endif

		//一手ずつ進めながら学習
		for (int t = 0; t <= kifu.moves.size(); t++) {
			
			const auto& rootplayer = tree.getRootPlayer();
			//自手番を学習
			if (t >= startnum - 2 && rootplayer.kyokumen.teban() == kifu.teban) {
				
				//終局ならループ離脱
				if (t >= kifu.moves.size() - 1) {
					break;
				}

#ifndef SEARCH_WITH_SAMPLING
				//探索
				std::cout << "s";
				search(tree, samplingnum);
				std::cout << "(d:" << tree.getRoot()->mass.load() << ") ";

				//詰みが発生しているならループ離脱
				if (tree.getRoot()->isMate()) {
					break;
				}

				//勾配ベクトル
				std::cout << "l";
				LearnVec dv;
				double eval;
#ifdef SAMPLING_GRAD
#ifdef LEARN_PG
				double pge_Z, pge_min_eval;
				pge_Z = LearnUtil::getChildrenZ(tree.getRoot(), T, pge_min_eval);
#endif
				{	//サンプリングによる勾配
					for (int _i = 0; _i < samplingnum; _i++) {
						Simulation sim(tree, T);
						sim.init();
						double c = 1;
#ifdef LEARN_PG
						bool pge_first_child = true;
						double pge_P = 0, pge_invc = 1;
#endif
						while (!sim.current_node()->isLeaf()) {
#ifndef SAMPLE_WITH_PROB_EVAL
							const auto node = sim.current_node();
							const double V = node->eval;
							const auto child = sim.select_child();
							if (child->isLeaf())break;
							const double Q = -child->eval;
							c *= (Q - V) / T + 1;
#else
							const auto node = sim.current_node();
							const double V = LearnUtil::EvalToProb(node->eval);
							const auto child = sim.select_child();
							if (child->isLeaf())break;
							const double Q = LearnUtil::EvalToProb(-node->eval);
							c *= (Q - V) / LearnUtil::change_evalTs_to_probTs(SearchTemperature::Te) + 1;
#endif	
							sim.proceed_to_child(child);
							sim.proceed_to_child(sim.select_child());
#ifdef LEARN_PG
							if (pge_first_child) {
								if (child->eval > pge_min_eval) {
									pge_P = -std::exp(-(child->eval - pge_min_eval) / T) / pge_Z;
								}
								else {
									pge_P = 1 - 1.0 / pge_Z;
								}
								pge_invc = 1.0 / c;
								pge_first_child = false;
							}
#endif
						}
						//std::cout << c << " "; //dbg
						dv.addGrad(c, sim.player);
#ifdef LEARN_PG
						dw_pg.addGrad(pge_P * c * pge_invc, sim.player);
#endif
					}
					dv *= 1.0 / samplingnum;
					eval = LearnUtil::EvalToProb(tree.getRoot()->eval);
				}
#endif
#ifdef PVLEAF_GRAD
#if defined(LEARN_TD_LAMBDA) || defined(LEARN_REGRESSION)
				{	//PV-Leafによる勾配
					const SearchNode* node = tree.getRoot();
					SearchPlayer player = rootplayer;
					int depth = 0;
					while (!node->isLeaf()) {
						//二手ずつ進める (相手番が末端になっていればその手前で終了)
						const SearchNode* const child = LearnUtil::choiceBestChild(node);
						if (child->isLeaf()) break;
						player.proceed(child->move);
						node = LearnUtil::choiceBestChild(child);
						player.proceed(node->move);
						depth++;
					}
					dv.addGrad(1, player);
					eval = LearnUtil::EvalToProb(Evaluator::evaluate(player));
					std::cout << "(d:" << depth << ") ";
				}
#endif
#ifdef LEARN_PG
				{//PG-Leaf
					LearnVec e_omega;
					double Z, min_eval;
					Z = LearnUtil::getChildrenZ(tree.getRoot(), T, min_eval);
					const auto& root = tree.getRoot();
					const auto& nextmove = kifu.moves[t];

					for (const auto& child : root->children) {
						SearchPlayer player = tree.getRootPlayer();
						player.proceed(child.move);
						const SearchNode* node = &child;

						while (!node->isLeaf()) {
							const SearchNode* const next = LearnUtil::choiceBestChild(node);
							if (next->isLeaf()) break;
							player.proceed(next->move);
							node = LearnUtil::choiceBestChild(next);
							player.proceed(next->move);
						}

						const double pi = std::exp(-(child.eval - min_eval) / T) / Z;
						if (child.move == nextmove) {	
							e_omega.addGrad(1.0 - pi, player);
						}
						else {
							e_omega.addGrad(-pi, player);
						}
					}

					dw_pg += 1.0 / T * e_omega;
				}
#endif
#endif //#ifdef PVLEAF_GRAD
#else //#ifndef SEARCH_WITH_SAMPLING
				//探索中サンプリング
				std::cout << "s ";
				LearnVec dv;
				double eval;

				std::cout << "l ";
#endif //#ifndef SEARCH_WITH_SAMPLING

				//学習
#ifdef LEARN_TD_LAMBDA
				{	//TD
					if (!td_first) {
						const double delta = gamma * eval - Vt_1;
						dw_td += delta * e_vec;
					}
					else {
						td_first = false;
					}
					Vt_1 = eval;
					e_vec *= gamma * lambda;
					e_vec += dv;
				}
#endif
#ifdef LEARN_REGRESSION
				{	//回帰
					const double Pwin = LearnUtil::EvalToProb(eval);
					dw_reg += (reg_r - Pwin) / LearnUtil::probT * (1 - Pwin) * Pwin * dv;
					std::cout << "reg(" << ((reg_r - Pwin) / LearnUtil::probT * (1 - Pwin) * Pwin) << ") ";
				}
#endif

			}
			//一手進める
			if (t >= kifu.moves.size()) break;
			std::cout << kifu.moves[t].toUSI() << " ";
			history.push_back(kifu.moves[t]);
			tree.set(kifu.startpos, history);
		}
		std::cout << "gameover" << std::endl;
		//終局の学習
#ifdef LEARN_TD_LAMBDA
		{	//TD
			const auto& player = tree.getRootPlayer();
			const double eval = LearnUtil::EvalToProb(Evaluator::evaluate(player));
			const double r = LearnUtil::ResultToReward(kifu.result, kifu.teban, td_r_win, td_r_draw, td_r_lose);
			const double delta = r + gamma * eval - Vt_1;
			dw_td += delta * e_vec;
			dw += alpha * dw_td;
		}
#endif
#ifdef LEARN_PG
		dw += pg_epsilon * pg_r * dw_pg;
#endif
#ifdef LEARN_REGRESSION
		dw += reg_epsilon * dw_reg;
#endif

		//保存 
		batch++;
		if (batch >= bat_max) {
			batch = 0;
			dw.updateEval();
			Evaluator::save();
			itr++;
		}
		SaveTempVec(dw);
		SaveTempInfo(itr, batch, itr_max, bat_max, datestr);
		std::cout << "learning end." << std::endl;
		std::cout << "continue? (y/n)> " << std::endl;
		std::string str;
		std::getline(std::cin, str);
		if (str != "y") break;
	}
}

bool Learner::LoadTempInfo(int& itr, int& batch, int& itr_max, int& bat_max, std::string& datestr, const std::string& dir) {
	std::ifstream fs(dir + "/.learninfo");
	if (fs) {
		std::getline(fs, datestr);
		std::string buff;
		std::getline(fs, buff);
		itr = std::stoi(buff);
		std::getline(fs, buff);
		batch = std::stoi(buff);
		std::getline(fs, buff);
		itr_max = std::stoi(buff);
		std::getline(fs, buff);
		bat_max = std::stoi(buff);
		return true;
	}
	else {
		return false;
	}
}

bool Learner::SaveTempInfo(int itr, int batch, int itr_max, int bat_max, const std::string& datestr, const std::string& dir) {
	std::filesystem::create_directories(dir);
	std::ofstream fs(dir + "/.learninfo");
	if (fs.is_open()) {
		fs << datestr << "\n";
		fs << itr << "\n";
		fs << batch << "\n";
		fs << itr_max << "\n";
		fs << bat_max << "\n";
		return true;
	}
	else {
		return false;
	}
}

bool Learner::LoadTempVec(LearnVec &vec, const int vecnum, const std::string& dir) {
	const std::string filepath = dir + "/.learngrad" + std::to_string(vecnum);
	if (!std::filesystem::exists(filepath)) return false;
	vec.load(filepath);
	return true;
}

bool Learner::SaveTempVec(LearnVec& vec, const int vecnum, const std::string& dir) {
	std::filesystem::create_directories(dir);
	vec.save(dir + "/.learngrad" + std::to_string(vecnum));
	return true;
}


void Learner::test() {
	LearnVec vec;
	Kyokumen k;
	SearchPlayer player(k);
	vec.addGrad(20, player);
	{
		LearnVec _tvec;
		_tvec += vec;
		SaveTempVec(_tvec, 2);
	}
	{
		LearnVec _tvec;
		LoadTempVec(_tvec, 2);
		if (_tvec == vec) {
			std::cout << "save-load is ok\n";
		}
		else {
			_tvec.print(15, true);
			vec.print(15, true);
			_tvec.print(15, false);
			vec.print(15, false);
			std::cout << "save-load is ng\n";
		}
	}
}