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

#ifndef ULTIMA8_WORLD_COORDUTILS_H
#define ULTIMA8_WORLD_COORDUTILS_H

#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

/**
 * Convert from c++ code coordinates to usecode coordinates
 */
template<typename T>
static inline void World_ToUsecodeXY(T &x, T &y) {
	if (GAME_IS_CRUSADER) {
		x /= 2;
		y /= 2;
	}
}

/**
 * Convert from usecode coordinates to c++ code coordinates
 */
template<typename T>
static inline void World_FromUsecodeXY(T &x, T &y) {
	if (GAME_IS_CRUSADER) {
		x *= 2;
		y *= 2;
	}
}

/**
 * Convert a single x or y value from c++ coordinate to usecode coordinate
 */
template<typename T>
static inline T World_ToUsecodeCoord(T v) {
	if (GAME_IS_CRUSADER)
		return v / 2;
	else
		return v;
}

/**
 * Convert a single x or y value from usecode coordinate to c++ code coordinate
 */
template<typename T>
static inline T World_FromUsecodeCoord(T v) {
	if (GAME_IS_CRUSADER)
		return v * 2;
	else
		return v;
}

}
}

#endif // ULTIMA8_WORLD_COORDUTILS_H
