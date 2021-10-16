﻿#pragma once
#include "koma.h"
#include <string>
#include <array>
#include <vector>

namespace usi {
	extern std::string komaToUsi(koma::Koma koma);//Koma::Noneは渡してはいけないので注意
	extern std::string mochigomaToUsi(bool teban, koma::Mochigoma koma);
	extern std::string mposToUsi(koma::Position mpos);

	extern const std::array<char, 5> ColumUsi;
	extern const std::array<char, 5> RawUsi;

	extern koma::Mochigoma usiToMochi(const char usi);
	extern koma::Koma usiToKoma(const char usi);
	extern std::string posToUsi(const koma::Position pos);
	extern char tebanToUsi(bool teban);
	extern std::vector<std::string> split(const std::string& str, char splitter);
	extern std::string combine(std::vector<std::string>::const_iterator begin, const std::vector<std::string>::const_iterator end, char splitter);
}