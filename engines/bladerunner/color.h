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

#ifndef BLADERUNNER_COLOR_H
#define BLADERUNNER_COLOR_H

#include "common/system.h"

namespace BladeRunner {

class Color {

	static const uint8 map5BitsTo8Bits[32];

public:
	float r;
	float g;
	float b;

	Color() : r(0.0f), g(0.0f), b(0.0f) {}

	Color(float r_, float g_, float b_) : r(r_), g(g_), b(b_) {}

	static uint8 get8BitColorFrom5Bit(uint8 col5b);
};

#include "common/pack-start.h"

struct Color256 {
	uint8 r;
	uint8 g;
	uint8 b;
} PACKED_STRUCT;

#include "common/pack-end.h"

} // End of namespace BladeRunner

#endif
