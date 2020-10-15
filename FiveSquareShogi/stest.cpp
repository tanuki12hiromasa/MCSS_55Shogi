#include "stdafx.h"
#include "stest.h"
#include "usi.h"
#include "kppt_learn.h"
#include <iostream>

void coutKPP() {
	for (unsigned k = 0; k < kppt::SquareNum; k++) {
		for (unsigned p1 = 0; p1 < kppt::fe_end; p1++) {
			for (unsigned p2 = 0; p2 < kppt::fe_end; p2++) {
				auto& vec = kppt::KPP[k][p1][p2];
				if (vec[0] != 0 || vec[1] != 0) {
					std::cout << k << " " << p1 << " " << p2 << ": " << vec[0] << " " << vec[1] << "\n";
				}
			}
		}
	}
	for (unsigned sk = 0; sk < kppt::SquareNum; sk++) {
		for (unsigned gk = 0; gk < kppt::SquareNum; gk++) {
			for (unsigned p = 0; p < kppt::fe_end; p++) {
				auto& vec = kppt::KPP[sk][gk][p];
				if (vec[0] != 0 || vec[1] != 0) {
					std::cout << sk << " " << gk << " " << p << ": " << vec[0] << " " << vec[1] << "\n";
				}
			}
		}
	}
}

void ShogiTest::test() {
	using namespace std;
	std::cout << "now initializing..." << std::endl;
	BBkiki::init();
	Evaluator::init();
	std::cout << "initialized." << std::endl;
	coutKPP();
#if 0
	checkGenMove("position startpos");
	checkGenMove("position startpos moves 4e4d");
	checkGenMove("position startpos moves 4e4d 4a5b");
	checkGenMove("position startpos moves 4e4d 4a5b 2e5b 5a5b");
	checkGenMove("position startpos moves 4e4d 4a5b 2e5b 5a5b 1e2e B*2b");
#endif
#if 0
	{
		auto sfen = usi::split("position startpos", ' ');
		Learner learner;
		SearchTree tree;
		tree.makeNewTree(sfen);
		learner.search(tree);
		auto vec = LearnUtil::getGrad(tree.getRoot(), tree.getRootPlayer(), tree.getRootPlayer().kyokumen.teban(), 1000);
		//vec.showLearnVec_kkpt(0.01);
		//vec.showLearnVec_kppt(0.01);
	}
#endif
#if 0
	{
		auto sfen = usi::split("rlearn startpos moves 4e4d 4a2c 5d5c 3a4b 2e3d 2a3b 3d2e 4b5c 3e3d 5c4d 5e4d P*4c 4d3e 5a5e+ resign", ' ');
		Learner learner;
		auto vec = learner.reinforcement_learn(sfen, true);
		vec.showLearnVec_kkpt(1.0);
		vec.showLearnVec_kppt(1.0);
		vec.updateEval();
		//vec.showLearnVec_kkpt(0.6);
		//vec.showLearnVec_kppt(0.6);
		//coutKPP();
	}
# endif
#if 0
	{
		Kyokumen k;
		k.proceed(Move(4, 1, false));
		SearchPlayer p(k);
		auto w = kppt::kppt_paramVector();
		w.addGrad(1, p, k.teban());
		w.showLearnVec_kkpt(0.5);
		w.showLearnVec_kppt(0.5);
	}
#endif
}

void ShogiTest::checkGenMove(const std::string& usipos) {
	const auto tokens = usi::split(usipos, ' ');
	Kyokumen kyokumen(tokens);
	const auto history = Move::usiToMoves(tokens);
	for (const auto m : history) {
		kyokumen.proceed(m);
	}
	Move move = (history.empty()) ? Move() : history.back();
	auto moves = MoveGenerator::genMove(move, kyokumen);
	std::cout << kyokumen.toBanFigure() << std::endl;
	for (const auto m : moves) {
		std::cout << m.toUSI() << " ";
	}
	std::cout << std::endl;
}

