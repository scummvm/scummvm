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

// battle.cpp: Outdoor planet surface battle system.
// Reimplements battle.c from the original Mac source as proper OpenGL 3D.
// Original used software 2D projection + wireframe; this uses the existing
// OpenGL renderer with filled polygons and depth-tested 3D.

#include "common/debug.h"
#include "common/system.h"

#include "colony/colony.h"
#include "colony/renderer.h"
#include "colony/sound.h"

namespace Colony {

// =====================================================================
// Constants
// =====================================================================
static const int kBattleSize = 150;   // BSIZE: collision/spawn radius
static const int kMaxQuad = 15;       // pyramids per quadrant
static const int kTankMax = 24;       // turret pincer animation range
static const int kFloor = 160;        // ground z-offset
static const float kBattleFovY = 75.0f;

// =====================================================================
// Battle color constants (original Mac QuickDraw pattern indices)
// Values 0-5 are QuickDraw patterns; >=6 are ObjColor enum values.
// =====================================================================
static const int kBLtGray = 1;
static const int kBBlack  = 4;

// Map battle surface color index to a packed ARGB color.
static uint32 battleColor(int colorIdx) {
	switch (colorIdx) {
	case 0:  return 0xFFFFFFFF; // WHITE
	case 1:  return 0xFFC0C0C0; // LTGRAY
	case 2:  return 0xFF808080; // GRAY
	case 3:  return 0xFF505050; // DKGRAY
	case 4:  return 0xFF000000; // BLACK
	case 5:  return 0x00000000; // CLEAR (transparent)
	// ObjColor values for drone parts
	case kColorDrone:  return 0xFF8B6914; // brownish yellow
	case kColorClaw1:  return 0xFF707070; // claw gray 1
	case kColorClaw2:  return 0xFF505050; // claw gray 2
	case kColorEyes:   return 0xFFCC0000; // red eyes
	default: return 0xFFC0C0C0; // fallback light gray
	}
}

static int battleHorizonY(const Common::Rect &screenR, int lookY) {
	const float halfHeight = screenR.height() * 0.5f;
	const float centerY = screenR.top + halfHeight;
	const float clampedLookY = CLIP<float>((float)lookY, -63.5f, 63.5f);
	const float pitchRad = clampedLookY * 2.0f * (float)M_PI / 256.0f;
	const float focalY = halfHeight / tanf(kBattleFovY * (float)M_PI / 360.0f);

	return (int)roundf(centerY - focalY * tanf(pitchRad));
}

static int battlePowerLevel(int32 power) {
	int level = 0;
	while (power > 0) {
		power >>= 1;
		level++;
	}
	return level;
}

static int battleNormalizeCoord(int coord) {
	return (int16)coord;
}

static int wrapBattleCoord(int coord) {
	coord = battleNormalizeCoord(coord);
	if (coord < 0)
		coord += 0x8000;
	return coord;
}

static bool battleProjectPoint(const Common::Rect &screenR, uint8 look, int8 lookY, const int *sint,
	const int *cost, int camX, int camY,
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

	const float focal = (screenR.height() * 0.5f) / tanf(kBattleFovY * (float)M_PI / 360.0f);
	const float centerX = screenR.left + screenR.width() * 0.5f;
	const float centerY = screenR.top + screenR.height() * 0.5f;

	screenX = (int)roundf(centerX + (eyeX * focal / -eyeZ));
	screenY = (int)roundf(centerY - (eyeY * focal / -eyeZ));
	return true;
}

static void battleResetBounds(const Common::Rect &screenR, Locate &loc) {
	loc.xmn = screenR.right;
	loc.xmx = screenR.left;
	loc.zmn = screenR.bottom;
	loc.zmx = screenR.top;
}

static bool battleAccumulateBounds(const Common::Rect &screenR, const ColonyEngine::PrismPartDef &def,
	Locate &loc, int worldX, int worldY, uint8 ang, int zShift,
	uint8 look, int8 lookY, const int *sint, const int *cost,
	int camX, int camY) {
	const uint8 rotAng = ang + 32;
	const long rotCos = cost[rotAng];
	const long rotSin = sint[rotAng];
	bool hasPoint = false;

	for (int i = 0; i < def.pointCount; i++) {
		const int ox = def.points[i][0];
		const int oy = def.points[i][1];
		const int oz = def.points[i][2];

		const long rx = ((long)ox * rotCos - (long)oy * rotSin) >> 7;
		const long ry = ((long)ox * rotSin + (long)oy * rotCos) >> 7;

		int sx = 0;
		int sy = 0;
		if (!battleProjectPoint(screenR, look, lookY, sint, cost, camX, camY,
			(float)(rx + worldX), (float)(ry + worldY), (float)(oz + zShift), sx, sy))
			continue;

		loc.xmn = MIN(loc.xmn, sx);
		loc.xmx = MAX(loc.xmx, sx);
		loc.zmn = MIN(loc.zmn, sy);
		loc.zmx = MAX(loc.zmx, sy);
		hasPoint = true;
	}

	return hasPoint;
}

// =====================================================================
// 3D Model Data - Rock (pyramid obstacle)
// Original: base at z=0, peak at z=200. Floor subtracted → base -160, peak 40.
// We store raw vertices (pre-Floor); draw3DBattlePrism applies zShift=-160.
// =====================================================================
static const int kRockPts[5][3] = {
	{-75,  75, 0},
	{ 75,  75, 0},
	{ 75, -75, 0},
	{-75, -75, 0},
	{  0,   0, 200}
};
static const int kRockSurf[4][8] = {
	{kBLtGray, 3, 1, 0, 4, 0, 0, 0},
	{kBLtGray, 3, 2, 1, 4, 0, 0, 0},
	{kBLtGray, 3, 3, 2, 4, 0, 0, 0},
	{kBLtGray, 3, 0, 3, 4, 0, 0, 0}
};
static const ColonyEngine::PrismPartDef kRockDef = {5, kRockPts, 4, kRockSurf};

// =====================================================================
// 3D Model Data - Entrance (airlock structure)
// =====================================================================
static const int kEntPts[8][3] = {
	{-300,  300,   0},
	{ 300,  300,   0},
	{ 300, -300,   0},
	{-300, -300,   0},
	{-250,  250, 600},
	{ 250,  250, 600},
	{ 250, -250, 600},
	{-250, -250, 600}
};
static const int kEntSurf[4][8] = {
	{kBLtGray, 4, 1, 0, 4, 5, 0, 0},
	{kBLtGray, 4, 0, 3, 7, 4, 0, 0},
	{kBLtGray, 4, 3, 2, 6, 7, 0, 0},
	{kBLtGray, 4, 2, 1, 5, 6, 0, 0}
};
static const ColonyEngine::PrismPartDef kEntDef = {8, kEntPts, 4, kEntSurf};

static const int kEntDoorPts[4][3] = {
	{-60,  300,   0},
	{ 60,  300,   0},
	{ 50,  275, 200},
	{-50,  275, 200}
};
static const int kEntDoorSurf[1][8] = {
	{kBLtGray, 4, 0, 3, 2, 1, 0, 0}
};
static const ColonyEngine::PrismPartDef kEntDoorDef = {4, kEntDoorPts, 1, kEntDoorSurf};

// =====================================================================
// 3D Model Data - Shuttle (spaceship)
// =====================================================================
// Body (fuselage)
static const int kSBodyPts[12][3] = {
	{ 500,  250,   0}, { 500,  350, 200}, { 500,  150, 400},
	{ 500, -150, 400}, { 500, -350, 200}, { 500, -250,   0},
	{-500,  250,   0}, {-500,  350, 200}, {-500,  150, 400},
	{-500, -150, 400}, {-500, -350, 200}, {-500, -250,   0}
};
static const int kSBodySurf[4][8] = {
	{kBLtGray, 4, 0, 6, 7, 1, 0, 0},
	{kBLtGray, 4, 1, 7, 8, 2, 0, 0},
	{kBLtGray, 4, 3, 9, 10, 4, 0, 0},
	{kBLtGray, 4, 4, 10, 11, 5, 0, 0}
};
static const ColonyEngine::PrismPartDef kSBodyDef = {12, kSBodyPts, 4, kSBodySurf};

// Front (nose cone)
static const int kSFrontPts[7][3] = {
	{ 500,  250,   0}, { 500,  350, 200}, { 500,  150, 400},
	{ 500, -150, 400}, { 500, -350, 200}, { 500, -250,   0},
	{ 900,    0,  50}
};
static const int kSFrontSurf[6][8] = {
	{kBLtGray, 3, 0, 1, 6, 0, 0, 0},
	{kBLtGray, 3, 1, 2, 6, 0, 0, 0},
	{kBLtGray, 3, 2, 3, 6, 0, 0, 0},
	{kBLtGray, 3, 3, 4, 6, 0, 0, 0},
	{kBLtGray, 3, 4, 5, 6, 0, 0, 0},
	{kBLtGray, 3, 5, 0, 6, 0, 0, 0}
};
static const ColonyEngine::PrismPartDef kSFrontDef = {7, kSFrontPts, 6, kSFrontSurf};

// Back (engine section)
static const int kSBackPts[7][3] = {
	{-500,  250,   0}, {-500,  350, 200}, {-500,  150, 400},
	{-500, -150, 400}, {-500, -350, 200}, {-500, -250,   0},
	{-900,    0, 400}
};
static const int kSBackSurf[5][8] = {
	{kBLtGray, 3, 0, 6, 1, 0, 0, 0},
	{kBLtGray, 3, 1, 6, 2, 0, 0, 0},
	{kBLtGray, 3, 3, 6, 4, 0, 0, 0},
	{kBLtGray, 3, 4, 6, 5, 0, 0, 0},
	{kBLtGray, 3, 5, 6, 0, 0, 0, 0}
};
static const ColonyEngine::PrismPartDef kSBackDef = {7, kSBackPts, 5, kSBackSurf};

// Top fin
static const int kFTopPts[4][3] = {
	{ -500, 0, 400}, {-900, 0, 400}, {-1000, 0, 800}, {-700, 0, 800}
};
static const int kFTopSurf[1][8] = {
	{kBLtGray, 4, 0, 1, 2, 3, 0, 0}
};
static const ColonyEngine::PrismPartDef kFTopDef = {4, kFTopPts, 1, kFTopSurf};

// Left fin
static const int kFLeftPts[4][3] = {
	{-100, -350, 200}, {-700, -350, 200}, {-900, -750, 200}, {-500, -750, 200}
};
static const int kFLeftSurf[1][8] = {
	{kBLtGray, 4, 0, 1, 2, 3, 0, 0}
};
static const ColonyEngine::PrismPartDef kFLeftDef = {4, kFLeftPts, 1, kFLeftSurf};

// Right fin
static const int kFRightPts[4][3] = {
	{-100, 350, 200}, {-700, 350, 200}, {-900, 750, 200}, {-500, 750, 200}
};
static const int kFRightSurf[1][8] = {
	{kBLtGray, 4, 0, 1, 2, 3, 0, 0}
};
static const ColonyEngine::PrismPartDef kFRightDef = {4, kFRightPts, 1, kFRightSurf};

// Shuttle door
static const int kSDoorPts[4][3] = {
	{-50, 262, 25}, {50, 262, 25}, {60, 325, 150}, {-60, 325, 150}
};
static const int kSDoorSurf[1][8] = {
	{kBLtGray, 4, 0, 3, 2, 1, 0, 0}
};
static const ColonyEngine::PrismPartDef kSDoorDef = {4, kSDoorPts, 1, kSDoorSurf};

// =====================================================================
// 3D Model Data - Projectile
// =====================================================================
static const int kProjPts[5][3] = {
	{ 80,   0,  80},
	{-80,  20,  80},
	{-80,   0, 100},
	{-80, -20,  80},
	{-80,   0,  60}
};
static const int kProjSurf[5][8] = {
	{kBBlack,  4, 1, 4, 3, 2, 0, 0},
	{kBLtGray, 3, 0, 1, 2, 0, 0, 0},
	{kBLtGray, 3, 0, 2, 3, 0, 0, 0},
	{kBLtGray, 3, 0, 3, 4, 0, 0, 0},
	{kBLtGray, 3, 0, 4, 1, 0, 0, 0}
};
static const ColonyEngine::PrismPartDef kProjDef = {5, kProjPts, 5, kProjSurf};

// =====================================================================
// 3D Model Data - Drone (enemy tank)
// These vertices are NOT Floor-shifted; they float above ground.
// Drawn with zShift=0 (their z values are in absolute world coords).
// =====================================================================
// Abdomen (body)
static const int kBDroneAbdPts[6][3] = {
	{0, 0, 170}, {120, 0, 130}, {0, 100, 130},
	{-130, 0, 130}, {0, -100, 130}, {0, 0, 100}
};
static const int kBDroneAbdSurf[8][8] = {
	{kColorDrone, 3, 0, 1, 2, 0, 0, 0}, {kColorDrone, 3, 0, 2, 3, 0, 0, 0},
	{kColorDrone, 3, 0, 3, 4, 0, 0, 0}, {kColorDrone, 3, 0, 4, 1, 0, 0, 0},
	{kColorDrone, 3, 5, 2, 1, 0, 0, 0}, {kColorDrone, 3, 5, 3, 2, 0, 0, 0},
	{kColorDrone, 3, 5, 4, 3, 0, 0, 0}, {kColorDrone, 3, 5, 1, 4, 0, 0, 0}
};
static const ColonyEngine::PrismPartDef kBDroneAbdDef = {6, kBDroneAbdPts, 8, kBDroneAbdSurf};

// Left pincer base points (for rotation source)
static const int kLLPincerPts[4][3] = {
	{0, 0, 130}, {50, -2, 130}, {35, -20, 140}, {35, -20, 120}
};
// Right pincer base points (for rotation source)
static const int kRRPincerPts[4][3] = {
	{0, 0, 130}, {50, 2, 130}, {35, 20, 140}, {35, 20, 120}
};

// Left pincer surfaces
static const int kBLPincerSurf[4][8] = {
	{kColorClaw1, 3, 0, 2, 1, 0, 0, 0}, {kColorClaw1, 3, 0, 1, 3, 0, 0, 0},
	{kColorClaw2, 3, 0, 3, 2, 0, 0, 0}, {kColorClaw2, 3, 1, 2, 3, 0, 0, 0}
};
// Right pincer surfaces
static const int kBRPincerSurf[4][8] = {
	{kColorClaw1, 3, 0, 1, 2, 0, 0, 0}, {kColorClaw1, 3, 0, 3, 1, 0, 0, 0},
	{kColorClaw2, 3, 0, 2, 3, 0, 0, 0}, {kColorClaw2, 3, 1, 3, 2, 0, 0, 0}
};

// Left eye
static const int kBLEyePts[3][3] = {
	{60, 0, 150}, {60, 50, 130}, {60, 25, 150}
};
static const int kBLEyeSurf[2][8] = {
	{kColorEyes, 3, 0, 1, 2, 0, 0, 0}, {kColorEyes, 3, 0, 2, 1, 0, 0, 0}
};
static const ColonyEngine::PrismPartDef kBLEyeDef = {3, kBLEyePts, 2, kBLEyeSurf};

// Right eye
static const int kBREyePts[3][3] = {
	{60, 0, 150}, {60, -50, 130}, {60, -25, 150}
};
static const int kBREyeSurf[2][8] = {
	{kColorEyes, 3, 0, 1, 2, 0, 0, 0}, {kColorEyes, 3, 0, 2, 1, 0, 0, 0}
};
static const ColonyEngine::PrismPartDef kBREyeDef = {3, kBREyePts, 2, kBREyeSurf};

// =====================================================================
// draw3DBattlePrism: Render a PrismPartDef at a world position.
// Like draw3DPrism but applies zShift instead of hardcoded -160.
// zShift = -kFloor for ground objects (rock/entrance/shuttle/projectile).
// zShift = 0 for floating objects (drones, whose z is absolute).
// =====================================================================
void ColonyEngine::draw3DBattlePrism(const PrismPartDef &def, int worldX, int worldY, uint8 ang, int zShift) {
	// +32 compensates for sine table's 45-degree phase offset
	const uint8 rotAng = ang + 32;
	const long rotCos = _cost[rotAng];
	const long rotSin = _sint[rotAng];

	for (int i = 0; i < def.surfaceCount; i++) {
		const int colorIdx = def.surfaces[i][0];
		const int n = def.surfaces[i][1];
		if (n < 2)
			continue;

		float px[8], py[8], pz[8];
		int count = 0;

		for (int j = 0; j < n; j++) {
			const int cur = def.surfaces[i][j + 2];
			if (cur < 0 || cur >= def.pointCount)
				continue;

			int ox = def.points[cur][0];
			int oy = def.points[cur][1];
			int oz = def.points[cur][2];

			// Rotate around Z axis by object angle
			long rx = ((long)ox * rotCos - (long)oy * rotSin) >> 7;
			long ry = ((long)ox * rotSin + (long)oy * rotCos) >> 7;

			px[count] = (float)(rx + worldX);
			py[count] = (float)(ry + worldY);
			pz[count] = (float)(oz + zShift);
			count++;
		}

		if (count >= 3) {
			if (colorIdx == 5) // CLEAR: skip
				continue;

			uint32 fillColor = battleColor(colorIdx);
			uint32 outlineColor = 0xFF000000; // black outlines
			_gfx->setWireframe(true, fillColor);
			_gfx->draw3DPolygon(px, py, pz, count, outlineColor);
		}
	}
}

// =====================================================================
// battleInit: Initialize all battle data structures and 3D models.
// Called once at game start.
// =====================================================================
void ColonyEngine::battleInit() {
	// Entrance position
	_battleEnter.xloc = 16000;
	_battleEnter.yloc = 16000;
	_battleEnter.look = _battleEnter.ang = 32;

	// Shuttle position
	_battleShip.xloc = 0;
	_battleShip.yloc = 0;
	_battleShip.look = _battleShip.ang = 32;

	_battleRound = 0;
	_projon = false;
	_pcount = 0;

	// Mountain parallax
	_battledx = _width / 59;

	// Generate mountain height profile (smoothed random)
	int temp[257];
	for (int i = 0; i < 257; i++)
		temp[i] = 10 + (0x1F & _randomSource.getRandomNumber(0x7FFF));
	for (int i = 0; i < 256; i++)
		_mountains[i] = (temp[i] + temp[i + 1]) >> 1;
}

// =====================================================================
// battleSet: Spawn 16 enemies in a 4x4 grid + 15 pyramids per quadrant.
// Called when entering battle mode.
// =====================================================================
void ColonyEngine::battleSet() {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			int k = i + j * 4;
			_bfight[k].xloc = (0x2000 * i + kBattleSize) - 0x3000;
			_bfight[k].yloc = (0x2000 * j + kBattleSize) - 0x3000;
			_bfight[k].look = _bfight[k].ang = 0xFF & _randomSource.getRandomNumber(0xFF);
			_bfight[k].lookx = 0;
			_bfight[k].delta = 4;
			_bfight[k].type = kRobCube;
			_bfight[k].power[1] = 15 + (0x0F & _randomSource.getRandomNumber(0xFF));

			// Spawn pyramids in this quadrant
			for (int p = 0; p < kMaxQuad; p++) {
				int fx, fy;
				do {
					fx = 0x2000 * i + kBattleSize + (0x1FFF & _randomSource.getRandomNumber(0x7FFF));
					while (fx > 0x2000 * (i + 1) - kBattleSize)
						fx = 0x2000 * i + kBattleSize + (0x1FFF & _randomSource.getRandomNumber(0x7FFF));
					fy = 0x2000 * j + kBattleSize + (0x1FFF & _randomSource.getRandomNumber(0x7FFF));
					while (fy > 0x2000 * (j + 1) - kBattleSize)
						fy = 0x2000 * j + kBattleSize + (0x1FFF & _randomSource.getRandomNumber(0x7FFF));
				} while (
					(fx > _battleEnter.xloc - 8 * kBattleSize &&
						fx < _battleEnter.xloc + 2 * kBattleSize &&
						fy > _battleEnter.yloc - 2 * kBattleSize &&
						fy < _battleEnter.yloc + 2 * kBattleSize) ||
					(fx > _battleShip.xloc - 2 * kBattleSize &&
						fx < _battleShip.xloc + 2 * kBattleSize &&
						fy > _battleShip.yloc - 2 * kBattleSize &&
						fy < _battleShip.yloc + 2 * kBattleSize));

				_pyramids[i][j][p].xloc = fx;
				_pyramids[i][j][p].yloc = fy;
				_pyramids[i][j][p].ang = 0xFF & _randomSource.getRandomNumber(0xFF);
				_pyramids[i][j][p].type = kRobPyramid;
			}
		}
	}
}

