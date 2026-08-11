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

#ifndef SCUMM_HE_MOONBASE_MAP_MIF_H
#define SCUMM_HE_MOONBASE_MAP_MIF_H

#ifdef ENABLE_HE

#include "scumm/he/intern_he.h"

namespace Scumm {

#define MAX_TILE_COUNT 80

#include "common/pack-start.h"	// START STRUCT PACKING

struct PixelLoc {
	uint16 x;
	uint16 y;
};

struct EnergyPoolLoc {
	PixelLoc location;
	PixelLoc dummy;
};


struct MapFile {
	// Header:
	uint16 headerDummy;
	uint16 terrainDimX;
	uint16 terrainDimY;
	uint16 mapType;
	uint16 numEnergyPools;

	uint16 terrainStates[80][161];
	uint8 space1[230];
	char name[17];
	uint8 space2[25837];
	EnergyPoolLoc poolLocs[49];
	PixelLoc lastPool;
	uint16 dummy;
	PixelLoc fourPlayerPoints[4];  // 2^0
	PixelLoc threePlayerPoints[3]; // 2^1
	PixelLoc twoPlayerPoints[2];   // 2^2
	PixelLoc twoVTwoPoints[4];     // 2^3
	PixelLoc oneVThreePoints[4];   // 2^4
	PixelLoc oneVTwoPoints[3];     // 2^5

	MapFile() {
		memset(this, 0, sizeof(MapFile));

		mapType = 1;
		memset(fourPlayerPoints, 0xFF, sizeof(fourPlayerPoints));
		memset(threePlayerPoints, 0xFF, sizeof(threePlayerPoints));
		memset(twoPlayerPoints, 0xFF, sizeof(twoPlayerPoints));
		memset(twoVTwoPoints, 0xFF, sizeof(twoVTwoPoints));
		memset(oneVThreePoints, 0xFF, sizeof(oneVThreePoints));
		memset(oneVTwoPoints, 0xFF, sizeof(oneVTwoPoints));
	}
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

class MIF {
public:
	MIF();

	void generateMap(MapFile *map);

	int _dimension = 0; // 32 (small), 40 (medium), 48 (large), 56 (huge), 64 (SAI)
	int _mapType = 0;
	char _name[17] = {};
	byte _cornerMap[MAX_TILE_COUNT][MAX_TILE_COUNT] = { {}, {} };
	int8 _centerMap[MAX_TILE_COUNT][MAX_TILE_COUNT] = { {}, {} };
private:

	void defineStartLocations(MapFile *map);
	void defineEnergyPools(MapFile *map);
	void makeCraters(MapFile *map);
	uint16 findTileFor(int x, int y);

	inline char tlCenter(int x, int y) const {
		return _centerMap[(0 == x) ? _dimension - 1 : x - 1][(0 == y) ? _dimension - 1 : y - 1];
	}

	inline char tCenter(int x, int y) const {
		return _centerMap[x][(0 == y) ? _dimension - 1 : y - 1];
	}

	inline char trCenter(int x, int y) const {
		return _centerMap[(x + 1) % _dimension][(0 == y) ? _dimension - 1 : y - 1];
	}

	inline char lCenter(int x, int y) const {
		return _centerMap[(0 == x) ? _dimension - 1 : x - 1][y];
	}

	inline char rCenter(int x, int y) const {
		return _centerMap[(x + 1) % _dimension][y];
	}

	inline char blCenter(int x, int y) const {
		return _centerMap[(0 == x) ? _dimension - 1 : x - 1][(y + 1) % _dimension];
	}

	inline char bCenter(int x, int y) const {
		return _centerMap[x][(y + 1) % _dimension];
	}

	inline char brCenter(int x, int y) const {
		return _centerMap[(x + 1) % _dimension][(y + 1) % _dimension];
	}

	inline byte tlCorner(int x, int y) const {
		return _cornerMap[x][y];
	}

	inline byte trCorner(int x, int y) const {
		return _cornerMap[(x + 1) % _dimension][y];
	}

	inline byte blCorner(int x, int y) const {
		return _cornerMap[x][(y + 1) % _dimension];
	}

	inline byte brCorner(int x, int y) const {
		return _cornerMap[(x + 1) % _dimension][(y + 1) % _dimension];
	}

	inline byte ttllCorner(int x, int y) const {
		return tlCorner((x == 0) ? _dimension - 1 : x - 1, (y == 0) ? _dimension - 1: y - 1);
	}

	inline byte ttlCorner(int x, int y) const {
		return trCorner((x == 0) ? _dimension - 1 : x - 1, (y == 0) ? _dimension - 1: y - 1);
	}

	inline byte ttrCorner(int x, int y) const {
		return tlCorner((x + 1) % _dimension, (y == 0) ? _dimension - 1: y - 1);
	}

	inline byte ttrrCorner(int x, int y) const {
		return trCorner((x + 1) % _dimension, (y == 0) ? _dimension - 1: y - 1);
	}

	inline byte tllCorner(int x, int y) const {
		return tlCorner((x == 0) ? _dimension - 1 : x - 1, y);
	}

	inline byte trrCorner(int x, int y) const {
		return trCorner((x + 1) % _dimension, y);
	}

	inline byte bllCorner(int x, int y) const {
		return blCorner((x == 0) ? _dimension - 1 : x - 1, y);
	}

	inline byte brrCorner(int x, int y) const {
		return brCorner((x + 1) % _dimension, y);
	}

	inline byte bbllCorner(int x, int y) const {
		return blCorner((x == 0) ? _dimension - 1 : x - 1, (y + 1) % _dimension);
	}

	inline byte bblCorner(int x, int y) const {
		return brCorner((x == 0) ? _dimension - 1 : x - 1, (y + 1) % _dimension);
	}

	inline byte bbrCorner(int x, int y) const {
		return blCorner((x + 1) % _dimension, (y + 1) % _dimension);
	}

	inline byte bbrrCorner(int x, int y) const {
		return brCorner((x + 1) % _dimension, (y + 1) % _dimension);
	}

};

} // End of namespace Scumm

#endif // ENABLE_HE

#endif // SCUMM_HE_MOONBASE_MAP_MIF_H
