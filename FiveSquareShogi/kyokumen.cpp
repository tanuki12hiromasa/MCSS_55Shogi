#include "stdafx.h"
#include "kyokumen.h"
#include "usi.h"
#include "bb_kiki.h"

using namespace koma;

const Bammen prime_bammen = {
	static_cast<std::uint8_t>(Koma::g_Ou),static_cast<std::uint8_t>(Koma::g_Fu),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Hi),
	static_cast<std::uint8_t>(Koma::g_Kin),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Kaku),
	static_cast<std::uint8_t>(Koma::g_Gin),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Gin),
	static_cast<std::uint8_t>(Koma::g_Kaku),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Kin),
	static_cast<std::uint8_t>(Koma::g_Hi),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::None),static_cast<std::uint8_t>(Koma::s_Fu),static_cast<std::uint8_t>(Koma::s_Ou),
	0u,0u,0u,0u,0u,
	0u,0u,0u,0u,0u
};

const std::array<Koma, 8> sStepKomas = { Koma::s_Fu, Koma::s_Gin, Koma::s_Kin, Koma::s_Ou, Koma::s_nFu, Koma::s_nGin, Koma::s_nKaku, Koma::s_nHi };
const std::array<Koma, 4> sDashKomas = { Koma::s_Kaku, Koma::s_Hi, Koma::s_nKaku, Koma::s_nHi };
const std::array<Koma, 8> gStepKomas = { Koma::g_Fu, Koma::g_Gin, Koma::g_Kin, Koma::g_Ou, Koma::g_nFu, Koma::g_nGin, Koma::g_nKaku, Koma::g_nHi };
const std::array<Koma, 4> gDashKomas = { Koma::g_Kaku, Koma::g_Hi, Koma::g_nKaku, Koma::g_nHi };

Kyokumen::Kyokumen() :
	Kyokumen(prime_bammen, true) {}

Kyokumen::Kyokumen(const Bammen& bammen, bool teban)
	: bammen(bammen), isSente(teban)
{
	reflectBitboard();
}

Kyokumen::Kyokumen(Bammen&& bammen, bool teban)
	: bammen(std::move(bammen)), isSente(teban)
{
	reflectBitboard();
}


Kyokumen::Kyokumen(const std::vector<std::string>& tokens) {
	if (tokens[1] == "startpos") {
		bammen = prime_bammen;
		isSente = true;
	}
	else {
		assert(tokens[1] == "sfen");
		//盤面
		std::vector<std::string> usiban(usi::split(tokens[2], '/'));
		int dan = 0;
		for (auto& line : usiban) {
			int suji = 4;
			for (auto c = line.begin(), end = line.end(); c != end; ++c) {
				if ('0' < *c && *c <= '5') {
					for (int num = *c - '0'; num > 0; --num) {
						setKoma(XYtoPos(suji, dan), Koma::None);
						--suji;
					}
				}
				else {
					if (*c == '+') {
						++c;
						setKoma(XYtoPos(suji, dan), promote(usi::usiToKoma(*c)));
					}
					else {
						setKoma(XYtoPos(suji, dan), usi::usiToKoma(*c));
					}
					--suji;
				}
			}
			++dan;
		}
		//手番
		isSente = (tokens[3] == "b");
		//持ち駒
		for (size_t t = static_cast<size_t>(Position::SQm_Min); t <= static_cast<size_t>(Position::SQm_Max); t++) {
			bammen[t] = 0;
		}
		auto mStr = tokens[4];
		if (mStr.front() != '-') {
			for (auto c = mStr.begin(), end = mStr.end(); c != end; ++c) {
				unsigned num = 1;
				if (*c == '1') {
					c++;
					num = *c - '0' + 10u;
					c++;
				}
				else if ('1' < *c && *c <= '5') {
					num = *c - '0';
					c++;
				}
				bammen[MochiToMpos(usi::usiToMochi(*c), std::isupper(*c))] = num;
			}
		}
	}
	reflectBitboard();
}

