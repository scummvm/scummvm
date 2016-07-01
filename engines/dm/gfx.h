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

#ifndef GFX_H
#define GFX_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/memstream.h"
#include "dm.h"
#include "common/array.h"

namespace DM {

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

#define k0_NativeBitmapIndex 0 // @ C0_NATIVE_BITMAP_INDEX
#define k1_CoordinateSet 1 // @ C1_COORDINATE_SET

/* View lanes */
#define k0_ViewLaneCenter 0 // @ C0_VIEW_LANE_CENTER 
#define k1_ViewLaneLeft 1 // @ C1_VIEW_LANE_LEFT   
#define k2_ViewLaneRight 2 // @ C2_VIEW_LANE_RIGHT  

#define k0_HalfSizedViewCell_LeftColumn 0 // @ C00_VIEW_CELL_LEFT_COLUMN  
#define k1_HalfSizedViewCell_RightColumn 1 // @ C01_VIEW_CELL_RIGHT_COLUMN 
#define k2_HalfSizedViewCell_BackRow 2 // @ C02_VIEW_CELL_BACK_ROW     
#define k3_HalfSizedViewCell_CenterColumn 3 // @ C03_VIEW_CELL_CENTER_COLUMN
#define k4_HalfSizedViewCell_FrontRow 4 // @ C04_VIEW_CELL_FRONT_ROW    

/* Shift sets */
#define k0_ShiftSet_D0BackD1Front 0 // @ C0_SHIFT_SET_D0_BACK_OR_D1_FRONT
#define k1_ShiftSet_D1BackD2Front 1 // @ C1_SHIFT_SET_D1_BACK_OR_D2_FRONT
#define k2_ShiftSet_D2BackD3Front 2 // @ C2_SHIFT_SET_D2_BACK_OR_D3_FRONT

#define k0x0008_CellOrder_DoorFront 0x0008 // @ MASK0x0008_DOOR_FRONT                                   
#define k0x0000_CellOrder_Alcove 0x0000 // @ C0000_CELL_ORDER_ALCOVE                                 
#define k0x0001_CellOrder_BackLeft 0x0001 // @ C0001_CELL_ORDER_BACKLEFT                               
#define k0x0002_CellOrder_BackRight 0x0002 // @ C0002_CELL_ORDER_BACKRIGHT                              
#define k0x0018_CellOrder_DoorPass1_BackLeft 0x0018 // @ C0018_CELL_ORDER_DOORPASS1_BACKLEFT                     
#define k0x0021_CellOrder_BackLeft_BackRight 0x0021 // @ C0021_CELL_ORDER_BACKLEFT_BACKRIGHT                     
#define k0x0028_CellOrder_DoorPass1_BackRight 0x0028 // @ C0028_CELL_ORDER_DOORPASS1_BACKRIGHT                    
#define k0x0032_CellOrder_BackRight_FrontRight 0x0032 // @ C0032_CELL_ORDER_BACKRIGHT_FRONTRIGHT                   
#define k0x0039_CellOrder_DoorPass2_FrontRight 0x0039 // @ C0039_CELL_ORDER_DOORPASS2_FRONTRIGHT                   
#define k0x0041_CellOrder_BackLeft_FrontLeft 0x0041 // @ C0041_CELL_ORDER_BACKLEFT_FRONTLEFT                     
#define k0x0049_CellOrder_DoorPass2_FrontLeft 0x0049 // @ C0049_CELL_ORDER_DOORPASS2_FRONTLEFT                    
#define k0x0128_CellOrder_DoorPass1_BackRight_BackLeft 0x0128 // @ C0128_CELL_ORDER_DOORPASS1_BACKRIGHT_BACKLEFT           
#define k0x0218_CellOrder_DoorPass1_BackLeft_BackRight 0x0218 // @ C0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT           
#define k0x0321_CellOrder_BackLeft_BackRight_FrontRight 0x0321 // @ C0321_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTRIGHT          
#define k0x0342_CellOrder_BackRight_FrontLeft_FrontRight 0x0342 // @ C0342_CELL_ORDER_BACKRIGHT_FRONTLEFT_FRONTRIGHT         
#define k0x0349_CellOrder_DoorPass2_FrontLeft_FrontRight 0x0349 // @ C0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT         
#define k0x0412_CellOrder_BackRight_BackLeft_FrontLeft 0x0412 // @ C0412_CELL_ORDER_BACKRIGHT_BACKLEFT_FRONTLEFT           
#define k0x0431_CellOrder_BackLeft_FrontRight_FrontLeft 0x0431 // @ C0431_CELL_ORDER_BACKLEFT_FRONTRIGHT_FRONTLEFT          
#define k0x0439_CellOrder_DoorPass2_FrontRight_FrontLeft 0x0439 // @ C0439_CELL_ORDER_DOORPASS2_FRONTRIGHT_FRONTLEFT         
#define k0x3421_CellOrder_BackLeft_BackRight_FrontLeft_FrontRight 0x3421 // @ C3421_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT
#define k0x4312_CellOrder_BackRight_BackLeft_FrontRight_FrontLeft 0x4312 // @ C4312_CELL_ORDER_BACKRIGHT_BACKLEFT_FRONTRIGHT_FRONTLEFT




/* Explosion aspects */
#define k0_ExplosionAspectFire 0 // @ C0_EXPLOSION_ASPECT_FIRE  
#define k1_ExplosionAspectSpell 1 // @ C1_EXPLOSION_ASPECT_SPELL 
#define k2_ExplosionAspectPoison 2 // @ C2_EXPLOSION_ASPECT_POISON
#define k3_ExplosionAspectSmoke 3 // @ C3_EXPLOSION_ASPECT_SMOKE 

/* Creature info GraphicInfo */
#define k0x0003_CreatureInfoGraphicMaskAdditional 0x0003 // @ MASK0x0003_ADDITIONAL                        
#define k0x0004_CreatureInfoGraphicMaskFlipNonAttack 0x0004 // @ MASK0x0004_FLIP_NON_ATTACK                  
#define k0x0008_CreatureInfoGraphicMaskSide 0x0008 // @ MASK0x0008_SIDE                             
#define k0x0010_CreatureInfoGraphicMaskBack 0x0010 // @ MASK0x0010_BACK                             
#define k0x0020_CreatureInfoGraphicMaskAttack 0x0020 // @ MASK0x0020_ATTACK                           
#define k0x0080_CreatureInfoGraphicMaskSpecialD2Front 0x0080 // @ MASK0x0080_SPECIAL_D2_FRONT                                                                               
#define k0x0100_CreatureInfoGraphicMaskSpecialD2FrontIsFlipped 0x0100 // @ MASK0x0100_SPECIAL_D2_FRONT_IS_FLIPPED_FRONT 
#define k0x0200_CreatureInfoGraphicMaskFlipAttack 0x0200 // @ MASK0x0200_FLIP_ATTACK                      
#define k0x0400_CreatureInfoGraphicMaskFlipDuringAttack 0x0400 // @ MASK0x0400_FLIP_DURING_ATTACK               

class ExplosionAspect {
public:
	uint16 _pixelWidth;
	uint16 _height;

