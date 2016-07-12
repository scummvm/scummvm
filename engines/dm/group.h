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
	class TimelineEvent;
	class CreatureInfo;

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
	uint16 _type;
	uint16 _cells;
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

	byte &getActiveGroupIndex() { return *(byte*)&_cells; }

	uint16 getBehaviour() { return _flags & 0xF; }
	uint16 setBehaviour(uint16 val) { _flags = (_flags & ~0xF) | (val & 0xF); return (val & 0xF); }
	uint16 getCount() { return (_flags >> 5) & 0x3; }
	void setCount(uint16 val) { _flags = (_flags & ~(0x3 << 5)) | ((val & 0x3) << 5); }
	direction getDir() { return (direction)((_flags >> 8) & 0x3); }
	void setDir(uint16 val) { _flags = (_flags & ~(0x3 << 8)) | ((val & 0x3) << 8); }
	uint16 getDoNotDiscard() { return (_flags >> 10) & 0x1; }
	void setDoNotDiscard(bool val) { _flags = (_flags & ~(1 << 10)) | ((val & 1) << 10); }
}; // @ GROUP

#define k0_behavior_WANDER 0 // @ C0_BEHAVIOR_WANDER
#define k2_behavior_USELESS 2 // @ C2_BEHAVIOR_USELESS
#define k3_behavior_USELESS 3 // @ C3_BEHAVIOR_USELESS
#define k4_behavior_USELESS 4 // @ C4_BEHAVIOR_USELESS
#define k5_behavior_FLEE 5 // @ C5_BEHAVIOR_FLEE
#define k6_behavior_ATTACK 6 // @ C6_BEHAVIOR_ATTACK
#define k7_behavior_APPROACH 7 // @ C7_BEHAVIOR_APPROACH

#define k15_immuneToFear 15 // @ C15_IMMUNE_TO_FEAR

#define k255_immobile 255 // @ C255_IMMOBILE
#define kM1_wholeCreatureGroup -1 // @ CM1_WHOLE_CREATURE_GROUP 

#define k34_D13_soundCount 34 // @ D13_SOUND_COUNT

int32 M32_setTime(int32 &map_time, int32 time); // @ M32_SET_TIME


class GroupMan {
	DMEngine *_vm;
	byte _g392_dropMovingCreatureFixedPossessionsCell[4]; // @ G0392_auc_DropMovingCreatureFixedPossessionsCells
	uint16 _g391_dropMovingCreatureFixedPossCellCount; // @ G0391_ui_DropMovingCreatureFixedPossessionsCellCount
	uint16 _g386_fluxCageCount; // @ G0386_ui_FluxCageCount
	int16 _g385_fluxCages[4]; // @ G0385_ac_FluxCages
	int16 _g378_currentGroupMapX; // @ G0378_i_CurrentGroupMapX
	int16 _g379_currentGroupMapY; // @ G0379_i_CurrentGroupMapY
	Thing _g380_currGroupThing; // @ G0380_T_CurrentGroupThing
	int16 _g384_groupMovementTestedDirections[4]; // @ G0384_auc_GroupMovementTestedDirections
	uint16 _g381_currGroupDistanceToParty; // @ G0381_ui_CurrentGroupDistanceToParty
	int16 _g382_currGroupPrimaryDirToParty; // @ G0382_i_CurrentGroupPrimaryDirectionToParty
	int16 _g383_currGroupSecondaryDirToParty; // @ G0383_i_CurrentGroupSecondaryDirectionToParty

	Thing _g388_groupMovementBlockedByGroupThing; // @ G0388_T_GroupMovementBlockedByGroupThing
	bool _g389_groupMovementBlockedByDoor; // @ G0389_B_GroupMovementBlockedByDoor
	bool _g390_groupMovementBlockedByParty; // @ G0390_B_GroupMovementBlockedByParty
	bool _g387_groupMovBlockedByWallStairsPitFakeWalFluxCageTeleporter; // @ G0387_B_GroupMovementBlockedByWallStairsPitFakeWallFluxcageTeleporter
public:
	uint16 _g376_maxActiveGroupCount; // @ G0376_ui_MaximumActiveGroupCount
	ActiveGroup *_g375_activeGroups; // @ G0375_ps_ActiveGroups
	uint16 _g377_currActiveGroupCount; // @ G0377_ui_CurrentActiveGroupCount
	explicit GroupMan(DMEngine *vm);
	~GroupMan();

