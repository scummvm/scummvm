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

#ifndef CRAB_TEXTCOLOR_H
#define CRAB_TEXTCOLOR_H

#include "common/array.h"
#include "crab/loaders.h"

namespace Crab {

namespace pyrodactyl {
namespace text {
// Storage pool for saving colors from one file, then using them

class ColorPool {

	Common::Array<Color> _pool;

	// Default invalid color
	Color _invalid;

public:
	ColorPool() {
		_pool.clear();
		_invalid.r = 255;
		_invalid.g = 0;
		_invalid.b = 220;
		_invalid.a = 255;
	}

	Color &get(const int &num);
	void load(const Common::Path &filename);
};
} // End of namespace text
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_TEXTCOLOR_H