	ExplosionAspect(uint16 byteWidth, uint16 height) :_pixelWidth(byteWidth * 2), _height(height) {}
}; // @ EXPLOSION_ASPECT

extern ExplosionAspect g211_ExplosionAspects[k4_ExplosionAspectCount]; // @ G0211_as_Graphic558_ExplosionAspects

extern byte g215_ProjectileScales[7]; // @ G0215_auc_Graphic558_ProjectileScales


#define k0_DerivedBitmapViewport 0 // @ C000_DERIVED_BITMAP_VIEWPORT                    
#define k1_DerivedBitmapThievesEyeVisibleArea 1 // @ C001_DERIVED_BITMAP_THIEVES_EYE_VISIBLE_AREA    
#define k2_DerivedBitmapDamageToCreatureMedium 2 // @ C002_DERIVED_BITMAP_DAMAGE_TO_CREATURE_MEDIUM   
#define k3_DerivedBitmapDamageToCreatureSmall 3 // @ C003_DERIVED_BITMAP_DAMAGE_TO_CREATURE_SMALL    
#define k4_DerivedBitmapFirstWallOrnament 4 // @ C004_DERIVED_BITMAP_FIRST_WALL_ORNAMENT         
#define k68_DerivedBitmapFirstDoorOrnament_D3 68 // @ C068_DERIVED_BITMAP_FIRST_DOOR_ORNAMENT_D3     
#define k69_DerivedBitmapFirstDoorOrnament_D2 69 // @ C069_DERIVED_BITMAP_FIRST_DOOR_ORNAMENT_D2    
#define k102_DerivedBitmapFirstDoorButton 102 // @ C102_DERIVED_BITMAP_FIRST_DOOR_BUTTON         
#define k104_DerivedBitmapFirstObject 104 // @ C104_DERIVED_BITMAP_FIRST_OBJECT              
#define k282_DerivedBitmapFirstProjectile 282 // @ C282_DERIVED_BITMAP_FIRST_PROJECTILE          
#define k438_DerivedBitmapFirstExplosion 438 // @ C438_DERIVED_BITMAP_FIRST_EXPLOSION           
#define k495_DerivedBitmapFirstCreature 495 // @ C495_DERIVED_BITMAP_FIRST_CREATURE            


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

enum ViewCell {
	k0_ViewCellFronLeft = 0, // @ C00_VIEW_CELL_FRONT_LEFT
	k1_ViewCellFrontRight = 1, // @ C01_VIEW_CELL_FRONT_RIGHT
	k2_ViewCellBackRight = 2, // @ C02_VIEW_CELL_BACK_RIGHT
	k3_ViewCellBackLeft = 3, // @ C03_VIEW_CELL_BACK_LEFT
	k4_ViewCellAlcove = 4, // @ C04_VIEW_CELL_ALCOVE
	k5_ViewCellDoorButtonOrWallOrn = 5 // @ C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT
};

enum GraphicIndice {
	k8_StatusBoxDeadChampion = 8, // @ C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION
	k9_MenuSpellAreaBackground = 9, // @ C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND
	k10_MenuActionAreaIndice = 10, // @ C010_GRAPHIC_MENU_ACTION_AREA
	k11_MenuSpellAreLinesIndice = 11, // @ C011_GRAPHIC_MENU_SPELL_AREA_LINES
	k13_MovementArrowsIndice = 13, // @ C013_GRAPHIC_MOVEMENT_ARROWS
	k17_InventoryGraphicIndice = 17, // @ C017_GRAPHIC_INVENTORY
	k18_ArrowForChestContentIndice = 18, // @ C018_GRAPHIC_ARROW_FOR_CHEST_CONTENT
	k19_EyeForObjectDescriptionIndice = 19, // @ C019_GRAPHIC_EYE_FOR_OBJECT_DESCRIPTION
	k20_PanelEmptyIndice = 20, // @ C020_GRAPHIC_PANEL_EMPTY
	k23_PanelOpenScrollIndice = 23, // @ C023_GRAPHIC_PANEL_OPEN_SCROLL
	k25_PanelOpenChestIndice = 25, // @ C025_GRAPHIC_PANEL_OPEN_CHEST
	k26_ChampionPortraitsIndice = 26, // @ C026_GRAPHIC_CHAMPION_PORTRAITS
	k27_PanelRenameChampionIndice = 27, // @ C027_GRAPHIC_PANEL_RENAME_CHAMPION
	k28_ChampionIcons = 28, // @ C028_GRAPHIC_CHAMPION_ICONS
	k29_ObjectDescCircleIndice = 29, // @ C029_GRAPHIC_OBJECT_DESCRIPTION_CIRCLE
	k30_FoodLabelIndice = 30, // @ C030_GRAPHIC_FOOD_LABEL
	k31_WaterLabelIndice = 31, // @ C031_GRAPHIC_WATER_LABEL
	k32_PoisionedLabelIndice = 32, // @ C032_GRAPHIC_POISONED_LABEL
	k33_SlotBoxNormalIndice = 33, // @ C033_GRAPHIC_SLOT_BOX_NORMAL                   
	k34_SlotBoxWoundedIndice = 34, // @ C034_GRAPHIC_SLOT_BOX_WOUNDED                  
	k35_SlotBoxActingHandIndice = 35, // @ C035_GRAPHIC_SLOT_BOX_ACTING_HAND
	k37_BorderPartyShieldIndice = 37, // @ C037_GRAPHIC_BORDER_PARTY_SHIELD
	k38_BorderPartyFireshieldIndice = 38, // @ C038_GRAPHIC_BORDER_PARTY_FIRESHIELD
	k39_BorderPartySpellshieldIndice = 39, // @ C039_GRAPHIC_BORDER_PARTY_SPELLSHIELD
	k40_PanelResurectReincaranteIndice = 40, // @ C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE 
	k42_ObjectIcons_000_TO_031 = 42, // @ C042_GRAPHIC_OBJECT_ICONS_000_TO_031
	k43_ObjectIcons_032_TO_063 = 43, // @ C043_GRAPHIC_OBJECT_ICONS_032_TO_063
	k44_ObjectIcons_064_TO_095 = 44, // @ C044_GRAPHIC_OBJECT_ICONS_064_TO_095
	k45_ObjectIcons_096_TO_127 = 45, // @ C045_GRAPHIC_OBJECT_ICONS_096_TO_127
	k46_ObjectIcons_128_TO_159 = 46, // @ C046_GRAPHIC_OBJECT_ICONS_128_TO_159
	k47_ObjectIcons_160_TO_191 = 47, // @ C047_GRAPHIC_OBJECT_ICONS_160_TO_191
	k48_ObjectIcons_192_TO_223 = 48, // @ C048_GRAPHIC_OBJECT_ICONS_192_TO_223
	k49_FloorPit_D3L_GraphicIndice = 49, // @ C049_GRAPHIC_FLOOR_PIT_D3L
	k50_FloorPit_D3C_GraphicIndice = 50, // @ C050_GRAPHIC_FLOOR_PIT_D3C
	k51_FloorPit_D2L_GraphicIndice = 51, // @ C051_GRAPHIC_FLOOR_PIT_D2L
	k52_FloorPit_D2C_GraphicIndice = 52, // @ C052_GRAPHIC_FLOOR_PIT_D2C
	k53_FloorPit_D1L_GraphicIndice = 53, // @ C053_GRAPHIC_FLOOR_PIT_D1L
	k54_FloorPit_D1C_GraphicIndice = 54, // @ C054_GRAPHIC_FLOOR_PIT_D1C
	k55_FloorPit_D0L_GraphicIndice = 55, // @ C055_GRAPHIC_FLOOR_PIT_D0L
	k56_FloorPit_D0C_GraphicIndice = 56, // @ C056_GRAPHIC_FLOOR_PIT_D0C
	k69_FieldMask_D3R_GraphicIndice = 69, // @ C069_GRAPHIC_FIELD_MASK_D3R
	k73_FieldTeleporterGraphicIndice = 73, // @ C073_GRAPHIC_FIELD_TELEPORTER
	k120_InscriptionFontIndice = 120, // @ C120_GRAPHIC_INSCRIPTION_FONT
	k241_FloorOrn_15_D3L_footprints = 241, // @ C241_GRAPHIC_FLOOR_ORNAMENT_15_D3L_FOOTPRINTS
	k301_DoorMaskDestroyedIndice = 301, // @ C301_GRAPHIC_DOOR_MASK_DESTROYED
	k316_FirstProjectileGraphicIndice = 316, // @ C316_GRAPHIC_FIRST_PROJECTILE 
	k348_FirstExplosionGraphicIndice = 348, // @ C348_GRAPHIC_FIRST_EXPLOSION
	k351_FirstExplosionPatternGraphicIndice = 351, // @ C351_GRAPHIC_FIRST_EXPLOSION_PATTERN 
	k360_FirstObjectGraphicIndice = 360, // @ C360_GRAPHIC_FIRST_OBJECT
	k446_FirstCreatureGraphicIndice = 446, // @ C446_GRAPHIC_FIRST_CREATURE
	k557_FontGraphicIndice = 557 // @ C557_GRAPHIC_FONT 
};

extern uint16 gK57_PalSwoosh[16]; // @ K0057_aui_Palette_Swoosh
extern uint16 gK150_PalMousePointer[16];	// @ K0150_aui_Palette_MousePointer
extern uint16 g19_PalCredits[16];  // @ G0019_aui_Graphic562_Palette_Credits
extern uint16 g20_PalEntrance[16]; // @ G0020_aui_Graphic562_Palette_Entrance
extern uint16 g21_PalDungeonView[6][16]; // @ G0021_aaui_Graphic562_Palette_DungeonView

class Box {
public:
	uint16 _x1;
	uint16 _x2;
	uint16 _y1;
	uint16 _y2;