	void f196_initActiveGroups(); // @ F0196_GROUP_InitializeActiveGroups
	uint16 f145_getGroupCells(Group *group, int16 mapIndex); // @ F0145_DUNGEON_GetGroupCells
	uint16 f147_getGroupDirections(Group *group, int16 mapIndex); // @ F0147_DUNGEON_GetGroupDirections
	int16 f176_getCreatureOrdinalInCell(Group *group, uint16 cell); // @ F0176_GROUP_GetCreatureOrdinalInCell
	uint16 M50_getCreatureValue(uint16 groupVal, uint16 creatureIndex); // @ M50_CREATURE_VALUE
	void f188_dropGroupPossessions(int16 mapX, int16 mapY, Thing groupThing, int16 mode); // @ F0188_GROUP_DropGroupPossessions
	void f186_dropCreatureFixedPossessions(uint16 creatureType, int16 mapX, int16 mapY, uint16 cell,
										   int16 mode); // @ F0186_GROUP_DropCreatureFixedPossessions
	int16 f228_getDirsWhereDestIsVisibleFromSource(int16 srcMapX, int16 srcMapY,
												   int16 destMapX, int16 destMapY); // @ F0228_GROUP_GetDirectionsWhereDestinationIsVisibleFromSource
	bool f227_isDestVisibleFromSource(uint16 dir, int16 srcMapX, int16 srcMapY, int16 destMapX,
									  int16 destMapY); // @ F0227_GROUP_IsDestinationVisibleFromSource
	bool f232_groupIsDoorDestoryedByAttack(uint16 mapX, uint16 mapY, int16 attack,
										   bool magicAttack, int16 ticks); // @ F0232_GROUP_IsDoorDestroyedByAttack
	Thing f175_groupGetThing(int16 mapX, int16 mapY); // @ F0175_GROUP_GetThing
	int16 f190_groupGetDamageCreatureOutcome(Group *group, uint16 creatureIndex,
											 int16 mapX, int16 mapY, int16 damage, bool notMoving); // @ F0190_GROUP_GetDamageCreatureOutcome
	void f189_delete(int16 mapX, int16 mapY); // @ F0189_GROUP_Delete
	void f181_groupDeleteEvents(int16 mapX, int16 mapY); // @ F0181_GROUP_DeleteEvents
	uint16 f178_getGroupValueUpdatedWithCreatureValue(uint16 groupVal, uint16 creatureIndex, uint16 creatreVal); // @ F0178_GROUP_GetGroupValueUpdatedWithCreatureValue
	int16 f191_getDamageAllCreaturesOutcome(Group *group, int16 mapX, int16 mapY, int16 attack, bool notMoving); // @ F0191_GROUP_GetDamageAllCreaturesOutcome
	int16 f192_groupGetResistanceAdjustedPoisonAttack(uint16 creatreType, int16 poisonAttack); // @ F0192_GROUP_GetResistanceAdjustedPoisonAttack
	void f209_processEvents29to41(int16 eventMapX, int16 eventMapY, int16 eventType, uint16 ticks); // @ F0209_GROUP_ProcessEvents29to41
	bool f202_isMovementPossible(CreatureInfo *creatureInfo, int16 mapX, int16 mapY,
								 uint16 dir, bool allowMovementOverImaginaryPitsAndFakeWalls); // @ F0202_GROUP_IsMovementPossible
	int16 f226_getDistanceBetweenSquares(int16 srcMapX, int16 srcMapY, int16 destMapX,
										 int16 destMapY); // @ F0226_GROUP_GetDistanceBetweenSquares

