#include "stdafx.h"
#include "stest.h"
#include "usi.h"
#include <iostream>

void ShogiTest::test() {
	using namespace std;
	std::cout << "now initializing..." << std::endl;
	BBkiki::init();
	Evaluator::init();
	std::cout << "initialized." << std::endl;
	checkGenMove("position startpos");
	checkGenMove("position startpos moves 4e4d");
	checkGenMove("position startpos moves 4e4d 4a5b");
	checkGenMove("position startpos moves 4e4d 4a5b 2e5b 5a5b");
	checkGenMove("position startpos moves 4e4d 4a5b 2e5b 5a5b 1e2e B*2b");
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