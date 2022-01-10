#pragma once
#include "kpp_kkpt_param.h"

#ifdef _LEARN
#ifndef _LEARN_COMMANDER
#define _EVAL_FLOAT
#endif
#endif

namespace kpp_kkpt {
#ifndef _EVAL_FLOAT
	using PieceScoreType = int;
	using EvalElementTypeh = std::int16_t;
	using EvalElementType = std::array<int16_t, 2>;
#else
	using PieceScoreType = float;
	using EvalElementTypeh = float;
	using EvalElementType = std::array<float, 2>;
#endif
	using KPPEvalElementType0 = EvalElementTypeh[fe_end];
	using KPPEvalElementType1 = KPPEvalElementType0[fe_end];
	using KPPEvalElementType2 = KPPEvalElementType1[SquareNum];
	using KKPEvalElementType0 = EvalElementType[fe_end];
	using KKPEvalElementType1 = KKPEvalElementType0[SquareNum];
	using KKPEvalElementType2 = KKPEvalElementType1[SquareNum];
	using KKEvalElementType0 = EvalElementType[SquareNum];
	using KKEvalElementType1 = KKEvalElementType0[SquareNum];

	extern std::array<PieceScoreType, static_cast<size_t>(koma::Koma::KomaNum)> PieceScoreArr;
	extern KPPEvalElementType1* KPP;
	extern KKPEvalElementType1* KKP;
	extern KKEvalElementType0* KK;
	extern bool dynamicPieceScore;
	extern bool allocated;
	inline int PieceScore(koma::Koma k) { return PieceScoreArr[static_cast<size_t>(k)]; }
	inline int PieceScore(koma::Mochigoma m) { return PieceScoreArr[static_cast<size_t>(m)]; }
	inline int PieceScore(koma::Mochigoma m, bool teban) { return teban ? PieceScoreArr[static_cast<size_t>(m)] : -PieceScoreArr[static_cast<size_t>(m)]; }

	struct EvalList {
		static constexpr int EvalListSize = 10;
		std::array<EvalIndex, EvalListSize> list0;
		std::array<EvalIndex, EvalListSize> list1;
		int material = 0;

		EvalList() :list0{ {f_hand_pawn} }, list1{ {f_hand_pawn} } {} //search_treeはコンストラクトを持たないのでデフォルトコンストラクタが要る
		EvalList(const Kyokumen& k) { set(k); }
		void set(const Kyokumen&);
	};

	struct EvalSum {
		EvalSum() :p{ {{0,0},{0,0},{0,0}} } {}
		std::array<std::array<std::int32_t, 2>, 3> p; //{{KPP盤面先手視点,KPP盤面後手視点},{KKPT盤面,KKPT手番},{KKT盤面,KKT手番}}
		std::int32_t sum(bool teban) const {
			std::int32_t BanScore = p[0][0] - p[0][1] + p[1][0] + p[2][0];
			std::int32_t TebanScore = p[1][1] + p[2][1];
			return (teban ? BanScore : -BanScore) + TebanScore;
		}
		EvalSum& operator+=(const EvalSum& rhs) {
			p[0][0] += rhs.p[0][0]; p[0][1] += rhs.p[0][1];
			p[1][0] += rhs.p[1][0];	p[1][1] += rhs.p[1][1];
			p[2][0] += rhs.p[2][0];	p[2][1] += rhs.p[2][1];
			return *this;
		}
		EvalSum& operator-=(const EvalSum& rhs) {
			p[0][0] -= rhs.p[0][0]; p[0][1] -= rhs.p[0][1];
			p[1][0] -= rhs.p[1][0];	p[1][1] -= rhs.p[1][1];
			p[2][0] -= rhs.p[2][0];	p[2][1] -= rhs.p[2][1];
			return *this;
		}
		EvalSum operator+(const EvalSum& rhs) { return EvalSum(*this) += rhs; }
		EvalSum operator-(const EvalSum& rhs) { return EvalSum(*this) -= rhs; }
	};

	inline std::array<std::int32_t, 2> operator += (std::array<std::int32_t, 2>& lhs, const EvalElementType& rhs) {
		lhs[0] += rhs[0];
		lhs[1] += rhs[1];
		return lhs;
	}
	inline std::array<std::int32_t, 2> operator -= (std::array<std::int32_t, 2>& lhs, const EvalElementType& rhs) {
		lhs[0] -= rhs[0];
		lhs[1] -= rhs[1];
		return lhs;
	}

	class kpp_kkpt_feat {
	public:
		static void init(const std::string& folderpath);
		static void save(const std::string& folderpath);
		static EvalSum EvalFull(const Kyokumen&, const EvalList&);

	public:
		kpp_kkpt_feat() {}
		kpp_kkpt_feat(const Kyokumen& k) :idlist(k) { sum = EvalFull(k, idlist); }
		EvalList idlist;
		EvalSum sum;
		void set(const Kyokumen& kyokumen);
		void proceed(const Kyokumen& before, const Move& move);
		void recede(const Kyokumen& before, const koma::Koma moved, const koma::Koma captured, const Move move, const EvalSum& cache);
		EvalSum getCache() { return sum; }
		bool operator==(const kpp_kkpt_feat& rhs)const;
		bool operator!=(const kpp_kkpt_feat& rhs)const {
			return !operator==(rhs);
		}
		std::string toString()const;

		friend class kpp_kkpt_learn;
	};
}