// =====================================================================
// battleBackdrop: Draw 2D sky gradient and ground fill.
// Called before 3D rendering begins.
// =====================================================================
void ColonyEngine::battleBackdrop() {
	const int horizonY = battleHorizonY(_screenR, _me.lookY);
	const int skyBottom = CLIP<int>(horizonY, _screenR.top, _screenR.bottom);
	const int pitchOffset = horizonY - _centerY;

	// Sky gradient: dark blue at top → lighter blue toward horizon
	for (int i = 0; i < 16; i++) {
		const int bandTop = _screenR.top + ((skyBottom - _screenR.top) * i) / 16;
		const int bandBottom = _screenR.top + ((skyBottom - _screenR.top) * (i + 1)) / 16;
		if (bandBottom <= bandTop)
			continue;

		int blue = (i * 16);
		if (blue > 255)
			blue = 255;
		uint32 color = (0xFF << 24) | (0 << 16) | (0 << 8) | blue;
		Common::Rect band(_screenR.left, bandTop, _screenR.right, bandBottom);
		_gfx->fillRect(band, color);
	}

	// Ground fill (below horizon)
	uint32 groundColor = 0xFF404040;
	Common::Rect ground(_screenR.left, CLIP<int>(horizonY, _screenR.top, _screenR.bottom),
		_screenR.right, _screenR.bottom);
	if (ground.bottom > ground.top)
		_gfx->fillRect(ground, groundColor);

	// Mountain silhouette
	uint32 mtColor = 0xFF606060;
	uint8 ang = _me.look;
	int xloc = -_battledx;
	if (ang & 0x01) {
		xloc += _battledx;
		ang--;
	}

	int sunx = -1;
	bool sunon = false;

	int prevX = xloc;
	int prevY = horizonY - _mountains[ang];
	for (int i = 0; i < 63; i += 2) {
		xloc += 2 * _battledx;
		uint8 prevAng = ang;
		ang -= 2;
		// Detect sun position (when angle wraps through 0)
		if (ang > prevAng) { // unsigned wrap
			sunx = xloc - _battledx;
			sunon = true;
		}
		int curY = horizonY - _mountains[ang];
		_gfx->drawLine(prevX, prevY, xloc, curY, mtColor);
		prevX = xloc;
		prevY = curY;
	}

	// Sun
	if (sunon && sunx >= 0) {
		int ht = _screenR.height() >> 4;
		_gfx->fillEllipse(sunx, 5 + ht + pitchOffset, ht, ht, 0xFFFFFF00);
	}
}

