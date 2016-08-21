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
#include "champion.h"
#include "eventman.h"
#include "lzw.h"

namespace DM {
DisplayMan::DisplayMan(DMEngine *dmEngine) : _vm(dmEngine) {
	_g348_bitmapScreen = nullptr;
	_bitmaps = nullptr;
	_grapItemCount = 0;
	_packedItemPos = nullptr;
	_bitmapCompressedByteCount = nullptr;
	_bitmapDecompressedByteCount = nullptr;
	_packedBitmaps = nullptr;
	_bitmaps = nullptr;
	_g74_tmpBitmap = nullptr;
	_g84_bitmapFloor = nullptr;
	_g85_bitmapCeiling = nullptr;
	_g264_currMapAllowedCreatureTypes = nullptr;
	_g639_derivedBitmapByteCount = nullptr;
	_g638_derivedBitmaps = nullptr;

	_screenWidth = _screenHeight = 0;
	_g289_championPortraitOrdinal = 0;
	_g266_currMapViAltarIndex = 0;
	_g297_drawFloorAndCeilingRequested = true;

	for (int i = 0; i < 4; i++)
		_g75_palChangesProjectile[i] = nullptr;

	for (int i = 0; i < k3_AlcoveOrnCount; i++)
		_g267_currMapAlcoveOrnIndices[i] = 0;

	for (int i = 0; i < k1_FountainOrnCount; i++)
		_g268_currMapFountainOrnIndices[i] = 0;

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 16; j++) {
			_g101_currMapWallOrnInfo[j][i] = 0;
			_g102_currMapFloorOrnInfo[j][i] = 0;
		}

		for (int j = 0; j < 17; j++)
			_g103_currMapDoorOrnInfo[j][i] = 0;
	}

	for (int i = 0; i < 16; i++) {
		_g261_currMapWallOrnIndices[i] = 0;
		_g262_currMapFloorOrnIndices[i] = 0;
	}

	for (int i = 0; i < 18; i++)
		_g263_currMapDoorOrnIndices[i] = 0;

	_g290_inscriptionThing = Thing::_none;
	_g578_useByteBoxCoordinates = false;

	_g85_bitmapCeiling = nullptr;
	_g84_bitmapFloor = nullptr;
	_g697_bitmapWallSet_Wall_D3L2 = nullptr;
	_g696_bitmapWallSet_Wall_D3R2 = nullptr;
	_g698_bitmapWallSet_Wall_D3LCR = nullptr;
	_g699_bitmapWallSet_Wall_D2LCR = nullptr;
	_g700_bitmapWallSet_Wall_D1LCR = nullptr;
	_g701_bitmapWallSet_Wall_D0L = nullptr;
	_g702_bitmapWallSet_Wall_D0R = nullptr;
	_g703_bitmapWallSet_DoorFrameTop_D2LCR = nullptr;
	_g704_bitmapWallSet_DoorFrameTop_D1LCR = nullptr;
	_g705_bitmapWallSet_DoorFrameLeft_D3L = nullptr;
	_g706_bitmapWallSet_DoorFrameLeft_D3C = nullptr;
	_g707_bitmapWallSet_DoorFrameLeft_D2C = nullptr;
	_g708_bitmapWallSet_DoorFrameLeft_D1C = nullptr;
	_g710_bitmapWallSet_DoorFrameRight_D1C = nullptr;
	_g709_bitmapWallSet_DoorFrameFront = nullptr;
	_g296_bitmapViewport = nullptr;

	_g231_currentWallSet = -1;
	_g230_currentFloorSet = -1;

	_g90_bitmapWall_D3LCR_Flipped = nullptr;
	_g91_bitmapWall_D2LCR_Flipped = nullptr;
	_g92_bitmapWall_D1LCR_Flipped = nullptr;
	_g93_bitmapWall_D0L_Flipped = nullptr;
	_g94_bitmapWall_D0R_Flipped = nullptr;
	_g95_bitmapWall_D3LCR_Native = nullptr;
	_g96_bitmapWall_D2LCR_Native = nullptr;
	_g97_bitmapWall_D1LCR_Native = nullptr;
	_g98_bitmapWall_D0L_Native = nullptr;
	_g99_bitmapWall_D0R_Native = nullptr;

	_g322_paletteSwitchingEnabled = false;
	_g304_dungeonViewPaletteIndex = 0;

	for (uint16 i = 0; i < 16; ++i) {
		_g347_paletteTopAndBottomScreen[i] = 0;
		_g346_paletteMiddleScreen[i] = 0;
	}

	for (uint16 i = 0; i < 32; i++)
		_g345_aui_BlankBuffer[i] = 0;

	_gK17_paletteFadeFrom = nullptr;
	for (uint16 i = 0; i < 16; ++i)
		_gK16_paletteFadeTemporary[i] = 0;

	initConstants();
}

