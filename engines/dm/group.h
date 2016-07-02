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
	kCreatureTypeGiantScorpionScorpion = 0, // @ C00_CREATURE_GIANT_SCORPION_SCORPION     
	kCreatureTypeSwampSlimeSlime = 1, // @ C01_CREATURE_SWAMP_SLIME_SLIME_DEVIL     
	kCreatureTypeGiggler = 2, // @ C02_CREATURE_GIGGLER                     
	kCreatureTypeWizardEyeFlyingEye = 3, // @ C03_CREATURE_WIZARD_EYE_FLYING_EYE       
	kCreatureTypePainRatHellHound = 4, // @ C04_CREATURE_PAIN_RAT_HELLHOUND          
	kCreatureTypeRuster = 5, // @ C05_CREATURE_RUSTER                      
	kCreatureTypeScreamer = 6, // @ C06_CREATURE_SCREAMER                    
	kCreatureTypeRockpile = 7, // @ C07_CREATURE_ROCK_ROCKPILE               
	kCreatureTypeGhostRive = 8, // @ C08_CREATURE_GHOST_RIVE                  
	kCreatureTypeStoneGolem = 9, // @ C09_CREATURE_STONE_GOLEM                 
	kCreatureTypeMummy = 10, // @ C10_CREATURE_MUMMY                      
	kCreatureTypeBlackFlame = 11, // @ C11_CREATURE_BLACK_FLAME                
	kCreatureTypeSkeleton = 12, // @ C12_CREATURE_SKELETON                   
	kCreatureTypeCouatl = 13, // @ C13_CREATURE_COUATL                     
	kCreatureTypeVexirk = 14, // @ C14_CREATURE_VEXIRK                     
	kCreatureTypeMagnetaWormWorm = 15, // @ C15_CREATURE_MAGENTA_WORM_WORM          
	kCreatureTypeTrolinAntman = 16, // @ C16_CREATURE_TROLIN_ANTMAN              
	kCreatureTypeGiantWaspMuncher = 17, // @ C17_CREATURE_GIANT_WASP_MUNCHER         
	kCreatureTypeAnimatedArmourDethKnight = 18, // @ C18_CREATURE_ANIMATED_ARMOUR_DETH_KNIGHT
	kCreatureTypeMaterializerZytaz = 19, // @ C19_CREATURE_MATERIALIZER_ZYTAZ         
	kCreatureTypeWaterElemental = 20, // @ C20_CREATURE_WATER_ELEMENTAL            
	kCreatureTypeOitu = 21, // @ C21_CREATURE_OITU                       
	kCreatureTypeDemon = 22, // @ C22_CREATURE_DEMON                      
	kCreatureTypeLordChaos = 23, // @ C23_CREATURE_LORD_CHAOS                 
	kCreatureTypeRedDragon = 24, // @ C24_CREATURE_RED_DRAGON                 
	kCreatureTypeLordOrder = 25, // @ C25_CREATURE_LORD_ORDER                 
	kCreatureTypeGreyLord = 26, // @ C26_CREATURE_GREY_LORD                  
	kCreatureTypeSingleCenteredCreature = 255 // @ C255_SINGLE_CENTERED_CREATURE
};

#define kMaskCreatureSizeQuarter 0 // @ C0_SIZE_QUARTER_SQUARE
#define kMaskCreatureSizeHalf 1 // @ C1_SIZE_HALF_SQUARE   
#define kMaskCreatureSizeFull 2 // @ C2_SIZE_FULL_SQUARE   

#define kMaskCreatureInfo_size 0x0003 // @ MASK0x0003_SIZE                     
#define kMaskCreatureInfo_sideAttack 0x0004 // @ MASK0x0004_SIDE_ATTACK              
#define kMaskCreatureInfo_preferBackRow 0x0008 // @ MASK0x0008_PREFER_BACK_ROW          
#define kMaskCreatureInfo_attackAnyChamp 0x0010 // @ MASK0x0010_ATTACK_ANY_CHAMPION      
#define kMaskCreatureInfo_levitation 0x0020 // @ MASK0x0020_LEVITATION               
#define kMaskCreatureInfo_nonMaterial 0x0040 // @ MASK0x0040_NON_MATERIAL             
#define kMaskCreatureInfo_dropFixedPoss 0x0200 // @ MASK0x0200_DROP_FIXED_POSSESSIONS   
#define kMaskCreatureInfo_keepThrownSharpWeapon 0x0400 // @ MASK0x0400_KEEP_THROWN_SHARP_WEAPONS
#define kMaskCreatureInfo_seeInvisible 0x0800 // @ MASK0x0800_SEE_INVISIBLE            
#define kMaskCreatureInfo_nightVision 0x1000 // @ MASK0x1000_NIGHT_VISION             
#define kMaskCreatureInfo_archenemy 0x2000 // @ MASK0x2000_ARCHENEMY                
#define kMaskCreatureInfo_magicmap 0x4000 // @ MASK0x4000_MAGICMAP  


#define kMaskActiveGroupFlipBitmap 0x0040 // @ MASK0x0040_FLIP_BITMAP  
#define kMaskActiveGroupIsAttacking 0x0080 // @ MASK0x0080_IS_ATTACKING 

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
	uint16 _maxActiveGroupCount = 60; // @ G0376_ui_MaximumActiveGroupCount
	ActiveGroup *_activeGroups; // @ G0375_ps_ActiveGroups
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