// =====================================================================
// battleDrawPyramids: Render visible pyramid obstacles.
// Determines which 6 quadrants are visible based on player facing,
// then renders each pyramid via draw3DBattlePrism.
// =====================================================================
void ColonyEngine::battleDrawPyramids() {
	int xloc = wrapBattleCoord(_me.xloc);
	int yloc = wrapBattleCoord(_me.yloc);
	int qx = xloc >> 13;
	int qy = yloc >> 13;
	int quad = _me.look >> 6; // 0=N, 1=W, 2=S, 3=E

	// Render 6 quadrants based on facing direction
	// Each quadrant: render all pyramids within view distance
	auto renderQuadrant = [&](int qxi, int qyi) {
		int dx = 0, dy = 0;
		if (qxi < 0) { qxi = 3; dx = 0x7FFF; }
		if (qxi > 3) { qxi = 0; dx = 0x7FFF; }
		if (qyi < 0) { qyi = 3; dy = 0x7FFF; }
		if (qyi > 3) { qyi = 0; dy = 0x7FFF; }

		// Original BattleQuadrant relied on 16-bit signed wrap here.
		const int wrappedCamX = battleNormalizeCoord(xloc + dx);
		const int wrappedCamY = battleNormalizeCoord(yloc + dy);

		for (int i = 0; i < kMaxQuad; i++) {
			const int relX = battleNormalizeCoord(_pyramids[qxi][qyi][i].xloc - wrappedCamX);
			const int relY = battleNormalizeCoord(_pyramids[qxi][qyi][i].yloc - wrappedCamY);
			if (ABS(relX) + ABS(relY) >= 8000)
				continue;

			const int forward = (int)((relX * _cost[_me.look] + relY * _sint[_me.look]) >> 7);

			Locate &pyr = _pyramids[qxi][qyi][i];
			battleResetBounds(_screenR, pyr);

			const int wx = battleNormalizeCoord(_me.xloc + relX);
			const int wy = battleNormalizeCoord(_me.yloc + relY);

			battleAccumulateBounds(_screenR, kRockDef, pyr, wx, wy, pyr.ang, -kFloor,
				_me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc);
			pyr.dist = forward;
			draw3DBattlePrism(kRockDef, wx, wy, pyr.ang, -kFloor);

			if (_battleMaxP < 100) {
				_battlePwh[_battleMaxP] = &pyr;
				_battleMaxP++;
			}
		}
	};

	switch (quad) {
	case 0: // North
		renderQuadrant(qx, qy);
		renderQuadrant(qx - 1, qy);
		renderQuadrant(qx + 1, qy);
		renderQuadrant(qx, qy + 1);
		renderQuadrant(qx - 1, qy + 1);
		renderQuadrant(qx + 1, qy + 1);
		break;
	case 1: // West
		renderQuadrant(qx, qy);
		renderQuadrant(qx, qy - 1);
		renderQuadrant(qx, qy + 1);
		renderQuadrant(qx - 1, qy);
		renderQuadrant(qx - 1, qy - 1);
		renderQuadrant(qx - 1, qy + 1);
		break;
	case 2: // South
		renderQuadrant(qx, qy);
		renderQuadrant(qx - 1, qy);
		renderQuadrant(qx + 1, qy);
		renderQuadrant(qx, qy - 1);
		renderQuadrant(qx - 1, qy - 1);
		renderQuadrant(qx + 1, qy - 1);
		break;
	case 3: // East
		renderQuadrant(qx, qy);
		renderQuadrant(qx, qy - 1);
		renderQuadrant(qx, qy + 1);
		renderQuadrant(qx + 1, qy);
		renderQuadrant(qx + 1, qy - 1);
		renderQuadrant(qx + 1, qy + 1);
		break;
	}
}

