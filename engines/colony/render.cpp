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
#include "colony/render_internal.h"
#include "colony/renderer.h"
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

// DOS lsColor table from ROBOCOLR.C  maps color index to rendering attributes.
// Fields: monochrome (MONOCHROME  Mac B&W dither pattern, matches MacPattern enum),
//         backColor (BACKCOLOR), fillColor (FILLCOLOR), lineFillColor (LINEFILLCOLOR),
//         lineColor (LINECOLOR), pattern (PATTERN).
// DrawPrism uses: polyfill ON → fill with fillColor/backColor/pattern, outline with lineFillColor.
//                 polyfill OFF → outline only with lineColor.
// Mac B&W uses: monochrome field for dither pattern (WHITE/LTGRAY/GRAY/DKGRAY/BLACK/CLEAR).
struct DOSColorEntry {
	uint8 monochrome;
	uint8 backColor;
	uint8 fillColor;
	uint8 lineFillColor;
	uint8 lineColor;
	uint8 pattern;
};

static const DOSColorEntry g_dosColors[79] = {
	//                     MONO BK  FILL LFIL LINE PAT
	/* 0  cCLEAR      */ { 5,  0,  0,  0,  0, 1},
	/* 1  cBLACK      */ { 4,  0,  0,  0,  0, 1},
	/* 2  cBLUE       */ { 2,  0,  1,  1,  1, 1},
	/* 3  cGREEN      */ { 1,  0,  2,  2,  2, 1},
	/* 4  cCYAN       */ { 2,  0,  3,  3,  3, 1},
	/* 5  cRED        */ { 2,  0,  4,  4,  4, 1},
	/* 6  cMAGENTA    */ { 2,  0,  5,  5,  5, 1},
	/* 7  cBROWN      */ { 3,  0,  6,  6,  6, 1},
	/* 8  cWHITE      */ { 0,  0,  7,  7,  7, 1},
	/* 9  cDKGRAY     */ { 3,  0,  8,  8,  8, 1},
	/* 10 cLTBLUE     */ { 2,  0,  9,  9,  9, 1},
	/* 11 cLTGREEN    */ { 1,  0, 10, 10, 10, 1},
	/* 12 cLTCYAN     */ { 1,  0, 11, 11, 11, 1},
	/* 13 cLTRED      */ { 1,  0, 12, 12, 12, 1},
	/* 14 cLTMAGENTA  */ { 1,  0, 13, 13, 13, 1},
	/* 15 cYELLOW     */ { 0,  0, 14, 14, 14, 1},
	/* 16 cINTWHITE   */ { 0,  0, 15, 15, 15, 1},
	/* 17 cBATH       */ { 0,  0, 15, 15,  0, 1},
	/* 18 cWATER      */ { 1,  0,  7,  1,  1, 4},
	/* 19 cSILVER     */ { 1,  0,  7, 15,  1, 3},
	/* 20 cREACTOR    */ { 0,  0,  7,  7,  7, 1},
	/* 21 cBLANKET    */ { 2,  0,  2,  7,  2, 3},
	/* 22 cSHEET      */ { 0,  0, 15, 15, 15, 1},
	/* 23 cBED        */ { 3,  0,  6,  4,  6, 4},
	/* 24 cBOX        */ { 2,  0,  6,  7,  6, 3},
	/* 25 cBENCH      */ { 2,  0,  6,  6,  6, 1},
	/* 26 cCHAIR      */ { 1,  1,  9,  7,  1, 3},
	/* 27 cCHAIRBASE  */ { 3,  1,  1,  9,  1, 3},
	/* 28 cCOUCH      */ { 2,  0,  4,  3,  4, 3},
	/* 29 cCONSOLE    */ { 1,  0,  6,  4,  4, 3},
	/* 30 cTV         */ { 2,  0,  6,  7,  6, 3},
	/* 31 cTVSCREEN   */ { 3,  8,  8,  7,  8, 1},
	/* 32 cDRAWER     */ { 2,  0,  6,  7,  6, 3},
	/* 33 cCRYO       */ { 2,  1,  9, 15,  1, 3},
	/* 34 cCRYOGLASS  */ { 1,  1, 15,  9,  1, 3},
	/* 35 cCRYOBASE   */ { 3,  1,  1,  9,  1, 3},
	/* 36 cCUBE       */ { 1,  0,  3, 15,  3, 3},
	/* 37 cDESK       */ { 2,  0,  6,  7,  6, 3},
	/* 38 cDESKTOP    */ { 3,  0,  6,  6,  6, 1},
	/* 39 cDESKCHAIR  */ { 1,  0,  2,  8,  2, 3},
	/* 40 cMAC        */ { 0,  0, 15, 15,  0, 1},
	/* 41 cMACSCREEN  */ { 3,  0,  8,  8,  8, 1},
	/* 42 cDRONE      */ { 1,  0,  3,  3,  0, 1},
	/* 43 cCLAW1      */ { 3,  0,  4,  4,  4, 1},
	/* 44 cCLAW2      */ { 1,  0,  3,  3,  3, 1},
	/* 45 cEYES       */ { 0, 15, 15, 15, 15, 1},
	/* 46 cEYE        */ { 0, 15, 15, 15, 15, 1},
	/* 47 cIRIS       */ { 1,  0,  1,  7,  1, 3},
	/* 48 cPUPIL      */ { 4,  0,  0,  0,  0, 3},
	/* 49 cFORKLIFT   */ { 1,  0, 14,  6, 14, 3},
	/* 50 cTREAD1     */ { 3,  0, 14,  6, 14, 4},
	/* 51 cTREAD2     */ { 3,  0, 14,  6, 14, 5},
	/* 52 cPOT        */ { 2,  0,  6,  6,  6, 1},
	/* 53 cPLANT      */ { 2,  2,  2,  2,  2, 1},
	/* 54 cPOWER      */ { 1,  1,  9,  7,  1, 3},
	/* 55 cPBASE      */ { 3,  1,  9,  7,  1, 3},
	/* 56 cPSOURCE    */ { 2,  4,  4, 14,  4, 3},
	/* 57 cPYRAMID    */ { 2,  0,  4, 15,  4, 3},
	/* 58 cQUEEN      */ { 2, 14, 14, 15, 14, 3},
	/* 59 cTOPSNOOP   */ { 2,  5,  3,  5,  3, 3},
	/* 60 cBOTTOMSNOOP*/ { 3,  5,  8,  5,  5, 3},
	/* 61 cTABLE      */ { 2,  6,  6,  7,  6, 3},
	/* 62 cTABLEBASE  */ { 3,  6,  6,  8,  6, 3},
	/* 63 cPSTAND     */ { 3,  0,  5,  5,  5, 1},
	/* 64 cPLENS      */ { 4,  0,  0,  0,  0, 1},
	/* 65 cPROJECTOR  */ { 2,  0,  3,  3,  3, 1},
	/* 66 cTELE       */ { 1,  4,  8,  7,  4, 3},
	/* 67 cTELEDOOR   */ { 1,  0,  7, 15,  1, 3},
	/* 68 cUPYRAMID   */ { 1,  0,  9, 15,  1, 3},
	/* 69 cROCK       */ { 2,  4,  4,  4,  4, 1},
	/* 70 cCOLONY     */ { 2, 14, 14, 14, 14, 1},
	/* 71 cCDOOR      */ { 2, 14, 14, 14, 14, 1},
	/* 72 cSHIP       */ { 2,  9,  9,  9,  9, 1},
	/* 73 cPROJ       */ { 2,  3,  3,  3,  3, 1},
	/* 74 cSHADOW     */ { 3,  8,  8,  8,  8, 1},
	/* 75 cLTGRAY     */ { 1,  8,  8, 15,  8, 3},
	/* 76 cGRAY       */ { 2,  8,  8,  7,  8, 3},
	/* 77 cWALL       */ { 0,  0,  7,  7,  0, 3},
	/* 78 cQUEEN2     */ { 2,  0, 15, 15,  0, 3},
};

