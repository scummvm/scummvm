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

#ifndef DM_GFX_H
#define DM_GFX_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/memstream.h"
#include "common/array.h"

#include "dm/dm.h"

namespace DM {

enum ViewFloor {
	kDMViewFloorD3L = 0, // @ C0_VIEW_FLOOR_D3L
	kDMViewFloorD3C = 1, // @ C1_VIEW_FLOOR_D3C
	kDMViewFloorD3R = 2, // @ C2_VIEW_FLOOR_D3R
	kDMViewFloorD2L = 3, // @ C3_VIEW_FLOOR_D2L
	kDMViewFloorD2C = 4, // @ C4_VIEW_FLOOR_D2C
	kDMViewFloorD2R = 5, // @ C5_VIEW_FLOOR_D2R
	kDMViewFloorD1L = 6, // @ C6_VIEW_FLOOR_D1L
	kDMViewFloorD1C = 7, // @ C7_VIEW_FLOOR_D1C
	kDMViewFloorD1R = 8  // @ C8_VIEW_FLOOR_D1R
};

enum DoorState {
	kDMDoorStateOpen = 0,        // @ C0_DOOR_STATE_OPEN
	kDMDoorStateOneFourth = 1,   // @ C1_DOOR_STATE_CLOSED_ONE_FOURTH
	kDMDoorStateHalf = 2,        // @ k2_DoorStateAspect_CLOSED_HALF
	kDMDoorStateThreeFourth = 3, // @ C3_DOOR_STATE_CLOSED_THREE_FOURTH
	kDMDoorStateClosed = 4,      // @ C4_DOOR_STATE_CLOSED
	kDMDoorStateDestroyed = 5    // @ C5_DOOR_STATE_DESTROYED
};

enum DoorOrnament {
	kDMDoorOrnamentD3LCR = 0, // @ C0_VIEW_DOOR_ORNAMENT_D3LCR
	kDMDoorOrnamentD2LCR = 1, // @ C1_VIEW_DOOR_ORNAMENT_D2LCR
	kDMDoorOrnamentD1LCR = 2  // @ C2_VIEW_DOOR_ORNAMENT_D1LCR
};

enum DoorButton {
	kDMDoorButtonD3R = 0, // @ C0_VIEW_DOOR_BUTTON_D3R
	kDMDoorButtonD3C = 1, // @ C1_VIEW_DOOR_BUTTON_D3C
	kDMDoorButtonD2C = 2, // @ C2_VIEW_DOOR_BUTTON_D2C
	kDMDoorButtonD1C = 3  // @ C3_VIEW_DOOR_BUTTON_D1C
};

/* View lanes */
enum ViewLane {
	kDMViewLaneCenter = 0, // @ C0_VIEW_LANE_CENTER
	kDMViewLaneLeft = 1,   // @ C1_VIEW_LANE_LEFT
	kDMViewLaneRight = 2   // @ C2_VIEW_LANE_RIGHT
};

/* Explosion aspects */
enum ExplosionAspectEnum {
	kDMExplosionAspectFire = 0,   // @ C0_EXPLOSION_ASPECT_FIRE
	kDMExplosionAspectSpell = 1,  // @ C1_EXPLOSION_ASPECT_SPELL
	kDMExplosionAspectPoison = 2, // @ C2_EXPLOSION_ASPECT_POISON
	kDMExplosionAspectSmoke = 3   // @ C3_EXPLOSION_ASPECT_SMOKE
};

enum WallSet {
	kDMWallSetStone = 0 // @ C0_WALL_SET_STONE
};

enum FloorSet {
	kDMFloorSetStone = 0 // @ C0_FLOOR_SET_STONE
};

enum ViewWall {
	kDMViewWallD3LRight = 0, // @ C00_VIEW_WALL_D3L_RIGHT
	kDMViewWallD3RLeft = 1, // @ C01_VIEW_WALL_D3R_LEFT
	kDMViewWallD3LFront = 2, // @ C02_VIEW_WALL_D3L_FRONT
	kDMViewWallD3CFront = 3, // @ C03_VIEW_WALL_D3C_FRONT
	kDMViewWallD3RFront = 4, // @ C04_VIEW_WALL_D3R_FRONT
	kDMViewWallD2LRight = 5, // @ C05_VIEW_WALL_D2L_RIGHT
	kDMViewWallD2RLeft = 6, // @ C06_VIEW_WALL_D2R_LEFT
	kDMViewWallD2LFront = 7, // @ C07_VIEW_WALL_D2L_FRONT
	kDMViewWallD2CFront = 8, // @ C08_VIEW_WALL_D2C_FRONT
	kDMViewWallD2RFront = 9, // @ C09_VIEW_WALL_D2R_FRONT
	kDMViewWallD1LRight = 10, // @ C10_VIEW_WALL_D1L_RIGHT
	kDMViewWallD1RLeft = 11, // @ C11_VIEW_WALL_D1R_LEFT
	kDMViewWallD1CFront = 12  // @ C12_VIEW_WALL_D1C_FRONT
};

enum CellOrder {
	kDMCellOrderNone = 0xFFFF,
	kDMCellOrderAlcove = 0x0000,                                  // @ C0000_CELL_ORDER_ALCOVE
	kDMCellOrderBackLeft = 0x0001,                                // @ C0001_CELL_ORDER_BACKLEFT
	kDMCellOrderBackRight = 0x0002,                               // @ C0002_CELL_ORDER_BACKRIGHT
	kDMCellOrderDoorPass1BackLeft = 0x0018,                      // @ C0018_CELL_ORDER_DOORPASS1_BACKLEFT
	kDMCellOrderBackLeftBackRight = 0x0021,                      // @ C0021_CELL_ORDER_BACKLEFT_BACKRIGHT
	kDMCellOrderDoorPass1BackRight = 0x0028,                     // @ C0028_CELL_ORDER_DOORPASS1_BACKRIGHT
	kDMCellOrderBackRightFrontRight = 0x0032,                    // @ C0032_CELL_ORDER_BACKRIGHT_FRONTRIGHT
	kDMCellOrderDoorPass2FrontRight = 0x0039,                    // @ C0039_CELL_ORDER_DOORPASS2_FRONTRIGHT
	kDMCellOrderBackLeftFrontLeft = 0x0041,                      // @ C0041_CELL_ORDER_BACKLEFT_FRONTLEFT
	kDMCellOrderDoorPass2FrontLeft = 0x0049,                     // @ C0049_CELL_ORDER_DOORPASS2_FRONTLEFT
	kDMCellOrderDoorPass1BackRightBackLeft = 0x0128,            // @ C0128_CELL_ORDER_DOORPASS1_BACKRIGHT_BACKLEFT
	kDMCellOrderDoorPass1BackLeftBackRight = 0x0218,            // @ C0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT
	kDMCellOrderBackLeftBackRightFrontRight = 0x0321,           // @ C0321_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTRIGHT
	kDMCellOrderBackRightFrontLeftFrontRight = 0x0342,          // @ C0342_CELL_ORDER_BACKRIGHT_FRONTLEFT_FRONTRIGHT
	kDMCellOrderDoorPass2FrontLeftFrontRight = 0x0349,          // @ C0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT
	kDMCellOrderBackRightBackLeftFrontLeft = 0x0412,            // @ C0412_CELL_ORDER_BACKRIGHT_BACKLEFT_FRONTLEFT
	kDMCellOrderBackLeftFrontRightFrontLeft = 0x0431,           // @ C0431_CELL_ORDER_BACKLEFT_FRONTRIGHT_FRONTLEFT
	kDMCellOrderDoorPass2FrontRightFrontLeft = 0x0439,          // @ C0439_CELL_ORDER_DOORPASS2_FRONTRIGHT_FRONTLEFT
	kDMCellOrderBackLeftBackRightFrontLeftFrontRight = 0x3421, // @ C3421_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT
	kDMCellOrderBackRightBackLeftFrontRightFrontLeft = 0x4312 // @ C4312_CELL_ORDER_BACKRIGHT_BACKLEFT_FRONTRIGHT_FRONTLEFT
};

enum DerivedBitmap {
	kDMDerivedBitmapViewport = 0, // @ C000_DERIVED_BITMAP_VIEWPORT
	kDMDerivedBitmapThievesEyeVisibleArea = 1, // @ C001_DERIVED_BITMAP_THIEVES_EYE_VISIBLE_AREA
	kDMDerivedBitmapDamageToCreatureMedium = 2, // @ C002_DERIVED_BITMAP_DAMAGE_TO_CREATURE_MEDIUM
	kDMDerivedBitmapDamageToCreatureSmall = 3, // @ C003_DERIVED_BITMAP_DAMAGE_TO_CREATURE_SMALL
	kDMDerivedBitmapFirstWallOrnament = 4, // @ C004_DERIVED_BITMAP_FIRST_WALL_ORNAMENT
	kDMDerivedBitmapFirstDoorOrnamentD3 = 68, // @ C068_DERIVED_BITMAP_FIRST_DOOR_ORNAMENT_D3
	kDMDerivedBitmapFirstDoorOrnamentD2 = 69, // @ C069_DERIVED_BITMAP_FIRST_DOOR_ORNAMENT_D2
	kDMDerivedBitmapFirstDoorButton = 102, // @ C102_DERIVED_BITMAP_FIRST_DOOR_BUTTON
	kDMDerivedBitmapFirstObject = 104, // @ C104_DERIVED_BITMAP_FIRST_OBJECT
	kDMDerivedBitmapFirstProjectile = 282, // @ C282_DERIVED_BITMAP_FIRST_PROJECTILE
	kDMDerivedBitmapFirstExplosion = 438, // @ C438_DERIVED_BITMAP_FIRST_EXPLOSION
	kDMDerivedBitmapFirstCreature = 495 // @ C495_DERIVED_BITMAP_FIRST_CREATURE
};

enum ViewSquare {
	kDMViewSquareD4C = -3, // @ CM3_VIEW_SQUARE_D4C
	kViewSquareD4L = -2, // @ CM2_VIEW_SQUARE_D4L
	kDMViewSquareD4R = -1, // @ CM1_VIEW_SQUARE_D4R
	kDMViewSquareD3C = 0, // @ C00_VIEW_SQUARE_D3C
	kDMViewSquareD3L = 1, // @ C01_VIEW_SQUARE_D3L
	kDMViewSquareD3R = 2, // @ C02_VIEW_SQUARE_D3R
	kDMViewSquareD2C = 3, // @ C03_VIEW_SQUARE_D2C
	kDMViewSquareD2L = 4, // @ C04_VIEW_SQUARE_D2L
	kDMViewSquareD2R = 5, // @ C05_VIEW_SQUARE_D2R
	kDMViewSquareD1C = 6, // @ C06_VIEW_SQUARE_D1C
	kDMViewSquareD1L = 7, // @ C07_VIEW_SQUARE_D1L
	kDMViewSquareD1R = 8, // @ C08_VIEW_SQUARE_D1R
	kDMViewSquareD0C = 9, // @ C09_VIEW_SQUARE_D0C
	kDMViewSquareD0L = 10, // @ C10_VIEW_SQUARE_D0L
	kDMViewSquareD0R = 11, // @ C11_VIEW_SQUARE_D0R
	kDMViewSquareD3CExplosion = 3, // @ C03_VIEW_SQUARE_D3C_EXPLOSION
	kDMViewSquareD3LExplosion = 4, // @ C04_VIEW_SQUARE_D3L_EXPLOSION
	kDMViewSquareD1CExplosion = 9, // @ C09_VIEW_SQUARE_D1C_EXPLOSION
	kDMViewSquareD0CExplosion = 12 // @ C12_VIEW_SQUARE_D0C_EXPLOSION
};

enum ViewCell {
	kDMViewCellFronLeft = 0, // @ C00_VIEW_CELL_FRONT_LEFT
	kDMViewCellFrontRight = 1, // @ C01_VIEW_CELL_FRONT_RIGHT
	kDMViewCellBackRight = 2, // @ C02_VIEW_CELL_BACK_RIGHT
	kDMViewCellBackLeft = 3, // @ C03_VIEW_CELL_BACK_LEFT
	kDMViewCellAlcove = 4, // @ C04_VIEW_CELL_ALCOVE
	kDMViewCellDoorButtonOrWallOrn = 5 // @ C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT
};

enum Color {
	kDMColorNoTransparency = -1,
	kDMColorBlack = 0,
	kDMColorDarkGary = 1,
	kDMColorLightGray = 2,
	kDMColorDarkBrown = 3,
	kDMColorCyan = 4,
	kDMColorLightBrown = 5,
	kDMColorDarkGreen = 6,
	kDMColorLightGreen = 7,
	kDMColorRed = 8,
	kDMColorGold = 9,
	kDMColorFlesh = 10,
	kDMColorYellow = 11,
	kDMColorDarkestGray = 12,
	kDMColorLightestGray = 13,
	kDMColorBlue = 14,
	kDMColorWhite = 15
};

enum GraphicIndice {
	kDMGraphicIdxDialogBox = 0, // @ C000_GRAPHIC_DIALOG_BOX
	kDMGraphicIdxTitle = 1, // @ C001_GRAPHIC_TITLE
	kDMGraphicIdxEntranceLeftDoor = 2, // @ C002_GRAPHIC_ENTRANCE_LEFT_DOOR
	kDMGraphicIdxEntranceRightDoor = 3, // @ C003_GRAPHIC_ENTRANCE_RIGHT_DOOR
	kDMGraphicIdxEntrance = 4, // @ C004_GRAPHIC_ENTRANCE
	kDMGraphicIdxCredits = 5, // @ C005_GRAPHIC_CREDITS
	kDMGraphicIdxTheEnd = 6, // @ C006_GRAPHIC_THE_END
	kDMGraphicIdxStatusBoxDeadChampion = 8, // @ C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION
	kDMGraphicIdxMenuSpellAreaBackground = 9, // @ C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND
	kDMGraphicIdxMenuActionArea = 10, // @ C010_GRAPHIC_MENU_ACTION_AREA
	kDMGraphicIdxMenuSpellAreLines = 11, // @ C011_GRAPHIC_MENU_SPELL_AREA_LINES
	kDMGraphicIdxMovementArrows = 13, // @ C013_GRAPHIC_MOVEMENT_ARROWS
	kDMGraphicIdxDamageToCreature = 14, // @ C014_GRAPHIC_DAMAGE_TO_CREATURE
	kDMGraphicIdxDamageToChampionSmall = 15, // @ C015_GRAPHIC_DAMAGE_TO_CHAMPION_SMALL
	kDMGraphicIdxDamageToChampionBig = 16, // @ C016_GRAPHIC_DAMAGE_TO_CHAMPION_BIG
	kDMGraphicIdxInventory = 17, // @ C017_GRAPHIC_INVENTORY
	kDMGraphicIdxArrowForChestContent = 18, // @ C018_GRAPHIC_ARROW_FOR_CHEST_CONTENT
	kDMGraphicIdxEyeForObjectDescription = 19, // @ C019_GRAPHIC_EYE_FOR_OBJECT_DESCRIPTION
	kDMGraphicIdxPanelEmpty = 20, // @ C020_GRAPHIC_PANEL_EMPTY
	kDMGraphicIdxPanelOpenScroll = 23, // @ C023_GRAPHIC_PANEL_OPEN_SCROLL
	kDMGraphicIdxPanelOpenChest = 25, // @ C025_GRAPHIC_PANEL_OPEN_CHEST
	kDMGraphicIdxChampionPortraits = 26, // @ C026_GRAPHIC_CHAMPION_PORTRAITS
	kDMGraphicIdxPanelRenameChampion = 27, // @ C027_GRAPHIC_PANEL_RENAME_CHAMPION
	kDMGraphicIdxChampionIcons = 28, // @ C028_GRAPHIC_CHAMPION_ICONS
	kDMGraphicIdxObjectDescCircle = 29, // @ C029_GRAPHIC_OBJECT_DESCRIPTION_CIRCLE
	kDMGraphicIdxFoodLabel = 30, // @ C030_GRAPHIC_FOOD_LABEL
	kDMGraphicIdxWaterLabel = 31, // @ C031_GRAPHIC_WATER_LABEL
	kDMGraphicIdxPoisionedLabel = 32, // @ C032_GRAPHIC_POISONED_LABEL
	kDMGraphicIdxSlotBoxNormal = 33, // @ C033_GRAPHIC_SLOT_BOX_NORMAL
	kDMGraphicIdxSlotBoxWounded = 34, // @ C034_GRAPHIC_SLOT_BOX_WOUNDED
	kDMGraphicIdxSlotBoxActingHand = 35, // @ C035_GRAPHIC_SLOT_BOX_ACTING_HAND
	kDMGraphicIdxBorderPartyShield = 37, // @ C037_GRAPHIC_BORDER_PARTY_SHIELD
	kDMGraphicIdxBorderPartyFireshield = 38, // @ C038_GRAPHIC_BORDER_PARTY_FIRESHIELD
	kDMGraphicIdxBorderPartySpellshield = 39, // @ C039_GRAPHIC_BORDER_PARTY_SPELLSHIELD
	kDMGraphicIdxPanelResurectReincarnate = 40, // @ C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE
	kDMGraphicIdxHoleInWall = 41, // @ C041_GRAPHIC_HOLE_IN_WALL
	kDMGraphicIdxObjectIcons000To031 = 42, // @ C042_GRAPHIC_OBJECT_ICONS_000_TO_031
	kDMGraphicIdxObjectIcons032To063 = 43, // @ C043_GRAPHIC_OBJECT_ICONS_032_TO_063
	kDMGraphicIdxObjectIcons064To095 = 44, // @ C044_GRAPHIC_OBJECT_ICONS_064_TO_095
	kDMGraphicIdxObjectIcons096To127 = 45, // @ C045_GRAPHIC_OBJECT_ICONS_096_TO_127
	kDMGraphicIdxObjectIcons128To159 = 46, // @ C046_GRAPHIC_OBJECT_ICONS_128_TO_159
	kDMGraphicIdxObjectIcons160To191 = 47, // @ C047_GRAPHIC_OBJECT_ICONS_160_TO_191
	kDMGraphicIdxObjectIcons192To223 = 48, // @ C048_GRAPHIC_OBJECT_ICONS_192_TO_223
	kDMGraphicIdxFloorPitD3L = 49, // @ C049_GRAPHIC_FLOOR_PIT_D3L
	kDMGraphicIdxFloorPitD3C = 50, // @ C050_GRAPHIC_FLOOR_PIT_D3C
	kDMGraphicIdxFloorPitD2L = 51, // @ C051_GRAPHIC_FLOOR_PIT_D2L
	kDMGraphicIdxFloorPitD2C = 52, // @ C052_GRAPHIC_FLOOR_PIT_D2C
	kDMGraphicIdxFloorPitD1L = 53, // @ C053_GRAPHIC_FLOOR_PIT_D1L
	kDMGraphicIdxFloorPitD1C = 54, // @ C054_GRAPHIC_FLOOR_PIT_D1C
	kDMGraphicIdxFloorPitD0L = 55, // @ C055_GRAPHIC_FLOOR_PIT_D0L
	kDMGraphicIdxFloorPitD0C = 56, // @ C056_GRAPHIC_FLOOR_PIT_D0C
	kDMGraphicIdxFloorPitInvisibleD2L = 57, // @ C057_GRAPHIC_FLOOR_PIT_INVISIBLE_D2L
	kDMGraphicIdxFloorPitInvisibleD2C = 58, // @ C058_GRAPHIC_FLOOR_PIT_INVISIBLE_D2C
	kDMGraphicIdxFloorPitInvisibleD1L = 59, // @ C059_GRAPHIC_FLOOR_PIT_INVISIBLE_D1L
	kDMGraphicIdxFloorPitInvisibleD1C = 60, // @ C060_GRAPHIC_FLOOR_PIT_INVISIBLE_D1C
	kDMGraphicIdxFloorPitInvisibleD0L = 61, // @ C061_GRAPHIC_FLOOR_PIT_INVISIBLE_D0L
	kDMGraphicIdxFloorPitInvisibleD0C = 62, // @ C062_GRAPHIC_FLOOR_PIT_INVISIBLE_D0C
	kDMGraphicIdxCeilingPitD2L = 63, // @ C063_GRAPHIC_CEILING_PIT_D2L
	kDMGraphicIdxCeilingPitD2C = 64, // @ C064_GRAPHIC_CEILING_PIT_D2C
	kDMGraphicIdxCeilingPitD1L = 65, // @ C065_GRAPHIC_CEILING_PIT_D1L
	kDMGraphicIdxCeilingPitD1C = 66, // @ C066_GRAPHIC_CEILING_PIT_D1C
	kDMGraphicIdxCeilingPitD0L = 67, // @ C067_GRAPHIC_CEILING_PIT_D0L
	kDMGraphicIdxCeilingPitD0C = 68, // @ C068_GRAPHIC_CEILING_PIT_D0C
	kDMGraphicIdxFieldMaskD3R = 69, // @ C069_GRAPHIC_FIELD_MASK_D3R
	kDMGraphicIdxFieldTeleporter = 73, // @ C073_GRAPHIC_FIELD_TELEPORTER
	kDMGraphicIdxInscriptionFont = 120, // @ C120_GRAPHIC_INSCRIPTION_FONT
	kDMGraphicIdxWallOrnChampMirror = 208, // @ C208_GRAPHIC_WALL_ORNAMENT_43_CHAMPION_MIRROR
	kDMGraphicIdxFloorOrnD3LFootprints = 241, // @ C241_GRAPHIC_FLOOR_ORNAMENT_15_D3L_FOOTPRINTS
	kDMGraphicIdxDoorMaskDestroyed = 301, // @ C301_GRAPHIC_DOOR_MASK_DESTROYED
	kDMGraphicIdxFirstDoorButton = 315, // @ C315_GRAPHIC_FIRST_DOOR_BUTTON
	kDMGraphicIdxFirstProjectile = 316, // @ C316_GRAPHIC_FIRST_PROJECTILE
	kDMGraphicIdxFirstExplosion = 348, // @ C348_GRAPHIC_FIRST_EXPLOSION
	kDMGraphicIdxFirstExplosionPattern = 351, // @ C351_GRAPHIC_FIRST_EXPLOSION_PATTERN
	kDMGraphicIdxFirstObject = 360, // @ C360_GRAPHIC_FIRST_OBJECT
	kDMGraphicIdxFirstCreature = 446, // @ C446_GRAPHIC_FIRST_CREATURE
	kDMGraphicIdxFont = 557 // @ C557_GRAPHIC_FONT
};

#define kDMMaskDoorInfoCreaturesCanSeeThrough 0x0001 // @ MASK0x0001_CREATURES_CAN_SEE_THROUGH
#define kDMMaskDoorInfoProjectilesCanPassThrough 0x0002 // @ MASK0x0002_PROJECTILES_CAN_PASS_THROUGH
#define kDMMaskDoorInfoAnimated 0x0004 // @ MASK0x0004_ANIMATED
#define kDMMaskDoorFront 0x0008 // @ MASK0x0008_DOOR_FRONT

/* Field Aspect Mask */
#define kMaskFieldAspectFlipMask 0x0080 // @ MASK0x0080_FLIP_MASK
#define kMaskFieldAspectIndex 0x007F // @ MASK0x007F_MASK_INDEX
#define kMaskFieldAspectNoMask 255 // @ C255_NO_MASK

#define kDMCreatureMaskAdditional 0x0003 // @ MASK0x0003_ADDITIONAL
#define kDMCreatureMaskFlipNonAttack 0x0004 // @ MASK0x0004_FLIP_NON_ATTACK
#define kDMCreatureMaskSide 0x0008 // @ MASK0x0008_SIDE
#define kDMCreatureMaskBack 0x0010 // @ MASK0x0010_BACK
#define kDMCreatureMaskAttack 0x0020 // @ MASK0x0020_ATTACK
#define kDMCreatureMaskSpecialD2Front 0x0080 // @ MASK0x0080_SPECIAL_D2_FRONT
#define kDMCreatureMaskSpecialD2FrontIsFlipped 0x0100 // @ MASK0x0100_SPECIAL_D2_FRONT_IS_FLIPPED_FRONT
#define kDMCreatureMaskFlipAttack 0x0200 // @ MASK0x0200_FLIP_ATTACK
#define kDMCreatureMaskFlipDuringAttack 0x0400 // @ MASK0x0400_FLIP_DURING_ATTACK

#define k2_FloorSetGraphicCount 2 // @ C002_FLOOR_SET_GRAPHIC_COUNT
#define k13_WallSetGraphicCount 13 // @ C013_WALL_SET_GRAPHIC_COUNT
#define k18_StairsGraphicCount 18 // @ C018_STAIRS_GRAPHIC_COUNT
#define k3_DoorSetGraphicsCount 3 // @ C003_DOOR_SET_GRAPHIC_COUNT
#define k1_DoorButtonCount 1 // @ C001_DOOR_BUTTON_COUNT
#define k3_AlcoveOrnCount 3 // @ C003_ALCOVE_ORNAMENT_COUNT
#define k1_FountainOrnCount 1 // @ C001_FOUNTAIN_ORNAMENT_COUNT
#define k27_CreatureTypeCount 27 // @ C027_CREATURE_TYPE_COUNT
#define k4_ExplosionAspectCount 4 // @ C004_EXPLOSION_ASPECT_COUNT
#define k14_ProjectileAspectCount 14 // @ C014_PROJECTILE_ASPECT_COUNT
#define k85_ObjAspectCount 85 // @ C085_OBJECT_ASPECT_COUNT

#define k0_HalfSizedViewCell_LeftColumn 0 // @ C00_VIEW_CELL_LEFT_COLUMN
#define k1_HalfSizedViewCell_RightColumn 1 // @ C01_VIEW_CELL_RIGHT_COLUMN
#define k2_HalfSizedViewCell_BackRow 2 // @ C02_VIEW_CELL_BACK_ROW
#define k3_HalfSizedViewCell_CenterColumn 3 // @ C03_VIEW_CELL_CENTER_COLUMN
#define k4_HalfSizedViewCell_FrontRow 4 // @ C04_VIEW_CELL_FRONT_ROW

/* Shift sets */
#define k0_ShiftSet_D0BackD1Front 0 // @ C0_SHIFT_SET_D0_BACK_OR_D1_FRONT
#define k1_ShiftSet_D1BackD2Front 1 // @ C1_SHIFT_SET_D1_BACK_OR_D2_FRONT
#define k2_ShiftSet_D2BackD3Front 2 // @ C2_SHIFT_SET_D2_BACK_OR_D3_FRONT

#define k75_FirstFloorSet 75 // @ C075_GRAPHIC_FIRST_FLOOR_SET
#define k77_FirstWallSet 77 // @ C077_GRAPHIC_FIRST_WALL_SET
#define k90_FirstStairs 90 // @ C090_GRAPHIC_FIRST_STAIRS
#define k108_FirstDoorSet 108 // @ C108_GRAPHIC_FIRST_DOOR_SET
#define k120_InscriptionFont 120 // @ C120_GRAPHIC_INSCRIPTION_FONT
#define k121_FirstWallOrn 121 // @ C121_GRAPHIC_FIRST_WALL_ORNAMENT
#define k247_FirstFloorOrn 247 // @ C247_GRAPHIC_FIRST_FLOOR_ORNAMENT
#define k303_FirstDoorOrn 303 // @ C303_GRAPHIC_FIRST_DOOR_ORNAMENT
#define k730_DerivedBitmapMaximumCount 730 // @ C730_DERIVED_BITMAP_MAXIMUM_COUNT

#define k16_Scale_D3 16 // @ C16_SCALE_D3
#define k20_Scale_D2 20 // @ C20_SCALE_D2

/* Object aspect GraphicInfo */
#define k0x0001_ObjectFlipOnRightMask 0x0001 // @ MASK0x0001_FLIP_ON_RIGHT
#define k0x0010_ObjectAlcoveMask 0x0010 // @ MASK0x0010_ALCOVE

/* Projectile aspect GraphicInfo */
#define k0x0010_ProjectileSideMask 0x0010 // @ MASK0x0010_SIDE
#define k0x0100_ProjectileScaleWithKineticEnergyMask 0x0100 // @ MASK0x0100_SCALE_WITH_KINETIC_ENERGY
#define k0x0003_ProjectileAspectTypeMask 0x0003 // @ MASK0x0003_ASPECT_TYPE

/* Projectile aspect type */
#define k0_ProjectileAspectHasBackGraphicRotation 0 // @ C0_PROJECTILE_ASPECT_TYPE_HAS_BACK_GRAPHIC_AND_ROTATION
#define k1_ProjectileAspectBackGraphic 1 // @ C1_PROJECTILE_ASPECT_TYPE_HAS_BACK_GRAPHIC_AND_NO_ROTATION
#define k2_ProjectileAspectHasRotation 2 // @ C2_PROJECTILE_ASPECT_TYPE_NO_BACK_GRAPHIC_AND_ROTATION
#define k3_ProjectileAspectHasNone 3 // @ C3_PROJECTILE_ASPECT_TYPE_NO_BACK_GRAPHIC_AND_NO_ROTATION

/* Projectile aspects */
#define k3_ProjectileAspectExplosionLightningBolt 3 // @ C03_PROJECTILE_ASPECT_EXPLOSION_LIGHTNING_BOLT
#define k10_ProjectileAspectExplosionFireBall 10 // @ C10_PROJECTILE_ASPECT_EXPLOSION_FIREBALL
#define k11_ProjectileAspectExplosionDefault 11 // @ C11_PROJECTILE_ASPECT_EXPLOSION_DEFAULT
#define k12_ProjectileAspectExplosionSlime 12 // @ C12_PROJECTILE_ASPECT_EXPLOSION_SLIME
#define k13_ProjectileAspectExplosionPoisonBoltCloud 13 // @ C13_PROJECTILE_ASPECT_EXPLOSION_POISON_BOLT_POISON_CLOUD

#define k0x0080_BlitDoNotUseMask 0x0080 // @ MASK0x0080_DO_NOT_USE_MASK
#define kScaleThreshold 32768

class ExplosionAspect {
public:
	uint16 _byteWidth;
	uint16 _height;