// =====================================================================
// battleDrawTanks: Render enemies, entrance, shuttle, projectile.
// =====================================================================
void ColonyEngine::battleDrawTanks() {
	_insight = false;

	// --- 16 enemy drones ---
	for (int i = 0; i < 16; i++) {
		long relX = _bfight[i].xloc - _me.xloc;
		long relY = _bfight[i].yloc - _me.yloc;
		if (ABS(relX) + ABS(relY) >= 8000)
			continue;

		const int forward = (int)((relX * _cost[_me.look] + relY * _sint[_me.look]) >> 7);

		Locate &drone = _bfight[i];
		battleResetBounds(_screenR, drone);
		uint8 droneAng = _bfight[i].ang;

		// Abdomen
		battleAccumulateBounds(_screenR, kBDroneAbdDef, drone, drone.xloc, drone.yloc, droneAng, 0,
			_me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc);
		draw3DBattlePrism(kBDroneAbdDef, drone.xloc, drone.yloc, droneAng, 0);

		// Animated pincers: rotate base points by lookx offset
		drone.lookx += drone.delta;
		if (drone.lookx < -kTankMax || drone.lookx > kTankMax)
			drone.delta = -drone.delta;

		// Build animated left pincer vertices
		int lPincerPts[4][3];
		int nabs_lookx = (drone.lookx > 0) ? -drone.lookx : drone.lookx; // nabs
		int lLook = nabs_lookx - 32;
		if (lLook < 0)
			lLook += 256;
		for (int j = 0; j < 4; j++) {
			long tcos = _cost[(uint8)lLook];
			long tsin = _sint[(uint8)lLook];
			lPincerPts[j][0] = (int)(((long)kLLPincerPts[j][0] * tcos - (long)kLLPincerPts[j][1] * tsin) >> 7);
			lPincerPts[j][1] = (int)(((long)kLLPincerPts[j][0] * tsin + (long)kLLPincerPts[j][1] * tcos) >> 7);
			lPincerPts[j][2] = kLLPincerPts[j][2];
			lPincerPts[j][0] += 120; // offset from abdomen center
		}
		PrismPartDef lPincerDef = {4, lPincerPts, 4, kBLPincerSurf};
		battleAccumulateBounds(_screenR, lPincerDef, drone, drone.xloc, drone.yloc, droneAng, 0,
			_me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc);
		draw3DBattlePrism(lPincerDef, drone.xloc, drone.yloc, droneAng, 0);

		// Build animated right pincer vertices
		int rPincerPts[4][3];
		int rLook = ABS(drone.lookx) - 32;
		if (rLook < 0)
			rLook += 256;
		for (int j = 0; j < 4; j++) {
			long tcos = _cost[(uint8)rLook];
			long tsin = _sint[(uint8)rLook];
			rPincerPts[j][0] = (int)(((long)kRRPincerPts[j][0] * tcos - (long)kRRPincerPts[j][1] * tsin) >> 7);
			rPincerPts[j][1] = (int)(((long)kRRPincerPts[j][0] * tsin + (long)kRRPincerPts[j][1] * tcos) >> 7);
			rPincerPts[j][2] = kRRPincerPts[j][2];
			rPincerPts[j][0] += 120;
		}
		PrismPartDef rPincerDef = {4, rPincerPts, 4, kBRPincerSurf};
		battleAccumulateBounds(_screenR, rPincerDef, drone, drone.xloc, drone.yloc, droneAng, 0,
			_me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc);
		draw3DBattlePrism(rPincerDef, drone.xloc, drone.yloc, droneAng, 0);

		// Eyes
		battleAccumulateBounds(_screenR, kBLEyeDef, drone, drone.xloc, drone.yloc, droneAng, 0,
			_me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc);
		battleAccumulateBounds(_screenR, kBREyeDef, drone, drone.xloc, drone.yloc, droneAng, 0,
			_me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc);
		draw3DBattlePrism(kBLEyeDef, drone.xloc, drone.yloc, droneAng, 0);
		draw3DBattlePrism(kBREyeDef, drone.xloc, drone.yloc, droneAng, 0);
		drone.dist = forward;

		// Track for hit detection
		if (_battleMaxP < 100) {
			_battlePwh[_battleMaxP] = &drone;
			_battleMaxP++;
		}

		_insight = _insight || (drone.xmn < _centerX && drone.xmx > _centerX);
	}

	// --- Projectile ---
	if (_projon) {
		long relX = _battleProj.xloc - _me.xloc;
		long relY = _battleProj.yloc - _me.yloc;
		if (ABS(relX) + ABS(relY) < 20000) {
			const int forward = (int)((relX * _cost[_me.look] + relY * _sint[_me.look]) >> 7);
			battleResetBounds(_screenR, _battleProj);
			battleAccumulateBounds(_screenR, kProjDef, _battleProj,
				_battleProj.xloc, _battleProj.yloc, _battleProj.ang, -kFloor,
				_me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc);
			_battleProj.dist = forward;
			draw3DBattlePrism(kProjDef, _battleProj.xloc, _battleProj.yloc,
				_battleProj.ang, -kFloor);
			if (_battleMaxP < 100) {
				_battlePwh[_battleMaxP] = &_battleProj;
				_battleMaxP++;
			}
		}
	}

	// --- Entrance ---
	{
		long relX = _battleEnter.xloc - _me.xloc;
		long relY = _battleEnter.yloc - _me.yloc;
		if (ABS(relX) + ABS(relY) < 20000) {
			const int forward = (int)((relX * _cost[_me.look] + relY * _sint[_me.look]) >> 7);
			battleResetBounds(_screenR, _battleEnter);
			battleAccumulateBounds(_screenR, kEntDef, _battleEnter,
				_battleEnter.xloc, _battleEnter.yloc, _battleEnter.ang, -kFloor,
				_me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc);
			battleAccumulateBounds(_screenR, kEntDoorDef, _battleEnter,
				_battleEnter.xloc, _battleEnter.yloc, _battleEnter.ang, -kFloor,
				_me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc);
			_battleEnter.dist = forward;
			draw3DBattlePrism(kEntDef, _battleEnter.xloc, _battleEnter.yloc,
				_battleEnter.ang, -kFloor);
			// Draw door with depth test disabled so it renders fully
			// on top of the coplanar entrance wall (same fill color,
			// but the black outline becomes clearly visible).
			_gfx->setDepthState(true, false);
			_gfx->setDepthRange(0.0, 0.999);
			draw3DBattlePrism(kEntDoorDef, _battleEnter.xloc, _battleEnter.yloc,
				_battleEnter.ang, -kFloor);
			_gfx->setDepthRange(0.0, 1.0);
			_gfx->setDepthState(true, true);
			if (_battleMaxP < 100) {
				_battlePwh[_battleMaxP] = &_battleEnter;
				_battleMaxP++;
			}
		}
	}

	// --- Shuttle (only if not in orbit) ---
	if (!_orbit) {
		long relX = _battleShip.xloc - _me.xloc;
		long relY = _battleShip.yloc - _me.yloc;
		if (ABS(relX) + ABS(relY) < 20000) {
			const int forward = (int)((relX * _cost[_me.look] + relY * _sint[_me.look]) >> 7);
			battleResetBounds(_screenR, _battleShip);
			battleAccumulateBounds(_screenR, kSBodyDef, _battleShip,
				_battleShip.xloc, _battleShip.yloc, _battleShip.ang, -kFloor,
				_me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc);
			battleAccumulateBounds(_screenR, kSFrontDef, _battleShip,
				_battleShip.xloc, _battleShip.yloc, _battleShip.ang, -kFloor,
				_me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc);
			battleAccumulateBounds(_screenR, kSBackDef, _battleShip,
				_battleShip.xloc, _battleShip.yloc, _battleShip.ang, -kFloor,
				_me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc);
			battleAccumulateBounds(_screenR, kFTopDef, _battleShip,
				_battleShip.xloc, _battleShip.yloc, _battleShip.ang, -kFloor,
				_me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc);
			battleAccumulateBounds(_screenR, kFLeftDef, _battleShip,
				_battleShip.xloc, _battleShip.yloc, _battleShip.ang, -kFloor,
				_me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc);
			battleAccumulateBounds(_screenR, kFRightDef, _battleShip,
				_battleShip.xloc, _battleShip.yloc, _battleShip.ang, -kFloor,
				_me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc);
			battleAccumulateBounds(_screenR, kSDoorDef, _battleShip,
				_battleShip.xloc, _battleShip.yloc, _battleShip.ang, -kFloor,
				_me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc);
			_battleShip.dist = forward;
			draw3DBattlePrism(kSBodyDef, _battleShip.xloc, _battleShip.yloc,
				_battleShip.ang, -kFloor);
			draw3DBattlePrism(kSFrontDef, _battleShip.xloc, _battleShip.yloc,
				_battleShip.ang, -kFloor);
			draw3DBattlePrism(kSBackDef, _battleShip.xloc, _battleShip.yloc,
				_battleShip.ang, -kFloor);
			// Fins: force-draw (no backface cull) — single-sided surfaces
			draw3DBattlePrism(kFTopDef, _battleShip.xloc, _battleShip.yloc,
				_battleShip.ang, -kFloor);
			draw3DBattlePrism(kFLeftDef, _battleShip.xloc, _battleShip.yloc,
				_battleShip.ang, -kFloor);
			draw3DBattlePrism(kFRightDef, _battleShip.xloc, _battleShip.yloc,
				_battleShip.ang, -kFloor);
			_gfx->setDepthState(true, false);
			_gfx->setDepthRange(0.0, 0.999);
			draw3DBattlePrism(kSDoorDef, _battleShip.xloc, _battleShip.yloc,
				_battleShip.ang, -kFloor);
			_gfx->setDepthRange(0.0, 1.0);
			_gfx->setDepthState(true, true);
			if (_battleMaxP < 100) {
				_battlePwh[_battleMaxP] = &_battleShip;
				_battleMaxP++;
			}
		}
	}
}