// Look up the DOS lsColor entry for a given ObjColor index.
// Returns a fallback entry for out-of-range indices.
static const DOSColorEntry &lookupDOSColor(int colorIdx, int level) {
	// DOS pcycle for animated reactor/suit: WHITE,LTGRAY,GRAY,DKGRAY,BLACK (bounce)
	static const DOSColorEntry kHCore1 = {0, 0, 15, 15, 15, 1}; // WHITE
	static const DOSColorEntry kHCore2 = {1, 8,  8, 15,  8, 3}; // LTGRAY
	static const DOSColorEntry kHCore3 = {2, 8,  8,  7,  8, 3}; // GRAY
	static const DOSColorEntry kHCore4 = {3, 0,  8,  8,  8, 1}; // DKGRAY
	static const DOSColorEntry kCCoreEntry = {0, 0, 15, 15, 15, 1}; // WHITE (cold core)
	static const DOSColorEntry fallback = {2, 0, 0, 0, 0, 1}; // GRAY monochrome

	if (colorIdx >= 0 && colorIdx < 79)
		return g_dosColors[colorIdx];
	switch (colorIdx) {
	case kColorHCore1: return kHCore1;
	case kColorHCore2: return kHCore2;
	case kColorHCore3: return kHCore3;
	case kColorHCore4: return kHCore4;
	case kColorCCore:  return kCCoreEntry;
	case kColorEyeball: return g_dosColors[kColorEye];
	case kColorEyeIris:
	case kColorMiniEyeIris:
	case kColorQueenEye: return g_dosColors[kColorIris];
	case kColorDroneEye:
	case kColorSoldierEye: return g_dosColors[kColorEyes];
	case kColorSoldierBody: return g_dosColors[kColorDrone];
	case kColorQueenBody: return g_dosColors[(level == 7) ? 78 : kColorQueen];
	case kColorQueenWingRed: return g_dosColors[63];
	// Ring animation colors: cycle through DOS EGA colors
	// DOS reactor rings use color=1+count%5 → values 1-5 → cColor[1..5]
	case kColorRainbow1: return g_dosColors[2];  // BLUE
	case kColorRainbow2: return g_dosColors[3];  // GREEN
	case kColorRainbow3: return g_dosColors[4];  // CYAN
	case kColorRainbow4: return g_dosColors[5];  // RED
	default: return fallback;
	}
}

