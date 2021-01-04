#pragma once
#include "player.h"

#include "kppt_evaluate.h"
#include "kppt_feature.h"

#include "kkppt_evaluate.h"
#include "kkppt_feature.h"

//ここで使用する評価関数を切り替える
#define USE_KPPT
//#define USE_KKPPT

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

using SearchPlayer = Player<Feature, FeatureCache>;