void DisplayMan::initConstants() {
	const byte palChangesDoorButtonAndWallOrnD3[16] = {0, 0, 120, 30, 40, 30, 0, 60, 30, 90, 100, 110, 0, 10, 0, 20}; // @ G0198_auc_Graphic558_PaletteChanges_DoorButtonAndWallOrnament_D3
	const byte palChangesDoorButtonAndWallOrnD2[16] = {0, 120, 10, 30, 40, 30, 60, 70, 50, 90, 100, 110, 0, 20, 140, 130}; // @ G0199_auc_Graphic558_PaletteChanges_DoorButtonAndWallOrnament_D2
	const FieldAspect fieldAspects188[12] = { // @ G0188_as_Graphic558_FieldAspects
	/* { NativeBitmapRelativeIndex, BaseStartUnitIndex, Transparent color, Mask, ByteWidth, Height, X, BitPlaneWordCount } */
		FieldAspect(0, 63, 0x8A, 0xFF,  0,   0,  0, 64),  /* D3C */
		FieldAspect(0, 63, 0x0A, 0x80, 48,  51, 11, 64),  /* D3L */
		FieldAspect(0, 63, 0x0A, 0x00, 48,  51,  0, 64),  /* D3R */
		FieldAspect(0, 60, 0x8A, 0xFF,  0,   0,  0, 64),  /* D2C */
		FieldAspect(0, 63, 0x0A, 0x81, 40,  71,  5, 64),  /* D2L */
		FieldAspect(0, 63, 0x0A, 0x01, 40,  71,  0, 64),  /* D2R */
		FieldAspect(0, 61, 0x8A, 0xFF,  0,   0,  0, 64),  /* D1C */
		FieldAspect(0, 63, 0x0A, 0x82, 32, 111,  0, 64),  /* D1L */
		FieldAspect(0, 63, 0x0A, 0x02, 32, 111,  0, 64),  /* D1R */
		FieldAspect(0, 59, 0x8A, 0xFF,  0,   0,  0, 64),  /* D0C */
		FieldAspect(0, 63, 0x0A, 0x83, 16, 136,  0, 64),  /* D0L */
		FieldAspect(0, 63, 0x0A, 0x03, 16, 136,  0, 64)   /* D0R */
	};

	const ExplosionAspect explosionAspects[k4_ExplosionAspectCount] = { // @ G0211_as_Graphic558_ExplosionAspects
		// ByteWidth, Height 
		ExplosionAspect(80, 111),   // Fire
		ExplosionAspect(64,  97),   // Spell
		ExplosionAspect(80,  91),   // Poison
		ExplosionAspect(80,  91)    // Death
	};

	const byte palChangeSmoke[16] = {0, 10, 20, 30, 40, 50, 120, 10, 80, 90, 100, 110, 120, 130, 140, 150}; // @ G0212_auc_Graphic558_PaletteChanges_Smoke
	const byte projectileScales[7] = {
		13,   /* D4 Back  */
		16,   /* D4 Front */
		19,   /* D3 Back  */
		22,   /* D3 Front */
		25,   /* D2 Back  */
		28,   /* D2 Front */
		32    /* D1 Back  */
	};

	const Frame frameWalls163[12] = { // @ G0163_as_Graphic558_Frame_Walls
		/* { X1, X2, Y1, Y2, pixelWidth, Height, X, Y } */
		Frame(74, 149, 25,  75,  64,  51,  18, 0),  /* D3C */
		Frame(0,  83, 25,  75,  64,  51,  32, 0),   /* D3L */
		Frame(139, 223, 25,  75,  64,  51,   0, 0), /* D3R */
		Frame(60, 163, 20,  90,  72,  71,  16, 0),  /* D2C */
		Frame(0,  74, 20,  90,  72,  71,  61, 0),   /* D2L */
		Frame(149, 223, 20,  90,  72,  71,   0, 0), /* D2R */
		Frame(32, 191,  9, 119, 128, 111,  48, 0),  /* D1C */
		Frame(0,  63,  9, 119, 128, 111, 192, 0),   /* D1L */
		Frame(160, 223,  9, 119, 128, 111,   0, 0), /* D1R */
		Frame(0, 223,  0, 135,   0,   0,   0, 0),   /* D0C */
		Frame(0,  31,  0, 135,  16, 136,   0, 0),   /* D0L */
		Frame(192, 223,  0, 135,  16, 136,   0, 0)  /* D0R */
	};

	const CreatureAspect creatureAspects219[k27_CreatureTypeCount] = { // @ G0219_as_Graphic558_CreatureAspects
	/* { FirstNativeBitmapRelativeIndex, FirstDerivedBitmapIndex, pixelWidthFront, HeightFront,
		 pixelWidthSide, HeightSide, pixelWidthAttack, HeightAttack, CoordinateSet / TransparentColor,
		 Replacement Color Set Index for color 10 / Replacement Color Set Index for color 9 } */
		CreatureAspect(0, 0, 56 ,  84, 56 ,  84, 56 ,  84, 0x1D, 0x01),    /* Creature #00 Giant Scorpion / Scorpion */
		CreatureAspect(4, 0, 32 ,  66,  0 ,   0, 32 ,  69, 0x0B, 0x20),    /* Creature #01 Swamp Slime / Slime Devil */
		CreatureAspect(6, 0, 24 ,  48, 24 ,  48,  0 ,   0, 0x0B, 0x00),    /* Creature #02 Giggler */
		CreatureAspect(10, 0, 32 ,  61,  0 ,   0, 32 ,  61, 0x24, 0x31),   /* Creature #03 Wizard Eye / Flying Eye */
		CreatureAspect(12, 0, 32 ,  64, 56 ,  64, 32 ,  64, 0x14, 0x34),   /* Creature #04 Pain Rat / Hellhound */
		CreatureAspect(16, 0, 24 ,  49, 40 ,  49,  0 ,   0, 0x18, 0x34),   /* Creature #05 Ruster */
		CreatureAspect(19, 0, 32 ,  60,  0 ,   0, 32 ,  60, 0x0D, 0x00),   /* Creature #06 Screamer */
		CreatureAspect(21, 0, 32 ,  43,  0 ,   0, 32 ,  64, 0x04, 0x00),   /* Creature #07 Rockpile / Rock pile */
		CreatureAspect(23, 0, 32 ,  83,  0 ,   0, 32 ,  93, 0x04, 0x00),   /* Creature #08 Ghost / Rive */
		CreatureAspect(25, 0, 32 , 101, 32 , 101, 32 , 101, 0x14, 0x00),   /* Creature #09 Stone Golem */
		CreatureAspect(29, 0, 32 ,  82, 32 ,  82, 32 ,  83, 0x04, 0x00),   /* Creature #10 Mummy */
		CreatureAspect(33, 0, 32 ,  80,  0 ,   0, 32 ,  99, 0x14, 0x00),   /* Creature #11 Black Flame */
		CreatureAspect(35, 0, 32 ,  80, 32 ,  80, 32 ,  76, 0x04, 0x00),   /* Creature #12 Skeleton */
		CreatureAspect(39, 0, 32 ,  96, 56 ,  93, 32 ,  90, 0x1D, 0x20),   /* Creature #13 Couatl */
		CreatureAspect(43, 0, 32 ,  49, 16 ,  49, 32 ,  56, 0x04, 0x30),   /* Creature #14 Vexirk */
		CreatureAspect(47, 0, 32 ,  59, 56 ,  43, 32 ,  67, 0x14, 0x78),   /* Creature #15 Magenta Worm / Worm */
		CreatureAspect(51, 0, 32 ,  83, 32 ,  74, 32 ,  74, 0x04, 0x65),   /* Creature #16 Trolin / Ant Man */
		CreatureAspect(55, 0, 24 ,  49, 24 ,  53, 24 ,  53, 0x24, 0x00),   /* Creature #17 Giant Wasp / Muncher */
		CreatureAspect(59, 0, 32 ,  89, 32 ,  89, 32 ,  89, 0x04, 0x00),   /* Creature #18 Animated Armour / Deth Knight */
		CreatureAspect(63, 0, 32 ,  84, 32 ,  84, 32 ,  84, 0x0D, 0xA9),   /* Creature #19 Materializer / Zytaz */
		CreatureAspect(67, 0, 56 ,  27,  0 ,   0, 56 ,  80, 0x04, 0x65),   /* Creature #20 Water Elemental */
		CreatureAspect(69, 0, 56 ,  77, 56 ,  81, 56 ,  77, 0x04, 0xA9),   /* Creature #21 Oitu */
		CreatureAspect(73, 0, 32 ,  87, 32 ,  89, 32 ,  89, 0x04, 0xCB),   /* Creature #22 Demon */
		CreatureAspect(77, 0, 32 ,  96, 32 ,  94, 32 ,  96, 0x04, 0x00),   /* Creature #23 Lord Chaos */
		CreatureAspect(81, 0, 64 ,  94, 72 ,  94, 64 ,  94, 0x04, 0xCB),   /* Creature #24 Red Dragon / Dragon */
		CreatureAspect(85, 0, 32 ,  93,  0 ,   0,  0 ,   0, 0x04, 0xCB),   /* Creature #25 Lord Order */
		CreatureAspect(86, 0, 32 ,  93,  0 ,   0,  0 ,   0, 0x04, 0xCB)    /* Creature #26 Grey Lord */
	};
	static ObjectAspect objectAspects209[k85_ObjAspectCount] = { // @ G0209_as_Graphic558_ObjectAspects
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

	static ProjectileAspect projectileAspect[k14_ProjectileAspectCount] = { // @ G0210_as_Graphic558_ProjectileAspects
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

	/* Atari ST: { 0x003, 0x055, 0x773, 0x420, 0x774, 0x000, 0x040, 0x500, 0x642, 0x775, 0x742, 0x760, 0x750, 0x000, 0x310, 0x776 }, RGB colors are different */
	static uint16 palCredits[16] = {0x006, 0x0AA, 0xFF6, 0x840, 0xFF8, 0x000, 0x080, 0xA00, 0xC84, 0xFFA, 0xF84, 0xFC0, 0xFA0, 0x000, 0x620, 0xFFC}; // @ G0019_aui_Graphic562_Palette_Credits
	static uint16 palDungeonView[6][16] = { // @ G0021_aaui_Graphic562_Palette_DungeonView
		/* Atari ST: { 0x000, 0x333, 0x444, 0x310, 0x066, 0x420, 0x040, 0x060, 0x700, 0x750, 0x643, 0x770, 0x222, 0x555, 0x007, 0x777 }, RGB colors are different */
		{ 0x000, 0x666, 0x888, 0x620, 0x0CC, 0x840, 0x080, 0x0C0, 0xF00, 0xFA0, 0xC86, 0xFF0, 0x444, 0xAAA, 0x00F, 0xFFF },
		/* Atari ST: { 0x000, 0x222, 0x333, 0x310, 0x066, 0x410, 0x030, 0x050, 0x600, 0x640, 0x532, 0x760, 0x111, 0x444, 0x006, 0x666 }, RGB colors are different */
		{ 0x000, 0x444, 0x666, 0x620, 0x0CC, 0x820, 0x060, 0x0A0, 0xC00, 0x000, 0x000, 0xFC0, 0x222, 0x888, 0x00C, 0xCCC },
		/* Atari ST: { 0x000, 0x111, 0x222, 0x210, 0x066, 0x310, 0x020, 0x040, 0x500, 0x530, 0x421, 0x750, 0x000, 0x333, 0x005, 0x555 }, RGB colors are different */
		{ 0x000, 0x222, 0x444, 0x420, 0x0CC, 0x620, 0x040, 0x080, 0xA00, 0x000, 0x000, 0xFA0, 0x000, 0x666, 0x00A, 0xAAA },
		/* Atari ST: { 0x000, 0x000, 0x111, 0x100, 0x066, 0x210, 0x010, 0x030, 0x400, 0x420, 0x310, 0x640, 0x000, 0x222, 0x004, 0x444 }, RGB colors are different */
		{ 0x000, 0x000, 0x222, 0x200, 0x0CC, 0x420, 0x020, 0x060, 0x800, 0x000, 0x000, 0xC80, 0x000, 0x444, 0x008, 0x888 },
		/* Atari ST: { 0x000, 0x000, 0x000, 0x000, 0x066, 0x100, 0x000, 0x020, 0x300, 0x310, 0x200, 0x530, 0x000, 0x111, 0x003, 0x333 }, RGB colors are different */
		{ 0x000, 0x000, 0x000, 0x000, 0x0CC, 0x200, 0x000, 0x040, 0x600, 0x000, 0x000, 0xA60, 0x000, 0x222, 0x006, 0x666 },
		/* Atari ST: { 0x000, 0x000, 0x000, 0x000, 0x066, 0x000, 0x000, 0x010, 0x200, 0x200, 0x100, 0x320, 0x000, 0x000, 0x002, 0x222 }, RGB colors are different */
		{ 0x000, 0x000, 0x000, 0x000, 0x0CC, 0x000, 0x000, 0x020, 0x400, 0x000, 0x000, 0x640, 0x000, 0x000, 0x004, 0x444 }
	};

	static byte palChangesCreatureD3[16] = {0, 120, 10, 30, 40, 30, 0, 60, 30, 0, 0, 110, 0, 20, 0, 130}; // @ G0221_auc_Graphic558_PaletteChanges_Creature_D3
	static byte palChangesCreatureD2[16] = {0, 10, 20, 30, 40, 30, 60, 70, 50, 0, 0, 110, 120, 130, 140, 150}; // @ G0222_auc_Graphic558_PaletteChanges_Creature_D2
	static byte palChangesNoChanges[16] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150}; // @ G0017_auc_Graphic562_PaletteChanges_NoChanges
	static byte palChangesFloorOrnD3[16] = {0, 120, 10, 30, 40, 30, 0, 60, 30, 90, 100, 110, 0, 20, 140, 130}; // @ G0213_auc_Graphic558_PaletteChanges_FloorOrnament_D3
	static byte palChangesFloorOrnD2[16] = {0, 10, 20, 30, 40, 30, 60, 70, 50, 90, 100, 110, 120, 130, 140, 150}; // @ G0214_auc_Graphic558_PaletteChanges_FloorOrnament_D2

	_frameWallD3R2 = Frame(208, 223, 25, 73, 8, 49, 0, 0); // @ G0712_s_Graphic558_Frame_Wall_D3R2

	_doorFrameLeftD1C = Frame(43, 74, 14, 107, 16, 94, 0, 0); // @ G0170_s_Graphic558_Frame_DoorFrameLeft_D1C
	_doorFrameRightD1C = Frame(149, 180, 14, 107, 16, 94, 0, 0); // @ G0171_s_Graphic558_Frame_DoorFrameRight_D1C

	for (int i = 0; i < 16; i++) {
		_palChangesDoorButtonAndWallOrnD3[i] = palChangesDoorButtonAndWallOrnD3[i];
		_palChangesDoorButtonAndWallOrnD2[i] = palChangesDoorButtonAndWallOrnD2[i];
		_palChangeSmoke[i] = palChangeSmoke[i];
		_palCredits[i] = palCredits[i];
		_palChangesCreatureD3[i] = palChangesCreatureD3[i];
		_palChangesCreatureD2[i] = palChangesCreatureD2[i];
		_palChangesNoChanges[i] = palChangesNoChanges[i];
		_palChangesFloorOrnD3[i] = palChangesFloorOrnD3[i];
		_palChangesFloorOrnD2[i] = palChangesFloorOrnD2[i];
		for (int j = 0; j < 6; j++)
			_palDungeonView[j][i] = palDungeonView[j][i];
	}

	for (int i = 0; i < 12; i++) {
		_fieldAspects188[i] = fieldAspects188[i];
		_frameWalls163[i] = frameWalls163[i];
	}

	for (int i = 0; i < 7; i++)
		_projectileScales[i] = projectileScales[i];

	for (int i = 0; i < k4_ExplosionAspectCount; i++)
		_explosionAspects[i] = explosionAspects[i];

	for (int i = 0; i < k27_CreatureTypeCount; i++)
		_creatureAspects219[i] = creatureAspects219[i];

	for (int i = 0; i < k85_ObjAspectCount; i++)
		_objectAspects209[i] = objectAspects209[i];

	for (int i = 0; i < k14_ProjectileAspectCount; i++)
		_projectileAspect[i] = projectileAspect[i];

	_doorFrameD1C = new DoorFrames( // @ G0186_s_Graphic558_Frames_Door_D1C
	   Frame(64, 159, 17, 102, 48, 88, 0, 0),	 /* Closed Or Destroyed */
	   Frame(64, 159, 17, 38, 48, 88, 0, 66),	 /* Vertical Closed one fourth */
	   Frame(64, 159, 17, 60, 48, 88, 0, 44),	 /* Vertical Closed half */
	   Frame(64, 159, 17, 82, 48, 88, 0, 22),	 /* Vertical Closed three fourth */
	   Frame(64, 75, 17, 102, 48, 88, 36, 0),	 /* Left Horizontal Closed one fourth */
	   Frame(64, 87, 17, 102, 48, 88, 24, 0),	 /* Left Horizontal Closed half */
	   Frame(64, 99, 17, 102, 48, 88, 12, 0),	 /* Left Horizontal Closed three fourth */
	   Frame(148, 159, 17, 102, 48, 88, 48, 0), /* Right Horizontal Closed one fourth */
	   Frame(136, 159, 17, 102, 48, 88, 48, 0), /* Right Horizontal Closed half */
	   Frame(124, 159, 17, 102, 48, 88, 48, 0)	 /* Right Horizontal Closed three fourth */
	);

	_boxThievesEyeViewPortVisibleArea = Box(64, 159, 19, 113); // @ G0106_s_Graphic558_Box_ThievesEye_ViewportVisibleArea 
	_boxMovementArrows = Box(224, 319, 124, 168); // @ G0002_s_Graphic562_Box_MovementArrows
}

DisplayMan::~DisplayMan() {
	delete[] _packedItemPos;
	delete[] _packedBitmaps;
	delete[] _g348_bitmapScreen;
	if (_bitmaps) {
		delete[] _bitmaps[0];
		delete[] _bitmaps;
	}
	delete[] _bitmapCompressedByteCount;
	delete[] _bitmapDecompressedByteCount;

	delete[] _g639_derivedBitmapByteCount;
	if (_g638_derivedBitmaps) {
		for (uint16 i = 0; i < k730_DerivedBitmapMaximumCount; ++i)
			delete[] _g638_derivedBitmaps[i];
		delete[] _g638_derivedBitmaps;
	}

	delete[] _g85_bitmapCeiling;
	delete[] _g84_bitmapFloor;
	delete[] _g697_bitmapWallSet_Wall_D3L2;
	delete[] _g696_bitmapWallSet_Wall_D3R2;
	delete[] _g698_bitmapWallSet_Wall_D3LCR;
	delete[] _g699_bitmapWallSet_Wall_D2LCR;
	delete[] _g700_bitmapWallSet_Wall_D1LCR;
	delete[] _g701_bitmapWallSet_Wall_D0L;
	delete[] _g702_bitmapWallSet_Wall_D0R;
	delete[] _g703_bitmapWallSet_DoorFrameTop_D2LCR;
	delete[] _g704_bitmapWallSet_DoorFrameTop_D1LCR;
	delete[] _g705_bitmapWallSet_DoorFrameLeft_D3L;
	delete[] _g706_bitmapWallSet_DoorFrameLeft_D3C;
	delete[] _g707_bitmapWallSet_DoorFrameLeft_D2C;
	delete[] _g708_bitmapWallSet_DoorFrameLeft_D1C;
	delete[] _g710_bitmapWallSet_DoorFrameRight_D1C;
	delete[] _g709_bitmapWallSet_DoorFrameFront;
	delete[] _g296_bitmapViewport;

	delete[] _g90_bitmapWall_D3LCR_Flipped;
	delete[] _g91_bitmapWall_D2LCR_Flipped;
	delete[] _g92_bitmapWall_D1LCR_Flipped;
	delete[] _g93_bitmapWall_D0L_Flipped;
	delete[] _g94_bitmapWall_D0R_Flipped;

	delete _doorFrameD1C;
}

void DisplayMan::setUpScreens(uint16 width, uint16 height) {
	_screenWidth = width;
	_screenHeight = height;
	delete[] _g74_tmpBitmap;
	delete[] _g348_bitmapScreen;
	_g348_bitmapScreen = new byte[_screenWidth * _screenHeight];
	fillScreen(k0_ColorBlack);

	_g74_tmpBitmap = new byte[_screenWidth * _screenHeight];
}


void DisplayMan::f460_initializeGraphicData() {
	_g85_bitmapCeiling = new byte[224 * 29];
	_g84_bitmapFloor = new byte[224 * 70];
	_g697_bitmapWallSet_Wall_D3L2 = new byte[16 * 49];
	_g696_bitmapWallSet_Wall_D3R2 = new byte[16 * 49];
	_g698_bitmapWallSet_Wall_D3LCR = new byte[128 * 51];
	_g699_bitmapWallSet_Wall_D2LCR = new byte[144 * 71];
	_g700_bitmapWallSet_Wall_D1LCR = new byte[256 * 111];
	_g701_bitmapWallSet_Wall_D0L = new byte[32 * 136];
	_g702_bitmapWallSet_Wall_D0R = new byte[32 * 136];
	_g703_bitmapWallSet_DoorFrameTop_D2LCR = new byte[96 * 3];
	_g704_bitmapWallSet_DoorFrameTop_D1LCR = new byte[128 * 4];
	_g705_bitmapWallSet_DoorFrameLeft_D3L = new byte[32 * 44];
	_g706_bitmapWallSet_DoorFrameLeft_D3C = new byte[32 * 44];
	_g707_bitmapWallSet_DoorFrameLeft_D2C = new byte[48 * 65];
	_g708_bitmapWallSet_DoorFrameLeft_D1C = new byte[32 * 94];
	_g710_bitmapWallSet_DoorFrameRight_D1C = new byte[32 * 94];
	_g709_bitmapWallSet_DoorFrameFront = new byte[32 * 123];
	_g296_bitmapViewport = new byte[224 * 136];

	if (!_g639_derivedBitmapByteCount)
		_g639_derivedBitmapByteCount = new uint16[k730_DerivedBitmapMaximumCount];
	if (!_g638_derivedBitmaps) {
		_g638_derivedBitmaps = new byte *[k730_DerivedBitmapMaximumCount];
		for (uint16 i = 0; i < k730_DerivedBitmapMaximumCount; ++i)
			_g638_derivedBitmaps[i] = nullptr;
	}

	_g639_derivedBitmapByteCount[k0_DerivedBitmapViewport] = 112 * 136;
	_g639_derivedBitmapByteCount[k1_DerivedBitmapThievesEyeVisibleArea] = 48 * 95;
	_g639_derivedBitmapByteCount[k2_DerivedBitmapDamageToCreatureMedium] = 32 * 37;
	_g639_derivedBitmapByteCount[k3_DerivedBitmapDamageToCreatureSmall] = 24 * 37;

	for (int16 doorOrnamentIndex = k15_DoorOrnDestroyedMask; doorOrnamentIndex <= k16_DoorOrnThivesEyeMask; doorOrnamentIndex++) {
		_g103_currMapDoorOrnInfo[doorOrnamentIndex][k0_NativeBitmapIndex] = doorOrnamentIndex + (k301_DoorMaskDestroyedIndice - k15_DoorOrnDestroyedMask);
		_g103_currMapDoorOrnInfo[doorOrnamentIndex][k1_CoordinateSet] = 1;

		_g639_derivedBitmapByteCount[doorOrnamentIndex * 2 + k68_DerivedBitmapFirstDoorOrnament_D3] = 24 * 41;
		_g639_derivedBitmapByteCount[doorOrnamentIndex * 2 + k69_DerivedBitmapFirstDoorOrnament_D2] = 32 * 61;
	}

	_g102_currMapFloorOrnInfo[k15_FloorOrnFootprints][k0_NativeBitmapIndex] = k241_FloorOrn_15_D3L_footprints;
	_g102_currMapFloorOrnInfo[k15_FloorOrnFootprints][k1_CoordinateSet] = 1;

	ObjectAspect *objectAspect = _objectAspects209;
	int16 derivedBitmapIndex;
	for (int16 objectAspectIndex = 0; objectAspectIndex < k85_ObjAspectCount; ++objectAspectIndex, ++objectAspect) {
		derivedBitmapIndex = k104_DerivedBitmapFirstObject + objectAspect->_firstDerivedBitmapRelativeIndex;

		_g639_derivedBitmapByteCount[derivedBitmapIndex++] = f459_getScaledBitmapByteCount(objectAspect->_byteWidth, objectAspect->_height, k16_Scale_D3);
		_g639_derivedBitmapByteCount[derivedBitmapIndex++] = f459_getScaledBitmapByteCount(objectAspect->_byteWidth, objectAspect->_height, k20_Scale_D2);

		if (getFlag(objectAspect->_graphicInfo, k0x0001_ObjectFlipOnRightMask)) {
			_g639_derivedBitmapByteCount[derivedBitmapIndex] = _g639_derivedBitmapByteCount[derivedBitmapIndex - 2];
			derivedBitmapIndex++;
			_g639_derivedBitmapByteCount[derivedBitmapIndex] = _g639_derivedBitmapByteCount[derivedBitmapIndex - 2];
			derivedBitmapIndex++;
		}

		if (getFlag(objectAspect->_graphicInfo, k0x0010_ObjectAlcoveMask)) {
			_g639_derivedBitmapByteCount[derivedBitmapIndex] = _g639_derivedBitmapByteCount[derivedBitmapIndex - 2];
			derivedBitmapIndex++;
			_g639_derivedBitmapByteCount[derivedBitmapIndex] = _g639_derivedBitmapByteCount[derivedBitmapIndex - 2];
		}
	}

	ProjectileAspect *projectileAspect = _projectileAspect;
	for (int16 projectileAspectIndex = 0; projectileAspectIndex < k14_ProjectileAspectCount; projectileAspectIndex++, projectileAspect++) {
		if (!getFlag(projectileAspect->_graphicInfo, k0x0100_ProjectileScaleWithKineticEnergyMask)) {
			derivedBitmapIndex = k282_DerivedBitmapFirstProjectile + projectileAspect->_firstDerivedBitmapRelativeIndex;

			for (int16 projectileScaleIndex = 0; projectileScaleIndex < 6; projectileScaleIndex++) {
				int16 bitmapByteCount = f459_getScaledBitmapByteCount(projectileAspect->_byteWidth, projectileAspect->_height, _projectileScales[projectileScaleIndex]);
				_g639_derivedBitmapByteCount[derivedBitmapIndex] = bitmapByteCount;

				if (getFlag(projectileAspect->_graphicInfo, k0x0003_ProjectileAspectTypeMask) != k3_ProjectileAspectHasNone) {
					_g639_derivedBitmapByteCount[derivedBitmapIndex + 6] = bitmapByteCount;

					if (getFlag(projectileAspect->_graphicInfo, k0x0003_ProjectileAspectTypeMask) != k2_ProjectileAspectHasRotation)
						_g639_derivedBitmapByteCount[derivedBitmapIndex + 12] = bitmapByteCount;
				}
			}
		}
	}

	_g75_palChangesProjectile[0] = _palChangesFloorOrnD3;
	_g75_palChangesProjectile[1] = _palChangesFloorOrnD2;
	_g75_palChangesProjectile[2] = _g75_palChangesProjectile[3] = _palChangesNoChanges;

	derivedBitmapIndex = k438_DerivedBitmapFirstExplosion;
	ExplosionAspect *expAsp = _explosionAspects;
	for (uint16 expAspIndex = 0; expAspIndex < k4_ExplosionAspectCount; ++expAspIndex, expAsp++) {
		for (int16 scale = 4; scale < 32; scale += 2)
			_g639_derivedBitmapByteCount[derivedBitmapIndex++] = f459_getScaledBitmapByteCount(expAsp->_byteWidth, expAsp->_height, scale);

		if (expAspIndex == k3_ExplosionAspectSmoke)
			_g639_derivedBitmapByteCount[derivedBitmapIndex++] = expAsp->_byteWidth * expAsp->_height;
	}

	derivedBitmapIndex = k495_DerivedBitmapFirstCreature;
	CreatureAspect *creatureAsp;
	for (int16 creatureIndex = 0; creatureIndex < k27_CreatureTypeCount; creatureIndex++) {
		creatureAsp = &_creatureAspects219[creatureIndex];

		int16 creatureGraphicInfo = g243_CreatureInfo[creatureIndex]._graphicInfo;
		creatureAsp->_firstDerivedBitmapIndex = derivedBitmapIndex;

		int16 creatureFrontBitmapD3PixelCount = f459_getScaledBitmapByteCount(creatureAsp->_byteWidthFront, creatureAsp->_heightFront, k16_Scale_D3);
		_g639_derivedBitmapByteCount[derivedBitmapIndex++] = creatureFrontBitmapD3PixelCount;

		int16 creatureFrontBitmapD2PixelCount = f459_getScaledBitmapByteCount(creatureAsp->_byteWidthFront, creatureAsp->_heightFront, k20_Scale_D2);
		_g639_derivedBitmapByteCount[derivedBitmapIndex++] = creatureFrontBitmapD2PixelCount;

		if (getFlag(creatureGraphicInfo, k0x0008_CreatureInfoGraphicMaskSide)) {
			_g639_derivedBitmapByteCount[derivedBitmapIndex++] = f459_getScaledBitmapByteCount(creatureAsp->_byteWidthSide, creatureAsp->_heightSide, k16_Scale_D3);
			_g639_derivedBitmapByteCount[derivedBitmapIndex++] = f459_getScaledBitmapByteCount(creatureAsp->_byteWidthSide, creatureAsp->_heightSide, k20_Scale_D2);
		}

		if (getFlag(creatureGraphicInfo, k0x0010_CreatureInfoGraphicMaskBack)) {
			_g639_derivedBitmapByteCount[derivedBitmapIndex++] = creatureFrontBitmapD3PixelCount;
			_g639_derivedBitmapByteCount[derivedBitmapIndex++] = creatureFrontBitmapD2PixelCount;
		}

		if (getFlag(creatureGraphicInfo, k0x0020_CreatureInfoGraphicMaskAttack)) {
			_g639_derivedBitmapByteCount[derivedBitmapIndex++] = f459_getScaledBitmapByteCount(creatureAsp->_byteWidthAttack, creatureAsp->_heightAttack, k16_Scale_D3);
			_g639_derivedBitmapByteCount[derivedBitmapIndex++] = f459_getScaledBitmapByteCount(creatureAsp->_byteWidthAttack, creatureAsp->_heightAttack, k20_Scale_D2);
		}

		int16 additionalFronGraphicCount = getFlag(creatureGraphicInfo, k0x0003_CreatureInfoGraphicMaskAdditional);
		if (additionalFronGraphicCount) {
			do {
				_g639_derivedBitmapByteCount[derivedBitmapIndex++] = creatureAsp->_byteWidthFront * creatureAsp->_heightFront;
				_g639_derivedBitmapByteCount[derivedBitmapIndex++] = f459_getScaledBitmapByteCount(creatureAsp->_byteWidthFront, creatureAsp->_heightFront, k16_Scale_D3);
				_g639_derivedBitmapByteCount[derivedBitmapIndex++] = f459_getScaledBitmapByteCount(creatureAsp->_byteWidthFront, creatureAsp->_heightFront, k20_Scale_D2);
			} while (--additionalFronGraphicCount);
		}
	}
}

void DisplayMan::f479_loadGraphics() {
	Common::File f;
	f.open("graphics.dat");
	_grapItemCount = f.readUint16BE();

	delete[] _bitmapCompressedByteCount;
	_bitmapCompressedByteCount = new uint32[_grapItemCount];
	for (uint16 i = 0; i < _grapItemCount; ++i)
		_bitmapCompressedByteCount[i] = f.readUint16BE();

	delete[] _bitmapDecompressedByteCount;
	_bitmapDecompressedByteCount = new uint32[_grapItemCount];
	for (uint16 i = 0; i < _grapItemCount; ++i)
		_bitmapDecompressedByteCount[i] = f.readUint16BE();

	delete[] _packedItemPos;
	_packedItemPos = new uint32[_grapItemCount + 1];
	_packedItemPos[0] = 0;
	for (uint16 i = 1; i < _grapItemCount + 1; ++i) {
		_packedItemPos[i] = _packedItemPos[i - 1] + _bitmapDecompressedByteCount[i - 1];
	}

	delete[] _packedBitmaps;
	_packedBitmaps = new uint8[_packedItemPos[_grapItemCount]];

	LZWdecompressor lzw;
	Common::Array<byte> tmpBuffer;
	f.seek(2 + _grapItemCount * 4);
	for (uint32 i = 0; i < _grapItemCount; ++i) {
		byte *bitmap = _packedBitmaps + _packedItemPos[i];
		f.read(bitmap, _bitmapCompressedByteCount[i]);
		if (_bitmapCompressedByteCount[i] != _bitmapDecompressedByteCount[i]) {
			tmpBuffer.reserve(_bitmapDecompressedByteCount[i]);
			Common::MemoryReadStream stream(bitmap, _bitmapCompressedByteCount[i]);
			lzw.decompress(stream, _bitmapCompressedByteCount[i], tmpBuffer.begin());
			memcpy(bitmap, tmpBuffer.begin(), _bitmapDecompressedByteCount[i]);
		}
	}

	f.close();
	unpackGraphics();
}

void DisplayMan::unpackGraphics() {
	uint32 unpackedBitmapsSize = 0;
	for (uint16 i = 0; i <= 20; ++i)
		unpackedBitmapsSize += getPixelWidth(i) * getPixelHeight(i);
	for (uint16 i = 22; i <= 532; ++i)
		unpackedBitmapsSize += getPixelWidth(i) * getPixelHeight(i);
	unpackedBitmapsSize += (5 + 1) * 6 * 128; // 5 x 6 characters, 128 of them, +1 for convenience padding
	// graphics items go from 0-20 and 22-532 inclusive, _unpackedItemPos 21 and 22 are there for indexing convenience
	if (_bitmaps) {
		delete[] _bitmaps[0];
		delete[] _bitmaps;
	}
	_bitmaps = new byte *[575]; // largest graphic indice (i think)
	_bitmaps[0] = new byte[unpackedBitmapsSize];
	f466_loadIntoBitmap(0, _bitmaps[0]);
	for (uint16 i = 1; i <= 20; ++i) {
		_bitmaps[i] = _bitmaps[i - 1] + getPixelWidth(i - 1) * getPixelHeight(i - 1);
		f466_loadIntoBitmap(i, _bitmaps[i]);
	}
	_bitmaps[22] = _bitmaps[20] + getPixelWidth(20) * getPixelHeight(20);
	for (uint16 i = 23; i <= 532; ++i) {
		_bitmaps[i] = _bitmaps[i - 1] + getPixelWidth(i - 1) * getPixelHeight(i - 1);
		f466_loadIntoBitmap(i, _bitmaps[i]);
	}
	_bitmaps[k557_FontGraphicIndice] = _bitmaps[532] + getPixelWidth(532) * getPixelHeight(532);
	loadFNT1intoBitmap(k557_FontGraphicIndice, _bitmaps[k557_FontGraphicIndice]);
}

void DisplayMan::loadFNT1intoBitmap(uint16 index, byte * destBitmap) {
	uint8 *data = _packedBitmaps + _packedItemPos[index];

	for (uint16 i = 0; i < 6; i++) {
		for (uint16 w = 0; w < 128; ++w) {
			*destBitmap++ = k0_ColorBlack;

			uint16 nextByte = *data++;
			for (int16 pixel = 4; pixel >= 0; --pixel) {
				*destBitmap++ = (nextByte >> pixel) & 0x1;
			}
		}
	}
}

void DisplayMan::f461_allocateFlippedWallBitmaps() {
	_g90_bitmapWall_D3LCR_Flipped = new byte[128 * 51];
	_g91_bitmapWall_D2LCR_Flipped = new byte[144 * 71];
	_g92_bitmapWall_D1LCR_Flipped = new byte[256 * 111];
	_g93_bitmapWall_D0L_Flipped = new byte[32 * 136];
	_g94_bitmapWall_D0R_Flipped = new byte[32 * 136];
}

void DisplayMan::f102_drawDoorBitmap(Frame* frame) {
	if (frame->_srcByteWidth) {
		f132_blitToBitmap(_g74_tmpBitmap, _g296_bitmapViewport, frame->_box, frame->_srcX, frame->_srcY,
						  frame->_srcByteWidth, k112_byteWidthViewport, k10_ColorFlesh, frame->_srcHeight, k136_heightViewport);
	}
}

void DisplayMan::f103_drawDoorFrameBitmapFlippedHorizontally(byte *bitmap, Frame *frame) {
	if (frame->_srcByteWidth) {
		f130_flipBitmapHorizontal(bitmap, frame->_srcByteWidth, frame->_srcHeight);
		f132_blitToBitmap(bitmap, _g296_bitmapViewport, frame->_box, frame->_srcX, frame->_srcY,
						  frame->_srcByteWidth, k112_byteWidthViewport, k10_ColorFlesh, frame->_srcHeight, k136_heightViewport);
	}
}

void DisplayMan::f110_drawDoorButton(int16 doorButtonOrdinal, int16 viewDoorButtonIndex) {
	static byte doorButtonCoordSet[1] = {0}; // @ G0197_auc_Graphic558_DoorButtonCoordinateSet
	static uint16 doorButtonCoordSets[1][4][6] = { // @ G0208_aaauc_Graphic558_DoorButtonCoordinateSets
		  // X1, X2, Y1, Y2, ByteWidth, Height
		{ {199, 204, 41, 44, 8, 4},   /* D3R */
		  {136, 141, 41, 44, 8, 4},   /* D3C */
		  {144, 155, 42, 47, 8, 6},   /* D2C */
		  {160, 175, 44, 52, 8, 9}    /* D1C */
		}
	};

	if (doorButtonOrdinal) {
		doorButtonOrdinal--;

		assert(doorButtonOrdinal == 0);

		int16 nativeBitmapIndex = doorButtonOrdinal + k315_firstDoorButton_GraphicIndice;
		int coordSet = doorButtonCoordSet[doorButtonOrdinal];
		uint16 *coordSetRedEagle = doorButtonCoordSets[coordSet][viewDoorButtonIndex];

		byte *bitmap = nullptr;
		if (viewDoorButtonIndex == k3_viewDoorButton_D1C) {
			bitmap = f489_getNativeBitmapOrGraphic(nativeBitmapIndex);

			_vm->_dungeonMan->_g291_dungeonViewClickableBoxes[k5_ViewCellDoorButtonOrWallOrn]._x1 = coordSetRedEagle[0];
			_vm->_dungeonMan->_g291_dungeonViewClickableBoxes[k5_ViewCellDoorButtonOrWallOrn]._x2 = coordSetRedEagle[1];
			_vm->_dungeonMan->_g291_dungeonViewClickableBoxes[k5_ViewCellDoorButtonOrWallOrn]._y1 = coordSetRedEagle[2];
			_vm->_dungeonMan->_g291_dungeonViewClickableBoxes[k5_ViewCellDoorButtonOrWallOrn]._y2 = coordSetRedEagle[3];
		} else {
			if (!f491_isDerivedBitmapInCache(doorButtonOrdinal = k102_DerivedBitmapFirstDoorButton + (doorButtonOrdinal * 2) + ((!viewDoorButtonIndex) ? 0 : viewDoorButtonIndex - 1))) {
				uint16 *coordSetBlueGoat = doorButtonCoordSets[coordSet][k3_viewDoorButton_D1C];
				byte *bitmapNative = f489_getNativeBitmapOrGraphic(nativeBitmapIndex);
				f129_blitToBitmapShrinkWithPalChange(bitmapNative, f492_getDerivedBitmap(doorButtonOrdinal),
													 coordSetBlueGoat[4] << 1, coordSetBlueGoat[5],
													 // modified code line
													 coordSetRedEagle[4] << 1,
													 coordSetRedEagle[5],
													 (viewDoorButtonIndex == k2_viewDoorButton_D2C) ? _palChangesDoorButtonAndWallOrnD2 : _palChangesDoorButtonAndWallOrnD3);

				f493_addDerivedBitmap(doorButtonOrdinal);
			}
			bitmap = f492_getDerivedBitmap(doorButtonOrdinal);
		}
		f132_blitToBitmap(bitmap, _g296_bitmapViewport, *(Box *)coordSetRedEagle, 0, 0,
						  coordSetRedEagle[4], k112_byteWidthViewport, k10_ColorFlesh, coordSetRedEagle[5], k136_heightViewport);
	}
}

void DisplayMan::f565_viewportSetPalette(uint16* middleScreenPalette, uint16* topAndBottomScreen) {
	if (middleScreenPalette && topAndBottomScreen)
		f508_buildPaletteChangeCopperList(middleScreenPalette, topAndBottomScreen);

	f566_viewportBlitToScreen();
}

void DisplayMan::f566_viewportBlitToScreen() {
	Box box(0, 223, 33, 33 + 135);

	f132_blitToBitmap(_g296_bitmapViewport, _g348_bitmapScreen, box, 0, 0, k112_byteWidthViewport, k160_byteWidthScreen, kM1_ColorNoTransparency,
					  k136_heightViewport, k200_heightScreen);
}

void DisplayMan::f466_loadIntoBitmap(uint16 index, byte *destBitmap) {
	uint8 *data = _packedBitmaps + _packedItemPos[index];

	uint16 width = READ_BE_UINT16(data);
	uint16 height = READ_BE_UINT16(data + 2);
	uint16 nextByteIndex = 4;

	for (int32 k = 0; k < width * height;) {
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

void DisplayMan::f132_blitToBitmap(byte *srcBitmap, byte *destBitmap, Box &box, uint16 srcX, uint16 srcY, uint16 srcByteWidth,
								   uint16 destByteWidth, Color transparent, int16 srcHeight, int16 destHight) {
	uint16 srcWidth = srcByteWidth * 2;
	uint16 destWidth = destByteWidth * 2;
	for (uint16 y = 0; y < box._y2 + 1 - box._y1; ++y) { // + 1 for inclusive boundaries
		for (uint16 x = 0; x < box._x2 + 1 - box._x1; ++x) { // + 1 for inclusive boundaries
			if (srcX + x < srcWidth && y + srcY < srcHeight
				&& box._x1 + x < destWidth && y + box._y1 < destHight) {
				byte srcPixel = srcBitmap[srcWidth * (y + srcY) + srcX + x];
				if (srcPixel != transparent)
					destBitmap[destWidth * (y + box._y1) + box._x1 + x] = srcPixel;
			}
		}
	}
}

void DisplayMan::D24_fillScreenBox(Box &box, Color color) {
	uint16 width = box._x2 + 1 - box._x1; // + 1 for inclusive boundaries
	for (int16 y = box._y1; y < box._y2 + 1; ++y) // + 1 for inclusive boundaries
		memset(_g348_bitmapScreen + y * _screenWidth + box._x1, color, sizeof(byte) * width);
}

void DisplayMan::f135_fillBoxBitmap(byte *destBitmap, Box &box, Color color, int16 byteWidth, int16 height) {
	for (int16 y = box._y1; y < box._y2 + 1; ++y) // + 1 for inclusive boundaries
		memset(destBitmap + y * byteWidth * 2 + box._x1, color, sizeof(byte) * (box._x2 - box._x1 + 1)); // + 1 for inclusive boundaries
}

void DisplayMan::f133_blitBoxFilledWithMaskedBitmap(byte *src, byte *dest, byte *mask, byte *tmp, Box& box,
													int16 lastUnitIndex, int16 firstUnitIndex, int16 destByteWidth, Color transparent,
													int16 xPos, int16 yPos, int16 destHeight, int16 height2) {
	// make sure to take care of inclusive boundaries, color can have 0x8000 flag to not use mask
	warning(false, "STUB: f133_blitBoxFilledWithMaskedBitmap");
}



void DisplayMan::f130_flipBitmapHorizontal(byte *bitmap, uint16 byteWidth, uint16 height) {
	uint16 width = byteWidth * 2;
	for (uint16 y = 0; y < height; ++y) {
		for (uint16 x = 0; x < width / 2; ++x)
			SWAP<byte>(bitmap[y * width + x], bitmap[y * width + width - 1 - x]);
	}
}

void DisplayMan::f131_flipVertical(byte *bitmap, uint16 byteWidth, uint16 height) {
	uint16 width = byteWidth * 2;
	byte *tmp = new byte[width];

	for (uint16 y = 0; y < height / 2; ++y) {
		memmove(tmp, bitmap + y * width, width);
		memmove(bitmap + y * width, bitmap + (height - 1 - y) * width, width);
		memmove(bitmap + (height - 1 - y) * width, tmp, width);
	}

	delete[] tmp;
}

byte *DisplayMan::f114_getExplosionBitmap(uint16 explosionAspIndex, uint16 scale, int16& returnByteWidth, int16& returnHeight) {
	ExplosionAspect *explAsp = &_explosionAspects[explosionAspIndex];
	if (scale > 32)
		scale = 32;
	int16 pixelWidth = M78_getScaledDimension(explAsp->_byteWidth, scale);
	int16 height = M78_getScaledDimension(explAsp->_height, scale);
	byte *bitmap;
	int16 derBitmapIndex = (explosionAspIndex * 14) + scale / 2 + k438_DerivedBitmapFirstExplosion - 2;
	if ((scale == 32) && (explosionAspIndex != k3_ExplosionAspectSmoke))
		bitmap = f489_getNativeBitmapOrGraphic(explosionAspIndex + k348_FirstExplosionGraphicIndice);
	else if (f491_isDerivedBitmapInCache(derBitmapIndex))
		bitmap = f492_getDerivedBitmap(derBitmapIndex);
	else {
		byte *nativeBitmap = f489_getNativeBitmapOrGraphic(MIN(explosionAspIndex, (uint16)k2_ExplosionAspectPoison) + k348_FirstExplosionGraphicIndice);
		bitmap = f492_getDerivedBitmap(derBitmapIndex);
		f129_blitToBitmapShrinkWithPalChange(nativeBitmap, bitmap, explAsp->_byteWidth, explAsp->_height, pixelWidth * 2, height,
			(explosionAspIndex == k3_ExplosionAspectSmoke) ? _palChangeSmoke : _palChangesNoChanges);
		f493_addDerivedBitmap(derBitmapIndex);
	}

	returnByteWidth = pixelWidth;
	returnHeight = height;
	return bitmap;
}

void DisplayMan::updateScreen() {
	// apply copper
	for (uint32 i = 320 * 30; i < 320 * 170; ++i)
		_g348_bitmapScreen[i] += 16;
	_vm->_system->copyRectToScreen(_g348_bitmapScreen, _screenWidth, 0, 0, _screenWidth, _screenHeight);
	_vm->_console->onFrame();
	_vm->_system->updateScreen();
	for (uint32 i = 320 * 30; i < 320 * 170; ++i)
		_g348_bitmapScreen[i] -= 16;
}

void DisplayMan::f97_drawViewport(int16 palSwitchingRequestedState) {
	static uint16 *dungeonViewCurrentPalette; // @ K0010_pui_DungeonViewCurrentPalette

	// ignored code F0510_AMIGA_WaitBottomOfViewPort
	if (palSwitchingRequestedState == k2_viewportAsBeforeSleepOrFreezeGame)
		palSwitchingRequestedState = _g322_paletteSwitchingEnabled ? 1 : 0;

	if (_g342_refreshDungeonViewPaleteRequested) {
		dungeonViewCurrentPalette = _palDungeonView[_g304_dungeonViewPaletteIndex];
		_g342_refreshDungeonViewPaleteRequested = false;
		if (palSwitchingRequestedState == k0_viewportNotDungeonView)
			_g322_paletteSwitchingEnabled = true;
		else
			_g322_paletteSwitchingEnabled = false;
	}

	if (palSwitchingRequestedState != (_g322_paletteSwitchingEnabled ? 1 : 0)) {
		if (palSwitchingRequestedState) {
			f565_viewportSetPalette(dungeonViewCurrentPalette, _g347_paletteTopAndBottomScreen);
			_g322_paletteSwitchingEnabled = true;
		} else {
			f565_viewportSetPalette(_g347_paletteTopAndBottomScreen, _g347_paletteTopAndBottomScreen);
			_g322_paletteSwitchingEnabled = false;
		}
	} else
		f565_viewportSetPalette(nullptr, nullptr);

	updateScreen();
}

byte *DisplayMan::getCurrentVgaBuffer() {
	return _g348_bitmapScreen;
}

uint16 DisplayMan::getPixelWidth(uint16 index) {
	byte *data = _packedBitmaps + _packedItemPos[index];
	return READ_BE_UINT16(data);
}

uint16 DisplayMan::getPixelHeight(uint16 index) {
	uint8 *data = _packedBitmaps + _packedItemPos[index];
	return READ_BE_UINT16(data + 2);
}

void DisplayMan::f99_copyBitmapAndFlipHorizontal(byte *srcBitmap, byte *destBitmap, uint16 byteWidth, uint16 height) {
	memmove(destBitmap, srcBitmap, byteWidth * 2 * height * sizeof(byte));
	f130_flipBitmapHorizontal(destBitmap, byteWidth, height);
}

void DisplayMan::f108_drawFloorOrnament(uint16 floorOrnOrdinal, uint16 viewFloorIndex) {
	static byte g191_floorOrnNativeBitmapndexInc[9] = { // @ G0191_auc_Graphic558_FloorOrnamentNativeBitmapIndexIncrements
		0,   /* D3L */
		1,   /* D3C */
		0,   /* D3R */
		2,   /* D2L */
		3,   /* D2C */
		2,   /* D2R */
		4,   /* D1L */
		5,   /* D1C */
		4};  /* D1R */

	static uint16 g206_floorOrnCoordSets[3][9][6] = { // @ G0206_aaauc_Graphic558_FloorOrnamentCoordinateSets
		/* { X1, X2, Y1, Y2, ByteWidth, Height } */
		{
			{32,  79, 66,  71, 24,  6},   /* D3L */
			{96, 127, 66,  71, 16,  6},   /* D3C */
			{144, 191, 66,  71, 24,  6},  /* D3R */
			{0,  63, 77,  87, 32, 11},    /* D2L */
			{80, 143, 77,  87, 32, 11},   /* D2C */
			{160, 223, 77,  87, 32, 11},  /* D2R */
			{0,  31, 92, 116, 16, 25},    /* D1L */
			{80, 143, 92, 116, 32, 25},   /* D1C */
			{192, 223, 92, 116, 16, 25}   /* D1R */
		},
		{
			{0,  95, 66,  74, 48,  9},    /* D3L */
			{64, 159, 66,  74, 48,  9},   /* D3C */
			{128, 223, 66,  74, 48,  9},  /* D3R */
			{0,  79, 75,  89, 40, 15},    /* D2L */
			{56, 167, 75,  89, 56, 15},   /* D2C */
			{144, 223, 75,  89, 40, 15},  /* D2R */
			{0,  63, 90, 118, 32, 29},    /* D1L */
			{32, 191, 90, 118, 80, 29},   /* D1C */
			{160, 223, 90, 118, 32, 29}   /* D1R */
		},
		{
			{42,  57, 68,  72,  8,  5},   /* D3L */
			{104, 119, 68,  72,  8,  5},  /* D3C */
			{166, 181, 68,  72,  8,  5},  /* D3R */
			{9,  40, 80,  85, 16,  6},    /* D2L */
			{96, 127, 80,  85, 16,  6},   /* D2C */
			{183, 214, 80,  85, 16,  6},  /* D2R */
			{0,  15, 97, 108,  8, 12},    /* D1L */
			{96, 127, 97, 108, 16, 12},   /* D1C */
			{208, 223, 97, 108,  8, 12}   /* D1R */
		}
	};

	if (!floorOrnOrdinal)
		return;

	bool drawFootprints = (getFlag(floorOrnOrdinal, k0x8000_FootprintsAspect) ? true : false);
	byte *bitmap;
	if (!drawFootprints || clearFlag(floorOrnOrdinal, k0x8000_FootprintsAspect)) {
		floorOrnOrdinal--;
		uint16 floorOrnIndex = floorOrnOrdinal;
		int16 nativeBitmapIndex = _g102_currMapFloorOrnInfo[floorOrnIndex][k0_NativeBitmapIndex]
			+ g191_floorOrnNativeBitmapndexInc[viewFloorIndex];
		uint16 *coordSets = g206_floorOrnCoordSets[_g102_currMapFloorOrnInfo[floorOrnIndex][k1_CoordinateSet]][viewFloorIndex];
		if ((viewFloorIndex == k8_viewFloor_D1R) || (viewFloorIndex == k5_viewFloor_D2R)
			|| (viewFloorIndex == k2_viewFloor_D3R)
			|| ((floorOrnIndex == k15_FloorOrnFootprints) && _g76_useFlippedWallAndFootprintsBitmap &&
			((viewFloorIndex == k7_viewFloor_D1C) || (viewFloorIndex == k4_viewFloor_D2C) || (viewFloorIndex == k1_viewFloor_D3C)))) {
			bitmap = _g74_tmpBitmap;
			f99_copyBitmapAndFlipHorizontal(f489_getNativeBitmapOrGraphic(nativeBitmapIndex), bitmap, coordSets[4], coordSets[5]);
		} else
			bitmap = f489_getNativeBitmapOrGraphic(nativeBitmapIndex);

		f132_blitToBitmap(bitmap, _g296_bitmapViewport,
						  *(Box *)coordSets, 0, 0, coordSets[4], k112_byteWidthViewport, k10_ColorFlesh, coordSets[5], k136_heightViewport);
	}

	if (drawFootprints)
		f108_drawFloorOrnament(_vm->M0_indexToOrdinal(k15_FloorOrnFootprints), viewFloorIndex);
}

void DisplayMan::f111_drawDoor(uint16 doorThingIndex, uint16 doorState, int16* doorNativeBitmapIndices, int16 byteCount, int16 viewDoorOrnIndex, DoorFrames* doorFrames) {
	if (doorState == k0_doorState_OPEN)
		return;

	DoorFrames *doorFramesTemp = doorFrames;
	Door *door = (Door *)(_vm->_dungeonMan->_g284_thingData[k0_DoorThingType]) + doorThingIndex;
	uint16 doorType = door->getType();
	memmove(_g74_tmpBitmap, f489_getNativeBitmapOrGraphic(doorNativeBitmapIndices[doorType]), byteCount * 2);
	f109_drawDoorOrnament(door->getOrnOrdinal(), viewDoorOrnIndex);
	if (getFlag(_vm->_dungeonMan->_g275_currMapDoorInfo[doorType]._attributes, k0x0004_MaskDoorInfo_Animated)) {
		if (_vm->getRandomNumber(2))
			f130_flipBitmapHorizontal(_g74_tmpBitmap, doorFramesTemp->_closedOrDestroyed._srcByteWidth, doorFramesTemp->_closedOrDestroyed._srcHeight);

		if (_vm->getRandomNumber(2))
			f131_flipVertical(_g74_tmpBitmap, doorFramesTemp->_closedOrDestroyed._srcByteWidth, doorFramesTemp->_closedOrDestroyed._srcHeight);
	}

	if ((doorFramesTemp == _doorFrameD1C) && _vm->_championMan->_g407_party._event73Count_ThievesEye)
		f109_drawDoorOrnament(_vm->M0_indexToOrdinal(k16_DoorOrnThivesEyeMask), k2_ViewDoorOrnament_D1LCR);

	if (doorState == k4_doorState_CLOSED)
		f102_drawDoorBitmap(&doorFramesTemp->_closedOrDestroyed);
	else if (doorState == k5_doorState_DESTROYED) {
		f109_drawDoorOrnament(_vm->M0_indexToOrdinal(k15_DoorOrnDestroyedMask), viewDoorOrnIndex);
		f102_drawDoorBitmap(&doorFramesTemp->_closedOrDestroyed);
	} else {
		doorState--;
		if (door->opensVertically())
			f102_drawDoorBitmap(&doorFramesTemp->_vertical[doorState]);
		else {
			f102_drawDoorBitmap(&doorFramesTemp->_leftHorizontal[doorState]);
			f102_drawDoorBitmap(&doorFramesTemp->_rightHorizontal[doorState]);
		}
	}
}

void DisplayMan::f109_drawDoorOrnament(int16 doorOrnOrdinal, int16 viewDoorOrnIndex) {
	static byte palChangesDoorOrnD3[16] = {0, 120, 10, 30, 40, 30, 0, 60, 30, 90, 100, 110, 0, 20, 0, 130}; // @ G0200_auc_Graphic558_PaletteChanges_DoorOrnament_D3
	static byte palChangesDoorOrnd2[16] = {0, 10, 20, 30, 40, 30, 60, 70, 50, 90, 100, 110, 120, 130, 140, 150}; // @ G0201_auc_Graphic558_PaletteChanges_DoorOrnament_D2
	static uint16 doorOrnCoordSets[4][3][6] = { // @ G0207_aaauc_Graphic558_DoorOrnamentCoordinateSets
		/* { X1, X2, Y1, Y2, ByteWidth, Height } */
		{
			{17, 31,  8, 17,  8, 10},   /* D3LCR */
			{22, 42, 11, 23, 16, 13},   /* D2LCR */
			{32, 63, 13, 31, 16, 19}    /* D1LCR */
		},
		{
			{0, 47,  0, 40, 24, 41},    /* D3LCR */
			{0, 63,  0, 60, 32, 61},    /* D2LCR */
			{0, 95,  0, 87, 48, 88}     /* D1LCR */
		},
		{
			{17, 31, 15, 24,  8, 10},   /* D3LCR */
			{22, 42, 22, 34, 16, 13},   /* D2LCR */
			{32, 63, 31, 49, 16, 19}    /* D1LCR */
		},
		{
			{23, 35, 31, 39,  8,  9},   /* D3LCR */
			{30, 48, 41, 52, 16, 12},   /* D2LCR */
			{44, 75, 61, 79, 16, 19}    /* D1LCR */
		}
	};

	int16 height = doorOrnOrdinal;

	if (!height)
		return;

	int16 byteWidth = viewDoorOrnIndex;
	height--;

	int16 nativeBitmapIndex = _g103_currMapDoorOrnInfo[height][k0_NativeBitmapIndex];
	int16 coordSetGreenToad = _g103_currMapDoorOrnInfo[height][k1_CoordinateSet];
	uint16 *coordSetOrangeElk = &doorOrnCoordSets[coordSetGreenToad][byteWidth][0];
	byte *blitBitmap;
	if (byteWidth == k2_ViewDoorOrnament_D1LCR) {
		blitBitmap = f489_getNativeBitmapOrGraphic(nativeBitmapIndex);
		byteWidth = k48_byteWidth;
		height = 88;
	} else {
		height = k68_DerivedBitmapFirstDoorOrnament_D3 + (height * 2) + byteWidth;
		if (!f491_isDerivedBitmapInCache(height)) {
			uint16 *coordSetRedEagle = &doorOrnCoordSets[coordSetGreenToad][k2_ViewDoorOrnament_D1LCR][0];
			byte *nativeBitmap = f489_getNativeBitmapOrGraphic(nativeBitmapIndex);
			f129_blitToBitmapShrinkWithPalChange(nativeBitmap, f492_getDerivedBitmap(height), coordSetRedEagle[4] << 1, coordSetRedEagle[5], coordSetOrangeElk[1] - coordSetOrangeElk[0] + 1, coordSetOrangeElk[5], (byteWidth == k0_ViewDoorOrnament_D3LCR) ? palChangesDoorOrnD3 : palChangesDoorOrnd2);
			f493_addDerivedBitmap(height);
		}
		blitBitmap = f492_getDerivedBitmap(height);
		if (byteWidth == k0_ViewDoorOrnament_D3LCR) {
			byteWidth = k24_byteWidth;
			height = 41;
		} else {
			byteWidth = k32_byteWidth;
			height = 61;
		}
	}

	Box box(coordSetOrangeElk[0], coordSetOrangeElk[1], coordSetOrangeElk[2], coordSetOrangeElk[3]);
	f132_blitToBitmap(blitBitmap, _g74_tmpBitmap, box, 0, 0, coordSetOrangeElk[4], byteWidth, k9_ColorGold, coordSetOrangeElk[5], height);
}

void DisplayMan::f112_drawCeilingPit(int16 nativeBitmapIndex, Frame *frame, int16 mapX, int16 mapY, bool flipHorizontal) {
	int16 mapIndex = _vm->_dungeonMan->f154_getLocationAfterLevelChange(_vm->_dungeonMan->_g272_currMapIndex, -1, &mapX, &mapY);

	if (mapIndex < 0)
		return;

	int16 mapSquare = _vm->_dungeonMan->_g279_dungeonMapData[mapIndex][mapX][mapY];
	if ((Square(mapSquare).getType() == k2_PitElemType) && getFlag(mapSquare, k0x0008_PitOpen)) {
		if (flipHorizontal)
			f105_drawFloorPitOrStairsBitmapFlippedHorizontally(nativeBitmapIndex, *frame);
		else
			f104_drawFloorPitOrStairsBitmap(nativeBitmapIndex, *frame);
	}
}

void DisplayMan::f20_blitToViewport(byte *bitmap, Box& box, int16 byteWidth, Color transparent, int16 height) {
	f132_blitToBitmap(bitmap, _g296_bitmapViewport, box, 0, 0, byteWidth, k112_byteWidthViewport, transparent, height, k136_heightViewport);
}

void DisplayMan::f20_blitToViewport(byte *bitmap, int16 *box, int16 byteWidth, Color transparent, int16 height) {
	Box actualBox(box[0], box[1], box[2], box[3]);
	f20_blitToViewport(bitmap, actualBox, byteWidth, transparent, height);
}

void DisplayMan::f21_blitToScreen(byte *bitmap, int16 *box, int16 byteWidth, Color transparent, int16 height) {
	Box actualBox(box[0], box[1], box[2], box[3]);
	f21_blitToScreen(bitmap, &actualBox, byteWidth, transparent, height);
}

void DisplayMan::f21_blitToScreen(byte *bitmap, Box *box, int16 byteWidth, Color transparent, int16 height) {
	_g578_useByteBoxCoordinates = false;
	f132_blitToBitmap(bitmap, _g348_bitmapScreen, *box, 0, 0, byteWidth, k160_byteWidthScreen, transparent, height, k200_heightScreen);
}

void DisplayMan::f101_drawWallSetBitmapWithoutTransparency(byte *bitmap, Frame &f) {
	if (!f._srcByteWidth)
		return;

	f132_blitToBitmap(bitmap, _g296_bitmapViewport, f._box, f._srcX, f._srcY, f._srcByteWidth, k112_byteWidthViewport, kM1_ColorNoTransparency, f._srcHeight, k136_heightViewport);
}

void DisplayMan::f100_drawWallSetBitmap(byte *bitmap, Frame &f) {
	if (!f._srcByteWidth)
		return;

	f132_blitToBitmap(bitmap, _g296_bitmapViewport, f._box, f._srcX, f._srcY, f._srcByteWidth, k112_byteWidthViewport, k10_ColorFlesh, f._srcHeight, k136_heightViewport);
}


void DisplayMan::f116_drawSquareD3L(Direction dir, int16 posX, int16 posY) {
	static Frame doorFrameLeftD3L = Frame(0, 31, 28, 70, 16, 43, 0, 0); // @ G0164_s_Graphic558_Frame_DoorFrameLeft_D3L
	static Frame frameStairsUpFrontD3L = Frame(0, 79, 25, 70, 40, 46, 0, 0); // @ G0110_s_Graphic558_Frame_StairsUpFront_D3L
	static Frame frameStairsDownFrontD3L = Frame(0, 79, 28, 68, 40, 41, 0, 0); // @ G0121_s_Graphic558_Frame_StairsDownFront_D3L
	static Frame frameFloorPitD3L = Frame(0, 79, 66, 73, 40, 8, 0, 0); // @ G0140_s_Graphic558_Frame_FloorPit_D3L
	static DoorFrames doorFrameD3L = DoorFrames( // @ G0179_s_Graphic558_Frames_Door_D3L
	/* { X1, X2, Y1, Y2, ByteWidth, Height, X, Y } */
		Frame(24, 71, 28, 67, 24, 41, 0, 0),   /* Closed Or Destroyed */
		Frame(24, 71, 28, 38, 24, 41, 0, 30),  /* Vertical Closed one fourth */
		Frame(24, 71, 28, 48, 24, 41, 0, 20),  /* Vertical Closed half */
		Frame(24, 71, 28, 58, 24, 41, 0, 10),  /* Vertical Closed three fourth */
		Frame(24, 29, 28, 67, 24, 41, 18, 0),  /* Left Horizontal Closed one fourth */
		Frame(24, 35, 28, 67, 24, 41, 12, 0),  /* Left Horizontal Closed half */
		Frame(24, 41, 28, 67, 24, 41, 6, 0),   /* Left Horizontal Closed three fourth */
		Frame(66, 71, 28, 67, 24, 41, 24, 0),  /* Right Horizontal Closed one fourth */
		Frame(60, 71, 28, 67, 24, 41, 24, 0),  /* Right Horizontal Closed half */
		Frame(54, 71, 28, 67, 24, 41, 24, 0)   /* Right Horizontal Closed three fourth */
	);

	uint16 squareAspect[5];
	int16 order;
	bool skip = false;
	_vm->_dungeonMan->f172_setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[k0_ElementAspect]) {
	case k19_StairsFrontElemType:
		if (squareAspect[k2_StairsUpAspect])
			f104_drawFloorPitOrStairsBitmap(_g675_stairsNativeBitmapIndex_Up_Front_D3L, frameStairsUpFrontD3L);
		else
			f104_drawFloorPitOrStairsBitmap(_g682_stairsNativeBitmapIndex_Down_Front_D3L, frameStairsDownFrontD3L);
		order = k0x3421_CellOrder_BackLeft_BackRight_FrontLeft_FrontRight;
		/* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k0_viewFloor_D3L);
		break;
	case k0_WallElemType:
		f100_drawWallSetBitmap(_g698_bitmapWallSet_Wall_D3LCR, _frameWalls163[k1_ViewSquare_D3L]);
		f107_isDrawnWallOrnAnAlcove(squareAspect[k2_RightWallOrnOrdAspect], k0_ViewWall_D3L_RIGHT);
		if (f107_isDrawnWallOrnAnAlcove(squareAspect[k3_FrontWallOrnOrdAspect], k2_ViewWall_D3L_FRONT))
			order = k0x0000_CellOrder_Alcove;
		else
			return;
		break;
	case k16_ElementTypeDoorSide:
	case k18_ElementTypeStairsSide:
		order = k0x0321_CellOrder_BackLeft_BackRight_FrontRight;
		/* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k0_viewFloor_D3L);
		break;
	case k17_ElementTypeDoorFront:
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k0_viewFloor_D3L);
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k1_ViewSquare_D3L, k0x0218_CellOrder_DoorPass1_BackLeft_BackRight);
		f100_drawWallSetBitmap(_g705_bitmapWallSet_DoorFrameLeft_D3L, doorFrameLeftD3L);
		f111_drawDoor(squareAspect[k3_DoorThingIndexAspect], squareAspect[k2_DoorStateAspect],
					  _g693_doorNativeBitmapIndex_Front_D3LCR, M75_bitmapByteCount(48, 41), k0_ViewDoorOrnament_D3LCR, &doorFrameD3L);
		order = k0x0349_CellOrder_DoorPass2_FrontLeft_FrontRight;
		break;
	case k2_ElementTypePit:
		if (!squareAspect[k2_PitInvisibleAspect])
			f104_drawFloorPitOrStairsBitmap(k49_FloorPit_D3L_GraphicIndice, frameFloorPitD3L);
	// no break on purpose
	case k5_ElementTypeTeleporter:
	case k1_ElementTypeCorridor:
		order = k0x3421_CellOrder_BackLeft_BackRight_FrontLeft_FrontRight;
		/* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k0_viewFloor_D3L);
		break;
	default:
		skip = true;
		break;
	}

	if (!skip)
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k1_ViewSquare_D3L, order);

	if ((squareAspect[k0_ElementAspect] == k5_ElementTypeTeleporter) && squareAspect[k2_TeleporterVisibleAspect])
		f113_drawField(&_fieldAspects188[k1_ViewSquare_D3L], _frameWalls163[k1_ViewSquare_D3L]._box);
}

void DisplayMan::f117_drawSquareD3R(Direction dir, int16 posX, int16 posY) {
	static Frame doorFrameRightD3R = Frame(192, 223, 28, 70, 16, 43, 0, 0); // @ G0165_s_Graphic558_Frame_DoorFrameRight_D3R
	static Frame frameStairsUpFrontD3R = Frame(149, 223, 25, 70, 40, 46, 5, 0); // @ G0112_s_Graphic558_Frame_StairsUpFront_D3R
	static Frame frameStairsDownFrontD3R = Frame(149, 223, 28, 68, 40, 41, 5, 0); // @ G0123_s_Graphic558_Frame_StairsDownFront_D3R
	static Frame frameFloorPitD3R = Frame(144, 223, 66, 73, 40, 8, 0, 0); // @ G0142_s_Graphic558_Frame_FloorPit_D3R
	static DoorFrames doorFrameD3R = DoorFrames( // @ G0181_s_Graphic558_Frames_Door_D3R
		/* { X1, X2, Y1, Y2, ByteWidth, Height, X, Y } */
		Frame(150, 197, 28, 67, 24, 41, 0, 0),	/* Closed Or Destroyed */
		Frame(150, 197, 28, 38, 24, 41, 0, 30),	/* Vertical Closed one fourth */
		Frame(150, 197, 28, 48, 24, 41, 0, 20),	/* Vertical Closed half */
		Frame(150, 197, 28, 58, 24, 41, 0, 10),	/* Vertical Closed three fourth */
		Frame(150, 153, 28, 67, 24, 41, 18, 0),	/* Left Horizontal Closed one fourth */
		Frame(150, 161, 28, 67, 24, 41, 12, 0),	/* Left Horizontal Closed half */
		Frame(150, 167, 28, 67, 24, 41, 6, 0),	/* Left Horizontal Closed three fourth */
		Frame(192, 197, 28, 67, 24, 41, 24, 0),	/* Right Horizontal Closed one fourth */
		Frame(186, 197, 28, 67, 24, 41, 24, 0),	/* Right Horizontal Closed half */
		Frame(180, 197, 28, 67, 24, 41, 24, 0)	/* Right Horizontal Closed three fourth */
	);

	int16 order;
	uint16 squareAspect[5];
	bool skip = false;

	_vm->_dungeonMan->f172_setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[k0_ElementAspect]) {
	case k19_ElementTypeStaisFront:
		if (squareAspect[k2_StairsUpAspect])
			f105_drawFloorPitOrStairsBitmapFlippedHorizontally(_g675_stairsNativeBitmapIndex_Up_Front_D3L, frameStairsUpFrontD3R);
		else
			f105_drawFloorPitOrStairsBitmapFlippedHorizontally(_g682_stairsNativeBitmapIndex_Down_Front_D3L, frameStairsDownFrontD3R);

		order = k0x4312_CellOrder_BackRight_BackLeft_FrontRight_FrontLeft;
		/* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k2_viewFloor_D3R);
		break;
	case k0_ElementTypeWall:
		f100_drawWallSetBitmap(_g698_bitmapWallSet_Wall_D3LCR, _frameWalls163[k2_ViewSquare_D3R]);
		f107_isDrawnWallOrnAnAlcove(squareAspect[k4_LeftWallOrnOrdAspect], k1_ViewWall_D3R_LEFT);
		if (f107_isDrawnWallOrnAnAlcove(squareAspect[k3_FrontWallOrnOrdAspect], k4_ViewWall_D3R_FRONT))
			order = k0x0000_CellOrder_Alcove;
		else
			return;
		break;
	case k16_ElementTypeDoorSide:
	case k18_ElementTypeStairsSide:
		order = k0x0412_CellOrder_BackRight_BackLeft_FrontLeft;
		/* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k2_viewFloor_D3R);
		break;
	case k17_ElementTypeDoorFront:
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k2_viewFloor_D3R);
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k2_ViewSquare_D3R, k0x0128_CellOrder_DoorPass1_BackRight_BackLeft);
		memmove(_g74_tmpBitmap, _g705_bitmapWallSet_DoorFrameLeft_D3L, 32 * 44);
		f103_drawDoorFrameBitmapFlippedHorizontally(_g74_tmpBitmap, &doorFrameRightD3R);
		if (((Door *)_vm->_dungeonMan->_g284_thingData[k0_DoorThingType])[squareAspect[k3_DoorThingIndexAspect]].hasButton())
			f110_drawDoorButton(_vm->M0_indexToOrdinal(k0_DoorButton), k0_viewDoorButton_D3R);

		f111_drawDoor(squareAspect[k3_DoorThingIndexAspect],
					  squareAspect[k2_DoorStateAspect], _g693_doorNativeBitmapIndex_Front_D3LCR,
					  M75_bitmapByteCount(48, 41), k0_ViewDoorOrnament_D3LCR, &doorFrameD3R);
		break;;
	case k2_ElementTypePit:
		if (!squareAspect[k2_PitInvisibleAspect])
			f105_drawFloorPitOrStairsBitmapFlippedHorizontally(k49_FloorPit_D3L_GraphicIndice, frameFloorPitD3R);
		// No break on purpose
	case k5_ElementTypeTeleporter:
	case k1_ElementTypeCorridor:
		order = k0x4312_CellOrder_BackRight_BackLeft_FrontRight_FrontLeft;
		/* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k2_viewFloor_D3R);
		break;
	default:
		skip = true;
		break;
	}

	if (!skip)
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k2_ViewSquare_D3R, order);

	if ((squareAspect[k0_ElementAspect] == k5_ElementTypeTeleporter) && squareAspect[k2_TeleporterVisibleAspect])
		f113_drawField(&_fieldAspects188[k2_ViewSquare_D3R], _frameWalls163[k2_ViewSquare_D3R]._box);
}

void DisplayMan::f118_drawSquareD3C(Direction dir, int16 posX, int16 posY) {
	static Frame doorFrameLeftD3C = Frame(64, 95, 27, 70, 16, 44, 0, 0); // @ G0166_s_Graphic558_Frame_DoorFrameLeft_D3C
	static Frame doorFrameRightD3C = Frame(128, 159, 27, 70, 16, 44, 0, 0); // @ G0167_s_Graphic558_Frame_DoorFrameRight_D3C
	static Frame frameStairsUpFrontD3C = Frame(64, 159, 25, 70, 48, 46, 0, 0); // @ G0111_s_Graphic558_Frame_StairsUpFront_D3C
	static Frame frameStairsDownFrontD3C = Frame(64, 159, 28, 70, 48, 43, 0, 0); // @ G0122_s_Graphic558_Frame_StairsDownFront_D3C
	static Frame frameFloorPitD3C = Frame(64, 159, 66, 73, 48, 8, 0, 0); // @ G0141_s_Graphic558_Frame_FloorPit_D3C
	static DoorFrames doorFrameD3C = DoorFrames( // @ G0180_s_Graphic558_Frames_Door_D3C
		/* { X1, X2, Y1, Y2, ByteWidth, Height, X, Y } */
		Frame(88, 135, 28, 67, 24, 41, 0, 0),		/* Closed Or Destroyed */
		Frame(88, 135, 28, 38, 24, 41, 0, 30),		/* Vertical Closed one fourth */
		Frame(88, 135, 28, 48, 24, 41, 0, 20),		/* Vertical Closed half */
		Frame(88, 135, 28, 58, 24, 41, 0, 10),		/* Vertical Closed three fourth */
		Frame(88, 93, 28, 67, 24, 41, 18, 0),		/* Left Horizontal Closed one fourth */
		Frame(88, 99, 28, 67, 24, 41, 12, 0),		/* Left Horizontal Closed half */
		Frame(88, 105, 28, 67, 24, 41, 6, 0),		/* Left Horizontal Closed three fourth */
		Frame(130, 135, 28, 67, 24, 41, 24, 0),		/* Right Horizontal Closed one fourth */
		Frame(124, 135, 28, 67, 24, 41, 24, 0),		/* Right Horizontal Closed half */
		Frame(118, 135, 28, 67, 24, 41, 24, 0)		/* Right Horizontal Closed three fourth */
	);

	uint16 squareAspect[5];
	int16 order;
	bool skip = false;

	_vm->_dungeonMan->f172_setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[k0_ElementAspect]) {
	case k19_ElementTypeStaisFront:
		if (squareAspect[k2_StairsUpAspect])
			f104_drawFloorPitOrStairsBitmap(_g676_stairsNativeBitmapIndex_Up_Front_D3C, frameStairsUpFrontD3C);
		else
			f104_drawFloorPitOrStairsBitmap(_g683_stairsNativeBitmapIndex_Down_Front_D3C, frameStairsDownFrontD3C);

		order = k0x3421_CellOrder_BackLeft_BackRight_FrontLeft_FrontRight;
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k1_viewFloor_D3C); /* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		break;
	case k0_ElementTypeWall:
		f101_drawWallSetBitmapWithoutTransparency(_g698_bitmapWallSet_Wall_D3LCR, _frameWalls163[k0_ViewSquare_D3C]);
		if (f107_isDrawnWallOrnAnAlcove(squareAspect[k3_FrontWallOrnOrdAspect], k3_ViewWall_D3C_FRONT))
			order = k0x0000_CellOrder_Alcove;
		else
			return;

		break;
	case k17_DoorFrontElemType:
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k1_viewFloor_D3C);
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k0_ViewSquare_D3C, k0x0218_CellOrder_DoorPass1_BackLeft_BackRight);
		f100_drawWallSetBitmap(_g706_bitmapWallSet_DoorFrameLeft_D3C, doorFrameLeftD3C);
		memmove(_g74_tmpBitmap, _g706_bitmapWallSet_DoorFrameLeft_D3C, 32 * 44);
		f103_drawDoorFrameBitmapFlippedHorizontally(_g74_tmpBitmap, &doorFrameRightD3C);
		if (((Door *)_vm->_dungeonMan->_g284_thingData[k0_DoorThingType])[squareAspect[k3_DoorThingIndexAspect]].hasButton())
			f110_drawDoorButton(_vm->M0_indexToOrdinal(k0_DoorButton), k1_ViewDoorOrnament_D2LCR);

		f111_drawDoor(squareAspect[k3_DoorThingIndexAspect], squareAspect[k2_DoorStateAspect],
					  _g693_doorNativeBitmapIndex_Front_D3LCR, M75_bitmapByteCount(48, 41), k0_ViewDoorOrnament_D3LCR, &doorFrameD3C);
		order = k0x0349_CellOrder_DoorPass2_FrontLeft_FrontRight;
		break;
	case k2_ElementTypePit:
		if (!squareAspect[k2_PitInvisibleAspect])
			f104_drawFloorPitOrStairsBitmap(k50_FloorPit_D3C_GraphicIndice, frameFloorPitD3C);
	// No break on purpose
	case k5_ElementTypeTeleporter:
	case k1_CorridorElemType:
		order = k0x3421_CellOrder_BackLeft_BackRight_FrontLeft_FrontRight;
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k1_viewFloor_D3C); /* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		break;
	default:
		skip = true;
		break;
	}

	if (!skip)
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k0_ViewSquare_D3C, order);

	if ((squareAspect[k0_ElementAspect] == k5_ElementTypeTeleporter) && squareAspect[k2_TeleporterVisibleAspect])
		f113_drawField(&_fieldAspects188[k0_ViewSquare_D3C], _frameWalls163[k0_ViewSquare_D3C]._box);
}

void DisplayMan::f119_drawSquareD2L(Direction dir, int16 posX, int16 posY) {
	static Frame doorFrameTopD2L = Frame(0, 59, 22, 24, 48, 3, 16, 0); // @ G0173_s_Graphic558_Frame_DoorFrameTop_D2L
	static Frame frameStairsUpFrontD2L = Frame(0, 63, 22, 83, 32, 62, 0, 0); // @ G0113_s_Graphic558_Frame_StairsUpFront_D2L
	static Frame frameStairsDownFrontD2L = Frame(0, 63, 24, 85, 32, 62, 0, 0); // @ G0124_s_Graphic558_Frame_StairsDownFront_D2L
	static Frame frameStairsSideD2L = Frame(60, 75, 57, 61, 8, 5, 0, 0); // @ G0132_s_Graphic558_Frame_StairsSide_D2L
	static Frame frameFloorPitD2L = Frame(0, 79, 77, 88, 40, 12, 0, 0); // @ G0143_s_Graphic558_Frame_FloorPit_D2L
	static Frame FrameCeilingPitD2L = Frame(0, 79, 19, 23, 40, 5, 0, 0); // @ G0152_s_Graphic558_Frame_CeilingPit_D2L
	static DoorFrames doorFrameD2L = DoorFrames( // @ G0182_s_Graphic558_Frames_Door_D2L
		/* { X1, X2, Y1, Y2, ByteWidth, Height, X, Y } */
		Frame(0, 63, 24, 82, 32, 61, 0, 0),	   /* Closed Or Destroyed */
		Frame(0, 63, 24, 39, 32, 61, 0, 45),   /* Vertical Closed one fourth */
		Frame(0, 63, 24, 54, 32, 61, 0, 30),   /* Vertical Closed half */
		Frame(0, 63, 24, 69, 32, 61, 0, 15),   /* Vertical Closed three fourth */
		Frame(0, 7, 24, 82, 32, 61, 24, 0),    /* Left Horizontal Closed one fourth */
		Frame(0, 15, 24, 82, 32, 61, 16, 0),   /* Left Horizontal Closed half */
		Frame(0, 23, 24, 82, 32, 61, 8, 0),    /* Left Horizontal Closed three fourth */
		Frame(56, 63, 24, 82, 32, 61, 32, 0),  /* Right Horizontal Closed one fourth */
		Frame(48, 63, 24, 82, 32, 61, 32, 0),  /* Right Horizontal Closed half */
		Frame(40, 63, 24, 82, 32, 61, 32, 0)   /* Right Horizontal Closed three fourth */
	);

	int16 order;
	uint16 squareAspect[5];
	bool skip = false;

	_vm->_dungeonMan->f172_setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[k0_ElementAspect]) {
	case k19_ElementTypeStaisFront:
		if (squareAspect[k2_StairsUpAspect])
			f104_drawFloorPitOrStairsBitmap(_g677_stairsNativeBitmapIndex_Up_Front_D2L, frameStairsUpFrontD2L);
		else
			f104_drawFloorPitOrStairsBitmap(_g684_stairsNativeBitmapIndex_Down_Front_D2L, frameStairsDownFrontD2L);

		order = k0x3421_CellOrder_BackLeft_BackRight_FrontLeft_FrontRight;
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k3_viewFloor_D2L); /* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		break;
	case k0_ElementTypeWall:
		f100_drawWallSetBitmap(_g699_bitmapWallSet_Wall_D2LCR, _frameWalls163[k4_ViewSquare_D2L]);
		f107_isDrawnWallOrnAnAlcove(squareAspect[k2_RightWallOrnOrdAspect], k5_ViewWall_D2L_RIGHT);
		if (f107_isDrawnWallOrnAnAlcove(squareAspect[k3_FrontWallOrnOrdAspect], k7_ViewWall_D2L_FRONT))
			order = k0x0000_CellOrder_Alcove;
		else 
			return;
		break;
	case k18_ElementTypeStairsSide:
		f104_drawFloorPitOrStairsBitmap(_g689_stairsNativeBitmapIndex_Side_D2L, frameStairsSideD2L);
		// No break on purpose
	case k16_DoorSideElemType:
		order = k0x0342_CellOrder_BackRight_FrontLeft_FrontRight;
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k3_viewFloor_D2L); /* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		break;
	case k17_DoorFrontElemType:
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k3_viewFloor_D2L);
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k4_ViewSquare_D2L, k0x0218_CellOrder_DoorPass1_BackLeft_BackRight);
		f100_drawWallSetBitmap(_g703_bitmapWallSet_DoorFrameTop_D2LCR, doorFrameTopD2L);
		f111_drawDoor(squareAspect[k3_DoorThingIndexAspect], squareAspect[k2_DoorStateAspect], _g694_doorNativeBitmapIndex_Front_D2LCR,
					  M75_bitmapByteCount(64, 61), k1_ViewDoorOrnament_D2LCR, &doorFrameD2L);
		order = k0x0349_CellOrder_DoorPass2_FrontLeft_FrontRight;
		break;
	case k2_ElementTypePit:
		f104_drawFloorPitOrStairsBitmap(squareAspect[k2_PitInvisibleAspect] ? k57_FloorPir_Invisible_D2L_GraphicIndice : k51_FloorPit_D2L_GraphicIndice,
										frameFloorPitD2L);
		// No break on purpose
	case k5_ElementTypeTeleporter:
	case k1_CorridorElemType:
		order = k0x3421_CellOrder_BackLeft_BackRight_FrontLeft_FrontRight;
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k3_viewFloor_D2L); /* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		break;

	default:
		skip = true;
		break;
	}

	if (!skip) {
		f112_drawCeilingPit(k63_ceilingPit_D2L_GraphicIndice, &FrameCeilingPitD2L, posX, posY, false);
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k4_ViewSquare_D2L, order);
	}

	if ((squareAspect[k0_ElementAspect] == k5_ElementTypeTeleporter) && squareAspect[k2_TeleporterVisibleAspect])
		f113_drawField(&_fieldAspects188[k4_ViewSquare_D2L], _frameWalls163[k4_ViewSquare_D2L]._box);
}

void DisplayMan::f120_drawSquareD2R(Direction dir, int16 posX, int16 posY) {
	static Frame doorFrameTopD2R = Frame(164, 223, 22, 24, 48, 3, 16, 0); // @ G0175_s_Graphic558_Frame_DoorFrameTop_D2R
	static Frame frameStairsUpFrontD2R = Frame(160, 223, 22, 83, 32, 62, 0, 0); // @ G0115_s_Graphic558_Frame_StairsUpFront_D2R
	static Frame frameStairsDownFrontD2R = Frame(160, 223, 24, 85, 32, 62, 0, 0); // @ G0126_s_Graphic558_Frame_StairsDownFront_D2R
	static Frame frameStairsSideD2R = Frame(148, 163, 57, 61, 8, 5, 0, 0); // @ G0133_s_Graphic558_Frame_StairsSide_D2R
	static Frame frameFloorPitD2R = Frame(144, 223, 77, 88, 40, 12, 0, 0); // @ G0145_s_Graphic558_Frame_FloorPit_D2R
	static Frame frameCeilingPitD2R = Frame(144, 223, 19, 23, 40, 5, 0, 0); // @ G0154_s_Graphic558_Frame_CeilingPit_D2R
	static DoorFrames doorFrameD2R = DoorFrames( // @ G0184_s_Graphic558_Frames_Door_D2R
		/* { X1, X2, Y1, Y2, ByteWidth, Height, X, Y } */
		Frame(160, 223, 24, 82, 32, 61, 0, 0),	/* Closed Or Destroyed */
		Frame(160, 223, 24, 39, 32, 61, 0, 45),	/* Vertical Closed one fourth */
		Frame(160, 223, 24, 54, 32, 61, 0, 30),	/* Vertical Closed half */
		Frame(160, 223, 24, 69, 32, 61, 0, 15),	/* Vertical Closed three fourth */
		Frame(160, 167, 24, 82, 32, 61, 24, 0),	/* Left Horizontal Closed one fourth */
		Frame(160, 175, 24, 82, 32, 61, 16, 0),	/* Left Horizontal Closed half */
		Frame(160, 183, 24, 82, 32, 61, 8, 0),	/* Left Horizontal Closed three fourth */
		Frame(216, 223, 24, 82, 32, 61, 32, 0),	/* Right Horizontal Closed one fourth */
		Frame(208, 223, 24, 82, 32, 61, 32, 0),	/* Right Horizontal Closed half */
		Frame(200, 223, 24, 82, 32, 61, 32, 0)		/* Right Horizontal Closed three fourth */
	);

	int16 order;
	uint16 squareAspect[5];
	bool skip = false;

	_vm->_dungeonMan->f172_setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[k0_ElementAspect]) {
	case k19_ElementTypeStaisFront:
		if (squareAspect[k2_StairsUpAspect])
			f105_drawFloorPitOrStairsBitmapFlippedHorizontally(_g677_stairsNativeBitmapIndex_Up_Front_D2L, frameStairsUpFrontD2R);
		else
			f105_drawFloorPitOrStairsBitmapFlippedHorizontally(_g684_stairsNativeBitmapIndex_Down_Front_D2L, frameStairsDownFrontD2R);

		order = k0x4312_CellOrder_BackRight_BackLeft_FrontRight_FrontLeft;
		/* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k5_viewFloor_D2R);
		f112_drawCeilingPit(k63_ceilingPit_D2L_GraphicIndice, &frameCeilingPitD2R, posX, posY, true);
		break;
	case k0_ElementTypeWall:
		f100_drawWallSetBitmap(_g699_bitmapWallSet_Wall_D2LCR, _frameWalls163[k5_ViewSquare_D2R]);
		f107_isDrawnWallOrnAnAlcove(squareAspect[k4_LeftWallOrnOrdAspect], k6_ViewWall_D2R_LEFT);
		if (f107_isDrawnWallOrnAnAlcove(squareAspect[k3_FrontWallOrnOrdAspect], k9_ViewWall_D2R_FRONT))
			order = k0x0000_CellOrder_Alcove;
		else
			return;
		break;
	case k18_ElementTypeStairsSide:
		f105_drawFloorPitOrStairsBitmapFlippedHorizontally(_g689_stairsNativeBitmapIndex_Side_D2L, frameStairsSideD2R);
		// No break on purpose
	case k16_DoorSideElemType:
		order = k0x0431_CellOrder_BackLeft_FrontRight_FrontLeft;
		/* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k5_viewFloor_D2R);
		f112_drawCeilingPit(k63_ceilingPit_D2L_GraphicIndice, &frameCeilingPitD2R, posX, posY, true);
		break;
	case k17_DoorFrontElemType:
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k5_ViewSquare_D2R);
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k5_ViewSquare_D2R, k0x0128_CellOrder_DoorPass1_BackRight_BackLeft);
		f100_drawWallSetBitmap(_g703_bitmapWallSet_DoorFrameTop_D2LCR, doorFrameTopD2R);
		f111_drawDoor(squareAspect[k3_DoorThingIndexAspect], squareAspect[k2_DoorStateAspect],
					  _g694_doorNativeBitmapIndex_Front_D2LCR, M75_bitmapByteCount(64, 61), k1_ViewDoorOrnament_D2LCR, &doorFrameD2R);
		order = k0x0439_CellOrder_DoorPass2_FrontRight_FrontLeft;
		break;
	case k2_ElementTypePit:
		f105_drawFloorPitOrStairsBitmapFlippedHorizontally(
			squareAspect[k2_PitInvisibleAspect] ? k57_FloorPir_Invisible_D2L_GraphicIndice : k51_FloorPit_D2L_GraphicIndice, frameFloorPitD2R);
		// No break on purpose
	case k5_ElementTypeTeleporter:
	case k1_CorridorElemType:
		order = k0x4312_CellOrder_BackRight_BackLeft_FrontRight_FrontLeft;
		/* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k5_viewFloor_D2R);
		f112_drawCeilingPit(k63_ceilingPit_D2L_GraphicIndice, &frameCeilingPitD2R, posX, posY, true);
		break;
	default:
		skip = true;
		break;
	}

	if (!skip)
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k5_ViewSquare_D2R, order);

	if ((squareAspect[k0_ElementAspect] == k5_ElementTypeTeleporter) && squareAspect[k2_TeleporterVisibleAspect])
		f113_drawField(&_fieldAspects188[k5_ViewSquare_D2R], _frameWalls163[k5_ViewSquare_D2R]._box);
}

void DisplayMan::f121_drawSquareD2C(Direction dir, int16 posX, int16 posY) {
	static Frame doorFrameLeftD2C = Frame(48, 95, 22, 86, 24, 65, 0, 0); // @ G0168_s_Graphic558_Frame_DoorFrameLeft_D2C
	static Frame doorFrameRightD2C = Frame(128, 175, 22, 86, 24, 65, 0, 0); // @ G0169_s_Graphic558_Frame_DoorFrameRight_D2C
	static Frame doorFrameTopD2C = Frame(64, 159, 22, 24, 48, 3, 0, 0); // @ G0174_s_Graphic558_Frame_DoorFrameTop_D2C
	static Frame frameStairsUpFrontD2C = Frame(64, 159, 22, 83, 48, 62, 0, 0); // @ G0114_s_Graphic558_Frame_StairsUpFront_D2C
	static Frame frameStairsDownFrontD2C = Frame(64, 159, 24, 85, 48, 62, 0, 0); // @ G0125_s_Graphic558_Frame_StairsDownFront_D2C
	static Frame frameFloorPitD2C = Frame(64, 159, 77, 88, 48, 12, 0, 0); // @ G0144_s_Graphic558_Frame_FloorPit_D2C
	static Frame frameCeilingPitD2C = Frame(64, 159, 19, 23, 48, 5, 0, 0); // @ G0153_s_Graphic558_Frame_CeilingPit_D2C
	static DoorFrames doorFrameD2C = DoorFrames( // @ G0183_s_Graphic558_Frames_Door_D2C
		/* { X1, X2, Y1, Y2, ByteWidth, Height, X, Y } */
		Frame(80, 143, 24, 82, 32, 61, 0, 0),    /* Closed Or Destroyed */
		Frame(80, 143, 24, 39, 32, 61, 0, 45),   /* Vertical Closed one fourth */
		Frame(80, 143, 24, 54, 32, 61, 0, 30),   /* Vertical Closed half */
		Frame(80, 143, 24, 69, 32, 61, 0, 15),   /* Vertical Closed three fourth */
		Frame(80, 87, 24, 82, 32, 61, 24, 0),    /* Left Horizontal Closed one fourth */
		Frame(80, 95, 24, 82, 32, 61, 16, 0),    /* Left Horizontal Closed half */
		Frame(80, 103, 24, 82, 32, 61, 8, 0),    /* Left Horizontal Closed three fourth */
		Frame(136, 143, 24, 82, 32, 61, 32, 0),  /* Right Horizontal Closed one fourth */
		Frame(128, 143, 24, 82, 32, 61, 32, 0),  /* Right Horizontal Closed half */
		Frame(120, 143, 24, 82, 32, 61, 32, 0)   /* Right Horizontal Closed three fourth */
	);

	int16 order;
	uint16 squareAspect[5];
	bool skip = false;

	_vm->_dungeonMan->f172_setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[k0_ElementAspect]) {
	case k19_ElementTypeStaisFront:
		if (squareAspect[k2_StairsUpAspect])
			f104_drawFloorPitOrStairsBitmap(_g678_stairsNativeBitmapIndex_Up_Front_D2C, frameStairsUpFrontD2C);
		else
			f104_drawFloorPitOrStairsBitmap(_g685_stairsNativeBitmapIndex_Down_Front_D2C, frameStairsDownFrontD2C);

		order = k0x3421_CellOrder_BackLeft_BackRight_FrontLeft_FrontRight;
		/* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k4_viewFloor_D2C);
		f112_drawCeilingPit(k64_ceilingPitD2C_GraphicIndice, &frameCeilingPitD2C, posX, posY, false);
		break;
	case k0_ElementTypeWall:
		f101_drawWallSetBitmapWithoutTransparency(_g699_bitmapWallSet_Wall_D2LCR, _frameWalls163[k3_ViewSquare_D2C]);
		if (f107_isDrawnWallOrnAnAlcove(squareAspect[k3_FrontWallOrnOrdAspect], k8_ViewWall_D2C_FRONT))
			order = k0x0000_CellOrder_Alcove;
		else
			return;
		break;
	case k17_DoorFrontElemType:
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k4_viewFloor_D2C);
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k3_ViewSquare_D2C, k0x0218_CellOrder_DoorPass1_BackLeft_BackRight);
		f100_drawWallSetBitmap(_g703_bitmapWallSet_DoorFrameTop_D2LCR, doorFrameTopD2C);
		f100_drawWallSetBitmap(_g707_bitmapWallSet_DoorFrameLeft_D2C, doorFrameLeftD2C);
		memcpy(_g74_tmpBitmap, _g707_bitmapWallSet_DoorFrameLeft_D2C, 48 * 65);
		f103_drawDoorFrameBitmapFlippedHorizontally(_g74_tmpBitmap, &doorFrameRightD2C);
		if (((Door *)_vm->_dungeonMan->_g284_thingData[k0_DoorThingType])[squareAspect[k3_DoorThingIndexAspect]].hasButton())
			f110_drawDoorButton(_vm->M0_indexToOrdinal(k0_DoorButton), k2_viewDoorButton_D2C);

		f111_drawDoor(squareAspect[k3_DoorThingIndexAspect], squareAspect[k2_DoorStateAspect],
					  _g694_doorNativeBitmapIndex_Front_D2LCR, M75_bitmapByteCount(64, 61), k1_ViewDoorOrnament_D2LCR, &doorFrameD2C);
		order = k0x0349_CellOrder_DoorPass2_FrontLeft_FrontRight;
		break;
	case k2_ElementTypePit:
		f104_drawFloorPitOrStairsBitmap(squareAspect[k2_PitInvisibleAspect] ? k58_FloorPit_invisible_D2C_GraphicIndice : k52_FloorPit_D2C_GraphicIndice, frameFloorPitD2C);
		// No break on purpose
	case k5_ElementTypeTeleporter:
	case k1_CorridorElemType:
		order = k0x3421_CellOrder_BackLeft_BackRight_FrontLeft_FrontRight;
		/* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k4_viewFloor_D2C);
		f112_drawCeilingPit(k64_ceilingPitD2C_GraphicIndice, &frameCeilingPitD2C, posX, posY, false);
		break;
	default:
		skip = true;
		break;
	}

	if (!skip)
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k3_ViewSquare_D2C, order);

	if ((squareAspect[k0_ElementAspect] == k5_ElementTypeTeleporter) && squareAspect[k2_TeleporterVisibleAspect])
		f113_drawField(&_fieldAspects188[k3_ViewSquare_D2C], _frameWalls163[k3_ViewSquare_D2C]._box);
}

void DisplayMan::f122_drawSquareD1L(Direction dir, int16 posX, int16 posY) {
	static Frame doorFrameTopD1L = Frame(0, 31, 14, 17, 64, 4, 16, 0); // @ G0176_s_Graphic558_Frame_DoorFrameTop_D1L
	static Frame frameStairsUpFrontD1L = Frame(0, 31, 9, 108, 16, 100, 0, 0); // @ G0116_s_Graphic558_Frame_StairsUpFront_D1L
	static Frame frameStairsDownFrontD1L = Frame(0, 31, 18, 108, 16, 91, 0, 0); // @ G0127_s_Graphic558_Frame_StairsDownFront_D1L
	static Frame frameStairsUpSideD1L = Frame(32, 63, 57, 99, 16, 43, 0, 0); // @ G0134_s_Graphic558_Frame_StairsUpSide_D1L
	static Frame frameStairsDownSideD1L = Frame(32, 63, 60, 98, 16, 39, 0, 0); // @ G0136_s_Graphic558_Frame_StairsDownSide_D1L
	static Frame frameFloorPitD1L = Frame(0, 63, 93, 116, 32, 24, 0, 0); // @ G0146_s_Graphic558_Frame_FloorPit_D1L
	static Frame frameCeilingPitD1L = Frame(0, 63, 8, 16, 32, 9, 0, 0); // @ G0155_s_Graphic558_Frame_CeilingPit_D1L
	static DoorFrames doorFrameD1L = DoorFrames( // @ G0185_s_Graphic558_Frames_Door_D1L
		/* { X1, X2, Y1, Y2, ByteWidth, Height, X, Y } */
		Frame(0, 31, 17, 102, 48, 88, 64, 0),   /* Closed Or Destroyed */
		Frame(0, 31, 17, 38, 48, 88, 64, 66),   /* Vertical Closed one fourth */
		Frame(0, 31, 17, 60, 48, 88, 64, 44),   /* Vertical Closed half */
		Frame(0, 31, 17, 82, 48, 88, 64, 22),   /* Vertical Closed three fourth */
		Frame(0, 0, 0, 0, 0, 0, 0, 0),          /* Left Horizontal Closed one fourth */
		Frame(0, 0, 0, 0, 0, 0, 0, 0),          /* Left Horizontal Closed half */
		Frame(0, 0, 0, 0, 0, 0, 0, 0),          /* Left Horizontal Closed three fourth */
		Frame(20, 31, 17, 102, 48, 88, 48, 0),  /* Right Horizontal Closed one fourth */
		Frame(8, 31, 17, 102, 48, 88, 48, 0),   /* Right Horizontal Closed half */
		Frame(0, 31, 17, 102, 48, 88, 52, 0)    /* Right Horizontal Closed three fourth */
	);

	int16 order;
	uint16 squareAspect[5];
	bool skip = false;

	_vm->_dungeonMan->f172_setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[k0_ElementAspect]) {
	case k19_ElementTypeStaisFront:
		if (squareAspect[k2_StairsUpAspect])
			f104_drawFloorPitOrStairsBitmap(_g679_stairsNativeBitmapIndex_Up_Front_D1L, frameStairsUpFrontD1L);
		else
			f104_drawFloorPitOrStairsBitmap(_g686_stairsNativeBitmapIndex_Down_Front_D1L, frameStairsDownFrontD1L);

		order = k0x0032_CellOrder_BackRight_FrontRight;
		/* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k6_viewFloor_D1L);
		f112_drawCeilingPit(k65_ceilingPitD1L_GraphicIndice, &frameCeilingPitD1L, posX, posY, false);
		break;
	case k0_ElementTypeWall:
		f100_drawWallSetBitmap(_g700_bitmapWallSet_Wall_D1LCR, _frameWalls163[k7_ViewSquare_D1L]);
		f107_isDrawnWallOrnAnAlcove(squareAspect[k2_RightWallOrnOrdAspect], k10_ViewWall_D1L_RIGHT);
		return;
	case k18_ElementTypeStairsSide:
		if (squareAspect[k2_StairsUpAspect])
			f104_drawFloorPitOrStairsBitmap(_g690_stairsNativeBitmapIndex_Up_Side_D1L, frameStairsUpSideD1L);
		else
			f104_drawFloorPitOrStairsBitmap(_g691_stairsNativeBitmapIndex_Down_Side_D1L, frameStairsDownSideD1L);
		// No break on purpose
	case k16_DoorSideElemType:
		order = k0x0032_CellOrder_BackRight_FrontRight;
		/* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k6_viewFloor_D1L);
		f112_drawCeilingPit(k65_ceilingPitD1L_GraphicIndice, &frameCeilingPitD1L, posX, posY, false);
		break;
	case k17_DoorFrontElemType:
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k6_viewFloor_D1L);
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k7_ViewSquare_D1L, k0x0028_CellOrder_DoorPass1_BackRight);
		f100_drawWallSetBitmap(_g704_bitmapWallSet_DoorFrameTop_D1LCR, doorFrameTopD1L);
		f111_drawDoor(squareAspect[k3_DoorThingIndexAspect], squareAspect[k2_DoorStateAspect],
					  _g695_doorNativeBitmapIndex_Front_D1LCR, M75_bitmapByteCount(96, 88), k2_ViewDoorOrnament_D1LCR, &doorFrameD1L);
		order = k0x0039_CellOrder_DoorPass2_FrontRight;
		break;
	case k2_ElementTypePit:
		f104_drawFloorPitOrStairsBitmap(squareAspect[k2_PitInvisibleAspect] ? k59_floorPit_invisible_D1L_GraphicIndice : k53_FloorPit_D1L_GraphicIndice, frameFloorPitD1L);
		// No break on purpose
	case k5_ElementTypeTeleporter:
	case k1_CorridorElemType:
		order = k0x0032_CellOrder_BackRight_FrontRight;
		 /* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k6_viewFloor_D1L);
		f112_drawCeilingPit(k65_ceilingPitD1L_GraphicIndice, &frameCeilingPitD1L, posX, posY, false);
		break;
	default:
		skip = true;
		break;
	}

	if (!skip)
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k7_ViewSquare_D1L, order);

	if ((squareAspect[k0_ElementAspect] == k5_ElementTypeTeleporter) && squareAspect[k2_TeleporterVisibleAspect])
		f113_drawField(&_fieldAspects188[k7_ViewSquare_D1L], _frameWalls163[k7_ViewSquare_D1L]._box);
}

void DisplayMan::f123_drawSquareD1R(Direction dir, int16 posX, int16 posY) {
	static Frame doorFrameTopD1R = Frame(192, 223, 14, 17, 64, 4, 16, 0); // @ G0178_s_Graphic558_Frame_DoorFrameTop_D1R
	static Frame frameStairsUpFrontD1R = Frame(192, 223, 9, 108, 16, 100, 0, 0); // @ G0118_s_Graphic558_Frame_StairsUpFront_D1R
	static Frame frameStairsDownFrontD1R = Frame(192, 223, 18, 108, 16, 91, 0, 0); // @ G0129_s_Graphic558_Frame_StairsDownFront_D1R
	static Frame frameStairsUpSideD1R = Frame(160, 191, 57, 99, 16, 43, 0, 0); // @ G0135_s_Graphic558_Frame_StairsUpSide_D1R
	static Frame frameStairsDownSideD1R = Frame(160, 191, 60, 98, 16, 39, 0, 0); // @ G0137_s_Graphic558_Frame_StairsDownSide_D1R
	static Frame frameFloorPitD1R = Frame(160, 223, 93, 116, 32, 24, 0, 0); // @ G0148_s_Graphic558_Frame_FloorPit_D1R
	static Frame frameCeilingPitD1R = Frame(160, 223, 8, 16, 32, 9, 0, 0); // @ G0157_s_Graphic558_Frame_CeilingPit_D1R
	static DoorFrames doorFrameD1R = DoorFrames( // @ G0187_s_Graphic558_Frames_Door_D1R
		/* { X1, X2, Y1, Y2, ByteWidth, Height, X, Y } */
		Frame(192, 223, 17, 102, 48, 88, 0, 0),   /* Closed Or Destroyed */
		Frame(192, 223, 17, 38, 48, 88, 0, 66),   /* Vertical Closed one fourth */
		Frame(192, 223, 17, 60, 48, 88, 0, 44),   /* Vertical Closed half */
		Frame(192, 223, 17, 82, 48, 88, 0, 22),   /* Vertical Closed three fourth */
		Frame(192, 203, 17, 102, 48, 88, 36, 0),  /* Left Horizontal Closed one fourth */
		Frame(192, 215, 17, 102, 48, 88, 24, 0),  /* Left Horizontal Closed half */
		Frame(192, 223, 17, 102, 48, 88, 12, 0),  /* Left Horizontal Closed three fourth */
		Frame(0, 0, 0, 0, 0, 0, 0, 0),            /* Right Horizontal Closed one fourth */
		Frame(0, 0, 0, 0, 0, 0, 0, 0),            /* Right Horizontal Closed half */
		Frame(0, 0, 0, 0, 0, 0, 0, 0)             /* Right Horizontal Closed three fourth */
	);

	int16 order;
	uint16 squareAspect[5];
	bool skip = false;

	_vm->_dungeonMan->f172_setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[k0_ElementAspect]) {
	case k19_ElementTypeStaisFront:
		if (squareAspect[k2_StairsUpAspect])
			f105_drawFloorPitOrStairsBitmapFlippedHorizontally(_g679_stairsNativeBitmapIndex_Up_Front_D1L, frameStairsUpFrontD1R);
		else
			f105_drawFloorPitOrStairsBitmapFlippedHorizontally(_g686_stairsNativeBitmapIndex_Down_Front_D1L, frameStairsDownFrontD1R);

		order = k0x0041_CellOrder_BackLeft_FrontLeft;
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k8_viewFloor_D1R); /* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		f112_drawCeilingPit(k65_ceilingPitD1L_GraphicIndice, &frameCeilingPitD1R, posX, posY, true);
		break;
	case k0_ElementTypeWall:
		f100_drawWallSetBitmap(_g700_bitmapWallSet_Wall_D1LCR, _frameWalls163[k8_ViewSquare_D1R]);
		f107_isDrawnWallOrnAnAlcove(squareAspect[k4_LeftWallOrnOrdAspect], k11_ViewWall_D1R_LEFT);
		return;
	case k18_ElementTypeStairsSide:
		if (squareAspect[k2_StairsUpAspect])
			f105_drawFloorPitOrStairsBitmapFlippedHorizontally(_g690_stairsNativeBitmapIndex_Up_Side_D1L, frameStairsUpSideD1R);
		else
			f105_drawFloorPitOrStairsBitmapFlippedHorizontally(_g691_stairsNativeBitmapIndex_Down_Side_D1L, frameStairsDownSideD1R);

		// No break on purpose
	case k16_DoorSideElemType:
		order = k0x0041_CellOrder_BackLeft_FrontLeft;
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k8_viewFloor_D1R); /* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		f112_drawCeilingPit(k65_ceilingPitD1L_GraphicIndice, &frameCeilingPitD1R, posX, posY, true);
		break;
	case k17_DoorFrontElemType:
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k8_viewFloor_D1R);
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k8_ViewSquare_D1R, k0x0018_CellOrder_DoorPass1_BackLeft);
		f100_drawWallSetBitmap(_g704_bitmapWallSet_DoorFrameTop_D1LCR, doorFrameTopD1R);
		f111_drawDoor(squareAspect[k3_DoorThingIndexAspect], squareAspect[k2_DoorStateAspect],
					  _g695_doorNativeBitmapIndex_Front_D1LCR, M75_bitmapByteCount(96, 88), k2_ViewDoorOrnament_D1LCR, &doorFrameD1R);
		order = k0x0049_CellOrder_DoorPass2_FrontLeft;
		break;
	case k2_ElementTypePit:
		f105_drawFloorPitOrStairsBitmapFlippedHorizontally(squareAspect[k2_PitInvisibleAspect] ? k59_floorPit_invisible_D1L_GraphicIndice
														   : k53_FloorPit_D1L_GraphicIndice, frameFloorPitD1R);
		// No break on purpose
	case k5_ElementTypeTeleporter:
	case k1_CorridorElemType:
		order = k0x0041_CellOrder_BackLeft_FrontLeft;
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k8_viewFloor_D1R); /* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		f112_drawCeilingPit(k65_ceilingPitD1L_GraphicIndice, &frameCeilingPitD1R, posX, posY, true);
		break;
	default:
		skip = true;
		break;
	}

	if (!skip)
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k8_ViewSquare_D1R, order);

	if ((squareAspect[k0_ElementAspect] == k5_ElementTypeTeleporter) && squareAspect[k2_TeleporterVisibleAspect])
		f113_drawField(&_fieldAspects188[k8_ViewSquare_D1R], _frameWalls163[k8_ViewSquare_D1R]._box);
}

void DisplayMan::f124_drawSquareD1C(Direction dir, int16 posX, int16 posY) {
	static Frame doorFrameTopD1C = Frame(48, 175, 14, 17, 64, 4, 0, 0); // @ G0177_s_Graphic558_Frame_DoorFrameTop_D1C
	static Frame frameStairsUpFrontD1C = Frame(32, 191, 9, 108, 80, 100, 0, 0); // @ G0117_s_Graphic558_Frame_StairsUpFront_D1C
	static Frame frameStairsDownFrontD1C = Frame(32, 191, 18, 108, 80, 91, 0, 0); // @ G0128_s_Graphic558_Frame_StairsDownFront_D1C
	static Frame frameFloorPitD1C = Frame(32, 191, 93, 116, 80, 24, 0, 0); // @ G0147_s_Graphic558_Frame_FloorPit_D1C
	static Frame frameCeilingPitD1C = Frame(32, 191, 8, 16, 80, 9, 0, 0); // @ G0156_s_Graphic558_Frame_CeilingPit_D1C
	static Box boxThievesEyeVisibleArea(0, 95, 0, 94); // @ G0107_s_Graphic558_Box_ThievesEye_VisibleArea 

	int16 order;
	uint16 squareAspect[5];
	bool skip = false;

	_vm->_dungeonMan->f172_setSquareAspect(squareAspect, dir, posX, posY);
	switch (_vm->_dungeonMan->_g285_squareAheadElement = (ElementType)squareAspect[k0_ElementAspect]) {
	case k19_ElementTypeStaisFront:
		if (squareAspect[k2_StairsUpAspect])
			f104_drawFloorPitOrStairsBitmap(_g680_stairsNativeBitmapIndex_Up_Front_D1C, frameStairsUpFrontD1C);
		else
			f104_drawFloorPitOrStairsBitmap(_g687_stairsNativeBitmapIndex_Down_Front_D1C, frameStairsDownFrontD1C);

		order = k0x3421_CellOrder_BackLeft_BackRight_FrontLeft_FrontRight;
		/* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k7_viewFloor_D1C);
		f112_drawCeilingPit(k66_ceilingPitD1C_GraphicIndice, &frameCeilingPitD1C, posX, posY, false);
		break;
	case k0_ElementTypeWall:
		_vm->_dungeonMan->_g286_isFacingAlcove = false;
		_vm->_dungeonMan->_g287_isFacingViAltar = false;
		_vm->_dungeonMan->_g288_isFacingFountain = false;
		if (_vm->_championMan->_g407_party._event73Count_ThievesEye) {
			f491_isDerivedBitmapInCache(k1_DerivedBitmapThievesEyeVisibleArea);
			f132_blitToBitmap(_g296_bitmapViewport, f492_getDerivedBitmap(k1_DerivedBitmapThievesEyeVisibleArea),
							  boxThievesEyeVisibleArea, _boxThievesEyeViewPortVisibleArea._x1, _boxThievesEyeViewPortVisibleArea._y1,
							  k112_byteWidthViewport, 48, kM1_ColorNoTransparency, 136, 95);
			byte *bitmap = f489_getNativeBitmapOrGraphic(k41_holeInWall_GraphicIndice);
			f132_blitToBitmap(bitmap, f492_getDerivedBitmap(k1_DerivedBitmapThievesEyeVisibleArea),
							  boxThievesEyeVisibleArea, 0, 0, 48, 48, k10_ColorFlesh, 95, 95);
		}
		f101_drawWallSetBitmapWithoutTransparency(_g700_bitmapWallSet_Wall_D1LCR, _frameWalls163[k6_ViewSquare_D1C]);
		if (f107_isDrawnWallOrnAnAlcove(squareAspect[k3_FrontWallOrnOrdAspect], k12_ViewWall_D1C_FRONT))
			f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k6_ViewSquare_D1C, k0x0000_CellOrder_Alcove);

		if (_vm->_championMan->_g407_party._event73Count_ThievesEye) {
			f132_blitToBitmap(f492_getDerivedBitmap(k1_DerivedBitmapThievesEyeVisibleArea),
							  _g296_bitmapViewport, _boxThievesEyeViewPortVisibleArea, 0, 0,
							  48, k112_byteWidthViewport, k9_ColorGold, 95, k136_heightViewport); /* BUG0_74 */
			f493_addDerivedBitmap(k1_DerivedBitmapThievesEyeVisibleArea);
			f480_releaseBlock(k1_DerivedBitmapThievesEyeVisibleArea | 0x8000);
		}
		return;
	case k17_DoorFrontElemType:
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k7_viewFloor_D1C);
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k6_ViewSquare_D1C, k0x0218_CellOrder_DoorPass1_BackLeft_BackRight);
		f100_drawWallSetBitmap(_g704_bitmapWallSet_DoorFrameTop_D1LCR, doorFrameTopD1C);
		f100_drawWallSetBitmap(_g708_bitmapWallSet_DoorFrameLeft_D1C, _doorFrameLeftD1C);
		f100_drawWallSetBitmap(_g710_bitmapWallSet_DoorFrameRight_D1C, _doorFrameRightD1C);
		if (((Door *)_vm->_dungeonMan->_g284_thingData[k0_DoorThingType])[squareAspect[k3_DoorThingIndexAspect]].hasButton())
			f110_drawDoorButton(_vm->M0_indexToOrdinal(k0_DoorButton), k3_viewDoorButton_D1C);

		f111_drawDoor(squareAspect[k3_DoorThingIndexAspect], squareAspect[k2_DoorStateAspect],
					  _g695_doorNativeBitmapIndex_Front_D1LCR, M75_bitmapByteCount(96, 88), k2_ViewDoorOrnament_D1LCR, _doorFrameD1C);
		order = k0x0349_CellOrder_DoorPass2_FrontLeft_FrontRight;
		break;
	case k2_ElementTypePit:
		f104_drawFloorPitOrStairsBitmap(squareAspect[k2_PitInvisibleAspect] ? k60_floorPitInvisibleD1C_GraphicIndice : k54_FloorPit_D1C_GraphicIndice, frameFloorPitD1C);
		// No break on purpose
	case k5_ElementTypeTeleporter:
	case k1_CorridorElemType:
		order = k0x3421_CellOrder_BackLeft_BackRight_FrontLeft_FrontRight;
		/* BUG0_64 Floor ornaments are drawn over open pits. There is no check to prevent drawing floor ornaments over open pits */
		f108_drawFloorOrnament(squareAspect[k4_FloorOrnOrdAspect], k7_viewFloor_D1C);
		f112_drawCeilingPit(k66_ceilingPitD1C_GraphicIndice, &frameCeilingPitD1C, posX, posY, false);
		break;
	default:
		skip = true;
		break;
	}

	if (!skip)
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k6_ViewSquare_D1C, order);

	if ((squareAspect[k0_ElementAspect] == k5_ElementTypeTeleporter) && squareAspect[k2_TeleporterVisibleAspect])
		f113_drawField(&_fieldAspects188[k6_ViewSquare_D1C], _frameWalls163[k6_ViewSquare_D1C]._box);
}

