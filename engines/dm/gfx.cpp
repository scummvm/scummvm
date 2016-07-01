/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#include "engines/util.h"
#include "common/system.h"
#include "common/file.h"
#include "graphics/palette.h"
#include "common/endian.h"

#include "gfx.h"
#include "dungeonman.h"
#include "group.h"
#include "timeline.h"


namespace DM {

Frame g0164Frame_DoorFrameLeft_D3L = Frame(0, 31, 28, 70, 16, 43, 0, 0); // @ G0164_s_Graphic558_Frame_DoorFrameLeft_D3L
Frame g0165Frame_DoorFrameRight_D3R = Frame(192, 223, 28, 70, 16, 43, 0, 0); // @ G0165_s_Graphic558_Frame_DoorFrameRight_D3R
Frame g0166Frame_DoorFrameLeft_D3C = Frame(64, 95, 27, 70, 16, 44, 0, 0); // @ G0166_s_Graphic558_Frame_DoorFrameLeft_D3C
Frame g0167Frame_DoorFrameRight_D3C = Frame(128, 159, 27, 70, 16, 44, 0, 0); // @ G0167_s_Graphic558_Frame_DoorFrameRight_D3C
Frame g0168Frame_DoorFrameLeft_D2C = Frame(48, 95, 22, 86, 24, 65, 0, 0); // @ G0168_s_Graphic558_Frame_DoorFrameLeft_D2C
Frame g0169Frame_DoorFrameRight_D2C = Frame(128, 175, 22, 86, 24, 65, 0, 0); // @ G0169_s_Graphic558_Frame_DoorFrameRight_D2C
Frame g0170Frame_DoorFrameLeft_D1C = Frame(43, 74, 14, 107, 16, 94, 0, 0); // @ G0170_s_Graphic558_Frame_DoorFrameLeft_D1C
Frame g0171Frame_DoorFrameRight_D1C = Frame(149, 180, 14, 107, 16, 94, 0, 0); // @ G0171_s_Graphic558_Frame_DoorFrameRight_D1C
Frame g0172Frame_DoorFrame_D0C = Frame(96, 127, 0, 122, 16, 123, 0, 0); // @ G0172_s_Graphic558_Frame_DoorFrame_D0C
Frame g0173Frame_DoorFrameTop_D2L = Frame(0, 59, 22, 24, 48, 3, 16, 0); // @ G0173_s_Graphic558_Frame_DoorFrameTop_D2L
Frame g0174Frame_DoorFrameTop_D2C = Frame(64, 159, 22, 24, 48, 3, 0, 0); // @ G0174_s_Graphic558_Frame_DoorFrameTop_D2C
Frame g0175Frame_DoorFrameTop_D2R = Frame(164, 223, 22, 24, 48, 3, 16, 0); // @ G0175_s_Graphic558_Frame_DoorFrameTop_D2R
Frame g0176Frame_DoorFrameTop_D1L = Frame(0, 31, 14, 17, 64, 4, 16, 0); // @ G0176_s_Graphic558_Frame_DoorFrameTop_D1L
Frame g0177Frame_DoorFrameTop_D1C = Frame(48, 175, 14, 17, 64, 4, 0, 0); // @ G0177_s_Graphic558_Frame_DoorFrameTop_D1C
Frame g0178Frame_DoorFrameTop_D1R = Frame(192, 223, 14, 17, 64, 4, 16, 0); // @ G0178_s_Graphic558_Frame_DoorFrameTop_D1R
Frame g0140FrameFloorPit_D3L = Frame(0, 79, 66, 73, 40, 8, 0, 0); // @ G0140_s_Graphic558_Frame_FloorPit_D3L
Frame g0141FrameFloorPit_D3C = Frame(64, 159, 66, 73, 48, 8, 0, 0); // @ G0141_s_Graphic558_Frame_FloorPit_D3C
Frame g0142FrameFloorPit_D3R = Frame(144, 223, 66, 73, 40, 8, 0, 0); // @ G0142_s_Graphic558_Frame_FloorPit_D3R
Frame g0143FrameFloorPit_D2L = Frame(0, 79, 77, 88, 40, 12, 0, 0); // @ G0143_s_Graphic558_Frame_FloorPit_D2L
Frame g0144FrameFloorPit_D2C = Frame(64, 159, 77, 88, 48, 12, 0, 0); // @ G0144_s_Graphic558_Frame_FloorPit_D2C
Frame g0145FrameFloorPit_D2R = Frame(144, 223, 77, 88, 40, 12, 0, 0); // @ G0145_s_Graphic558_Frame_FloorPit_D2R
Frame g0146FrameFloorPit_D1L = Frame(0, 63, 93, 116, 32, 24, 0, 0); // @ G0146_s_Graphic558_Frame_FloorPit_D1L
Frame g0147FrameFloorPit_D1C = Frame(32, 191, 93, 116, 80, 24, 0, 0); // @ G0147_s_Graphic558_Frame_FloorPit_D1C
Frame g0148FrameFloorPit_D1R = Frame(160, 223, 93, 116, 32, 24, 0, 0); // @ G0148_s_Graphic558_Frame_FloorPit_D1R
Frame g0149FrameFloorPit_D0L = Frame(0, 31, 124, 135, 16, 12, 0, 0); // @ G0149_s_Graphic558_Frame_FloorPit_D0L
Frame g0150FrameFloorPit_D0C = Frame(16, 207, 124, 135, 96, 12, 0, 0); // @ G0150_s_Graphic558_Frame_FloorPit_D0C
Frame g0151FrameFloorPit_D0R = Frame(192, 223, 124, 135, 16, 12, 0, 0); // @ G0151_s_Graphic558_Frame_FloorPit_D0R
Frame g0152FrameFloorPit_D2L = Frame(0, 79, 19, 23, 40, 5, 0, 0); // @ G0152_s_Graphic558_Frame_CeilingPit_D2L
Frame g0153FrameFloorPit_D2C = Frame(64, 159, 19, 23, 48, 5, 0, 0); // @ G0153_s_Graphic558_Frame_CeilingPit_D2C
Frame g0154FrameFloorPit_D2R = Frame(144, 223, 19, 23, 40, 5, 0, 0); // @ G0154_s_Graphic558_Frame_CeilingPit_D2R
Frame g0155FrameFloorPit_D1L = Frame(0, 63, 8, 16, 32, 9, 0, 0); // @ G0155_s_Graphic558_Frame_CeilingPit_D1L
Frame g0156FrameFloorPit_D1C = Frame(32, 191, 8, 16, 80, 9, 0, 0); // @ G0156_s_Graphic558_Frame_CeilingPit_D1C
Frame g0157FrameFloorPit_D1R = Frame(160, 223, 8, 16, 32, 9, 0, 0); // @ G0157_s_Graphic558_Frame_CeilingPit_D1R
Frame g0158FrameFloorPit_D0L = Frame(0, 15, 0, 3, 8, 4, 0, 0); // @ G0158_s_Graphic558_Frame_CeilingPit_D0L
Frame g0159FrameFloorPit_D0C = Frame(16, 207, 0, 3, 96, 4, 0, 0); // @ G0159_s_Graphic558_Frame_CeilingPit_D0C
Frame g0160FrameFloorPit_D0R = Frame(208, 223, 0, 3, 8, 4, 0, 0); // @ G0160_s_Graphic558_Frame_CeilingPit_D0R

FieldAspect gFieldAspects[12] = { // @ G0188_as_Graphic558_FieldAspects
								  /* { NativeBitmapRelativeIndex, BaseStartUnitIndex, Transparent color, Mask, ByteWidth, Height, X, BitPlaneWordCount } */
	FieldAspect(0, 63, 0x8A, 0xFF,  0,   0,  0, 64),   /* D3C */
	FieldAspect(0, 63, 0x0A, 0x80, 48,  51, 11, 64),   /* D3L */
	FieldAspect(0, 63, 0x0A, 0x00, 48,  51,  0, 64),   /* D3R */
	FieldAspect(0, 60, 0x8A, 0xFF,  0,   0,  0, 64),   /* D2C */
	FieldAspect(0, 63, 0x0A, 0x81, 40,  71,  5, 64),   /* D2L */
	FieldAspect(0, 63, 0x0A, 0x01, 40,  71,  0, 64),   /* D2R */
	FieldAspect(0, 61, 0x8A, 0xFF,  0,   0,  0, 64),   /* D1C */
	FieldAspect(0, 63, 0x0A, 0x82, 32, 111,  0, 64),   /* D1L */
	FieldAspect(0, 63, 0x0A, 0x02, 32, 111,  0, 64),   /* D1R */
	FieldAspect(0, 59, 0x8A, 0xFF,  0,   0,  0, 64),   /* D0C */
	FieldAspect(0, 63, 0x0A, 0x83, 16, 136,  0, 64),   /* D0L */
	FieldAspect(0, 63, 0x0A, 0x03, 16, 136,  0, 64)}; /* D0R */

Box gBoxMovementArrows = Box(224, 319, 124, 168);

byte gPalChangeSmoke[16] = {0, 10, 20, 30, 40, 50, 120, 10, 80, 90, 100, 110, 120, 130, 140, 150}; // @ G0212_auc_Graphic558_PaletteChanges_Smoke

ExplosionAspect gExplosionAspects[kExplosionAspectCount] = { // @ G0211_as_Graphic558_ExplosionAspects
	/* { ByteWidth, Height } */
	ExplosionAspect(80, 111),   /* Fire   */
ExplosionAspect(64,  97),   /* Spell  */
ExplosionAspect(80,  91),   /* Poison */
ExplosionAspect(80,  91)}; /* Death  */

#define kDerivedBitmapMaximumCount 730 // @ C730_DERIVED_BITMAP_MAXIMUM_COUNT

byte gProjectileScales[7] = {
	13,   /* D4 Back  */
	16,   /* D4 Front */
	19,   /* D3 Back  */
	22,   /* D3 Front */
	25,   /* D2 Back  */
	28,   /* D2 Front */
	32}; /* D1 Back  */

enum StairFrameIndex {
	kFrameStairsUpFront_D3L = 0, // @ G0110_s_Graphic558_Frame_StairsUpFront_D3L 
	kFrameStairsUpFront_D3C = 1, // @ G0111_s_Graphic558_Frame_StairsUpFront_D3C 
	kFrameStairsUpFront_D3R = 2, // @ G0112_s_Graphic558_Frame_StairsUpFront_D3R 
	kFrameStairsUpFront_D2L = 3, // @ G0113_s_Graphic558_Frame_StairsUpFront_D2L 
	kFrameStairsUpFront_D2C = 4, // @ G0114_s_Graphic558_Frame_StairsUpFront_D2C 
	kFrameStairsUpFront_D2R = 5, // @ G0115_s_Graphic558_Frame_StairsUpFront_D2R 
	kFrameStairsUpFront_D1L = 6, // @ G0116_s_Graphic558_Frame_StairsUpFront_D1L 
	kFrameStairsUpFront_D1C = 7, // @ G0117_s_Graphic558_Frame_StairsUpFront_D1C 
	kFrameStairsUpFront_D1R = 8, // @ G0118_s_Graphic558_Frame_StairsUpFront_D1R 
	kFrameStairsUpFront_D0L = 9, // @ G0119_s_Graphic558_Frame_StairsUpFront_D0L 
	kFrameStairsUpFront_D0R = 10, // @ G0120_s_Graphic558_Frame_StairsUpFront_D0R 
	kFrameStairsDownFront_D3L = 11, // @ G0121_s_Graphic558_Frame_StairsDownFront_D3L 
	kFrameStairsDownFront_D3C = 12, // @ G0122_s_Graphic558_Frame_StairsDownFront_D3C 
	kFrameStairsDownFront_D3R = 13, // @ G0123_s_Graphic558_Frame_StairsDownFront_D3R 
	kFrameStairsDownFront_D2L = 14, // @ G0124_s_Graphic558_Frame_StairsDownFront_D2L 
	kFrameStairsDownFront_D2C = 15, // @ G0125_s_Graphic558_Frame_StairsDownFront_D2C 
	kFrameStairsDownFront_D2R = 16, // @ G0126_s_Graphic558_Frame_StairsDownFront_D2R 
	kFrameStairsDownFront_D1L = 17, // @ G0127_s_Graphic558_Frame_StairsDownFront_D1L 
	kFrameStairsDownFront_D1C = 18, // @ G0128_s_Graphic558_Frame_StairsDownFront_D1C 
	kFrameStairsDownFront_D1R = 19, // @ G0129_s_Graphic558_Frame_StairsDownFront_D1R 
	kFrameStairsDownFront_D0L = 20, // @ G0130_s_Graphic558_Frame_StairsDownFront_D0L 
	kFrameStairsDownFront_D0R = 21, // @ G0131_s_Graphic558_Frame_StairsDownFront_D0R 
	kFrameStairsSide_D2L = 22, // @ G0132_s_Graphic558_Frame_StairsSide_D2L 
	kFrameStairsSide_D2R = 23, // @ G0133_s_Graphic558_Frame_StairsSide_D2R 
	kFrameStairsUpSide_D1L = 24, // @ G0134_s_Graphic558_Frame_StairsUpSide_D1L 
	kFrameStairsUpSide_D1R = 25, // @ G0135_s_Graphic558_Frame_StairsUpSide_D1R 
	kFrameStairsDownSide_D1L = 26, // @ G0136_s_Graphic558_Frame_StairsDownSide_D1L 
	kFrameStairsDownSide_D1R = 27, // @ G0137_s_Graphic558_Frame_StairsDownSide_D1R 
	kFrameStairsSide_D0L = 28, // @ G0138_s_Graphic558_Frame_StairsSide_D0L 
	kFrameStairsSide_D0R = 29 // @ G0139_s_Graphic558_Frame_StairsSide_D0R 
};

Frame gStairFrames[] = {
	Frame(0, 79, 25, 70, 40, 46, 0, 0),
	Frame(64, 159, 25, 70, 48, 46, 0, 0),
	Frame(149, 223, 25, 70, 40, 46, 5, 0),
	Frame(0, 63, 22, 83, 32, 62, 0, 0),
	Frame(64, 159, 22, 83, 48, 62, 0, 0),
	Frame(160, 223, 22, 83, 32, 62, 0, 0),
	Frame(0, 31, 9, 108, 16, 100, 0, 0),
	Frame(32, 191, 9, 108, 80, 100, 0, 0),
	Frame(192, 223, 9, 108, 16, 100, 0, 0),
	Frame(0, 31, 58, 101, 16, 44, 0, 0),
	Frame(192, 223, 58, 101, 16, 44, 0, 0),
	Frame(0, 79, 28, 68, 40, 41, 0, 0),
	Frame(64, 159, 28, 70, 48, 43, 0, 0),
	Frame(149, 223, 28, 68, 40, 41, 5, 0),
	Frame(0, 63, 24, 85, 32, 62, 0, 0),
	Frame(64, 159, 24, 85, 48, 62, 0, 0),
	Frame(160, 223, 24, 85, 32, 62, 0, 0),
	Frame(0, 31, 18, 108, 16, 91, 0, 0),
	Frame(32, 191, 18, 108, 80, 91, 0, 0),
	Frame(192, 223, 18, 108, 16, 91, 0, 0),
	Frame(0, 31, 76, 135, 16, 60, 0, 0),
	Frame(192, 223, 76, 135, 16, 60, 0, 0),
	Frame(60, 75, 57, 61, 8, 5, 0, 0),
	Frame(148, 163, 57, 61, 8, 5, 0, 0),
	Frame(32, 63, 57, 99, 16, 43, 0, 0),
	Frame(160, 191, 57, 99, 16, 43, 0, 0),
	Frame(32, 63, 60, 98, 16, 39, 0, 0),
	Frame(160, 191, 60, 98, 16, 39, 0, 0),
	Frame(0, 15, 73, 85, 8, 13, 0, 0),
	Frame(208, 223, 73, 85, 8, 13, 0, 0)
};

#define kFirstWallOrn 121 // @ C121_GRAPHIC_FIRST_WALL_ORNAMENT
#define kFirstFloorSet 75 // @ C075_GRAPHIC_FIRST_FLOOR_SET
#define kFirstWallSet 77 // @ C077_GRAPHIC_FIRST_WALL_SET
#define kFirstStairs 90 // @ C090_GRAPHIC_FIRST_STAIRS
#define kFirstDoorSet 108 // @ C108_GRAPHIC_FIRST_DOOR_SET
#define kInscriptionFont 120 // @ C120_GRAPHIC_INSCRIPTION_FONT
#define kFirstWallOrn 121 // @ C121_GRAPHIC_FIRST_WALL_ORNAMENT
#define kFirstFloorOrn 247 // @ C247_GRAPHIC_FIRST_FLOOR_ORNAMENT
#define kFirstDoorOrn 303 // @ C303_GRAPHIC_FIRST_DOOR_ORNAMENT


enum ViewSquare {
	kViewSquare_D4C = -3, // @ CM3_VIEW_SQUARE_D4C
	kViewSquare_D4L = -2, // @ CM2_VIEW_SQUARE_D4L
	kViewSquare_D4R = -1, // @ CM1_VIEW_SQUARE_D4R
	kViewSquare_D3C = 0, // @ C00_VIEW_SQUARE_D3C
	kViewSquare_D3L = 1, // @ C01_VIEW_SQUARE_D3L
	kViewSquare_D3R = 2, // @ C02_VIEW_SQUARE_D3R
	kViewSquare_D2C = 3, // @ C03_VIEW_SQUARE_D2C
	kViewSquare_D2L = 4, // @ C04_VIEW_SQUARE_D2L
	kViewSquare_D2R = 5, // @ C05_VIEW_SQUARE_D2R
	kViewSquare_D1C = 6, // @ C06_VIEW_SQUARE_D1C
	kViewSquare_D1L = 7, // @ C07_VIEW_SQUARE_D1L
	kViewSquare_D1R = 8, // @ C08_VIEW_SQUARE_D1R
	kViewSquare_D0C = 9, // @ C09_VIEW_SQUARE_D0C
	kViewSquare_D0L = 10, // @ C10_VIEW_SQUARE_D0L
	kViewSquare_D0R = 11, // @ C11_VIEW_SQUARE_D0R
	kViewSquare_D3C_Explosion = 3, // @ C03_VIEW_SQUARE_D3C_EXPLOSION
	kViewSquare_D3L_Explosion = 4, // @ C04_VIEW_SQUARE_D3L_EXPLOSION
	kViewSquare_D1C_Explosion = 9, // @ C09_VIEW_SQUARE_D1C_EXPLOSION
	kViewSquare_D0C_Explosion = 12 // @ C12_VIEW_SQUARE_D0C_EXPLOSION
};


Frame gCeilingFrame(0, 223, 0, 28, 112, 29, 0, 0); // @ K0012_s_Frame_Ceiling
Frame gFloorFrame(0, 223, 66, 135, 112, 70, 0, 0); // @ K0013_s_Frame_Floor

Frame gFrameWall_D3L2 = Frame(0, 15, 25, 73, 8, 49, 0, 0); // @ G0711_s_Graphic558_Frame_Wall_D3L2
Frame gFrameWall_D3R2 = Frame(208, 223, 25, 73, 8, 49, 0, 0); // @ G0712_s_Graphic558_Frame_Wall_D3R2
Frame gFrameWalls[12] = { // @ G0163_as_Graphic558_Frame_Walls
	/* { X1, X2, Y1, Y2, pixelWidth, Height, X, Y } */
	Frame(74, 149, 25,  75,  64,  51,  18, 0),   /* D3C */
	Frame(0,  83, 25,  75,  64,  51,  32, 0),   /* D3L */
	Frame(139, 223, 25,  75,  64,  51,   0, 0),   /* D3R */
	Frame(60, 163, 20,  90,  72,  71,  16, 0),   /* D2C */
	Frame(0,  74, 20,  90,  72,  71,  61, 0),   /* D2L */
	Frame(149, 223, 20,  90,  72,  71,   0, 0),   /* D2R */
	Frame(32, 191,  9, 119, 128, 111,  48, 0),   /* D1C */
	Frame(0,  63,  9, 119, 128, 111, 192, 0),   /* D1L */
	Frame(160, 223,  9, 119, 128, 111,   0, 0),   /* D1R */
	Frame(0, 223,  0, 135,   0,   0,   0, 0),   /* D0C */
	Frame(0,  31,  0, 135,  16, 136,   0, 0),   /* D0L */
	Frame(192, 223,  0, 135,  16, 136,   0, 0)
}; /* D0R */

enum WallSetIndices {
	kDoorFrameFront = 0, // @  G0709_puc_Bitmap_WallSet_DoorFrameFront
	kDoorFrameLeft_D1C = 1, // @  G0708_puc_Bitmap_WallSet_DoorFrameLeft_D1C
	kDoorFameLeft_D2C = 2, // @  G0707_puc_Bitmap_WallSet_DoorFrameLeft_D2C
	kDoorFrameLeft_D3C = 3, // @  G0706_puc_Bitmap_WallSet_DoorFrameLeft_D3C
	kDoorFrameLeft_D3L = 4, // @  G0705_puc_Bitmap_WallSet_DoorFrameLeft_D3L
	kDoorFrameTop_D1LCR = 5, // @  G0704_puc_Bitmap_WallSet_DoorFrameTop_D1LCR
	kDoorFrameTop_D2LCR = 6, // @  G0703_puc_Bitmap_WallSet_DoorFrameTop_D2LCR
	kWall_D0R = 7, // @  G0702_puc_Bitmap_WallSet_Wall_D0R
	kWall_D0L = 8, // @  G0701_puc_Bitmap_WallSet_Wall_D0L
	kWall_D1LCR = 9, // @  G0700_puc_Bitmap_WallSet_Wall_D1LCR
	kWall_D2LCR = 10, // @  G0699_puc_Bitmap_WallSet_Wall_D2LCR
	kWall_D3LCR = 11, // @  G0698_puc_Bitmap_WallSet_Wall_D3LCR
	kWall_D3L2 = 12, // @  G0697_puc_Bitmap_WallSet_Wall_D3L2

	kWall_D3R2 = 13, // @  G0696_puc_Bitmap_WallSet_Wall_D3R2
	kDoorFrameRight_D1C = 14, // @  G0710_puc_Bitmap_WallSet_DoorFrameRight_D1C

	kWall_D0R_Flipped = 15,
	kWall_D0L_Flipped = 16,
	kWall_D1LCR_Flipped = 17,
	kWall_D2LCR_Flipped = 18,
	kWall_D3LCR_Flipped = 19,

