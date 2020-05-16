#pragma once
#include "bitboard.h"
#include "move.h"
#include <vector>

using Bammen = std::array<std::uint8_t, 35>;

class Kyokumen {
	friend class MoveGenerator;
	friend class ShogiTest;
public:
	Kyokumen();
	std::string toSfen()const;
	std::string toBanFigure()const;

	koma::Koma proceed(const Move);
	koma::Koma recede(const Move, const koma::Koma cap);

	std::uint64_t getHash()const;
	bool teban()const { return isSente; }
	koma::Koma getKoma(const koma::Position pos)const { assert(pos < 25); return static_cast<koma::Koma> (bammen[static_cast<size_t>(pos)]); }
	koma::Koma getKoma(const unsigned pos)const { assert(pos < 25); return static_cast<koma::Koma> (bammen[static_cast<size_t>(pos)]); }
	unsigned getMochigomaNum(koma::Position mpos)const { assert(mpos >= 25); return static_cast<unsigned> (bammen[static_cast<size_t>(mpos)]); }
	unsigned getMochigomaNum(bool teban, koma::Mochigoma koma)const { return teban ? static_cast<unsigned>(bammen[static_cast<size_t>(koma) + 25]) : static_cast<unsigned>(bammen[static_cast<size_t>(koma) + 25 + 5]); }
	const Bitboard& getAllBB()const { return allKomaBB; }
	const Bitboard& getSenteBB()const { return senteKomaBB; }
	const Bitboard& getGoteBB()const { return goteKomaBB; }
	const Bitboard& getEachBB(const koma::Koma k)const { return eachKomaBB[static_cast<size_t>(k)]; }
	unsigned sOuPos()const { return eachKomaBB[static_cast<size_t>(koma::Koma::s_Ou)].find_first(); }
	unsigned gOuPos()const { return eachKomaBB[static_cast<size_t>(koma::Koma::g_Ou)].find_first(); }

	

	bool isSente;
	Bammen bammen;
	Bitboard allKomaBB;
	Bitboard senteKomaBB;
	Bitboard goteKomaBB;
	std::array<Bitboard, static_cast<size_t>(koma::Koma::KomaNum)> eachKomaBB;
};