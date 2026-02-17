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

// Color lookup: maps ObjColor constant → LINECOLOR (EGA palette index)
// From DOS ROBOCOLR.C lsColor[index][LINECOLOR] (field 4).
// Default DOS mode is polyfill=False: objects are wireframe-only, no polygon fills.
// The fill color stays as the wall background; only outline color varies per surface.
static uint8 lookupLineColor(int colorIdx) {
	switch (colorIdx) {
	case kColorClear:     return 0;  // Transparent (skipped before drawing)
	case kColorBlack:     return 0;  // vBLACK
	case kColorDkGray:    return 8;  // vDKGRAY
	case kColorLtGreen:   return 10; // vLTGREEN
	case kColorBath:      return 0;  // vBLACK
	case kColorWater:     return 1;  // vBLUE
	case kColorSilver:    return 1;  // vBLUE
	case kColorReactor:   return 0;  // vBLACK (LINEFILLCOLOR for fill mode)
	case kColorBlanket:   return 2;  // vGREEN
	case kColorSheet:     return 0;  // vBLACK (LINEFILLCOLOR for fill mode)
	case kColorBed:       return 6;  // vBROWN
	case kColorBox:       return 6;  // vBROWN
	case kColorChair:     return 1;  // vBLUE
	case kColorChairBase: return 1;  // vBLUE
	case kColorCouch:     return 4;  // vRED
	case kColorConsole:   return 4;  // vRED
	case kColorTV:        return 6;  // vBROWN
	case kColorTVScreen:  return 8;  // vDKGRAY
	case kColorDrawer:    return 6;  // vBROWN
	case kColorDesk:      return 6;  // vBROWN
	case kColorDeskTop:   return 6;  // vBROWN
	case kColorDeskChair: return 2;  // vGREEN
	case kColorMac:       return 0;  // vBLACK
	case kColorMacScreen: return 8;  // vDKGRAY
	case kColorCryo:      return 1;  // vBLUE
	case kColorCryoGlass: return 1;  // vBLUE
	case kColorCryoBase:  return 1;  // vBLUE
	case kColorForklift:  return 14; // vYELLOW
	case kColorTread1:    return 14; // vYELLOW
	case kColorTread2:    return 14; // vYELLOW
	case kColorPot:       return 6;  // vBROWN
	case kColorPlant:     return 2;  // vGREEN
	case kColorPower:     return 1;  // vBLUE
	case kColorPBase:     return 1;  // vBLUE
	case kColorPSource:   return 4;  // vRED
	case kColorTable:     return 6;  // vBROWN
	case kColorTableBase: return 6;  // vBROWN
	case kColorPStand:    return 5;  // vMAGENTA
	case kColorPLens:     return 0;  // vBLACK
	case kColorProjector: return 3;  // vCYAN
	case kColorTele:      return 4;  // vRED
	case kColorTeleDoor:  return 1;  // vBLUE
	case kColorWall:      return 0;  // vBLACK
	case kColorRainbow1:  return 4;  // vRED
	case kColorRainbow2:  return 14; // vYELLOW
	default:              return 0;  // vBLACK
	}
}

// Mac Classic dither patterns (from colorize.c cColor[].pattern).
// Mac Classic was a 1-bit B&W display. QuickDraw used 8x8 dither patterns
// to simulate grayscale: WHITE, LTGRAY, GRAY, DKGRAY, BLACK, CLEAR.
// Rendered via GL_POLYGON_STIPPLE: two-pass fill (white bg + black fg with stipple mask).
enum MacPattern {
	kPatternWhite  = 0, // Solid white (all background)
	kPatternLtGray = 1, // 25% foreground (sparse black dots on white)
	kPatternGray   = 2, // 50% foreground (checkerboard)
	kPatternDkGray = 3, // 75% foreground (dense black, sparse white)
	kPatternBlack  = 4, // Solid black (all foreground)
	kPatternClear  = 5  // Outline only (no fill)
};

// GL_POLYGON_STIPPLE patterns: 32x32 bit arrays (128 bytes), tiled from 8x8 Mac patterns.
// Bit=1 → fragment drawn (black foreground), bit=0 → fragment discarded (white background shows).
// Mac QuickDraw convention: bit=1 = foreground (BLACK), bit=0 = background (WHITE).
static const byte kStippleLtGray[128] = {
	// 0x88,0x22 alternating rows = 25% coverage (sparse dots)
	0x88,0x88,0x88,0x88, 0x22,0x22,0x22,0x22, 0x88,0x88,0x88,0x88, 0x22,0x22,0x22,0x22,
	0x88,0x88,0x88,0x88, 0x22,0x22,0x22,0x22, 0x88,0x88,0x88,0x88, 0x22,0x22,0x22,0x22,
	0x88,0x88,0x88,0x88, 0x22,0x22,0x22,0x22, 0x88,0x88,0x88,0x88, 0x22,0x22,0x22,0x22,
	0x88,0x88,0x88,0x88, 0x22,0x22,0x22,0x22, 0x88,0x88,0x88,0x88, 0x22,0x22,0x22,0x22,
	0x88,0x88,0x88,0x88, 0x22,0x22,0x22,0x22, 0x88,0x88,0x88,0x88, 0x22,0x22,0x22,0x22,
	0x88,0x88,0x88,0x88, 0x22,0x22,0x22,0x22, 0x88,0x88,0x88,0x88, 0x22,0x22,0x22,0x22,
	0x88,0x88,0x88,0x88, 0x22,0x22,0x22,0x22, 0x88,0x88,0x88,0x88, 0x22,0x22,0x22,0x22,
	0x88,0x88,0x88,0x88, 0x22,0x22,0x22,0x22, 0x88,0x88,0x88,0x88, 0x22,0x22,0x22,0x22,
};

static const byte kStippleGray[128] = {
	// 0xAA,0x55 alternating rows = 50% coverage (checkerboard)
	0xAA,0xAA,0xAA,0xAA, 0x55,0x55,0x55,0x55, 0xAA,0xAA,0xAA,0xAA, 0x55,0x55,0x55,0x55,
	0xAA,0xAA,0xAA,0xAA, 0x55,0x55,0x55,0x55, 0xAA,0xAA,0xAA,0xAA, 0x55,0x55,0x55,0x55,
	0xAA,0xAA,0xAA,0xAA, 0x55,0x55,0x55,0x55, 0xAA,0xAA,0xAA,0xAA, 0x55,0x55,0x55,0x55,
	0xAA,0xAA,0xAA,0xAA, 0x55,0x55,0x55,0x55, 0xAA,0xAA,0xAA,0xAA, 0x55,0x55,0x55,0x55,
	0xAA,0xAA,0xAA,0xAA, 0x55,0x55,0x55,0x55, 0xAA,0xAA,0xAA,0xAA, 0x55,0x55,0x55,0x55,
	0xAA,0xAA,0xAA,0xAA, 0x55,0x55,0x55,0x55, 0xAA,0xAA,0xAA,0xAA, 0x55,0x55,0x55,0x55,
	0xAA,0xAA,0xAA,0xAA, 0x55,0x55,0x55,0x55, 0xAA,0xAA,0xAA,0xAA, 0x55,0x55,0x55,0x55,
	0xAA,0xAA,0xAA,0xAA, 0x55,0x55,0x55,0x55, 0xAA,0xAA,0xAA,0xAA, 0x55,0x55,0x55,0x55,
};

static const byte kStippleDkGray[128] = {
	// 0x77,0xDD alternating rows = 75% coverage (dense dots)
	0x77,0x77,0x77,0x77, 0xDD,0xDD,0xDD,0xDD, 0x77,0x77,0x77,0x77, 0xDD,0xDD,0xDD,0xDD,
	0x77,0x77,0x77,0x77, 0xDD,0xDD,0xDD,0xDD, 0x77,0x77,0x77,0x77, 0xDD,0xDD,0xDD,0xDD,
	0x77,0x77,0x77,0x77, 0xDD,0xDD,0xDD,0xDD, 0x77,0x77,0x77,0x77, 0xDD,0xDD,0xDD,0xDD,
	0x77,0x77,0x77,0x77, 0xDD,0xDD,0xDD,0xDD, 0x77,0x77,0x77,0x77, 0xDD,0xDD,0xDD,0xDD,
	0x77,0x77,0x77,0x77, 0xDD,0xDD,0xDD,0xDD, 0x77,0x77,0x77,0x77, 0xDD,0xDD,0xDD,0xDD,
	0x77,0x77,0x77,0x77, 0xDD,0xDD,0xDD,0xDD, 0x77,0x77,0x77,0x77, 0xDD,0xDD,0xDD,0xDD,
	0x77,0x77,0x77,0x77, 0xDD,0xDD,0xDD,0xDD, 0x77,0x77,0x77,0x77, 0xDD,0xDD,0xDD,0xDD,
	0x77,0x77,0x77,0x77, 0xDD,0xDD,0xDD,0xDD, 0x77,0x77,0x77,0x77, 0xDD,0xDD,0xDD,0xDD,
};

// Lookup: MacPattern enum → GL stipple data pointer (null for solid/clear patterns)
static const byte *kMacStippleData[] = {
	nullptr,        // kPatternWhite  - solid white, no stipple
	kStippleLtGray, // kPatternLtGray - 25% black dots
	kStippleGray,   // kPatternGray   - 50% checkerboard
	kStippleDkGray, // kPatternDkGray - 75% black
	nullptr,        // kPatternBlack  - solid black, no stipple
	nullptr         // kPatternClear  - outline only
};

