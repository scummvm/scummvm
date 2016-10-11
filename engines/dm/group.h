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

#include "dm/dm.h"
#include "dm/sounds.h"
#include "dm/timeline.h"

namespace DM {
	class Champion;
	class TimelineEvent;
	class CreatureInfo;

/* Creature types */
enum CreatureType {
	kDMCreatureTypeGiantScorpion = 0, // @ C00_CREATURE_GIANT_SCORPION_SCORPION
	kDMCreatureTypeSwampSlime = 1, // @ C01_CREATURE_SWAMP_SLIME_SLIME_DEVIL
	kDMCreatureTypeGiggler = 2, // @ C02_CREATURE_GIGGLER
	kDMCreatureTypeWizardEye = 3, // @ C03_CREATURE_WIZARD_EYE_FLYING_EYE
	kDMCreatureTypePainRat = 4, // @ C04_CREATURE_PAIN_RAT_HELLHOUND
	kDMCreatureTypeRuster = 5, // @ C05_CREATURE_RUSTER
	kDMCreatureTypeScreamer = 6, // @ C06_CREATURE_SCREAMER
	kDMCreatureTypeRockpile = 7, // @ C07_CREATURE_ROCK_ROCKPILE
	kDMCreatureTypeGhostRive = 8, // @ C08_CREATURE_GHOST_RIVE
	kDMCreatureTypeStoneGolem = 9, // @ C09_CREATURE_STONE_GOLEM
	kDMCreatureTypeMummy = 10, // @ C10_CREATURE_MUMMY
	kDMCreatureTypeBlackFlame = 11, // @ C11_CREATURE_BLACK_FLAME
	kDMCreatureTypeSkeleton = 12, // @ C12_CREATURE_SKELETON
	kDMCreatureTypeCouatl = 13, // @ C13_CREATURE_COUATL
	kDMCreatureTypeVexirk = 14, // @ C14_CREATURE_VEXIRK
	kDMCreatureTypeMagentaWorm = 15, // @ C15_CREATURE_MAGENTA_WORM_WORM
	kDMCreatureTypeAntman = 16, // @ C16_CREATURE_TROLIN_ANTMAN
	kDMCreatureTypeGiantWasp = 17, // @ C17_CREATURE_GIANT_WASP_MUNCHER
	kDMCreatureTypeAnimatedArmour = 18, // @ C18_CREATURE_ANIMATED_ARMOUR_DETH_KNIGHT
	kDMCreatureTypeMaterializerZytaz = 19, // @ C19_CREATURE_MATERIALIZER_ZYTAZ
	kDMCreatureTypeWaterElemental = 20, // @ C20_CREATURE_WATER_ELEMENTAL
	kDMCreatureTypeOitu = 21, // @ C21_CREATURE_OITU
	kDMCreatureTypeDemon = 22, // @ C22_CREATURE_DEMON
	kDMCreatureTypeLordChaos = 23, // @ C23_CREATURE_LORD_CHAOS
	kDMCreatureTypeRedDragon = 24, // @ C24_CREATURE_RED_DRAGON
	kDMCreatureTypeLordOrder = 25, // @ C25_CREATURE_LORD_ORDER
	kDMCreatureTypeGreyLord = 26 // @ C26_CREATURE_GREY_LORD
};

enum CreatureSize {
	kDMCreatureSizeQuarter = 0, // @ C0_SIZE_QUARTER_SQUARE
	kDMCreatureSizeHalf = 1,    // @ C1_SIZE_HALF_SQUARE
	kDMCreatureSizeFull = 2     // @ C2_SIZE_FULL_SQUARE
};

enum Behavior {
	kDMBehaviorWander = 0,   // @ C0_BEHAVIOR_WANDER
	kDMBehaviorUnknown2 = 2, // @ C2_BEHAVIOR_USELESS
	kDMBehaviorUnknown3 = 3, // @ C3_BEHAVIOR_USELESS
	kDMBehaviorUnknown4 = 4, // @ C4_BEHAVIOR_USELESS
	kDMBehaviorFlee = 5,     // @ C5_BEHAVIOR_FLEE
	kDMBehaviorAttack = 6,   // @ C6_BEHAVIOR_ATTACK
	kDMBehaviorApproach = 7  // @ C7_BEHAVIOR_APPROACH
};

#define kDMImmuneToFear 15 // @ C15_IMMUNE_TO_FEAR
#define kDMMovementTicksImmobile 255 // @ C255_IMMOBILE
#define kDMWholeCreatureGroup -1 // @ CM1_WHOLE_CREATURE_GROUP
#define kDMCreatureTypeSingleCenteredCreature 255 // @ C255_SINGLE_CENTERED_CREATURE

enum CreatureMask {
	kDMCreatureMaskSize = 0x0003,           // @ MASK0x0003_SIZE
	kDMCreatureMaskSideAttack = 0x0004,     // @ MASK0x0004_SIDE_ATTACK
	kDMCreatureMaskPreferBackRow = 0x0008,  // @ MASK0x0008_PREFER_BACK_ROW
	kDMCreatureMaskAttackAnyChamp = 0x0010, // @ MASK0x0010_ATTACK_ANY_CHAMPION
	kDMCreatureMaskLevitation = 0x0020,     // @ MASK0x0020_LEVITATION
	kDMCreatureMaskNonMaterial = 0x0040,    // @ MASK0x0040_NON_MATERIAL
	kDMCreatureMaskDropFixedPoss = 0x0200,  // @ MASK0x0200_DROP_FIXED_POSSESSIONS
	kDMCreatureMaskKeepThrownSharpWeapon = 0x0400, // @ MASK0x0400_KEEP_THROWN_SHARP_WEAPONS
	kDMCreatureMaskSeeInvisible = 0x0800,   // @ MASK0x0800_SEE_INVISIBLE
	kDMCreatureMaskNightVision = 0x1000,    // @ MASK0x1000_NIGHT_VISION
	kDMCreatureMaskArchenemy = 0x2000,      // @ MASK0x2000_ARCHENEMY
	kDMCreatureMaskMagicMap = 0x4000        // @ MASK0x4000_MAGICMAP
};

enum aspectMask {
	kDMAspectMaskActiveGroupFlipBitmap = 0x0040, // @ MASK0x0040_FLIP_BITMAP
	kDMAspectMaskActiveGroupIsAttacking = 0x0080 // @ MASK0x0080_IS_ATTACKING
};

class ActiveGroup {
public:
	int16 _groupThingIndex;
	Direction _directions;
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
	CreatureType _type;
	uint16 _cells;
	uint16 _health[4];
	uint16 _flags;
public:
	explicit Group(uint16 *rawDat) : _nextThing(rawDat[0]), _slot(rawDat[1]), _cells(rawDat[3]), _flags(rawDat[8]) {
		_type = (CreatureType)rawDat[2];
		_health[0] = rawDat[4];
		_health[1] = rawDat[5];
		_health[2] = rawDat[6];
		_health[3] = rawDat[7];
	}

