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

#define k0_viewFloor_D3L 0 // @ C0_VIEW_FLOOR_D3L
#define k1_viewFloor_D3C 1 // @ C1_VIEW_FLOOR_D3C
#define k2_viewFloor_D3R 2 // @ C2_VIEW_FLOOR_D3R
#define k3_viewFloor_D2L 3 // @ C3_VIEW_FLOOR_D2L
#define k4_viewFloor_D2C 4 // @ C4_VIEW_FLOOR_D2C
#define k5_viewFloor_D2R 5 // @ C5_VIEW_FLOOR_D2R
#define k6_viewFloor_D1L 6 // @ C6_VIEW_FLOOR_D1L
#define k7_viewFloor_D1C 7 // @ C7_VIEW_FLOOR_D1C
#define k8_viewFloor_D1R 8 // @ C8_VIEW_FLOOR_D1R

#define k0_doorState_OPEN 0 // @ C0_DOOR_STATE_OPEN 
#define k1_doorState_FOURTH 1 // @ C1_DOOR_STATE_CLOSED_ONE_FOURTH 
#define k2_doorState_HALF 2 // @ k2_DoorStateAspect_CLOSED_HALF 
#define k3_doorState_FOURTH 3 // @ C3_DOOR_STATE_CLOSED_THREE_FOURTH
#define k4_doorState_CLOSED 4 // @ C4_DOOR_STATE_CLOSED 
#define k5_doorState_DESTROYED 5 // @ C5_DOOR_STATE_DESTROYED 

#define k0_ViewDoorOrnament_D3LCR 0 // @ C0_VIEW_DOOR_ORNAMENT_D3LCR
#define k1_ViewDoorOrnament_D2LCR 1 // @ C1_VIEW_DOOR_ORNAMENT_D2LCR
#define k2_ViewDoorOrnament_D1LCR 2 // @ C2_VIEW_DOOR_ORNAMENT_D1LCR

#define k0_viewDoorButton_D3R 0 // @ C0_VIEW_DOOR_BUTTON_D3R
#define k1_viewDoorButton_D3C 1 // @ C1_VIEW_DOOR_BUTTON_D3C
#define k2_viewDoorButton_D2C 2 // @ C2_VIEW_DOOR_BUTTON_D2C
#define k3_viewDoorButton_D1C 3 // @ C3_VIEW_DOOR_BUTTON_D1C