	Box(uint16 x1, uint16 x2, uint16 y1, uint16 y2) : _x1(x1), _x2(x2 + 1), _y1(y1), _y2(y2 + 1) {}
	Box() {}
	bool isPointInside(Common::Point point) {
		return (_x1 <= point.x) && (point.x < _x2) && (_y1 <= point.y) && (point.y < _y2);
	}
	void setToZero() { _x1 = _x2 = _y1 = _y2 = 0; }
}; // @ BOX_BYTE, BOX_WORD

extern Box g2_BoxMovementArrows; // G0002_s_Graphic562_Box_MovementArrows

class Frame {
public:
	Box _box;
	uint16 _srcWidth, _srcHeight;
	uint16 _srcX, _srcY;

	Frame() {}
	Frame(uint16 destFromX, uint16 destToX, uint16 destFromY, uint16 destToY,
		  uint16 srcWidth, uint16 srcHeight, uint16 srcX, uint16 srcY) :
		_box(destFromX, destToX, destFromY, destToY),
		_srcWidth(srcWidth * 2), _srcHeight(srcHeight), _srcX(srcX), _srcY(srcY) {}
};

enum WallSet {
	k0_WallSetStone = 0 // @ C0_WALL_SET_STONE
};

enum FloorSet {
	k0_FloorSetStone = 0 // @ C0_FLOOR_SET_STONE
};

enum ViewWall {
	k0_ViewWall_D3L_RIGHT = 0, // @ C00_VIEW_WALL_D3L_RIGHT 
	k1_ViewWall_D3R_LEFT = 1, // @ C01_VIEW_WALL_D3R_LEFT  
	k2_ViewWall_D3L_FRONT = 2, // @ C02_VIEW_WALL_D3L_FRONT 
	k3_ViewWall_D3C_FRONT = 3, // @ C03_VIEW_WALL_D3C_FRONT 
	k4_ViewWall_D3R_FRONT = 4, // @ C04_VIEW_WALL_D3R_FRONT 
	k5_ViewWall_D2L_RIGHT = 5, // @ C05_VIEW_WALL_D2L_RIGHT 
	k6_ViewWall_D2R_LEFT = 6, // @ C06_VIEW_WALL_D2R_LEFT  
	k7_ViewWall_D2L_FRONT = 7, // @ C07_VIEW_WALL_D2L_FRONT 
	k8_ViewWall_D2C_FRONT = 8, // @ C08_VIEW_WALL_D2C_FRONT 
	k9_ViewWall_D2R_FRONT = 9, // @ C09_VIEW_WALL_D2R_FRONT 
	k10_ViewWall_D1L_RIGHT = 10, // @ C10_VIEW_WALL_D1L_RIGHT
	k11_ViewWall_D1R_LEFT = 11, // @ C11_VIEW_WALL_D1R_LEFT 
	k12_ViewWall_D1C_FRONT = 12  // @ C12_VIEW_WALL_D1C_FRONT
};

enum Color {
	k255_ColorNoTransparency = 255,
	k0_ColorBlack = 0,
	k1_ColorDarkGary = 1,
	k2_ColorLightGray = 2,
	k3_ColorDarkBrown = 3,
	k4_ColorCyan = 4,
	k5_ColorLightBrown = 5,
	k6_ColorDarkGreen = 6,
	k7_ColorLightGreen = 7,
	k8_ColorRed = 8,
	k9_ColorGold = 9,
	k10_ColorFlesh = 10,
	k11_ColorYellow = 11,
	k12_ColorDarkestGray = 12,
	k13_ColorLightestGray = 13,
	k14_ColorBlue = 14,
	k15_ColorWhite = 15
};

class FieldAspect {
public:
	uint16 _nativeBitmapRelativeIndex;
	uint16 _baseStartUnitIndex; /* Index of the unit (16 pixels = 8 bytes) in bitmap where blit will start from. A random value of 0 or 1 is added to this base index */
	uint16 _transparentColor; /* Bit 7: Do not use mask if set, Bits 6-0: Transparent color index. 0xFF = no transparency */
	byte _mask; /* Bit 7: Flip, Bits 6-0: Mask index. 0xFF = no mask */
	uint16 _pixelWidth;
	uint16 _height;
	uint16 _xPos;
	uint16 _bitplaneWordCount;
	FieldAspect(uint16 native, uint16 base, uint16 transparent, byte mask, uint16 byteWidth, uint16 height, uint16 xPos, uint16 bitplane)
		: _nativeBitmapRelativeIndex(native), _baseStartUnitIndex(base), _transparentColor(transparent), _mask(mask),
		_pixelWidth(byteWidth * 2), _height(height), _xPos(xPos), _bitplaneWordCount(bitplane) {}
	FieldAspect() {}
}; // @ FIELD_ASPECT


class Viewport {
public:
	uint16 _posX, _posY;
	uint16 _width, _height;
	Viewport() {}
	Viewport(uint16 posX, uint16 posY, uint16 width, uint16 height)
		:_posX(posX), _posY(posY), _width(width), _height(height) {}
};

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
		: _firstNativeBitmapRelativeIndex(uint161),
		_firstDerivedBitmapIndex(uint162),
		_byteWidthFront(byte0 * 2),
		_heightFront(byte1),
		_byteWidthSide(byte2 * 2),
		_heightSide(byte3),
		_byteWidthAttack(byte4 * 2),
		_heightAttack(byte5),
		_coordinateSet_TransparentColor(byte6),
		_replacementColorSetIndices(byte7) {}

