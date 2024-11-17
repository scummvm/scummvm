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

#ifndef SCUMM_HE_MOONBASE_MAP_SPIFF_H
#define SCUMM_HE_MOONBASE_MAP_SPIFF_H

#ifdef ENABLE_HE

#include "engines/scumm/he/moonbase/map_mif.h"

#define MAXELEVVAL 4 // for array size

// These were "HIGH", "MEDIUM", and "LOW", but
// was renamed to prevent potential clashing
// with different platforms.
#define kElevHigh 3 // elevations
#define kElevMedium 2
#define kElevLow 1

#define WATER 0 // special types
#define HUB 1
#define SMALLPOOL 2
#define MEDIUMPOOL 3
#define LARGEPOOLTOP 4
#define LARGEPOOLBOTTOM 5 // placeholder for top half of a large pool

#define UNASSIGNED -1
#define LOW_OR_WATER -2

#define MAXSIZE 80

#define NORMALMIRROR 0
#define XOFFSETMIRROR 1
#define YOFFSETMIRROR 2
#define MAXDISTMIRROR 3

namespace Scumm {

class SpiffGenerator {
public:
	SpiffGenerator(int seed);
	~SpiffGenerator() = default;

	MapFile *generateMap(int water, int tileset, int mapSize, int energy, int terrain);

private:
	int _seed = 0;

	int _numPoolsG = 0; // per quadrant
	int _energyAmountG = 0; // 2048 = min energy on small map, 51200 = max energy on max map, etc.
	int _cliffAmountG = 0;  // amount of cliffs, 10 is min, 70 is max
	int _waterAmountG = 0;  // 0 is min, 30 is max
	int _totalMapSizeG = 0;

	int _terrainSeedFlagG = 0; // disables kElevHigh or kElevLow terrain for the initial elevation when appropriate
	int _islandsFlagG = 0;     // enables islands
	int _advancedMirrorOK_G = 0; // low terrain roughness can leave too abrupt changes at the edge, so set false to disable some mirroring types
	int _mirrorTypeG = 0;        // what mirroring is used

	int _mapCornerMaxG = 0; // size of random section
	int _mapMiddleMaxG = 0;
	int _mapCorner[MAXSIZE+1][MAXSIZE+1] = { {}, {} };
	int _mapMiddle[MAXSIZE][MAXSIZE] = { {}, {} };

	float getRandomFloat();
	int spiffRand(int min, int max);
	int pickFrom2(int a, int probA, int b, int probB);
	int pickFrom3(int a, int probA, int b, int probB, int c, int probC);
	int pickFrom4(int a, int probA, int b, int probB, int c, int probC, int d, int probD);

	void getSpecials();
	void copyMap(int XOffset, int YOffset, int XDirection, int YDirection);
	void mirrorMap();
	void errorCorrection();
	void generate();
};

} // End of namespace Scumm

#endif // ENABLE_HE

#endif // SCUMM_HE_MOONBASE_MAP_SPIFF_H