	ExplosionAspect(uint16 byteWidth, uint16 height) :_byteWidth(byteWidth), _height(height) {}
	ExplosionAspect() : _byteWidth(0), _height(0) {}
}; // @ EXPLOSION_ASPECT

// in all cases, where a function takes a Box, it expects it to contain inclusive boundaries
class Box {
public:
	Common::Rect _rect;

	Box(int16 x1, int16 x2, int16 y1, int16 y2) {
		// +1 because Rect.constains is not inclusive for right and bottom, at the opposite of the isPointInside
		_rect = Common::Rect(x1, y1, x2, y2);
	}

	Box() {}

	bool isPointInside(Common::Point point) {
		// not using Common::Rect::contains() because we need both boundaries to be included
		return (_rect.left <= point.x) && (point.x <= _rect.right) && (_rect.top <= point.y) && (point.y <= _rect.bottom);
	}

	bool isPointInside(int16 x, int16 y) {
		return isPointInside(Common::Point(x, y));
	}

	void setToZero() { _rect = Common::Rect(0, 0, 0, 0); }
}; // @ BOX_BYTE, BOX_WORD

extern Box g2_BoxMovementArrows; // @ G0002_s_Graphic562_Box_MovementArrows

class Frame {
public:
	Box _box;
	uint16 _srcByteWidth, _srcHeight;
	uint16 _srcX, _srcY;