	kWall_D0R_Native = 20,
	kWall_D0L_Native = 21,
	kWall_D1LCR_Native = 22,
	kWall_D2LCR_Native = 23,
	kWall_D3LCR_Native = 24
};


byte gDoorOrnCoordIndices[12] = { // @ G0196_auc_Graphic558_DoorOrnamentCoordinateSetIndices
	0,   /* Door Ornament #00 Square Grid */
	1,   /* Door Ornament #01 Iron Bars */
	1,   /* Door Ornament #02 Jewels */
	1,   /* Door Ornament #03 Wooden Bars */
	0,   /* Door Ornament #04 Arched Grid */
	2,   /* Door Ornament #05 Block Lock */
	3,   /* Door Ornament #06 Corner Lock */
	1,   /* Door Ornament #07 Black door */
	2,   /* Door Ornament #08 Red Triangle Lock */
	2,   /* Door Ornament #09 Triangle Lock */
	1,   /* Door Ornament #10 Ra Door */
	1}; /* Door Ornament #11 Iron Door Damages */

byte gFloorOrnCoordSetIndices[9] = { // @ G0195_auc_Graphic558_FloorOrnamentCoordinateSetIndices
	0,   /* Floor Ornament 00 Square Grate */
	0,   /* Floor Ornament 01 Square Pressure Pad */
	0,   /* Floor Ornament 02 Moss */
	0,   /* Floor Ornament 03 Round Grate */
	2,   /* Floor Ornament 04 Round Pressure Plate */
	0,   /* Floor Ornament 05 Black Flame Pit */
	0,   /* Floor Ornament 06 Crack */
	2,   /* Floor Ornament 07 Tiny Pressure Pad */
	0}; /* Floor Ornament 08 Puddle */

uint16 gWallOrnCoordSets[8][13][6] = { // @ G0205_aaauc_Graphic558_WallOrnamentCoordinateSets
	/* { X1, X2, Y1, Y2, PixelWidth, Height } */
	{{80,  83, 41,  45,  8 * 2,   5},     /* D3L */
	{140, 143, 41,  45,  8 * 2,   5},     /* D3R */
	{16,  29, 39,  50,  8 * 2,  12},     /* D3L */
	{107, 120, 39,  50,  8 * 2,  12},     /* D3C */
	{187, 200, 39,  50,  8 * 2,  12},     /* D3R */
	{67,  77, 40,  49,  8 * 2,  10},     /* D2L */
	{146, 156, 40,  49,  8 * 2,  10},     /* D2R */
	{0,  17, 38,  55, 16 * 2,  18},     /* D2L */
	{102, 123, 38,  55, 16 * 2,  18},     /* D2C */
	{206, 223, 38,  55, 16 * 2,  18},     /* D2R */
	{48,  63, 38,  56,  8 * 2,  19},     /* D1L */
	{160, 175, 38,  56,  8 * 2,  19},     /* D1R */
	{96, 127, 36,  63, 16 * 2,  28}},   /* D1C */
	{{74,  82, 41,  60,  8 * 2,  20},     /* D3L */
	{141, 149, 41,  60,  8 * 2,  20},     /* D3R */
	{1,  47, 37,  63, 24 * 2,  27},     /* D3L */
	{88, 134, 37,  63, 24 * 2,  27},     /* D3C */
	{171, 217, 37,  63, 24 * 2,  27},     /* D3R */
	{61,  76, 38,  67,  8 * 2,  30},     /* D2L */
	{147, 162, 38,  67,  8 * 2,  30},     /* D2R */
	{0,  43, 37,  73, 32 * 2,  37},     /* D2L */
	{80, 143, 37,  73, 32 * 2,  37},     /* D2C */
	{180, 223, 37,  73, 32 * 2,  37},     /* D2R */
	{32,  63, 36,  83, 16 * 2,  48},     /* D1L */
	{160, 191, 36,  83, 16 * 2,  48},     /* D1R */
	{64, 159, 36,  91, 48 * 2,  56}},   /* D1C */
	{{80,  83, 66,  70,  8 * 2,   5},     /* D3L */
	{140, 143, 66,  70,  8 * 2,   5},     /* D3R */
	{16,  29, 64,  75,  8 * 2,  12},     /* D3L */
	{106, 119, 64,  75,  8 * 2,  12},     /* D3C */
	{187, 200, 64,  75,  8 * 2,  12},     /* D3R */
	{67,  77, 74,  83,  8 * 2,  10},     /* D2L */
	{146, 156, 74,  83,  8 * 2,  10},     /* D2R */
	{0,  17, 73,  90, 16 * 2,  18},     /* D2L */
	{100, 121, 73,  90, 16 * 2,  18},     /* D2C */
	{206, 223, 73,  90, 16 * 2,  18},     /* D2R */
	{48,  63, 84, 102,  8 * 2,  19},     /* D1L */
	{160, 175, 84, 102,  8 * 2,  19},     /* D1R */
	{96, 127, 92, 119, 16 * 2,  28}},   /* D1C */
	{{80,  83, 49,  53,  8 * 2,   5},     /* D3L */
	{140, 143, 49,  53,  8 * 2,   5},     /* D3R */
	{16,  29, 50,  61,  8 * 2,  12},     /* D3L */
	{106, 119, 50,  61,  8 * 2,  12},     /* D3C */
	{187, 200, 50,  61,  8 * 2,  12},     /* D3R */
	{67,  77, 53,  62,  8 * 2,  10},     /* D2L */
	{146, 156, 53,  62,  8 * 2,  10},     /* D2R */
	{0,  17, 55,  72, 16 * 2,  18},     /* D2L */
	{100, 121, 55,  72, 16 * 2,  18},     /* D2C */
	{206, 223, 55,  72, 16 * 2,  18},     /* D2R */
	{48,  63, 57,  75,  8 * 2,  19},     /* D1L */
	{160, 175, 57,  75,  8 * 2,  19},     /* D1R */
	{96, 127, 64,  91, 16 * 2,  28}},   /* D1C */
	{{75,  90, 40,  44,  8 * 2,   5},     /* D3L */
	{133, 148, 40,  44,  8 * 2,   5},     /* D3R */
	{1,  48, 44,  49, 24 * 2,   6},     /* D3L */
	{88, 135, 44,  49, 24 * 2,   6},     /* D3C */
	{171, 218, 44,  49, 24 * 2,   6},     /* D3R */
	{60,  77, 40,  46, 16 * 2,   7},     /* D2L */
	{146, 163, 40,  46, 16 * 2,   7},     /* D2R */
	{0,  35, 43,  50, 32 * 2,   8},     /* D2L */
	{80, 143, 43,  50, 32 * 2,   8},     /* D2C */
	{184, 223, 43,  50, 32 * 2,   8},     /* D2R */
	{32,  63, 41,  52, 16 * 2,  12},     /* D1L */
	{160, 191, 41,  52, 16 * 2,  12},     /* D1R */
	{64, 159, 41,  52, 48 * 2,  12}},   /* D1C */
	{{78,  85, 36,  51,  8 * 2,  16},     /* D3L */
	{138, 145, 36,  51,  8 * 2,  16},     /* D3R */
	{10,  41, 34,  53, 16 * 2,  20},     /* D3L */
	{98, 129, 34,  53, 16 * 2,  20},     /* D3C */
	{179, 210, 34,  53, 16 * 2,  20},     /* D3R */
	{66,  75, 34,  56,  8 * 2,  23},     /* D2L */
	{148, 157, 34,  56,  8 * 2,  23},     /* D2R */
	{0,  26, 33,  61, 24 * 2,  29},     /* D2L */
	{91, 133, 33,  61, 24 * 2,  29},     /* D2C */
	{194, 223, 33,  61, 24 * 2,  29},     /* D2R */
	{41,  56, 31,  65,  8 * 2,  35},     /* D1L */
	{167, 182, 31,  65,  8 * 2,  35},     /* D1R */
	{80, 143, 29,  71, 32 * 2,  43}},   /* D1C */
	{{75,  82, 25,  75,  8 * 2,  51},     /* D3L */
	{142, 149, 25,  75,  8 * 2,  51},     /* D3R */
	{12,  60, 25,  75, 32 * 2,  51},     /* D3L */
	{88, 136, 25,  75, 32 * 2,  51},     /* D3C */
	{163, 211, 25,  75, 32 * 2,  51},     /* D3R */
	{64,  73, 20,  90,  8 * 2,  71},     /* D2L */
	{150, 159, 20,  90,  8 * 2,  71},     /* D2R */
	{0,  38, 20,  90, 32 * 2,  71},     /* D2L */
	{82, 142, 20,  90, 32 * 2,  71},     /* D2C */
	{184, 223, 20,  90, 32 * 2,  71},     /* D2R */
	{41,  56,  9, 119,  8 * 2, 111},     /* D1L */
	{169, 184,  9, 119,  8 * 2, 111},     /* D1R */
	{64, 159,  9, 119, 48 * 2, 111}},   /* D1C */
	{{74,  85, 25,  75,  8 * 2,  51},     /* D3L */
	{137, 149, 25,  75,  8 * 2,  51},     /* D3R */
	{0,  75, 25,  75, 40 * 2,  51},     /* D3L Atari ST: {   0,  83, 25,  75, 48*2,  51 } */
	{74, 149, 25,  75, 40 * 2,  51},     /* D3C Atari ST: {  74, 149, 25,  75, 48*2,  51 } */
	{148, 223, 25,  75, 40 * 2,  51},     /* D3R Atari ST: { 139, 223, 25,  75, 48*2,  51 } */
	{60,  77, 20,  90, 16 * 2,  71},     /* D2L */
	{146, 163, 20,  90, 16 * 2,  71},     /* D2R */
	{0,  74, 20,  90, 56 * 2,  71},     /* D2L */
	{60, 163, 20,  90, 56 * 2,  71},     /* D2C */
	{149, 223, 20,  90, 56 * 2,  71},     /* D2R */
	{32,  63,  9, 119, 16 * 2, 111},     /* D1L */
	{160, 191,  9, 119, 16 * 2, 111},     /* D1R */
	{32, 191,  9, 119, 80 * 2, 111}}}; /* D1C */

byte gWallOrnCoordSetIndices[60] = { // @ G0194_auc_Graphic558_WallOrnamentCoordinateSetIndices
	1,   /* Wall Ornament 00 Unreadable Inscription */
	1,   /* Wall Ornament 01 Square Alcove */
	1,   /* Wall Ornament 02 Vi Altar */
	1,   /* Wall Ornament 03 Arched Alcove */
	0,   /* Wall Ornament 04 Hook */
	0,   /* Wall Ornament 05 Iron Lock */
	0,   /* Wall Ornament 06 Wood Ring */
	0,   /* Wall Ornament 07 Small Switch */
	0,   /* Wall Ornament 08 Dent 1 */
	0,   /* Wall Ornament 09 Dent 2 */
	0,   /* Wall Ornament 10 Iron Ring */
	2,   /* Wall Ornament 11 Crack */
	3,   /* Wall Ornament 12 Slime Outlet */
	0,   /* Wall Ornament 13 Dent 3 */
	0,   /* Wall Ornament 14 Tiny Switch */
	0,   /* Wall Ornament 15 Green Switch Out */
	0,   /* Wall Ornament 16 Blue Switch Out */
	0,   /* Wall Ornament 17 Coin Slot */
	0,   /* Wall Ornament 18 Double Iron Lock */
	0,   /* Wall Ornament 19 Square Lock */
	0,   /* Wall Ornament 20 Winged Lock */
	0,   /* Wall Ornament 21 Onyx Lock */
	0,   /* Wall Ornament 22 Stone Lock */
	0,   /* Wall Ornament 23 Cross Lock */
	0,   /* Wall Ornament 24 Topaz Lock */
	0,   /* Wall Ornament 25 Skeleton Lock */
	0,   /* Wall Ornament 26 Gold Lock */
	0,   /* Wall Ornament 27 Tourquoise Lock */
	0,   /* Wall Ornament 28 Emerald Lock */
	0,   /* Wall Ornament 29 Ruby Lock */
	0,   /* Wall Ornament 30 Ra Lock */
	0,   /* Wall Ornament 31 Master Lock */
	0,   /* Wall Ornament 32 Gem Hole */
	2,   /* Wall Ornament 33 Slime */
	2,   /* Wall Ornament 34 Grate */
	1,   /* Wall Ornament 35 Fountain */
	1,   /* Wall Ornament 36 Manacles */
	1,   /* Wall Ornament 37 Ghoul's Head */
	1,   /* Wall Ornament 38 Empty Torch Holder */
	1,   /* Wall Ornament 39 Scratches */
	4,   /* Wall Ornament 40 Poison Holes */
	4,   /* Wall Ornament 41 Fireball Holes */
	4,   /* Wall Ornament 42 Dagger Holes */
	5,   /* Wall Ornament 43 Champion Mirror */
	0,   /* Wall Ornament 44 Lever Up */
	0,   /* Wall Ornament 45 Lever Down */
	1,   /* Wall Ornament 46 Full Torch Holder */
	0,   /* Wall Ornament 47 Red Switch Out */
	0,   /* Wall Ornament 48 Eye Switch */
	0,   /* Wall Ornament 49 Big Switch Out */
	2,   /* Wall Ornament 50 Crack Switch Out */
	0,   /* Wall Ornament 51 Green Switch In */
	0,   /* Wall Ornament 52 Blue Switch In */
	0,   /* Wall Ornament 53 Red Switch In */
	0,   /* Wall Ornament 54 Big Switch In */
	2,   /* Wall Ornament 55 Crack Switch In. Atari ST Version 1.0 1987-12-08: 0  */
	6,   /* Wall Ornament 56 Amalgam (Encased Gem) */
	6,   /* Wall Ornament 57 Amalgam (Free Gem) */
	6,   /* Wall Ornament 58 Amalgam (Without Gem) */
	7}; /* Wall Ornament 59 Lord Order (Outside) */

CreatureAspect gCreatureAspects[kCreatureTypeCount] = { // @ G0219_as_Graphic558_CreatureAspects
/* { FirstNativeBitmapRelativeIndex, FirstDerivedBitmapIndex, pixelWidthFront, HeightFront,
pixelWidthSide, HeightSide, pixelWidthAttack, HeightAttack, CoordinateSet / TransparentColor,
Replacement Color Set Index for color 10 / Replacement Color Set Index for color 9 } */
	{0, 0, 56 * 2,  84, 56 * 2,  84, 56 * 2,  84, 0x1D, 0x01},   /* Creature #00 Giant Scorpion / Scorpion */
	{4, 0, 32 * 2,  66,  0 * 2,   0, 32 * 2,  69, 0x0B, 0x20},   /* Creature #01 Swamp Slime / Slime Devil */
	{6, 0, 24 * 2,  48, 24 * 2,  48,  0 * 2,   0, 0x0B, 0x00},   /* Creature #02 Giggler */
	{10, 0, 32 * 2,  61,  0 * 2,   0, 32 * 2,  61, 0x24, 0x31},   /* Creature #03 Wizard Eye / Flying Eye */
	{12, 0, 32 * 2,  64, 56 * 2,  64, 32 * 2,  64, 0x14, 0x34},   /* Creature #04 Pain Rat / Hellhound */
	{16, 0, 24 * 2,  49, 40 * 2,  49,  0 * 2,   0, 0x18, 0x34},   /* Creature #05 Ruster */
	{19, 0, 32 * 2,  60,  0 * 2,   0, 32 * 2,  60, 0x0D, 0x00},   /* Creature #06 Screamer */
	{21, 0, 32 * 2,  43,  0 * 2,   0, 32 * 2,  64, 0x04, 0x00},   /* Creature #07 Rockpile / Rock pile */
	{23, 0, 32 * 2,  83,  0 * 2,   0, 32 * 2,  93, 0x04, 0x00},   /* Creature #08 Ghost / Rive */
	{25, 0, 32 * 2, 101, 32 * 2, 101, 32 * 2, 101, 0x14, 0x00},   /* Creature #09 Stone Golem */
	{29, 0, 32 * 2,  82, 32 * 2,  82, 32 * 2,  83, 0x04, 0x00},   /* Creature #10 Mummy */
	{33, 0, 32 * 2,  80,  0 * 2,   0, 32 * 2,  99, 0x14, 0x00},   /* Creature #11 Black Flame */
	{35, 0, 32 * 2,  80, 32 * 2,  80, 32 * 2,  76, 0x04, 0x00},   /* Creature #12 Skeleton */
	{39, 0, 32 * 2,  96, 56 * 2,  93, 32 * 2,  90, 0x1D, 0x20},   /* Creature #13 Couatl */
	{43, 0, 32 * 2,  49, 16 * 2,  49, 32 * 2,  56, 0x04, 0x30},   /* Creature #14 Vexirk */
	{47, 0, 32 * 2,  59, 56 * 2,  43, 32 * 2,  67, 0x14, 0x78},   /* Creature #15 Magenta Worm / Worm */
	{51, 0, 32 * 2,  83, 32 * 2,  74, 32 * 2,  74, 0x04, 0x65},   /* Creature #16 Trolin / Ant Man */
	{55, 0, 24 * 2,  49, 24 * 2,  53, 24 * 2,  53, 0x24, 0x00},   /* Creature #17 Giant Wasp / Muncher */
	{59, 0, 32 * 2,  89, 32 * 2,  89, 32 * 2,  89, 0x04, 0x00},   /* Creature #18 Animated Armour / Deth Knight */
	{63, 0, 32 * 2,  84, 32 * 2,  84, 32 * 2,  84, 0x0D, 0xA9},   /* Creature #19 Materializer / Zytaz */
	{67, 0, 56 * 2,  27,  0 * 2,   0, 56 * 2,  80, 0x04, 0x65},   /* Creature #20 Water Elemental */
	{69, 0, 56 * 2,  77, 56 * 2,  81, 56 * 2,  77, 0x04, 0xA9},   /* Creature #21 Oitu */
	{73, 0, 32 * 2,  87, 32 * 2,  89, 32 * 2,  89, 0x04, 0xCB},   /* Creature #22 Demon */
	{77, 0, 32 * 2,  96, 32 * 2,  94, 32 * 2,  96, 0x04, 0x00},   /* Creature #23 Lord Chaos */
	{81, 0, 64 * 2,  94, 72 * 2,  94, 64 * 2,  94, 0x04, 0xCB},   /* Creature #24 Red Dragon / Dragon */
	{85, 0, 32 * 2,  93,  0 * 2,   0,  0 * 2,   0, 0x04, 0xCB},   /* Creature #25 Lord Order */
	{86, 0, 32 * 2,  93,  0 * 2,   0,  0 * 2,   0, 0x04, 0xCB}}; /* Creature #26 Grey Lord */

ObjectAspect gObjectAspects[kObjAspectCount] = { // @ G0209_as_Graphic558_ObjectAspects
	/* FirstNativeBitmapRelativeIndex, FirstDerivedBitmapRelativeIndex, ByteWidth, Height, GraphicInfo, CoordinateSet */
	ObjectAspect(0,   0, 24, 27, 0x11, 0),
	ObjectAspect(2,   6, 24,  8, 0x00, 1),
	ObjectAspect(3,   8,  8, 18, 0x00, 1),
	ObjectAspect(4,  10,  8,  8, 0x00, 1),
	ObjectAspect(5,  12,  8,  4, 0x00, 1),
	ObjectAspect(6,  14, 16, 11, 0x00, 1),
	ObjectAspect(7,  16, 24, 13, 0x00, 0),
	ObjectAspect(8,  18, 32, 16, 0x00, 0),
	ObjectAspect(9,  20, 40, 24, 0x00, 0),
	ObjectAspect(10,  22, 16, 20, 0x00, 1),
	ObjectAspect(11,  24, 40, 20, 0x00, 0),
	ObjectAspect(12,  26, 32,  4, 0x00, 1),
	ObjectAspect(13,  28, 40,  8, 0x00, 1),
	ObjectAspect(14,  30, 32, 17, 0x00, 0),
	ObjectAspect(15,  32, 40, 17, 0x00, 2),
	ObjectAspect(16,  34, 16,  9, 0x00, 1),
	ObjectAspect(17,  36, 24,  5, 0x00, 1),
	ObjectAspect(18,  38, 16,  9, 0x00, 0),
	ObjectAspect(19,  40,  8,  4, 0x00, 1),
	ObjectAspect(20,  42, 32, 21, 0x00, 2),
	ObjectAspect(21,  44, 32, 25, 0x00, 2),
	ObjectAspect(22,  46, 32, 14, 0x00, 1),
	ObjectAspect(23,  48, 32, 26, 0x00, 2),
	ObjectAspect(24,  50, 32, 16, 0x00, 0),
	ObjectAspect(25,  52, 32, 16, 0x00, 0),
	ObjectAspect(26,  54, 16, 16, 0x00, 1),
	ObjectAspect(27,  56, 16, 15, 0x00, 1),
	ObjectAspect(28,  58, 16, 13, 0x00, 1),
	ObjectAspect(29,  60, 16, 10, 0x00, 1),
	ObjectAspect(30,  62, 40, 24, 0x00, 0),
	ObjectAspect(31,  64, 40,  9, 0x00, 1),
	ObjectAspect(32,  66, 16,  3, 0x00, 1),
	ObjectAspect(33,  68, 32,  5, 0x00, 1),
	ObjectAspect(34,  70, 40, 16, 0x00, 0),
	ObjectAspect(35,  72,  8,  7, 0x00, 1),
	ObjectAspect(36,  74, 32,  7, 0x00, 1),
	ObjectAspect(37,  76, 24, 14, 0x00, 0),
	ObjectAspect(38,  78, 16,  8, 0x00, 0),
	ObjectAspect(39,  80,  8,  3, 0x00, 1),
	ObjectAspect(40,  82, 40,  9, 0x00, 1),
	ObjectAspect(41,  84, 24, 14, 0x00, 0),
	ObjectAspect(42,  86, 40, 20, 0x00, 0),
	ObjectAspect(43,  88, 40, 15, 0x00, 1),
	ObjectAspect(44,  90, 32, 10, 0x00, 1),
	ObjectAspect(45,  92, 32, 19, 0x00, 0),
	ObjectAspect(46,  94, 40, 25, 0x00, 2),
	ObjectAspect(47,  96, 24,  7, 0x00, 1),
	ObjectAspect(48,  98,  8,  7, 0x00, 1),
	ObjectAspect(49, 100, 16,  5, 0x00, 1),
	ObjectAspect(50, 102,  8,  9, 0x00, 1),
	ObjectAspect(51, 104, 32, 11, 0x00, 1),
	ObjectAspect(52, 106, 32, 14, 0x00, 0),
	ObjectAspect(53, 108, 24, 20, 0x00, 0),
	ObjectAspect(54, 110, 16, 14, 0x00, 1),
	ObjectAspect(55, 112, 32, 23, 0x00, 0),
	ObjectAspect(56, 114, 24, 16, 0x00, 0),
	ObjectAspect(57, 116, 32, 25, 0x00, 0),
	ObjectAspect(58, 118, 24, 25, 0x00, 0),
	ObjectAspect(59, 120,  8,  8, 0x00, 1),
	ObjectAspect(60, 122,  8,  7, 0x00, 1),
	ObjectAspect(61, 124,  8,  8, 0x00, 1),
	ObjectAspect(62, 126,  8,  8, 0x00, 1),
	ObjectAspect(63, 128,  8,  5, 0x00, 1),
	ObjectAspect(64, 130,  8, 13, 0x01, 1),
	ObjectAspect(65, 134, 16, 13, 0x00, 1),
	ObjectAspect(66, 136, 16, 14, 0x00, 0),
	ObjectAspect(67, 138, 16, 10, 0x00, 1),
	ObjectAspect(68, 140,  8, 18, 0x00, 1),
	ObjectAspect(69, 142,  8, 17, 0x00, 1),
	ObjectAspect(70, 144, 32, 18, 0x00, 0),
	ObjectAspect(71, 146, 16, 23, 0x00, 0),
	ObjectAspect(72, 148, 16, 24, 0x00, 0),
	ObjectAspect(73, 150, 16, 15, 0x00, 0),
	ObjectAspect(74, 152,  8,  7, 0x00, 1),
	ObjectAspect(75, 154,  8, 15, 0x00, 1),
	ObjectAspect(76, 156,  8,  9, 0x00, 1),
	ObjectAspect(77, 158, 16, 14, 0x00, 0),
	ObjectAspect(78, 160,  8,  8, 0x00, 1),
	ObjectAspect(79, 162, 16,  9, 0x00, 1),
	ObjectAspect(80, 164,  8, 13, 0x01, 1),
	ObjectAspect(81, 168,  8, 18, 0x00, 1),
	ObjectAspect(82, 170, 24, 28, 0x00, 0),
	ObjectAspect(83, 172, 40, 13, 0x00, 1),
	ObjectAspect(84, 174,  8,  4, 0x00, 1),
	ObjectAspect(85, 176, 32, 17, 0x00, 0)
};

ProjectileAspect gProjectileAspect[kProjectileAspectCount] = { // @ G0210_as_Graphic558_ProjectileAspects
	/* ProjectileAspect( FirstNativeBitmapRelativeIndex, FirstDerivedBitmapRelativeIndex, ByteWidth, Height, GraphicInfo ) */
	ProjectileAspect(0,   0, 32, 11, 0x0011),   /* Arrow */
	ProjectileAspect(3,  18, 16, 11, 0x0011),   /* Dagger */
	ProjectileAspect(6,  36, 24, 47, 0x0010),   /* Axe - Executioner */
	ProjectileAspect(9,  54, 32, 15, 0x0112),   /* Explosion Lightning Bolt */
	ProjectileAspect(11,  54, 32, 12, 0x0011),   /* Slayer */
	ProjectileAspect(14,  72, 24, 47, 0x0010),   /* Stone Club */
	ProjectileAspect(17,  90, 24, 47, 0x0010),   /* Club */
	ProjectileAspect(20, 108, 16, 11, 0x0011),   /* Poison Dart */
	ProjectileAspect(23, 126, 48, 18, 0x0011),   /* Storm - Side Splitter - Diamond Edge - Falchion - Ra Blade - Rapier - Biter - Samurai Sword - Sword - Dragon Fang */
	ProjectileAspect(26, 144,  8, 15, 0x0012),   /* Throwing Star */
	ProjectileAspect(28, 156, 16, 28, 0x0103),   /* Explosion Fireball */
	ProjectileAspect(29, 156, 16, 11, 0x0103),   /* Explosion Default */
	ProjectileAspect(30, 156, 16, 28, 0x0103),   /* Explosion Slime */
	ProjectileAspect(31, 156, 16, 24, 0x0103) /* Explosion Poison Bolt Poison Cloud */
};

// TODO: this is ONLY for the Amiga version, name will have to be refactored

/* Identical to the palette at the end of the swoosh palette animation */
uint16 gPalSwoosh[16] = {0x000, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0x000, 0xFFF, 0xAAA, 0xFFF, 0xAAA, 0x444, 0xFF0, 0xFF0}; // @ K0057_aui_Palette_Swoosh
uint16 gPalMousePointer[16] = {0x000, 0x666, 0x888, 0x620, 0x0CC, 0x840, 0x080, 0x0C0, 0xF00, 0xFA0, 0xC86, 0xFF0, 0x000, 0xAAA, 0x00F, 0xFFF}; // @ K0150_aui_Palette_MousePointer
/* Atari ST: { 0x003, 0x055, 0x773, 0x420, 0x774, 0x000, 0x040, 0x500, 0x642, 0x775, 0x742, 0x760, 0x750, 0x000, 0x310, 0x776 }, RGB colors are different */
uint16 gPalCredits[16] = {0x006, 0x0AA, 0xFF6, 0x840, 0xFF8, 0x000, 0x080, 0xA00, 0xC84, 0xFFA, 0xF84, 0xFC0, 0xFA0, 0x000, 0x620, 0xFFC}; // @ G0019_aui_Graphic562_Palette_Credits
/* Atari ST: { 0x000, 0x333, 0x444, 0x420, 0x654, 0x210, 0x040, 0x050, 0x432, 0x700, 0x543, 0x321, 0x222, 0x555, 0x310, 0x777 }, RGB colors are different */
uint16 gPalEntrance[16] = {0x000, 0x666, 0x888, 0x840, 0xCA8, 0x0C0, 0x080, 0x0A0, 0x864, 0xF00, 0xA86, 0x642, 0x444, 0xAAA, 0x620, 0xFFF}; // @ G0020_aui_Graphic562_Palette_Entrance
uint16 gPalDungeonView[6][16] = { // @ G0021_aaui_Graphic562_Palette_DungeonView
	/* Atari ST: { 0x000, 0x333, 0x444, 0x310, 0x066, 0x420, 0x040, 0x060, 0x700, 0x750, 0x643, 0x770, 0x222, 0x555, 0x007, 0x777 }, RGB colors are different */
	0x000, 0x666, 0x888, 0x620, 0x0CC, 0x840, 0x080, 0x0C0, 0xF00, 0xFA0, 0xC86, 0xFF0, 0x444, 0xAAA, 0x00F, 0xFFF,
	/* Atari ST: { 0x000, 0x222, 0x333, 0x310, 0x066, 0x410, 0x030, 0x050, 0x600, 0x640, 0x532, 0x760, 0x111, 0x444, 0x006, 0x666 }, RGB colors are different */
	0x000, 0x444, 0x666, 0x620, 0x0CC, 0x820, 0x060, 0x0A0, 0xC00, 0x000, 0x000, 0xFC0, 0x222, 0x888, 0x00C, 0xCCC,
	/* Atari ST: { 0x000, 0x111, 0x222, 0x210, 0x066, 0x310, 0x020, 0x040, 0x500, 0x530, 0x421, 0x750, 0x000, 0x333, 0x005, 0x555 }, RGB colors are different */
	0x000, 0x222, 0x444, 0x420, 0x0CC, 0x620, 0x040, 0x080, 0xA00, 0x000, 0x000, 0xFA0, 0x000, 0x666, 0x00A, 0xAAA,
	/* Atari ST: { 0x000, 0x000, 0x111, 0x100, 0x066, 0x210, 0x010, 0x030, 0x400, 0x420, 0x310, 0x640, 0x000, 0x222, 0x004, 0x444 }, RGB colors are different */
	0x000, 0x000, 0x222, 0x200, 0x0CC, 0x420, 0x020, 0x060, 0x800, 0x000, 0x000, 0xC80, 0x000, 0x444, 0x008, 0x888,
	/* Atari ST: { 0x000, 0x000, 0x000, 0x000, 0x066, 0x100, 0x000, 0x020, 0x300, 0x310, 0x200, 0x530, 0x000, 0x111, 0x003, 0x333 }, RGB colors are different */
	0x000, 0x000, 0x000, 0x000, 0x0CC, 0x200, 0x000, 0x040, 0x600, 0x000, 0x000, 0xA60, 0x000, 0x222, 0x006, 0x666,
	/* Atari ST: { 0x000, 0x000, 0x000, 0x000, 0x066, 0x000, 0x000, 0x010, 0x200, 0x200, 0x100, 0x320, 0x000, 0x000, 0x002, 0x222 }, RGB colors are different */
	0x000, 0x000, 0x000, 0x000, 0x0CC, 0x000, 0x000, 0x020, 0x400, 0x000, 0x000, 0x640, 0x000, 0x000, 0x004, 0x444
};

CreatureReplColorSet gCreatureReplColorSets[13] = { // @ G0220_as_Graphic558_CreatureReplacementColorSets
	/* { Color, Color, Color, Color, Color, Color, D2 replacement color index (x10), D3 replacement color index (x10) } */
	{0x0CA0, 0x0A80, 0x0860, 0x0640, 0x0420, 0x0200,  90,  90},    /* Atari ST: { 0x0650, 0x0540, 0x0430, 0x0320, 0x0210, 0x0100,  90,  90 }, RGB colors are different */
	{0x0060, 0x0040, 0x0020, 0x0000, 0x0000, 0x0000,   0,   0},    /* Atari ST: { 0x0030, 0x0020, 0x0010, 0x0000, 0x0000, 0x0000,   0,   0 }, */
	{0x0860, 0x0640, 0x0420, 0x0200, 0x0000, 0x0000, 100, 100},    /* Atari ST: { 0x0430, 0x0320, 0x0210, 0x0100, 0x0000, 0x0000, 100, 100 }, */
	{0x0640, 0x0420, 0x0200, 0x0000, 0x0000, 0x0000,  90,   0},    /* Atari ST: { 0x0320, 0x0210, 0x0100, 0x0000, 0x0000, 0x0000,  90,   0 }, */
	{0x000A, 0x0008, 0x0006, 0x0004, 0x0002, 0x0000,  90, 100},    /* Atari ST: { 0x0005, 0x0004, 0x0003, 0x0002, 0x0001, 0x0000,  90, 100 }, */
	{0x0008, 0x0006, 0x0004, 0x0002, 0x0000, 0x0000, 100,   0},    /* Atari ST: { 0x0004, 0x0003, 0x0002, 0x0001, 0x0000, 0x0000, 100,   0 }, */
	{0x0808, 0x0606, 0x0404, 0x0202, 0x0000, 0x0000,  90,   0},    /* Atari ST: { 0x0404, 0x0303, 0x0202, 0x0101, 0x0000, 0x0000,  90,   0 }, */
	{0x0A0A, 0x0808, 0x0606, 0x0404, 0x0202, 0x0000, 100,  90},    /* Atari ST: { 0x0505, 0x0404, 0x0303, 0x0202, 0x0101, 0x0000, 100,  90 }, */
	{0x0FA0, 0x0C80, 0x0A60, 0x0840, 0x0620, 0x0400, 100,  50},    /* Atari ST: { 0x0750, 0x0640, 0x0530, 0x0420, 0x0310, 0x0200, 100,  50 }, */
	{0x0F80, 0x0C60, 0x0A40, 0x0820, 0x0600, 0x0200,  50,  70},    /* Atari ST: { 0x0740, 0x0630, 0x0520, 0x0410, 0x0300, 0x0100,  50,  30 }, D3 replacement color index is different */
	{0x0800, 0x0600, 0x0400, 0x0200, 0x0000, 0x0000, 100, 120},    /* Atari ST: { 0x0400, 0x0300, 0x0200, 0x0100, 0x0000, 0x0000, 100, 100 }, D3 replacement color index is different */
	{0x0600, 0x0400, 0x0200, 0x0000, 0x0000, 0x0000, 120,   0},    /* Atari ST: { 0x0300, 0x0200, 0x0100, 0x0000, 0x0000, 0x0000, 120,   0 }, */
	{0x0C86, 0x0A64, 0x0842, 0x0620, 0x0400, 0x0200, 100,  50}};  /* Atari ST: { 0x0643, 0x0532, 0x0421, 0x0310, 0x0200, 0x0100, 100,  50 } }; */

byte gPalChangesCreature_D3[16] = {0, 120, 10, 30, 40, 30, 0, 60, 30, 0, 0, 110, 0, 20, 0, 130}; // @ G0221_auc_Graphic558_PaletteChanges_Creature_D3
byte gPalChangesCreature_D2[16] = {0, 10, 20, 30, 40, 30, 60, 70, 50, 0, 0, 110, 120, 130, 140, 150}; // @ G0222_auc_Graphic558_PaletteChanges_Creature_D2




Viewport gDefultViewPort(0, 0, 320, 200);
// TODO: I guessed the numbers
Viewport gDungeonViewport(0, 33, 224, 126); // @ G0296_puc_Bitmap_Viewport

byte gPalChangesNoChanges[16] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150}; // @ G0017_auc_Graphic562_PaletteChanges_NoChanges