void DisplayMan::f125_drawSquareD0L(Direction dir, int16 posX, int16 posY) {
	static Frame frameStairsSideD0L = Frame(0, 15, 73, 85, 8, 13, 0, 0); // @ G0138_s_Graphic558_Frame_StairsSide_D0L
	static Frame frameFloorPitD0L = Frame(0, 31, 124, 135, 16, 12, 0, 0); // @ G0149_s_Graphic558_Frame_FloorPit_D0L
	static Frame frameCeilingPitD0L = Frame(0, 15, 0, 3, 8, 4, 0, 0); // @ G0158_s_Graphic558_Frame_CeilingPit_D0L

	uint16 squareAspect[5];
	_vm->_dungeonMan->f172_setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[k0_ElementAspect]) {
	case k0_WallElemType:
		f100_drawWallSetBitmap(_g701_bitmapWallSet_Wall_D0L, _frameWalls163[k10_ViewSquare_D0L]);
		break;
	case k1_CorridorElemType:
	case k5_TeleporterElemType:
	case k16_DoorSideElemType:
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k10_ViewSquare_D0L, k0x0002_CellOrder_BackRight);
		break;
	case k2_PitElemType:
		f104_drawFloorPitOrStairsBitmap(squareAspect[k2_PitInvisibleAspect] ? k61_floorPitInvisibleD0L_GraphicIndice : k55_FloorPit_D0L_GraphicIndice, frameFloorPitD0L);
	case k18_StairsSideElemType:
		if (squareAspect[k2_StairsUpAspect])
			f104_drawFloorPitOrStairsBitmap(_g692_stairsNativeBitmapIndex_Side_D0L, frameStairsSideD0L);
		break;
	default:
		break;
	}

	f112_drawCeilingPit(k67_ceilingPitD0L_grahicIndice, &frameCeilingPitD0L, posX, posY, false);
	if ((squareAspect[k0_ElementAspect] == k5_ElementTypeTeleporter) && squareAspect[k2_TeleporterVisibleAspect])
		f113_drawField(&_fieldAspects188[k10_ViewSquare_D0L], _frameWalls163[k10_ViewSquare_D0L]._box);
}