void ColonyEngine::battleThink() {
	if (_projon) {
		const int fx = battleNormalizeCoord(_battleProj.xloc + (_cost[_battleProj.ang] * 4));
		const int fy = battleNormalizeCoord(_battleProj.yloc + (_sint[_battleProj.ang] * 4));
		if (0 == (_pcount--))
			_projon = false;
		battleProjCommand(fx, fy);
	}

	const int xcheck = _bfight[_battleRound].xloc;
	const int ycheck = _bfight[_battleRound].yloc;
	int bestShootDist = 4000;
	bool shouldShoot = false;
	int shooter = 0;

	for (int i = 0; i < 16; i++) {
		if (i != _battleRound &&
			xcheck > _bfight[i].xloc - kBattleSize &&
			xcheck < _bfight[i].xloc + kBattleSize &&
			ycheck > _bfight[i].yloc - kBattleSize &&
			ycheck < _bfight[i].yloc + kBattleSize) {
			while (_bfight[i].xloc - _me.xloc < 2000 &&
				_bfight[i].yloc - _me.yloc < 2000) {
				_bfight[i].xloc = _randomSource.getRandomNumber(0x7FFF);
				_bfight[i].yloc = _randomSource.getRandomNumber(0x7FFF);
			}
		}

		uint8 &ang = _bfight[i].ang;
		long dx = _bfight[i].xloc - _me.xloc;
		long dy = _bfight[i].yloc - _me.yloc;
		long adx = ABS(dx);
		long ady = ABS(dy);
		bool tooFar = false;
		long distance = 0;

		if (adx > 4000 || ady > 4000) {
			dx >>= 8;
			dy >>= 8;
			tooFar = true;
		}

		long dir = dx * _sint[ang] - dy * _cost[ang];
		if (!tooFar) {
			distance = (long)sqrt((double)(dx * dx + dy * dy));
			if (distance > 0) {
				dir /= distance;
				if (ABS(dir) < 10) {
					if (dir < 0)
						ang--;
					if (dir > 0)
						ang++;
				} else {
					if (dir < 0)
						ang -= 4;
					if (dir > 0)
						ang += 4;
				}

				if (ABS(dir) == 0 && !_projon && distance < bestShootDist) {
					bestShootDist = (int)distance;
					shooter = i;
					shouldShoot = true;
				}
			}
		} else {
			if (dir < 0)
				ang -= 4;
			if (dir > 0)
				ang += 4;
		}

		const int fx = _bfight[i].xloc + (_cost[ang] >> 2);
		const int fy = _bfight[i].yloc + (_sint[ang] >> 2);
		if (distance > 250 || tooFar) {
			if ((!_orbit) &&
				fx > _battleShip.xloc - 2 * kBattleSize &&
				fx < _battleShip.xloc + 2 * kBattleSize &&
				fy > _battleShip.yloc - 4 * kBattleSize &&
				fy < _battleShip.yloc + 4 * kBattleSize) {
				ang += 8;
			} else if (fx > _battleEnter.xloc - 2 * kBattleSize &&
				fx < _battleEnter.xloc + 2 * kBattleSize &&
				fy > _battleEnter.yloc - 2 * kBattleSize &&
				fy < _battleEnter.yloc + 2 * kBattleSize) {
				ang += 8;
			} else {
				_bfight[i].xloc = battleNormalizeCoord(fx);
				_bfight[i].yloc = battleNormalizeCoord(fy);
			}
		} else {
			_sound->play(Sound::kBonk);
		}

		_bfight[i].look = ang;
	}

	_battleRound = (_battleRound + 1) & 0x0F;
	if (shouldShoot) {
		_sound->play(Sound::kShoot);
		_battleProj.ang = _bfight[shooter].ang;
		_battleProj.look = _bfight[shooter].look;
		_battleProj.xloc = battleNormalizeCoord(_bfight[shooter].xloc + (_cost[_battleProj.ang] * 2));
		_battleProj.yloc = battleNormalizeCoord(_bfight[shooter].yloc + (_sint[_battleProj.ang] * 2));
		debugC(1, kColonyDebugCombat,
			"battleEnemyShoot: enemy=%d pos=(%d,%d) ang=%d proj=(%d,%d)",
			shooter, _bfight[shooter].xloc, _bfight[shooter].yloc, _battleProj.ang,
			_battleProj.xloc, _battleProj.yloc);
		_pcount = 10;
		_projon = true;
	}
}

