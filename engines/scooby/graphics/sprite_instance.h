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

#ifndef SCOOBY_SPRITE_INSTANCE_H
#define SCOOBY_SPRITE_INSTANCE_H

#include "common/scummsys.h"

// Describes one linked sprite after decoding its packed Genesis attribute entry.

namespace Scooby {

struct SpriteInstance {
	int32 x;
	int32 y;
	int32 widthInTiles;
	int32 heightInTiles;
	int32 tileIndex;
	uint8 paletteIndex;
	bool flipHorizontally;
	bool flipVertically;
	bool highPriority;

	SpriteInstance()
		: x(0), y(0), widthInTiles(0), heightInTiles(0), tileIndex(0), paletteIndex(0), flipHorizontally(false),
		  flipVertically(false), highPriority(false) {
	}

	SpriteInstance(int32 x, int32 y, int32 widthInTiles, int32 heightInTiles,
				   int32 tileIndex, uint8 paletteIndex, bool flipHorizontally,
				   bool flipVertically,
				   bool highPriority)
		: x(x), y(y), widthInTiles(widthInTiles), heightInTiles(heightInTiles), tileIndex(tileIndex),
		  paletteIndex(paletteIndex), flipHorizontally(flipHorizontally), flipVertically(flipVertically),
		  highPriority(highPriority) {
	}
};
} // namespace Scooby

#endif // SCOOBY_SPRITE_INSTANCE_H