void DisplayMan::f126_drawSquareD0R(Direction dir, int16 posX, int16 posY) {
	static Frame frameStairsSideD0R = Frame(208, 223, 73, 85, 8, 13, 0, 0); // @ G0139_s_Graphic558_Frame_StairsSide_D0R
	static Frame frameFloorPitD0R = Frame(192, 223, 124, 135, 16, 12, 0, 0); // @ G0151_s_Graphic558_Frame_FloorPit_D0R
	static Frame frameCeilingPitD0R = Frame(208, 223, 0, 3, 8, 4, 0, 0); // @ G0160_s_Graphic558_Frame_CeilingPit_D0R

	uint16 squareAspect[5];

	_vm->_dungeonMan->f172_setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[k0_ElementAspect]) {
	case k18_ElementTypeStairsSide:
		f105_drawFloorPitOrStairsBitmapFlippedHorizontally(_g692_stairsNativeBitmapIndex_Side_D0L, frameStairsSideD0R);
		return;
	case k2_ElementTypePit:
		f105_drawFloorPitOrStairsBitmapFlippedHorizontally(squareAspect[k2_PitInvisibleAspect] ? k61_floorPitInvisibleD0L_GraphicIndice
														   : k55_FloorPit_D0L_GraphicIndice, frameFloorPitD0R);
	case k1_CorridorElemType:
	case k16_DoorSideElemType:
	case k5_ElementTypeTeleporter:
		f112_drawCeilingPit(k67_ceilingPitD0L_grahicIndice, &frameCeilingPitD0R, posX, posY, true);
		f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k11_ViewSquare_D0R, k0x0001_CellOrder_BackLeft);
		break;
	case k0_ElementTypeWall:
		f100_drawWallSetBitmap(_g702_bitmapWallSet_Wall_D0R, _frameWalls163[k11_ViewSquare_D0R]);
		return;
	}
	if ((squareAspect[k0_ElementAspect] == k5_ElementTypeTeleporter) && squareAspect[k2_TeleporterVisibleAspect])
		f113_drawField(&_fieldAspects188[k11_ViewSquare_D0R], _frameWalls163[k11_ViewSquare_D0R]._box);
}

