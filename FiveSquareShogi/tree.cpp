#include "stdafx.h"
#include "tree.h"
#include "move_gen.h"
#include <queue>
#include <fstream>

SearchTree::SearchTree()
	:rootPlayer(), startKyokumen()
{
	leave_branchNode = false;
	enable_deleteTrees = true;
	history.push_back(new SearchNode(Move(koma::Position::NullMove, koma::Position::NullMove, false)));
	nodecount = 1;
}

SearchTree::~SearchTree() {
	enable_deleteTrees = false;
	cv_deleteTrees.notify_one();
	auto root = getGameRoot();
	delete root;
}

void SearchTree::set(const std::vector<std::string>& usitokens) {
	const auto moves = Move::usiToMoves(usitokens);
	set(Kyokumen(usitokens), moves);
}

void SearchTree::set(const Kyokumen& startpos, const std::vector<Move>& usihis) {
	std::vector<SearchNode::Children*> deletedRoots;
	if (!history.empty() && (history.size() <= usihis.size()) && startKyokumen == startpos) {
		int i;
		for (i = 0; i < history.size() - 1; i++) {
			if (history[i + 1ull]->move != usihis[i]) {
				//与えられた棋譜が内部の棋譜と一致しないので探索木を作り直す
				auto root = getGameRoot();
				deletedRoots.push_back(root->purge());
				delete root;
				deleteTrees(deletedRoots);
				makeNewTree(startpos, usihis);
				return;
			}
		}
		for (; i < usihis.size(); i++) {
			SearchNode* parent = getRoot();
			const Move nextmove = usihis[i];
			SearchNode* nextNode = nullptr;
			if (parent->isTerminal()) {
				std::copy(history.rbegin(), history.rend(), deletedNodes.begin());
				deleteTrees(std::move(deletedNodes), true);
				makeNewTree(startpos, usihis);
				return;
			}
			if (parent->isLeaf()) {
				const auto moves = MoveGenerator::genAllMove(parent->move, rootPlayer.kyokumen);
				parent->addChildren(moves);
				nodecount += moves.size();
			}
			for (auto& child : parent->children) {
				//子ノードの中から棋譜での次の手を探す
				if (child.move == nextmove) {
					nextNode = &child;
				}
				//棋譜から逸れる手の探索木は削除リストに入れる
				else {
					deletedNodes.push_back(&child);
				}
			}
			if (nextNode == nullptr) {
				nextNode = new SearchNode(nextmove);
				nodecount++;
			}
			proceed(nextNode);
		}
		deleteTrees(deletedRoots);
		return;
	}
	//与えられた棋譜が内部の棋譜と一致しないので探索木を作り直す
	auto root = getGameRoot();
	deletedRoots.push_back(root->purge());
	delete root;
	deleteTrees(deletedRoots);
	makeNewTree(startpos, usihis);
	
}

void SearchTree::makeNewTree(const std::vector<std::string>& usitokens) {
	const auto moves = Move::usiToMoves(usitokens);
	makeNewTree(Kyokumen(usitokens), moves);
}

void SearchTree::makeNewTree(const Kyokumen& startpos, const std::vector<Move>& usihis) {
	history.clear();
	startKyokumen = startpos;
	nodecount = 1;
	history.push_back(new SearchNode(Move(koma::Position::NullMove, koma::Position::NullMove, false)));
	rootPlayer = SearchPlayer(startKyokumen);
	for (const auto& usimove : usihis) {
		SearchNode* rootNode = getRoot();
		const auto moves = MoveGenerator::genAllMove(rootNode->move, rootPlayer.kyokumen);
		rootNode->addChildren(moves);
		nodecount += moves.size();
		SearchNode* next = nullptr;
		for (auto& child : rootNode->children) {
			assert(child != nullptr);
			if (child.move == usimove) {
				next = &child;
				break;
			}
		}
		if (next == nullptr) {
			next = new SearchNode(usimove);
			nodecount++;
		}
		proceed(next);
	}
}

