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
	{1, 4, 0, 3, 7, 4, 0, 0}, {1, 4, 3, 2, 6, 7, 0, 0},
	{1, 4, 2, 1, 5, 6, 0, 0}, {1, 4, 7, 6, 5, 4, 0, 0}
};
static const int kTableTopPts[4][3] = {
	{-64, 64, 100}, {64, 64, 100}, {64, -64, 100}, {-64, -64, 100}
};
static const int kTableTopSurf[1][8] = {{6, 4, 3, 2, 1, 0, 0, 0}};
static const int kTableBasePts[8][3] = {
	{-20, 20, 0}, {20, 20, 0}, {20, -20, 0}, {-20, -20, 0},
	{-20, 20, 100}, {20, 20, 100}, {20, -20, 100}, {-20, -20, 100}
};
static const int kTableBaseSurf[4][8] = {
	{6, 4, 0, 3, 7, 4, 0, 0}, {6, 4, 3, 2, 6, 7, 0, 0},
	{6, 4, 1, 0, 4, 5, 0, 0}, {6, 4, 2, 1, 5, 6, 0, 0}
};
static const int kBedPostPts[4][3] = {
	{-80, 180, 0}, {80, 180, 0}, {80, 180, 100}, {-80, 180, 100}
};
static const int kBedPostSurf[1][8] = {{6, 4, 3, 2, 1, 0, 0, 0}};
static const int kBedBlanketPts[8][3] = {
	{-80, 70, 0}, {80, 70, 0}, {80, -175, 0}, {-80, -175, 0},
	{-80, 70, 60}, {80, 70, 60}, {80, -175, 60}, {-80, -175, 60}
};
static const int kBlanketSurf[4][8] = {
	{2, 4, 0, 3, 7, 4, 0, 0}, {2, 4, 3, 2, 6, 7, 0, 0},
	{2, 4, 2, 1, 5, 6, 0, 0}, {2, 4, 7, 6, 5, 4, 0, 0}
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
	{-150, 110, 100}, {150, 110, 100}, {150, -110, 100}, {-150, -110, 100}
};
static const int kDeskTopSurf[1][8] = {{6, 4, 3, 2, 1, 0, 0, 0}};
static const int kDeskLeftPts[8][3] = {
	{-135, 95, 0}, {-55, 95, 0}, {-55, -95, 0}, {-135, -95, 0},
	{-135, 95, 100}, {-55, 95, 100}, {-55, -95, 100}, {-135, -95, 100}
};
static const int kDeskRightPts[8][3] = {
	{55, 95, 0}, {135, 95, 0}, {135, -95, 0}, {55, -95, 0},
	{55, 95, 100}, {135, 95, 100}, {135, -95, 100}, {55, -95, 100}
};
static const int kDeskCabSurf[4][8] = {
	{6, 4, 0, 3, 7, 4, 0, 0}, {6, 4, 3, 2, 6, 7, 0, 0},
	{6, 4, 1, 0, 4, 5, 0, 0}, {6, 4, 2, 1, 5, 6, 0, 0}
};
static const int kSeatPts[4][3] = {
	{-40, 210, 60}, {40, 210, 60}, {40, 115, 60}, {-40, 115, 60}
};
static const int kSeatSurf[1][8] = {{1, 4, 3, 2, 1, 0, 0, 0}};
static const int kArmLeftPts[4][3] = {
	{-40, 210, 90}, {-40, 210, 0}, {-40, 115, 0}, {-40, 115, 90}
};
static const int kArmRightPts[4][3] = {
	{40, 210, 90}, {40, 210, 0}, {40, 115, 0}, {40, 115, 90}
};
static const int kArmSurf[2][8] = {
	{1, 4, 3, 2, 1, 0, 0, 0}, {1, 4, 0, 1, 2, 3, 0, 0}
};
static const int kBackPts[4][3] = {
	{-40, 210, 130}, {40, 210, 130}, {40, 210, 70}, {-40, 210, 70}
};
static const int kBackSurf[2][8] = {
	{1, 4, 3, 2, 1, 0, 0, 0}, {1, 4, 0, 1, 2, 3, 0, 0}
};
static const int kComputerPts[8][3] = {
	{70, 25, 100}, {120, 25, 100}, {120, -25, 100}, {70, -25, 100},
	{70, 25, 120}, {120, 25, 120}, {120, -25, 120}, {70, -25, 120}
};
static const int kMonitorPts[8][3] = {
	{75, 20, 120}, {115, 20, 120}, {115, -20, 120}, {75, -20, 120},
	{75, 20, 155}, {115, 20, 155}, {115, -20, 145}, {75, -20, 145}
};
static const int kComputerSurf[5][8] = {
	{6, 4, 7, 6, 5, 4, 0, 0}, {6, 4, 0, 3, 7, 4, 0, 0},
	{6, 4, 3, 2, 6, 7, 0, 0}, {6, 4, 1, 0, 4, 5, 0, 0},
	{6, 4, 2, 1, 5, 6, 0, 0}
};
static const int kDeskScreenPts[4][3] = {
	{80, 20, 125}, {110, 20, 125}, {110, 20, 150}, {80, 20, 150}
};
static const int kDeskScreenSurf[1][8] = {{1, 4, 3, 2, 1, 0, 0, 0}};
static const int kCSeatPts[4][3] = {
	{-40, 40, 60}, {40, 40, 60}, {40, -40, 60}, {-40, -40, 60}
};
static const int kCSeatSurf[1][8] = {{1, 4, 3, 2, 1, 0, 0, 0}};
static const int kCArmLeftPts[4][3] = {
	{-50, 40, 90}, {-40, 40, 60}, {-40, -40, 60}, {-50, -40, 90}
};
static const int kCArmLeftSurf[1][8] = {{1, 4, 3, 2, 1, 0, 0, 0}};
static const int kCArmRightPts[4][3] = {
	{50, 40, 90}, {40, 40, 60}, {40, -40, 60}, {50, -40, 90}
};
static const int kCArmRightSurf[1][8] = {{1, 4, 3, 2, 1, 0, 0, 0}};
static const int kCBackPts[4][3] = {
	{-20, 60, 150}, {20, 60, 150}, {40, 40, 60}, {-40, 40, 60}
};
static const int kCBackSurf[2][8] = {
	{1, 4, 3, 2, 1, 0, 0, 0}, {1, 4, 0, 1, 2, 3, 0, 0}
};
static const int kCBasePts[8][3] = {
	{-5, 5, 0}, {5, 5, 0}, {5, -5, 0}, {-5, -5, 0},
	{-5, 5, 60}, {5, 5, 60}, {5, -5, 60}, {-5, -5, 60}
};
static const int kCBaseSurf[4][8] = {
	{1, 4, 0, 3, 7, 4, 0, 0}, {1, 4, 3, 2, 6, 7, 0, 0},
	{1, 4, 1, 0, 4, 5, 0, 0}, {1, 4, 2, 1, 5, 6, 0, 0}
};
static const int kConsolePts[8][3] = {
	{-5, 5, 0}, {5, 5, 0}, {5, -5, 0}, {-5, -5, 0},
	{-100, 70, 100}, {-35, 70, 140}, {-35, -70, 140}, {-100, -70, 100}
};
static const int kConsoleSurf[5][8] = {
	{4, 4, 4, 0, 3, 7, 0, 0}, {4, 4, 7, 3, 2, 6, 0, 0},
	{4, 4, 5, 1, 0, 4, 0, 0}, {4, 4, 6, 2, 1, 5, 0, 0},
	{0, 4, 7, 6, 5, 4, 0, 0}
};
static const int kCouchSurf[5][8] = {
	{4, 4, 0, 3, 7, 4, 0, 0}, {4, 4, 3, 2, 6, 7, 0, 0},
	{4, 4, 1, 0, 4, 5, 0, 0}, {4, 4, 2, 1, 5, 6, 0, 0},
	{4, 4, 7, 6, 5, 4, 0, 0}
};
static const int kACouchPts[8][3] = {
	{-50, 150, 0}, {50, 150, 0}, {50, -150, 0}, {-50, -150, 0},
	{-50, 150, 50}, {50, 150, 50}, {50, -150, 50}, {-50, -150, 50}
};
static const int kBCouchPts[8][3] = {
	{-80, 150, 0}, {-45, 150, 0}, {-45, -150, 0}, {-80, -150, 0},
	{-80, 150, 120}, {-55, 150, 120}, {-55, -150, 120}, {-80, -150, 120}
};
static const int kCCouchPts[8][3] = {
	{-70, 170, 0}, {50, 170, 0}, {50, 150, 0}, {-70, 150, 0},
	{-70, 170, 80}, {50, 170, 80}, {50, 150, 80}, {-70, 150, 80}
};
static const int kDCouchPts[8][3] = {
	{-70, -150, 0}, {50, -150, 0}, {50, -170, 0}, {-70, -170, 0},
	{-70, -150, 80}, {50, -150, 80}, {50, -170, 80}, {-70, -170, 80}
};
static const int kAChairPts[8][3] = {
	{-50, 50, 0}, {50, 50, 0}, {50, -50, 0}, {-50, -50, 0},
	{-50, 50, 50}, {50, 50, 50}, {50, -50, 50}, {-50, -50, 50}
};
static const int kBChairPts[8][3] = {
	{-80, 50, 0}, {-45, 50, 0}, {-45, -50, 0}, {-80, -50, 0},
	{-80, 50, 120}, {-55, 50, 120}, {-55, -50, 120}, {-80, -50, 120}
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
	{6, 4, 0, 3, 7, 4, 0, 0}, {6, 4, 3, 2, 6, 7, 0, 0},
	{6, 4, 1, 0, 4, 5, 0, 0}, {6, 4, 2, 1, 5, 6, 0, 0},
	{6, 4, 7, 6, 5, 4, 0, 0}
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
	{6, 4, 0, 3, 7, 4, 0, 0}, {6, 4, 3, 2, 6, 7, 0, 0},
	{6, 4, 1, 0, 4, 5, 0, 0}, {6, 4, 2, 1, 5, 6, 0, 0},
	{6, 4, 7, 6, 5, 4, 0, 0}
};
static const int kMirrorPts[4][3] = {
	{-80, 65, 100}, {-80, -65, 100}, {-80, 65, 210}, {-80, -65, 210}
};
static const int kMirrorSurf[1][8] = {{0, 4, 1, 0, 2, 3, 0, 0}};

