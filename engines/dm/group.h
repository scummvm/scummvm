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


#ifndef DM_GROUP_H
#define DM_GROUP_H

#include "dm.h"

namespace DM {
// this doesn't seem to be used anywhere at all
/* Creature types */
enum CreatureType {
	k0_CreatureTypeGiantScorpionScorpion = 0, // @ C00_CREATURE_GIANT_SCORPION_SCORPION     
	k1_CreatureTypeSwampSlimeSlime = 1, // @ C01_CREATURE_SWAMP_SLIME_SLIME_DEVIL     
	k2_CreatureTypeGiggler = 2, // @ C02_CREATURE_GIGGLER                     
	k3_CreatureTypeWizardEyeFlyingEye = 3, // @ C03_CREATURE_WIZARD_EYE_FLYING_EYE       
	k4_CreatureTypePainRatHellHound = 4, // @ C04_CREATURE_PAIN_RAT_HELLHOUND          
	k5_CreatureTypeRuster = 5, // @ C05_CREATURE_RUSTER                      
	k6_CreatureTypeScreamer = 6, // @ C06_CREATURE_SCREAMER                    
	k7_CreatureTypeRockpile = 7, // @ C07_CREATURE_ROCK_ROCKPILE               
	k8_CreatureTypeGhostRive = 8, // @ C08_CREATURE_GHOST_RIVE                  
	k9_CreatureTypeStoneGolem = 9, // @ C09_CREATURE_STONE_GOLEM                 
	k10_CreatureTypeMummy = 10, // @ C10_CREATURE_MUMMY                      
	k11_CreatureTypeBlackFlame = 11, // @ C11_CREATURE_BLACK_FLAME                
	k12_CreatureTypeSkeleton = 12, // @ C12_CREATURE_SKELETON                   
	k13_CreatureTypeCouatl = 13, // @ C13_CREATURE_COUATL                     
	k14_CreatureTypeVexirk = 14, // @ C14_CREATURE_VEXIRK                     
	k15_CreatureTypeMagnetaWormWorm = 15, // @ C15_CREATURE_MAGENTA_WORM_WORM          
	k16_CreatureTypeTrolinAntman = 16, // @ C16_CREATURE_TROLIN_ANTMAN              
	k17_CreatureTypeGiantWaspMuncher = 17, // @ C17_CREATURE_GIANT_WASP_MUNCHER         
	k18_CreatureTypeAnimatedArmourDethKnight = 18, // @ C18_CREATURE_ANIMATED_ARMOUR_DETH_KNIGHT
	k19_CreatureTypeMaterializerZytaz = 19, // @ C19_CREATURE_MATERIALIZER_ZYTAZ         
	k20_CreatureTypeWaterElemental = 20, // @ C20_CREATURE_WATER_ELEMENTAL            
	k21_CreatureTypeOitu = 21, // @ C21_CREATURE_OITU                       
	k22_CreatureTypeDemon = 22, // @ C22_CREATURE_DEMON                      
	k23_CreatureTypeLordChaos = 23, // @ C23_CREATURE_LORD_CHAOS                 
	k24_CreatureTypeRedDragon = 24, // @ C24_CREATURE_RED_DRAGON                 
	k25_CreatureTypeLordOrder = 25, // @ C25_CREATURE_LORD_ORDER                 
	k26_CreatureTypeGreyLord = 26, // @ C26_CREATURE_GREY_LORD                  
	k255_CreatureTypeSingleCenteredCreature = 255 // @ C255_SINGLE_CENTERED_CREATURE
};

#define k0_MaskCreatureSizeQuarter 0 // @ C0_SIZE_QUARTER_SQUARE
#define k1_MaskCreatureSizeHalf 1 // @ C1_SIZE_HALF_SQUARE   
#define k2_MaskCreatureSizeFull 2 // @ C2_SIZE_FULL_SQUARE   

#define k0x0003_MaskCreatureInfo_size 0x0003 // @ MASK0x0003_SIZE                     
#define k0x0004_MaskCreatureInfo_sideAttack 0x0004 // @ MASK0x0004_SIDE_ATTACK              
#define k0x0008_MaskCreatureInfo_preferBackRow 0x0008 // @ MASK0x0008_PREFER_BACK_ROW          
#define k0x0010_MaskCreatureInfo_attackAnyChamp 0x0010 // @ MASK0x0010_ATTACK_ANY_CHAMPION      
#define k0x0020_MaskCreatureInfo_levitation 0x0020 // @ MASK0x0020_LEVITATION               
#define k0x0040_MaskCreatureInfo_nonMaterial 0x0040 // @ MASK0x0040_NON_MATERIAL             
#define k0x0200_MaskCreatureInfo_dropFixedPoss 0x0200 // @ MASK0x0200_DROP_FIXED_POSSESSIONS   
#define k0x0400_MaskCreatureInfo_keepThrownSharpWeapon 0x0400 // @ MASK0x0400_KEEP_THROWN_SHARP_WEAPONS
#define k0x0800_MaskCreatureInfo_seeInvisible 0x0800 // @ MASK0x0800_SEE_INVISIBLE            
#define k0x1000_MaskCreatureInfo_nightVision 0x1000 // @ MASK0x1000_NIGHT_VISION             
#define k0x2000_MaskCreatureInfo_archenemy 0x2000 // @ MASK0x2000_ARCHENEMY                
#define k0x4000_MaskCreatureInfo_magicmap 0x4000 // @ MASK0x4000_MAGICMAP  


#define k0x0040_MaskActiveGroupFlipBitmap 0x0040 // @ MASK0x0040_FLIP_BITMAP  
#define k0x0080_MaskActiveGroupIsAttacking 0x0080 // @ MASK0x0080_IS_ATTACKING 

class ActiveGroup {
public:
	int _groupThingIndex;
	direction _directions;
	byte _cells;
	byte _lastMoveTime;
	byte _delayFleeingFromTarget;
	byte _targetMapX;
	byte _targetMapY;
	byte _priorMapX;
	byte _priorMapY;
	byte _homeMapX;
	byte _homeMapY;
	byte _aspect[4];
}; // @ ACTIVE_GROUP


class Group {
public:
	Thing _nextThing;
	Thing _slot;
	byte _type;
	byte _cells;
	uint16 _health[4];
private:
	uint16 _flags;
public:
	explicit Group(uint16 *rawDat) : _nextThing(rawDat[0]), _slot(rawDat[1]), _type(rawDat[2]),
		_cells(rawDat[3]), _flags(rawDat[8]) {
		_health[0] = rawDat[4];
		_health[1] = rawDat[5];
		_health[2] = rawDat[6];
		_health[3] = rawDat[7];
	}