#define k0x0001_MaskDoorInfo_CraturesCanSeeThrough 0x0001 // @ MASK0x0001_CREATURES_CAN_SEE_THROUGH   
#define k0x0002_MaskDoorInfo_ProjectilesCanPassThrough 0x0002 // @ MASK0x0002_PROJECTILES_CAN_PASS_THROUGH
#define k0x0004_MaskDoorInfo_Animated 0x0004 // @ MASK0x0004_ANIMATED                    


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
	uint16 _byteWidth;
	uint16 _height;

	ExplosionAspect(uint16 byteWidth, uint16 height) :_byteWidth(byteWidth), _height(height) {}
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
	k0_dialogBoxGraphicIndice = 0, // @ C000_GRAPHIC_DIALOG_BOX
	k2_entranceLeftDoorGraphicIndice = 2, // @ C002_GRAPHIC_ENTRANCE_LEFT_DOOR
	k3_entranceRightDoorGraphicIndice = 3, // @ C003_GRAPHIC_ENTRANCE_RIGHT_DOOR
	k4_entranceGraphicIndice = 4, // @ C004_GRAPHIC_ENTRANCE
	k5_creditsGraphicIndice = 5, // @ C005_GRAPHIC_CREDITS
	k6_theEndIndice = 6, // @ C006_GRAPHIC_THE_END
	k8_StatusBoxDeadChampion = 8, // @ C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION
	k9_MenuSpellAreaBackground = 9, // @ C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND
	k10_MenuActionAreaIndice = 10, // @ C010_GRAPHIC_MENU_ACTION_AREA
	k11_MenuSpellAreLinesIndice = 11, // @ C011_GRAPHIC_MENU_SPELL_AREA_LINES
	k13_MovementArrowsIndice = 13, // @ C013_GRAPHIC_MOVEMENT_ARROWS
	k14_damageToCreatureIndice = 14, // @ C014_GRAPHIC_DAMAGE_TO_CREATURE
	k15_damageToChampionSmallIndice = 15, // @ C015_GRAPHIC_DAMAGE_TO_CHAMPION_SMALL
	k16_damageToChampionBig = 16, // @ C016_GRAPHIC_DAMAGE_TO_CHAMPION_BIG
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
	k41_holeInWall_GraphicIndice = 41, // @ C041_GRAPHIC_HOLE_IN_WALL
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
	k57_FloorPir_Invisible_D2L_GraphicIndice = 57, // @ C057_GRAPHIC_FLOOR_PIT_INVISIBLE_D2L
	k58_FloorPit_invisible_D2C_GraphicIndice = 58, // @ C058_GRAPHIC_FLOOR_PIT_INVISIBLE_D2C
	k59_floorPit_invisible_D1L_GraphicIndice = 59, // @ C059_GRAPHIC_FLOOR_PIT_INVISIBLE_D1L
	k60_floorPitInvisibleD1C_GraphicIndice = 60, // @ C060_GRAPHIC_FLOOR_PIT_INVISIBLE_D1C
	k61_floorPitInvisibleD0L_GraphicIndice = 61, // @ C061_GRAPHIC_FLOOR_PIT_INVISIBLE_D0L
	k62_flootPitInvisibleD0C_graphicIndice = 62, // @ C062_GRAPHIC_FLOOR_PIT_INVISIBLE_D0C
	k63_ceilingPit_D2L_GraphicIndice = 63, // @ C063_GRAPHIC_CEILING_PIT_D2L
	k64_ceilingPitD2C_GraphicIndice = 64, // @ C064_GRAPHIC_CEILING_PIT_D2C
	k65_ceilingPitD1L_GraphicIndice = 65, // @ C065_GRAPHIC_CEILING_PIT_D1L
	k66_ceilingPitD1C_GraphicIndice = 66, // @ C066_GRAPHIC_CEILING_PIT_D1C
	k67_ceilingPitD0L_grahicIndice = 67, // @ C067_GRAPHIC_CEILING_PIT_D0L
	k68_ceilingPitD0C_graphicIndice = 68, // @ C068_GRAPHIC_CEILING_PIT_D0C
	k69_FieldMask_D3R_GraphicIndice = 69, // @ C069_GRAPHIC_FIELD_MASK_D3R
	k73_FieldTeleporterGraphicIndice = 73, // @ C073_GRAPHIC_FIELD_TELEPORTER
	k120_InscriptionFontIndice = 120, // @ C120_GRAPHIC_INSCRIPTION_FONT
	k208_wallOrn_43_champMirror = 208, // @ C208_GRAPHIC_WALL_ORNAMENT_43_CHAMPION_MIRROR
	k241_FloorOrn_15_D3L_footprints = 241, // @ C241_GRAPHIC_FLOOR_ORNAMENT_15_D3L_FOOTPRINTS
	k301_DoorMaskDestroyedIndice = 301, // @ C301_GRAPHIC_DOOR_MASK_DESTROYED
	k315_firstDoorButton_GraphicIndice = 315, // @ C315_GRAPHIC_FIRST_DOOR_BUTTON
	k316_FirstProjectileGraphicIndice = 316, // @ C316_GRAPHIC_FIRST_PROJECTILE 
	k348_FirstExplosionGraphicIndice = 348, // @ C348_GRAPHIC_FIRST_EXPLOSION
	k351_FirstExplosionPatternGraphicIndice = 351, // @ C351_GRAPHIC_FIRST_EXPLOSION_PATTERN 
	k360_FirstObjectGraphicIndice = 360, // @ C360_GRAPHIC_FIRST_OBJECT
	k446_FirstCreatureGraphicIndice = 446, // @ C446_GRAPHIC_FIRST_CREATURE
	k557_FontGraphicIndice = 557 // @ C557_GRAPHIC_FONT 
};

extern byte g17_PalChangesNoChanges[16];
extern uint16 gK57_PalSwoosh[16]; // @ K0057_aui_Palette_Swoosh
extern uint16 gK150_PalMousePointer[16];	// @ K0150_aui_Palette_MousePointer
extern uint16 g19_PalCredits[16];  // @ G0019_aui_Graphic562_Palette_Credits
extern uint16 g20_PalEntrance[16]; // @ G0020_aui_Graphic562_Palette_Entrance
extern uint16 g21_PalDungeonView[6][16]; // @ G0021_aaui_Graphic562_Palette_DungeonView