// Map ObjColor constant → Mac dither pattern.
// From colorize.c cColor[] array: ~90% of objects use GRAY,
// special cases: c_dwall=WHITE, c_lwall=LTGRAY, c_window=DKGRAY,
// c_desktop=WHITE, c_shelves=LTGRAY.
static int lookupMacPattern(int colorIdx) {
	switch (colorIdx) {
	case kColorClear:     return kPatternClear;
	case kColorBlack:     return kPatternBlack;
	case kColorWall:      return kPatternWhite;  // c_dwall = WHITE
	case kColorDeskTop:   return kPatternWhite;  // c_desktop = WHITE
	case kColorSheet:     return kPatternWhite;  // c_bedsheet = WHITE (bright surface)
	case kColorBath:      return kPatternWhite;  // c_bath = WHITE (porcelain)
	case kColorMac:       return kPatternWhite;  // c_computer = WHITE (bright casing)
	case kColorSilver:    return kPatternLtGray; // c_mirror = LTGRAY
	case kColorReactor:   return kPatternLtGray; // c_reactor = LTGRAY
	case kColorTVScreen:  return kPatternDkGray; // c_tvscreen = DKGRAY
	case kColorMacScreen: return kPatternDkGray; // c_screen = DKGRAY
	case kColorWater:     return kPatternDkGray; // c_water = DKGRAY
	default:              return kPatternGray;   // Most objects = GRAY
	}
}

// DOS object geometry constants
static const int kScreenPts[8][3] = {
	{-16, 64, 0}, {16, 64, 0}, {16, -64, 0}, {-16, -64, 0},
	{-16, 64, 288}, {16, 64, 288}, {16, -64, 288}, {-16, -64, 288}
};
static const int kScreenSurf[4][8] = {
	{kColorBlack, 4, 0, 3, 7, 4, 0, 0}, {kColorBlack, 4, 3, 2, 6, 7, 0, 0},
	{kColorBlack, 4, 1, 0, 4, 5, 0, 0}, {kColorBlack, 4, 2, 1, 5, 6, 0, 0}
};
static const int kTableTopPts[4][3] = {
	{-128, 128, 100}, {128, 128, 100}, {128, -128, 100}, {-128, -128, 100}
};
static const int kTableTopSurf[1][8] = {{kColorTable, 4, 3, 2, 1, 0, 0, 0}};
static const int kTableBasePts[8][3] = {
	{-5, 5, 0}, {5, 5, 0}, {5, -5, 0}, {-5, -5, 0},
	{-5, 5, 100}, {5, 5, 100}, {5, -5, 100}, {-5, -5, 100}
};
static const int kTableBaseSurf[4][8] = {
	{kColorTableBase, 4, 0, 3, 7, 4, 0, 0}, {kColorTableBase, 4, 3, 2, 6, 7, 0, 0},
	{kColorTableBase, 4, 1, 0, 4, 5, 0, 0}, {kColorTableBase, 4, 2, 1, 5, 6, 0, 0}
};
static const int kBedPostPts[4][3] = {
	{-82, 128, 100}, {82, 128, 100}, {82, 128, 0}, {-82, 128, 0}
};
static const int kBedPostSurf[1][8] = {{kColorBed, 4, 3, 2, 1, 0, 0, 0}};
static const int kBedBlanketPts[8][3] = {
	{-80, 70, 0}, {80, 70, 0}, {80, -175, 0}, {-80, -175, 0},
	{-80, 70, 60}, {80, 70, 60}, {80, -175, 60}, {-80, -175, 60}
};
static const int kBlanketSurf[4][8] = {
	{kColorBlanket, 4, 0, 3, 7, 4, 0, 0}, {kColorBlanket, 4, 3, 2, 6, 7, 0, 0},
	{kColorBlanket, 4, 2, 1, 5, 6, 0, 0}, {kColorBlanket, 4, 7, 6, 5, 4, 0, 0}
};
static const int kBedSheetPts[8][3] = {
	{-80, 128, 30}, {80, 128, 30}, {80, 70, 30}, {-80, 70, 30},
	{-80, 128, 60}, {80, 128, 60}, {80, 70, 60}, {-80, 70, 60}
};
static const int kSheetSurf[3][8] = {
	{kColorSheet, 4, 0, 3, 7, 4, 0, 0}, {kColorSheet, 4, 2, 1, 5, 6, 0, 0},
	{kColorSheet, 4, 7, 6, 5, 4, 0, 0}
};
static const int kBBedBlanketPts[8][3] = {
	{-128, 70, 0}, {128, 70, 0}, {128, -175, 0}, {-128, -175, 0},
	{-128, 70, 60}, {128, 70, 60}, {128, -175, 60}, {-128, -175, 60}
};
static const int kBBedSheetPts[8][3] = {
	{-128, 128, 30}, {128, 128, 30}, {128, 70, 30}, {-128, 70, 30},
	{-128, 128, 60}, {128, 128, 60}, {128, 70, 60}, {-128, 70, 60}
};
static const int kBBedPostPts[4][3] = {
	{-130, 128, 100}, {130, 128, 100}, {130, 128, 0}, {-130, 128, 0}
};
static const int kDeskTopPts[4][3] = {
	{-150, 110, 100}, {150, 110, 100}, {150, -110, 100}, {-150, -110, 100}
};
static const int kDeskTopSurf[1][8] = {{kColorDeskTop, 4, 3, 2, 1, 0, 0, 0}};
static const int kDeskLeftPts[8][3] = {
	{-135, 95, 0}, {-55, 95, 0}, {-55, -95, 0}, {-135, -95, 0},
	{-135, 95, 100}, {-55, 95, 100}, {-55, -95, 100}, {-135, -95, 100}
};
static const int kDeskRightPts[8][3] = {
	{55, 95, 0}, {135, 95, 0}, {135, -95, 0}, {55, -95, 0},
	{55, 95, 100}, {135, 95, 100}, {135, -95, 100}, {55, -95, 100}
};
static const int kDeskCabSurf[4][8] = {
	{kColorDesk, 4, 0, 3, 7, 4, 0, 0}, {kColorDesk, 4, 3, 2, 6, 7, 0, 0},
	{kColorDesk, 4, 1, 0, 4, 5, 0, 0}, {kColorDesk, 4, 2, 1, 5, 6, 0, 0}
};
static const int kSeatPts[4][3] = {
	{-40, 210, 60}, {40, 210, 60}, {40, 115, 60}, {-40, 115, 60}
};
static const int kSeatSurf[1][8] = {{kColorDeskChair, 4, 3, 2, 1, 0, 0, 0}};
static const int kArmLeftPts[4][3] = {
	{-40, 210, 90}, {-40, 210, 0}, {-40, 115, 0}, {-40, 115, 90}
};
static const int kArmRightPts[4][3] = {
	{40, 210, 90}, {40, 210, 0}, {40, 115, 0}, {40, 115, 90}
};
static const int kArmSurf[2][8] = {
	{kColorClear, 4, 3, 2, 1, 0, 0, 0}, {kColorClear, 4, 0, 1, 2, 3, 0, 0}
};
static const int kBackPts[4][3] = {
	{-40, 210, 130}, {40, 210, 130}, {40, 210, 70}, {-40, 210, 70}
};
static const int kBackSurf[2][8] = {
	{kColorDeskChair, 4, 3, 2, 1, 0, 0, 0}, {kColorDeskChair, 4, 0, 1, 2, 3, 0, 0}
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
	{kColorMac, 4, 7, 6, 5, 4, 0, 0}, {kColorMac, 4, 0, 3, 7, 4, 0, 0},
	{kColorMac, 4, 3, 2, 6, 7, 0, 0}, {kColorMac, 4, 1, 0, 4, 5, 0, 0},
	{kColorMac, 4, 2, 1, 5, 6, 0, 0}
};
static const int kDeskScreenPts[4][3] = {
	{80, 20, 125}, {110, 20, 125}, {110, 20, 150}, {80, 20, 150}
};
static const int kDeskScreenSurf[1][8] = {{kColorMacScreen, 4, 3, 2, 1, 0, 0, 0}};
static const int kCSeatPts[4][3] = {
	{-40, 40, 60}, {40, 40, 60}, {40, -40, 60}, {-40, -40, 60}
};
static const int kCSeatSurf[1][8] = {{kColorChair, 4, 3, 2, 1, 0, 0, 0}};
static const int kCArmLeftPts[4][3] = {
	{-50, 40, 90}, {-40, 40, 60}, {-40, -40, 60}, {-50, -40, 90}
};
static const int kCArmLeftSurf[1][8] = {{kColorChair, 4, 3, 2, 1, 0, 0, 0}};
static const int kCArmRightPts[4][3] = {
	{50, 40, 90}, {40, 40, 60}, {40, -40, 60}, {50, -40, 90}
};
static const int kCArmRightSurf[1][8] = {{kColorChair, 4, 3, 2, 1, 0, 0, 0}};
static const int kCBackPts[4][3] = {
	{-20, 60, 150}, {20, 60, 150}, {40, 40, 60}, {-40, 40, 60}
};
static const int kCBackSurf[2][8] = {
	{kColorChair, 4, 3, 2, 1, 0, 0, 0}, {kColorChair, 4, 0, 1, 2, 3, 0, 0}
};
static const int kCBasePts[8][3] = {
	{-5, 5, 0}, {5, 5, 0}, {5, -5, 0}, {-5, -5, 0},
	{-5, 5, 60}, {5, 5, 60}, {5, -5, 60}, {-5, -5, 60}
};
static const int kCBaseSurf[4][8] = {
	{kColorChairBase, 4, 0, 3, 7, 4, 0, 0}, {kColorChairBase, 4, 3, 2, 6, 7, 0, 0},
	{kColorChairBase, 4, 1, 0, 4, 5, 0, 0}, {kColorChairBase, 4, 2, 1, 5, 6, 0, 0}
};
static const int kConsolePts[8][3] = {
	{-5, 5, 0}, {5, 5, 0}, {5, -5, 0}, {-5, -5, 0},
	{-100, 70, 100}, {-35, 70, 140}, {-35, -70, 140}, {-100, -70, 100}
};
static const int kConsoleSurf[5][8] = {
	{kColorConsole, 4, 4, 0, 3, 7, 0, 0}, {kColorConsole, 4, 7, 3, 2, 6, 0, 0},
	{kColorConsole, 4, 5, 1, 0, 4, 0, 0}, {kColorConsole, 4, 6, 2, 1, 5, 0, 0},
	{kColorBlack, 4, 7, 6, 5, 4, 0, 0}
};
static const int kCouchSurf[5][8] = {
	{kColorCouch, 4, 0, 3, 7, 4, 0, 0}, {kColorCouch, 4, 3, 2, 6, 7, 0, 0},
	{kColorCouch, 4, 1, 0, 4, 5, 0, 0}, {kColorCouch, 4, 2, 1, 5, 6, 0, 0},
	{kColorCouch, 4, 7, 6, 5, 4, 0, 0}
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
	{kColorTV, 4, 0, 3, 7, 4, 0, 0}, {kColorTV, 4, 3, 2, 6, 7, 0, 0},
	{kColorTV, 4, 1, 0, 4, 5, 0, 0}, {kColorTV, 4, 2, 1, 5, 6, 0, 0},
	{kColorTV, 4, 7, 6, 5, 4, 0, 0}
};
static const int kTVScreenPts[4][3] = {
	{30, 50, 10}, {30, -50, 10}, {30, 50, 110}, {30, -50, 110}
};
static const int kTVScreenSurf[1][8] = {{kColorTVScreen, 4, 1, 0, 2, 3, 0, 0}};
static const int kDrawerPts[8][3] = {
	{-80, 70, 0}, {0, 70, 0}, {0, -70, 0}, {-80, -70, 0},
	{-80, 70, 100}, {0, 70, 100}, {0, -70, 100}, {-80, -70, 100}
};
static const int kDrawerSurf[5][8] = {
	{kColorDrawer, 4, 0, 3, 7, 4, 0, 0}, {kColorDrawer, 4, 3, 2, 6, 7, 0, 0},
	{kColorDrawer, 4, 1, 0, 4, 5, 0, 0}, {kColorDrawer, 4, 2, 1, 5, 6, 0, 0},
	{kColorDrawer, 4, 7, 6, 5, 4, 0, 0}
};
static const int kMirrorPts[4][3] = {
	{-80, 65, 100}, {-80, -65, 100}, {-80, 65, 210}, {-80, -65, 210}
};
static const int kMirrorSurf[1][8] = {{kColorSilver, 4, 1, 0, 2, 3, 0, 0}};

