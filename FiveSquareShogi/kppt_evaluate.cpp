#include "stdafx.h"
#include "kppt_evaluate.h"

namespace kppt {

	std::string kppt_evaluator::ifolderpath = "data/kppt";
	std::string kppt_evaluator::ofolderpath = "data/learn/kppt";

	void kppt_evaluator::init() {
		kppt_feat::init(ifolderpath);
	}

	void kppt_evaluator::save() {
		//folderが無ければ作る
		kppt_feat::save(ofolderpath);
	}
	
	double kppt_evaluator::evaluate(const SearchPlayer& player) {
		return (double)player.feature.sum.sum(player.kyokumen.teban()) / FVScale;
	}
}