	Frame() {}
	Frame(uint16 destFromX, uint16 destToX, uint16 destFromY, uint16 destToY,
		  uint16 srcWidth, uint16 srcHeight, uint16 srcX, uint16 srcY) :
		_box(destFromX, destToX, destFromY, destToY),
		_srcByteWidth(srcWidth), _srcHeight(srcHeight), _srcX(srcX), _srcY(srcY) {}
};

class FieldAspect {
public:
	uint16 _nativeBitmapRelativeIndex;
	uint16 _baseStartUnitIndex; /* Index of the unit (16 pixels = 8 bytes) in bitmap where blit will start from. A random value of 0 or 1 is added to this base index */
	uint16 _transparentColor; /* Bit 7: Do not use mask if set, Bits 6-0: Transparent color index. 0xFF = no transparency */
	byte _mask; /* Bit 7: Flip, Bits 6-0: Mask index. 0xFF = no mask */
	uint16 _byteWidth;
	uint16 _height;
	uint16 _xPos;
	uint16 _bitplaneWordCount;
	FieldAspect(uint16 native, uint16 base, uint16 transparent, byte mask, uint16 byteWidth, uint16 height, uint16 xPos, uint16 bitplane);
	FieldAspect();
}; // @ FIELD_ASPECT

class CreatureAspect {
public:
	uint16 _firstNativeBitmapRelativeIndex;
	uint16 _firstDerivedBitmapIndex;
	byte _byteWidthFront;
	byte _heightFront;
	byte _byteWidthSide;
	byte _heightSide;
	byte _byteWidthAttack;
	byte _heightAttack;
private:
	byte _coordinateSet_TransparentColor;
	byte _replacementColorSetIndices;
public:

	CreatureAspect(uint16 uint161, uint16 uint162, byte byte0, byte byte1, byte byte2, byte byte3, byte byte4, byte byte5, byte byte6, byte byte7)
		: _firstNativeBitmapRelativeIndex(uint161), _firstDerivedBitmapIndex(uint162), _byteWidthFront(byte0),
		_heightFront(byte1), _byteWidthSide(byte2), _heightSide(byte3), _byteWidthAttack(byte4),
		_heightAttack(byte5), _coordinateSet_TransparentColor(byte6), _replacementColorSetIndices(byte7) {}

	CreatureAspect() :
		_firstNativeBitmapRelativeIndex(0), _firstDerivedBitmapIndex(0), _byteWidthFront(0),
		_heightFront(0), _byteWidthSide(0), _heightSide(0), _byteWidthAttack(0),
		_heightAttack(0), _coordinateSet_TransparentColor(0), _replacementColorSetIndices(0) {}

	byte getCoordSet() { return (_coordinateSet_TransparentColor >> 4) & 0xF; } // @ M71_COORDINATE_SET
	byte getTranspColour() { return  _coordinateSet_TransparentColor & 0xF; } // @ M72_TRANSPARENT_COLOR
	byte getReplColour10() { return (_replacementColorSetIndices >> 4) & 0xF; } // @ M74_COLOR_10_REPLACEMENT_COLOR_SET
	byte getReplColour9() { return _replacementColorSetIndices & 0xF; } // @ M73_COLOR_09_REPLACEMENT_COLOR_SET
}; // @ CREATURE_ASPECT

class ObjectAspect {
public:
	byte _firstNativeBitmapRelativeIndex;
	byte _firstDerivedBitmapRelativeIndex;
	byte _byteWidth;
	byte _height;
	byte _graphicInfo; /* Bits 7-5 and 3-1 Unreferenced */
	byte _coordinateSet;
	ObjectAspect(byte firstN, byte firstD, byte byteWidth, byte h, byte grap, byte coord) :
		_firstNativeBitmapRelativeIndex(firstN), _firstDerivedBitmapRelativeIndex(firstD),
		_byteWidth(byteWidth), _height(h), _graphicInfo(grap), _coordinateSet(coord) {}
	ObjectAspect() : _firstNativeBitmapRelativeIndex(0), _firstDerivedBitmapRelativeIndex(0),
		_byteWidth(0), _height(0), _graphicInfo(0), _coordinateSet(0) {}
}; // @ OBJECT_ASPECT

class ProjectileAspect {
public:
	byte _firstNativeBitmapRelativeIndex;
	byte _firstDerivedBitmapRelativeIndex;
	byte _byteWidth;
	byte _height;
	uint16 _graphicInfo; /* Bits 15-9, 7-5 and 3-2 Unreferenced */