std::string Kyokumen::toSfen()const {
	std::string usi = "sfen ";
	//sq
	for (int y = 0; y < 5; y++) {
		int nonecount = 0;
		for (int x = 4; x >= 0; x--) {
			if (bammen[x * 5ull + y] != static_cast<std::uint8_t>(Koma::None)) {
				if (nonecount != 0) {
					usi += std::to_string(nonecount);
					nonecount = 0;
				}
				usi += usi::komaToUsi(getKoma(static_cast<Position>(x * 5 + y)));
			}
			else {
				nonecount++;
			}
		}
		if (nonecount != 0) {
			usi += std::to_string(nonecount);
		}
		if (y < 8)usi += '/';
	}
	usi += ' ';
	usi += usi::tebanToUsi(teban());
	usi += ' ';
	//mochi
	bool wrote_mochi = false;
	for (auto mpos : { Position::m_sHi,Position::m_sKaku,Position::m_sKin,
		Position::m_sGin,Position::m_sFu,
		Position::m_gHi,Position::m_gKaku,Position::m_gKin,
		Position::m_gGin,Position::m_gFu })
	{
		auto num = bammen[mpos];
		if (num > 0) {
			if (num > 1) {
				usi += std::to_string(num);
			}
			usi += usi::mposToUsi(mpos);
			wrote_mochi = true;
		}
	}
	if (!wrote_mochi) {
		usi += '-';
	}
	usi += " 1 ";
	return usi;
}


std::string Kyokumen::toBanFigure()const {
	std::string fig;
	fig += "teban: ";
	fig += (teban() ? "sente\n" : "gote\n");
	for (int y = 0; y < 5; y++) {
		for (int x = 5 - 1; x >= 0; x--) {
			Koma k = getKoma(static_cast<Position>(x * 5 + y));
			auto s = (k != koma::Koma::None) ? usi::komaToUsi(k) : "-";
			if (s.length() == 1) {
				s = ' ' + s;
			}
			fig += s + ' ';
		}
		fig += '\n';
	}
	fig += "SenteMochi: ";
	std::string smochistr;
	for (Mochigoma m = Mochigoma::Fu; m != Mochigoma::MochigomaNum; m = static_cast<Mochigoma>(static_cast<uint8_t>(m) + 1)) {
		int mNum = getMochigomaNum(true, m);
		if (mNum > 0) {
			smochistr += usi::mochigomaToUsi(true, m) + std::to_string(mNum) + " ";
		}
	}
	if (smochistr == "") smochistr = "none";
	fig += smochistr;
	fig += '\n';
	fig += "GoteMochi: ";
	std::string gmochistr;
	for (Mochigoma m = Mochigoma::Fu; m != Mochigoma::MochigomaNum; m = static_cast<Mochigoma>(static_cast<uint8_t>(m) + 1)) {
		int mNum = getMochigomaNum(false, m);
		if (mNum > 0) {
			gmochistr += usi::mochigomaToUsi(false, m) + std::to_string(mNum) + " ";
		}
	}
	if (gmochistr == "") gmochistr = "none";
	fig += gmochistr;
	return fig;
}

koma::Koma Kyokumen::proceed(const Move move) {
	const unsigned from = move.from(), to = move.to();
	const bool prom = move.promote();
	const Koma captured = getKoma(to);
	//sente,gote,allBB
	if (teban()) { //先手
		if (koma::isInside(from)) { //元位置が盤内ならそのビットを消す
			senteKomaBB.reset(from);
		}
		senteKomaBB.set(to); //行き先のビットを立てる
		goteKomaBB.reset(to); //行き先の敵盤ビットを下げる
	}
	else { //後手
		if (koma::isInside(from)) {
			goteKomaBB.reset(from);
		}
		goteKomaBB.set(to);
		senteKomaBB.reset(to);
	} //全体
	allKomaBB = senteKomaBB | goteKomaBB;

	//eachKomaBB,bammen
	if (koma::isInside(from)) {//盤上の駒を動かす場合
		const Koma fromKoma = getKoma(from);
		Bitboard& fromKomaBB = eachKomaBB[static_cast<size_t>(fromKoma)];
		fromKomaBB.reset(from);	//BB from
		bammen[from] = static_cast<std::uint8_t>(Koma::None);	//ban from
		if (prom) {//成る場合
			Koma fromPromKoma = promote(fromKoma);
			Bitboard& fromPromKomaBB = eachKomaBB[static_cast<size_t>(fromPromKoma)];
			fromPromKomaBB.set(to);	//BB to
			bammen[to] = static_cast<std::uint8_t>(fromPromKoma);//ban to
		}
		else {//成らない場合
			fromKomaBB.set(to);		//BB to
			bammen[to] = static_cast<std::uint8_t>(fromKoma);	//ban to
		}
		if (captured != Koma::None) {//駒を取っていた場合 
			Bitboard& toKomaBB = eachKomaBB[static_cast<size_t>(captured)];
			toKomaBB.reset(to);//BB cap
			bammen[KomaToMpos(captured)]++;//mban cap
			//bammen[to]は後で更新される
		}
	}
	else {//駒台から打つ場合
		const Koma fromKoma = MposToKoma(static_cast<Position>(from));
		Bitboard& fromKomaBB = eachKomaBB[static_cast<size_t>(fromKoma)];
		fromKomaBB.set(to);		//BB to
		bammen[from]--;		//mban from
		bammen[to] = static_cast<std::uint8_t>(fromKoma);	//ban to
	}
	isSente = !isSente;
	return captured;
}


