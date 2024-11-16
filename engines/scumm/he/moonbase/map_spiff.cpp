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

SpiffGenerator::SpiffGenerator(int seed) {
	_seed = seed;
}

MapFile *SpiffGenerator::generateMap(int water, int tileset, int mapSize, int energy, int terrain) {
	_totalMapSizeG = mapSize;
	_energyAmountG = (2 + energy) * _totalMapSizeG * _totalMapSizeG;

	_islandsFlagG = pickFrom2(0, 1, water - 4, (water >= 5)); // 1 is large islands, 2 is small
	if (_islandsFlagG) {
		water -= 3;
		_energyAmountG = (int)(_energyAmountG * (5 - _islandsFlagG) / 6); // *2/3 or *1/2
	}

	_waterAmountG = 4 * water;
	_cliffAmountG = 1 << terrain;
	_advancedMirrorOK_G = ((terrain > 1) && (water < 6)) || _islandsFlagG;
	_terrainSeedFlagG = 2 * water - terrain;

	int n = (int)(_energyAmountG / 2700);
	if (n > 12) {
		n = 12;
	}
	if (n < 1) {
		n = 1;
	}
	_numPoolsG = spiffRand((int)(_energyAmountG / 4000) + 1, n);
	if (_numPoolsG > 12) {
		_numPoolsG = 12;
	}

	generate();

	// Populate MIF for map data generation:
	MIF mif = MIF();

	int levelMap[MAXELEVVAL];
	levelMap[kElevHigh] = 2;
	levelMap[kElevMedium] = 1;
	levelMap[kElevLow] = 0;

	mif._mapType = tileset;
	Common::sprintf_s(mif._name, "Spiff %04X", (uint16)_seed);

	mif._dimension = _totalMapSizeG;

	int y;
	int x;
	byte t;
	int XOffset = spiffRand(0, _totalMapSizeG-1);
	int YOffset = spiffRand(0, _totalMapSizeG-1);
	int newX;
	int newY;

	for (y = 0, newY = YOffset; y < _totalMapSizeG; ++y, ++newY) {
		for (x = 0, newX = XOffset; x < _totalMapSizeG; ++x, ++newX) {
			if (newX == _totalMapSizeG)
				newX = 0;
			if (newY == _totalMapSizeG)
				newY = 0;
			mif._cornerMap[newX][newY] = levelMap[_mapCorner[x][y]];
			switch (_mapMiddle[x][y]) {
			case HUB:
				t = 0xff;
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
			mif._centerMap[newX][newY] = t;
		}
	}

	// Generate new map:
	MapFile *map = new MapFile();
	mif.generateMap(map);

	return map;
}

float SpiffGenerator::getRandomFloat() {
	// This is the exact linear congruential generator
	// algorithm used on MSVCRT (Windows Visual C++ Runtime), with
	// RAND_MAX being 0x7fff (32767).  This is implemented here
	// to match the RNG between the original Moonbase Console
	// program and ScummVM.
	//
	// (Common::RandomSource uses Xorshift and uses unsigned
	// integers compared to MSVCRT's rand)
	_seed = _seed * 214013 + 2531011;
	return (float)((_seed >> 16) & 0x7fff) / 32767;
}

int SpiffGenerator::spiffRand(int min, int max) {
	// returns a random integer min to max inclusive
	return ((int)(getRandomFloat() * (max + 1 - min))) + min;
}

int SpiffGenerator::pickFrom2(int a, int probA, int b, int probB) {
	debug(3, "SpiffGenerator::pickFrom2(%d, %d, %d, %d)", a, probA, b, probB);
	float r = getRandomFloat() * (probA + probB);
	debug(3, "  r = %f", r);
	if (r < probA)
		return a;
	else
		return b;
}

int SpiffGenerator::pickFrom3(int a, int probA, int b, int probB, int c, int probC) {
	debug(3, "SpiffGenerator::pickFrom3(%d, %d, %d, %d, %d, %d)", a, probA, b, probB, c, probC);
	float r = getRandomFloat() * (probA + probB + probC);
	debug(3, "  r = %f", r);
	if (r < probA)
		return a;
	else if (r < probA + probB)
		return b;
	else
		return c;
}

int SpiffGenerator::pickFrom4(int a, int probA, int b, int probB, int c, int probC, int d, int probD) {
	debug(3, "SpiffGenerator::pickFrom4(%d, %d, %d, %d, %d, %d, %d, %d)", a, probA, b, probB, c, probC, d, probD);
	float r = getRandomFloat() * (probA + probB + probC + probD);
	debug(3, "  r = %f", r);
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
	int edgeWaterA = (int)(_islandsFlagG * _totalMapSizeG / 16 + 0.5);
	int edgeWaterB = (int)(_islandsFlagG * _totalMapSizeG / 16); // don't put pools between islands

	// No matter what, they get a start hub spot.
	if (_mirrorTypeG == MAXDISTMIRROR)
		x = (int)((_totalMapSizeG * 3 + 8) / 16);
	else
		x = (int)(_mapMiddleMaxG / 2);
	y = x;

	_mapMiddle[x][y] = HUB; // hub start position

	for (p = 1; p <= _numPoolsG; ++p) {
		x = spiffRand(edgeWaterA, _mapMiddleMaxG - edgeWaterB);
		y = spiffRand(edgeWaterA, _mapMiddleMaxG - edgeWaterB);
		if (_mapMiddle[x][y] != UNASSIGNED)
			--p; // repick this pool
		else {
			t = pickFrom3(SMALLPOOL, 40000 * _numPoolsG, MEDIUMPOOL, 20000 * _numPoolsG + _energyAmountG, LARGEPOOLTOP, 2 * _energyAmountG);
			if (t == LARGEPOOLTOP) {
				if ((y == _mapMiddleMaxG - edgeWaterB) || (_mapMiddle[x][y + 1] != UNASSIGNED))
					t = SMALLPOOL; // keep large pool from being too high or overlapping another pool or start
				else
					_mapMiddle[x][y + 1] = LARGEPOOLBOTTOM;
			}
			_mapMiddle[x][y] = t;
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
		newCY += _mapCornerMaxG;

	for (y = 0; y <= _mapCornerMaxG; ++y) {
		if (newCY < 0)
			newCY += _totalMapSizeG;
		else if (newCY >= _totalMapSizeG)
			newCY -= _totalMapSizeG;

		newCX = XOffset;
		if (XDirection < 0)
			newCX += _mapCornerMaxG;

		for (x = 0; x <= _mapCornerMaxG; ++x) {
			if (newCX < 0)
				newCX += _totalMapSizeG;
			else if (newCX >= _totalMapSizeG)
				newCX -= _totalMapSizeG;

			_mapCorner[newCX][newCY] = _mapCorner[x][y];
			if ((x != _mapCornerMaxG) && (y != _mapCornerMaxG)) {
				tempMiddle = _mapMiddle[x][y];
				newMX = newCX;
				newMY = newCY;
				if (YDirection < 0) {
					newMY--;
					if (newMY == -1)
						newMY = _totalMapSizeG - 1;
					if (tempMiddle == LARGEPOOLTOP)
						tempMiddle = LARGEPOOLBOTTOM;
					else if (tempMiddle == LARGEPOOLBOTTOM)
						tempMiddle = LARGEPOOLTOP;
				}
				if (XDirection < 0) {
					newMX--;
					if (newMX == -1)
						newMX = _totalMapSizeG - 1;
				}
				_mapMiddle[newMX][newMY] = tempMiddle;
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

	int swapXa = pickFrom2(-1, 1, 1, _advancedMirrorOK_G);
	int swapYa = pickFrom2(-1, _advancedMirrorOK_G, 1, 1);
	int swapXb = pickFrom2(-1, _advancedMirrorOK_G, 1, 1);
	int swapYb = pickFrom2(-1, 1, 1, _advancedMirrorOK_G);

	switch (_mirrorTypeG) {
	case NORMALMIRROR: // four quadrants
		// ABCBA
		// DEFED
		// GHIHG
		// DEFED
		// ABCBA
		copyMap(_mapCornerMaxG, 0, swapXa, swapYa);
		copyMap(0, _mapCornerMaxG, swapXb, swapYb);
		copyMap(_mapCornerMaxG, _mapCornerMaxG, swapXa * swapXb, swapYa * swapYb);
		break;
	case XOFFSETMIRROR: // Like normal, but one half is moved horizontally by 1/4 totalmapsize
		// ABABABABA
		// DEFGHGFED
		// CDCDCDCDC
		// FGHGFEDEF
		// ABABABABA
		if (swapYa == -1) // ensures fairness
			swapXb = -1;
		copyMap(_mapCornerMaxG, 0, 1, swapYa);
		copyMap(_mapCornerMaxG / 2, _mapCornerMaxG, swapXb, swapYb);
		copyMap(_mapCornerMaxG * 3 / 2, _mapCornerMaxG, swapXb, swapYa * swapYb);
		break;
	case YOFFSETMIRROR:   // Like normal, but one half is moved vertically by 1/4 totalmapsize
		if (swapXb == -1) // ensures fairness
			swapYa = -1;
		copyMap(_mapCornerMaxG, _mapCornerMaxG / 2, swapXa, swapYa);
		copyMap(0, _mapCornerMaxG, swapXb, 1);
		copyMap(_mapCornerMaxG, _mapCornerMaxG * 3 / 2, swapXa * swapXb, swapYa);
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

		copyMap(_mapCornerMaxG, 0, 1, -1);
		copyMap(0, _mapCornerMaxG, -1, 1);
		copyMap(_mapCornerMaxG, _mapCornerMaxG, -1, -1);
	}
}

void SpiffGenerator::errorCorrection() {
	// corrects errors caused by pool placement and mirroring
	// doesn't correct _mapCorner[x][_totalMapSizeG+1] or _mapCorner[_totalMapSizeG+1][y], since it isn't used

	// for any kElevHigh to kElevLow transitions, makes the kElevHigh kElevMedium
	// for pools on nonflat terrain, makes the terrain kElevMedium
	// removes invalid water

	int x;
	int y;
	int tempX;
	int tempY;
	int dx;
	int dy;
	int redo;
	int elev;

	for (y = 0; y < _totalMapSizeG; ++y) {
		for (x = 0; x < _totalMapSizeG; ++x) {
			if (_mapCorner[x][y] == kElevHigh) {
				for (dy = -1; dy <= 1; ++dy) {
					tempY = y + dy;
					if (tempY == _totalMapSizeG) {
						tempY = 0;
					} else if (tempY == -1) {
						tempY = _totalMapSizeG - 1;
					}

					for (dx = -1; dx <= 1; ++dx) {
						tempX = x + dx;
						if (tempX == _totalMapSizeG) {
							tempX = 0;
						} else if (tempX == -1) {
							tempX = _totalMapSizeG - 1;
						}

						if (_mapCorner[tempX][tempY] == kElevLow) {
							_mapCorner[x][y] = kElevMedium;
						}
					}
				}
			} else if ((_mapCorner[x][y] != kElevLow) && (_mapCorner[x][y] != kElevMedium)) {
				_mapCorner[x][y] = kElevMedium; // should not happen anymore
			}
		}
	}

	do {
		redo = 0;
		for (y = 0; y < _totalMapSizeG; ++y) {
			for (x = 0; x < _totalMapSizeG; ++x) {
				if (_mapMiddle[x][y] != UNASSIGNED) {
					tempY = y + 1;
					if (tempY == _totalMapSizeG)
						tempY = 0;

					tempX = x + 1;
					if (tempX == _totalMapSizeG)
						tempX = 0;

					elev = _mapCorner[x][y];
					if ((_mapMiddle[x][y] == WATER) && (elev != kElevLow))
						_mapMiddle[x][y] = UNASSIGNED;
					else if ((elev != _mapCorner[x][tempY]) || (elev != _mapCorner[tempX][y]) || (elev != _mapCorner[tempX][tempY])) {
						if (_mapMiddle[x][y] == WATER)
							_mapMiddle[x][y] = UNASSIGNED;
						else {
							_mapCorner[x][y] = kElevMedium;
							_mapCorner[x][tempY] = kElevMedium;
							_mapCorner[tempX][y] = kElevMedium;
							_mapCorner[tempX][tempY] = kElevMedium;
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

	_mapCornerMaxG = _totalMapSizeG / 2;
	_mapMiddleMaxG = _mapCornerMaxG - 1;

	for (y = 0; y <= _mapCornerMaxG; ++y) {
		// initialise map to UNASSIGNED tiles
		for (x = 0; x <= _mapCornerMaxG; ++x) {
			_mapCorner[x][y] = UNASSIGNED;
			_mapMiddle[x][y] = UNASSIGNED;
		}
	}
	if (_advancedMirrorOK_G)
		_mirrorTypeG = pickFrom4(NORMALMIRROR, 1, XOFFSETMIRROR, 2, YOFFSETMIRROR, 2, MAXDISTMIRROR, 4);
	else
		_mirrorTypeG = NORMALMIRROR;
	getSpecials(); // get start and pools

	// --------------------------------------------------------------
	//  loop through each square
	// --------------------------------------------------------------
	_mapCorner[0][0] = pickFrom3(kElevLow, 1, kElevMedium, (_terrainSeedFlagG < 9), kElevHigh, (_terrainSeedFlagG < 8)); // seed
	// fill in the rest of the random map
	for (y = 0; y <= _mapCornerMaxG; ++y) {
		for (x = 0; x <= _mapCornerMaxG; ++x) {
			special = _mapMiddle[x][y]; // water wouldn't have been assigned yet, so must be pool, start, or UNASSIGNED

			// --------------------------------------------------------------
			//  check neighbors
			// --------------------------------------------------------------
			if ((_mapCorner[x][y] != UNASSIGNED) && (_mapCorner[x][y] != LOW_OR_WATER))
				nextElevation = _mapCorner[x][y]; // already defined because of a special or (0,0), so no change
			else {
				neighbors[kElevHigh] = 0;
				neighbors[kElevMedium] = 0;
				neighbors[kElevLow] = 0;
				neighbors[WATER] = 0;

				if (x > 0) {
					a = _mapCorner[x - 1][y];
					if ((y > 1) && (_mapMiddle[x - 1][y - 2] == WATER))
						++neighbors[WATER];
					if (y > 0)
						neighbors[_mapCorner[x - 1][y - 1]] += 3;
				} else {
					a = _mapCorner[x][y - 1];
				}

				neighbors[a] += 3;
				if (y > 0) {
					b = _mapCorner[x][y - 1];
					neighbors[b] += 3;
					if (x < _mapCornerMaxG) {
						++neighbors[_mapCorner[x + 1][y - 1]]; // so this value can be ignored when choosing water
						if ((special != UNASSIGNED) && (x < _mapCornerMaxG - 1))
							++neighbors[_mapCorner[x + 2][y - 1]];
					}
					if ((x > 1) && (_mapMiddle[x - 2][y - 1] == WATER))
						++neighbors[WATER];
				} else {
					b = _mapCorner[x - 1][y]; // for probability equations for edges
				}

				// --------------------------------------------------------------
				//  pick new elevation
				// --------------------------------------------------------------
				// neighbors                  possible new elevation
				// kElevHigh or kElevHigh with kElevMedium   kElevHigh or kElevMedium
				// kElevMedium only                kElevHigh, kElevMedium or kElevLow
				// kElevLow or WATER only          kElevMedium, kElevLow or WATER
				// kElevMedium with kElevLow or WATER   kElevMedium or kElevLow, possible WATER if no kElevMedium left, down, or down-left
				// kElevHigh with kElevLow or WATER     kElevMedium

				static int highAmt = 105;
				static int mediumAmt = 100 + _waterAmountG;
				static int lowAmt = 105 + 3 * _waterAmountG;
				static int waterAmt = 15 * _waterAmountG;



				if (neighbors[kElevLow]) {
					if (neighbors[kElevHigh]) { // kElevHigh with kElevLow or WATER
						nextElevation = kElevMedium;
					} else if (neighbors[kElevMedium] >= 3) { // kElevMedium with kElevLow or WATER
						if (a != b) {
							nextElevation = pickFrom2(kElevLow, lowAmt, kElevMedium, mediumAmt);
						} else if (a == kElevLow) {
							nextElevation = pickFrom2(kElevLow, 100 * lowAmt, kElevMedium, mediumAmt * _cliffAmountG);
						} else {
							nextElevation = pickFrom2(kElevLow, lowAmt * _cliffAmountG, kElevMedium, 100 * mediumAmt);
						}
					} else { // kElevLow or WATER only, possibly kElevMedium down-right
						if (neighbors[WATER] == 1) {
							nextElevation = pickFrom3(WATER, 100 * waterAmt, kElevLow, 100 * lowAmt, kElevMedium, mediumAmt * _cliffAmountG);
						} else if (neighbors[WATER] == 0) {
							nextElevation = pickFrom3(WATER, waterAmt * _cliffAmountG, kElevLow, 100 * lowAmt, kElevMedium, mediumAmt * _cliffAmountG);
						} else {
							nextElevation = pickFrom3(WATER, 10000 * waterAmt, kElevLow, lowAmt * 100 * _cliffAmountG, kElevMedium, mediumAmt * _cliffAmountG * _cliffAmountG);
						}
					}
				} else {
					if (neighbors[kElevHigh]) { // kElevHigh or kElevHigh with kElevMedium
						if (a != b) {
							nextElevation = pickFrom2(kElevMedium, mediumAmt, kElevHigh, highAmt);
						} else if (a == kElevHigh) {
							nextElevation = pickFrom2(kElevMedium, mediumAmt * _cliffAmountG, kElevHigh, 100 * highAmt);
						} else {
							nextElevation = pickFrom2(kElevMedium, 100 * mediumAmt, kElevHigh, highAmt * _cliffAmountG);
						}
					} else {
						nextElevation = pickFrom3(kElevLow, lowAmt * _cliffAmountG, kElevMedium, 200 * mediumAmt, kElevHigh, highAmt * _cliffAmountG);
					}
				}

				// --------------------------------------------------------------
				//  set elevation
				// --------------------------------------------------------------
				if ((_mapCorner[x][y] == LOW_OR_WATER) && (nextElevation != WATER)) {
					// bottom and left edges of a special on kElevLow ground there may only be kElevLow or WATER
					nextElevation = kElevLow;
				}

				if (nextElevation == WATER) {
					if ((x != 0) && (y != 0) && (_mapMiddle[x - 1][y - 1] == UNASSIGNED)) {
						_mapMiddle[x - 1][y - 1] = WATER; // set WATER
					}
					nextElevation = kElevLow;
				}

				_mapCorner[x][y] = nextElevation; // set elevation

				if (special != UNASSIGNED) { // if special, make flat spot (don't worry about going over map edge, will go into mirrored part)
					tempElevation = nextElevation;
					if (tempElevation == kElevLow)
						tempElevation = LOW_OR_WATER; // allow for water on left and bottom edges
					_mapCorner[x + 1][y + 1] = nextElevation;
					_mapCorner[x + 1][y] = tempElevation;
					_mapCorner[x][y + 1] = tempElevation;
				}
			}
		}
	}

	if (_islandsFlagG) { // replace borders with water, errorCorrection() finishes it.
		int edgeWaterA = (int)(_islandsFlagG * _totalMapSizeG / 16 + 0.5);
		int edgeWaterB = _mapMiddleMaxG - (int)(_islandsFlagG * _totalMapSizeG / 16);
		for (y = 0; y <= _mapCornerMaxG; ++y) {
			for (x = 0; x < edgeWaterA; ++x) {
				_mapCorner[x][y] = kElevLow;
				_mapMiddle[x][y] = WATER;
			}
			if (_mapCorner[edgeWaterA + 1][y] == kElevHigh)
				_mapCorner[edgeWaterA][y] = kElevMedium;

			for (x = _mapMiddleMaxG; x > edgeWaterB; --x) {
				_mapCorner[x + 1][y] = kElevLow;
				_mapMiddle[x][y] = WATER;
			}
			if (_mapCorner[edgeWaterB][y] == kElevHigh) {
				_mapCorner[edgeWaterB + 1][y] = kElevMedium;
			}
		}

		for (x = edgeWaterA; x <= edgeWaterB + 1; ++x) {
			for (y = 0; y < edgeWaterA; ++y) {
				_mapCorner[x][y] = kElevLow;
				_mapMiddle[x][y] = WATER;
			}
			if (_mapCorner[x][edgeWaterA + 1] == kElevHigh)
				_mapCorner[x][edgeWaterA] = kElevMedium;

			for (y = _mapMiddleMaxG; y > edgeWaterB; --y) {
				_mapCorner[x][y + 1] = kElevLow;
				_mapMiddle[x][y] = WATER;
			}
			if (_mapCorner[x][edgeWaterB] == kElevHigh) {
				_mapCorner[x][edgeWaterB + 1] = kElevMedium;
			}
		}

		if (_islandsFlagG == 2) { // add tiny islands to help bridge wide channels
			int j;
			for (int i = 0; i < _totalMapSizeG / 16; ++i) {
				x = (int)(_totalMapSizeG / 16 - .5);
				y = spiffRand(x, _totalMapSizeG / 2 - 1 - x);
				if (spiffRand(0, 1)) {
					x = _totalMapSizeG / 2 - 1 - x;
				}
				if (spiffRand(0, 1)) {
					_mapMiddle[x][y] = UNASSIGNED;
					for (j = 0; j < 4; ++j) {
						_mapMiddle[x + spiffRand(-1, 1)][y + spiffRand(-1, 1)] = UNASSIGNED;
					}
				} else {
					_mapMiddle[y][x] = UNASSIGNED;
					for (j = 0; j < 4; ++j) {
						_mapMiddle[y + spiffRand(-1, 1)][x + spiffRand(-1, 1)] = UNASSIGNED;
					}
				}
			}
		}
	}

	mirrorMap();
	errorCorrection();
}

} // End of namespace Scumm