// Map ObjColor → Mac B&W dither pattern (from ROBOCOLR.C MONOCHROME field).
// The monochrome field in the DOS table matches MacPattern enum values directly:
// WHITE=0, LTGRAY=1, GRAY=2, DKGRAY=3, BLACK=4, CLEAR=5.
static int lookupMacPattern(int colorIdx, int level) {
	return lookupDOSColor(colorIdx, level).monochrome;
}

// Map ObjColor constant → Mac Color256 index (cColor[] from colordef.h).
static int mapObjColorToMacColor(int colorIdx, int level) {
	switch (colorIdx) {
	case kColorBath:      return 97;  // c_tub
	case kColorWater:     return 102; // c_water
	case kColorSilver:    return 103; // c_mirror
	case kColorReactor:   return 106; // c_reactor
	case kColorBlanket:   return 68;  // c_bedblnkt
	case kColorSheet:     return 69;  // c_bedsheet
	case kColorBed:       return 70;  // c_bedhead
	case kColorBox:       return 84;  // c_box1
	case kColorBench:     return 104; // c_bench
	case kColorChair:     return 66;  // c_cchair (captain's chair; DOS cCHAIR != Mac c_chair)
	case kColorChairBase: return 67;  // c_cchairbase
	case kColorCouch:     return 74;  // c_couch
	case kColorConsole:   return 79;  // c_console
	case kColorTV:        return 76;  // c_tv
	case kColorTVScreen:  return 77;  // c_tvscreen
	case kColorDrawer:    return 96;  // c_vanity
	case kColorDesk:      return 58;  // c_desk
	case kColorDeskTop:   return 59;  // c_desktop
	case kColorDeskChair: return 60;  // c_deskchair
	case kColorMac:       return 61;  // c_computer
	case kColorMacScreen: return 62;  // c_screen
	case kColorCryo:      return 90;  // c_cryo
	case kColorCryoGlass: return 91;  // c_cryoglass
	case kColorCryoBase:  return 92;  // c_cryostand
	case kColorForklift:  return 86;  // c_forklift
	case kColorTread1:    return 88;  // c_fltreadtop
	case kColorTread2:    return 89;  // c_fltreadside
	case kColorPot:       return 64;  // c_pot
	case kColorPlant:     return 63;  // c_plant
	case kColorPower:     return 80;  // c_consoletop
	case kColorPBase:     return 81;  // c_powerbase
	case kColorPSource:   return 83;  // c_powersource
	case kColorTable:     return 71;  // c_table
	case kColorTableBase: return 71;  // c_table
	case kColorPStand:    return 72;  // c_proj
	case kColorPLens:     return 73;  // c_projlens
	case kColorProjector: return 72;  // c_proj
	case kColorTele:      return 93;  // c_teleport
	case kColorTeleDoor:  return 94;  // c_teledoor
	case kColorRainbow1:  return 26;  // c_color0
	case kColorRainbow2:  return 27;  // c_color1
	case kColorRainbow3:  return 28;  // c_color2
	case kColorRainbow4:  return 29;  // c_color3 (c_hole in Mac enum)
	case kColorCube:      return 39;  // c_diamond (DOS cCUBE = diamond/octahedron alien)
	case kColorDrone:     return 50;  // c_drone
	case kColorClaw1:     return 54;  // c_jaws1
	case kColorClaw2:     return 55;  // c_jaws2
	case kColorEyes:      return 32;  // c_eye
	case kColorEye:       return 34;  // c_eyeball (DOS cEYE = eyeball globe, not c_meye)
	case kColorIris:      return 35;  // c_iris
	case kColorPupil:     return 36;  // c_pupil
	case kColorPyramid:   return 37;  // c_pyramid
	case kColorQueen:     return 48;  // c_queenP
	case kColorEyeball:   return 34;  // c_eyeball
	case kColorEyeIris:   return 32;  // c_eye
	case kColorMiniEyeIris: return 33; // c_meye
	case kColorDroneEye:  return 51;  // c_edrone
	case kColorSoldierBody: return 52; // c_soldier
	case kColorSoldierEye: return 53; // c_esoldier
	case kColorQueenBody: return 43 + CLIP(level - 2, 0, 4); // c_queen1..c_queen5
	case kColorQueenEye:  return 49;  // c_equeen
	case kColorQueenWingRed: return 48; // unused in Mac mode
	case kColorTopSnoop:  return 56;  // c_snooper1
	case kColorBottomSnoop: return 57; // c_snooper2
	case kColorUPyramid:  return 41;  // c_upyramid
	case kColorShadow:    return 26;  // c_color0
	case kColorWall:      return 6;   // c_dwall
	// Animated reactor/power suit colors
	case kColorHCore1:    return 107; // c_hcore1
	case kColorHCore2:    return 108; // c_hcore2
	case kColorHCore3:    return 109; // c_hcore3
	case kColorHCore4:    return 110; // c_hcore4
	case kColorCCore:     return 111; // c_ccore
	default: return 6; // c_dwall fallback
	}
}

