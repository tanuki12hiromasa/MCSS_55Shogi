#include "stdafx.h"
#include "kkppt_feature.h"
#include <iostream>
#include <fstream>

namespace kkppt {
	std::array<int, static_cast<size_t>(koma::Koma::KomaNum)> PieceScoreArr;
	KKPPEvalElement2* KKPP;
	bool allocated = false;

	void EvalList::set(const Kyokumen& kyokumen) {
		using namespace ::koma;
		int nlist = 0;
		material = 0;
#define Mochi(index0,koma,teban) \
 for(int32_t i=kyokumen.getMochigomaNum(teban, koma); i>0; --i) {\
	 assert(nlist < EvalListSize);\
	 list0[nlist]=index0+i;\
	 material+= teban ? PieceScore(koma) : -PieceScore(koma);\
	 ++nlist; \
 }
		Mochi(f_hand_pawn, Mochigoma::Fu, true);
		Mochi(e_hand_pawn, Mochigoma::Fu, false);
		Mochi(f_hand_silver, Mochigoma::Gin, true);
		Mochi(e_hand_silver, Mochigoma::Gin, false);
		Mochi(f_hand_gold, Mochigoma::Kin, true);
		Mochi(e_hand_gold, Mochigoma::Kin, false);
		Mochi(f_hand_bishop, Mochigoma::Kaku, true);
		Mochi(e_hand_bishop, Mochigoma::Kaku, false);
		Mochi(f_hand_rook, Mochigoma::Hi, true);
		Mochi(e_hand_rook, Mochigoma::Hi, false);
#undef Mochi
		Bitboard bb = kyokumen.getAllBB();
		bb &= ~(kyokumen.getEachBB(Koma::s_Ou) | kyokumen.getEachBB(Koma::g_Ou));
		for (int i = bb.find_first(); i < bb.size(); i = bb.find_next(i)) {
			assert(nlist < EvalListSize);
			Koma koma = kyokumen.getKoma(i);
			list0[nlist] = komaToIndex(koma) + i;
			material += PieceScore(koma);
			++nlist;
		}
		assert(nlist == EvalListSize);
	}


	void kkppt_feat::init(const std::string& folderpath) {
		if (!allocated) {
			allocated = true;
			PieceScoreArr = { PawnScore, SilverScore, BishopScore, RookScore, GoldScore, ScoreZero, // King
							 ProPawnScore, ProSilverScore, HorseScore, DragonScore,
							-PawnScore, -SilverScore, -BishopScore, -RookScore, -GoldScore, ScoreZero, // King
							-ProPawnScore, -ProSilverScore, -HorseScore, -DragonScore };
			KKPP = new KKPPEvalElement2[SquareNum];
			memset(KKPP, 0, sizeof(KKPPEvalElement2) * (size_t)SquareNum);
		}
		{
			std::ifstream fs(folderpath + "/KKPP.bin", std::ios::binary);
			if (!fs) {
				std::cerr << "error:file(KKPP.bin) cannot open" << std::endl;
				return;
			}
			fs.read(reinterpret_cast<char*>(KKPP), sizeof(KKPPEvalElement3));
		}
		std::cout << "Parameters have been read from " << folderpath << std::endl;
	}

	void kkppt_feat::save(const std::string& folderpath) {
		std::ofstream fs(folderpath + "/KKPP.bin", std::ios::binary);
		if (!fs) {
			std::cerr << "error:file(KKPP.bin) cannot open" << std::endl;
			return;
		}
		fs.write(reinterpret_cast<char*>(KKPP), sizeof(KKPPEvalElement3));
		std::cout << "Parameters have been written to " << folderpath << std::endl;
	}

	EvalSum kkppt_feat::EvalFull(const Kyokumen& kyokumen, const EvalList& elist) {
		const unsigned skpos = kyokumen.sOuPos();
		const unsigned gkpos = kyokumen.gOuPos();
		const auto* pp = KKPP[skpos][gkpos];
		EvalSum sum;
		for (int i = 0; i < EvalList::EvalListSize; ++i) {
			const int k0 = elist.list0[i];
			const auto* p = pp[k0];
			for (int j = 0; j < i; j++) {
				const int l0 = elist.list0[j];
				sum.p += p[l0];
			}
		}
		sum.p[0] += elist.material * FVScale;
		return sum;
	}

	void kkppt_feat::set(const Kyokumen& kyokumen) {
		idlist.set(kyokumen);
		sum = EvalFull(kyokumen, idlist);
	}

