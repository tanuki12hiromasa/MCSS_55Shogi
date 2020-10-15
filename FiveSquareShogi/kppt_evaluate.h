#pragma once
#include "kppt_feature.h"
#include "player.h"

namespace kppt {
	class kppt_evaluator {
	public:
		static void init();
		static void save();
		static double evaluate(const SearchPlayer& player);
		static double evaluate(const SearchPlayer& player, bool jiteban);

		static void setpath_input(const std::string& path) { ifolderpath = path; }
		static void setpath_output(const std::string& path) { ofolderpath = path; }
		
		static void genFirstEvalFile(const std::string& folderpath);
	private:
		static std::string ifolderpath;
		static std::string ofolderpath;
	};
}