	ProjectileAspect(byte firstN, byte firstD, byte byteWidth, byte h, uint16 grap) :
		_firstNativeBitmapRelativeIndex(firstN), _firstDerivedBitmapRelativeIndex(firstD),
		_byteWidth(byteWidth), _height(h), _graphicInfo(grap) {}

	ProjectileAspect() : _firstNativeBitmapRelativeIndex(0),
		_firstDerivedBitmapRelativeIndex(0), _byteWidth(0), _height(0), _graphicInfo(0) {}
}; // @ PROJECTIL_ASPECT

class CreatureReplColorSet {
public:
	uint16 _RGBColor[6];
	byte _d2ReplacementColor;
	byte _d3ReplacementColor;

	CreatureReplColorSet(uint16 col1, uint16 col2, uint16 col3, uint16 col4, uint16 col5, uint16 col6, byte d2Rep, byte d3Rep) {
		_RGBColor[0] = col1;
		_RGBColor[1] = col2;
		_RGBColor[2] = col3;
		_RGBColor[3] = col4;
		_RGBColor[4] = col5;
		_RGBColor[5] = col6;
		_d2ReplacementColor = d2Rep;
		_d3ReplacementColor = d3Rep;
	}
}; // @ CREATURE_REPLACEMENT_COLOR_SET

struct OrnamentInfo {
	int16 nativeIndice;
	int16 coordinateSet;
};

#define k0_DoorButton 0 // @ C0_DOOR_BUTTON
#define k0_WallOrnInscription 0 // @ C0_WALL_ORNAMENT_INSCRIPTION
#define k15_FloorOrnFootprints 15 // @ C15_FLOOR_ORNAMENT_FOOTPRINTS
#define k15_DoorOrnDestroyedMask 15 // @ C15_DOOR_ORNAMENT_DESTROYED_MASK
#define k16_DoorOrnThivesEyeMask 16 // @ C16_DOOR_ORNAMENT_THIEVES_EYE_MASK

#define k0_viewportNotDungeonView 0 // @ C0_VIEWPORT_NOT_DUNGEON_VIEW
#define k1_viewportDungeonView 1 // @ C1_VIEWPORT_DUNGEON_VIEW
#define k2_viewportAsBeforeSleepOrFreezeGame 2 // @ C2_VIEWPORT_AS_BEFORE_SLEEP_OR_FREEZE_GAME


#define k112_byteWidthViewport 112 // @ C112_BYTE_WIDTH_VIEWPORT
#define k136_heightViewport 136 // @ C136_HEIGHT_VIEWPORT

#define k160_byteWidthScreen 160 // @ C160_BYTE_WIDTH_SCREEN
#define k200_heightScreen 200 // @ C200_HEIGHT_SCREEN

#define k8_byteWidth 8 // @ C008_BYTE_WIDTH
#define k16_byteWidth 16 // @ C016_BYTE_WIDTH
#define k24_byteWidth 24 // @ C024_BYTE_WIDTH
#define k32_byteWidth 32 // @ C032_BYTE_WIDTH
#define k40_byteWidth 40 // @ C040_BYTE_WIDTH
#define k48_byteWidth 48 // @ C048_BYTE_WIDTH
#define k64_byteWidth 64 // @ C064_BYTE_WIDTH
#define k72_byteWidth 72 // @ C072_BYTE_WIDTH
#define k128_byteWidth 128 // @ C128_BYTE_WIDTH
#define k144_byteWidth 144 // @ C144_BYTE_WIDTH


class DoorFrames {
public:
	Frame _closedOrDestroyed;
	Frame _vertical[3];
	Frame _leftHorizontal[3];
	Frame _rightHorizontal[3];
	DoorFrames(Frame f1, Frame f2_1, Frame f2_2, Frame f2_3,
			   Frame f3_1, Frame f3_2, Frame f3_3,
			   Frame f4_1, Frame f4_2, Frame f4_3);
}; // @ DOOR_FRAMES

#define D00_RGB_BLACK                                0x0000
#define D01_RGB_DARK_BLUE                            0x0004
#define D02_RGB_LIGHT_BROWN                          0x0842
#define D03_RGB_PINK                                 0x086F
#define D04_RGB_LIGHTER_BROWN                        0x0A62
#define D05_RGB_DARK_GOLD                            0x0A82
#define D06_RGB_GOLD                                 0x0CA2
#define D07_RGB_RED                                  0x0F00
#define D08_RGB_YELLOW                               0x0FF4
#define D09_RGB_WHITE                                0x0FFF
#define D10_MASK_RED_COMPONENT                       0x0F00
#define D10_MASK_RED_COMPONENT 0x0F00
#define D11_MASK_GREEN_COMPONENT 0x00F0
#define D12_MASK_BLUE_COMPONENT 0x000F

class DisplayMan {
	friend class DM::TextMan;