koma::Koma Kyokumen::recede(const Move move, const koma::Koma captured) {
	using namespace koma;
	const unsigned from = move.from(), to = move.to();
	const bool prom = move.promote();
	const Koma fromKoma = getKoma(to);
	//手番を戻す
	isSente = !isSente;
	//senteBB,goteBB
	if (teban()) {
		if (captured != Koma::None) {
			goteKomaBB.set(to);
		}
		senteKomaBB.reset(to);
		if (isInside(from)) {
			senteKomaBB.set(from);
		}
	}
	else {
		if (captured != Koma::None) {
			senteKomaBB.set(to);
		}
		goteKomaBB.reset(to);
		if (isInside(from)) {
			goteKomaBB.set(from);
		}
	}
	allKomaBB = senteKomaBB | goteKomaBB;

	//eachBB,bammen
	if (koma::isInside(move.from())) {
		Bitboard& fromKomaBB = eachKomaBB[static_cast<size_t>(fromKoma)];
		fromKomaBB.reset(to);
		if (prom) {//成った場合,成りを戻す
			const Koma fromDispromKoma = dispromote(fromKoma);
			eachKomaBB[static_cast<size_t>(fromDispromKoma)].set(from);
			bammen[from] = static_cast<size_t>(fromDispromKoma);
		}
		else {//成らなかった場合
			fromKomaBB.set(from);
			bammen[from] = static_cast<size_t>(fromKoma);
		}
		if (captured != Koma::None) {//駒を取っていた場合
			eachKomaBB[static_cast<size_t>(captured)].set(to);
			bammen[KomaToMpos(captured)]--;
		}
	}
	else {//持ち駒から打っていた場合
		eachKomaBB[static_cast<size_t>(fromKoma)].reset(to);
		bammen[from]++;
	}
	bammen[to] = static_cast<uint8_t>(captured);
	return fromKoma;
}

std::uint64_t Kyokumen::getHash()const {
	return senteKomaBB.val() ^ (goteKomaBB.val() << 25)
		^ ((static_cast<uint64_t>(bammen[25]) & 1) << 50) ^ ((static_cast<uint64_t>(bammen[26]) & 1) << 51)
		^ ((static_cast<uint64_t>(bammen[27]) & 1) << 52) ^ ((static_cast<uint64_t>(bammen[28]) & 1) << 53)
		^ ((static_cast<uint64_t>(bammen[29]) & 1) << 54)
		^ ((static_cast<uint64_t>(bammen[30]) & 1) << 55) ^ ((static_cast<uint64_t>(bammen[31]) & 1) << 56)
		^ ((static_cast<uint64_t>(bammen[32]) & 1) << 57) ^ ((static_cast<uint64_t>(bammen[33]) & 1) << 58)
		^ ((static_cast<uint64_t>(bammen[34]) & 1) << 59)
		^ (isSente ? (1ull << 60) : 0);
}