void ColonyEngine::enterColonyFromBattle(int mapNum, int xloc, int yloc) {
	playTunnelAirlockEffect();
	_gameMode = kModeColony;
	_projon = false;
	_pcount = 0;
	_me.xloc = xloc;
	_me.yloc = yloc;
	_me.xindex = _me.xloc >> 8;
	_me.yindex = _me.yloc >> 8;
	loadMap(mapNum);
	_coreIndex = (mapNum == 1) ? 0 : 1;
}

void ColonyEngine::battleCommand(int xnew, int ynew) {
	xnew = battleNormalizeCoord(xnew);
	ynew = battleNormalizeCoord(ynew);

	auto bonk = [&]() {
		_sound->play(Sound::kBonk);
	};

	for (int i = 0; i < 16; i++) {
		if (xnew > _bfight[i].xloc - kBattleSize &&
			xnew < _bfight[i].xloc + kBattleSize &&
			ynew > _bfight[i].yloc - kBattleSize &&
			ynew < _bfight[i].yloc + kBattleSize) {
			bonk();
			return;
		}
	}

	if (!_orbit &&
		xnew > _battleShip.xloc - 2 * kBattleSize &&
		xnew < _battleShip.xloc &&
		ynew > _battleShip.yloc - kBattleSize / 2 &&
		ynew < _battleShip.yloc + kBattleSize / 2) {
		enterColonyFromBattle(1, 900, 3000);
		return;
	}

	if (xnew > _battleEnter.xloc - 2 * kBattleSize &&
		xnew < _battleEnter.xloc &&
		ynew > _battleEnter.yloc - kBattleSize / 2 &&
		ynew < _battleEnter.yloc + kBattleSize / 2) {
		enterColonyFromBattle(2, 384, 640);
		return;
	}

	if ((!_orbit &&
		xnew > _battleShip.xloc - 2 * kBattleSize &&
		xnew < _battleShip.xloc + 2 * kBattleSize &&
		ynew > _battleShip.yloc - 4 * kBattleSize &&
		ynew < _battleShip.yloc + 4 * kBattleSize) ||
		(xnew > _battleEnter.xloc - 2 * kBattleSize &&
		xnew < _battleEnter.xloc + 2 * kBattleSize &&
		ynew > _battleEnter.yloc - 2 * kBattleSize &&
		ynew < _battleEnter.yloc + 2 * kBattleSize)) {
		bonk();
		return;
	}

	const int wrappedX = wrapBattleCoord(xnew);
	const int wrappedY = wrapBattleCoord(ynew);
	const int qx = wrappedX >> 13;
	const int qy = wrappedY >> 13;
	Locate *pw = _pyramids[qx][qy];
	for (int i = 0; i < kMaxQuad; i++) {
		if (wrappedX > pw[i].xloc - kBattleSize && wrappedX < pw[i].xloc + kBattleSize &&
			wrappedY > pw[i].yloc - kBattleSize && wrappedY < pw[i].yloc + kBattleSize) {
			bonk();
			return;
		}
	}

	_me.xloc = battleNormalizeCoord(xnew);
	_me.yloc = battleNormalizeCoord(ynew);
	_me.xindex = wrapBattleCoord(_me.xloc) >> 8;
	_me.yindex = wrapBattleCoord(_me.yloc) >> 8;
}