// Bathtub geometry
static const int kTubPts[8][3] = {
	{-128, 128,  0}, {   0, 128,  0}, {   0,-128,  0}, {-128,-128,  0},
	{-128, 128, 70}, {   0, 128, 70}, {   0,-128, 70}, {-128,-128, 70}
};
static const int kTubSurf[5][8] = {
	{kColorBath, 4, 0, 3, 7, 4, 0, 0}, {kColorBath, 4, 3, 2, 6, 7, 0, 0},
	{kColorBath, 4, 1, 0, 4, 5, 0, 0}, {kColorBath, 4, 2, 1, 5, 6, 0, 0},
	{kColorBath, 4, 7, 6, 5, 4, 0, 0}
};
static const int kDTubPts[6][3] = {
	{-16, 112, 70}, {-8, 0, 70}, {-16, -112, 70}, {-112, -112, 70}, {-120, 0, 70}, {-112, 112, 70}
};
static const int kDTubSurf[1][8] = {{kColorWater, 6, 5, 4, 3, 2, 1, 0}};

// Toilet geometry
static const int kAToiletPts[8][3] = {
	{-128, 45, 30}, {-100, 45, 30}, {-100, -45, 30}, {-128, -45, 30},
	{-128, 45, 100}, {-100, 45, 100}, {-100, -45, 100}, {-128, -45, 100}
};
static const int kAToiletSurf[5][8] = {
	{kColorBath, 4, 0, 3, 7, 4, 0, 0}, {kColorBath, 4, 3, 2, 6, 7, 0, 0},
	{kColorBath, 4, 1, 0, 4, 5, 0, 0}, {kColorBath, 4, 2, 1, 5, 6, 0, 0},
	{kColorBath, 4, 7, 6, 5, 4, 0, 0}
};
static const int kBToiletPts[12][3] = {
	{-100, 20, 50}, {-60, 40, 50}, {-20, 20, 50}, {-20, -20, 50}, {-60, -40, 50}, {-100, -20, 50},
	{-80, 10,  0}, {-60, 20,  0}, {-40, 10,  0}, {-40, -10,  0}, {-60, -20,  0}, {-80, -10,  0}
};
static const int kBToiletSurf[7][8] = {
	{kColorBath, 4, 0, 1, 7, 6, 0, 0}, {kColorBath, 4, 1, 2, 8, 7, 0, 0}, {kColorBath, 4, 2, 3, 9, 8, 0, 0},
	{kColorBath, 4, 3, 4, 10, 9, 0, 0}, {kColorBath, 4, 4, 5, 11, 10, 0, 0}, {kColorBath, 4, 5, 0, 6, 11, 0, 0},
	{kColorBath, 6, 5, 4, 3, 2, 1, 0}
};
static const int kCToiletPts[6][3] = {
	{-95, 15, 50}, {-60, 35, 50}, {-25, 15, 50}, {-25, -15, 50}, {-60, -35, 50}, {-95, -15, 50}
};
static const int kCToiletSurf[1][8] = {{kColorWater, 6, 5, 4, 3, 2, 1, 0}};
static const int kDToiletPts[6][3] = {
	{-100, 20, 50}, {-100, 40, 90}, {-100, 20, 130}, {-100, -20, 130}, {-100, -40, 90}, {-100, -20, 50}
};
static const int kDToiletSurf[1][8] = {{kColorLtGreen, 6, 5, 4, 3, 2, 1, 0}};
static const int kEToiletPts[4][3] = {
	{-128,-128, 20}, {-128,-128, 200}, { 128,-128, 200}, { 128,-128, 20}
};
static const int kEToiletSurf[1][8] = {{kColorDkGray, 4, 0, 1, 2, 3, 0, 0}};

// Sink geometry
static const int kSinkPts[8][3] = {
	{-128, 50, 70}, {-50, 50, 90}, {-50,-50, 90}, {-128,-50, 70},
	{-128, 50, 110}, {-50, 50, 110}, {-50,-50, 110}, {-128,-50, 110}
};
static const int kSinkSurf[5][8] = {
	{kColorBath, 4, 0, 3, 7, 4, 0, 0}, {kColorBath, 4, 3, 2, 6, 7, 0, 0},
	{kColorBath, 4, 1, 0, 4, 5, 0, 0}, {kColorBath, 4, 2, 1, 5, 6, 0, 0},
	{kColorBath, 4, 7, 6, 5, 4, 0, 0}
};
static const int kDSinkPts[6][3] = {
	{-55, 0, 110}, {-60, -45, 110}, {-118, -45, 110}, {-123, 0, 110}, {-118, 45, 110}, {-60, 45, 110}
};
static const int kDSinkSurf[1][8] = {{kColorWater, 6, 5, 4, 3, 2, 1, 0}};
static const int kSinkMirrorPts[4][3] = {
	{-128, 65, 130}, {-128, -65, 130}, {-128, 65, 250}, {-128, -65, 250}
};
static const int kSinkMirrorSurf[1][8] = {{kColorSilver, 4, 1, 0, 2, 3, 0, 0}};

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
	{4, kCBackPts, 1, kCBackSurf},
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

static const int kFWallPts[4][3] = {
	{-128, 128, 0}, {128, -128, 0}, {-128, 128, 320}, {128, -128, 320}
};
static const int kFWallSurf[1][8] = {{kColorWall, 4, 2, 3, 1, 0, 0, 0}};
static const Colony::ColonyEngine::PrismPartDef kFWallPart = {4, kFWallPts, 1, kFWallSurf};

static const int kCWallPts[8][3] = {
	{-128, 128, 0}, {0, 112, 0}, {112, 0, 0}, {128, -128, 0},
	{-128, 128, 320},  {0, 112, 320},  {112, 0, 320},  {128, -128, 320}
};
static const int kCWallSurf[3][8] = {
	{kColorWall, 4, 1, 0, 4, 5, 0, 0}, {kColorWall, 4, 2, 1, 5, 6, 0, 0}, {kColorWall, 4, 3, 2, 6, 7, 0, 0}
};
static const Colony::ColonyEngine::PrismPartDef kCWallPart = {8, kCWallPts, 3, kCWallSurf};

