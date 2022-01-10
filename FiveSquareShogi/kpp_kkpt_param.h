#pragma once
#include "kyokumen.h"

namespace kpp_kkpt {
	constexpr int SquareNum = 25;
	inline constexpr int inverse(int pos) { return SquareNum - 1 - pos; }

	constexpr int FVScale = 10;
	constexpr int ScoreZero = 0;
	constexpr int PawnScore = (100 * 9 / 10);
	constexpr int SilverScore = (400 * 9 / 10);
	constexpr int GoldScore = (500 * 9 / 10);
	constexpr int BishopScore = (700 * 9 / 10);
	constexpr int RookScore = (700 * 9 / 10);
	constexpr int ProPawnScore = (600 * 9 / 10);
	constexpr int ProSilverScore = (550 * 9 / 10);
	constexpr int HorseScore = (900 * 9 / 10);
	constexpr int DragonScore = (1000 * 9 / 10);
	constexpr int KingScore = (15000);

	enum EvalIndex :int32_t {
		f_hand_pawn = 0, // 0
		e_hand_pawn = f_hand_pawn + 2,
		f_hand_silver = e_hand_pawn + 2,
		e_hand_silver = f_hand_silver + 2,
		f_hand_gold = e_hand_silver + 2,
		e_hand_gold = f_hand_gold + 2,
		f_hand_bishop = e_hand_gold + 2,
		e_hand_bishop = f_hand_bishop + 2,
		f_hand_rook = e_hand_bishop + 2,
		e_hand_rook = f_hand_rook + 2,
		fe_hand_end = e_hand_rook + 2,

		f_pawn = fe_hand_end,
		e_pawn = f_pawn + 25,
		f_silver = e_pawn + 25,
		e_silver = f_silver + 25,
		f_gold = e_silver + 25,
		e_gold = f_gold + 25,
		f_bishop = e_gold + 25,
		e_bishop = f_bishop + 25,
		f_horse = e_bishop + 25,
		e_horse = f_horse + 25,
		f_rook = e_horse + 25,
		e_rook = f_rook + 25,
		f_dragon = e_rook + 25,
		e_dragon = f_dragon + 25,
		fe_end = e_dragon + 25
	};

	inline constexpr EvalIndex operator+(const EvalIndex lhs, const int32_t rhs) { return static_cast<EvalIndex>(static_cast<int32_t>(lhs) + rhs); }

	const std::array<EvalIndex, static_cast<size_t>(koma::Koma::KomaNum)> komaToIndexArr = {
		f_pawn, f_silver, f_bishop, f_rook, f_gold, fe_end,
		f_gold, f_gold, f_horse,  f_dragon,
		e_pawn, e_silver, e_bishop, e_rook, e_gold, fe_end,
		e_gold, e_gold, e_horse,  e_dragon
	};
	inline EvalIndex komaToIndex(koma::Koma k) { return komaToIndexArr[static_cast<size_t>(k)]; }
	const std::array<EvalIndex, static_cast<size_t>(koma::Mochigoma::MochigomaNum)> sMochiToIndexArr = {
		f_hand_pawn,f_hand_silver,f_hand_bishop,f_hand_rook,f_hand_gold
	};
	const std::array<EvalIndex, static_cast<size_t>(koma::Mochigoma::MochigomaNum)> gMochiToIndexArr = {
		e_hand_pawn,e_hand_silver,e_hand_bishop,e_hand_rook,e_hand_gold
	};
	inline EvalIndex mochiToIndex(koma::Mochigoma k, bool teban) { return teban ? sMochiToIndexArr[static_cast<size_t>(k)] : gMochiToIndexArr[static_cast<size_t>(k)]; }
	constexpr EvalIndex mirror(const EvalIndex index) {
		if (index < fe_hand_end)return index;
		int pos = (index - fe_hand_end) % 25;
		return static_cast<EvalIndex>((int)index - pos + koma::mirrorX(pos));
	}

}