﻿#include "kppt_learn.h"

namespace kppt {
	kppt_paramVector::kppt_paramVector() {
		KPP = new EvalVectorFloat[lkpptnum];
		KKP = new EvalVectorFloat[lkkptnum];
		reset();
	}

	kppt_paramVector::kppt_paramVector(kppt_paramVector&& rhs) noexcept {
		KPP = rhs.KPP;
		KKP = rhs.KKP;
		rhs.KPP = nullptr;
		rhs.KKP = nullptr;
	}

	kppt_paramVector& kppt_paramVector::operator=(kppt_paramVector&& rhs)noexcept {
		delete[] KPP;
		delete[] KKP;
		KPP = rhs.KPP;
		KKP = rhs.KKP;
		rhs.KPP = nullptr;
		rhs.KKP = nullptr;
		return *this;
	}

	kppt_paramVector::~kppt_paramVector() {
		delete[] KPP;
		delete[] KKP;
	}

	void kppt_paramVector::reset() {
		EvalVectorFloat* const kpp = KPP;
		for (int i = 0; i < lkpptnum; i++) {
			kpp[i] = 0;
		}
		EvalVectorFloat* const kkp = KKP;
		for (int i = 0; i < lkkptnum; i++) {
			kkp[i] = 0;
		}
	}

	void kppt_paramVector::addGrad(float scalar,const SearchPlayer& player,bool rootTeban) {
		if (std::abs(scalar) < 0.00000001f) return;
		EvalVectorFloat* const kpp = KPP;
		EvalVectorFloat* const kkp = KKP;
		const unsigned skpos = player.kyokumen.sOuPos();
		const unsigned gkpos = player.kyokumen.gOuPos();
		const unsigned invgkpos = inverse(gkpos);
		const float bammenscalar = (player.kyokumen.teban()) ? scalar : -scalar;
		const float tebanscalar = (player.kyokumen.teban() == rootTeban) ? scalar : -scalar;
		for (unsigned i = 0; i < EvalList::EvalListSize; ++i) {
			const int k0 = player.feature.idlist.list0[i];
			const int k1 = player.feature.idlist.list1[i];
			for (unsigned j = 0; j < i; j++) {
				const int l0 = player.feature.idlist.list0[j];
				const int l1 = player.feature.idlist.list1[j];

				kpp[kpptToLkpptnum(skpos, k0, l0, 0)] += bammenscalar;
				kpp[kpptToLkpptnum(skpos, k0, l0, 1)] += tebanscalar;
				kpp[kpptToLkpptnum(invgkpos, k1, l1, 0)] -= bammenscalar;
				kpp[kpptToLkpptnum(invgkpos, k1, l1, 1)] += tebanscalar;
			}
			kkp[kkptToLkkptnum(skpos, gkpos, k0, 0)] += bammenscalar;
			kkp[kkptToLkkptnum(skpos, gkpos, k0, 1)] += tebanscalar;
		}
	}

	void kppt_paramVector::updateEval()const {
		//KPPのテーブル形式の違いに注意する
		for (unsigned k = 0; k < SquareNum; k++) {
			for (unsigned p1 = 0; p1 < fe_end; p1++) {
				for (unsigned p2 = 0; p2 < p1; p2++) {
					float val = KPP[kpptToLkpptnum(k, p1, p2, 0)];
					kppt::KPP[k][p1][p2][0] += val; kppt::KPP[k][p2][p1][0] += val;
					val = KPP[kpptToLkpptnum(k, p1, p2, 1)];
					kppt::KPP[k][p1][p2][1] += val; kppt::KPP[k][p2][p1][1] += val;
				}
			}
		}
		for (unsigned sk = 0; sk < SquareNum; sk++) {
			for (unsigned gk = 0; gk < SquareNum; gk++) {
				for (unsigned p = 0; p < fe_end; p++) {
					kppt::KKP[sk][gk][p][0] += KKP[kkptToLkkptnum(sk, gk, p, 0)];
					kppt::KKP[sk][gk][p][1] += KKP[kkptToLkkptnum(sk, gk, p, 1)];
				}
			}
		}
	}

	kppt_paramVector& kppt_paramVector::operator+=(const kppt_paramVector& rhs) {
		for (size_t i = 0; i < lkpptnum; i++) {
			KPP[i] += rhs.KPP[i];
		}
		for (size_t i = 0; i < lkkptnum; i++) {
			KKP[i] += rhs.KKP[i];
		}
	}
	kppt_paramVector& kppt_paramVector::operator+=(const fvpair& rhs) {
		for (size_t i = 0; i < lkpptnum; i++) {
			KPP[i] += rhs.f * rhs.v.KPP[i];
		}
		for (size_t i = 0; i < lkkptnum; i++) {
			KKP[i] += rhs.f * rhs.v.KKP[i];
		}
	}
	kppt_paramVector& kppt_paramVector::operator*=(const double c) {
		for (size_t i = 0; i < lkpptnum; i++) {
			KPP[i] *= c;
		}
		for (size_t i = 0; i < lkkptnum; i++) {
			KKP[i] *= c;
		}
	}
}