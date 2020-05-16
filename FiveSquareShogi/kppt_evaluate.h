#pragma once
#include "kppt_feature.h"
#include "player.h"

namespace kppt {
	class kppt_evaluator {
	public:
		static void init();
		static void save();
		static double evaluate(const SearchPlayer& player);

		static void setpath_input(const std::string& path) { ifolderpath = path; }
		static void setpath_output(const std::string& path) { ofolderpath = path; }
	private:
		static std::string ifolderpath;
		static std::string ofolderpath;
	};
}