byte gPalChangesFloorOrn_D3[16] = {0, 120, 10, 30, 40, 30, 0, 60, 30, 90, 100, 110, 0, 20, 140, 130}; // @ G0213_auc_Graphic558_PaletteChanges_FloorOrnament_D3
byte gPalChangesFloorOrn_D2[16] = {0, 10, 20, 30, 40, 30, 60, 70, 50, 90, 100, 110, 120, 130, 140, 150}; // @ G0214_auc_Graphic558_PaletteChanges_FloorOrnament_D2

int gFountainOrnIndices[kFountainOrnCount] = {35}; // @ G0193_ai_Graphic558_FountainOrnamentIndices
byte gAlcoveOrnIndices[kAlcoveOrnCount] = { // @ G0192_auc_Graphic558_AlcoveOrnamentIndices
	1,   /* Square Alcove */
	2,   /* Vi Altar */
	3};  /* Arched Alcove */


DisplayMan::DisplayMan(DMEngine *dmEngine) : _vm(dmEngine) {
	_vgaBuffer = nullptr;
	_bitmaps = nullptr;
	_grapItemCount = 0;
	_packedItemPos = nullptr;
	_packedBitmaps = nullptr;
	_bitmaps = nullptr;
	_tmpBitmap = nullptr;
	_floorBitmap = nullptr;
	_ceilingBitmap = nullptr;
	_currMapAllowedCreatureTypes = nullptr;
	_derivedBitmapByteCount = nullptr;
	_derivedBitmaps = nullptr;

	_screenWidth = _screenHeight = 0;
	_championPortraitOrdinal = 0;
	_currMapViAltarIndex = 0;

	for (int i = 0; i < 25; i++)
		_wallSetBitMaps[i] = nullptr;

	//for (int i = 0; i < kStairsGraphicCount; i++)
	//	_stairIndices[i] = 0;

	for (int i = 0; i < 4; i++)
		_palChangesProjectile[i] = nullptr;

	for (int i = 0; i < kAlcoveOrnCount; i++)
		_currMapAlcoveOrnIndices[i] = 0;

	for (int i = 0; i < kFountainOrnCount; i++)
		_currMapFountainOrnIndices[i] = 0;

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 16; j++) {
			_currMapWallOrnInfo[j][i] = 0;
			_currMapFloorOrnInfo[j][i] = 0;
		}
		for (int j = 0; j < 17; j++) {
			_currMapDoorOrnInfo[j][i] = 0;
		}
	}

	for (int i = 0; i < 16; i++) {
		_currMapWallOrnIndices[i] = 0;
		_currMapFloorOrnIndices[i] = 0;
	}

	for (int i = 0; i < 18; i++)
		_currMapDoorOrnIndices[i] = 0;

	_inscriptionThing = Thing::_none;
	_useByteBoxCoordinates = false;
}

DisplayMan::~DisplayMan() {
	delete[] _packedItemPos;
	delete[] _packedBitmaps;
	delete[] _vgaBuffer;
	if (_bitmaps) {
		delete[] _bitmaps[0];
		delete[] _bitmaps;
	}
	delete[] _wallSetBitMaps[kWall_D3R2]; // copy of another bitmap, but flipped
	delete[] _wallSetBitMaps[kDoorFrameRight_D1C]; // copy of another bitmap, but flipped
	for (uint16 i = kWall_D0L_Flipped; i <= kWall_D3LCR_Flipped; ++i)
		delete[] _wallSetBitMaps[i];

	delete[] _derivedBitmapByteCount;
	if (_derivedBitmaps) {
		for (uint16 i = 0; i < kDerivedBitmapMaximumCount; ++i)
			delete[] _derivedBitmaps;
		delete[] _derivedBitmaps;
	}
}

void DisplayMan::setUpScreens(uint16 width, uint16 height) {
	_screenWidth = width;
	_screenHeight = height;
	delete[] _tmpBitmap;
	_tmpBitmap = new byte[_screenWidth * _screenHeight];
	delete[] _vgaBuffer;
	_vgaBuffer = new byte[_screenWidth * _screenHeight];
	clearScreen(kColorBlack);
}

void DisplayMan::loadGraphics() {
	Common::File f;
	f.open("graphics.dat");

	_grapItemCount = f.readUint16BE();
	delete[] _packedItemPos;
	_packedItemPos = new uint32[_grapItemCount + 1];
	_packedItemPos[0] = 0;
	for (uint16 i = 1; i < _grapItemCount + 1; ++i)
		_packedItemPos[i] = f.readUint16BE() + _packedItemPos[i - 1];

	delete[] _packedBitmaps;
	_packedBitmaps = new uint8[_packedItemPos[_grapItemCount]];

	f.seek(2 + _grapItemCount * 4);
	for (uint32 i = 0; i < _packedItemPos[_grapItemCount]; ++i)
		_packedBitmaps[i] = f.readByte();

	f.close();

	unpackGraphics();

	loadFloorSet(kFloorSetStone);
	loadWallSet(kWallSetStone);



	if (!_derivedBitmapByteCount)
		_derivedBitmapByteCount = new uint16[kDerivedBitmapMaximumCount];
	if (!_derivedBitmaps) {
		_derivedBitmaps = new byte*[kDerivedBitmapMaximumCount];
		for (uint16 i = 0; i < kDerivedBitmapMaximumCount; ++i)
			_derivedBitmaps[i] = nullptr;
	}

	_derivedBitmapByteCount[kDerivedBitmapViewport] = 224 * 136;
	_derivedBitmapByteCount[kDerivedBitmapThievesEyeVisibleArea] = 96 * 95;
	_derivedBitmapByteCount[kDerivedBitmapDamageToCreatureMedium] = 64 * 37;
	_derivedBitmapByteCount[kDerivedBitmapDamageToCreatureSmall] = 48 * 37;

	for (int16 doorOrnamentIndex = kDoorOrnDestroyedMask; doorOrnamentIndex <= kDoorOrnThivesEyeMask; doorOrnamentIndex++) {
		_currMapDoorOrnInfo[doorOrnamentIndex][kNativeBitmapIndex] = doorOrnamentIndex + (kDoorMaskDestroyedIndice - kDoorOrnDestroyedMask);
		_currMapDoorOrnInfo[doorOrnamentIndex][kCoordinateSet] = 1;

		_derivedBitmapByteCount[doorOrnamentIndex * 2 + kDerivedBitmapFirstDoorOrnament_D3] = 48 * 41;
		_derivedBitmapByteCount[doorOrnamentIndex * 2 + kDerivedBitmapFirstDoorOrnament_D2] = 64 * 61;
	}

	_currMapFloorOrnInfo[kFloorOrnFootprints][kNativeBitmapIndex] = kFloorOrn_15_D3L_footprints;
	_currMapFloorOrnInfo[kFloorOrnFootprints][kCoordinateSet] = 1;

	ObjectAspect *objectAspect = gObjectAspects;
	int16 derivedBitmapIndex;
	for (int16 objectAspectIndex = 0; objectAspectIndex < kObjAspectCount; ++objectAspectIndex, ++objectAspect) {
		derivedBitmapIndex = kDerivedBitmapFirstObject + objectAspect->_firstDerivedBitmapRelativeIndex;

		_derivedBitmapByteCount[derivedBitmapIndex++] = getScaledBitmapPixelCount(objectAspect->_width, objectAspect->_height, kScale16_D3);
		_derivedBitmapByteCount[derivedBitmapIndex++] = getScaledBitmapPixelCount(objectAspect->_width, objectAspect->_height, kScale20_D2);

		if (getFlag(objectAspect->_graphicInfo, kObjectFlipOnRightMask)) {
			_derivedBitmapByteCount[derivedBitmapIndex] = _derivedBitmapByteCount[derivedBitmapIndex - 2];
			derivedBitmapIndex++;
			_derivedBitmapByteCount[derivedBitmapIndex] = _derivedBitmapByteCount[derivedBitmapIndex - 2];
			derivedBitmapIndex++;
		}

		if (getFlag(objectAspect->_graphicInfo, kObjectAlcoveMask)) {
			_derivedBitmapByteCount[derivedBitmapIndex] = _derivedBitmapByteCount[derivedBitmapIndex - 2];
			derivedBitmapIndex++;
			_derivedBitmapByteCount[derivedBitmapIndex] = _derivedBitmapByteCount[derivedBitmapIndex - 2];
		}
	}

	ProjectileAspect *projectileAspect = gProjectileAspect;
	for (int16 projectileAspectIndex = 0; projectileAspectIndex < kProjectileAspectCount; projectileAspectIndex++, projectileAspect++) {

		if (!getFlag(projectileAspect->_graphicInfo, kProjectileScaleWithKineticEnergyMask)) {
			derivedBitmapIndex = kDerivedBitmapFirstProjectile + projectileAspect->_firstDerivedBitmapRelativeIndex;

			for (int16 projectileScaleIndex = 0; projectileScaleIndex < 6; projectileScaleIndex++) {
				int16 bitmapPixelCount = getScaledBitmapPixelCount(projectileAspect->_width, projectileAspect->_height, gProjectileScales[projectileScaleIndex]);
				_derivedBitmapByteCount[derivedBitmapIndex] = bitmapPixelCount;

				if (getFlag(projectileAspect->_graphicInfo, kProjectileAspectTypeMask) != kProjectileAspectHasNone) {
					_derivedBitmapByteCount[derivedBitmapIndex + 6] = bitmapPixelCount;

					if (getFlag(projectileAspect->_graphicInfo, kProjectileAspectTypeMask) != kProjectileAspectHasRotation) {
						_derivedBitmapByteCount[derivedBitmapIndex + 12] = bitmapPixelCount;
					}
				}
			}
		}
	}

	_palChangesProjectile[0] = gPalChangesFloorOrn_D3;
	_palChangesProjectile[1] = gPalChangesFloorOrn_D2;
	_palChangesProjectile[2] = _palChangesProjectile[3] = gPalChangesNoChanges;

	derivedBitmapIndex = kDerivedBitmapFirstExplosion;
	ExplosionAspect *expAsp = gExplosionAspects;
	for (uint16 expAspIndex = 0; expAspIndex < kExplosionAspectCount; ++expAspIndex, expAsp++) {
		for (int16 scale = 4; scale < 32; scale += 2)
			_derivedBitmapByteCount[derivedBitmapIndex++] = getScaledBitmapPixelCount(expAsp->_pixelWidth, expAsp->_height, scale);

		if (expAspIndex == kExplosionAspectSmoke) {
			_derivedBitmapByteCount[derivedBitmapIndex++] = expAsp->_pixelWidth * expAsp->_height;
		}
	}

	derivedBitmapIndex = kDerivedBitmapFirstCreature;
	CreatureAspect *creatureAsp;
	for (int16 creatureIndex = 0; creatureIndex < kCreatureTypeCount; creatureIndex++) {
		creatureAsp = &gCreatureAspects[creatureIndex];

		int16 creatureGraphicInfo = gCreatureInfo[creatureIndex]._graphicInfo;
		creatureAsp->_firstDerivedBitmapIndex = derivedBitmapIndex;

		int16 creatureFrontBitmapD3PixelCount;
		_derivedBitmapByteCount[derivedBitmapIndex++] = creatureFrontBitmapD3PixelCount = getScaledBitmapPixelCount(creatureAsp->_byteWidthFront, creatureAsp->_heightFront, kScale16_D3);

		int16 creatureFrontBitmapD2PixelCount;
		_derivedBitmapByteCount[derivedBitmapIndex++] = creatureFrontBitmapD2PixelCount = getScaledBitmapPixelCount(creatureAsp->_byteWidthFront, creatureAsp->_heightFront, kScale20_D2);

		if (getFlag(creatureGraphicInfo, kCreatureInfoGraphicMaskSide)) {
			_derivedBitmapByteCount[derivedBitmapIndex++] = getScaledBitmapPixelCount(creatureAsp->_byteWidthSide, creatureAsp->_heightSide, kScale16_D3);
			_derivedBitmapByteCount[derivedBitmapIndex++] = getScaledBitmapPixelCount(creatureAsp->_byteWidthSide, creatureAsp->_heightSide, kScale20_D2);
		}

		if (getFlag(creatureGraphicInfo, kCreatureInfoGraphicMaskBack)) {
			_derivedBitmapByteCount[derivedBitmapIndex++] = creatureFrontBitmapD3PixelCount;
			_derivedBitmapByteCount[derivedBitmapIndex++] = creatureFrontBitmapD2PixelCount;
		}

		if (getFlag(creatureGraphicInfo, kCreatureInfoGraphicMaskAttack)) {
			_derivedBitmapByteCount[derivedBitmapIndex++] = getScaledBitmapPixelCount(creatureAsp->_byteWidthAttack, creatureAsp->_heightAttack, kScale16_D3);
			_derivedBitmapByteCount[derivedBitmapIndex++] = getScaledBitmapPixelCount(creatureAsp->_byteWidthAttack, creatureAsp->_heightAttack, kScale20_D2);
		}

		int16 additionalFronGraphicCount;
		if (additionalFronGraphicCount = getFlag(creatureGraphicInfo, kCreatureInfoGraphicMaskAdditional)) {
			do {
				_derivedBitmapByteCount[derivedBitmapIndex++] = creatureAsp->_byteWidthFront * creatureAsp->_heightFront;
				_derivedBitmapByteCount[derivedBitmapIndex++] = getScaledBitmapPixelCount(creatureAsp->_byteWidthFront, creatureAsp->_heightFront, kScale16_D3);
				_derivedBitmapByteCount[derivedBitmapIndex++] = getScaledBitmapPixelCount(creatureAsp->_byteWidthFront, creatureAsp->_heightFront, kScale20_D2);
			} while (--additionalFronGraphicCount);
		}
	}
}

