#pragma once
#include "kkppt_param.h"

namespace kkppt {
	using EvalElement = std::array<int16_t, 2>;
	using KKPPEvalElement0 = EvalElement[fe_end];
	using KKPPEvalElement1 = KKPPEvalElement0[fe_end];
	using KKPPEvalElement2 = KKPPEvalElement1[SquareNum];
	using KKPPEvalElement3 = KKPPEvalElement2[SquareNum];

	extern std::array<int, static_cast<size_t>(koma::Koma::KomaNum)> PieceScoreArr;
	extern KKPPEvalElement2* KKPP;
	extern bool allocated;
	inline int PieceScore(koma::Koma k) { return PieceScoreArr[static_cast<size_t>(k)]; }
	inline int PieceScore(koma::Mochigoma m) { return PieceScoreArr[static_cast<size_t>(m)]; }
	inline int PieceScore(koma::Mochigoma m, bool teban) { return teban ? PieceScoreArr[static_cast<size_t>(m)] : -PieceScoreArr[static_cast<size_t>(m)]; }

	struct EvalList {
		static constexpr int EvalListSize = 10;
		std::array<EvalIndex, EvalListSize> list0;
		int material = 0;

		EvalList() :list0{ {f_hand_pawn} } {} //search_treeはコンストラクトを持たないのでデフォルトコンストラクタが要る
		EvalList(const Kyokumen& k) { set(k); }
		void set(const Kyokumen&);
	};

	struct EvalSum {
		EvalSum() :p({0,0}) {}
		std::array<std::int32_t, 2> p;
		std::int32_t sum(bool teban)const {
			return (teban ? p[0] : -p[0]) + p[1];
		}
		EvalSum& operator+=(const EvalSum& rhs) {
			p[0] += rhs.p[0]; p[1] += rhs.p[1];
			return *this;
		}
		EvalSum& operator-=(const EvalSum& rhs) {
			p[0] -= rhs.p[0]; p[1] -= rhs.p[1];
			return *this;
		}
		EvalSum operator+(const EvalSum& rhs) { return EvalSum(*this) += rhs; }
		EvalSum operator-(const EvalSum& rhs) { return EvalSum(*this) -= rhs; }
	};

	inline std::array<std::int32_t, 2> operator += (std::array<std::int32_t, 2>& lhs, const std::array<std::int16_t, 2>& rhs) {
		lhs[0] += rhs[0];
		lhs[1] += rhs[1];
		return lhs;
	}
	inline std::array<std::int32_t, 2> operator -= (std::array<std::int32_t, 2>& lhs, const std::array<std::int16_t, 2>& rhs) {
		lhs[0] -= rhs[0];
		lhs[1] -= rhs[1];
		return lhs;
	}

	class kkppt_feat {
	public:
		static void init(const std::string& folderpath);
		static void save(const std::string& folderpath);
		static EvalSum EvalFull(const Kyokumen&, const EvalList&);

	public:
		kkppt_feat() {}
		kkppt_feat(const Kyokumen& k) :idlist(k) { sum = EvalFull(k, idlist); }
		EvalList idlist;
		EvalSum sum;
		void set(const Kyokumen& kyokumen);
		void proceed(const Kyokumen& before, const Move& move);
		void recede(const Kyokumen& before, const koma::Koma moved, const koma::Koma captured, const Move move, const EvalSum& cache);
		EvalSum getCache() { return sum; }
		bool operator==(const kkppt_feat& rhs)const;
		bool operator!=(const kkppt_feat& rhs)const {
			return !operator==(rhs);
		}
		std::string toString()const;


		friend class kkppt_learn;
	};

}