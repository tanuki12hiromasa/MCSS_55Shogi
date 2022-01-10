#include "kpp_kkpt_learn.h"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace kpp_kkpt {
	void kpp_kkpt_paramVector::EvalClamp(std::int16_t absmax) {
		absmax = std::abs(absmax);
		for (unsigned k = 0; k < SquareNum; k++) {
			for (unsigned p1 = 0; p1 < fe_end; p1++) {
				for (unsigned p2 = 0; p2 < fe_end; p2++) {
					auto& val = kpp_kkpt::KPP[k][p1][p2];
					if (val > absmax) {
						val = absmax;
					}
					else if (val < -absmax) {
						val = -absmax;
					}
				}
			}
		}
		for (unsigned sk = 0; sk < SquareNum; sk++) {
			for (unsigned gk = 0; gk < SquareNum; gk++) {
				for (unsigned p = 0; p < fe_end; p++) {
					auto& vec = kpp_kkpt::KKP[sk][gk][p];
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
				auto& vec = kpp_kkpt::KK[sk][gk];
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

	kpp_kkpt_paramVector::kpp_kkpt_paramVector() {
		KPP = new EvalVectorFloat[lkppnum];
		KKP = new EvalVectorFloat[lkkptnum];
		KK = new EvalVectorFloat[lkktnum];
		reset();
	}

	kpp_kkpt_paramVector::kpp_kkpt_paramVector(kpp_kkpt_paramVector&& rhs) noexcept :
		PieceScoreArr(std::move(rhs.PieceScoreArr))
	{
		KPP = rhs.KPP;
		KKP = rhs.KKP;
		KK = rhs.KK;
		rhs.KPP = nullptr;
		rhs.KKP = nullptr;
		rhs.KK = nullptr;
	}

	kpp_kkpt_paramVector& kpp_kkpt_paramVector::operator=(kpp_kkpt_paramVector&& rhs)noexcept {
		if(KPP) delete[] KPP;
		if(KKP) delete[] KKP;
		if(KK) delete[] KK;
		KPP = rhs.KPP;
		KKP = rhs.KKP;
		KK = rhs.KK;
		rhs.KPP = nullptr;
		rhs.KKP = nullptr;
		rhs.KK = nullptr;
		return *this;
	}

	kpp_kkpt_paramVector::~kpp_kkpt_paramVector() {
		delete[] KPP;
		delete[] KKP;
		delete[] KK;
	}

	void kpp_kkpt_paramVector::reset() {
		EvalVectorFloat* const kpp = KPP;
		for (int i = 0; i < lkppnum; i++) {
			kpp[i] = 0;
		}
		EvalVectorFloat* const kkp = KKP;
		for (int i = 0; i < lkkptnum; i++) {
			kkp[i] = 0;
		}
		for (auto& p : PieceScoreArr) {
			p = 0;
		}
	}

	inline void kpp_addGrad(EvalVectorFloat* const kpp, const int kpos, const int k, const int l,const float bg) {
		kpp[kppToLkppnum(kpos, k, l)] += bg;
		//kpp[kppToLkppnum(koma::mirrorX(kpos), mirror((EvalIndex)k), mirror((EvalIndex)l))] += bg;
	}
	inline void kkp_addGrad(EvalVectorFloat* const kkp, const int skpos, const int gkpos, const int k, const float bg, const float tg) {
		kkp[kkptToLkkptnum(skpos, gkpos, k, 0)] += bg;
		kkp[kkptToLkkptnum(skpos, gkpos, k, 1)] += tg;
		//kkp[kkptToLkkptnum(koma::mirrorX(skpos), koma::mirrorX(gkpos), mirror((EvalIndex)k), 0)] += bg;
		//kkp[kkptToLkkptnum(koma::mirrorX(skpos), koma::mirrorX(gkpos), mirror((EvalIndex)k), 1)] += tg;
	}
	inline void kk_addGrad(EvalVectorFloat* const kk, const int skpos, const int gkpos, const float bg, const float tg) {
		kk[kktToLkktnum(skpos, gkpos, 0)] += bg;
		kk[kktToLkktnum(skpos, gkpos, 1)] += tg;
	}
	void kpp_kkpt::kpp_kkpt_paramVector::piece_addGrad(const float scalar, const Kyokumen& kyokumen) {
		{ //歩
			const int fu = (int)kyokumen.getEachBB(koma::Koma::s_Fu).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_Fu).popcount();
			PieceScoreArr[0] += scalar * fu;
		}
		{ //銀
			const int gin = (int)kyokumen.getEachBB(koma::Koma::s_Gin).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_Gin).popcount();
			PieceScoreArr[1] += scalar * gin;
		}
		{ //角
			const int kaku = (int)kyokumen.getEachBB(koma::Koma::s_Kaku).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_Kaku).popcount();
			PieceScoreArr[2] += scalar * kaku;
		}
		{ //飛
			const int hi = (int)kyokumen.getEachBB(koma::Koma::s_Hi).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_Hi).popcount();
			PieceScoreArr[3] += scalar * hi;
		}
		{ //金
			const int kin = (int)kyokumen.getEachBB(koma::Koma::s_Kin).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_Kin).popcount();
			PieceScoreArr[4] += scalar * kin;
		}
		{ //と金
			const int nfu = (int)kyokumen.getEachBB(koma::Koma::s_nFu).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_nFu).popcount();
			PieceScoreArr[5] += scalar * nfu;
		}
		{ //成銀
			const int ngin = (int)kyokumen.getEachBB(koma::Koma::s_nGin).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_nGin).popcount();
			PieceScoreArr[6] += scalar * ngin;
		}
		{ //馬
			const int uma = (int)kyokumen.getEachBB(koma::Koma::s_nKaku).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_nKaku).popcount();
			PieceScoreArr[7] += scalar * uma;
		}
		{ //龍
			const int ryu = (int)kyokumen.getEachBB(koma::Koma::s_nHi).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_nHi).popcount();
			PieceScoreArr[8] += scalar * ryu;
		}
	}

	void kpp_kkpt_paramVector::addGrad(const float scalar,const SearchPlayer& player) {
		if (std::abs(scalar) < 0.00000001f) return;
		EvalVectorFloat* const kpp = KPP;
		EvalVectorFloat* const kkp = KKP;
		const unsigned skpos = player.kyokumen.sOuPos();
		const unsigned gkpos = player.kyokumen.gOuPos();
		const unsigned invgkpos = inverse(gkpos);
		const unsigned invskpos = inverse(skpos);
		const float bammenscalar = (player.kyokumen.teban()) ? scalar : -scalar;
		const float tebanscalar = scalar / 8.0;
		for (unsigned i = 0; i < EvalList::EvalListSize; ++i) {
			const int k0 = player.feature.idlist.list0[i];
			const int k1 = player.feature.idlist.list1[i];
			for (unsigned j = 0; j < i; j++) {
				const int l0 = player.feature.idlist.list0[j];
				const int l1 = player.feature.idlist.list1[j];

				kpp_addGrad(kpp, skpos, k0, l0, bammenscalar);
				kpp_addGrad(kpp, invgkpos, k1, l1, -bammenscalar);
			}
			kkp_addGrad(kkp, skpos, gkpos, k0, bammenscalar, tebanscalar);
			kkp_addGrad(kkp, invgkpos, invskpos, k1, -bammenscalar, tebanscalar);
		}
		kk_addGrad(KK, skpos, gkpos, bammenscalar, tebanscalar);
		kk_addGrad(KK, invgkpos, invskpos, -bammenscalar, tebanscalar);
		if(dynamicPieceScore) piece_addGrad(scalar, player.kyokumen);
	}

	void kpp_kkpt_paramVector::clamp(float absmax) {
		absmax = std::abs(absmax);
		for (size_t i = 0; i < lkppnum; i++) {
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
		for (size_t i = 0; i < lkktnum; i++) {
			if (KK[i] > absmax) {
				KK[i] = absmax;
			}
			else if (KK[i] < -absmax) {
				KK[i] = -absmax;
			}
		}
		for (size_t i = 0; i < lpiecenum; i++) {
			if (PieceScoreArr[i] > absmax) {
				PieceScoreArr[i] = absmax;
			}
			else if (PieceScoreArr[i] < -absmax) {
				PieceScoreArr[i] = -absmax;
			}
		}
	}

	//ベクトル中の最大値を1にするように補正する
	void kpp_kkpt_paramVector::normalize() {
		const auto max = abs_max_value();
		//std::cout << "absmax: " << max << std::endl;
		if (max != 0 && max < 1) {
			operator*=(1 / max);
		}
	}

	EvalVectorFloat kpp_kkpt_paramVector::abs_max_value() const {
		EvalVectorFloat max = 0;
		for (size_t i = 0; i < lkppnum; i++) {
			max = std::max(max, std::abs(KPP[i]));
		}
		for (size_t i = 0; i < lkkptnum; i++) {
			max = std::max(max, std::abs(KKP[i]));
		}
		for (size_t i = 0; i < lkktnum; i++) {
			max = std::max(max, std::abs(KK[i]));
		}
		return max;
	}

	void kpp_kkpt_paramVector::updateEval() {
		//KPPのテーブル形式の違いに注意する
		for (unsigned k = 0; k < SquareNum; k++) {
			for (unsigned p1 = 0; p1 < fe_end; p1++) {
				for (unsigned p2 = 0; p2 < p1; p2++) {
					const EvalElementTypeh val = KPP[kppToLkppnum(k, p1, p2)];
					kpp_kkpt::KPP[k][p1][p2] += val; kpp_kkpt::KPP[k][p2][p1] += val;
					KPP[kppToLkppnum(k, p1, p2)] -= val;
				}
			}
		}
		for (unsigned sk = 0; sk < SquareNum; sk++) {
			for (unsigned gk = 0; gk < SquareNum; gk++) {
				for (unsigned p = 0; p < fe_end; p++) {
					EvalElementTypeh val = KKP[kkptToLkkptnum(sk, gk, p, 0)];
					kpp_kkpt::KKP[sk][gk][p][0] += val;
					KKP[kkptToLkkptnum(sk, gk, p, 0)] -= val;
					val = KKP[kkptToLkkptnum(sk, gk, p, 1)];
					kpp_kkpt::KKP[sk][gk][p][1] += val;
					KKP[kkptToLkkptnum(sk, gk, p, 1)] -= val;
				}
			}
		}
		for (unsigned sk = 0; sk < SquareNum; sk++) {
			for (unsigned gk = 0; gk < SquareNum; gk++) {
				EvalElementTypeh val = KK[kktToLkktnum(sk, gk, 0)];
				kpp_kkpt::KK[sk][gk][0] += val;
				KK[kktToLkktnum(sk, gk, 0)] -= val;
				val = KK[kktToLkktnum(sk, gk, 1)];
				kpp_kkpt::KK[sk][gk][1] += val;
				KK[kktToLkktnum(sk, gk, 1)] -= val;
			}
		}
		if (dynamicPieceScore) {
			for (size_t i = 0; i < lpiecenum_plain; i++) {
				const PieceScoreType val = PieceScoreArr[i];
				kpp_kkpt::PieceScoreArr[i] += val;
				kpp_kkpt::PieceScoreArr[i + 10] -= val;
				PieceScoreArr[i] -= val;
			}
			//王の分を学習時に数えていないので、その分飛ばす
			for (size_t i = lpiecenum_plain; i < lpiecenum; i++) {
				const PieceScoreType val = PieceScoreArr[i];
				kpp_kkpt::PieceScoreArr[i + 1] += val;
				kpp_kkpt::PieceScoreArr[i + 11] -= val;
				PieceScoreArr[i] -= val;
			}
		}
	}

	kpp_kkpt_paramVector& kpp_kkpt_paramVector::operator+=(const kpp_kkpt_paramVector& rhs) {
		for (size_t i = 0; i < lkppnum; i++) {
			KPP[i] += rhs.KPP[i];
		}
		for (size_t i = 0; i < lkkptnum; i++) {
			KKP[i] += rhs.KKP[i];
		}
		for (size_t i = 0; i < lkktnum; i++) {
			KK[i] += rhs.KK[i];
		}
		if (dynamicPieceScore) {
			for (size_t i = 0; i < lpiecenum; i++) {
				PieceScoreArr[i] += rhs.PieceScoreArr[i];
			}
		}
		return *this;
	}
	kpp_kkpt_paramVector& kpp_kkpt_paramVector::operator+=(const fvpair& rhs) {
		for (size_t i = 0; i < lkppnum; i++) {
			KPP[i] += rhs.f * rhs.v.KPP[i];
		}
		for (size_t i = 0; i < lkkptnum; i++) {
			KKP[i] += rhs.f * rhs.v.KKP[i];
		}
		for (size_t i = 0; i < lkktnum; i++) {
			KK[i] += rhs.f * rhs.v.KK[i];
		}
		if (dynamicPieceScore) {
			for (size_t i = 0; i < lpiecenum; i++) {
				PieceScoreArr[i] += rhs.f * rhs.v.PieceScoreArr[i];
			}
		}
		return *this;
	}
	kpp_kkpt_paramVector& kpp_kkpt_paramVector::operator*=(const double c) {
		for (size_t i = 0; i < lkppnum; i++) {
			KPP[i] *= c;
		}
		for (size_t i = 0; i < lkkptnum; i++) {
			KKP[i] *= c;
		}
		for (size_t i = 0; i < lkktnum; i++) {
			KK[i] *= c;
		}
		if (dynamicPieceScore) {
			for (size_t i = 0; i < lpiecenum; i++) {
				PieceScoreArr[i] *= c;
			}
		}
		return *this;
	}

	void kpp_kkpt_paramVector::save(const std::string& path) {
		std::ofstream fs(path,std::ios::binary);
		if (!fs) {
			std::cerr << "error:file canot generate" << std::endl;
			return;
		}
		for (auto it = (char*)KPP, end = (char*)KPP + sizeof(KPPEvalVectorFloat); it < end; it += (1 << 30)) {
			size_t size = (it + (1 << 30) < end ? (1 << 30) : end - it);
			fs.write(it, size);
		}
		for (auto it = (char*)KKP, end = (char*)KKP + sizeof(KKPEvalVectorFloat); it < end; it += (1 << 30)) {
			size_t size = (it + (1 << 30) < end ? (1 << 30) : end - it);
			fs.write(it, size);
		}
		for (auto it = (char*)KK, end = (char*)KK + sizeof(KKEvalVectorFloat); it < end; it += (1 << 30)) {
			size_t size = (it + (1 << 30) < end ? (1 << 30) : end - it);
			fs.write(it, size);
		}
		if (dynamicPieceScore) {
			std::ofstream fs_p(path + "p", std::ios::binary);
			if (!fs) {
				std::cerr << "error:file(piece) canot generate" << std::endl;
				return;
			}
			for (const auto& p : PieceScoreArr) {
				fs_p.write((char*)&p, sizeof(p));
			}
		}
	}

	void kpp_kkpt_paramVector::load(const std::string& path) {
		std::ifstream fs(path, std::ios::binary);
		if (!fs) {
			std::cerr << "error:file canot open" << std::endl;
			return;
		}
		for (auto it = (char*)KPP, end = (char*)KPP + sizeof(KPPEvalVectorFloat); it < end; it += (1 << 30)) {
			size_t size = (it + (1 << 30) < end ? (1 << 30) : end - it);
			fs.read(it, size);
		}
		for (auto it = (char*)KKP, end = (char*)KKP + sizeof(KKPEvalVectorFloat); it < end; it += (1 << 30)) {
			size_t size = (it + (1 << 30) < end ? (1 << 30) : end - it);
			fs.read(it, size);
		}
		for (auto it = (char*)KK, end = (char*)KK + sizeof(KKEvalVectorFloat); it < end; it += (1 << 30)) {
			size_t size = (it + (1 << 30) < end ? (1 << 30) : end - it);
			fs.read(it, size);
		}
		if (dynamicPieceScore) {
			std::ifstream fs_p(path + "p", std::ios::binary);
			if (!fs) {
				std::cerr << "error:file(piece) canot open" << std::endl;
				return;
			}
			for (const auto& p : PieceScoreArr) {
				fs_p.read((char*)&p, sizeof(p));
			}
		}
	}

	void kpp_kkpt_paramVector::print(const double displaymin,int option)const {
		using namespace std;
		if (option == 0 || option == 1) {
			cout << "show kpp" << endl;
			for (int i = 0; i < kpp_kkpt::SquareNum; i++) {
				for (int j = 0; j < kpp_kkpt::fe_end; j++) {
					for (int k = 0; k < j; k++) {
						if (j == k)continue;
						if (std::abs(KPP[kpp_kkpt::kppToLkppnum(i, j, k)]) > displaymin) {
							cout << "kpp " << i << " " << j << " " << k << ": ";
							cout << KPP[kpp_kkpt::kppToLkppnum(i, j, k)] << "\n";
						}
					}
				}
			}
		}
		else if(option == 0 || option == 2) {
			cout << "show kkp" << endl;
			for (int i = 0; i < kpp_kkpt::SquareNum; i++) {
				for (int j = 0; j < kpp_kkpt::SquareNum; j++) {
					for (int k = 0; k < kpp_kkpt::fe_end; k++) {
						if (std::abs(KKP[kpp_kkpt::kkptToLkkptnum(i, j, k, 0)]) > displaymin || std::abs(KKP[kpp_kkpt::kkptToLkkptnum(i, j, k, 1)]) > displaymin) {
							cout << "kkp " << i << " " << j << " " << k << ": ";
							cout << KKP[kpp_kkpt::kkptToLkkptnum(i, j, k, 0)] << " " << KKP[kpp_kkpt::kkptToLkkptnum(i, j, k, 1)] << "\n";
						}
					}
				}
			}
		}
		else if(option == 0 || option == 3) {
			cout << "show kk" << endl;
			for (int i = 0; i < kpp_kkpt::SquareNum; i++) {
				for (int j = 0; j < kpp_kkpt::SquareNum; j++) {
					if (std::abs(KK[kpp_kkpt::kktToLkktnum(i, j, 0)]) > displaymin || std::abs(KKP[kpp_kkpt::kktToLkktnum(i, j, 1)]) > displaymin) {
						cout << "kk " << i << " " << j << ": ";
						cout << KK[kpp_kkpt::kktToLkktnum(i, j, 0)] << " " << KK[kpp_kkpt::kktToLkktnum(i, j, 1)] << "\n";
					}
				}
			}
		}
		if(dynamicPieceScore){
			cout << "show Piece" << endl;
			for (int i = 0; i < lpiecenum; i++) {
				cout << i << ":" << PieceScoreArr[i] << " ";
			}
			cout << endl;
		}
	}

	bool kpp_kkpt_paramVector::operator==(const kpp_kkpt_paramVector& rhs)const {
		for (size_t i = 0; i < lkppnum; i++) {
			if (KPP[i] != rhs.KPP[i]) return false;
		}
		for (size_t i = 0; i < lkkptnum; i++) {
			if (KKP[i] != rhs.KKP[i]) return false;
		}
		for (size_t i = 0; i < lkktnum; i++) {
			if (KK[i] != rhs.KK[i]) return false;
		}
		if (dynamicPieceScore && (PieceScoreArr != rhs.PieceScoreArr)) { 
			return false; 
		}
		return true;
	}

	void Adam::updateEval(kpp_kkpt_paramVector& dw) {
		t++;
		const auto b1t = std::pow(b1, t);
		const auto b2t = std::pow(b2, t);
		//KPP
		for (unsigned k = 0; k < SquareNum; k++) {
			for (unsigned p1 = 0; p1 < fe_end; p1++) {
				for (unsigned p2 = 0; p2 < p1; p2++) {	
					const auto idx = kppToLkppnum(k, p1, p2);
					const auto val = dw.KPP[idx];
					mt.KPP[idx] = mt.KPP[idx] * b1 + (1 - b1) * val;
					vt.KPP[idx] = vt.KPP[idx] * b2 + (1 - b2) * val * val;
					const double dwt = -alpha * (mt.KPP[idx] / (1 - b1t)) / (std::sqrt(vt.KPP[idx] / (1 - b2t)) + epsilon);
					kpp_kkpt::KPP[k][p1][p2] += dwt; kpp_kkpt::KPP[k][p2][p1] += dwt;
					/*if (std::abs(dwt) > 0) {
						std::cout << "t:"<< t << " val:" << val;
						std::cout << " mt,vt:" << mt.KPP[idx] << "," << vt.KPP[idx];
						std::cout << " dwt:" << dwt;
						std::cout << "\n";
					}*/
					dw.KPP[idx] = 0;
				}
			}
		}
		//KKPT
		for (unsigned sk = 0; sk < SquareNum; sk++) {
			for (unsigned gk = 0; gk < SquareNum; gk++) {
				for (unsigned p = 0; p < fe_end; p++) {
					{
						const auto idx = kkptToLkkptnum(sk, gk, p, 0);
						const auto val = dw.KKP[idx];
						mt.KKP[idx] = mt.KKP[idx] * b1 + (1 - b1) * val;
						vt.KKP[idx] = vt.KKP[idx] * b2 + (1 - b2) * val * val;
						const double dwt = -alpha * (mt.KKP[idx] / (1 - b1t)) / (std::sqrt(vt.KKP[idx] / (1 - b2t)) + epsilon);
						kpp_kkpt::KKP[sk][gk][p][0] += dwt;
						/*if (std::abs(dwt) > 0) {
							std::cout << "t:" << t << " val:" << val;
							std::cout << " mt,vt:" << mt.KKP[idx] << "," << vt.KKP[idx];
							std::cout << " dwt:" << std::fixed << std::setprecision(6) << dwt;
							std::cout << "\n";
						}*/
						dw.KKP[idx] = 0;
					}
					{
						const auto idx = kkptToLkkptnum(sk, gk, p, 1);
						const auto val = dw.KKP[idx];
						mt.KKP[idx] = mt.KKP[idx] * b1 + (1 - b1) * val;
						vt.KKP[idx] = vt.KKP[idx] * b2 + (1 - b2) * val * val;
						const double dwt = -alpha * (mt.KKP[idx] / (1 - b1t)) / (std::sqrt(vt.KKP[idx] / (1 - b2t)) + epsilon);
						kpp_kkpt::KKP[sk][gk][p][1] += dwt;
						dw.KKP[idx] = 0;
					}
				}
			}
		}
		//KKT
		for (unsigned sk = 0; sk < SquareNum; sk++) {
			for (unsigned gk = 0; gk < SquareNum; gk++) {
				{
					const auto idx = kktToLkktnum(sk, gk, 0);
					const auto val = dw.KK[idx];
					mt.KK[idx] = mt.KK[idx] * b1 + (1 - b1) * val;
					vt.KK[idx] = vt.KK[idx] * b2 + (1 - b2) * val * val;
					const double dwt = -alpha * (mt.KK[idx] / (1 - b1t)) / (std::sqrt(vt.KK[idx] / (1 - b2t)) + epsilon);
					kpp_kkpt::KK[sk][gk][0] += dwt;
					dw.KK[idx] = 0;
				}
				{
					const auto idx = kktToLkktnum(sk, gk, 1);
					const auto val = dw.KK[idx];
					mt.KK[idx] = mt.KK[idx] * b1 + (1 - b1) * val;
					vt.KK[idx] = vt.KK[idx] * b2 + (1 - b2) * val * val;
					const double dwt = -alpha * (mt.KK[idx] / (1 - b1t)) / (std::sqrt(vt.KK[idx] / (1 - b2t)) + epsilon);
					kpp_kkpt::KK[sk][gk][1] += dwt;
					dw.KK[idx] = 0;
				}

			}
		}
		if (dynamicPieceScore) {
			for (size_t i = 0; i < lpiecenum; i++) {
				const auto val = dw.PieceScoreArr[i];
				mt.PieceScoreArr[i] = mt.PieceScoreArr[i] * b1 + (1 - b1) * val;
				vt.PieceScoreArr[i] = vt.PieceScoreArr[i] * b2 + (1 - b2) * val * val;
				const double dwt = -alpha * (mt.PieceScoreArr[i] / (1 - b1t)) / (std::sqrt(vt.PieceScoreArr[i] / (1 - b2t)) + epsilon);
				kpp_kkpt::PieceScoreArr[i] += dwt;
				dw.PieceScoreArr[i] = 0;
			}
		}
	}

	void Adam::save(const std::string& path) {
		std::filesystem::create_directories(path);
		{
			std::ofstream fs(path + "/adam.bin");
			fs << t << std::endl;
		}
		mt.save(path + "/mt.bin");
		vt.save(path + "/vt.bin");
	}
	void Adam::load(const std::string& path) {
		{
			std::ifstream fs(path + "/adam.bin");
			fs >> t;
		}
		mt.load(path + "/mt.bin");
		vt.load(path + "/vt.bin");
	}
}