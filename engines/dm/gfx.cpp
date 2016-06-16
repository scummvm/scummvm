#include "engines/util.h"
#include "common/system.h"
#include "common/file.h"
#include "graphics/palette.h"
#include "common/endian.h"

#include "gfx.h"
#include "dungeonman.h"


namespace DM {

enum ViewCell {
	kViewCellFronLeft = 0, // @ C00_VIEW_CELL_FRONT_LEFT
	kViewCellFrontRight = 1, // @ C01_VIEW_CELL_FRONT_RIGHT
	kViewCellBackRight = 2, // @ C02_VIEW_CELL_BACK_RIGHT
	kViewCellBackLeft = 3, // @ C03_VIEW_CELL_BACK_LEFT
	kViewCellAlcove = 4, // @ C04_VIEW_CELL_ALCOVE
	kViewCellDoorButtonOrWallOrn = 5 // @ C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT
};

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

Frame gFrameWall_D3L2 = Frame(0,  15, 25, 73, 8, 49, 0, 0); // @ G0711_s_Graphic558_Frame_Wall_D3L2
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
	kWall_D3LCR_Native = 24,
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

enum GraphicIndice {
	kInscriptionFontIndice = 120, // @ C120_GRAPHIC_INSCRIPTION_FONT
	kDoorMaskDestroyedIndice = 301, // @ C301_GRAPHIC_DOOR_MASK_DESTROYED
	kChampionPortraitsIndice = 26 // @ C026_GRAPHIC_CHAMPION_PORTRAITS
};


Viewport gDefultViewPort = {0, 0};
// TODO: I guessed the numbers
Viewport gDungeonViewport = {0, 64}; // @ G0296_puc_Bitmap_Viewport

byte gPalChangesNoChanges[16] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150}; // @ G0017_auc_Graphic562_PaletteChanges_NoChanges

byte gPalChangesFloorOrn_D3[16] = {0, 120, 10, 30, 40, 30, 0, 60, 30, 90, 100, 110, 0, 20, 140, 130}; // @ G0213_auc_Graphic558_PaletteChanges_FloorOrnament_D3
byte gPalChangesFloorOrn_D2[16] = {0, 10, 20, 30, 40, 30, 60, 70, 50, 90, 100, 110, 120, 130, 140, 150}; // @ G0214_auc_Graphic558_PaletteChanges_FloorOrnament_D2

int gFountainOrnIndices[kFountainOrnCount] = {35}; // @ G0193_ai_Graphic558_FountainOrnamentIndices
byte gAlcoveOrnIndices[kAlcoveOrnCount] = { // @ G0192_auc_Graphic558_AlcoveOrnamentIndices
	1,   /* Square Alcove */
	2,   /* Vi Altar */
	3};  /* Arched Alcove */
}

using namespace DM;

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

	_screenWidth = _screenHeight = 0;
	_championPortraitOrdinal = 0;
	_currMapViAltarIndex = 0;

	for (int i = 0; i < 25; i++)
		_wallSetBitMaps[i] = nullptr;

	for (int i = 0; i < kStairsGraphicCount; i++)
		_stairIndices[i] = 0;
	
	for (int i = 0; i < 4; i++)
		_palChangesProjectile[i] =  nullptr;

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

	_inscriptionThing = Thing::thingNone;
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

	for (uint16 i = kDoorOrnDestroyedMask; i <= kDoorOrnThivesEyeMask; ++i) {
		_currMapDoorOrnInfo[i][kNativeBitmapIndex] = i + (kDoorMaskDestroyedIndice - kDoorOrnDestroyedMask);
		_currMapDoorOrnInfo[i][kNativeCoordinateSet] = 1;
	}

	_currMapFloorOrnInfo[kFloorOrnFootprints][kNativeBitmapIndex] = 1;
	_currMapFloorOrnInfo[kFloorOrnFootprints][kNativeCoordinateSet] = 1;

	_palChangesProjectile[0] = gPalChangesFloorOrn_D3;
	_palChangesProjectile[1] = gPalChangesFloorOrn_D2;
	_palChangesProjectile[2] = _palChangesProjectile[3] = gPalChangesNoChanges;

	loadFloorSet(kFloorSetStone);
	loadWallSet(kWallSetStone);

}

