﻿#pragma once
#include "kpp_kkpt_evaluate.h"
#include <cassert>
#include <functional>

namespace kpp_kkpt {
	constexpr size_t lkppnum = SquareNum * fe_end * (fe_end - 1) / 2; //kppテーブルのppの被りを消した三角テーブルを一次元にしている セル数はSQnum*(fe*(fe-1)/2)*2
	constexpr size_t lkkptnum = SquareNum * SquareNum * fe_end * 2;//こちらは先後の区別があるため単純に一次元にしている
	constexpr size_t lkktnum = SquareNum * SquareNum * 2;
	constexpr size_t lpiecenum_plain = 5; //成り駒を除いた駒の数
	constexpr size_t lpiecenum = 9; //歩、銀、角、飛、金、と金、成銀、馬、龍 で9つ
	using EvalVectorFloat = float;
	using KPPEvalVectorFloat = EvalVectorFloat[lkppnum];
	using KKPEvalVectorFloat = EvalVectorFloat[lkkptnum];
	using KKEvalVectorFloat = EvalVectorFloat[lkktnum];
	constexpr size_t l_pp_num = fe_end * (fe_end - 1) / 2;
	constexpr size_t l_kk_num = SquareNum * (SquareNum - 1);
	constexpr size_t kppToLkppnum(const unsigned k, const unsigned p1, const unsigned p2) { assert(p1 != p2); const size_t p = (p1 > p2) ? (p1 * (p1 - 1) / 2 + p2) : (p2 * (p2 - 1) / 2 + p1); return k * l_pp_num + p; }
	constexpr size_t kkptToLkkptnum(const unsigned k1, const unsigned k2, const unsigned p, const unsigned t) { return (size_t)k1 * SquareNum * fe_end * 2 + (size_t)k2 * fe_end * 2 + (size_t)p * 2 + t; }
	constexpr size_t kktToLkktnum(const unsigned k1, const unsigned k2, const unsigned t) { return (size_t)k1 * SquareNum * 2 + (size_t)k2 * 2 + t; }

	class kpp_kkpt_paramVector {
	public:
		static void EvalClamp(std::int16_t absmax);

	public:
		kpp_kkpt_paramVector();
		kpp_kkpt_paramVector(kpp_kkpt_paramVector&&)noexcept;
		kpp_kkpt_paramVector& operator=(kpp_kkpt_paramVector&&)noexcept;
		~kpp_kkpt_paramVector();
		kpp_kkpt_paramVector(const kpp_kkpt_paramVector&) = delete;
		kpp_kkpt_evaluator& operator=(const kpp_kkpt_paramVector&) = delete;

		void reset();
		void piece_addGrad(const float scalar, const Kyokumen&);
		void addGrad(const float scalar,const SearchPlayer&);
		void clamp(float absmax);
		void normalize();
		EvalVectorFloat abs_max_value()const;

		void updateEval();
		void save(const std::string& path);//勾配ファイルを出力
		void load(const std::string& path);//勾配ファイルを読み込む
	private:
		std::array<EvalVectorFloat, lpiecenum> PieceScoreArr;
		EvalVectorFloat* KPP;
		EvalVectorFloat* KKP;
		EvalVectorFloat* KK;

	public:
		struct fvpair { const float f; const kpp_kkpt_paramVector& v; fvpair(const float f, const kpp_kkpt_paramVector& v) :f(f), v(v) {} };

	public:
		kpp_kkpt_paramVector& operator+=(const kpp_kkpt_paramVector& rhs);
		kpp_kkpt_paramVector& operator+=(const fvpair& rhs);
		kpp_kkpt_paramVector& operator*=(const double c);

		bool operator==(const kpp_kkpt_paramVector& rhs)const;
		bool operator!=(const kpp_kkpt_paramVector& rhs)const { return !operator==(rhs); }
		void print(double displaymin,int isKPP)const;
	
		friend class kpp_kkpt_learn;
		friend class Adam;
		friend class ShogiTest;
	};
	inline kpp_kkpt_paramVector::fvpair operator*(const float lhs, const kpp_kkpt_paramVector& rhs) { return kpp_kkpt_paramVector::fvpair(lhs,rhs); }

	class Adam {
	public:
		void updateEval(kpp_kkpt_paramVector& dw);
		void save(const std::string& folderpath);
		void load(const std::string& folderpath);

		kpp_kkpt_paramVector mt;
		kpp_kkpt_paramVector vt;
		long long t = 0;
		const double b1 = 0.9;
		const double b2 = 0.999;
		const double epsilon = 1.0e-8;
		const double alpha = 1 / FVScale;
	};
}