	void kkppt_feat::proceed(const Kyokumen& before, const Move& move) {
#define Replace(before,after,list) {int i=0;for(;i<EvalList::EvalListSize;i++){if(list[i]==before){list[i]=after;break;}}assert(i<EvalList::EvalListSize);}
		using namespace ::koma;
		const Position from = move.from();
		const Position to = move.to();
		if (!isInside(from)) { //駒台から
			Mochigoma m = MposToMochi(from);
			Koma k = MposToKoma(from);
			int mNum = before.getMochigomaNum(before.teban(), m);
			EvalIndex bIndex0 = mochiToIndex(m, before.teban()) + mNum;//b:before, a:after
			EvalIndex aIndex0 = komaToIndex(k) + to;

			const unsigned skpos = before.sOuPos();
			const unsigned gkpos = before.gOuPos();
			{
				const auto* p = KKPP[skpos][gkpos][bIndex0];
				for (int i = 0; i < EvalList::EvalListSize; i++) {
					const auto index0 = idlist.list0[i];
					sum.p -= p[index0];
				}
			}
			Replace(bIndex0, aIndex0, idlist.list0);
			{
				const auto* p = KKPP[skpos][gkpos][aIndex0];
				for (int i = 0; i < EvalList::EvalListSize; i++) {
					const auto index0 = idlist.list0[i];
					sum.p += p[index0];
				}
			}
		}
		else {
			Koma k = before.getKoma(from);
			Koma cap = before.getKoma(to);
			if (k == Koma::s_Ou) {//先手玉が動いた場合
				const unsigned gkpos = before.gOuPos();
				if (cap != Koma::None) {
					//駒を取った場合 非手番側の駒が取られて手番側の駒になる
					Mochigoma m_cap = KomaToMochi(cap);
					int mNum = before.getMochigomaNum(before.teban(), m_cap) + 1;
					EvalIndex bcIndex0 = komaToIndex(cap) + to;
					EvalIndex acIndex0 = mochiToIndex(m_cap, before.teban()) + mNum;
					Replace(bcIndex0, acIndex0, idlist.list0);
					idlist.material -= PieceScore(cap);
					idlist.material += PieceScore(m_cap, before.teban());
				}
				//KKPP全体を再計算
				//玉はリストにないのでリストは更新されない
				sum.p[0] = idlist.material * FVScale; sum.p[1] = 0;
				const unsigned skpos = to;
				const auto* pp = KKPP[skpos][gkpos];
				for (int i = 0; i < EvalList::EvalListSize; i++) {
					const auto index0_i = idlist.list0[i];
					for (int j = 0; j < i; j++) {
						const auto index0_j = idlist.list0[j];
						sum.p += pp[index0_i][index0_j];
					}
				}
			}
			else if (k == Koma::g_Ou) {//後手玉が動いた場合
				const unsigned skpos = before.sOuPos();
				if (cap != Koma::None) {
					//駒を取った場合 非手番側の駒が取られて手番側の駒になる
					Mochigoma m_cap = KomaToMochi(cap);
					int mNum = before.getMochigomaNum(before.teban(), m_cap) + 1;
					EvalIndex bcIndex0 = komaToIndex(cap) + to;
					EvalIndex acIndex0 = mochiToIndex(m_cap, before.teban()) + mNum;
					Replace(bcIndex0, acIndex0, idlist.list0);
					idlist.material -= PieceScore(cap);
					idlist.material += PieceScore(m_cap, before.teban());
				}
				//KKPP全体を再計算
				sum.p[0] = idlist.material * FVScale; sum.p[1] = 0;
				const unsigned gkpos = to;
				const auto* pp = KKPP[skpos][gkpos];
				for (int i = 0; i < EvalList::EvalListSize; i++) {
					const auto index0_i = idlist.list0[i];
					for (int j = 0; j < i; j++) {
						const auto index0_j = idlist.list0[j];
						sum.p += pp[index0_i][index0_j];
					}
				}
			}
			else if (cap != Koma::None) {//駒をとっていた場合
				Koma k = before.getKoma(from);
				EvalIndex bIndex0 = komaToIndex(k) + from;
				if (move.promote()) {
					int material_diff = -PieceScore(k);
					k = promote(k);
					material_diff += PieceScore(k);
					idlist.material += material_diff;
					sum.p[0] += material_diff * FVScale;
				}
				EvalIndex aIndex0 = komaToIndex(k) + to;

				Mochigoma m_cap = KomaToMochi(cap);
				int mNum = before.getMochigomaNum(before.teban(), m_cap) + 1;
				EvalIndex bcIndex0 = komaToIndex(cap) + to;
				EvalIndex acIndex0 = mochiToIndex(m_cap, before.teban()) + mNum;

				const unsigned skpos = before.sOuPos();
				const unsigned gkpos = before.gOuPos();
				{
					const auto* p = KKPP[skpos][gkpos][bIndex0];
					const auto* cp = KKPP[skpos][gkpos][bcIndex0];
					for (int i = 0; i < EvalList::EvalListSize; i++) {
						const auto index0 = idlist.list0[i];
						sum.p -= p[index0];
						sum.p -= cp[index0];
					}
					sum.p += KKPP[skpos][gkpos][bIndex0][bcIndex0];//二重に引き算している部分を戻す
				}
				Replace(bIndex0, aIndex0, idlist.list0);
				Replace(bcIndex0, acIndex0, idlist.list0);
				{
					int material_diff = PieceScore(m_cap, before.teban()) - PieceScore(cap);
					idlist.material += material_diff;
					sum.p[0] += material_diff * FVScale;
				}
				{
					const auto* p = KKPP[skpos][gkpos][aIndex0];
					const auto* cp = KKPP[skpos][gkpos][acIndex0];
					for (int i = 0; i < EvalList::EvalListSize; i++) {
						const auto index0 = idlist.list0[i];
						sum.p += p[index0];
						sum.p += cp[index0];
					}
					sum.p -= KKPP[skpos][gkpos][aIndex0][acIndex0];//二重に足し算している部分を戻す
				}
			}
			else {//玉以外の駒が何も取らずに動いたとき
				Koma k = before.getKoma(from);
				EvalIndex bIndex0 = komaToIndex(k) + from;
				EvalIndex bIndex1 = komaToIndex(sgInv(k)) + inverse(from);
				if (move.promote()) {
					int material_diff = -PieceScore(k);
					k = promote(k);
					material_diff += PieceScore(k);
					idlist.material += material_diff;
					sum.p[0] += material_diff * FVScale;
				}
				EvalIndex aIndex0 = komaToIndex(k) + to;
				EvalIndex aIndex1 = komaToIndex(sgInv(k)) + inverse(to);

				const unsigned skpos = before.sOuPos();
				const unsigned gkpos = before.gOuPos();
				{
					const auto* p = KKPP[skpos][gkpos][bIndex0];
					for (int i = 0; i < EvalList::EvalListSize; i++) {
						const auto index0 = idlist.list0[i];
						sum.p -= p[index0];
					}
				}
				Replace(bIndex0, aIndex0, idlist.list0);
				{
					const auto* p = KKPP[skpos][gkpos][aIndex0];
					for (int i = 0; i < EvalList::EvalListSize; i++) {
						const auto index0 = idlist.list0[i];
						sum.p += p[index0];
					}
				}
			}
		}
#undef Replace
	}

