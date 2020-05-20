#pragma once
#include "kppt_evaluate.h"

namespace kppt {
	using EvalVectorFloat = std::array<float, 2>;
	using KPPEvalVectorFloat0 = EvalVectorFloat[fe_end];
	using KPPEvalVectorFloat1 = KPPEvalVectorFloat0[fe_end];
	using KPPEvalVectorFloat2 = KPPEvalVectorFloat1[SquareNum];
	using KKPEvalVectorFloat0 = EvalVectorFloat[fe_end];
	using KKPEvalVectorFloat1 = KKPEvalVectorFloat0[SquareNum];
	using KKPEvalVectorFloat2 = KKPEvalVectorFloat1[SquareNum];

	class kppt_paramVector {
	public:
		kppt_paramVector(const SearchPlayer&);
		kppt_paramVector(kppt_paramVector&&)noexcept;
		kppt_paramVector& operator=(kppt_paramVector&&)noexcept;
		kppt_paramVector(const kppt_paramVector&) = delete;
		kppt_evaluator& operator=(const kppt_paramVector&) = delete;

		void set(const SearchPlayer&);
	private:
		KPPEvalVectorFloat1* KPP;
		KKPEvalVectorFloat1* KKP;

	public:
		kppt_paramVector& operator+=(const kppt_paramVector& rhs);
		kppt_paramVector& operator-=(const kppt_paramVector& rhs);
		kppt_paramVector& operator*=(const double c);

		friend class kppt_learn;
		friend class ShogiTest;
	};

	class kppt_learn {
		static void updateEvalParam(const kppt_paramVector&);
	};
}