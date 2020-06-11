#include "kppt_learn.h"

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
		for (int i = 0; i < lkpptnum; i++) {
			kpp[i] = 0;
		}
		EvalVectorFloat* const kkp = KKP;
		for (int i = 0; i < lkkptnum; i++) {
			kkp[i] = 0;
		}
		const unsigned skpos = player.kyokumen.sOuPos();
		const unsigned gkpos = player.kyokumen.gOuPos();
		const unsigned invskpos = inverse(skpos);
		const unsigned invgkpos = inverse(gkpos);
		auto* ppskpp = KPP[skpos];
		auto* ppgkpp = KPP[inverse(gkpos)];
		if (player.kyokumen.teban()) {
			for (int i = 0; i < EvalList::EvalListSize; ++i) {
				const int k0 = player.feature.idlist.list0[i];
				const int k1 = player.feature.idlist.list1[i];
				auto* pskpp = ppskpp[k0];
				auto* pgkpp = ppgkpp[k1];
				for (int j = 0; j < i; j++) {
					const int l0 = player.feature.idlist.list0[j];
					const int l1 = player.feature.idlist.list1[j];
					pskpp[l0][0] += 1; pskpp[l0][1] += 1;
					pgkpp[l1][0] -= 1; pgkpp[l1][1] += 1;
				}
				KKP[skpos][gkpos][k0][0] += 1;
				KKP[gkpos][skpos][k1][0] -= 1;
				KKP[skpos][gkpos][k0][1] += 1;
				KKP[gkpos][skpos][k1][1] += 1;
			}
		}
		else {
			for (int i = 0; i < EvalList::EvalListSize; ++i) {
				const int k0 = player.feature.idlist.list0[i];
				const int k1 = player.feature.idlist.list1[i];
				auto* pskpp = ppskpp[k0];
				auto* pgkpp = ppgkpp[k1];
				for (int j = 0; j < EvalList::EvalListSize; j++) {
					if (i == j)continue;
					const int l0 = player.feature.idlist.list0[j];
					const int l1 = player.feature.idlist.list1[j];
					pskpp[l0][0] -= 1; pskpp[l0][1] += 1;
					pgkpp[l1][0] += 1; pgkpp[l1][1] += 1;
				}
				KKP[skpos][gkpos][k0][0] -= 1;
				KKP[gkpos][skpos][k1][0] += 1;
				KKP[skpos][gkpos][k0][1] += 1;
				KKP[gkpos][skpos][k1][1] += 1;
			}
		}
	}

	void kppt_paramVector::updateEval()const {
		//KPPのテーブル形式の違いに注意する

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