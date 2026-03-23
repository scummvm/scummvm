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
 * Based on the original sources
 *   https://github.com/Croquetx/thecolony
 * Copyright (C) 1988, David A. Smith
 *
 * Distributed under Apache Version 2.0 License
 *
 */

#include "common/system.h"

#include "colony/colony.h"
#include "colony/render_internal.h"

namespace Colony {

uint32 packEyeOverlayMacColor(const uint16 rgb[3]) {
	return 0xFF000000 | ((rgb[0] >> 8) << 16) | ((rgb[1] >> 8) << 8) | (rgb[2] >> 8);
}

int mapEyeOverlayColorToMacColor(int colorIdx, int level) {
	switch (colorIdx) {
	case kColorPupil:        return 36; // c_pupil
	case kColorEyeball:      return 34; // c_eyeball
	case kColorIris:         return 35; // c_iris
	case kColorEyeIris:      return 32; // c_eye
	case kColorMiniEyeIris:  return 33; // c_meye
	case kColorQueenEye:     return 49; // c_equeen
	default:                 return 6;  // c_dwall fallback
	}
}

uint8 mapEyeOverlayColorToDOSFill(int colorIdx, int level) {
	switch (colorIdx) {
	case kColorBlack:
	case kColorPupil:
		return 0;
	case kColorEyeball:
		return 15;
	case kColorIris:
	case kColorEyeIris:
	case kColorMiniEyeIris:
	case kColorQueenEye:
		return 1;
	default:
		if (colorIdx >= 0 && colorIdx <= 15)
			return (uint8)colorIdx;
		if (colorIdx == kColorQueenBody && level == 7)
			return 15;
		return 7;
	}
}

int mapEyeOverlayColorToMacPattern(int colorIdx) {
	switch (colorIdx) {
	case kColorBlack:
	case kColorPupil:
		return kPatternBlack;
	case kColorEyeball:
		return kPatternWhite;
	case kColorIris:
	case kColorEyeIris:
	case kColorMiniEyeIris:
	case kColorQueenEye:
	default:
		return kPatternGray;
	}
}

bool projectCorridorPointRaw(const Common::Rect &screenR, uint8 look, int8 lookY,
	const int *sint, const int *cost, int camX, int camY,
	float worldX, float worldY, float worldZ,
	int &screenX, int &screenY) {
	const float dx = worldX - camX;
	const float dy = worldY - camY;
	const float dz = worldZ;

	const float sinYaw = sint[look] / 128.0f;
	const float cosYaw = cost[look] / 128.0f;
	const float side = dx * sinYaw - dy * cosYaw;
	const float forward = dx * cosYaw + dy * sinYaw;

	const float pitchRad = lookY * 2.0f * (float)M_PI / 256.0f;
	const float sinPitch = sinf(pitchRad);
	const float cosPitch = cosf(pitchRad);

	const float eyeX = side;
	const float eyeY = dz * cosPitch + forward * sinPitch;
	const float eyeZ = dz * sinPitch - forward * cosPitch;
	if (eyeZ >= -1.0f)
		return false;

	const float focal = (screenR.height() * 0.5f) / tanf(75.0f * (float)M_PI / 360.0f);
	const float centerX = screenR.left + screenR.width() * 0.5f;
	const float centerY = screenR.top + screenR.height() * 0.5f;

	screenX = (int)roundf(centerX + (eyeX * focal / -eyeZ));
	screenY = (int)roundf(centerY - (eyeY * focal / -eyeZ));
	return true;
}

bool isProjectedSurfaceVisible(const int *surface, int pointCount, const int *screenX, const int *screenY) {
	if (pointCount < 3)
		return false;

	for (int i = 0; i < pointCount; ++i) {
		const int cur = surface[i];
		const int next = surface[(i + 1) % pointCount];
		const int next2 = surface[(i + 2) % pointCount];
		const int32 dx = screenX[cur] - screenX[next];
		const int32 dy = screenY[cur] - screenY[next];
		const int32 dxp = screenX[next2] - screenX[next];
		const int32 dyp = screenY[next2] - screenY[next];

		if (dx < 0) {
			if (dy == 0) {
				if (dyp > 0)
					return false;
				if (dyp < 0)
					return true;
			} else {
				const int32 b = dy * dxp - dx * dyp;
				if (b > 0)
					return false;
				if (b < 0)
					return true;
			}
		} else if (dx > 0) {
			if (dy == 0) {
				if (dyp < 0)
					return false;
				if (dyp > 0)
					return true;
			} else {
				const int32 b = dx * dyp - dy * dxp;
				if (b < 0)
					return false;
				if (b > 0)
					return true;
			}
		} else {
			if (dy < 0) {
				if (dxp > 0)
					return true;
				if (dxp < 0)
					return false;
			}
			if (dy > 0) {
				if (dxp < 0)
					return true;
				if (dxp > 0)
					return false;
			}
		}
	}

	return false;
}

bool isProjectedPrismSurfaceVisible(const Common::Rect &screenR, const Colony::Thing &thing,
		const Colony::ColonyEngine::PrismPartDef &def, bool useLook, int surfaceIndex,
		uint8 cameraLook, int8 cameraLookY, int cameraX, int cameraY,
		const int *sint, const int *cost) {
	if (surfaceIndex < 0 || surfaceIndex >= def.surfaceCount)
		return false;

	const int pointCount = def.surfaces[surfaceIndex][1];
	if (pointCount < 3)
		return false;

	const uint8 ang = (useLook ? thing.where.look : thing.where.ang) + 32;
	const int32 rotCos = cost[ang];
	const int32 rotSin = sint[ang];
	int projectedX[32];
	int projectedY[32];
	bool projected[32];

	assert(def.pointCount <= ARRAYSIZE(projectedX));

	for (int i = 0; i < def.pointCount; ++i) {
		const int ox = def.points[i][0];
		const int oy = def.points[i][1];
		const int oz = def.points[i][2];
		const int32 rx = ((int32)ox * rotCos - (int32)oy * rotSin) >> 7;
		const int32 ry = ((int32)ox * rotSin + (int32)oy * rotCos) >> 7;
		projected[i] = projectCorridorPointRaw(screenR, cameraLook, cameraLookY, sint, cost, cameraX, cameraY,
												   (float)(rx + thing.where.xloc), (float)(ry + thing.where.yloc), (float)(oz - 160),
												   projectedX[i], projectedY[i]);
	}

	const int *surface = &def.surfaces[surfaceIndex][2];
	for (int i = 0; i < pointCount; ++i) {
		const int pointIdx = surface[i];
		if (pointIdx < 0 || pointIdx >= def.pointCount || !projected[pointIdx])
			return false;
	}

	return isProjectedSurfaceVisible(surface, pointCount, projectedX, projectedY);
}

const int kScreenPts[8][3] = {
	{-16, 64, 0}, {16, 64, 0}, {16, -64, 0}, {-16, -64, 0},
	{-16, 64, 288}, {16, 64, 288}, {16, -64, 288}, {-16, -64, 288}
};
const int kScreenSurf[4][8] = {
	{kColorBlack, 4, 0, 3, 7, 4, 0, 0}, {kColorBlack, 4, 3, 2, 6, 7, 0, 0},
	{kColorBlack, 4, 1, 0, 4, 5, 0, 0}, {kColorBlack, 4, 2, 1, 5, 6, 0, 0}
};
const int kTableTopPts[4][3] = {
	{-128, 128, 100}, {128, 128, 100}, {128, -128, 100}, {-128, -128, 100}
};
const int kTableTopSurf[1][8] = {{kColorTable, 4, 3, 2, 1, 0, 0, 0}};
const int kTableBasePts[8][3] = {
	{-5, 5, 0}, {5, 5, 0}, {5, -5, 0}, {-5, -5, 0},
	{-5, 5, 100}, {5, 5, 100}, {5, -5, 100}, {-5, -5, 100}
};
const int kTableBaseSurf[4][8] = {
	{kColorTableBase, 4, 0, 3, 7, 4, 0, 0}, {kColorTableBase, 4, 3, 2, 6, 7, 0, 0},
	{kColorTableBase, 4, 1, 0, 4, 5, 0, 0}, {kColorTableBase, 4, 2, 1, 5, 6, 0, 0}
};
const int kBedPostPts[4][3] = {
	{-82, 128, 100}, {82, 128, 100}, {82, 128, 0}, {-82, 128, 0}
};
const int kBedPostSurf[1][8] = {{kColorBed, 4, 3, 2, 1, 0, 0, 0}};
const int kBedBlanketPts[8][3] = {
	{-80, 70, 0}, {80, 70, 0}, {80, -175, 0}, {-80, -175, 0},
	{-80, 70, 60}, {80, 70, 60}, {80, -175, 60}, {-80, -175, 60}
};
const int kBlanketSurf[4][8] = {
	{kColorBlanket, 4, 0, 3, 7, 4, 0, 0}, {kColorBlanket, 4, 3, 2, 6, 7, 0, 0},
	{kColorBlanket, 4, 2, 1, 5, 6, 0, 0}, {kColorBlanket, 4, 7, 6, 5, 4, 0, 0}
};
const int kBedSheetPts[8][3] = {
	{-80, 128, 30}, {80, 128, 30}, {80, 70, 30}, {-80, 70, 30},
	{-80, 128, 60}, {80, 128, 60}, {80, 70, 60}, {-80, 70, 60}
};
const int kSheetSurf[3][8] = {
	{kColorSheet, 4, 0, 3, 7, 4, 0, 0}, {kColorSheet, 4, 2, 1, 5, 6, 0, 0},
	{kColorSheet, 4, 7, 6, 5, 4, 0, 0}
};
const int kBBedBlanketPts[8][3] = {
	{-128, 70, 0}, {128, 70, 0}, {128, -175, 0}, {-128, -175, 0},
	{-128, 70, 60}, {128, 70, 60}, {128, -175, 60}, {-128, -175, 60}
};
const int kBBedSheetPts[8][3] = {
	{-128, 128, 30}, {128, 128, 30}, {128, 70, 30}, {-128, 70, 30},
	{-128, 128, 60}, {128, 128, 60}, {128, 70, 60}, {-128, 70, 60}
};
const int kBBedPostPts[4][3] = {
	{-130, 128, 100}, {130, 128, 100}, {130, 128, 0}, {-130, 128, 0}
};
const int kDeskTopPts[4][3] = {
	{-150, 110, 100}, {150, 110, 100}, {150, -110, 100}, {-150, -110, 100}
};
const int kDeskTopSurf[1][8] = {{kColorDeskTop, 4, 3, 2, 1, 0, 0, 0}};
const int kDeskLeftPts[8][3] = {
	{-135, 95, 0}, {-55, 95, 0}, {-55, -95, 0}, {-135, -95, 0},
	{-135, 95, 100}, {-55, 95, 100}, {-55, -95, 100}, {-135, -95, 100}
};
const int kDeskRightPts[8][3] = {
	{55, 95, 0}, {135, 95, 0}, {135, -95, 0}, {55, -95, 0},
	{55, 95, 100}, {135, 95, 100}, {135, -95, 100}, {55, -95, 100}
};
const int kDeskCabSurf[4][8] = {
	{kColorDesk, 4, 0, 3, 7, 4, 0, 0}, {kColorDesk, 4, 3, 2, 6, 7, 0, 0},
	{kColorDesk, 4, 1, 0, 4, 5, 0, 0}, {kColorDesk, 4, 2, 1, 5, 6, 0, 0}
};
const int kSeatPts[4][3] = {
	{-40, 210, 60}, {40, 210, 60}, {40, 115, 60}, {-40, 115, 60}
};
const int kSeatSurf[1][8] = {{kColorDeskChair, 4, 3, 2, 1, 0, 0, 0}};
const int kArmLeftPts[4][3] = {
	{-40, 210, 90}, {-40, 210, 0}, {-40, 115, 0}, {-40, 115, 90}
};
const int kArmRightPts[4][3] = {
	{40, 210, 90}, {40, 210, 0}, {40, 115, 0}, {40, 115, 90}
};
const int kArmSurf[2][8] = {
	{kColorClear, 4, 3, 2, 1, 0, 0, 0}, {kColorClear, 4, 0, 1, 2, 3, 0, 0}
};
const int kBackPts[4][3] = {
	{-40, 210, 130}, {40, 210, 130}, {40, 210, 70}, {-40, 210, 70}
};
const int kBackSurf[2][8] = {
	{kColorDeskChair, 4, 3, 2, 1, 0, 0, 0}, {kColorDeskChair, 4, 0, 1, 2, 3, 0, 0}
};
const int kComputerPts[8][3] = {
	{70, 25, 100}, {120, 25, 100}, {120, -25, 100}, {70, -25, 100},
	{70, 25, 120}, {120, 25, 120}, {120, -25, 120}, {70, -25, 120}
};
const int kMonitorPts[8][3] = {
	{75, 20, 120}, {115, 20, 120}, {115, -20, 120}, {75, -20, 120},
	{75, 20, 155}, {115, 20, 155}, {115, -20, 145}, {75, -20, 145}
};
const int kComputerSurf[5][8] = {
	{kColorMac, 4, 7, 6, 5, 4, 0, 0}, {kColorMac, 4, 0, 3, 7, 4, 0, 0},
	{kColorMac, 4, 3, 2, 6, 7, 0, 0}, {kColorMac, 4, 1, 0, 4, 5, 0, 0},
	{kColorMac, 4, 2, 1, 5, 6, 0, 0}
};
const int kDeskScreenPts[4][3] = {
	{80, 20, 125}, {110, 20, 125}, {110, 20, 150}, {80, 20, 150}
};
const int kDeskScreenSurf[1][8] = {{kColorMacScreen, 4, 3, 2, 1, 0, 0, 0}};
const int kCSeatPts[4][3] = {
	{-40, 40, 60}, {40, 40, 60}, {40, -40, 60}, {-40, -40, 60}
};
const int kCSeatSurf[2][8] = {
	{kColorChair, 4, 3, 2, 1, 0, 0, 0}, {kColorChair, 4, 0, 1, 2, 3, 0, 0}
};
const int kCArmLeftPts[4][3] = {
	{-50, 40, 90}, {-40, 40, 60}, {-40, -40, 60}, {-50, -40, 90}
};
const int kCArmLeftSurf[2][8] = {
	{kColorChair, 4, 3, 2, 1, 0, 0, 0}, {kColorChair, 4, 0, 1, 2, 3, 0, 0}
};
const int kCArmRightPts[4][3] = {
	{50, 40, 90}, {40, 40, 60}, {40, -40, 60}, {50, -40, 90}
};
const int kCArmRightSurf[2][8] = {
	{kColorChair, 4, 3, 2, 1, 0, 0, 0}, {kColorChair, 4, 0, 1, 2, 3, 0, 0}
};
const int kCBackPts[4][3] = {
	{-20, 60, 150}, {20, 60, 150}, {40, 40, 60}, {-40, 40, 60}
};
const int kCBackSurf[2][8] = {
	{kColorChair, 4, 3, 2, 1, 0, 0, 0}, {kColorChair, 4, 0, 1, 2, 3, 0, 0}
};
const int kCBasePts[8][3] = {
	{-5, 5, 0}, {5, 5, 0}, {5, -5, 0}, {-5, -5, 0},
	{-5, 5, 60}, {5, 5, 60}, {5, -5, 60}, {-5, -5, 60}
};
const int kCBaseSurf[4][8] = {
	{kColorChairBase, 4, 0, 3, 7, 4, 0, 0}, {kColorChairBase, 4, 3, 2, 6, 7, 0, 0},
	{kColorChairBase, 4, 1, 0, 4, 5, 0, 0}, {kColorChairBase, 4, 2, 1, 5, 6, 0, 0}
};
const int kConsolePts[8][3] = {
	{-5, 5, 0}, {5, 5, 0}, {5, -5, 0}, {-5, -5, 0},
	{-100, 70, 100}, {-35, 70, 140}, {-35, -70, 140}, {-100, -70, 100}
};
const int kConsoleSurf[5][8] = {
	{kColorConsole, 4, 4, 0, 3, 7, 0, 0}, {kColorConsole, 4, 7, 3, 2, 6, 0, 0},
	{kColorConsole, 4, 5, 1, 0, 4, 0, 0}, {kColorConsole, 4, 6, 2, 1, 5, 0, 0},
	{kColorBlack, 4, 7, 6, 5, 4, 0, 0}
};
const int kCouchSurf[5][8] = {
	{kColorCouch, 4, 0, 3, 7, 4, 0, 0}, {kColorCouch, 4, 3, 2, 6, 7, 0, 0},
	{kColorCouch, 4, 1, 0, 4, 5, 0, 0}, {kColorCouch, 4, 2, 1, 5, 6, 0, 0},
	{kColorCouch, 4, 7, 6, 5, 4, 0, 0}
};
const int kACouchPts[8][3] = {
	{-50, 150, 0}, {50, 150, 0}, {50, -150, 0}, {-50, -150, 0},
	{-50, 150, 50}, {50, 150, 50}, {50, -150, 50}, {-50, -150, 50}
};
const int kBCouchPts[8][3] = {
	{-80, 150, 0}, {-45, 150, 0}, {-45, -150, 0}, {-80, -150, 0},
	{-80, 150, 120}, {-55, 150, 120}, {-55, -150, 120}, {-80, -150, 120}
};
const int kCCouchPts[8][3] = {
	{-70, 170, 0}, {50, 170, 0}, {50, 150, 0}, {-70, 150, 0},
	{-70, 170, 80}, {50, 170, 80}, {50, 150, 80}, {-70, 150, 80}
};
const int kDCouchPts[8][3] = {
	{-70, -150, 0}, {50, -150, 0}, {50, -170, 0}, {-70, -170, 0},
	{-70, -150, 80}, {50, -150, 80}, {50, -170, 80}, {-70, -170, 80}
};
const int kAChairPts[8][3] = {
	{-50, 50, 0}, {50, 50, 0}, {50, -50, 0}, {-50, -50, 0},
	{-50, 50, 50}, {50, 50, 50}, {50, -50, 50}, {-50, -50, 50}
};
const int kBChairPts[8][3] = {
	{-80, 50, 0}, {-45, 50, 0}, {-45, -50, 0}, {-80, -50, 0},
	{-80, 50, 120}, {-55, 50, 120}, {-55, -50, 120}, {-80, -50, 120}
};
const int kCChairPts2[8][3] = {
	{-70, 70, 0}, {50, 70, 0}, {50, 50, 0}, {-70, 50, 0},
	{-70, 70, 80}, {50, 70, 80}, {50, 50, 80}, {-70, 50, 80}
};
const int kDChairPts[8][3] = {
	{-70, -50, 0}, {50, -50, 0}, {50, -70, 0}, {-70, -70, 0},
	{-70, -50, 80}, {50, -50, 80}, {50, -70, 80}, {-70, -70, 80}
};
const int kTVBodyPts[8][3] = {
	{-30, 60, 0}, {30, 60, 0}, {30, -60, 0}, {-30, -60, 0},
	{-30, 60, 120}, {30, 60, 120}, {30, -60, 120}, {-30, -60, 120}
};
const int kTVBodySurf[5][8] = {
	{kColorTV, 4, 0, 3, 7, 4, 0, 0}, {kColorTV, 4, 3, 2, 6, 7, 0, 0},
	{kColorTV, 4, 1, 0, 4, 5, 0, 0}, {kColorTV, 4, 2, 1, 5, 6, 0, 0},
	{kColorTV, 4, 7, 6, 5, 4, 0, 0}
};
const int kTVScreenPts[4][3] = {
	{30, 50, 10}, {30, -50, 10}, {30, 50, 110}, {30, -50, 110}
};
const int kTVScreenSurf[1][8] = {{kColorTVScreen, 4, 1, 0, 2, 3, 0, 0}};
const int kDrawerPts[8][3] = {
	{-80, 70, 0}, {0, 70, 0}, {0, -70, 0}, {-80, -70, 0},
	{-80, 70, 100}, {0, 70, 100}, {0, -70, 100}, {-80, -70, 100}
};
const int kDrawerSurf[5][8] = {
	{kColorDrawer, 4, 0, 3, 7, 4, 0, 0}, {kColorDrawer, 4, 3, 2, 6, 7, 0, 0},
	{kColorDrawer, 4, 1, 0, 4, 5, 0, 0}, {kColorDrawer, 4, 2, 1, 5, 6, 0, 0},
	{kColorDrawer, 4, 7, 6, 5, 4, 0, 0}
};
const int kMirrorPts[4][3] = {
	{-80, 65, 100}, {-80, -65, 100}, {-80, 65, 210}, {-80, -65, 210}
};
const int kMirrorSurf[1][8] = {{kColorSilver, 4, 1, 0, 2, 3, 0, 0}};

// Bathtub geometry
const int kTubPts[8][3] = {
	{-128, 128,  0}, {   0, 128,  0}, {   0,-128,  0}, {-128,-128,  0},
	{-128, 128, 70}, {   0, 128, 70}, {   0,-128, 70}, {-128,-128, 70}
};
const int kTubSurf[5][8] = {
	{kColorBath, 4, 0, 3, 7, 4, 0, 0}, {kColorBath, 4, 3, 2, 6, 7, 0, 0},
	{kColorBath, 4, 1, 0, 4, 5, 0, 0}, {kColorBath, 4, 2, 1, 5, 6, 0, 0},
	{kColorBath, 4, 7, 6, 5, 4, 0, 0}
};
const int kDTubPts[6][3] = {
	{-16, 112, 70}, {-8, 0, 70}, {-16, -112, 70}, {-112, -112, 70}, {-120, 0, 70}, {-112, 112, 70}
};
const int kDTubSurf[1][8] = {{kColorWater, 6, 5, 4, 3, 2, 1, 0}};

// Toilet geometry
const int kAToiletPts[8][3] = {
	{-128, 45, 30}, {-100, 45, 30}, {-100, -45, 30}, {-128, -45, 30},
	{-128, 45, 100}, {-100, 45, 100}, {-100, -45, 100}, {-128, -45, 100}
};
const int kAToiletSurf[5][8] = {
	{kColorBath, 4, 0, 3, 7, 4, 0, 0}, {kColorBath, 4, 3, 2, 6, 7, 0, 0},
	{kColorBath, 4, 1, 0, 4, 5, 0, 0}, {kColorBath, 4, 2, 1, 5, 6, 0, 0},
	{kColorBath, 4, 7, 6, 5, 4, 0, 0}
};
const int kBToiletPts[12][3] = {
	{-100, 20, 50}, {-60, 40, 50}, {-20, 20, 50}, {-20, -20, 50}, {-60, -40, 50}, {-100, -20, 50},
	{-80, 10,  0}, {-60, 20,  0}, {-40, 10,  0}, {-40, -10,  0}, {-60, -20,  0}, {-80, -10,  0}
};
const int kBToiletSurf[7][8] = {
	{kColorBath, 4, 0, 1, 7, 6, 0, 0}, {kColorBath, 4, 1, 2, 8, 7, 0, 0}, {kColorBath, 4, 2, 3, 9, 8, 0, 0},
	{kColorBath, 4, 3, 4, 10, 9, 0, 0}, {kColorBath, 4, 4, 5, 11, 10, 0, 0}, {kColorBath, 4, 5, 0, 6, 11, 0, 0},
	{kColorBath, 6, 5, 4, 3, 2, 1, 0}
};
const int kCToiletPts[6][3] = {
	{-95, 15, 50}, {-60, 35, 50}, {-25, 15, 50}, {-25, -15, 50}, {-60, -35, 50}, {-95, -15, 50}
};
const int kCToiletSurf[1][8] = {{kColorWater, 6, 5, 4, 3, 2, 1, 0}};
const int kDToiletPts[6][3] = {
	{-100, 20, 50}, {-100, 40, 90}, {-100, 20, 130}, {-100, -20, 130}, {-100, -40, 90}, {-100, -20, 50}
};
const int kDToiletSurf[1][8] = {{kColorLtGreen, 6, 5, 4, 3, 2, 1, 0}};
const int kEToiletPts[4][3] = {
	{-128,-128, 20}, {-128,-128, 200}, { 128,-128, 200}, { 128,-128, 20}
};
const int kEToiletSurf[1][8] = {{kColorDkGray, 4, 0, 1, 2, 3, 0, 0}};

// Sink geometry
const int kSinkPts[8][3] = {
	{-128, 50, 70}, {-50, 50, 90}, {-50,-50, 90}, {-128,-50, 70},
	{-128, 50, 110}, {-50, 50, 110}, {-50,-50, 110}, {-128,-50, 110}
};
const int kSinkSurf[5][8] = {
	{kColorBath, 4, 0, 3, 7, 4, 0, 0}, {kColorBath, 4, 3, 2, 6, 7, 0, 0},
	{kColorBath, 4, 1, 0, 4, 5, 0, 0}, {kColorBath, 4, 2, 1, 5, 6, 0, 0},
	{kColorBath, 4, 7, 6, 5, 4, 0, 0}
};
const int kDSinkPts[6][3] = {
	{-55, 0, 110}, {-60, -45, 110}, {-118, -45, 110}, {-123, 0, 110}, {-118, 45, 110}, {-60, 45, 110}
};
const int kDSinkSurf[1][8] = {{kColorWater, 6, 5, 4, 3, 2, 1, 0}};
const int kSinkMirrorPts[4][3] = {
	{-128, 65, 130}, {-128, -65, 130}, {-128, 65, 250}, {-128, -65, 250}
};
const int kSinkMirrorSurf[1][8] = {{kColorSilver, 4, 1, 0, 2, 3, 0, 0}};

const Colony::ColonyEngine::PrismPartDef kScreenPart = {8, kScreenPts, 4, kScreenSurf};
const Colony::ColonyEngine::PrismPartDef kTableParts[2] = {
	{4, kTableTopPts, 1, kTableTopSurf},
	{8, kTableBasePts, 4, kTableBaseSurf}
};
const Colony::ColonyEngine::PrismPartDef kBedParts[3] = {
	{4, kBedPostPts, 1, kBedPostSurf},
	{8, kBedBlanketPts, 4, kBlanketSurf},
	{8, kBedSheetPts, 3, kSheetSurf}
};
const Colony::ColonyEngine::PrismPartDef kBBedParts[3] = {
	{4, kBBedPostPts, 1, kBedPostSurf},
	{8, kBBedBlanketPts, 4, kBlanketSurf},
	{8, kBBedSheetPts, 3, kSheetSurf}
};
const Colony::ColonyEngine::PrismPartDef kDeskParts[10] = {
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
const Colony::ColonyEngine::PrismPartDef kCChairParts[5] = {
	{4, kCSeatPts, 2, kCSeatSurf},
	{4, kCArmLeftPts, 2, kCArmLeftSurf},
	{4, kCArmRightPts, 2, kCArmRightSurf},
	{4, kCBackPts, 1, kCBackSurf},
	{8, kCBasePts, 4, kCBaseSurf}
};
const Colony::ColonyEngine::PrismPartDef kConsolePart = {8, kConsolePts, 5, kConsoleSurf};
const Colony::ColonyEngine::PrismPartDef kCouchParts[4] = {
	{8, kACouchPts, 5, kCouchSurf},
	{8, kBCouchPts, 5, kCouchSurf},
	{8, kCCouchPts, 5, kCouchSurf},
	{8, kDCouchPts, 5, kCouchSurf}
};
const Colony::ColonyEngine::PrismPartDef kChairParts[4] = {
	{8, kAChairPts, 5, kCouchSurf},
	{8, kBChairPts, 5, kCouchSurf},
	{8, kCChairPts2, 5, kCouchSurf},
	{8, kDChairPts, 5, kCouchSurf}
};
const Colony::ColonyEngine::PrismPartDef kTVParts[2] = {
	{8, kTVBodyPts, 5, kTVBodySurf},
	{4, kTVScreenPts, 1, kTVScreenSurf}
};
const Colony::ColonyEngine::PrismPartDef kDrawerParts[2] = {
	{8, kDrawerPts, 5, kDrawerSurf},
	{4, kMirrorPts, 1, kMirrorSurf}
};
const Colony::ColonyEngine::PrismPartDef kTubParts[2] = {
	{8, kTubPts, 5, kTubSurf},
	{6, kDTubPts, 1, kDTubSurf}
};
const Colony::ColonyEngine::PrismPartDef kSinkParts[3] = {
	{8, kSinkPts, 5, kSinkSurf},
	{6, kDSinkPts, 1, kDSinkSurf},
	{4, kSinkMirrorPts, 1, kSinkMirrorSurf}
};
const Colony::ColonyEngine::PrismPartDef kToiletParts[4] = {
	{8, kAToiletPts, 5, kAToiletSurf},
	{12, kBToiletPts, 7, kBToiletSurf},
	{6, kCToiletPts, 1, kCToiletSurf},
	{6, kDToiletPts, 1, kDToiletSurf}
};
const Colony::ColonyEngine::PrismPartDef kPToiletParts[5] = {
	{8, kAToiletPts, 5, kAToiletSurf},
	{12, kBToiletPts, 7, kBToiletSurf},
	{6, kCToiletPts, 1, kCToiletSurf},
	{6, kDToiletPts, 1, kDToiletSurf},
	{4, kEToiletPts, 1, kEToiletSurf}
};

const int kFWallPts[4][3] = {
	{-128, 128, 0}, {128, -128, 0}, {-128, 128, 320}, {128, -128, 320}
};
const int kFWallSurf[1][8] = {{kColorWall, 4, 2, 3, 1, 0, 0, 0}};
const Colony::ColonyEngine::PrismPartDef kFWallPart = {4, kFWallPts, 1, kFWallSurf};

const int kCWallPts[8][3] = {
	{-128, 128, 0}, {0, 112, 0}, {112, 0, 0}, {128, -128, 0},
	{-128, 128, 320},  {0, 112, 320},  {112, 0, 320},  {128, -128, 320}
};
const int kCWallSurf[3][8] = {
	{kColorWall, 4, 1, 0, 4, 5, 0, 0}, {kColorWall, 4, 2, 1, 5, 6, 0, 0}, {kColorWall, 4, 3, 2, 6, 7, 0, 0}
};
const Colony::ColonyEngine::PrismPartDef kCWallPart = {8, kCWallPts, 3, kCWallSurf};

const int kPlantPotPts[12][3] = {
	{10, 17, 40}, {20, 0, 40}, {10, -17, 40}, {-10, -17, 40}, {-20, 0, 40}, {-10, 17, 40},
	{8, 12, 0}, {15, 0, 0}, {8, -12, 0}, {-8, -12, 0}, {-15, 0, 0}, {-8, 12, 0}
};
const int kPlantPotSurf[6][8] = {
	{kColorPot, 4, 0, 1, 7, 6, 0, 0}, {kColorPot, 4, 1, 2, 8, 7, 0, 0}, {kColorPot, 4, 2, 3, 9, 8, 0, 0},
	{kColorPot, 4, 3, 4, 10, 9, 0, 0}, {kColorPot, 4, 4, 5, 11, 10, 0, 0}, {kColorPot, 4, 5, 0, 6, 11, 0, 0}
};
const int kPlantTopPotPts[6][3] = {
	{10, 17, 40}, {20, 0, 40}, {10, -17, 40}, {-10, -17, 40}, {-20, 0, 40}, {-10, 17, 40}
};
const int kPlantTopPotSurf[1][8] = {{kColorDkGray, 6, 5, 4, 3, 2, 1, 0}};

const int kPlantLeaf0Pts[3][3] = {{0, 0, 0}, {20, 20, 150}, {70, 70, 100}};
const int kPlantLeaf1Pts[3][3] = {{0, 0, 0}, {-20, 30, 100}, {-60, 50, 50}};
const int kPlantLeaf2Pts[3][3] = {{0, 0, 0}, {-20, -10, 70}, {-90, -70, 50}};
const int kPlantLeaf3Pts[3][3] = {{0, 0, 0}, {20, -10, 50}, {90, -70, 70}};
const int kPlantLeaf4Pts[3][3] = {{0, 0, 0}, {20, -30, 190}, {40, -60, 150}};
const int kPlantLeaf5Pts[3][3] = {{0, 0, 0}, {20, -10, 130}, {50, -80, 200}};

const int kPlantLeafSurf[2][8] = {{kColorClear, 3, 0, 1, 2, 0, 0, 0}, {kColorClear, 3, 2, 1, 0, 0, 0, 0}};

const Colony::ColonyEngine::PrismPartDef kPlantParts[8] = {
	{12, kPlantPotPts, 6, kPlantPotSurf},
	{6, kPlantTopPotPts, 1, kPlantTopPotSurf},
	{3, kPlantLeaf0Pts, 2, kPlantLeafSurf},
	{3, kPlantLeaf1Pts, 2, kPlantLeafSurf},
	{3, kPlantLeaf2Pts, 2, kPlantLeafSurf},
	{3, kPlantLeaf3Pts, 2, kPlantLeafSurf},
	{3, kPlantLeaf4Pts, 2, kPlantLeafSurf},
	{3, kPlantLeaf5Pts, 2, kPlantLeafSurf}
};

const int kBox1Pts[8][3] = {
	{-50, 50, 0}, {50, 50, 0}, {50, -50, 0}, {-50, -50, 0},
	{-50, 50, 100}, {50, 50, 100}, {50, -50, 100}, {-50, -50, 100}
};
const int kBox1Surf[5][8] = {
	{kColorBox, 4, 0, 3, 7, 4, 0, 0}, {kColorBox, 4, 3, 2, 6, 7, 0, 0},
	{kColorBox, 4, 1, 0, 4, 5, 0, 0}, {kColorBox, 4, 2, 1, 5, 6, 0, 0}, {kColorBox, 4, 7, 6, 5, 4, 0, 0}
};
const int kBox2Pts[8][3] = {
	{-50, 50, 100}, {50, 50, 100}, {50, -50, 100}, {-50, -50, 100},
	{-50, 50, 200}, {50, 50, 200}, {50, -50, 200}, {-50, -50, 200}
};

const int kReactorCorePts[12][3] = {
	{-40, 20, 288}, {0, 40, 288}, {40, 20, 288}, {40, -20, 288}, {0, -40, 288}, {-40, -20, 288},
	{-40, 20, 32}, {0, 40, 32}, {40, 20, 32}, {40, -20, 32}, {0, -40, 32}, {-40, -20, 32}
};
const int kReactorCoreSurf[7][8] = {
	{kColorCCore, 4, 0, 1, 7, 6, 0, 0}, {kColorCCore, 4, 1, 2, 8, 7, 0, 0}, {kColorCCore, 4, 2, 3, 9, 8, 0, 0},
	{kColorCCore, 4, 3, 4, 10, 9, 0, 0}, {kColorCCore, 4, 4, 5, 11, 10, 0, 0}, {kColorCCore, 4, 5, 0, 6, 11, 0, 0},
	{kColorCCore, 6, 5, 4, 3, 2, 1, 0}
};
const int kReactorBasePts[8][3] = {
	{-128, 128, 0}, {128, 128, 0}, {128, -128, 0}, {-128, -128, 0},
	{-128, 128, 32}, {128, 128, 32}, {128, -128, 32}, {-128, -128, 32}
};
const int kReactorBaseSurf[6][8] = {
	{kColorReactor, 4, 0, 3, 7, 4, 0, 0}, {kColorReactor, 4, 3, 2, 6, 7, 0, 0}, {kColorReactor, 4, 1, 0, 4, 5, 0, 0},
	{kColorReactor, 4, 2, 1, 5, 6, 0, 0}, {kColorReactor, 4, 7, 6, 5, 4, 0, 0}, {kColorReactor, 4, 0, 1, 2, 3, 0, 0}
};
const int kReactorTopPts[8][3] = {
	{-128, 128, 288}, {128, 128, 288}, {128, -128, 288}, {-128, -128, 288},
	{-128, 128, 320}, {128, 128, 320}, {128, -128, 320}, {-128, -128, 320}
};

const Colony::ColonyEngine::PrismPartDef kBox1Part = {8, kBox1Pts, 5, kBox1Surf};
const Colony::ColonyEngine::PrismPartDef kBox2Parts[2] = {
	{8, kBox2Pts, 4, kBox1Surf}, // Body (stacked on box1)
	{8, kBox1Pts, 5, kBox1Surf}  // Lid (same geometry as box1 base)
};
// Bench: simple box (1 part). DOS INITOBJ.C InitBench.
const int kBenchPts[8][3] = {
	{-60, 128, 0}, {60, 128, 0}, {60, -128, 0}, {-60, -128, 0},
	{-60, 128, 120}, {60, 128, 120}, {60, -128, 120}, {-60, -128, 120}
};
const int kBenchSurf[5][8] = {
	{kColorBench, 4, 0, 3, 7, 4, 0, 0}, {kColorBench, 4, 3, 2, 6, 7, 0, 0},
	{kColorBench, 4, 1, 0, 4, 5, 0, 0}, {kColorBench, 4, 2, 1, 5, 6, 0, 0},
	{kColorBlack, 4, 7, 6, 5, 4, 0, 0}
};
const Colony::ColonyEngine::PrismPartDef kBenchPart = {8, kBenchPts, 5, kBenchSurf};

// CBench: L-shaped corner bench (2 parts). DOS INITOBJ.C InitCBench.
// Part 0: slanted front face (front-left Y=60, front-right Y=-60)
const int kCBenchPts[8][3] = {
	{-60, 60, 0}, {60, -60, 0}, {60, -128, 0}, {-60, -128, 0},
	{-60, 60, 120}, {60, -60, 120}, {60, -128, 120}, {-60, -128, 120}
};
// Part 1: wider perpendicular section
const int kDBenchPts[8][3] = {
	{-60, 60, 0}, {128, 60, 0}, {128, -60, 0}, {60, -60, 0},
	{-60, 60, 120}, {128, 60, 120}, {128, -60, 120}, {60, -60, 120}
};
const Colony::ColonyEngine::PrismPartDef kCBenchParts[2] = {
	{8, kCBenchPts, 5, kBenchSurf},
	{8, kDBenchPts, 5, kBenchSurf}
};

// Power Suit: triangular prism body + small rectangular pedestal + flat table + hexagonal power source
// DOS INITOBJ.C: 5 prism parts. Floor=160, so 2*Floor=320.
const int kPowerTopPts[3][3] = {{-150, 120, 320}, {150, 120, 320}, {0, -150, 320}};
const int kPowerTopSurf[1][8] = {{kColorPower, 3, 0, 1, 2, 0, 0, 0}};

const int kPowerBottomPts[3][3] = {{-150, 120, 0}, {150, 120, 0}, {0, -150, 0}};
const int kPowerBottomSurf[1][8] = {{kColorPower, 3, 2, 1, 0, 0, 0, 0}};

const int kPowerBasePts[8][3] = {
	{-5, 100, 0}, {5, 100, 0}, {5, 90, 0}, {-5, 90, 0},
	{-5, 100, 100}, {5, 100, 100}, {5, 90, 100}, {-5, 90, 100}
};
const int kPowerBaseSurf[4][8] = {
	{kColorPBase, 4, 0, 3, 7, 4, 0, 0}, {kColorPBase, 4, 3, 2, 6, 7, 0, 0},
	{kColorPBase, 4, 1, 0, 4, 5, 0, 0}, {kColorPBase, 4, 2, 1, 5, 6, 0, 0}
};

const int kPowerTablePts[4][3] = {{-50, 135, 100}, {50, 135, 100}, {50, 55, 100}, {-50, 55, 100}};
const int kPowerTableSurf[1][8] = {{kColorBlack, 4, 3, 2, 1, 0, 0, 0}};

const int kPowerSourcePts[12][3] = {
	{-75, 0, 290}, {-35, 60, 290}, {35, 60, 290}, {75, 0, 290}, {35, -60, 290}, {-35, -60, 290},
	{-75, 0, 320}, {-35, 60, 320}, {35, 60, 320}, {75, 0, 320}, {35, -60, 320}, {-35, -60, 320}
};
const int kPowerSourceSurf[7][8] = {
	{kColorRainbow1, 6, 0, 1, 2, 3, 4, 5},
	{kColorPSource, 4, 0, 6, 7, 1, 0, 0}, {kColorPSource, 4, 1, 7, 8, 2, 0, 0},
	{kColorPSource, 4, 2, 8, 9, 3, 0, 0}, {kColorPSource, 4, 3, 9, 10, 4, 0, 0},
	{kColorPSource, 4, 4, 10, 11, 5, 0, 0}, {kColorPSource, 4, 5, 11, 6, 0, 0, 0}
};

const Colony::ColonyEngine::PrismPartDef kPowerSuitParts[5] = {
	{3, kPowerTopPts, 1, kPowerTopSurf},
	{3, kPowerBottomPts, 1, kPowerBottomSurf},
	{8, kPowerBasePts, 4, kPowerBaseSurf},
	{4, kPowerTablePts, 1, kPowerTableSurf},
	{12, kPowerSourcePts, 7, kPowerSourceSurf}
};

// Cryo tube: top (coffin-shaped lid) + base
const int kCryoTopPts[16][3] = {
	{-130,  50,  80}, { 130,  50,  80}, { 130, -50,  80}, {-130, -50,  80},
	{-130,  50, 140}, { 130,  50, 140}, { 130, -50, 140}, {-130, -50, 140},
	{   0,  50, 140}, {   0, -50, 140},
	{-140,  70, 110}, { 140,  70, 110}, { 140, -70, 110}, {-140, -70, 110},
	{   0,  70, 110}, {   0, -70, 110}
};
const int kCryoTopSurf[12][8] = {
	{kColorCryo,      4, 7, 9, 8, 4, 0, 0},
	{kColorCryoGlass, 4, 9, 6, 5, 8, 0, 0},
	{kColorCryo,      4, 0, 10, 11, 1, 0, 0},
	{kColorCryo,      4, 1, 11, 12, 2, 0, 0},
	{kColorCryo,      4, 2, 12, 13, 3, 0, 0},
	{kColorCryo,      4, 3, 13, 10, 0, 0, 0},
	{kColorCryo,      4, 7, 13, 15, 9, 0, 0},
	{kColorCryo,      4, 4, 10, 13, 7, 0, 0},
	{kColorCryo,      4, 14, 10, 4, 8, 0, 0},
	{kColorSilver,    4, 5, 11, 14, 8, 0, 0},
	{kColorSilver,    4, 6, 12, 11, 5, 0, 0},
	{kColorSilver,    4, 9, 15, 12, 6, 0, 0}
};
const int kCryoBasePts[8][3] = {
	{-130,  50,  0}, { 130,  50,  0}, { 130, -50,  0}, {-130, -50,  0},
	{-130,  50, 80}, { 130,  50, 80}, { 130, -50, 80}, {-130, -50, 80}
};
const int kCryoBaseSurf[5][8] = {
	{kColorCryoBase, 4, 0, 3, 7, 4, 0, 0}, {kColorCryoBase, 4, 3, 2, 6, 7, 0, 0},
	{kColorCryoBase, 4, 1, 0, 4, 5, 0, 0}, {kColorCryoBase, 4, 2, 1, 5, 6, 0, 0},
	{kColorCryo,     4, 7, 6, 5, 4, 0, 0}
};
const Colony::ColonyEngine::PrismPartDef kCryoParts[2] = {
	{16, kCryoTopPts, 12, kCryoTopSurf},
	{8, kCryoBasePts, 5, kCryoBaseSurf}
};

// Forklift: cab + treads + upper-left arm + lower-left fork + upper-right arm + lower-right fork
const int kFLCabPts[14][3] = {
	{-35, 60, 40}, {35, 60, 40}, {35, -60, 40}, {-35, -60, 40},
	{-15, 60, 260}, {15, 60, 260}, {15, -60, 260}, {-15, -60, 260},
	{25, 60, 140}, {25, -60, 140},
	{70, 35, 120}, {70, -35, 120},
	{-70, 40, 80}, {-70, -40, 80}
};
const int kFLCabSurf[12][8] = {
	{kColorForklift, 4, 0, 3, 13, 12, 0, 0}, {kColorForklift, 4, 12, 13, 7, 4, 0, 0},
	{kColorForklift, 3, 0, 12, 4, 0, 0, 0},  {kColorForklift, 3, 3, 7, 13, 0, 0, 0},
	{kColorForklift, 4, 3, 2, 6, 7, 0, 0},   {kColorForklift, 4, 1, 0, 4, 5, 0, 0},
	{kColorForklift, 3, 1, 8, 10, 0, 0, 0},  {kColorForklift, 3, 2, 11, 9, 0, 0, 0},
	{kColorForklift, 4, 1, 10, 11, 2, 0, 0},
	{kColorSilver,   3, 8, 5, 10, 0, 0, 0},  {kColorSilver, 3, 11, 6, 9, 0, 0, 0},
	{kColorSilver,   4, 10, 5, 6, 11, 0, 0}
};
const int kFLTreadPts[12][3] = {
	{-60, 60, 20}, {60, 60, 20}, {60, -60, 20}, {-60, -60, 20},
	{-35, 60, 40}, {35, 60, 40}, {35, -60, 40}, {-35, -60, 40},
	{-35, 60, 0}, {35, 60, 0}, {35, -60, 0}, {-35, -60, 0}
};
const int kFLTreadSurf[6][8] = {
	{kColorTread1, 4, 0, 3, 7, 4, 0, 0},
	{kColorTread2, 6, 3, 11, 10, 2, 6, 7},
	{kColorTread2, 6, 0, 4, 5, 1, 9, 8},
	{kColorTread1, 4, 2, 1, 5, 6, 0, 0},
	{kColorTread1, 4, 0, 8, 11, 3, 0, 0},
	{kColorTread1, 4, 10, 9, 1, 2, 0, 0}
};
const int kFLULPts[8][3] = {
	{-15, 70, 120}, {15, 70, 120}, {15, 60, 120}, {-15, 60, 120},
	{-25, 70, 230}, {25, 70, 230}, {25, 60, 230}, {-25, 60, 230}
};
const int kFLArmSurf[4][8] = {
	{kColorForklift, 4, 0, 3, 7, 4, 0, 0}, {kColorForklift, 4, 3, 2, 6, 7, 0, 0},
	{kColorForklift, 4, 1, 0, 4, 5, 0, 0}, {kColorForklift, 4, 2, 1, 5, 6, 0, 0}
};
const int kFLLLPts[8][3] = {
	{-15, 80, 120}, {100, 80, 125}, {100, 70, 125}, {-15, 70, 120},
	{-15, 80, 150}, {100, 80, 140}, {100, 70, 140}, {-15, 70, 150}
};
const int kFLForkSurf[6][8] = {
	{kColorForklift, 4, 0, 3, 7, 4, 0, 0}, {kColorForklift, 4, 3, 2, 6, 7, 0, 0},
	{kColorForklift, 4, 1, 0, 4, 5, 0, 0}, {kColorBlack,    4, 2, 1, 5, 6, 0, 0},
	{kColorForklift, 4, 7, 6, 5, 4, 0, 0}, {kColorForklift, 4, 0, 1, 2, 3, 0, 0}
};
const int kFLURPts[8][3] = {
	{-15, -60, 120}, {15, -60, 120}, {15, -70, 120}, {-15, -70, 120},
	{-25, -60, 230}, {25, -60, 230}, {25, -70, 230}, {-25, -70, 230}
};
const int kFLLRPts[8][3] = {
	{-15, -70, 120}, {100, -70, 125}, {100, -80, 125}, {-15, -80, 120},
	{-15, -70, 150}, {100, -70, 140}, {100, -80, 140}, {-15, -80, 150}
};
const Colony::ColonyEngine::PrismPartDef kForkliftParts[6] = {
	{14, kFLCabPts, 12, kFLCabSurf},
	{12, kFLTreadPts, 6, kFLTreadSurf},
	{8, kFLULPts, 4, kFLArmSurf},
	{8, kFLLLPts, 6, kFLForkSurf},
	{8, kFLURPts, 4, kFLArmSurf},
	{8, kFLLRPts, 6, kFLForkSurf}
};

// Teleport: octagonal booth with flared middle
const int kTelePts[24][3] = {
	// Ring 0: outer flared ring at z=140
	{   0, 175, 140}, { 125, 125, 140}, { 175,   0, 140}, { 125,-125, 140},
	{   0,-175, 140}, {-125,-125, 140}, {-175,   0, 140}, {-125, 125, 140},
	// Ring 1: inner ring at z=0 (bottom)
	{  0,  80, 0}, { 65,  65, 0}, { 80,   0, 0}, { 65, -65, 0},
	{  0, -80, 0}, {-65, -65, 0}, {-80,   0, 0}, {-65,  65, 0},
	// Ring 2: inner ring at z=280 (top)
	{  0,  80, 280}, { 65,  65, 280}, { 80,   0, 280}, { 65, -65, 280},
	{  0, -80, 280}, {-65, -65, 280}, {-80,   0, 280}, {-65,  65, 280}
};
const int kTeleSurf[16][8] = {
	// Bottom 8 panels (outer mid to inner bottom)
	{kColorTeleDoor, 4, 0, 1, 9, 8, 0, 0},
	{kColorTele,     4, 1, 2, 10, 9, 0, 0}, {kColorTele, 4, 2, 3, 11, 10, 0, 0},
	{kColorTele,     4, 3, 4, 12, 11, 0, 0}, {kColorTele, 4, 4, 5, 13, 12, 0, 0},
	{kColorTele,     4, 5, 6, 14, 13, 0, 0}, {kColorTele, 4, 6, 7, 15, 14, 0, 0},
	{kColorTele,     4, 7, 0, 8, 15, 0, 0},
	// Top 8 panels (outer mid to inner top)
	{kColorSilver,   4, 1, 0, 16, 17, 0, 0},
	{kColorTele,     4, 2, 1, 17, 18, 0, 0}, {kColorTele, 4, 3, 2, 18, 19, 0, 0},
	{kColorTele,     4, 4, 3, 19, 20, 0, 0}, {kColorTele, 4, 5, 4, 20, 21, 0, 0},
	{kColorTele,     4, 6, 5, 21, 22, 0, 0}, {kColorTele, 4, 7, 6, 22, 23, 0, 0},
	{kColorTele,     4, 0, 7, 23, 16, 0, 0}
};
const Colony::ColonyEngine::PrismPartDef kTelePart = {24, kTelePts, 16, kTeleSurf};

// Projector: body + stand + lens (sits on table)
const int kProjectorPts[8][3] = {
	{-30, 30, 140}, {30, 30, 140}, {30, -30, 140}, {-30, -30, 140},
	{-20, 30, 160}, {30, 30, 160}, {30, -30, 160}, {-20, -30, 160}
};
const int kProjectorSurf[5][8] = {
	{kColorProjector, 4, 0, 3, 7, 4, 0, 0}, {kColorProjector, 4, 3, 2, 6, 7, 0, 0},
	{kColorProjector, 4, 1, 0, 4, 5, 0, 0}, {kColorProjector, 4, 2, 1, 5, 6, 0, 0},
	{kColorProjector, 4, 7, 6, 5, 4, 0, 0}
};
const int kPStandPts[4][3] = {
	{-25, 50, 100}, {0, 10, 140}, {0, -10, 140}, {-25, -50, 100}
};
const int kPStandSurf[1][8] = {{kColorPStand, 4, 0, 1, 2, 3, 0, 0}};
const int kPLensPts[12][3] = {
	{30,  8, 154}, {30,  0, 158}, {30, -8, 154}, {30, -8, 146}, {30,  0, 142}, {30,  8, 146},
	{55, 10, 155}, {55,  0, 160}, {55,-10, 155}, {55,-10, 145}, {55,  0, 140}, {55, 10, 145}
};
const int kPLensSurf[7][8] = {
	{kColorPLens, 4, 0, 1, 7, 6, 0, 0}, {kColorPLens, 4, 1, 2, 8, 7, 0, 0},
	{kColorPLens, 4, 2, 3, 9, 8, 0, 0}, {kColorPLens, 4, 3, 4, 10, 9, 0, 0},
	{kColorPLens, 4, 4, 5, 11, 10, 0, 0}, {kColorPLens, 4, 5, 0, 6, 11, 0, 0},
	{kColorBlack, 6, 6, 7, 8, 9, 10, 11}
};
const Colony::ColonyEngine::PrismPartDef kProjectorParts[3] = {
	{8, kProjectorPts, 5, kProjectorSurf},
	{4, kPStandPts, 1, kPStandSurf},
	{12, kPLensPts, 7, kPLensSurf}
};

// ============================================================================
// Robot geometry data (from DOS PYRAMID.H, CUBE.H, EYE.H, UPYRAMID.H,
// QUEEN.H, DRONE.H, SNOOP.H)
// ============================================================================

// --- Pyramid (type 2) ---
const int kPyramidPts[5][3] = {
	{-75, 75, 30}, {75, 75, 30}, {75, -75, 30}, {-75, -75, 30}, {0, 0, 200}
};
const int kPyramidSurf[4][8] = {
	{kColorPyramid, 3, 1, 0, 4, 1, 0, 0}, {kColorPyramid, 3, 2, 1, 4, 2, 0, 0},
	{kColorPyramid, 3, 3, 2, 4, 3, 0, 0}, {kColorPyramid, 3, 0, 3, 4, 0, 0, 0}
};
const int kPShadowPts[4][3] = {
	{-75, 75, 0}, {75, 75, 0}, {75, -75, 0}, {-75, -75, 0}
};
const int kPShadowSurf[1][8] = {{kColorBlack, 4, 3, 2, 1, 0, 3, 0}};
// Pyramid eye (ball on top)
const int kPIrisPts[4][3] = {
	{15, 0, 185}, {15, 15, 200}, {15, 0, 215}, {15, -15, 200}
};
const int kPIrisSurf[1][8] = {{kColorIris, 4, 0, 1, 2, 3, 0, 0}};
const int kPPupilPts[4][3] = {
	{16, 0, 194}, {16, 6, 200}, {16, 0, 206}, {16, -6, 200}
};
const int kPPupilSurf[1][8] = {{kColorPupil, 4, 0, 1, 2, 3, 0, 0}};

const Colony::ColonyEngine::PrismPartDef kPyramidBodyDef = {5, kPyramidPts, 4, kPyramidSurf};
const Colony::ColonyEngine::PrismPartDef kPShadowDef = {4, kPShadowPts, 1, kPShadowSurf};
const Colony::ColonyEngine::PrismPartDef kPIrisDef = {4, kPIrisPts, 1, kPIrisSurf};
const Colony::ColonyEngine::PrismPartDef kPPupilDef = {4, kPPupilPts, 1, kPPupilSurf};

// --- Cube (type 3) --- (octahedron)
const int kCubePts[6][3] = {
	{0, 0, 200}, {100, 0, 100}, {0, 100, 100}, {-100, 0, 100}, {0, -100, 100}, {0, 0, 0}
};
const int kCubeSurf[8][8] = {
	{kColorCube, 3, 0, 1, 2, 0, 0, 0}, {kColorCube, 3, 0, 2, 3, 0, 0, 0},
	{kColorCube, 3, 0, 3, 4, 0, 0, 0}, {kColorCube, 3, 0, 4, 1, 0, 0, 0},
	{kColorCube, 3, 5, 2, 1, 5, 0, 0}, {kColorCube, 3, 5, 3, 2, 5, 0, 0},
	{kColorCube, 3, 5, 4, 3, 5, 0, 0}, {kColorCube, 3, 5, 1, 4, 5, 0, 0}
};
const Colony::ColonyEngine::PrismPartDef kCubeBodyDef = {6, kCubePts, 8, kCubeSurf};

// --- UPyramid (type 4) --- (inverted pyramid)
const int kUPyramidPts[5][3] = {
	{-75, 75, 190}, {75, 75, 190}, {75, -75, 190}, {-75, -75, 190}, {0, 0, 30}
};
const int kUPyramidSurf[5][8] = {
	{kColorUPyramid, 3, 0, 1, 4, 0, 0, 0}, {kColorUPyramid, 3, 1, 2, 4, 1, 0, 0},
	{kColorUPyramid, 3, 2, 3, 4, 2, 0, 0}, {kColorUPyramid, 3, 3, 0, 4, 3, 0, 0},
	{kColorUPyramid, 4, 3, 2, 1, 0, 3, 0}
};
const int kUPShadowPts[4][3] = {
	{-25, 25, 0}, {25, 25, 0}, {25, -25, 0}, {-25, -25, 0}
};
const int kUPShadowSurf[1][8] = {{kColorBlack, 4, 3, 2, 1, 0, 3, 0}};

const Colony::ColonyEngine::PrismPartDef kUPyramidBodyDef = {5, kUPyramidPts, 5, kUPyramidSurf};
const Colony::ColonyEngine::PrismPartDef kUPShadowDef = {4, kUPShadowPts, 1, kUPShadowSurf};

// --- Eye (type 1) ---
// Ball is rendered by draw3DSphere(), not as a prism
const int kEyeIrisPts[4][3] = {
	{60, 0, 140}, {60, 60, 200}, {60, 0, 260}, {60, -60, 200}
};
const int kEyeIrisSurf[1][8] = {{kColorEyeIris, 4, 0, 1, 2, 3, 0, 0}};
const int kEyePupilPts[4][3] = {
	{66, 0, 175}, {66, 25, 200}, {66, 0, 225}, {66, -25, 200}
};
const int kEyePupilSurf[1][8] = {{kColorBlack, 4, 0, 1, 2, 3, 0, 0}};

const Colony::ColonyEngine::PrismPartDef kEyeIrisDef = {4, kEyeIrisPts, 1, kEyeIrisSurf};
const Colony::ColonyEngine::PrismPartDef kEyePupilDef = {4, kEyePupilPts, 1, kEyePupilSurf};

// --- Floating Pyramid (type 6) --- egg on ground
const int kFPyramidPts[5][3] = {
	{-75, 75, 0}, {75, 75, 0}, {75, -75, 0}, {-75, -75, 0}, {0, 0, 170}
};
const Colony::ColonyEngine::PrismPartDef kFPyramidBodyDef = {5, kFPyramidPts, 4, kPyramidSurf};

// --- Small Pyramid (type 10) ---
const int kSPyramidPts[5][3] = {
	{-40, 40, 0}, {40, 40, 0}, {40, -40, 0}, {-40, -40, 0}, {0, 0, 100}
};
const Colony::ColonyEngine::PrismPartDef kSPyramidBodyDef = {5, kSPyramidPts, 4, kPyramidSurf};

// --- Mini Pyramid (type 14) ---
const int kMPyramidPts[5][3] = {
	{-20, 20, 0}, {20, 20, 0}, {20, -20, 0}, {-20, -20, 0}, {0, 0, 50}
};
const Colony::ColonyEngine::PrismPartDef kMPyramidBodyDef = {5, kMPyramidPts, 4, kPyramidSurf};

// --- Floating Cube (type 7) ---
const int kFCubePts[6][3] = {
	{0, 0, 150}, {75, 0, 75}, {0, 75, 75}, {-75, 0, 75}, {0, -75, 75}, {0, 0, 0}
};
const Colony::ColonyEngine::PrismPartDef kFCubeBodyDef = {6, kFCubePts, 8, kCubeSurf};

// --- Small Cube (type 11) ---
const int kSCubePts[6][3] = {
	{0, 0, 100}, {50, 0, 50}, {0, 50, 50}, {-50, 0, 50}, {0, -50, 50}, {0, 0, 0}
};
const Colony::ColonyEngine::PrismPartDef kSCubeBodyDef = {6, kSCubePts, 8, kCubeSurf};

// --- Mini Cube (type 15) ---
const int kMCubePts[6][3] = {
	{0, 0, 50}, {25, 0, 25}, {0, 25, 25}, {-25, 0, 25}, {0, -25, 25}, {0, 0, 0}
};
const Colony::ColonyEngine::PrismPartDef kMCubeBodyDef = {6, kMCubePts, 8, kCubeSurf};

// --- Floating UPyramid (type 8) ---
const int kFUPyramidPts[5][3] = {
	{-75, 75, 170}, {75, 75, 170}, {75, -75, 170}, {-75, -75, 170}, {0, 0, 0}
};
const Colony::ColonyEngine::PrismPartDef kFUPyramidBodyDef = {5, kFUPyramidPts, 5, kUPyramidSurf};

// --- Small UPyramid (type 12) ---
const int kSUPyramidPts[5][3] = {
	{-40, 40, 100}, {40, 40, 100}, {40, -40, 100}, {-40, -40, 100}, {0, 0, 0}
};
const Colony::ColonyEngine::PrismPartDef kSUPyramidBodyDef = {5, kSUPyramidPts, 5, kUPyramidSurf};

// --- Mini UPyramid (type 16) ---
const int kMUPyramidPts[5][3] = {
	{-20, 20, 50}, {20, 20, 50}, {20, -20, 50}, {-20, -20, 50}, {0, 0, 0}
};
const Colony::ColonyEngine::PrismPartDef kMUPyramidBodyDef = {5, kMUPyramidPts, 5, kUPyramidSurf};

// --- Floating Eye (type 5) ---
const int kFEyeIrisPts[4][3] = {
	{60, 0, 40}, {60, 60, 100}, {60, 0, 160}, {60, -60, 100}
};
const int kFEyePupilPts[4][3] = {
	{66, 0, 75}, {66, 25, 100}, {66, 0, 125}, {66, -25, 100}
};
const Colony::ColonyEngine::PrismPartDef kFEyeIrisDef = {4, kFEyeIrisPts, 1, kEyeIrisSurf};
const Colony::ColonyEngine::PrismPartDef kFEyePupilDef = {4, kFEyePupilPts, 1, kEyePupilSurf};

// --- Small Eye (type 9) ---
const int kSEyeIrisPts[4][3] = {
	{30, 0, 20}, {30, 30, 50}, {30, 0, 80}, {30, -30, 50}
};
const int kSEyePupilPts[4][3] = {
	{33, 0, 38}, {33, 13, 50}, {33, 0, 63}, {33, -13, 50}
};
const Colony::ColonyEngine::PrismPartDef kSEyeIrisDef = {4, kSEyeIrisPts, 1, kEyeIrisSurf};
const Colony::ColonyEngine::PrismPartDef kSEyePupilDef = {4, kSEyePupilPts, 1, kEyePupilSurf};

// --- Mini Eye (type 13) ---
const int kMEyeIrisPts[4][3] = {
	{15, 0, 10}, {15, 15, 25}, {15, 0, 40}, {15, -15, 25}
};
const int kMEyePupilPts[4][3] = {
	{16, 0, 19}, {16, 6, 25}, {16, 0, 31}, {16, -6, 25}
};
const Colony::ColonyEngine::PrismPartDef kMEyeIrisDef = {4, kMEyeIrisPts, 1, kEyeIrisSurf};
const Colony::ColonyEngine::PrismPartDef kMEyePupilDef = {4, kMEyePupilPts, 1, kEyePupilSurf};

// --- Queen (type 17) ---
// Queen eye (ball rendered by draw3DSphere)
const int kQIrisPts[4][3] = {
	{15, 0, 140}, {15, 15, 155}, {15, 0, 170}, {15, -15, 155}
};
const int kQIrisSurf[1][8] = {{kColorQueenEye, 4, 0, 1, 2, 3, 0, 0}};
const int kQPupilPts[4][3] = {
	{16, 0, 148}, {16, 6, 155}, {16, 0, 161}, {16, -6, 155}
};
const int kQPupilSurf[1][8] = {{kColorPupil, 4, 0, 1, 2, 3, 0, 0}};
// Queen abdomen
const int kQAbdomenPts[9][3] = {
	{120, 0, 130}, {30, 0, 160}, {30, 0, 100}, {30, 50, 130}, {30, -50, 130},
	{0, 0, 150}, {0, 0, 110}, {0, 25, 130}, {0, -25, 130}
};
const int kQAbdomenSurf[9][8] = {
	{kColorQueenBody, 3, 0, 3, 1, 0, 0, 0}, {kColorQueenBody, 3, 0, 1, 4, 0, 0, 0},
	{kColorQueenBody, 3, 0, 2, 3, 0, 0, 0}, {kColorQueenBody, 3, 0, 4, 2, 0, 0, 0},
	{kColorQueenBody, 4, 1, 5, 8, 4, 1, 0}, {kColorQueenBody, 4, 1, 3, 7, 5, 1, 0},
	{kColorQueenBody, 4, 2, 4, 8, 6, 2, 0}, {kColorQueenBody, 4, 2, 6, 7, 3, 2, 0},
	{kColorClear, 4, 5, 7, 6, 8, 5, 0}
};
// Queen thorax
const int kQThoraxPts[9][3] = {
	{-120, 0, 130}, {-50, 0, 170}, {-50, 0, 90}, {-50, 60, 130}, {-50, -60, 130},
	{0, 0, 150}, {0, 0, 110}, {0, 25, 130}, {0, -25, 130}
};
const int kQThoraxSurf[8][8] = {
	{kColorQueenBody, 3, 0, 1, 3, 0, 0, 0}, {kColorQueenBody, 3, 0, 4, 1, 0, 0, 0},
	{kColorQueenBody, 3, 0, 3, 2, 0, 0, 0}, {kColorQueenBody, 3, 0, 2, 4, 0, 0, 0},
	{kColorQueenBody, 4, 1, 4, 8, 5, 1, 0}, {kColorQueenBody, 4, 1, 5, 7, 3, 1, 0},
	{kColorQueenBody, 4, 2, 6, 8, 4, 2, 0}, {kColorQueenBody, 4, 2, 3, 7, 6, 2, 0}
};
// Queen wings
const int kQLWingPts[4][3] = {
	{80, 0, 140}, {-40, 10, 200}, {-120, 60, 170}, {-40, 120, 140}
};
const int kQLWingSurf[1][8] = {{kColorClear, 4, 0, 1, 2, 3, 0, 0}};
const int kQRWingPts[4][3] = {
	{80, 0, 140}, {-40, -10, 200}, {-120, -60, 170}, {-40, -120, 140}
};
const int kQRWingSurf[1][8] = {{kColorClear, 4, 0, 1, 2, 3, 0, 0}};

const Colony::ColonyEngine::PrismPartDef kQIrisDef = {4, kQIrisPts, 1, kQIrisSurf};
const Colony::ColonyEngine::PrismPartDef kQPupilDef = {4, kQPupilPts, 1, kQPupilSurf};
const Colony::ColonyEngine::PrismPartDef kQAbdomenDef = {9, kQAbdomenPts, 9, kQAbdomenSurf};
const Colony::ColonyEngine::PrismPartDef kQThoraxDef = {9, kQThoraxPts, 8, kQThoraxSurf};
const Colony::ColonyEngine::PrismPartDef kQLWingDef = {4, kQLWingPts, 1, kQLWingSurf};
const Colony::ColonyEngine::PrismPartDef kQRWingDef = {4, kQRWingPts, 1, kQRWingSurf};

// --- Drone / Soldier (types 18, 19) ---
const int kDAbdomenPts[6][3] = {
	{0, 0, 170}, {120, 0, 130}, {0, 100, 130}, {-130, 0, 130}, {0, -100, 130}, {0, 0, 100}
};
const int kDAbdomenSurf[8][8] = {
	{kColorDrone, 3, 0, 1, 2, 0, 0, 0}, {kColorDrone, 3, 0, 2, 3, 0, 0, 0},
	{kColorDrone, 3, 0, 3, 4, 0, 0, 0}, {kColorDrone, 3, 0, 4, 1, 0, 0, 0},
	{kColorDrone, 3, 5, 2, 1, 5, 0, 0}, {kColorDrone, 3, 5, 3, 2, 5, 0, 0},
	{kColorDrone, 3, 5, 4, 3, 5, 0, 0}, {kColorDrone, 3, 5, 1, 4, 5, 0, 0}
};
// Drone static pincers (llPincer/rrPincer)
const int kDLLPincerPts[4][3] = {
	{0, 0, 130}, {50, -2, 130}, {35, -20, 140}, {35, -20, 120}
};
const int kDLPincerSurf[4][8] = {
	{kColorClaw1, 3, 0, 2, 1, 0, 0, 0}, {kColorClaw1, 3, 0, 1, 3, 0, 0, 0},
	{kColorClaw2, 3, 0, 3, 2, 0, 0, 0}, {kColorClaw2, 3, 1, 2, 3, 1, 0, 0}
};
const int kDRRPincerPts[4][3] = {
	{0, 0, 130}, {50, 2, 130}, {35, 20, 140}, {35, 20, 120}
};
const int kDRPincerSurf[4][8] = {
	{kColorClaw1, 3, 0, 1, 2, 0, 0, 0}, {kColorClaw1, 3, 0, 3, 1, 0, 0, 0},
	{kColorClaw2, 3, 0, 2, 3, 0, 0, 0}, {kColorClaw2, 3, 1, 3, 2, 1, 0, 0}
};
// Drone eyes
const int kDLEyePts[3][3] = {
	{60, 0, 150}, {60, 50, 130}, {60, 25, 150}
};
const int kDLEyeSurf[2][8] = {
	{kColorDroneEye, 3, 0, 1, 2, 0, 0, 0}, {kColorDroneEye, 3, 0, 2, 1, 0, 0, 0}
};
const int kDREyePts[3][3] = {
	{60, 0, 150}, {60, -50, 130}, {60, -25, 150}
};
const int kDREyeSurf[2][8] = {
	{kColorDroneEye, 3, 0, 1, 2, 0, 0, 0}, {kColorDroneEye, 3, 0, 2, 1, 0, 0, 0}
};

const Colony::ColonyEngine::PrismPartDef kDAbdomenDef = {6, kDAbdomenPts, 8, kDAbdomenSurf};
const Colony::ColonyEngine::PrismPartDef kDLLPincerDef = {4, kDLLPincerPts, 4, kDLPincerSurf};
const Colony::ColonyEngine::PrismPartDef kDRRPincerDef = {4, kDRRPincerPts, 4, kDRPincerSurf};
const Colony::ColonyEngine::PrismPartDef kDLEyeDef = {3, kDLEyePts, 2, kDLEyeSurf};
const Colony::ColonyEngine::PrismPartDef kDREyeDef = {3, kDREyePts, 2, kDREyeSurf};

// --- Snoop (type 20) ---
const int kSnoopAbdomenPts[4][3] = {
	{0, 100, 0}, {-180, 0, 0}, {0, -100, 0}, {0, 0, 70}
};
const int kSnoopAbdomenSurf[2][8] = {
	{kColorTopSnoop, 3, 0, 1, 3, 0, 0, 0}, {kColorTopSnoop, 3, 2, 3, 1, 2, 0, 0}
};
const int kSnoopHeadPts[4][3] = {
	{0, 100, 0}, {150, 0, 0}, {0, -100, 0}, {0, 0, 70}
};
const int kSnoopHeadSurf[3][8] = {
	{kColorTopSnoop, 3, 0, 3, 1, 0, 0, 0}, {kColorTopSnoop, 3, 2, 1, 3, 2, 0, 0},
	{kColorBottomSnoop, 3, 0, 1, 2, 0, 0, 0}
};

const Colony::ColonyEngine::PrismPartDef kSnoopAbdomenDef = {4, kSnoopAbdomenPts, 2, kSnoopAbdomenSurf};
const Colony::ColonyEngine::PrismPartDef kSnoopHeadDef = {4, kSnoopHeadPts, 3, kSnoopHeadSurf};

int wrapAngle256(int angle) {
	angle %= 256;
	return (angle < 0) ? (angle + 256) : angle;
}

void rotatePoint(int angle, const int src[3], int dst[3], const int *cost, const int *sint) {
	const int32 tcos = cost[angle];
	const int32 tsin = sint[angle];
	dst[0] = (int)(((int32)src[0] * tcos - (int32)src[1] * tsin) >> 7);
	dst[1] = (int)(((int32)src[0] * tsin + (int32)src[1] * tcos) >> 7);
	dst[2] = src[2];
}

void resetObjectBounds(const Common::Rect &screenR, Locate &loc) {
	loc.xmn = screenR.right;
	loc.xmx = screenR.left;
	loc.zmn = screenR.bottom;
	loc.zmx = screenR.top;
}

void ColonyEngine::drawStaticObjects() {
	for (uint i = 0; i < _objects.size(); i++) {
		Thing &obj = _objects[i];
		if (!obj.alive)
			continue;
		resetObjectBounds(_screenR, obj.where);
		int ox = obj.where.xindex;
		int oy = obj.where.yindex;
		if (ox < 0 || ox >= 32 || oy < 0 || oy >= 32 || !_visibleCell[ox][oy])
			continue;
		drawStaticObjectPrisms3D(obj);
		// MAKEROBO.C: if shootable robot straddles centerX, set insight
		// (narrows crosshair brackets to indicate a target is in the line of fire)
		int t = obj.type;
		if ((t >= kRobEye && t <= kRobUPyramid) ||
			(t >= kRobQueen && t <= kRobSoldier)) {
			if (obj.where.xmn < _centerX && obj.where.xmx > _centerX)
				_insight = true;
		}
	}
}

void ColonyEngine::drawPrismOval3D(Thing &thing, const PrismPartDef &def, bool useLook, int colorOverride, bool forceVisible) {
	if (def.pointCount < 4 || def.surfaceCount < 1)
		return;

	const uint8 ang = (useLook ? thing.where.look : thing.where.ang) + 32;
	const int32 rotCos = _cost[ang];
	const int32 rotSin = _sint[ang];
	const bool lit = (_corePower[_coreIndex] > 0);
	float transformedX[32];
	float transformedY[32];
	float transformedZ[32];
	int projectedX[32];
	int projectedY[32];
	bool projected[32];

	assert(def.pointCount <= ARRAYSIZE(projectedX));

	for (int i = 0; i < def.pointCount; ++i) {
		const int ox = def.points[i][0];
		const int oy = def.points[i][1];
		const int oz = def.points[i][2];
		const int32 rx = ((int32)ox * rotCos - (int32)oy * rotSin) >> 7;
		const int32 ry = ((int32)ox * rotSin + (int32)oy * rotCos) >> 7;
		transformedX[i] = (float)(rx + thing.where.xloc);
		transformedY[i] = (float)(ry + thing.where.yloc);
		transformedZ[i] = (float)(oz - 160);
		projected[i] = projectCorridorPointRaw(_screenR, _me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc,
			transformedX[i], transformedY[i], transformedZ[i],
			projectedX[i], projectedY[i]);
	}

	const int *surface = &def.surfaces[0][2];
	const int pointCount = def.surfaces[0][1];
	if (pointCount < 4)
		return;
	for (int i = 0; i < pointCount; ++i) {
		const int pointIdx = surface[i];
		if (pointIdx < 0 || pointIdx >= def.pointCount || !projected[pointIdx])
			return;
	}

	if (!forceVisible && !isProjectedSurfaceVisible(surface, pointCount, projectedX, projectedY))
		return;

	const int left = projectedX[3];
	const int right = projectedX[1];
	const int top = projectedY[2];
	const int bottom = projectedY[0];
	if (right <= left || bottom <= top)
		return;

	const int fillColorIdx = (colorOverride >= 0) ? colorOverride : def.surfaces[0][0];

	const float centerX = (transformedX[0] + transformedX[1] + transformedX[2] + transformedX[3]) * 0.25f;
	const float centerY = (transformedY[0] + transformedY[1] + transformedY[2] + transformedY[3]) * 0.25f;
	const float centerZ = (transformedZ[0] + transformedZ[1] + transformedZ[2] + transformedZ[3]) * 0.25f;
	const float axisHX = (transformedX[1] - transformedX[3]) * 0.5f;
	const float axisHY = (transformedY[1] - transformedY[3]) * 0.5f;
	const float axisHZ = (transformedZ[1] - transformedZ[3]) * 0.5f;
	const float axisVX = (transformedX[2] - transformedX[0]) * 0.5f;
	const float axisVY = (transformedY[2] - transformedY[0]) * 0.5f;
	const float axisVZ = (transformedZ[2] - transformedZ[0]) * 0.5f;

	const int kSegments = 20;
	float px[kSegments];
	float py[kSegments];
	float pz[kSegments];
	for (int i = 0; i < kSegments; ++i) {
		const float angleRad = 2.0f * (float)M_PI * (float)i / (float)kSegments;
		const float ca = cosf(angleRad);
		const float sa = sinf(angleRad);
		px[i] = centerX + ca * axisHX + sa * axisVX;
		py[i] = centerY + ca * axisHY + sa * axisVY;
		pz[i] = centerZ + ca * axisHZ + sa * axisVZ;
	}

	if (_renderMode == Common::kRenderMacintosh && _hasMacColors) {
		const int macColorIdx = mapEyeOverlayColorToMacColor(fillColorIdx, _level);
		int pattern = _macColors[macColorIdx].pattern;
		uint32 fg = packEyeOverlayMacColor(_macColors[macColorIdx].fg);
		uint32 bg = packEyeOverlayMacColor(_macColors[macColorIdx].bg);
		uint32 line = 0xFF000000;
		if (!lit) {
			fg = 0xFF000000;
			bg = 0xFF000000;
			line = 0xFF000000;
			pattern = kPatternBlack;
		}
		const byte *stipple = setupMacPattern(_gfx, pattern, fg, bg);
		_gfx->draw3DPolygon(px, py, pz, kSegments, line);
		if (stipple)
			_gfx->setStippleData(nullptr);
	} else if (lit) {
		if (_renderMode == Common::kRenderMacintosh) {
			int pattern = mapEyeOverlayColorToMacPattern(fillColorIdx);
			if (pattern == kPatternClear)
				return;
			if (!_wireframe)
				_gfx->setWireframe(true, pattern == kPatternBlack ? 0 : 255);
			_gfx->setStippleData(kMacStippleData[pattern]);
			_gfx->draw3DPolygon(px, py, pz, kSegments, 0);
			_gfx->setStippleData(nullptr);
		} else {
			if (!_wireframe)
				_gfx->setWireframe(true, mapEyeOverlayColorToDOSFill(fillColorIdx, _level));
			_gfx->draw3DPolygon(px, py, pz, kSegments, 0);
		}
	} else {
		if (!_wireframe)
			_gfx->setWireframe(true, 0);
		_gfx->draw3DPolygon(px, py, pz, kSegments, 15);
	}

	thing.where.xmn = MIN(thing.where.xmn, left);
	thing.where.xmx = MAX(thing.where.xmx, right);
	thing.where.zmn = MIN(thing.where.zmn, top);
	thing.where.zmx = MAX(thing.where.zmx, bottom);
}

void ColonyEngine::drawEyeOverlays3D(Thing &thing, const PrismPartDef &irisDef, int irisColorOverride,
		const PrismPartDef &pupilDef, int pupilColorOverride, bool useLook) {
	if (!isProjectedPrismSurfaceVisible(_screenR, thing, irisDef, useLook, 0,
			_me.look, _me.lookY, _me.xloc, _me.yloc, _sint, _cost)) {
		return;
	}

	// Original Mac eye rendering uses the iris quad as the single visibility
	// gate, then draws both iris and pupil unconditionally.
	drawPrismOval3D(thing, irisDef, useLook, irisColorOverride, true);
	drawPrismOval3D(thing, pupilDef, useLook, pupilColorOverride, true);
}

bool ColonyEngine::drawStaticObjectPrisms3D(Thing &obj) {
	const int eyeballColor = (_level == 1 || _level == 7) ? kColorPupil : kColorEyeball;
	const int pupilColor = (_level == 1 || _level == 7) ? kColorEyeball : kColorPupil;

	switch (obj.type) {
	case kObjConsole:
		draw3DPrism(obj, kConsolePart, false, -1, true, false);
		break;
	case kObjCChair:
		for (int i = 0; i < 5; i++) {
			_gfx->setDepthRange((4 - i) * 0.002, 1.0);
			// Flat quad parts (seat, arms, back) need forceVisible to avoid
			// edge-on culling; box base (i==4) uses normal culling.
			draw3DPrism(obj, kCChairParts[i], false, -1, true, i < 4);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kObjPlant:
		// DOS MakePlant draw order: top pot, then green leaf lines, then pot on top.
		draw3DPrism(obj, kPlantParts[1], false, -1, true, false); // top pot (soil)
		for (int i = 2; i < 8; i++)
			draw3DLeaf(obj, kPlantParts[i]); // leaves as lines
		draw3DPrism(obj, kPlantParts[0], false, -1, true, false); // pot (drawn last, on top)
		break;
	case kObjCouch:
	case kObjChair: {
		const PrismPartDef *parts = (obj.type == kObjCouch) ? kCouchParts : kChairParts;
		for (int i = 0; i < 4; i++) {
			_gfx->setDepthRange((3 - i) * 0.002, 1.0);
			draw3DPrism(obj, parts[i], false, -1, true, false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	}
	case kObjTV:
		_gfx->setDepthRange(0.002, 1.0); // body base layer (pushed back)
		draw3DPrism(obj, kTVParts[0], false, -1, true, false);
		_gfx->setDepthRange(0.0, 1.0);   // screen on top of body face
		draw3DPrism(obj, kTVParts[1], false, -1, true, false);
		break;
	case kObjDrawer:
		for (int i = 0; i < 2; i++) {
			_gfx->setDepthRange((1 - i) * 0.002, 1.0);
			draw3DPrism(obj, kDrawerParts[i], false, -1, true, false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kObjFWall:
		if (_renderMode == Common::kRenderMacintosh)
			draw3DPrism(obj, kFWallPart, false, kColorCorridorWall, true, true);
		else
			draw3DPrism(obj, kFWallPart, false, -1, true, true);
		break;
	case kObjCWall:
		if (_renderMode == Common::kRenderMacintosh)
			draw3DPrism(obj, kCWallPart, false, kColorCorridorWall, true, true);
		else
			draw3DPrism(obj, kCWallPart, false, -1, true, true);
		break;
	case kObjScreen:
		draw3DPrism(obj, kScreenPart, false, -1, true, false);
		break;
	case kObjTable:
		for (int i = 0; i < 2; i++) {
			_gfx->setDepthRange((1 - i) * 0.002, 1.0);
			draw3DPrism(obj, kTableParts[i], false, -1, true, false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kObjBed:
	case kObjBBed: {
		const PrismPartDef *parts = (obj.type == kObjBBed) ? kBBedParts : kBedParts;
		for (int i = 0; i < 3; i++) {
			_gfx->setDepthRange((2 - i) * 0.002, 1.0);
			draw3DPrism(obj, parts[i], false, -1, true, false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	}
	case kObjDesk:
		for (int i = 0; i < 10; i++) {
			_gfx->setDepthRange((9 - i) * 0.002, 1.0);
			draw3DPrism(obj, kDeskParts[i], false, -1, true, false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kObjBox1:
		draw3DPrism(obj, kBox1Part, false, -1, true, false);
		break;
	case kObjBench:
		draw3DPrism(obj, kBenchPart, false, -1, true, false);
		break;
	case kObjCBench:
		for (int i = 0; i < 2; i++) {
			_gfx->setDepthRange((1 - i) * 0.002, 1.0);
			draw3DPrism(obj, kCBenchParts[i], false, -1, true, false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kObjBox2:
		_gfx->setDepthRange(0.002, 1.0);
		draw3DPrism(obj, kBox2Parts[1], false, -1, true, false); // base first
		_gfx->setDepthRange(0.0, 1.0);
		draw3DPrism(obj, kBox2Parts[0], false, -1, true, false); // top second
		break;
	case kObjReactor: {
		// MakeReactor: animate core height and recolor only the original
		// side faces. The reactor body stays c_reactor and the core cap stays c_ccore.
		switch (_coreState[_coreIndex]) {
		case 0:
			if (_coreHeight[_coreIndex] < 256)
				_coreHeight[_coreIndex] += 16;
			break;
		case 1:
			if (_coreHeight[_coreIndex] > 0)
				_coreHeight[_coreIndex] -= 16;
			break;
		case 2:
			_coreHeight[_coreIndex] = 0;
			break;
		}
		int height = _coreHeight[_coreIndex];

		// Create modified point arrays for height animation.
		// Core bottom hex (pts 6-11) slides up with height.
		// When height=256 (closed), bottom matches top → core invisible.
		// When height=0 (open), bottom at Z=32 → full core visible.
		int modCorePts[12][3];
		memcpy(modCorePts, kReactorCorePts, sizeof(modCorePts));
		for (int i = 6; i < 12; i++)
			modCorePts[i][2] = height + 32;

		// Ring slides vertically with height.
		// When height=256, ring at Z=256..288 (encasing core top).
		// When height=0, ring at Z=0..32 (at base, core exposed).
		int modRingPts[8][3];
		memcpy(modRingPts, kReactorBasePts, sizeof(modRingPts));
		for (int i = 0; i < 4; i++)
			modRingPts[i][2] = height;
		for (int i = 4; i < 8; i++)
			modRingPts[i][2] = height + 32;

		int modCoreSurf[7][8];
		int modRingSurf[6][8];
		int modTopSurf[6][8];
		memcpy(modCoreSurf, kReactorCoreSurf, sizeof(modCoreSurf));
		memcpy(modRingSurf, kReactorBaseSurf, sizeof(modRingSurf));
		memcpy(modTopSurf, kReactorBaseSurf, sizeof(modTopSurf));

		// Mac MakeReactor(): first 4 ring/top faces cycle through c_color0..c_color3.
		const int kRingColors[] = {kColorRainbow1, kColorRainbow2, kColorRainbow3, kColorRainbow4};
		const int ringColor = kRingColors[_displayCount % 4];
		for (int i = 0; i < 4; ++i) {
			modRingSurf[i][0] = ringColor;
			modTopSurf[i][0] = ringColor;
		}

		// Only the 6 core side faces animate. The top face remains c_ccore.
		const int kCoreCycle[] = {kColorHCore1, kColorHCore2, kColorHCore3, kColorHCore4, kColorHCore3, kColorHCore2};
		int coreColor;
		if (_corePower[_coreIndex] > 1)
			coreColor = kCoreCycle[_displayCount % 6];
		else
			coreColor = kColorCCore;
		for (int i = 0; i < 6; ++i)
			modCoreSurf[i][0] = coreColor;

		PrismPartDef modCoreDef = {12, modCorePts, 7, modCoreSurf};
		PrismPartDef modRingDef = {8, modRingPts, 6, modRingSurf};
		PrismPartDef modTopDef = {8, kReactorTopPts, 6, modTopSurf};

		// Depth separation matches the original draw order closely, but the
		// per-face colors now follow MakeReactor() exactly.
		_gfx->setDepthRange(0.004, 1.0);
		draw3DPrism(obj, modTopDef, false, -1, true, false);
		_gfx->setDepthRange(0.002, 1.0);
		draw3DPrism(obj, modRingDef, false, -1, true, false);
		if (_coreState[_coreIndex] < 2) {
			_gfx->setDepthRange(0.0, 1.0);
			draw3DPrism(obj, modCoreDef, false, -1, true, false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	}
	case kObjPowerSuit: {
		// MakePowerSuit: part[4] (power source hexagon) surface[0] pulsates.
		// Mac: pcycle[count%6]; DOS: pcycle[count%8]
		const int kSuitCycle[] = {kColorHCore1, kColorHCore2, kColorHCore3, kColorHCore4, kColorHCore3, kColorHCore2};
		int sourceColor = kSuitCycle[_displayCount % 6];

		for (int i = 0; i < 4; i++) {
			_gfx->setDepthRange((4 - i) * 0.002, 1.0);
			draw3DPrism(obj, kPowerSuitParts[i], false, -1, true, false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		draw3DPrism(obj, kPowerSuitParts[4], false, sourceColor, true, false);
		break;
	}
	case kObjTeleport:
		draw3DPrism(obj, kTelePart, false, -1, true, false);
		break;
	case kObjCryo:
		_gfx->setDepthRange(0.002, 1.0);
		draw3DPrism(obj, kCryoParts[1], false, -1, true, false); // base first
		_gfx->setDepthRange(0.0, 1.0);
		draw3DPrism(obj, kCryoParts[0], false, -1, true, false); // top second
		break;
	case kObjProjector: {
		// Match the original Mac MakeProjector() ordering.
		// The body/lens order flips depending on whether the front lens cap
		// surface is facing the camera, because each part uses a separate
		// depth range bucket rather than true inter-part depth sorting.
		const bool lensFrontVisible = isProjectedPrismSurfaceVisible(_screenR, obj, kProjectorParts[2], false, 6,
															_me.look, _me.lookY, _me.xloc, _me.yloc, _sint, _cost);
		_gfx->setDepthRange(0.008, 1.0);
		draw3DPrism(obj, kTableParts[1], false, -1, true, false); // table base
		_gfx->setDepthRange(0.006, 1.0);
		draw3DPrism(obj, kTableParts[0], false, -1, true, false); // table top
		_gfx->setDepthRange(0.004, 1.0);
		draw3DPrism(obj, kProjectorParts[1], false, -1, true, true); // stand
		if (lensFrontVisible) {
			_gfx->setDepthRange(0.002, 1.0);
			draw3DPrism(obj, kProjectorParts[0], false, -1, true, false); // body
			_gfx->setDepthRange(0.0, 1.0);
			draw3DPrism(obj, kProjectorParts[2], false, -1, true, false); // lens
		} else {
			_gfx->setDepthRange(0.002, 1.0);
			draw3DPrism(obj, kProjectorParts[2], false, -1, true, false); // lens
			_gfx->setDepthRange(0.0, 1.0);
			draw3DPrism(obj, kProjectorParts[0], false, -1, true, false); // body
		}
		break;
	}
	case kObjTub:
		for (int i = 0; i < 2; i++) {
			_gfx->setDepthRange((1 - i) * 0.002, 1.0);
			draw3DPrism(obj, kTubParts[i], false, -1, true, false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kObjSink:
		for (int i = 0; i < 3; i++) {
			_gfx->setDepthRange((2 - i) * 0.002, 1.0);
			draw3DPrism(obj, kSinkParts[i], false, -1, true, false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kObjToilet:
		for (int i = 0; i < 4; i++) {
			_gfx->setDepthRange((3 - i) * 0.002, 1.0);
			draw3DPrism(obj, kToiletParts[i], false, -1, true, false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kObjPToilet:
		for (int i = 0; i < 5; i++) {
			_gfx->setDepthRange((4 - i) * 0.002, 1.0);
			draw3DPrism(obj, kPToiletParts[i], false, -1, true, false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kObjForkLift:
		// Draw order: forks, arms, treads, cab (back-to-front)
		_gfx->setDepthRange(0.010, 1.0);
		draw3DPrism(obj, kForkliftParts[3], false, -1, true, false); // FLLL (left fork)
		_gfx->setDepthRange(0.008, 1.0);
		draw3DPrism(obj, kForkliftParts[2], false, -1, true, false); // FLUL (left arm)
		_gfx->setDepthRange(0.006, 1.0);
		draw3DPrism(obj, kForkliftParts[5], false, -1, true, false); // FLLR (right fork)
		_gfx->setDepthRange(0.004, 1.0);
		draw3DPrism(obj, kForkliftParts[4], false, -1, true, false); // FLUR (right arm)
		_gfx->setDepthRange(0.002, 1.0);
		draw3DPrism(obj, kForkliftParts[1], false, -1, true, false); // treads
		_gfx->setDepthRange(0.0, 1.0);
		draw3DPrism(obj, kForkliftParts[0], false, -1, true, false); // cab
		break;
	// === Robot types (1-20) ===
	case kRobEye:
		if ((obj.where.xloc - _me.xloc) * (obj.where.xloc - _me.xloc) +
			(obj.where.yloc - _me.yloc) * (obj.where.yloc - _me.yloc) <= 64 * 64) {
			break;
		}
		draw3DSphere(obj, 0, 0, 100, 0, 0, 200, eyeballColor, kColorBlack, true);
		drawEyeOverlays3D(obj, kEyeIrisDef, -1, kEyePupilDef, pupilColor, false);
		break;
	case kRobPyramid:
		_gfx->setDepthRange(0.030, 1.0);
		draw3DPrism(obj, kPShadowDef, false, -1, true, false);
		_gfx->setDepthRange(0.020, 1.0);
		draw3DPrism(obj, kPyramidBodyDef, false, -1, true, false);
		_gfx->setDepthRange(0.004, 1.0);
		_gfx->setDepthState(true, false);
		_gfx->setDepthRange(0.0, 1.0);
		draw3DSphere(obj, 0, 0, 175, 0, 0, 200, eyeballColor, kColorBlack, true);
		drawEyeOverlays3D(obj, kPIrisDef, -1, kPPupilDef, pupilColor, false);
		_gfx->setDepthState(true, true);
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kRobCube:
		// DOS CUBE.C: body + draweye() (same shared eye as Pyramid/UPyramid)
		draw3DPrism(obj, kCubeBodyDef, false, -1, true, false);
		_gfx->setDepthState(true, false);
		draw3DSphere(obj, 0, 0, 175, 0, 0, 200, eyeballColor, kColorBlack, true);
		drawEyeOverlays3D(obj, kPIrisDef, -1, kPPupilDef, pupilColor, false);
		_gfx->setDepthState(true, true);
		break;
	case kRobUPyramid:
		// DOS UPYRAMID.C: draweye() drawn first, then shadow, then body.
		// Same eye geometry as Pyramid (shared draweye function).
		_gfx->setDepthRange(0.030, 1.0);
		draw3DPrism(obj, kUPShadowDef, false, -1, true, false);
		_gfx->setDepthRange(0.020, 1.0);
		draw3DPrism(obj, kUPyramidBodyDef, false, -1, true, false);
		_gfx->setDepthRange(0.004, 1.0);
		_gfx->setDepthState(true, false);
		_gfx->setDepthRange(0.0, 1.0);
		draw3DSphere(obj, 0, 0, 175, 0, 0, 200, eyeballColor, kColorBlack, true);
		drawEyeOverlays3D(obj, kPIrisDef, -1, kPPupilDef, pupilColor, false);
		_gfx->setDepthState(true, true);
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kRobFEye:
		draw3DSphere(obj, 0, 0, 0, 0, 0, 100, eyeballColor, kColorBlack, true);
		drawEyeOverlays3D(obj, kFEyeIrisDef, -1, kFEyePupilDef, pupilColor, false);
		break;
	case kRobFPyramid:
		draw3DPrism(obj, kFPyramidBodyDef, false, -1, true, false);
		break;
	case kRobFCube:
		draw3DPrism(obj, kFCubeBodyDef, false, -1, true, false);
		break;
	case kRobFUPyramid:
		draw3DPrism(obj, kFUPyramidBodyDef, false, -1, true, false);
		break;
	case kRobSEye:
		draw3DSphere(obj, 0, 0, 0, 0, 0, 50, eyeballColor, kColorBlack, true);
		drawEyeOverlays3D(obj, kSEyeIrisDef, -1, kSEyePupilDef, pupilColor, false);
		break;
	case kRobSPyramid:
		draw3DPrism(obj, kSPyramidBodyDef, false, -1, true, false);
		break;
	case kRobSCube:
		draw3DPrism(obj, kSCubeBodyDef, false, -1, true, false);
		break;
	case kRobSUPyramid:
		draw3DPrism(obj, kSUPyramidBodyDef, false, -1, true, false);
		break;
	case kRobMEye:
		draw3DSphere(obj, 0, 0, 0, 0, 0, 25, eyeballColor, kColorBlack, true);
		drawEyeOverlays3D(obj, kMEyeIrisDef, kColorMiniEyeIris, kMEyePupilDef, pupilColor, false);
		break;
	case kRobMPyramid:
		draw3DPrism(obj, kMPyramidBodyDef, false, -1, true, false);
		break;
	case kRobMCube:
		draw3DPrism(obj, kMCubeBodyDef, false, -1, true, false);
		break;
	case kRobMUPyramid:
		draw3DPrism(obj, kMUPyramidBodyDef, false, -1, true, false);
		break;
	case kRobQueen:
		{
			const int32 s1 = _sint[obj.where.ang] >> 1;
			const int32 c1 = _cost[obj.where.ang] >> 1;
			const int32 s2 = s1 >> 1;
			const int32 c2 = c1 >> 1;
			const int32 eyeBaseX = obj.where.xloc + c1;
			const int32 eyeBaseY = obj.where.yloc + s1;

			Thing leftEye = obj;
			leftEye.where.xloc = (int)(eyeBaseX - s2);
			leftEye.where.yloc = (int)(eyeBaseY + c2);
			resetObjectBounds(_screenR, leftEye.where);

			Thing rightEye = obj;
			rightEye.where.xloc = (int)(eyeBaseX + s2);
			rightEye.where.yloc = (int)(eyeBaseY - c2);
			resetObjectBounds(_screenR, rightEye.where);

			const int32 leftDist = (leftEye.where.xloc - _me.xloc) * (leftEye.where.xloc - _me.xloc) +
				(leftEye.where.yloc - _me.yloc) * (leftEye.where.yloc - _me.yloc);
			const int32 rightDist = (rightEye.where.xloc - _me.xloc) * (rightEye.where.xloc - _me.xloc) +
				(rightEye.where.yloc - _me.yloc) * (rightEye.where.yloc - _me.yloc);
			const bool leftFirst = leftDist >= rightDist;
			Thing &farEye = leftFirst ? leftEye : rightEye;
			Thing &nearEye = leftFirst ? rightEye : leftEye;
			const PrismPartDef &farWing = leftFirst ? kQLWingDef : kQRWingDef;
			const PrismPartDef &nearWing = leftFirst ? kQRWingDef : kQLWingDef;
			const int wingColor = (_renderMode != Common::kRenderMacintosh && _level == 7) ? kColorQueenWingRed : kColorClear;

			// DOS draweyes(): queen eyeball is RED fill + WHITE outline
			// (hardcoded, not from color table). Iris is GREEN.
			const int queenEyeballColor = 5; // vRED (EGA index 4 = kColorRed... use raw 5 = cRED index)
			draw3DPrism(obj, farWing, false, wingColor, true, true);
			draw3DSphere(farEye, 0, 0, 130, 0, 0, 155, queenEyeballColor, kColorBlack, true);
			drawEyeOverlays3D(farEye, kQIrisDef, -1, kQPupilDef, pupilColor, true);
			if (farEye.where.xmn < obj.where.xmn)
				obj.where.xmn = farEye.where.xmn;
			if (farEye.where.xmx > obj.where.xmx)
				obj.where.xmx = farEye.where.xmx;
			if (farEye.where.zmn < obj.where.zmn)
				obj.where.zmn = farEye.where.zmn;
			if (farEye.where.zmx > obj.where.zmx)
				obj.where.zmx = farEye.where.zmx;

			draw3DPrism(obj, kQThoraxDef, false, -1, true, false);
			draw3DPrism(obj, kQAbdomenDef, false, -1, true, false);

			draw3DPrism(obj, nearWing, false, wingColor, true, true);
			draw3DSphere(nearEye, 0, 0, 130, 0, 0, 155, queenEyeballColor, kColorBlack, true);
			drawEyeOverlays3D(nearEye, kQIrisDef, -1, kQPupilDef, pupilColor, true);
			if (nearEye.where.xmn < obj.where.xmn)
				obj.where.xmn = nearEye.where.xmn;
			if (nearEye.where.xmx > obj.where.xmx)
				obj.where.xmx = nearEye.where.xmx;
			if (nearEye.where.zmn < obj.where.zmn)
				obj.where.zmn = nearEye.where.zmn;
			if (nearEye.where.zmx > obj.where.zmx)
				obj.where.zmx = nearEye.where.zmx;
		}
		break;
	case kRobDrone:
		{
			// DOS DRONE.C: body + seteyes()/draweyes() — same two-eye
			// system as Queen, positioned at offsets from body center.
			const int32 s1 = _sint[obj.where.ang] >> 1;
			const int32 c1 = _cost[obj.where.ang] >> 1;
			const int32 s2 = s1 >> 1;
			const int32 c2 = c1 >> 1;
			const int32 eyeBaseX = obj.where.xloc + c1;
			const int32 eyeBaseY = obj.where.yloc + s1;

			Thing leftEye = obj;
			leftEye.where.xloc = (int)(eyeBaseX - s2);
			leftEye.where.yloc = (int)(eyeBaseY + c2);
			resetObjectBounds(_screenR, leftEye.where);
			Thing rightEye = obj;
			rightEye.where.xloc = (int)(eyeBaseX + s2);
			rightEye.where.yloc = (int)(eyeBaseY - c2);
			resetObjectBounds(_screenR, rightEye.where);

			// DOS draweyes(): shared with Queen — RED eyeball, GREEN iris
			draw3DPrism(obj, kDAbdomenDef, false, -1, true, false);
			draw3DPrism(obj, kDLLPincerDef, false, -1, true, false);
			draw3DPrism(obj, kDRRPincerDef, false, -1, true, false);
			draw3DPrism(obj, kDLEyeDef, false, -1, true, false);
			draw3DPrism(obj, kDREyeDef, false, -1, true, false);
			draw3DSphere(leftEye, 0, 0, 130, 0, 0, 155, 5, kColorBlack, true);
			drawEyeOverlays3D(leftEye, kQIrisDef, -1, kQPupilDef, pupilColor, true);
			draw3DSphere(rightEye, 0, 0, 130, 0, 0, 155, 5, kColorBlack, true);
			drawEyeOverlays3D(rightEye, kQIrisDef, -1, kQPupilDef, pupilColor, true);
		}
		break;
	case kRobSoldier:
		{
			// DOS DRONE.C MakeSoldier(): body + animated pincers +
			// seteyes()/draweyes() — same two-eye system as Queen.
			int leftPincerPts[4][3];
			int rightPincerPts[4][3];
			const int lookAmount = (obj.where.lookx < 0) ? -obj.where.lookx : obj.where.lookx;
			const int leftLook = wrapAngle256(-lookAmount - 32);
			const int rightLook = wrapAngle256(lookAmount - 32);

			for (int i = 0; i < 4; ++i) {
				rotatePoint(leftLook, kDLLPincerPts[i], leftPincerPts[i], _cost, _sint);
				leftPincerPts[i][0] += 120;
				rotatePoint(rightLook, kDRRPincerPts[i], rightPincerPts[i], _cost, _sint);
				rightPincerPts[i][0] += 120;
			}

			const PrismPartDef leftPincerDef = {4, leftPincerPts, 4, kDLPincerSurf};
			const PrismPartDef rightPincerDef = {4, rightPincerPts, 4, kDRPincerSurf};

			const int32 s1 = _sint[obj.where.ang] >> 1;
			const int32 c1 = _cost[obj.where.ang] >> 1;
			const int32 s2 = s1 >> 1;
			const int32 c2 = c1 >> 1;
			const int32 eyeBaseX = obj.where.xloc + c1;
			const int32 eyeBaseY = obj.where.yloc + s1;

			Thing leftEye = obj;
			leftEye.where.xloc = (int)(eyeBaseX - s2);
			leftEye.where.yloc = (int)(eyeBaseY + c2);
			resetObjectBounds(_screenR, leftEye.where);
			Thing rightEye = obj;
			rightEye.where.xloc = (int)(eyeBaseX + s2);
			rightEye.where.yloc = (int)(eyeBaseY - c2);
			resetObjectBounds(_screenR, rightEye.where);

			draw3DPrism(obj, kDAbdomenDef, false, kColorSoldierBody, true, false);
			draw3DPrism(obj, leftPincerDef, false, -1, true, false);
			draw3DPrism(obj, rightPincerDef, false, -1, true, false);
			// DOS draweyes(): shared with Queen — RED eyeball, GREEN iris
			draw3DPrism(obj, kDLEyeDef, false, kColorSoldierEye, true, false);
			draw3DPrism(obj, kDREyeDef, false, kColorSoldierEye, true, false);
			draw3DSphere(leftEye, 0, 0, 130, 0, 0, 155, 5, kColorBlack, true);
			drawEyeOverlays3D(leftEye, kQIrisDef, -1, kQPupilDef, pupilColor, true);
			draw3DSphere(rightEye, 0, 0, 130, 0, 0, 155, 5, kColorBlack, true);
			drawEyeOverlays3D(rightEye, kQIrisDef, -1, kQPupilDef, pupilColor, true);
		}
		break;
	case kRobSnoop:
		draw3DPrism(obj, kSnoopAbdomenDef, false, -1, true, false);
		draw3DPrism(obj, kSnoopHeadDef, false, -1, true, false);
		break;
	default:
		return false;
	}
	return true;
}

} // End of namespace Colony