void DisplayMan::unpackGraphics() {
	uint32 unpackedBitmapsSize = 0;
	for (uint16 i = 0; i <= 20; ++i)
		unpackedBitmapsSize += width(i) * height(i);
	for (uint16 i = 22; i <= 532; ++i)
		unpackedBitmapsSize += width(i) * height(i);
	// graphics items go from 0-20 and 22-532 inclusive, _unpackedItemPos 21 and 22 are there for indexing convenience
	if (_bitmaps) {
		delete[] _bitmaps[0];
		delete[] _bitmaps;
	}
	_bitmaps = new byte*[533];
	_bitmaps[0] = new byte[unpackedBitmapsSize];
	loadIntoBitmap(0, _bitmaps[0]);
	for (uint16 i = 1; i <= 20; ++i) {
		_bitmaps[i] = _bitmaps[i - 1] + width(i - 1) * height(i - 1);
		loadIntoBitmap(i, _bitmaps[i]);
	}
	_bitmaps[22] = _bitmaps[20] + width(20) * height(20);
	for (uint16 i = 23; i < 533; ++i) {
		_bitmaps[i] = _bitmaps[i - 1] + width(i - 1) * height(i - 1);
		loadIntoBitmap(i, _bitmaps[i]);
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
				destBitmap[destWidth * (y + destFromY + destViewport.posY) + destFromX + x + destViewport.posX] = srcPixel;
		}
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

void DisplayMan::flipBitmapHorizontal(byte *bitmap, uint16 width, uint16 height) {
	for (uint16 y = 0; y < height; ++y)
		for (uint16 x = 0; x < width / 2; ++x) {
			byte tmp;
			tmp = bitmap[y*width + x];
			bitmap[y*width + x] = bitmap[y*width + width - 1 - x];
			bitmap[y*width + width - 1 - x] = tmp;
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



void DisplayMan::updateScreen() {
	_vm->_system->copyRectToScreen(_vgaBuffer, _screenWidth, 0, 0, _screenWidth, _screenHeight);
	_vm->_system->updateScreen();
}

byte *DisplayMan::getCurrentVgaBuffer() {
	return _vgaBuffer;
}

uint16 DisplayMan::width(uint16 index) {
	byte *data = _packedBitmaps + _packedItemPos[index];
	return READ_BE_UINT16(data);
}

uint16 DisplayMan::height(uint16 index) {
	uint8 *data = _packedBitmaps + _packedItemPos[index];
	return READ_BE_UINT16(data + 2);
}


void DisplayMan::drawWallSetBitmapWithoutTransparency(byte *bitmap, Frame &f) {
	if (f.srcWidth)
		blitToScreen(bitmap, f.srcWidth, f.srcX, f.srcY, f.destFromX, f.destToX, f.destFromY, f.destToY, kColorNoTransparency, gDungeonViewport);
}

void DisplayMan::drawWallSetBitmap(byte *bitmap, Frame &f) {
	if (f.srcWidth)
		blitToScreen(bitmap, f.srcWidth, f.srcX, f.srcY, f.destFromX, f.destToX, f.destFromY, f.destToY, kColorFlesh, gDungeonViewport);
}

void DisplayMan::drawSquareD3L(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[kElemAspect]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmap(kStairsNativeIndex_Up_Front_D3L, gStairFrames[kFrameStairsUpFront_D3L]);
		else
			drawFloorPitOrStairsBitmap(kStairsNativeIndex_Down_Front_D3L, gStairFrames[kFrameStairsDownFront_D3L]);
		break;
	case kWallElemType:
		drawWallSetBitmap(_wallSetBitMaps[kWall_D3LCR], gFrameWalls[kViewSquare_D3L]);
		isDrawnWallOrnAnAlcove(squareAspect[kRightWallOrnOrdAspect], kViewWall_D3L_RIGHT);
		if (isDrawnWallOrnAnAlcove(squareAspect[kFrontWallOrnOrdAspect], kViewWall_D3L_FRONT)) {
			// ... missing code
		}
		break;
	}
}

void DisplayMan::drawSquareD3R(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmapFlippedHorizontally(kStairsNativeIndex_Up_Front_D3L, gStairFrames[kFrameStairsUpFront_D3R]);
		else
			drawFloorPitOrStairsBitmapFlippedHorizontally(kStairsNativeIndex_Down_Front_D3L, gStairFrames[kFrameStairsDownFront_D3R]);
		break;
	case kWallElemType:
		drawWallSetBitmap(_wallSetBitMaps[kWall_D3LCR], gFrameWalls[kViewSquare_D3R]);
		isDrawnWallOrnAnAlcove(squareAspect[kLeftWallOrnOrdAspect], kViewWall_D3R_LEFT);
		if (isDrawnWallOrnAnAlcove(squareAspect[kFrontWallOrnOrdAspect], kViewWall_D3R_FRONT)) {
			// ... missing code
		}
		break;
	}
}
void DisplayMan::drawSquareD3C(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmap(kStairsNativeIndex_Up_Front_D3C, gStairFrames[kFrameStairsUpFront_D3C]);
		else
			drawFloorPitOrStairsBitmap(kStairsNativeIndex_Down_Front_D3C, gStairFrames[kFrameStairsDownFront_D3C]);
		break;
	case kWallElemType:
		drawWallSetBitmapWithoutTransparency(_wallSetBitMaps[kWall_D3LCR], gFrameWalls[kViewSquare_D3C]);
		if (isDrawnWallOrnAnAlcove(squareAspect[kFrontWallOrnOrdAspect], kViewWall_D3C_FRONT)) {
			//... missing code
		}
		break;
	}
}
void DisplayMan::drawSquareD2L(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmap(kStairsNativeIndex_Up_Front_D2L, gStairFrames[kFrameStairsUpFront_D2L]);
		else
			drawFloorPitOrStairsBitmap(kStairsNativeIndex_Down_Front_D2L, gStairFrames[kFrameStairsDownFront_D2L]);
		break;
	case kWallElemType:
		drawWallSetBitmap(_wallSetBitMaps[kWall_D2LCR], gFrameWalls[kViewSquare_D2L]);
		isDrawnWallOrnAnAlcove(squareAspect[kRightWallOrnOrdAspect], kViewWall_D2L_RIGHT);
		if (isDrawnWallOrnAnAlcove(squareAspect[kFrontWallOrnOrdAspect], kViewWall_D2L_FRONT)) {
			// ... missing code
		}
		break;
	case kStairsSideElemType:
		drawFloorPitOrStairsBitmap(kStairsNativeIndex_Side_D2L, gFrameWalls[kFrameStairsSide_D2L]);
		break;
	}
}
void DisplayMan::drawSquareD2R(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmapFlippedHorizontally(kStairsNativeIndex_Up_Front_D2L, gStairFrames[kFrameStairsUpFront_D2R]);
		else
			drawFloorPitOrStairsBitmapFlippedHorizontally(kStairsNativeIndex_Down_Front_D2L, gStairFrames[kFrameStairsDownFront_D2R]);
		break;
	case kWallElemType:
		drawWallSetBitmap(_wallSetBitMaps[kWall_D2LCR], gFrameWalls[kViewSquare_D2R]);
		isDrawnWallOrnAnAlcove(squareAspect[kLeftWallOrnOrdAspect], kViewWall_D2R_LEFT);
		if (isDrawnWallOrnAnAlcove(squareAspect[kFrontWallOrnOrdAspect], kViewWall_D2R_FRONT)) {
			// ... missing code
		}
		break;
	case kStairsSideElemType:
		drawFloorPitOrStairsBitmapFlippedHorizontally(kStairsNativeIndex_Side_D2L, gStairFrames[kFrameStairsSide_D2R]);
		break;
	}
}
void DisplayMan::drawSquareD2C(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmap(kStairsNativeIndex_Up_Front_D2C, gStairFrames[kFrameStairsUpFront_D2C]);
		else
			drawFloorPitOrStairsBitmap(kStairsNativeIndex_Down_Front_D2C, gStairFrames[kFrameStairsDownFront_D2C]);
		break;
	case kWallElemType:
		drawWallSetBitmapWithoutTransparency(_wallSetBitMaps[kWall_D2LCR], gFrameWalls[kViewSquare_D2C]);
		if (isDrawnWallOrnAnAlcove(squareAspect[kFrontWallOrnOrdAspect], kViewWall_D2C_FRONT)) {
			// ... missing code
		}
		break;
	}
}
void DisplayMan::drawSquareD1L(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmap(kStairsNativeIndex_Up_Front_D1L, gStairFrames[kFrameStairsUpFront_D1L]);
		else
			drawFloorPitOrStairsBitmap(kStairsNativeIndex_Down_Front_D1L, gStairFrames[kFrameStairsDownFront_D1L]);
		break;
	case kWallElemType:
		drawWallSetBitmap(_wallSetBitMaps[kWall_D1LCR], gFrameWalls[kViewSquare_D1L]);
		isDrawnWallOrnAnAlcove(squareAspect[kRightWallOrnOrdAspect], kViewWall_D1L_RIGHT);
		break;
	case kStairsSideElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmap(kStairsNativeIndex_Up_Side_D1L, gStairFrames[kFrameStairsUpSide_D1L]);
		else
			drawFloorPitOrStairsBitmap(kStairsNativeIndex_Down_Side_D1L, gStairFrames[kFrameStairsDownSide_D1L]);
		break;
	}
}
void DisplayMan::drawSquareD1R(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmapFlippedHorizontally(kStairsNativeIndex_Up_Front_D1L, gStairFrames[kFrameStairsUpFront_D1R]);
		else
			drawFloorPitOrStairsBitmapFlippedHorizontally(kStairsNativeIndex_Down_Front_D1L, gStairFrames[kFrameStairsDownFront_D1R]);
		break;
	case kWallElemType:
		drawWallSetBitmap(_wallSetBitMaps[kWall_D1LCR], gFrameWalls[kViewSquare_D1R]);
		isDrawnWallOrnAnAlcove(squareAspect[kLeftWallOrnOrdAspect], kViewWall_D1R_LEFT);
		break;
	case kStairsSideElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmapFlippedHorizontally(kStairsNativeIndex_Up_Side_D1L, gStairFrames[kFrameStairsUpSide_D1R]);
		else
			drawFloorPitOrStairsBitmapFlippedHorizontally(kStairsNativeIndex_Down_Side_D1L, gStairFrames[kFrameStairsDownSide_D1R]);
		break;
	}
}
void DisplayMan::drawSquareD1C(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmap(kStairsNativeIndex_Up_Front_D1C, gStairFrames[kFrameStairsUpFront_D1C]);
		else
			drawFloorPitOrStairsBitmap(kStairsNativeIndex_Down_Front_D1C, gStairFrames[kFrameStairsDownFront_D1C]);
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
	}
}