void DisplayMan::f127_drawSquareD0C(Direction dir, int16 posX, int16 posY) {
	static Frame doorFrameD0C = Frame(96, 127, 0, 122, 16, 123, 0, 0); // @ G0172_s_Graphic558_Frame_DoorFrame_D0C
	static Frame frameStairsUpFrontD0L = Frame(0, 31, 58, 101, 16, 44, 0, 0); // @ G0119_s_Graphic558_Frame_StairsUpFront_D0L
	static Frame frameStairsDownFrontD0L = Frame(0, 31, 76, 135, 16, 60, 0, 0); // @ G0130_s_Graphic558_Frame_StairsDownFront_D0L
	static Frame frameStairsUpFrontD0R = Frame(192, 223, 58, 101, 16, 44, 0, 0); // @ G0120_s_Graphic558_Frame_StairsUpFront_D0R
	static Frame frameStairsDownFrontD0R = Frame(192, 223, 76, 135, 16, 60, 0, 0); // @ G0131_s_Graphic558_Frame_StairsDownFront_D0R
	static Frame frameFloorPitD0C = Frame(16, 207, 124, 135, 96, 12, 0, 0); // @ G0150_s_Graphic558_Frame_FloorPit_D0C
	static Frame frameCeilingPitD0C = Frame(16, 207, 0, 3, 96, 4, 0, 0); // @ G0159_s_Graphic558_Frame_CeilingPit_D0C
	static Box boxThievesEyeHoleInDoorFrame(0, 31, 19, 113); // @ G0108_s_Graphic558_Box_ThievesEye_HoleInDoorFrame 

	uint16 squareAspect[5];

	_vm->_dungeonMan->f172_setSquareAspect(squareAspect, dir, posX, posY);
	switch (squareAspect[k0_ElementAspect]) {
	case k16_DoorSideElemType:
		if (_vm->_championMan->_g407_party._event73Count_ThievesEye) {
			memmove(_g74_tmpBitmap, _g709_bitmapWallSet_DoorFrameFront, 32 * 123);
			f132_blitToBitmap(f489_getNativeBitmapOrGraphic(k41_holeInWall_GraphicIndice),
							  _g74_tmpBitmap, boxThievesEyeHoleInDoorFrame, doorFrameD0C._box._x1 - _boxThievesEyeViewPortVisibleArea._x1,
							  0, 48, 16, k9_ColorGold, 95, 123);
			f100_drawWallSetBitmap(_g74_tmpBitmap, doorFrameD0C);
		} else
			f100_drawWallSetBitmap(_g709_bitmapWallSet_DoorFrameFront, doorFrameD0C);
		break;
	case k19_ElementTypeStaisFront:
		if (squareAspect[k2_StairsUpAspect]) {
			f104_drawFloorPitOrStairsBitmap(_g681_stairsNativeBitmapIndex_Up_Front_D0C_Left, frameStairsUpFrontD0L);
			f105_drawFloorPitOrStairsBitmapFlippedHorizontally(_g681_stairsNativeBitmapIndex_Up_Front_D0C_Left, frameStairsUpFrontD0R);
		} else {
			f104_drawFloorPitOrStairsBitmap(_g688_stairsNativeBitmapIndex_Down_Front_D0C_Left, frameStairsDownFrontD0L);
			f105_drawFloorPitOrStairsBitmapFlippedHorizontally(_g688_stairsNativeBitmapIndex_Down_Front_D0C_Left, frameStairsDownFrontD0R);
		}
		break;
	case k2_ElementTypePit:
		f104_drawFloorPitOrStairsBitmap(squareAspect[k2_PitInvisibleAspect] ? k62_flootPitInvisibleD0C_graphicIndice : k56_FloorPit_D0C_GraphicIndice, frameFloorPitD0C);
		break;
	}
	f112_drawCeilingPit(k68_ceilingPitD0C_graphicIndice, &frameCeilingPitD0C, posX, posY, false);
	f115_cthulhu(Thing(squareAspect[k1_FirstGroupOrObjectAspect]), dir, posX, posY, k9_ViewSquare_D0C, k0x0021_CellOrder_BackLeft_BackRight);
	if ((squareAspect[k0_ElementAspect] == k5_ElementTypeTeleporter) && squareAspect[k2_TeleporterVisibleAspect])
		f113_drawField(&_fieldAspects188[k9_ViewSquare_D0C], _frameWalls163[k9_ViewSquare_D0C]._box);
}

void DisplayMan::f128_drawDungeon(Direction dir, int16 posX, int16 posY) {
	static Frame ceilingFrame(0, 223, 0, 28, 112, 29, 0, 0); // @ K0012_s_Frame_Ceiling
	static Frame floorFrame(0, 223, 66, 135, 112, 70, 0, 0); // @ K0013_s_Frame_Floor
	static Frame frameWallD3L2 = Frame(0, 15, 25, 73, 8, 49, 0, 0); // @ G0711_s_Graphic558_Frame_Wall_D3L2

	if (_g297_drawFloorAndCeilingRequested)
		f98_drawFloorAndCeiling();

	_g578_useByteBoxCoordinates = true;
	for (int16 i = 0; i < 6; ++i)
		_vm->_dungeonMan->_g291_dungeonViewClickableBoxes[i].setToZero();

	for (uint16 i = 0; i < 6; ++i)
		_vm->_dungeonMan->_g291_dungeonViewClickableBoxes[i]._x1 = 255;

	_g76_useFlippedWallAndFootprintsBitmap = (posX + posY + dir) & 1;
	if (_g76_useFlippedWallAndFootprintsBitmap) {
		f100_drawWallSetBitmap(_g85_bitmapCeiling, ceilingFrame);
		f99_copyBitmapAndFlipHorizontal(_g84_bitmapFloor, _g74_tmpBitmap, k112_byteWidthViewport, 70);
		f100_drawWallSetBitmap(_g74_tmpBitmap, floorFrame);

		_g698_bitmapWallSet_Wall_D3LCR = _g90_bitmapWall_D3LCR_Flipped;
		_g699_bitmapWallSet_Wall_D2LCR = _g91_bitmapWall_D2LCR_Flipped;
		_g700_bitmapWallSet_Wall_D1LCR = _g92_bitmapWall_D1LCR_Flipped;
		_g701_bitmapWallSet_Wall_D0L = _g93_bitmapWall_D0L_Flipped;
		_g702_bitmapWallSet_Wall_D0R = _g94_bitmapWall_D0R_Flipped;
	} else {
		f99_copyBitmapAndFlipHorizontal(_g85_bitmapCeiling, _g74_tmpBitmap, k112_byteWidthViewport, 29);
		f100_drawWallSetBitmap(_g74_tmpBitmap, ceilingFrame);
		f100_drawWallSetBitmap(_g84_bitmapFloor, floorFrame);
	}

	if (_vm->_dungeonMan->f153_getRelSquareType(dir, 3, -2, posX, posY) == k0_WallElemType)
		f100_drawWallSetBitmap(_g697_bitmapWallSet_Wall_D3L2, frameWallD3L2);

	if (_vm->_dungeonMan->f153_getRelSquareType(dir, 3, 2, posX, posY) == k0_WallElemType)
		f100_drawWallSetBitmap(_g696_bitmapWallSet_Wall_D3R2, _frameWallD3R2);

	int16 tmpPosX = posX;
	int16 tmpPosY = posY;
	_vm->_dungeonMan->f150_mapCoordsAfterRelMovement(dir, 4, -1, tmpPosX, tmpPosY);
	f115_cthulhu(_vm->_dungeonMan->f162_getSquareFirstObject(tmpPosX, tmpPosY), dir, tmpPosX, tmpPosY, kM2_ViewSquare_D4L, k0x0001_CellOrder_BackLeft);
	tmpPosX = posX;
	tmpPosY = posY;
	_vm->_dungeonMan->f150_mapCoordsAfterRelMovement(dir, 4, 1, tmpPosX, tmpPosY);
	f115_cthulhu(_vm->_dungeonMan->f162_getSquareFirstObject(tmpPosX, tmpPosY), dir, tmpPosX, tmpPosY, kM1_ViewSquare_D4R, k0x0001_CellOrder_BackLeft);
	tmpPosX = posX;
	tmpPosY = posY;
	_vm->_dungeonMan->f150_mapCoordsAfterRelMovement(dir, 4, 0, tmpPosX, tmpPosY);
	f115_cthulhu(_vm->_dungeonMan->f162_getSquareFirstObject(tmpPosX, tmpPosY), dir, tmpPosX, tmpPosY, kM3_ViewSquare_D4C, k0x0001_CellOrder_BackLeft);
	tmpPosX = posX;
	tmpPosY = posY;
	_vm->_dungeonMan->f150_mapCoordsAfterRelMovement(dir, 3, -1, tmpPosX, tmpPosY);
	f116_drawSquareD3L(dir, tmpPosX, tmpPosY);
	tmpPosX = posX;
	tmpPosY = posY;
	_vm->_dungeonMan->f150_mapCoordsAfterRelMovement(dir, 3, 1, tmpPosX, tmpPosY);
	f117_drawSquareD3R(dir, tmpPosX, tmpPosY);
	tmpPosX = posX;
	tmpPosY = posY;
	_vm->_dungeonMan->f150_mapCoordsAfterRelMovement(dir, 3, 0, tmpPosX, tmpPosY);
	f118_drawSquareD3C(dir, tmpPosX, tmpPosY);
	tmpPosX = posX;
	tmpPosY = posY;
	_vm->_dungeonMan->f150_mapCoordsAfterRelMovement(dir, 2, -1, tmpPosX, tmpPosY);
	f119_drawSquareD2L(dir, tmpPosX, tmpPosY);
	tmpPosX = posX;
	tmpPosY = posY;
	_vm->_dungeonMan->f150_mapCoordsAfterRelMovement(dir, 2, 1, tmpPosX, tmpPosY);
	f120_drawSquareD2R(dir, tmpPosX, tmpPosY);
	tmpPosX = posX;
	tmpPosY = posY;
	_vm->_dungeonMan->f150_mapCoordsAfterRelMovement(dir, 2, 0, tmpPosX, tmpPosY);
	f121_drawSquareD2C(dir, tmpPosX, tmpPosY);
	tmpPosX = posX;
	tmpPosY = posY;
	_vm->_dungeonMan->f150_mapCoordsAfterRelMovement(dir, 1, -1, tmpPosX, tmpPosY);
	f122_drawSquareD1L(dir, tmpPosX, tmpPosY);
	tmpPosX = posX;
	tmpPosY = posY;
	_vm->_dungeonMan->f150_mapCoordsAfterRelMovement(dir, 1, 1, tmpPosX, tmpPosY);
	f123_drawSquareD1R(dir, tmpPosX, tmpPosY);
	tmpPosX = posX;
	tmpPosY = posY;
	_vm->_dungeonMan->f150_mapCoordsAfterRelMovement(dir, 1, 0, tmpPosX, tmpPosY);
	f124_drawSquareD1C(dir, tmpPosX, tmpPosY);
	tmpPosX = posX;
	tmpPosY = posY;
	_vm->_dungeonMan->f150_mapCoordsAfterRelMovement(dir, 0, -1, tmpPosX, tmpPosY);
	f125_drawSquareD0L(dir, tmpPosX, tmpPosY);
	tmpPosX = posX;
	tmpPosY = posY;
	_vm->_dungeonMan->f150_mapCoordsAfterRelMovement(dir, 0, 1, tmpPosX, tmpPosY);
	f126_drawSquareD0R(dir, tmpPosX, tmpPosY);
	f127_drawSquareD0C(dir, posX, posY);

	if (_g76_useFlippedWallAndFootprintsBitmap) {
		_g698_bitmapWallSet_Wall_D3LCR = _g95_bitmapWall_D3LCR_Native;
		_g699_bitmapWallSet_Wall_D2LCR = _g96_bitmapWall_D2LCR_Native;
		_g700_bitmapWallSet_Wall_D1LCR = _g97_bitmapWall_D1LCR_Native;
		_g701_bitmapWallSet_Wall_D0L = _g98_bitmapWall_D0L_Native;
		_g702_bitmapWallSet_Wall_D0R = _g99_bitmapWall_D0R_Native;
	}

	f97_drawViewport((_vm->_dungeonMan->_g309_partyMapIndex != k255_mapIndexEntrance) ? 1 : 0);
	if (_vm->_dungeonMan->_g309_partyMapIndex != k255_mapIndexEntrance)
		f98_drawFloorAndCeiling();
}

void DisplayMan::f98_drawFloorAndCeiling() {
	Box box(0, 223, 0, 36);
	f135_fillBoxBitmap(_g296_bitmapViewport, box, k0_ColorBlack, k112_byteWidthViewport, k136_heightViewport);
	_g297_drawFloorAndCeilingRequested = false;
}

void DisplayMan::fillScreen(Color color) {
	memset(getCurrentVgaBuffer(), color, sizeof(byte) * _screenWidth * _screenHeight);
}

void DisplayMan::f134_fillBitmap(byte *bitmap, Color color, uint16 byteWidth, uint16 height) {
	uint16 width = byteWidth * 2;
	memset(bitmap, color, sizeof(byte) * width * height);
}

void DisplayMan::f94_loadFloorSet(FloorSet set) {
	if (_g230_currentFloorSet == set)
		return;

	_g230_currentFloorSet = set;
	int16 index = (set * k2_FloorSetGraphicCount) + k75_FirstFloorSet;
	f466_loadIntoBitmap(index, _g84_bitmapFloor);
	f466_loadIntoBitmap(index + 1, _g85_bitmapCeiling);
}

void DisplayMan::f95_loadWallSet(WallSet set) {
	if ((_g231_currentWallSet == set) && !_vm->_g523_restartGameRequest)
		return;

	_g231_currentWallSet = set;

	int16 graphicIndice = (set * k13_WallSetGraphicCount) + k77_FirstWallSet;
	f466_loadIntoBitmap(graphicIndice++, _g709_bitmapWallSet_DoorFrameFront);
	f466_loadIntoBitmap(graphicIndice++, _g708_bitmapWallSet_DoorFrameLeft_D1C);
	f466_loadIntoBitmap(graphicIndice++, _g707_bitmapWallSet_DoorFrameLeft_D2C);
	f466_loadIntoBitmap(graphicIndice++, _g706_bitmapWallSet_DoorFrameLeft_D3C);
	f466_loadIntoBitmap(graphicIndice++, _g705_bitmapWallSet_DoorFrameLeft_D3L);
	f466_loadIntoBitmap(graphicIndice++, _g704_bitmapWallSet_DoorFrameTop_D1LCR);
	f466_loadIntoBitmap(graphicIndice++, _g703_bitmapWallSet_DoorFrameTop_D2LCR);
	f466_loadIntoBitmap(graphicIndice++, _g702_bitmapWallSet_Wall_D0R);
	f466_loadIntoBitmap(graphicIndice++, _g701_bitmapWallSet_Wall_D0L);
	f466_loadIntoBitmap(graphicIndice++, _g700_bitmapWallSet_Wall_D1LCR);
	f466_loadIntoBitmap(graphicIndice++, _g699_bitmapWallSet_Wall_D2LCR);
	f466_loadIntoBitmap(graphicIndice++, _g698_bitmapWallSet_Wall_D3LCR);
	f466_loadIntoBitmap(graphicIndice++, _g697_bitmapWallSet_Wall_D3L2);

	f99_copyBitmapAndFlipHorizontal(_g708_bitmapWallSet_DoorFrameLeft_D1C, _g710_bitmapWallSet_DoorFrameRight_D1C,
									_doorFrameRightD1C._srcByteWidth, _doorFrameRightD1C._srcHeight);
	f99_copyBitmapAndFlipHorizontal(_g697_bitmapWallSet_Wall_D3L2, _g696_bitmapWallSet_Wall_D3R2,
									_frameWallD3R2._srcByteWidth, _frameWallD3R2._srcHeight);
}

void DisplayMan::f96_loadCurrentMapGraphics() {
	static Box BoxWallD3LCR = Box(0, 115, 0, 50); // @ G0161_s_Graphic558_Box_WallBitmap_D3LCR
	static Box BoxWallD2LCR = Box(0, 135, 0, 70); // @ G0162_s_Graphic558_Box_WallBitmap_D2LCR
	static byte doorOrnCoordIndices[12] = { // @ G0196_auc_Graphic558_DoorOrnamentCoordinateSetIndices
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
		1    /* Door Ornament #11 Iron Door Damages */
	};
	static byte floorOrnCoordSetIndices[9] = { // @ G0195_auc_Graphic558_FloorOrnamentCoordinateSetIndices
		0,   /* Floor Ornament 00 Square Grate */
		0,   /* Floor Ornament 01 Square Pressure Pad */
		0,   /* Floor Ornament 02 Moss */
		0,   /* Floor Ornament 03 Round Grate */
		2,   /* Floor Ornament 04 Round Pressure Plate */
		0,   /* Floor Ornament 05 Black Flame Pit */
		0,   /* Floor Ornament 06 Crack */
		2,   /* Floor Ornament 07 Tiny Pressure Pad */
		0    /* Floor Ornament 08 Puddle */
	};
	static byte g194_WallOrnCoordSetIndices[60] = { // @ G0194_auc_Graphic558_WallOrnamentCoordinateSetIndices
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
		7    /* Wall Ornament 59 Lord Order (Outside) */
	};
	static byte g192_AlcoveOrnIndices[k3_AlcoveOrnCount] = { // @ G0192_auc_Graphic558_AlcoveOrnamentIndices
		1,   /* Square Alcove */
		2,   /* Vi Altar */
		3};  /* Arched Alcove */
	static int16 g193_FountainOrnIndices[k1_FountainOrnCount] = {35}; // @ G0193_ai_Graphic558_FountainOrnamentIndices

	f94_loadFloorSet(_vm->_dungeonMan->_g269_currMap->_floorSet);
	f95_loadWallSet(_vm->_dungeonMan->_g269_currMap->_wallSet);

	_g578_useByteBoxCoordinates = true;

	f99_copyBitmapAndFlipHorizontal(_g95_bitmapWall_D3LCR_Native = _g698_bitmapWallSet_Wall_D3LCR, _g74_tmpBitmap,
									_frameWalls163[k0_ViewSquare_D3C]._srcByteWidth, _frameWalls163[k0_ViewSquare_D3C]._srcHeight);
	f134_fillBitmap(_g90_bitmapWall_D3LCR_Flipped, k10_ColorFlesh, 64, 51);
	f132_blitToBitmap(_g74_tmpBitmap, _g90_bitmapWall_D3LCR_Flipped, BoxWallD3LCR, 11, 0, 64, 64, kM1_ColorNoTransparency, 51, 51);

	f99_copyBitmapAndFlipHorizontal(_g96_bitmapWall_D2LCR_Native = _g699_bitmapWallSet_Wall_D2LCR, _g74_tmpBitmap,
									_frameWalls163[k3_ViewSquare_D2C]._srcByteWidth, _frameWalls163[k3_ViewSquare_D2C]._srcHeight);
	f134_fillBitmap(_g91_bitmapWall_D2LCR_Flipped, k10_ColorFlesh, 72, 71);
	f132_blitToBitmap(_g74_tmpBitmap, _g91_bitmapWall_D2LCR_Flipped, BoxWallD2LCR, 8, 0, 72, 72, kM1_ColorNoTransparency, 71, 71);

	f99_copyBitmapAndFlipHorizontal(_g97_bitmapWall_D1LCR_Native = _g700_bitmapWallSet_Wall_D1LCR, _g92_bitmapWall_D1LCR_Flipped,
									_frameWalls163[k6_ViewSquare_D1C]._srcByteWidth, _frameWalls163[k6_ViewSquare_D1C]._srcHeight);
	f99_copyBitmapAndFlipHorizontal(_g98_bitmapWall_D0L_Native = _g701_bitmapWallSet_Wall_D0L, _g94_bitmapWall_D0R_Flipped,
									_frameWalls163[k10_ViewSquare_D0L]._srcByteWidth, _frameWalls163[k10_ViewSquare_D0L]._srcHeight);
	f99_copyBitmapAndFlipHorizontal(_g99_bitmapWall_D0R_Native = _g702_bitmapWallSet_Wall_D0R, _g93_bitmapWall_D0L_Flipped,
									_frameWalls163[k10_ViewSquare_D0L]._srcByteWidth, _frameWalls163[k10_ViewSquare_D0L]._srcHeight);

	int16 val = _vm->_dungeonMan->_g269_currMap->_wallSet * k18_StairsGraphicCount + k90_FirstStairs;
	_g675_stairsNativeBitmapIndex_Up_Front_D3L = val++;
	_g676_stairsNativeBitmapIndex_Up_Front_D3C = val++;
	_g677_stairsNativeBitmapIndex_Up_Front_D2L = val++;
	_g678_stairsNativeBitmapIndex_Up_Front_D2C = val++;
	_g679_stairsNativeBitmapIndex_Up_Front_D1L = val++;
	_g680_stairsNativeBitmapIndex_Up_Front_D1C = val++;
	_g681_stairsNativeBitmapIndex_Up_Front_D0C_Left = val++;
	_g682_stairsNativeBitmapIndex_Down_Front_D3L = val++;
	_g683_stairsNativeBitmapIndex_Down_Front_D3C = val++;
	_g684_stairsNativeBitmapIndex_Down_Front_D2L = val++;
	_g685_stairsNativeBitmapIndex_Down_Front_D2C = val++;
	_g686_stairsNativeBitmapIndex_Down_Front_D1L = val++;
	_g687_stairsNativeBitmapIndex_Down_Front_D1C = val++;
	_g688_stairsNativeBitmapIndex_Down_Front_D0C_Left = val++;
	_g689_stairsNativeBitmapIndex_Side_D2L = val++;
	_g690_stairsNativeBitmapIndex_Up_Side_D1L = val++;
	_g691_stairsNativeBitmapIndex_Down_Side_D1L = val++;
	_g692_stairsNativeBitmapIndex_Side_D0L = val++;

	for (int16 i = 0; i < k3_AlcoveOrnCount; ++i)
		_g267_currMapAlcoveOrnIndices[i] = -1;

	for (int16 i = 0; i < k1_FountainOrnCount; ++i)
		_g268_currMapFountainOrnIndices[i] = -1;

	uint16 doorSets[2];
	doorSets[0] = _vm->_dungeonMan->_g269_currMap->_doorSet0;
	doorSets[1] = _vm->_dungeonMan->_g269_currMap->_doorSet1;
	for (uint16 doorSet = 0; doorSet <= 1; doorSet++) {
		int16 counter = k108_FirstDoorSet + (doorSets[doorSet] * k3_DoorSetGraphicsCount);
		_g693_doorNativeBitmapIndex_Front_D3LCR[doorSet] = counter++;
		_g694_doorNativeBitmapIndex_Front_D2LCR[doorSet] = counter++;
		_g695_doorNativeBitmapIndex_Front_D1LCR[doorSet] = counter++;
	}

	uint16 alcoveCount = 0;
	uint16 fountainCount = 0;
	Map &currMap = *_vm->_dungeonMan->_g269_currMap;

	_g266_currMapViAltarIndex = -1;

	for (int16 ornamentIndex = 0; ornamentIndex <= currMap._wallOrnCount; ornamentIndex++) {
		int16 greenOrn = _g261_currMapWallOrnIndices[ornamentIndex];
		int16 counter = k121_FirstWallOrn + greenOrn * 2; /* Each wall ornament has 2 graphics */
		_g101_currMapWallOrnInfo[ornamentIndex][k0_NativeBitmapIndex] = counter;
		for (int16 ornamentCounter = 0; ornamentCounter < k3_AlcoveOrnCount; ornamentCounter++) {
			if (greenOrn == g192_AlcoveOrnIndices[ornamentCounter]) {
				_g267_currMapAlcoveOrnIndices[alcoveCount++] = ornamentIndex;
				if (greenOrn == 2) /* Wall ornament #2 is the Vi Altar */
					_g266_currMapViAltarIndex = ornamentIndex;
			}
		}
		for (int16 ornamentCounter = 0; ornamentCounter < k1_FountainOrnCount; ornamentCounter++) {
			if (greenOrn == g193_FountainOrnIndices[ornamentCounter])
				_g268_currMapFountainOrnIndices[fountainCount++] = ornamentIndex;
		}

		_g101_currMapWallOrnInfo[ornamentIndex][k1_CoordinateSet] = g194_WallOrnCoordSetIndices[greenOrn];
	}


	for (uint16 i = 0; i < currMap._floorOrnCount; ++i) {
		uint16 ornIndice = _g262_currMapFloorOrnIndices[i];
		uint16 nativeIndice = k247_FirstFloorOrn + ornIndice * 6;
		_g102_currMapFloorOrnInfo[i][k0_NativeBitmapIndex] = nativeIndice;
		_g102_currMapFloorOrnInfo[i][k1_CoordinateSet] = floorOrnCoordSetIndices[ornIndice];
	}

	for (uint16 i = 0; i < currMap._doorOrnCount; ++i) {
		uint16 ornIndice = _g263_currMapDoorOrnIndices[i];
		uint16 nativeIndice = k303_FirstDoorOrn + ornIndice;
		_g103_currMapDoorOrnInfo[i][k0_NativeBitmapIndex] = nativeIndice;
		_g103_currMapDoorOrnInfo[i][k1_CoordinateSet] = doorOrnCoordIndices[ornIndice];
	}

	f93_applyCreatureReplColors(9, 8);
	f93_applyCreatureReplColors(10, 12);

	for (uint16 creatureType = 0; creatureType < currMap._creatureTypeCount; ++creatureType) {
		CreatureAspect &aspect = _creatureAspects219[_g264_currMapAllowedCreatureTypes[creatureType]];
		uint16 replColorOrdinal = aspect.getReplColour9();
		if (replColorOrdinal)
			f93_applyCreatureReplColors(9, _vm->M1_ordinalToIndex(replColorOrdinal));

		replColorOrdinal = aspect.getReplColour10();
		if (replColorOrdinal)
			f93_applyCreatureReplColors(10, _vm->M1_ordinalToIndex(replColorOrdinal));
	}

	_g297_drawFloorAndCeilingRequested = true;
	_g342_refreshDungeonViewPaleteRequested = true;
}