SearchNode* SearchTree::getBestMove()const {
	SearchNode* const rootNode = getRoot();
	return rootNode->getBestChild();
}

std::vector<SearchNode*> SearchTree::getPV()const {
	SearchNode* node = getRoot();
	std::vector<SearchNode*> pv = { node };
	while (!node->isLeaf()) {
		node = node->getBestChild();
		if (node == nullptr)break;
		pv.push_back(node);
	}
	return pv;
}

void SearchTree::proceed(SearchNode* node) {
	historymap.emplace(rootPlayer.kyokumen.getHash(), std::make_pair(rootPlayer.kyokumen.getBammen(), history.size() - 1));
	rootPlayer.kyokumen.proceed(node->move);
	rootPlayer.feature.set(rootPlayer.kyokumen);
	history.push_back(node);
}

void SearchTree::deleteBranch(SearchNode* base, const std::vector<SearchNode*>& savedNodes) {
	if (leave_branchNode) return;
	for (auto saved : savedNodes) {
		for (auto node : base->children) {
			if (node != saved) {
				const size_t delnum = node->deleteTree();
				nodecount -= delnum;
			}
		}
		base = saved;
	}
}

void SearchTree::deleteTree(SearchNode* const root) {
	const size_t delnum = root->deleteTree();
	nodecount -= delnum;
	delete(root);
	nodecount--;
}

void SearchTree::clear() {
	SearchNode* const root = history.front();
	root->deleteTree();
	history.clear();
	historymap.clear();
	evaluationcount = 0;
	nodecount = 0;
}
#pragma optimize("",on)

std::pair<unsigned, SearchNode*> SearchTree::findRepetition(const Kyokumen& kyokumen)const {
	auto range = historymap.equal_range(kyokumen.getHash());
	unsigned num = 0;
	size_t latest = 0;
	SearchNode* latestNode = nullptr;
	for (auto it = range.first; it != range.second; it++) {
		if (kyokumen.teban() == ((*it).second.second % 2 == 0) && (*it).second.first == kyokumen.getBammen()) {
			num++;
			if ((*it).second.second > latest) {
				latest = (*it).second.second;
				latestNode = history[latest];
			}
		}
	}
	return std::make_pair(num, latestNode);
}

void SearchTree::foutTree()const {
	std::ofstream fs("treelog.txt");
	std::queue<SearchNode*> nq;
	fs << rootPlayer.kyokumen.toSfen() << "\n";
	nq.push(history.front());
	size_t index = 0;
	size_t c_index = 1;
	while (!nq.empty()) {
		const SearchNode* const node = nq.front();
		nq.pop();
		int st = static_cast<int>(node->status.load());
		fs << index << ", " << st << ", " << node->move.toUSI() << ", " << node->eval << ", " << node->mass << ", [";
		for (auto& c : node->children) {
			nq.push(&c);
			fs << c_index << ",";
			c_index++;
		}
		fs << "]\n";
		index++;
	}
	fs.close();
}

void SearchTree::deleteTrees(const std::vector<SearchNode::Children*>& roots) {
	{
		std::lock_guard<std::mutex> lock(mtx_deleteTrees);
		for (auto root : roots) {
			roots_deleteTrees.push(root);
		}
	}
	cv_deleteTrees.notify_one();
}

void SearchTree::deleteTreesLoop() {
	std::unique_lock<std::mutex> lock(mtx_deleteTrees);
	while (enable_deleteTrees) {
		cv_deleteTrees.wait(lock, [this] {return !enable_deleteTrees || !roots_deleteTrees.empty(); });
		while (true) {
			lock.lock();
			if (!roots_deleteTrees.empty()) {
				auto root = roots_deleteTrees.front();
				roots_deleteTrees.pop();
				lock.unlock();
				delete root;
			}
			else {
				lock.unlock();
				break;
			}
		}
	}

}