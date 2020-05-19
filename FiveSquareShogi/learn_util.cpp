#include "learn_util.h"

SearchNode* LearnUtil::choiceChildRandom(const SearchNode* const root, const double T, double pip) {
	using dn = std::pair<double, SearchNode*>;
	double CE = std::numeric_limits<double>::max();
	std::vector<dn> evals; evals.reserve(root->children.size());
	for (const auto& child : root->children) {
		if (child->isSearchable()) {
			double eval = child->getEs();
			evals.push_back(std::make_pair(eval, child));
			if (eval < CE) {
				CE = eval;
			}
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
