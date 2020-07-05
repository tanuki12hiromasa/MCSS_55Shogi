﻿#include "kppt_learn.h"
#include <iostream>

namespace kppt {
	void kppt_paramVector::EvalClamp(std::int16_t absmax) {
		absmax = std::abs(absmax);
		for (unsigned k = 0; k < SquareNum; k++) {
			for (unsigned p1 = 0; p1 < fe_end; p1++) {
				for (unsigned p2 = 0; p2 < fe_end; p2++) {
					auto& vec = kppt::KPP[k][p1][p2];
					if (vec[0] > absmax) {
						vec[0] = absmax;
					}
					else if (vec[0] < -absmax) {
						vec[0] = -absmax;
					}
					if (vec[1] > absmax) {
						vec[1] = absmax;
					}
					else if (vec[1] < -absmax) {
						vec[1] = -absmax;
					}
				}
			}
		}
		for (unsigned sk = 0; sk < SquareNum; sk++) {
			for (unsigned gk = 0; gk < SquareNum; gk++) {
				for (unsigned p = 0; p < fe_end; p++) {
					auto& vec = kppt::KPP[sk][gk][p];
					if (vec[0] > absmax) {
						vec[0] = absmax;
					}
					else if (vec[0] < -absmax) {
						vec[0] = -absmax;
					}
					if (vec[1] > absmax) {
						vec[1] = absmax;
					}
					else if (vec[1] < -absmax) {
						vec[1] = -absmax;
					}
				}
			}
		}
	}

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

	void kppt_paramVector::addGrad(const float scalar,const SearchPlayer& player,bool rootTeban) {
		if (std::abs(scalar) < 0.00000001f) return;
		EvalVectorFloat* const kpp = KPP;
		EvalVectorFloat* const kkp = KKP;
		const unsigned skpos = player.kyokumen.sOuPos();
		const unsigned gkpos = player.kyokumen.gOuPos();
		const unsigned invgkpos = inverse(gkpos);
		const unsigned invskpos = inverse(skpos);
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

	void kppt_paramVector::clamp(float absmax) {
		absmax = std::abs(absmax);
		for (size_t i = 0; i < lkpptnum; i++) {
			if (KPP[i] > absmax) {
				KPP[i] = absmax;
			}
			else if (KPP[i] < -absmax) {
				KPP[i] = -absmax;
			}
		}
		for (size_t i = 0; i < lkkptnum; i++) {
			if (KKP[i] > absmax) {
				KKP[i] = absmax;
			}
			else if (KKP[i] < -absmax) {
				KKP[i] = -absmax;
			}
		}
	}

	void kppt_paramVector::updateEval() {
		clamp(1000);
		EvalClamp(30000);
		//KPPのテーブル形式の違いに注意する
		for (unsigned k = 0; k < SquareNum; k++) {
			for (unsigned p1 = 0; p1 < fe_end; p1++) {
				for (unsigned p2 = 0; p2 < p1; p2++) {
					int16_t val = KPP[kpptToLkpptnum(k, p1, p2, 0)];
					kppt::KPP[k][p1][p2][0] += val; kppt::KPP[k][p2][p1][0] += val;
					KPP[kpptToLkpptnum(k, p1, p2, 0)] -= val;
					val = KPP[kpptToLkpptnum(k, p1, p2, 1)];
					kppt::KPP[k][p1][p2][1] += val; kppt::KPP[k][p2][p1][1] += val;
					KPP[kpptToLkpptnum(k, p1, p2, 1)] -= val;
				}
			}
		}
		for (unsigned sk = 0; sk < SquareNum; sk++) {
			for (unsigned gk = 0; gk < SquareNum; gk++) {
				for (unsigned p = 0; p < fe_end; p++) {
					int16_t val = KKP[kkptToLkkptnum(sk, gk, p, 0)];
					kppt::KKP[sk][gk][p][0] += val;
					KKP[kkptToLkkptnum(sk, gk, p, 0)] -= val;
					val = KKP[kkptToLkkptnum(sk, gk, p, 1)];
					kppt::KKP[sk][gk][p][1] += val;
					KKP[kkptToLkkptnum(sk, gk, p, 1)] -= val;
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
		return *this;
	}
	kppt_paramVector& kppt_paramVector::operator+=(const fvpair& rhs) {
		for (size_t i = 0; i < lkpptnum; i++) {
			KPP[i] += rhs.f * rhs.v.KPP[i];
		}
		for (size_t i = 0; i < lkkptnum; i++) {
			KKP[i] += rhs.f * rhs.v.KKP[i];
		}
		return *this;
	}
	kppt_paramVector& kppt_paramVector::operator*=(const double c) {
		for (size_t i = 0; i < lkpptnum; i++) {
			KPP[i] *= c;
		}
		for (size_t i = 0; i < lkkptnum; i++) {
			KKP[i] *= c;
		}
		return *this;
	}

	void kppt_paramVector::showLearnVec_kppt(const double displaymin)const {
		using namespace std;
		for (int i = 0; i < kppt::SquareNum; i++) {
			for (int j = 0; j < kppt::fe_end; j++) {
				for (int k = 0; k < j; k++) {
					if (j == k)continue;
					if (std::abs(KPP[kppt::kpptToLkpptnum(i, j, k, 0)]) > displaymin || std::abs(KPP[kppt::kpptToLkpptnum(i, j, k, 1)]) > displaymin) {
						cout << "kpp " << i << " " << j << " " << k << ": ";
						cout << KPP[kppt::kpptToLkpptnum(i, j, k, 0)] << " " << KPP[kppt::kpptToLkpptnum(i, j, k, 1)] << "\n";
					}
				}
			}
		}
	}

	void kppt_paramVector::showLearnVec_kkpt(const double displaymin)const {
		using namespace std;
		for (int i = 0; i < kppt::SquareNum; i++) {
			for (int j = 0; j < i; j++) {
				for (int k = 0; k < kppt::fe_end; k++) {
					if (std::abs(KKP[kppt::kkptToLkkptnum(i, j, k, 0)]) > displaymin || std::abs(KKP[kppt::kkptToLkkptnum(i, j, k, 1)]) > displaymin) {
						cout << "kkp " << i << " " << j << " " << k << ": ";
						cout << KKP[kppt::kkptToLkkptnum(i, j, k, 0)] << " " << KKP[kppt::kkptToLkkptnum(i, j, k, 1)] << "\n";
					}
				}
			}
		}
	}
}