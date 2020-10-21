#include "kkppt_learn.h"
#include <iostream>
#include <fstream>

namespace kkppt {
	void kkppt_paramVector::EvalClamp(std::int16_t absmax) {
		absmax = std::abs(absmax);
		for (unsigned sk = 0; sk < SquareNum; sk++) {
			for (unsigned gk = 0; gk < SquareNum; gk++) {
				for (unsigned p1 = 0; p1 < fe_end; p1++) {
					for (unsigned p2 = 0; p2 < fe_end; p2++) {
						auto& vec = kkppt::KKPP[sk][gk][p1][p2];
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
	}

	kkppt_paramVector::kkppt_paramVector() {
		KKPP = new EvalVectorFloat[lkkpptnum];
		reset();
	}

	kkppt_paramVector::kkppt_paramVector(kkppt_paramVector&& rhs)noexcept {
		KKPP = rhs.KKPP;
		rhs.KKPP = nullptr;
	}

	kkppt_paramVector& kkppt_paramVector::operator=(kkppt_paramVector&& rhs)noexcept {
		delete[] KKPP;
		KKPP = rhs.KKPP;
		rhs.KKPP = nullptr;
		return *this;
	}

	kkppt_paramVector::~kkppt_paramVector() {
		delete[] KKPP;
	}

	void kkppt_paramVector::reset() {
		const auto kkpp = KKPP;
		for (int i = 0; i < lkkpptnum; i++) {
			kkpp[i] = 0;
		}
	}

	void kkppt_paramVector::addGrad(const float scalar, const SearchPlayer& player) {
		if (std::abs(scalar) < 0.00000001f) return;
		const auto kkpp = KKPP;
		const unsigned skpos = player.kyokumen.sOuPos();
		const unsigned gkpos = player.kyokumen.gOuPos();
		const unsigned invgkpos = inverse(gkpos);
		const unsigned invskpos = inverse(skpos);
		const float bammenscalar = (player.kyokumen.teban()) ? scalar : -scalar;
		const float tebanscalar = scalar;
		for (unsigned i = 0; i < EvalList::EvalListSize; i++) {
			const int k0 = player.feature.idlist.list0[i];
			for (unsigned j = 0; j < i; j++) {
				const int l0 = player.feature.idlist.list0[j];
				kkpp[kkpptToLkkpptnum(skpos, gkpos, k0, l0, 0)] += bammenscalar;
				kkpp[kkpptToLkkpptnum(skpos, gkpos, k0, l0, 1)] += tebanscalar;
			}
		}
	}

	void kkppt_paramVector::clamp(float absmax) {
		absmax = std::abs(absmax);
		for (size_t i = 0; i < lkkpptnum; i++) {
			if (KKPP[i] > absmax) {
				KKPP[i] = absmax;
			}
			else if (KKPP[i] < -absmax) {
				KKPP[i] = -absmax;
			}
		}
	}

	void kkppt_paramVector::updateEval() {
		clamp(2000);
		EvalClamp(30000);
		for (unsigned sk = 0; sk < SquareNum; sk++) {
			for (unsigned gk = 0; gk < SquareNum; gk++) {
				for (unsigned p1 = 0; p1 < fe_end; p1++) {
					for (unsigned p2 = 0; p2 < p1; p2++) {
						const auto val0 = KKPP[kkpptToLkkpptnum(sk, gk, p1, p2, 0)];
						kkppt::KKPP[sk][gk][p1][p2][0] += val0;
						kkppt::KKPP[sk][gk][p2][p1][0] += val0;
						KKPP[kkpptToLkkpptnum(sk, gk, p1, p2, 0)] -= val0;
						const auto val1 = KKPP[kkpptToLkkpptnum(sk, gk, p1, p2, 1)];
						kkppt::KKPP[sk][gk][p1][p2][1] += val1;
						kkppt::KKPP[sk][gk][p2][p1][1] += val1;
						KKPP[kkpptToLkkpptnum(sk, gk, p1, p2, 1)] -= val1;
					}
				}
			}
		}
	}

	kkppt_paramVector& kkppt_paramVector::operator+=(const kkppt_paramVector& rhs) {
		for (size_t i = 0; i < lkkpptnum; i++) {
			KKPP[i] += rhs.KKPP[i];
		}
		return *this;
	}
	kkppt_paramVector& kkppt_paramVector::operator+=(const fvpair& rhs) {
		for (size_t i = 0; i < lkkpptnum; i++) {
			KKPP[i] += rhs.f * rhs.v.KKPP[i];
		}
		return *this;
	}
	kkppt_paramVector& kkppt_paramVector::operator*=(const double c) {
		for (size_t i = 0; i < lkkpptnum; i++) {
			KKPP[i] *= c;
		}
		return *this;
	}

	void kkppt_paramVector::save(const std::string& path) {
		std::ofstream fs(path, std::ios::binary);
		if (!fs) {
			std::cerr << "error:file canot generate" << std::endl;
			return;
		}
		fs.write(reinterpret_cast<char*>(KKPP), sizeof(KKPPEvalVectorFloat));
	}

	void kkppt_paramVector::load(const std::string& path) {
		std::ifstream fs(path, std::ios::binary);
		if (!fs) {
			std::cerr << "error:file canot open" << std::endl;
			return;
		}
		fs.read(reinterpret_cast<char*>(KKPP), sizeof(KKPPEvalVectorFloat));
	}


	void kkppt_paramVector::showLearnVec(const double displaymin,int dummy)const {
		using namespace std;
		cout << "show kkpp" << endl;
		for (int sk = 0; sk < SquareNum; sk++) {
			for (int gk = 0; gk < SquareNum; gk++) {
				for (int p1 = 0; p1 < fe_end; p1++) {
					for (int p2 = 0; p2 < p1; p2++) {
						if (p1 == p2)continue;
						if (std::abs(KKPP[kkpptToLkkpptnum(sk,gk,p1,p2,0)]) > displaymin ||
							std::abs(KKPP[kkpptToLkkpptnum(sk,gk,p1,p2, 1)]) > displaymin) {
							cout << "kkpp " << sk << " " << gk << " " << p1 << " " << p2 << ": ";
							cout << KKPP[kkpptToLkkpptnum(sk, gk, p1, p2, 0)] << " " << KKPP[kkpptToLkkpptnum(sk, gk, p1, p2, 1)] << "\n";
						}
					}
				}
			}
		}
	}


}