void DisplayMan::unpackGraphics() {
	uint32 unpackedBitmapsSize = 0;
	for (uint16 i = 0; i <= 20; ++i)
		unpackedBitmapsSize += getWidth(i) * getHeight(i);
	for (uint16 i = 22; i <= 532; ++i)
		unpackedBitmapsSize += getWidth(i) * getHeight(i);
	unpackedBitmapsSize += (5 + 1) * 6 * 128; // 5 x 6 characters, 128 of them, +1 for convenience padding
	// graphics items go from 0-20 and 22-532 inclusive, _unpackedItemPos 21 and 22 are there for indexing convenience
	if (_bitmaps) {
		delete[] _bitmaps[0];
		delete[] _bitmaps;
	}
	_bitmaps = new byte*[575]; // largest graphic indice (i think)
	_bitmaps[0] = new byte[unpackedBitmapsSize];
	loadIntoBitmap(0, _bitmaps[0]);
	for (uint16 i = 1; i <= 20; ++i) {
		_bitmaps[i] = _bitmaps[i - 1] + getWidth(i - 1) * getHeight(i - 1);
		loadIntoBitmap(i, _bitmaps[i]);
	}
	_bitmaps[22] = _bitmaps[20] + getWidth(20) * getHeight(20);
	for (uint16 i = 23; i <= 532; ++i) {
		_bitmaps[i] = _bitmaps[i - 1] + getWidth(i - 1) * getHeight(i - 1);
		loadIntoBitmap(i, _bitmaps[i]);
	}
	_bitmaps[kFontGraphicIndice] = _bitmaps[532] + getWidth(532) * getHeight(532);
	loadFNT1intoBitmap(kFontGraphicIndice, _bitmaps[kFontGraphicIndice]);
}

void DisplayMan::loadFNT1intoBitmap(uint16 index, byte* destBitmap) {
	uint8 *data = _packedBitmaps + _packedItemPos[index];
	for (uint16 i = 0; i < 6; i++) {
		for (uint16 w = 0; w < 128; ++w) {
			*destBitmap++ = kColorBlack;

			uint16 nextByte = *data++;
			for (int16 pixel = 4; pixel >= 0; --pixel) {
				*destBitmap++ = (nextByte >> pixel) & 0x1;
			}
		}
	}
}

void DisplayMan::loadPalette(uint16 *palette) {
	byte colorPalette[16 * 3];
	for (int i = 0; i < 16; ++i) {
		colorPalette[i * 3] = (palette[i] >> 8) * (256 / 16);
		colorPalette[i * 3 + 1] = (palette[i] >> 4) * (256 / 16);
		colorPalette[i * 3 + 2] = palette[i] * (256 / 16);
	}
	_vm->_system->getPaletteManager()->setPalette(colorPalette, 0, 16);
}


void DisplayMan::loadIntoBitmap(uint16 index, byte *destBitmap) {
	uint8 *data = _packedBitmaps + _packedItemPos[index];
	uint16 width = READ_BE_UINT16(data);
	uint16 height = READ_BE_UINT16(data + 2);
	uint16 nextByteIndex = 4;
	for (uint16 k = 0; k < width * height;) {
		uint8 nextByte = data[nextByteIndex++];
		uint8 nibble1 = (nextByte & 0xF0) >> 4;
		uint8 nibble2 = (nextByte & 0x0F);
		if (nibble1 <= 7) {
			for (int j = 0; j < nibble1 + 1; ++j)
				destBitmap[k++] = nibble2;
		} else if (nibble1 == 0x8) {
			uint8 byte1 = data[nextByteIndex++];
			for (int j = 0; j < byte1 + 1; ++j)
				destBitmap[k++] = nibble2;
		} else if (nibble1 == 0xC) {
			uint16 word1 = READ_BE_UINT16(data + nextByteIndex);
			nextByteIndex += 2;
			for (int j = 0; j < word1 + 1; ++j)
				destBitmap[k++] = nibble2;
		} else if (nibble1 == 0xB) {
			uint8 byte1 = data[nextByteIndex++];
			for (int j = 0; j < byte1 + 1; ++j, ++k)
				destBitmap[k] = destBitmap[k - width];
			destBitmap[k++] = nibble2;
		} else if (nibble1 == 0xF) {
			uint16 word1 = READ_BE_UINT16(data + nextByteIndex);
			nextByteIndex += 2;
			for (int j = 0; j < word1 + 1; ++j, ++k)
				destBitmap[k] = destBitmap[k - width];
			destBitmap[k++] = nibble2;
		} else if (nibble1 == 9) {
			uint8 byte1 = data[nextByteIndex++];
			if (byte1 % 2)
				byte1++;
			else
				destBitmap[k++] = nibble2;

			for (int j = 0; j < byte1 / 2; ++j) {
				uint8 byte2 = data[nextByteIndex++];
				destBitmap[k++] = (byte2 & 0xF0) >> 4;
				destBitmap[k++] = byte2 & 0x0F;
			}
		}
	}
}

void DisplayMan::blitToBitmap(byte *srcBitmap, uint16 srcWidth, uint16 srcX, uint16 srcY,
							  byte *destBitmap, uint16 destWidth,
							  uint16 destFromX, uint16 destToX, uint16 destFromY, uint16 destToY,
							  Color transparent, Viewport &destViewport) {
	for (uint16 y = 0; y < destToY - destFromY; ++y)
		for (uint16 x = 0; x < destToX - destFromX; ++x) {
			byte srcPixel = srcBitmap[srcWidth * (y + srcY) + srcX + x];
			if (srcPixel != transparent)
				destBitmap[destWidth * (y + destFromY + destViewport._posY) + destFromX + x + destViewport._posX] = srcPixel;
		}
}

void DisplayMan::blitToBitmap(byte* srcBitmap, uint16 srcWidth, uint16 srcX, uint16 srcY, byte* destBitmap, uint16 destWidth, Box& box, Color transparent, Viewport& viewport) {
	blitToBitmap(srcBitmap, srcWidth, srcX, srcY, destBitmap, destWidth, box._x1, box._x2, box._y1, box._y2, transparent, viewport);
}

void DisplayMan::blitToScreen(byte *srcBitmap, uint16 srcWidth, uint16 srcX, uint16 srcY,
							  uint16 destFromX, uint16 destToX, uint16 destFromY, uint16 destToY,
							  Color transparent, Viewport &viewport) {
	blitToBitmap(srcBitmap, srcWidth, srcX, srcY,
				 getCurrentVgaBuffer(), _screenWidth, destFromX, destToX, destFromY, destToY, transparent, viewport);
}

void DisplayMan::blitToBitmap(byte *srcBitmap, uint16 srcWidth, uint16 srcHeight, byte *destBitmap, uint16 destWidth, uint16 destX, uint16 destY) {
	for (uint16 y = 0; y < srcHeight; ++y)
		memcpy(destBitmap + destWidth*(y + destY) + destX, srcBitmap + y * srcWidth, sizeof(byte)* srcWidth);
}

void DisplayMan::blitBoxFilledWithMaskedBitmapToScreen(byte* src, byte* mask, byte* tmp, Box& box,
													   int16 lastUnitIndex, int16 firstUnitIndex, int16 destPixelWidth, Color transparent,
													   int16 xPos, int16 yPos, int16 destHeight, int16 height2, Viewport& viewport) {
	blitBoxFilledWithMaskedBitmap(src, _vgaBuffer, mask, tmp, box, lastUnitIndex, firstUnitIndex, _screenWidth, transparent, xPos, yPos, _screenHeight, height2, viewport);
}

void DisplayMan::flipBitmapHorizontal(byte *bitmap, uint16 width, uint16 height) {
	for (uint16 y = 0; y < height; ++y) {
		for (uint16 x = 0; x < width / 2; ++x) {
			byte tmp;
			tmp = bitmap[y*width + x];
			bitmap[y*width + x] = bitmap[y*width + width - 1 - x];
			bitmap[y*width + width - 1 - x] = tmp;
		}
	}
}

void DisplayMan::flipBitmapVertical(byte *bitmap, uint16 width, uint16 height) {
	byte *tmp = new byte[width];

	for (uint16 y = 0; y < height / 2; ++y) {
		memcpy(tmp, bitmap + y * width, width);
		memcpy(bitmap + y * width, bitmap + (height - 1 - y) * width, width);
		memcpy(bitmap + (height - 1 - y) * width, tmp, width);
	}

	delete[] tmp;
}

byte* DisplayMan::getExplosionBitmap(uint16 explosionAspIndex, uint16 scale, int16& returnPixelWidth, int16& returnHeight) {
	ExplosionAspect *explAsp = &gExplosionAspects[explosionAspIndex];
	if (scale > 32)
		scale = 32;
	int16 pixelWidth = getScaledDimension(explAsp->_pixelWidth, scale);
	int16 height = getScaledDimension(explAsp->_height, scale);
	byte *bitmap;
	int16 derBitmapIndex = (explosionAspIndex * 14) + scale / 2 + kDerivedBitmapFirstExplosion - 2;
	if ((scale == 32) && (explosionAspIndex != kExplosionAspectSmoke)) {
		bitmap = getBitmap(explosionAspIndex + kFirstExplosionGraphicIndice);
	} else if (isDerivedBitmapInCache(derBitmapIndex)) {
		bitmap = getDerivedBitmap(derBitmapIndex);
	} else {
		byte *nativeBitmap = getBitmap(MIN(explosionAspIndex, (uint16)kExplosionAspectPoison) + kFirstExplosionGraphicIndice);
		bitmap = getDerivedBitmap(derBitmapIndex);
		blitToBitmapShrinkWithPalChange(nativeBitmap, explAsp->_pixelWidth, explAsp->_height, bitmap, pixelWidth, height,
			(explosionAspIndex == kExplosionAspectSmoke) ? gPalChangeSmoke : gPalChangesNoChanges);
		warning("IGNORED CODE: F0493_CACHE_AddDerivedBitmap");
	}

	returnPixelWidth = pixelWidth;
	returnHeight = height;
	return bitmap;
}


void DisplayMan::updateScreen() {
	_vm->_system->copyRectToScreen(_vgaBuffer, _screenWidth, 0, 0, _screenWidth, _screenHeight);
	_vm->_system->updateScreen();
}

byte *DisplayMan::getCurrentVgaBuffer() {
	return _vgaBuffer;
}

uint16 DisplayMan::getWidth(uint16 index) {
	byte *data = _packedBitmaps + _packedItemPos[index];
	return READ_BE_UINT16(data);
}

uint16 DisplayMan::getHeight(uint16 index) {
	uint8 *data = _packedBitmaps + _packedItemPos[index];
	return READ_BE_UINT16(data + 2);
}


void DisplayMan::drawWallSetBitmapWithoutTransparency(byte *bitmap, Frame &f) {
	if (f._srcWidth)
		blitToScreen(bitmap, f._srcWidth, f._srcX, f._srcY, f._box, kColorNoTransparency, gDungeonViewport);
}

void DisplayMan::drawWallSetBitmap(byte *bitmap, Frame &f) {
	if (f._srcWidth)
		blitToScreen(bitmap, f._srcWidth, f._srcX, f._srcY, f._box, kColorFlesh, gDungeonViewport);
}


// NOTE: has been screened for missing code
void DisplayMan::drawSquareD3L(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	int16 order;

	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[kElemAspect]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmap(_g0675stairsNativeBitmapIndex_Up_Front_D3L, gStairFrames[kFrameStairsUpFront_D3L]);
		else
			drawFloorPitOrStairsBitmap(_g0682stairsNativeBitmapIndex_Down_Front_D3L, gStairFrames[kFrameStairsDownFront_D3L]);
		goto T0116015_redEagle;
	case kWallElemType:
		drawWallSetBitmap(_wallSetBitMaps[kWall_D3LCR], gFrameWalls[kViewSquare_D3L]);
		isDrawnWallOrnAnAlcove(squareAspect[kRightWallOrnOrdAspect], kViewWall_D3L_RIGHT);
		if (isDrawnWallOrnAnAlcove(squareAspect[kFrontWallOrnOrdAspect], kViewWall_D3L_FRONT)) {
			order = kCellOrder_Alcove;
			goto T0116017_orangeElk;
		}
		return;
	case kElementTypeDoorSide:
	case kElementTypeStairsSide:
		order = kCellOrder_BackLeft_BackRight_FrontRight;
		goto T0116016_blueToad;
	case kElementTypeDoorFront:
		warning("MISSING CODE: F0108_DUNGEONVIEW_DrawFloorOrnament");
		cthulhu(Thing(squareAspect[kFirstGroupOrObjectAspect]), dir, posX, posY, kViewSquare_D3L, kCellOrder_DoorPass1_BackLeft_BackRight);
		drawWallSetBitmap(_wallSetBitMaps[kDoorFrameLeft_D3L], g0164Frame_DoorFrameLeft_D3L);
		warning("MISSING CODE: F0111_DUNGEONVIEW_DrawDoor");
		order = kCellOrder_DoorPass2_FrontLeft_FrontRight;
		goto T0116017_orangeElk;
	case kElementTypePit:
		if (!squareAspect[kPitInvisibleAspect]) {
			drawFloorPitOrStairsBitmap(k049FloorPit_D3L_GraphicIndice, g0140FrameFloorPit_D3L);
		}
	case kElementTypeTeleporter:
	case kElementTypeCorridor:
T0116015_redEagle:
		order = kCellOrder_BackLeft_BackRight_FrontLeft_FrontRight;
T0116016_blueToad:
		warning("MISSING CODE: F0108_DUNGEONVIEW_DrawFloorOrnament");
T0116017_orangeElk:
		cthulhu(Thing(squareAspect[kFirstGroupOrObjectAspect]), dir, posX, posY, kViewSquare_D3L, order);
	}
	if ((squareAspect[kElemAspect] == kElementTypeTeleporter) && squareAspect[kTeleporterVisibleAspect]) {
		drawField(&gFieldAspects[kViewSquare_D3L], gFrameWalls[kViewSquare_D3L]._box);
	}
}

void DisplayMan::drawSquareD3R(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmapFlippedHorizontally(_g0675stairsNativeBitmapIndex_Up_Front_D3L, gStairFrames[kFrameStairsUpFront_D3R]);
		else
			drawFloorPitOrStairsBitmapFlippedHorizontally(_g0682stairsNativeBitmapIndex_Down_Front_D3L, gStairFrames[kFrameStairsDownFront_D3R]);
		break;
	case kWallElemType:
		drawWallSetBitmap(_wallSetBitMaps[kWall_D3LCR], gFrameWalls[kViewSquare_D3R]);
		isDrawnWallOrnAnAlcove(squareAspect[kLeftWallOrnOrdAspect], kViewWall_D3R_LEFT);
		if (isDrawnWallOrnAnAlcove(squareAspect[kFrontWallOrnOrdAspect], kViewWall_D3R_FRONT)) {
			// ... missing code
		}
		break;
	default:
		break;
	}
}
void DisplayMan::drawSquareD3C(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmap(_g0676stairsNativeBitmapIndex_Up_Front_D3C, gStairFrames[kFrameStairsUpFront_D3C]);
		else
			drawFloorPitOrStairsBitmap(_g0683stairsNativeBitmapIndex_Down_Front_D3C, gStairFrames[kFrameStairsDownFront_D3C]);
		break;
	case kWallElemType:
		drawWallSetBitmapWithoutTransparency(_wallSetBitMaps[kWall_D3LCR], gFrameWalls[kViewSquare_D3C]);
		if (isDrawnWallOrnAnAlcove(squareAspect[kFrontWallOrnOrdAspect], kViewWall_D3C_FRONT)) {
			//... missing code
		}
		break;
	default:
		break;
	}
}
void DisplayMan::drawSquareD2L(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmap(_g0677stairsNativeBitmapIndex_Up_Front_D2L, gStairFrames[kFrameStairsUpFront_D2L]);
		else
			drawFloorPitOrStairsBitmap(_g0684stairsNativeBitmapIndex_Down_Front_D2L, gStairFrames[kFrameStairsDownFront_D2L]);
		break;
	case kWallElemType:
		drawWallSetBitmap(_wallSetBitMaps[kWall_D2LCR], gFrameWalls[kViewSquare_D2L]);
		isDrawnWallOrnAnAlcove(squareAspect[kRightWallOrnOrdAspect], kViewWall_D2L_RIGHT);
		if (isDrawnWallOrnAnAlcove(squareAspect[kFrontWallOrnOrdAspect], kViewWall_D2L_FRONT)) {
			// ... missing code
		}
		break;
	case kStairsSideElemType:
		drawFloorPitOrStairsBitmap(_g0689stairsNativeBitmapIndex_Side_D2L, gStairFrames[kFrameStairsSide_D2L]);
		break;
	default:
		break;
	}
}
void DisplayMan::drawSquareD2R(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmapFlippedHorizontally(_g0677stairsNativeBitmapIndex_Up_Front_D2L, gStairFrames[kFrameStairsUpFront_D2R]);
		else
			drawFloorPitOrStairsBitmapFlippedHorizontally(_g0684stairsNativeBitmapIndex_Down_Front_D2L, gStairFrames[kFrameStairsDownFront_D2R]);
		break;
	case kWallElemType:
		drawWallSetBitmap(_wallSetBitMaps[kWall_D2LCR], gFrameWalls[kViewSquare_D2R]);
		isDrawnWallOrnAnAlcove(squareAspect[kLeftWallOrnOrdAspect], kViewWall_D2R_LEFT);
		if (isDrawnWallOrnAnAlcove(squareAspect[kFrontWallOrnOrdAspect], kViewWall_D2R_FRONT)) {
			// ... missing code
		}
		break;
	case kStairsSideElemType:
		drawFloorPitOrStairsBitmapFlippedHorizontally(_g0689stairsNativeBitmapIndex_Side_D2L, gStairFrames[kFrameStairsSide_D2R]);
		break;
	default:
		break;
	}
}
void DisplayMan::drawSquareD2C(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmap(_g0678stairsNativeBitmapIndex_Up_Front_D2C, gStairFrames[kFrameStairsUpFront_D2C]);
		else
			drawFloorPitOrStairsBitmap(_g0685stairsNativeBitmapIndex_Down_Front_D2C, gStairFrames[kFrameStairsDownFront_D2C]);
		break;
	case kWallElemType:
		drawWallSetBitmapWithoutTransparency(_wallSetBitMaps[kWall_D2LCR], gFrameWalls[kViewSquare_D2C]);
		if (isDrawnWallOrnAnAlcove(squareAspect[kFrontWallOrnOrdAspect], kViewWall_D2C_FRONT)) {
			// ... missing code
		}
		break;
	default:
		break;
	}
}
void DisplayMan::drawSquareD1L(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmap(_g0679stairsNativeBitmapIndex_Up_Front_D1L, gStairFrames[kFrameStairsUpFront_D1L]);
		else
			drawFloorPitOrStairsBitmap(_g0686stairsNativeBitmapIndex_Down_Front_D1L, gStairFrames[kFrameStairsDownFront_D1L]);
		break;
	case kWallElemType:
		drawWallSetBitmap(_wallSetBitMaps[kWall_D1LCR], gFrameWalls[kViewSquare_D1L]);
		isDrawnWallOrnAnAlcove(squareAspect[kRightWallOrnOrdAspect], kViewWall_D1L_RIGHT);
		break;
	case kStairsSideElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmap(_g0690stairsNativeBitmapIndex_Up_Side_D1L, gStairFrames[kFrameStairsUpSide_D1L]);
		else
			drawFloorPitOrStairsBitmap(_g0691stairsNativeBitmapIndex_Down_Side_D1L, gStairFrames[kFrameStairsDownSide_D1L]);
		break;
	default:
		break;
	}
}
void DisplayMan::drawSquareD1R(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmapFlippedHorizontally(_g0679stairsNativeBitmapIndex_Up_Front_D1L, gStairFrames[kFrameStairsUpFront_D1R]);
		else
			drawFloorPitOrStairsBitmapFlippedHorizontally(_g0686stairsNativeBitmapIndex_Down_Front_D1L, gStairFrames[kFrameStairsDownFront_D1R]);
		break;
	case kWallElemType:
		drawWallSetBitmap(_wallSetBitMaps[kWall_D1LCR], gFrameWalls[kViewSquare_D1R]);
		isDrawnWallOrnAnAlcove(squareAspect[kLeftWallOrnOrdAspect], kViewWall_D1R_LEFT);
		break;
	case kStairsSideElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmapFlippedHorizontally(_g0690stairsNativeBitmapIndex_Up_Side_D1L, gStairFrames[kFrameStairsUpSide_D1R]);
		else
			drawFloorPitOrStairsBitmapFlippedHorizontally(_g0691stairsNativeBitmapIndex_Down_Side_D1L, gStairFrames[kFrameStairsDownSide_D1R]);
		break;
	default:
		break;
	}
}
void DisplayMan::drawSquareD1C(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmap(_g0680stairsNativeBitmapIndex_Up_Front_D1C, gStairFrames[kFrameStairsUpFront_D1C]);
		else
			drawFloorPitOrStairsBitmap(_g0687stairsNativeBitmapIndex_Down_Front_D1C, gStairFrames[kFrameStairsDownFront_D1C]);
		break;
	case kWallElemType:
		_vm->_dungeonMan->_isFacingAlcove = false;
		_vm->_dungeonMan->_isFacingViAltar = false;
		_vm->_dungeonMan->_isFacingFountain = false;
		drawWallSetBitmapWithoutTransparency(_wallSetBitMaps[kWall_D1LCR], gFrameWalls[kViewSquare_D1C]);
		if (isDrawnWallOrnAnAlcove(squareAspect[kFrontWallOrnOrdAspect], kViewWall_D1C_FRONT)) {
			// .... code not yet implemneted
		}
		break;
	default:
		break;
	}
}

void DisplayMan::drawSquareD0L(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsSideElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmap(_g0692stairsNativeBitmapIndex_Side_D0L, gStairFrames[kFrameStairsSide_D0L]);
		break;
	case kWallElemType:
		drawWallSetBitmap(_wallSetBitMaps[kWall_D0L], gFrameWalls[kViewSquare_D0L]);
		break;
	default:
		break;
	}
}

void DisplayMan::drawSquareD0R(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsSideElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmapFlippedHorizontally(_g0692stairsNativeBitmapIndex_Side_D0L, gStairFrames[kFrameStairsSide_D0R]);
		return;
	case kWallElemType:
		drawWallSetBitmap(_wallSetBitMaps[kWall_D0R], gFrameWalls[kViewSquare_D0R]);
		break;
	default:
		break;
	}
}