	byte &getActiveGroupIndex() { return _cells; }

	uint16 getBehaviour() { return _flags & 0xF; }
	uint16 getCount() { return (_flags >> 5) & 0x3; }
	direction getDir() { return (direction)((_flags >> 8) & 0x3); }
	uint16 getDoNotDiscard() { return (_flags >> 10) & 0x1; }
}; // @ GROUP


class GroupMan {
	DMEngine *_vm;
public:
	uint16 _g376_maxActiveGroupCount = 60; // @ G0376_ui_MaximumActiveGroupCount
	ActiveGroup *_g375_activeGroups; // @ G0375_ps_ActiveGroups
	GroupMan(DMEngine *vm);
	~GroupMan();
	void initActiveGroups(); // @ F0196_GROUP_InitializeActiveGroups
	uint16 getGroupCells(Group *group, int16 mapIndex); // @ F0145_DUNGEON_GetGroupCells
	uint16 getGroupDirections(Group *group, int16 mapIndex); // @ F0147_DUNGEON_GetGroupDirections
	int16 getCreatureOrdinalInCell(Group *group, uint16 cell); // @ F0176_GROUP_GetCreatureOrdinalInCell
	uint16 getCreatureValue(uint16 groupVal, uint16 creatureIndex); // @ M50_CREATURE_VALUE
};



}

#endif
