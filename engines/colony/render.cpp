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

#include "colony/colony.h"
#include "colony/gfx.h"
#include "common/system.h"
#include "common/util.h"
#include "common/algorithm.h"
#include "common/debug.h"
#include <math.h>

namespace Colony {

static const int g_indexTable[4][10] = {
	{0, 0,  0, 0,  0,  1,  1,  0,  1, 2},
	{1, 0,  0, 0, -1,  0,  0,  1,  2, 1},
	{0, 1,  1, 0,  0, -1, -1,  0,  1, 2},
	{0, 0,  0, 1,  1,  0,  0, -1,  2, 1}
};

// DOS object geometry constants
static const int kScreenPts[8][3] = {
	{-16, 64, 0}, {16, 64, 0}, {16, -64, 0}, {-16, -64, 0},
	{-16, 64, 288}, {16, 64, 288}, {16, -64, 288}, {-16, -64, 288}
};
static const int kScreenSurf[4][8] = {
	{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
	{0, 4, 2, 1, 5, 6, 0, 0}, {0, 4, 7, 6, 5, 4, 0, 0}
};
static const int kTableTopPts[4][3] = {
	{-64, 64, 100}, {64, 64, 100}, {64, -64, 100}, {-64, -64, 100}
};
static const int kTableTopSurf[1][8] = {{0, 4, 3, 2, 1, 0, 0, 0}};
static const int kTableBasePts[8][3] = {
	{-20, 20, 0}, {20, 20, 0}, {20, -20, 0}, {-20, -20, 0},
	{-20, 20, 100}, {20, 20, 100}, {20, -20, 100}, {-20, -20, 100}
};
static const int kTableBaseSurf[4][8] = {
	{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
	{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0}
};
static const int kBedPostPts[4][3] = {
	{-80, 180, 0}, {80, 180, 0}, {80, 180, 100}, {-80, 180, 100}
};
static const int kBedPostSurf[1][8] = {{0, 4, 3, 2, 1, 0, 0, 0}};
static const int kBedBlanketPts[8][3] = {
	{-80, 70, 0}, {80, 70, 0}, {80, -175, 0}, {-80, -175, 0},
	{-80, 70, 60}, {80, 70, 60}, {80, -175, 60}, {-80, -175, 60}
};
static const int kBlanketSurf[4][8] = {
	{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
	{0, 4, 2, 1, 5, 6, 0, 0}, {0, 4, 7, 6, 5, 4, 0, 0}
};
static const int kBedSheetPts[8][3] = {
	{-80, 70, 60}, {80, 70, 60}, {80, -175, 60}, {-80, -175, 60},
	{-80, 70, 80}, {80, 70, 80}, {80, -175, 80}, {-80, -175, 80}
};
static const int kSheetSurf[3][8] = {
	{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0},
	{0, 4, 7, 6, 5, 4, 0, 0}
};
static const int kBBedBlanketPts[8][3] = {
	{-120, 96, 0}, {120, 96, 0}, {120, -96, 0}, {-120, -96, 0},
	{-120, 96, 60}, {120, 96, 60}, {120, -96, 60}, {-120, -96, 60}
};
static const int kBBedSheetPts[8][3] = {
	{-120, 96, 60}, {120, 96, 60}, {120, -96, 60}, {-120, -96, 60},
	{-120, 96, 80}, {120, 96, 80}, {120, -96, 80}, {-120, -96, 80}
};
static const int kBBedPostPts[4][3] = {
	{-120, 96, 0}, {120, 96, 0}, {120, 96, 100}, {-120, 96, 100}
};
static const int kDeskTopPts[4][3] = {
	{-128, 80, 100}, {128, 80, 100}, {128, -80, 100}, {-128, -80, 100}
};
static const int kDeskTopSurf[1][8] = {{0, 4, 3, 2, 1, 0, 0, 0}};
static const int kDeskLeftPts[8][3] = {
	{-128, 80, 0}, {-60, 80, 0}, {-60, -80, 0}, {-128, -80, 0},
	{-128, 80, 100}, {-60, 80, 100}, {-60, -80, 100}, {-128, -80, 100}
};
static const int kDeskRightPts[8][3] = {
	{60, 80, 0}, {128, 80, 0}, {128, -80, 0}, {60, -80, 0},
	{60, 80, 100}, {128, 80, 100}, {128, -80, 100}, {60, -80, 100}
};
static const int kDeskCabSurf[4][8] = {
	{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
	{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0}
};
static const int kSeatPts[4][3] = {
	{-40, 210, 60}, {40, 210, 60}, {40, 115, 60}, {-40, 115, 60}
};
static const int kSeatSurf[1][8] = {{0, 4, 3, 2, 1, 0, 0, 0}};
static const int kArmLeftPts[4][3] = {
	{-40, 210, 90}, {-40, 210, 0}, {-40, 115, 0}, {-40, 115, 90}
};
static const int kArmRightPts[4][3] = {
	{40, 115, 90}, {40, 115, 0}, {40, 210, 0}, {40, 210, 90}
};
static const int kArmSurf[2][8] = {
	{0, 4, 3, 2, 1, 0, 0, 0}, {0, 3, 3, 0, 2, 0, 0, 0}
};
static const int kBackPts[4][3] = {
	{-40, 115, 130}, {40, 115, 130}, {40, 115, 0}, {-40, 115, 0}
};
static const int kBackSurf[2][8] = {
	{0, 4, 3, 2, 1, 0, 0, 0}, {0, 3, 3, 0, 2, 0, 0, 0}
};
static const int kComputerPts[8][3] = {
	{-50, 60, 100}, {50, 60, 100}, {50, -60, 100}, {-50, -60, 100},
	{-50, 60, 130}, {50, 60, 130}, {50, -60, 130}, {-50, -60, 130}
};
static const int kComputerSurf[5][8] = {
	{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
	{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0},
	{0, 4, 7, 6, 5, 4, 0, 0}
};
static const int kMonitorPts[8][3] = {
	{-40, 60, 130}, {40, 60, 130}, {40, -60, 130}, {-40, -60, 130},
	{-40, 60, 200}, {40, 60, 200}, {40, -60, 200}, {-40, -60, 200}
};
static const int kDeskScreenPts[4][3] = {
	{-30, -60, 135}, {30, -60, 135}, {30, -60, 195}, {-30, -60, 195}
};
static const int kDeskScreenSurf[1][8] = {{0, 4, 3, 2, 1, 0, 0, 0}};
static const int kCSeatPts[4][3] = {
	{-40, 40, 60}, {40, 40, 60}, {40, -40, 60}, {-40, -40, 60}
};
static const int kCSeatSurf[1][8] = {{0, 4, 3, 2, 1, 0, 0, 0}};
static const int kCArmLeftPts[4][3] = {
	{-50, 40, 90}, {-40, 40, 60}, {-40, -40, 60}, {-50, -40, 90}
};
static const int kCArmLeftSurf[1][8] = {{0, 4, 3, 2, 1, 0, 0, 0}};
static const int kCArmRightPts[4][3] = {
	{40, -40, 60}, {40, 40, 60}, {50, 40, 90}, {50, -40, 90}
};
static const int kCArmRightSurf[1][8] = {{0, 4, 3, 2, 1, 0, 0, 0}};
static const int kCBackPts[4][3] = {
	{-50, -40, 120}, {50, -40, 120}, {50, -40, 60}, {-50, -40, 60}
};
static const int kCBackSurf[2][8] = {
	{0, 4, 3, 2, 1, 0, 0, 0}, {0, 3, 3, 0, 2, 0, 0, 0}
};
static const int kCBasePts[8][3] = {
	{-8, 8, 0}, {8, 8, 0}, {8, -8, 0}, {-8, -8, 0},
	{-8, 8, 60}, {8, 8, 60}, {8, -8, 60}, {-8, -8, 60}
};
static const int kCBaseSurf[4][8] = {
	{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
	{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0}
};
static const int kConsolePts[8][3] = {
	{-70, 110, 0}, {70, 110, 0}, {70, -110, 0}, {-70, -110, 0},
	{-70, 110, 180}, {70, 110, 180}, {70, -110, 180}, {-70, -110, 180}
};
static const int kConsoleSurf[5][8] = {
	{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
	{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0},
	{0, 4, 7, 6, 5, 4, 0, 0}
};
static const int kACouchPts[8][3] = {
	{-50, 150, 0}, {50, 150, 0}, {50, -150, 0}, {-50, -150, 0},
	{-50, 150, 50}, {50, 150, 50}, {50, -150, 50}, {-50, -150, 50}
};
static const int kBCouchPts[8][3] = {
	{-50, 150, 50}, {50, 150, 50}, {50, -150, 50}, {-50, -150, 50},
	{-50, 150, 100}, {50, 150, 100}, {50, -150, 100}, {-50, -150, 100}
};
static const int kCCouchPts[8][3] = {
	{-70, 150, 0}, {-50, 150, 0}, {-50, -150, 0}, {-70, -150, 0},
	{-70, 150, 100}, {-50, 150, 100}, {-50, -150, 100}, {-70, -150, 100}
};
static const int kDCouchPts[8][3] = {
	{50, 150, 0}, {70, 150, 0}, {70, -150, 0}, {50, -150, 0},
	{50, 150, 100}, {70, 150, 100}, {70, -150, 100}, {50, -150, 100}
};
static const int kCouchSurf[5][8] = {
	{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
	{0, 4, 5, 1, 0, 4, 0, 0}, {0, 4, 6, 2, 1, 5, 0, 0},
	{0, 4, 7, 6, 5, 4, 0, 0}
};
static const int kAChairPts[8][3] = {
	{-70, 70, 0}, {50, 70, 0}, {50, -70, 0}, {-70, -70, 0},
	{-70, 70, 40}, {50, 70, 40}, {50, -70, 40}, {-70, -70, 40}
};
static const int kBChairPts[8][3] = {
	{-70, 70, 40}, {50, 70, 40}, {50, -70, 40}, {-70, -70, 40},
	{-70, 70, 80}, {50, 70, 80}, {50, -70, 80}, {-70, -70, 80}
};
static const int kCChairPts2[8][3] = {
	{-70, 70, 0}, {50, 70, 0}, {50, 50, 0}, {-70, 50, 0},
	{-70, 70, 80}, {50, 70, 80}, {50, 50, 80}, {-70, 50, 80}
};
static const int kDChairPts[8][3] = {
	{-70, -50, 0}, {50, -50, 0}, {50, -70, 0}, {-70, -70, 0},
	{-70, -50, 80}, {50, -50, 80}, {50, -70, 80}, {-70, -70, 80}
};
static const int kTVBodyPts[8][3] = {
	{-30, 60, 0}, {30, 60, 0}, {30, -60, 0}, {-30, -60, 0},
	{-30, 60, 120}, {30, 60, 120}, {30, -60, 120}, {-30, -60, 120}
};
static const int kTVBodySurf[5][8] = {
	{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
	{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0},
	{0, 4, 7, 6, 5, 4, 0, 0}
};
static const int kTVScreenPts[4][3] = {
	{30, 50, 10}, {30, -50, 10}, {30, 50, 110}, {30, -50, 110}
};
static const int kTVScreenSurf[1][8] = {{0, 4, 1, 0, 2, 3, 0, 0}};
static const int kDrawerPts[8][3] = {
	{-80, 70, 0}, {0, 70, 0}, {0, -70, 0}, {-80, -70, 0},
	{-80, 70, 100}, {0, 70, 100}, {0, -70, 100}, {-80, -70, 100}
};
static const int kDrawerSurf[5][8] = {
	{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
	{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0},
	{0, 4, 7, 6, 5, 4, 0, 0}
};
static const int kMirrorPts[4][3] = {
	{-80, 65, 100}, {-80, -65, 100}, {-80, 65, 210}, {-80, -65, 210}
};
static const int kMirrorSurf[1][8] = {{0, 4, 1, 0, 2, 3, 0, 0}};
static const Colony::ColonyEngine::PrismPartDef kScreenPart = {8, kScreenPts, 4, kScreenSurf};
static const Colony::ColonyEngine::PrismPartDef kTableParts[2] = {
	{4, kTableTopPts, 1, kTableTopSurf},
	{8, kTableBasePts, 4, kTableBaseSurf}
};
static const Colony::ColonyEngine::PrismPartDef kBedParts[3] = {
	{4, kBedPostPts, 1, kBedPostSurf},
	{8, kBedBlanketPts, 4, kBlanketSurf},
	{8, kBedSheetPts, 3, kSheetSurf}
};
static const Colony::ColonyEngine::PrismPartDef kBBedParts[3] = {
	{4, kBBedPostPts, 1, kBedPostSurf},
	{8, kBBedBlanketPts, 4, kBlanketSurf},
	{8, kBBedSheetPts, 3, kSheetSurf}
};
static const Colony::ColonyEngine::PrismPartDef kDeskParts[10] = {
	{4, kDeskTopPts, 1, kDeskTopSurf},
	{8, kDeskLeftPts, 4, kDeskCabSurf},
	{8, kDeskRightPts, 4, kDeskCabSurf},
	{4, kSeatPts, 1, kSeatSurf},
	{4, kArmLeftPts, 2, kArmSurf},
	{4, kArmRightPts, 2, kArmSurf},
	{4, kBackPts, 2, kBackSurf},
	{8, kComputerPts, 5, kComputerSurf},
	{8, kMonitorPts, 5, kComputerSurf},
	{4, kDeskScreenPts, 1, kDeskScreenSurf}
};
static const Colony::ColonyEngine::PrismPartDef kCChairParts[5] = {
	{4, kCSeatPts, 1, kCSeatSurf},
	{4, kCArmLeftPts, 1, kCArmLeftSurf},
	{4, kCArmRightPts, 1, kCArmRightSurf},
	{4, kCBackPts, 2, kCBackSurf},
	{8, kCBasePts, 4, kCBaseSurf}
};
static const Colony::ColonyEngine::PrismPartDef kConsolePart = {8, kConsolePts, 5, kConsoleSurf};
static const Colony::ColonyEngine::PrismPartDef kCouchParts[4] = {
	{8, kACouchPts, 5, kCouchSurf},
	{8, kBCouchPts, 5, kCouchSurf},
	{8, kCCouchPts, 5, kCouchSurf},
	{8, kDCouchPts, 5, kCouchSurf}
};
static const Colony::ColonyEngine::PrismPartDef kChairParts[4] = {
	{8, kAChairPts, 5, kCouchSurf},
	{8, kBChairPts, 5, kCouchSurf},
	{8, kCChairPts2, 5, kCouchSurf},
	{8, kDChairPts, 5, kCouchSurf}
};
static const Colony::ColonyEngine::PrismPartDef kTVParts[2] = {
	{8, kTVBodyPts, 5, kTVBodySurf},
	{4, kTVScreenPts, 1, kTVScreenSurf}
};
static const Colony::ColonyEngine::PrismPartDef kDrawerParts[2] = {
	{8, kDrawerPts, 5, kDrawerSurf},
	{4, kMirrorPts, 1, kMirrorSurf}
};
static const int kCWallPts[8][3] = {
	{-128, 128, 0}, {0, 112, 0}, {112, 0, 0}, {128, -128, 0},
	{-128, 128, 320},  {0, 112, 320},  {112, 0, 320},  {128, -128, 320}
};
static const int kCWallSurf[3][8] = {
	{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0}
};
static const Colony::ColonyEngine::PrismPartDef kCWallParts[1] = {{8, kCWallPts, 3, kCWallSurf}};
static const int kPlantPotPts[8][3] = {
	{-20, 20, 0}, {20, 20, 0}, {20, -20, 0}, {-20, -20, 0},
	{-20, 20, 40}, {20, 20, 40}, {20, -20, 40}, {-20, -20, 40}
};
static const int kPlantPotSurf[5][8] = {
	{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
	{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0}, {0, 4, 7, 6, 5, 4, 0, 0}
};
static const int kPlantStemPts[8][3] = {
	{-5, 5, 40}, {5, 5, 40}, {5, -5, 40}, {-5, -5, 40},
	{-5, 5, 120}, {5, 5, 120}, {5, -5, 120}, {-5, -5, 120}
};
static const int kPlantStemSurf[4][8] = {
	{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
	{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0}
};
static const int kPlantLeaf1Pts[4][3] = {
	{-40, 0, 120}, {40, 0, 120}, {40, 0, 140}, {-40, 0, 140}
};
static const int kPlantLeaf2Pts[4][3] = {
	{0, 40, 120}, {0, -40, 120}, {0, 40, 140}, {0, -40, 140}
};
static const int kPlantLeafSurf[1][8] = {{0, 4, 0, 1, 2, 3, 0, 0}};
static const Colony::ColonyEngine::PrismPartDef kPlantParts[4] = {
	{8, kPlantPotPts, 5, kPlantPotSurf},
	{8, kPlantStemPts, 4, kPlantStemSurf},
	{4, kPlantLeaf1Pts, 1, kPlantLeafSurf},
	{4, kPlantLeaf2Pts, 1, kPlantLeafSurf}
};


void ColonyEngine::quadrant() {
	int remain;
	int quad;

	quad = _me.look >> 6;				/*divide by 64		*/
	remain = _me.look - (quad << 6);		/*multiply by 64	*/
	_tsin = _sint[remain];
	_tcos = _cost[remain];

	switch (quad) {
	case 0:
		_direction = 0; // NORTH
		break;
	case 1:
		_direction = 2; // WEST
		break;
	case 2:
		_direction = 3; // SOUTH
		break;
	case 3:
		_direction = 1; // EAST
		break;
	}

	_frntxWall = g_indexTable[quad][0];
	_frntyWall = g_indexTable[quad][1];
	_sidexWall = g_indexTable[quad][2];
	_sideyWall = g_indexTable[quad][3];
	_frntx = g_indexTable[quad][4];
	_frnty = g_indexTable[quad][5];
	_sidex = g_indexTable[quad][6];
	_sidey = g_indexTable[quad][7];
	_front = g_indexTable[quad][8];
	_side = g_indexTable[quad][9];
}

void ColonyEngine::corridor() {
	quadrant();
	renderCorridor3D();
}

uint8 ColonyEngine::wallAt(int x, int y) const {
	if (x < 0 || x >= 32 || y < 0 || y >= 32)
		return 3;
	return _wall[x][y];
}

const uint8 *ColonyEngine::mapFeatureAt(int x, int y, int direction) const {
	if (x < 0 || x >= 31 || y < 0 || y >= 31 || direction < 0 || direction >= 5)
		return nullptr;
	return _mapData[x][y][direction];
}

uint32 ColonyEngine::objectColor(int type) const {
	switch (type) {
	case 21: // DESK
		return 220;
	case 22: // PLANT
		return 100;
	case 24: // BED
	case 42: // BBED
		return 180;
	case 29: // SCREEN
	case 30: // CONSOLE
		return 240;
	case 31: // POWERSUIT
	case 46: // REACTOR
		return 255;
	case 36: // TELEPORT
		return 140;
	default:
		return 160 + ((uint32)(type * 7) & 0x3F);
	}
}

void ColonyEngine::draw3DPrism(const Thing &obj, const PrismPartDef &def, bool useLook, uint32 color) {
	// +32 compensates for the original sine table's 45° phase offset.
	// Object angles from game data were stored assuming that offset.
	const uint8 ang = (useLook ? obj.where.look : obj.where.ang) + 32;
	const long rotCos = _cost[ang];
	const long rotSin = _sint[ang];
 
	for (int i = 0; i < def.surfaceCount; i++) {
		const int n = def.surfaces[i][1];
		if (n < 2) continue;

		float px[8];
		float py[8];
		float pz[8];
		int count = 0;

		for (int j = 0; j < n; j++) {
			const int cur = def.surfaces[i][j + 2];
			if (cur < 0 || cur >= def.pointCount) continue;

			int ox = def.points[cur][0];
			int oy = def.points[cur][1];
			int oz = def.points[cur][2];

			// World relative rotation
			long rx = ((long)ox * rotCos - (long)oy * rotSin) >> 7;
			long ry = ((long)ox * rotSin + (long)oy * rotCos) >> 7;
			
			px[count] = (float)(rx + obj.where.xloc);
			py[count] = (float)(ry + obj.where.yloc);
			pz[count] = (float)(oz - 160); // Shift from floor-relative (z=0) to world (z=-160)
			count++;
		}

		if (count >= 3) {
			_gfx->draw3DPolygon(px, py, pz, count, color);
		}
	}
}

void ColonyEngine::drawStaticObjects() {
	for (uint i = 0; i < _objects.size(); i++) {
		const Thing &obj = _objects[i];
		if (!obj.alive)
			continue;
		uint32 color = objectColor(obj.type);
		drawStaticObjectPrisms3D(obj, color);
	}
}

void ColonyEngine::renderCorridor3D() {
	_gfx->begin3D(_me.xloc, _me.yloc, 0, _me.look, _me.lookY, _screenR);
 
	// Draw large black floor and ceiling quads
	_gfx->draw3DQuad(-100000.0f, -100000.0f, -160.1f, 
	                100000.0f, -100000.0f, -160.1f, 
	                100000.0f, 100000.0f, -160.1f, 
	                -100000.0f, 100000.0f, -160.1f, 0); // Black floor
 
	_gfx->draw3DQuad(-100000.0f, -100000.0f, 160.1f, 
	                100000.0f, -100000.0f, 160.1f, 
	                100000.0f, 100000.0f, 160.1f, 
	                -100000.0f, 100000.0f, 160.1f, 0); // Black ceiling
 
	uint32 wallColor = 15; // White
 
	for (int y = 0; y < 32; y++) {
		for (int x = 0; x < 32; x++) {
			uint8 w = _wall[x][y];
			if (w & 0x01) {
				_gfx->draw3DWall(x, y, x + 1, y, wallColor);
			}
			if (w & 0x02) {
				_gfx->draw3DWall(x, y, x, y + 1, wallColor);
			}
		}
	}
	
	drawStaticObjects();
		
	_gfx->end3D();
}

bool ColonyEngine::drawStaticObjectPrisms3D(const Thing &obj, uint32 baseColor) {
	auto tint = [](uint32 base, int delta) -> uint32 {
		return (uint32)CLIP<int>((int)base + delta, 0, 255);
	};

	switch (obj.type) {
	case kObjConsole:
		draw3DPrism(obj, kConsolePart, false, tint(baseColor, 0));
		break;
	case kObjCChair:
		for (int i = 0; i < 5; i++)
			draw3DPrism(obj, kCChairParts[i], false, tint(baseColor, 0));
		break;
	case kObjPlant:
		for (int i = 0; i < 4; i++)
			draw3DPrism(obj, kPlantParts[i], false, tint(baseColor, 0));
		break;
	case kObjCouch:
	case kObjChair: {
		const PrismPartDef *parts = (obj.type == kObjCouch) ? kCouchParts : kChairParts;
		for (int i = 0; i < 4; i++)
			draw3DPrism(obj, parts[i], false, tint(baseColor, 0));
		break;
	}
	case kObjTV:
		draw3DPrism(obj, kTVParts[0], false, tint(baseColor, 0));
		draw3DPrism(obj, kTVParts[1], false, tint(baseColor, 35));
		break;
	case kObjDrawer:
		draw3DPrism(obj, kDrawerParts[0], false, tint(baseColor, 0));
		draw3DPrism(obj, kDrawerParts[1], false, tint(baseColor, 30));
		break;
	case kObjFWall:
	case kObjCWall:
		draw3DPrism(obj, kCWallParts[0], false, tint(baseColor, -5));
		break;
	case kObjScreen:
		draw3DPrism(obj, kScreenPart, false, tint(baseColor, 0));
		break;
	case kObjTable:
		draw3DPrism(obj, kTableParts[0], false, tint(baseColor, 20));
		draw3DPrism(obj, kTableParts[1], false, tint(baseColor, -10));
		break;
	case kObjBed:
	case kObjBBed: {
		const PrismPartDef *parts = (obj.type == kObjBBed) ? kBBedParts : kBedParts;
		draw3DPrism(obj, parts[0], false, tint(baseColor, 15));
		draw3DPrism(obj, parts[1], false, tint(baseColor, -10));
		draw3DPrism(obj, parts[2], false, tint(baseColor, 5));
		break;
	}
	case kObjDesk:
		for (int i = 0; i < 10; i++)
			draw3DPrism(obj, kDeskParts[i], false, tint(baseColor, 0));
		break;
	default:
		return false;
	}
	return true;
}

} // End of namespace Colony