void DisplayMan::f93_applyCreatureReplColors(int replacedColor, int replacementColor) {
	CreatureReplColorSet creatureReplColorSets[13] = { // @ G0220_as_Graphic558_CreatureReplacementColorSets
		/* { Color, Color, Color, Color, Color, Color, D2 replacement color index (x10), D3 replacement color index (x10) } */
		CreatureReplColorSet(0x0CA0, 0x0A80, 0x0860, 0x0640, 0x0420, 0x0200,  90,  90),    /* Atari ST: { 0x0650, 0x0540, 0x0430, 0x0320, 0x0210, 0x0100,  90,  90 }, RGB colors are different */
		CreatureReplColorSet(0x0060, 0x0040, 0x0020, 0x0000, 0x0000, 0x0000,   0,   0),    /* Atari ST: { 0x0030, 0x0020, 0x0010, 0x0000, 0x0000, 0x0000,   0,   0 }, */
		CreatureReplColorSet(0x0860, 0x0640, 0x0420, 0x0200, 0x0000, 0x0000, 100, 100),    /* Atari ST: { 0x0430, 0x0320, 0x0210, 0x0100, 0x0000, 0x0000, 100, 100 }, */
		CreatureReplColorSet(0x0640, 0x0420, 0x0200, 0x0000, 0x0000, 0x0000,  90,   0),    /* Atari ST: { 0x0320, 0x0210, 0x0100, 0x0000, 0x0000, 0x0000,  90,   0 }, */
		CreatureReplColorSet(0x000A, 0x0008, 0x0006, 0x0004, 0x0002, 0x0000,  90, 100),    /* Atari ST: { 0x0005, 0x0004, 0x0003, 0x0002, 0x0001, 0x0000,  90, 100 }, */
		CreatureReplColorSet(0x0008, 0x0006, 0x0004, 0x0002, 0x0000, 0x0000, 100,   0),    /* Atari ST: { 0x0004, 0x0003, 0x0002, 0x0001, 0x0000, 0x0000, 100,   0 }, */
		CreatureReplColorSet(0x0808, 0x0606, 0x0404, 0x0202, 0x0000, 0x0000,  90,   0),    /* Atari ST: { 0x0404, 0x0303, 0x0202, 0x0101, 0x0000, 0x0000,  90,   0 }, */
		CreatureReplColorSet(0x0A0A, 0x0808, 0x0606, 0x0404, 0x0202, 0x0000, 100,  90),    /* Atari ST: { 0x0505, 0x0404, 0x0303, 0x0202, 0x0101, 0x0000, 100,  90 }, */
		CreatureReplColorSet(0x0FA0, 0x0C80, 0x0A60, 0x0840, 0x0620, 0x0400, 100,  50),    /* Atari ST: { 0x0750, 0x0640, 0x0530, 0x0420, 0x0310, 0x0200, 100,  50 }, */
		CreatureReplColorSet(0x0F80, 0x0C60, 0x0A40, 0x0820, 0x0600, 0x0200,  50,  70),    /* Atari ST: { 0x0740, 0x0630, 0x0520, 0x0410, 0x0300, 0x0100,  50,  30 }, D3 replacement color index is different */
		CreatureReplColorSet(0x0800, 0x0600, 0x0400, 0x0200, 0x0000, 0x0000, 100, 120),    /* Atari ST: { 0x0400, 0x0300, 0x0200, 0x0100, 0x0000, 0x0000, 100, 100 }, D3 replacement color index is different */
		CreatureReplColorSet(0x0600, 0x0400, 0x0200, 0x0000, 0x0000, 0x0000, 120,   0),    /* Atari ST: { 0x0300, 0x0200, 0x0100, 0x0000, 0x0000, 0x0000, 120,   0 }, */
		CreatureReplColorSet(0x0C86, 0x0A64, 0x0842, 0x0620, 0x0400, 0x0200, 100,  50)     /* Atari ST: { 0x0643, 0x0532, 0x0421, 0x0310, 0x0200, 0x0100, 100,  50 } }; */
	};

	for (int16 i = 0; i < 6; ++i)
		_palDungeonView[i][replacedColor] = creatureReplColorSets[replacementColor]._RGBColor[i];

	_palChangesCreatureD2[replacedColor] = creatureReplColorSets[replacementColor]._d2ReplacementColor;
	_palChangesCreatureD3[replacedColor] = creatureReplColorSets[replacementColor]._d3ReplacementColor;
}

void DisplayMan::f104_drawFloorPitOrStairsBitmap(uint16 nativeIndex, Frame &f) {
	if (f._srcByteWidth)
		f132_blitToBitmap(f489_getNativeBitmapOrGraphic(nativeIndex), _g296_bitmapViewport, f._box, f._srcX, f._srcY,
						f._srcByteWidth, k112_byteWidthViewport, k10_ColorFlesh, f._srcHeight, k136_heightViewport);
}

void DisplayMan::f105_drawFloorPitOrStairsBitmapFlippedHorizontally(uint16 nativeIndex, Frame &f) {
	if (f._srcByteWidth) {
		f99_copyBitmapAndFlipHorizontal(f489_getNativeBitmapOrGraphic(nativeIndex), _g74_tmpBitmap, f._srcByteWidth, f._srcHeight);
		f132_blitToBitmap(_g74_tmpBitmap, _g296_bitmapViewport, f._box, f._srcX, f._srcY, f._srcByteWidth,
						k112_byteWidthViewport, k10_ColorFlesh, f._srcHeight, k136_heightViewport);
	}
}

bool DisplayMan::f107_isDrawnWallOrnAnAlcove(int16 wallOrnOrd, ViewWall viewWallIndex) {
	static Box boxWallPatchBehindInscription = Box(110, 113, 37, 63); // @ G0202_ac_Graphic558_Box_WallPatchBehindInscription
	static const byte inscriptionLineY[4] = { // @ G0203_auc_Graphic558_InscriptionLineY
		48,   /* 1 Line  */
		59,   /* 2 lines */
		75,   /* 3 lines */
		86    /* 4 lines */
	};
	static const byte wallOrnDerivedBitmapIndexIncrement[12] = { // @ G0190_auc_Graphic558_WallOrnamentDerivedBitmapIndexIncrement
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
		4    /* D1R Left */
	};

	static byte unreadableInscriptionBoxY2[15] = { // @ G0204_auc_Graphic558_UnreadableInscriptionBoxY2
		/* { Y for 1 line, Y for 2 lines, Y for 3 lines } */
		45, 48, 53,   /* D3L Right, D3R Left */
		43, 49, 56,   /* D3L Front, D3C Front, D3R Front */
		42, 49, 56,   /* D2L Right, D2R Left */
		46, 53, 63,   /* D2L Front, D2C Front, D2R Front */
		46, 57, 68    /* D1L Right, D1R Left */
	};

	static byte g205_WallOrnCoordSets[8][13][6] = { // @ G0205_aaauc_Graphic558_WallOrnamentCoordinateSets
		/* { X1, X2, Y1, Y2, ByteWidth, Height } */
		{
			{80,  83, 41,  45,  8,   5},   /* D3L */
			{140, 143, 41,  45,  8,   5},  /* D3R */
			{16,  29, 39,  50,  8,  12},   /* D3L */
			{107, 120, 39,  50,  8,  12},  /* D3C */
			{187, 200, 39,  50,  8,  12},  /* D3R */
			{67,  77, 40,  49,  8,  10},   /* D2L */
			{146, 156, 40,  49,  8,  10},  /* D2R */
			{0,  17, 38,  55, 16,  18},    /* D2L */
			{102, 123, 38,  55, 16,  18},  /* D2C */
			{206, 223, 38,  55, 16,  18},  /* D2R */
			{48,  63, 38,  56,  8,  19},   /* D1L */
			{160, 175, 38,  56,  8,  19},  /* D1R */
			{96, 127, 36,  63, 16,  28}    /* D1C */
		},
		{
			{74,  82, 41,  60,  8,  20},   /* D3L */
			{141, 149, 41,  60,  8,  20},  /* D3R */
			{1,  47, 37,  63, 24,  27},    /* D3L */
			{88, 134, 37,  63, 24,  27},   /* D3C */
			{171, 217, 37,  63, 24,  27},  /* D3R */
			{61,  76, 38,  67,  8,  30},   /* D2L */
			{147, 162, 38,  67,  8,  30},  /* D2R */
			{0,  43, 37,  73, 32,  37},    /* D2L */
			{80, 143, 37,  73, 32,  37},   /* D2C */
			{180, 223, 37,  73, 32,  37},  /* D2R */
			{32,  63, 36,  83, 16,  48},   /* D1L */
			{160, 191, 36,  83, 16,  48},  /* D1R */
			{64, 159, 36,  91, 48,  56}    /* D1C */
		},
		{
			{80,  83, 66,  70,  8,   5},   /* D3L */
			{140, 143, 66,  70,  8,   5},  /* D3R */
			{16,  29, 64,  75,  8,  12},   /* D3L */
			{106, 119, 64,  75,  8,  12},  /* D3C */
			{187, 200, 64,  75,  8,  12},  /* D3R */
			{67,  77, 74,  83,  8,  10},   /* D2L */
			{146, 156, 74,  83,  8,  10},  /* D2R */
			{0,  17, 73,  90, 16,  18},    /* D2L */
			{100, 121, 73,  90, 16,  18},  /* D2C */
			{206, 223, 73,  90, 16,  18},  /* D2R */
			{48,  63, 84, 102,  8,  19},   /* D1L */
			{160, 175, 84, 102,  8,  19},  /* D1R */
			{96, 127, 92, 119, 16,  28}    /* D1C */
		},
		{
			{80,  83, 49,  53,  8,   5},   /* D3L */
			{140, 143, 49,  53,  8,   5},  /* D3R */
			{16,  29, 50,  61,  8,  12},   /* D3L */
			{106, 119, 50,  61,  8,  12},  /* D3C */
			{187, 200, 50,  61,  8,  12},  /* D3R */
			{67,  77, 53,  62,  8,  10},   /* D2L */
			{146, 156, 53,  62,  8,  10},  /* D2R */
			{0,  17, 55,  72, 16,  18},    /* D2L */
			{100, 121, 55,  72, 16,  18},  /* D2C */
			{206, 223, 55,  72, 16,  18},  /* D2R */
			{48,  63, 57,  75,  8,  19},   /* D1L */
			{160, 175, 57,  75,  8,  19},  /* D1R */
			{96, 127, 64,  91, 16,  28}    /* D1C */
		},
		{
			{75,  90, 40,  44,  8,   5},   /* D3L */
			{133, 148, 40,  44,  8,   5},  /* D3R */
			{1,  48, 44,  49, 24,   6},    /* D3L */
			{88, 135, 44,  49, 24,   6},   /* D3C */
			{171, 218, 44,  49, 24,   6},  /* D3R */
			{60,  77, 40,  46, 16,   7},   /* D2L */
			{146, 163, 40,  46, 16,   7},  /* D2R */
			{0,  35, 43,  50, 32,   8},    /* D2L */
			{80, 143, 43,  50, 32,   8},   /* D2C */
			{184, 223, 43,  50, 32,   8},  /* D2R */
			{32,  63, 41,  52, 16,  12},   /* D1L */
			{160, 191, 41,  52, 16,  12},  /* D1R */
			{64, 159, 41,  52, 48,  12}    /* D1C */
		},
		{
			{78,  85, 36,  51,  8,  16},   /* D3L */
			{138, 145, 36,  51,  8,  16},  /* D3R */
			{10,  41, 34,  53, 16,  20},   /* D3L */
			{98, 129, 34,  53, 16,  20},   /* D3C */
			{179, 210, 34,  53, 16,  20},  /* D3R */
			{66,  75, 34,  56,  8,  23},   /* D2L */
			{148, 157, 34,  56,  8,  23},  /* D2R */
			{0,  26, 33,  61, 24,  29},    /* D2L */
			{91, 133, 33,  61, 24,  29},   /* D2C */
			{194, 223, 33,  61, 24,  29},  /* D2R */
			{41,  56, 31,  65,  8,  35},   /* D1L */
			{167, 182, 31,  65,  8,  35},  /* D1R */
			{80, 143, 29,  71, 32,  43}    /* D1C */
		},
		{
			{75,  82, 25,  75,  8,  51},   /* D3L */
			{142, 149, 25,  75,  8,  51},  /* D3R */
			{12,  60, 25,  75, 32,  51},   /* D3L */
			{88, 136, 25,  75, 32,  51},   /* D3C */
			{163, 211, 25,  75, 32,  51},  /* D3R */
			{64,  73, 20,  90,  8,  71},   /* D2L */
			{150, 159, 20,  90,  8,  71},  /* D2R */
			{0,  38, 20,  90, 32,  71},    /* D2L */
			{82, 142, 20,  90, 32,  71},   /* D2C */
			{184, 223, 20,  90, 32,  71},  /* D2R */
			{41,  56,  9, 119,  8, 111},   /* D1L */
			{169, 184,  9, 119,  8, 111},  /* D1R */
			{64, 159,  9, 119, 48, 111}    /* D1C */
		},
		{
			{74,  85, 25,  75,  8,  51},   /* D3L */
			{137, 149, 25,  75,  8,  51},  /* D3R */
			{0,  75, 25,  75, 40,  51},    /* D3L Atari ST: {   0,  83, 25,  75, 48,  51 } */
			{74, 149, 25,  75, 40,  51},   /* D3C Atari ST: {  74, 149, 25,  75, 48,  51 } */
			{148, 223, 25,  75, 40,  51},  /* D3R Atari ST: { 139, 223, 25,  75, 48,  51 } */
			{60,  77, 20,  90, 16,  71},   /* D2L */
			{146, 163, 20,  90, 16,  71},  /* D2R */
			{0,  74, 20,  90, 56,  71},    /* D2L */
			{60, 163, 20,  90, 56,  71},   /* D2C */
			{149, 223, 20,  90, 56,  71},  /* D2R */
			{32,  63,  9, 119, 16, 111},   /* D1L */
			{160, 191,  9, 119, 16, 111},  /* D1R */
			{32, 191,  9, 119, 80, 111}    /* D1C */
		}
	};

	static Box boxChampionPortraitOnWall = Box(96, 127, 35, 63); // G0109_s_Graphic558_Box_ChampionPortraitOnWall

	if (!wallOrnOrd)
		return false;
	wallOrnOrd--;
	int16 wallOrnamentIndex = wallOrnOrd;
	int16 ornNativeBitmapIndex = _g101_currMapWallOrnInfo[wallOrnamentIndex][k0_NativeBitmapIndex];
	int16 wallOrnamentCoordinateSetIndex = _g101_currMapWallOrnInfo[wallOrnamentIndex][k1_CoordinateSet];
	byte *ornCoordSet = g205_WallOrnCoordSets[wallOrnamentCoordinateSetIndex][viewWallIndex];
	bool isAlcove = _vm->_dungeonMan->f149_isWallOrnAnAlcove(wallOrnamentIndex);
	unsigned char inscriptionString[70];
	bool isInscription = (wallOrnamentIndex == _vm->_dungeonMan->_g265_currMapInscriptionWallOrnIndex);
	if (isInscription)
		_vm->_dungeonMan->f168_decodeText((char*)inscriptionString, _g290_inscriptionThing, k0_TextTypeInscription);

	int16 blitPosX;
	byte *ornBlitBitmap;

	if (viewWallIndex >= k10_ViewWall_D1L_RIGHT) {
		if (viewWallIndex == k12_ViewWall_D1C_FRONT) {
			if (isInscription) {
				f132_blitToBitmap(_g700_bitmapWallSet_Wall_D1LCR, _g296_bitmapViewport, boxWallPatchBehindInscription, 94, 28, _frameWalls163[k6_ViewSquare_D1C]._srcByteWidth, k112_byteWidthViewport, kM1_ColorNoTransparency, _frameWalls163[k6_ViewSquare_D1C]._srcHeight, k136_heightViewport);
				byte *inscrString = inscriptionString;
				byte *L0092_puc_Bitmap = f489_getNativeBitmapOrGraphic(k120_InscriptionFont);
				int16 textLineIndex = 0;
				do {
					int16 characterCount = 0;
					byte *AL0091_puc_Character = inscrString;
					while (*AL0091_puc_Character++ < 128) { /* Hexadecimal: 0x80 (Megamax C does not support hexadecimal character constants) */
						characterCount++;
					}
					Frame blitFrame;
					blitFrame._box._x2 = (blitFrame._box._x1 = 112 - (characterCount << 2)) + 7;
					blitFrame._box._y1 = (blitFrame._box._y2 = inscriptionLineY[textLineIndex++]) - 7;
					while (characterCount--) {
						f132_blitToBitmap(L0092_puc_Bitmap, _g296_bitmapViewport, blitFrame._box, *inscrString++ << 3, 0, k144_byteWidth, k112_byteWidthViewport, k10_ColorFlesh, 8, k136_heightViewport);
						blitFrame._box._x1 += 8;
						blitFrame._box._x2 += 8;
					}
				} while (*inscrString++ != 129); /* Hexadecimal: 0x81 (Megamax C does not support hexadecimal character constants) */
				return isAlcove;
			}
			ornNativeBitmapIndex++;
			{
				Box tmpBox(ornCoordSet);
				_vm->_dungeonMan->_g291_dungeonViewClickableBoxes[k5_ViewCellDoorButtonOrWallOrn] = tmpBox;
			}
			_vm->_dungeonMan->_g286_isFacingAlcove = isAlcove;
			_vm->_dungeonMan->_g287_isFacingViAltar =
				(wallOrnamentIndex == _g266_currMapViAltarIndex);
			_vm->_dungeonMan->_g288_isFacingFountain = false;
			for (int16 idx = 0; idx < k1_FountainOrnCount; idx++) {
				if (_g268_currMapFountainOrnIndices[idx] == wallOrnamentIndex) {
					_vm->_dungeonMan->_g288_isFacingFountain = true;
					break;
				}
			}
		}
		ornBlitBitmap = f489_getNativeBitmapOrGraphic(ornNativeBitmapIndex);
		if (viewWallIndex == k11_ViewWall_D1R_LEFT) {
			f99_copyBitmapAndFlipHorizontal(ornBlitBitmap, _g74_tmpBitmap, ornCoordSet[4], ornCoordSet[5]);
			ornBlitBitmap = _g74_tmpBitmap;
		}
		blitPosX = 0;
	} else {
		int16 coordinateSetOffset = 0;
		bool flipHorizontal = (viewWallIndex == k6_ViewWall_D2R_LEFT) || (viewWallIndex == k1_ViewWall_D3R_LEFT);
		if (flipHorizontal)
			ornBlitBitmap = g205_WallOrnCoordSets[wallOrnamentCoordinateSetIndex][k11_ViewWall_D1R_LEFT];
		else if ((viewWallIndex == k5_ViewWall_D2L_RIGHT) || (viewWallIndex == k0_ViewWall_D3L_RIGHT))
			ornBlitBitmap = g205_WallOrnCoordSets[wallOrnamentCoordinateSetIndex][k10_ViewWall_D1L_RIGHT];
		else {
			ornNativeBitmapIndex++;
			ornBlitBitmap = g205_WallOrnCoordSets[wallOrnamentCoordinateSetIndex][k12_ViewWall_D1C_FRONT];
			if (viewWallIndex == k7_ViewWall_D2L_FRONT)
				coordinateSetOffset = 6;
			else if (viewWallIndex == k9_ViewWall_D2R_FRONT)
				coordinateSetOffset = -6;
		}
		blitPosX = (ornCoordSet + coordinateSetOffset)[1] - (ornCoordSet + coordinateSetOffset)[0];
		if (!f491_isDerivedBitmapInCache(wallOrnamentIndex = k4_DerivedBitmapFirstWallOrnament + (wallOrnamentIndex << 2) + wallOrnDerivedBitmapIndexIncrement[viewWallIndex])) {
			byte *blitBitmap = f489_getNativeBitmapOrGraphic(ornNativeBitmapIndex);
			f129_blitToBitmapShrinkWithPalChange(blitBitmap, f492_getDerivedBitmap(wallOrnamentIndex), ornBlitBitmap[4] << 1, ornBlitBitmap[5], ornCoordSet[4] << 1, ornCoordSet[5], (viewWallIndex <= k4_ViewWall_D3R_FRONT) ? _palChangesDoorButtonAndWallOrnD3 : _palChangesDoorButtonAndWallOrnD2);
			f493_addDerivedBitmap(wallOrnamentIndex);
		}
		ornBlitBitmap = f492_getDerivedBitmap(wallOrnamentIndex);
		if (flipHorizontal) {
			f99_copyBitmapAndFlipHorizontal(ornBlitBitmap, _g74_tmpBitmap, ornCoordSet[4], ornCoordSet[5]);
			ornBlitBitmap = _g74_tmpBitmap;
			blitPosX = 15 - (blitPosX & 0x000F);
		} else if (viewWallIndex == k7_ViewWall_D2L_FRONT) 
			blitPosX -= ornCoordSet[1] - ornCoordSet[0];
		else
			blitPosX = 0;
	}
	byte byteFrame[6];
	if (isInscription) {
		byte *blitBitmap = ornCoordSet;
		byte *inscrString2 = inscriptionString;
		int16 unreadableTextLineCount = 0;
		do {
			while (*inscrString2 < 128) { /* Hexadecimal: 0x80 (Megamax C does not support hexadecimal character constants) */
				inscrString2++;
			}
			unreadableTextLineCount++;
		} while (*inscrString2++ != 129); /* Hexadecimal: 0x81 (Megamax C does not support hexadecimal character constants) */
		ornCoordSet = blitBitmap;
		if (unreadableTextLineCount < 4) {
			for (uint16 i = 0; i < 6; ++i)
				byteFrame[i] = ornCoordSet[i];
			ornCoordSet = byteFrame;
			ornCoordSet[3] = unreadableInscriptionBoxY2[wallOrnDerivedBitmapIndexIncrement[viewWallIndex] * 3 + unreadableTextLineCount - 1];
		}
	}

	Box tmpBox(ornCoordSet);
	f132_blitToBitmap(ornBlitBitmap, _g296_bitmapViewport, tmpBox,
		blitPosX, 0,
		ornCoordSet[4], k112_byteWidthViewport, k10_ColorFlesh, ornCoordSet[5], k136_heightViewport);

	if ((viewWallIndex == k12_ViewWall_D1C_FRONT) && _g289_championPortraitOrdinal--) {
		f132_blitToBitmap(f489_getNativeBitmapOrGraphic(k26_ChampionPortraitsIndice), _g296_bitmapViewport, boxChampionPortraitOnWall,
			(_g289_championPortraitOrdinal & 0x0007) << 5, (_g289_championPortraitOrdinal >> 3) * 29,
			k128_byteWidth, k112_byteWidthViewport, k1_ColorDarkGary, 87, k136_heightViewport); /* A portrait is 32x29 pixels */
	}

	return isAlcove;
}

void DisplayMan::f129_blitToBitmapShrinkWithPalChange(byte *srcBitmap, byte *destBitmap,
													  int16 srcPixelWidth, int16 srcHeight,
													  int16 destPixelWidth, int16 destHeight, byte *palChange) {
	warning(false, "DUMMY CODE: f129_blitToBitmapShrinkWithPalChange");
	warning(false, "MISSING CODE: No palette change takes place in f129_blitToBitmapShrinkWithPalChange");


	if (destPixelWidth % 8)
		destPixelWidth = (destPixelWidth / 8) * 8 + 8;

	uint32 scaleX = (kScaleThreshold * srcPixelWidth) / destPixelWidth;
	uint32 scaleY = (kScaleThreshold * srcHeight) / destHeight;

	// Loop through drawing output lines
	for (uint32 destY = 0, scaleYCtr = 0; destY < (uint32)destHeight; ++destY, scaleYCtr += scaleY) {

		const byte *srcLine = &srcBitmap[(scaleYCtr / kScaleThreshold) * srcPixelWidth];
		byte *destLine = &destBitmap[destY * destPixelWidth];

		// Loop through drawing the pixels of the row
		for (uint32 destX = 0, xCtr = 0, scaleXCtr = 0; destX < (uint32)destPixelWidth; ++destX, ++xCtr, scaleXCtr += scaleX)
			destLine[xCtr] = srcLine[scaleXCtr / kScaleThreshold];
	}
}

byte *DisplayMan::f489_getNativeBitmapOrGraphic(uint16 index) {
	return _bitmaps[index];
}

Common::MemoryReadStream DisplayMan::getCompressedData(uint16 index) {
	return Common::MemoryReadStream(_packedBitmaps + _packedItemPos[index], getCompressedDataSize(index), DisposeAfterUse::NO);
}

uint32 DisplayMan::getCompressedDataSize(uint16 index) {
	return _packedItemPos[index + 1] - _packedItemPos[index];
}

void DisplayMan::f113_drawField(FieldAspect* fieldAspect, Box& box) {
	byte *bitmapMask = nullptr;

	if (fieldAspect->_mask != kMaskFieldAspectNoMask) {
		bitmapMask = _g74_tmpBitmap;
		memmove(bitmapMask, f489_getNativeBitmapOrGraphic(k69_FieldMask_D3R_GraphicIndice + getFlag(fieldAspect->_mask, kMaskFieldAspectIndex)),
				fieldAspect->_height * fieldAspect->_byteWidth * 2);
		if (getFlag(fieldAspect->_mask, kMaskFieldAspectFlipMask)) {
			f130_flipBitmapHorizontal(bitmapMask, fieldAspect->_byteWidth, fieldAspect->_height);
		}
	}

	f491_isDerivedBitmapInCache(k0_DerivedBitmapViewport);
	byte *bitmap = f489_getNativeBitmapOrGraphic(k73_FieldTeleporterGraphicIndice + fieldAspect->_nativeBitmapRelativeIndex);
	f133_blitBoxFilledWithMaskedBitmap(bitmap, _g296_bitmapViewport, bitmapMask, f492_getDerivedBitmap(k0_DerivedBitmapViewport), box,
									   _vm->getRandomNumber(2) + fieldAspect->_baseStartUnitIndex, _vm->getRandomNumber(32), k112_byteWidthViewport,
									   (Color)fieldAspect->_transparentColor, fieldAspect->_xPos, 0, 136, fieldAspect->_bitplaneWordCount);
	f493_addDerivedBitmap(k0_DerivedBitmapViewport);
	f480_releaseBlock(k0_DerivedBitmapViewport | 0x8000);
}

int16 DisplayMan::f459_getScaledBitmapByteCount(int16 byteWidth, int16 height, int16 scale) {
	return M77_getNormalizedByteWidth(M78_getScaledDimension(byteWidth, scale)) * M78_getScaledDimension(height, scale);
}

int16 DisplayMan::M78_getScaledDimension(int16 dimension, int16 scale) {
	return (dimension * scale + scale / 2) / 32;
}

