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

#ifndef CRAB_COLOR_H
#define CRAB_COLOR_H

#include "crab/XMLDoc.h"
#include "crab/common_header.h"
#include "crab/loaders.h"

namespace Crab {

namespace pyrodactyl {
namespace text {
// Storage pool for saving colors from one file, then using them

class ColorPool {

	std::vector<SDL_Color> pool;

	// Default invalid color
	SDL_Color invalid;

public:
	ColorPool() {
		pool.clear();
		invalid.r = 255;
		invalid.g = 0;
		invalid.b = 220;
	}

	SDL_Color &Get(const int &num);
	void Load(const std::string &filename);
};
} // End of namespace text
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_COLOR_H