void DisplayMan::drawSquareD0L(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsSideElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmap(kStairsNativeIndex_Side_D0L, gStairFrames[kFrameStairsSide_D0L]);
		break;
	case kWallElemType:
		drawWallSetBitmap(_wallSetBitMaps[kWall_D0L], gFrameWalls[kViewSquare_D0L]);
		break;
	}
}

void DisplayMan::drawSquareD0R(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsSideElemType:
		if (squareAspect[kStairsUpAspect])
			drawFloorPitOrStairsBitmapFlippedHorizontally(kStairsNativeIndex_Side_D0L, gStairFrames[kFrameStairsSide_D0R]);
		return;
	case kWallElemType:
		drawWallSetBitmap(_wallSetBitMaps[kWall_D0R], gFrameWalls[kViewSquare_D0R]);
		break;
	}
}

void DisplayMan::drawSquareD0C(direction dir, int16 posX, int16 posY) {
	uint16 squareAspect[5];
	_vm->_dungeonMan->setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[0]) {
	case kStairsFrontElemType:
		if (squareAspect[kStairsUpAspect]) {
			drawFloorPitOrStairsBitmap(kStairsNativeIndex_Up_Front_D0C_Left, gStairFrames[kFrameStairsUpFront_D0L]);
			drawFloorPitOrStairsBitmapFlippedHorizontally(kStairsNativeIndex_Up_Front_D0C_Left, gStairFrames[kFrameStairsUpFront_D0R]);
		} else {
			drawFloorPitOrStairsBitmap(kStairsNativeIndex_Down_Front_D0C_Left, gStairFrames[kFrameStairsDownFront_D0L]);
			drawFloorPitOrStairsBitmapFlippedHorizontally(kStairsNativeIndex_Down_Front_D0C_Left, gStairFrames[kFrameStairsDownFront_D0R]);
		}
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

	memset(_vm->_dungeonMan->_dungeonViewClickableBoxes, 0, sizeof(_vm->_dungeonMan->_dungeonViewClickableBoxes));
	for (uint16 i = 0; i < 6; ++i) {
		_vm->_dungeonMan->_dungeonViewClickableBoxes[i][0] = 255;
	}

	if (flippedFloorCeiling) {
		uint16 w = gFloorFrame.srcWidth, h = gFloorFrame.srcHeight;
		blitToBitmap(_floorBitmap, w, h, tmpBitmap, w);
		flipBitmapHorizontal(tmpBitmap, w, h);
		drawWallSetBitmap(tmpBitmap, gFloorFrame);
		drawWallSetBitmap(_ceilingBitmap, gCeilingFrame);

		for (uint16 i = 0; i <= kWall_D3LCR - kWall_D0R; ++i)
			_wallSetBitMaps[i + kWall_D0R] = _wallSetBitMaps[i + kWall_D0R_Flipped];
	} else {
		uint16 w = gCeilingFrame.srcWidth, h = gCeilingFrame.srcHeight;
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


Box gBoxWallBitmap_D3LCR = {0, 115, 0, 50}; // @ G0161_s_Graphic558_Box_WallBitmap_D3LCR
Box gBoxWallBitmap_D2LCR = {0, 135, 0, 70}; // @ G0162_s_Graphic558_Box_WallBitmap_D2LCR

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
		uint16 w = width(srcGraphicIndice), h = height(srcGraphicIndice);
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
	loadFloorSet(_vm->_dungeonMan->_currMap.map->floorSet);
	loadWallSet(_vm->_dungeonMan->_currMap.map->wallSet);

	// the original loads some flipped walls here, I moved it to loadWallSet

	for (uint16 i = 0, firstGraphicIndex = _vm->_dungeonMan->_currMap.map->wallSet * kStairsGraphicCount + kFirstStairs; i < kStairsGraphicCount; ++i)
		_stairIndices[i] = firstGraphicIndex + i;

	for (int16 i = 0; i < kAlcoveOrnCount; ++i)
		_currMapAlcoveOrnIndices[i] = -1;
	for (int16 i = 0; i < kFountainOrnCount; ++i)
		_currMapFountainOrnIndices[i] = -1;




	uint16 alcoveCount = 0;
	uint16 fountainCount = 0;
	Map &currMap = *_vm->_dungeonMan->_currMap.map;

	_currMapViAltarIndex = -1;

	for (uint16 i = 0; i < currMap.wallOrnCount; ++i) {
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

		_currMapWallOrnInfo[i][kNativeCoordinateSet] = gWallOrnCoordSetIndices[ornIndice];
	}

	for (uint16 i = 0; i < currMap.floorOrnCount; ++i) {
		uint16 ornIndice = _currMapFloorOrnIndices[i];
		uint16 nativeIndice = kFirstFloorOrn + ornIndice * 6;
		_currMapFloorOrnInfo[i][kNativeBitmapIndex] = nativeIndice;
		_currMapFloorOrnInfo[i][kNativeCoordinateSet] = gFloorOrnCoordSetIndices[ornIndice];
	}

	for (uint16 i = 0; i < currMap.doorOrnCount; ++i) {
		uint16 ornIndice = _currMapDoorOrnIndices[i];
		uint16 nativeIndice = kFirstDoorOrn + ornIndice;
		_currMapDoorOrnInfo[i][kNativeBitmapIndex] = nativeIndice;
		_currMapDoorOrnInfo[i][kNativeCoordinateSet] = gDoorOrnCoordIndices[ornIndice];
	}

	applyCreatureReplColors(9, 8);
	applyCreatureReplColors(10, 12);

	for (uint16 creatureType = 0; creatureType < currMap.creatureTypeCount; ++creatureType) {
		CreatureAspect &aspect = gCreatureAspects[_currMapAllowedCreatureTypes[creatureType]];
		uint16 replColorOrdinal = aspect.getReplColour9();
		if (replColorOrdinal)
			applyCreatureReplColors(9, ordinalToIndex(replColorOrdinal));
		replColorOrdinal = aspect.getReplColour10();
		if (replColorOrdinal)
			applyCreatureReplColors(10, ordinalToIndex(replColorOrdinal));
	}
}

void DisplayMan::applyCreatureReplColors(int replacedColor, int replacementColor) {
	for (int16 i = 0; i < 6; ++i)
		gPalDungeonView[i][replacedColor] = gCreatureReplColorSets[replacementColor].RGBColor[i];

	gPalChangesCreature_D2[replacedColor] = gCreatureReplColorSets[replacementColor].D2ReplacementColor;
	gPalChangesCreature_D3[replacedColor] = gCreatureReplColorSets[replacementColor].D3ReplacementColor;
}

void DisplayMan::drawFloorPitOrStairsBitmap(StairIndex relIndex, Frame &f) {
	if (f.srcWidth) {
		blitToScreen(_bitmaps[_stairIndices[relIndex]], f.srcWidth, f.srcX, f.srcY, f.destFromX, f.destToX, f.destFromY, f.destToY, kColorFlesh, gDungeonViewport);
	}
}

void DisplayMan::drawFloorPitOrStairsBitmapFlippedHorizontally(StairIndex relIndex, Frame &f) {
	if (f.srcWidth) {
		blitToBitmap(_bitmaps[_stairIndices[relIndex]], f.srcWidth, f.srcHeight, _tmpBitmap, f.srcWidth);
		flipBitmapHorizontal(_tmpBitmap, f.srcWidth, f.srcHeight);
		blitToScreen(_tmpBitmap, f.srcWidth, f.srcX, f.srcY, f.destFromX, f.destToX, f.destFromY, f.destToY, kColorFlesh, gDungeonViewport);
	}
}


Box gBoxWallPatchBehindInscription = {110, 113, 37, 63}; // @ G0202_ac_Graphic558_Box_WallPatchBehindInscription 
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

Box gBoxChampionPortraitOnWall = {96, 127, 35, 63}; // G0109_s_Graphic558_Box_ChampionPortraitOnWall

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

		uint16 *coordinateSetA = gWallOrnCoordSets[_currMapWallOrnInfo[wallOrnIndex][kNativeCoordinateSet]][viewWallIndex];
		isAlcove = _vm->_dungeonMan->isWallOrnAnAlcove(wallOrnIndex);
		if (isInscription = (wallOrnIndex == _vm->_dungeonMan->_currMapInscriptionWallOrnIndex)) {
			_vm->_dungeonMan->decodeText((char*)inscriptionString, _inscriptionThing, kTextTypeInscription);
		}

		if (viewWallIndex >= kViewWall_D1L_RIGHT) {
			if (viewWallIndex == kViewWall_D1C_FRONT) {
				if (isInscription) {
					Frame &D1CFrame = gFrameWalls[kViewSquare_D1C];
					blitToScreen(_wallSetBitMaps[kWall_D1LCR], D1CFrame.srcWidth, 94, 28, gBoxWallPatchBehindInscription.X1, gBoxWallPatchBehindInscription.X2,
								 gBoxWallPatchBehindInscription.Y1, gBoxWallPatchBehindInscription.Y2, kColorNoTransparency, gDungeonViewport);

					unsigned char *string = inscriptionString;
					bitmapRed = _bitmaps[kInscriptionFontIndice];
					int16 textLineIndex = 0;
					do {
						int16 characterCount = 0;
						unsigned char *character = string;
						while (*character++ < 0x80) {
							characterCount++;
						}
						frame.destToX = (frame.destFromX = 112 - (characterCount * 4)) + 7;
						frame.destFromY = (frame.destToY = gInscriptionLineY[textLineIndex++]) - 7;
						while (characterCount--) {
							blitToScreen(bitmapRed, 288, (*string++) * 8, 0, frame.destFromX, frame.destToX, frame.destFromY, frame.destToY, kColorFlesh, gDungeonViewport);
							frame.destFromX += 8;
							frame.destToX += 8;
						}
					} while (*string++ != 0x81);
					return isAlcove;
				}
				nativeBitmapIndex++;
				for (uint16 i = 0; i < 4; ++i)
					_vm->_dungeonMan->_dungeonViewClickableBoxes[kViewCellDoorButtonOrWallOrn][i] = coordinateSetA[i];
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
			int16 wallOrnCoordSetIndex = _currMapWallOrnInfo[wallOrnIndex][kNativeCoordinateSet];
			if (flipHorizontal = (viewWallIndex == kViewWall_D2R_LEFT) || (viewWallIndex == kViewWall_D3R_LEFT)) {
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
				if(bitmapGreen != _tmpBitmap)
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
				frame.destFromX = coordinateSetA[0];
				frame.destToX = coordinateSetA[1];
				frame.destFromY = coordinateSetA[2];
				frame.destToY = coordinateSetA[3];
				frame.srcWidth = coordinateSetA[4];
				frame.srcHeight = coordinateSetA[5];

				coordinateSetA = &frame.destFromX;

				coordinateSetA[3] = gUnreadableInscriptionBoxY2[gWallOrnDerivedBitmapIndexIncrement[viewWallIndex] * 3 + unreadableTextLineCount - 1];
			}
		}
		blitToScreen(bitmapGreen, coordinateSetA[4], var_X, 0, coordinateSetA[0], coordinateSetA[1], coordinateSetA[2], coordinateSetA[3], kColorFlesh, gDungeonViewport);

		if ((viewWallIndex == kViewWall_D1C_FRONT) && _championPortraitOrdinal--) {
			Box &box = gBoxChampionPortraitOnWall;
			blitToScreen(_bitmaps[kChampionPortraitsIndice], 256, (_championPortraitOrdinal & 0x7) << 5, (_championPortraitOrdinal >> 3) * 29, box.X1, box.X2, box.Y1, box.Y2,
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