static void projectCorridorPointClamped(const Common::Rect &screenR, int look, int lookY,
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
	const float depth = MAX(-eyeZ, 1.0f);

	const float focal = (screenR.height() * 0.5f) / tanf(75.0f * (float)M_PI / 360.0f);
	const float centerX = screenR.left + screenR.width() * 0.5f;
	const float centerY = screenR.top + screenR.height() * 0.5f;

	screenX = (int)roundf(centerX + (eyeX * focal / depth));
	screenY = (int)roundf(centerY - (eyeY * focal / depth));
}

static bool isSurfaceVisible(const int *surface, int pointCount, const int *screenX, const int *screenY) {
	if (pointCount < 3)
		return false;

	for (int i = 0; i < pointCount; ++i) {
		const int cur = surface[i];
		const int next = surface[(i + 1) % pointCount];
		const int next2 = surface[(i + 2) % pointCount];
		const long dx = screenX[cur] - screenX[next];
		const long dy = screenY[cur] - screenY[next];
		const long dxp = screenX[next2] - screenX[next];
		const long dyp = screenY[next2] - screenY[next];

		if (dx < 0) {
			if (dy == 0) {
				if (dyp > 0)
					return false;
				if (dyp < 0)
					return true;
			} else {
				const long b = dy * dxp - dx * dyp;
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
				const long b = dx * dyp - dy * dxp;
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

static bool projectCorridorPoint(const Common::Rect &screenR, uint8 look, int8 lookY,
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

void ColonyEngine::draw3DPrism(Thing &obj, const PrismPartDef &def, bool useLook, int colorOverride, bool accumulateBounds, bool forceVisible) {
	// +32 compensates for the original sine table's 45° phase offset.
	// Object angles from game data were stored assuming that offset.
	const uint8 ang = (useLook ? obj.where.look : obj.where.ang) + 32;
	const long rotCos = _cost[ang];
	const long rotSin = _sint[ang];
	const bool lit = (_corePower[_coreIndex] > 0);
	float transformedX[32];
	float transformedY[32];
	float transformedZ[32];
	int projectedX[32];
	int projectedY[32];

	assert(def.pointCount <= ARRAYSIZE(transformedX));

	for (int i = 0; i < def.pointCount; ++i) {
		const int ox = def.points[i][0];
		const int oy = def.points[i][1];
		const int oz = def.points[i][2];
		const long rx = ((long)ox * rotCos - (long)oy * rotSin) >> 7;
		const long ry = ((long)ox * rotSin + (long)oy * rotCos) >> 7;

		transformedX[i] = (float)(rx + obj.where.xloc);
		transformedY[i] = (float)(ry + obj.where.yloc);
		transformedZ[i] = (float)(oz - 160);
		projectCorridorPointClamped(_screenR, _me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc,
		                            transformedX[i], transformedY[i], transformedZ[i],
		                            projectedX[i], projectedY[i]);

		if (accumulateBounds) {
			int sx = 0;
			int sy = 0;
			if (projectCorridorPoint(_screenR, _me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc,
			                         transformedX[i], transformedY[i], transformedZ[i], sx, sy)) {
				obj.where.xmn = MIN(obj.where.xmn, sx);
				obj.where.xmx = MAX(obj.where.xmx, sx);
				obj.where.zmn = MIN(obj.where.zmn, sy);
				obj.where.zmx = MAX(obj.where.zmx, sy);
			}
		}
	}

	for (int i = 0; i < def.surfaceCount; i++) {
		const int colorIdx = (colorOverride >= 0) ? colorOverride : def.surfaces[i][0];
		const int n = def.surfaces[i][1];
		if (n < 2)
			continue;

		float px[8];
		float py[8];
		float pz[8];
		int pointIdx[8];
		int count = 0;

		for (int j = 0; j < n; j++) {
			const int cur = def.surfaces[i][j + 2];
			if (cur < 0 || cur >= def.pointCount)
				continue;
			pointIdx[count] = cur;
			px[count] = transformedX[cur];
			py[count] = transformedY[cur];
			pz[count] = transformedZ[cur];
			count++;
		}

		if (!forceVisible && count >= 3 && !isSurfaceVisible(pointIdx, count, projectedX, projectedY))
			continue;

		if (count >= 3) {
			if (colorIdx == kColorClear) {
				// CLEAR = no fill, but still draw wireframe outline.
				// DOS DrawPrism skips CLEAR in the fill pass but draws
				// outlines for ALL surfaces in the wireframe pass.
				uint32 color = lit ? 0 : 15; // vBLACK lit, vINTWHITE unlit
				for (int j = 0; j < count; j++) {
					int next = (j + 1) % count;
					_gfx->draw3DLine(px[j], py[j], pz[j],
					                 px[next], py[next], pz[next], color);
				}
				continue;
			}

			if (_renderMode == Common::kRenderMacintosh && _hasMacColors) {
				// Mac color rendering: follows SuperPoly() from calcrobo.c:429-505.
				int pattern;
				uint32 fg;
				uint32 bg;

				if (colorIdx == kColorCorridorWall) {
					// Original Mac calcrobo.c special-cases c_lwall/c_dwall:
					// lit c_lwall uses the current corridor wall tint + black outline,
					// unlit c_dwall collapses to a solid dark wall color.
					pattern = 0; // WHITE pattern = solid bg fill + fg outline
					if (lit) {
						fg = 0xFF000000;
						bg = packMacColor(_macColors[8 + _level - 1].fg);
					} else {
						bg = packMacColor(_macColors[6].bg);
						fg = bg;
					}
					debugC(5, kColonyDebugRender, "draw3DPrism Mac corridor wall: fg=0x%08X bg=0x%08X lit=%d",
					      fg, bg, lit);
				} else {
					int mIdx = mapObjColorToMacColor(colorIdx, _level);
					pattern = _macColors[mIdx].pattern;
					fg = packMacColor(_macColors[mIdx].fg);
					bg = packMacColor(_macColors[mIdx].bg);
					debugC(5, kColonyDebugRender, "draw3DPrism Mac: colorIdx=%d mIdx=%d pat=%d fg=0x%08X bg=0x%08X lit=%d",
					      colorIdx, mIdx, pattern, fg, bg, lit);

					if (!lit) {
						// Mac unlit: all non-wall surfaces fill solid black
						fg = 0xFF000000;
						bg = 0xFF000000;
						pattern = 4; // force BLACK (solid fill)
					}
				}

				if (pattern == 5) {
					// CLEAR: outline only (Mac: FramePoly with fg, no fill)
					_gfx->setWireframe(true, -1);
					_gfx->draw3DPolygon(px, py, pz, count, 0xFF000000);
				} else {
					const byte *stipple = setupMacPattern(_gfx, pattern, fg, bg);
					// Mac SuperPoly: FramePoly uses fg color for outline.
					// In Color256, fg is (0,0,0) for ~90% of materials.
					// BLACK pattern has no FramePoly in original, so outline
					// matches fill (invisible). Others: black outline.
					uint32 outlineColor = (pattern == 4) ? fg : (uint32)0xFF000000;
					_gfx->draw3DPolygon(px, py, pz, count, outlineColor);
					if (stipple)
					_gfx->setStippleData(nullptr);
				}
			} else if (lit) {
				if (_renderMode == Common::kRenderMacintosh) {
					// Mac B&W: stipple dither pattern fill + black outline
					int pattern = lookupMacPattern(colorIdx, _level);
					if (pattern == kPatternClear)
						continue;
					if (!_wireframe) {
						_gfx->setWireframe(true, pattern == kPatternBlack ? 0 : 255);
					}
					_gfx->setStippleData(kMacStippleData[pattern]);
					_gfx->draw3DPolygon(px, py, pz, count, 0); // black outline
					_gfx->setStippleData(nullptr);
				} else {
					// EGA: per-surface colors from DOS lsColor table.
					// polyfill ON  → B&W fill (from MONOCHROME field), colored LINECOLOR outline.
					// polyfill OFF → outline only with LINECOLOR.
					const DOSColorEntry &dc = lookupDOSColor(colorIdx, _level);
					if (!_wireframe) {
						// Polyfill mode: B&W fill + colored LINECOLOR outline.
						// LINECOLOR (not LINEFILLCOLOR)  has proper contrast against B&W fills.
						if (dc.monochrome == kPatternClear)
							continue;
						_gfx->setWireframe(true, 7); // all surfaces white; colored outlines provide distinction
						_gfx->draw3DPolygon(px, py, pz, count, (uint32)dc.lineColor);
					} else {
						// Wireframe only: LINECOLOR outline, no fill.
						_gfx->draw3DPolygon(px, py, pz, count, (uint32)dc.lineColor);
					}
				}
			} else {
				// Unlit: black fill, white outline (DOS: PenColor(vINTWHITE))
				if (!_wireframe) {
					_gfx->setWireframe(true, 0); // black fill
				}
				_gfx->draw3DPolygon(px, py, pz, count, 15);
			}
		}
	}
}

void ColonyEngine::draw3DLeaf(const Thing &obj, const PrismPartDef &def) {
	// DOS DrawLeaf: draws leaf surfaces as lines (MoveTo/LineTo), not filled polygons.
	// PenColor is set to vGREEN by the caller (MakePlant).
	const uint8 ang = obj.where.ang + 32;
	const long rotCos = _cost[ang];
	const long rotSin = _sint[ang];
	const bool lit = (_corePower[_coreIndex] > 0);
	// Mac color: c_plant bg; Mac B&W: black; EGA: green; unlit: white/black
	uint32 color;
	if (_renderMode == Common::kRenderMacintosh && _hasMacColors) {
		color = lit ? packMacColor(_macColors[63].bg) : 0xFF000000; // c_plant
	} else {
		color = lit ? (_renderMode == Common::kRenderMacintosh ? 0 : 2) : 15;
	}

	for (int i = 0; i < def.surfaceCount; i++) {
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
			long rx = ((long)ox * rotCos - (long)oy * rotSin) >> 7;
			long ry = ((long)ox * rotSin + (long)oy * rotCos) >> 7;
			px[count] = (float)(rx + obj.where.xloc);
			py[count] = (float)(ry + obj.where.yloc);
			pz[count] = (float)(oz - 160);
			count++;
		}

		// Draw as connected line segments (MoveTo first point, LineTo the rest)
		for (int j = 0; j < count - 1; j++)
			_gfx->draw3DLine(px[j], py[j], pz[j], px[j + 1], py[j + 1], pz[j + 1], color);
	}
}

void ColonyEngine::draw3DSphere(Thing &obj, int pt0x, int pt0y, int pt0z,
                                int pt1x, int pt1y, int pt1z,
                                uint32 fillColor, uint32 outlineColor, bool accumulateBounds) {
	// Original Colony eye/ball primitives store the bottom pole in pt0 and the
	// sphere center in pt1. The classic renderer builds the oval from the
	// screen-space delta between those two projected points, so the world-space
	// radius is the full pt0↔pt1 distance, not half.
	// Rendered as a billboard polygon facing the camera.
	const uint8 ang = obj.where.ang + 32;
	const long rotCos = _cost[ang];
	const long rotSin = _sint[ang];
	const bool lit = (_corePower[_coreIndex] > 0);

	// Transform both points to world space
	long rx0 = ((long)pt0x * rotCos - (long)pt0y * rotSin) >> 7;
	long ry0 = ((long)pt0x * rotSin + (long)pt0y * rotCos) >> 7;
	float wx0 = (float)(rx0 + obj.where.xloc);
	float wy0 = (float)(ry0 + obj.where.yloc);
	float wz0 = (float)(pt0z - 160);

	long rx1 = ((long)pt1x * rotCos - (long)pt1y * rotSin) >> 7;
	long ry1 = ((long)pt1x * rotSin + (long)pt1y * rotCos) >> 7;
	float wx1 = (float)(rx1 + obj.where.xloc);
	float wy1 = (float)(ry1 + obj.where.yloc);
	float wz1 = (float)(pt1z - 160);

	// Center and radius (pt1 is the center, pt0 is the bottom pole).
	float cx = wx1;
	float cy = wy1;
	float cz = wz1;
	float dx = wx1 - wx0, dy = wy1 - wy0, dz = wz1 - wz0;
	float radius = sqrtf(dx * dx + dy * dy + dz * dz);

	// Billboard: create a polygon perpendicular to the camera direction.
	// Camera is at (_me.xloc, _me.yloc, 0).
	float viewDx = cx - (float)_me.xloc;
	float viewDy = cy - (float)_me.yloc;
	float viewLen = sqrtf(viewDx * viewDx + viewDy * viewDy);
	if (viewLen < 0.001f)
		return;

	// "right" vector: perpendicular to view in XY plane
	float rightX = -viewDy / viewLen;
	float rightY = viewDx / viewLen;
	// "up" vector: world Z axis
	float upZ = 1.0f;

	// Create 12-sided polygon
	static const int N = 12;
	float px[N], py[N], pz[N];
	for (int i = 0; i < N; i++) {
		float a = (float)i * 2.0f * (float)M_PI / (float)N;
		float cosA = cosf(a);
		float sinA = sinf(a);
		px[i] = cx + radius * (cosA * rightX);
		py[i] = cy + radius * (cosA * rightY);
		pz[i] = cz + radius * (sinA * upZ);
	}

	if (accumulateBounds) {
		for (int i = 0; i < N; i++) {
			int sx = 0;
			int sy = 0;
			if (projectCorridorPoint(_screenR, _me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc,
			                         px[i], py[i], pz[i], sx, sy)) {
				obj.where.xmn = MIN(obj.where.xmn, sx);
				obj.where.xmx = MAX(obj.where.xmx, sx);
				obj.where.zmn = MIN(obj.where.zmn, sy);
				obj.where.zmx = MAX(obj.where.zmx, sy);
			}
		}
	}

	if (_renderMode == Common::kRenderMacintosh && _hasMacColors) {
		// Mac color: map fillColor to Mac color index and use RGB
		// fillColor is an ObjColor enum value passed by the caller
		const int fillIdx = mapObjColorToMacColor((int)fillColor, _level);
		const int outlineIdx = mapObjColorToMacColor((int)outlineColor, _level);
		int pattern = _macColors[fillIdx].pattern;
		uint32 fg = packMacColor(_macColors[fillIdx].fg);
		uint32 bg = packMacColor(_macColors[fillIdx].bg);
		uint32 line = packMacColor(_macColors[outlineIdx].fg);
		if (!lit) {
			fg = 0xFF000000;
			bg = 0xFF000000;
			line = 0xFF000000;
			pattern = 4;
		}
		const byte *stipple = setupMacPattern(_gfx, pattern, fg, bg);
		_gfx->draw3DPolygon(px, py, pz, N, line);
		if (stipple)
			_gfx->setStippleData(nullptr);
	} else if (lit) {
		if (_renderMode == Common::kRenderMacintosh) {
			int pattern = lookupMacPattern((int)fillColor, _level);
			if (pattern == kPatternClear)
				pattern = kPatternGray;
			if (!_wireframe) {
				_gfx->setWireframe(true, pattern == kPatternBlack ? 0 : 255);
			}
			_gfx->setStippleData(kMacStippleData[pattern]);
			_gfx->draw3DPolygon(px, py, pz, N, lookupDOSColor((int)outlineColor, _level).lineColor);
			_gfx->setStippleData(nullptr);
		} else {
			const DOSColorEntry &fill = lookupDOSColor((int)fillColor, _level);
			const DOSColorEntry &outline = lookupDOSColor((int)outlineColor, _level);
			if (!_wireframe) {
				_gfx->setWireframe(true, fill.fillColor);
			}
			_gfx->draw3DPolygon(px, py, pz, N, outline.lineColor);
		}
	} else {
		// Unlit: black fill, white outline.
		if (!_wireframe) {
			_gfx->setWireframe(true, 0);
		}
		_gfx->draw3DPolygon(px, py, pz, N, 15);
	}
}

void ColonyEngine::computeVisibleCells() {
	memset(_visibleCell, 0, sizeof(_visibleCell));

	int px = _me.xindex;
	int py = _me.yindex;
	if (px < 0 || px >= 32 || py < 0 || py >= 32)
		return;

	// Check if a wall feature exists on a cell boundary (either side).
	// Features like doors define room boundaries and block visibility.
	auto hasFeatureAt = [this](int x, int y, int dir) -> bool {
		const uint8 *map = mapFeatureAt(x, y, dir);
		return map && map[0] != kWallFeatureNone;
	};

	// BFS from player cell, stopping at walls AND wall features.
	// This limits visibility to the current "room"  doors, shelves, etc.
	// act as solid barriers even if the cells are connected via open paths.
	_visibleCell[px][py] = true;
	int queueX[1024], queueY[1024];
	int head = 0, tail = 0;
	queueX[tail] = px;
	queueY[tail] = py;
	tail++;

	while (head < tail) {
		int cx = queueX[head];
		int cy = queueY[head];
		head++;

		// North: (cx, cy+1)
		if (cy + 1 < 32 && !_visibleCell[cx][cy + 1]) {
			bool blocked = (wallAt(cx, cy + 1) & 0x01) != 0;
			if (!blocked)
				blocked = hasFeatureAt(cx, cy, kDirNorth) || hasFeatureAt(cx, cy + 1, kDirSouth);
			if (!blocked) {
				_visibleCell[cx][cy + 1] = true;
				queueX[tail] = cx;
				queueY[tail] = cy + 1;
				tail++;
			}
		}
		// South: (cx, cy-1)
		if (cy - 1 >= 0 && !_visibleCell[cx][cy - 1]) {
			bool blocked = (wallAt(cx, cy) & 0x01) != 0;
			if (!blocked)
				blocked = hasFeatureAt(cx, cy, kDirSouth) || hasFeatureAt(cx, cy - 1, kDirNorth);
			if (!blocked) {
				_visibleCell[cx][cy - 1] = true;
				queueX[tail] = cx;
				queueY[tail] = cy - 1;
				tail++;
			}
		}
		// East: (cx+1, cy)
		if (cx + 1 < 32 && !_visibleCell[cx + 1][cy]) {
			bool blocked = (wallAt(cx + 1, cy) & 0x02) != 0;
			if (!blocked)
				blocked = hasFeatureAt(cx, cy, kDirEast) || hasFeatureAt(cx + 1, cy, kDirWest);
			if (!blocked) {
				_visibleCell[cx + 1][cy] = true;
				queueX[tail] = cx + 1;
				queueY[tail] = cy;
				tail++;
			}
		}
		// West: (cx-1, cy)
		if (cx - 1 >= 0 && !_visibleCell[cx - 1][cy]) {
			bool blocked = (wallAt(cx, cy) & 0x02) != 0;
			if (!blocked)
				blocked = hasFeatureAt(cx, cy, kDirWest) || hasFeatureAt(cx - 1, cy, kDirEast);
			if (!blocked) {
				_visibleCell[cx - 1][cy] = true;
				queueX[tail] = cx - 1;
				queueY[tail] = cy;
				tail++;
			}
		}
	}
}

void ColonyEngine::renderCorridor3D() {
	computeVisibleCells();

	bool lit = (_corePower[_coreIndex] > 0);
	bool macMode = (_renderMode == Common::kRenderMacintosh);

	uint32 wallFill, wallLine, floorColor, ceilColor;

	if (macMode && _hasMacColors) {
		if (lit) {
			// Mac Display(): wallColor = cColor[c_char0+level-1].f (level-specific color).
			// SuperPoly(c_lwall) uses wallColor as fill, giving all walls the level tint.
			// c_char0 = index 8 in Color256.
			wallFill = packMacColor(_macColors[8 + _level - 1].fg);
			wallLine = 0xFF000000; // black outlines
			floorColor = packMacColor(_macColors[7].fg);   // c_lwall fg (darker gray)
			ceilColor = packMacColor(_macColors[7].bg);    // c_lwall bg (light gray)
		} else {
			wallFill = packMacColor(_macColors[6].bg);     // c_dwall bg
			wallLine = packMacColor(_macColors[6].fg);     // c_dwall fg
			floorColor = wallFill;
			ceilColor = wallFill;
		}
	} else {
		// Mac B&W: walls are pure white (c_dwall=WHITE); EGA: light gray (7)
		wallFill = lit ? (macMode ? 255 : 7) : 0;
		wallLine = lit ? 0 : (macMode ? 255 : 7);
		floorColor = macMode ? (lit ? 255 : 0) : wallFill;
		ceilColor  = macMode ? (lit ? 255 : 0) : wallFill;
	}

	_gfx->begin3D(_me.xloc, _me.yloc, 0, _me.look, _me.lookY, _screenR);
	_gfx->clear(ceilColor);

	uint32 wallColor = wallLine;

	// --- Phase 1: Background (floor + ceiling) ---
	// No depth test or write  these are pure background, everything overwrites them.
	_gfx->setDepthState(false, false);

	// Draw large floor and ceiling quads.
	// Mac Display(): EraseRect fills ceiling with c_lwall.bg and floor with c_lwall.fg.
	// Set wireframe fill to each surface's own color so they aren't all wallFill.
	_gfx->setWireframe(true, floorColor);
	_gfx->draw3DQuad(-100000.0f, -100000.0f, -160.0f,
	                100000.0f, -100000.0f, -160.0f,
	                100000.0f, 100000.0f, -160.0f,
	                -100000.0f, 100000.0f, -160.0f, floorColor);

	_gfx->setWireframe(true, ceilColor);
	_gfx->draw3DQuad(-100000.0f, -100000.0f, 160.0f,
	                100000.0f, -100000.0f, 160.0f,
	                100000.0f, 100000.0f, 160.0f,
	                -100000.0f, 100000.0f, 160.0f, ceilColor);

	// Ceiling grid (Cuadricule) - DOS wireframe mode only.
	// Mac color mode: original corridor renderer only showed ceiling edges at wall
	// boundaries (via 2D perspective), not a full-map grid. Wall tops from draw3DWall
	// already provide the ceiling lines where walls exist.
	if (!(macMode && _hasMacColors)) {
		for (int i = 0; i <= 32; i++) {
			float d = i * 256.0f;
			float maxD = 32.0f * 256.0f;
			float zCeil = 160.0f;

			_gfx->draw3DLine(d, 0.0f, zCeil, d, maxD, zCeil, wallColor);
			_gfx->draw3DLine(0.0f, d, zCeil, maxD, d, zCeil, wallColor);
		}
	}

	// --- Phase 2: Walls ---
	// Depth test + write enabled. Pushed-back depth range so features/objects beat walls.
	_gfx->setDepthState(true, true);
	_gfx->setDepthRange(0.01, 1.0);
	_gfx->setWireframe(true, wallFill);

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

	// --- Phase 3: Wall & cell features ---
	// Closer depth range than walls  features always beat their own wall surface.
	// Depth test still active so far-away features are hidden behind nearer walls.
	_gfx->setDepthRange(0.005, 1.0);
	drawWallFeatures3D();

	// --- Phase 4: Objects ---
	// Full depth range  objects beat walls and features at the same distance.
	_gfx->setDepthRange(0.0, 1.0);

	// F7 toggles object fill.
	// EGA: default is filled (wall background); F7 = outline-only (see-through).
	// Mac: default is per-surface fill; F7 = "Fast mode" (outline-only).
	if (_wireframe) {
		_gfx->setWireframe(true); // No fill = outline-only objects
	}
	drawStaticObjects();
	// Always restore wall fill after objects.
	// Mac mode's draw3DPrism changes fill per surface; must reset for subsequent rendering.
	_gfx->setWireframe(true, wallFill);

	_gfx->end3D();
	_gfx->setWireframe(false);
}

} // End of namespace Colony