	DMEngine *_vm;

	uint16 _grapItemCount; // @ G0632_ui_GraphicCount
	uint32 *_bitmapCompressedByteCount;
	uint32 *_bitmapDecompressedByteCount;
	uint32 *_packedItemPos;
	byte *_packedBitmaps;
	byte **_bitmaps;
	DoorFrames *_doorFrameD1C;
	// pointers are not owned by these fields
	byte *_palChangesProjectile[4]; // @G0075_apuc_PaletteChanges_Projectile

	DisplayMan(const DisplayMan &other); // no implementation on purpose
	void operator=(const DisplayMan &rhs); // no implementation on purpose

	byte *getCurrentVgaBuffer();
	// the original function has two position parameters, but they are always set to zero
	void unpackGraphics();
	void loadFNT1intoBitmap(uint16 index, byte *destBitmap);

	void viewportSetPalette(uint16 *middleScreenPalette, uint16 *topAndBottomScreen); // @ F0565_VIEWPORT_SetPalette
	void viewportBlitToScreen(); // @ F0566_VIEWPORT_BlitToScreen

	void drawFloorPitOrStairsBitmapFlippedHorizontally(uint16 nativeIndex, Frame &frame); // @ F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally
	void drawFloorPitOrStairsBitmap(uint16 nativeIndex, Frame &frame); // @ F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap
	void drawWallSetBitmap(byte *bitmap, Frame &f); // @ F0100_DUNGEONVIEW_DrawWallSetBitmap
	void drawWallSetBitmapWithoutTransparency(byte *bitmap, Frame &f); // @ F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency
	void drawSquareD3L(Direction dir, int16 posX, int16 posY); // @ F0116_DUNGEONVIEW_DrawSquareD3L
	void drawSquareD3R(Direction dir, int16 posX, int16 posY); // @ F0117_DUNGEONVIEW_DrawSquareD3R
	void drawSquareD3C(Direction dir, int16 posX, int16 posY); // @ F0118_DUNGEONVIEW_DrawSquareD3C_CPSF
	void drawSquareD2L(Direction dir, int16 posX, int16 posY); // @ F0119_DUNGEONVIEW_DrawSquareD2L
	void drawSquareD2R(Direction dir, int16 posX, int16 posY); // @ F0120_DUNGEONVIEW_DrawSquareD2R_CPSF
	void drawSquareD2C(Direction dir, int16 posX, int16 posY); // @ F0121_DUNGEONVIEW_DrawSquareD2C
	void drawSquareD1L(Direction dir, int16 posX, int16 posY); // @ F0122_DUNGEONVIEW_DrawSquareD1L
	void drawSquareD1R(Direction dir, int16 posX, int16 posY); // @ F0123_DUNGEONVIEW_DrawSquareD1R
	void drawSquareD1C(Direction dir, int16 posX, int16 posY); // @ F0124_DUNGEONVIEW_DrawSquareD1C
	void drawSquareD0L(Direction dir, int16 posX, int16 posY); // @ F0125_DUNGEONVIEW_DrawSquareD0L
	void drawSquareD0R(Direction dir, int16 posX, int16 posY); // @ F0126_DUNGEONVIEW_DrawSquareD0R
	void drawSquareD0C(Direction dir, int16 posX, int16 posY); // @ F0127_DUNGEONVIEW_DrawSquareD0C

	void applyCreatureReplColors(int replacedColor, int replacementColor); // @ F0093_DUNGEONVIEW_ApplyCreatureReplacementColors

	bool isDrawnWallOrnAnAlcove(int16 wallOrnOrd, ViewWall viewWallIndex); // @ F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF

	uint16 *_derivedBitmapByteCount; // @ G0639_pui_DerivedBitmapByteCount
	byte **_derivedBitmaps; // @ G0638_pui_DerivedBitmapBlockIndices

	int16 _stairsNativeBitmapIndexUpFrontD3L; // @ G0675_i_StairsNativeBitmapIndex_Up_Front_D3L
	int16 _stairsNativeBitmapIndexUpFrontD3C; // @ G0676_i_StairsNativeBitmapIndex_Up_Front_D3C
	int16 _stairsNativeBitmapIndexUpFrontD2L; // @ G0677_i_StairsNativeBitmapIndex_Up_Front_D2L
	int16 _stairsNativeBitmapIndexUpFrontD2C; // @ G0678_i_StairsNativeBitmapIndex_Up_Front_D2C
	int16 _stairsNativeBitmapIndexUpFrontD1L; // @ G0679_i_StairsNativeBitmapIndex_Up_Front_D1L
	int16 _stairsNativeBitmapIndexUpFrontD1C; // @ G0680_i_StairsNativeBitmapIndex_Up_Front_D1C
	int16 _stairsNativeBitmapIndexUpFrontD0CLeft; // @ G0681_i_StairsNativeBitmapIndex_Up_Front_D0C_Left
	int16 _stairsNativeBitmapIndexDownFrontD3L; // @ G0682_i_StairsNativeBitmapIndex_Down_Front_D3L
	int16 _stairsNativeBitmapIndexDownFrontD3C; // @ G0683_i_StairsNativeBitmapIndex_Down_Front_D3C
	int16 _stairsNativeBitmapIndexDownFrontD2L; // @ G0684_i_StairsNativeBitmapIndex_Down_Front_D2L
	int16 _stairsNativeBitmapIndexDownFrontD2C; // @ G0685_i_StairsNativeBitmapIndex_Down_Front_D2C
	int16 _stairsNativeBitmapIndexDownFrontD1L; // @ G0686_i_StairsNativeBitmapIndex_Down_Front_D1L
	int16 _stairsNativeBitmapIndexDownFrontD1C; // @ G0687_i_StairsNativeBitmapIndex_Down_Front_D1C
	int16 _stairsNativeBitmapIndexDownFrontD0CLeft; // @ G0688_i_StairsNativeBitmapIndex_Down_Front_D0C_Left
	int16 _stairsNativeBitmapIndexSideD2L; // @ G0689_i_StairsNativeBitmapIndex_Side_D2L
	int16 _stairsNativeBitmapIndexUpSideD1L; // @ G0690_i_StairsNativeBitmapIndex_Up_Side_D1L
	int16 _stairsNativeBitmapIndexDownSideD1L; // @ G0691_i_StairsNativeBitmapIndex_Down_Side_D1L
	int16 _stairsNativeBitmapIndexSideD0L; // @ G0692_i_StairsNativeBitmapIndex_Side_D0L

	byte *_bitmapFloor; // @ G0084_puc_Bitmap_Floor
	byte *_bitmapCeiling; // @ G0085_puc_Bitmap_Ceiling
	byte *_bitmapWallSetD3L2; // @ G0697_puc_Bitmap_WallSet_Wall_D3L2
	byte *_bitmapWallSetD3R2; // @ G0696_puc_Bitmap_WallSet_Wall_D3R2
	byte *_bitmapWallSetD3LCR; // @ G0698_puc_Bitmap_WallSet_Wall_D3LCR
	byte *_bitmapWallSetD2LCR; // @ G0699_puc_Bitmap_WallSet_Wall_D2LCR
public:
	byte *_bitmapWallSetD1LCR; // @ G0700_puc_Bitmap_WallSet_Wall_D1LCR
private:
	Box _boxThievesEyeViewPortVisibleArea; // @ G0106_s_Graphic558_Box_ThievesEye_ViewportVisibleArea
	byte _palChangesDoorButtonAndWallOrnD3[16]; // @ G0198_auc_Graphic558_PaletteChanges_DoorButtonAndWallOrnament_D3
	byte _palChangesDoorButtonAndWallOrnD2[16]; // @ G0199_auc_Graphic558_PaletteChanges_DoorButtonAndWallOrnament_D2