	byte getCoordSet() { return (_coordinateSet_TransparentColor >> 4) & 0xF; } // @ M71_COORDINATE_SET
	byte getTranspColour() { return  _coordinateSet_TransparentColor & 0xF; } // @ M72_TRANSPARENT_COLOR
	byte getReplColour10() { return (_replacementColorSetIndices >> 4) & 0xF; } // @ M74_COLOR_10_REPLACEMENT_COLOR_SET
	byte getReplColour9() { return _replacementColorSetIndices & 0xF; } // @ M73_COLOR_09_REPLACEMENT_COLOR_SET


}; // @ CREATURE_ASPECT

class ObjectAspect {
public:
	byte _firstNativeBitmapRelativeIndex;
	byte _firstDerivedBitmapRelativeIndex;
	byte _width;
	byte _height;
	byte _graphicInfo; /* Bits 7-5 and 3-1 Unreferenced */
	byte _coordinateSet;
	ObjectAspect(byte firstN, byte firstD, byte byteWidth, byte h, byte grap, byte coord) :
		_firstNativeBitmapRelativeIndex(firstN), _firstDerivedBitmapRelativeIndex(firstD),
		_width(byteWidth * 2), _height(h), _graphicInfo(grap), _coordinateSet(coord) {}
}; // @ OBJECT_ASPECT