void ColonyEngine::battleShoot() {
	static bool s_sendFarX = false;

	if (_me.power[0] <= 0 || _weapons <= 0 || _fl)
		return;

	_sound->play(Sound::kBang);

	// DOS BATTLE.C: doShootCircles(cx, cy) then SetPower(-2,0,0)
	const Common::Point aim = getAimPoint();
	const int cx = aim.x;
	const int cy = aim.y;
	doShootCircles(cx, cy);
	setPower(-2, 0, 0);

	int bestDist = 11584;
	int bestIndex = -1;
	for (int i = 0; i < _battleMaxP; i++) {
		Locate *target = _battlePwh[i];
		if (target->xmn < cx && target->xmx > cx &&
			target->zmn < cy && target->zmx > cy &&
			target->dist < bestDist) {
			bestDist = target->dist;
			bestIndex = i;
		}
	}

	if (bestIndex < 0 || bestDist >= 4000)
		return;

	// DOS BATTLE.C: doBurnHole(&r) with r sized from rtable[dist]
	int hitRadius = (bestDist > 0) ? CLIP<int>(kFloor * 128 / bestDist, 1, 100) : 50;
	doBurnHole(cx, cy, hitRadius);

	Locate *target = _battlePwh[bestIndex];
	if (target->type != kRobCube)
		return;

	target->power[1] -= battlePowerLevel(_me.power[0]);
	if (target->power[1] < 0) {
		target->power[1] = 15 + (_randomSource.getRandomNumber(0x0F) & 0x0F);
		if (s_sendFarX) {
			target->xloc = battleNormalizeCoord(_me.xloc + 16000);
			target->yloc = battleNormalizeCoord(_me.yloc + _randomSource.getRandomNumber(0x7FFF));
		} else {
			target->xloc = battleNormalizeCoord(_me.xloc + _randomSource.getRandomNumber(0x7FFF));
			target->yloc = battleNormalizeCoord(_me.yloc + 16000);
		}
		s_sendFarX = !s_sendFarX;
		_sound->play(Sound::kExplode);
	}
}

