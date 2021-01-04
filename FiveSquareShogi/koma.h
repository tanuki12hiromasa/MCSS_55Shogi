#pragma once
#include <cstdint>
#include <cassert>

namespace koma {
	enum class Koma : std::uint8_t {
		s_Fu, s_Gin, s_Kaku, s_Hi, s_Kin, s_Ou,
		s_nFu, s_nGin, s_nKaku, s_nHi,
		g_Fu, g_Gin, g_Kaku, g_Hi, g_Kin, g_Ou,
		g_nFu, g_nGin, g_nKaku, g_nHi,
		KomaNum,
		None,
		Nari = s_nFu - s_Fu,
		Sengo = g_Fu - s_Fu,
		Min = s_Fu,
		s_Min = s_Fu, s_Max = s_nHi, s_Num = s_Max + 1,
		g_Min = g_Fu, g_Max = g_nHi, g_Num = KomaNum
	};
	enum class Mochigoma : std::uint8_t {
		Fu, Gin, Kaku, Hi, Kin, MochigomaNum, None
	};
	extern Mochigoma KomaToMochi(const Koma koma);
	extern Koma MochiToKoma(const Mochigoma mochi, const bool sengo);

	enum Position : std::uint8_t {
		SQ11, SQ12, SQ13, SQ14, SQ15,
		SQ21, SQ22, SQ23, SQ24, SQ25,
		SQ31, SQ32, SQ33, SQ34, SQ35,
		SQ41, SQ42, SQ43, SQ44, SQ45,
		SQ51, SQ52, SQ53, SQ54, SQ55,

		m_sFu, m_sGin, m_sKaku, m_sHi, m_sKin,
		m_gFu, m_gGin, m_gKaku, m_gHi, m_gKin,

		SQMin = SQ11, SQMax = SQ55,
		SQNum = SQMax + 1,
		SQm_Min = m_sFu, SQm_Max = m_gKin,
		SQm_Num = SQm_Max + 1,
		NullMove
	};

	extern bool isSenteKoma(Koma koma);
	extern bool isGoteKoma(Koma koma);

	extern Koma promote(Koma koma);
	extern Koma dispromote(Koma koma);
	//遠距離移動できる駒かどうか
	inline bool isDashable(Koma koma) {
		switch (koma)
		{
			case Koma::s_Kaku:
			case Koma::s_Hi:
			case Koma::s_nHi:
			case Koma::s_nKaku:
			case Koma::g_Kaku:
			case Koma::g_Hi:
			case Koma::g_nKaku:
			case Koma::g_nHi:
				return true;
			default:
				return false;
		}
	}
	//1マス移動できる駒かどうか(桂もこの部類に入る)
	inline bool isSteppable(Koma koma) {
		switch (koma)
		{
			case Koma::s_Kaku:
			case Koma::s_Hi:
			case Koma::g_Kaku:
			case Koma::g_Hi:
				return false;
			default:
				return true;
		}
	}

	inline Koma sgInv(Koma koma) {
		if (static_cast<std::uint8_t>(koma) < static_cast<std::uint8_t>(Koma::g_Min))
			return static_cast<Koma>(static_cast<std::uint8_t>(koma) + static_cast<std::uint8_t>(Koma::Sengo));
		else
			return static_cast<Koma>(static_cast<std::uint8_t>(koma) - static_cast<std::uint8_t>(Koma::Sengo));
	}

	//x,yはそれぞれ[0,4]
	inline unsigned XYtoPos(const unsigned x, const unsigned y) {
		return x * 5 + y;
	}
	inline int XYtoPos(const int x, const int y) {
		assert(0 <= x && x < 5 && 0 <= y && y < 5);
		return x * 5 + y;
	}
	inline int mirrorX(const int& pos) {
		int x = (int)pos / 5; int y = (int)pos - x * 5;
		return XYtoPos(4 - x, y);
	}
	inline Position mirrorX(const Position& pos) {
		if (pos > Position::SQMax)return pos;
		else return (Position)mirrorX((int)pos);
	}

	inline bool isInside(const int pos) {
		return pos >= Position::SQMin && pos < Position::SQNum;
	}

	//position内の持ち駒をKomaに変換する関数
	extern Koma MposToKoma(const Position pos);
	extern Mochigoma MposToMochi(const Position pos);
	//Komaをposition内の持ち駒に変換する関数(取られるので所属が変わることに注意)
	extern Position KomaToMpos(Koma koma);
	extern Position MochiToMpos(Mochigoma koma, bool sente);
}