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
static const DOSColorEntry &lookupDOSColor(int colorIdx) {
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
	// Ring animation colors: cycle through DOS EGA colors
	// DOS reactor rings use color=1+count%5 → values 1-5 → cColor[1..5]
	case kColorRainbow1: return g_dosColors[2];  // BLUE
	case kColorRainbow2: return g_dosColors[3];  // GREEN
	case kColorRainbow3: return g_dosColors[4];  // CYAN
	case kColorRainbow4: return g_dosColors[5];  // RED
	default: return fallback;
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

// Internal-only surface color for Mac wall helper objects (FWALL/CWALL).
// In the original Mac renderer these use c_lwall/c_dwall, which are
// special corridor-wall colors rather than normal material palette entries.
static const int kColorCorridorWall = 1000;

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

// Map ObjColor → Mac B&W dither pattern (from ROBOCOLR.C MONOCHROME field).
// The monochrome field in the DOS table matches MacPattern enum values directly:
// WHITE=0, LTGRAY=1, GRAY=2, DKGRAY=3, BLACK=4, CLEAR=5.
static int lookupMacPattern(int colorIdx) {
	return lookupDOSColor(colorIdx).monochrome;
}

// Pack Mac 16-bit RGB into 32-bit ARGB with 0xFF000000 marker for direct RGB.
static uint32 packMacColor(const uint16 rgb[3]) {
	return 0xFF000000 | ((rgb[0] >> 8) << 16) | ((rgb[1] >> 8) << 8) | (rgb[2] >> 8);
}

// Map ObjColor constant → Mac Color256 index (cColor[] from colordef.h).
static int mapObjColorToMacColor(int colorIdx) {
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

// Helper: set up Mac color stipple rendering for a given cColor[] pattern.
// Configures setMacColors/setStippleData/setWireframe for the pattern type.
// Returns the stipple pointer (null for solid patterns)  caller must clear with setStippleData(nullptr).
static const byte *setupMacPattern(Renderer *gfx, int pattern, uint32 fg, uint32 bg) {
	const byte *stipple = (pattern >= 1 && pattern <= 3) ? kMacStippleData[pattern] : nullptr;
	if (stipple) {
		gfx->setMacColors(fg, bg);
		gfx->setStippleData(stipple);
		gfx->setWireframe(true, bg);
	} else if (pattern == 4) {
		// BLACK: solid fg fill
		gfx->setWireframe(true, fg);
	} else {
		// WHITE (0): solid bg fill + fg outline
		gfx->setWireframe(true, bg);
	}
	return stipple;
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
	{kColorCCore, 4, 0, 1, 7, 6, 0, 0}, {kColorCCore, 4, 1, 2, 8, 7, 0, 0}, {kColorCCore, 4, 2, 3, 9, 8, 0, 0},
	{kColorCCore, 4, 3, 4, 10, 9, 0, 0}, {kColorCCore, 4, 4, 5, 11, 10, 0, 0}, {kColorCCore, 4, 5, 0, 6, 11, 0, 0},
	{kColorCCore, 6, 5, 4, 3, 2, 1, 0}
};
static const int kReactorBasePts[8][3] = {
	{-128, 128, 0}, {128, 128, 0}, {128, -128, 0}, {-128, -128, 0},
	{-128, 128, 32}, {128, 128, 32}, {128, -128, 32}, {-128, -128, 32}
};
static const int kReactorBaseSurf[6][8] = {
	{kColorReactor, 4, 0, 3, 7, 4, 0, 0}, {kColorReactor, 4, 3, 2, 6, 7, 0, 0}, {kColorReactor, 4, 1, 0, 4, 5, 0, 0},
	{kColorReactor, 4, 2, 1, 5, 6, 0, 0}, {kColorReactor, 4, 7, 6, 5, 4, 0, 0}, {kColorReactor, 4, 0, 1, 2, 3, 0, 0}
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
// Bench: simple box (1 part). DOS INITOBJ.C InitBench.
static const int kBenchPts[8][3] = {
	{-60, 128, 0}, {60, 128, 0}, {60, -128, 0}, {-60, -128, 0},
	{-60, 128, 120}, {60, 128, 120}, {60, -128, 120}, {-60, -128, 120}
};
static const int kBenchSurf[5][8] = {
	{kColorBench, 4, 0, 3, 7, 4, 0, 0}, {kColorBench, 4, 3, 2, 6, 7, 0, 0},
	{kColorBench, 4, 1, 0, 4, 5, 0, 0}, {kColorBench, 4, 2, 1, 5, 6, 0, 0},
	{kColorBlack, 4, 7, 6, 5, 4, 0, 0}
};
static const Colony::ColonyEngine::PrismPartDef kBenchPart = {8, kBenchPts, 5, kBenchSurf};

// CBench: L-shaped corner bench (2 parts). DOS INITOBJ.C InitCBench.
// Part 0: slanted front face (front-left Y=60, front-right Y=-60)
static const int kCBenchPts[8][3] = {
	{-60, 60, 0}, {60, -60, 0}, {60, -128, 0}, {-60, -128, 0},
	{-60, 60, 120}, {60, -60, 120}, {60, -128, 120}, {-60, -128, 120}
};
// Part 1: wider perpendicular section
static const int kDBenchPts[8][3] = {
	{-60, 60, 0}, {128, 60, 0}, {128, -60, 0}, {60, -60, 0},
	{-60, 60, 120}, {128, 60, 120}, {128, -60, 120}, {60, -60, 120}
};
static const Colony::ColonyEngine::PrismPartDef kCBenchParts[2] = {
	{8, kCBenchPts, 5, kBenchSurf},
	{8, kDBenchPts, 5, kBenchSurf}
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

// ============================================================================
// Robot geometry data (from DOS PYRAMID.H, CUBE.H, EYE.H, UPYRAMID.H,
// QUEEN.H, DRONE.H, SNOOP.H)
// ============================================================================

// --- Pyramid (type 2) ---
static const int kPyramidPts[5][3] = {
	{-75, 75, 30}, {75, 75, 30}, {75, -75, 30}, {-75, -75, 30}, {0, 0, 200}
};
static const int kPyramidSurf[4][8] = {
	{kColorPyramid, 3, 1, 0, 4, 1, 0, 0}, {kColorPyramid, 3, 2, 1, 4, 2, 0, 0},
	{kColorPyramid, 3, 3, 2, 4, 3, 0, 0}, {kColorPyramid, 3, 0, 3, 4, 0, 0, 0}
};
static const int kPShadowPts[4][3] = {
	{-75, 75, 0}, {75, 75, 0}, {75, -75, 0}, {-75, -75, 0}
};
static const int kPShadowSurf[1][8] = {{kColorShadow, 4, 3, 2, 1, 0, 3, 0}};
// Pyramid eye (ball on top)
static const int kPIrisPts[4][3] = {
	{15, 0, 185}, {15, 15, 200}, {15, 0, 215}, {15, -15, 200}
};
static const int kPIrisSurf[1][8] = {{kColorIris, 4, 0, 1, 2, 3, 0, 0}};
static const int kPPupilPts[4][3] = {
	{16, 0, 194}, {16, 6, 200}, {16, 0, 206}, {16, -6, 200}
};
static const int kPPupilSurf[1][8] = {{kColorPupil, 4, 0, 1, 2, 3, 0, 0}};

static const Colony::ColonyEngine::PrismPartDef kPyramidBodyDef = {5, kPyramidPts, 4, kPyramidSurf};
static const Colony::ColonyEngine::PrismPartDef kPShadowDef = {4, kPShadowPts, 1, kPShadowSurf};
static const Colony::ColonyEngine::PrismPartDef kPIrisDef = {4, kPIrisPts, 1, kPIrisSurf};
static const Colony::ColonyEngine::PrismPartDef kPPupilDef = {4, kPPupilPts, 1, kPPupilSurf};

// --- Cube (type 3) --- (octahedron)
static const int kCubePts[6][3] = {
	{0, 0, 200}, {100, 0, 100}, {0, 100, 100}, {-100, 0, 100}, {0, -100, 100}, {0, 0, 0}
};
static const int kCubeSurf[8][8] = {
	{kColorCube, 3, 0, 1, 2, 0, 0, 0}, {kColorCube, 3, 0, 2, 3, 0, 0, 0},
	{kColorCube, 3, 0, 3, 4, 0, 0, 0}, {kColorCube, 3, 0, 4, 1, 0, 0, 0},
	{kColorCube, 3, 5, 2, 1, 5, 0, 0}, {kColorCube, 3, 5, 3, 2, 5, 0, 0},
	{kColorCube, 3, 5, 4, 3, 5, 0, 0}, {kColorCube, 3, 5, 1, 4, 5, 0, 0}
};
static const Colony::ColonyEngine::PrismPartDef kCubeBodyDef = {6, kCubePts, 8, kCubeSurf};

// --- UPyramid (type 4) --- (inverted pyramid)
static const int kUPyramidPts[5][3] = {
	{-75, 75, 190}, {75, 75, 190}, {75, -75, 190}, {-75, -75, 190}, {0, 0, 30}
};
static const int kUPyramidSurf[5][8] = {
	{kColorUPyramid, 3, 0, 1, 4, 0, 0, 0}, {kColorUPyramid, 3, 1, 2, 4, 1, 0, 0},
	{kColorUPyramid, 3, 2, 3, 4, 2, 0, 0}, {kColorUPyramid, 3, 3, 0, 4, 3, 0, 0},
	{kColorUPyramid, 4, 3, 2, 1, 0, 3, 0}
};
static const int kUPShadowPts[4][3] = {
	{-25, 25, 0}, {25, 25, 0}, {25, -25, 0}, {-25, -25, 0}
};
static const int kUPShadowSurf[1][8] = {{kColorShadow, 4, 3, 2, 1, 0, 3, 0}};

static const Colony::ColonyEngine::PrismPartDef kUPyramidBodyDef = {5, kUPyramidPts, 5, kUPyramidSurf};
static const Colony::ColonyEngine::PrismPartDef kUPShadowDef = {4, kUPShadowPts, 1, kUPShadowSurf};

// --- Eye (type 1) ---
// Ball is rendered by draw3DSphere(), not as a prism
static const int kEyeIrisPts[4][3] = {
	{60, 0, 140}, {60, 60, 200}, {60, 0, 260}, {60, -60, 200}
};
static const int kEyeIrisSurf[1][8] = {{kColorIris, 4, 0, 1, 2, 3, 0, 0}};
static const int kEyePupilPts[4][3] = {
	{66, 0, 175}, {66, 25, 200}, {66, 0, 225}, {66, -25, 200}
};
static const int kEyePupilSurf[1][8] = {{kColorBlack, 4, 0, 1, 2, 3, 0, 0}};

static const Colony::ColonyEngine::PrismPartDef kEyeIrisDef = {4, kEyeIrisPts, 1, kEyeIrisSurf};
static const Colony::ColonyEngine::PrismPartDef kEyePupilDef = {4, kEyePupilPts, 1, kEyePupilSurf};

// --- Floating Pyramid (type 6) --- egg on ground
static const int kFPyramidPts[5][3] = {
	{-75, 75, 0}, {75, 75, 0}, {75, -75, 0}, {-75, -75, 0}, {0, 0, 170}
};
static const Colony::ColonyEngine::PrismPartDef kFPyramidBodyDef = {5, kFPyramidPts, 4, kPyramidSurf};

// --- Small Pyramid (type 10) ---
static const int kSPyramidPts[5][3] = {
	{-40, 40, 0}, {40, 40, 0}, {40, -40, 0}, {-40, -40, 0}, {0, 0, 100}
};
static const Colony::ColonyEngine::PrismPartDef kSPyramidBodyDef = {5, kSPyramidPts, 4, kPyramidSurf};

// --- Mini Pyramid (type 14) ---
static const int kMPyramidPts[5][3] = {
	{-20, 20, 0}, {20, 20, 0}, {20, -20, 0}, {-20, -20, 0}, {0, 0, 50}
};
static const Colony::ColonyEngine::PrismPartDef kMPyramidBodyDef = {5, kMPyramidPts, 4, kPyramidSurf};

// --- Floating Cube (type 7) ---
static const int kFCubePts[6][3] = {
	{0, 0, 150}, {75, 0, 75}, {0, 75, 75}, {-75, 0, 75}, {0, -75, 75}, {0, 0, 0}
};
static const Colony::ColonyEngine::PrismPartDef kFCubeBodyDef = {6, kFCubePts, 8, kCubeSurf};

// --- Small Cube (type 11) ---
static const int kSCubePts[6][3] = {
	{0, 0, 100}, {50, 0, 50}, {0, 50, 50}, {-50, 0, 50}, {0, -50, 50}, {0, 0, 0}
};
static const Colony::ColonyEngine::PrismPartDef kSCubeBodyDef = {6, kSCubePts, 8, kCubeSurf};

// --- Mini Cube (type 15) ---
static const int kMCubePts[6][3] = {
	{0, 0, 50}, {25, 0, 25}, {0, 25, 25}, {-25, 0, 25}, {0, -25, 25}, {0, 0, 0}
};
static const Colony::ColonyEngine::PrismPartDef kMCubeBodyDef = {6, kMCubePts, 8, kCubeSurf};

// --- Floating UPyramid (type 8) ---
static const int kFUPyramidPts[5][3] = {
	{-75, 75, 170}, {75, 75, 170}, {75, -75, 170}, {-75, -75, 170}, {0, 0, 0}
};
static const Colony::ColonyEngine::PrismPartDef kFUPyramidBodyDef = {5, kFUPyramidPts, 5, kUPyramidSurf};

// --- Small UPyramid (type 12) ---
static const int kSUPyramidPts[5][3] = {
	{-40, 40, 100}, {40, 40, 100}, {40, -40, 100}, {-40, -40, 100}, {0, 0, 0}
};
static const Colony::ColonyEngine::PrismPartDef kSUPyramidBodyDef = {5, kSUPyramidPts, 5, kUPyramidSurf};

// --- Mini UPyramid (type 16) ---
static const int kMUPyramidPts[5][3] = {
	{-20, 20, 50}, {20, 20, 50}, {20, -20, 50}, {-20, -20, 50}, {0, 0, 0}
};
static const Colony::ColonyEngine::PrismPartDef kMUPyramidBodyDef = {5, kMUPyramidPts, 5, kUPyramidSurf};

// --- Floating Eye (type 5) ---
static const int kFEyeIrisPts[4][3] = {
	{60, 0, 40}, {60, 60, 100}, {60, 0, 160}, {60, -60, 100}
};
static const int kFEyePupilPts[4][3] = {
	{66, 0, 75}, {66, 25, 100}, {66, 0, 125}, {66, -25, 100}
};
static const Colony::ColonyEngine::PrismPartDef kFEyeIrisDef = {4, kFEyeIrisPts, 1, kEyeIrisSurf};
static const Colony::ColonyEngine::PrismPartDef kFEyePupilDef = {4, kFEyePupilPts, 1, kEyePupilSurf};

// --- Small Eye (type 9) ---
static const int kSEyeIrisPts[4][3] = {
	{30, 0, 20}, {30, 30, 50}, {30, 0, 80}, {30, -30, 50}
};
static const int kSEyePupilPts[4][3] = {
	{33, 0, 38}, {33, 13, 50}, {33, 0, 63}, {33, -13, 50}
};
static const Colony::ColonyEngine::PrismPartDef kSEyeIrisDef = {4, kSEyeIrisPts, 1, kEyeIrisSurf};
static const Colony::ColonyEngine::PrismPartDef kSEyePupilDef = {4, kSEyePupilPts, 1, kEyePupilSurf};

// --- Mini Eye (type 13) ---
static const int kMEyeIrisPts[4][3] = {
	{15, 0, 10}, {15, 15, 25}, {15, 0, 40}, {15, -15, 25}
};
static const int kMEyePupilPts[4][3] = {
	{16, 0, 19}, {16, 6, 25}, {16, 0, 31}, {16, -6, 25}
};
static const Colony::ColonyEngine::PrismPartDef kMEyeIrisDef = {4, kMEyeIrisPts, 1, kEyeIrisSurf};
static const Colony::ColonyEngine::PrismPartDef kMEyePupilDef = {4, kMEyePupilPts, 1, kEyePupilSurf};

// --- Queen (type 17) ---
// Queen eye (ball rendered by draw3DSphere)
static const int kQIrisPts[4][3] = {
	{15, 0, 140}, {15, 15, 155}, {15, 0, 170}, {15, -15, 155}
};
static const int kQIrisSurf[1][8] = {{kColorIris, 4, 0, 1, 2, 3, 0, 0}};
static const int kQPupilPts[4][3] = {
	{16, 0, 148}, {16, 6, 155}, {16, 0, 161}, {16, -6, 155}
};
static const int kQPupilSurf[1][8] = {{kColorPupil, 4, 0, 1, 2, 3, 0, 0}};
// Queen abdomen
static const int kQAbdomenPts[9][3] = {
	{120, 0, 130}, {30, 0, 160}, {30, 0, 100}, {30, 50, 130}, {30, -50, 130},
	{0, 0, 150}, {0, 0, 110}, {0, 25, 130}, {0, -25, 130}
};
static const int kQAbdomenSurf[9][8] = {
	{kColorQueen, 3, 0, 3, 1, 0, 0, 0}, {kColorQueen, 3, 0, 1, 4, 0, 0, 0},
	{kColorQueen, 3, 0, 2, 3, 0, 0, 0}, {kColorQueen, 3, 0, 4, 2, 0, 0, 0},
	{kColorQueen, 4, 1, 5, 8, 4, 1, 0}, {kColorQueen, 4, 1, 3, 7, 5, 1, 0},
	{kColorQueen, 4, 2, 4, 8, 6, 2, 0}, {kColorQueen, 4, 2, 6, 7, 3, 2, 0},
	{kColorClear, 4, 5, 7, 6, 8, 5, 0}
};
// Queen thorax
static const int kQThoraxPts[9][3] = {
	{-120, 0, 130}, {-50, 0, 170}, {-50, 0, 90}, {-50, 60, 130}, {-50, -60, 130},
	{0, 0, 150}, {0, 0, 110}, {0, 25, 130}, {0, -25, 130}
};
static const int kQThoraxSurf[8][8] = {
	{kColorQueen, 3, 0, 1, 3, 0, 0, 0}, {kColorQueen, 3, 0, 4, 1, 0, 0, 0},
	{kColorQueen, 3, 0, 3, 2, 0, 0, 0}, {kColorQueen, 3, 0, 2, 4, 0, 0, 0},
	{kColorQueen, 4, 1, 4, 8, 5, 1, 0}, {kColorQueen, 4, 1, 5, 7, 3, 1, 0},
	{kColorQueen, 4, 2, 6, 8, 4, 2, 0}, {kColorQueen, 4, 2, 3, 7, 6, 2, 0}
};
// Queen wings
static const int kQLWingPts[4][3] = {
	{80, 0, 140}, {-40, 10, 200}, {-120, 60, 170}, {-40, 120, 140}
};
static const int kQLWingSurf[1][8] = {{kColorClear, 4, 0, 1, 2, 3, 0, 0}};
static const int kQRWingPts[4][3] = {
	{80, 0, 140}, {-40, -10, 200}, {-120, -60, 170}, {-40, -120, 140}
};
static const int kQRWingSurf[1][8] = {{kColorClear, 4, 0, 1, 2, 3, 0, 0}};

static const Colony::ColonyEngine::PrismPartDef kQIrisDef = {4, kQIrisPts, 1, kQIrisSurf};
static const Colony::ColonyEngine::PrismPartDef kQPupilDef = {4, kQPupilPts, 1, kQPupilSurf};
static const Colony::ColonyEngine::PrismPartDef kQAbdomenDef = {9, kQAbdomenPts, 9, kQAbdomenSurf};
static const Colony::ColonyEngine::PrismPartDef kQThoraxDef = {9, kQThoraxPts, 8, kQThoraxSurf};
static const Colony::ColonyEngine::PrismPartDef kQLWingDef = {4, kQLWingPts, 1, kQLWingSurf};
static const Colony::ColonyEngine::PrismPartDef kQRWingDef = {4, kQRWingPts, 1, kQRWingSurf};

// --- Drone / Soldier (types 18, 19) ---
static const int kDAbdomenPts[6][3] = {
	{0, 0, 170}, {120, 0, 130}, {0, 100, 130}, {-130, 0, 130}, {0, -100, 130}, {0, 0, 100}
};
static const int kDAbdomenSurf[8][8] = {
	{kColorDrone, 3, 0, 1, 2, 0, 0, 0}, {kColorDrone, 3, 0, 2, 3, 0, 0, 0},
	{kColorDrone, 3, 0, 3, 4, 0, 0, 0}, {kColorDrone, 3, 0, 4, 1, 0, 0, 0},
	{kColorDrone, 3, 5, 2, 1, 5, 0, 0}, {kColorDrone, 3, 5, 3, 2, 5, 0, 0},
	{kColorDrone, 3, 5, 4, 3, 5, 0, 0}, {kColorDrone, 3, 5, 1, 4, 5, 0, 0}
};
// Drone static pincers (llPincer/rrPincer)
static const int kDLLPincerPts[4][3] = {
	{0, 0, 130}, {50, -2, 130}, {35, -20, 140}, {35, -20, 120}
};
static const int kDLPincerSurf[4][8] = {
	{kColorClaw1, 3, 0, 2, 1, 0, 0, 0}, {kColorClaw1, 3, 0, 1, 3, 0, 0, 0},
	{kColorClaw2, 3, 0, 3, 2, 0, 0, 0}, {kColorClaw2, 3, 1, 2, 3, 1, 0, 0}
};
static const int kDRRPincerPts[4][3] = {
	{0, 0, 130}, {50, 2, 130}, {35, 20, 140}, {35, 20, 120}
};
static const int kDRPincerSurf[4][8] = {
	{kColorClaw1, 3, 0, 1, 2, 0, 0, 0}, {kColorClaw1, 3, 0, 3, 1, 0, 0, 0},
	{kColorClaw2, 3, 0, 2, 3, 0, 0, 0}, {kColorClaw2, 3, 1, 3, 2, 1, 0, 0}
};
// Drone eyes
static const int kDLEyePts[3][3] = {
	{60, 0, 150}, {60, 50, 130}, {60, 25, 150}
};
static const int kDLEyeSurf[2][8] = {
	{kColorEyes, 3, 0, 1, 2, 0, 0, 0}, {kColorEyes, 3, 0, 2, 1, 0, 0, 0}
};
static const int kDREyePts[3][3] = {
	{60, 0, 150}, {60, -50, 130}, {60, -25, 150}
};
static const int kDREyeSurf[2][8] = {
	{kColorEyes, 3, 0, 1, 2, 0, 0, 0}, {kColorEyes, 3, 0, 2, 1, 0, 0, 0}
};

static const Colony::ColonyEngine::PrismPartDef kDAbdomenDef = {6, kDAbdomenPts, 8, kDAbdomenSurf};
static const Colony::ColonyEngine::PrismPartDef kDLLPincerDef = {4, kDLLPincerPts, 4, kDLPincerSurf};
static const Colony::ColonyEngine::PrismPartDef kDRRPincerDef = {4, kDRRPincerPts, 4, kDRPincerSurf};
static const Colony::ColonyEngine::PrismPartDef kDLEyeDef = {3, kDLEyePts, 2, kDLEyeSurf};
static const Colony::ColonyEngine::PrismPartDef kDREyeDef = {3, kDREyePts, 2, kDREyeSurf};

// --- Snoop (type 20) ---
static const int kSnoopAbdomenPts[4][3] = {
	{0, 100, 0}, {-180, 0, 0}, {0, -100, 0}, {0, 0, 70}
};
static const int kSnoopAbdomenSurf[2][8] = {
	{kColorTopSnoop, 3, 0, 1, 3, 0, 0, 0}, {kColorTopSnoop, 3, 2, 3, 1, 2, 0, 0}
};
static const int kSnoopHeadPts[4][3] = {
	{0, 100, 0}, {150, 0, 0}, {0, -100, 0}, {0, 0, 70}
};
static const int kSnoopHeadSurf[3][8] = {
	{kColorTopSnoop, 3, 0, 3, 1, 0, 0, 0}, {kColorTopSnoop, 3, 2, 1, 3, 2, 0, 0},
	{kColorBottomSnoop, 3, 0, 1, 2, 0, 0, 0}
};

static const Colony::ColonyEngine::PrismPartDef kSnoopAbdomenDef = {4, kSnoopAbdomenPts, 2, kSnoopAbdomenSurf};
static const Colony::ColonyEngine::PrismPartDef kSnoopHeadDef = {4, kSnoopHeadPts, 3, kSnoopHeadSurf};


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

static void resetObjectBounds(const Common::Rect &screenR, Locate &loc) {
	loc.xmn = screenR.right;
	loc.xmx = screenR.left;
	loc.zmn = screenR.bottom;
	loc.zmx = screenR.top;
}

void ColonyEngine::draw3DPrism(Thing &obj, const PrismPartDef &def, bool useLook, int colorOverride, bool accumulateBounds) {
	// +32 compensates for the original sine table's 45° phase offset.
	// Object angles from game data were stored assuming that offset.
	const uint8 ang = (useLook ? obj.where.look : obj.where.ang) + 32;
	const long rotCos = _cost[ang];
	const long rotSin = _sint[ang];
	const bool lit = (_corePower[_coreIndex] > 0);

	for (int i = 0; i < def.surfaceCount; i++) {
		const int colorIdx = (colorOverride >= 0) ? colorOverride : def.surfaces[i][0];
		const int n = def.surfaces[i][1];
		if (n < 2)
			continue;

		float px[8];
		float py[8];
		float pz[8];
		int count = 0;

		for (int j = 0; j < n; j++) {
			const int cur = def.surfaces[i][j + 2];
			if (cur < 0 || cur >= def.pointCount)
				continue;

			int ox = def.points[cur][0];
			int oy = def.points[cur][1];
			int oz = def.points[cur][2];

			// World relative rotation
			long rx = ((long)ox * rotCos - (long)oy * rotSin) >> 7;
			long ry = ((long)ox * rotSin + (long)oy * rotCos) >> 7;

			px[count] = (float)(rx + obj.where.xloc);
			py[count] = (float)(ry + obj.where.yloc);
			pz[count] = (float)(oz - 160); // Shift from floor-relative (z=0) to world (z=-160)
			if (accumulateBounds) {
				int sx = 0;
				int sy = 0;
				if (projectCorridorPoint(_screenR, _me.look, _me.lookY, _sint, _cost, _me.xloc, _me.yloc,
				                         px[count], py[count], pz[count], sx, sy)) {
					obj.where.xmn = MIN(obj.where.xmn, sx);
					obj.where.xmx = MAX(obj.where.xmx, sx);
					obj.where.zmn = MIN(obj.where.zmn, sy);
					obj.where.zmx = MAX(obj.where.zmx, sy);
				}
			}
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
					int mIdx = mapObjColorToMacColor(colorIdx);
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
					int pattern = lookupMacPattern(colorIdx);
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
					const DOSColorEntry &dc = lookupDOSColor(colorIdx);
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
	// Sphere defined by two object-local points: pt0 (center bottom) and pt1 (center top).
	// Center is midpoint, radius is half the distance (along z typically).
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

	// Center and radius
	float cx = (wx0 + wx1) * 0.5f;
	float cy = (wy0 + wy1) * 0.5f;
	float cz = (wz0 + wz1) * 0.5f;
	float dx = wx1 - wx0, dy = wy1 - wy0, dz = wz1 - wz0;
	float radius = sqrtf(dx * dx + dy * dy + dz * dz) * 0.5f;

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
		int mIdx = mapObjColorToMacColor((int)fillColor);
		int pattern = _macColors[mIdx].pattern;
		uint32 fg = packMacColor(_macColors[mIdx].fg);
		uint32 bg = packMacColor(_macColors[mIdx].bg);
		if (!lit) {
			fg = 0xFF000000;
			bg = 0xFF000000;
			pattern = 4;
		}
		const byte *stipple = setupMacPattern(_gfx, pattern, fg, bg);
		_gfx->draw3DPolygon(px, py, pz, N, fg);
		if (stipple)
			_gfx->setStippleData(nullptr);
	} else if (lit) {
		if (_renderMode == Common::kRenderMacintosh) {
			int pattern = (fillColor == 15) ? kPatternWhite : kPatternGray;
			if (!_wireframe) {
				_gfx->setWireframe(true, pattern == kPatternBlack ? 0 : 255);
			}
			_gfx->setStippleData(kMacStippleData[pattern]);
			_gfx->draw3DPolygon(px, py, pz, N, 0);
			_gfx->setStippleData(nullptr);
		} else {
			// EGA: per-surface fill + outline.
			if (!_wireframe) {
				_gfx->setWireframe(true, fillColor);
			}
			_gfx->draw3DPolygon(px, py, pz, N, outlineColor);
		}
	} else {
		// Unlit: black fill, white outline.
		if (!_wireframe) {
			_gfx->setWireframe(true, 0);
		}
		_gfx->draw3DPolygon(px, py, pz, N, outlineColor);
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
	float px[64], py[64], pz[64];
	if (count > 64)
		count = 64;
	for (int i = 0; i < count; i++) {
		float p[3];
		wallPoint(corners, u[i], v[i], p);
		px[i] = p[0]; py[i] = p[1]; pz[i] = p[2];
	}
	_gfx->draw3DPolygon(px, py, pz, count, color);
}

static float signedArea2D(const float *x, const float *y, int count) {
	float area = 0.0f;
	for (int i = 0; i < count; ++i) {
		const int next = (i + 1) % count;
		area += x[i] * y[next] - x[next] * y[i];
	}
	return area * 0.5f;
}

static bool pointInTriangle2D(float px, float py, float ax, float ay, float bx, float by, float cx, float cy) {
	const float d1 = (px - bx) * (ay - by) - (ax - bx) * (py - by);
	const float d2 = (px - cx) * (by - cy) - (bx - cx) * (py - cy);
	const float d3 = (px - ax) * (cy - ay) - (cx - ax) * (py - ay);
	const bool hasNeg = (d1 < 0.0f) || (d2 < 0.0f) || (d3 < 0.0f);
	const bool hasPos = (d1 > 0.0f) || (d2 > 0.0f) || (d3 > 0.0f);
	return !(hasNeg && hasPos);
}

static const char *const kWallCharData[] = {
	"\00",
	"\02\10\02\00\03\00\03\01\02\01\10\02\02\03\02\03\06\02\06",
	"\02\10\01\04\02\04\02\05\01\05\10\03\04\04\04\04\05\03\05",
	"\04\10\01\00\02\00\02\05\01\05\10\03\00\04\00\04\05\03\05\10\00\01\05\01\05\02\00\02\10\00\03\05\03\05\04\00\04",
	"\02\10\02\00\03\00\03\06\02\06\050\00\02\00\01\01\00\04\00\05\01\05\02\01\04\01\05\04\05\04\04\05\04\05\05\04\06\01\06\00\05\00\04\04\02\04\01\01\01\01\02",
	"\03\10\01\00\06\05\05\06\00\01\032\01\03\02\03\03\04\03\05\02\06\01\06\00\05\00\04\01\03\01\05\02\05\02\04\01\04\032\04\00\05\00\06\01\06\02\05\03\04\03\03\02\03\01\04\00\04\02\05\02\05\01\04\01",
	"\03\10\05\01\05\02\03\04\02\04\014\02\04\01\05\02\06\01\06\00\05\01\04\032\05\04\03\01\01\01\01\03\03\05\02\06\01\06\02\05\00\03\00\01\01\00\03\00\05\03",
	"\01\10\02\04\03\05\03\06\02\06",
	"\01\014\04\00\03\02\03\04\04\06\02\04\02\02",
	"\01\014\02\00\04\02\04\04\02\06\03\04\03\02",
	"\06\06\01\00\03\03\00\02\06\02\00\04\00\03\03\06\05\00\06\02\03\03\06\06\04\05\06\03\03\06\02\06\04\06\03\03\06\00\04\01\06\03\03",
	"\02\10\02\00\03\00\03\05\02\05\10\00\02\05\02\05\03\00\03",
	"\01\10\02\00\03\01\03\02\02\02",
	"\01\10\00\02\05\02\05\03\00\03",
	"\01\10\02\00\03\00\03\01\02\01",
	"\01\10\01\00\06\05\05\06\00\01",
	"\02\032\01\00\05\00\06\01\06\05\05\06\01\06\00\05\00\01\01\00\01\05\05\05\05\01\01\01\10\01\01\02\01\05\05\04\05",
	"\01\026\01\00\04\00\04\01\03\01\03\06\02\06\01\05\01\04\02\04\02\01\01\01",
	"\01\042\06\00\06\01\02\01\05\02\06\03\06\05\05\06\01\06\00\05\00\04\01\04\01\05\05\05\05\04\01\02\00\01\00\00",
	"\01\054\00\02\00\01\01\00\05\00\06\01\06\02\05\03\06\04\06\05\05\06\01\06\00\05\00\04\01\04\01\05\05\05\05\04\04\03\05\02\05\01\01\01\01\02",
	"\01\036\04\00\05\00\05\02\06\02\06\03\05\03\05\06\04\06\04\03\01\03\02\06\01\06\00\03\00\02\04\02",
	"\01\044\00\02\00\01\01\00\05\00\06\01\06\03\05\04\01\04\02\05\06\05\06\06\01\06\00\04\00\03\05\03\05\01\01\01\01\02",
	"\01\046\01\02\05\02\05\01\01\01\01\05\05\05\05\04\06\04\06\05\05\06\01\06\00\05\00\01\01\00\05\00\06\01\06\02\05\03\01\03",
	"\01\020\02\00\03\00\03\03\06\06\00\06\00\05\04\05\02\03",
	"\02\040\03\00\03\01\01\01\01\02\04\03\01\04\01\05\03\05\03\06\01\06\00\05\00\04\01\03\00\02\00\01\01\00\040\03\00\05\00\06\01\06\02\05\03\06\04\06\05\05\06\03\06\03\05\05\05\05\04\02\03\05\02\05\01\03\01",
	"\01\046\00\02\00\01\01\00\05\00\06\01\06\05\05\06\01\06\00\05\00\04\01\03\05\03\05\04\01\04\01\05\05\05\05\01\01\01\01\02",
	"\02\10\02\01\03\01\03\02\02\02\10\02\03\03\03\03\04\02\04",
	"\02\10\02\00\03\01\03\02\02\02\10\02\03\03\03\03\04\02\04",
	"\01\014\06\00\06\01\02\03\06\05\06\06\00\03",
	"\02\10\00\01\05\01\05\02\00\02\10\00\03\05\03\05\04\00\04",
	"\01\014\00\00\06\03\00\06\00\05\04\03\00\01",
	"\02\10\02\00\03\00\03\01\02\01\030\02\02\03\02\05\04\05\05\04\06\01\06\00\05\00\04\01\04\01\05\04\05\04\04",
	"\05\012\04\00\01\01\00\04\00\01\01\00\012\02\00\05\00\06\01\06\04\05\01\012\06\02\06\05\05\06\02\06\05\05\012\04\06\01\06\00\05\00\02\01\05\034\05\01\05\02\03\05\02\05\01\04\01\02\02\01\03\01\05\04\05\05\03\02\02\02\02\04\03\04",
	"\01\034\03\06\04\06\06\00\05\00\04\02\02\02\01\00\00\00\02\06\03\06\03\05\02\03\04\03\03\05",
	"\01\050\00\00\00\06\05\06\06\05\06\04\05\03\06\02\06\01\05\00\01\00\01\01\05\01\05\02\04\03\01\03\01\04\05\04\05\05\01\05\01\00",
	"\01\040\06\02\06\01\05\00\01\00\00\01\00\05\01\06\05\06\06\05\06\04\05\04\05\05\01\05\01\01\05\01\05\02",
	"\01\034\00\00\00\06\04\06\06\04\06\02\04\00\01\00\01\01\04\01\05\02\05\04\04\05\01\05\01\00",
	"\01\030\00\00\00\06\06\06\06\05\01\05\01\04\04\04\04\03\01\03\01\01\06\01\06\00",
	"\01\024\00\00\00\06\06\06\06\05\01\05\01\03\04\03\04\02\01\02\01\00",
	"\01\044\03\03\06\03\06\01\05\00\01\00\00\01\00\05\01\06\05\06\06\05\06\04\05\04\05\05\01\05\01\01\05\01\05\02\03\02",
	"\01\030\00\00\00\06\01\06\01\04\05\04\05\06\06\06\06\00\05\00\05\03\01\03\01\00",
	"\01\030\01\00\01\01\02\01\02\05\01\05\01\06\04\06\04\05\03\05\03\01\04\01\04\00",
	"\01\034\00\02\00\01\01\00\04\00\05\01\05\05\06\05\06\06\03\06\03\05\04\05\04\01\01\01\01\02",
	"\01\026\00\00\00\06\01\06\01\04\04\06\06\06\02\03\06\00\04\00\01\02\01\00",
	"\01\014\00\06\00\00\06\00\06\01\01\01\01\06",
	"\01\030\00\00\00\06\01\06\03\04\05\06\06\06\06\00\05\00\05\04\03\03\01\04\01\00",
	"\01\024\00\00\00\06\02\06\05\01\05\06\06\06\06\00\04\00\01\05\01\00",
	"\01\032\00\01\00\05\01\06\05\06\06\05\06\01\05\00\01\00\00\01\05\01\05\05\01\05\01\01",
	"\01\030\00\00\00\06\05\06\06\05\06\03\05\02\01\02\01\03\05\03\05\05\01\05\01\00",
	"\02\036\04\00\01\00\00\01\00\05\01\06\05\06\06\05\06\02\04\00\04\02\05\02\05\05\01\05\01\01\04\01\014\06\00\06\01\05\02\04\02\04\01\05\00",
	"\01\036\00\00\00\06\05\06\06\05\06\03\05\02\06\00\05\00\04\02\01\02\01\03\05\03\05\05\01\05\01\00",
	"\01\054\00\02\00\01\01\00\05\00\06\01\06\02\05\03\01\04\01\05\05\05\05\04\06\04\06\05\05\06\01\06\00\05\00\04\01\03\05\02\05\01\01\01\01\02",
	"\01\020\02\00\02\05\00\05\00\06\05\06\05\05\03\05\03\00",
	"\01\024\00\06\00\01\01\00\05\00\06\01\06\06\05\06\05\01\01\01\01\06",
	"\01\016\00\06\02\00\04\00\06\06\05\06\03\01\01\06",
	"\01\030\00\06\01\00\02\00\03\02\04\00\05\00\06\06\05\06\04\02\03\04\02\02\01\06",
	"\01\030\00\00\02\03\00\06\01\06\03\04\05\06\06\06\04\03\06\00\05\00\03\02\01\00",
	"\02\014\00\06\02\03\02\00\03\00\03\03\01\06\10\02\03\03\03\05\06\04\06",
	"\01\024\00\05\04\05\00\01\00\00\06\00\06\01\02\01\06\05\06\06\00\06",
	"\01\020\04\00\02\00\02\06\04\06\04\05\03\05\03\01\04\01",
	"\01\10\00\05\05\00\06\01\01\06",
	"\01\020\02\00\02\01\03\01\03\05\02\05\02\06\04\06\04\00",
	"\01\014\00\02\03\06\06\02\05\02\03\05\01\02",
	"\01\10\00\01\06\01\06\02\00\02",
	"\01\10\02\04\03\05\03\06\02\06",
	"\04\06\03\04\03\05\04\06\024\00\02\00\04\01\05\02\05\03\04\04\05\05\05\06\04\05\04\04\02\012\00\02\04\02\04\01\02\00\01\00\014\02\01\04\04\05\02\06\02\05\00\04\00",
	"\01\016\00\03\03\00\03\02\06\02\06\04\03\04\03\06",
	"\01\016\00\02\00\04\03\04\03\06\06\03\03\00\03\02",
	"\01\06\00\00\03\06\06\00",
	"\03\06\01\00\05\00\03\03\06\03\03\00\03\01\06\06\03\03\06\03\05\06",
};

void ColonyEngine::wallChar(const float corners[4][3], uint8 cnum) {
	if (cnum < 0x20 || cnum > 0x65)
		cnum = 0x20;

	const uint8 *data = reinterpret_cast<const uint8 *>(kWallCharData[cnum - 0x20]);
	if (!data || data[0] == 0)
		return;

	const bool macMode = (_renderMode == Common::kRenderMacintosh);
	const bool macColors = (macMode && _hasMacColors);
	const uint32 fillColor = macColors ? packMacColor(_macColors[8 + _level - 1].bg) : 0;
	const uint32 lineColor = macColors ? (uint32)0xFF000000 : 0;

	auto drawFilledCharPolygon = [&](const float *u, const float *v, int count) {
		if (!macMode || count < 3)
			return;

		const float area = signedArea2D(u, v, count);
		if (fabsf(area) < 0.0001f)
			return;

		int indices[32];
		for (int i = 0; i < count; ++i)
			indices[i] = i;

		const bool ccw = area > 0.0f;
		int remaining = count;
		int guard = 0;

		while (remaining > 3 && guard++ < 64) {
			bool earFound = false;
			for (int i = 0; i < remaining; ++i) {
				const int prev = indices[(i + remaining - 1) % remaining];
				const int curr = indices[i];
				const int next = indices[(i + 1) % remaining];
				const float cross = (u[curr] - u[prev]) * (v[next] - v[prev]) -
					(v[curr] - v[prev]) * (u[next] - u[prev]);
				if (ccw ? (cross <= 0.0001f) : (cross >= -0.0001f))
					continue;

				bool containsPoint = false;
				for (int j = 0; j < remaining; ++j) {
					const int test = indices[j];
					if (test == prev || test == curr || test == next)
						continue;
					if (pointInTriangle2D(u[test], v[test], u[prev], v[prev], u[curr], v[curr], u[next], v[next])) {
						containsPoint = true;
						break;
					}
				}
				if (containsPoint)
					continue;

				const float triU[3] = {u[prev], u[curr], u[next]};
				const float triV[3] = {v[prev], v[curr], v[next]};
				wallPolygon(corners, triU, triV, 3, fillColor);

				for (int j = i; j < remaining - 1; ++j)
					indices[j] = indices[j + 1];
				--remaining;
				earFound = true;
				break;
			}

			if (!earFound) {
				for (int i = 1; i < remaining - 1; ++i) {
					const float triU[3] = {u[indices[0]], u[indices[i]], u[indices[i + 1]]};
					const float triV[3] = {v[indices[0]], v[indices[i]], v[indices[i + 1]]};
					wallPolygon(corners, triU, triV, 3, fillColor);
				}
				return;
			}
		}

		if (remaining == 3) {
			const float triU[3] = {u[indices[0]], u[indices[1]], u[indices[2]]};
			const float triV[3] = {v[indices[0]], v[indices[1]], v[indices[2]]};
			wallPolygon(corners, triU, triV, 3, fillColor);
		}
	};

	if (macMode)
		_gfx->setWireframe(false);

	int offset = 1;
	for (int poly = 0; poly < data[0]; ++poly) {
		const int coordCount = data[offset++];
		int count = coordCount / 2;
		if (count > 32)
			count = 32;

		float u[32], v[32];
		for (int i = 0; i < count; ++i) {
			u[i] = (float)data[offset + i * 2] / 6.0f;
			v[i] = (float)data[offset + i * 2 + 1] / 6.0f;
		}

		drawFilledCharPolygon(u, v, count);
		for (int i = 0; i < count; ++i) {
			const int next = (i + 1) % count;
			wallLine(corners, u[i], v[i], u[next], v[next], lineColor);
		}

		offset += coordCount;
	}

	if (macMode) {
		const uint32 wallFill = _hasMacColors
			? packMacColor(_macColors[8 + _level - 1].fg)
			: (uint32)255;
		_gfx->setWireframe(true, wallFill);
	}
}

void ColonyEngine::getCellFace3D(int cellX, int cellY, bool ceiling, float corners[4][3]) {
	float z = ceiling ? 160.0f : -160.0f;
	float x0 = cellX * 256.0f;
	float y0 = cellY * 256.0f;
	float x1 = x0 + 256.0f;
	float y1 = y0 + 256.0f;
	const float eps = 0.1f;
	if (ceiling)
		z -= eps;
	else
		z += eps;

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

	const bool macMode = (_renderMode == Common::kRenderMacintosh);
	const bool macColors = (macMode && _hasMacColors);

	// Helper lambda: draw a filled hole polygon with Mac color or B&W fallback
	auto drawHolePoly = [&](const float *u, const float *v, int cnt, int macIdx) {
		if (macColors) {
			uint32 fg = packMacColor(_macColors[macIdx].fg);
			uint32 bg = packMacColor(_macColors[macIdx].bg);
			int pat = _macColors[macIdx].pattern;
			const byte *stipple = setupMacPattern(_gfx, pat, fg, bg);
			wallPolygon(corners, u, v, cnt, fg);
			if (stipple)
			_gfx->setStippleData(nullptr);
		} else if (macMode) {
			_gfx->setStippleData(kStippleGray);
			wallPolygon(corners, u, v, cnt, 0);
			_gfx->setStippleData(nullptr);
		} else {
			wallPolygon(corners, u, v, cnt, holeColor);
		}
	};

	switch (map[0]) {
	case 1: // SMHOLEFLR
	case 3: // SMHOLECEIL
	{
		float u[4] = {0.25f, 0.75f, 0.75f, 0.25f};
		float v[4] = {0.25f, 0.25f, 0.75f, 0.75f};
		drawHolePoly(u, v, 4, 30); // c_hole
		break;
	}
	case 2: // LGHOLEFLR
	case 4: // LGHOLECEIL
	{
		float u[4] = {0.0f, 1.0f, 1.0f, 0.0f};
		float v[4] = {0.0f, 0.0f, 1.0f, 1.0f};
		drawHolePoly(u, v, 4, 30); // c_hole
		break;
	}
	case 5: // HOTFOOT
	{
		float u[4] = {0.0f, 1.0f, 1.0f, 0.0f};
		float v[4] = {0.0f, 0.0f, 1.0f, 1.0f};
		if (macMode) {
			drawHolePoly(u, v, 4, 31); // c_hotplate
		} else {
			// DOS non-polyfill: X pattern (two diagonals)
			wallLine(corners, 0.0f, 0.0f, 1.0f, 1.0f, holeColor);
			wallLine(corners, 1.0f, 0.0f, 0.0f, 1.0f, holeColor);
		}
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
	case kDirNorth:
		if (_me.yloc > (cellY + 1) * 256)
			return;
		break;
	case kDirSouth:
		if (_me.yloc < cellY * 256)
			return;
		break;
	case kDirWest:
		if (_me.xloc < cellX * 256)
			return;
		break;
	case kDirEast:
		if (_me.xloc > (cellX + 1) * 256)
			return;
		break;
	default: break;
	}
	
	float corners[4][3];
	getWallFace3D(cellX, cellY, direction, corners);
	const bool macMode = (_renderMode == Common::kRenderMacintosh);
	const bool macColors = (_renderMode == Common::kRenderMacintosh && _hasMacColors);
	const bool lit = (_corePower[_coreIndex] > 0);
	const uint32 wallFeatureFill = macColors
		? packMacColor(lit ? _macColors[8 + _level - 1].fg : _macColors[6].bg)
		: (lit ? (macMode ? 255u : 7u) : 0u);

	// Wall faces are already filled with level-specific color (c_char0+level-1.fg)
	// by the wall grid in renderCorridor3D(). Features are drawn on top.

	// Helper lambda: Mac color fill for a wall feature polygon
	auto macFillPoly = [&](const float *u, const float *v, int cnt, int macIdx) {
		uint32 fg = packMacColor(_macColors[macIdx].fg);
		uint32 bg = packMacColor(_macColors[macIdx].bg);
		const byte *stipple = setupMacPattern(_gfx, _macColors[macIdx].pattern, fg, bg);
		wallPolygon(corners, u, v, cnt, fg);
		if (stipple)
			_gfx->setStippleData(nullptr);
	};

	switch (map[0]) {
	case kWallFeatureDoor: {
		const uint32 doorColor = macColors ? (uint32)0xFF000000 : (macMode ? 0 : 8);
		bool shipLevel = (_level == 1 || _level == 5 || _level == 6);

		if (shipLevel) {
			static const float uSs[8] = { 0.375f, 0.250f, 0.250f, 0.375f, 0.625f, 0.750f, 0.750f, 0.625f };
			static const float vSs[8] = { 0.125f, 0.250f, 0.750f, 0.875f, 0.875f, 0.750f, 0.250f, 0.125f };
			const uint32 shipDoorColor = macColors ? (uint32)0xFF000000 : (macMode ? 0 : (_wireframe ? 8u : 0u));

			if (macMode) {
				if (map[1] != 0) {
					if (macColors) {
						macFillPoly(uSs, vSs, 8, 15); // c_bulkhead
					} else {
						_gfx->setStippleData(kStippleGray);
						wallPolygon(corners, uSs, vSs, 8, 0);
						_gfx->setStippleData(nullptr);
					}
				} else {
					// Open: fill with BLACK (passable opening)
					_gfx->setWireframe(true, 0);
					wallPolygon(corners, uSs, vSs, 8, 0);
				}
			} else if (!_wireframe) {
				if (map[1] != 0) {
					const byte *stipple = setupMacPattern(_gfx, kPatternLtGray, 0, 15);
					wallPolygon(corners, uSs, vSs, 8, 0);
					if (stipple)
						_gfx->setStippleData(nullptr);
				} else {
					_gfx->setWireframe(true, 0);
					wallPolygon(corners, uSs, vSs, 8, 0);
				}
			} else {
				_gfx->setWireframe(true);
				wallPolygon(corners, uSs, vSs, 8, 8);
			}

			for (int i = 0; i < 8; i++)
				wallLine(corners, uSs[i], vSs[i], uSs[(i + 1) % 8], vSs[(i + 1) % 8], shipDoorColor);

			if (map[1] != 0) {
				wallLine(corners, 0.375f, 0.25f, 0.375f, 0.75f, shipDoorColor);
				wallLine(corners, 0.375f, 0.75f, 0.625f, 0.75f, shipDoorColor);
				wallLine(corners, 0.625f, 0.75f, 0.625f, 0.25f, shipDoorColor);
				wallLine(corners, 0.625f, 0.25f, 0.375f, 0.25f, shipDoorColor);
			}
		} else {
			static const float xl = 0.25f, xr = 0.75f;
			static const float yb = 0.0f, yt = 0.875f;

			if (macMode) {
				float ud[4] = {xl, xr, xr, xl};
				float vd[4] = {yb, yb, yt, yt};
				if (map[1] != 0) {
					if (macColors) {
						macFillPoly(ud, vd, 4, 16); // c_door
					} else {
						_gfx->setStippleData(kStippleGray);
						wallPolygon(corners, ud, vd, 4, 0);
						_gfx->setStippleData(nullptr);
					}
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
		const uint32 winColor = macColors ? (uint32)0xFF000000 : (macMode ? 0 : 8);
		float xl = 0.25f, xr = 0.75f;
		float yb = 0.25f, yt = 0.75f;
		float xc = 0.5f, yc = 0.5f;

		// Mac: fill window pane
		if (macMode) {
			float uw[4] = {xl, xr, xr, xl};
			float vw[4] = {yb, yb, yt, yt};
			if (macColors) {
				macFillPoly(uw, vw, 4, 17); // c_window
			} else {
				_gfx->setStippleData(kStippleDkGray);
				wallPolygon(corners, uw, vw, 4, 0);
				_gfx->setStippleData(nullptr);
			}
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
		const uint32 shelfColor = macColors ? (uint32)0xFF000000 : (macMode ? 0 : 8);

		// Mac: fill shelves area
		if (macMode) {
			float us[4] = {0.0f, 1.0f, 1.0f, 0.0f};
			float vs[4] = {0.0f, 0.0f, 1.0f, 1.0f};
			if (macColors) {
				macFillPoly(us, vs, 4, 18); // c_shelves
			} else {
				_gfx->setStippleData(kStippleLtGray);
				wallPolygon(corners, us, vs, 4, 0);
				_gfx->setStippleData(nullptr);
			}
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
		// DOS: draw_up_stairs  staircase ascending into the wall with perspective
		const uint32 col = macColors ? (uint32)0xFF000000 : 0; // vBLACK

		// Mac: fill entire wall face (c_upstairs)
		if (_renderMode == Common::kRenderMacintosh) {
			float uf[4] = {0.0f, 1.0f, 1.0f, 0.0f};
			float vf2[4] = {0.0f, 0.0f, 1.0f, 1.0f};
			if (macColors) {
				macFillPoly(uf, vf2, 4, 19); // c_upstairs1
			} else {
				_gfx->setStippleData(kStippleGray);
				wallPolygon(corners, uf, vf2, 4, 0);
				_gfx->setStippleData(nullptr);
			}
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
		// DOS: draw_dn_stairs  staircase descending into the wall with perspective
		const uint32 col = macColors ? (uint32)0xFF000000 : 0; // vBLACK

		// Mac: fill entire wall face (c_dnstairs)
		if (_renderMode == Common::kRenderMacintosh) {
			float uf[4] = {0.0f, 1.0f, 1.0f, 0.0f};
			float vf2[4] = {0.0f, 0.0f, 1.0f, 1.0f};
			if (macColors) {
				macFillPoly(uf, vf2, 4, 21); // c_dnstairs
			} else {
				_gfx->setStippleData(kStippleGray);
				wallPolygon(corners, uf, vf2, 4, 0);
				_gfx->setStippleData(nullptr);
			}
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
		const uint32 glyphColor = macColors ? (uint32)0xFF000000 : (macMode ? 0 : 8);

		// Mac: fill glyph area
		if (macMode) {
			float ug[4] = {0.0f, 1.0f, 1.0f, 0.0f};
			float vg[4] = {0.0f, 0.0f, 1.0f, 1.0f};
			if (macColors) {
				macFillPoly(ug, vg, 4, 22); // c_glyph
			} else {
				_gfx->setStippleData(kStippleGray);
				wallPolygon(corners, ug, vg, 4, 0);
				_gfx->setStippleData(nullptr);
			}
		}

		for (int i = 0; i < 7; i++) {
			float v = 0.2f + i * 0.1f;
			wallLine(corners, 0.2f, v, 0.8f, v, glyphColor);
		}
		break;
	}
	case kWallFeatureElevator: {
		const uint32 elevColor = macColors ? (uint32)0xFF000000 : (macMode ? 0 : 8);
		float xl = 0.2f, xr = 0.8f;
		float yb = 0.1f, yt = 0.9f;

		// Mac: fill elevator door
		if (macMode) {
			float ue[4] = {xl, xr, xr, xl};
			float ve[4] = {yb, yb, yt, yt};
			if (macColors) {
				macFillPoly(ue, ve, 4, 23); // c_elevator
			} else {
				_gfx->setStippleData(kStippleGray);
				wallPolygon(corners, ue, ve, 4, 0);
				_gfx->setStippleData(nullptr);
			}
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
		static const float uT[6] = { 0.0f,    0.0f,    1/6.0f,  5/6.0f,  1.0f,    1.0f };
		static const float vT[6] = { 0.0f,    0.750f,  0.875f,  0.875f,  0.750f,  0.0f };
		if (_renderMode == Common::kRenderMacintosh) {
			if (macColors) {
				macFillPoly(uT, vT, 6, 24); // c_tunnel
			} else {
				_gfx->setStippleData(kStippleGray);
				wallPolygon(corners, uT, vT, 6, 0);
				_gfx->setStippleData(nullptr);
			}
		} else {
			wallPolygon(corners, uT, vT, 6, 0); // vBLACK outline
		}
		break;
	}
	case kWallFeatureAirlock: {
		// Direct port of drawALOpen/drawALClosed from WALLFTRS.C / wallftrs.c.
		// These are the exact split7x7 positions on the wall face.
		static const float u[8] = {0.125f, 0.25f, 0.5f, 0.75f, 0.875f, 0.75f, 0.5f, 0.25f};
		static const float v[8] = {0.5f, 0.75f, 0.875f, 0.75f, 0.5f, 0.25f, 0.125f, 0.25f};
		static const float spokeU[8] = {0.375f, 0.5f, 0.625f, 0.625f, 0.625f, 0.5f, 0.375f, 0.375f};
		static const float spokeV[8] = {0.625f, 0.625f, 0.625f, 0.5f, 0.375f, 0.375f, 0.375f, 0.5f};
		static const float centerU = 0.5f;
		static const float centerV = 0.5f;

		if (map[1] == 0) {
			// Original drawALOpen: solid black opening on both DOS and Mac.
			if (macMode || !_wireframe)
				_gfx->setWireframe(true, 0);
			else
				_gfx->setWireframe(true);
			wallPolygon(corners, u, v, 8, 0);
		} else {
			// Mac: fill airlock when closed
			if (macMode) {
				if (macColors) {
					macFillPoly(u, v, 8, 25); // c_airlock
				} else {
					_gfx->setStippleData(kStippleGray);
					wallPolygon(corners, u, v, 8, 0);
					_gfx->setStippleData(nullptr);
				}
			} else if (!_wireframe) {
				const byte *stipple = setupMacPattern(_gfx, kPatternLtGray, 0, 15);
				wallPolygon(corners, u, v, 8, 0);
				if (stipple)
					_gfx->setStippleData(nullptr);
			}

			const uint32 airlockColor = macColors ? (uint32)0xFF000000 : (macMode ? 0 : 8);
			for (int i = 0; i < 8; i++) {
				int n = (i + 1) % 8;
				wallLine(corners, u[i], v[i], u[n], v[n], airlockColor);
			}
			for (int i = 0; i < 8; i++) {
				wallLine(corners, u[i], v[i], spokeU[i], spokeV[i], airlockColor);
				wallLine(corners, spokeU[i], spokeV[i], centerU, centerV, airlockColor);
			}
		}
		break;
	}
	case kWallFeatureColor: {
		// Mac drawColor / DOS drawColor: 4 horizontal bands.
		// map[1..4] = pattern ID per band (0=WHITE, 1=LTGRAY, 2=GRAY, 3=DKGRAY, 4=BLACK).
		// Values >= 5 trigger animation: color = (map[i+1] + _displayCount) % 5.
		// Band 0 (top): v=0.75..1.0, Band 1: v=0.5..0.75, Band 2: v=0.25..0.5, Band 3: v=0..0.25.
		if (_renderMode == Common::kRenderMacintosh) {
			if (macColors) {
				// Mac drawColor: map[i+1] selects color (0→c_color0..3→c_color3, 4→BLACK).
				// Values >= 5: animated = (map[i+1] + _displayCount) % 5.
				for (int i = 0; i < 4; i++) {
					int val = map[i + 1];
					if (val > 4)
						val = (val + _displayCount / 6) % 5;
					float vb = (3 - i) / 4.0f;
					float vt = (4 - i) / 4.0f;
					float ub[4] = {0.0f, 1.0f, 1.0f, 0.0f};
					float vb4[4] = {vb, vb, vt, vt};
					if (val == 4) {
						// BLACK: solid black fill
						_gfx->setWireframe(true, (uint32)0xFF000000);
						wallPolygon(corners, ub, vb4, 4, 0xFF000000);
						_gfx->setWireframe(true, packMacColor(_macColors[8 + _level - 1].fg));
					} else {
						macFillPoly(ub, vb4, 4, 26 + val); // c_color0 + val
					}
				}
			} else {
				static const byte *stripPatterns[5] = {
					nullptr, kStippleLtGray, kStippleGray, kStippleDkGray, nullptr
				};
				for (int i = 0; i < 4; i++) {
					int pat = map[i + 1];
					if (pat > 4)
						pat = (pat + _displayCount / 6) % 5; // animated cycling
					float vb = (3 - i) / 4.0f;
					float vt = (4 - i) / 4.0f;
					float ub[4] = {0.0f, 1.0f, 1.0f, 0.0f};
					float vb4[4] = {vb, vb, vt, vt};
					if (pat == 4) {
						_gfx->setWireframe(true, 0);
						wallPolygon(corners, ub, vb4, 4, 0);
						_gfx->setWireframe(true, 255);
					} else if (pat == 0) {
						// WHITE: no fill needed (wall background is white)
					} else {
						_gfx->setStippleData(stripPatterns[pat]);
						wallPolygon(corners, ub, vb4, 4, 0);
						_gfx->setStippleData(nullptr);
					}
				}
			}
		}

		// EGA / Mac B&W: colored lines at band boundaries.
		// DOS non-polyfill draws 3 lines; we animate line colors for bands > 4.
		// Mac color mode uses macFillPoly for band fills instead.
		if (!macColors) {
			for (int i = 1; i <= 3; i++) {
				int val = map[i];
				if (val > 4)
					val = (val + _displayCount / 6) % 5; // animated cycling
				uint32 c = 120 + val * 20;
				if (c == 120 && val == 0 && !map[1] && !map[2] && !map[3] && !map[4])
					c = 100 + (_level * 15);
				float v = (float)i / 4.0f;
				wallLine(corners, 0.0f, v, 1.0f, v, c);
			}
		}
		break;
	}
	default:
		break;
	}

	_gfx->setStippleData(nullptr);
	_gfx->setWireframe(true, wallFeatureFill);
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

bool ColonyEngine::drawStaticObjectPrisms3D(Thing &obj) {
	const auto drawPrism = [&](const PrismPartDef &def, bool useLook, int colorOverride = -1) {
		draw3DPrism(obj, def, useLook, colorOverride, true);
	};
	const auto drawSphere = [&](int pt0x, int pt0y, int pt0z, int pt1x, int pt1y, int pt1z,
	                            uint32 fillColor, uint32 outlineColor) {
		draw3DSphere(obj, pt0x, pt0y, pt0z, pt1x, pt1y, pt1z, fillColor, outlineColor, true);
	};

	switch (obj.type) {
	case kObjConsole:
		drawPrism(kConsolePart, false);
		break;
	case kObjCChair:
		for (int i = 0; i < 5; i++) {
			_gfx->setDepthRange((4 - i) * 0.002, 1.0);
			drawPrism(kCChairParts[i], false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kObjPlant:
		// DOS MakePlant draw order: top pot, then green leaf lines, then pot on top.
		drawPrism(kPlantParts[1], false); // top pot (soil)
		for (int i = 2; i < 8; i++)
			draw3DLeaf(obj, kPlantParts[i]); // leaves as lines
		drawPrism(kPlantParts[0], false); // pot (drawn last, on top)
		break;
	case kObjCouch:
	case kObjChair: {
		const PrismPartDef *parts = (obj.type == kObjCouch) ? kCouchParts : kChairParts;
		for (int i = 0; i < 4; i++) {
			_gfx->setDepthRange((3 - i) * 0.002, 1.0);
			drawPrism(parts[i], false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	}
	case kObjTV:
		_gfx->setDepthRange(0.002, 1.0); // body base layer (pushed back)
		drawPrism(kTVParts[0], false);
		_gfx->setDepthRange(0.0, 1.0);   // screen on top of body face
		drawPrism(kTVParts[1], false);
		break;
	case kObjDrawer:
		for (int i = 0; i < 2; i++) {
			_gfx->setDepthRange((1 - i) * 0.002, 1.0);
			drawPrism(kDrawerParts[i], false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kObjFWall:
		if (_renderMode == Common::kRenderMacintosh && _hasMacColors)
			drawPrism(kFWallPart, false, kColorCorridorWall);
		else
			drawPrism(kFWallPart, false);
		break;
	case kObjCWall:
		if (_renderMode == Common::kRenderMacintosh && _hasMacColors)
			drawPrism(kCWallPart, false, kColorCorridorWall);
		else
			drawPrism(kCWallPart, false);
		break;
	case kObjScreen:
		drawPrism(kScreenPart, false);
		break;
	case kObjTable:
		for (int i = 0; i < 2; i++) {
			_gfx->setDepthRange((1 - i) * 0.002, 1.0);
			drawPrism(kTableParts[i], false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kObjBed:
	case kObjBBed: {
		const PrismPartDef *parts = (obj.type == kObjBBed) ? kBBedParts : kBedParts;
		for (int i = 0; i < 3; i++) {
			_gfx->setDepthRange((2 - i) * 0.002, 1.0);
			drawPrism(parts[i], false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	}
	case kObjDesk:
		for (int i = 0; i < 10; i++) {
			_gfx->setDepthRange((9 - i) * 0.002, 1.0);
			drawPrism(kDeskParts[i], false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kObjBox1:
		drawPrism(kBox1Part, false);
		break;
	case kObjBench:
		drawPrism(kBenchPart, false);
		break;
	case kObjCBench:
		for (int i = 0; i < 2; i++) {
			_gfx->setDepthRange((1 - i) * 0.002, 1.0);
			drawPrism(kCBenchParts[i], false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kObjBox2:
		_gfx->setDepthRange(0.002, 1.0);
		drawPrism(kBox2Parts[1], false); // base first
		_gfx->setDepthRange(0.0, 1.0);
		drawPrism(kBox2Parts[0], false); // top second
		break;
	case kObjReactor: {
		// MakeReactor: animate core height and recolor only the original
		// side faces. The reactor body stays c_reactor and the core cap stays c_ccore.
		switch (_coreState[_coreIndex]) {
		case 0: if (_coreHeight[_coreIndex] < 256) _coreHeight[_coreIndex] += 16; break;
		case 1: if (_coreHeight[_coreIndex] > 0) _coreHeight[_coreIndex] -= 16; break;
		case 2: _coreHeight[_coreIndex] = 0; break;
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
		static const int kRingColors[] = {kColorRainbow1, kColorRainbow2, kColorRainbow3, kColorRainbow4};
		const int ringColor = kRingColors[_displayCount % 4];
		for (int i = 0; i < 4; ++i) {
			modRingSurf[i][0] = ringColor;
			modTopSurf[i][0] = ringColor;
		}

		// Only the 6 core side faces animate. The top face remains c_ccore.
		static const int kCoreCycle[] = {kColorHCore1, kColorHCore2, kColorHCore3, kColorHCore4, kColorHCore3, kColorHCore2};
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
		drawPrism(modTopDef, false);
		_gfx->setDepthRange(0.002, 1.0);
		drawPrism(modRingDef, false);
		if (_coreState[_coreIndex] < 2) {
			_gfx->setDepthRange(0.0, 1.0);
			drawPrism(modCoreDef, false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	}
	case kObjPowerSuit: {
		// MakePowerSuit: part[4] (power source hexagon) surface[0] pulsates.
		// Mac: pcycle[count%6]; DOS: pcycle[count%8]
		static const int kSuitCycle[] = {kColorHCore1, kColorHCore2, kColorHCore3, kColorHCore4, kColorHCore3, kColorHCore2};
		int sourceColor = kSuitCycle[_displayCount % 6];

		for (int i = 0; i < 4; i++) {
			_gfx->setDepthRange((4 - i) * 0.002, 1.0);
			drawPrism(kPowerSuitParts[i], false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		drawPrism(kPowerSuitParts[4], false, sourceColor);
		break;
	}
	case kObjTeleport:
		drawPrism(kTelePart, false);
		break;
	case kObjCryo:
		_gfx->setDepthRange(0.002, 1.0);
		drawPrism(kCryoParts[1], false); // base first
		_gfx->setDepthRange(0.0, 1.0);
		drawPrism(kCryoParts[0], false); // top second
		break;
	case kObjProjector:
		// Projector sits on table  draw table first, then projector parts
		_gfx->setDepthRange(0.008, 1.0);
		drawPrism(kTableParts[0], false); // table base
		_gfx->setDepthRange(0.006, 1.0);
		drawPrism(kTableParts[1], false); // table top
		_gfx->setDepthRange(0.004, 1.0);
		drawPrism(kProjectorParts[1], false); // stand
		_gfx->setDepthRange(0.002, 1.0);
		drawPrism(kProjectorParts[0], false); // body
		_gfx->setDepthRange(0.0, 1.0);
		drawPrism(kProjectorParts[2], false); // lens
		break;
	case kObjTub:
		for (int i = 0; i < 2; i++) {
			_gfx->setDepthRange((1 - i) * 0.002, 1.0);
			drawPrism(kTubParts[i], false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kObjSink:
		for (int i = 0; i < 3; i++) {
			_gfx->setDepthRange((2 - i) * 0.002, 1.0);
			drawPrism(kSinkParts[i], false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kObjToilet:
		for (int i = 0; i < 4; i++) {
			_gfx->setDepthRange((3 - i) * 0.002, 1.0);
			drawPrism(kToiletParts[i], false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kObjPToilet:
		for (int i = 0; i < 5; i++) {
			_gfx->setDepthRange((4 - i) * 0.002, 1.0);
			drawPrism(kPToiletParts[i], false);
		}
		_gfx->setDepthRange(0.0, 1.0);
		break;
	case kObjForkLift:
		// Draw order: forks, arms, treads, cab (back-to-front)
		_gfx->setDepthRange(0.010, 1.0);
		drawPrism(kForkliftParts[3], false); // FLLL (left fork)
		_gfx->setDepthRange(0.008, 1.0);
		drawPrism(kForkliftParts[2], false); // FLUL (left arm)
		_gfx->setDepthRange(0.006, 1.0);
		drawPrism(kForkliftParts[5], false); // FLLR (right fork)
		_gfx->setDepthRange(0.004, 1.0);
		drawPrism(kForkliftParts[4], false); // FLUR (right arm)
		_gfx->setDepthRange(0.002, 1.0);
		drawPrism(kForkliftParts[1], false); // treads
		_gfx->setDepthRange(0.0, 1.0);
		drawPrism(kForkliftParts[0], false); // cab
		break;
	// === Robot types (1-20) ===
	case kRobEye:
		drawSphere(0, 0, 100, 0, 0, 200, 15, 15); // ball: white
		drawPrism(kEyeIrisDef, false);
		drawPrism(kEyePupilDef, false);
		break;
	case kRobPyramid:
		drawPrism(kPShadowDef, false);
		drawPrism(kPyramidBodyDef, false);
		drawSphere(0, 0, 175, 0, 0, 200, 15, 15); // ball on top
		drawPrism(kPIrisDef, false);
		drawPrism(kPPupilDef, false);
		break;
	case kRobCube:
		drawPrism(kCubeBodyDef, false);
		break;
	case kRobUPyramid:
		drawPrism(kUPShadowDef, false);
		drawPrism(kUPyramidBodyDef, false);
		break;
	case kRobFEye:
		drawSphere(0, 0, 0, 0, 0, 100, 15, 15);
		drawPrism(kFEyeIrisDef, false);
		drawPrism(kFEyePupilDef, false);
		break;
	case kRobFPyramid:
		drawPrism(kFPyramidBodyDef, false);
		break;
	case kRobFCube:
		drawPrism(kFCubeBodyDef, false);
		break;
	case kRobFUPyramid:
		drawPrism(kFUPyramidBodyDef, false);
		break;
	case kRobSEye:
		drawSphere(0, 0, 0, 0, 0, 50, 15, 15);
		drawPrism(kSEyeIrisDef, false);
		drawPrism(kSEyePupilDef, false);
		break;
	case kRobSPyramid:
		drawPrism(kSPyramidBodyDef, false);
		break;
	case kRobSCube:
		drawPrism(kSCubeBodyDef, false);
		break;
	case kRobSUPyramid:
		drawPrism(kSUPyramidBodyDef, false);
		break;
	case kRobMEye:
		drawSphere(0, 0, 0, 0, 0, 25, 15, 15);
		drawPrism(kMEyeIrisDef, false);
		drawPrism(kMEyePupilDef, false);
		break;
	case kRobMPyramid:
		drawPrism(kMPyramidBodyDef, false);
		break;
	case kRobMCube:
		drawPrism(kMCubeBodyDef, false);
		break;
	case kRobMUPyramid:
		drawPrism(kMUPyramidBodyDef, false);
		break;
	case kRobQueen:
		drawPrism(kQThoraxDef, false);
		drawPrism(kQAbdomenDef, false);
		drawPrism(kQLWingDef, false);
		drawPrism(kQRWingDef, false);
		drawSphere(0, 0, 130, 0, 0, 155, 15, 15); // queen ball
		drawPrism(kQIrisDef, false);
		drawPrism(kQPupilDef, false);
		break;
	case kRobDrone:
	case kRobSoldier:
		drawPrism(kDAbdomenDef, false);
		drawPrism(kDLLPincerDef, false);
		drawPrism(kDRRPincerDef, false);
		drawPrism(kDLEyeDef, false);
		drawPrism(kDREyeDef, false);
		break;
	case kRobSnoop:
		drawPrism(kSnoopAbdomenDef, false);
		drawPrism(kSnoopHeadDef, false);
		break;
	default:
		return false;
	}
	return true;
}

} // End of namespace Colony