	byte *_bitmapWallSetWallD0L; // @ G0701_puc_Bitmap_WallSet_Wall_D0L
	byte *_bitmapWallSetWallD0R; // @ G0702_puc_Bitmap_WallSet_Wall_D0R
	byte *_bitmapWallSetDoorFrameTopD2LCR; // @ G0703_puc_Bitmap_WallSet_DoorFrameTop_D2LCR
	byte *_bitmapWallSetDoorFrameTopD1LCR; // @ G0704_puc_Bitmap_WallSet_DoorFrameTop_D1LCR
	byte *_bitmapWallSetDoorFrameLeftD3L; // @ G0705_puc_Bitmap_WallSet_DoorFrameLeft_D3L
	byte *_bitmapWallSetDoorFrameLeftD3C; // @ G0706_puc_Bitmap_WallSet_DoorFrameLeft_D3C
	byte *_bitmapWallSetDoorFrameLeftD2C; // @ G0707_puc_Bitmap_WallSet_DoorFrameLeft_D2C
	byte *_bitmapWallSetDoorFrameLeftD1C; // @ G0708_puc_Bitmap_WallSet_DoorFrameLeft_D1C
	byte *_bitmapWallSetDoorFrameRightD1C; // @ G0710_puc_Bitmap_WallSet_DoorFrameRight_D1C
	byte *_bitmapWallSetDoorFrameFront; // @ G0709_puc_Bitmap_WallSet_DoorFrameFront

	byte *_bitmapWallD3LCRFlipped; // @ G0090_puc_Bitmap_WallD3LCR_Flipped;
	byte *_bitmapWallD2LCRFlipped; // @ G0091_puc_Bitmap_WallD2LCR_Flipped;
	byte *_bitmapWallD1LCRFlipped; // @ G0092_puc_Bitmap_WallD1LCR_Flipped;
	byte *_bitmapWallD0LFlipped; // @ G0093_puc_Bitmap_WallD0L_Flipped;
	byte *_bitmapWallD0RFlipped; // @ G0094_puc_Bitmap_WallD0R_Flipped;
	byte *_bitmapWallD3LCRNative; // @ G0095_puc_Bitmap_WallD3LCR_Native;
	byte *_bitmapWallD2LCRNative; // @ G0096_puc_Bitmap_WallD2LCR_Native;
	byte *_bitmapWallD1LCRNative; // @ G0097_puc_Bitmap_WallD1LCR_Native;
	byte *_bitmapWallD0LNative; // @ G0098_puc_Bitmap_WallD0L_Native;
	byte *_bitmapWallD0RNative; // @ G0099_puc_Bitmap_WallD0R_Native;

	int16 _currentWallSet; // @ G0231_i_CurrentWallSet
	int16 _currentFloorSet;// @ G0230_i_CurrentFloorSet

	bool _useFlippedWallAndFootprintsBitmap; // @ G0076_B_UseFlippedWallAndFootprintsBitmaps

	byte _wallOrnamentCoordSets[8][13][6]; // @ G0205_aaauc_Graphic558_WallOrnamentCoordinateSets
	uint16 _doorOrnCoordSets[4][3][6]; // @ G0207_aaauc_Graphic558_DoorOrnamentCoordinateSet
	byte _doorButtonCoordSet[1]; // @ G0197_auc_Graphic558_DoorButtonCoordinateSet
	uint16 _doorButtonCoordSets[1][4][6]; // @ G0208_aaauc_Graphic558_DoorButtonCoordinate

	int16 _doorNativeBitmapIndexFrontD3LCR[2]; // @ G0693_ai_DoorNativeBitmapIndex_Front_D3LCR
	int16 _doorNativeBitmapIndexFrontD2LCR[2]; // @ G0694_ai_DoorNativeBitmapIndex_Front_D2LCR
	int16 _doorNativeBitmapIndexFrontD1LCR[2]; // @ G0695_ai_DoorNativeBitmapIndex_Front_D1LCR

	uint16 *_paletteFadeFrom; // @ K0017_pui_Palette_FadeFrom
	uint16 _paletteFadeTemporary[16]; // @ K0016_aui_Palette_FadeTemporary
public:

	uint16 _screenWidth;
	uint16 _screenHeight;
	byte *_bitmapScreen; // @ G0348_pl_Bitmap_Screen
	byte *_bitmapViewport; // @ G0296_puc_Bitmap_Viewport

	// some methods use this for a stratchpad, don't make assumptions about content between function calls
	byte *_tmpBitmap; // @ G0074_puc_Bitmap_Temporary
	bool _paletteSwitchingEnabled; // @ G0322_B_PaletteSwitchingEnabled
	bool _refreshDungeonViewPaleteRequested; // @ G0342_B_RefreshDungeonViewPaletteRequested
	int16 _dungeonViewPaletteIndex; // @ G0304_i_DungeonViewPaletteIndex
	uint16 _blankBuffer[32]; // @G0345_aui_BlankBuffer
	uint16 _paletteTopAndBottomScreen[16]; // @ G0347_aui_Palette_TopAndBottomScreen
	uint16 _paletteMiddleScreen[16]; // @ G0346_aui_Palette_MiddleScreen

	explicit DisplayMan(DMEngine *dmEngine);
	~DisplayMan();

	void loadWallSet(WallSet set); // @ F0095_DUNGEONVIEW_LoadWallSet
	void loadFloorSet(FloorSet set); // @ F0094_DUNGEONVIEW_LoadFloorSet

	void loadIntoBitmap(uint16 index, byte *destBitmap); // @ F0466_EXPAND_GraphicToBitmap
	void setUpScreens(uint16 width, uint16 height);
	void loadGraphics(); // @ F0479_MEMORY_ReadGraphicsDatHeader
	void initializeGraphicData(); // @ F0460_START_InitializeGraphicData
	void loadCurrentMapGraphics(); // @ F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF
	void allocateFlippedWallBitmaps(); // @ F0461_START_AllocateFlippedWallBitmaps
	void drawDoorBitmap(Frame *frame);// @ F0102_DUNGEONVIEW_DrawDoorBitmap
	void drawDoorFrameBitmapFlippedHorizontally(byte *bitmap, Frame *frame); // @ F0103_DUNGEONVIEW_DrawDoorFrameBitmapFlippedHorizontally
	void drawDoorButton(int16 doorButtonOrdinal, DoorButton doorButton); // @ F0110_DUNGEONVIEW_DrawDoorButton

	/// Gives the width of an IMG0 type item
	uint16 getPixelWidth(uint16 index);
	/// Gives the height of an IMG1 type item
	uint16 getPixelHeight(uint16 index);

	void copyBitmapAndFlipHorizontal(byte *srcBitmap, byte *destBitmap, uint16 byteWidth, uint16 height); // @ F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal
	void drawFloorOrnament(uint16 floorOrnOrdinal, ViewFloor viewFloorIndex); // @ F0108_DUNGEONVIEW_DrawFloorOrnament
	void drawDoor(uint16 doorThingIndex, DoorState doorState, int16 *doorNativeBitmapIndices, int16 byteCount,
					   DoorOrnament doorOrnament, DoorFrames *doorFrames); // @ F0111_DUNGEONVIEW_DrawDoor
	void drawDoorOrnament(int16 doorOrnOdinal, DoorOrnament doorOrnament); // @ F0109_DUNGEONVIEW_DrawDoorOrnament
	void drawCeilingPit(int16 nativeBitmapIndex, Frame *frame, int16 mapX, int16 mapY, bool flipHorizontal); // @ F0112_DUNGEONVIEW_DrawCeilingPit

	void blitToViewport(byte *bitmap, Box &box, int16 byteWidth, Color transparent, int16 height); // @ F0020_MAIN_BlitToViewport
	void blitToViewport(byte *bitmap, int16 *box, int16 byteWidth, Color transparent, int16 height); // @ F0020_MAIN_BlitToViewport
	void blitToScreen(byte *bitmap, const Box *box, int16 byteWidth, Color transparent, int16 height); // @ F0021_MAIN_BlitToScreen