std::vector<Bitboard> Kyokumen::getSenteOuCheck(const Move m)const {
	std::vector<Bitboard> kusemono;
	const unsigned ouPos = sOuPos();
	const unsigned from = m.from();
	const unsigned to = m.to();
	//親局面が存在しない、または玉自身が動いた場合は全体を調べる
	if (from == Position::NullMove || to == ouPos) {
		return getSenteOuCheck();
	}
	//fromでどいたところから空き王手がないか調べる
	if (isInside(from)) {
		Bitboard fpBB = pinMaskSente(from);
		if (fpBB != bbmask::AllOne) {
			fpBB.set(from);
			kusemono.push_back(fpBB);
		}
	}
	//toに移動した駒が玉に効いているか調べる
	const Koma movedKoma = getKoma(to);
	if (isDashable(movedKoma)) {
		Bitboard kiki = BBkiki::getDashKiki(allKomaBB, movedKoma, to);
		if ((kiki & getEachBB(Koma::s_Ou)).any()) {
			kiki.set(to);
			kiki &= BBkiki::getDashKiki(allKomaBB, sgInv(movedKoma), ouPos);
			kusemono.push_back(kiki);
		}
	}
	if (isSteppable(movedKoma)) {
		Bitboard tpBB = Bitboard(to);
		tpBB &= BBkiki::getStepKiki(sgInv(movedKoma), ouPos);
		if (tpBB.any()) {
			kusemono.push_back(tpBB);
		}
	}
	return kusemono;
}

std::vector<Bitboard> Kyokumen::getGoteOuCheck(const Move m)const {
	std::vector<Bitboard> kusemono;
	const unsigned ouPos = gOuPos();
	const unsigned from = m.from();
	const unsigned to = m.to();
	//親局面が存在しない、または玉自身が動いた場合は全体を調べる
	if (from == Position::NullMove || to == ouPos) {
		return getGoteOuCheck();
	}
	//fromでどいたところから空き王手がないか調べる
	if (koma::isInside(from)) {
		Bitboard fpBB = pinMaskGote(from);
		if (fpBB != bbmask::AllOne) {
			fpBB.set(from);
			kusemono.push_back(fpBB);
		}
	}
	//toに移動した駒が玉に効いているか調べる
	const Koma movedKoma = getKoma(to);
	if (isDashable(movedKoma)) {
		Bitboard kiki = BBkiki::getDashKiki(allKomaBB, movedKoma, to);
		if ((kiki & getEachBB(Koma::g_Ou)).any()) {
			kiki.set(to);
			kiki &= BBkiki::getDashKiki(allKomaBB, sgInv(movedKoma), ouPos);
			kusemono.push_back(kiki);
		}
	}
	if (isSteppable(movedKoma)) {
		Bitboard tpBB = Bitboard(to);
		tpBB &= BBkiki::getStepKiki(sgInv(movedKoma), ouPos);
		if (tpBB.any()) {
			kusemono.push_back(tpBB);
		}
	}
	return kusemono;
}

std::vector<Bitboard> Kyokumen::getSenteOuCheck()const {
	std::vector<Bitboard> kusemono;
	const unsigned ouPos = sOuPos();
	for (const Koma koma : gStepKomas) {
		Bitboard kusemonoBB = BBkiki::getStepKiki(sgInv(koma), ouPos) & getEachBB(koma);
		if (kusemonoBB.any()) {
			kusemono.push_back(kusemonoBB);
			break;
		}
	}
	for (const Koma koma : gDashKomas) {
		Bitboard kikiBB = BBkiki::getDashKiki(allKomaBB, sgInv(koma), ouPos);
		Bitboard eBB = kikiBB & getEachBB(koma);
		if (eBB.any()) {
			for (unsigned i = eBB.pop_first(); i != eBB.size(); i = eBB.pop_first()) {
				Bitboard kusemonoBB = BBkiki::getDashKiki(allKomaBB, koma, i) & kikiBB;
				kusemonoBB.set(i);
				kusemono.push_back(kusemonoBB);
			}
		}
	}
	return kusemono;
}

std::vector<Bitboard> Kyokumen::getGoteOuCheck()const {
	std::vector<Bitboard> kusemono;
	const unsigned ouPos = gOuPos();
	for (const Koma koma : sStepKomas) {
		Bitboard kusemonoBB = BBkiki::getStepKiki(sgInv(koma), ouPos) & getEachBB(koma);
		if (kusemonoBB.any()) {
			kusemono.push_back(kusemonoBB);
			break;
		}
	}
	for (const Koma koma : sDashKomas) {
		Bitboard kikiBB = BBkiki::getDashKiki(allKomaBB, sgInv(koma), ouPos);
		Bitboard eBB = kikiBB & getEachBB(koma);
		if (eBB.any()) {
			for (unsigned i = eBB.pop_first(); i != eBB.size(); i = eBB.pop_first()) {
				Bitboard kusemonoBB = BBkiki::getDashKiki(allKomaBB, koma, i) & kikiBB;
				kusemonoBB.set(i);
				kusemono.push_back(kusemonoBB);
			}
		}
	}
	return kusemono;
}

