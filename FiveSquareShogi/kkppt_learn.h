#pragma once
#include "kkppt_evaluate.h"

namespace kkppt {
	constexpr size_t lkkpptnum = SquareNum * SquareNum * fe_end * (fe_end - 1);
	constexpr size_t l_pp_num = fe_end * (fe_end - 1);
	using EvalVectorFloat = float;
	using KKPPEvalVectorFloat = EvalVectorFloat[lkkpptnum];
	constexpr size_t kkpptToLkkpptnum(const unsigned k1, const unsigned k2, const unsigned p1, const unsigned p2, const unsigned t) {
		assert(p1 != p2);
		const size_t pp = (p1 > p2) ? (p1 * (p1 - 1) + p2 * 2) : (p2 * (p2 - 1) + p1 * 2);
		return (size_t)k1 * SquareNum * l_pp_num + (size_t)k2 * l_pp_num + pp + t; 
	}

	class kkppt_paramVector {
	public:
		static void EvalClamp(std::int16_t absmax);

		kkppt_paramVector();
		kkppt_paramVector(kkppt_paramVector&&)noexcept;
		kkppt_paramVector& operator=(kkppt_paramVector&&)noexcept;
		~kkppt_paramVector();
		kkppt_paramVector(const kkppt_paramVector&) = delete;
		kkppt_paramVector& operator=(const kkppt_paramVector&)= delete;

		void reset();
		void addGrad(const float scalar, const SearchPlayer&);
		void clamp(float absmax);

		void updateEval();
		void save(const std::string& path);
		void load(const std::string& path);
	private:
		EvalVectorFloat* KKPP;

	public:
		struct fvpair { const float f; const kkppt_paramVector& v; fvpair(const float f, const kkppt_paramVector& v) :f(f), v(v) {} };

	public:
		kkppt_paramVector& operator+=(const kkppt_paramVector& rhs);
		kkppt_paramVector& operator+=(const fvpair& rhs);
		kkppt_paramVector& operator*=(const double c);

		void showLearnVec(double displaymin,int dummy)const;

		friend class kppt_learn;
		friend class ShogiTest;
	};
	inline kkppt_paramVector::fvpair operator*(const float lhs, const kkppt_paramVector& rhs) { return kkppt_paramVector::fvpair(lhs, rhs); }
	
}