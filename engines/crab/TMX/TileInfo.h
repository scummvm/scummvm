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

#ifndef CRAB_TILEINFO_H
#define CRAB_TILEINFO_H
#include "crab/common_header.h"

#include "crab/loaders.h"

namespace Crab {

namespace TMX {
typedef unsigned int GidFormat;

// Bits on the far end of the 32-bit global tile ID are used for tile flags
const static GidFormat FlippedHorizontallyFlag = 0x80000000;
const static GidFormat FlippedVerticallyFlag = 0x40000000;
const static GidFormat FlippedAntiDiagonallyFlag = 0x20000000;

struct TileInfo {
	// The gid of the tile
	GidFormat gid;

	// Do we need to flip this tile?
	TextureFlipType flip;

	TileInfo() {
		gid = 0;
		flip = FLIP_NONE;
	}

	TileInfo(rapidxml::xml_node<char> *node) {
		// Load the gid of the tile
		if (!loadNum(gid, "gid", node))
			gid = 0;

		bool horizontal = (gid & FlippedHorizontallyFlag) != 0;
		bool vertical = (gid & FlippedVerticallyFlag) != 0;
		bool antidiagonal = (gid & FlippedAntiDiagonallyFlag) != 0;

		// Find how the tile is flipped
		if (horizontal) {
			if (vertical) {
				if (antidiagonal)
					flip = FLIP_XYD;
				else
					flip = FLIP_XY;
			} else if (antidiagonal)
				flip = FLIP_DX;
			else
				flip = FLIP_X;
		} else if (vertical) {
			if (antidiagonal)
				flip = FLIP_DY;
			else
				flip = FLIP_Y;
		} else if (antidiagonal)
			flip = FLIP_D;
		else
			flip = FLIP_NONE;

		// Clear the flags
		gid &= ~(FlippedHorizontallyFlag | FlippedVerticallyFlag | FlippedAntiDiagonallyFlag);
	}

	bool operator==(const TileInfo& other) const {
		return (gid == other.gid) && (flip == other.flip);
	}
};
} // End of namespace TMX

} // End of namespace Crab

#endif // CRAB_TILEINFO_H
