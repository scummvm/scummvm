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

#include "scumm/he/moonbase/map_spiff.h"

namespace Scumm {

SpiffGenerator::SpiffGenerator(Common::RandomSource *rnd) {
	_rnd = rnd;
}

SpiffGenerator::~SpiffGenerator() {
}

MapFile *SpiffGenerator::generateMap(int water, int mapSize, int energy, int terrain) {
	totalMapSizeG = mapSize;
	energyAmountG = (2 + energy) * totalMapSizeG * totalMapSizeG;

	islandsFlagG = pickFrom2(0, 1, water - 4, (water >= 5)); // 1 is large islands, 2 is small
	if (islandsFlagG) {
		water -= 3;
		energyAmountG = (int)(energyAmountG * (5 - islandsFlagG) / 6); // *2/3 or *1/2
	}

	waterAmountG = 4 * water;
	cliffAmountG = 1 << terrain;
	advancedMirrorOK_G = ((terrain > 1) && (water < 6)) || islandsFlagG;
	terrainSeedFlagG = 2 * water - terrain;

	int n = (int)(energyAmountG / 2700);
	if (n > 12) {
		n = 12;
	}
	if (n < 1) {
		n = 1;
	}
	numPoolsG = _rnd->getRandomNumberRngSigned((int)(energyAmountG / 4000) + 1, n);
	if (numPoolsG > 12) {
		numPoolsG = 12;
	}

	generate();

	// Populate MIF for map data generation:
	MIF mif = MIF();

	int levelMap[MAXELEVVAL];
	levelMap[HIGH] = 2;
	levelMap[MEDIUM] = 1;
	levelMap[LOW] = 0;

	mif.mapType = terrain;
	Common::sprintf_s(mif.name, "Spiff %d", _rnd->getSeed());

	mif.dimension = totalMapSizeG;

	int y;
	int x;
	char t;
	int XOffset = _rnd->getRandomNumberRngSigned(0, totalMapSizeG-1);
	int YOffset = _rnd->getRandomNumberRngSigned(0, totalMapSizeG-1);
	int newX;
	int newY;

	for (y = 0, newY = YOffset; y < totalMapSizeG; ++y, ++newY) {
		for (x = 0, newX = XOffset; x < totalMapSizeG; ++x, ++newX) {
			if (newX == totalMapSizeG)
				newX = 0;
			if (newY == totalMapSizeG)
				newY = 0;
			mif.cornerMap[newX][newY] = levelMap[mapCorner[x][y]];
			switch (mapMiddle[x][y]) {
			case HUB:
				t = -1;
				break;
			case SMALLPOOL:
				t = 'S';
				break;
			case MEDIUMPOOL:
				t = 'M';
				break;
			case LARGEPOOLBOTTOM:
				t = 'L';
				break;
			case WATER:
				t = 'W';
				break;
			case UNASSIGNED:
			case LARGEPOOLTOP:
				t = '.';
				break;
			default:
				t = '?';
			}
			mif.centerMap[newX][newY] = t;
		}
	}

	// Generate new map:
	MapFile *map = new MapFile();
	mif.generateMap(map);

	return map;
}

int SpiffGenerator::pickFrom2(int a, int probA, int b, int probB) {
	debug(3, "SpiffGenerator::pickFrom2(%d, %d, %d, %d)", a, probA, b, probB);
	int r = _rnd->getRandomNumber(probA + probB);
	debug(3, "  r = %d", r);
	if (r < probA)
		return a;
	else
		return b;
}

int SpiffGenerator::pickFrom3(int a, int probA, int b, int probB, int c, int probC) {
	debug(3, "SpiffGenerator::pickFrom3(%d, %d, %d, %d, %d, %d)", a, probA, b, probB, c, probC);
	int r = _rnd->getRandomNumber(probA + probB + probC);
	debug(3, "  r = %d", r);
	if (r < probA)
		return a;
	else if (r < probA + probB)
		return b;
	else
		return c;
}

int SpiffGenerator::pickFrom4(int a, int probA, int b, int probB, int c, int probC, int d, int probD) {
	debug(3, "SpiffGenerator::pickFrom4(%d, %d, %d, %d, %d, %d, %d, %d)", a, probA, b, probB, c, probC, d, probD);
	int r = _rnd->getRandomNumber(probA + probB + probC + probD);
	debug(3, "  r = %d", r);
	if (r < probA)
		return a;
	else if (r < probA + probB)
		return b;
	else if (r < probA + probB + probC)
		return c;
	else
		return d;
}

void SpiffGenerator::getSpecials() {
	// choose where the starting points and pools are
	int x, y, p, t;
	int edgeWaterA = (int)(islandsFlagG * totalMapSizeG / 16 + 0.5);
	int edgeWaterB = (int)(islandsFlagG * totalMapSizeG / 16); // don't put pools between islands

	// No matter what, they get a start hub spot.
	if (mirrorTypeG == MAXDISTMIRROR)
		x = (int)((totalMapSizeG * 3 + 8) / 16);
	else
		x = (int)(mapMiddleMaxG / 2);
	y = x;

	mapMiddle[x][y] = HUB; // hub start position

	for (p = 1; p <= numPoolsG; ++p) {
		x = _rnd->getRandomNumberRngSigned(edgeWaterA, mapMiddleMaxG - edgeWaterB);
		y = _rnd->getRandomNumberRngSigned(edgeWaterA, mapMiddleMaxG - edgeWaterB);
		if (mapMiddle[x][y] != UNASSIGNED)
			--p; // repick this pool
		else {
			t = pickFrom3(SMALLPOOL, 40000 * numPoolsG, MEDIUMPOOL, 20000 * numPoolsG + energyAmountG, LARGEPOOLTOP, 2 * energyAmountG);
			if (t == LARGEPOOLTOP) {
				if ((y == mapMiddleMaxG - edgeWaterB) || (mapMiddle[x][y + 1] != UNASSIGNED))
					t = SMALLPOOL; // keep large pool from being too high or overlapping another pool or start
				else
					mapMiddle[x][y + 1] = LARGEPOOLBOTTOM;
			}
			mapMiddle[x][y] = t;
		}
	}
}

void SpiffGenerator::copyMap(int XOffset, int YOffset, int XDirection, int YDirection) {
	// copies the first quadrant of the map
	// XOffset and YOffset are the distances moved
	// XDirection and/or YDirection are/is -1 for mirrored, 1 for not mirrored
	int x, y, tempMiddle, newCX;
	int newCY = YOffset;
	int newMX, newMY;

	if (YDirection < 0)
		newCY += mapCornerMaxG;

	for (y = 0; y <= mapCornerMaxG; ++y) {
		if (newCY < 0)
			newCY += totalMapSizeG;
		else if (newCY >= totalMapSizeG)
			newCY -= totalMapSizeG;

		newCX = XOffset;
		if (XDirection < 0)
			newCX += mapCornerMaxG;

		for (x = 0; x <= mapCornerMaxG; ++x) {
			if (newCX < 0)
				newCX += totalMapSizeG;
			else if (newCX >= totalMapSizeG)
				newCX -= totalMapSizeG;

			mapCorner[newCX][newCY] = mapCorner[x][y];
			if ((x != mapCornerMaxG) && (y != mapCornerMaxG)) {
				tempMiddle = mapMiddle[x][y];
				newMX = newCX;
				newMY = newCY;
				if (YDirection < 0) {
					newMY--;
					if (newMY == -1)
						newMY = totalMapSizeG - 1;
					if (tempMiddle == LARGEPOOLTOP)
						tempMiddle = LARGEPOOLBOTTOM;
					else if (tempMiddle == LARGEPOOLBOTTOM)
						tempMiddle = LARGEPOOLTOP;
				}
				if (XDirection < 0) {
					newMX--;
					if (newMX == -1)
						newMX = totalMapSizeG - 1;
				}
				mapMiddle[newMX][newMY] = tempMiddle;
			}

			newCX += XDirection;
		}

		newCY += YDirection;
	}
}

void SpiffGenerator::mirrorMap() {
	// --------------------------------------------------------------
	//  mirror map
	// --------------------------------------------------------------

	int swapXa = pickFrom2(-1, 1, 1, advancedMirrorOK_G);
	int swapYa = pickFrom2(-1, advancedMirrorOK_G, 1, 1);
	int swapXb = pickFrom2(-1, advancedMirrorOK_G, 1, 1);
	int swapYb = pickFrom2(-1, 1, 1, advancedMirrorOK_G);

	switch (mirrorTypeG) {
	case NORMALMIRROR: // four quadrants
		// ABCBA
		// DEFED
		// GHIHG
		// DEFED
		// ABCBA
		copyMap(mapCornerMaxG, 0, swapXa, swapYa);
		copyMap(0, mapCornerMaxG, swapXb, swapYb);
		copyMap(mapCornerMaxG, mapCornerMaxG, swapXa * swapXb, swapYa * swapYb);
		break;
	case XOFFSETMIRROR: // Like normal, but one half is moved horizontally by 1/4 totalmapsize
		// ABABABABA
		// DEFGHGFED
		// CDCDCDCDC
		// FGHGFEDEF
		// ABABABABA
		if (swapYa == -1) // ensures fairness
			swapXb = -1;
		copyMap(mapCornerMaxG, 0, 1, swapYa);
		copyMap(mapCornerMaxG / 2, mapCornerMaxG, swapXb, swapYb);
		copyMap(mapCornerMaxG * 3 / 2, mapCornerMaxG, swapXb, swapYa * swapYb);
		break;
	case YOFFSETMIRROR:   // Like normal, but one half is moved vertically by 1/4 totalmapsize
		if (swapXb == -1) // ensures fairness
			swapYa = -1;
		copyMap(mapCornerMaxG, mapCornerMaxG / 2, swapXa, swapYa);
		copyMap(0, mapCornerMaxG, swapXb, 1);
		copyMap(mapCornerMaxG, mapCornerMaxG * 3 / 2, swapXa * swapXb, swapYa);
		break;
	case MAXDISTMIRROR: // Allows maximum distance between starting points
	default:
		// ABCDCBA
		// E*GHIJE
		// HIJE*GH
		// DCBABCD
		// HG*EJIH
		// EJIHG*E
		// ABCDCBA

		copyMap(mapCornerMaxG, 0, 1, -1);
		copyMap(0, mapCornerMaxG, -1, 1);
		copyMap(mapCornerMaxG, mapCornerMaxG, -1, -1);
	}
}

void SpiffGenerator::errorCorrection() {
	// corrects errors caused by pool placement and mirroring
	// doesn't correct mapCorner[x][totalMapSizeG+1] or mapCorner[totalMapSizeG+1][y], since it isn't used

	// for any HIGH to LOW transitions, makes the HIGH MEDIUM
	// for pools on nonflat terrain, makes the terrain MEDIUM
	// removes invalid water

	int x;
	int y;
	int tempX;
	int tempY;
	int dx;
	int dy;
	int redo;
	int elev;

	for (y = 0; y < totalMapSizeG; ++y) {
		for (x = 0; x < totalMapSizeG; ++x) {
			if (mapCorner[x][y] == HIGH) {
				for (dy = -1; dy <= 1; ++dy) {
					tempY = y + dy;
					if (tempY == totalMapSizeG)
						tempY = 0;
					else if (tempY == -1)
						tempY = totalMapSizeG - 1;
				}

				for (dx = -1; dx <= 1; ++dx) {
					tempX = x + dx;
					if (tempX == totalMapSizeG)
						tempX = 0;
					else if (tempX == -1)
						tempX = totalMapSizeG - 1;
					if (mapCorner[tempX][tempY] == LOW)
						mapCorner[x][y] = MEDIUM;
				}
			} else if ((mapCorner[x][y] != LOW) && (mapCorner[x][y] != MEDIUM)) {
				mapCorner[x][y] = MEDIUM; // should not happen anymore
			}
		}
	}

	do {
		redo = 0;
		for (y = 0; y < totalMapSizeG; ++y) {
			for (x = 0; x < totalMapSizeG; ++x) {
				if (mapMiddle[x][y] != UNASSIGNED) {
					tempY = y + 1;
					if (tempY == totalMapSizeG)
						tempY = 0;

					tempX = x + 1;
					if (tempX == totalMapSizeG)
						tempX = 0;

					elev = mapCorner[x][y];
					if ((mapMiddle[x][y] == WATER) && (elev != LOW))
						mapMiddle[x][y] = UNASSIGNED;
					else if ((elev != mapCorner[x][tempY]) || (elev != mapCorner[tempX][y]) || (elev != mapCorner[tempX][tempY])) {
						if (mapMiddle[x][y] == WATER)
							mapMiddle[x][y] = UNASSIGNED;
						else {
							mapCorner[x][y] = MEDIUM;
							mapCorner[x][tempY] = MEDIUM;
							mapCorner[tempX][y] = MEDIUM;
							mapCorner[tempX][tempY] = MEDIUM;
							redo = 1;
						}
					}
				}
			}
		}
	} while (redo);
}

void SpiffGenerator::generate() {
	// --------------------------------------------------------------
	//  initialize
	// --------------------------------------------------------------
	int x;
	int y;
	int neighbors[MAXELEVVAL];
	int a;
	int b;
	int tempElevation;
	int nextElevation;
	int special;

	mapCornerMaxG = totalMapSizeG / 2;
	mapMiddleMaxG = mapCornerMaxG - 1;

	for (y = 0; y <= mapCornerMaxG; ++y) {
		// initialise map to UNASSIGNED tiles
		for (x = 0; x <= mapCornerMaxG; ++x) {
			mapCorner[x][y] = UNASSIGNED;
			mapMiddle[x][y] = UNASSIGNED;
		}
	}
	if (advancedMirrorOK_G)
		mirrorTypeG = pickFrom4(NORMALMIRROR, 1, XOFFSETMIRROR, 2, YOFFSETMIRROR, 2, MAXDISTMIRROR, 4);
	else
		mirrorTypeG = NORMALMIRROR;
	getSpecials(); // get start and pools

	// --------------------------------------------------------------
	//  loop through each square
	// --------------------------------------------------------------
	mapCorner[0][0] = pickFrom3(LOW, 1, MEDIUM, (terrainSeedFlagG < 9), HIGH, (terrainSeedFlagG < 8)); // seed
	// fill in the rest of the random map
	for (y = 0; y <= mapCornerMaxG; ++y) {
		for (x = 0; x <= mapCornerMaxG; ++x) {
			special = mapMiddle[x][y]; // water wouldn't have been assigned yet, so must be pool, start, or UNASSIGNED

			// --------------------------------------------------------------
			//  check neighbors
			// --------------------------------------------------------------
			if ((mapCorner[x][y] != UNASSIGNED) && (mapCorner[x][y] != LOW_OR_WATER))
				nextElevation = mapCorner[x][y]; // already defined because of a special or (0,0), so no change
			else {
				neighbors[HIGH] = 0;
				neighbors[MEDIUM] = 0;
				neighbors[LOW] = 0;
				neighbors[WATER] = 0;

				if (x > 0) {
					a = mapCorner[x - 1][y];
					if ((y > 1) && (mapMiddle[x - 1][y - 2] == WATER))
						++neighbors[WATER];
					if (y > 0)
						neighbors[mapCorner[x - 1][y - 1]] += 3;
				} else {
					a = mapCorner[x][y - 1];
				}

				neighbors[a] += 3;
				if (y > 0) {
					b = mapCorner[x][y - 1];
					neighbors[b] += 3;
					if (x < mapCornerMaxG) {
						++neighbors[mapCorner[x + 1][y - 1]]; // so this value can be ignored when choosing water
						if ((special != UNASSIGNED) && (x < mapCornerMaxG - 1))
							++neighbors[mapCorner[x + 2][y - 1]];
					}
					if ((x > 1) && (mapMiddle[x - 2][y - 1] == WATER))
						++neighbors[WATER];
				} else {
					b = mapCorner[x - 1][y]; // for probability equations for edges
				}

				// --------------------------------------------------------------
				//  pick new elevation
				// --------------------------------------------------------------
				// neighbors                  possible new elevation
				// HIGH or HIGH with MEDIUM   HIGH or MEDIUM
				// MEDIUM only                HIGH, MEDIUM or LOW
				// LOW or WATER only          MEDIUM, LOW or WATER
				// MEDIUM with LOW or WATER   MEDIUM or LOW, possible WATER if no MEDIUM left, down, or down-left
				// HIGH with LOW or WATER     MEDIUM

				static int highAmt = 105;
				static int mediumAmt = 100 + waterAmountG;
				static int lowAmt = 105 + 3 * waterAmountG;
				static int waterAmt = 15 * waterAmountG;



				if (neighbors[LOW]) {
					if (neighbors[HIGH]) { // HIGH with LOW or WATER
						nextElevation = MEDIUM;
					} else if (neighbors[MEDIUM] >= 3) { // MEDIUM with LOW or WATER
						if (a != b) {
							nextElevation = pickFrom2(LOW, lowAmt, MEDIUM, mediumAmt);
						} else if (a == LOW) {
							nextElevation = pickFrom2(LOW, 100 * lowAmt, MEDIUM, mediumAmt * cliffAmountG);
						} else {
							nextElevation = pickFrom2(LOW, lowAmt * cliffAmountG, MEDIUM, 100 * mediumAmt);
						}
					} else { // LOW or WATER only, possibly MEDIUM down-right
						if (neighbors[WATER] == 1) {
							nextElevation = pickFrom3(WATER, 100 * waterAmt, LOW, 100 * lowAmt, MEDIUM, mediumAmt * cliffAmountG);
						} else if (neighbors[WATER] == 0) {
							nextElevation = pickFrom3(WATER, waterAmt * cliffAmountG, LOW, 100 * lowAmt, MEDIUM, mediumAmt * cliffAmountG);
						} else {
							nextElevation = pickFrom3(WATER, 10000 * waterAmt, LOW, lowAmt * 100 * cliffAmountG, MEDIUM, mediumAmt * cliffAmountG * cliffAmountG);
						}
					}
				} else {
					if (neighbors[HIGH]) { // HIGH or HIGH with MEDIUM
						if (a != b) {
							nextElevation = pickFrom2(MEDIUM, mediumAmt, HIGH, highAmt);
						} else if (a == HIGH) {
							nextElevation = pickFrom2(MEDIUM, mediumAmt * cliffAmountG, HIGH, 100 * highAmt);
						} else {
							nextElevation = pickFrom2(MEDIUM, 100 * mediumAmt, HIGH, highAmt * cliffAmountG);
						}
					} else {
						nextElevation = pickFrom3(LOW, lowAmt * cliffAmountG, MEDIUM, 200 * mediumAmt, HIGH, highAmt * cliffAmountG);
					}
				}

				// --------------------------------------------------------------
				//  set elevation
				// --------------------------------------------------------------
				if ((mapCorner[x][y] == LOW_OR_WATER) && (nextElevation != WATER)) {
					// bottom and left edges of a special on LOW ground there may only be LOW or WATER
					nextElevation = LOW;
				}

				if (nextElevation == WATER) {
					if ((x != 0) && (y != 0) && (mapMiddle[x - 1][y - 1] == UNASSIGNED)) {
						mapMiddle[x - 1][y - 1] = WATER; // set WATER
					}
					nextElevation = LOW;
				}

				mapCorner[x][y] = nextElevation; // set elevation

				if (special != UNASSIGNED) { // if special, make flat spot (don't worry about going over map edge, will go into mirrored part)
					tempElevation = nextElevation;
					if (tempElevation == LOW)
						tempElevation = LOW_OR_WATER; // allow for water on left and bottom edges
					mapCorner[x + 1][y + 1] = nextElevation;
					mapCorner[x + 1][y] = tempElevation;
					mapCorner[x][y + 1] = tempElevation;
				}
			}
		}
	}

	if (islandsFlagG) { // replace borders with water, errorCorrection() finishes it.
		int edgeWaterA = (int)(islandsFlagG * totalMapSizeG / 16 + 0.5);
		int edgeWaterB = mapMiddleMaxG - (int)(islandsFlagG * totalMapSizeG / 16);
		for (y = 0; y <= mapCornerMaxG; ++y) {
			for (x = 0; x < edgeWaterA; ++x) {
				mapCorner[x][y] = LOW;
				mapMiddle[x][y] = WATER;
			}
			if (mapCorner[edgeWaterA + 1][y] == HIGH)
				mapCorner[edgeWaterA][y] = MEDIUM;

			for (x = mapMiddleMaxG; x > edgeWaterB; --x) {
				mapCorner[x + 1][y] = LOW;
				mapMiddle[x][y] = WATER;
			}
			if (mapCorner[edgeWaterB][y] == HIGH) {
				mapCorner[edgeWaterB + 1][y] = MEDIUM;
			}
		}

		for (x = edgeWaterA; x <= edgeWaterB + 1; ++x) {
			for (y = 0; y < edgeWaterA; ++y) {
				mapCorner[x][y] = LOW;
				mapMiddle[x][y] = WATER;
			}
			if (mapCorner[x][edgeWaterA + 1] == HIGH)
				mapCorner[x][edgeWaterA] = MEDIUM;

			for (y = mapMiddleMaxG; y > edgeWaterB; --y) {
				mapCorner[x][y + 1] = LOW;
				mapMiddle[x][y] = WATER;
			}
			if (mapCorner[x][edgeWaterB] == HIGH) {
				mapCorner[x][edgeWaterB + 1] = MEDIUM;
			}
		}

		if (islandsFlagG == 2) { // add tiny islands to help bridge wide channels
			int j;
			for (int i = 0; i < totalMapSizeG / 16; ++i) {
				x = (int)(totalMapSizeG / 16 - .5);
				y = _rnd->getRandomNumberRngSigned(x, totalMapSizeG / 2 - 1 - x);
				if (_rnd->getRandomBit()) {
					x = totalMapSizeG / 2 - 1 - x;
				}
				if (_rnd->getRandomBit()) {
					mapMiddle[x][y] = UNASSIGNED;
					for (j = 0; j < 4; ++j) {
						mapMiddle[x + _rnd->getRandomNumberRngSigned(-1, 1)][y + _rnd->getRandomNumberRngSigned(-1, 1)] = UNASSIGNED;
					}
				} else {
					mapMiddle[y][x] = UNASSIGNED;
					for (j = 0; j < 4; ++j) {
						mapMiddle[y + _rnd->getRandomNumberRngSigned(-1, 1)][x + _rnd->getRandomNumberRngSigned(-1, 1)] = UNASSIGNED;
					}
				}
			}
		}
	}

	mirrorMap();
	errorCorrection();
}

} // End of namespace Scumm
