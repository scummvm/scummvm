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

#include "common/debug.h"
#include "common/str.h"
#include "common/textconsole.h"

#include "scumm/he/moonbase/map_mif.h"

namespace Scumm {

static const byte waterTileMap[] = {
	0x44, 0x40, 0x32, 0x32, 0x3C, 0x38, 0x32, 0x32, // 0x00
	0x2C, 0x2C, 0x26, 0x26, 0x2A, 0x2A, 0x26, 0x26,
	0x2F, 0x2D, 0x27, 0x27, 0x2F, 0x2D, 0x27, 0x27, // 0x10
	0x1B, 0x1B, 0x49, 0x49, 0x1B, 0x1B, 0x49, 0x49,
	0x42, 0x3E, 0x30, 0x30, 0x3A, 0x36, 0x30, 0x30, // 0x20
	0x2C, 0x2C, 0x26, 0x26, 0x2A, 0x2A, 0x26, 0x26,
	0x2E, 0x22, 0x1E, 0x1E, 0x2E, 0x22, 0x1E, 0x1E, // 0x30
	0x1B, 0x1B, 0x49, 0x49, 0x1B, 0x1B, 0x49, 0x49,
	0x35, 0x33, 0x1C, 0x1C, 0x34, 0x24, 0x1C, 0x1C, // 0x40
	0x28, 0x28, 0x4B, 0x4B, 0x1F, 0x1F, 0x4B, 0x4B,
	0x29, 0x20, 0x4C, 0x4C, 0x29, 0x20, 0x4C, 0x4C, // 0x50
	0x48, 0x48, 0x4A, 0x4A, 0x48, 0x48, 0x4A, 0x4A,
	0x35, 0x33, 0x1C, 0x1C, 0x34, 0x24, 0x1C, 0x1C, // 0x60
	0x28, 0x28, 0x4B, 0x4B, 0x1F, 0x1F, 0x4B, 0x4B,
	0x29, 0x20, 0x4C, 0x4C, 0x29, 0x20, 0x4C, 0x4C, // 0x70
	0x48, 0x48, 0x4A, 0x4A, 0x48, 0x48, 0x4A, 0x4A,
	0x43, 0x3F, 0x31, 0x31, 0x3B, 0x37, 0x31, 0x31, // 0x80
	0x2B, 0x2B, 0x1D, 0x1D, 0x21, 0x21, 0x1D, 0x1D,
	0x2F, 0x2D, 0x27, 0x27, 0x2F, 0x2D, 0x27, 0x27, // 0x90
	0x1B, 0x1B, 0x49, 0x49, 0x1B, 0x1B, 0x49, 0x49,
	0x41, 0x3D, 0x23, 0x23, 0x39, 0x25, 0x23, 0x23, // 0xA0
	0x2B, 0x2B, 0x1D, 0x1D, 0x21, 0x21, 0x1D, 0x1D,
	0x2E, 0x22, 0x1E, 0x1E, 0x2E, 0x22, 0x1E, 0x1E, // 0xB0
	0x1B, 0x1B, 0x49, 0x49, 0x1B, 0x1B, 0x49, 0x49,
	0x35, 0x33, 0x1C, 0x1C, 0x34, 0x24, 0x1C, 0x1C, // 0xC0
	0x28, 0x28, 0x4B, 0x4B, 0x1F, 0x1F, 0x4B, 0x4B,
	0x29, 0x20, 0x4C, 0x4C, 0x29, 0x20, 0x4C, 0x4C, // 0xD0
	0x48, 0x48, 0x4A, 0x4A, 0x48, 0x48, 0x4A, 0x4A,
	0x35, 0x33, 0x1C, 0x1C, 0x34, 0x24, 0x1C, 0x1C, // 0xE0
	0x28, 0x28, 0x4B, 0x4B, 0x1F, 0x1F, 0x4B, 0x4B,
	0x29, 0x20, 0x4C, 0x4C, 0x29, 0x20, 0x4C, 0x4C, // 0xF0
	0x48, 0x48, 0x4A, 0x4A, 0x48, 0x48, 0x4A, 0x4A
};

static int magic(int x, int y) {
	static const byte matrix[8][8] = {
		{ 2, 0, 2, 1, 3, 0, 3, 1 } ,
		{ 3, 1, 0, 3, 2, 1, 0, 2 } ,
		{ 0, 2, 1, 2, 0, 3, 1, 3 } ,
		{ 1, 3, 0, 3, 1, 2, 0, 2 } ,
		{ 2, 0, 1, 2, 3, 0, 1, 3 } ,
		{ 3, 1, 3, 0, 2, 1, 2, 0 } ,
		{ 0, 2, 0, 1, 3, 0, 3, 2 } ,
		{ 1, 3, 0, 3, 2, 1, 2, 0 }
	};
	return matrix[y % 8][x % 8];
}

MIF::MIF() {
}

void MIF::generateMap(MapFile *map) {
	map->terrainDimX = _dimension;
	map->terrainDimY = _dimension;
	map->mapType = _mapType;
	Common::strlcpy(map->name, _name, 17);

	int x, y;
	for (y = 0; y < _dimension; ++y) {
		for (x = 0; x < _dimension; ++x) {
			map->terrainStates[x][y] = findTileFor(x, y);
		}
	}

	defineEnergyPools(map);
	defineStartLocations(map);
	makeCraters(map);
}

void MIF::defineStartLocations(MapFile *map) {
	int x, y;

	for (y = 0; y < _dimension; ++y) {
		for (x = 0; x < _dimension; ++x) {
			int8 ch = _centerMap[x][y];

			if (ch < 0) {
				int i;
				ch = -ch;

				if (ch & 1) {
					// 4 player start
					i = 0;
					while (i < 4) {
						if (map->fourPlayerPoints[i].x == 0xFFFF) {
							map->fourPlayerPoints[i].x = x * 60;
							map->fourPlayerPoints[i].y = y * 60;
							break;
						}
						++i;
					}
				}
				ch = ch >> 1;
				if (ch & 1) {
					// 3 player start
					i = 0;
					while (i < 3) {
						if (map->threePlayerPoints[i].x == 0xFFFF) {
							map->threePlayerPoints[i].x = x * 60;
							map->threePlayerPoints[i].y = y * 60;
							break;
						}
						++i;
					}
				}
				ch = ch >> 1;
				if (ch & 1) {
					// 2 player start
					i = 0;
					while (i < 2) {
						if (map->twoPlayerPoints[i].x == 0xFFFF) {
							map->twoPlayerPoints[i].x = x * 60;
							map->twoPlayerPoints[i].y = y * 60;
							break;
						}
						++i;
					}
				}
				ch = ch >> 1;
				if (ch & 1) {
					// 2v2 player start
					i = 0;
					while (i < 4) {
						if (map->twoVTwoPoints[i].x == 0xFFFF) {
							map->twoVTwoPoints[i].x = x * 60;
							map->twoVTwoPoints[i].y = y * 60;
							break;
						}
						++i;
					}
				}
				ch = ch >> 1;
				if (ch & 1) {
					// 1v3 player start
					i = 0;
					while (i < 4) {
						if (map->oneVThreePoints[i].x == 0xFFFF) {
							map->oneVThreePoints[i].x = x * 60;
							map->oneVThreePoints[i].y = y * 60;
							break;
						}
						++i;
					}
				}
				ch = ch >> 1;
				if (ch & 1) {
					// 1v2 player start
					i = 0;
					while (i < 3) {
						if (map->oneVTwoPoints[i].x == 0xFFFF) {
							map->oneVTwoPoints[i].x = x * 60;
							map->oneVTwoPoints[i].y = y * 60;
							break;
						}
						++i;
					}
				}
			}
		}
	}
}

void MIF::defineEnergyPools(MapFile *map) {
	int x, y;

	for (y = 0; y < _dimension; ++y) {
		for (x = 0; x < _dimension; ++x) {
			char ch = _centerMap[x][y];

			if ('S' == ch || 'M' == ch || 'L' == ch) {
				// Verify legal position
				if (!((tlCorner(x, y) == trCorner(x, y)) && (blCorner(x, y) == brCorner(x, y)) &&
					(tlCorner(x, y) == blCorner(x, y)) && (trCorner(x, y) == brCorner(x, y)))) {
					error("small and medium energy pools must be on a flat tile (%d, %d)", x, y);
				}

				if ('L' == ch) {
					byte nHeight;
					nHeight = blCorner(x, y);
					if (!(tlCorner(x, y) == nHeight && ttlCorner(x, y) == nHeight && ttrCorner(x, y) == nHeight && trCorner(x, y) == nHeight && brCorner(x, y) == nHeight)) {
						error("large energy pools must be on the lower of two flat tiles (%d, %d)", x, y);
					}
				}

				int xLoc;
				int yLoc;

				if ('S' == ch) {
					xLoc = 60 * x + 30 + 20000;
					yLoc = 60 * y + 30;
				} else if ('M' == ch) {
					xLoc = 60 * x + 30 + 10000;
					yLoc = 60 * y + 30;
				} else {
					xLoc = 60 * x + 30;
					yLoc = 60 * y;
				}

				if (map->numEnergyPools < 49) {
					map->poolLocs[map->numEnergyPools].location.x = xLoc;
					map->poolLocs[map->numEnergyPools].location.y = yLoc;

					++map->numEnergyPools;
				} else if (map->numEnergyPools == 49) {
					map->lastPool.x = xLoc;
					map->lastPool.y = yLoc;

					++map->numEnergyPools;
				} else {
					error("only 50 energy pools are allowed, this is the 51st (%d, %d)", x, y);
				}
			}
		}
	}
}

void MIF::makeCraters(MapFile *map) {
	// squarenumber, type, x, y (offset from top left (abs y)). x/y = 9 if none of that type
	static const byte locations[8][3][2] = {
		{ {1, 1}, {5, 2}, {3, 5} },
		{ {6, 1}, {1, 6}, {2, 0} },
		{ {0, 4}, {3, 2}, {6, 5} },
		{ {4, 4}, {5, 0}, {9, 9} },
		{ {3, 6}, {9, 9}, {2, 1} },
		{ {9, 9}, {3, 3}, {0, 2} },
		{ {2, 4}, {0, 0}, {5, 3} },
		{ {4, 1}, {0, 3}, {5, 6} }
	};

	// I made up the crater patterns for sizes larger than SAI
	// This will work for maps up to 80x80
	static byte const largegrid[10][10] =  {
		{0, 1, 2, 3, 4, 5, 6, 7, 0, 1},
		{2, 3, 4, 5, 6, 7, 0, 1, 2, 3},
		{4, 5, 6, 7, 0, 1, 2, 3, 4, 5},
		{3, 0, 1, 2, 6, 4, 5, 7, 3, 0},
		{1, 2, 3, 4, 5, 6, 7, 0, 1, 2},
		{3, 4, 5, 6, 7, 0, 1, 2, 3, 4},
		{6, 3, 0, 1, 2, 7, 4, 5, 6, 3},
		{5, 6, 7, 0, 1, 2, 3, 4, 5, 6},
		{0, 1, 2, 3, 4, 5, 6, 7, 0, 1},
		{2, 3, 4, 5, 6, 7, 0, 1, 2, 3}
	};

	for (int i = 0; i < _dimension / 8; i++) {
		for (int j = 0; j < _dimension / 8; j++) {
			for (int nCrater = 0; nCrater < 3; nCrater++) {
				if (9 == locations[largegrid[j][i]][nCrater][0]) {
					continue;
				}

				int x = locations[largegrid[j][i]][nCrater][0] + i * 8;
				int y = locations[largegrid[j][i]][nCrater][1] + j * 8;

				byte nLevel = tlCorner(x, y);
				if ((tlCorner(x, y) == nLevel) && (trCorner(x, y) == nLevel) && (trrCorner(x, y) == nLevel) &&
					(_centerMap[x][y] != 'W') && (_centerMap[x + 1][y] != 'W') &&
					(blCorner(x, y) == nLevel) && (brCorner(x, y) == nLevel) && (brrCorner(x, y) == nLevel) &&
					(_centerMap[x][y + 1] != 'W') && (_centerMap[x + 1][y + 1] != 'W') &&
					(bblCorner(x, y) == nLevel) && (bbrCorner(x, y) == nLevel) && (bbrrCorner(x, y) == nLevel)) {
					// The tile values follow a predictable pattern, level one craters in order, etc.
					int16 nBase = 0xA6 + (tlCorner(x, y) * 12) + (nCrater * 4);

					map->terrainStates[x][y] = nBase;
					map->terrainStates[x + 1][y] = nBase + 1;
					map->terrainStates[x][y + 1] = nBase + 2;
					map->terrainStates[x + 1][y + 1] = nBase + 3;
				}
			}
		}
	}
}

uint16 MIF::findTileFor(int x, int y) {
	int index;
	int8 ch;

	byte aLowBlanks[] = {0x93, 0x94, 0x00, 0x96};
	byte aMedBlanks[] = {0x97, 0x99, 0x0D, 0x9A};
	byte aHiBlanks[] = {0x9B, 0x9C, 0x1A, 0x9D};
	ch = _centerMap[x][y];

	debug(5, "MIF: Tile for %d, %d is %c", x, y, ch);

	if ('S' == ch || 'M' == ch || 'L' == ch || '.' == ch || ch < 0) {
		// Do the easy cases, things with no transitions.
		if (0 == tlCorner(x, y) && 0 == trCorner(x, y) && 0 == blCorner(x, y) && 0 == brCorner(x, y))
			return aLowBlanks[magic(x, y)];
		if (1 == tlCorner(x, y) && 1 == trCorner(x, y) && 1 == blCorner(x, y) && 1 == brCorner(x, y))
			return aMedBlanks[magic(x, y)];
		if (2 == tlCorner(x, y) && 2 == trCorner(x, y) && 2 == blCorner(x, y) && 2 == brCorner(x, y))
			return aHiBlanks[magic(x, y)];

		//
		// Low to med transitions
		//
		if (0 == tlCorner(x, y) || 0 == trCorner(x, y) || 0 == blCorner(x, y) || 0 == brCorner(x, y)) {
			// Corner cases

			int cornerSum = tlCorner(x, y) + trCorner(x, y) + blCorner(x, y) + brCorner(x, y);

			if (1 == cornerSum) {
				if (tlCorner(x, y)) {
					if (tllCorner(x, y) > 0 && ttlCorner(x, y) > 0)
						return 0x03;
					else
						return 0x89;
				} else if (trCorner(x, y)) {
					if (trrCorner(x, y) > 0 && ttrCorner(x, y) > 0)
						return 0x04;
					else
						return 0x8C;
				} else if (blCorner(x, y)) {
					if (bllCorner(x, y) > 0 && bblCorner(x, y) > 0)
						return 0x02;
					else
						return 0x86;
				} else // brCorner
				{
					if (brrCorner(x, y) > 0 && bbrCorner(x, y) > 0)
						return 0x01;
					else
						return 0x83;
				}
			}

			// Straight edges

			// edge on bottom
			if (tlCorner(x, y) == 0 && trCorner(x, y) == 0 && blCorner(x, y) == 1 && brCorner(x, y) == 1) {
				bool bLeftEased = (bllCorner(x, y) == 0 && bblCorner(x, y) == 1);
				bool bRightEased = (brrCorner(x, y) == 0 && bbrCorner(x, y) == 1);

				if (bLeftEased && bRightEased)
					return 0x0A;
				if (!bLeftEased && bRightEased)
					return 0x54;
				if (bLeftEased && !bRightEased)
					return 0x55;
				if (!bLeftEased && !bRightEased)
					return (magic(x, y) & 0x01) ? 0x9F : 0x56;
			}
			// edge on top
			else if (tlCorner(x, y) == 1 && trCorner(x, y) == 1 && blCorner(x, y) == 0 && brCorner(x, y) == 0) {
				bool bLeftEased = (tllCorner(x, y) == 0 && ttlCorner(x, y) == 1);
				bool bRightEased = (trrCorner(x, y) == 0 && ttrCorner(x, y) == 1);

				if (bLeftEased && bRightEased)
					return 0x0C;
				if (!bLeftEased && bRightEased)
					return 0x52;
				if (bLeftEased && !bRightEased)
					return 0x51;
				if (!bLeftEased && !bRightEased)
					return (magic(x, y) & 0x01) ? 0xA1 : 0x53;
			}
			// edge on right
			if (tlCorner(x, y) == 0 && blCorner(x, y) == 0 && trCorner(x, y) == 1 && brCorner(x, y) == 1) {
				bool bTopEased = (ttrCorner(x, y) == 0 && trrCorner(x, y) == 1);
				bool bBotEased = (bbrCorner(x, y) == 0 && brrCorner(x, y) == 1);

				if (bTopEased && bBotEased)
					return 0x09;
				if (!bTopEased && bBotEased)
					return 0x5B;
				if (bTopEased && !bBotEased)
					return 0x5A;
				if (!bTopEased && !bBotEased)
					return (magic(x, y) & 0x01) ? 0x9E : 0x5C;
			}
			// edge on left
			if (tlCorner(x, y) == 1 && blCorner(x, y) == 1 && trCorner(x, y) == 0 && brCorner(x, y) == 0) {
				bool bTopEased = (ttlCorner(x, y) == 0 && tllCorner(x, y) == 1);
				bool bBotEased = (bblCorner(x, y) == 0 && bllCorner(x, y) == 1);

				if (bTopEased && bBotEased)
					return 0x0B;
				if (!bTopEased && bBotEased)
					return 0x57;
				if (bTopEased && !bBotEased)
					return 0x58;
				if (!bTopEased && !bBotEased)
					return (magic(x, y) & 0x01) ? 0xA0 : 0x59;
			}

			// Three corner cases

			// 0 1         1 1           0 0 0       0 0 0
			// 0 1 1 0x5F  0 1 1 0x5E    0 1 1 0x62  0 1 1 0x60
			// 0 0 0       0 0 0         0 1         1 1
			//
			// 0 1         1 1           0 0 1       0 0 1
			// 0 1 1 0x5D  0 1 1 0x06    0 1 1 0x61  0 1 1 0x07
			// 0 0 1       0 0 1         0 1         1 1
			//
			//
			// 0 0 0       1 0 0           1 0         1 1
			// 1 1 0 0x65  1 1 0 0x63    1 1 0 0x68  1 1 0 0x66
			//   1 0         1 0         0 0 0       0 0 0
			//
			// 0 0 0       1 0 0           1 0         1 1
			// 1 1 0 0x64  1 1 0 0x08    1 1 0 0x67  1 1 0 0x05
			//   1 1         1 1         1 0 0       1 0 0

			// corner in upper left
			if (blCorner(x, y) == 1 && tlCorner(x, y) == 1 && trCorner(x, y) == 1) {
				bool BLDiag = (bllCorner(x, y) > 0 && bblCorner(x, y) == 0);
				bool TRDiag = (ttrCorner(x, y) > 0 && trrCorner(x, y) == 0);

				if (!BLDiag && !TRDiag)
					return 0x62;
				else if (!BLDiag && TRDiag)
					return 0x61;
				else if (BLDiag && !TRDiag)
					return 0x60;
				else
					return 0x07;
			}

			// corner in upper right
			if (tlCorner(x, y) == 1 && trCorner(x, y) == 1 && brCorner(x, y) == 1) {
				bool TLDiag = (ttlCorner(x, y) > 0 && tllCorner(x, y) == 0);
				bool BRDiag = (brrCorner(x, y) > 0 && bbrCorner(x, y) == 0);

				if (!TLDiag && !BRDiag)
					return 0x65;
				else if (!TLDiag && BRDiag)
					return 0x64;
				else if (TLDiag && !BRDiag)
					return 0x63;
				else
					return 0x08;
			}

			// corner in bottom right
			if (trCorner(x, y) == 1 && brCorner(x, y) == 1 && blCorner(x, y) == 1) {
				bool TRDiag = (trrCorner(x, y) > 0 && ttrCorner(x, y) == 0);
				bool BLDiag = (bblCorner(x, y) > 0 && bllCorner(x, y) == 0);

				if (!TRDiag && !BLDiag)
					return 0x68;
				else if (!TRDiag && BLDiag)
					return 0x67;
				else if (TRDiag && !BLDiag)
					return 0x66;
				else
					return 0x05;
			}

			// corner in bottom left
			if (brCorner(x, y) == 1 && blCorner(x, y) == 1 && tlCorner(x, y) == 1) {
				bool TLDiag = (tllCorner(x, y) > 0 && ttlCorner(x, y) == 0);
				bool BRDiag = (bbrCorner(x, y) > 0 && brrCorner(x, y) == 0);

				if (!TLDiag && !BRDiag)
					return 0x5F;
				else if (!TLDiag && BRDiag)
					return 0x5D;
				else if (TLDiag && !BRDiag)
					return 0x5E;
				else
					return 0x06;
			}

			// Opposing corner cases
			if (tlCorner(x, y) == 1 && brCorner(x, y) == 1) {
				// There are four cases, big big, big small, small big, small small

				// big big
				if (tllCorner(x, y) > 0 && ttlCorner(x, y) > 0 && brrCorner(x, y) > 0 && bbrCorner(x, y) > 0)
					return 0x4D;
				// big small
				if (tllCorner(x, y) > 0 && ttlCorner(x, y) > 0)
					return 0x81;
				// small big
				if (brrCorner(x, y) > 0 && bbrCorner(x, y) > 0)
					return 0x82;
				// small small
				return 0x84;
			}
			if (trCorner(x, y) == 1 && blCorner(x, y) == 1) {
				// There are four cases, big big, big small, small big, small small

				// big big
				if (trrCorner(x, y) > 0 && ttrCorner(x, y) > 0 && bllCorner(x, y) > 0 && bblCorner(x, y) > 0)
					return 0x4E;
				// big small
				if (trrCorner(x, y) > 0 && ttrCorner(x, y) > 0)
					return 0x85;
				// small big
				if (bllCorner(x, y) > 0 && bblCorner(x, y) > 0)
					return 0x87;
				// small small
				return 0x88;
			}
		}

		//
		// Med to high transitions
		//
		if (1 == tlCorner(x, y) || 1 == trCorner(x, y) || 1 == blCorner(x, y) || 1 == brCorner(x, y)) {
			// Corner cases

			int cornerSum = (tlCorner(x, y) == 2) + (trCorner(x, y) == 2) + (blCorner(x, y) == 2) + (brCorner(x, y) == 2);

			if (1 == cornerSum) {
				if (tlCorner(x, y) == 2) {
					if (tllCorner(x, y) == 2 && ttlCorner(x, y) == 2)
						return 0x10;
					else
						return 0x95;
				} else if (trCorner(x, y) == 2) {
					if (trrCorner(x, y) == 2 && ttrCorner(x, y) == 2)
						return 0x11;
					else
						return 0x98;
				} else if (blCorner(x, y) == 2) {
					if (bllCorner(x, y) == 2 && bblCorner(x, y) == 2)
						return 0x0F;
					else
						return 0x92;
				} else // brCorner
				{
					if (brrCorner(x, y) == 2 && bbrCorner(x, y) == 2)
						return 0x0E;
					else
						return 0x8F;
				}
			}

			// Straight edges

			// edge on bottom
			if (tlCorner(x, y) < 2 && trCorner(x, y) < 2 && blCorner(x, y) == 2 && brCorner(x, y) == 2) {
				bool bLeftEased = (bllCorner(x, y) < 2 && bblCorner(x, y) == 2);
				bool bRightEased = (brrCorner(x, y) < 2 && bbrCorner(x, y) == 2);

				if (bLeftEased && bRightEased)
					return 0x17;
				if (!bLeftEased && bRightEased)
					return 0x6C;
				if (bLeftEased && !bRightEased)
					return 0x6D;
				if (!bLeftEased && !bRightEased)
					return (magic(x, y) & 0x01) ? 0xA3 : 0x6E;
			}
			// edge on top
			else if (tlCorner(x, y) == 2 && trCorner(x, y) == 2 && blCorner(x, y) < 2 && brCorner(x, y) < 2) {
				bool bLeftEased = (tllCorner(x, y) < 2 && ttlCorner(x, y) == 2);
				bool bRightEased = (trrCorner(x, y) < 2 && ttrCorner(x, y) == 2);

				if (bLeftEased && bRightEased)
					return 0x19;
				if (!bLeftEased && bRightEased)
					return 0x6A;
				if (bLeftEased && !bRightEased)
					return 0x69;
				if (!bLeftEased && !bRightEased)
					return (magic(x, y) & 0x01) ? 0xA5 : 0x6B;
			}
			// edge on right
			if (tlCorner(x, y) < 2 && blCorner(x, y) < 2 && trCorner(x, y) == 2 && brCorner(x, y) == 2) {
				bool bTopEased = (ttrCorner(x, y) < 2 && trrCorner(x, y) == 2);
				bool bBotEased = (bbrCorner(x, y) < 2 && brrCorner(x, y) == 2);

				if (bTopEased && bBotEased)
					return 0x16;
				if (!bTopEased && bBotEased)
					return 0x73;
				if (bTopEased && !bBotEased)
					return 0x72;
				if (!bTopEased && !bBotEased)
					return (magic(x, y) & 0x01) ? 0xA2 : 0x74;
			}
			// edge on left
			if (tlCorner(x, y) == 2 && blCorner(x, y) == 2 && trCorner(x, y) < 2 && brCorner(x, y) < 2) {
				bool bTopEased = (ttlCorner(x, y) < 2 && tllCorner(x, y) == 2);
				bool bBotEased = (bblCorner(x, y) < 2 && bllCorner(x, y) == 2);

				if (bTopEased && bBotEased)
					return 0x18;
				if (!bTopEased && bBotEased)
					return 0x6F;
				if (bTopEased && !bBotEased)
					return 0x70;
				if (!bTopEased && !bBotEased)
					return (magic(x, y) & 0x01) ? 0xA4 : 0x71;
			}

			// edge on bottom
			if (tlCorner(x, y) == 1 && trCorner(x, y) == 1 && blCorner(x, y) == 2 && brCorner(x, y) == 2) {
				// no other high corners
				if (bllCorner(x, y) < 2 && brrCorner(x, y) < 2)
					return 0x17;
				// high corner on left
				if (bllCorner(x, y) == 2 && brrCorner(x, y) < 2)
					return 0x6C;
				// high corner on right
				if (bllCorner(x, y) < 2 && brrCorner(x, y) == 2)
					return 0x6D;
				// both neighbor corners high
				if (bllCorner(x, y) == 2 && brrCorner(x, y) == 2)
					return (magic(x, y) & 0x01) ? 0xA3 : 0x6E;
			}
			// edge on top
			else if (tlCorner(x, y) == 2 && trCorner(x, y) == 2 && blCorner(x, y) == 1 && brCorner(x, y) == 1) {
				// no other high corners
				if (tllCorner(x, y) < 2 && trrCorner(x, y) < 2)
					return 0x19;
				// high corner on left
				if (tllCorner(x, y) == 2 && trrCorner(x, y) < 2)
					return 0x6A;
				// high corner on right
				if (tllCorner(x, y) < 2 && trrCorner(x, y) == 2)
					return 0x69;
				// both neighbor corners high
				if (tllCorner(x, y) == 2 && trrCorner(x, y) == 2)
					return (magic(x, y) & 0x01) ? 0xA5 : 0x6B;
			}
			// edge on right
			if (tlCorner(x, y) == 1 && blCorner(x, y) == 1 && trCorner(x, y) == 2 && brCorner(x, y) == 2) {
				// no high neighbor corners
				if (ttrCorner(x, y) < 2 && bbrCorner(x, y) < 2)
					return 0x16;
				// high neighbor corner on top
				if (ttrCorner(x, y) == 2 && bbrCorner(x, y) < 2)
					return 0x73;
				// high neighbor corner on bottom
				if (ttrCorner(x, y) < 2 && bbrCorner(x, y) == 2)
					return 0x72;
				// both neighbor corners high
				if (ttrCorner(x, y) == 2 && bbrCorner(x, y) == 2)
					return (magic(x, y) & 0x01) ? 0xA2 : 0x74;
			}
			// edge on left
			if (tlCorner(x, y) == 2 && blCorner(x, y) == 2 && trCorner(x, y) == 1 && brCorner(x, y) == 1) {
				// no high neighbor corners
				if (ttlCorner(x, y) < 2 && bblCorner(x, y) < 2)
					return 0x18;
				// high neighbor corner on top
				if (ttlCorner(x, y) == 2 && bblCorner(x, y) < 2)
					return 0x6F;
				// high neighbor corner on bottom
				if (ttlCorner(x, y) < 2 && bblCorner(x, y) == 2)
					return 0x70;
				// both neighbor corners high
				if (ttlCorner(x, y) == 2 && bblCorner(x, y) == 2)
					return (magic(x, y) & 0x01) ? 0xA4 : 0x71;
			}

			// Three corner cases

			// Three corner cases

			// 0 1         1 1           0 0 0       0 0 0
			// 0 1 1 0x77  0 1 1 0x76    0 1 1 0x7A  0 1 1 0x78
			// 0 0 0       0 0 0         0 1         1 1
			//
			// 0 1         1 1           0 0 1       0 0 1
			// 0 1 1 0x75  0 1 1 0x13    0 1 1 0x79  0 1 1 0x14
			// 0 0 1       0 0 1         0 1         1 1
			//
			//
			// 0 0 0       1 0 0           1 0         1 1
			// 1 1 0 0x7D  1 1 0 0x7B    1 1 0 0x80  1 1 0 0x7E
			//   1 0         1 0         0 0 0       0 0 0
			//
			// 0 0 0       1 0 0           1 0         1 1
			// 1 1 0 0x7C  1 1 0 0x15    1 1 0 0x7F  1 1 0 0x12
			//   1 1         1 1         1 0 0       1 0 0

			// corner in upper left
			if (blCorner(x, y) == 2 && tlCorner(x, y) == 2 && trCorner(x, y) == 2) {
				bool BLDiag = (bllCorner(x, y) > 1 && bblCorner(x, y) < 2);
				bool TRDiag = (ttrCorner(x, y) > 1 && trrCorner(x, y) < 2);

				if (!BLDiag && !TRDiag)
					return 0x7A;
				else if (!BLDiag && TRDiag)
					return 0x79;
				else if (BLDiag && !TRDiag)
					return 0x78;
				else
					return 0x14;
			}

			// corner in upper right
			if (tlCorner(x, y) == 2 && trCorner(x, y) == 2 && brCorner(x, y) == 2) {
				bool TLDiag = ((ttlCorner(x, y) > 1) && (tllCorner(x, y) < 2));
				bool BRDiag = ((brrCorner(x, y) > 1) && (bbrCorner(x, y) < 2));

				if (!TLDiag && !BRDiag)
					return 0x7D;
				else if (!TLDiag && BRDiag)
					return 0x7C;
				else if (TLDiag && !BRDiag)
					return 0x7B;
				else
					return 0x15;
			}

			// corner in bottom right
			if (trCorner(x, y) == 2 && brCorner(x, y) == 2 && blCorner(x, y) == 2) {
				bool TRDiag = (trrCorner(x, y) > 1 && ttrCorner(x, y) < 2);
				bool BLDiag = (bblCorner(x, y) > 1 && bllCorner(x, y) < 2);

				if (!TRDiag && !BLDiag)
					return 0x80;
				else if (!TRDiag && BLDiag)
					return 0x7F;
				else if (TRDiag && !BLDiag)
					return 0x7E;
				else
					return 0x12;
			}

			// corner in bottom left
			if (brCorner(x, y) == 2 && blCorner(x, y) == 2 && tlCorner(x, y) == 2) {
				bool TLDiag = (tllCorner(x, y) > 1 && ttlCorner(x, y) < 2);
				bool BRDiag = (bbrCorner(x, y) > 1 && brrCorner(x, y) < 2);

				if (!TLDiag && !BRDiag)
					return 0x77;
				else if (!TLDiag && BRDiag)
					return 0x75;
				else if (TLDiag && !BRDiag)
					return 0x76;
				else
					return 0x13;
			}

			// Opposing corner cases
			if (tlCorner(x, y) == 2 && brCorner(x, y) == 2) {
				// There are four cases, big big, big small, small big, small small

				// big big
				if (tllCorner(x, y) == 2 && ttlCorner(x, y) == 2 && brrCorner(x, y) == 2 && bbrCorner(x, y) == 2)
					return 0x4F;
				// big small
				if (tllCorner(x, y) == 2 && ttlCorner(x, y) == 2)
					return 0x8A;
				// small big
				if (brrCorner(x, y) == 2 && bbrCorner(x, y) == 2)
					return 0x8B;
				// small small
				return 0x8D;
			}
			if (trCorner(x, y) == 2 && blCorner(x, y) == 2) {
				// There are four cases, big big, big small, small big, small small

				// big big
				if (trrCorner(x, y) == 2 && ttrCorner(x, y) == 2 && bllCorner(x, y) == 2 && bblCorner(x, y) == 2)
					return 0x50;
				// big small
				if (trrCorner(x, y) == 2 && ttrCorner(x, y) == 2)
					return 0x8E;
				// small big
				if (bllCorner(x, y) == 2 && bblCorner(x, y) == 2)
					return 0x90;
				// small small
				return 0x91;
			}
		}

		error("illegal corner height arrangement (%d, %d)", x, y);
	} else if ('W' == ch) {
		// Check to make sure that we're on ground level
		if (tlCorner(x, y) > 0 || trCorner(x, y) > 0 || blCorner(x, y) > 0 || brCorner(x, y) > 0)
			error("water must be on a flat tile (%d, %d)", x, y);

		index = (('W' != tlCenter(x, y)) << 7) |
				(('W' != tCenter(x, y)) << 6) |
				(('W' != trCenter(x, y)) << 5) |
				(('W' != lCenter(x, y)) << 4) |
				(('W' != rCenter(x, y)) << 3) |
				(('W' != blCenter(x, y)) << 2) |
				(('W' != bCenter(x, y)) << 1) |
				('W' != brCenter(x, y));

		uint16 nWaterTile = waterTileMap[index];

		if (0x44 == nWaterTile) {
			uint16 aWaterBlanks[] = {0x45, 0x46, 0x44, 0x47};

			nWaterTile = aWaterBlanks[magic(x, y)];
		}

		return nWaterTile;
	} else
		error("illegal tile character (%d, %d)", x, y);

	error("unknown tile find error (%d, %d)", x, y);
}

} // End of namespace Scumm