class ProjectileAspect {
public:
	byte _firstNativeBitmapRelativeIndex;
	byte _firstDerivedBitmapRelativeIndex;
	byte _width;
	byte _height;
	uint16 _graphicInfo; /* Bits 15-9, 7-5 and 3-2 Unreferenced */

	ProjectileAspect(byte firstN, byte firstD, byte byteWidth, byte h, uint16 grap) :
		_firstNativeBitmapRelativeIndex(firstN), _firstDerivedBitmapRelativeIndex(firstD),
		_width(byteWidth * 2), _height(h), _graphicInfo(grap) {}
}; // @ PROJECTIL_ASPECT

class CreatureReplColorSet {
public:
	uint16 _RGBColor[6];
	byte _D2ReplacementColor;
	byte _D3ReplacementColor;
}; // @ CREATURE_REPLACEMENT_COLOR_SET

extern Viewport gDefultViewPort;
extern Viewport gDungeonViewport;


#define k0_DoorButton 0 // @ C0_DOOR_BUTTON
#define k0_WallOrnInscription 0 // @ C0_WALL_ORNAMENT_INSCRIPTION
#define k15_FloorOrnFootprints 15 // @ C15_FLOOR_ORNAMENT_FOOTPRINTS
#define k15_DoorOrnDestroyedMask 15 // @ C15_DOOR_ORNAMENT_DESTROYED_MASK
#define k16_DoorOrnThivesEyeMask 16 // @ C16_DOOR_ORNAMENT_THIEVES_EYE_MASK

