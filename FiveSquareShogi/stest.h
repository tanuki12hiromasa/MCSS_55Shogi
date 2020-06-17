#pragma once
#include "commander.h"
#include "move_gen.h"
#include "kppt_learn.h"
#include "learner.h"

class ShogiTest {
	using strv = std::vector<std::string>;
public:
	static void test();
	static void checkGenMove(const std::string& usipos);
};