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

#ifndef SCUMM_HE_MOONBASE_MAP_SPIFF
#define SCUMM_HE_MOONBASE_MAP_SPIFF

#ifdef ENABLE_HE

#include "common/random.h"

#include "engines/scumm/he/moonbase/map_mif.h"

#define MAXELEVVAL 4 // for array size
#define HIGH 3 // elevations
#define MEDIUM 2
#define LOW 1
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
	SpiffGenerator(Common::RandomSource *rnd);
	~SpiffGenerator();

	MapFile *generateMap(int water, int mapSize, int energy, int terrain);

private:
	Common::RandomSource *_rnd;

	int numPoolsG; // per quadrant
	int energyAmountG; // 2048 = min energy on small map, 51200 = max energy on max map, etc.
	int cliffAmountG; // amount of cliffs, 10 is min, 70 is max
	int waterAmountG; // 0 is min, 30 is max
	int totalMapSizeG;

	int terrainSeedFlagG; // disables HIGH or LOW terrain for the initial elevation when appropriate
	int islandsFlagG; // enables islands
	int advancedMirrorOK_G; // low terrain roughness can leave too abrupt changes at the edge, so set false to disable some mirroring types
	int mirrorTypeG; // what mirroring is used

	int mapCornerMaxG; // size of random section
	int mapMiddleMaxG;
	int mapCorner[MAXSIZE+1][MAXSIZE+1];
	int mapMiddle[MAXSIZE][MAXSIZE];

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

#endif // SCUMM_HE_MOONBASE_MAP_SPIFF