void DisplayMan::drawSquareD0C(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect]) {
			drawFloorPitOrStairsBitmap(_g0681stairsNativeBitmapIndex_Up_Front_D0C_Left, gStairFrames[kFrameStairsUpFront_D0L]);
			drawFloorPitOrStairsBitmapFlippedHorizontally(_g0681stairsNativeBitmapIndex_Up_Front_D0C_Left, gStairFrames[kFrameStairsUpFront_D0R]);
		} else {
			drawFloorPitOrStairsBitmap(_g0688stairsNativeBitmapIndex_Down_Front_D0C_Left, gStairFrames[kFrameStairsDownFront_D0L]);
			drawFloorPitOrStairsBitmapFlippedHorizontally(_g0688stairsNativeBitmapIndex_Down_Front_D0C_Left, gStairFrames[kFrameStairsDownFront_D0R]);
		}
		break;
	default:
		break;
	}
}

void DisplayMan::drawDungeon(direction dir, int16 posX, int16 posY) {
	loadPalette(gPalEntrance);
	// TODO: this is a global variable, set from here
	bool flippedFloorCeiling = (posX + posY + dir) & 1;

	// NOTE: this can hold every bitmap, width and height is "flexible"
	byte  *tmpBitmap = new byte[305 * 111];
	clearBitmap(tmpBitmap, 305, 111, kColorBlack);

	for (int16 i = 0; i < 6; ++i)
		_vm->_dungeonMan->_dungeonViewClickableBoxes[i].setToZero();

	for (uint16 i = 0; i < 6; ++i) {
		_vm->_dungeonMan->_dungeonViewClickableBoxes[i]._x1 = 255 + 1;
	}

	if (flippedFloorCeiling) {
		uint16 w = gFloorFrame._srcWidth, h = gFloorFrame._srcHeight;
		blitToBitmap(_floorBitmap, w, h, tmpBitmap, w);
		flipBitmapHorizontal(tmpBitmap, w, h);
		drawWallSetBitmap(tmpBitmap, gFloorFrame);
		drawWallSetBitmap(_ceilingBitmap, gCeilingFrame);

		for (uint16 i = 0; i <= kWall_D3LCR - kWall_D0R; ++i)
			_wallSetBitMaps[i + kWall_D0R] = _wallSetBitMaps[i + kWall_D0R_Flipped];
	} else {
		uint16 w = gCeilingFrame._srcWidth, h = gCeilingFrame._srcHeight;
		blitToBitmap(_ceilingBitmap, w, h, tmpBitmap, w);
		flipBitmapHorizontal(tmpBitmap, w, h);
		drawWallSetBitmap(tmpBitmap, gCeilingFrame);
		drawWallSetBitmap(_floorBitmap, gFloorFrame);
	}

	if (_vm->_dungeonMan->getRelSquareType(dir, 3, -2, posX, posY) == kWallElemType)
		drawWallSetBitmap(_wallSetBitMaps[kWall_D3L2], gFrameWall_D3L2);
	if (_vm->_dungeonMan->getRelSquareType(dir, 3, 2, posX, posY) == kWallElemType)
		drawWallSetBitmap(_wallSetBitMaps[kWall_D3R2], gFrameWall_D3R2);

	int16 tmpPosX = posX, tmpPosY = posY;
	_vm->_dungeonMan->mapCoordsAfterRelMovement(dir, 3, -1, tmpPosX, tmpPosY);
	drawSquareD3L(dir, tmpPosX, tmpPosY);
	tmpPosX = posX, tmpPosY = posY;
	_vm->_dungeonMan->mapCoordsAfterRelMovement(dir, 3, 1, tmpPosX, tmpPosY);
	drawSquareD3R(dir, tmpPosX, tmpPosY);
	tmpPosX = posX, tmpPosY = posY;
	_vm->_dungeonMan->mapCoordsAfterRelMovement(dir, 3, 0, tmpPosX, tmpPosY);
	drawSquareD3C(dir, tmpPosX, tmpPosY);
	tmpPosX = posX, tmpPosY = posY;
	_vm->_dungeonMan->mapCoordsAfterRelMovement(dir, 2, -1, tmpPosX, tmpPosY);
	drawSquareD2L(dir, tmpPosX, tmpPosY);
	tmpPosX = posX, tmpPosY = posY;
	_vm->_dungeonMan->mapCoordsAfterRelMovement(dir, 2, 1, tmpPosX, tmpPosY);
	drawSquareD2R(dir, tmpPosX, tmpPosY);
	tmpPosX = posX, tmpPosY = posY;
	_vm->_dungeonMan->mapCoordsAfterRelMovement(dir, 2, 0, tmpPosX, tmpPosY);
	drawSquareD2C(dir, tmpPosX, tmpPosY);
	tmpPosX = posX, tmpPosY = posY;
	_vm->_dungeonMan->mapCoordsAfterRelMovement(dir, 1, -1, tmpPosX, tmpPosY);
	drawSquareD1L(dir, tmpPosX, tmpPosY);
	tmpPosX = posX, tmpPosY = posY;
	_vm->_dungeonMan->mapCoordsAfterRelMovement(dir, 1, 1, tmpPosX, tmpPosY);
	drawSquareD1R(dir, tmpPosX, tmpPosY);
	tmpPosX = posX, tmpPosY = posY;
	_vm->_dungeonMan->mapCoordsAfterRelMovement(dir, 1, 0, tmpPosX, tmpPosY);
	drawSquareD1C(dir, tmpPosX, tmpPosY);
	tmpPosX = posX, tmpPosY = posY;
	_vm->_dungeonMan->mapCoordsAfterRelMovement(dir, 0, -1, tmpPosX, tmpPosY);
	drawSquareD0L(dir, tmpPosX, tmpPosY);
	tmpPosX = posX, tmpPosY = posY;
	_vm->_dungeonMan->mapCoordsAfterRelMovement(dir, 0, 1, tmpPosX, tmpPosY);
	drawSquareD0R(dir, tmpPosX, tmpPosY);
	drawSquareD0C(dir, posX, posY);


	for (uint16 i = 0; i <= kWall_D3LCR - kWall_D0R; ++i)
		_wallSetBitMaps[i + kWall_D0R] = _wallSetBitMaps[i + kWall_D0R_Native];

	delete[] tmpBitmap;
}

void DisplayMan::clearScreen(Color color) {
	memset(getCurrentVgaBuffer(), color, sizeof(byte) * _screenWidth * _screenHeight);
}

void DisplayMan::clearBitmap(byte *bitmap, uint16 width, uint16 height, Color color) {
	memset(bitmap, color, sizeof(byte) * width * height);
}


void DisplayMan::loadFloorSet(FloorSet set) {
	// there are 2 bitmaps per set, first one is at 75
	GraphicIndice indice = (GraphicIndice)(kFirstFloorSet + (kFloorSetGraphicCount * set));
	_floorBitmap = _bitmaps[indice];
	_ceilingBitmap = _bitmaps[indice + 1];
}


Box gBoxWallBitmap_D3LCR = Box(0, 115, 0, 50); // @ G0161_s_Graphic558_Box_WallBitmap_D3LCR
Box gBoxWallBitmap_D2LCR = Box(0, 135, 0, 70); // @ G0162_s_Graphic558_Box_WallBitmap_D2LCR

void DisplayMan::loadWallSet(WallSet set) {
	uint16 firstIndice = (set * kWallSetGraphicCount) + kFirstWallSet;
	for (uint16 i = 0; i < kWallSetGraphicCount; ++i) {
		_wallSetBitMaps[i] = _bitmaps[i + firstIndice];
	}

	for (uint16 i = 0; i <= kWall_D3LCR - kWall_D0R; ++i)
		_wallSetBitMaps[i + kWall_D0R_Native] = _wallSetBitMaps[i + kWall_D0R];

	uint16 srcIndex[7] = {kDoorFrameLeft_D1C,  kWall_D3L2, kWall_D1LCR,         kWall_D0L,         kWall_D0R,
	kWall_D2LCR,         kWall_D3LCR};

	uint16 destIndex[7] = {kDoorFrameRight_D1C, kWall_D3R2, kWall_D1LCR_Flipped, kWall_D0R_Flipped, kWall_D0L_Flipped,
	kWall_D2LCR_Flipped, kWall_D3LCR_Flipped};

	// the original loads these flipped walls in loadCurrentMapGraphics

	for (uint16 i = 0; i < 7; ++i) {
		uint16 srcGraphicIndice = firstIndice + srcIndex[i];
		uint16 w = getWidth(srcGraphicIndice), h = getHeight(srcGraphicIndice);
		delete[] _wallSetBitMaps[destIndex[i]];
		_wallSetBitMaps[destIndex[i]] = new byte[w * h];
		blitToBitmap(_wallSetBitMaps[srcIndex[i]], w, h, _wallSetBitMaps[destIndex[i]], w);
		if (srcIndex[i] != kWall_D2LCR && srcIndex[i] != kWall_D3LCR) // TODO: implement flipping of these two bitmaps, disabled with if
			flipBitmapHorizontal(_wallSetBitMaps[destIndex[i]], w, h);
	}


	/*
	uint16 graphicIndice = firstIndice + kWall_D2LCR;
	uint16 w = width(graphicIndice), h = height(graphicIndice);
	byte *tmp = new byte[w * h];
	clearBitmap(tmp, w, h, kColorFlesh);
	Box *box = &gBoxWallBitmap_D2LCR;
	blitToBitmap(_wallSetBitMaps[kWall_D2LCR_Flipped], w, 8, 0, tmp, w, box->X1, box->X2, box->Y1, box->Y2, kColorNoTransparency);
	delete[] _wallSetBitMaps[kWall_D2LCR_Flipped];
	_wallSetBitMaps[kWall_D2LCR_Flipped] = tmp;

	graphicIndice = firstIndice + kWall_D3LCR;
	w = width(graphicIndice), h = height(graphicIndice);
	tmp = new byte[w * h];
	clearBitmap(tmp, w, h, kColorFlesh);
	box = &gBoxWallBitmap_D3LCR;
	blitToBitmap(_wallSetBitMaps[kWall_D3LCR_Flipped], w, 8, 0, tmp, w, box->X1, box->X2, box->Y1, box->Y2, kColorNoTransparency);
	delete[] _wallSetBitMaps[kWall_D3LCR_Flipped];
	_wallSetBitMaps[kWall_D3LCR_Flipped] = tmp;*/
}


void DisplayMan::loadCurrentMapGraphics() {
	loadFloorSet(_vm->_dungeonMan->_currMap._map->_floorSet);
	loadWallSet(_vm->_dungeonMan->_currMap._map->_wallSet);

	// the original loads some flipped walls here, I moved it to loadWallSet

	{
		int16 val = _vm->_dungeonMan->_currMap._map->_wallSet * kStairsGraphicCount + kFirstStairs;
		_g0675stairsNativeBitmapIndex_Up_Front_D3L = val++;
		_g0676stairsNativeBitmapIndex_Up_Front_D3C = val++;
		_g0677stairsNativeBitmapIndex_Up_Front_D2L = val++;
		_g0678stairsNativeBitmapIndex_Up_Front_D2C = val++;
		_g0679stairsNativeBitmapIndex_Up_Front_D1L = val++;
		_g0680stairsNativeBitmapIndex_Up_Front_D1C = val++;
		_g0681stairsNativeBitmapIndex_Up_Front_D0C_Left = val++;
		_g0682stairsNativeBitmapIndex_Down_Front_D3L = val++;
		_g0683stairsNativeBitmapIndex_Down_Front_D3C = val++;
		_g0684stairsNativeBitmapIndex_Down_Front_D2L = val++;
		_g0685stairsNativeBitmapIndex_Down_Front_D2C = val++;
		_g0686stairsNativeBitmapIndex_Down_Front_D1L = val++;
		_g0687stairsNativeBitmapIndex_Down_Front_D1C = val++;
		_g0688stairsNativeBitmapIndex_Down_Front_D0C_Left = val++;
		_g0689stairsNativeBitmapIndex_Side_D2L = val++;
		_g0690stairsNativeBitmapIndex_Up_Side_D1L = val++;
		_g0691stairsNativeBitmapIndex_Down_Side_D1L = val++;
		_g0692stairsNativeBitmapIndex_Side_D0L = val++;
	}

	for (int16 i = 0; i < kAlcoveOrnCount; ++i)
		_currMapAlcoveOrnIndices[i] = -1;
	for (int16 i = 0; i < kFountainOrnCount; ++i)
		_currMapFountainOrnIndices[i] = -1;




	uint16 alcoveCount = 0;
	uint16 fountainCount = 0;
	Map &currMap = *_vm->_dungeonMan->_currMap._map;

	_currMapViAltarIndex = -1;

	for (uint16 i = 0; i < currMap._wallOrnCount; ++i) {
		uint16 ornIndice = _currMapWallOrnIndices[i];
		uint16 nativeIndice = kFirstWallOrn + ornIndice * 2;

		_currMapWallOrnInfo[i][kNativeBitmapIndex] = nativeIndice;
		for (uint16 ornCounter = 0; ornCounter < kAlcoveOrnCount; ++ornCounter) {
			if (ornIndice == gAlcoveOrnIndices[ornCounter]) {
				_currMapAlcoveOrnIndices[alcoveCount++] = i;
				if (ornIndice == 2)
					_currMapViAltarIndex = i;
			}
		}

		for (uint16 ornCounter = 0; ornCounter < kFountainOrnCount; ++ornCounter)
			if (ornIndice == gFountainOrnIndices[ornCounter])
				_currMapFountainOrnIndices[fountainCount++] = i;

		_currMapWallOrnInfo[i][kCoordinateSet] = gWallOrnCoordSetIndices[ornIndice];
	}

	for (uint16 i = 0; i < currMap._floorOrnCount; ++i) {
		uint16 ornIndice = _currMapFloorOrnIndices[i];
		uint16 nativeIndice = kFirstFloorOrn + ornIndice * 6;
		_currMapFloorOrnInfo[i][kNativeBitmapIndex] = nativeIndice;
		_currMapFloorOrnInfo[i][kCoordinateSet] = gFloorOrnCoordSetIndices[ornIndice];
	}

	for (uint16 i = 0; i < currMap._doorOrnCount; ++i) {
		uint16 ornIndice = _currMapDoorOrnIndices[i];
		uint16 nativeIndice = kFirstDoorOrn + ornIndice;
		_currMapDoorOrnInfo[i][kNativeBitmapIndex] = nativeIndice;
		_currMapDoorOrnInfo[i][kCoordinateSet] = gDoorOrnCoordIndices[ornIndice];
	}

	applyCreatureReplColors(9, 8);
	applyCreatureReplColors(10, 12);

	for (uint16 creatureType = 0; creatureType < currMap._creatureTypeCount; ++creatureType) {
		CreatureAspect &aspect = gCreatureAspects[_currMapAllowedCreatureTypes[creatureType]];
		uint16 replColorOrdinal = aspect.getReplColour9();
		if (replColorOrdinal)
			applyCreatureReplColors(9, _vm->ordinalToIndex(replColorOrdinal));
		replColorOrdinal = aspect.getReplColour10();
		if (replColorOrdinal)
			applyCreatureReplColors(10, _vm->ordinalToIndex(replColorOrdinal));
	}
}

void DisplayMan::applyCreatureReplColors(int replacedColor, int replacementColor) {
	for (int16 i = 0; i < 6; ++i)
		gPalDungeonView[i][replacedColor] = gCreatureReplColorSets[replacementColor]._RGBColor[i];

	gPalChangesCreature_D2[replacedColor] = gCreatureReplColorSets[replacementColor]._D2ReplacementColor;
	gPalChangesCreature_D3[replacedColor] = gCreatureReplColorSets[replacementColor]._D3ReplacementColor;
}

void DisplayMan::drawFloorPitOrStairsBitmap(uint16 nativeIndex, Frame &f) {
	if (f._srcWidth) {
		blitToScreen(_bitmaps[nativeIndex], f._srcWidth, f._srcX, f._srcY, f._box, kColorFlesh, gDungeonViewport);
	}
}

void DisplayMan::drawFloorPitOrStairsBitmapFlippedHorizontally(uint16 nativeIndex, Frame &f) {
	if (f._srcWidth) {
		blitToBitmap(_bitmaps[nativeIndex], f._srcWidth, f._srcHeight, _tmpBitmap, f._srcWidth);
		flipBitmapHorizontal(_tmpBitmap, f._srcWidth, f._srcHeight);
		blitToScreen(_tmpBitmap, f._srcWidth, f._srcX, f._srcY, f._box, kColorFlesh, gDungeonViewport);
	}
}


Box gBoxWallPatchBehindInscription = Box(110, 113, 37, 63); // @ G0202_ac_Graphic558_Box_WallPatchBehindInscription 
byte gInscriptionLineY[4] = { // @ G0203_auc_Graphic558_InscriptionLineY
	48,   /* 1 Line  */
	59,   /* 2 lines */
	75,   /* 3 lines */
	86}; /* 4 lines */
byte gWallOrnDerivedBitmapIndexIncrement[12] = { // @ G0190_auc_Graphic558_WallOrnamentDerivedBitmapIndexIncrement
	0,   /* D3L Right */
	0,   /* D3R Left */
	1,   /* D3L Front */
	1,   /* D3C Front */
	1,   /* D3R Front */
	2,   /* D2L Right */
	2,   /* D2R Left */
	3,   /* D2L Front */
	3,   /* D2C Front */
	3,   /* D2R Front */
	4,   /* D1L Right */
	4}; /* D1R Left */

byte gPalChangesDoorButtonAndWallOrn_D3[16] = {0, 0, 120, 30, 40, 30, 0, 60, 30, 90, 100, 110, 0, 10, 0, 20}; // @ G0198_auc_Graphic558_PaletteChanges_DoorButtonAndWallOrnament_D3
byte gPalChangesDoorButtonAndWallOrn_D2[16] = {0, 120, 10, 30, 40, 30, 60, 70, 50, 90, 100, 110, 0, 20, 140, 130}; // @ G0199_auc_Graphic558_PaletteChanges_DoorButtonAndWallOrnament_D2

byte gUnreadableInscriptionBoxY2[15] = { // @ G0204_auc_Graphic558_UnreadableInscriptionBoxY2
	/* { Y for 1 line, Y for 2 lines, Y for 3 lines } */
	45, 48, 53,   /* D3L Right, D3R Left */
	43, 49, 56,   /* D3L Front, D3C Front, D3R Front */
	42, 49, 56,   /* D2L Right, D2R Left */
	46, 53, 63,   /* D2L Front, D2C Front, D2R Front */
	46, 57, 68}; /* D1L Right, D1R Left */

Box gBoxChampionPortraitOnWall = Box(96, 127, 35, 63); // G0109_s_Graphic558_Box_ChampionPortraitOnWall

bool DisplayMan::isDrawnWallOrnAnAlcove(int16 wallOrnOrd, ViewWall viewWallIndex) {
	byte *bitmapGreen;
	byte *bitmapRed;
	int16 coordinateSetOffset;
	bool flipHorizontal;
	bool isInscription;
	bool isAlcove;
	Frame frame;
	unsigned char inscriptionString[70];


	if (wallOrnOrd) {
		int16 var_X;
		int16 wallOrnIndex = wallOrnOrd - 1;
		int16 nativeBitmapIndex = _currMapWallOrnInfo[wallOrnIndex][kNativeBitmapIndex];

		uint16 *coordinateSetA = gWallOrnCoordSets[_currMapWallOrnInfo[wallOrnIndex][kCoordinateSet]][viewWallIndex];
		isAlcove = _vm->_dungeonMan->isWallOrnAnAlcove(wallOrnIndex);
		isInscription = (wallOrnIndex == _vm->_dungeonMan->_currMapInscriptionWallOrnIndex);
		if (isInscription) {
			_vm->_dungeonMan->decodeText((char*)inscriptionString, _inscriptionThing, kTextTypeInscription);
		}

		if (viewWallIndex >= kViewWall_D1L_RIGHT) {
			if (viewWallIndex == kViewWall_D1C_FRONT) {
				if (isInscription) {
					Frame &D1CFrame = gFrameWalls[kViewSquare_D1C];
					blitToScreen(_wallSetBitMaps[kWall_D1LCR], D1CFrame._srcWidth, 94, 28, gBoxWallPatchBehindInscription._x1, gBoxWallPatchBehindInscription._x2,
								 gBoxWallPatchBehindInscription._y1, gBoxWallPatchBehindInscription._y2, kColorNoTransparency, gDungeonViewport);

					unsigned char *string = inscriptionString;
					bitmapRed = _bitmaps[kInscriptionFontIndice];
					int16 textLineIndex = 0;
					do {
						int16 characterCount = 0;
						unsigned char *character = string;
						while (*character++ < 0x80) {
							characterCount++;
						}
						frame._box._x2 = (frame._box._x1 = 112 - (characterCount * 4)) + 7;
						frame._box._y1 = (frame._box._y2 = gInscriptionLineY[textLineIndex++]) - 7;
						while (characterCount--) {
							blitToScreen(bitmapRed, 288, (*string++) * 8, 0, frame._box, kColorFlesh, gDungeonViewport);
							frame._box._x1 += 8;
							frame._box._x2 += 8;
						}
					} while (*string++ != 0x81);
					return isAlcove;
				}
				nativeBitmapIndex++;

				_vm->_dungeonMan->_dungeonViewClickableBoxes[kViewCellDoorButtonOrWallOrn]._x1 = coordinateSetA[0];
				_vm->_dungeonMan->_dungeonViewClickableBoxes[kViewCellDoorButtonOrWallOrn]._x2 = coordinateSetA[1];
				_vm->_dungeonMan->_dungeonViewClickableBoxes[kViewCellDoorButtonOrWallOrn]._y1 = coordinateSetA[2];
				_vm->_dungeonMan->_dungeonViewClickableBoxes[kViewCellDoorButtonOrWallOrn]._y2 = coordinateSetA[3];

				_vm->_dungeonMan->_isFacingAlcove = isAlcove;
				_vm->_dungeonMan->_isFacingViAltar = (wallOrnIndex == _currMapViAltarIndex);
				_vm->_dungeonMan->_isFacingFountain = false;
				for (int16 fountainOrnIndex = 0; fountainOrnIndex < kFountainOrnCount; ++fountainOrnIndex) {
					if (_currMapFountainOrnIndices[fountainOrnIndex] == wallOrnIndex) {
						_vm->_dungeonMan->_isFacingFountain = true;
						break;
					}
				}
			}
			bitmapGreen = _bitmaps[nativeBitmapIndex];
			if (viewWallIndex == kViewWall_D1R_LEFT) {
				blitToBitmap(bitmapGreen, coordinateSetA[4], coordinateSetA[5], _tmpBitmap, coordinateSetA[4]);
				flipBitmapHorizontal(_tmpBitmap, coordinateSetA[4], coordinateSetA[5]);
				bitmapGreen = _tmpBitmap;
			}
			var_X = 0;
		} else {
			coordinateSetOffset = 0;
			uint16 *coordSetB;
			int16 wallOrnCoordSetIndex = _currMapWallOrnInfo[wallOrnIndex][kCoordinateSet];
			flipHorizontal = (viewWallIndex == kViewWall_D2R_LEFT) || (viewWallIndex == kViewWall_D3R_LEFT);
			if (flipHorizontal) {
				coordSetB = gWallOrnCoordSets[wallOrnCoordSetIndex][kViewWall_D1R_LEFT];
			} else if ((viewWallIndex == kViewWall_D2L_RIGHT) || (viewWallIndex == kViewWall_D3L_RIGHT)) {
				coordSetB = gWallOrnCoordSets[wallOrnCoordSetIndex][kViewWall_D1L_RIGHT];
			} else {
				nativeBitmapIndex++;
				coordSetB = gWallOrnCoordSets[wallOrnCoordSetIndex][kViewWall_D1C_FRONT];
				if (viewWallIndex == kViewWall_D2L_FRONT) {
					coordinateSetOffset = 6;
				} else if (viewWallIndex == kViewWall_D2R_FRONT) {
					coordinateSetOffset = -6;
				}
			}
			int16 pixelWidth = (coordinateSetA + coordinateSetOffset)[1] - (coordinateSetA + coordinateSetOffset)[0];
			blitToBitmapShrinkWithPalChange(_bitmaps[nativeBitmapIndex], coordSetB[4] << 1, coordSetB[5], _tmpBitmap, pixelWidth + 1, coordinateSetA[5],
				(viewWallIndex <= kViewWall_D3R_FRONT) ? gPalChangesDoorButtonAndWallOrn_D3 : gPalChangesDoorButtonAndWallOrn_D2);
			bitmapGreen = _bitmaps[nativeBitmapIndex];
			var_X = pixelWidth;
			if (flipHorizontal) {
				if (bitmapGreen != _tmpBitmap)
					blitToBitmap(bitmapGreen, coordSetB[4], coordSetB[5], _tmpBitmap, coordSetB[4]);
				flipBitmapHorizontal(_tmpBitmap, coordSetB[4], coordSetB[5]);
				bitmapGreen = _tmpBitmap;
				var_X = 15 - (var_X & 0xF);
			} else if (viewWallIndex == kViewWall_D2L_FRONT) {
				var_X -= coordinateSetA[1] - coordinateSetA[0];
			} else {
				var_X = 0;
			}
		}
		if (isInscription) {
			unsigned char *string = inscriptionString;
			int16 unreadableTextLineCount = 0;
			do {
				while (*string < 0x80) {
					string++;
				}
				unreadableTextLineCount++;
			} while (*string++ != 0x81);

			if (unreadableTextLineCount < 4) {
				frame._box._x1 = coordinateSetA[0];
				frame._box._x2 = coordinateSetA[1];
				frame._box._y1 = coordinateSetA[2];
				frame._box._y2 = coordinateSetA[3];
				frame._srcWidth = coordinateSetA[4];
				frame._srcHeight = coordinateSetA[5];

				coordinateSetA = (uint16*)&frame._box;

				coordinateSetA[3] = gUnreadableInscriptionBoxY2[gWallOrnDerivedBitmapIndexIncrement[viewWallIndex] * 3 + unreadableTextLineCount - 1];
			}
		}
		blitToScreen(bitmapGreen, coordinateSetA[4], var_X, 0, coordinateSetA[0], coordinateSetA[1], coordinateSetA[2], coordinateSetA[3], kColorFlesh, gDungeonViewport);

		if ((viewWallIndex == kViewWall_D1C_FRONT) && _championPortraitOrdinal--) {
			Box &box = gBoxChampionPortraitOnWall;
			blitToScreen(_bitmaps[kChampionPortraitsIndice], 256, (_championPortraitOrdinal & 0x7) << 5, (_championPortraitOrdinal >> 3) * 29, box._x1, box._x2, box._y1, box._y2,
						 kColorDarkGary, gDungeonViewport);
		}
		return isAlcove;
	}
	return false;
}