// in all cases, where a function takes a Box, it expects it to contain inclusive boundaries
class Box {
public:
	int16 _x1;
	int16 _x2;
	int16 _y1;
	int16 _y2;

	Box(int16 x1, int16 x2, int16 y1, int16 y2) : _x1(x1), _x2(x2), _y1(y1), _y2(y2) {}
	Box() {}
	template <typename T>
	explicit Box(T *ptr) {
		_x1 = *ptr++;
		_x2 = *ptr++;
		_y1 = *ptr++;
		_y2 = *ptr++;
	}
	bool isPointInside(Common::Point point) {
		return (_x1 <= point.x) && (point.x <= _x2) && (_y1 <= point.y) && (point.y <= _y2); // <= because incluseive boundaries
	}
	bool isPointInside(int16 x, int16 y) { return isPointInside(Common::Point(x, y)); }
	void setToZero() { _x1 = _x2 = _y1 = _y2 = 0; }
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
	kM1_ColorNoTransparency = -1,
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
	uint16 _byteWidth;
	uint16 _height;
	uint16 _xPos;
	uint16 _bitplaneWordCount;
	FieldAspect(uint16 native, uint16 base, uint16 transparent, byte mask, uint16 byteWidth, uint16 height, uint16 xPos, uint16 bitplane)
		: _nativeBitmapRelativeIndex(native), _baseStartUnitIndex(base), _transparentColor(transparent), _mask(mask),
		_byteWidth(byteWidth), _height(height), _xPos(xPos), _bitplaneWordCount(bitplane) {}
	FieldAspect() {}
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
		: _firstNativeBitmapRelativeIndex(uint161),
		_firstDerivedBitmapIndex(uint162),
		_byteWidthFront(byte0),
		_heightFront(byte1),
		_byteWidthSide(byte2),
		_heightSide(byte3),
		_byteWidthAttack(byte4),
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
	byte _byteWidth;
	byte _height;
	byte _graphicInfo; /* Bits 7-5 and 3-1 Unreferenced */
	byte _coordinateSet;
	ObjectAspect(byte firstN, byte firstD, byte byteWidth, byte h, byte grap, byte coord) :
		_firstNativeBitmapRelativeIndex(firstN), _firstDerivedBitmapRelativeIndex(firstD),
		_byteWidth(byteWidth), _height(h), _graphicInfo(grap), _coordinateSet(coord) {}
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
}; // @ PROJECTIL_ASPECT