static const int kPlantPotPts[12][3] = {
	{10, 17, 40}, {20, 0, 40}, {10, -17, 40}, {-10, -17, 40}, {-20, 0, 40}, {-10, 17, 40},
	{8, 12, 0}, {15, 0, 0}, {8, -12, 0}, {-8, -12, 0}, {-15, 0, 0}, {-8, 12, 0}
};
static const int kPlantPotSurf[6][8] = {
	{kColorPot, 4, 0, 1, 7, 6, 0, 0}, {kColorPot, 4, 1, 2, 8, 7, 0, 0}, {kColorPot, 4, 2, 3, 9, 8, 0, 0},
	{kColorPot, 4, 3, 4, 10, 9, 0, 0}, {kColorPot, 4, 4, 5, 11, 10, 0, 0}, {kColorPot, 4, 5, 0, 6, 11, 0, 0}
};
static const int kPlantTopPotPts[6][3] = {
	{10, 17, 40}, {20, 0, 40}, {10, -17, 40}, {-10, -17, 40}, {-20, 0, 40}, {-10, 17, 40}
};
static const int kPlantTopPotSurf[1][8] = {{kColorDkGray, 6, 5, 4, 3, 2, 1, 0}};

static const int kPlantLeaf0Pts[3][3] = {{0, 0, 0}, {20, 20, 150}, {70, 70, 100}};
static const int kPlantLeaf1Pts[3][3] = {{0, 0, 0}, {-20, 30, 100}, {-60, 50, 50}};
static const int kPlantLeaf2Pts[3][3] = {{0, 0, 0}, {-20, -10, 70}, {-90, -70, 50}};
static const int kPlantLeaf3Pts[3][3] = {{0, 0, 0}, {20, -10, 50}, {90, -70, 70}};
static const int kPlantLeaf4Pts[3][3] = {{0, 0, 0}, {20, -30, 190}, {40, -60, 150}};
static const int kPlantLeaf5Pts[3][3] = {{0, 0, 0}, {20, -10, 130}, {50, -80, 200}};

static const int kPlantLeafSurf[2][8] = {{kColorClear, 3, 0, 1, 2, 0, 0, 0}, {kColorClear, 3, 2, 1, 0, 0, 0, 0}};

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
	{kColorBox, 4, 0, 3, 7, 4, 0, 0}, {kColorBox, 4, 3, 2, 6, 7, 0, 0},
	{kColorBox, 4, 1, 0, 4, 5, 0, 0}, {kColorBox, 4, 2, 1, 5, 6, 0, 0}, {kColorBox, 4, 7, 6, 5, 4, 0, 0}
};
static const int kBox2Pts[8][3] = {
	{-50, 50, 100}, {50, 50, 100}, {50, -50, 100}, {-50, -50, 100},
	{-50, 50, 200}, {50, 50, 200}, {50, -50, 200}, {-50, -50, 200}
};

