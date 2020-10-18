#pragma once
#include "player.h"

#include "kppt_evaluate.h"
#include "kppt_feature.h"

#include "kkppt_evaluate.h"
#include "kkppt_feature.h"

//kppt
/*
using Evaluator = kppt::kppt_evaluator;
using Feature = kppt::kppt_feat;
using FeatureCache = kppt::EvalSum;
*/

//kkppt
using Evaluator = kkppt::kkppt_evaluator;
using Feature = kkppt::kkppt_feat;
using FeatureCache = kkppt::EvalSum;

using SearchPlayer = Player<Feature, FeatureCache>;