class CreatureReplColorSet {
public:
	uint16 _RGBColor[6];
	byte _D2ReplacementColor;
	byte _D3ReplacementColor;
}; // @ CREATURE_REPLACEMENT_COLOR_SET


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
			   Frame f4_1, Frame f4_2, Frame f4_3) {
		_closedOrDestroyed = f1;
		_vertical[0] = f2_1;
		_vertical[1] = f2_2;
		_vertical[2] = f2_3;
		_leftHorizontal[0] = f3_1;
		_leftHorizontal[1] = f3_2;
		_leftHorizontal[2] = f3_3;
		_rightHorizontal[0] = f4_1;
		_rightHorizontal[1] = f4_2;
		_rightHorizontal[2] = f4_3;
	}
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
	uint32 *_packedItemPos;
	byte *_packedBitmaps;
	byte **_bitmaps;
	DoorFrames *g186_doorFrame_D1C;
	// pointers are not owned by these fields
	byte *_g75_palChangesProjectile[4]; // @G0075_apuc_PaletteChanges_Projectile

	DisplayMan(const DisplayMan &other); // no implementation on purpose
	void operator=(const DisplayMan &rhs); // no implementation on purpose

	byte *getCurrentVgaBuffer();
	// the original function has two position parameters, but they are always set to zero
	void unpackGraphics();
	void loadFNT1intoBitmap(uint16 index, byte *destBitmap);

	void f565_viewportSetPalette(uint16 * middleScreenPalette, uint16 * topAndBottomScreen); // @ F0565_VIEWPORT_SetPalette
	void f566_viewportBlitToScreen(); // @ F0566_VIEWPORT_BlitToScreen

	void f105_drawFloorPitOrStairsBitmapFlippedHorizontally(uint16 nativeIndex, Frame &frame); // @ F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally
	void f104_drawFloorPitOrStairsBitmap(uint16 nativeIndex, Frame &frame); // @ F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap
	void f100_drawWallSetBitmap(byte *bitmap, Frame &f); // @ F0100_DUNGEONVIEW_DrawWallSetBitmap
	void f101_drawWallSetBitmapWithoutTransparency(byte *bitmap, Frame &f); // @ F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency
	void f116_drawSquareD3L(Direction dir, int16 posX, int16 posY); // @ F0116_DUNGEONVIEW_DrawSquareD3L
	void f117_drawSquareD3R(Direction dir, int16 posX, int16 posY); // @ F0117_DUNGEONVIEW_DrawSquareD3R
	void f118_drawSquareD3C(Direction dir, int16 posX, int16 posY); // @ F0118_DUNGEONVIEW_DrawSquareD3C_CPSF
	void f119_drawSquareD2L(Direction dir, int16 posX, int16 posY); // @ F0119_DUNGEONVIEW_DrawSquareD2L
	void f120_drawSquareD2R(Direction dir, int16 posX, int16 posY); // @ F0120_DUNGEONVIEW_DrawSquareD2R_CPSF
	void f121_drawSquareD2C(Direction dir, int16 posX, int16 posY); // @ F0121_DUNGEONVIEW_DrawSquareD2C
	void f122_drawSquareD1L(Direction dir, int16 posX, int16 posY); // @ F0122_DUNGEONVIEW_DrawSquareD1L
	void f123_drawSquareD1R(Direction dir, int16 posX, int16 posY); // @ F0123_DUNGEONVIEW_DrawSquareD1R
	void f124_drawSquareD1C(Direction dir, int16 posX, int16 posY); // @ F0124_DUNGEONVIEW_DrawSquareD1C
	void f125_drawSquareD0L(Direction dir, int16 posX, int16 posY); // @ F0125_DUNGEONVIEW_DrawSquareD0L
	void f126_drawSquareD0R(Direction dir, int16 posX, int16 posY); // @ F0126_DUNGEONVIEW_DrawSquareD0R
	void f127_drawSquareD0C(Direction dir, int16 posX, int16 posY); // @ F0127_DUNGEONVIEW_DrawSquareD0C


	void f93_applyCreatureReplColors(int replacedColor, int replacementColor); // @ F0093_DUNGEONVIEW_ApplyCreatureReplacementColors

	bool f107_isDrawnWallOrnAnAlcove(int16 wallOrnOrd, ViewWall viewWallIndex); // @ F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF

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


	byte *_g84_bitmapFloor; // @ G0084_puc_Bitmap_Floor
	byte *_g85_bitmapCeiling; // @ G0085_puc_Bitmap_Ceiling
	byte *_g697_bitmapWallSet_Wall_D3L2; // @ G0697_puc_Bitmap_WallSet_Wall_D3L2
	byte *_g696_bitmapWallSet_Wall_D3R2; // @ G0696_puc_Bitmap_WallSet_Wall_D3R2
	byte *_g698_bitmapWallSet_Wall_D3LCR; // @ G0698_puc_Bitmap_WallSet_Wall_D3LCR
	byte *_g699_bitmapWallSet_Wall_D2LCR; // @ G0699_puc_Bitmap_WallSet_Wall_D2LCR
public:
	byte *_g700_bitmapWallSet_Wall_D1LCR; // @ G0700_puc_Bitmap_WallSet_Wall_D1LCR