class DisplayMan {
	friend class DM::TextMan;

	DMEngine *_vm;
	uint16 _screenWidth;
	uint16 _screenHeight;
	byte *_vgaBuffer;

	/// Related to graphics.dat file
	uint16 _grapItemCount; // @ G0632_ui_GraphicCount
	uint32 *_packedItemPos;
	byte *_packedBitmaps;
	byte **_bitmaps;

	// pointers 13,14 and [15-19] are owned by this array
	byte *_wallSetBitMaps[25];	// @G[0696..0710]_puc_Bitmap_WallSet_...

	// pointers are not owned by these fields
	byte *_g84_floorBitmap; // @ G0084_puc_Bitmap_Floor
	byte *_g85_ceilingBitmap; // @ G0085_puc_Bitmap_Ceiling
	byte *_g75_palChangesProjectile[4]; // @G0075_apuc_PaletteChanges_Projectile

	DisplayMan(const DisplayMan &other); // no implementation on purpose
	void operator=(const DisplayMan &rhs); // no implementation on purpose

	byte *getCurrentVgaBuffer();
	// the original function has two position parameters, but they are always set to zero
	void loadIntoBitmap(uint16 index, byte *destBitmap); // @ F0466_EXPAND_GraphicToBitmap
	void unpackGraphics();
	void loadFNT1intoBitmap(uint16 index, byte *destBitmap);

	void drawFloorPitOrStairsBitmapFlippedHorizontally(uint16 nativeIndex, Frame &frame); // @ F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally
	void drawFloorPitOrStairsBitmap(uint16 nativeIndex, Frame &frame); // @ F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap
	void drawWallSetBitmap(byte *bitmap, Frame &f); // @ F0100_DUNGEONVIEW_DrawWallSetBitmap
	void drawWallSetBitmapWithoutTransparency(byte *bitmap, Frame &f); // @ F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency
	void drawSquareD3L(direction dir, int16 posX, int16 posY); // @ F0116_DUNGEONVIEW_DrawSquareD3L
	void drawSquareD3R(direction dir, int16 posX, int16 posY); // @ F0117_DUNGEONVIEW_DrawSquareD3R
	void drawSquareD3C(direction dir, int16 posX, int16 posY); // @ F0118_DUNGEONVIEW_DrawSquareD3C_CPSF
	void drawSquareD2L(direction dir, int16 posX, int16 posY); // @ F0119_DUNGEONVIEW_DrawSquareD2L
	void drawSquareD2R(direction dir, int16 posX, int16 posY); // @ F0120_DUNGEONVIEW_DrawSquareD2R_CPSF
	void drawSquareD2C(direction dir, int16 posX, int16 posY); // @ F0121_DUNGEONVIEW_DrawSquareD2C
	void drawSquareD1L(direction dir, int16 posX, int16 posY); // @ F0122_DUNGEONVIEW_DrawSquareD1L
	void drawSquareD1R(direction dir, int16 posX, int16 posY); // @ F0122_DUNGEONVIEW_DrawSquareD1R
	void drawSquareD1C(direction dir, int16 posX, int16 posY); // @ F0124_DUNGEONVIEW_DrawSquareD1C
	void drawSquareD0L(direction dir, int16 posX, int16 posY); // @ F0125_DUNGEONVIEW_DrawSquareD0L
	void drawSquareD0R(direction dir, int16 posX, int16 posY); // @ F0126_DUNGEONVIEW_DrawSquareD0R
	void drawSquareD0C(direction dir, int16 posX, int16 posY); // @ F0127_DUNGEONVIEW_DrawSquareD0C


	void applyCreatureReplColors(int replacedColor, int replacementColor); // @ F0093_DUNGEONVIEW_ApplyCreatureReplacementColors

	bool isDrawnWallOrnAnAlcove(int16 wallOrnOrd, ViewWall viewWallIndex); // @ F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF

	uint16 *_g639_derivedBitmapByteCount; // @ G0639_pui_DerivedBitmapByteCount
	byte **_g638_derivedBitmaps; // @ G0638_pui_DerivedBitmapBlockIndices