static const int kReactorCorePts[12][3] = {
	{-40, 20, 288}, {0, 40, 288}, {40, 20, 288}, {40, -20, 288}, {0, -40, 288}, {-40, -20, 288},
	{-40, 20, 32}, {0, 40, 32}, {40, 20, 32}, {40, -20, 32}, {0, -40, 32}, {-40, -20, 32}
};
static const int kReactorCoreSurf[7][8] = {
	{kColorReactor, 4, 0, 1, 7, 6, 0, 0}, {kColorReactor, 4, 1, 2, 8, 7, 0, 0}, {kColorReactor, 4, 2, 3, 9, 8, 0, 0},
	{kColorReactor, 4, 3, 4, 10, 9, 0, 0}, {kColorReactor, 4, 4, 5, 11, 10, 0, 0}, {kColorReactor, 4, 5, 0, 6, 11, 0, 0},
	{kColorReactor, 6, 5, 4, 3, 2, 1, 0}
};
static const int kReactorBasePts[8][3] = {
	{-128, 128, 0}, {128, 128, 0}, {128, -128, 0}, {-128, -128, 0},
	{-128, 128, 32}, {128, 128, 32}, {128, -128, 32}, {-128, -128, 32}
};
static const int kReactorBaseSurf[6][8] = {
	{kColorRainbow1, 4, 0, 3, 7, 4, 0, 0}, {kColorRainbow1, 4, 3, 2, 6, 7, 0, 0}, {kColorRainbow1, 4, 1, 0, 4, 5, 0, 0},
	{kColorRainbow1, 4, 2, 1, 5, 6, 0, 0}, {kColorRainbow1, 4, 7, 6, 5, 4, 0, 0}, {kColorRainbow1, 4, 0, 1, 2, 3, 0, 0}
};
static const int kReactorTopPts[8][3] = {
	{-128, 128, 288}, {128, 128, 288}, {128, -128, 288}, {-128, -128, 288},
	{-128, 128, 320}, {128, 128, 320}, {128, -128, 320}, {-128, -128, 320}
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

// Power Suit: triangular prism body + small rectangular pedestal + flat table + hexagonal power source
// DOS INITOBJ.C: 5 prism parts. Floor=160, so 2*Floor=320.
static const int kPowerTopPts[3][3] = {{-150, 120, 320}, {150, 120, 320}, {0, -150, 320}};
static const int kPowerTopSurf[1][8] = {{kColorPower, 3, 0, 1, 2, 0, 0, 0}};

static const int kPowerBottomPts[3][3] = {{-150, 120, 0}, {150, 120, 0}, {0, -150, 0}};
static const int kPowerBottomSurf[1][8] = {{kColorPower, 3, 2, 1, 0, 0, 0, 0}};

static const int kPowerBasePts[8][3] = {
	{-5, 100, 0}, {5, 100, 0}, {5, 90, 0}, {-5, 90, 0},
	{-5, 100, 100}, {5, 100, 100}, {5, 90, 100}, {-5, 90, 100}
};
static const int kPowerBaseSurf[4][8] = {
	{kColorPBase, 4, 0, 3, 7, 4, 0, 0}, {kColorPBase, 4, 3, 2, 6, 7, 0, 0},
	{kColorPBase, 4, 1, 0, 4, 5, 0, 0}, {kColorPBase, 4, 2, 1, 5, 6, 0, 0}
};

static const int kPowerTablePts[4][3] = {{-50, 135, 100}, {50, 135, 100}, {50, 55, 100}, {-50, 55, 100}};
static const int kPowerTableSurf[1][8] = {{kColorBlack, 4, 3, 2, 1, 0, 0, 0}};

static const int kPowerSourcePts[12][3] = {
	{-75, 0, 290}, {-35, 60, 290}, {35, 60, 290}, {75, 0, 290}, {35, -60, 290}, {-35, -60, 290},
	{-75, 0, 320}, {-35, 60, 320}, {35, 60, 320}, {75, 0, 320}, {35, -60, 320}, {-35, -60, 320}
};
static const int kPowerSourceSurf[7][8] = {
	{kColorRainbow1, 6, 0, 1, 2, 3, 4, 5},
	{kColorPSource, 4, 0, 6, 7, 1, 0, 0}, {kColorPSource, 4, 1, 7, 8, 2, 0, 0},
	{kColorPSource, 4, 2, 8, 9, 3, 0, 0}, {kColorPSource, 4, 3, 9, 10, 4, 0, 0},
	{kColorPSource, 4, 4, 10, 11, 5, 0, 0}, {kColorPSource, 4, 5, 11, 6, 0, 0, 0}
};

static const Colony::ColonyEngine::PrismPartDef kPowerSuitParts[5] = {
	{3, kPowerTopPts, 1, kPowerTopSurf},
	{3, kPowerBottomPts, 1, kPowerBottomSurf},
	{8, kPowerBasePts, 4, kPowerBaseSurf},
	{4, kPowerTablePts, 1, kPowerTableSurf},
	{12, kPowerSourcePts, 7, kPowerSourceSurf}
};

// Cryo tube: top (coffin-shaped lid) + base
static const int kCryoTopPts[16][3] = {
	{-130,  50,  80}, { 130,  50,  80}, { 130, -50,  80}, {-130, -50,  80},
	{-130,  50, 140}, { 130,  50, 140}, { 130, -50, 140}, {-130, -50, 140},
	{   0,  50, 140}, {   0, -50, 140},
	{-140,  70, 110}, { 140,  70, 110}, { 140, -70, 110}, {-140, -70, 110},
	{   0,  70, 110}, {   0, -70, 110}
};
static const int kCryoTopSurf[12][8] = {
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
static const int kCryoBasePts[8][3] = {
	{-130,  50,  0}, { 130,  50,  0}, { 130, -50,  0}, {-130, -50,  0},
	{-130,  50, 80}, { 130,  50, 80}, { 130, -50, 80}, {-130, -50, 80}
};
static const int kCryoBaseSurf[5][8] = {
	{kColorCryoBase, 4, 0, 3, 7, 4, 0, 0}, {kColorCryoBase, 4, 3, 2, 6, 7, 0, 0},
	{kColorCryoBase, 4, 1, 0, 4, 5, 0, 0}, {kColorCryoBase, 4, 2, 1, 5, 6, 0, 0},
	{kColorCryo,     4, 7, 6, 5, 4, 0, 0}
};
static const Colony::ColonyEngine::PrismPartDef kCryoParts[2] = {
	{16, kCryoTopPts, 12, kCryoTopSurf},
	{8, kCryoBasePts, 5, kCryoBaseSurf}
};

// Forklift: cab + treads + upper-left arm + lower-left fork + upper-right arm + lower-right fork
static const int kFLCabPts[14][3] = {
	{-35, 60, 40}, {35, 60, 40}, {35, -60, 40}, {-35, -60, 40},
	{-15, 60, 260}, {15, 60, 260}, {15, -60, 260}, {-15, -60, 260},
	{25, 60, 140}, {25, -60, 140},
	{70, 35, 120}, {70, -35, 120},
	{-70, 40, 80}, {-70, -40, 80}
};
static const int kFLCabSurf[12][8] = {
	{kColorForklift, 4, 0, 3, 13, 12, 0, 0}, {kColorForklift, 4, 12, 13, 7, 4, 0, 0},
	{kColorForklift, 3, 0, 12, 4, 0, 0, 0},  {kColorForklift, 3, 3, 7, 13, 0, 0, 0},
	{kColorForklift, 4, 3, 2, 6, 7, 0, 0},   {kColorForklift, 4, 1, 0, 4, 5, 0, 0},
	{kColorForklift, 3, 1, 8, 10, 0, 0, 0},  {kColorForklift, 3, 2, 11, 9, 0, 0, 0},
	{kColorForklift, 4, 1, 10, 11, 2, 0, 0},
	{kColorSilver,   3, 8, 5, 10, 0, 0, 0},  {kColorSilver, 3, 11, 6, 9, 0, 0, 0},
	{kColorSilver,   4, 10, 5, 6, 11, 0, 0}
};
static const int kFLTreadPts[12][3] = {
	{-60, 60, 20}, {60, 60, 20}, {60, -60, 20}, {-60, -60, 20},
	{-35, 60, 40}, {35, 60, 40}, {35, -60, 40}, {-35, -60, 40},
	{-35, 60, 0}, {35, 60, 0}, {35, -60, 0}, {-35, -60, 0}
};
static const int kFLTreadSurf[6][8] = {
	{kColorTread1, 4, 0, 3, 7, 4, 0, 0},
	{kColorTread2, 6, 3, 11, 10, 2, 6, 7},
	{kColorTread2, 6, 0, 4, 5, 1, 9, 8},
	{kColorTread1, 4, 2, 1, 5, 6, 0, 0},
	{kColorTread1, 4, 0, 8, 11, 3, 0, 0},
	{kColorTread1, 4, 10, 9, 1, 2, 0, 0}
};
static const int kFLULPts[8][3] = {
	{-15, 70, 120}, {15, 70, 120}, {15, 60, 120}, {-15, 60, 120},
	{-25, 70, 230}, {25, 70, 230}, {25, 60, 230}, {-25, 60, 230}
};
static const int kFLArmSurf[4][8] = {
	{kColorForklift, 4, 0, 3, 7, 4, 0, 0}, {kColorForklift, 4, 3, 2, 6, 7, 0, 0},
	{kColorForklift, 4, 1, 0, 4, 5, 0, 0}, {kColorForklift, 4, 2, 1, 5, 6, 0, 0}
};
static const int kFLLLPts[8][3] = {
	{-15, 80, 120}, {100, 80, 125}, {100, 70, 125}, {-15, 70, 120},
	{-15, 80, 150}, {100, 80, 140}, {100, 70, 140}, {-15, 70, 150}
};
static const int kFLForkSurf[6][8] = {
	{kColorForklift, 4, 0, 3, 7, 4, 0, 0}, {kColorForklift, 4, 3, 2, 6, 7, 0, 0},
	{kColorForklift, 4, 1, 0, 4, 5, 0, 0}, {kColorBlack,    4, 2, 1, 5, 6, 0, 0},
	{kColorForklift, 4, 7, 6, 5, 4, 0, 0}, {kColorForklift, 4, 0, 1, 2, 3, 0, 0}
};
static const int kFLURPts[8][3] = {
	{-15, -60, 120}, {15, -60, 120}, {15, -70, 120}, {-15, -70, 120},
	{-25, -60, 230}, {25, -60, 230}, {25, -70, 230}, {-25, -70, 230}
};
static const int kFLLRPts[8][3] = {
	{-15, -70, 120}, {100, -70, 125}, {100, -80, 125}, {-15, -80, 120},
	{-15, -70, 150}, {100, -70, 140}, {100, -80, 140}, {-15, -80, 150}
};
static const Colony::ColonyEngine::PrismPartDef kForkliftParts[6] = {
	{14, kFLCabPts, 12, kFLCabSurf},
	{12, kFLTreadPts, 6, kFLTreadSurf},
	{8, kFLULPts, 4, kFLArmSurf},
	{8, kFLLLPts, 6, kFLForkSurf},
	{8, kFLURPts, 4, kFLArmSurf},
	{8, kFLLRPts, 6, kFLForkSurf}
};

// Teleport: octagonal booth with flared middle
static const int kTelePts[24][3] = {
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
static const int kTeleSurf[16][8] = {
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
static const Colony::ColonyEngine::PrismPartDef kTelePart = {24, kTelePts, 16, kTeleSurf};

// Projector: body + stand + lens (sits on table)
static const int kProjectorPts[8][3] = {
	{-30, 30, 140}, {30, 30, 140}, {30, -30, 140}, {-30, -30, 140},
	{-20, 30, 160}, {30, 30, 160}, {30, -30, 160}, {-20, -30, 160}
};
static const int kProjectorSurf[5][8] = {
	{kColorProjector, 4, 0, 3, 7, 4, 0, 0}, {kColorProjector, 4, 3, 2, 6, 7, 0, 0},
	{kColorProjector, 4, 1, 0, 4, 5, 0, 0}, {kColorProjector, 4, 2, 1, 5, 6, 0, 0},
	{kColorProjector, 4, 7, 6, 5, 4, 0, 0}
};
static const int kPStandPts[4][3] = {
	{-25, 50, 100}, {0, 10, 140}, {0, -10, 140}, {-25, -50, 100}
};
static const int kPStandSurf[1][8] = {{kColorPStand, 4, 0, 1, 2, 3, 0, 0}};
static const int kPLensPts[12][3] = {
	{30,  8, 154}, {30,  0, 158}, {30, -8, 154}, {30, -8, 146}, {30,  0, 142}, {30,  8, 146},
	{55, 10, 155}, {55,  0, 160}, {55,-10, 155}, {55,-10, 145}, {55,  0, 140}, {55, 10, 145}
};
static const int kPLensSurf[7][8] = {
	{kColorPLens, 4, 0, 1, 7, 6, 0, 0}, {kColorPLens, 4, 1, 2, 8, 7, 0, 0},
	{kColorPLens, 4, 2, 3, 9, 8, 0, 0}, {kColorPLens, 4, 3, 4, 10, 9, 0, 0},
	{kColorPLens, 4, 4, 5, 11, 10, 0, 0}, {kColorPLens, 4, 5, 0, 6, 11, 0, 0},
	{kColorBlack, 6, 6, 7, 8, 9, 10, 11}
};
static const Colony::ColonyEngine::PrismPartDef kProjectorParts[3] = {
	{8, kProjectorPts, 5, kProjectorSurf},
	{4, kPStandPts, 1, kPStandSurf},
	{12, kPLensPts, 7, kPLensSurf}
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

void ColonyEngine::draw3DPrism(const Thing &obj, const PrismPartDef &def, bool useLook) {
	// +32 compensates for the original sine table's 45° phase offset.
	// Object angles from game data were stored assuming that offset.
	const uint8 ang = (useLook ? obj.where.look : obj.where.ang) + 32;
	const long rotCos = _cost[ang];
	const long rotSin = _sint[ang];
	const bool lit = (_corePower[_coreIndex] > 0);

	for (int i = 0; i < def.surfaceCount; i++) {
		const int colorIdx = def.surfaces[i][0];
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

			if (lit) {
				if (_renderMode == Common::kRenderMacintosh) {
					// Mac B&W: stipple dither pattern fill + black outline
					int pattern = lookupMacPattern(colorIdx);
					if (pattern == kPatternClear) continue;
					if (!_wireframe) {
						_gfx->setWireframe(true, pattern == kPatternBlack ? 0 : 255);
					}
					_gfx->setStippleData(kMacStippleData[pattern]);
					_gfx->draw3DPolygon(px, py, pz, count, 0); // black outline
					_gfx->setStippleData(nullptr);
				} else {
					// EGA: global wall fill, per-surface colored outline
					_gfx->draw3DPolygon(px, py, pz, count, (uint32)lookupLineColor(colorIdx));
				}
			} else {
				// Unlit: same for both modes — black fill, white outline
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
	// Mac B&W: black outlines; EGA: green outlines; unlit: white
	const uint32 color = lit ? (_renderMode == Common::kRenderMacintosh ? 0 : 2) : 15;

	for (int i = 0; i < def.surfaceCount; i++) {
		const int n = def.surfaces[i][1];
		if (n < 2) continue;

		float px[8], py[8], pz[8];
		int count = 0;

		for (int j = 0; j < n; j++) {
			const int cur = def.surfaces[i][j + 2];
			if (cur < 0 || cur >= def.pointCount) continue;
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
	// This limits visibility to the current "room" — doors, shelves, etc.
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

void ColonyEngine::drawStaticObjects() {
	for (uint i = 0; i < _objects.size(); i++) {
		const Thing &obj = _objects[i];
		if (!obj.alive)
			continue;
		int ox = obj.where.xindex;
		int oy = obj.where.yindex;
		if (ox < 0 || ox >= 32 || oy < 0 || oy >= 32 || !_visibleCell[ox][oy])
			continue;
		drawStaticObjectPrisms3D(obj);
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
		// Mac: fill arrow polygon with BLACK.
		// drawchar() sets cColor[cc].pattern=WHITE (all background) and
		// background is {0,0,0}=BLACK, so SuperPoly fills solid black.
		// Arrow shapes are concave — GL_POLYGON only handles convex polys.
		// Decompose into convex parts: triangle head + rectangle shaft.
		if (_renderMode == Common::kRenderMacintosh) {
			_gfx->setWireframe(true, 0); // BLACK fill
			if (cnum == 'b') {
				// Right arrow head: (0,3)-(3,0)-(3,6)
				float hu[3] = {u[0], u[1], u[6]};
				float hv[3] = {v[0], v[1], v[6]};
				wallPolygon(corners, hu, hv, 3, 0);
				// Right arrow shaft: (3,2)-(6,2)-(6,4)-(3,4)
				float su[4] = {u[2], u[3], u[4], u[5]};
				float sv[4] = {v[2], v[3], v[4], v[5]};
				wallPolygon(corners, su, sv, 4, 0);
			} else if (cnum == 'c') {
				// Left arrow shaft: (0,2)-(0,4)-(3,4)-(3,2)
				float su[4] = {u[0], u[1], u[2], u[6]};
				float sv[4] = {v[0], v[1], v[2], v[6]};
				wallPolygon(corners, su, sv, 4, 0);
				// Left arrow head: (3,6)-(6,3)-(3,0)
				float hu[3] = {u[3], u[4], u[5]};
				float hv[3] = {v[3], v[4], v[5]};
				wallPolygon(corners, hu, hv, 3, 0);
			}
			_gfx->setWireframe(true, 255); // restore white wall fill
		}
		for (int i = 0; i < count; i++) {
			int n = (i + 1) % count;
			wallLine(corners, u[i], v[i], u[n], v[n], 0); // vBLACK
		}
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

	// DOS uses color_wall (PenColor) for hole outlines.
	// In our inverted 3D renderer: lit=black outlines on white fill, dark=white on black.
	bool lit = (_corePower[_coreIndex] > 0);
	uint32 holeColor = lit ? 0 : 7;

	switch (map[0]) {
	case 1: // SMHOLEFLR
	case 3: // SMHOLECEIL
	{
		// DOS floor1hole/ceil1hole: hole spans 25%-75% of cell in each dimension
		// Matching the CCenter proximity trigger zone (64..192 of 256)
		float u[4] = {0.25f, 0.75f, 0.75f, 0.25f};
		float v[4] = {0.25f, 0.25f, 0.75f, 0.75f};
		wallPolygon(corners, u, v, 4, holeColor);
		break;
	}
	case 2: // LGHOLEFLR
	case 4: // LGHOLECEIL
	{
		// DOS floor2hole/ceil2hole: full-cell hole
		float u[4] = {0.0f, 1.0f, 1.0f, 0.0f};
		float v[4] = {0.0f, 0.0f, 1.0f, 1.0f};
		wallPolygon(corners, u, v, 4, holeColor);
		break;
	}
	case 5: // HOTFOOT — DOS draws X pattern (two diagonals), not a rectangle
	{
		wallLine(corners, 0.0f, 0.0f, 1.0f, 1.0f, holeColor); // front-left to back-right
		wallLine(corners, 1.0f, 0.0f, 0.0f, 1.0f, holeColor); // front-right to back-left
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

	// Backface culling: only draw features for the side facing the camera.
	// This prevents backside decorations (like Level 2 lines) from bleeding through.
	// We use non-inclusive comparisons so features remain visible while standing on the boundary.
	switch (direction) {
	case kDirNorth: if (_me.yloc > (cellY + 1) * 256) return; break;
	case kDirSouth: if (_me.yloc < cellY * 256) return;       break;
	case kDirWest:  if (_me.xloc < cellX * 256) return;       break;
	case kDirEast:  if (_me.xloc > (cellX + 1) * 256) return; break;
	default: break;
	}
	
	float corners[4][3];
	getWallFace3D(cellX, cellY, direction, corners);

	switch (map[0]) {
	case kWallFeatureDoor: {
		// EGA: vDKGRAY outlines; Mac B&W: black outlines + gray fills
		const bool macMode = (_renderMode == Common::kRenderMacintosh);
		const uint32 doorColor = macMode ? 0 : 8; // Mac: black, EGA: vDKGRAY
		bool shipLevel = (_level == 1 || _level == 5 || _level == 6);

		if (shipLevel) {
			static const float u_ss[8] = { 0.375f, 0.250f, 0.250f, 0.375f, 0.625f, 0.750f, 0.750f, 0.625f };
			static const float v_ss[8] = { 0.125f, 0.250f, 0.750f, 0.875f, 0.875f, 0.750f, 0.250f, 0.125f };

			if (macMode) {
				if (map[1] != 0) {
					// Closed: fill octagon (c_bulkhead = GRAY stipple)
					_gfx->setStippleData(kStippleGray);
					wallPolygon(corners, u_ss, v_ss, 8, 0);
					_gfx->setStippleData(nullptr);
				} else {
					// Open: fill with BLACK (passable opening)
					_gfx->setWireframe(true, 0);
					wallPolygon(corners, u_ss, v_ss, 8, 0);
					_gfx->setWireframe(true, 255);
				}
			}

			for (int i = 0; i < 8; i++)
				wallLine(corners, u_ss[i], v_ss[i], u_ss[(i + 1) % 8], v_ss[(i + 1) % 8], doorColor);

			if (map[1] != 0) {
				wallLine(corners, 0.375f, 0.25f, 0.375f, 0.75f, doorColor);
				wallLine(corners, 0.375f, 0.75f, 0.625f, 0.75f, doorColor);
				wallLine(corners, 0.625f, 0.75f, 0.625f, 0.25f, doorColor);
				wallLine(corners, 0.625f, 0.25f, 0.375f, 0.25f, doorColor);
			}
		} else {
			static const float xl = 0.25f, xr = 0.75f;
			static const float yb = 0.0f, yt = 0.875f;

			if (macMode) {
				float ud[4] = {xl, xr, xr, xl};
				float vd[4] = {yb, yb, yt, yt};
				if (map[1] != 0) {
					// Closed: fill (c_door = GRAY stipple)
					_gfx->setStippleData(kStippleGray);
					wallPolygon(corners, ud, vd, 4, 0);
					_gfx->setStippleData(nullptr);
				} else {
					// Open: fill with BLACK (passable opening)
					_gfx->setWireframe(true, 0);
					wallPolygon(corners, ud, vd, 4, 0);
					_gfx->setWireframe(true, 255);
				}
			}

			wallLine(corners, xl, yb, xl, yt, doorColor);
			wallLine(corners, xl, yt, xr, yt, doorColor);
			wallLine(corners, xr, yt, xr, yb, doorColor);
			wallLine(corners, xr, yb, xl, yb, doorColor);

			if (map[1] != 0) {
				wallLine(corners, 0.3125f, 0.4375f, 0.6875f, 0.4375f, doorColor);
			}
		}
		break;
	}
	case kWallFeatureWindow: {
		const bool macMode = (_renderMode == Common::kRenderMacintosh);
		const uint32 winColor = macMode ? 0 : 8; // Mac: black, EGA: vDKGRAY
		float xl = 0.25f, xr = 0.75f;
		float yb = 0.25f, yt = 0.75f;
		float xc = 0.5f, yc = 0.5f;

		// Mac: fill window pane (c_window = DKGRAY stipple)
		if (macMode) {
			_gfx->setStippleData(kStippleDkGray);
			float uw[4] = {xl, xr, xr, xl};
			float vw[4] = {yb, yb, yt, yt};
			wallPolygon(corners, uw, vw, 4, 0);
			_gfx->setStippleData(nullptr);
		}

		wallLine(corners, xl, yb, xl, yt, winColor);
		wallLine(corners, xl, yt, xr, yt, winColor);
		wallLine(corners, xr, yt, xr, yb, winColor);
		wallLine(corners, xr, yb, xl, yb, winColor);
		wallLine(corners, xc, yb, xc, yt, winColor);
		wallLine(corners, xl, yc, xr, yc, winColor);
		break;
	}
	case kWallFeatureShelves: {
		// DOS drawbooks: recessed bookcase with 3D depth.
		const bool macMode = (_renderMode == Common::kRenderMacintosh);
		const uint32 shelfColor = macMode ? 0 : 8;

		// Mac: fill shelves area (c_shelves = LTGRAY stipple)
		if (macMode) {
			_gfx->setStippleData(kStippleLtGray);
			float us[4] = {0.0f, 1.0f, 1.0f, 0.0f};
			float vs[4] = {0.0f, 0.0f, 1.0f, 1.0f};
			wallPolygon(corners, us, vs, 4, 0);
			_gfx->setStippleData(nullptr);
		}
		float bx = 0.1875f, bxr = 0.8125f;
		float by = 0.1875f, byt = 0.8125f;
		// Back face rectangle
		wallLine(corners, bx, by, bxr, by, shelfColor);
		wallLine(corners, bxr, by, bxr, byt, shelfColor);
		wallLine(corners, bxr, byt, bx, byt, shelfColor);
		wallLine(corners, bx, byt, bx, by, shelfColor);
		// Connecting lines (front corners to back corners)
		wallLine(corners, 0.0f, 0.0f, bx, by, shelfColor);
		wallLine(corners, 0.0f, 1.0f, bx, byt, shelfColor);
		wallLine(corners, 1.0f, 0.0f, bxr, by, shelfColor);
		wallLine(corners, 1.0f, 1.0f, bxr, byt, shelfColor);
		// 7 shelf lines across front face (DOS split7 at 1/8..7/8 intervals)
		for (int i = 1; i <= 7; i++) {
			float v = (float)i / 8.0f;
			wallLine(corners, 0.0f, v, 1.0f, v, shelfColor);
		}
		break;
	}
	case kWallFeatureUpStairs: {
		// DOS: draw_up_stairs — staircase ascending into the wall with perspective
		const uint32 col = 0; // vBLACK

		// Mac: fill entire wall face (c_upstairs = GRAY stipple)
		if (_renderMode == Common::kRenderMacintosh) {
			_gfx->setStippleData(kStippleGray);
			float uf[4] = {0.0f, 1.0f, 1.0f, 0.0f};
			float vf[4] = {0.0f, 0.0f, 1.0f, 1.0f};
			wallPolygon(corners, uf, vf, 4, 0);
			_gfx->setStippleData(nullptr);
		}

		// Perspective convergence: back of passage at ~1/3 width (1 cell deep)
		float ul[7], ur[7], vf[7], vc[7];
		for (int i = 0; i < 7; i++) {
			float f = (i + 1) / 8.0f;
			float inset = f * (1.0f / 3.0f);
			ul[i] = inset;
			ur[i] = 1.0f - inset;
			vf[i] = inset;        // floor rises toward center
			vc[i] = 1.0f - inset; // ceiling drops toward center
		}
		// Step height: at depth d, step s is at fraction (s+1)/8 from floor to ceiling
		auto vh = [&](int d, int s) -> float {
			return vf[d] + (s + 1) / 8.0f * (vc[d] - vf[d]);
		};
		// Back of passage (full depth)
		float bi = 1.0f / 3.0f; // back inset
		float bu = bi, bur = 1.0f - bi, bvc = 1.0f - bi;

		// 1. Side wall verticals at back of passage
		wallLine(corners, bu, bvc, bu, 0.5f, col);
		wallLine(corners, bur, 0.5f, bur, bvc, col);

		// 2. Back wall landing (depth 6 to full depth)
		wallLine(corners, ul[6], vh(6, 6), bu, bvc, col);
		wallLine(corners, bu, bvc, bur, bvc, col);
		wallLine(corners, bur, bvc, ur[6], vh(6, 6), col);
		wallLine(corners, ur[6], vh(6, 6), ul[6], vh(6, 6), col);

		// 3. First step tread (floor from wall face to depth 0)
		wallLine(corners, 0.0f, 0.0f, ul[0], vf[0], col);
		wallLine(corners, ul[0], vf[0], ur[0], vf[0], col);
		wallLine(corners, ur[0], vf[0], 1.0f, 0.0f, col);
		wallLine(corners, 1.0f, 0.0f, 0.0f, 0.0f, col);

		// 4. First step riser (at depth 0)
		wallLine(corners, ul[0], vh(0, 0), ul[0], vf[0], col);
		wallLine(corners, ur[0], vf[0], ur[0], vh(0, 0), col);
		wallLine(corners, ur[0], vh(0, 0), ul[0], vh(0, 0), col);

		// 5. Step treads (i=3..0: depth i to depth i+1)
		for (int i = 3; i >= 0; i--) {
			wallLine(corners, ul[i], vh(i, i), ul[i + 1], vh(i + 1, i), col);
			wallLine(corners, ul[i + 1], vh(i + 1, i), ur[i + 1], vh(i + 1, i), col);
			wallLine(corners, ur[i + 1], vh(i + 1, i), ur[i], vh(i, i), col);
			wallLine(corners, ur[i], vh(i, i), ul[i], vh(i, i), col);
		}

		// 6. Step risers (i=5..0: vertical face at depth i+1)
		for (int i = 5; i >= 0; i--) {
			wallLine(corners, ul[i + 1], vh(i + 1, i + 1), ul[i + 1], vh(i + 1, i), col);
			wallLine(corners, ul[i + 1], vh(i + 1, i), ur[i + 1], vh(i + 1, i), col);
			wallLine(corners, ur[i + 1], vh(i + 1, i), ur[i + 1], vh(i + 1, i + 1), col);
			wallLine(corners, ur[i + 1], vh(i + 1, i + 1), ul[i + 1], vh(i + 1, i + 1), col);
		}

		// 7. Handrails: from center of wall edges up to near-ceiling at mid-depth
		wallLine(corners, 0.0f, 0.5f, ul[3], vc[0], col);
		wallLine(corners, 1.0f, 0.5f, ur[3], vc[0], col);
		break;
	}
	case kWallFeatureDnStairs: {
		// DOS: draw_dn_stairs — staircase descending into the wall with perspective
		const uint32 col = 0; // vBLACK

		// Mac: fill entire wall face (c_dnstairs = GRAY stipple)
		if (_renderMode == Common::kRenderMacintosh) {
			_gfx->setStippleData(kStippleGray);
			float uf[4] = {0.0f, 1.0f, 1.0f, 0.0f};
			float vf[4] = {0.0f, 0.0f, 1.0f, 1.0f};
			wallPolygon(corners, uf, vf, 4, 0);
			_gfx->setStippleData(nullptr);
		}

		float ul[7], ur[7], vf[7], vc[7];
		for (int i = 0; i < 7; i++) {
			float f = (i + 1) / 8.0f;
			float inset = f * (1.0f / 3.0f);
			ul[i] = inset;
			ur[i] = 1.0f - inset;
			vf[i] = inset;
			vc[i] = 1.0f - inset;
		}
		float bi = 1.0f / 3.0f;
		float bu = bi, bur = 1.0f - bi;

		// 1. Ceiling: front ceiling slopes down to mid-depth
		wallLine(corners, 0.0f, 1.0f, ul[3], vc[3], col);
		wallLine(corners, ul[3], vc[3], ur[3], vc[3], col);
		wallLine(corners, ur[3], vc[3], 1.0f, 1.0f, col);

		// 2. Slant: from mid-depth ceiling down to center at back
		wallLine(corners, ul[3], vc[3], bu, 0.5f, col);
		wallLine(corners, bu, 0.5f, bur, 0.5f, col);
		wallLine(corners, bur, 0.5f, ur[3], vc[3], col);

		// 3. Side wall verticals: from center at back down to floor level
		wallLine(corners, bu, 0.5f, bu, vf[0], col);
		wallLine(corners, bur, 0.5f, bur, vf[0], col);

		// 4. First step (floor from wall face to depth 0)
		wallLine(corners, 0.0f, 0.0f, ul[0], vf[0], col);
		wallLine(corners, ul[0], vf[0], ur[0], vf[0], col);
		wallLine(corners, ur[0], vf[0], 1.0f, 0.0f, col);
		wallLine(corners, 1.0f, 0.0f, 0.0f, 0.0f, col);

		// 5. Handrails: from center of wall edges down to floor at mid-depth
		wallLine(corners, 0.0f, 0.5f, ul[3], vf[3], col);
		wallLine(corners, 1.0f, 0.5f, ur[3], vf[3], col);
		break;
	}
	case kWallFeatureChar:
		wallChar(corners, map[1]);
		break;
	case kWallFeatureGlyph: {
		// DOS wireframe: PenColor(realcolor[vDKGRAY]) = 8
		const bool macMode = (_renderMode == Common::kRenderMacintosh);
		const uint32 glyphColor = macMode ? 0 : 8;

		// Mac: fill glyph area (c_glyph = GRAY stipple)
		if (macMode) {
			_gfx->setStippleData(kStippleGray);
			float ug[4] = {0.0f, 1.0f, 1.0f, 0.0f};
			float vg[4] = {0.0f, 0.0f, 1.0f, 1.0f};
			wallPolygon(corners, ug, vg, 4, 0);
			_gfx->setStippleData(nullptr);
		}

		for (int i = 0; i < 7; i++) {
			float v = 0.2f + i * 0.1f;
			wallLine(corners, 0.2f, v, 0.8f, v, glyphColor);
		}
		break;
	}
	case kWallFeatureElevator: {
		const bool macMode = (_renderMode == Common::kRenderMacintosh);
		const uint32 elevColor = macMode ? 0 : 8; // Mac: black, EGA: vDKGRAY
		float xl = 0.2f, xr = 0.8f;
		float yb = 0.1f, yt = 0.9f;

		// Mac: fill elevator door (c_elevator = GRAY stipple)
		if (macMode) {
			_gfx->setStippleData(kStippleGray);
			float ue[4] = {xl, xr, xr, xl};
			float ve[4] = {yb, yb, yt, yt};
			wallPolygon(corners, ue, ve, 4, 0);
			_gfx->setStippleData(nullptr);
		}

		wallLine(corners, xl, yb, xl, yt, elevColor);
		wallLine(corners, xl, yt, xr, yt, elevColor);
		wallLine(corners, xr, yt, xr, yb, elevColor);
		wallLine(corners, xr, yb, xl, yb, elevColor);
		wallLine(corners, 0.5f, yb, 0.5f, yt, elevColor);
		break;
	}
	case kWallFeatureTunnel: {
		// Tunnel: hexagonal opening from Grid (0,0 0,5 1,6 5,6 6,5 6,0)
		static const float u_t[6] = { 0.0f,    0.0f,    1/6.0f,  5/6.0f,  1.0f,    1.0f };
		static const float v_t[6] = { 0.0f,    0.750f,  0.875f,  0.875f,  0.750f,  0.0f };
		if (_renderMode == Common::kRenderMacintosh) {
			// Mac: c_tunnel = GRAY stipple fill + black outline
			_gfx->setStippleData(kStippleGray);
			wallPolygon(corners, u_t, v_t, 6, 0);
			_gfx->setStippleData(nullptr);
		} else {
			wallPolygon(corners, u_t, v_t, 6, 0); // vBLACK outline
		}
		break;
	}
	case kWallFeatureAirlock: {
		const bool macMode = (_renderMode == Common::kRenderMacintosh);
		float pts[][2] = {{0.0f, 0.5f}, {0.15f, 0.85f}, {0.5f, 1.0f}, {0.85f, 0.85f},
		                  {1.0f, 0.5f}, {0.85f, 0.15f}, {0.5f, 0.0f}, {0.15f, 0.15f}};
		float u[8], v[8];
		for (int i = 0; i < 8; i++) {
			u[i] = 0.1f + pts[i][0] * 0.8f;
			v[i] = 0.1f + pts[i][1] * 0.8f;
		}
		if (map[1] == 0) {
			// Open: black fill (passable opening)
			_gfx->setWireframe(true, 0);
			wallPolygon(corners, u, v, 8, 0);
			bool lit = (_corePower[_coreIndex] > 0);
			_gfx->setWireframe(true, lit ? (macMode ? 255 : 7) : 0);
		} else {
			// Mac: fill airlock (c_airlock = GRAY stipple) when closed
			if (macMode) {
				_gfx->setStippleData(kStippleGray);
				wallPolygon(corners, u, v, 8, 0);
				_gfx->setStippleData(nullptr);
			}

			const uint32 airlockColor = macMode ? 0 : 8; // Mac: black, EGA: vDKGRAY
			for (int i = 0; i < 8; i++) {
				int n = (i + 1) % 8;
				wallLine(corners, u[i], v[i], u[n], v[n], airlockColor);
			}
			if (macMode) {
				// Mac: 8 radial lines from each vertex to center (drawALClosed)
				float cu = 0.5f, cv = 0.5f;
				for (int i = 0; i < 8; i++)
					wallLine(corners, u[i], v[i], cu, cv, airlockColor);
			} else {
				// EGA: simple crosshairs
				wallLine(corners, 0.1f, 0.5f, 0.9f, 0.5f, airlockColor);
				wallLine(corners, 0.5f, 0.1f, 0.5f, 0.9f, airlockColor);
			}
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
			if (!_visibleCell[x][y])
				continue;
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
	computeVisibleCells();

	bool lit = (_corePower[_coreIndex] > 0);
	bool macMode = (_renderMode == Common::kRenderMacintosh);

	// Mac B&W: walls are pure white (c_dwall=WHITE); EGA: light gray (7)
	uint32 wallFill = lit ? (macMode ? 255 : 7) : 0;
	uint32 wallLine = lit ? 0 : (macMode ? 255 : 7);

	// Walls always use wireframe with fill (opaque walls).
	_gfx->setWireframe(true, wallFill);

	_gfx->begin3D(_me.xloc, _me.yloc, 0, _me.look, _me.lookY, _screenR);
	_gfx->clear(wallFill);

	uint32 wallColor = wallLine;
	// Mac: floor = black (c_lwall foreground), ceiling = white (c_lwall background)
	// EGA: both use wallLine color
	uint32 floorColor = macMode ? (lit ? 0 : 255) : wallLine;
	uint32 ceilColor  = macMode ? (lit ? 255 : 0) : wallLine;

	// Draw large floor and ceiling quads
	_gfx->draw3DQuad(-100000.0f, -100000.0f, -160.1f,
	                100000.0f, -100000.0f, -160.1f,
	                100000.0f, 100000.0f, -160.1f,
	                -100000.0f, 100000.0f, -160.1f, floorColor);

	_gfx->draw3DQuad(-100000.0f, -100000.0f, 160.1f,
	                100000.0f, -100000.0f, 160.1f,
	                100000.0f, 100000.0f, 160.1f,
	                -100000.0f, 100000.0f, 160.1f, ceilColor);
 
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

bool ColonyEngine::drawStaticObjectPrisms3D(const Thing &obj) {
	switch (obj.type) {
	case kObjConsole:
		draw3DPrism(obj, kConsolePart, false);
		break;
	case kObjCChair:
		for (int i = 0; i < 5; i++)
			draw3DPrism(obj, kCChairParts[i], false);
		break;
	case kObjPlant:
		// DOS MakePlant draw order: top pot, then green leaf lines, then pot on top.
		draw3DPrism(obj, kPlantParts[1], false); // top pot (soil)
		for (int i = 2; i < 8; i++)
			draw3DLeaf(obj, kPlantParts[i]); // leaves as lines
		draw3DPrism(obj, kPlantParts[0], false); // pot (drawn last, on top)
		break;
	case kObjCouch:
	case kObjChair: {
		const PrismPartDef *parts = (obj.type == kObjCouch) ? kCouchParts : kChairParts;
		for (int i = 0; i < 4; i++)
			draw3DPrism(obj, parts[i], false);
		break;
	}
	case kObjTV:
		for (int i = 0; i < 2; i++)
			draw3DPrism(obj, kTVParts[i], false);
		break;
	case kObjDrawer:
		for (int i = 0; i < 2; i++)
			draw3DPrism(obj, kDrawerParts[i], false);
		break;
	case kObjFWall:
		draw3DPrism(obj, kFWallPart, false);
		break;
	case kObjCWall:
		draw3DPrism(obj, kCWallPart, false);
		break;
	case kObjScreen:
		draw3DPrism(obj, kScreenPart, false);
		break;
	case kObjTable:
		for (int i = 0; i < 2; i++)
			draw3DPrism(obj, kTableParts[i], false);
		break;
	case kObjBed:
	case kObjBBed: {
		const PrismPartDef *parts = (obj.type == kObjBBed) ? kBBedParts : kBedParts;
		for (int i = 0; i < 3; i++)
			draw3DPrism(obj, parts[i], false);
		break;
	}
	case kObjDesk:
		for (int i = 0; i < 10; i++)
			draw3DPrism(obj, kDeskParts[i], false);
		break;
	case kObjBox1:
		draw3DPrism(obj, kBox1Part, false);
		break;
	case kObjBox2:
		draw3DPrism(obj, kBox2Parts[1], false); // base first
		draw3DPrism(obj, kBox2Parts[0], false); // top second
		break;
	case kObjReactor:
		for (int i = 0; i < 3; i++)
			draw3DPrism(obj, kReactorParts[i], false);
		break;
	case kObjPowerSuit:
		for (int i = 0; i < 5; i++)
			draw3DPrism(obj, kPowerSuitParts[i], false);
		break;
	case kObjTeleport:
		draw3DPrism(obj, kTelePart, false);
		break;
	case kObjCryo:
		draw3DPrism(obj, kCryoParts[1], false); // base first
		draw3DPrism(obj, kCryoParts[0], false); // top second
		break;
	case kObjProjector:
		// Projector sits on table — draw table first, then projector parts
		for (int i = 0; i < 2; i++)
			draw3DPrism(obj, kTableParts[i], false);
		draw3DPrism(obj, kProjectorParts[1], false); // stand
		draw3DPrism(obj, kProjectorParts[0], false); // body
		draw3DPrism(obj, kProjectorParts[2], false); // lens
		break;
	case kObjTub:
		for (int i = 0; i < 2; i++)
			draw3DPrism(obj, kTubParts[i], false);
		break;
	case kObjSink:
		for (int i = 0; i < 3; i++)
			draw3DPrism(obj, kSinkParts[i], false);
		break;
	case kObjToilet:
		for (int i = 0; i < 4; i++)
			draw3DPrism(obj, kToiletParts[i], false);
		break;
	case kObjPToilet:
		for (int i = 0; i < 5; i++)
			draw3DPrism(obj, kPToiletParts[i], false);
		break;
	case kObjForkLift:
		// Default draw order: forks, arms, treads, cab (back-to-front)
		draw3DPrism(obj, kForkliftParts[3], false); // FLLL (left fork)
		draw3DPrism(obj, kForkliftParts[2], false); // FLUL (left arm)
		draw3DPrism(obj, kForkliftParts[5], false); // FLLR (right fork)
		draw3DPrism(obj, kForkliftParts[4], false); // FLUR (right arm)
		draw3DPrism(obj, kForkliftParts[1], false); // treads
		draw3DPrism(obj, kForkliftParts[0], false); // cab
		break;
	default:
		return false;
	}
	return true;
}

} // End of namespace Colony