private:
	byte *_g701_bitmapWallSet_Wall_D0L; // @ G0701_puc_Bitmap_WallSet_Wall_D0L
	byte *_g702_bitmapWallSet_Wall_D0R; // @ G0702_puc_Bitmap_WallSet_Wall_D0R
	byte *_g703_bitmapWallSet_DoorFrameTop_D2LCR; // @ G0703_puc_Bitmap_WallSet_DoorFrameTop_D2LCR
	byte *_g704_bitmapWallSet_DoorFrameTop_D1LCR; // @ G0704_puc_Bitmap_WallSet_DoorFrameTop_D1LCR
	byte *_g705_bitmapWallSet_DoorFrameLeft_D3L; // @ G0705_puc_Bitmap_WallSet_DoorFrameLeft_D3L
	byte *_g706_bitmapWallSet_DoorFrameLeft_D3C; // @ G0706_puc_Bitmap_WallSet_DoorFrameLeft_D3C
	byte *_g707_bitmapWallSet_DoorFrameLeft_D2C; // @ G0707_puc_Bitmap_WallSet_DoorFrameLeft_D2C
	byte *_g708_bitmapWallSet_DoorFrameLeft_D1C; // @ G0708_puc_Bitmap_WallSet_DoorFrameLeft_D1C
	byte *_g710_bitmapWallSet_DoorFrameRight_D1C; // @ G0710_puc_Bitmap_WallSet_DoorFrameRight_D1C
	byte *_g709_bitmapWallSet_DoorFrameFront; // @ G0709_puc_Bitmap_WallSet_DoorFrameFront

	byte *_g90_bitmapWall_D3LCR_Flipped; // @ G0090_puc_Bitmap_WallD3LCR_Flipped;
	byte *_g91_bitmapWall_D2LCR_Flipped; // @ G0091_puc_Bitmap_WallD2LCR_Flipped;
	byte *_g92_bitmapWall_D1LCR_Flipped; // @ G0092_puc_Bitmap_WallD1LCR_Flipped;
	byte *_g93_bitmapWall_D0L_Flipped; // @ G0093_puc_Bitmap_WallD0L_Flipped;
	byte *_g94_bitmapWall_D0R_Flipped; // @ G0094_puc_Bitmap_WallD0R_Flipped;
	byte *_g95_bitmapWall_D3LCR_Native; // @ G0095_puc_Bitmap_WallD3LCR_Native;
	byte *_g96_bitmapWall_D2LCR_Native; // @ G0096_puc_Bitmap_WallD2LCR_Native;
	byte *_g97_bitmapWall_D1LCR_Native; // @ G0097_puc_Bitmap_WallD1LCR_Native;
	byte *_g98_bitmapWall_D0L_Native; // @ G0098_puc_Bitmap_WallD0L_Native;
	byte *_g99_bitmapWall_D0R_Native; // @ G0099_puc_Bitmap_WallD0R_Native;

	int16 _g231_currentWallSet; // @ G0231_i_CurrentWallSet
	int16 _g230_currentFloorSet;// @ G0230_i_CurrentFloorSet

	bool _g76_useFlippedWallAndFootprintsBitmap; // @ G0076_B_UseFlippedWallAndFootprintsBitmaps

	int16 _g693_doorNativeBitmapIndex_Front_D3LCR[2]; // @ G0693_ai_DoorNativeBitmapIndex_Front_D3LCR
	int16 _g694_doorNativeBitmapIndex_Front_D2LCR[2]; // @ G0694_ai_DoorNativeBitmapIndex_Front_D2LCR
	int16 _g695_doorNativeBitmapIndex_Front_D1LCR[2]; // @ G0695_ai_DoorNativeBitmapIndex_Front_D1LCR