	int16 f200_groupGetDistanceToVisibleParty(Group *group, int16 creatureIndex, int16 mapX, int16 mapY); // @ F0200_GROUP_GetDistanceToVisibleParty
	int16 f199_getDistanceBetweenUnblockedSquares(int16 srcMapX, int16 srcMapY,
												  int16 destMapX, int16 destMapY, bool (GroupMan::*isBlocked)(uint16, uint16)); // @ F0199_GROUP_GetDistanceBetweenUnblockedSquares
	bool f197_isViewPartyBlocked(uint16 mapX, uint16 mapY); // @ F0197_GROUP_IsViewPartyBlocked
	int32 f179_getCreatureAspectUpdateTime(ActiveGroup *activeGroup, int16 creatureIndex,
										   bool isAttacking); // @ F0179_GROUP_GetCreatureAspectUpdateTime
	void f205_setDirection(ActiveGroup *activeGroup, int16 dir, int16 creatureIndex, bool twoHalfSquareSizedCreatures); // @ F0205_GROUP_SetDirection
	void f208_groupAddEvent(TimelineEvent *event, uint32 time); // @ F0208_GROUP_AddEvent
	int16 f201_getSmelledPartyPrimaryDirOrdinal(CreatureInfo *creatureInfo, int16 mapY, int16 mapX); // @ F0201_GROUP_GetSmelledPartyPrimaryDirectionOrdinal
	bool f198_isSmellPartyBlocked(uint16 mapX, uint16 mapY); // @ F0198_GROUP_IsSmellPartyBlocked
	int16 f203_getFirstPossibleMovementDirOrdinal(CreatureInfo *info, int16 mapX, int16 mapY,
												  bool allowMovementOverImaginaryPitsAndFakeWalls); // @ F0203_GROUP_GetFirstPossibleMovementDirectionOrdinal
	void f206_groupSetDirGroup(ActiveGroup *activeGroup, int16 dir, int16 creatureIndex,
							   int16 creatureSize); // @ F0206_GROUP_SetDirectionGroup
	void f182_stopAttacking(ActiveGroup *group, int16 mapX, int16 mapY);// @ F0182_GROUP_StopAttacking
	bool f204_isArchenemyDoubleMovementPossible(CreatureInfo *info, int16 mapX, int16 mapY, uint16 dir); // @ F0204_GROUP_IsArchenemyDoubleMovementPossible
	bool f207_isCreatureAttacking(Group *group, int16 mapX, int16 mapY, uint16 creatureIndex); // @ F0207_GROUP_IsCreatureAttacking
	void f229_setOrderedCellsToAttack(signed char * orderedCellsToAttack, int16 targetMapX,
	                                  int16 targetMapY, int16 attackerMapX, int16 attackerMapY, uint16 cellSource); // @ F0229_GROUP_SetOrderedCellsToAttack
	void f193_stealFromChampion(Group *group, uint16 championIndex); // @ F0193_GROUP_StealFromChampion
	int16 f230_getChampionDamage(Group *group, uint16 champIndex); // @ F0230_GROUP_GetChampionDamage
	void f187_dropMovingCreatureFixedPossession(Thing thing, int16 mapX, int16 mapY); // @ F0187_GROUP_DropMovingCreatureFixedPossessions
	void f180_startWanedring(int16 mapX, int16 mapY); // @ F0180_GROUP_StartWandering
	void f183_addActiveGroup(Thing thing, int16 mapX, int16 mapY); // @ F0183_GROUP_AddActiveGroup
	void f184_removeActiveGroup(uint16 activeGroupIndex); // @ F0184_GROUP_RemoveActiveGroup
	void f194_removeAllActiveGroups(); // @ F0194_GROUP_RemoveAllActiveGroups
	void f195_addAllActiveGroups(); // @ F0195_GROUP_AddAllActiveGroups
	Thing f185_groupGetGenerated(int16 creatureType, int16 healthMultiplier, uint16 creatureCount, direction dir, int16 mapX, int16 mapY); // @ F0185_GROUP_GetGenerated
	bool f223_isSquareACorridorTeleporterPitOrDoor(int16 mapX, int16 mapY); // @ F0223_GROUP_IsSquareACorridorTeleporterPitOrDoor

};



}

#endif
