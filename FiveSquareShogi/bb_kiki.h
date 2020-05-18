#pragma once

#include "bitboard.h"
#include "koma.h"
#include <array>

class BBkiki {
public:
	static void init();
	static const Bitboard getStepKiki(const koma::Koma koma, const unsigned komapos);
	static const Bitboard getDashKiki(const Bitboard& allbb, const koma::Koma koma, const unsigned komapos);
	static const Bitboard getKiki(const Bitboard& allbb, const koma::Koma koma, const unsigned komapos);
private:
	static const Bitboard getHiDashKiki(const Bitboard& allbb, const unsigned komapos);
	static const Bitboard getKakuDashKiki(const Bitboard& allbb, const unsigned komapos);

public:
	using Barray25 = std::array<Bitboard, 25>;
private:
	static Barray25 sFu;
	static Barray25 sGin;
	static Barray25 sKin;
	static Barray25 gFu;
	static Barray25 gGin;
	static Barray25 gKin;
	static Barray25 UmaStep;
	static Barray25 RyuStep;
	static Barray25 Ou;

	static std::array<std::uint32_t, 25> KakuPositiveInclinationMask;//角の斜めは、傾きの正負で名前を付けている
	static std::array<std::uint32_t, 25> KakuNegativeInclinationMask;
	static std::array<std::uint32_t, 5> HiVerticalMask;
	static std::array<std::uint32_t, 5> HiHorizontalMask;

	static std::array<size_t, 25> KPIndex;
	static std::array<size_t, 25> KNIndex;
	static std::array<size_t, 5> HIndex;//飛車はのindexは一つで十分

	static std::array<Bitboard, 68> KakuDashPositive;//初期位置で角が睨み合う方の向き
	static std::array<Bitboard, 68> KakuDashNegative;
	static std::array<std::uint32_t, 28> HiDashVertical;//縦
	static std::array<std::uint32_t, 28> HiDashHorizontal;//横

private:
	static void genData();//利きテーブルを生成する(事前に実行して準備しておく)
	static void genMask();
	static void genIndex();
	static void genDashTable();
	static void genStepTable();
};


inline const Bitboard BBkiki::getStepKiki(const koma::Koma fromkoma, const unsigned komapos) {
	using namespace koma;
	switch (fromkoma)
	{
		case Koma::s_Fu:
			return Bitboard(sFu[komapos]);
		case Koma::s_Gin:
			return Bitboard(sGin[komapos]);
		case Koma::s_Kin:
		case Koma::s_nFu:
		case Koma::s_nGin:
			return Bitboard(sKin[komapos]);
		case Koma::g_Fu:
			return Bitboard(gFu[komapos]);
		case Koma::g_Gin:
			return Bitboard(gGin[komapos]);
		case Koma::g_Kin:
		case Koma::g_nFu:
		case Koma::g_nGin:
			return Bitboard(gKin[komapos]);
		case Koma::s_nKaku:
		case Koma::g_nKaku:
			return Bitboard(UmaStep[komapos]);
		case Koma::s_nHi:
		case Koma::g_nHi:
			return Bitboard(RyuStep[komapos]);
		case Koma::s_Ou:
		case Koma::g_Ou:
			return Bitboard(Ou[komapos]);
		default:
			assert(0);
			return bbmask::AllZero;
	}
}

inline const Bitboard BBkiki::getDashKiki(const Bitboard& allBB, const koma::Koma dkoma, const unsigned komapos) {
	using namespace koma;
	switch (dkoma) {
		case Koma::s_Hi:
		case Koma::g_Hi:
		case Koma::s_nHi:
		case Koma::g_nHi:
			return getHiDashKiki(allBB, komapos);
		case Koma::s_Kaku:
		case Koma::g_Kaku:
		case Koma::s_nKaku:
		case Koma::g_nKaku:
			return getKakuDashKiki(allBB, komapos);
		default:
			assert(0);
			return bbmask::AllZero;
	}
}


inline const Bitboard BBkiki::getKiki(const Bitboard& allBB, const koma::Koma koma, const unsigned komapos) {
	using namespace koma;
	switch (koma)
	{
		case Koma::s_Fu:
			return Bitboard(sFu[komapos]);
		case Koma::s_Gin:
			return Bitboard(sGin[komapos]);
		case Koma::s_Kin:
		case Koma::s_nFu:
		case Koma::s_nGin:
			return Bitboard(sKin[komapos]);
		case Koma::g_Fu:
			return Bitboard(gFu[komapos]);
		case Koma::g_Gin:
			return Bitboard(gGin[komapos]);
		case Koma::g_Kin:
		case Koma::g_nFu:
		case Koma::g_nGin:
			return Bitboard(gKin[komapos]);
		case Koma::s_Ou:
		case Koma::g_Ou:
			return Bitboard(Ou[komapos]);
		case Koma::s_Hi:
		case Koma::g_Hi:
			return getHiDashKiki(allBB, komapos);
		case Koma::s_nHi:
		case Koma::g_nHi:
			return getHiDashKiki(allBB, komapos) | Bitboard(RyuStep[komapos]);
		case Koma::s_Kaku:
		case Koma::g_Kaku:
			return getKakuDashKiki(allBB, komapos);
		case Koma::s_nKaku:
		case Koma::g_nKaku:
			return getKakuDashKiki(allBB, komapos) | Bitboard(UmaStep[komapos]);
		default:
			assert(0);
			return bbmask::AllZero;
	}
}