void ColonyEngine::battleProjCommand(int xcheck, int ycheck) {
	xcheck = battleNormalizeCoord(xcheck);
	ycheck = battleNormalizeCoord(ycheck);
	_battleProj.xloc = xcheck;
	_battleProj.yloc = ycheck;

	if (xcheck > _me.xloc - 200 && xcheck < _me.xloc + 200 &&
		ycheck > _me.yloc - 200 && ycheck < _me.yloc + 200) {
		debugC(1, kColonyDebugCombat,
			"battleProjHitPlayer: proj=(%d,%d) player=(%d,%d) delta=[-4,-4,-4]",
			xcheck, ycheck, _me.xloc, _me.yloc);
		// Mac battle.c: InvertRect(&Clip) flash when projectile hits player
		meGetShot();
		setPower(-4, -4, -4);
		_sound->play(Sound::kExplode);
		_projon = false;
		return;
	}

	for (int i = 0; i < 16; i++) {
		if (xcheck > _bfight[i].xloc - kBattleSize &&
			xcheck < _bfight[i].xloc + kBattleSize &&
			ycheck > _bfight[i].yloc - kBattleSize &&
			ycheck < _bfight[i].yloc + kBattleSize) {
			while (_bfight[i].xloc - _me.xloc < 2000 &&
				_bfight[i].yloc - _me.yloc < 2000) {
				_bfight[i].xloc = _randomSource.getRandomNumber(0x7FFF);
				_bfight[i].yloc = _randomSource.getRandomNumber(0x7FFF);
			}
			_sound->play(Sound::kBonk);
			_projon = false;
			return;
		}
	}

	if ((!_orbit &&
		xcheck > _battleShip.xloc - 2 * kBattleSize &&
		xcheck < _battleShip.xloc + 2 * kBattleSize &&
		ycheck > _battleShip.yloc - 4 * kBattleSize &&
		ycheck < _battleShip.yloc + 4 * kBattleSize) ||
		(xcheck > _battleEnter.xloc - 2 * kBattleSize &&
		xcheck < _battleEnter.xloc + 2 * kBattleSize &&
		ycheck > _battleEnter.yloc - 2 * kBattleSize &&
		ycheck < _battleEnter.yloc + 2 * kBattleSize)) {
		_sound->play(Sound::kBonk);
		_projon = false;
		return;
	}

	const int wrappedX = wrapBattleCoord(xcheck);
	const int wrappedY = wrapBattleCoord(ycheck);
	const int qx = wrappedX >> 13;
	const int qy = wrappedY >> 13;
	Locate *pw = _pyramids[qx][qy];
	for (int i = 0; i < kMaxQuad; i++) {
		if (wrappedX > pw[i].xloc - kBattleSize && wrappedX < pw[i].xloc + kBattleSize &&
			wrappedY > pw[i].yloc - kBattleSize && wrappedY < pw[i].yloc + kBattleSize) {
			_sound->play(Sound::kBonk);
			_projon = false;
			return;
		}
	}
}

// =====================================================================
// renderBattle: Main battle rendering dispatch.
// Called from the main loop when _gameMode == kModeBattle.
// =====================================================================
void ColonyEngine::renderBattle() {
	_battleMaxP = 0;

	// Phase 1: 2D backdrop (sky gradient, mountains, sun) follows camera pitch
	battleBackdrop();

	// Phase 2: Begin 3D scene with the full battle camera transform.
	_gfx->begin3D(_me.xloc, _me.yloc, 0, _me.look, _me.lookY, _screenR);

	// Phase 3: Ground plane at z=-160.
	{
		uint32 groundColor = 0xFF404040;
		_gfx->setDepthState(false, false);
		_gfx->setWireframe(true, groundColor);
		_gfx->draw3DQuad(-100000.0f, -100000.0f, -160.0f,
			100000.0f, -100000.0f, -160.0f,
			100000.0f,  100000.0f, -160.0f,
			-100000.0f,  100000.0f, -160.0f, groundColor);
	}

	// Phase 4: 3D objects with depth testing.
	_gfx->setDepthState(true, true);
	_gfx->setDepthRange(0.0, 1.0);

	// Draw pyramids (obstacles)
	battleDrawPyramids();

	// Draw tanks, entrance, shuttle, projectile
	battleDrawTanks();

	// Phase 5: End 3D
	_gfx->end3D();
}

} // End of namespace Colony