void DisplayMan::f115_cthulhu(Thing thingParam, Direction directionParam, int16 mapXpos,
							  int16 mapYpos, int16 viewSquareIndex, uint16 orderedViewCellOrdinals) {
	int16 AL_0_creatureGraphicInfoRed;
	int16 AL_0_creatureIndexRed;
#define AL_1_viewSquareExplosionIndex viewSquareIndex
	int16 L0126_i_Multiple;
#define AL_2_viewCell      L0126_i_Multiple
#define AL_2_cellPurpleMan          L0126_i_Multiple
#define AL_2_explosionSize L0126_i_Multiple
	int16 L0127_i_Multiple;
#define AL_4_thingType            L0127_i_Multiple
#define AL_4_nativeBitmapIndex    L0127_i_Multiple
#define AL_4_xPos                    L0127_i_Multiple
#define AL_4_groupCells           L0127_i_Multiple
#define AL_4_normalizdByteWidth  L0127_i_Multiple
#define AL_4_yPos                    L0127_i_Multiple
#define AL_4_projectileAspect     L0127_i_Multiple
#define AL_4_explosionType        L0127_i_Multiple
#define AL_4_explosionAspectIndex L0127_i_Multiple
	ObjectAspect* objectAspect;
	uint32 remainingViewCellOrdinalsToProcess;
	byte *coordinateSet;
	int16 derivedBitmapIndex = -1;
	bool L0135_B_DrawAlcoveObjects;
	int16 byteWidth;
	int16 heightRedEagle;
	int16 viewLane; /* The lane (center/left/right) that the specified square is part of */
	int16 cellYellowBear;
	int16 paddingPixelCount;
	int16 heightGreenGoat;
	bool useAlcoveObjectImage; /* true for objects that have a special graphic when drawn in an alcove, like the Chest */
	bool flipHorizontal;
	bool drawingGrabbableObject;
	Box boxByteGreen;
	Thing firstThingToDraw; /* Initialized to thingParam and never changed afterwards. Used as a backup of the specified first object to draw */
	uint16 L0147_ui_Multiple;
#define AL_10_viewSquareIndexBackup L0147_ui_Multiple
#define AL_10_explosionScaleIndex   L0147_ui_Multiple
	int16 cellCounter;
	uint16 objectShiftIndex;
	uint16 L0150_ui_Multiple = 0;
#define AL_8_shiftSetIndex        L0150_ui_Multiple
#define AL_8_projectileScaleIndex L0150_ui_Multiple
	CreatureAspect* creatureAspectStruct;
	int16 creatureSize;
	int16 creatureDirectionDelta;
	int16 creatureGraphicInfoGreen;
	int16 creatureAspectInt;
	int16 creatureIndexGreen;
	int16 transparentColor;
	int16 creaturePaddingPixelCount;
	bool drawingLastBackRowCell;
	bool useCreatureSideBitmap;
	bool useCreatureBackBitmap;
	bool useCreatureSpecialD2FrontBitmap;
	bool useCreatureAttackBitmap;
	bool useFlippedHorizontallyCreatureFrontImage;
	bool drawCreaturesCompleted; /* Set to true when the last creature that the function should draw is being drawn. This is used to avoid processing the code to draw creatures for the remaining square cells */
	int16 doorFrontViewDrawingPass; /* Value 0, 1 or 2 */
	int16 projectilePosX;
	int16 projectileDirection;
	int16 projectileAspectType;
	int16 projectileBitmapIndexDelta;
	bool drawProjectileAsObject; /* When true, the code section to draw an object is called (with a goto) to draw the projectile, then the code section goes back to projectile processing with another goto */
	uint16 currentViewCellToDraw = 0;
	bool projectileFlipVertical = false;

	/* This is the full dungeon view */
	static Box boxExplosionPatternD0C = Box(0, 223, 0, 135); // @ G0105_s_Graphic558_Box_ExplosionPattern_D0C 

	static byte explosionBaseScales[5] = { // @ G0216_auc_Graphic558_ExplosionBaseScales
		10,  /* D4 */
		16,  /* D3 */
		23,  /* D2 */
		32,  /* D1 */
		32   /* D0 */
	};

	static byte objectPileShiftSetIndices[16][2] = { // @ G0217_aauc_Graphic558_ObjectPileShiftSetIndices
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
		{5, 3}
	};

	static byte objectCoordinateSets[3][10][5][2] = { // @ G0218_aaaauc_Graphic558_ObjectCoordinateSets
	/* { {X, Y }, {X, Y }, {X, Y }, {X, Y }, {X, Y } } */
		{
			{{  0,   0}, {  0,   0}, {125,  72}, { 95,  72}, {112, 64}},     /* D3C */
			{{  0,   0}, {  0,   0}, { 62,  72}, { 25,  72}, { 24, 64}},     /* D3L */
			{{  0,   0}, {  0,   0}, {200,  72}, {162,  72}, {194, 64}},     /* D3R */
			{{ 92,  78}, {132,  78}, {136,  86}, { 88,  86}, {112, 74}},     /* D2C */
			{{ 10,  78}, { 53,  78}, { 41,  86}, {  0,   0}, {  3, 74}},     /* D2L */
			{{171,  78}, {218,  78}, {  0,   0}, {183,  86}, {219, 74}},     /* D2R */
			{{ 83,  96}, {141,  96}, {148, 111}, { 76, 111}, {112, 94}},     /* D1C */
			{{  0,   0}, { 26,  96}, {  5, 111}, {  0,   0}, {  0,  0}},     /* D1L */
			{{197,  96}, {  0,   0}, {  0,   0}, {220, 111}, {  0,  0}},     /* D1R */
			{{ 66, 131}, {158, 131}, {  0,   0}, {  0,   0}, {  0,  0}}      /* D0C */
		},
		{
			{{  0,   0}, {  0,   0}, {125,  72}, { 95,  72}, {112, 63}},     /* D3C */
			{{  0,   0}, {  0,   0}, { 62,  72}, { 25,  72}, { 24, 63}},     /* D3L */
			{{  0,   0}, {  0,   0}, {200,  72}, {162,  72}, {194, 63}},     /* D3R */
			{{ 92,  78}, {132,  78}, {136,  86}, { 88,  86}, {112, 73}},     /* D2C */
			{{ 10,  78}, { 53,  78}, { 41,  86}, {  0,   0}, {  3, 73}},     /* D2L */
			{{171,  78}, {218,  78}, {  0,   0}, {183,  86}, {219, 73}},     /* D2R */
			{{ 83,  96}, {141,  96}, {148, 111}, { 76, 111}, {112, 89}},     /* D1C */
			{{  0,   0}, { 26,  96}, {  5, 111}, {  0,   0}, {  0,  0}},     /* D1L */
			{{197,  96}, {  0,   0}, {  0,   0}, {220, 111}, {  0,  0}},     /* D1R */
			{{ 66, 131}, {158, 131}, {  0,   0}, {  0,   0}, {  0,  0}}      /* D0C */
		},
		{
			{{  0,   0}, {  0,   0}, {125,  75}, { 95,  75}, {112, 65}},     /* D3C */
			{{  0,   0}, {  0,   0}, { 62,  75}, { 25,  75}, { 24, 65}},     /* D3L */
			{{  0,   0}, {  0,   0}, {200,  75}, {162,  75}, {194, 65}},     /* D3R */
			{{ 92,  81}, {132,  81}, {136,  88}, { 88,  88}, {112, 76}},     /* D2C */
			{{ 10,  81}, { 53,  81}, { 41,  88}, {  0,   0}, {  3, 76}},     /* D2L */
			{{171,  81}, {218,  81}, {  0,   0}, {183,  88}, {219, 76}},     /* D2R */
			{{ 83,  98}, {141,  98}, {148, 115}, { 76, 115}, {112, 98}},     /* D1C */
			{{  0,   0}, { 26,  98}, {  5, 115}, {  0,   0}, {  0,  0}},     /* D1L */
			{{197,  98}, {  0,   0}, {  0,   0}, {220, 115}, {  0,  0}},     /* D1R */
			{{ 66, 135}, {158, 135}, {  0,   0}, {  0,   0}, {  0,  0}}      /* D0C */
		}
	};

	static int16 shiftSets[3][8] = { // @ G0223_aac_Graphic558_ShiftSets
		{0, 1, 2, 3, 0, -3, -2, -1},   /* D0 Back or D1 Front */
		{0, 1, 1, 2, 0, -2, -1, -1},   /* D1 Back or D2 Front */
		{0, 1, 1, 1, 0, -1, -1, -1}    /* D2 Back or D3 Front */
	};

	static byte creatureCoordinateSets[3][11][5][2] = { // @ G0224_aaaauc_Graphic558_CreatureCoordinateSets
	/* { { X, Y }, { X, Y }, { X, Y }, { X, Y }, { X, Y } } */
		{
			{{ 95,  70}, {127,  70}, {129,  75}, { 93,  75}, {111,  72}},     /* D3C */
			{{131,  70}, {163,  70}, {158,  75}, {120,  75}, {145,  72}},     /* D3L */
			{{ 59,  70}, { 91,  70}, {107,  75}, { 66,  75}, { 79,  72}},     /* D3R */
			{{ 92,  81}, {131,  81}, {132,  90}, { 91,  90}, {111,  85}},     /* D2C */
			{{ 99,  81}, {146,  81}, {135,  90}, { 80,  90}, {120,  85}},     /* D2L */
			{{ 77,  81}, {124,  81}, {143,  90}, { 89,  90}, {105,  85}},     /* D2R */
			{{ 83, 103}, {141, 103}, {148, 119}, { 76, 119}, {109, 111}},     /* D1C */
			{{ 46, 103}, {118, 103}, {101, 119}, {  0,   0}, { 79, 111}},     /* D1L */
			{{107, 103}, {177, 103}, {  0,   0}, {123, 119}, {144, 111}},     /* D1R */
			{{  0,   0}, { 67, 135}, {  0,   0}, {  0,   0}, {  0,   0}},     /* D0L */
			{{156, 135}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}}      /* D0R */
		},
		{
			{{ 94,  75}, {128,  75}, {111,  70}, {111,  72}, {111,  75}},     /* D3C */
			{{120,  75}, {158,  75}, {149,  70}, {145,  72}, {150,  75}},     /* D3L */
			{{ 66,  75}, {104,  75}, { 75,  70}, { 79,  72}, { 73,  75}},     /* D3R */
			{{ 91,  90}, {132,  90}, {111,  83}, {111,  85}, {111,  90}},     /* D2C */
			{{ 80,  90}, {135,  90}, {125,  83}, {120,  85}, {125,  90}},     /* D2L */
			{{ 89,  90}, {143,  90}, { 99,  83}, {105,  85}, { 98,  90}},     /* D2R */
			{{ 81, 119}, {142, 119}, {111, 105}, {111, 111}, {111, 119}},     /* D1C */
			{{  0,   0}, {101, 119}, { 84, 105}, { 70, 111}, { 77, 119}},     /* D1L */
			{{123, 119}, {  0,   0}, {139, 105}, {153, 111}, {146, 119}},     /* D1R */
			{{  0,   0}, { 83, 130}, { 57, 121}, { 47, 126}, { 57, 130}},     /* D0L */
			{{140, 130}, {  0,   0}, {166, 121}, {176, 126}, {166, 130}}      /* D0R */
		},
		{
			{{ 95,  59}, {127,  59}, {129,  61}, { 93,  61}, {111,  60}},     /* D3C */
			{{131,  59}, {163,  59}, {158,  61}, {120,  61}, {145,  60}},     /* D3L */
			{{ 59,  59}, { 91,  59}, {107,  61}, { 66,  61}, { 79,  60}},     /* D3R */
			{{ 92,  65}, {131,  65}, {132,  67}, { 91,  67}, {111,  66}},     /* D2C */
			{{ 99,  65}, {146,  65}, {135,  67}, { 80,  67}, {120,  66}},     /* D2L */
			{{ 77,  65}, {124,  65}, {143,  67}, { 89,  67}, {105,  66}},     /* D2R */
			{{ 83,  79}, {141,  79}, {148,  85}, { 76,  85}, {111,  81}},     /* D1C */
			{{ 46,  79}, {118,  79}, {101,  85}, {  0,   0}, { 79,  81}},     /* D1L */
			{{107,  79}, {177,  79}, {  0,   0}, {123,  85}, {144,  81}},     /* D1R */
			{{  0,   0}, { 67,  96}, {  0,   0}, {  0,   0}, {  0,   0}},     /* D0L */
			{{156,  96}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}}      /* D0R */
		}
	};

	static int16 explosionCoordinatesArray[15][2][2] = { // @ G0226_aaai_Graphic558_ExplosionCoordinates
	/* { { Front Left X, Front Left Y }, { Front Right X, Front Right Y } } */
		{{100, 47}, {122, 47}},   /* D4C */
		{{ 52, 47}, { 76, 47}},   /* D4L */
		{{148, 47}, {172, 47}},   /* D4R */
		{{ 95, 50}, {127, 50}},   /* D3C */
		{{ 31, 50}, { 63, 50}},   /* D3L */
		{{159, 50}, {191, 50}},   /* D3R */
		{{ 92, 53}, {131, 53}},   /* D2C */
		{{ -3, 53}, { 46, 53}},   /* D2L */
		{{177, 53}, {226, 53}},   /* D2R */
		{{ 83, 57}, {141, 57}},   /* D1C */
		{{-54, 57}, { 18, 57}},   /* D1L */
		{{207, 57}, {277, 57}},   /* D1R */
		{{  0,  0}, {  0,  0}},   /* D0C */
		{{-73, 60}, {-33, 60}},   /* D0L */
		{{256, 60}, {296, 60}}    /* D0R */
	};

	static int16 rebirthStep2ExplosionCoordinates[7][3] = { // @ G0227_aai_Graphic558_RebirthStep2ExplosionCoordinates
	/* { X, Y, Scale } */
		{113, 57, 12},   /* D3C */
		{ 24, 57, 12},   /* D3L */
		{195, 57, 12},   /* D3R */
		{111, 63, 16},   /* D2C */
		{ 12, 63, 16},   /* D2L */
		{213, 63, 16},   /* D2R */
		{112, 76, 24}    /* D1C */
	};

	static int16 rebirthStep1ExplosionCoordinates[7][3] = { // @ G0228_aai_Graphic558_RebirthStep1ExplosionCoordinates
	/* { X, Y, Scale } */
		{112, 53, 15},   /* D3C */
		{ 24, 53, 15},   /* D3L */
		{194, 53, 15},   /* D3R */
		{112, 59, 20},   /* D2C */
		{ 15, 59, 20},   /* D2L */
		{208, 59, 20},   /* D2R */
		{112, 70, 32} /* D1C */
	};

	static int16 centeredExplosionCoordinates[15][2] = { // @ G0225_aai_Graphic558_CenteredExplosionCoordinates
		/* { X, Y } */
		{111, 47},   /* D4C */
		{ 57, 47},   /* D4L */
		{167, 47},   /* D4R */
		{111, 50},   /* D3C */
		{ 45, 50},   /* D3L */
		{179, 50},   /* D3R */
		{111, 53},   /* D2C */
		{ 20, 53},   /* D2L */
		{205, 53},   /* D2R */
		{111, 57},   /* D1C */
		{-30, 57},   /* D1L */
		{253, 57},   /* D1R */
		{111, 60},   /* D0C */
		{-53, 60},   /* D0L */
		{276, 60}    /* D0R */
	};

	if (thingParam == Thing::_endOfList)
		return;

	Group *group = nullptr;
	Thing groupThing = Thing::_none;
	bool squareHasExplosion = drawCreaturesCompleted = false;
	bool squareHasProjectile = false;
	cellCounter = 0;
	firstThingToDraw = thingParam;
	if (getFlag(orderedViewCellOrdinals, k0x0008_CellOrder_DoorFront)) { /* If the function call is to draw objects on a door square viewed from the front */
		doorFrontViewDrawingPass = (orderedViewCellOrdinals & 0x0001) + 1; /* Two function calls are made in that case to draw objects on both sides of the door frame. The door and its frame are drawn between the two calls. This value indicates the drawing pass so that creatures are drawn in the right order and so that Fluxcages are not drawn twice */
		orderedViewCellOrdinals >>= 4; /* Remove the first nibble that was used for the door front view pass */
	} else
		doorFrontViewDrawingPass = 0; /* The function call is not to draw objects on a door square viewed from the front */

	L0135_B_DrawAlcoveObjects = !(remainingViewCellOrdinalsToProcess = orderedViewCellOrdinals);
	AL_10_viewSquareIndexBackup = viewSquareIndex;
	viewLane = (viewSquareIndex + 3) % 3;
	bool twoHalfSquareCreaturesFrontView;
	byte *bitmapRedBanana;
	byte *bitmapGreenAnt;
	do {
		/* Draw objects */
		if (L0135_B_DrawAlcoveObjects) {
			AL_2_viewCell = k4_ViewCellAlcove; /* Index of coordinates to draw objects in alcoves */
			cellYellowBear = returnOppositeDir(directionParam); /* Alcove is on the opposite direction of the viewing direction */
			objectShiftIndex = 2;
		} else {
			AL_2_viewCell = _vm->M1_ordinalToIndex((int16)remainingViewCellOrdinalsToProcess & 0x000F); /* View cell is the index of coordinates to draw object */
			currentViewCellToDraw = AL_2_viewCell;
			remainingViewCellOrdinalsToProcess >>= 4; /* Proceed to the next cell ordinal */
			cellCounter++;
			cellYellowBear = M21_normalizeModulo4(AL_2_viewCell + directionParam); /* Convert view cell to absolute cell */
			thingParam = firstThingToDraw;
			viewSquareIndex = AL_10_viewSquareIndexBackup; /* Restore value as it may have been modified while drawing a creature */
			objectShiftIndex = 0;
		}
		objectShiftIndex += (cellYellowBear & 0x0001) << 3;
		drawProjectileAsObject = false;
		do {
			if ((AL_4_thingType = thingParam.getType()) == k4_GroupThingType) {
				groupThing = thingParam;
				continue;
			}

			if (AL_4_thingType == k14_ProjectileThingType) {
				squareHasProjectile = true;
				continue;
			}

			if (AL_4_thingType == k15_ExplosionThingType) {
				squareHasExplosion = true;
				continue;
			}

			if ((viewSquareIndex >= k0_ViewSquare_D3C) && (viewSquareIndex <= k9_ViewSquare_D0C) && (thingParam.getCell() == cellYellowBear)) { /* Square where objects are visible and object is located on cell being processed */
				objectAspect = &(_objectAspects209[_vm->_dungeonMan->_objectInfo[_vm->_dungeonMan->f141_getObjectInfoIndex(thingParam)]._objectAspectIndex]);
				AL_4_nativeBitmapIndex = k360_FirstObjectGraphicIndice + objectAspect->_firstNativeBitmapRelativeIndex;
				useAlcoveObjectImage = (L0135_B_DrawAlcoveObjects && getFlag(objectAspect->_graphicInfo, k0x0010_ObjectAlcoveMask) && !viewLane);
				if (useAlcoveObjectImage)
					AL_4_nativeBitmapIndex++;

				coordinateSet = objectCoordinateSets[objectAspect->_coordinateSet][viewSquareIndex][AL_2_viewCell];
				if (!coordinateSet[1]) /* If object is not visible */
					continue;
T0115015_DrawProjectileAsObject:
				flipHorizontal = getFlag(objectAspect->_graphicInfo, k0x0001_ObjectFlipOnRightMask) &&
					!useAlcoveObjectImage &&
					((viewLane == k2_ViewLaneRight) || (!viewLane && ((AL_2_viewCell == k1_ViewCellFrontRight) || (AL_2_viewCell == k2_ViewCellBackRight))));
				/* Flip horizontally if object graphic requires it and is not being drawn in an alcove and the object is either on the right lane or on the right column of the center lane */
				paddingPixelCount = 0;

				if ((viewSquareIndex == k9_ViewSquare_D0C) || ((viewSquareIndex >= k6_ViewSquare_D1C) && (AL_2_viewCell >= k2_ViewCellBackRight))) {
					drawingGrabbableObject = (!viewLane && !drawProjectileAsObject); /* If object is in the center lane (only D0C or D1C with condition above) and is not a projectile */
					AL_8_shiftSetIndex = k0_ShiftSet_D0BackD1Front;
					bitmapRedBanana = f489_getNativeBitmapOrGraphic(AL_4_nativeBitmapIndex); /* Use base graphic, no resizing */
					byteWidth = objectAspect->_byteWidth;
					heightRedEagle = objectAspect->_height;
					if (flipHorizontal) {
						f99_copyBitmapAndFlipHorizontal(bitmapRedBanana, _g74_tmpBitmap, byteWidth, heightRedEagle);
						bitmapRedBanana = _g74_tmpBitmap;
					}
				} else {
					drawingGrabbableObject = false;
					derivedBitmapIndex = k104_DerivedBitmapFirstObject + objectAspect->_firstDerivedBitmapRelativeIndex;
					byte* paletteChanges;
					if ((viewSquareIndex >= k6_ViewSquare_D1C) || ((viewSquareIndex >= k3_ViewSquare_D2C) && (AL_2_viewCell >= k2_ViewCellBackRight))) {
						derivedBitmapIndex++;
						AL_8_shiftSetIndex = k1_ShiftSet_D1BackD2Front;
						byteWidth = M78_getScaledDimension(objectAspect->_byteWidth, k20_Scale_D2);
						heightRedEagle = M78_getScaledDimension(objectAspect->_height, k20_Scale_D2);
						paletteChanges = _palChangesFloorOrnD2;
					} else {
						AL_8_shiftSetIndex = k2_ShiftSet_D2BackD3Front;
						byteWidth = M78_getScaledDimension(objectAspect->_byteWidth, k16_Scale_D3);
						heightRedEagle = M78_getScaledDimension(objectAspect->_height, k16_Scale_D3);
						paletteChanges = _palChangesFloorOrnD3;
					}
					if (flipHorizontal) {
						derivedBitmapIndex += 2;
						paddingPixelCount = (7 - ((byteWidth - 1) & 0x0007)) << 1;
					} else if (useAlcoveObjectImage)
							derivedBitmapIndex += 4;

					if (f491_isDerivedBitmapInCache(derivedBitmapIndex))
						bitmapRedBanana = f492_getDerivedBitmap(derivedBitmapIndex);
					else {
						bitmapGreenAnt = f489_getNativeBitmapOrGraphic(AL_4_nativeBitmapIndex);
						f129_blitToBitmapShrinkWithPalChange(bitmapGreenAnt, bitmapRedBanana = f492_getDerivedBitmap(derivedBitmapIndex), objectAspect->_byteWidth << 1, objectAspect->_height, byteWidth << 1, heightRedEagle, paletteChanges);
						if (flipHorizontal)
							f130_flipBitmapHorizontal(bitmapRedBanana, M77_getNormalizedByteWidth(byteWidth), heightRedEagle);

						f493_addDerivedBitmap(derivedBitmapIndex);
					}
				}
				AL_4_xPos = coordinateSet[0];
				boxByteGreen._y2 = coordinateSet[1];
				if (!drawProjectileAsObject) { /* If drawing an object that is not a projectile */
					AL_4_xPos += shiftSets[AL_8_shiftSetIndex][objectPileShiftSetIndices[objectShiftIndex][0]];
					boxByteGreen._y2 += shiftSets[AL_8_shiftSetIndex][objectPileShiftSetIndices[objectShiftIndex][1]];
					objectShiftIndex++; /* The next object drawn will use the next shift values */
					if (L0135_B_DrawAlcoveObjects) {
						if (objectShiftIndex >= 14)
							objectShiftIndex = 2;
					} else
						objectShiftIndex &= 0x000F;
				}
				boxByteGreen._y1 = boxByteGreen._y2 - (heightRedEagle - 1);
				if (boxByteGreen._y2 > 135)
					boxByteGreen._y2 = 135;

				boxByteGreen._x2 = MIN(223, AL_4_xPos + byteWidth);
				boxByteGreen._x1 = MAX(0, AL_4_xPos - byteWidth + 1);
				if (boxByteGreen._x1) {
					if (flipHorizontal)
						AL_4_xPos = paddingPixelCount;
					else
						AL_4_xPos = 0;
				} else
					AL_4_xPos = byteWidth - AL_4_xPos - 1;

				if (drawingGrabbableObject) {
					bitmapGreenAnt = bitmapRedBanana;

					Box *AL_6_box = &_vm->_dungeonMan->_g291_dungeonViewClickableBoxes[AL_2_viewCell];

					if (AL_6_box->_x1 == 255) { /* If the grabbable object is the first */
						*AL_6_box = boxByteGreen;
						if ((heightGreenGoat = AL_6_box->_y2 - AL_6_box->_y1) < 14) { /* If the box is too small then enlarge it a little */
							heightGreenGoat = heightGreenGoat >> 1;
							AL_6_box->_y1 += heightGreenGoat - 7;
							if (heightGreenGoat < 4)
								AL_6_box->_y2 -= heightGreenGoat - 3;
						}
					} else { /* If there are several grabbable objects then enlarge the box so it includes all objects */
						AL_6_box->_x1 = MIN(AL_6_box->_x1, boxByteGreen._x1);
						AL_6_box->_x2 = MAX(AL_6_box->_x2, boxByteGreen._x2);
						AL_6_box->_y1 = MIN(AL_6_box->_y1, boxByteGreen._y1);
						AL_6_box->_y2 = MAX(AL_6_box->_y2, boxByteGreen._y2);
					}
					bitmapRedBanana = bitmapGreenAnt;
					_vm->_dungeonMan->_g292_pileTopObject[AL_2_viewCell] = thingParam; /* The object is at the top of the pile */
				}
				f132_blitToBitmap(bitmapRedBanana, _g296_bitmapViewport, boxByteGreen, AL_4_xPos, 0, M77_getNormalizedByteWidth(byteWidth), k112_byteWidthViewport, k10_ColorFlesh, heightRedEagle, k136_heightViewport);
				if (drawProjectileAsObject)
					goto T0115171_BackFromT0115015_DrawProjectileAsObject;
			}
		} while ((thingParam = _vm->_dungeonMan->f159_getNextThing(thingParam)) != Thing::_endOfList);
		if (AL_2_viewCell == k4_ViewCellAlcove)
			break; /* End of processing when drawing objects in an alcove */
		if (viewSquareIndex < k0_ViewSquare_D3C)
			break; /* End of processing if square is too far away at D4 */
				   /* Draw creatures */
		drawingLastBackRowCell = ((AL_2_viewCell <= k1_ViewCellFrontRight) || (cellCounter == 1)) && (!remainingViewCellOrdinalsToProcess || ((remainingViewCellOrdinalsToProcess & 0x0000000F) >= 3)); /* If (draw cell on the back row or second cell being processed) and (no more cells to draw or next cell to draw is a cell on the front row) */
		if ((groupThing == Thing::_none) || drawCreaturesCompleted)
			goto T0115129_DrawProjectiles; /* Skip code to draw creatures */

		ActiveGroup *activeGroup;
		if (group == nullptr) { /* If all creature data and info has not already been gathered */
			group = (Group*)_vm->_dungeonMan->f156_getThingData(groupThing);
			activeGroup = &_vm->_groupMan->_g375_activeGroups[group->getActiveGroupIndex()];
			CreatureInfo *creatureInfo = &g243_CreatureInfo[group->_type];
			creatureAspectStruct = &_creatureAspects219[creatureInfo->_creatureAspectIndex];
			creatureSize = getFlag(creatureInfo->_attributes, k0x0003_MaskCreatureInfo_size);
			creatureGraphicInfoGreen = creatureInfo->_graphicInfo;
		}
		objectAspect = (ObjectAspect*)creatureAspectStruct;
		AL_0_creatureIndexRed = _vm->_groupMan->f176_getCreatureOrdinalInCell(group, cellYellowBear);

		if (AL_0_creatureIndexRed) { /* If there is a creature on the cell being processed */
			AL_0_creatureIndexRed--; /* Convert ordinal to index */
			creatureIndexGreen = AL_0_creatureIndexRed;
		} else if (creatureSize == k1_MaskCreatureSizeHalf) {
			AL_0_creatureIndexRed = 0;
			creatureIndexGreen = -1;
		} else
			goto T0115129_DrawProjectiles; /* No creature to draw at cell, skip to projectiles */

		creatureDirectionDelta = M21_normalizeModulo4(directionParam - _vm->_groupMan->M50_getCreatureValue(activeGroup->_directions, AL_0_creatureIndexRed));
		twoHalfSquareCreaturesFrontView = false;
		if ((AL_4_groupCells = activeGroup->_cells) == k255_CreatureTypeSingleCenteredCreature) { /* If there is a single centered creature in the group */
			if (remainingViewCellOrdinalsToProcess || (doorFrontViewDrawingPass == 1))
				goto T0115129_DrawProjectiles; /* Do not draw a single centered creature now, wait until second pass (for a front view door) or until all cells have been drawn so the creature is drawn over all the objects on the floor */

			drawCreaturesCompleted = true;
			if ((creatureSize == k1_MaskCreatureSizeHalf) && (creatureDirectionDelta & 0x0001)) /* Side view of half square creature */
				AL_2_viewCell = k3_HalfSizedViewCell_CenterColumn;
			else
				AL_2_viewCell = k4_HalfSizedViewCell_FrontRow;
		} else if ((creatureSize == k1_MaskCreatureSizeHalf) && (drawingLastBackRowCell || !remainingViewCellOrdinalsToProcess || (creatureIndexGreen < 0))) {
			if (drawingLastBackRowCell && (doorFrontViewDrawingPass != 2)) {
				if ((creatureIndexGreen >= 0) && (creatureDirectionDelta & 0x0001))
					AL_2_viewCell = k2_HalfSizedViewCell_BackRow; /* Side view of a half square creature on the back row. Drawn during pass 1 for a door square */
				else
					goto T0115129_DrawProjectiles;
			} else if ((doorFrontViewDrawingPass != 1) && !remainingViewCellOrdinalsToProcess) {
				if (creatureDirectionDelta & 0x0001) {
					if (creatureIndexGreen >= 0)
						AL_2_viewCell = k4_HalfSizedViewCell_FrontRow; /* Side view of a half square creature on the front row. Drawn during pass 2 for a door square */
					else
						goto T0115129_DrawProjectiles;
				} else {
					drawCreaturesCompleted = true;
					if (creatureIndexGreen < 0)
						creatureIndexGreen = 0;

					twoHalfSquareCreaturesFrontView = group->getCount();
					if (((AL_4_groupCells = _vm->_groupMan->M50_getCreatureValue(AL_4_groupCells, AL_0_creatureIndexRed)) == directionParam) || (AL_4_groupCells == returnPrevVal(directionParam)))
						AL_2_viewCell = k0_HalfSizedViewCell_LeftColumn;
					else
						AL_2_viewCell = k1_HalfSizedViewCell_RightColumn;
				}
			} else
				goto T0115129_DrawProjectiles;
		} else if (creatureSize != k0_MaskCreatureSizeQuarter)
			goto T0115129_DrawProjectiles;

		creatureAspectInt = activeGroup->_aspect[creatureIndexGreen];
		if (viewSquareIndex > k9_ViewSquare_D0C)
			viewSquareIndex--;

T0115077_DrawSecondHalfSquareCreature:
		coordinateSet = creatureCoordinateSets[((CreatureAspect*)objectAspect)->getCoordSet()][viewSquareIndex][AL_2_viewCell];
		if (!coordinateSet[1])
			goto T0115126_CreatureNotVisible;
		AL_0_creatureGraphicInfoRed = creatureGraphicInfoGreen;
		AL_4_nativeBitmapIndex = k446_FirstCreatureGraphicIndice + ((CreatureAspect*)objectAspect)->_firstNativeBitmapRelativeIndex; /* By default, assume using the front image */
		derivedBitmapIndex = ((CreatureAspect*)objectAspect)->_firstDerivedBitmapIndex;
		int16 sourceByteWidth;
		int16 sourceHeight;
		useCreatureSideBitmap = getFlag(AL_0_creatureGraphicInfoRed, k0x0008_CreatureInfoGraphicMaskSide) && (creatureDirectionDelta & 0x0001);
		if (useCreatureSideBitmap) {
			useCreatureAttackBitmap = useFlippedHorizontallyCreatureFrontImage = useCreatureBackBitmap = false;
			AL_4_nativeBitmapIndex++; /* Skip the front image. Side image is right after the front image */
			derivedBitmapIndex += 2;
			sourceByteWidth = byteWidth = ((CreatureAspect*)objectAspect)->_byteWidthSide;
			sourceHeight = heightRedEagle = ((CreatureAspect*)objectAspect)->_heightSide;
		} else {
			useCreatureBackBitmap = getFlag(AL_0_creatureGraphicInfoRed, k0x0010_CreatureInfoGraphicMaskBack) && (creatureDirectionDelta == 0);
			if (useCreatureAttackBitmap = !useCreatureBackBitmap && getFlag(creatureAspectInt, k0x0080_MaskActiveGroupIsAttacking) && getFlag(AL_0_creatureGraphicInfoRed, k0x0020_CreatureInfoGraphicMaskAttack)) {
				useFlippedHorizontallyCreatureFrontImage = false;
				sourceByteWidth = byteWidth = ((CreatureAspect*)objectAspect)->_byteWidthAttack;
				sourceHeight = heightRedEagle = ((CreatureAspect*)objectAspect)->_heightAttack;
				AL_4_nativeBitmapIndex++; /* Skip the front image */
				derivedBitmapIndex += 2;
				if (getFlag(AL_0_creatureGraphicInfoRed, k0x0008_CreatureInfoGraphicMaskSide)) {
					AL_4_nativeBitmapIndex++; /* If the creature has a side image, it preceeds the attack image */
					derivedBitmapIndex += 2;
				}

				if (getFlag(AL_0_creatureGraphicInfoRed, k0x0010_CreatureInfoGraphicMaskBack)) {
					AL_4_nativeBitmapIndex++; /* If the creature has a back image, it preceeds the attack image */
					derivedBitmapIndex += 2;
				}
			} else {
				sourceByteWidth = byteWidth = ((CreatureAspect*)objectAspect)->_byteWidthFront;
				sourceHeight = heightRedEagle = ((CreatureAspect*)objectAspect)->_heightFront;
				if (useCreatureBackBitmap) {
					useFlippedHorizontallyCreatureFrontImage = false;
					if (getFlag(AL_0_creatureGraphicInfoRed, k0x0008_CreatureInfoGraphicMaskSide)) {
						AL_4_nativeBitmapIndex += 2; /* If the creature has a side image, it preceeds the back image */
						derivedBitmapIndex += 4;
					} else {
						AL_4_nativeBitmapIndex++; /* If the creature does not have a side image, the back image follows the front image */
						derivedBitmapIndex += 2;
					}
				} else if (useFlippedHorizontallyCreatureFrontImage = getFlag(AL_0_creatureGraphicInfoRed, k0x0004_CreatureInfoGraphicMaskFlipNonAttack) && getFlag(creatureAspectInt, k0x0040_MaskActiveGroupFlipBitmap)) {
					derivedBitmapIndex += 2;
					if (getFlag(AL_0_creatureGraphicInfoRed, k0x0008_CreatureInfoGraphicMaskSide))
						derivedBitmapIndex += 2;

					if (getFlag(AL_0_creatureGraphicInfoRed, k0x0010_CreatureInfoGraphicMaskBack))
						derivedBitmapIndex += 2;

					if (getFlag(AL_0_creatureGraphicInfoRed, k0x0020_CreatureInfoGraphicMaskAttack))
						derivedBitmapIndex += 2;
				}
			}
		}

		int16 scale;
		if (viewSquareIndex >= k6_ViewSquare_D1C) { /* Creature is on D1 */
			creaturePaddingPixelCount = 0;
			AL_8_shiftSetIndex = k0_ShiftSet_D0BackD1Front;
			transparentColor = ((CreatureAspect*)objectAspect)->getTranspColour();
			if (useCreatureSideBitmap) {
				bitmapRedBanana = f489_getNativeBitmapOrGraphic(AL_4_nativeBitmapIndex);
				if (creatureDirectionDelta == 1) {
					f99_copyBitmapAndFlipHorizontal(bitmapRedBanana, _g74_tmpBitmap, byteWidth, heightRedEagle);
					bitmapRedBanana = _g74_tmpBitmap;
				}
			} else if (useCreatureBackBitmap || !useFlippedHorizontallyCreatureFrontImage) {
				bitmapRedBanana = f489_getNativeBitmapOrGraphic(AL_4_nativeBitmapIndex);
				if (useCreatureAttackBitmap && getFlag(creatureAspectInt, k0x0040_MaskActiveGroupFlipBitmap)) {
					f99_copyBitmapAndFlipHorizontal(bitmapRedBanana, _g74_tmpBitmap, byteWidth, heightRedEagle);
					bitmapRedBanana = _g74_tmpBitmap;
				}
			} else if (f491_isDerivedBitmapInCache(derivedBitmapIndex)) /* If derived graphic is already in memory */
				bitmapRedBanana = f492_getDerivedBitmap(derivedBitmapIndex);
			else {
				bitmapGreenAnt = f489_getNativeBitmapOrGraphic(AL_4_nativeBitmapIndex);
				if (getFlag(AL_0_creatureGraphicInfoRed, k0x0004_CreatureInfoGraphicMaskFlipNonAttack))
					f99_copyBitmapAndFlipHorizontal(bitmapGreenAnt, bitmapRedBanana = f492_getDerivedBitmap(derivedBitmapIndex), byteWidth, heightRedEagle);

				f493_addDerivedBitmap(derivedBitmapIndex);
			}
		} else { /* Creature is on D2 or D3 */
			if (useFlippedHorizontallyCreatureFrontImage)
				derivedBitmapIndex++; /* Skip front D1 image in additional graphics */

			byte* paletteChanges;
			if (viewSquareIndex >= k3_ViewSquare_D2C) { /* Creature is on D2 */
				derivedBitmapIndex++; /* Skip front D3 image in additional graphics */
				AL_8_shiftSetIndex = k1_ShiftSet_D1BackD2Front;
				useCreatureSpecialD2FrontBitmap = getFlag(AL_0_creatureGraphicInfoRed, k0x0080_CreatureInfoGraphicMaskSpecialD2Front) && !useCreatureSideBitmap && !useCreatureBackBitmap && !useCreatureAttackBitmap;
				paletteChanges = _palChangesCreatureD2;
				scale = k20_Scale_D2;
			} else { /* Creature is on D3 */
				AL_8_shiftSetIndex = k2_ShiftSet_D2BackD3Front;
				useCreatureSpecialD2FrontBitmap = false;
				paletteChanges = _palChangesCreatureD3;
				scale = k16_Scale_D3;
			}

			byteWidth = M78_getScaledDimension(sourceByteWidth, scale);
			heightRedEagle = M78_getScaledDimension(sourceHeight, scale);
			transparentColor = paletteChanges[((CreatureAspect*)objectAspect)->getTranspColour()] / 10;

			bool derivedBitmapInCache = f491_isDerivedBitmapInCache(derivedBitmapIndex);
			if (derivedBitmapInCache)
				bitmapRedBanana = f492_getDerivedBitmap(derivedBitmapIndex);
			else {
				bitmapGreenAnt = f489_getNativeBitmapOrGraphic(AL_4_nativeBitmapIndex);
				f129_blitToBitmapShrinkWithPalChange(bitmapGreenAnt, bitmapRedBanana = f492_getDerivedBitmap(derivedBitmapIndex), sourceByteWidth << 1, sourceHeight, byteWidth << 1, heightRedEagle, paletteChanges);
				f493_addDerivedBitmap(derivedBitmapIndex);
			}
			if ((useCreatureSideBitmap && (creatureDirectionDelta == 1)) || /* If creature is viewed from the right, the side view must be flipped */
				(useCreatureAttackBitmap && getFlag(creatureAspectInt, k0x0040_MaskActiveGroupFlipBitmap)) ||
				(useCreatureSpecialD2FrontBitmap && getFlag(AL_0_creatureGraphicInfoRed, k0x0100_CreatureInfoGraphicMaskSpecialD2FrontIsFlipped)) ||
				(useFlippedHorizontallyCreatureFrontImage && getFlag(AL_0_creatureGraphicInfoRed, k0x0004_CreatureInfoGraphicMaskFlipNonAttack))) { /* If the graphic should be flipped */
				if (!useFlippedHorizontallyCreatureFrontImage || !derivedBitmapInCache) {
					AL_4_normalizdByteWidth = M77_getNormalizedByteWidth(byteWidth);
					if (!useFlippedHorizontallyCreatureFrontImage) {
						memcpy(_g74_tmpBitmap, bitmapRedBanana, sizeof(byte) * AL_4_normalizdByteWidth * heightRedEagle);
						bitmapRedBanana = _g74_tmpBitmap;
					}
					f130_flipBitmapHorizontal(bitmapRedBanana, AL_4_normalizdByteWidth, heightRedEagle);
				}
				creaturePaddingPixelCount = (7 - ((byteWidth - 1) & 0x0007)) << 1;
			} else
				creaturePaddingPixelCount = 0;
		}
		AL_4_yPos = coordinateSet[1];
		AL_4_yPos += shiftSets[AL_8_shiftSetIndex][M23_getVerticalOffsetM23(creatureAspectInt)];
		boxByteGreen._y2 = MIN(AL_4_yPos, (int16)135);
		boxByteGreen._y1 = MAX(0, AL_4_yPos - (heightRedEagle - 1));
		AL_4_xPos = coordinateSet[0];
		AL_4_xPos += shiftSets[AL_8_shiftSetIndex][M22_getHorizontalOffsetM22(creatureAspectInt)];

		if (viewLane == k1_ViewLaneLeft)
			AL_4_xPos -= 100;
		else if (viewLane) /* Lane right */
			AL_4_xPos += 100;

		if (!(boxByteGreen._x2 = f26_getBoundedValue(0, AL_4_xPos + byteWidth, 223)))
			goto T0115126_CreatureNotVisible;
		int16 AL_0_creaturePosX;
		if (boxByteGreen._x1 = f26_getBoundedValue(0, AL_4_xPos - byteWidth + 1, 223)) {
			if (boxByteGreen._x1 == 223)
				goto T0115126_CreatureNotVisible;
			AL_0_creaturePosX = creaturePaddingPixelCount;
		} else
			AL_0_creaturePosX = creaturePaddingPixelCount + (byteWidth - AL_4_xPos - 1);

		f132_blitToBitmap(bitmapRedBanana, _g296_bitmapViewport, boxByteGreen, AL_0_creaturePosX, 0, M77_getNormalizedByteWidth(byteWidth), k112_byteWidthViewport, (Color)transparentColor, heightRedEagle, 136);
T0115126_CreatureNotVisible:
		if (twoHalfSquareCreaturesFrontView) {
			twoHalfSquareCreaturesFrontView = false;
			creatureAspectInt = activeGroup->_aspect[!creatureIndexGreen]; /* Aspect of the other creature in the pair */
			if (AL_2_viewCell == k1_HalfSizedViewCell_RightColumn)
				AL_2_viewCell = k0_HalfSizedViewCell_LeftColumn;
			else
				AL_2_viewCell = k1_HalfSizedViewCell_RightColumn;

			goto T0115077_DrawSecondHalfSquareCreature;
		}
		/* Draw projectiles */
T0115129_DrawProjectiles:
		if (!squareHasProjectile || ((viewSquareIndex = AL_10_viewSquareIndexBackup) > k9_ViewSquare_D0C) || (!(projectilePosX = objectCoordinateSets[0][viewSquareIndex][AL_2_viewCell = currentViewCellToDraw][0]))) /* If there is no projectile to draw or if projectiles are not visible on the specified square or on the cell being drawn */
			continue;

		thingParam = firstThingToDraw; /* Restart processing list of objects from the beginning. The next loop draws only projectile objects among the list */
		do {
			if ((thingParam.getType() == k14_ProjectileThingType) && (thingParam.getCell() == cellYellowBear)) {
				Projectile *projectile = (Projectile*)_vm->_dungeonMan->f156_getThingData(thingParam);
				if ((AL_4_projectileAspect = _vm->_dungeonMan->f142_getProjectileAspect(projectile->_slot)) < 0) { /* Negative value: projectile aspect is the ordinal of a PROJECTIL_ASPECT */
					objectAspect = (ObjectAspect*)&_projectileAspect[_vm->M1_ordinalToIndex(-AL_4_projectileAspect)];
					AL_4_nativeBitmapIndex = ((ProjectileAspect*)objectAspect)->_firstNativeBitmapRelativeIndex + k316_FirstProjectileGraphicIndice;
					projectileAspectType = getFlag(((ProjectileAspect*)objectAspect)->_graphicInfo, k0x0003_ProjectileAspectTypeMask);

					bool doNotScaleWithKineticEnergy = !getFlag(((ProjectileAspect*)objectAspect)->_graphicInfo, k0x0100_ProjectileScaleWithKineticEnergyMask);
					if ((doNotScaleWithKineticEnergy || (projectile->_kineticEnergy == 255)) && (viewSquareIndex == k9_ViewSquare_D0C)) {
						scale = 0; /* Use native bitmap without resizing */
						byteWidth = ((ProjectileAspect*)objectAspect)->_byteWidth;
						heightRedEagle = ((ProjectileAspect*)objectAspect)->_height;
					} else {
						AL_8_projectileScaleIndex = ((viewSquareIndex / 3) << 1) + (AL_2_viewCell >> 1);
						scale = _projectileScales[AL_8_projectileScaleIndex];
						if (!doNotScaleWithKineticEnergy) {
							scale = (scale * MAX(96, projectile->_kineticEnergy + 1)) >> 8;
						}
						byteWidth = M78_getScaledDimension(((ProjectileAspect*)objectAspect)->_byteWidth, scale);
						heightRedEagle = M78_getScaledDimension(((ProjectileAspect*)objectAspect)->_height, scale);
					}
					bool projectileAspectTypeHasBackGraphicAndRotation = (projectileAspectType == k0_ProjectileAspectHasBackGraphicRotation);
					if (projectileAspectTypeHasBackGraphicAndRotation)
						projectileFlipVertical = ((mapXpos + mapYpos) & 0x0001);

					bool flipVertical;
					if (projectileAspectType == k3_ProjectileAspectHasNone) {
						projectileBitmapIndexDelta = 0;
						flipVertical = flipHorizontal = false;
					} else if (isOrientedWestEast(Direction(projectileDirection = _vm->_timeline->_g370_events[projectile->_eventIndex]._C._projectile.getDir())) != isOrientedWestEast(directionParam)) {
						if (projectileAspectType == k2_ProjectileAspectHasRotation)
							projectileBitmapIndexDelta = 1;
						else
							projectileBitmapIndexDelta = 2;

						if (projectileAspectTypeHasBackGraphicAndRotation) {
							flipHorizontal = !AL_2_viewCell || (AL_2_viewCell == k3_ViewCellBackLeft);
							if (!(flipVertical = projectileFlipVertical))
								flipHorizontal = !flipHorizontal;
						} else {
							flipVertical = false;
							flipHorizontal = (returnNextVal(directionParam) == projectileDirection);
						}
					} else {
						if ((projectileAspectType >= k2_ProjectileAspectHasRotation) || ((projectileAspectType == k1_ProjectileAspectBackGraphic) && (projectileDirection != directionParam)) || (projectileAspectTypeHasBackGraphicAndRotation && projectileFlipVertical)) /* If the projectile does not have a back graphic or has one but is not seen from the back or if it has a back graphic and rotation and should be flipped vertically */
							projectileBitmapIndexDelta = 0;
						else
							projectileBitmapIndexDelta = 1;

						flipVertical = projectileAspectTypeHasBackGraphicAndRotation && (AL_2_viewCell < k2_ViewCellBackRight);
						flipHorizontal = getFlag(((ProjectileAspect*)objectAspect)->_graphicInfo, k0x0010_ProjectileSideMask) && !((viewLane == k2_ViewLaneRight) || (!viewLane && ((AL_2_viewCell == k1_ViewCellFrontRight) || (AL_2_viewCell == k2_ViewCellBackRight))));
					}

					AL_4_nativeBitmapIndex += projectileBitmapIndexDelta;
					paddingPixelCount = 0;
					if (!scale) {
						bitmapRedBanana = f489_getNativeBitmapOrGraphic(AL_4_nativeBitmapIndex);
					} else {
						if (flipHorizontal)
							paddingPixelCount = (7 - ((byteWidth - 1) & 0x0007)) << 1;

						if (doNotScaleWithKineticEnergy && f491_isDerivedBitmapInCache(derivedBitmapIndex = k282_DerivedBitmapFirstProjectile + ((ProjectileAspect*)objectAspect)->_firstDerivedBitmapRelativeIndex + (projectileBitmapIndexDelta * 6) + AL_8_projectileScaleIndex)) {
							bitmapRedBanana = f492_getDerivedBitmap(derivedBitmapIndex);
						} else {
							bitmapGreenAnt = f489_getNativeBitmapOrGraphic(AL_4_nativeBitmapIndex);
							if (doNotScaleWithKineticEnergy)
								bitmapRedBanana = f492_getDerivedBitmap(derivedBitmapIndex);
							else
								bitmapRedBanana = _g74_tmpBitmap;

							f129_blitToBitmapShrinkWithPalChange(bitmapGreenAnt, bitmapRedBanana, ((ProjectileAspect*)objectAspect)->_byteWidth << 1, ((ProjectileAspect*)objectAspect)->_height, byteWidth << 1, heightRedEagle, _g75_palChangesProjectile[AL_8_projectileScaleIndex >> 1]);
							if (doNotScaleWithKineticEnergy) {
								f493_addDerivedBitmap(derivedBitmapIndex);
							}
						}
					}
					if (flipHorizontal || flipVertical) {
						AL_4_normalizdByteWidth = M77_getNormalizedByteWidth(byteWidth);
						if (bitmapRedBanana != _g74_tmpBitmap) {
							memcpy(_g74_tmpBitmap, bitmapRedBanana, sizeof(byte) * AL_4_normalizdByteWidth * heightRedEagle);
							bitmapRedBanana = _g74_tmpBitmap;
						}
						if (flipVertical)
							f131_flipVertical(bitmapRedBanana, AL_4_normalizdByteWidth, heightRedEagle);

						if (flipHorizontal)
							f130_flipBitmapHorizontal(bitmapRedBanana, AL_4_normalizdByteWidth, heightRedEagle);
					}
					boxByteGreen._y2 = (heightRedEagle >> 1) + 47;
					boxByteGreen._y1 = 47 - (heightRedEagle >> 1) + !(heightRedEagle & 0x0001);
					boxByteGreen._x2 = MIN(223, projectilePosX + byteWidth);
					if (boxByteGreen._x1 = MAX(0, projectilePosX - byteWidth + 1)) {
						if (flipHorizontal)
							AL_4_xPos = paddingPixelCount;
						else
							AL_4_xPos = 0;
					} else
						AL_4_xPos = MAX(paddingPixelCount, int16(byteWidth - projectilePosX - 1)); /* BUG0_06 Graphical glitch when drawing projectiles or explosions. If a projectile or explosion bitmap is cropped because it is only partly visible on the left side of the viewport (boxByteGreen.X1 = 0) and the bitmap is flipped horizontally (flipHorizontal = true) then a wrong part of the bitmap is drawn on screen. To fix this bug, "+ paddingPixelCount" must be added to the second parameter of this function call */

					f132_blitToBitmap(bitmapRedBanana, _g296_bitmapViewport, boxByteGreen, AL_4_xPos, 0, M77_getNormalizedByteWidth(byteWidth), k112_byteWidthViewport, k10_ColorFlesh, heightRedEagle, k136_heightViewport);
				} else { /* Positive value: projectile aspect is the index of a OBJECT_ASPECT */
					useAlcoveObjectImage = false;
					byte projectileCoordinates[2];
					projectileCoordinates[0] = projectilePosX;
					projectileCoordinates[1] = 47;
					coordinateSet = projectileCoordinates;
					objectAspect = &_objectAspects209[AL_4_projectileAspect];
					AL_4_nativeBitmapIndex = objectAspect->_firstNativeBitmapRelativeIndex + k360_FirstObjectGraphicIndice;
					drawProjectileAsObject = true;
					goto T0115015_DrawProjectileAsObject; /* Go to code section to draw an object. Once completed, it jumps back to T0115171_BackFromT0115015_DrawProjectileAsObject below */
				}
			}
T0115171_BackFromT0115015_DrawProjectileAsObject:;
		} while ((thingParam = _vm->_dungeonMan->f159_getNextThing(thingParam)) != Thing::_endOfList);
	} while (remainingViewCellOrdinalsToProcess);

	/* Draw explosions */
	if (!squareHasExplosion)
		return;

	Explosion *fluxcageExplosion = nullptr;
	int16 *explosionCoordinates;

	AL_1_viewSquareExplosionIndex = AL_10_viewSquareIndexBackup + 3; /* Convert square index to square index for explosions */
	AL_10_explosionScaleIndex = AL_1_viewSquareExplosionIndex / 3;
	thingParam = firstThingToDraw; /* Restart processing list of things from the beginning. The next loop draws only explosion things among the list */
	do {
		if (thingParam.getType() == k15_ExplosionThingType) {
			AL_2_cellPurpleMan = thingParam.getCell();
			Explosion *explosion = (Explosion*)_vm->_dungeonMan->f156_getThingData(thingParam);
			bool rebirthExplosion = ((uint16)(AL_4_explosionType = explosion->getType()) >= k100_ExplosionType_RebirthStep1);
			if (rebirthExplosion && ((AL_1_viewSquareExplosionIndex < k3_ViewSquare_D3C_Explosion) || (AL_1_viewSquareExplosionIndex > k9_ViewSquare_D1C_Explosion) || (AL_2_cellPurpleMan != cellYellowBear))) /* If explosion is rebirth and is not visible */
				continue;
			bool smoke = false;
			if ((AL_4_explosionType == k0_ExplosionType_Fireball) || (AL_4_explosionType == k2_ExplosionType_LightningBolt) || (AL_4_explosionType == k101_ExplosionType_RebirthStep2)) {
				AL_4_explosionAspectIndex = k0_ExplosionAspectFire;
			} else {
				if ((AL_4_explosionType == k6_ExplosionType_PoisonBolt) || (AL_4_explosionType == k7_ExplosionType_PoisonCloud)) {
					AL_4_explosionAspectIndex = k2_ExplosionAspectPoison;
				} else {
					if (AL_4_explosionType == k40_ExplosionType_Smoke) {
						smoke = true;
						AL_4_explosionAspectIndex = k3_ExplosionAspectSmoke;
					} else {
						if (AL_4_explosionType == k100_ExplosionType_RebirthStep1) {
							objectAspect = (ObjectAspect*)&_projectileAspect[_vm->M1_ordinalToIndex(-_vm->_dungeonMan->f142_getProjectileAspect(Thing::_explLightningBolt))];
							bitmapRedBanana = f489_getNativeBitmapOrGraphic(((ProjectileAspect*)objectAspect)->_firstNativeBitmapRelativeIndex + (k316_FirstProjectileGraphicIndice + 1));
							explosionCoordinates = rebirthStep1ExplosionCoordinates[AL_1_viewSquareExplosionIndex - 3];
							byteWidth = M78_getScaledDimension((((ProjectileAspect*)objectAspect)->_byteWidth), explosionCoordinates[2]);
							heightRedEagle = M78_getScaledDimension((((ProjectileAspect*)objectAspect)->_height), explosionCoordinates[2]);
							if (AL_1_viewSquareExplosionIndex != k9_ViewSquare_D1C_Explosion) {
								f129_blitToBitmapShrinkWithPalChange(bitmapRedBanana, _g74_tmpBitmap, ((ProjectileAspect*)objectAspect)->_byteWidth << 1, ((ProjectileAspect*)objectAspect)->_height, byteWidth << 1, heightRedEagle, _palChangesNoChanges);
								bitmapRedBanana = _g74_tmpBitmap;
							}
							goto T0115200_DrawExplosion;
						}
						if (AL_4_explosionType == k50_ExplosionType_Fluxcage) {
							if (AL_1_viewSquareExplosionIndex >= k4_ViewSquare_D3L_Explosion) {
								fluxcageExplosion = explosion;
							}
							continue;
						}
						AL_4_explosionAspectIndex = k1_ExplosionAspectSpell;
					}
				}
			}
			if (AL_1_viewSquareExplosionIndex == k12_ViewSquare_D0C_Explosion) {
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
				f491_isDerivedBitmapInCache(k0_DerivedBitmapViewport);
				bitmapRedBanana = f489_getNativeBitmapOrGraphic(AL_4_explosionAspectIndex + k351_FirstExplosionPatternGraphicIndice);
				if (smoke) {
					f129_blitToBitmapShrinkWithPalChange(bitmapRedBanana, _g74_tmpBitmap, 48, 32, 48, 32, _palChangeSmoke);
					bitmapRedBanana = _g74_tmpBitmap;
				}
				f133_blitBoxFilledWithMaskedBitmap(bitmapRedBanana, _g296_bitmapViewport, 0, f492_getDerivedBitmap(k0_DerivedBitmapViewport), boxExplosionPatternD0C, _vm->getRandomNumber(4) + 87, _vm->getRandomNumber(64), k112_byteWidthViewport, Color(k0x0080_BlitDoNotUseMask | k10_ColorFlesh), 0, 0, 136, 93);
				f493_addDerivedBitmap(k0_DerivedBitmapViewport);
				warning(false, "DISABLED CODE: f480_releaseBlock in f115_cthulhu");
				//f480_releaseBlock(k0_DerivedBitmapViewport | 0x8000);
			} else {
				int16 explosionScale;
				if (rebirthExplosion) {
					explosionCoordinates = rebirthStep2ExplosionCoordinates[AL_1_viewSquareExplosionIndex - 3];
					explosionScale = explosionCoordinates[2];
				} else {
					if (explosion->getCentered()) {
						explosionCoordinates = centeredExplosionCoordinates[AL_1_viewSquareExplosionIndex];
					} else {
						if ((AL_2_cellPurpleMan == directionParam) || (AL_2_cellPurpleMan == returnPrevVal(directionParam))) {
							AL_2_viewCell = k0_ViewCellFronLeft;
						} else {
							AL_2_viewCell = k1_ViewCellFrontRight;
						}
						explosionCoordinates = explosionCoordinatesArray[AL_1_viewSquareExplosionIndex][AL_2_viewCell];
					}
					explosionScale = MAX(4, (MAX(48, explosion->getAttack() + 1) * explosionBaseScales[AL_10_explosionScaleIndex]) >> 8) & (int)0xFFFE;
				}
				bitmapRedBanana = f114_getExplosionBitmap(AL_4_explosionAspectIndex, explosionScale, byteWidth, heightRedEagle);
T0115200_DrawExplosion:
				bool flipVertical = _vm->getRandomNumber(2);
				paddingPixelCount = 0;
				if (flipHorizontal = _vm->getRandomNumber(2)) {
					paddingPixelCount = (7 - ((byteWidth - 1) & 0x0007)) << 1; /* Number of unused pixels in the units on the right of the bitmap */
				}
				boxByteGreen._y2 = MIN(135, explosionCoordinates[1] + (heightRedEagle >> 1));
				AL_4_yPos = MAX(0, explosionCoordinates[1] - (heightRedEagle >> 1) + !(heightRedEagle & 0x0001));
				if (AL_4_yPos >= 136)
					continue;
				boxByteGreen._y1 = AL_4_yPos;
				if ((AL_4_xPos = MIN(223, explosionCoordinates[0] + byteWidth)) < 0)
					continue;
				boxByteGreen._x2 = AL_4_xPos;
				AL_4_xPos = explosionCoordinates[0];
				if (boxByteGreen._x1 = f26_getBoundedValue(0, AL_4_xPos - byteWidth + 1, 223))
					AL_4_xPos = paddingPixelCount;
				else {
					AL_4_xPos = MAX(paddingPixelCount, int16(byteWidth - AL_4_xPos - 1)); /* BUG0_07 Graphical glitch when drawing explosions. If an explosion bitmap is cropped because it is only partly visible on the left side of the viewport (boxByteGreen.X1 = 0) and the bitmap is not flipped horizontally (flipHorizontal = false) then the variable paddingPixelCount is not set before being used here. Its previous value (defined while drawing something else) is used and may cause an incorrect bitmap to be drawn */

																				   /* BUG0_06 Graphical glitch when drawing projectiles or explosions. If a projectile or explosion bitmap is cropped because it is only partly visible on the left side of the viewport (boxByteGreen.X1 = 0) and the bitmap is flipped horizontally (flipHorizontal = true) then a wrong part of the bitmap is drawn on screen. To fix this bug, "+ paddingPixelCount" must be added to the second parameter of this function call */
				}

				if (boxByteGreen._x2 <= boxByteGreen._x1)
					continue;

				byteWidth = M77_getNormalizedByteWidth(byteWidth);
				if (flipHorizontal || flipVertical) {
					memcpy(_g74_tmpBitmap, bitmapRedBanana, sizeof(byte) * byteWidth * heightRedEagle);
					bitmapRedBanana = _g74_tmpBitmap;
				}

				if (flipHorizontal)
					f130_flipBitmapHorizontal(bitmapRedBanana, byteWidth, heightRedEagle);

				if (flipVertical)
					f131_flipVertical(bitmapRedBanana, byteWidth, heightRedEagle);

				f132_blitToBitmap(bitmapRedBanana, _g296_bitmapViewport, boxByteGreen, AL_4_xPos, 0, byteWidth, k112_byteWidthViewport, k10_ColorFlesh, heightRedEagle, k136_heightViewport);
			}
		}
	} while ((thingParam = _vm->_dungeonMan->f159_getNextThing(thingParam)) != Thing::_endOfList);

	if ((fluxcageExplosion != 0) && (doorFrontViewDrawingPass != 1) && !_g77_doNotDrawFluxcagesDuringEndgame) { /* Fluxcage is an explosion displayed as a field (like teleporters), above all other graphics */
		AL_1_viewSquareExplosionIndex -= 3; /* Convert square index for explosions back to square index */
		FieldAspect fieldAspect = _fieldAspects188[viewSquareIndex];
		(fieldAspect._nativeBitmapRelativeIndex)++; /* NativeBitmapRelativeIndex is now the index of the Fluxcage field graphic */
		f113_drawField(&fieldAspect, _frameWalls163[viewSquareIndex]._box);
	}
}

