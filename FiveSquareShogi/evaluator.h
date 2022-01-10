#pragma once
#include "player.h"

#include "kppt_evaluate.h"
#include "kppt_feature.h"

#include "kkppt_evaluate.h"
#include "kkppt_feature.h"

#include "kpp_kkpt_evaluate.h"
#include "kpp_kkpt_feature.h"

#include "kpp_evaluate.h"
#include "kpp_feature.h"

//ここで使用する評価関数を切り替える
#define USE_KPPT
//#define USE_KKPPT
//#define USE_KPP_KKPT
//#define USE_KPP

//kppt
#ifdef  USE_KPPT
using Evaluator = kppt::kppt_evaluator;
using Feature = kppt::kppt_feat;
using FeatureCache = kppt::EvalSum;
#endif //  USE_KPPT

//kkppt
#ifdef USE_KKPPT
using Evaluator = kkppt::kkppt_evaluator;
using Feature = kkppt::kkppt_feat;
using FeatureCache = kkppt::EvalSum;
#endif 

//kpp_kkpt
#ifdef USE_KPP_KKPT
using Evaluator = kpp_kkpt::kpp_kkpt_evaluator;
using Feature = kpp_kkpt::kpp_kkpt_feat;
using FeatureCache = kpp_kkpt::EvalSum;
#endif

//kpp
#ifdef USE_KPP
using Evaluator = kpp::kpp_evaluator;
using Feature = kpp::kpp_feat;
using FeatureCache = kpp::EvalSum;
#endif

using SearchPlayer = Player<Feature, FeatureCache>;
