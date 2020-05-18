#include "stdafx.h"
#include "bb_kiki.h"
#include "koma_motion.h"
#include <intrin.h>

BBkiki::Barray25 BBkiki::sFu;
BBkiki::Barray25 BBkiki::sGin;
BBkiki::Barray25 BBkiki::sKin;
BBkiki::Barray25 BBkiki::gFu;
BBkiki::Barray25 BBkiki::gGin;
BBkiki::Barray25 BBkiki::gKin;
BBkiki::Barray25 BBkiki::UmaStep;
BBkiki::Barray25 BBkiki::RyuStep;
BBkiki::Barray25 BBkiki::Ou;
std::array<std::uint32_t, 25> BBkiki::KakuPositiveInclinationMask;
std::array<std::uint32_t, 25> BBkiki::KakuNegativeInclinationMask;
std::array<std::uint32_t, 5> BBkiki::HiVerticalMask;
std::array<std::uint32_t, 5> BBkiki::HiHorizontalMask;
std::array<size_t, 25> BBkiki::KPIndex;
std::array<size_t, 25> BBkiki::KNIndex;
std::array<size_t, 5> BBkiki::HIndex;
std::array<Bitboard, 68> BBkiki::KakuDashPositive;
std::array<Bitboard, 68> BBkiki::KakuDashNegative;
std::array<std::uint32_t, 28> BBkiki::HiDashVertical;
std::array<std::uint32_t, 28> BBkiki::HiDashHorizontal;

void BBkiki::init() {
	genData();
}

const Bitboard BBkiki::getHiDashKiki(const Bitboard& allbb, const unsigned komapos) {
	Bitboard kiki;
	unsigned x = komapos / 5u, y = komapos % 5u;
	//縦
	{//1筋
		size_t p = _pext_u32(allbb._p, HiVerticalMask[y] << (x * 5u));//対象の筋までmaskを移動
		kiki |= Bitboard(HiDashVertical[HIndex[y] + p] << (x * 5u));//結果を対象の筋にシフトしBBを返す
	}
	//横
	{
		size_t p = _pext_u32(allbb._p, HiHorizontalMask[x] << y);//対象の段までmaskを移動させてpext
		kiki |= Bitboard(HiDashHorizontal[HIndex[x] + p] << y);
	}
	return kiki;
}

const Bitboard BBkiki::getKakuDashKiki(const Bitboard& allbb, const unsigned komapos) {
	Bitboard kiki;
	//posi
	{
		size_t p = _pext_u32(allbb._p, KakuPositiveInclinationMask[komapos]);
		kiki |= Bitboard(KakuDashPositive[KPIndex[komapos] + p]);
	}
	//nega
	{
		size_t p = _pext_u32(allbb._p, KakuNegativeInclinationMask[komapos]);
		kiki |= Bitboard(KakuDashNegative[KNIndex[komapos] + p]);
	}
	return kiki;
}

void BBkiki::genData() {
	genStepTable();
	genMask();
	genIndex();
	genDashTable();
}

#pragma optimize("",off)
void BBkiki::genMask() {
	//端の情報は利きの判定には不要なので、bitboardの真ん中部分の中3段分のみを利用すればよい
	//飛車mask
	{	//縦 
		for (unsigned long y = 0; y < 5; y++) {
			Bitboard u(0b01110UL);
			u.reset(y);
			HiVerticalMask[y] = u._p;
		}
		//横
		for (unsigned long x = 0; x < 5; x++) {
			Bitboard u(0b0000000001000010000100000UL);
			u.reset(x * 5u);
			HiHorizontalMask[x] = u._p;
		}
	}
	//角mask
	{
		//傾き正
		for (unsigned long i = 0; i < 25; i++) {
			Bitboard u;
			for (long x = (i / 5) - 1, y = (i % 5) - 1; 1 <= x && x <= 3 && 1 <= y && y <= 3; x--, y--) {
				u.set(x * 5u + y);
			}
			for (long x = (i / 5) + 1, y = (i % 5) + 1; 1 <= x && x <= 3 && 1 <= y && y <= 3; x++, y++) {
				u.set(x * 5u + y);
			}
			KakuPositiveInclinationMask[i] = u._p;
		}
		//傾き負
		for (unsigned long i = 0; i < 25; i++) {
			Bitboard u;
			for (long x = (i / 5) + 1, y = (i % 5) - 1; 1 <= x && x <= 3 && 1 <= y && y <= 3; x++, y--) {
				u.set(x * 5u + y);
			}
			for (long x = (i / 5) - 1, y = (i % 5) + 1; 1 <= x && x <= 3 && 1 <= y && y <= 3; x--, y++) {
				u.set(x * 5u + y);
			}
			KakuNegativeInclinationMask[i] = u._p;
		}
	}
}