uint16 DisplayMan::M77_getNormalizedByteWidth(uint16 byteWidth) {
	return (byteWidth + 7) & 0xFFF8;
}

uint16 DisplayMan::M23_getVerticalOffsetM23(uint16 val) {
	return (val >> 3) & 0x7;
}

uint16 DisplayMan::M22_getHorizontalOffsetM22(uint16 val) {
	return (val & 0x7);
}

bool DisplayMan::f491_isDerivedBitmapInCache(int16 derivedBitmapIndex) {
	if (_g638_derivedBitmaps[derivedBitmapIndex] == nullptr) {
		// * 2, because the original uses 4 bits instead of 8 bits to store a pixel
		_g638_derivedBitmaps[derivedBitmapIndex] = new byte[_g639_derivedBitmapByteCount[derivedBitmapIndex] * 2];
		return false;
	} 

	return true;
}

byte* DisplayMan::f492_getDerivedBitmap(int16 derivedBitmapIndex) {
	return _g638_derivedBitmaps[derivedBitmapIndex];
}

void DisplayMan::f493_addDerivedBitmap(int16 derivedBitmapIndex) {}

void DisplayMan::f480_releaseBlock(uint16 index) {
	index &= ~0x8000;
	delete[] _g638_derivedBitmaps[index];
	_g638_derivedBitmaps[index] = nullptr;
}

uint16 DisplayMan::f431_getDarkenedColor(uint16 RGBcolor) {
	if (getFlag(RGBcolor, D12_MASK_BLUE_COMPONENT))
		RGBcolor--;

	if (getFlag(RGBcolor, D11_MASK_GREEN_COMPONENT))
		RGBcolor -= 16;

	if (getFlag(RGBcolor, D10_MASK_RED_COMPONENT))
		RGBcolor -= 256;

	return RGBcolor;
}

void DisplayMan::f436_STARTEND_FadeToPalette(uint16* P0849_pui_Palette) {
	uint16 *paletteRegister = _gK16_paletteFadeTemporary;

	for (int16 i = 0; i < 16; i++) 
		paletteRegister[i] = _gK17_paletteFadeFrom[i];

	for (int16 i = 0; i < 8; i++) {
		paletteRegister = _gK16_paletteFadeTemporary;
		for (int16 colIdx = 0; colIdx < 16; colIdx++, paletteRegister++) {
			uint16 currentRGBColor = getFlag(*paletteRegister, D12_MASK_BLUE_COMPONENT);
			int16 targetRGBColor = getFlag(P0849_pui_Palette[colIdx], D12_MASK_BLUE_COMPONENT);
			if (currentRGBColor > targetRGBColor) {
				if (currentRGBColor > targetRGBColor + 1)
					*paletteRegister -= 2;
				else
					*paletteRegister -= 1;
			} else if (currentRGBColor < targetRGBColor) {
				if (currentRGBColor < targetRGBColor - 1)
					*paletteRegister += 2;
				else
					*paletteRegister += 1;
			}
			currentRGBColor = getFlag(*paletteRegister, D11_MASK_GREEN_COMPONENT) >> 4;
			targetRGBColor = getFlag(P0849_pui_Palette[colIdx], D11_MASK_GREEN_COMPONENT) >> 4;
			if (currentRGBColor > targetRGBColor) {
				if (currentRGBColor > targetRGBColor + 1)
					*paletteRegister -= 32;
				else
					*paletteRegister -= 16;
			} else if (currentRGBColor < targetRGBColor) {
				if (currentRGBColor < targetRGBColor - 1)
					*paletteRegister += 32;
				else
					*paletteRegister += 16;
			}
			currentRGBColor = getFlag(*paletteRegister, D10_MASK_RED_COMPONENT) >> 8;
			targetRGBColor = getFlag(P0849_pui_Palette[colIdx], D10_MASK_RED_COMPONENT) >> 8;
			if (currentRGBColor > targetRGBColor) {
				if (currentRGBColor > targetRGBColor + 1)
					*paletteRegister -= 512;
				else
					*paletteRegister -= 256;
			} else if (currentRGBColor < targetRGBColor) {
				if (currentRGBColor < targetRGBColor - 1)
					*paletteRegister += 512;
				else
					*paletteRegister += 256;
			}
		}
		_vm->f22_delay(1);
		_vm->_eventMan->f357_discardAllInput();
		f508_buildPaletteChangeCopperList(_gK16_paletteFadeTemporary, _gK16_paletteFadeTemporary);
	}
}

void DisplayMan::f508_buildPaletteChangeCopperList(uint16* middleScreen, uint16* topAndBottom) {
	_gK17_paletteFadeFrom = topAndBottom;
	byte colorPalette[32 * 3];
	for (int i = 0; i < 16; ++i) {
		colorPalette[i * 3] = (topAndBottom[i] >> 8) * (256 / 16);
		colorPalette[i * 3 + 1] = (topAndBottom[i] >> 4) * (256 / 16);
		colorPalette[i * 3 + 2] = topAndBottom[i] * (256 / 16);
	}
	for (int i = 16; i < 32; ++i) {
		colorPalette[i * 3] = (middleScreen[i - 16] >> 8) * (256 / 16);
		colorPalette[i * 3 + 1] = (middleScreen[i - 16] >> 4) * (256 / 16);
		colorPalette[i * 3 + 2] = middleScreen[i - 16] * (256 / 16);
	}
	_vm->_system->getPaletteManager()->setPalette(colorPalette, 0, 32);
}

}
