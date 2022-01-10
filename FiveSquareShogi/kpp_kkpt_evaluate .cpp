#include "stdafx.h"
#include "kpp_kkpt_evaluate.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace kpp_kkpt {

	std::string kpp_kkpt_evaluator::ifolderpath = "data/kpp_kkpt";
	std::string kpp_kkpt_evaluator::ofolderpath = "data/kpp_kkpt";

	void kpp_kkpt_evaluator::init() {
		kpp_kkpt_feat::init(ifolderpath);
	}

	void kpp_kkpt_evaluator::save() {
		std::filesystem::create_directories(ofolderpath);
		kpp_kkpt_feat::save(ofolderpath);
	}
	
	void kpp_kkpt_evaluator::save(const std::string& path) {
		kpp_kkpt_feat::save(path);
	}

	double kpp_kkpt_evaluator::evaluate(const SearchPlayer& player) {
		return (double)player.feature.sum.sum(player.kyokumen.teban()) / FVScale;
	}

	double kpp_kkpt_evaluator::evaluate(const SearchPlayer& player, bool jiteban) {
		if (jiteban) {
			return (double)player.feature.sum.sum(player.kyokumen.teban()) / FVScale;
		}
		else {
			return -(double)player.feature.sum.sum(!player.kyokumen.teban()) / FVScale;
		}
	}

	void kpp_kkpt_evaluator::genFirstEvalFile(const std::string& folderpath) {
		std::filesystem::create_directories(folderpath);
		auto* KPP = new KPPEvalElementType1[SquareNum];
		auto* KKP = new KKPEvalElementType1[SquareNum];
		auto* KK = new KKEvalElementType0[SquareNum];
		memset(KPP, 0, sizeof(KPPEvalElementType1) * (size_t)SquareNum);
		memset(KKP, 0, sizeof(KKPEvalElementType1) * (size_t)SquareNum);
		memset(KK, 0, sizeof(KKEvalElementType0) * (size_t)SquareNum);
		{
			std::ofstream fs(folderpath + "/KPP.bin", std::ios::binary);
			if (!fs) {
				std::cerr << "error:file(KPP.bin) cannot make" << std::endl;
				return;
			}
			auto end = (char*)KPP + sizeof(KPPEvalElementType2);
			for (auto it = (char*)KPP; it < end; it += (1 << 30)) {
				size_t size = (it + (1 << 30) < end ? (1 << 30) : end - it);
				fs.write(it, size);
			}
		}
		{
			std::ofstream fs(folderpath + "/KKP.bin", std::ios::binary);
			if (!fs) {
				std::cerr << "error:file(KKP.bin) cannot make" << std::endl;
				return;
			}
			auto end = (char*)KKP + sizeof(KKPEvalElementType2);
			for (auto it = (char*)KKP; it < end; it += (1 << 30)) {
				size_t size = (it + (1 << 30) < end ? (1 << 30) : end - it);
				fs.write(it, size);
			}
		}
		{
			std::ofstream fs(folderpath + "/KK.bin", std::ios::binary);
			if (!fs) {
				std::cerr << "error:file(KK.bin) cannot make" << std::endl;
				return;
			}
			auto end = (char*)KK + sizeof(KKEvalElementType1);
			for (auto it = (char*)KK; it < end; it += (1 << 30)) {
				size_t size = (it + (1 << 30) < end ? (1 << 30) : end - it);
				fs.write(it, size);
			}
		}
		if (dynamicPieceScore) {
			const std::array<PieceScoreType, static_cast<size_t>(koma::Koma::KomaNum)> PieceScoreArr = { 0 };
			std::ofstream fs(folderpath + "/Piece.bin", std::ios::binary);
			if (fs) {
				for (auto& p : PieceScoreArr) {
					fs.write((char*)&p, sizeof(p));
				}
			}
			else {
				std::cerr << "error:file(" << folderpath << "/Piece.bin) cannot open" << std::endl;
			}
		}
	}

	void kpp_kkpt_evaluator::print(int option) {
		using namespace std;
		if (option == 0 || option == 1) {
			cout << "show kpp" << endl;
			for (int i = 0; i < kpp_kkpt::SquareNum; i++) {
				for (int j = 0; j < kpp_kkpt::fe_end; j++) {
					for (int k = 0; k < j; k++) {
						if (j == k)continue;
						if(KPP[i][j][k]!=0)
							cout << i << "," << j << "," << k << ": " << KPP[i][j][k]  << "\n";
					}
				}
			}
		}
		else if(option == 0 || option == 2) {
			cout << "show kkp" << endl;
			for (int i = 0; i < kpp_kkpt::SquareNum; i++) {
				for (int j = 0; j < kpp_kkpt::SquareNum; j++) {
					for (int k = 0; k < kpp_kkpt::fe_end; k++) {
						if (KKP[i][j][k][0] != 0 || KKP[i][j][k][1] != 0)
							cout << i << "," << j << "," << k << ": " << KKP[i][j][k][0] << " " << KKP[i][j][k][1] << "\n";
					}
				}
			}
		}
		else if (option == 0 || option == 3) {
			cout << "show kk" << endl;
			for (int i = 0; i < kpp_kkpt::SquareNum; i++) {
				for (int j = 0; j < kpp_kkpt::SquareNum; j++) {
					if (KK[i][j][0] != 0 || KK[i][j][1] != 0) {
						cout << i << "," << j << ": " << KK[i][j][0] << " " << KK[i][j][1] << "\n";
					}
				}
			}
		}
		if (dynamicPieceScore) {
			cout << "show piece score\n";
			for (int i = 0; i < static_cast<size_t>(koma::Koma::KomaNum); i++) {
				cout << i << ": " << kpp_kkpt::PieceScoreArr[i] << "\n";
			}
		}
	}
}