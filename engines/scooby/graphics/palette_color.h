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

#ifndef SCOOBY_PALETTE_COLOR_H
#define SCOOBY_PALETTE_COLOR_H

#include "common/scummsys.h"

// Stores one indexed-scene color as three quantized asset levels (0-7). The
// ROM decoder supplies values from zero through seven; the raylib host
// expands them only when presenting a frame.

namespace Scooby {

struct PaletteColor {
	uint8 redLevel;
	uint8 greenLevel;
	uint8 blueLevel;

	PaletteColor() : redLevel(0), greenLevel(0), blueLevel(0) {
	}

	PaletteColor(uint8 redLevel, uint8 greenLevel, uint8 blueLevel)
		: redLevel(redLevel), greenLevel(greenLevel), blueLevel(blueLevel) {
	}
};
} // namespace Scooby

#endif // SCOOBY_PALETTE_COLOR_H