	int16 _g675_stairsNativeBitmapIndex_Up_Front_D3L; // @ G0675_i_StairsNativeBitmapIndex_Up_Front_D3L
	int16 _g676_stairsNativeBitmapIndex_Up_Front_D3C; // @ G0676_i_StairsNativeBitmapIndex_Up_Front_D3C
	int16 _g677_stairsNativeBitmapIndex_Up_Front_D2L; // @ G0677_i_StairsNativeBitmapIndex_Up_Front_D2L
	int16 _g678_stairsNativeBitmapIndex_Up_Front_D2C; // @ G0678_i_StairsNativeBitmapIndex_Up_Front_D2C
	int16 _g679_stairsNativeBitmapIndex_Up_Front_D1L; // @ G0679_i_StairsNativeBitmapIndex_Up_Front_D1L
	int16 _g680_stairsNativeBitmapIndex_Up_Front_D1C; // @ G0680_i_StairsNativeBitmapIndex_Up_Front_D1C
	int16 _g681_stairsNativeBitmapIndex_Up_Front_D0C_Left; // @ G0681_i_StairsNativeBitmapIndex_Up_Front_D0C_Left
	int16 _g682_stairsNativeBitmapIndex_Down_Front_D3L; // @ G0682_i_StairsNativeBitmapIndex_Down_Front_D3L
	int16 _g683_stairsNativeBitmapIndex_Down_Front_D3C; // @ G0683_i_StairsNativeBitmapIndex_Down_Front_D3C
	int16 _g684_stairsNativeBitmapIndex_Down_Front_D2L; // @ G0684_i_StairsNativeBitmapIndex_Down_Front_D2L
	int16 _g685_stairsNativeBitmapIndex_Down_Front_D2C; // @ G0685_i_StairsNativeBitmapIndex_Down_Front_D2C
	int16 _g686_stairsNativeBitmapIndex_Down_Front_D1L; // @ G0686_i_StairsNativeBitmapIndex_Down_Front_D1L
	int16 _g687_stairsNativeBitmapIndex_Down_Front_D1C; // @ G0687_i_StairsNativeBitmapIndex_Down_Front_D1C
	int16 _g688_stairsNativeBitmapIndex_Down_Front_D0C_Left; // @ G0688_i_StairsNativeBitmapIndex_Down_Front_D0C_Left
	int16 _g689_stairsNativeBitmapIndex_Side_D2L; // @ G0689_i_StairsNativeBitmapIndex_Side_D2L
	int16 _g690_stairsNativeBitmapIndex_Up_Side_D1L; // @ G0690_i_StairsNativeBitmapIndex_Up_Side_D1L
	int16 _g691_stairsNativeBitmapIndex_Down_Side_D1L; // @ G0691_i_StairsNativeBitmapIndex_Down_Side_D1L
	int16 _g692_stairsNativeBitmapIndex_Side_D0L; // @ G0692_i_StairsNativeBitmapIndex_Side_D0L
public:
	// some methods use this for a stratchpad, don't make assumptions about content between function calls
	byte *_g74_tmpBitmap; // @ G0074_puc_Bitmap_Temporary

	explicit DisplayMan(DMEngine *dmEngine);
	~DisplayMan();

	void loadWallSet(WallSet set); // @ F0095_DUNGEONVIEW_LoadWallSet
	void loadFloorSet(FloorSet set); // @ F0094_DUNGEONVIEW_LoadFloorSet

	void setUpScreens(uint16 width, uint16 height);
	void loadGraphics(); // @ F0479_MEMORY_ReadGraphicsDatHeader, F0460_START_InitializeGraphicData
	void loadCurrentMapGraphics(); // @ F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF
	void loadPalette(uint16 *palette);

	/// Gives the width of an IMG0 type item
	uint16 getWidth(uint16 index);
	/// Gives the height of an IMG1 type item
	uint16 getHeight(uint16 index);

	void blitToBitmap(byte *srcBitmap, uint16 srcWidth, uint16 srcX, uint16 srcY,
					  byte *destBitmap, uint16 destWidth,
					  uint16 destFromX, uint16 destToX, uint16 destFromY, uint16 destToY,
					  Color transparent = k255_ColorNoTransparency, Viewport &viewport = gDefultViewPort);
	void blitToBitmap(byte *srcBitmap, uint16 srcWidth, uint16 srcX, uint16 srcY,
					  byte *destBitmap, uint16 destWidth, Box &box, Color transparent = k255_ColorNoTransparency, Viewport &viewport = gDefultViewPort);
	void blitToBitmapShrinkWithPalChange(byte *srcBitmap, int16 srcWidth, int16 srcHight,
										 byte *destBitmap, int16 destWidth, int16 destHeight, byte *palChange); // @ F0129_VIDEO_BlitShrinkWithPaletteChanges