void DisplayMan::blitToBitmapShrinkWithPalChange(byte *srcBitmap, int16 srcWidth, int16 srcHeight, byte *destBitmap, int16 destWidth, int16 destHeight, byte *palChange) {
	double rateW = srcWidth / destWidth;
	double rateH = srcHeight / destHeight;

	for (uint16 y = 0; y < destHeight; ++y) {
		for (uint16 x = 0; x < destWidth; ++x) {
			if (palChange)
				destBitmap[y * destWidth + x] = palChange[srcBitmap[(int)(y * rateH * srcWidth) + (int)(x * rateW)]];
			else
				destBitmap[y * destWidth + x] = srcBitmap[(int)(y * rateH * srcWidth) + (int)(x * rateW)];
		}
	}

}

byte* DisplayMan::getBitmap(uint16 index) {
	return _bitmaps[index];
}

Common::MemoryReadStream DisplayMan::getCompressedData(uint16 index) {
	return Common::MemoryReadStream(_packedBitmaps + _packedItemPos[index], getCompressedDataSize(index), DisposeAfterUse::NO);
}

uint32 DisplayMan::getCompressedDataSize(uint16 index) {
	return _packedItemPos[index + 1] - _packedItemPos[index];
}

/* Field Aspect Mask */
#define kMaskFieldAspectFlipMask 0x0080 // @ MASK0x0080_FLIP_MASK 
#define kMaskFieldAspectIndex 0x007F // @ MASK0x007F_MASK_INDEX
#define kMaskFieldAspectNoMask 255 // @ C255_NO_MASK            

void DisplayMan::drawField(FieldAspect* fieldAspect, Box& box) {
	DisplayMan &dispMan = *_vm->_displayMan;

	byte *bitmapMask;
	if (fieldAspect->_mask == kMaskFieldAspectNoMask) {
		bitmapMask = nullptr;
	} else {
		bitmapMask = dispMan._tmpBitmap;
		memcpy(bitmapMask, dispMan.getBitmap(kFieldMask_D3R_GraphicIndice + getFlag(fieldAspect->_mask, kMaskFieldAspectIndex)),
			   fieldAspect->_height * fieldAspect->_pixelWidth * sizeof(bitmapMask[0]));
		if (getFlag(fieldAspect->_mask, kMaskFieldAspectFlipMask)) {
			dispMan.flipBitmapHorizontal(bitmapMask, fieldAspect->_pixelWidth, fieldAspect->_height);
		}
	}

	byte *bitmap = dispMan.getBitmap(kFieldTeleporterGraphicIndice + fieldAspect->_nativeBitmapRelativeIndex);
	warning("MISSING CODE: F0133_VIDEO_BlitBoxFilledWithMaskedBitmap");

	warning("IGNORED CODE: F0491_CACHE_IsDerivedBitmapInCache, F0493_CACHE_AddDerivedBitmap, F0480_CACHE_ReleaseBlock");
}

int16 DisplayMan::getScaledBitmapPixelCount(int16 pixelWidth, int16 pixelHeight, int16 scale) {
	return getScaledDimension(pixelWidth, scale) * getScaledDimension(pixelHeight, scale);
}

int16 DisplayMan::getScaledDimension(int16 dimension, int16 scale) {
	return (dimension * scale + scale / 2) / 32;
}

/* This is the full dungeon view */
Box gBoxExplosionPattern_D0C = Box(0, 223, 0, 135); // @ G0105_s_Graphic558_Box_ExplosionPattern_D0C 

byte gExplosionBaseScales[5] = { // @ G0216_auc_Graphic558_ExplosionBaseScales
	10,/* D4 */   16,/* D3 */   23,/* D2 */   32,/* D1 */   32};/* D0 */

byte gObjectPileShiftSetIndices[16][2] = { // @ G0217_aauc_Graphic558_ObjectPileShiftSetIndices
	/* { X shift index, Y shift index } */
	{2, 5},
	{0, 6},
	{5, 7},
	{3, 0},
	{7, 1},
	{1, 2},
	{6, 3},
	{3, 3},
	{5, 5},
	{2, 6},
	{7, 7},
	{1, 0},
	{3, 1},
	{6, 2},
	{1, 3},
	{5, 3}}; /* 16 pairs of X and Y shift values */

byte gObjectCoordinateSets[3][10][5][2] = { // @ G0218_aaaauc_Graphic558_ObjectCoordinateSets
	/* { {X, Y }, {X, Y }, {X, Y }, {X, Y }, {X, Y } } */
	{{{0,   0},{0,   0},{125,  72},{95,  72},{112, 64}},     /* D3C */
	{{0,   0},{0,   0},{62,  72},{25,  72},{24, 64}},     /* D3L */
	{{0,   0},{0,   0},{200,  72},{162,  72},{194, 64}},     /* D3R */
	{{92,  78},{132,  78},{136,  86},{88,  86},{112, 74}},     /* D2C */
	{{10,  78},{53,  78},{41,  86},{0,   0},{3, 74}},     /* D2L */
	{{171,  78},{218,  78},{0,   0},{183,  86},{219, 74}},     /* D2R */
	{{83,  96},{141,  96},{148, 111},{76, 111},{112, 94}},     /* D1C */
	{{0,   0},{26,  96},{5, 111},{0,   0},{0,  0}},     /* D1L */
	{{197,  96},{0,   0},{0,   0},{220, 111},{0,  0}},     /* D1R */
	{{66, 131},{158, 131},{0,   0},{0,   0},{0,  0}}},   /* D0C */
	{{{0,   0},{0,   0},{125,  72},{95,  72},{112, 63}},     /* D3C */
	{{0,   0},{0,   0},{62,  72},{25,  72},{24, 63}},     /* D3L */
	{{0,   0},{0,   0},{200,  72},{162,  72},{194, 63}},     /* D3R */
	{{92,  78},{132,  78},{136,  86},{88,  86},{112, 73}},     /* D2C */
	{{10,  78},{53,  78},{41,  86},{0,   0},{3, 73}},     /* D2L */
	{{171,  78},{218,  78},{0,   0},{183,  86},{219, 73}},     /* D2R */
	{{83,  96},{141,  96},{148, 111},{76, 111},{112, 89}},     /* D1C */
	{{0,   0},{26,  96},{5, 111},{0,   0},{0,  0}},     /* D1L */
	{{197,  96},{0,   0},{0,   0},{220, 111},{0,  0}},     /* D1R */
	{{66, 131},{158, 131},{0,   0},{0,   0},{0,  0}}},   /* D0C */
	{{{0,   0},{0,   0},{125,  75},{95,  75},{112, 65}},     /* D3C */
	{{0,   0},{0,   0},{62,  75},{25,  75},{24, 65}},     /* D3L */
	{{0,   0},{0,   0},{200,  75},{162,  75},{194, 65}},     /* D3R */
	{{92,  81},{132,  81},{136,  88},{88,  88},{112, 76}},     /* D2C */
	{{10,  81},{53,  81},{41,  88},{0,   0},{3, 76}},     /* D2L */
	{{171,  81},{218,  81},{0,  0},{183,  88},{219, 76}},     /* D2R */
	{{83,  98},{141,  98},{148, 115},{76, 115},{112, 98}},     /* D1C */
	{{0,   0},{26,  98},{5, 115},{0,   0},{0,  0}},     /* D1L */
	{{197,  98},{0,   0},{0,   0},{220, 115},{0,  0}},     /* D1R */
	{{66, 135},{158, 135},{0,   0},{0,   0},{0,  0}}}}; /* D0C */


int16 gShiftSets[3][8] = { // @ G0223_aac_Graphic558_ShiftSets
	{0, 1, 2, 3, 0, -3, -2, -1},   /* D0 Back or D1 Front */
	{0, 1, 1, 2, 0, -2, -1, -1},   /* D1 Back or D2 Front */
	{0, 1, 1, 1, 0, -1, -1, -1}}; /* D2 Back or D3 Front */

byte gCreatureCoordinateSets[3][11][5][2] = { // @ G0224_aaaauc_Graphic558_CreatureCoordinateSets
	/* { { X, Y }, { X, Y }, { X, Y }, { X, Y }, { X, Y } } */
	{{{95,  70},{127,  70},{129,  75},{93,  75},{111,  72}},     /* D3C */
	{{131,  70},{163,  70},{158,  75},{120,  75},{145,  72}},     /* D3L */
	{{59,  70},{91,  70},{107,  75},{66,  75},{79,  72}},     /* D3R */
	{{92,  81},{131,  81},{132,  90},{91,  90},{111,  85}},     /* D2C */
	{{99,  81},{146,  81},{135,  90},{80,  90},{120,  85}},     /* D2L */
	{{77,  81},{124,  81},{143,  90},{89,  90},{105,  85}},     /* D2R */
	{{83, 103},{141, 103},{148, 119},{76, 119},{109, 111}},     /* D1C */
	{{46, 103},{118, 103},{101, 119},{0,   0},{79, 111}},     /* D1L */
	{{107, 103},{177, 103},{0,   0},{123, 119},{144, 111}},     /* D1R */
	{{0,   0},{67, 135},{0,   0},{0,   0},{0,   0}},     /* D0L */
	{{156, 135},{0,   0},{0,   0},{0,   0},{0,   0}}},   /* D0R */
	{{{94,  75},{128,  75},{111,  70},{111,  72},{111,  75}},     /* D3C */
	{{120,  75},{158,  75},{149,  70},{145,  72},{150,  75}},     /* D3L */
	{{66,  75},{104,  75},{75,  70},{79,  72},{73,  75}},     /* D3R */
	{{91,  90},{132,  90},{111,  83},{111,  85},{111,  90}},     /* D2C */
	{{80,  90},{135,  90},{125,  83},{120,  85},{125,  90}},     /* D2L */
	{{89,  90},{143,  90},{99,  83},{105,  85},{98,  90}},     /* D2R */
	{{81, 119},{142, 119},{111, 105},{111, 111},{111, 119}},     /* D1C */
	{{0,   0},{101, 119},{84, 105},{70, 111},{77, 119}},     /* D1L */
	{{123, 119},{0,   0},{139, 105},{153, 111},{146, 119}},     /* D1R */
	{{0,   0},{83, 130},{57, 121},{47, 126},{57, 130}},     /* D0L */
	{{140, 130},{0,   0},{166, 121},{176, 126},{166, 130}}},   /* D0R */
	{{{95,  59},{127,  59},{129,  61},{93,  61},{111,  60}},     /* D3C */
	{{131,  59},{163,  59},{158,  61},{120,  61},{145,  60}},     /* D3L */
	{{59,  59},{91,  59},{107,  61},{66,  61},{79,  60}},     /* D3R */
	{{92,  65},{131,  65},{132,  67},{91,  67},{111,  66}},     /* D2C */
	{{99,  65},{146,  65},{135,  67},{80,  67},{120,  66}},     /* D2L */
	{{77,  65},{124,  65},{143,  67},{89,  67},{105,  66}},     /* D2R */
	{{83,  79},{141,  79},{148,  85},{76,  85},{111,  81}},     /* D1C */
	{{46,  79},{118,  79},{101,  85},{0,   0},{79,  81}},     /* D1L */
	{{107,  79},{177,  79},{0,   0},{123,  85},{144,  81}},     /* D1R */
	{{0,   0},{67,  96},{0,   0},{0,   0},{0,   0}},     /* D0L */
	{{156,  96},{0,   0},{0,   0},{0,   0},{0,   0}}}}; /* D0R */

int16 gExplosionCoordinates[15][2][2] = { // @ G0226_aaai_Graphic558_ExplosionCoordinates
	/* { { Front Left X, Front Left Y }, { Front Right X, Front Right Y } } */
	{{100, 47},{122, 47}},   /* D4C */
	{{52, 47},{76, 47}},   /* D4L */
	{{148, 47},{172, 47}},   /* D4R */
	{{95, 50},{127, 50}},   /* D3C */
	{{31, 50},{63, 50}},   /* D3L */
	{{159, 50},{191, 50}},   /* D3R */
	{{92, 53},{131, 53}},   /* D2C */
	{{-3, 53},{46, 53}},   /* D2L */
	{{177, 53},{226, 53}},   /* D2R */
	{{83, 57},{141, 57}},   /* D1C */
	{{-54, 57},{18, 57}},   /* D1L */
	{{207, 57},{277, 57}},   /* D1R */
	{{0,  0},{0,  0}},   /* D0C */
	{{-73, 60},{-33, 60}},   /* D0L */
	{{256, 60},{296, 60}}}; /* D0R */
int16 gRebirthStep2ExplosionCoordinates[7][3] = { // @ G0227_aai_Graphic558_RebirthStep2ExplosionCoordinates
	/* { X, Y, Scale } */
	{113, 57, 12},   /* D3C */
	{24, 57, 12},   /* D3L */
	{195, 57, 12},   /* D3R */
	{111, 63, 16},   /* D2C */
	{12, 63, 16},   /* D2L */
	{213, 63, 16},   /* D2R */
	{112, 76, 24}}; /* D1C */
int16 gRebirthStep1ExplosionCoordinates[7][3] = { // @ G0228_aai_Graphic558_RebirthStep1ExplosionCoordinates
	/* { X, Y, Scale } */
	{112, 53, 15},   /* D3C */
	{24, 53, 15},   /* D3L */
	{194, 53, 15},   /* D3R */
	{112, 59, 20},   /* D2C */
	{15, 59, 20},   /* D2L */
	{208, 59, 20},   /* D2R */
	{112, 70, 32}}; /* D1C */

int16 gCenteredExplosionCoordinates[15][2] = { // @ G0225_aai_Graphic558_CenteredExplosionCoordinates
	/* { X, Y } */
	{111, 47},   /* D4C */
	{57, 47},   /* D4L */
	{167, 47},   /* D4R */
	{111, 50},   /* D3C */
	{45, 50},   /* D3L */
	{179, 50},   /* D3R */
	{111, 53},   /* D2C */
	{20, 53},   /* D2L */
	{205, 53},   /* D2R */
	{111, 57},   /* D1C */
	{-30, 57},   /* D1L */
	{253, 57},   /* D1R */
	{111, 60},   /* D0C */
	{-53, 60},   /* D0L */
	{276, 60}}; /* D0R */

#define kBlitDoNotUseMask 0x0080 // @ MASK0x0080_DO_NOT_USE_MASK