	uint16 &getActiveGroupIndex() { return _cells; }

	uint16 getBehaviour() { return _flags & 0xF; }
	uint16 setBehaviour(uint16 val) { _flags = (_flags & ~0xF) | (val & 0xF); return (val & 0xF); }
	uint16 getCount() { return (_flags >> 5) & 0x3; }
	void setCount(uint16 val) { _flags = (_flags & ~(0x3 << 5)) | ((val & 0x3) << 5); }
	Direction getDir() { return (Direction)((_flags >> 8) & 0x3); }
	void setDir(uint16 val) { _flags = (_flags & ~(0x3 << 8)) | ((val & 0x3) << 8); }
	uint16 getDoNotDiscard() { return (_flags >> 10) & 0x1; }
	void setDoNotDiscard(bool val) { _flags = (_flags & ~(1 << 10)) | ((val & 1) << 10); }
}; // @ GROUP

class GroupMan {
	DMEngine *_vm;
	byte _dropMovingCreatureFixedPossessionsCell[4]; // @ G0392_auc_DropMovingCreatureFixedPossessionsCells
	uint16 _dropMovingCreatureFixedPossCellCount; // @ G0391_ui_DropMovingCreatureFixedPossessionsCellCount
	uint16 _fluxCageCount; // @ G0386_ui_FluxCageCount
	int16 _fluxCages[4]; // @ G0385_ac_FluxCages
	int16 _currentGroupMapX; // @ G0378_i_CurrentGroupMapX
	int16 _currentGroupMapY; // @ G0379_i_CurrentGroupMapY
	Thing _currGroupThing; // @ G0380_T_CurrentGroupThing
	int16 _groupMovementTestedDirections[4]; // @ G0384_auc_GroupMovementTestedDirections
	uint16 _currGroupDistanceToParty; // @ G0381_ui_CurrentGroupDistanceToParty
	int16 _currGroupPrimaryDirToParty; // @ G0382_i_CurrentGroupPrimaryDirectionToParty
	int16 _currGroupSecondaryDirToParty; // @ G0383_i_CurrentGroupSecondaryDirectionToParty