	void blitToBitmap(byte *srcBitmap, uint16 srcWidth, uint16 srcHeight, byte *destBitmap, uint16 destWidth, uint16 destX = 0, uint16 destY = 0);
	void blitToScreen(byte *srcBitmap, uint16 srcWidth, uint16 srcX, uint16 srcY,
					  uint16 destFromX, uint16 destToX, uint16 destFromY, uint16 destToY,
					  Color transparent = k255_ColorNoTransparency, Viewport &viewport = gDefultViewPort);
	void blitToScreen(byte *srcBitmap, uint16 srcWidth, uint16 srcX, uint16 srcY,
					  Box &box,
					  Color transparent = k255_ColorNoTransparency, Viewport &viewport = gDefultViewPort);
	void blitBoxFilledWithMaskedBitmap(byte *src, byte *dest, byte *mask, byte *tmp, Box &box, int16 lastUnitIndex,
									   int16 firstUnitIndex, int16 destPixelWidth, Color transparent,
									   int16 xPos, int16 yPos, int16 destHeight, int16 height2, Viewport &viewport = gDefultViewPort); // @ F0133_VIDEO_BlitBoxFilledWithMaskedBitmap
	void blitBoxFilledWithMaskedBitmapToScreen(byte *src, byte *mask, byte *tmp, Box &box, int16 lastUnitIndex,
											   int16 firstUnitIndex, int16 destPixelWidth, Color transparent,
											   int16 xPos, int16 yPos, int16 destHeight, int16 height2, Viewport &viewport = gDungeonViewport); // @ F0133_VIDEO_BlitBoxFilledWithMaskedBitmap

	void flipBitmapHorizontal(byte *bitmap, uint16 width, uint16 height);
	void flipBitmapVertical(byte *bitmap, uint16 width, uint16 height);
	byte *getExplosionBitmap(uint16 explosionAspIndex, uint16 scale, int16 &returnPixelWidth, int16 &returnHeight); // @ F0114_DUNGEONVIEW_GetExplosionBitmap

	void clearBitmap(byte *bitmap, uint16 width, uint16 height, Color color);
	void clearScreen(Color color);
	void clearScreenBox(Color color, Box &box, Viewport &viewport = gDefultViewPort); // @ D24_FillScreenBox
	void drawDungeon(direction dir, int16 posX, int16 posY); // @ F0128_DUNGEONVIEW_Draw_CPSF
	void updateScreen();
	byte* getBitmap(uint16 index);
	Common::MemoryReadStream getCompressedData(uint16 index);
	uint32 getCompressedDataSize(uint16 index);
	void drawField(FieldAspect *fieldAspect, Box &box); // @ F0113_DUNGEONVIEW_DrawField

	int16 getScaledBitmapPixelCount(int16 pixelWidth, int16 pixelHeight, int16 scale); // @ F0459_START_GetScaledBitmapByteCount
	int16 getScaledDimension(int16 dimension, int16 scale); // @ M78_SCALED_DIMENSION
	void cthulhu(Thing thingParam, direction directionParam,
				 int16 mapXpos, int16 mapYpos, int16 viewSquareIndex,
				 uint16 orderedViewCellOrdinals); // @ F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF
	uint16 getNormalizedByteWidthM77(uint16 byteWidth); // @ M77_NORMALIZED_BYTE_WIDTH
	uint16 getVerticalOffsetM23(uint16 val); // @ M23_VERTICAL_OFFSET
	uint16 getHorizontalOffsetM22(uint16 val); // @ M22_HORIZONTAL_OFFSET

	int16 _g289_championPortraitOrdinal; // @ G0289_i_DungeonView_ChampionPortraitOrdinal
	int16 _g267_currMapAlcoveOrnIndices[k3_AlcoveOrnCount]; // @ G0267_ai_CurrentMapAlcoveOrnamentIndices
	int16 _g268_currMapFountainOrnIndices[k1_FountainOrnCount]; // @ G0268_ai_CurrentMapFountainOrnamentIndices
	int16 _g101_currMapWallOrnInfo[16][2]; // @ G0101_aai_CurrentMapWallOrnamentsInfo
	int16 _g102_currMapFloorOrnInfo[16][2]; // @ G0102_aai_CurrentMapFloorOrnamentsInfo
	int16 _g103_currMapDoorOrnInfo[17][2]; // @ G0103_aai_CurrentMapDoorOrnamentsInfo
	byte *_g264_currMapAllowedCreatureTypes; // @ G0264_puc_CurrentMapAllowedCreatureTypes
	byte _g261_currMapWallOrnIndices[16]; // @ G0261_auc_CurrentMapWallOrnamentIndices
	byte _g262_currMapFloorOrnIndices[16]; // @ G0262_auc_CurrentMapFloorOrnamentIndices
	byte _g263_currMapDoorOrnIndices[18]; // @ G0263_auc_CurrentMapDoorOrnamentIndices

	int16 _g266_currMapViAltarIndex; // @ G0266_i_CurrentMapViAltarWallOrnamentIndex

	Thing _g290_inscriptionThing; // @ G0290_T_DungeonView_InscriptionThing

	bool _g578_useByteBoxCoordinates; // @ G0578_B_UseByteBoxCoordinates
	bool _g77_doNotDrawFluxcagesDuringEndgame; // @ G0077_B_DoNotDrawFluxcagesDuringEndgame

	bool isDerivedBitmapInCache(int16 derivedBitmapIndex); // @  F0491_CACHE_IsDerivedBitmapInCache
	byte *getDerivedBitmap(int16 derivedBitmapIndex); // @ F0492_CACHE_GetDerivedBitmap



};

}

#endif
