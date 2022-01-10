#pragma once
#include "kpp_feature.h"
#include "player.h"

namespace kpp {
	using SearchPlayer = Player<kpp_feat, EvalSum>;
	class kpp_evaluator {
	public:
		static void init();
		static void save();
		static void save(const std::string& path);
		static double evaluate(const SearchPlayer& player);
		static double evaluate(const SearchPlayer& player, bool jiteban);

		static void setpath_input(const std::string& path) { ifolderpath = path; }
		static void setpath_output(const std::string& path) { ofolderpath = path; }
		static const std::string& getpath_input() { return ifolderpath; }
		static const std::string& getpath_output() { return ofolderpath; }
		static void use_dynamicPieceScore(const bool b) { dynamicPieceScore = b; }
		
		static void genFirstEvalFile(const std::string& folderpath);

		static void print(int iskpp);
		static std::string name() { return "KPP"; }
	private:
		static std::string ifolderpath;
		static std::string ofolderpath;
	};
}