	Thing _groupMovementBlockedByGroupThing; // @ G0388_T_GroupMovementBlockedByGroupThing
	bool _groupMovementBlockedByDoor; // @ G0389_B_GroupMovementBlockedByDoor
	bool _groupMovementBlockedByParty; // @ G0390_B_GroupMovementBlockedByParty
	bool _groupMovBlockedByWallStairsPitFakeWalFluxCageTeleporter; // @ G0387_B_GroupMovementBlockedByWallStairsPitFakeWallFluxcageTeleporter
	int32 twoHalfSquareSizedCreaturesGroupLastDirectionSetTime; // @ G0395_l_TwoHalfSquareSizedCreaturesGroupLastDirectionSetTime
	uint16 toggleFlag(uint16 &val, uint16 mask); // @ M10_TOGGLE
	int32 setTime(int32 &map_time, int32 time); // @ M32_SET_TIME

public:
	uint16 _maxActiveGroupCount; // @ G0376_ui_MaximumActiveGroupCount
	ActiveGroup *_activeGroups; // @ G0375_ps_ActiveGroups
	uint16 _currActiveGroupCount; // @ G0377_ui_CurrentActiveGroupCount
	explicit GroupMan(DMEngine *vm);
	~GroupMan();

	void initActiveGroups(); // @ F0196_GROUP_InitializeActiveGroups
	uint16 getGroupCells(Group *group, int16 mapIndex); // @ F0145_DUNGEON_GetGroupCells
	uint16 getGroupDirections(Group *group, int16 mapIndex); // @ F0147_DUNGEON_GetGroupDirections
	int16 getCreatureOrdinalInCell(Group *group, uint16 cell); // @ F0176_GROUP_GetCreatureOrdinalInCell
	uint16 getCreatureValue(uint16 groupVal, uint16 creatureIndex); // @ M50_CREATURE_VALUE
	void dropGroupPossessions(int16 mapX, int16 mapY, Thing groupThing, SoundMode mode); // @ F0188_GROUP_DropGroupPossessions
	void dropCreatureFixedPossessions(CreatureType creatureType, int16 mapX, int16 mapY, uint16 cell,
										   SoundMode soundMode); // @ F0186_GROUP_DropCreatureFixedPossessions
	int16 getDirsWhereDestIsVisibleFromSource(int16 srcMapX, int16 srcMapY,
												   int16 destMapX, int16 destMapY); // @ F0228_GROUP_GetDirectionsWhereDestinationIsVisibleFromSource
	bool isDestVisibleFromSource(uint16 dir, int16 srcMapX, int16 srcMapY, int16 destMapX,
									  int16 destMapY); // @ F0227_GROUP_IsDestinationVisibleFromSource
	bool groupIsDoorDestoryedByAttack(uint16 mapX, uint16 mapY, int16 attack,
										   bool magicAttack, int16 ticks); // @ F0232_GROUP_IsDoorDestroyedByAttack
	Thing groupGetThing(int16 mapX, int16 mapY); // @ F0175_GROUP_GetThing
	int16 groupGetDamageCreatureOutcome(Group *group, uint16 creatureIndex,
											 int16 mapX, int16 mapY, int16 damage, bool notMoving); // @ F0190_GROUP_GetDamageCreatureOutcome
	void groupDelete(int16 mapX, int16 mapY); // @ F0189_GROUP_Delete
	void groupDeleteEvents(int16 mapX, int16 mapY); // @ F0181_GROUP_DeleteEvents
	uint16 getGroupValueUpdatedWithCreatureValue(uint16 groupVal, uint16 creatureIndex, uint16 creatureVal); // @ F0178_GROUP_GetGroupValueUpdatedWithCreatureValue
	int16 getDamageAllCreaturesOutcome(Group *group, int16 mapX, int16 mapY, int16 attack, bool notMoving); // @ F0191_GROUP_GetDamageAllCreaturesOutcome
	int16 groupGetResistanceAdjustedPoisonAttack(CreatureType creatureType, int16 poisonAttack); // @ F0192_GROUP_GetResistanceAdjustedPoisonAttack
	void processEvents29to41(int16 eventMapX, int16 eventMapY, TimelineEventType eventType, uint16 ticks); // @ F0209_GROUP_ProcessEvents29to41
	bool isMovementPossible(CreatureInfo *creatureInfo, int16 mapX, int16 mapY,
								 uint16 dir, bool allowMovementOverImaginaryPitsAndFakeWalls); // @ F0202_GROUP_IsMovementPossible
	int16 getDistanceBetweenSquares(int16 srcMapX, int16 srcMapY, int16 destMapX,
										 int16 destMapY); // @ F0226_GROUP_GetDistanceBetweenSquares

