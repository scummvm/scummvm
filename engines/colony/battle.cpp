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

// battle.cpp: Outdoor planet surface battle system.
// Reimplements battle.c from the original Mac source as proper OpenGL 3D.
// Original used software 2D projection + wireframe; this uses the existing
// OpenGL renderer with filled polygons and depth-tested 3D.

#include "colony/colony.h"
#include "colony/gfx.h"
#include "colony/sound.h"
#include "common/system.h"
#include <math.h>

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
	int xloc = _me.xloc;
	int yloc = _me.yloc;
	if (xloc < 0) xloc += 0x7FFF;
	if (yloc < 0) yloc += 0x7FFF;
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

		for (int i = 0; i < kMaxQuad; i++) {
			long pxloc = _pyramids[qxi][qyi][i].xloc - (xloc + dx);
			long pyloc = _pyramids[qxi][qyi][i].yloc - (yloc + dy);
			if (ABS(pxloc) + ABS(pyloc) < 8000) {
				// Within render distance — draw as 3D
				int wx = _pyramids[qxi][qyi][i].xloc;
				int wy = _pyramids[qxi][qyi][i].yloc;
				if (dx) wx -= dx;
				if (dy) wy -= dy;
				draw3DBattlePrism(kRockDef, wx, wy,
				                  _pyramids[qxi][qyi][i].ang, -kFloor);

				// Track for hit detection
				if (_battleMaxP < 100) {
					_battlePwh[_battleMaxP] = &_pyramids[qxi][qyi][i];
					_battleMaxP++;
				}
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
		long dxloc = _bfight[i].xloc - _me.xloc;
		long dyloc = _bfight[i].yloc - _me.yloc;
		if (ABS(dxloc) + ABS(dyloc) >= 8000)
			continue;

		uint8 droneAng = _bfight[i].ang;

		// Abdomen
		draw3DBattlePrism(kBDroneAbdDef, _bfight[i].xloc, _bfight[i].yloc, droneAng, 0);

		// Animated pincers: rotate base points by lookx offset
		_bfight[i].lookx += _bfight[i].delta;
		if (_bfight[i].lookx < -kTankMax || _bfight[i].lookx > kTankMax)
			_bfight[i].delta = -_bfight[i].delta;

		// Build animated left pincer vertices
		int lPincerPts[4][3];
		int nabs_lookx = (_bfight[i].lookx > 0) ? -_bfight[i].lookx : _bfight[i].lookx; // nabs
		int lLook = nabs_lookx - 32;
		if (lLook < 0) lLook += 256;
		for (int j = 0; j < 4; j++) {
			long tcos = _cost[(uint8)lLook];
			long tsin = _sint[(uint8)lLook];
			lPincerPts[j][0] = (int)(((long)kLLPincerPts[j][0] * tcos - (long)kLLPincerPts[j][1] * tsin) >> 7);
			lPincerPts[j][1] = (int)(((long)kLLPincerPts[j][0] * tsin + (long)kLLPincerPts[j][1] * tcos) >> 7);
			lPincerPts[j][2] = kLLPincerPts[j][2];
			lPincerPts[j][0] += 120; // offset from abdomen center
		}
		PrismPartDef lPincerDef = {4, lPincerPts, 4, kBLPincerSurf};
		draw3DBattlePrism(lPincerDef, _bfight[i].xloc, _bfight[i].yloc, droneAng, 0);

		// Build animated right pincer vertices
		int rPincerPts[4][3];
		int rLook = ABS(_bfight[i].lookx) - 32;
		if (rLook < 0) rLook += 256;
		for (int j = 0; j < 4; j++) {
			long tcos = _cost[(uint8)rLook];
			long tsin = _sint[(uint8)rLook];
			rPincerPts[j][0] = (int)(((long)kRRPincerPts[j][0] * tcos - (long)kRRPincerPts[j][1] * tsin) >> 7);
			rPincerPts[j][1] = (int)(((long)kRRPincerPts[j][0] * tsin + (long)kRRPincerPts[j][1] * tcos) >> 7);
			rPincerPts[j][2] = kRRPincerPts[j][2];
			rPincerPts[j][0] += 120;
		}
		PrismPartDef rPincerDef = {4, rPincerPts, 4, kBRPincerSurf};
		draw3DBattlePrism(rPincerDef, _bfight[i].xloc, _bfight[i].yloc, droneAng, 0);

		// Eyes
		draw3DBattlePrism(kBLEyeDef, _bfight[i].xloc, _bfight[i].yloc, droneAng, 0);
		draw3DBattlePrism(kBREyeDef, _bfight[i].xloc, _bfight[i].yloc, droneAng, 0);

		// Track for hit detection
		if (_battleMaxP < 100) {
			_battlePwh[_battleMaxP] = &_bfight[i];
			_bfight[i].dist = (int)(ABS(dxloc) + ABS(dyloc)); // manhattan approx
			_battleMaxP++;
		}

		// Check if enemy is in crosshair (simple screen-center check)
		// TODO: proper insight detection via projected screen bounds
	}

	// --- Projectile ---
	if (_projon) {
		long pxloc = _battleProj.xloc - _me.xloc;
		long pyloc = _battleProj.yloc - _me.yloc;
		if (ABS(pxloc) + ABS(pyloc) < 20000) {
			draw3DBattlePrism(kProjDef, _battleProj.xloc, _battleProj.yloc,
			                  _battleProj.ang, -kFloor);
		}
	}

	// --- Entrance ---
	{
		long exloc = _battleEnter.xloc - _me.xloc;
		long eyloc = _battleEnter.yloc - _me.yloc;
		if (ABS(exloc) + ABS(eyloc) < 20000) {
			draw3DBattlePrism(kEntDef, _battleEnter.xloc, _battleEnter.yloc,
			                  _battleEnter.ang, -kFloor);
			draw3DBattlePrism(kEntDoorDef, _battleEnter.xloc, _battleEnter.yloc,
			                  _battleEnter.ang, -kFloor);
			if (_battleMaxP < 100) {
				_battlePwh[_battleMaxP] = &_battleEnter;
				_battleMaxP++;
			}
		}
	}

	// --- Shuttle (only if not in orbit) ---
	if (!_orbit) {
		long sxloc = _battleShip.xloc - _me.xloc;
		long syloc = _battleShip.yloc - _me.yloc;
		if (ABS(sxloc) + ABS(syloc) < 20000) {
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
			draw3DBattlePrism(kSDoorDef, _battleShip.xloc, _battleShip.yloc,
			                  _battleShip.ang, -kFloor);
			if (_battleMaxP < 100) {
				_battlePwh[_battleMaxP] = &_battleShip;
				_battleMaxP++;
			}
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