public:

	uint16 _screenWidth;
	uint16 _screenHeight;
	byte *_g348_bitmapScreen; // @ G0348_pl_Bitmap_Screen
	byte* _g296_bitmapViewport; // @ G0296_puc_Bitmap_Viewport

	// some methods use this for a stratchpad, don't make assumptions about content between function calls
	byte *_g74_tmpBitmap; // @ G0074_puc_Bitmap_Temporary
	bool _g322_paletteSwitchingEnabled; // @ G0322_B_PaletteSwitchingEnabled
	bool _g342_refreshDungeonViewPaleteRequested; // @ G0342_B_RefreshDungeonViewPaletteRequested
	int16 _g304_dungeonViewPaletteIndex; // @ G0304_i_DungeonViewPaletteIndex
	uint16 _g345_aui_BlankBuffer[32]; // @G0345_aui_BlankBuffer
	uint16 _g347_paletteTopAndBottomScreen[16]; // @ G0347_aui_Palette_TopAndBottomScreen
	uint16 _g346_paletteMiddleScreen[16]; // @ G0346_aui_Palette_MiddleScreen

	explicit DisplayMan(DMEngine *dmEngine);
	~DisplayMan();

	void f95_loadWallSet(WallSet set); // @ F0095_DUNGEONVIEW_LoadWallSet
	void f94_loadFloorSet(FloorSet set); // @ F0094_DUNGEONVIEW_LoadFloorSet

	void f466_loadIntoBitmap(uint16 index, byte *destBitmap); // @ F0466_EXPAND_GraphicToBitmap
	void setUpScreens(uint16 width, uint16 height);
	void f479_loadGraphics(); // @ F0479_MEMORY_ReadGraphicsDatHeader	
	void f460_initializeGraphicData(); // @ F0460_START_InitializeGraphicData
	void f96_loadCurrentMapGraphics(); // @ F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF
	void loadPalette(uint16 *palette);
	void f461_allocateFlippedWallBitmaps(); // @ F0461_START_AllocateFlippedWallBitmaps
	void f102_drawDoorBitmap(Frame *frame);// @ F0102_DUNGEONVIEW_DrawDoorBitmap
	void f103_drawDoorFrameBitmapFlippedHorizontally(byte *bitmap, Frame *frame); // @ F0103_DUNGEONVIEW_DrawDoorFrameBitmapFlippedHorizontally
	void f110_drawDoorButton(int16 doorButtonOrdinal, int16 viewDoorButtonIndex); // @ F0110_DUNGEONVIEW_DrawDoorButton

	/// Gives the width of an IMG0 type item
	uint16 getPixelWidth(uint16 index);
	/// Gives the height of an IMG1 type item
	uint16 getPixelHeight(uint16 index);

	void f99_copyBitmapAndFlipHorizontal(byte *srcBitmap, byte *destBitmap, uint16 byteWidth, uint16 height); // @ F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal
	void f108_drawFloorOrnament(uint16 floorOrnOrdinal, uint16 viewFloorIndex); // @ F0108_DUNGEONVIEW_DrawFloorOrnament
	void f111_drawDoor(uint16 doorThingIndex, uint16 doorState, int16 *doorNativeBitmapIndices, int16 byteCount,
					   int16 viewDoorOrnIndex, DoorFrames *doorFrames); // @ F0111_DUNGEONVIEW_DrawDoor
	void f109_drawDoorOrnament(int16 doorOrnOdinal, int16 viewDoorOrnIndex); // @ F0109_DUNGEONVIEW_DrawDoorOrnament
	void f112_drawCeilingPit(int16 nativeBitmapIndex, Frame *frame, int16 mapX, int16 mapY, bool flipHorizontal); // @ F0112_DUNGEONVIEW_DrawCeilingPit

	void f20_blitToViewport(byte *bitmap, Box &box, int16 byteWidth, Color transparent, int16 height); // @ F0020_MAIN_BlitToViewport
	void f20_blitToViewport(byte *bitmap, int16 *box, int16 byteWidth, Color transparent, int16 height); // @ F0020_MAIN_BlitToViewport
	void f21_blitToScreen(byte *bitmap, int16* box, int16 byteWidth, Color transparent, int16 height); // @ F0021_MAIN_BlitToScreen
	void f21_blitToScreen(byte* bitmap, Box* box, int16 byteWidth, Color transparent, int16 height); // @ F0021_MAIN_BlitToScreen


	/* srcHeight and destHeight are not necessary for blitting, only error checking, thus they are defaulted for existing code which
	does not pass anything, newly imported calls do pass srcHeght and srcWidth, so this is a ceonvenience change so the the parameters
	match the original exatcly, if need arises for heights then we'll have to retrospectively add them in old function calls*/
	/* Expects inclusive boundaries in box */
	void f132_blitToBitmap(byte *srcBitmap, byte *destBitmap, Box &box, uint16 srcX, uint16 srcY, uint16 srcByteWidth,
						   uint16 destByteWidth, Color transparent = kM1_ColorNoTransparency, int16 srcHeight = -1, int16 destHight = -1); // @ F0132_VIDEO_Blit
	 /* Expects inclusive boundaries in box */
	void f133_blitBoxFilledWithMaskedBitmap(byte *src, byte *dest, byte *mask, byte *tmp, Box &box, int16 lastUnitIndex,
											int16 firstUnitIndex, int16 destByteWidth, Color transparent,
											int16 xPos, int16 yPos, int16 destHeight, int16 height2); // @ F0133_VIDEO_BlitBoxFilledWithMaskedBitmap
		 // this function takes pixel widths
	void f129_blitToBitmapShrinkWithPalChange(byte *srcBitmap, byte *destBitmap,
											  int16 srcPixelWidth, int16 srcHight, int16 destPixelWidth, int16 destHeight, byte *palChange); // @ F0129_VIDEO_BlitShrinkWithPaletteChanges
	void f130_flipBitmapHorizontal(byte *bitmap, uint16 byteWidth, uint16 height); // @ F0130_VIDEO_FlipHorizontal
	void f131_flipVertical(byte *bitmap, uint16 byteWidth, uint16 height);
	byte *f114_getExplosionBitmap(uint16 explosionAspIndex, uint16 scale, int16 &returnByteWidth, int16 &returnHeight); // @ F0114_DUNGEONVIEW_GetExplosionBitmap

	void f134_fillBitmap(byte *bitmap, Color color, uint16 byteWidth, uint16 height); // @ F0134_VIDEO_FillBitmap
	void fillScreen(Color color);
	/* Expects inclusive boundaries in box */
	void D24_fillScreenBox(Box &box, Color color); // @ D24_FillScreenBox, F0550_VIDEO_FillScreenBox