// Bathtub geometry
static const int kTubPts[8][3] = {
	{-128, 128,  0}, {   0, 128,  0}, {   0,-128,  0}, {-128,-128,  0},
	{-128, 128, 70}, {   0, 128, 70}, {   0,-128, 70}, {-128,-128, 70}
};
static const int kTubSurf[5][8] = {
	{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
	{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0},
	{0, 4, 7, 6, 5, 4, 0, 0}
};
static const int kDTubPts[6][3] = {
	{-16, 112, 70}, {-8, 0, 70}, {-16, -112, 70}, {-112, -112, 70}, {-120, 0, 70}, {-112, 112, 70}
};
static const int kDTubSurf[1][8] = {{1, 6, 5, 4, 3, 2, 1, 0}};

// Toilet geometry
static const int kAToiletPts[8][3] = {
	{-128, 45, 30}, {-100, 45, 30}, {-100, -45, 30}, {-128, -45, 30},
	{-128, 45, 100}, {-100, 45, 100}, {-100, -45, 100}, {-128, -45, 100}
};
static const int kAToiletSurf[5][8] = {
	{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
	{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0},
	{0, 4, 7, 6, 5, 4, 0, 0}
};
static const int kBToiletPts[12][3] = {
	{-100, 20, 50}, {-60, 40, 50}, {-20, 20, 50}, {-20, -20, 50}, {-60, -40, 50}, {-100, -20, 50},
	{-80, 10,  0}, {-60, 20,  0}, {-40, 10,  0}, {-40, -10,  0}, {-60, -20,  0}, {-80, -10,  0}
};
static const int kBToiletSurf[7][8] = {
	{0, 4, 0, 1, 7, 6, 0, 0}, {0, 4, 1, 2, 8, 7, 0, 0}, {0, 4, 2, 3, 9, 8, 0, 0},
	{0, 4, 3, 4, 10, 9, 0, 0}, {0, 4, 4, 5, 11, 10, 0, 0}, {0, 4, 5, 0, 6, 11, 0, 0},
	{0, 6, 5, 4, 3, 2, 1, 0}
};
static const int kCToiletPts[6][3] = {
	{-95, 15, 50}, {-60, 35, 50}, {-25, 15, 50}, {-25, -15, 50}, {-60, -35, 50}, {-95, -15, 50}
};
static const int kCToiletSurf[1][8] = {{1, 6, 5, 4, 3, 2, 1, 0}};
static const int kDToiletPts[6][3] = {
	{-100, 20, 50}, {-100, 40, 90}, {-100, 20, 130}, {-100, -20, 130}, {-100, -40, 90}, {-100, -20, 50}
};
static const int kDToiletSurf[1][8] = {{10, 6, 5, 4, 3, 2, 1, 0}};
static const int kEToiletPts[4][3] = {
	{-128,-128, 20}, {-128,-128, 200}, { 128,-128, 200}, { 128,-128, 20}
};
static const int kEToiletSurf[1][8] = {{0, 4, 0, 1, 2, 3, 0, 0}};

// Sink geometry
static const int kSinkPts[8][3] = {
	{-128, 50, 70}, {-50, 50, 90}, {-50,-50, 90}, {-128,-50, 70},
	{-128, 50, 110}, {-50, 50, 110}, {-50,-50, 110}, {-128,-50, 110}
};
static const int kSinkSurf[5][8] = {
	{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
	{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0},
	{0, 4, 7, 6, 5, 4, 0, 0}
};
static const int kDSinkPts[6][3] = {
	{-55, 0, 110}, {-60, -45, 110}, {-118, -45, 110}, {-123, 0, 110}, {-118, 45, 110}, {-60, 45, 110}
};
static const int kDSinkSurf[1][8] = {{1, 6, 5, 4, 3, 2, 1, 0}};
static const int kSinkMirrorPts[4][3] = {
	{-128, 65, 130}, {-128, -65, 130}, {-128, 65, 250}, {-128, -65, 250}
};
static const int kSinkMirrorSurf[1][8] = {{0, 4, 1, 0, 2, 3, 0, 0}};

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
static const Colony::ColonyEngine::PrismPartDef kTubParts[2] = {
	{8, kTubPts, 5, kTubSurf},
	{6, kDTubPts, 1, kDTubSurf}
};
static const Colony::ColonyEngine::PrismPartDef kSinkParts[3] = {
	{8, kSinkPts, 5, kSinkSurf},
	{6, kDSinkPts, 1, kDSinkSurf},
	{4, kSinkMirrorPts, 1, kSinkMirrorSurf}
};
static const Colony::ColonyEngine::PrismPartDef kToiletParts[4] = {
	{8, kAToiletPts, 5, kAToiletSurf},
	{12, kBToiletPts, 7, kBToiletSurf},
	{6, kCToiletPts, 1, kCToiletSurf},
	{6, kDToiletPts, 1, kDToiletSurf}
};
static const Colony::ColonyEngine::PrismPartDef kPToiletParts[5] = {
	{8, kAToiletPts, 5, kAToiletSurf},
	{12, kBToiletPts, 7, kBToiletSurf},
	{6, kCToiletPts, 1, kCToiletSurf},
	{6, kDToiletPts, 1, kDToiletSurf},
	{4, kEToiletPts, 1, kEToiletSurf}
};

static const int kCWallPts[8][3] = {
	{-128, 128, 0}, {0, 112, 0}, {112, 0, 0}, {128, -128, 0},
	{-128, 128, 320},  {0, 112, 320},  {112, 0, 320},  {128, -128, 320}
};
static const int kCWallSurf[3][8] = {
	{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0}
};
static const Colony::ColonyEngine::PrismPartDef kCWallParts[1] = {{8, kCWallPts, 3, kCWallSurf}};

static const int kPlantPotPts[12][3] = {
	{10, 17, 40}, {20, 0, 40}, {10, -17, 40}, {-10, -17, 40}, {-20, 0, 40}, {-10, 17, 40},
	{8, 12, 0}, {15, 0, 0}, {8, -12, 0}, {-8, -12, 0}, {-15, 0, 0}, {-8, 12, 0}
};
static const int kPlantPotSurf[6][8] = {
	{6, 4, 0, 1, 7, 6, 0, 0}, {6, 4, 1, 2, 8, 7, 0, 0}, {6, 4, 2, 3, 9, 8, 0, 0},
	{6, 4, 3, 4, 10, 9, 0, 0}, {6, 4, 4, 5, 11, 10, 0, 0}, {6, 4, 5, 0, 6, 11, 0, 0}
};
static const int kPlantTopPotPts[6][3] = {
	{10, 17, 40}, {20, 0, 40}, {10, -17, 40}, {-10, -17, 40}, {-20, 0, 40}, {-10, 17, 40}
};
static const int kPlantTopPotSurf[1][8] = {{0, 6, 5, 4, 3, 2, 1, 0}};

static const int kPlantLeaf0Pts[3][3] = {{0, 0, 0}, {20, 20, 150}, {70, 70, 100}};
static const int kPlantLeaf1Pts[3][3] = {{0, 0, 0}, {-20, 30, 100}, {-60, 50, 50}};
static const int kPlantLeaf2Pts[3][3] = {{0, 0, 0}, {-20, -10, 70}, {-90, -70, 50}};
static const int kPlantLeaf3Pts[3][3] = {{0, 0, 0}, {20, -10, 50}, {90, -70, 70}};
static const int kPlantLeaf4Pts[3][3] = {{0, 0, 0}, {20, -30, 190}, {40, -60, 150}};
static const int kPlantLeaf5Pts[3][3] = {{0, 0, 0}, {20, -10, 130}, {50, -80, 200}};

static const int kPlantLeafSurf[2][8] = {{2, 3, 0, 1, 2, 0, 0, 0}, {2, 3, 2, 1, 0, 0, 0, 0}};

static const Colony::ColonyEngine::PrismPartDef kPlantParts[8] = {
	{12, kPlantPotPts, 6, kPlantPotSurf},
	{6, kPlantTopPotPts, 1, kPlantTopPotSurf},
	{3, kPlantLeaf0Pts, 2, kPlantLeafSurf},
	{3, kPlantLeaf1Pts, 2, kPlantLeafSurf},
	{3, kPlantLeaf2Pts, 2, kPlantLeafSurf},
	{3, kPlantLeaf3Pts, 2, kPlantLeafSurf},
	{3, kPlantLeaf4Pts, 2, kPlantLeafSurf},
	{3, kPlantLeaf5Pts, 2, kPlantLeafSurf}
};

static const int kBox1Pts[8][3] = {
	{-50, 50, 0}, {50, 50, 0}, {50, -50, 0}, {-50, -50, 0},
	{-50, 50, 100}, {50, 50, 100}, {50, -50, 100}, {-50, -50, 100}
};
static const int kBox1Surf[5][8] = {
	{6, 4, 0, 3, 7, 4, 0, 0}, {6, 4, 3, 2, 6, 7, 0, 0},
	{6, 4, 1, 0, 4, 5, 0, 0}, {6, 4, 2, 1, 5, 6, 0, 0}, {6, 4, 7, 6, 5, 4, 0, 0}
};
static const int kBox2Pts[8][3] = {
	{-50, 50, 100}, {50, 50, 100}, {50, -50, 100}, {-50, -50, 100},
	{-50, 50, 200}, {50, 50, 200}, {50, -50, 200}, {-50, -50, 200}
};

static const int kReactorCorePts[12][3] = {
	{32, 55, 120}, {64, 0, 120}, {32, -55, 120}, {-32, -55, 120}, {-64, 0, 120}, {-32, 55, 120},
	{32, 55, 168}, {64, 0, 168}, {32, -55, 168}, {-32, -55, 168}, {-64, 0, 168}, {-32, 55, 168}
};
static const int kReactorCoreSurf[7][8] = {
	{0, 4, 0, 1, 7, 6, 0, 0}, {0, 4, 1, 2, 8, 7, 0, 0}, {0, 4, 2, 3, 9, 8, 0, 0},
	{0, 4, 3, 4, 10, 9, 0, 0}, {0, 4, 4, 5, 11, 10, 0, 0}, {0, 4, 5, 0, 6, 11, 0, 0},
	{0, 6, 5, 4, 3, 2, 1, 0}
};
static const int kReactorBasePts[8][3] = {
	{-128, 128, 0}, {128, 128, 0}, {128, -128, 0}, {-128, -128, 0},
	{-128, 128, 120}, {128, 128, 120}, {128, -128, 120}, {-128, -128, 120}
};
static const int kReactorBaseSurf[6][8] = {
	{4, 4, 0, 3, 7, 4, 0, 0}, {4, 4, 3, 2, 6, 7, 0, 0}, {4, 4, 1, 0, 4, 5, 0, 0},
	{4, 4, 2, 1, 5, 6, 0, 0}, {4, 4, 7, 6, 5, 4, 0, 0}, {4, 4, 0, 1, 2, 3, 0, 0}
};
static const int kReactorTopPts[8][3] = {
	{-128, 128, 168}, {128, 128, 168}, {128, -128, 168}, {-128, -128, 168},
	{-128, 128, 288}, {128, 128, 288}, {128, -128, 288}, {-128, -128, 288}
};

static const Colony::ColonyEngine::PrismPartDef kBox1Part = {8, kBox1Pts, 5, kBox1Surf};
static const Colony::ColonyEngine::PrismPartDef kBox2Parts[2] = {
	{8, kBox2Pts, 4, kBox1Surf}, // Body (stacked on box1)
	{8, kBox1Pts, 5, kBox1Surf}  // Lid (same geometry as box1 base)
};
static const Colony::ColonyEngine::PrismPartDef kReactorParts[3] = {
	{12, kReactorCorePts, 7, kReactorCoreSurf},
	{8, kReactorBasePts, 6, kReactorBaseSurf},
	{8, kReactorTopPts, 6, kReactorBaseSurf}
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
			uint32 finalColor;
			if (_corePower[_coreIndex] > 0) {
				// Lit mode: use surface-specific colors defined in geometry
				finalColor = (uint32)def.surfaces[i][0];
			} else {
				// Dark mode: everything is white wireframe.
				finalColor = 15; // vINTWHITE
			}
			_gfx->draw3DPolygon(px, py, pz, count, finalColor);
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

// Get the 4 corners of a wall face in 3D world space.
// corners[0] = bottom-left, corners[1] = bottom-right, corners[2] = top-right, corners[3] = top-left
// "left" and "right" are as seen from the cell that owns the feature.
void ColonyEngine::getWallFace3D(int cellX, int cellY, int direction, float corners[4][3]) {
	float x0 = cellX * 256.0f;
	float y0 = cellY * 256.0f;
	float x1 = (cellX + 1) * 256.0f;
	float y1 = (cellY + 1) * 256.0f;
	const float zBot = -160.0f;
	const float zTop = 160.0f;
	// Pull slightly into the cell to prevent z-fighting
	const float eps = 1.0f;

	switch (direction) {
	case kDirNorth: // Wall at y1 (+Y); viewed from inside (looking North)
		corners[0][0] = x0;  corners[0][1] = y1 - eps;  corners[0][2] = zBot; // BL
		corners[1][0] = x1;  corners[1][1] = y1 - eps;  corners[1][2] = zBot; // BR
		corners[2][0] = x1;  corners[2][1] = y1 - eps;  corners[2][2] = zTop; // TR
		corners[3][0] = x0;  corners[3][1] = y1 - eps;  corners[3][2] = zTop; // TL
		break;
	case kDirSouth: // Wall at y0 (-Y); viewed from inside (looking South)
		corners[0][0] = x1;  corners[0][1] = y0 + eps;  corners[0][2] = zBot; // BL
		corners[1][0] = x0;  corners[1][1] = y0 + eps;  corners[1][2] = zBot; // BR
		corners[2][0] = x0;  corners[2][1] = y0 + eps;  corners[2][2] = zTop; // TR
		corners[3][0] = x1;  corners[3][1] = y0 + eps;  corners[3][2] = zTop; // TL
		break;
	case kDirEast: // Wall at x1 (+X); viewed from inside (looking East)
		corners[0][0] = x1 - eps;  corners[0][1] = y1;  corners[0][2] = zBot; // BL
		corners[1][0] = x1 - eps;  corners[1][1] = y0;  corners[1][2] = zBot; // BR
		corners[2][0] = x1 - eps;  corners[2][1] = y0;  corners[2][2] = zTop; // TR
		corners[3][0] = x1 - eps;  corners[3][1] = y1;  corners[3][2] = zTop; // TL
		break;
	case kDirWest: // Wall at x0 (-X); viewed from inside (looking West)
		corners[0][0] = x0 + eps;  corners[0][1] = y0;  corners[0][2] = zBot; // BL
		corners[1][0] = x0 + eps;  corners[1][1] = y1;  corners[1][2] = zBot; // BR
		corners[2][0] = x0 + eps;  corners[2][1] = y1;  corners[2][2] = zTop; // TR
		corners[3][0] = x0 + eps;  corners[3][1] = y0;  corners[3][2] = zTop; // TL
		break;
	default:
		return;
	}
}

// Interpolate a point on the wall face.
// u: 0=left, 1=right (horizontal fraction)
// v: 0=bottom, 1=top (vertical fraction)
static void wallPoint(const float corners[4][3], float u, float v, float out[3]) {
	float botX = corners[0][0] + (corners[1][0] - corners[0][0]) * u;
	float botY = corners[0][1] + (corners[1][1] - corners[0][1]) * u;
	float botZ = corners[0][2] + (corners[1][2] - corners[0][2]) * u;
	float topX = corners[3][0] + (corners[2][0] - corners[3][0]) * u;
	float topY = corners[3][1] + (corners[2][1] - corners[3][1]) * u;
	float topZ = corners[3][2] + (corners[2][2] - corners[3][2]) * u;
	out[0] = botX + (topX - botX) * v;
	out[1] = botY + (topY - botY) * v;
	out[2] = botZ + (topZ - botZ) * v;
}

// Draw a line on a wall face using normalized (u,v) coordinates
void ColonyEngine::wallLine(const float corners[4][3], float u1, float v1, float u2, float v2, uint32 color) {
	float p1[3], p2[3];
	wallPoint(corners, u1, v1, p1);
	wallPoint(corners, u2, v2, p2);
	// We assume this is only called when lit (handled in drawWallFeatures3D)
	_gfx->draw3DLine(p1[0], p1[1], p1[2], p2[0], p2[1], p2[2], color);
}

// Draw a filled polygon on a wall face using normalized (u,v) coordinates
void ColonyEngine::wallPolygon(const float corners[4][3], const float *u, const float *v, int count, uint32 color) {
	float px[8], py[8], pz[8];
	if (count > 8) count = 8;
	for (int i = 0; i < count; i++) {
		float p[3];
		wallPoint(corners, u[i], v[i], p);
		px[i] = p[0]; py[i] = p[1]; pz[i] = p[2];
	}
	_gfx->draw3DPolygon(px, py, pz, count, color);
}

void ColonyEngine::wallChar(const float corners[4][3], uint8 cnum) {
	// Character 'b' (right arrow) and 'c' (left arrow) coordinates on 0-6 grid
	static const uint8 wallchar_b[] = {7, 0,3, 3,0, 3,2, 6,2, 6,4, 3,4, 3,6};
	static const uint8 wallchar_c[] = {7, 0,2, 0,4, 3,4, 3,6, 6,3, 3,0, 3,2};
	
	const uint8 *data = nullptr;
	if (cnum == 'b') data = wallchar_b;
	else if (cnum == 'c') data = wallchar_c;
	
	if (data) {
		int count = data[0];
		float u[8], v[8];
		for (int i = 0; i < count; i++) {
			u[i] = 0.2f + (data[1 + i*2] / 6.0f) * 0.6f;
			v[i] = 0.2f + (data[2 + i*2] / 6.0f) * 0.6f;
		}
		// Characters are usually drawn in black or level-specific color. Color 0 is safe.
		wallPolygon(corners, u, v, count, 0); 
	}
}

void ColonyEngine::getCellFace3D(int cellX, int cellY, bool ceiling, float corners[4][3]) {
	float z = ceiling ? 160.0f : -160.0f;
	float x0 = cellX * 256.0f;
	float y0 = cellY * 256.0f;
	float x1 = x0 + 256.0f;
	float y1 = y0 + 256.0f;
	const float eps = 0.1f;
	if (ceiling) z -= eps; else z += eps;

	corners[0][0] = x0; corners[0][1] = y0; corners[0][2] = z;
	corners[1][0] = x1; corners[1][1] = y0; corners[1][2] = z;
	corners[2][0] = x1; corners[2][1] = y1; corners[2][2] = z;
	corners[3][0] = x0; corners[3][1] = y1; corners[3][2] = z;
}

void ColonyEngine::drawCellFeature3D(int cellX, int cellY) {
	const uint8 *map = mapFeatureAt(cellX, cellY, kDirCenter);
	if (!map || map[0] == 0)
		return;

	float corners[4][3];
	bool ceiling = (map[0] == 3 || map[0] == 4); // SMHOLECEIL, LGHOLECEIL
	getCellFace3D(cellX, cellY, ceiling, corners);

	switch (map[0]) {
	case 1: // SMHOLEFLR
	case 3: // SMHOLECEIL
	{
		float u[4] = {0.375f, 0.625f, 0.625f, 0.375f};
		float v[4] = {0.375f, 0.375f, 0.625f, 0.625f};
		wallPolygon(corners, u, v, 4, 7); // LTGRAY
		break;
	}
	case 2: // LGHOLEFLR
	case 4: // LGHOLECEIL
	{
		float u[4] = {0.0f, 1.0f, 1.0f, 0.0f};
		float v[4] = {0.0f, 0.0f, 1.0f, 1.0f};
		wallPolygon(corners, u, v, 4, 7); // LTGRAY
		break;
	}
	case 5: // HOTFOOT
	{
		float u[4] = {0.0f, 1.0f, 1.0f, 0.0f};
		float v[4] = {0.0f, 0.0f, 1.0f, 1.0f};
		wallPolygon(corners, u, v, 4, 4); // RED
		break;
	}
	default:
		break;
	}
}

void ColonyEngine::drawWallFeature3D(int cellX, int cellY, int direction) {
	const uint8 *map = mapFeatureAt(cellX, cellY, direction);
	if (!map || map[0] == kWallFeatureNone)
		return;

	float corners[4][3];
	getWallFace3D(cellX, cellY, direction, corners);

	switch (map[0]) {
	case kWallFeatureDoor: {
		bool shipLevel = (_level == 1 || _level == 5 || _level == 6);
		
		if (shipLevel) {
			// Octagonal ship door (Weighted 7-point grid from source)
			// Horizontal indices: 2, 1, 1, 2, 4, 5, 5, 4 (0.25..0.75 width)
			// Vertical indices:   0, 1, 5, 6, 6, 5, 1, 0 (0.125..0.875 height)
			static const float u_ss[8] = { 0.375f, 0.250f, 0.250f, 0.375f, 0.625f, 0.750f, 0.750f, 0.625f };
			static const float v_ss[8] = { 0.125f, 0.250f, 0.750f, 0.875f, 0.875f, 0.750f, 0.250f, 0.125f };
			
			if (map[1] == 0) {
				wallPolygon(corners, u_ss, v_ss, 8, 0);
			} else {
				wallPolygon(corners, u_ss, v_ss, 8, 7);
				for (int i = 0; i < 8; i++)
					wallLine(corners, u_ss[i], v_ss[i], u_ss[(i + 1) % 8], v_ss[(i + 1) % 8], 0);
				// Ship door inner panel
				wallLine(corners, 0.375f, 0.25f, 0.375f, 0.75f, 0);
				wallLine(corners, 0.625f, 0.25f, 0.625f, 0.75f, 0);
				wallLine(corners, 0.375f, 0.25f, 0.625f, 0.25f, 0);
				wallLine(corners, 0.375f, 0.75f, 0.625f, 0.75f, 0);
			}
		} else {
			// Standard rectangular door (Lab levels) - Smaller look
			float xl = 0.25f, xr = 0.75f;
			float yb = 0.125f, yt = 0.875f; // Suspended 1/8th from both ends
			float f_u[4] = {xl, xr, xr, xl};
			float f_v[4] = {yb, yb, yt, yt};
			
			if (map[1] == 0) {
				wallPolygon(corners, f_u, f_v, 4, 0);
				float leaf_u[4] = {xl, xl + 0.1f, xl + 0.1f, xl};
				float leaf_v[4] = {yb, yb + 0.05f, yt - 0.05f, yt};
				wallPolygon(corners, leaf_u, leaf_v, 4, 15);
			} else {
				uint32 fill = (_level >= 1 && _level <= 3) ? 12 : 7;
				wallPolygon(corners, f_u, f_v, 4, fill);
				wallLine(corners, xl, yb, xl, yt, 0);
				wallLine(corners, xl, yt, xr, yt, 0);
				wallLine(corners, xr, yt, xr, yb, 0);
				wallLine(corners, xr, yb, xl, yb, 0);
				// Diagonal Handle (Centered in the door)
				wallLine(corners, xr - 0.15f, 0.45f, xr - 0.05f, 0.55f, 0);
			}
		}
		break;
	}
	case kWallFeatureWindow: {
		// Window: Uses average midpoint shifts. 1/8th suspended.
		float xl = 0.3125f, xr = 0.6875f;
		float yb = 0.3125f, yt = 0.6875f; 
		float u[4] = {xl, xr, xr, xl};
		float v[4] = {yb, yb, yt, yt};
		wallPolygon(corners, u, v, 4, 7); 
		wallLine(corners, xl, yb, xr, yb, 0);
		wallLine(corners, xr, yb, xr, yt, 0);
		wallLine(corners, xr, yt, xl, yt, 0);
		wallLine(corners, xl, yt, xl, yb, 0);
		wallLine(corners, 0.5f, yb, 0.5f, yt, 0);
		wallLine(corners, xl, 0.5f, xr, 0.5f, 0);
		break;
	}
	case kWallFeatureShelves: {
		float xl = 0.15f, xr = 0.85f;
		float yb = 0.1f, yt = 0.9f;
		wallLine(corners, xl, yb, xr, yb, 0);
		wallLine(corners, xr, yb, xr, yt, 0);
		wallLine(corners, xr, yt, xl, yt, 0);
		wallLine(corners, xl, yt, xl, yb, 0);
		for (int i = 1; i <= 6; i++) {
			float t = yb + (yt - yb) * (float)i / 7.0f;
			wallLine(corners, xl, t, xr, t, 0);
		}
		break;
	}
	case kWallFeatureUpStairs: {
		float xl = 0.15f, xr = 0.85f;
		for (int i = 0; i < 6; i++) {
			float u = xl + (xr - xl) * (float)i / 6.0f;
			float u2 = xl + (xr - xl) * (float)(i + 1) / 6.0f;
			float v = 0.1f + 0.8f * (float)i / 6.0f;
			float v2 = 0.1f + 0.8f * (float)(i + 1) / 6.0f;
			wallLine(corners, u, v, u2, v2, 0);
		}
		wallLine(corners, xl, 0.1f, xr, 0.9f, 0);
		break;
	}
	case kWallFeatureDnStairs: {
		float xl = 0.15f, xr = 0.85f;
		for (int i = 0; i < 6; i++) {
			float u = xl + (xr - xl) * (float)i / 6.0f;
			float u2 = xl + (xr - xl) * (float)(i + 1) / 6.0f;
			float v = 0.9f - 0.8f * (float)i / 6.0f;
			float v2 = 0.9f - 0.8f * (float)(i + 1) / 6.0f;
			wallLine(corners, u, v, u2, v2, 0);
		}
		wallLine(corners, xl, 0.9f, xr, 0.1f, 0);
		break;
	}
	case kWallFeatureChar:
		wallChar(corners, map[1]);
		break;
	case kWallFeatureGlyph: {
		for (int i = 0; i < 7; i++) {
			float v = 0.2f + i * 0.1f;
			wallLine(corners, 0.2f, v, 0.8f, v, 0);
		}
		break;
	}
	case kWallFeatureElevator: {
		float xl = 0.2f, xr = 0.8f;
		float yb = 0.1f, yt = 0.9f;
		wallLine(corners, xl, yb, xl, yt, 0);
		wallLine(corners, xl, yt, xr, yt, 0);
		wallLine(corners, xr, yt, xr, yb, 0);
		wallLine(corners, xr, yb, xl, yb, 0);
		wallLine(corners, 0.5f, yb, 0.5f, yt, 0);
		break;
	}
	case kWallFeatureTunnel: {
		// Tunnel: hexagonal opening from Grid (0,0 0,5 1,6 5,6 6,5 6,0)
		static const float u_t[6] = { 0.0f,    0.0f,    1/6.0f,  5/6.0f,  1.0f,    1.0f };
		static const float v_t[6] = { 0.0f,    0.750f,  0.875f,  0.875f,  0.750f,  0.0f };
		wallPolygon(corners, u_t, v_t, 6, 0); // Black tunnel fill
		break;
	}
	case kWallFeatureAirlock: {
		float pts[][2] = {{0.0f, 0.5f}, {0.15f, 0.85f}, {0.5f, 1.0f}, {0.85f, 0.85f},
		                  {1.0f, 0.5f}, {0.85f, 0.15f}, {0.5f, 0.0f}, {0.15f, 0.15f}};
		float u[8], v[8];
		for (int i = 0; i < 8; i++) {
			u[i] = 0.1f + pts[i][0] * 0.8f;
			v[i] = 0.1f + pts[i][1] * 0.8f;
		}
		if (map[1] == 0) {
			wallPolygon(corners, u, v, 8, 0);
		} else {
			wallPolygon(corners, u, v, 8, 7);
			for (int i = 0; i < 8; i++) {
				int n = (i + 1) % 8;
				wallLine(corners, u[i], v[i], u[n], v[n], 0);
			}
			wallLine(corners, 0.1f, 0.5f, 0.9f, 0.5f, 0);
			wallLine(corners, 0.5f, 0.1f, 0.5f, 0.9f, 0);
		}
		break;
	}
	case kWallFeatureColor: {
		for (int i = 1; i <= 3; i++) {
			uint32 c = 120 + map[i] * 20;
			if (c == 120 && map[i] == 0 && !map[1] && !map[2] && !map[3] && !map[4])
				c = 100 + (_level * 15);
			float v = (float)i / 4.0f;
			wallLine(corners, 0.0f, v, 1.0f, v, c);
		}
		break;
	}
	default:
		break;
	}
}

void ColonyEngine::drawWallFeatures3D() {
	if (_corePower[_coreIndex] == 0)
		return;

	for (int y = 0; y < 31; y++) {
		for (int x = 0; x < 31; x++) {
			drawCellFeature3D(x, y);
			for (int dir = 0; dir < 4; dir++) {
				const uint8 *map = mapFeatureAt(x, y, dir);
				if (map && map[0] != kWallFeatureNone) {
					drawWallFeature3D(x, y, dir);
				}
			}
		}
	}
}


void ColonyEngine::renderCorridor3D() {
	bool lit = (_corePower[_coreIndex] > 0);
	bool oldWire = _wireframe;

	// Authentic look: Always wireframe for walls. 
	// Power ON = White background (fill), Black lines.
	// Power OFF = Black background (fill), White lines.
	_gfx->setWireframe(true, lit ? 15 : 0);

	_gfx->begin3D(_me.xloc, _me.yloc, 0, _me.look, _me.lookY, _screenR);
	_gfx->clear(lit ? 15 : 0);
 
	uint32 wallColor = lit ? 0 : 15; 
	uint32 floorColor = lit ? 0 : 15; 

	// Draw large floor and ceiling quads
	// These will be filled with the background color in the occlusion pass
	_gfx->draw3DQuad(-100000.0f, -100000.0f, -160.1f, 
	                100000.0f, -100000.0f, -160.1f, 
	                100000.0f, 100000.0f, -160.1f, 
	                -100000.0f, 100000.0f, -160.1f, floorColor);
 
	_gfx->draw3DQuad(-100000.0f, -100000.0f, 160.1f, 
	                100000.0f, -100000.0f, 160.1f, 
	                100000.0f, 100000.0f, 160.1f, 
	                -100000.0f, 100000.0f, 160.1f, floorColor);
 
	// Draw ceiling grid (Cuadricule) - Historically only on ceiling
	for (int i = 0; i <= 32; i++) {
		float d = i * 256.0f;
		float max_d = 32.0f * 256.0f;
		float zCeil = 160.0f;
		
		_gfx->draw3DLine(d, 0.0f, zCeil, d, max_d, zCeil, wallColor);
		_gfx->draw3DLine(0.0f, d, zCeil, max_d, d, zCeil, wallColor);
	}

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
	
	drawWallFeatures3D();
	drawStaticObjects();
		
	_gfx->end3D();
	_gfx->setWireframe(oldWire);
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
		for (int i = 0; i < 8; i++)
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
	case kObjBox1:
		draw3DPrism(obj, kBox1Part, false, tint(baseColor, 0));
		break;
	case kObjBox2:
		draw3DPrism(obj, kBox2Parts[0], false, tint(baseColor, 0));
		draw3DPrism(obj, kBox2Parts[1], false, tint(baseColor, 0));
		break;
	case kObjReactor:
		for (int i = 0; i < 3; i++)
			draw3DPrism(obj, kReactorParts[i], false, tint(baseColor, 0));
		break;
	case kObjPowerSuit:
		draw3DPrism(obj, kConsolePart, false, tint(baseColor, 0)); // Placeholder
		break;
	case kObjTeleport:
		draw3DPrism(obj, kCWallParts[0], false, tint(baseColor, 50)); // Placeholder
		break;
	case kObjTub:
		for (int i = 0; i < 2; i++)
			draw3DPrism(obj, kTubParts[i], false, tint(baseColor, 0));
		break;
	case kObjSink:
		for (int i = 0; i < 3; i++)
			draw3DPrism(obj, kSinkParts[i], false, tint(baseColor, 0));
		break;
	case kObjToilet:
		for (int i = 0; i < 4; i++)
			draw3DPrism(obj, kToiletParts[i], false, tint(baseColor, 0));
		break;
	case kObjPToilet:
		for (int i = 0; i < 5; i++)
			draw3DPrism(obj, kPToiletParts[i], false, tint(baseColor, 0));
		break;

	case kObjForkLift:
		draw3DPrism(obj, kBox1Part, false, tint(baseColor, 0)); // Placeholder
		break;
	default:
		return false;
	}
	return true;
}

} // End of namespace Colony
