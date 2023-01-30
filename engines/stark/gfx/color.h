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

#ifndef STARK_GFX_COLOR_H
#define STARK_GFX_COLOR_H

#include "common/scummsys.h"

namespace Stark {
namespace Gfx {

struct Color {
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 a;

	Color(uint8 red, uint8 green, uint8 blue, uint8 alpha = 0xFF) :
			r(red), g(green), b(blue), a(alpha) {}

	bool operator==(const Color &color) const {
		return r == color.r &&
		       g == color.g &&
		       b == color.b &&
		       a == color.a;
	}
};

} // End of namespace Gfx
} // End of namespace Stark

#endif // STARK_GFX_COLOR_H
