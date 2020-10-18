#include "stdafx.h"
#include "kkppt_evaluate.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace kkppt {

	std::string kkppt_evaluator::ifolderpath = "data/learn/kppt";
	std::string kkppt_evaluator::ofolderpath = "data/learn/kppt";

	void kkppt_evaluator::init() {
		kkppt_feat::init(ifolderpath);
	}

	void kkppt_evaluator::save() {
		//folderが無ければ作る
		std::filesystem::create_directories(ofolderpath);
		kkppt_feat::save(ofolderpath);
	}

	double kkppt_evaluator::evaluate(const SearchPlayer& player) {
		return (double)player.feature.sum.sum(player.kyokumen.teban()) / FVScale;
	}

	double kkppt_evaluator::evaluate(const SearchPlayer& player, bool jiteban) {
		if (jiteban) {
			return (double)player.feature.sum.sum(player.kyokumen.teban()) / FVScale;
		}
		else {
			return -(double)player.feature.sum.sum(!player.kyokumen.teban()) / FVScale;
		}
	}

	void kkppt_evaluator::genFirstEvalFile(const std::string& folderpath) {
		auto* KKPP = new KKPPEvalElement2[SquareNum];
		memset(KKPP, 0, sizeof(KKPPEvalElement2) * (size_t)SquareNum);
		std::ofstream fs(folderpath + "/KKPP.bin", std::ios::binary);
		if (!fs) {
			std::cerr << "error:file(KPP.bin) cannot make" << std::endl;
			return;
		}
		fs.write(reinterpret_cast<char*>(KKPP), sizeof(KKPPEvalElement3));
		delete[] KKPP;
	}
}