#pragma optimize("",on)
void BBkiki::genIndex() {
	unsigned sum = 0, count = 0;
	for (unsigned i = 0; i < 25; i++) {
		KPIndex[i] = sum;
		count = __popcnt(KakuPositiveInclinationMask[i]);
		sum += 1 << count;
	}
	sum = 0, count = 0;
	for (unsigned i = 0; i < 25; i++) {
		KNIndex[i] = sum;
		count = __popcnt(KakuNegativeInclinationMask[i]);
		sum += 1 << count;
	}
	sum = 0, count = 0;
	for (unsigned i = 0; i < 5; i++) {
		HIndex[i] = sum;
		count = __popcnt(HiVerticalMask[i]);
		sum += 1 << count;
	}
}

inline Bitboard tobikiki(koma::Vector2 from, koma::Vector2 step, Bitboard allBB) {
	Bitboard bb;
	for (koma::Vector2 v = from + step; v.isInside(); v += step) {
		bb.set(v.tou());
		if (allBB.test(v.tou())) break;
	}
	return bb;
}

void BBkiki::genDashTable() {
	using namespace koma;
	//HiVirtical
	for (unsigned y = 0; y < 5; y++) {
		std::uint32_t count = 1ULL << __popcnt(HiVerticalMask[y]);
		for (std::uint32_t k = 0; k < count; k++) {
			Bitboard ban(_pdep_u32(k, HiVerticalMask[y]));
			Bitboard kiki = tobikiki(Vector2(0, y), motion::Up, ban) | tobikiki(Vector2(0, y), motion::Down, ban);
			HiDashVertical[HIndex[y] + k] = kiki._p;
		}
	}
	//HiHorizontal
	for (unsigned x = 0; x < 5; x++) {
		std::uint32_t count = 1ULL << __popcnt(HiHorizontalMask[x]);
		for (std::uint32_t k = 0; k < count; k++) {
			Bitboard ban(_pdep_u32(k, HiHorizontalMask[x])); //横の利きは1段で計算する
			Bitboard kiki = tobikiki(Vector2(x, 0), motion::Right, ban) | tobikiki(Vector2(x, 0), motion::Left, ban);
			HiDashHorizontal[HIndex[x] + k] = kiki._p;
		}
	}
	//KakuPosi
	for (unsigned pos = 0; pos < 25; pos++) {
		unsigned x = pos / 5, y = pos % 5;
		std::uint32_t count = 1ULL << __popcnt(KakuPositiveInclinationMask[pos]);
		for (std::uint32_t k = 0; k < count; k++) {
			Bitboard ban(_pdep_u32(k, KakuPositiveInclinationMask[pos]));
			Bitboard kiki = tobikiki(Vector2(x, y), motion::UpRight, ban) | tobikiki(Vector2(x, y), motion::DownLeft, ban);
			KakuDashPositive[KPIndex[pos] + k] = kiki;
		}
	}
	//KakuNega
	for (unsigned pos = 0; pos < 25; pos++) {
		unsigned x = pos / 5, y = pos % 5;
		std::uint32_t count = 1ULL << __popcnt(KakuNegativeInclinationMask[pos]);
		for (std::uint32_t k = 0; k < count; k++) {
			Bitboard ban(_pdep_u32(k, KakuNegativeInclinationMask[pos]));
			Bitboard kiki = tobikiki(Vector2(x, y), motion::UpLeft, ban) | tobikiki(Vector2(x, y), motion::DownRight, ban);
			KakuDashNegative[KNIndex[pos] + k] = kiki;
		}
	}
}

inline void ippokiki(BBkiki::Barray25& lbs, const std::vector<koma::Vector2>& steps) {
	using namespace koma;
	for (unsigned pos = 0; pos < 25; pos++) {
		Bitboard kiki;
		Vector2 from(pos);
		for (Vector2 step : steps) {
			auto v = from + step;
			if (v.isInside()) {
				kiki.set(v.tou());
			}
		}
		lbs[pos] = kiki;
	}
}

void BBkiki::genStepTable() {
	using namespace koma;
	ippokiki(sFu, { motion::Up });
	ippokiki(sGin, { motion::Up,motion::UpLeft,motion::UpRight,motion::DownLeft,motion::DownRight });
	ippokiki(sKin, { motion::Up,motion::UpLeft,motion::UpRight,motion::Left,motion::Right,motion::Down });
	ippokiki(gFu, { motion::Down });
	ippokiki(gGin, { motion::Down,motion::UpLeft,motion::UpRight,motion::DownLeft,motion::DownRight });
	ippokiki(gKin, { motion::Up,motion::DownLeft,motion::DownRight,motion::Left,motion::Right,motion::Down });
	ippokiki(UmaStep, { motion::Up,motion::Left,motion::Right,motion::Down });
	ippokiki(RyuStep, { motion::UpLeft,motion::UpRight,motion::DownLeft,motion::DownRight });
	ippokiki(Ou, { motion::Up,motion::Left,motion::Right,motion::Down, motion::UpLeft,motion::UpRight,motion::DownLeft,motion::DownRight });
}