	void kkppt_feat::recede(const Kyokumen& before, const koma::Koma moved, const koma::Koma captured, const Move move, const EvalSum& cache) {
		using namespace koma;
#define Replace(before,after,list) {int i=0;for(;i<EvalList::EvalListSize;i++){if(list[i]==after){list[i]=before;break;}}assert(i<EvalList::EvalListSize);}
		const bool teban = before.teban();
		const Position from = move.from();
		const Position to = move.to();
		const bool prom = move.promote();
		if (isInside(from)) {//移動による手
			if (moved != Koma::s_Ou && moved != Koma::g_Ou) {//動かした駒が玉以外ならlistを更新（玉はlistに含まれない）
				const EvalIndex aIndex0 = komaToIndex(moved) + to;
				const EvalIndex aIndex1 = komaToIndex(sgInv(moved)) + inverse(to);
				if (prom) {
					const Koma movedDisprom = dispromote(moved);
					const EvalIndex bIndex0 = komaToIndex(movedDisprom) + from;
					Replace(bIndex0, aIndex0, idlist.list0);
					idlist.material += -PieceScore(moved) + PieceScore(movedDisprom); //駒価値
				}
				else {
					const EvalIndex bIndex0 = komaToIndex(moved) + from;
					Replace(bIndex0, aIndex0, idlist.list0);
				}
			}
			if (captured != Koma::None) {//駒を取っていた
				const Mochigoma capMochi = KomaToMochi(captured);
				const int mNum = before.getMochigomaNum(teban, capMochi) + 1;
				const EvalIndex acIndex0 = mochiToIndex(capMochi, teban) + mNum;
				const EvalIndex bcIndex0 = komaToIndex(captured) + to;
				Replace(bcIndex0, acIndex0, idlist.list0);
				idlist.material += -PieceScore(capMochi, teban) + PieceScore(captured); //駒価値
			}
		}
		else {//打ち駒による手
			const EvalIndex aIndex0 = komaToIndex(moved) + to;
			const Mochigoma fromMochi = MposToMochi(from);
			const int mNum = before.getMochigomaNum(from);
			const EvalIndex bIndex0 = mochiToIndex(fromMochi, teban) + mNum;
			Replace(bIndex0, aIndex0, idlist.list0);
		}
#undef Replace
		sum.p = cache.p;
	}

	bool kkppt_feat::operator==(const kkppt_feat& rhs)const {
		//return idlist.list0 == rhs.idlist.list0 && idlist.list1 == rhs.idlist.list1 && idlist.material == idlist.material && sum.p == rhs.sum.p;
		if (idlist.material != rhs.idlist.material || sum.p != rhs.sum.p) return false;
		if (idlist.list0 == rhs.idlist.list0) return true;//idlistは順番が不定だが、一致する場合はtrueなので先に返す
		std::vector<EvalIndex> checklist0(rhs.idlist.list0.begin(), rhs.idlist.list0.end());
		for (const auto val : idlist.list0) {
			auto result = std::find(checklist0.begin(), checklist0.end(), val);
			if (result == checklist0.end())return false;
			else checklist0.erase(result);
		}
		if (!checklist0.empty()) return false;
		return true;
	}

	std::string kkppt_feat::toString()const {
		std::string str;
		for (int j = 0; j < 2; j++) {
			str += "p[" + std::to_string(j) + "] = " + std::to_string(sum.p[j]) + "  ";
		}
		str += "\n";
		str += "list: ";
		for (const auto val : idlist.list0)str += std::to_string(val) + " ";
		str += "\n";
		return str;
	}
}