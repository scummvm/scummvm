#pragma once
#include "common_header.h"

#include "ImageManager.h"

enum LevelResultType { LR_NONE, LR_LEVEL, LR_GAMEOVER };

struct LevelResult
{
	LevelResultType type;
	std::string val;
	int x,y;

	LevelResult() : val("")
	{
		type = LR_NONE;
		x = -1;
		y = -1;
	}
};