#include "stdafx.h"
#include "learn_method.h"
#include <iostream>
#include <set>
#include <unordered_map>

void RootStrap::update(SearchNode* const root, const SearchPlayer& rootplayer) {
	const double H = Evaluator::evaluate(rootplayer);
	const double sigH = LearnUtil::EvalToProb(H);
	const double c = rate * -(sigH - LearnUtil::EvalToProb(root->eval))
		* LearnUtil::probT * sigH * (1 - sigH);
	dw.addGrad(c, rootplayer);
	samplingPosNum += 1;
	std::cout << H << " -> " << root->eval << "\n";
}

void SamplingBTS::update(SearchNode* const root, const SearchPlayer& rootplayer) {
	std::set<SearchNode*> learned_nodes;
	for (size_t t = 0; t < sampling_num; t++) {
		auto player = rootplayer;
		auto node = root;
		double realization = 1.0;
		while (node) {
			if (!learned_nodes.contains(node)) {
				learned_nodes.insert(node);

				const double H = Evaluator::evaluate(rootplayer);
				const double sigH = LearnUtil::EvalToProb(H);
				const double c = rate * std::sqrt(realization) * -(sigH - LearnUtil::EvalToProb(root->eval))
					* LearnUtil::probT * sigH * (1 - sigH);
				dw.addGrad(c, rootplayer);
			}
			if (node->children.empty() || node->isLeaf()) {	break;}
			const auto next = LearnUtil::choicePolicyRandomChild(node, T, random(engine));
			if (next == nullptr) { break;}
			player.proceed(next->move);
			realization *= LearnUtil::BackProb(*node, *next, T);
			node = next;
		}
	}
	std::cout << "learned kyokumen:" << learned_nodes.size() << "\n";
	samplingPosNum += learned_nodes.size();
}

void SamplingBTS::fin(SearchNode* const root, const SearchPlayer& player, GameResult result) {
	switch (result)
	{
		case GameResult::SenteWin:
		case GameResult::GoteWin:
			if (!root->children.empty()) update(root, player);
			break;
		case GameResult::Draw:
		default:
			//引き分けは学習しない
			break;
	}
}

std::vector<Move> unzipHistory(std::string history) {
	std::vector<Move> moves;
	for (int i = 0; i < history.size() - 1; i += 2) {
		const std::uint16_t b = ((history[i] << 8)& 0xFF00U) | (history[i + 1ULL] & 0xFFU);
		moves.emplace_back(Move(b));
	}
	return moves;
}

void SamplingPGLeaf::update(SearchNode* const root, const SearchPlayer& rootplayer) {
	std::unordered_map<std::string, std::size_t> visited;
	LearnVec testvec;
	double cmin = std::numeric_limits<double>::max();
	double Z = 0;
	for (const auto& child : root->children) {
		if (child.eval < cmin)cmin = child.eval;
	}
	for (const auto& child : root->children) {
		Z += std::exp(-(child.eval - cmin) / T);
	}
	for (const auto& child : root->children) {
		visited.clear();
		const double pi = std::exp(-(child.eval - cmin) / T) / Z;
		std::cout << pi << "\n";
		const std::size_t p_sanpling_num = pi * sampling_num;
		if (p_sanpling_num == 0)continue;
		std::vector<Move> check1;
		for (std::size_t i = 0; i < p_sanpling_num; i++) {
			auto player = rootplayer;
			auto node = root;
			std::string history;
			while (node) {
				if (node->children.empty() || node->isLeaf()) {
					visited[history] += 1;
					const auto check2 = unzipHistory(history);
					break; 
				}
				const auto next = LearnUtil::choicePolicyRandomChild(node, T, random(engine));
				if (next == nullptr) { break; }
				player.proceed(next->move);
				const auto binary = next->move.binary();
				history += (char)((binary >> 8) & 0xFFU);
				history += (char)(binary & 0xFFU);
				check1.push_back(next->move);
				node = next;
			}
		}
		LearnVec gradQ;
		for (const auto& v : visited) {
			auto player = rootplayer;
			const auto history = unzipHistory(v.first);
			for (const auto& move : history) {
				player.proceed(move);
			}
			const double c = (double)v.second / (p_sanpling_num);
			gradQ.addGrad(c, player);
		}
		if (child.eval == cmin) {
			testvec += (1 - pi) * gradQ;
		}
		else {
			testvec += -pi * gradQ;
		}
		//gradQ.print(0.01, 0);
		//gradQ.print(0.01, 1);
	}
	//testvec.print(0.01, 0);
	//testvec.print(0.01, 1);
	dw += rate * testvec;


}