Bitboard Kyokumen::pinMaskSente(const unsigned pos)const {
	const unsigned ouPos = sOuPos();
	Bitboard dpBB(allKomaBB);
	dpBB.reset(pos);
	for (Koma ek : gDashKomas) {
		Bitboard kikiBB = BBkiki::getDashKiki(dpBB, sgInv(ek), ouPos);
		Bitboard eBB = kikiBB & getEachBB(ek);
		if (eBB.any()) {
			for (unsigned i = eBB.pop_first(); i != eBB.size(); i = eBB.pop_first()) {
				Bitboard result = kikiBB & BBkiki::getDashKiki(dpBB, ek, i);
				result.set(i);
				if (result.test(pos)) {
					return result;
				}
			}
		}
	}
	return bbmask::AllOne;
}

Bitboard Kyokumen::pinMaskGote(const unsigned pos)const {
	const unsigned ouPos = gOuPos();
	Bitboard dpBB(allKomaBB);
	dpBB.reset(pos);
	for (Koma ek : sDashKomas) {
		Bitboard kikiBB = BBkiki::getDashKiki(dpBB, sgInv(ek), ouPos);
		Bitboard eBB = kikiBB & getEachBB(ek);
		if (eBB.any()) {
			for (unsigned i = eBB.pop_first(); i != eBB.size(); i = eBB.pop_first()) {
				Bitboard result = kikiBB & BBkiki::getDashKiki(dpBB, ek, i);
				result.set(i);
				if (result.test(pos)) {
					return result;
				}
			}
		}
	}
	return bbmask::AllOne;
}

Bitboard Kyokumen::senteKiki_ingnoreKing()const {
	Bitboard kikiBB;
	Bitboard enBB = senteKomaBB;
	enBB &= ~eachKomaBB[static_cast<size_t>(koma::Koma::s_Ou)];
	for (unsigned pos = enBB.pop_first(); pos != enBB.size(); pos = enBB.pop_first()) {
		const Koma k = getKoma(pos);
		kikiBB |= BBkiki::getKiki(allKomaBB, k, pos);
	}
	return kikiBB;
}

Bitboard Kyokumen::goteKiki_ingnoreKing()const {
	Bitboard kikiBB;
	Bitboard enBB = goteKomaBB;
	enBB &= ~eachKomaBB[static_cast<size_t>(koma::Koma::g_Ou)];
	for (unsigned pos = enBB.pop_first(); pos != enBB.size(); pos = enBB.pop_first()) {
		const Koma k = getKoma(pos);
		kikiBB |= BBkiki::getKiki(allKomaBB, k, pos);
	}
	return kikiBB;
}

bool Kyokumen::operator==(const Kyokumen& rhs) const {
	return (senteKomaBB == rhs.senteKomaBB && goteKomaBB == rhs.goteKomaBB && bammen == rhs.bammen);
}

void Kyokumen::reflectBitboard() {
	//bitboard
	for (auto& bb : eachKomaBB) {
		bb.all_reset();
	}
	for (int i = 0; i < 25; i++) {
		if (getKoma(i) != koma::Koma::None)
			eachKomaBB[bammen[i]].set(i);
	}
	senteKomaBB.all_reset();
	for (size_t i = static_cast<size_t>(koma::Koma::s_Min); i < static_cast<size_t>(koma::Koma::s_Num); i++) {
		senteKomaBB |= eachKomaBB[i]; //先手の駒をすべて集めたbb
	}
	goteKomaBB.all_reset();
	for (size_t i = static_cast<size_t>(koma::Koma::g_Min); i < static_cast<size_t>(koma::Koma::g_Num); i++) {
		goteKomaBB |= eachKomaBB[i]; //後手の駒をすべて集めたbb
	}
	allKomaBB = senteKomaBB | goteKomaBB;//全体のbbは先後のものを合成する
}