	int16 groupGetDistanceToVisibleParty(Group *group, int16 creatureIndex, int16 mapX, int16 mapY); // @ F0200_GROUP_GetDistanceToVisibleParty
	int16 getDistanceBetweenUnblockedSquares(int16 srcMapX, int16 srcMapY,
												  int16 destMapX, int16 destMapY, bool (GroupMan::*isBlocked)(uint16, uint16)); // @ F0199_GROUP_GetDistanceBetweenUnblockedSquares
	bool isViewPartyBlocked(uint16 mapX, uint16 mapY); // @ F0197_GROUP_IsViewPartyBlocked
	int32 getCreatureAspectUpdateTime(ActiveGroup *activeGroup, int16 creatureIndex,
										   bool isAttacking); // @ F0179_GROUP_GetCreatureAspectUpdateTime
	void setGroupDirection(ActiveGroup *activeGroup, int16 dir, int16 creatureIndex, bool twoHalfSquareSizedCreatures); // @ F0205_GROUP_SetDirection
	void addGroupEvent(TimelineEvent *event, uint32 time); // @ F0208_GROUP_AddEvent
	int16 getSmelledPartyPrimaryDirOrdinal(CreatureInfo *creatureInfo, int16 mapY, int16 mapX); // @ F0201_GROUP_GetSmelledPartyPrimaryDirectionOrdinal
	bool isSmellPartyBlocked(uint16 mapX, uint16 mapY); // @ F0198_GROUP_IsSmellPartyBlocked
	int16 getFirstPossibleMovementDirOrdinal(CreatureInfo *info, int16 mapX, int16 mapY,
												  bool allowMovementOverImaginaryPitsAndFakeWalls); // @ F0203_GROUP_GetFirstPossibleMovementDirectionOrdinal
	void setDirGroup(ActiveGroup *activeGroup, int16 dir, int16 creatureIndex,
							   int16 creatureSize); // @ F0206_GROUP_SetDirectionGroup
	void stopAttacking(ActiveGroup *group, int16 mapX, int16 mapY);// @ F0182_GROUP_StopAttacking
	bool isArchenemyDoubleMovementPossible(CreatureInfo *info, int16 mapX, int16 mapY, uint16 dir); // @ F0204_GROUP_IsArchenemyDoubleMovementPossible
	bool isCreatureAttacking(Group *group, int16 mapX, int16 mapY, uint16 creatureIndex); // @ F0207_GROUP_IsCreatureAttacking
	void setOrderedCellsToAttack(signed char *orderedCellsToAttack, int16 targetMapX,
	                                  int16 targetMapY, int16 attackerMapX, int16 attackerMapY, uint16 cellSource); // @ F0229_GROUP_SetOrderedCellsToAttack
	void stealFromChampion(Group *group, uint16 championIndex); // @ F0193_GROUP_StealFromChampion
	int16 getChampionDamage(Group *group, uint16 champIndex); // @ F0230_GROUP_GetChampionDamage
	void dropMovingCreatureFixedPossession(Thing thing, int16 mapX, int16 mapY); // @ F0187_GROUP_DropMovingCreatureFixedPossessions
	void startWandering(int16 mapX, int16 mapY); // @ F0180_GROUP_StartWandering
	void addActiveGroup(Thing thing, int16 mapX, int16 mapY); // @ F0183_GROUP_AddActiveGroup
	void removeActiveGroup(uint16 activeGroupIndex); // @ F0184_GROUP_RemoveActiveGroup
	void removeAllActiveGroups(); // @ F0194_GROUP_RemoveAllActiveGroups
	void addAllActiveGroups(); // @ F0195_GROUP_AddAllActiveGroups
	Thing groupGetGenerated(CreatureType creatureType, int16 healthMultiplier, uint16 creatureCount, Direction dir, int16 mapX, int16 mapY); // @ F0185_GROUP_GetGenerated
	bool isSquareACorridorTeleporterPitOrDoor(int16 mapX, int16 mapY); // @ F0223_GROUP_IsSquareACorridorTeleporterPitOrDoor
	int16 getMeleeTargetCreatureOrdinal(int16 groupX, int16 groupY, int16 partyX, int16 paryY,
											 uint16 champCell); // @ F0177_GROUP_GetMeleeTargetCreatureOrdinal
	int16 getMeleeActionDamage(Champion *champ, int16 champIndex, Group *group, int16 creatureIndex,
									int16 mapX, int16 mapY, uint16 actionHitProbability, uint16 actionDamageFactor, int16 skillIndex); // @ F0231_GROUP_GetMeleeActionDamage
	void fluxCageAction(int16 mapX, int16 mapY); // @ F0224_GROUP_FluxCageAction
	uint16 isLordChaosOnSquare(int16 mapX, int16 mapY); // @ F0222_GROUP_IsLordChaosOnSquare
	bool isFluxcageOnSquare(int16 mapX, int16 mapY); // @ F0221_GROUP_IsFluxcageOnSquare
	void fuseAction(uint16 mapX, uint16 mapY); // @ F0225_GROUP_FuseAction
	void saveActiveGroupPart(Common::OutSaveFile *file);
	void loadActiveGroupPart(Common::InSaveFile *file);
};
}

#endif