	/* srcHeight and destHeight are not necessary for blitting, only error checking, thus they are defaulted for existing code which
	does not pass anything, newly imported calls do pass srcHeght and srcWidth, so this is a ceonvenience change so the the parameters
	match the original exactly, if need arises for heights then we'll have to retrospectively add them in old function calls*/
	/* Expects inclusive boundaries in box */
	void blitToBitmap(byte *srcBitmap, byte *destBitmap, const Box &box, uint16 srcX, uint16 srcY, uint16 srcByteWidth,
						   uint16 destByteWidth, Color transparent, int16 srcHeight, int16 destHight); // @ F0132_VIDEO_Blit
	 /* Expects inclusive boundaries in box */
	void blitBoxFilledWithMaskedBitmap(byte *src, byte *dest, byte *mask, byte *tmp, Box &box, int16 lastUnitIndex,
											int16 firstUnitIndex, int16 destByteWidth, Color transparent,
											int16 xPos, int16 yPos, int16 destHeight, int16 height2); // @ F0133_VIDEO_BlitBoxFilledWithMaskedBitmap
		 // this function takes pixel widths
	void blitToBitmapShrinkWithPalChange(byte *srcBitmap, byte *destBitmap,
											  int16 srcPixelWidth, int16 srcHight, int16 destPixelWidth, int16 destHeight, byte *palChange); // @ F0129_VIDEO_BlitShrinkWithPaletteChanges
	void flipBitmapHorizontal(byte *bitmap, uint16 byteWidth, uint16 height); // @ F0130_VIDEO_FlipHorizontal
	void flipBitmapVertical(byte *bitmap, uint16 byteWidth, uint16 height);
	byte *getExplosionBitmap(uint16 explosionAspIndex, uint16 scale, int16 &returnByteWidth, int16 &returnHeight); // @ F0114_DUNGEONVIEW_GetExplosionBitmap

	void fillBitmap(byte *bitmap, Color color, uint16 byteWidth, uint16 height); // @ F0134_VIDEO_FillBitmap
	void fillScreen(Color color);
	/* Expects inclusive boundaries in box */
	void fillScreenBox(Box &box, Color color); // @ D24_FillScreenBox, F0550_VIDEO_FillScreenBox
	/* Expects inclusive boundaries in box */
	void fillBoxBitmap(byte *destBitmap, Box &box, Color color, int16 byteWidth, int16 height); // @ F0135_VIDEO_FillBox
	void drawDungeon(Direction dir, int16 posX, int16 posY); // @ F0128_DUNGEONVIEW_Draw_CPSF
	void drawFloorAndCeiling(); // @ F0098_DUNGEONVIEW_DrawFloorAndCeiling
	void updateScreen();
	void drawViewport(int16 palSwitchingRequestedState); // @ F0097_DUNGEONVIEW_DrawViewport

	byte *getNativeBitmapOrGraphic(uint16 index); // @ F0489_MEMORY_GetNativeBitmapOrGraphic
	Common::MemoryReadStream getCompressedData(uint16 index);
	uint32 getCompressedDataSize(uint16 index);
	void drawField(FieldAspect *fieldAspect, Box &box); // @ F0113_DUNGEONVIEW_DrawField

	int16 getScaledBitmapByteCount(int16 byteWidth, int16 height, int16 scale); // @ F0459_START_GetScaledBitmapByteCount
	int16 getScaledDimension(int16 dimension, int16 scale); // @ M78_SCALED_DIMENSION
	void drawObjectsCreaturesProjectilesExplosions(Thing thingParam, Direction directionParam,
					  int16 mapXpos, int16 mapYpos, int16 viewSquareIndex,
					  CellOrder orderedViewCellOrdinals); // @ F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF
	uint16 getNormalizedByteWidth(uint16 byteWidth); // @ M77_NORMALIZED_BYTE_WIDTH
	uint16 getVerticalOffsetM23(uint16 val); // @ M23_VERTICAL_OFFSET
	uint16 getHorizontalOffsetM22(uint16 val); // @ M22_HORIZONTAL_OFFSET

	int16 _championPortraitOrdinal; // @ G0289_i_DungeonView_ChampionPortraitOrdinal
	int16 _currMapAlcoveOrnIndices[k3_AlcoveOrnCount]; // @ G0267_ai_CurrentMapAlcoveOrnamentIndices
	int16 _currMapFountainOrnIndices[k1_FountainOrnCount]; // @ G0268_ai_CurrentMapFountainOrnamentIndices
	OrnamentInfo _currMapWallOrnInfo[16]; // @ G0101_aai_CurrentMapWallOrnamentsInfo
	OrnamentInfo _currMapFloorOrnInfo[16]; // @ G0102_aai_CurrentMapFloorOrnamentsInfo
	OrnamentInfo _currMapDoorOrnInfo[17]; // @ G0103_aai_CurrentMapDoorOrnamentsInfo
	byte *_currMapAllowedCreatureTypes; // @ G0264_puc_CurrentMapAllowedCreatureTypes
	byte _currMapWallOrnIndices[16]; // @ G0261_auc_CurrentMapWallOrnamentIndices
	byte _currMapFloorOrnIndices[16]; // @ G0262_auc_CurrentMapFloorOrnamentIndices
	byte _currMapDoorOrnIndices[18]; // @ G0263_auc_CurrentMapDoorOrnamentIndices

	int16 _currMapViAltarIndex; // @ G0266_i_CurrentMapViAltarWallOrnamentIndex

	Thing _inscriptionThing; // @ G0290_T_DungeonView_InscriptionThing

	bool _drawFloorAndCeilingRequested; // @ G0297_B_DrawFloorAndCeilingRequested

	// This tells blitting functions whether to assume a BYTE_BOX or a WORD_BOX has been passed to them,
	// I only use WORD_BOX, so this will probably deem useless
	bool _useByteBoxCoordinates; // @ G0578_B_UseByteBoxCoordinates
	bool _doNotDrawFluxcagesDuringEndgame; // @ G0077_B_DoNotDrawFluxcagesDuringEndgame

	Frame _doorFrameLeftD1C; // @ G0170_s_Graphic558_Frame_DoorFrameLeft_D1C
	Frame _doorFrameRightD1C; // @ G0171_s_Graphic558_Frame_DoorFrameRight_D1C
	FieldAspect _fieldAspects188[12];
	Box _boxMovementArrows;
	byte _palChangeSmoke[16];
	byte _projectileScales[7];
	ExplosionAspect _explosionAspects[k4_ExplosionAspectCount];
	Frame _frameWallD3R2;
	Frame _frameWalls163[12];
	CreatureAspect _creatureAspects219[k27_CreatureTypeCount];
	ObjectAspect _objectAspects209[k85_ObjAspectCount]; // @ G0209_as_Graphic558_ObjectAspects
	ProjectileAspect _projectileAspect[k14_ProjectileAspectCount]; // @ G0210_as_Graphic558_ProjectileAspects
	uint16 _palCredits[16]; // @ G0019_aui_Graphic562_Palette_Credits
	uint16 _palDungeonView[6][16]; // @ G0021_aaui_Graphic562_Palette_DungeonView
	byte _palChangesCreatureD3[16]; // @ G0221_auc_Graphic558_PaletteChanges_Creature_D3
	byte _palChangesCreatureD2[16]; // @ G0222_auc_Graphic558_PaletteChanges_Creature_D2
	byte _palChangesNoChanges[16]; // @ G0017_auc_Graphic562_PaletteChanges_NoChanges
	byte _palChangesFloorOrnD3[16]; // @ G0213_auc_Graphic558_PaletteChanges_FloorOrnament_D3
	byte _palChangesFloorOrnD2[16]; // @ G0214_auc_Graphic558_PaletteChanges_FloorOrnament_D2

	bool isDerivedBitmapInCache(int16 derivedBitmapIndex); // @  F0491_CACHE_IsDerivedBitmapInCache
	byte *getDerivedBitmap(int16 derivedBitmapIndex); // @ F0492_CACHE_GetDerivedBitmap
	void addDerivedBitmap(int16 derivedBitmapIndex); // @ F0493_CACHE_AddDerivedBitmap
	void releaseBlock(uint16 index); // @ F0480_CACHE_ReleaseBlock
	uint16 getDarkenedColor(uint16 RGBcolor);
	void startEndFadeToPalette(uint16 *P0849_pui_Palette); // @ F0436_STARTEND_FadeToPalette
	void buildPaletteChangeCopperList(uint16 *middleScreen, uint16 *topAndBottom); // @ F0508_AMIGA_BuildPaletteChangeCopperList
	void shadeScreenBox(Box *box, Color color) { warning("STUB METHOD: shadeScreenBox"); } // @ F0136_VIDEO_ShadeScreenBox

private:
	void initConstants();
	uint16 getBitmapByteCount(uint16 pixelWidth, uint16 height);  // @ M75_BITMAP_BYTE_COUNT
};

}

#endif
