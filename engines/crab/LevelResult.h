/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_LEVELRESULT_H
#define CRAB_LEVELRESULT_H
#include "crab/common_header.h"

#include "crab/image/ImageManager.h"

namespace Crab {

enum LevelResultType { LR_NONE,
					   LR_LEVEL,
					   LR_GAMEOVER };

struct LevelResult {
	LevelResultType type;
	Common::String val;
	int x, y;

	LevelResult() : val("") {
		type = LR_NONE;
		x = -1;
		y = -1;
	}
};

} // End of namespace Crab

#endif // CRAB_LEVELRESULT_H