void DisplayMan::cthulhu(Thing thingParam, direction directionParam, int16 mapXpos,
														   int16 mapYpos, int16 viewSquareIndex, uint16 orderedViewCellOrdinals) {

	DungeonMan &dunMan = *_vm->_dungeonMan;

	// AL_0 shared
	uint16 &AL_0_creatureIndexRed = *(uint16*)&thingParam;
	uint16 &AL_0_creatureGraphicInfoRed = *(uint16*)&thingParam;
	uint16 &AL_0_creaturePosX = *(uint16*)&thingParam;
	// AL_1 shared
	int16 &AL_1_viewSquareExplosionIndex = viewSquareIndex;
	// AL_2 shared
	int16 L0126_i_Multiple;
	int16 &AL_2_viewCell = L0126_i_Multiple;
	int16 &AL_2_cellPurpleMan = L0126_i_Multiple;
	int16 &AL_2_explosionSize = L0126_i_Multiple;
	// AL_4 shared
	int16 L0127_i_Multiple;
	int16 &AL_4_thingType = L0127_i_Multiple;
	int16 &AL_4_nativeBitmapIndex = L0127_i_Multiple;
	int16 &AL_4_xPos = L0127_i_Multiple;
	int16 &AL_4_groupCells = L0127_i_Multiple;
	int16 &AL_4_normalizdByteWidth = L0127_i_Multiple;
	int16 &AL_4_yPos = L0127_i_Multiple;
	int16 &AL_4_projectileAspect = L0127_i_Multiple;
	int16 &AL_4_explosionType = L0127_i_Multiple;
	int16 &AL_4_explosionAspectIndex = L0127_i_Multiple;
	// AL_6 shared
	byte *L0128_puc_Multiple;
	byte *&AL_6_bitmapRedBanana = L0128_puc_Multiple;

	ObjectAspect *objectAspect;
	uint32 remainingViewCellOrdinalsToProcess;
	byte* paletteChanges;
	byte* bitmapGreenAnt;
	byte* coordinateSet;
	int16 derivedBitmapIndex;

	int16 byteWidth;
	int16 heightRedEagle;
	int16 viewLane; /* The lane (center/left/right) that the specified square is part of */
	int16 cellYellowBear;
	int16 paddingPixelCount;
	int16 heightGreenGoat;
	bool useAlcoveObjectImage; /* C1_TRUE for objects that have a special graphic when drawn in an alcove, like the Chest */
	bool flipHorizontal;
	bool drawingGrabbableObject;
	Box boxByteGreen;
	Thing firstThingToDraw; /* Initialized to thingParam and never changed afterwards. Used as a backup of the specified first object to draw */

	int16 cellCounter;
	uint16 objectShiftIndex;

	uint16 L0150_ui_Multiple;
	uint16 &AL_8_shiftSetIndex = L0150_ui_Multiple;
	uint16 &AL_8_projectileScaleIndex = L0150_ui_Multiple;

	Thing groupThing;
	Group* group;
	ActiveGroup* activeGroup;
	CreatureInfo* creatureInfo;
	CreatureAspect* creatureAspectStruct;
	int16 creatureSize;
	int16 creatureDirectionDelta;
	int16 creatureGraphicInfoGreen;
	int16 creatureAspectInt;
	int16 creatureIndexGreen;
	int16 transparentColor;
	int16 sourceByteWidth;
	int16 sourceHeight;
	int16 creaturePaddingPixelCount;
	bool twoHalfSquareCreaturesFrontView;
	bool drawingLastBackRowCell;
	bool useCreatureSideBitmap;
	bool useCreatureBackBitmap;
	bool useCreatureSpecialD2FrontBitmap;
	bool useCreatureAttackBitmap;
	bool useFlippedHorizontallyCreatureFrontImage;

/* Set to C1_TRUE when the last creature that the function should draw is being drawn. This is used to avoid processing the code to draw creatures for the remaining square cells */
	bool drawCreaturesCompleted;

	int16 doorFrontViewDrawingPass; /* Value 0, 1 or 2 */
	int16 scale;
	bool derivedBitmapInCache;
	Projectile* projectile;
	byte projectileCoordinates[2];
	int16 projectilePosX;
	int16 projectileDirection;
	int16 projectileAspectType;
	int16 projectileBitmapIndexData;
	bool doNotScaleWithKineticEnergy;

/* When true, the code section to draw an object is called (with a goto) to draw the projectile, then the code section goes back to projectile processing with another goto */
	bool drawProjectileAsObject;

	bool sqaureHasProjectile;
	uint16 currentViewCellToDraw;
	bool projectileFlipVertical;
	bool projectileAspectTypeHasBackGraphicAndRotation;
	bool flipVertical;
	Explosion* explosion;
	Explosion* fluxcageExplosion;
	int16* explosionCoordinates;
	int16 explosionScale;
	bool squareHasExplosion;
	bool rebirthExplosion;
	bool smoke;
	FieldAspect fieldAspect;


	if (thingParam == Thing::_endOfList)
		return;

	group = 0;
	groupThing = Thing::_none;
	drawCreaturesCompleted = sqaureHasProjectile = squareHasExplosion = false;
	cellCounter = 0;
	firstThingToDraw = thingParam;
	if (getFlag(orderedViewCellOrdinals, kCellOrder_DoorFront)) { /* If the function call is to draw objects on a door square viewed from the front */
/* Two function calls are made in that case to draw objects on both sides of the door frame.
The door and its frame are drawn between the two calls. This value indicates the drawing pass so that
creatures are drawn in the right order and so that Fluxcages are not drawn twice */
		doorFrontViewDrawingPass = (orderedViewCellOrdinals & 0x1) + 1;
		orderedViewCellOrdinals >>= 4;	/* Remove the first nibble that was used for the door front view pass */
	} else {
		doorFrontViewDrawingPass = 0;  /* The function call is not to draw objects on a door square viewed from the front */
	}

	bool drawAlcoveObjects = !(remainingViewCellOrdinalsToProcess = orderedViewCellOrdinals);
	uint16 viewSquareIndexBackup = viewSquareIndex;
	viewLane = (viewSquareIndex + 3) % 3;


	do {
/* Draw objects */
		if (drawAlcoveObjects) {
			AL_2_viewCell = kViewCellAlcove; /* Index of coordinates to draw objects in alcoves */
			cellYellowBear = returnOppositeDir(directionParam); /* Alcove is on the opposite direction of the viewing direction */
			objectShiftIndex = 2;
		} else {
			AL_2_viewCell = _vm->ordinalToIndex((int16)remainingViewCellOrdinalsToProcess & 0x000F); /* View cell is the index of coordinates to draw object */
			currentViewCellToDraw = AL_2_viewCell;
			remainingViewCellOrdinalsToProcess >>= 4; /* Proceed to the next cell ordinal */
			cellCounter++;
			cellYellowBear = (AL_2_viewCell + directionParam) % 3; /* Convert view cell to absolute cell */
			thingParam = firstThingToDraw;
			viewSquareIndex = viewSquareIndexBackup; /* Restore value as it may have been modified while drawing a creature */
			objectShiftIndex = 0;
		}

		objectShiftIndex += (cellYellowBear & 0x0001) << 3;
		drawProjectileAsObject = false;
		do {
			if ((AL_4_thingType = thingParam.getType()) == kGroupThingType) {
				groupThing = thingParam;
				continue;
			}
			if (AL_4_thingType == kProjectileThingType) {
				sqaureHasProjectile = true;
				continue;
			}
			if (AL_4_thingType == kExplosionThingType) {
				squareHasExplosion = true;
				continue;
			}

			/* Square where objects are visible and object is located on cell being processed */
			if ((viewSquareIndex >= kViewSquare_D3C) && (viewSquareIndex <= kViewSquare_D0C) && (thingParam.getCell() == cellYellowBear)) {
				objectAspect = &(gObjectAspects[gObjectInfo[dunMan.getObjectInfoIndex(thingParam)]._objectAspectIndex]);
				AL_4_nativeBitmapIndex = kFirstObjectGraphicIndice + objectAspect->_firstNativeBitmapRelativeIndex;
				if (useAlcoveObjectImage = (drawAlcoveObjects && getFlag(objectAspect->_graphicInfo, kObjectAlcoveMask) && !viewLane)) {
					AL_4_nativeBitmapIndex++;
				}
				coordinateSet = gObjectCoordinateSets[objectAspect->_coordinateSet][viewSquareIndex][AL_2_viewCell];
				if (!coordinateSet[1]) /* If object is not visible */
					continue;
T0115015_DrawProjectileAsObject:
				flipHorizontal = getFlag(objectAspect->_graphicInfo, kObjectFlipOnRightMask) &&
					!useAlcoveObjectImage &&
					((viewLane == kViewLaneRight) || (!viewLane && ((AL_2_viewCell == kViewCellFrontRight) || (AL_2_viewCell == kViewCellBackRight))));
				/* Flip horizontally if object graphic requires it and is not being drawn in an alcove and the object is
				either on the right lane or on the right column of the center lane */
				paddingPixelCount = 0;

				if ((viewSquareIndex == kViewSquare_D0C) || ((viewSquareIndex >= kViewSquare_D1C) && (AL_2_viewCell >= kViewCellBackRight))) {
					/* If object is in the center lane (only D0C or D1C with condition above) and is not a projectile */
					drawingGrabbableObject = (!viewLane && !drawProjectileAsObject);
					AL_8_shiftSetIndex = kShiftSet_D0BackD1Front;
					AL_6_bitmapRedBanana = getBitmap(AL_4_nativeBitmapIndex); /* Use base graphic, no resizing */
					byteWidth = objectAspect->_width;
					heightRedEagle = objectAspect->_height;
					if (flipHorizontal) {
						memcpy(_tmpBitmap, AL_6_bitmapRedBanana, byteWidth * heightRedEagle * sizeof(byte));
						flipBitmapHorizontal(_tmpBitmap, byteWidth, heightRedEagle);
						AL_6_bitmapRedBanana = _tmpBitmap;
					}
				} else {
					drawingGrabbableObject = false;
					derivedBitmapIndex = kDerivedBitmapFirstObject + objectAspect->_firstDerivedBitmapRelativeIndex;
					if ((viewSquareIndex >= kViewSquare_D1C) || ((viewSquareIndex >= kViewSquare_D2C) && (AL_2_viewCell >= kViewCellBackRight))) {
						derivedBitmapIndex++;
						AL_8_shiftSetIndex = kShiftSet_D1BackD2Front;
						byteWidth = getScaledDimension(objectAspect->_width, kScale20_D2);
						heightRedEagle = getScaledDimension(objectAspect->_height, kScale20_D2);
						paletteChanges = gPalChangesFloorOrn_D2;
					} else {
						AL_8_shiftSetIndex = kShiftSet_D2BackD3Front;
						byteWidth = getScaledDimension(objectAspect->_width, kScale16_D3);
						heightRedEagle = getScaledDimension(objectAspect->_height, kScale16_D3);
						paletteChanges = gPalChangesFloorOrn_D3;
					}
					if (flipHorizontal) {
						derivedBitmapIndex += 2;
						paddingPixelCount = (7 - ((byteWidth / 2 - 1) & 0x0007)) << 1;
					} else if (useAlcoveObjectImage) {
						derivedBitmapIndex += 4;
					}

					if (isDerivedBitmapInCache(derivedBitmapIndex)) {
						AL_6_bitmapRedBanana = getDerivedBitmap(derivedBitmapIndex);
					} else {
						bitmapGreenAnt = getBitmap(AL_4_nativeBitmapIndex);
						blitToBitmapShrinkWithPalChange(bitmapGreenAnt, objectAspect->_width, objectAspect->_height, AL_6_bitmapRedBanana = getDerivedBitmap(derivedBitmapIndex),
														byteWidth, heightRedEagle, paletteChanges);
						if (flipHorizontal) {
							flipBitmapHorizontal(AL_6_bitmapRedBanana, byteWidth, heightRedEagle);
						}
						warning("IGNORED CODE: F0493_CACHE_AddDerivedBitmap");
					}
				}
				AL_4_xPos = coordinateSet[0];
				boxByteGreen._y2 = coordinateSet[1] + 1;
				if (!drawProjectileAsObject) { /* If drawing an object that is not a projectile */
					AL_4_xPos += gShiftSets[AL_8_shiftSetIndex][gObjectPileShiftSetIndices[objectShiftIndex][0]];
					boxByteGreen._y2 += gShiftSets[AL_8_shiftSetIndex][gObjectPileShiftSetIndices[objectShiftIndex][1]];
					objectShiftIndex++; /* The next object drawn will use the next shift values */
					if (drawAlcoveObjects) {
						if (objectShiftIndex >= 14) {
							objectShiftIndex = 2;
						}
					} else {
						objectShiftIndex &= 0x000F;
					}
				}
				boxByteGreen._y1 = boxByteGreen._y2 - (heightRedEagle - 1) - 1;
				if (boxByteGreen._y2 > 136) {
					boxByteGreen._y2 = 136;
				}
				boxByteGreen._x2 = MIN(224, AL_4_xPos + byteWidth);
				if (boxByteGreen._x1 = MAX(0, AL_4_xPos - byteWidth + 1)) {
					if (flipHorizontal) {
						AL_4_xPos = paddingPixelCount;
					} else {
						AL_4_xPos = 0;
					}
				} else {
					AL_4_xPos = byteWidth - AL_4_xPos - 1;
				}

				if (drawingGrabbableObject) {
					bitmapGreenAnt = AL_6_bitmapRedBanana;
					Box *AL_6_boxPtrRed = &dunMan._dungeonViewClickableBoxes[AL_2_viewCell];
					if (AL_6_boxPtrRed->_x1 == 255) { /* If the grabbable object is the first */
						*AL_6_boxPtrRed = boxByteGreen;

						if ((heightGreenGoat = AL_6_boxPtrRed->_y2 - AL_6_boxPtrRed->_y1) < 15) { /* If the box is too small then enlarge it a little */
							heightGreenGoat = heightGreenGoat >> 1;
							AL_6_boxPtrRed->_y1 += heightGreenGoat - 7;
							if (heightGreenGoat < 4) {
								AL_6_boxPtrRed->_y2 -= heightGreenGoat - 3;
							}
						}
					} else { /* If there are several grabbable objects then enlarge the box so it includes all objects */
						AL_6_boxPtrRed->_x1 = MIN(AL_6_boxPtrRed->_x1, boxByteGreen._x1);
						AL_6_boxPtrRed->_x2 = MIN(AL_6_boxPtrRed->_x2, boxByteGreen._x2);
						AL_6_boxPtrRed->_y1 = MIN(AL_6_boxPtrRed->_y1, boxByteGreen._y1);
						AL_6_boxPtrRed->_y2 = MIN(AL_6_boxPtrRed->_y2, boxByteGreen._y2);
					}
					AL_6_bitmapRedBanana = bitmapGreenAnt;
					dunMan._pileTopObject[AL_2_viewCell] = thingParam; /* The object is at the top of the pile */
				}
				blitToScreen(AL_6_bitmapRedBanana, byteWidth, AL_4_xPos, 0, boxByteGreen, kColorFlesh, gDungeonViewport);

				if (drawProjectileAsObject)
					goto T0115171_BackFromT0115015_DrawProjectileAsObject;
			}
		} while ((thingParam = dunMan.getNextThing(thingParam)) != Thing::_endOfList);
		if (AL_2_viewCell == kViewCellAlcove)
			break; /* End of processing when drawing objects in an alcove */
		if (viewSquareIndex < kViewSquare_D3C)
			break; /* End of processing if square is too far away at D4 */
				   /* Draw creatures */

		/* If (draw cell on the back row or second cell being processed) and (no more cells to draw or next cell to draw is a cell on the front row) */
		drawingLastBackRowCell = ((AL_2_viewCell <= kViewCellFrontRight) || (cellCounter == 1))
			&& (!remainingViewCellOrdinalsToProcess || ((remainingViewCellOrdinalsToProcess & 0x0000000F) >= 3));

		if ((groupThing == Thing::_none) || drawCreaturesCompleted)
			goto T0115129_DrawProjectiles; /* Skip code to draw creatures */
		if (group == nullptr) { /* If all creature data and info has not already been gathered */
			group = (Group*)dunMan.getThingData(groupThing);
			activeGroup = &_vm->_groupMan->_activeGroups[group->getActiveGroupIndex()];
			creatureInfo = &gCreatureInfo[group->_type];
			creatureAspectStruct = &gCreatureAspects[creatureInfo->_creatureAspectIndex];
			creatureSize = getFlag(creatureInfo->_attributes, kMaskCreatureInfo_size);
			creatureGraphicInfoGreen = creatureInfo->_graphicInfo;
		}
		objectAspect = (ObjectAspect*)creatureAspectStruct;
		if (AL_0_creatureIndexRed = _vm->_groupMan->getCreatureOrdinalInCell(group, cellYellowBear)) { /* If there is a creature on the cell being processed */
			AL_0_creatureIndexRed--; /* Convert ordinal to index */
			creatureIndexGreen = AL_0_creatureIndexRed;
		} else if (creatureSize == kMaskCreatureSizeHalf) {
			AL_0_creatureIndexRed = 0;
			creatureIndexGreen = -1;
		} else {
			goto T0115129_DrawProjectiles; /* No creature to draw at cell, skip to projectiles */
		}

		creatureDirectionDelta = (directionParam - _vm->_groupMan->getCreatureValue(activeGroup->_directions, AL_0_creatureIndexRed)) % 3;
		twoHalfSquareCreaturesFrontView = false;
		if ((AL_4_groupCells = activeGroup->_cells) == kCreatureTypeSingleCenteredCreature) { /* If there is a single centered creature in the group */
			if (remainingViewCellOrdinalsToProcess || (doorFrontViewDrawingPass == 1))
/* Do not draw a single centered creature now, wait until second pass (for a front view door)
 or until all cells have been drawn so the creature is drawn over all the objects on the floor */
				goto T0115129_DrawProjectiles; 
			drawCreaturesCompleted = true;
			if ((creatureSize == kMaskCreatureSizeHalf) && (creatureDirectionDelta & 0x0001)) { /* Side view of half square creature */
				AL_2_viewCell = kHalfSizedViewCell_CenterColumn;
			} else {
				AL_2_viewCell = kHalfSizedViewCell_FrontRow;
			}
		} else if ((creatureSize == kMaskCreatureSizeHalf) && (drawingLastBackRowCell || !remainingViewCellOrdinalsToProcess || (creatureIndexGreen < 0))) {
			if (drawingLastBackRowCell && (doorFrontViewDrawingPass != 2)) {
				if ((creatureIndexGreen >= 0) && (creatureDirectionDelta & 0x0001)) {
					AL_2_viewCell = kHalfSizedViewCell_BackRow; /* Side view of a half square creature on the back row. Drawn during pass 1 for a door square */
				} else {
					goto T0115129_DrawProjectiles;
				}
			} else if ((doorFrontViewDrawingPass != 1) && !remainingViewCellOrdinalsToProcess) {
				if (creatureDirectionDelta & 0x0001) {
					if (creatureIndexGreen >= 0) {
						AL_2_viewCell = kHalfSizedViewCell_FrontRow; /* Side view of a half square creature on the front row. Drawn during pass 2 for a door square */
					} else {
						goto T0115129_DrawProjectiles;
					}
				} else {
					drawCreaturesCompleted = true;
					if (creatureIndexGreen < 0) {
						creatureIndexGreen = 0;
					}
					twoHalfSquareCreaturesFrontView = group->getCount();
					if (((AL_4_groupCells = _vm->_groupMan->getCreatureValue(AL_4_groupCells, AL_0_creatureIndexRed)) == directionParam)
						|| (AL_4_groupCells == returnPrevVal(directionParam))) {
						AL_2_viewCell = kHalfSizedViewCell_LeftColumn;
					} else {
						AL_2_viewCell = kHalfSizedViewCell_RightColumn;
					}
				}
			} else {
				goto T0115129_DrawProjectiles;
			}

		} else if (creatureSize != kMaskCreatureSizeQuarter)
			goto T0115129_DrawProjectiles;


		creatureAspectInt = activeGroup->_aspect[creatureIndexGreen];
		if (viewSquareIndex > kViewSquare_D0C) {
			viewSquareIndex--;
		}
T0115077_DrawSecondHalfSquareCreature:
		coordinateSet = gCreatureCoordinateSets[((CreatureAspect*)objectAspect)->getCoordSet()][viewSquareIndex][AL_2_viewCell];
		if (!coordinateSet[1])
			goto T0115126_CreatureNotVisible;
		AL_0_creatureGraphicInfoRed = creatureGraphicInfoGreen;
		AL_4_nativeBitmapIndex = kFirstCreatureGraphicIndice + ((CreatureAspect*)objectAspect)->_firstNativeBitmapRelativeIndex; /* By default, assume using the front image */
		derivedBitmapIndex = ((CreatureAspect*)objectAspect)->_firstDerivedBitmapIndex;
		if (useCreatureSideBitmap = getFlag(AL_0_creatureGraphicInfoRed, kCreatureInfoGraphicMaskSide) && (creatureDirectionDelta & 0x0001)) {
			useCreatureAttackBitmap = useFlippedHorizontallyCreatureFrontImage = useCreatureBackBitmap = false;
			AL_4_nativeBitmapIndex++; /* Skip the front image. Side image is right after the front image */
			derivedBitmapIndex += 2;
			sourceByteWidth = byteWidth = ((CreatureAspect*)objectAspect)->_byteWidthSide;
			sourceHeight = heightRedEagle = ((CreatureAspect*)objectAspect)->_heightSide;
		} else {
			useCreatureBackBitmap = getFlag(AL_0_creatureGraphicInfoRed, kCreatureInfoGraphicMaskBack) && (creatureDirectionDelta == 0);
			if (useCreatureAttackBitmap = !useCreatureBackBitmap && getFlag(creatureAspectInt, kMaskActiveGroupIsAttacking)
				&& getFlag(AL_0_creatureGraphicInfoRed, kCreatureInfoGraphicMaskAttack)) {

				useFlippedHorizontallyCreatureFrontImage = false;
				sourceByteWidth = byteWidth = ((CreatureAspect*)objectAspect)->_byteWidthAttack;
				sourceHeight = heightRedEagle = ((CreatureAspect*)objectAspect)->_heightAttack;
				AL_4_nativeBitmapIndex++; /* Skip the front image */
				derivedBitmapIndex += 2;
				if (getFlag(AL_0_creatureGraphicInfoRed, kCreatureInfoGraphicMaskSide)) {
					AL_4_nativeBitmapIndex++; /* If the creature has a side image, it preceeds the attack image */
					derivedBitmapIndex += 2;
				}
				if (getFlag(AL_0_creatureGraphicInfoRed, kCreatureInfoGraphicMaskBack)) {
					AL_4_nativeBitmapIndex++; /* If the creature has a back image, it preceeds the attack image */
					derivedBitmapIndex += 2;
				}
			} else {
				sourceByteWidth = byteWidth = ((CreatureAspect*)objectAspect)->_byteWidthFront;
				sourceHeight = heightRedEagle = ((CreatureAspect*)objectAspect)->_heightFront;
				if (useCreatureBackBitmap) {
					useFlippedHorizontallyCreatureFrontImage = false;
					if (getFlag(AL_0_creatureGraphicInfoRed, kCreatureInfoGraphicMaskSide)) {
						AL_4_nativeBitmapIndex += 2; /* If the creature has a side image, it preceeds the back image */
						derivedBitmapIndex += 4;
					} else {
						AL_4_nativeBitmapIndex++; /* If the creature does not have a side image, the back image follows the front image */
						derivedBitmapIndex += 2;
					}
				} else {
					if (useFlippedHorizontallyCreatureFrontImage = getFlag(AL_0_creatureGraphicInfoRed, kCreatureInfoGraphicMaskFlipNonAttack)
						&& getFlag(creatureAspectInt, kMaskActiveGroupFlipBitmap)) {
						derivedBitmapIndex += 2;
						if (getFlag(AL_0_creatureGraphicInfoRed, kCreatureInfoGraphicMaskSide)) {
							derivedBitmapIndex += 2;
						}
						if (getFlag(AL_0_creatureGraphicInfoRed, kCreatureInfoGraphicMaskBack)) {
							derivedBitmapIndex += 2;
						}
						if (getFlag(AL_0_creatureGraphicInfoRed, kCreatureInfoGraphicMaskAttack)) {
							derivedBitmapIndex += 2;
						}
					}
				}
			}
		}
		if (viewSquareIndex >= kViewSquare_D1C) { /* Creature is on D1 */
			creaturePaddingPixelCount = 0;
			AL_8_shiftSetIndex = kShiftSet_D0BackD1Front;
			transparentColor = ((CreatureAspect*)objectAspect)->getTranspColour();
			if (useCreatureSideBitmap) {
				AL_6_bitmapRedBanana = getBitmap(AL_4_nativeBitmapIndex);
				if (creatureDirectionDelta == 1) {
					memcpy(_tmpBitmap, AL_6_bitmapRedBanana, byteWidth * heightRedEagle * sizeof(byte));
					flipBitmapHorizontal(_tmpBitmap, byteWidth, heightRedEagle);
					AL_6_bitmapRedBanana = _tmpBitmap;
				}
			} else {
				if (useCreatureBackBitmap || !useFlippedHorizontallyCreatureFrontImage) {
					AL_6_bitmapRedBanana = getBitmap(AL_4_nativeBitmapIndex);
					if (useCreatureAttackBitmap && getFlag(creatureAspectInt, kMaskActiveGroupFlipBitmap)) {
						memcpy(_tmpBitmap, AL_6_bitmapRedBanana, byteWidth * heightRedEagle * sizeof(byte));
						flipBitmapHorizontal(_tmpBitmap, byteWidth, heightRedEagle);
						AL_6_bitmapRedBanana = _tmpBitmap;
					}
				} else { /* Use first additional derived graphic: front D1 */
					if (isDerivedBitmapInCache(derivedBitmapIndex)) { /* If derived graphic is already in memory */
						AL_6_bitmapRedBanana = getDerivedBitmap(derivedBitmapIndex);
					} else {
						bitmapGreenAnt = getBitmap(AL_4_nativeBitmapIndex);
						if (getFlag(AL_0_creatureGraphicInfoRed, kCreatureInfoGraphicMaskFlipNonAttack)) {
							AL_6_bitmapRedBanana = getDerivedBitmap(derivedBitmapIndex);
							memcpy(AL_6_bitmapRedBanana, bitmapGreenAnt, byteWidth * heightRedEagle * sizeof(byte));
							flipBitmapHorizontal(AL_6_bitmapRedBanana, byteWidth, heightRedEagle);
						}
						warning("IGNORED CODE: F0493_CACHE_AddDerivedBitmap");
					}
				}
			}
		} else { /* Creature is on D2 or D3 */
			if (useFlippedHorizontallyCreatureFrontImage) {
				derivedBitmapIndex++; /* Skip front D1 image in additional graphics */
			}
			if (viewSquareIndex >= kViewSquare_D2C) { /* Creature is on D2 */
				derivedBitmapIndex++; /* Skip front D3 image in additional graphics */
				AL_8_shiftSetIndex = kShiftSet_D1BackD2Front;
				useCreatureSpecialD2FrontBitmap = getFlag(AL_0_creatureGraphicInfoRed, kCreatureInfoGraphicMaskSpecialD2Front)
					&& !useCreatureSideBitmap && !useCreatureBackBitmap && !useCreatureAttackBitmap;
				paletteChanges = gPalChangesCreature_D2;
				scale = kScale20_D2;
			} else { /* Creature is on D3 */
				AL_8_shiftSetIndex = kShiftSet_D2BackD3Front;
				useCreatureSpecialD2FrontBitmap = false;
				paletteChanges = gPalChangesCreature_D3;
				scale = kScale16_D3;
			}
			byteWidth = getScaledDimension(sourceByteWidth, scale);
			heightRedEagle = getScaledDimension(sourceHeight, scale);
			transparentColor = paletteChanges[((CreatureAspect*)objectAspect)->getTranspColour()] / 10;
			if (derivedBitmapInCache = isDerivedBitmapInCache(derivedBitmapIndex)) {
				AL_6_bitmapRedBanana = getDerivedBitmap(derivedBitmapIndex);
			} else {
				bitmapGreenAnt = getBitmap(AL_4_nativeBitmapIndex);
				AL_6_bitmapRedBanana = getDerivedBitmap(derivedBitmapIndex);
				blitToBitmapShrinkWithPalChange(bitmapGreenAnt, sourceByteWidth, sourceHeight, AL_6_bitmapRedBanana, byteWidth, heightRedEagle, paletteChanges);
				warning("IGNORED CODE: F0493_CACHE_AddDerivedBitmap");
			}
			if ((useCreatureSideBitmap && (creatureDirectionDelta == 1)) || /* If creature is viewed from the right, the side view must be flipped */
				(useCreatureAttackBitmap && getFlag(creatureAspectInt, kMaskActiveGroupFlipBitmap)) ||
				(useCreatureSpecialD2FrontBitmap && getFlag(AL_0_creatureGraphicInfoRed, kCreatureInfoGraphicMaskSpecialD2FrontIsFlipped)) ||
				(useFlippedHorizontallyCreatureFrontImage && getFlag(AL_0_creatureGraphicInfoRed, kCreatureInfoGraphicMaskFlipNonAttack))) { /* If the graphic should be flipped */
				if (!useFlippedHorizontallyCreatureFrontImage || !derivedBitmapInCache) {
					AL_4_normalizdByteWidth = byteWidth;
					warning("SUPER WARNING: we might need getNormalizedByteWidthM77");
					if (!useFlippedHorizontallyCreatureFrontImage) {
						memcpy(_tmpBitmap, AL_6_bitmapRedBanana, AL_4_normalizdByteWidth * heightRedEagle * sizeof(byte));
						AL_6_bitmapRedBanana = _tmpBitmap;
					}
					flipBitmapHorizontal(AL_6_bitmapRedBanana, AL_4_normalizdByteWidth, heightRedEagle);
				}
				creaturePaddingPixelCount = (7 - ((byteWidth / 2 - 1) & 0x0007)) << 1;
			} else {
				creaturePaddingPixelCount = 0;
			}
		}
		AL_4_yPos = coordinateSet[1];
		AL_4_yPos += gShiftSets[AL_8_shiftSetIndex][getVerticalOffsetM23(creatureAspectInt)];
		boxByteGreen._y2 = MIN(AL_4_yPos, (int16)135) + 1;
		boxByteGreen._y1 = MIN(0, AL_4_yPos - (heightRedEagle - 1));
		AL_4_xPos = coordinateSet[0];
		AL_4_xPos += gShiftSets[AL_8_shiftSetIndex][getHorizontalOffsetM22(creatureAspectInt)];
		if (viewLane == kViewLaneLeft) {
			AL_4_xPos -= 100;
		} else {
			if (viewLane) { /* Lane right */
				AL_4_xPos += 100;
			}
		}
		if (boxByteGreen._x2 = 1 + MIN(MAX(0, AL_4_xPos + byteWidth), 223) <= 1)
			goto T0115126_CreatureNotVisible;
		if (boxByteGreen._x1 = MIN(MAX(0, AL_4_xPos - byteWidth + 1), 223)) {
			if (boxByteGreen._x1 == 223)
				goto T0115126_CreatureNotVisible;
			AL_0_creaturePosX = creaturePaddingPixelCount;
		} else {
			AL_0_creaturePosX = creaturePaddingPixelCount + (byteWidth - AL_4_xPos - 1);
		}
		warning("SUPER WARNINIG: we might nee noralized with on byteWidth");
		blitToScreen(AL_6_bitmapRedBanana, byteWidth, AL_0_creaturePosX, 0, boxByteGreen, (Color)transparentColor, gDungeonViewport);

T0115126_CreatureNotVisible:
		if (twoHalfSquareCreaturesFrontView) {
			twoHalfSquareCreaturesFrontView = false;
			creatureAspectInt = activeGroup->_aspect[!creatureIndexGreen]; /* Aspect of the other creature in the pair */
			if (AL_2_viewCell == kHalfSizedViewCell_RightColumn) {
				AL_2_viewCell = kHalfSizedViewCell_LeftColumn;
			} else {
				AL_2_viewCell = kHalfSizedViewCell_RightColumn;
			}
			goto T0115077_DrawSecondHalfSquareCreature;
		}
		/* Draw projectiles */
T0115129_DrawProjectiles:
		if (!sqaureHasProjectile
			|| ((viewSquareIndex = viewSquareIndexBackup) > kViewSquare_D0C)
/* If there is no projectile to draw or if projectiles are not visible on the specified square or on the cell being drawn */
			|| (!(projectilePosX = gObjectCoordinateSets[0][viewSquareIndex][AL_2_viewCell = currentViewCellToDraw][0]))) 
			continue;
		thingParam = firstThingToDraw; /* Restart processing list of objects from the beginning. The next loop draws only projectile objects among the list */

		do {
			if ((thingParam.getType() == kProjectileThingType) && (thingParam.getCell() == cellYellowBear)) {
				projectile = (Projectile*)dunMan.getThingData(thingParam);
				if ((AL_4_projectileAspect = dunMan.getProjectileAspect(projectile->_object)) < 0) { /* Negative value: projectile aspect is the ordinal of a PROJECTIL_ASPECT */
					objectAspect = (ObjectAspect*)&gProjectileAspect[_vm->ordinalToIndex(-AL_4_projectileAspect)];
					AL_4_nativeBitmapIndex = ((ProjectileAspect*)objectAspect)->_firstNativeBitmapRelativeIndex + kFirstProjectileGraphicIndice;
					projectileAspectType = getFlag(((ProjectileAspect*)objectAspect)->_graphicInfo, kProjectileAspectTypeMask);
					if (((doNotScaleWithKineticEnergy = !getFlag(((ProjectileAspect*)objectAspect)->_graphicInfo, kProjectileScaleWithKineticEnergyMask))
						 || (projectile->_kineticEnergy == 255)) && (viewSquareIndex == kViewSquare_D0C)) {
						scale = 0; /* Use native bitmap without resizing */
						byteWidth = ((ProjectileAspect*)objectAspect)->_width;
						heightRedEagle = ((ProjectileAspect*)objectAspect)->_height;
					} else {
						AL_8_projectileScaleIndex = ((viewSquareIndex / 3) << 1) + (AL_2_viewCell >> 1);
						scale = gProjectileScales[AL_8_projectileScaleIndex];
						if (!doNotScaleWithKineticEnergy) {
							scale = (scale * MAX(96, projectile->_kineticEnergy + 1)) >> 8;
						}
						byteWidth = getScaledDimension(((ProjectileAspect*)objectAspect)->_width, scale);
						heightRedEagle = getScaledDimension(((ProjectileAspect*)objectAspect)->_height, scale);
					}
					if (projectileAspectTypeHasBackGraphicAndRotation = (projectileAspectType == kProjectileAspectHasBackGraphicRotation)) {
						projectileFlipVertical = ((mapXpos + mapYpos) & 0x0001);
					}
					if (projectileAspectType == kProjectileAspectHasNone) {
						projectileBitmapIndexData = 0;
						flipVertical = flipHorizontal = false;
					} else {
						if (isOrientedWestEast((direction)(projectileDirection = _vm->_timeline->_events[projectile->_timerIndex]._C._projectile.getDir()))
							!= isOrientedWestEast(directionParam)) {
							if (projectileAspectType == kProjectileAspectHasRotation) {
								projectileBitmapIndexData = 1;
							} else {
								projectileBitmapIndexData = 2;
							}
							if (projectileAspectTypeHasBackGraphicAndRotation) {
								flipHorizontal = !AL_2_viewCell || (AL_2_viewCell == kViewCellBackLeft);
								if (!(flipVertical = projectileFlipVertical)) {
									flipHorizontal = !flipHorizontal;
								}
							} else {
								flipVertical = false;
								flipHorizontal = (returnNextVal(directionParam) == projectileDirection);
							}
						} else {
/* If the projectile does not have a back graphic or has one but is not seen from the back or if it has a back graphic and rotation and should be flipped vertically */
							if ((projectileAspectType >= kProjectileAspectHasRotation)
								|| ((projectileAspectType == kProjectileAspectBackGraphic)
									&& (projectileDirection != directionParam)) || (projectileAspectTypeHasBackGraphicAndRotation && projectileFlipVertical)) {
								projectileBitmapIndexData = 0;
							} else {
								projectileBitmapIndexData = 1;
							}
							flipVertical = projectileAspectTypeHasBackGraphicAndRotation && (AL_2_viewCell < kViewCellBackRight);
							flipHorizontal = getFlag(((ProjectileAspect*)objectAspect)->_graphicInfo, kProjectileSideMask)
								&& !((viewLane == kViewLaneRight) || (!viewLane && ((AL_2_viewCell == kViewCellFrontRight) || (AL_2_viewCell == kViewCellBackRight))));
						}
					}
					AL_4_nativeBitmapIndex += projectileBitmapIndexData;
					paddingPixelCount = 0;
					if (!scale) {
						AL_6_bitmapRedBanana = getBitmap(AL_4_nativeBitmapIndex);
					} else {
						if (flipHorizontal) {
							paddingPixelCount = (7 - ((byteWidth / 2 - 1) & 0x0007)) << 1;
						}
						derivedBitmapIndex = kDerivedBitmapFirstProjectile + ((ProjectileAspect*)objectAspect)->_firstNativeBitmapRelativeIndex + (projectileBitmapIndexData * 6);
						if (doNotScaleWithKineticEnergy && isDerivedBitmapInCache(derivedBitmapIndex) + AL_8_projectileScaleIndex) {
							AL_6_bitmapRedBanana = getDerivedBitmap(derivedBitmapIndex);
						} else {
							bitmapGreenAnt = getBitmap(AL_4_nativeBitmapIndex);
							if (doNotScaleWithKineticEnergy) {
								AL_6_bitmapRedBanana = getDerivedBitmap(derivedBitmapIndex);
							} else {
								AL_6_bitmapRedBanana = _tmpBitmap;
							}
							blitToBitmapShrinkWithPalChange(bitmapGreenAnt, ((ProjectileAspect*)objectAspect)->_width, ((ProjectileAspect*)objectAspect)->_height,
															AL_6_bitmapRedBanana, byteWidth, heightRedEagle, _palChangesProjectile[AL_8_projectileScaleIndex >> 1]);
							if (doNotScaleWithKineticEnergy) {
								warning("IGNORED CODE F0493_CACHE_AddDerivedBitmap");
							}
						}
					}
					if (flipHorizontal || flipVertical) {
						warning("might need noralized bytewidth");
						AL_4_normalizdByteWidth = byteWidth;
						if (AL_6_bitmapRedBanana != _tmpBitmap) {
							memcpy(_tmpBitmap, AL_6_bitmapRedBanana, AL_4_normalizdByteWidth * heightRedEagle * sizeof(byte));
							AL_6_bitmapRedBanana = _tmpBitmap;
						}
						if (flipVertical) {
							flipBitmapVertical(AL_6_bitmapRedBanana, AL_4_normalizdByteWidth, heightRedEagle);
						}
						if (flipHorizontal) {
							flipBitmapHorizontal(AL_6_bitmapRedBanana, AL_4_normalizdByteWidth, heightRedEagle);
						}
					}
					boxByteGreen._y2 = (heightRedEagle >> 1) + 47 + 1;
					boxByteGreen._y1 = 47 - (heightRedEagle >> 1) + !(heightRedEagle & 0x0001);
					boxByteGreen._x2 = MIN(223, projectilePosX + byteWidth) + 1;
					if (boxByteGreen._x1 = MAX(0, projectilePosX - byteWidth + 1)) {
						if (flipHorizontal) {
							AL_4_xPos = paddingPixelCount;
						} else {
							AL_4_xPos = 0;
						}
					} else {
/* BUG0_06 Graphical glitch when drawing projectiles or explosions. If a projectile or explosion bitmap
is cropped because it is only partly visible on the left side of the viewport (boxByteGreen.X1 = 0) and
the bitmap is flipped horizontally (flipHorizontal = C1_TRUE) then a wrong part of the bitmap is drawn on
screen. To fix this bug, "+ paddingPixelCount" must be added to the second parameter of this function call */
						AL_4_xPos = MAX(paddingPixelCount, (int16)(byteWidth - projectilePosX - 1));
					}
					blitToScreen(AL_6_bitmapRedBanana, byteWidth, AL_4_xPos, 0, boxByteGreen, kColorFlesh, gDungeonViewport);
				} else { /* Positive value: projectile aspect is the index of a OBJECT_ASPECT */
					useAlcoveObjectImage = false;
					projectileCoordinates[0] = projectilePosX;
					projectileCoordinates[1] = 47;
					coordinateSet = projectileCoordinates;
					objectAspect = &gObjectAspects[AL_4_projectileAspect];
					AL_4_nativeBitmapIndex = objectAspect->_firstNativeBitmapRelativeIndex + kFirstObjectGraphicIndice;
					drawProjectileAsObject = true;
/* Go to code section to draw an object. Once completed, it jumps back to T0115171_BackFromT0115015_DrawProjectileAsObject below */
					goto T0115015_DrawProjectileAsObject; 
				}
			}
T0115171_BackFromT0115015_DrawProjectileAsObject:;
		} while ((thingParam = dunMan.getNextThing(thingParam)) != Thing::_endOfList);

	} while (remainingViewCellOrdinalsToProcess);


	/* Draw explosions */
	if (!squareHasExplosion)
		return;
	fluxcageExplosion = 0;
	AL_1_viewSquareExplosionIndex = viewSquareIndexBackup + 3; /* Convert square index to square index for explosions */
	uint16 explosionScaleIndex = AL_1_viewSquareExplosionIndex / 3;
	thingParam = firstThingToDraw; /* Restart processing list of things from the beginning. The next loop draws only explosion things among the list */
	do {
		if (thingParam.getType() == kExplosionThingType) {
			AL_2_cellPurpleMan = thingParam.getCell();
			explosion = (Explosion*)dunMan.getThingData(thingParam);
			if ((rebirthExplosion = ((unsigned int)(AL_4_explosionType = explosion->getType()) >= kExplosionType_RebirthStep1))
				&& ((AL_1_viewSquareExplosionIndex < kViewSquare_D3C_Explosion) 
					|| (AL_1_viewSquareExplosionIndex > kViewSquare_D1C_Explosion)
					|| (AL_2_cellPurpleMan != cellYellowBear))) /* If explosion is rebirth and is not visible */
				continue;
			smoke = false;
			if ((AL_4_explosionType == kExplosionType_Fireball) || (AL_4_explosionType == kExplosionType_LightningBolt) || (AL_4_explosionType == kExplosionType_RebirthStep2)) {
				AL_4_explosionAspectIndex = kExplosionAspectFire;
			} else {
				if ((AL_4_explosionType == kExplosionType_PoisonBolt) || (AL_4_explosionType == kExplosionType_PoisonCloud)) {
					AL_4_explosionAspectIndex = kExplosionAspectPoison;
				} else {
					if (AL_4_explosionType == kExplosionType_Smoke) {
						smoke = true;
						AL_4_explosionAspectIndex = kExplosionAspectSmoke;
					} else {
						if (AL_4_explosionType == kExplosionType_RebirthStep1) {
							objectAspect = (ObjectAspect*)&gProjectileAspect[_vm->ordinalToIndex(-dunMan.getProjectileAspect(Thing::_explLightningBolt))];
							AL_6_bitmapRedBanana = getBitmap(((ProjectileAspect*)objectAspect)->_firstNativeBitmapRelativeIndex + (kFirstProjectileGraphicIndice + 1));
							explosionCoordinates = gRebirthStep1ExplosionCoordinates[AL_1_viewSquareExplosionIndex - 3];
							byteWidth = getScaledDimension((((ProjectileAspect*)objectAspect)->_width), explosionCoordinates[2]);
							heightRedEagle = getScaledDimension((((ProjectileAspect*)objectAspect)->_height), explosionCoordinates[2]);
							if (AL_1_viewSquareExplosionIndex != kViewSquare_D1C_Explosion) {
								blitToBitmapShrinkWithPalChange(AL_6_bitmapRedBanana,
									((ProjectileAspect*)objectAspect)->_width, ((ProjectileAspect*)objectAspect)->_height, _tmpBitmap,
																		 byteWidth, heightRedEagle, gPalChangesNoChanges);
								AL_6_bitmapRedBanana = _tmpBitmap;
							}
							goto T0115200_DrawExplosion;
						}
						if (AL_4_explosionType == kExplosionType_Fluxcage) {
							if (AL_1_viewSquareExplosionIndex >= kViewSquare_D3L_Explosion) {
								fluxcageExplosion = explosion;
							}
							continue;
						}
						AL_4_explosionAspectIndex = kExplosionAspectSpell;
					}
				}
			}
			if (AL_1_viewSquareExplosionIndex == kViewSquare_D0C_Explosion) {
				if (smoke) {
					AL_4_explosionAspectIndex--; /* Smoke uses the same graphics as Poison Cloud, but with palette changes */
				}
				AL_4_explosionAspectIndex = AL_4_explosionAspectIndex * 3; /* 3 graphics per explosion pattern */
				if (AL_2_explosionSize = (explosion->getAttack() >> 5)) {
					AL_4_explosionAspectIndex++; /* Use second graphic in the pattern for medium explosion attack */
					if (AL_2_explosionSize > 3) {
						AL_4_explosionAspectIndex++; /* Use third graphic in the pattern for large explosion attack */
					}
				}
				warning("IGNORED CODE: F0491_CACHE_IsDerivedBitmapInCache");
				AL_6_bitmapRedBanana = getBitmap(AL_4_explosionAspectIndex + kFirstExplosionPatternGraphicIndice);
				if (smoke) {
					blitToBitmapShrinkWithPalChange(AL_6_bitmapRedBanana, 48, 32, _tmpBitmap, 48, 32, gPalChangeSmoke);
					AL_6_bitmapRedBanana = _tmpBitmap;
				}
				blitBoxFilledWithMaskedBitmapToScreen(AL_6_bitmapRedBanana, nullptr, getDerivedBitmap(kDerivedBitmapViewport), gBoxExplosionPattern_D0C,
													  _vm->_rnd->getRandomNumber(4) + 87, _vm->_rnd->getRandomNumber(64),
													  224, (Color)(kBlitDoNotUseMask | kColorFlesh), 0, 0, 136, 93);
				warning("IGNORED CODE: F0493_CACHE_AddDerivedBitmap");
				warning("IGNORED CODE: F0493_CACHE_AddDerivedBitmap");
			} else {
				if (rebirthExplosion) {
					explosionCoordinates = gRebirthStep2ExplosionCoordinates[AL_1_viewSquareExplosionIndex - 3];
					explosionScale = explosionCoordinates[2];
				} else {
					if (explosion->getCentered()) {
						explosionCoordinates = gCenteredExplosionCoordinates[AL_1_viewSquareExplosionIndex];
					} else {
						if ((AL_2_cellPurpleMan == directionParam) || (AL_2_cellPurpleMan == returnPrevVal(directionParam))) {
							AL_2_viewCell = kViewCellFronLeft;
						} else {
							AL_2_viewCell = kViewCellFrontRight;
						}
						explosionCoordinates = gExplosionCoordinates[AL_1_viewSquareExplosionIndex][AL_2_viewCell];
					}
					explosionScale = MAX(4, (MAX(48, explosion->getAttack() + 1) * gExplosionBaseScales[explosionScaleIndex]) >> 8) & (int16)0xFFFE;
				}
				AL_6_bitmapRedBanana = getExplosionBitmap(AL_4_explosionAspectIndex, explosionScale, byteWidth, heightRedEagle);
T0115200_DrawExplosion:
				flipVertical = _vm->_rnd->getRandomNumber(2);
				paddingPixelCount = 0;
				if (flipHorizontal = _vm->_rnd->getRandomNumber(2)) {
					paddingPixelCount = (7 - ((byteWidth / 2 - 1) & 0x0007)) << 1; /* Number of unused pixels in the units on the right of the bitmap */
				}
				boxByteGreen._y2 = MIN(135, explosionCoordinates[1] + (heightRedEagle >> 1)) + 1; 
				AL_4_yPos = MAX(0, explosionCoordinates[1] - (heightRedEagle >> 1) + !(heightRedEagle & 0x0001));
				if (AL_4_yPos >= 136)
					continue;
				boxByteGreen._y1 = AL_4_yPos;
				if ((AL_4_xPos = MIN(223, explosionCoordinates[0] + byteWidth)) < 0)
					continue;
				boxByteGreen._x2 = AL_4_xPos + 1;
				AL_4_xPos = explosionCoordinates[0];
				if (boxByteGreen._x1 = MIN(MAX(0, AL_4_xPos - byteWidth + 1), 223)) {
					AL_4_xPos = paddingPixelCount;
				} else {
/* BUG0_07 Graphical glitch when drawing explosions. If an explosion bitmap is cropped because it is only partly visible on the
left side of the viewport (boxByteGreen.X1 = 0) and the bitmap is not flipped horizontally (flipHorizontal = C0_FALSE) then the
variable paddingPixelCount is not set before being used here. Its previous value (defined while drawing something else) is used
and may cause an incorrect bitmap to be drawn */
					AL_4_xPos = MIN(paddingPixelCount,(int16)( byteWidth / 2 - AL_4_xPos - 1)); 

/* BUG0_06 Graphical glitch when drawing projectiles or explosions. If a projectile or explosion bitmap is cropped because it is
only partly visible on the left side of the viewport (boxByteGreen.X1 = 0) and the bitmap is flipped horizontally (flipHorizontal = C1_TRUE)
then a wrong part of the bitmap is drawn on screen. To fix this bug, "+ paddingPixelCount" must be added to the second parameter of this function call */
				}
				if (boxByteGreen._x2 - 1 <= boxByteGreen._x1)
					continue;
				warning("might need M77_NORMALIZED_BYTE_WIDTH");
				byteWidth = byteWidth;
				if (flipHorizontal || flipVertical) {
					memcpy(_tmpBitmap, AL_6_bitmapRedBanana, byteWidth * heightRedEagle);
					AL_6_bitmapRedBanana = _tmpBitmap;
				}
				if (flipHorizontal) {
					flipBitmapHorizontal(AL_6_bitmapRedBanana, byteWidth, heightRedEagle);
				}
				if (flipVertical) {
					flipBitmapVertical(AL_6_bitmapRedBanana, byteWidth, heightRedEagle);
				}
				blitToScreen(AL_6_bitmapRedBanana, byteWidth, AL_4_xPos, 0, boxByteGreen, kColorFlesh, gDungeonViewport);
			}
		}
	} while ((thingParam = dunMan.getNextThing(thingParam)) != Thing::_endOfList);
/* Fluxcage is an explosion displayed as a field (like teleporters), above all other graphics */
	if ((fluxcageExplosion != 0) && (doorFrontViewDrawingPass != 1) && !_doNotDrawFluxcagesDuringEndgame) { 
		AL_1_viewSquareExplosionIndex -= 3; /* Convert square index for explosions back to square index */
		fieldAspect = gFieldAspects[viewSquareIndex];
		(fieldAspect._nativeBitmapRelativeIndex)++; /* NativeBitmapRelativeIndex is now the index of the Fluxcage field graphic */
		drawField(&fieldAspect, *(Box*)&gFrameWalls[viewSquareIndex]);
	}
}