/* Expects inclusive boundaries in box */
	void f135_fillBoxBitmap(byte *destBitmap, Box &box, Color color, int16 byteWidth, int16 height); // @ F0135_VIDEO_FillBox
	void f128_drawDungeon(Direction dir, int16 posX, int16 posY); // @ F0128_DUNGEONVIEW_Draw_CPSF
	void f98_drawFloorAndCeiling(); // @ F0098_DUNGEONVIEW_DrawFloorAndCeiling
	void updateScreen();
	void f97_drawViewport(int16 palSwitchingRequestedState); // @ F0097_DUNGEONVIEW_DrawViewport

	byte* f489_getNativeBitmapOrGraphic(uint16 index); // @ F0489_MEMORY_GetNativeBitmapOrGraphic
	Common::MemoryReadStream getCompressedData(uint16 index);
	uint32 getCompressedDataSize(uint16 index);
	void f113_drawField(FieldAspect *fieldAspect, Box &box); // @ F0113_DUNGEONVIEW_DrawField

	int16 f459_getScaledBitmapByteCount(int16 byteWidth, int16 height, int16 scale); // @ F0459_START_GetScaledBitmapByteCount
	int16 M78_getScaledDimension(int16 dimension, int16 scale); // @ M78_SCALED_DIMENSION
	void f115_cthulhu(Thing thingParam, Direction directionParam,
					  int16 mapXpos, int16 mapYpos, int16 viewSquareIndex,
					  uint16 orderedViewCellOrdinals); // @ F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF
	uint16 M77_getNormalizedByteWidth(uint16 byteWidth); // @ M77_NORMALIZED_BYTE_WIDTH
	uint16 M23_getVerticalOffsetM23(uint16 val); // @ M23_VERTICAL_OFFSET
	uint16 M22_getHorizontalOffsetM22(uint16 val); // @ M22_HORIZONTAL_OFFSET

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

	bool _g297_drawFloorAndCeilingRequested; // @ G0297_B_DrawFloorAndCeilingRequested

	// This tells blitting functions whether to assume a BYTE_BOX or a WORD_BOX has been passed to them,
	// I only use WORD_BOX, so this will probably deem useless
	bool _g578_useByteBoxCoordinates; // @ G0578_B_UseByteBoxCoordinates
	bool _g77_doNotDrawFluxcagesDuringEndgame; // @ G0077_B_DoNotDrawFluxcagesDuringEndgame

	bool f491_isDerivedBitmapInCache(int16 derivedBitmapIndex); // @  F0491_CACHE_IsDerivedBitmapInCache
	byte *f492_getDerivedBitmap(int16 derivedBitmapIndex); // @ F0492_CACHE_GetDerivedBitmap
	void f493_addDerivedBitmap(int16 derivedBitmapIndex); // @ F0493_CACHE_AddDerivedBitmap
	void f480_releaseBlock(uint16 index); // @ F0480_CACHE_ReleaseBlock
	uint16 f431_getDarkenedColor(uint16 RGBcolor);
	void f436_STARTEND_FadeToPalette(uint16 *P0849_pui_Palette) { warning(false, "STUB METHOD: f436_STARTEND_FadeToPalette"); }
	void f508_buildPaletteChangeCopperList(uint16* middleScreen, uint16* topAndBottom);
	void f136_shadeScreenBox(Box* box, Color color) { warning(false, "STUB METHOD: f136_shadeScreenBox"); } // @ F0136_VIDEO_ShadeScreenBox
};

}

#endif