uint16 DisplayMan::getNormalizedByteWidthM77(uint16 byteWidth) {
	return (byteWidth + 7) & 0xFFF8;
}

uint16 DisplayMan::getVerticalOffsetM23(uint16 val) {
	return (val >> 3) & 0x7;
}

uint16 DisplayMan::getHorizontalOffsetM22(uint16 val) {
	return (val & 0x7);
}

bool DisplayMan::isDerivedBitmapInCache(int16 derivedBitmapIndex) {
	if (_derivedBitmaps[derivedBitmapIndex] == nullptr) {
		// * 2, because the original uses 4 bits instead of 8 bits to store a pixel
		_derivedBitmaps[derivedBitmapIndex] = new byte[_derivedBitmapByteCount[derivedBitmapIndex] * 2];
		return false;
	} else
		return true;
}

byte* DisplayMan::getDerivedBitmap(int16 derivedBitmapIndex) {
	return _derivedBitmaps[derivedBitmapIndex];
}

void DisplayMan::clearScreenBox(Color color, Box &box, Viewport &viewport) {
	uint16 width = box._x2 - box._x1;
	for (int y = box._y1 + viewport._posY; y < box._y2 + viewport._posY; ++y)
		memset(_vgaBuffer + y * _screenWidth + box._x1 + viewport._posX, color, sizeof(byte) * width);
}

void DisplayMan::blitToScreen(byte *srcBitmap, uint16 srcWidth, uint16 srcX, uint16 srcY,
							  Box &box,
							  Color transparent, Viewport &viewport) {
	blitToScreen(srcBitmap, srcWidth, srcX, srcY, box._x1, box._x2, box._y1, box._y2, transparent, viewport);
}

void DisplayMan::blitBoxFilledWithMaskedBitmap(byte* src, byte* dest, byte* mask, byte* tmp, Box& box,
											   int16 lastUnitIndex, int16 firstUnitIndex, int16 destPixelWidth, Color transparent,
											   int16 xPos, int16 yPos, int16 destHeight, int16 height2, Viewport &viewport) {
	warning("STUB FUNCTION: does nothing at all");
}

}
