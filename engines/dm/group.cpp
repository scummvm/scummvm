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

#include "group.h"
#include "dungeonman.h"
#include "champion.h"
#include "movesens.h"
#include "projexpl.h"
#include "timeline.h"
#include "objectman.h"
#include "menus.h"


namespace DM {
int32 M32_setTime(int32 &map_time, int32 time) {
	return map_time = (map_time & 0xFF000000) | time;
}

GroupMan::GroupMan(DMEngine *vm) : _vm(vm) {
	for (uint16 i = 0; i < 4; ++i)
		_g392_dropMovingCreatureFixedPossessionsCell[i] = 0;
	_g391_dropMovingCreatureFixedPossCellCount = 0;
	_g386_fluxCageCount = 0;
	for (uint16 i = 0; i < 4; ++i)
		_g385_fluxCages[i] = 0;
	_g378_currentGroupMapX = 0;
	_g379_currentGroupMapY = 0;
	_g380_currGroupThing = Thing(0);
	for (uint16 i = 0; i < 4; ++i)
		_g384_groupMovementTestedDirections[i] = 0;
	_g381_currGroupDistanceToParty = 0;
	_g382_currGroupPrimaryDirToParty = 0;
	_g383_currGroupSecondaryDirToParty = 0;
	_g388_groupMovementBlockedByGroupThing = Thing(0);
	_g389_groupMovementBlockedByDoor = false;
	_g390_groupMovementBlockedByParty = false;
	_g387_groupMovBlockedByWallStairsPitFakeWalFluxCageTeleporter = false;
	_g376_maxActiveGroupCount = 60;
	_g375_activeGroups = nullptr;
	_g377_currActiveGroupCount = 0;
}

GroupMan::~GroupMan() {
	delete[] _g375_activeGroups;
}

void GroupMan::f196_initActiveGroups() {
	if (_vm->_g298_newGame)
		_g376_maxActiveGroupCount = 60;
	if (_g375_activeGroups)
		delete[] _g375_activeGroups;
	_g375_activeGroups = new ActiveGroup[_g376_maxActiveGroupCount];
	for (uint16 i = 0; i < _g376_maxActiveGroupCount; ++i)
		_g375_activeGroups[i]._groupThingIndex = -1;
}

uint16 GroupMan::f145_getGroupCells(Group *group, int16 mapIndex) {
	byte cells;
	cells = group->_cells;
	if (mapIndex == _vm->_dungeonMan->_g309_partyMapIndex)
		cells = _g375_activeGroups[cells]._cells;
	return cells;
}

byte gGroupDirections[4] = {0x00, 0x55, 0xAA, 0xFF}; // @ G0258_auc_Graphic559_GroupDirections

uint16 GroupMan::f147_getGroupDirections(Group *group, int16 mapIndex) {
	if (mapIndex == _vm->_dungeonMan->_g309_partyMapIndex)
		return _g375_activeGroups[group->getActiveGroupIndex()]._directions;

	return gGroupDirections[group->getDir()];
}

int16 GroupMan::f176_getCreatureOrdinalInCell(Group *group, uint16 cell) {
	uint16 currMapIndex = _vm->_dungeonMan->_g272_currMapIndex;
	byte groupCells = f145_getGroupCells(group, currMapIndex);
	if (groupCells == k255_CreatureTypeSingleCenteredCreature)
		return _vm->M0_indexToOrdinal(0);

	byte creatureIndex = group->getCount();
	if (getFlag(g243_CreatureInfo[group->_type]._attributes, k0x0003_MaskCreatureInfo_size) == k1_MaskCreatureSizeHalf) {
		if ((f147_getGroupDirections(group, currMapIndex) & 1) == (cell & 1))
			cell = returnPrevVal(cell);

		do {
			byte creatureCell = M50_getCreatureValue(groupCells, creatureIndex);
			if (creatureCell == cell || creatureCell == returnNextVal(cell))
				return _vm->M0_indexToOrdinal(creatureIndex);
		} while (creatureIndex--);
	} else {
		do {
			if (M50_getCreatureValue(groupCells, creatureIndex) == cell)
				return _vm->M0_indexToOrdinal(creatureIndex);
		} while (creatureIndex--);
	}
	return 0;
}

uint16 GroupMan::M50_getCreatureValue(uint16 groupVal, uint16 creatureIndex) {
	return (groupVal >> (creatureIndex << 1)) & 0x3;
}

void GroupMan::f188_dropGroupPossessions(int16 mapX, int16 mapY, Thing groupThing, int16 mode) {
	Group *L0367_ps_Group = (Group *)_vm->_dungeonMan->f156_getThingData(groupThing);
	uint16 L0368_ui_CreatureType = L0367_ps_Group->_type;
	if ((mode >= k0_soundModePlayImmediately) && getFlag(g243_CreatureInfo[L0368_ui_CreatureType]._attributes, k0x0200_MaskCreatureInfo_dropFixedPoss)) {
		int16 L0369_i_CreatureIndex = L0367_ps_Group->getCount();
		uint16 L0370_ui_GroupCells = _vm->_groupMan->f145_getGroupCells(L0367_ps_Group, _vm->_dungeonMan->_g272_currMapIndex);
		do {
			_vm->_groupMan->f186_dropCreatureFixedPossessions(L0368_ui_CreatureType, mapX, mapY, (L0370_ui_GroupCells == k255_CreatureTypeSingleCenteredCreature) ? k255_CreatureTypeSingleCenteredCreature : _vm->_groupMan->M50_getCreatureValue(L0370_ui_GroupCells, L0369_i_CreatureIndex), mode);
		} while (L0369_i_CreatureIndex--);
	}
	Thing L0365_T_CurrentThing = L0367_ps_Group->_slot;
	if ((L0365_T_CurrentThing) != Thing::_endOfList) {
		bool L0371_B_WeaponDropped = false;
		Thing L0366_T_NextThing;
		do {
			L0366_T_NextThing = _vm->_dungeonMan->f159_getNextThing(L0365_T_CurrentThing);
			L0365_T_CurrentThing = M15_thingWithNewCell(L0365_T_CurrentThing, _vm->getRandomNumber(4));
			if ((L0365_T_CurrentThing).getType() == k5_WeaponThingType) {
				L0371_B_WeaponDropped = true;
			}
			_vm->_movsens->f267_getMoveResult(L0365_T_CurrentThing, kM1_MapXNotOnASquare, 0, mapX, mapY);
		} while ((L0365_T_CurrentThing = L0366_T_NextThing) != Thing::_endOfList);
		if (mode >= k0_soundModePlayImmediately) {
			_vm->f064_SOUND_RequestPlay_CPSD(L0371_B_WeaponDropped ? k00_soundMETALLIC_THUD : k04_soundWOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM, mapX, mapY, mode);
		}
	}
}

void GroupMan::f186_dropCreatureFixedPossessions(uint16 creatureType, int16 mapX, int16 mapY, uint16 cell, int16 mode) {
	static uint16 g245FixedPossessionCreature_12Skeleton[3] = { // @ G0245_aui_Graphic559_FixedPossessionsCreature12Skeleton
		k23_ObjectInfoIndexFirstWeapon + k9_WeaponTypeFalchion,
		k69_ObjectInfoIndexFirstArmour + k30_ArmourTypeWoodenShield, 0};
	static uint16 g246FixedPossessionCreature_9StoneGolem[2] = { // @ G0246_aui_Graphic559_FixedPossessionsCreature09StoneGolem
		k23_ObjectInfoIndexFirstWeapon + k24_WeaponTypeStoneClub, 0};
	static uint16 g247FixedPossessionCreature_16TrolinAntman[2] = { // @ G0247_aui_Graphic559_FixedPossessionsCreature16Trolin_Antman
		k23_ObjectInfoIndexFirstWeapon + k23_WeaponTypeClub, 0};
	static uint16 g248FixedPossessionCreature_18AnimatedArmourDethKnight[7] = { // @ G0248_aui_Graphic559_FixedPossessionsCreature18AnimatedArmour_DethKnight
		k69_ObjectInfoIndexFirstArmour + k41_ArmourTypeFootPlate,
		k69_ObjectInfoIndexFirstArmour + k40_ArmourTypeLegPlate,
		k69_ObjectInfoIndexFirstArmour + k39_ArmourTypeTorsoPlate,
		k23_ObjectInfoIndexFirstWeapon + k10_WeaponTypeSword,
		k69_ObjectInfoIndexFirstArmour + k38_ArmourTypeArmet,
		k23_ObjectInfoIndexFirstWeapon + k10_WeaponTypeSword, 0};
	static uint16 g249FixedPossessionCreature_7rockRockPile[5] = { // @ G0249_aui_Graphic559_FixedPossessionsCreature07Rock_RockPile
		k127_ObjectInfoIndexFirstJunk + k25_JunkTypeBoulder,
		k127_ObjectInfoIndexFirstJunk + k25_JunkTypeBoulder | k0x8000_randomDrop,
		k23_ObjectInfoIndexFirstWeapon + k30_WeaponTypeRock | k0x8000_randomDrop,
		k23_ObjectInfoIndexFirstWeapon + k30_WeaponTypeRock | k0x8000_randomDrop, 0};
	static uint16 g250FixedPossessionCreature_4PainRatHellHound[3] = { // @ G0250_aui_Graphic559_FixedPossessionsCreature04PainRat_Hellhound
		k127_ObjectInfoIndexFirstJunk + k35_JunkTypeDrumstickShank,
		k127_ObjectInfoIndexFirstJunk + k35_JunkTypeDrumstickShank | k0x8000_randomDrop, 0};
	static uint16 g251FixedPossessionCreature_6screamer[3] = { // @ G0251_aui_Graphic559_FixedPossessionsCreature06Screamer
		k127_ObjectInfoIndexFirstJunk + k33_JunkTypeScreamerSlice,
		k127_ObjectInfoIndexFirstJunk + k33_JunkTypeScreamerSlice | k0x8000_randomDrop, 0};
	static uint16 g252FixedPossessionCreature_15MagnetaWormWorm[4] = { // @ G0252_aui_Graphic559_FixedPossessionsCreature15MagentaWorm_Worm
		k127_ObjectInfoIndexFirstJunk + k34_JunkTypeWormRound,
		k127_ObjectInfoIndexFirstJunk + k34_JunkTypeWormRound | k0x8000_randomDrop,
		k127_ObjectInfoIndexFirstJunk + k34_JunkTypeWormRound | k0x8000_randomDrop, 0};
	static uint16 g253FixedPossessionCreature_24RedDragon[11] = { // @ G0253_aui_Graphic559_FixedPossessionsCreature24RedDragon
		k127_ObjectInfoIndexFirstJunk + k36_JunkTypeDragonSteak,
		k127_ObjectInfoIndexFirstJunk + k36_JunkTypeDragonSteak,
		k127_ObjectInfoIndexFirstJunk + k36_JunkTypeDragonSteak,
		k127_ObjectInfoIndexFirstJunk + k36_JunkTypeDragonSteak,
		k127_ObjectInfoIndexFirstJunk + k36_JunkTypeDragonSteak,
		k127_ObjectInfoIndexFirstJunk + k36_JunkTypeDragonSteak,
		k127_ObjectInfoIndexFirstJunk + k36_JunkTypeDragonSteak,
		k127_ObjectInfoIndexFirstJunk + k36_JunkTypeDragonSteak,
		k127_ObjectInfoIndexFirstJunk + k36_JunkTypeDragonSteak | k0x8000_randomDrop,
		k127_ObjectInfoIndexFirstJunk + k36_JunkTypeDragonSteak | k0x8000_randomDrop, 0};

	uint16 *L0359_pui_FixedPossessions;

	bool L0361_B_Cursed = false;
	switch (creatureType) {
	default:
		return;
	case k12_CreatureTypeSkeleton:
		L0359_pui_FixedPossessions = g245FixedPossessionCreature_12Skeleton;
		break;
	case k9_CreatureTypeStoneGolem:
		L0359_pui_FixedPossessions = g246FixedPossessionCreature_9StoneGolem;
		break;
	case k16_CreatureTypeTrolinAntman:
		L0359_pui_FixedPossessions = g247FixedPossessionCreature_16TrolinAntman;
		break;
	case k18_CreatureTypeAnimatedArmourDethKnight:
		L0361_B_Cursed = true;
		L0359_pui_FixedPossessions = g248FixedPossessionCreature_18AnimatedArmourDethKnight;
		break;
	case k7_CreatureTypeRockpile:
		L0359_pui_FixedPossessions = g249FixedPossessionCreature_7rockRockPile;
		break;
	case k4_CreatureTypePainRatHellHound:
		L0359_pui_FixedPossessions = g250FixedPossessionCreature_4PainRatHellHound;
		break;
	case k6_CreatureTypeScreamer:
		L0359_pui_FixedPossessions = g251FixedPossessionCreature_6screamer;
		break;
	case k15_CreatureTypeMagnetaWormWorm:
		L0359_pui_FixedPossessions = g252FixedPossessionCreature_15MagnetaWormWorm;
		break;
	case k24_CreatureTypeRedDragon:
		L0359_pui_FixedPossessions = g253FixedPossessionCreature_24RedDragon;
	}
	uint16 L0356_ui_FixedPossession;
	bool L0362_B_WeaponDropped = false;
	while (L0356_ui_FixedPossession = *L0359_pui_FixedPossessions++) {
		if (getFlag(L0356_ui_FixedPossession, k0x8000_randomDrop) && _vm->getRandomNumber(2))
			continue;
		int16 L0357_i_ThingType;
		if (clearFlag(L0356_ui_FixedPossession, k0x8000_randomDrop) >= k127_ObjectInfoIndexFirstJunk) {
			L0357_i_ThingType = k10_JunkThingType;
			L0356_ui_FixedPossession -= k127_ObjectInfoIndexFirstJunk;
		} else {
			if (L0356_ui_FixedPossession >= k69_ObjectInfoIndexFirstArmour) {
				L0357_i_ThingType = k6_ArmourThingType;
				L0356_ui_FixedPossession -= k69_ObjectInfoIndexFirstArmour;
			} else {
				L0362_B_WeaponDropped = true;
				L0357_i_ThingType = k5_WeaponThingType;
				L0356_ui_FixedPossession -= k23_ObjectInfoIndexFirstWeapon;
			}
		}
		Thing L0358_T_Thing = _vm->_dungeonMan->f166_getUnusedThing(L0357_i_ThingType);
		if ((L0358_T_Thing) == Thing::_none)
			continue;

		Weapon *L0360_ps_Weapon = (Weapon *)_vm->_dungeonMan->f156_getThingData(L0358_T_Thing);
/* The same pointer type is used no matter the actual type k5_WeaponThingType, k6_ArmourThingType or k10_JunkThingType */
		L0360_ps_Weapon->setType(L0356_ui_FixedPossession);
		L0360_ps_Weapon->setCursed(L0361_B_Cursed);
		L0358_T_Thing = M15_thingWithNewCell(L0358_T_Thing, ((cell == k255_CreatureTypeSingleCenteredCreature) || !_vm->getRandomNumber(4)) ? _vm->getRandomNumber(4) : cell);
		_vm->_movsens->f267_getMoveResult(L0358_T_Thing, kM1_MapXNotOnASquare, 0, mapX, mapY);
	}
	_vm->f064_SOUND_RequestPlay_CPSD(L0362_B_WeaponDropped ? k00_soundMETALLIC_THUD : k04_soundWOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM, mapX, mapY, mode);
}

int16 GroupMan::f228_getDirsWhereDestIsVisibleFromSource(int16 srcMapX, int16 srcMapY, int16 destMapX, int16 destMapY) {
#define AP0483_i_PrimaryDirection srcMapX
	int16 L0556_i_Direction;


	if (srcMapX == destMapX) {
		_vm->_projexpl->_g363_secondaryDirToOrFromParty = (_vm->getRandomNumber(65536) & 0x0002) + 1; /* Resulting direction may be 1 or 3 (East or West) */
		if (srcMapY > destMapY) {
			return kDirNorth;
		}
		return kDirSouth;
	}
	if (srcMapY == destMapY) {
		_vm->_projexpl->_g363_secondaryDirToOrFromParty = (_vm->getRandomNumber(65536) & 0x0002) + 0; /* Resulting direction may be 0 or 2 (North or South) */
		if (srcMapX > destMapX) {
			return kDirWest;
		}
		return kDirEast;
	}
	L0556_i_Direction = kDirNorth;
	for (;;) {
		if (f227_isDestVisibleFromSource(L0556_i_Direction, srcMapX, srcMapY, destMapX, destMapY)) {
			if (!f227_isDestVisibleFromSource(_vm->_projexpl->_g363_secondaryDirToOrFromParty = returnNextVal(L0556_i_Direction), srcMapX, srcMapY, destMapX, destMapY)) {
				if ((L0556_i_Direction != kDirNorth) || !f227_isDestVisibleFromSource(_vm->_projexpl->_g363_secondaryDirToOrFromParty = returnPrevVal(L0556_i_Direction), srcMapX, srcMapY, destMapX, destMapY)) {
					_vm->_projexpl->_g363_secondaryDirToOrFromParty = returnNextVal((_vm->getRandomNumber(65536) & 0x0002) + L0556_i_Direction);
					return L0556_i_Direction;
				}
			}
			if (_vm->getRandomNumber(2)) {
				AP0483_i_PrimaryDirection = _vm->_projexpl->_g363_secondaryDirToOrFromParty;
				_vm->_projexpl->_g363_secondaryDirToOrFromParty = L0556_i_Direction;
				return AP0483_i_PrimaryDirection;
			}
			return L0556_i_Direction;
		}
		L0556_i_Direction++;
	}
}

bool GroupMan::f227_isDestVisibleFromSource(uint16 dir, int16 srcMapX, int16 srcMapY, int16 destMapX, int16 destMapY) {
	int L1637_i_Temp;

	switch (dir) { /* If direction is not 'West' then swap variables so that the same test as for west can be applied */
	case kDirSouth:
		L1637_i_Temp = srcMapX;
		srcMapX = destMapY;
		destMapY = L1637_i_Temp;
		L1637_i_Temp = destMapX;
		destMapX = srcMapY;
		srcMapY = L1637_i_Temp;
		break;
	case kDirEast:
		L1637_i_Temp = srcMapX;
		srcMapX = destMapX;
		destMapX = L1637_i_Temp;
		L1637_i_Temp = destMapY;
		destMapY = srcMapY;
		srcMapY = L1637_i_Temp;
		break;
	case kDirNorth:
		L1637_i_Temp = srcMapX;
		srcMapX = srcMapY;
		srcMapY = L1637_i_Temp;
		L1637_i_Temp = destMapX;
		destMapX = destMapY;
		destMapY = L1637_i_Temp;
	}
	return ((srcMapX -= (destMapX - 1)) > 0) && ((((srcMapY -= destMapY) < 0) ? -srcMapY : srcMapY) <= srcMapX);
}

bool GroupMan::f232_groupIsDoorDestoryedByAttack(uint16 mapX, uint16 mapY, int16 attack, bool magicAttack, int16 ticks) {
	Door *L0573_ps_Door = (Door *)_vm->_dungeonMan->f157_getSquareFirstThingData(mapX, mapY);
	if ((magicAttack && !L0573_ps_Door->isMagicDestructible()) || (!magicAttack && !L0573_ps_Door->isMeleeDestructible())) {
		return false;
	}
	if (attack >= _vm->_dungeonMan->_g275_currMapDoorInfo[L0573_ps_Door->getType()]._defense) {
		byte *L0574_puc_Square = &_vm->_dungeonMan->_g271_currMapData[mapX][mapY];
		if (Square(*L0574_puc_Square).getDoorState() == k4_doorState_CLOSED) {
			if (ticks) {
				TimelineEvent L0575_s_Event;
				M33_setMapAndTime(L0575_s_Event._mapTime, _vm->_dungeonMan->_g272_currMapIndex, _vm->_g313_gameTime + ticks);
				L0575_s_Event._type = k2_TMEventTypeDoorDestruction;
				L0575_s_Event._priority = 0;
				L0575_s_Event._B._location._mapX = mapX;
				L0575_s_Event._B._location._mapY = mapY;
				_vm->_timeline->f238_addEventGetEventIndex(&L0575_s_Event);
			} else {
				((Square *)L0574_puc_Square)->setDoorState(k5_doorState_DESTROYED);
			}
			return true;
		}
	}
	return false;
}

Thing GroupMan::f175_groupGetThing(int16 mapX, int16 mapY) {
	Thing L0317_T_Thing;

	L0317_T_Thing = _vm->_dungeonMan->f161_getSquareFirstThing(mapX, mapY);
	while ((L0317_T_Thing != Thing::_endOfList) && ((L0317_T_Thing).getType() != k4_GroupThingType)) {
		L0317_T_Thing = _vm->_dungeonMan->f159_getNextThing(L0317_T_Thing);
	}
	return L0317_T_Thing;
}

int16 GroupMan::f190_groupGetDamageCreatureOutcome(Group *group, uint16 creatureIndex, int16 mapX, int16 mapY, int16 damage, bool notMoving) {
	uint16 L0374_ui_Multiple;
#define AL0374_ui_EventIndex    L0374_ui_Multiple
#define AL0374_ui_CreatureIndex L0374_ui_Multiple
#define AL0374_ui_CreatureSize  L0374_ui_Multiple
#define AL0374_ui_Attack        L0374_ui_Multiple
	uint16 L0375_ui_Multiple;
#define AL0375_ui_Outcome           L0375_ui_Multiple
#define AL0375_ui_EventType         L0375_ui_Multiple
#define AL0375_ui_NextCreatureIndex L0375_ui_Multiple
	CreatureInfo *L0376_ps_CreatureInfo;
	TimelineEvent *L0377_ps_Event;
	ActiveGroup *L0378_ps_ActiveGroup = nullptr;
	uint16 L0379_ui_CreatureCount;
	uint16 L0380_ui_Multiple = 0;
#define AL0380_ui_CreatureType   L0380_ui_Multiple
#define AL0380_ui_FearResistance L0380_ui_Multiple
	uint16 L0381_ui_GroupCells;
	uint16 L0382_ui_GroupDirections;
	bool L0383_B_CurrentMapIsPartyMap;
	uint16 L0384_ui_Cell;


	L0376_ps_CreatureInfo = &g243_CreatureInfo[AL0380_ui_CreatureType = group->_type];
	if (getFlag(L0376_ps_CreatureInfo->_attributes, k0x2000_MaskCreatureInfo_archenemy)) /* Lord Chaos cannot be damaged */
		goto T0190024;
	if (group->_health[creatureIndex] <= damage) {
		L0381_ui_GroupCells = _vm->_groupMan->f145_getGroupCells(group, _vm->_dungeonMan->_g272_currMapIndex);
		L0384_ui_Cell = (L0381_ui_GroupCells == k255_CreatureTypeSingleCenteredCreature) ? k255_CreatureTypeSingleCenteredCreature : _vm->_groupMan->M50_getCreatureValue(L0381_ui_GroupCells, creatureIndex);
		if (!(L0379_ui_CreatureCount = group->getCount())) { /* If there is a single creature in the group */
			if (notMoving) {
				f188_dropGroupPossessions(mapX, mapY, _vm->_groupMan->f175_groupGetThing(mapX, mapY), k2_soundModePlayOneTickLater);
				f189_delete(mapX, mapY);
			}
			AL0375_ui_Outcome = k2_outcomeKilledAllCreaturesInGroup;
		} else { /* If there are several creatures in the group */
			L0382_ui_GroupDirections = _vm->_groupMan->f147_getGroupDirections(group, _vm->_dungeonMan->_g272_currMapIndex);
			if (getFlag(L0376_ps_CreatureInfo->_attributes, k0x0200_MaskCreatureInfo_dropFixedPoss)) {
				if (notMoving) {
					f186_dropCreatureFixedPossessions(AL0380_ui_CreatureType, mapX, mapY, L0384_ui_Cell, k2_soundModePlayOneTickLater);
				} else {
					_g392_dropMovingCreatureFixedPossessionsCell[_g391_dropMovingCreatureFixedPossCellCount++] = L0384_ui_Cell;
				}
			}
			if (L0383_B_CurrentMapIsPartyMap = (_vm->_dungeonMan->_g272_currMapIndex == _vm->_dungeonMan->_g309_partyMapIndex)) {
				L0378_ps_ActiveGroup = &_vm->_groupMan->_g375_activeGroups[group->getActiveGroupIndex()];
			}
			if (group->getBehaviour() == k6_behavior_ATTACK) {
				L0377_ps_Event = _vm->_timeline->_g370_events;
				for (AL0374_ui_EventIndex = 0; AL0374_ui_EventIndex < _vm->_timeline->_g369_eventMaxCount; AL0374_ui_EventIndex++) {
					if ((M29_map(L0377_ps_Event->_mapTime) == _vm->_dungeonMan->_g272_currMapIndex) &&
						(L0377_ps_Event->_B._location._mapX == mapX) &&
						(L0377_ps_Event->_B._location._mapY == mapY) &&
						((AL0375_ui_EventType = L0377_ps_Event->_type) > k32_TMEventTypeUpdateAspectGroup) &&
						(AL0375_ui_EventType < k41_TMEventTypeUpdateBehaviour_3 + 1)) {
						if (AL0375_ui_EventType < k37_TMEventTypeUpdateBehaviourGroup) {
							AL0375_ui_EventType -= k33_TMEventTypeUpdateAspectCreature_0; /* Get creature index for events 33 to 36 */
						} else {
							AL0375_ui_EventType -= k38_TMEventTypeUpdateBehaviour_0; /* Get creature index for events 38 to 41 */
						}
						if (AL0375_ui_NextCreatureIndex == creatureIndex) {
							_vm->_timeline->f237_deleteEvent(AL0374_ui_EventIndex);
						} else {
							if (AL0375_ui_NextCreatureIndex > creatureIndex) {
								L0377_ps_Event->_type -= 1;
								_vm->_timeline->f236_fixChronology(_vm->_timeline->f235_getIndex(AL0374_ui_EventIndex));
							}
						}
					}
					L0377_ps_Event++;
				}
				if (L0383_B_CurrentMapIsPartyMap && ((AL0380_ui_FearResistance = L0376_ps_CreatureInfo->M57_getFearResistance()) != k15_immuneToFear) && ((AL0380_ui_FearResistance += L0379_ui_CreatureCount - 1) < (_vm->getRandomNumber(16)))) { /* Test if the death of a creature frigthens the remaining creatures in the group */
					L0378_ps_ActiveGroup->_delayFleeingFromTarget = _vm->getRandomNumber(100 - (AL0380_ui_FearResistance << 2)) + 20;
					group->setBehaviour(k5_behavior_FLEE);
				}
			}
			for (AL0375_ui_NextCreatureIndex = AL0374_ui_CreatureIndex = creatureIndex; AL0374_ui_CreatureIndex < L0379_ui_CreatureCount; AL0374_ui_CreatureIndex++) {
				AL0375_ui_NextCreatureIndex++;
				group->_health[AL0374_ui_CreatureIndex] = group->_health[AL0375_ui_NextCreatureIndex];
				L0382_ui_GroupDirections = f178_getGroupValueUpdatedWithCreatureValue(L0382_ui_GroupDirections, AL0374_ui_CreatureIndex, _vm->_groupMan->M50_getCreatureValue(L0382_ui_GroupDirections, AL0375_ui_NextCreatureIndex));
				L0381_ui_GroupCells = f178_getGroupValueUpdatedWithCreatureValue(L0381_ui_GroupCells, AL0374_ui_CreatureIndex, _vm->_groupMan->M50_getCreatureValue(L0381_ui_GroupCells, AL0375_ui_NextCreatureIndex));
				if (L0383_B_CurrentMapIsPartyMap) {
					L0378_ps_ActiveGroup->_aspect[AL0374_ui_CreatureIndex] = L0378_ps_ActiveGroup->_aspect[AL0375_ui_NextCreatureIndex];
				}
			}
			L0381_ui_GroupCells &= 0x003F;
			_vm->_dungeonMan->f146_setGroupCells(group, L0381_ui_GroupCells, _vm->_dungeonMan->_g272_currMapIndex);
			_vm->_dungeonMan->f148_setGroupDirections(group, L0382_ui_GroupDirections, _vm->_dungeonMan->_g272_currMapIndex);
			group->setCount(group->getCount() - 1);
			AL0375_ui_Outcome = k1_outcomeKilledSomeCreaturesInGroup;
		}
		if ((AL0374_ui_CreatureSize = getFlag(L0376_ps_CreatureInfo->_attributes, k0x0003_MaskCreatureInfo_size)) == k0_MaskCreatureSizeQuarter) {
			AL0374_ui_Attack = 110;
		} else {
			if (AL0374_ui_CreatureSize == k1_MaskCreatureSizeHalf) {
				AL0374_ui_Attack = 190;
			} else {
				AL0374_ui_Attack = 255;
			}
		}
		_vm->_projexpl->f213_explosionCreate(Thing::_explSmoke, AL0374_ui_Attack, mapX, mapY, L0384_ui_Cell); /* BUG0_66 Smoke is placed on the source map instead of the destination map when a creature dies by falling through a pit. The game has a special case to correctly drop the creature possessions on the destination map but there is no such special case for the smoke. Note that the death must be caused by the damage of the fall (there is no smoke if the creature is removed because its type is not allowed on the destination map). However this bug has no visible consequence because of BUG0_26: the smoke explosion falls in the pit right after being placed in the dungeon and before being drawn on screen so it is only visible on the destination square */
		return AL0375_ui_Outcome;
	}
	if (damage > 0) {
		group->_health[creatureIndex] -= damage;
	}
T0190024:
	return k0_outcomeKilledNoCreaturesInGroup;
}

void GroupMan::f189_delete(int16 mapX, int16 mapY) {
	Thing L0372_T_GroupThing;
	Group *L0373_ps_Group;


	if ((L0372_T_GroupThing = _vm->_groupMan->f175_groupGetThing(mapX, mapY)) == Thing::_endOfList) {
		return;
	}
	L0373_ps_Group = (Group *)_vm->_dungeonMan->f156_getThingData(L0372_T_GroupThing);
	for (uint16 i = 0; i < 4; ++i)
		L0373_ps_Group->_health[i] = 0;
	_vm->_movsens->f267_getMoveResult(L0372_T_GroupThing, mapX, mapY, kM1_MapXNotOnASquare, 0);
	L0373_ps_Group->_nextThing = Thing::_none;
	if (_vm->_dungeonMan->_g272_currMapIndex == _vm->_dungeonMan->_g309_partyMapIndex) {
		_vm->_groupMan->_g375_activeGroups[L0373_ps_Group->getActiveGroupIndex()]._groupThingIndex = -1;
		_g377_currActiveGroupCount--;
	}
	f181_groupDeleteEvents(mapX, mapY);
}

void GroupMan::f181_groupDeleteEvents(int16 mapX, int16 mapY) {
	int16 L0334_i_EventIndex;
	uint16 L0335_ui_EventType;
	TimelineEvent *L0336_ps_Event;


	L0336_ps_Event = _vm->_timeline->_g370_events;
	for (L0334_i_EventIndex = 0; L0334_i_EventIndex < _vm->_timeline->_g369_eventMaxCount; L0334_i_EventIndex++) {
		if ((M29_map(L0336_ps_Event->_mapTime) == _vm->_dungeonMan->_g272_currMapIndex) &&
			((L0335_ui_EventType = L0336_ps_Event->_type) > k29_TMEventTypeGroupReactionDangerOnSquare - 1) && (L0335_ui_EventType < k41_TMEventTypeUpdateBehaviour_3 + 1) &&
			(L0336_ps_Event->_B._location._mapX == mapX) && (L0336_ps_Event->_B._location._mapY == mapY)) {
			_vm->_timeline->f237_deleteEvent(L0334_i_EventIndex);
		}
		L0336_ps_Event++;
	}
}

uint16 GroupMan::f178_getGroupValueUpdatedWithCreatureValue(uint16 groupVal, uint16 creatureIndex, uint16 creatreVal) {
	creatreVal &= 0x0003;
	creatreVal <<= (creatureIndex <<= 1);
	return creatreVal | (groupVal & ~(3 << creatreVal));
}

int16 GroupMan::f191_getDamageAllCreaturesOutcome(Group *group, int16 mapX, int16 mapY, int16 attack, bool notMoving) {
	uint16 L0385_ui_RandomAttack;
	int16 L0386_i_CreatureIndex;
	int16 L0387_i_Outcome;
	bool L0388_B_KilledSomeCreatures;
	bool L0389_B_KilledAllCreatures;


	L0388_B_KilledSomeCreatures = false;
	L0389_B_KilledAllCreatures = true;
	_g391_dropMovingCreatureFixedPossCellCount = 0;
	if (attack > 0) {
		L0386_i_CreatureIndex = group->getCount();
		attack -= (L0385_ui_RandomAttack = (attack >> 3) + 1);
		L0385_ui_RandomAttack <<= 1;
		do {
			L0389_B_KilledAllCreatures = (L0387_i_Outcome = f190_groupGetDamageCreatureOutcome(group, L0386_i_CreatureIndex, mapX, mapY, attack + _vm->getRandomNumber(L0385_ui_RandomAttack), notMoving)) && L0389_B_KilledAllCreatures;
			L0388_B_KilledSomeCreatures = L0388_B_KilledSomeCreatures || L0387_i_Outcome;
		} while (L0386_i_CreatureIndex--);
		if (L0389_B_KilledAllCreatures) {
			return k2_outcomeKilledAllCreaturesInGroup;
		}
		if (L0388_B_KilledSomeCreatures) {
			return k1_outcomeKilledSomeCreaturesInGroup;
		}
		return k0_outcomeKilledNoCreaturesInGroup;
	} else {
		return k0_outcomeKilledNoCreaturesInGroup;
	}
}

int16 GroupMan::f192_groupGetResistanceAdjustedPoisonAttack(uint16 creatreType, int16 poisonAttack) {
	int16 L0390_i_PoisonResistance;


	if (!poisonAttack || ((L0390_i_PoisonResistance = g243_CreatureInfo[creatreType].M61_poisonResistance()) == k15_immuneToPoison)) {
		return 0;
	}
	return ((poisonAttack + _vm->getRandomNumber(4)) << 3) / ++L0390_i_PoisonResistance;
}

void GroupMan::f209_processEvents29to41(int16 eventMapX, int16 eventMapY, int16 eventType, uint16 ticks) {
	Group *L0444_ps_Group;
	ActiveGroup *L0445_ps_ActiveGroup;
	int16 L0446_i_Multiple;
#define AL0446_i_EventType           L0446_i_Multiple
#define AL0446_i_Direction           L0446_i_Multiple
#define AL0446_i_Ticks               L0446_i_Multiple
#define AL0446_i_Distance            L0446_i_Multiple
#define AL0446_i_Behavior2Or3        L0446_i_Multiple
#define AL0446_i_CreatureAspectIndex L0446_i_Multiple
#define AL0446_i_Range               L0446_i_Multiple
#define AL0446_i_CreatureAttributes  L0446_i_Multiple
#define AL0446_i_Cell                L0446_i_Multiple
#define AL0446_i_GroupCellsCriteria  L0446_i_Multiple
	int16 L0447_i_Multiple;
#define AL0447_i_Behavior           L0447_i_Multiple
#define AL0447_i_CreatureIndex      L0447_i_Multiple
#define AL0447_i_ReferenceDirection L0447_i_Multiple
#define AL0447_i_Ticks              L0447_i_Multiple
	CreatureInfo L0448_s_CreatureInfo;
	Thing L0449_T_GroupThing;
	int16 L0450_i_Multiple;
#define AL0450_i_DestinationMapX  L0450_i_Multiple
#define AL0450_i_DistanceXToParty L0450_i_Multiple
#define AL0450_i_TargetMapX       L0450_i_Multiple
	int16 L0451_i_Multiple;
#define AL0451_i_DestinationMapY  L0451_i_Multiple
#define AL0451_i_DistanceYToParty L0451_i_Multiple
#define AL0451_i_TargetMapY       L0451_i_Multiple
	int16 L0452_i_DistanceToVisibleParty = 0;
	bool L0453_B_NewGroupDirectionFound;
	int16 L0454_i_PrimaryDirectionToOrFromParty;
	bool L0455_B_CurrentEventTypeIsNotUpdateBehavior;
	bool L0456_B_AllowMovementOverImaginaryPitsAndFakeWalls;
	bool L0457_B_MoveToPriorLocation;
	bool L0458_B_SetBehavior7_ApproachAfterReaction = false;
	int16 L0459_i_CreatureSize;
	uint16 L0460_ui_CreatureCount;
	int16 L0461_i_MovementTicks;
	int16 L0462_i_TicksSinceLastMove;
	bool L0463_B_Archenemy;
	int32 L0464_l_NextAspectUpdateTime;
	TimelineEvent L0465_s_NextEvent;


	/* If the party is not on the map specified in the event and the event type is not one of 32, 33, 37, 38 then the event is ignored */
	if ((_vm->_dungeonMan->_g272_currMapIndex != _vm->_dungeonMan->_g309_partyMapIndex) && ((AL0446_i_EventType = eventType) != k37_TMEventTypeUpdateBehaviourGroup) && (AL0446_i_EventType != k32_TMEventTypeUpdateAspectGroup) && (AL0446_i_EventType != k38_TMEventTypeUpdateBehaviour_0) && (AL0446_i_EventType != k33_TMEventTypeUpdateAspectCreature_0))
		goto T0209139_Return;
	/* If there is no creature at the location specified in the event then the event is ignored */
	if ((L0449_T_GroupThing = _vm->_groupMan->f175_groupGetThing(eventMapX, eventMapY)) == Thing::_endOfList) {
		goto T0209139_Return;
	}
	L0444_ps_Group = (Group *)_vm->_dungeonMan->f156_getThingData(L0449_T_GroupThing);
	L0448_s_CreatureInfo = g243_CreatureInfo[L0444_ps_Group->_type];
	/* Update the event */
	M33_setMapAndTime(L0465_s_NextEvent._mapTime, _vm->_dungeonMan->_g272_currMapIndex, _vm->_g313_gameTime);
	L0465_s_NextEvent._priority = 255 - L0448_s_CreatureInfo._movementTicks; /* The fastest creatures (with small MovementTicks value) get higher event priority */
	L0465_s_NextEvent._B._location._mapX = eventMapX;
	L0465_s_NextEvent._B._location._mapY = eventMapY;
	/* If the creature is not on the party map then try and move the creature in a random direction and place a new event 37 in the timeline for the next creature movement */
	if (_vm->_dungeonMan->_g272_currMapIndex != _vm->_dungeonMan->_g309_partyMapIndex) {
		if (f202_isMovementPossible(&L0448_s_CreatureInfo, eventMapX, eventMapY, AL0446_i_Direction = _vm->getRandomNumber(4), false)) { /* BUG0_67 A group that is not on the party map may wrongly move or not move into a teleporter. Normally, a creature type with Wariness >= 10 (Vexirk, Materializer / Zytaz, Demon, Lord Chaos, Red Dragon / Dragon) would only move into a teleporter if the creature type is allowed on the destination map. However, the variable G0380_T_CurrentGroupThing identifying the group is not set before being used by F0139_DUNGEON_IsCreatureAllowedOnMap called by f202_isMovementPossible so the check to see if the creature type is allowed may operate on another creature type and thus return an incorrect result, causing the creature to teleport while it should not, or not to teleport while it should */
			AL0450_i_DestinationMapX = eventMapX;
			AL0451_i_DestinationMapY = eventMapY;
			AL0450_i_DestinationMapX += _vm->_dirIntoStepCountEast[AL0446_i_Direction], AL0451_i_DestinationMapY += _vm->_dirIntoStepCountNorth[AL0446_i_Direction];
			if (_vm->_movsens->f267_getMoveResult(L0449_T_GroupThing, eventMapX, eventMapY, AL0450_i_DestinationMapX, AL0451_i_DestinationMapY))
				goto T0209139_Return;
			L0465_s_NextEvent._B._location._mapX = _vm->_movsens->_g397_moveResultMapX;
			L0465_s_NextEvent._B._location._mapY = _vm->_movsens->_g398_moveResultMapY;
		}
		L0465_s_NextEvent._type = k37_TMEventTypeUpdateBehaviourGroup;
		AL0446_i_Ticks = MAX(ABS(_vm->_dungeonMan->_g272_currMapIndex - _vm->_dungeonMan->_g309_partyMapIndex) << 4, L0448_s_CreatureInfo._movementTicks << 1);
		/* BUG0_68 A group moves or acts with a wrong timing. Event is added below but L0465_s_NextEvent.C.Ticks has not been initialized. No consequence while the group is not on the party map. When the party enters the group map the first group event may have a wrong timing */
T0209005_AddEventAndReturn:
		L0465_s_NextEvent._mapTime += AL0446_i_Ticks;
		_vm->_timeline->f238_addEventGetEventIndex(&L0465_s_NextEvent);
		goto T0209139_Return;
	}
	/* If the creature is Lord Chaos then ignore the event if the game is won. Initialize data to analyze Fluxcages */
	if (L0463_B_Archenemy = getFlag(L0448_s_CreatureInfo._attributes, k0x2000_MaskCreatureInfo_archenemy)) {
		if (_vm->_g302_gameWon) {
			goto T0209139_Return;
		}
		_g386_fluxCageCount = 0;
		_g385_fluxCages[0] = 0;
	}
	L0445_ps_ActiveGroup = &_vm->_groupMan->_g375_activeGroups[L0444_ps_Group->getActiveGroupIndex()];
	if ((L0462_i_TicksSinceLastMove = (unsigned char)_vm->_g313_gameTime - L0445_ps_ActiveGroup->_lastMoveTime) < 0) {
		L0462_i_TicksSinceLastMove += 256;
	}
	if ((L0461_i_MovementTicks = L0448_s_CreatureInfo._movementTicks) == k255_immobile) {
		L0461_i_MovementTicks = 100;
	}
	if (_vm->_championMan->_g407_party._freezeLifeTicks && !L0463_B_Archenemy) { /* If life is frozen and the creature is not Lord Chaos (Lord Chaos is immune to Freeze Life) then reschedule the event later (except for reactions which are ignored when life if frozen) */
		if (eventType < 0)
			goto T0209139_Return;
		L0465_s_NextEvent._type = eventType;
		L0465_s_NextEvent._C._ticks = ticks;
		AL0446_i_Ticks = 4; /* Retry in 4 ticks */
		goto T0209005_AddEventAndReturn;
	}
	/* If the specified event type is a 'reaction' instead of a real event from the timeline then create the corresponding reaction event with a delay:
	For event kM1_TMEventTypeCreateReactionEvent31ParyIsAdjacent, the reaction time is 1 tick
	For event kM2_TMEventTypeCreateReactionEvent30HitByProjectile and kM3_TMEventTypeCreateReactionEvent29DangerOnSquare, the reaction time may be 1 tick or slower: slow moving creatures react more slowly. The more recent is the last creature move, the slower the reaction */
	if (eventType < 0) {
		L0465_s_NextEvent._type = eventType + k32_TMEventTypeUpdateAspectGroup;
		if ((eventType == kM1_TMEventTypeCreateReactionEvent31ParyIsAdjacent) || ((AL0446_i_Ticks = ((L0461_i_MovementTicks + 2) >> 2) - L0462_i_TicksSinceLastMove) < 1)) { /* AL0446_i_Ticks is the reaction time */
			AL0446_i_Ticks = 1; /* Retry in 1 tick */
		}
		goto T0209005_AddEventAndReturn; /* BUG0_68 A group moves or acts with a wrong timing. Event is added but L0465_s_NextEvent.C.Ticks has not been initialized */
	}
	AL0447_i_Behavior = L0444_ps_Group->getBehaviour();
	L0460_ui_CreatureCount = L0444_ps_Group->getCount();
	L0459_i_CreatureSize = getFlag(L0448_s_CreatureInfo._attributes, k0x0003_MaskCreatureInfo_size);
	AL0450_i_DistanceXToParty = ((AL0446_i_Distance = eventMapX - _vm->_dungeonMan->_g306_partyMapX) < 0) ? -AL0446_i_Distance : AL0446_i_Distance;
	AL0451_i_DistanceYToParty = ((AL0446_i_Distance = eventMapY - _vm->_dungeonMan->_g307_partyMapY) < 0) ? -AL0446_i_Distance : AL0446_i_Distance;
	_g378_currentGroupMapX = eventMapX;
	_g379_currentGroupMapY = eventMapY;
	_g380_currGroupThing = L0449_T_GroupThing;
	_g384_groupMovementTestedDirections[0] = 0;
	_g381_currGroupDistanceToParty = f226_getDistanceBetweenSquares(eventMapX, eventMapY, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY);
	_g382_currGroupPrimaryDirToParty = f228_getDirsWhereDestIsVisibleFromSource(eventMapX, eventMapY, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY);
	_g383_currGroupSecondaryDirToParty = _vm->_projexpl->_g363_secondaryDirToOrFromParty;
	L0464_l_NextAspectUpdateTime = 0;
	L0455_B_CurrentEventTypeIsNotUpdateBehavior = true;
	if (eventType <= k31_TMEventTypeGroupReactionPartyIsAdjecent) { /* Process Reaction events 29 to 31 */
		switch (eventType = eventType - k32_TMEventTypeUpdateAspectGroup) {
		case kM1_TMEventTypeCreateReactionEvent31ParyIsAdjacent: /* This event is used when the party bumps into a group or attacks a group physically (not with a spell). It causes the creature behavior to change to attack if it is not already attacking the party or fleeing from target */
			if ((AL0447_i_Behavior != k6_behavior_ATTACK) && (AL0447_i_Behavior != k5_behavior_FLEE)) {
				f181_groupDeleteEvents(eventMapX, eventMapY);
				goto T0209044_SetBehavior6_Attack;
			}
			L0445_ps_ActiveGroup->_targetMapX = _vm->_dungeonMan->_g306_partyMapX;
			L0445_ps_ActiveGroup->_targetMapY = _vm->_dungeonMan->_g307_partyMapY;
			goto T0209139_Return;
		case kM2_TMEventTypeCreateReactionEvent30HitByProjectile: /* This event is used for the reaction of a group after a projectile impacted with one creature in the group (some creatures may have been killed) */
			if ((AL0447_i_Behavior == k6_behavior_ATTACK) || (AL0447_i_Behavior == k5_behavior_FLEE)) /* If the creature is attacking the party or fleeing from the target then there is no reaction */
				goto T0209139_Return;
			if ((AL0446_i_Behavior2Or3 = ((AL0447_i_Behavior == k3_behavior_USELESS) || (AL0447_i_Behavior == k2_behavior_USELESS))) || (_vm->getRandomNumber(4))) { /* BUG0_00 Useless code. Behavior cannot be 2 nor 3 because these values are never used. The actual condition is thus: if 3/4 chances */
				if (!f200_groupGetDistanceToVisibleParty(L0444_ps_Group, kM1_wholeCreatureGroup, eventMapX, eventMapY)) { /* If the group cannot see the party then look in a random direction to try and search for the party */
					L0458_B_SetBehavior7_ApproachAfterReaction = L0453_B_NewGroupDirectionFound = false;
					goto T0209073_SetDirectionGroup;
				}
				if (AL0446_i_Behavior2Or3 || (_vm->getRandomNumber(4))) /* BUG0_00 Useless code. Behavior cannot be 2 nor 3 because these values are never used. The actual condition is thus: if 3/4 chances then no reaction */
					goto T0209139_Return;
			} /* No 'break': proceed to instruction after the next 'case' below. Reaction is to move in a random direction to try and avoid other projectiles */
		case kM3_TMEventTypeCreateReactionEvent29DangerOnSquare: /* This event is used when some creatures in the group were killed by a Poison Cloud or by a closing door or if Lord Chaos is surrounded by 3 Fluxcages. It causes the creature to move in a random direction to avoid the danger */
			L0458_B_SetBehavior7_ApproachAfterReaction = (AL0447_i_Behavior == k6_behavior_ATTACK); /* If the creature behavior is 'Attack' and it has to move to avoid danger then it will change its behavior to 'Approach' after the movement */
			L0453_B_NewGroupDirectionFound = false;
			goto T0209058_MoveInRandomDirection;
		}
	}
	if (eventType < k37_TMEventTypeUpdateBehaviourGroup) { /* Process Update Aspect events 32 to 36 */
		L0465_s_NextEvent._type = eventType + 5;
		if (f200_groupGetDistanceToVisibleParty(L0444_ps_Group, kM1_wholeCreatureGroup, eventMapX, eventMapY)) {
			if ((AL0447_i_Behavior != k6_behavior_ATTACK) && (AL0447_i_Behavior != k5_behavior_FLEE)) {
				if (M38_distance(_vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, eventMapX, eventMapY) <= 1)
					goto T0209044_SetBehavior6_Attack;
				if (((AL0447_i_Behavior == k0_behavior_WANDER) || (AL0447_i_Behavior == k3_behavior_USELESS)) && (AL0447_i_Behavior != k7_behavior_APPROACH)) /* BUG0_00 Useless code. Behavior cannot be 3 because this value is never used. Moreover, the second condition in the && is redundant (if the value is 0 or 3, it cannot be 7). The actual condition is: if (AL0447_i_Behavior == k0_behavior_WANDER) */
					goto T0209054_SetBehavior7_Approach;
			}
			L0445_ps_ActiveGroup->_targetMapX = _vm->_dungeonMan->_g306_partyMapX;
			L0445_ps_ActiveGroup->_targetMapY = _vm->_dungeonMan->_g307_partyMapY;
		}
		if (AL0447_i_Behavior == k6_behavior_ATTACK) {
			AL0446_i_CreatureAspectIndex = eventType - k33_TMEventTypeUpdateAspectCreature_0; /* Value -1 for event 32, meaning aspect will be updated for all creatures in the group */
			L0464_l_NextAspectUpdateTime = f179_getCreatureAspectUpdateTime(L0445_ps_ActiveGroup, AL0446_i_CreatureAspectIndex, getFlag(L0445_ps_ActiveGroup->_aspect[AL0446_i_CreatureAspectIndex], k0x0080_MaskActiveGroupIsAttacking));
			goto T0209136;
		}
		if ((AL0450_i_DistanceXToParty > 3) || (AL0451_i_DistanceYToParty > 3)) {
			L0464_l_NextAspectUpdateTime = _vm->_g313_gameTime + ((L0448_s_CreatureInfo._animationTicks >> 4) & 0xF);
			goto T0209136;
		}
	} else { /* Process Update Behavior events 37 to 41 */
		L0455_B_CurrentEventTypeIsNotUpdateBehavior = false;
		if (ticks) {
			L0464_l_NextAspectUpdateTime = _vm->_g313_gameTime;
		}
		if (eventType == k37_TMEventTypeUpdateBehaviourGroup) { /* Process event 37, Update Group Behavior */
			if ((AL0447_i_Behavior == k0_behavior_WANDER) || (AL0447_i_Behavior == k2_behavior_USELESS) || (AL0447_i_Behavior == k3_behavior_USELESS)) { /* BUG0_00 Useless code. Behavior cannot be 2 nor 3 because these values are never used. The actual condition is: if (AL0447_i_Behavior == k0_behavior_WANDER) */
				if (L0452_i_DistanceToVisibleParty = f200_groupGetDistanceToVisibleParty(L0444_ps_Group, kM1_wholeCreatureGroup, eventMapX, eventMapY)) {
					if ((L0452_i_DistanceToVisibleParty <= (L0448_s_CreatureInfo.M56_getAttackRange())) && ((!AL0450_i_DistanceXToParty) || (!AL0451_i_DistanceYToParty))) { /* If the creature is in range for attack and on the same row or column as the party on the map */
T0209044_SetBehavior6_Attack:
						if (eventType == kM2_TMEventTypeCreateReactionEvent30HitByProjectile) {
							f181_groupDeleteEvents(eventMapX, eventMapY);
						}
						L0445_ps_ActiveGroup->_targetMapX = _vm->_dungeonMan->_g306_partyMapX;
						L0445_ps_ActiveGroup->_targetMapY = _vm->_dungeonMan->_g307_partyMapY;
						L0444_ps_Group->setBehaviour(k6_behavior_ATTACK);
						AL0446_i_Direction = _g382_currGroupPrimaryDirToParty;
						for (AL0447_i_CreatureIndex = L0460_ui_CreatureCount; AL0447_i_CreatureIndex >= 0; AL0447_i_CreatureIndex--) {
							if ((_vm->_groupMan->M50_getCreatureValue(L0445_ps_ActiveGroup->_directions, AL0447_i_CreatureIndex) != AL0446_i_Direction) &&
								((!AL0447_i_CreatureIndex) || (!_vm->getRandomNumber(2)))) {
								f205_setDirection(L0445_ps_ActiveGroup, AL0446_i_Direction, AL0447_i_CreatureIndex, L0460_ui_CreatureCount && (L0459_i_CreatureSize == k1_MaskCreatureSizeHalf));
								M32_setTime(L0465_s_NextEvent._mapTime, _vm->_g313_gameTime + _vm->getRandomNumber(4) + 2); /* Random delay represents the time for the creature to turn */
							} else {
								M32_setTime(L0465_s_NextEvent._mapTime, _vm->_g313_gameTime + 1);
							}
							if (L0455_B_CurrentEventTypeIsNotUpdateBehavior) {
								L0465_s_NextEvent._mapTime += MIN((uint16)((L0448_s_CreatureInfo._attackTicks >> 1) + _vm->getRandomNumber(4)), ticks);
							}
							L0465_s_NextEvent._type = k38_TMEventTypeUpdateBehaviour_0 + AL0447_i_CreatureIndex;
							f208_groupAddEvent(&L0465_s_NextEvent, f179_getCreatureAspectUpdateTime(L0445_ps_ActiveGroup, AL0447_i_CreatureIndex, false));
						}
						goto T0209139_Return;
					}
					if (AL0447_i_Behavior != k2_behavior_USELESS) { /* BUG0_00 Useless code. Behavior cannot be 2 because this value is never used */
T0209054_SetBehavior7_Approach:
						L0444_ps_Group->setBehaviour(k7_behavior_APPROACH);
						L0445_ps_ActiveGroup->_targetMapX = _vm->_dungeonMan->_g306_partyMapX;
						L0445_ps_ActiveGroup->_targetMapY = _vm->_dungeonMan->_g307_partyMapY;
						L0465_s_NextEvent._mapTime += 1;
						goto T0209134_SetEvent37;
					}
				} else {
					if (AL0447_i_Behavior == k0_behavior_WANDER) {
						if (L0454_i_PrimaryDirectionToOrFromParty = f201_getSmelledPartyPrimaryDirOrdinal(&L0448_s_CreatureInfo, eventMapX, eventMapY)) {
							L0454_i_PrimaryDirectionToOrFromParty--;
							L0456_B_AllowMovementOverImaginaryPitsAndFakeWalls = false;
							goto T0209085_SingleSquareMove;
						}
						L0453_B_NewGroupDirectionFound = false;
						if (_vm->getRandomNumber(2)) {
T0209058_MoveInRandomDirection:
							AL0446_i_Direction = _vm->getRandomNumber(4);
							AL0447_i_ReferenceDirection = AL0446_i_Direction;
							L0457_B_MoveToPriorLocation = false;
							do {
								AL0450_i_DestinationMapX = eventMapX;
								AL0451_i_DestinationMapY = eventMapY;
								AL0450_i_DestinationMapX += _vm->_dirIntoStepCountEast[AL0446_i_Direction], AL0451_i_DestinationMapY += _vm->_dirIntoStepCountNorth[AL0446_i_Direction];
								if (((L0445_ps_ActiveGroup->_priorMapX != AL0450_i_DestinationMapX) ||
									(L0445_ps_ActiveGroup->_priorMapY != AL0451_i_DestinationMapY) ||
									 (L0457_B_MoveToPriorLocation = !_vm->getRandomNumber(4))) /* 1/4 chance of moving back to the square that the creature comes from */
									&& f202_isMovementPossible(&L0448_s_CreatureInfo, eventMapX, eventMapY, AL0446_i_Direction, false)) {
T0209061_MoveGroup:
									if (L0453_B_NewGroupDirectionFound = ((AL0447_i_Ticks = (L0461_i_MovementTicks >> 1) - L0462_i_TicksSinceLastMove) <= 0)) {
										if (_vm->_movsens->f267_getMoveResult(L0449_T_GroupThing, eventMapX, eventMapY, AL0450_i_DestinationMapX, AL0451_i_DestinationMapY))
											goto T0209139_Return;
										L0465_s_NextEvent._B._location._mapX = _vm->_movsens->_g397_moveResultMapX;
										L0465_s_NextEvent._B._location._mapY = _vm->_movsens->_g398_moveResultMapY;;
										L0445_ps_ActiveGroup->_priorMapX = eventMapX;
										L0445_ps_ActiveGroup->_priorMapY = eventMapY;
										L0445_ps_ActiveGroup->_lastMoveTime = _vm->_g313_gameTime;
									} else {
										L0461_i_MovementTicks = AL0447_i_Ticks;
										L0462_i_TicksSinceLastMove = -1;
									}
									break;
								}
								if (_g390_groupMovementBlockedByParty) {
									if ((eventType != kM3_TMEventTypeCreateReactionEvent29DangerOnSquare) &&
										((L0444_ps_Group->getBehaviour() != k5_behavior_FLEE) ||
										 !f203_getFirstPossibleMovementDirOrdinal(&L0448_s_CreatureInfo, eventMapX, eventMapY, false) ||
										 _vm->getRandomNumber(2)))
										goto T0209044_SetBehavior6_Attack;
									L0445_ps_ActiveGroup->_targetMapX = _vm->_dungeonMan->_g306_partyMapX;
									L0445_ps_ActiveGroup->_targetMapY = _vm->_dungeonMan->_g307_partyMapY;
								}
							} while ((AL0446_i_Direction = returnNextVal(AL0446_i_Direction)) != AL0447_i_ReferenceDirection);
						}
						if (!L0453_B_NewGroupDirectionFound &&
							(L0462_i_TicksSinceLastMove != -1) &&
							L0463_B_Archenemy &&
							((eventType == kM3_TMEventTypeCreateReactionEvent29DangerOnSquare) || !_vm->getRandomNumber(4))) { /* BUG0_15 The game hangs when you close a door on Lord Chaos. A condition is missing in the code to manage creatures and this may create an infinite loop between two parts in the code */
							_vm->_projexpl->_g363_secondaryDirToOrFromParty = returnNextVal(L0454_i_PrimaryDirectionToOrFromParty = _vm->getRandomNumber(4));
							goto T0209089_DoubleSquareMove; /* BUG0_69 Memory corruption when you close a door on Lord Chaos. The local variable (L0454_i_PrimaryDirectionToOrFromParty) containing the direction where Lord Chaos tries to move may be used as an array index without being initialized and cause memory corruption */
						}
						if (L0453_B_NewGroupDirectionFound || ((!_vm->getRandomNumber(4) || (L0452_i_DistanceToVisibleParty <= L0448_s_CreatureInfo.M55_getSmellRange())) && (eventType != kM3_TMEventTypeCreateReactionEvent29DangerOnSquare))) {
T0209073_SetDirectionGroup:
							if (!L0453_B_NewGroupDirectionFound && (L0462_i_TicksSinceLastMove >= 0)) { /* If direction is not found yet then look around in a random direction */
								AL0446_i_Direction = _vm->getRandomNumber(4);
							}
							f206_groupSetDirGroup(L0445_ps_ActiveGroup, AL0446_i_Direction, L0460_ui_CreatureCount, L0459_i_CreatureSize);
						}
						/* If event is kM3_TMEventTypeCreateReactionEvent29DangerOnSquare or kM2_TMEventTypeCreateReactionEvent30HitByProjectile */
						if (eventType < kM1_TMEventTypeCreateReactionEvent31ParyIsAdjacent) {
							if (!L0453_B_NewGroupDirectionFound)
								goto T0209139_Return;
							if (L0458_B_SetBehavior7_ApproachAfterReaction) {
								L0444_ps_Group->setBehaviour(k7_behavior_APPROACH);
							}
							f182_stopAttacking(L0445_ps_ActiveGroup, eventMapX, eventMapY);
						}
					}
				}
			} else {
				if (AL0447_i_Behavior == k7_behavior_APPROACH) {
					if (L0452_i_DistanceToVisibleParty = f200_groupGetDistanceToVisibleParty(L0444_ps_Group, kM1_wholeCreatureGroup, eventMapX, eventMapY)) {
						if ((L0452_i_DistanceToVisibleParty <= L0448_s_CreatureInfo.M56_getAttackRange()) && ((!AL0450_i_DistanceXToParty) || (!AL0451_i_DistanceYToParty))) /* If the creature is in range for attack and on the same row or column as the party on the map */
							goto T0209044_SetBehavior6_Attack;
T0209081_RunTowardParty:
						L0461_i_MovementTicks++;
						L0461_i_MovementTicks = L0461_i_MovementTicks >> 1; /* Running speed is half the movement ticks */
						AL0450_i_TargetMapX = (L0445_ps_ActiveGroup->_targetMapX = _vm->_dungeonMan->_g306_partyMapX);
						AL0451_i_TargetMapY = (L0445_ps_ActiveGroup->_targetMapY = _vm->_dungeonMan->_g307_partyMapY);
					} else {
T0209082_WalkTowardTarget:
						AL0450_i_TargetMapX = L0445_ps_ActiveGroup->_targetMapX;
						AL0451_i_TargetMapY = L0445_ps_ActiveGroup->_targetMapY;
						/* If the creature reached its target but the party is not there anymore */
						if ((eventMapX == AL0450_i_TargetMapX) && (eventMapY == AL0451_i_TargetMapY)) {
							L0453_B_NewGroupDirectionFound = false;
							L0444_ps_Group->setBehaviour(k0_behavior_WANDER);
							goto T0209073_SetDirectionGroup;
						}
					}
					L0456_B_AllowMovementOverImaginaryPitsAndFakeWalls = true;
T0209084_SingleSquareMoveTowardParty:
					L0454_i_PrimaryDirectionToOrFromParty = f228_getDirsWhereDestIsVisibleFromSource(eventMapX, eventMapY, AL0450_i_TargetMapX, AL0451_i_TargetMapY);
T0209085_SingleSquareMove:
					if (f202_isMovementPossible(&L0448_s_CreatureInfo, eventMapX, eventMapY, AL0446_i_Direction = L0454_i_PrimaryDirectionToOrFromParty, L0456_B_AllowMovementOverImaginaryPitsAndFakeWalls) ||
						f202_isMovementPossible(&L0448_s_CreatureInfo, eventMapX, eventMapY, AL0446_i_Direction = _vm->_projexpl->_g363_secondaryDirToOrFromParty, L0456_B_AllowMovementOverImaginaryPitsAndFakeWalls && _vm->getRandomNumber(2)) ||
						f202_isMovementPossible(&L0448_s_CreatureInfo, eventMapX, eventMapY, AL0446_i_Direction = returnOppositeDir((direction)AL0446_i_Direction), false) ||
						(!_vm->getRandomNumber(4) && f202_isMovementPossible(&L0448_s_CreatureInfo, eventMapX, eventMapY, AL0446_i_Direction = returnOppositeDir((direction)L0454_i_PrimaryDirectionToOrFromParty), false))) {
						AL0450_i_DestinationMapX = eventMapX;
						AL0451_i_DestinationMapY = eventMapY;
						AL0450_i_DestinationMapX += _vm->_dirIntoStepCountEast[AL0446_i_Direction], AL0451_i_DestinationMapY += _vm->_dirIntoStepCountNorth[AL0446_i_Direction];
						goto T0209061_MoveGroup;
					}
					if (L0463_B_Archenemy) {
T0209089_DoubleSquareMove:
						f203_getFirstPossibleMovementDirOrdinal(&L0448_s_CreatureInfo, eventMapX, eventMapY, false); /* BUG0_00 Useless code. Returned value is ignored. When Lord Chaos teleports two squares away the ability to move to the first square is ignored which means Lord Chaos can teleport through walls or any other obstacle */
						if (f204_isArchenemyDoubleMovementPossible(&L0448_s_CreatureInfo, eventMapX, eventMapY, AL0446_i_Direction = L0454_i_PrimaryDirectionToOrFromParty) ||
							f204_isArchenemyDoubleMovementPossible(&L0448_s_CreatureInfo, eventMapX, eventMapY, AL0446_i_Direction = _vm->_projexpl->_g363_secondaryDirToOrFromParty) ||
							(_g386_fluxCageCount && f204_isArchenemyDoubleMovementPossible(&L0448_s_CreatureInfo, eventMapX, eventMapY, AL0446_i_Direction = returnOppositeDir((direction)AL0446_i_Direction))) ||
							((_g386_fluxCageCount >= 2) && f204_isArchenemyDoubleMovementPossible(&L0448_s_CreatureInfo, eventMapX, eventMapY, AL0446_i_Direction = returnOppositeDir((direction)L0454_i_PrimaryDirectionToOrFromParty)))) {
							AL0450_i_DestinationMapX = eventMapX;
							AL0451_i_DestinationMapY = eventMapY;
							AL0450_i_DestinationMapX += _vm->_dirIntoStepCountEast[AL0446_i_Direction] * 2, AL0451_i_DestinationMapY += _vm->_dirIntoStepCountNorth[AL0446_i_Direction] * 2;
							_vm->f064_SOUND_RequestPlay_CPSD(k17_soundBUZZ, AL0450_i_DestinationMapX, AL0451_i_DestinationMapY, k1_soundModePlayIfPrioritized);
							goto T0209061_MoveGroup;
						}
					}
					f206_groupSetDirGroup(L0445_ps_ActiveGroup, L0454_i_PrimaryDirectionToOrFromParty, L0460_ui_CreatureCount, L0459_i_CreatureSize);
				} else {
					if (AL0447_i_Behavior == k5_behavior_FLEE) {
T0209094_FleeFromTarget:
						L0456_B_AllowMovementOverImaginaryPitsAndFakeWalls = true;
						/* If the creature can see the party then update target coordinates */
						if (L0452_i_DistanceToVisibleParty = f200_groupGetDistanceToVisibleParty(L0444_ps_Group, kM1_wholeCreatureGroup, eventMapX, eventMapY)) {
							AL0450_i_TargetMapX = (L0445_ps_ActiveGroup->_targetMapX = _vm->_dungeonMan->_g306_partyMapX);
							AL0451_i_TargetMapY = (L0445_ps_ActiveGroup->_targetMapY = _vm->_dungeonMan->_g307_partyMapY);
						} else {
							if (!(--(L0445_ps_ActiveGroup->_delayFleeingFromTarget))) { /* If the creature is not afraid anymore then stop fleeing from target */
T0209096_SetBehavior0_Wander:
								L0453_B_NewGroupDirectionFound = false;
								L0444_ps_Group->setBehaviour(k0_behavior_WANDER);
								goto T0209073_SetDirectionGroup;
							}
							if (_vm->getRandomNumber(2)) {
								/* If the creature cannot move and the party is adjacent then stop fleeing */
								if (!f203_getFirstPossibleMovementDirOrdinal(&L0448_s_CreatureInfo, eventMapX, eventMapY, false)) {
									if (M38_distance(eventMapX, eventMapY, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY) <= 1)
										goto T0209096_SetBehavior0_Wander;
								}
								/* Set creature target to the home square where the creature was located when the party entered the map */
								AL0450_i_TargetMapX = L0445_ps_ActiveGroup->_homeMapX;
								AL0451_i_TargetMapY = L0445_ps_ActiveGroup->_homeMapY;
								goto T0209084_SingleSquareMoveTowardParty;
							}
							AL0450_i_TargetMapX = L0445_ps_ActiveGroup->_targetMapX;
							AL0451_i_TargetMapY = L0445_ps_ActiveGroup->_targetMapY;
						}
						/* Try and flee from the party (opposite direction) */
						L0454_i_PrimaryDirectionToOrFromParty = returnOppositeDir((direction)f228_getDirsWhereDestIsVisibleFromSource(eventMapX, eventMapY, AL0450_i_TargetMapX, AL0451_i_TargetMapY));
						_vm->_projexpl->_g363_secondaryDirToOrFromParty = returnOppositeDir((direction)_vm->_projexpl->_g363_secondaryDirToOrFromParty);
						L0461_i_MovementTicks -= (L0461_i_MovementTicks >> 2);
						goto T0209085_SingleSquareMove;
					}
				}
			}
		} else { /* Process events 38 to 41, Update Creature Behavior */
			if (AL0447_i_Behavior == k5_behavior_FLEE) {
				if (L0460_ui_CreatureCount) {
					f182_stopAttacking(L0445_ps_ActiveGroup, eventMapX, eventMapY);
				}
				goto T0209094_FleeFromTarget;
			}
			/* If the creature is attacking, then compute the next aspect update time and the next attack time */
			if (getFlag(L0445_ps_ActiveGroup->_aspect[AL0447_i_CreatureIndex = eventType - k38_TMEventTypeUpdateBehaviour_0], k0x0080_MaskActiveGroupIsAttacking)) {
				L0464_l_NextAspectUpdateTime = f179_getCreatureAspectUpdateTime(L0445_ps_ActiveGroup, AL0447_i_CreatureIndex, false);
				L0465_s_NextEvent._mapTime += ((AL0447_i_Ticks = L0448_s_CreatureInfo._attackTicks) + _vm->getRandomNumber(4) - 1);
				if (AL0447_i_Ticks > 15) {
					L0465_s_NextEvent._mapTime += _vm->getRandomNumber(8) - 2;
				}
			} else { /* If the creature is not attacking, then try attacking if possible */
				if (AL0447_i_CreatureIndex > L0460_ui_CreatureCount) { /* Ignore event if it is for a creature that is not in the group */
					goto T0209139_Return;
				}
				L0454_i_PrimaryDirectionToOrFromParty = _g382_currGroupPrimaryDirToParty;
				/* If the party is visible, update the target coordinates */
				if (L0452_i_DistanceToVisibleParty = f200_groupGetDistanceToVisibleParty(L0444_ps_Group, AL0447_i_CreatureIndex, eventMapX, eventMapY)) {
					L0445_ps_ActiveGroup->_targetMapX = _vm->_dungeonMan->_g306_partyMapX;
					L0445_ps_ActiveGroup->_targetMapY = _vm->_dungeonMan->_g307_partyMapY;
				}
				/* If there is a single creature in the group that is not full square sized and 1/4 chance */
				if (!L0460_ui_CreatureCount && (L0459_i_CreatureSize != k2_MaskCreatureSizeFull) && !((AL0446_i_GroupCellsCriteria = _vm->getRandomNumber(65536)) & 0x00C0)) {
					if (L0445_ps_ActiveGroup->_cells != k255_CreatureTypeSingleCenteredCreature) {
						/* If the creature is not already on the center of the square then change its cell */
						if (AL0446_i_GroupCellsCriteria & 0x0038) { /* 7/8 chances of changing cell to the center of the square */
							L0445_ps_ActiveGroup->_cells = k255_CreatureTypeSingleCenteredCreature;
						} else { /* 1/8 chance of changing cell to the next or previous cell on the square */
							AL0446_i_GroupCellsCriteria = M21_normalizeModulo4(M21_normalizeModulo4(L0445_ps_ActiveGroup->_cells) + ((AL0446_i_GroupCellsCriteria & 0x0001) ? 1 : -1));
						}
					}
					/* If 1/8 chance and the creature is not adjacent to the party and is a quarter square sized creature then process projectile impacts and update the creature cell if still alive. When the creature is not in front of the party, it has 7/8 chances of dodging a projectile by moving to another cell or staying in the center of the square */
					if (!(AL0446_i_GroupCellsCriteria & 0x0038) && (L0452_i_DistanceToVisibleParty != 1) && (L0459_i_CreatureSize == k0_MaskCreatureSizeQuarter)) {
						if (_vm->_projexpl->f218_projectileGetImpactCount(kM1_CreatureElemType, eventMapX, eventMapY, L0445_ps_ActiveGroup->_cells) && (_vm->_projexpl->_g364_creatureDamageOutcome == k2_outcomeKilledAllCreaturesInGroup)) /* This call to F0218_PROJECTILE_GetImpactCount works fine because there is a single creature in the group so L0445_ps_ActiveGroup->Cells contains only one cell index */
							goto T0209139_Return;
						L0445_ps_ActiveGroup->_cells = M21_normalizeModulo4(AL0446_i_GroupCellsCriteria);
					}
				}
				/* If the creature can see the party and is looking in the party direction or can attack in all direction */
				if (L0452_i_DistanceToVisibleParty &&
					(getFlag(L0448_s_CreatureInfo._attributes, k0x0004_MaskCreatureInfo_sideAttack) ||
					 _vm->_groupMan->M50_getCreatureValue(L0445_ps_ActiveGroup->_directions, AL0447_i_CreatureIndex) == L0454_i_PrimaryDirectionToOrFromParty)) {
					/* If the creature is in range to attack the party and random test succeeds */
					if ((L0452_i_DistanceToVisibleParty <= (AL0446_i_Range = L0448_s_CreatureInfo.M56_getAttackRange())) &&
						(!AL0450_i_DistanceXToParty || !AL0451_i_DistanceYToParty) &&
						(AL0446_i_Range <= (_vm->getRandomNumber(16) + 1))) {
						if ((AL0446_i_Range == 1) &&
							(!getFlag(AL0446_i_CreatureAttributes = L0448_s_CreatureInfo._attributes, k0x0008_MaskCreatureInfo_preferBackRow) || !_vm->getRandomNumber(4) || !getFlag(AL0446_i_CreatureAttributes, k0x0010_MaskCreatureInfo_attackAnyChamp)) &&
							(L0459_i_CreatureSize == k0_MaskCreatureSizeQuarter) &&
							(L0445_ps_ActiveGroup->_cells != k255_CreatureTypeSingleCenteredCreature) &&
							((AL0446_i_Cell = _vm->_groupMan->M50_getCreatureValue(L0445_ps_ActiveGroup->_cells, AL0447_i_CreatureIndex)) != L0454_i_PrimaryDirectionToOrFromParty) &&
							(AL0446_i_Cell != returnNextVal(L0454_i_PrimaryDirectionToOrFromParty))) { /* If the creature cannot cast spells (range = 1) and is not on a cell where it can attack the party directly and is a quarter square sized creature not in the center of the square then the creature moves to another cell and attack does not occur immediately */
							if (!L0460_ui_CreatureCount && _vm->getRandomNumber(2)) {
								L0445_ps_ActiveGroup->_cells = k255_CreatureTypeSingleCenteredCreature;
							} else {
								if ((L0454_i_PrimaryDirectionToOrFromParty & 0x0001) == (AL0446_i_Cell & 0x0001)) {
									AL0446_i_Cell--;
								} else {
									AL0446_i_Cell++;
								}
								if (!_vm->_groupMan->f176_getCreatureOrdinalInCell(L0444_ps_Group, AL0446_i_Cell = M21_normalizeModulo4(AL0446_i_Cell)) ||
									(_vm->getRandomNumber(2) && !_vm->_groupMan->f176_getCreatureOrdinalInCell(L0444_ps_Group, AL0446_i_Cell = returnOppositeDir((direction)AL0446_i_Cell)))) { /* If the selected cell (or the opposite cell) is not already occupied by a creature */
									if (_vm->_projexpl->f218_projectileGetImpactCount(kM1_CreatureElemType, eventMapX, eventMapY, L0445_ps_ActiveGroup->_cells) && (_vm->_projexpl->_g364_creatureDamageOutcome == k2_outcomeKilledAllCreaturesInGroup)) /* BUG0_70 A projectile impact on a creature may be ignored. The function F0218_PROJECTILE_GetImpactCount to detect projectile impacts when a quarter square sized creature moves inside a group (to another cell on the same square) may fail if there are several creatures in the group because the function expects a single cell index for its last parameter. The function should be called once for each cell where there is a creature */
										goto T0209139_Return;
									if (_vm->_projexpl->_g364_creatureDamageOutcome != k1_outcomeKilledSomeCreaturesInGroup) {
										L0445_ps_ActiveGroup->_cells = f178_getGroupValueUpdatedWithCreatureValue(L0445_ps_ActiveGroup->_cells, AL0447_i_CreatureIndex, AL0446_i_Cell);
									}
								}
							}
							L0465_s_NextEvent._mapTime += MAX(1, (L0448_s_CreatureInfo._movementTicks >> 1) + _vm->getRandomNumber(2)); /* Time for the creature to change cell */
							L0465_s_NextEvent._type = eventType;
							goto T0209135;
						}
						L0464_l_NextAspectUpdateTime = f179_getCreatureAspectUpdateTime(L0445_ps_ActiveGroup, AL0447_i_CreatureIndex, f207_isCreatureAttacking(L0444_ps_Group, eventMapX, eventMapY, AL0447_i_CreatureIndex));
						L0465_s_NextEvent._mapTime += (L0448_s_CreatureInfo._animationTicks & 0xF) + _vm->getRandomNumber(2);
					} else {
						L0444_ps_Group->setBehaviour(k7_behavior_APPROACH);
						if (L0460_ui_CreatureCount) {
							f182_stopAttacking(L0445_ps_ActiveGroup, eventMapX, eventMapY);
						}
						goto T0209081_RunTowardParty;
					}
				} else {
					/* If the party is visible, update target coordinates */
					if (f200_groupGetDistanceToVisibleParty(L0444_ps_Group, kM1_wholeCreatureGroup, eventMapX, eventMapY)) {
						L0445_ps_ActiveGroup->_targetMapX = _vm->_dungeonMan->_g306_partyMapX;
						L0445_ps_ActiveGroup->_targetMapY = _vm->_dungeonMan->_g307_partyMapY;
						f205_setDirection(L0445_ps_ActiveGroup, L0454_i_PrimaryDirectionToOrFromParty, AL0447_i_CreatureIndex, L0460_ui_CreatureCount && (L0459_i_CreatureSize == k1_MaskCreatureSizeHalf));
						L0465_s_NextEvent._mapTime += 2;
						L0464_l_NextAspectUpdateTime = M30_time(L0465_s_NextEvent._mapTime);
					} else { /* If the party is not visible, move to the target (last known party location) */
						L0444_ps_Group->setBehaviour(k7_behavior_APPROACH);
						if (L0460_ui_CreatureCount) {
							f182_stopAttacking(L0445_ps_ActiveGroup, eventMapX, eventMapY);
						}
						goto T0209082_WalkTowardTarget;
					}
				}
			}
			L0465_s_NextEvent._type = eventType;
			goto T0209136;
		}
		L0465_s_NextEvent._mapTime += MAX(1, _vm->getRandomNumber(4) + L0461_i_MovementTicks - 1);
T0209134_SetEvent37:
		L0465_s_NextEvent._type = k37_TMEventTypeUpdateBehaviourGroup;
	}
T0209135:
	if (!L0464_l_NextAspectUpdateTime) {
		L0464_l_NextAspectUpdateTime = f179_getCreatureAspectUpdateTime(L0445_ps_ActiveGroup, kM1_wholeCreatureGroup, false);
	}
T0209136:
	if (L0455_B_CurrentEventTypeIsNotUpdateBehavior) {
		L0465_s_NextEvent._mapTime += ticks;
	} else {
		L0464_l_NextAspectUpdateTime += ticks;
	}
	f208_groupAddEvent(&L0465_s_NextEvent, L0464_l_NextAspectUpdateTime);
T0209139_Return:
	;
}

bool GroupMan::f202_isMovementPossible(CreatureInfo *creatureInfo, int16 mapX, int16 mapY, uint16 dir, bool allowMovementOverImaginaryPitsAndFakeWalls) {
	int16 L0428_i_MapX;
	int16 L0429_i_MapY;
	uint16 L0430_ui_Square = 0;
	int16 L0431_i_SquareType = 0;
	Teleporter *L0432_ps_Teleporter;
	Thing L0433_T_Thing;


	_g384_groupMovementTestedDirections[dir] = true;
	_g388_groupMovementBlockedByGroupThing = Thing::_endOfList;
	_g389_groupMovementBlockedByDoor = false;
	_g390_groupMovementBlockedByParty = false;
	if (creatureInfo->_movementTicks == k255_immobile) {
		return false;
	}
	_vm->_dungeonMan->f150_mapCoordsAfterRelMovement((direction)dir, 1, 0, mapX, mapY);
	L0428_i_MapX = mapX;
	L0429_i_MapY = mapY;
	if (_g387_groupMovBlockedByWallStairsPitFakeWalFluxCageTeleporter =
		!(((L0428_i_MapX >= 0) && (L0428_i_MapX < _vm->_dungeonMan->_g273_currMapWidth)) &&
		((L0429_i_MapY >= 0) && (L0429_i_MapY < _vm->_dungeonMan->_g274_currMapHeight)) &&
		  ((L0431_i_SquareType = Square(L0430_ui_Square = _vm->_dungeonMan->_g271_currMapData[L0428_i_MapX][L0429_i_MapY]).getType()) != k0_ElementTypeWall) &&
		  (L0431_i_SquareType != k3_ElementTypeStairs) &&
		  ((L0431_i_SquareType != k2_ElementTypePit) || (getFlag(L0430_ui_Square, k0x0001_PitImaginary) && allowMovementOverImaginaryPitsAndFakeWalls) || !getFlag(L0430_ui_Square, k0x0008_PitOpen) || getFlag(creatureInfo->_attributes, k0x0020_MaskCreatureInfo_levitation)) &&
		  ((L0431_i_SquareType != k6_ElementTypeFakeWall) || getFlag(L0430_ui_Square, k0x0004_FakeWallOpen) || (getFlag(L0430_ui_Square, k0x0001_FakeWallImaginary) && allowMovementOverImaginaryPitsAndFakeWalls)))) {
		return false;
	}
	if (getFlag(creatureInfo->_attributes, k0x2000_MaskCreatureInfo_archenemy)) {
		L0433_T_Thing = _vm->_dungeonMan->f161_getSquareFirstThing(L0428_i_MapX, L0429_i_MapY);
		while (L0433_T_Thing != Thing::_endOfList) {
			if ((L0433_T_Thing).getType() == k15_ExplosionThingType) {
				L0432_ps_Teleporter = (Teleporter *)_vm->_dungeonMan->f156_getThingData(L0433_T_Thing);
				if (((Explosion *)L0432_ps_Teleporter)->setType(k50_ExplosionType_Fluxcage)) {
					_g385_fluxCages[dir] = true;
					_g386_fluxCageCount++;
					_g387_groupMovBlockedByWallStairsPitFakeWalFluxCageTeleporter = true;
					return false;
				}
			}
			L0433_T_Thing = _vm->_dungeonMan->f159_getNextThing(L0433_T_Thing);
		}
	}
	if ((L0431_i_SquareType == k5_ElementTypeTeleporter) && getFlag(L0430_ui_Square, k0x0008_TeleporterOpen) && (creatureInfo->M59_getWariness() >= 10)) {
		L0432_ps_Teleporter = (Teleporter *)_vm->_dungeonMan->f157_getSquareFirstThingData(L0428_i_MapX, L0429_i_MapY);
		if (getFlag(L0432_ps_Teleporter->getScope(), k0x0001_TelepScopeCreatures) && !_vm->_dungeonMan->f139_isCreatureAllowedOnMap(_g380_currGroupThing, L0432_ps_Teleporter->getTargetMapIndex())) {
			_g387_groupMovBlockedByWallStairsPitFakeWalFluxCageTeleporter = true;
			return false;
		}
	}
	if (_g390_groupMovementBlockedByParty = (_vm->_dungeonMan->_g272_currMapIndex == _vm->_dungeonMan->_g309_partyMapIndex) && (L0428_i_MapX == _vm->_dungeonMan->_g306_partyMapX) && (L0429_i_MapY == _vm->_dungeonMan->_g307_partyMapY)) {
		return false;
	}
	if (L0431_i_SquareType == k4_DoorElemType) {
		L0432_ps_Teleporter = (Teleporter *)_vm->_dungeonMan->f157_getSquareFirstThingData(L0428_i_MapX, L0429_i_MapY);
		if (((Square(L0430_ui_Square).getDoorState()) > (((Door *)L0432_ps_Teleporter)->opensVertically() ? CreatureInfo::M51_height(creatureInfo->_attributes) : 1)) && ((Square(L0430_ui_Square).getDoorState()) != k5_doorState_DESTROYED) && !getFlag(creatureInfo->_attributes, k0x0040_MaskCreatureInfo_nonMaterial)) {
			_g389_groupMovementBlockedByDoor = true;
			return false;
		}
	}
	return (_g388_groupMovementBlockedByGroupThing = _vm->_groupMan->f175_groupGetThing(L0428_i_MapX, L0429_i_MapY)) == Thing::_endOfList;
}

int16 GroupMan::f226_getDistanceBetweenSquares(int16 srcMapX, int16 srcMapY, int16 destMapX, int16 destMapY) {
	return ((((srcMapX -= destMapX) < 0) ? -srcMapX : srcMapX) +
		(((srcMapY -= destMapY) < 0) ? -srcMapY : srcMapY));
}

int16 GroupMan::f200_groupGetDistanceToVisibleParty(Group *group, int16 creatureIndex, int16 mapX, int16 mapY) {
	int16 L0420_i_CreatureDirection;
	int16 L0421_i_CreatureViewDirectionCount; /* Count of directions to test in L0425_ai_CreatureViewDirections */
	int16 L0422_i_Multiple;
#define AL0422_i_Counter    L0422_i_Multiple
#define AL0422_i_SightRange L0422_i_Multiple
	uint16 L0423_ui_GroupDirections;
	CreatureInfo *L0424_ps_CreatureInfo;
	int16 L0425_ai_CreatureViewDirections[4]; /* List of directions to test */


	L0424_ps_CreatureInfo = &g243_CreatureInfo[group->_type];
	if (_vm->_championMan->_g407_party._event71Count_Invisibility && !getFlag(L0424_ps_CreatureInfo->_attributes, k0x0800_MaskCreatureInfo_seeInvisible)) {
		return 0;
	}
	if (getFlag(L0424_ps_CreatureInfo->_attributes, k0x0004_MaskCreatureInfo_sideAttack)) /* If creature can see in all directions */
		goto T0200011;
	L0423_ui_GroupDirections = _vm->_groupMan->_g375_activeGroups[group->getActiveGroupIndex()]._directions;
	if (creatureIndex < 0) { /* Negative index means test if each creature in the group can see the party in their respective direction */
		L0421_i_CreatureViewDirectionCount = 0;
		for (creatureIndex = group->getCount(); creatureIndex >= 0; creatureIndex--) {
			L0420_i_CreatureDirection = M21_normalizeModulo4(L0423_ui_GroupDirections >> (creatureIndex << 1));
			AL0422_i_Counter = L0421_i_CreatureViewDirectionCount;
			while (AL0422_i_Counter--) {
				if (L0425_ai_CreatureViewDirections[AL0422_i_Counter] == L0420_i_CreatureDirection) /* If the creature looks in the same direction as another one in the group */
					goto T0200006;
			}
			L0425_ai_CreatureViewDirections[L0421_i_CreatureViewDirectionCount++] = L0420_i_CreatureDirection;
T0200006:
			;
		}
	} else { /* Positive index means test only if the specified creature in the group can see the party in its direction */
		L0425_ai_CreatureViewDirections[0] = _vm->_groupMan->M50_getCreatureValue(L0423_ui_GroupDirections, creatureIndex);
		L0421_i_CreatureViewDirectionCount = 1;
	}
	while (L0421_i_CreatureViewDirectionCount--) {
		if (f227_isDestVisibleFromSource(L0425_ai_CreatureViewDirections[L0421_i_CreatureViewDirectionCount], mapX, mapY, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY)) {
T0200011:
			AL0422_i_SightRange = L0424_ps_CreatureInfo->M54_getSightRange();
			if (!getFlag(L0424_ps_CreatureInfo->_attributes, k0x1000_MaskCreatureInfo_nightVision)) {
				AL0422_i_SightRange -= _vm->_displayMan->_g304_dungeonViewPaletteIndex >> 1;
			}
			if (_g381_currGroupDistanceToParty > MAX((int16)1, AL0422_i_SightRange)) {
				return 0;
			}
			return f199_getDistanceBetweenUnblockedSquares(mapX, mapY, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, &GroupMan::f197_isViewPartyBlocked);
		}
	}
	return 0;
}

int16 GroupMan::f199_getDistanceBetweenUnblockedSquares(int16 srcMapX, int16 srcMapY,
														int16 destMapX, int16 destMapY, bool (GroupMan::*isBlocked)(uint16, uint16)) {
	int16 L0410_i_XAxisStep;
	int16 L0411_i_YAxisStep;
	int16 L0412_i_Multiple;
#define AL0412_i_DistanceX L0412_i_Multiple
#define AL0412_i_PathMapX  L0412_i_Multiple
	int16 L0413_i_Multiple;
#define AL0413_i_DistanceY L0413_i_Multiple
#define AL0413_i_PathMapY  L0413_i_Multiple
	int16 L0414_i_LargestAxisDistance;
	bool L0415_B_DistanceXSmallerThanDistanceY;
	int16 L0416_i_ValueA;
	int16 L0417_i_ValueB;
	bool L0418_B_DistanceXEqualsDistanceY;
	int16 L0419_i_ValueC;


	if (M38_distance(srcMapX, srcMapY, destMapX, destMapY) <= 1) {
		return 1;
	}
	L0415_B_DistanceXSmallerThanDistanceY = (AL0412_i_DistanceX = ((AL0412_i_DistanceX = destMapX - srcMapX) < 0) ? -AL0412_i_DistanceX : AL0412_i_DistanceX) < (AL0413_i_DistanceY = ((AL0413_i_DistanceY = destMapY - srcMapY) < 0) ? -AL0413_i_DistanceY : AL0413_i_DistanceY);
	L0418_B_DistanceXEqualsDistanceY = (AL0412_i_DistanceX == AL0413_i_DistanceY);
	L0410_i_XAxisStep = (((AL0412_i_PathMapX = destMapX) - srcMapX) > 0) ? -1 : 1;
	L0411_i_YAxisStep = (((AL0413_i_PathMapY = destMapY) - srcMapY) > 0) ? -1 : 1;
	L0419_i_ValueC = L0415_B_DistanceXSmallerThanDistanceY ? ((L0414_i_LargestAxisDistance = AL0413_i_PathMapY - srcMapY) ? ((AL0412_i_PathMapX - srcMapX) << 6) / L0414_i_LargestAxisDistance : 128)
		: ((L0414_i_LargestAxisDistance = AL0412_i_PathMapX - srcMapX) ? ((AL0413_i_PathMapY - srcMapY) << 6) / L0414_i_LargestAxisDistance : 128);
	/* 128 when the creature is on the same row or column as the party */
	do {
		if (L0418_B_DistanceXEqualsDistanceY) {
			if (((CALL_MEMBER_FN(*_vm->_groupMan, isBlocked))(AL0412_i_PathMapX + L0410_i_XAxisStep, AL0413_i_PathMapY) && (CALL_MEMBER_FN(*_vm->_groupMan, isBlocked))(AL0412_i_PathMapX, AL0413_i_PathMapY + L0411_i_YAxisStep)) || (CALL_MEMBER_FN(*_vm->_groupMan, isBlocked))(AL0412_i_PathMapX = AL0412_i_PathMapX + L0410_i_XAxisStep, AL0413_i_PathMapY = AL0413_i_PathMapY + L0411_i_YAxisStep)) {
				return 0;
			}
		} else {
			if ((L0416_i_ValueA = ((L0414_i_LargestAxisDistance = (L0415_B_DistanceXSmallerThanDistanceY ? ((L0414_i_LargestAxisDistance = AL0413_i_PathMapY - srcMapY) ? ((AL0412_i_PathMapX + L0410_i_XAxisStep - srcMapX) << 6) / L0414_i_LargestAxisDistance : 128) : ((L0414_i_LargestAxisDistance = AL0412_i_PathMapX + L0410_i_XAxisStep - srcMapX) ? ((AL0413_i_PathMapY - srcMapY) << 6) / L0414_i_LargestAxisDistance : 128)) - L0419_i_ValueC) < 0) ? -L0414_i_LargestAxisDistance : L0414_i_LargestAxisDistance) < (L0417_i_ValueB = ((L0414_i_LargestAxisDistance = (L0415_B_DistanceXSmallerThanDistanceY ? ((L0414_i_LargestAxisDistance = AL0413_i_PathMapY + L0411_i_YAxisStep - srcMapY) ? ((AL0412_i_PathMapX - srcMapX) << 6) / L0414_i_LargestAxisDistance : 128) : ((L0414_i_LargestAxisDistance = AL0412_i_PathMapX - srcMapX) ? ((AL0413_i_PathMapY + L0411_i_YAxisStep - srcMapY) << 6) / L0414_i_LargestAxisDistance : 128)) - L0419_i_ValueC) < 0) ? -L0414_i_LargestAxisDistance : L0414_i_LargestAxisDistance)) {
				AL0412_i_PathMapX += L0410_i_XAxisStep;
			} else {
				AL0413_i_PathMapY += L0411_i_YAxisStep;
			}
			if ((CALL_MEMBER_FN(*_vm->_groupMan, isBlocked))(AL0412_i_PathMapX, AL0413_i_PathMapY) && ((L0416_i_ValueA != L0417_i_ValueB) || (CALL_MEMBER_FN(*_vm->_groupMan, isBlocked))(AL0412_i_PathMapX = AL0412_i_PathMapX + L0410_i_XAxisStep, AL0413_i_PathMapY = AL0413_i_PathMapY - L0411_i_YAxisStep))) {
				return 0;
			}
		}
	} while (M38_distance(AL0412_i_PathMapX, AL0413_i_PathMapY, srcMapX, srcMapY) > 1);
	return f226_getDistanceBetweenSquares(srcMapX, srcMapY, destMapX, destMapY);
}

bool GroupMan::f197_isViewPartyBlocked(uint16 mapX, uint16 mapY) {
	uint16 L0404_ui_Square = _vm->_dungeonMan->_g271_currMapData[mapX][mapY];
	int16 L0405_i_SquareType = Square(L0404_ui_Square).getType();
	if (L0405_i_SquareType == k4_DoorElemType) {
		Door *L0407_ps_Door = (Door *)_vm->_dungeonMan->f157_getSquareFirstThingData(mapX, mapY);
		int16 L0406_i_DoorState = Square(L0404_ui_Square).getDoorState();
		return ((L0406_i_DoorState == k3_doorState_FOURTH) || (L0406_i_DoorState == k4_doorState_CLOSED)) && !getFlag(_vm->_dungeonMan->_g275_currMapDoorInfo[L0407_ps_Door->getType()]._attributes, k0x0001_MaskDoorInfo_CraturesCanSeeThrough);
	}
	return (L0405_i_SquareType == k0_ElementTypeWall) || ((L0405_i_SquareType == k6_ElementTypeFakeWall) && !getFlag(L0404_ui_Square, k0x0004_FakeWallOpen));
}

int32 GroupMan::f179_getCreatureAspectUpdateTime(ActiveGroup *activeGroup, int16 creatureIndex, bool isAttacking) {
	uint16 L0326_ui_Multiple;
#define AL0326_ui_Aspect         L0326_ui_Multiple
#define AL0326_ui_AnimationTicks L0326_ui_Multiple
	uint16 L0327_ui_CreatureGraphicInfo;
	int16 L0328_i_Offset;
	Group *L0329_ps_Group;
	bool L0330_B_ProcessGroup;
	uint16 L0331_ui_CreatureType;
	uint16 L1635_ui_SoundIndex;

	L0329_ps_Group = &(((Group *)_vm->_dungeonMan->_g284_thingData[k4_GroupThingType])[activeGroup->_groupThingIndex]);
	L0327_ui_CreatureGraphicInfo = g243_CreatureInfo[L0331_ui_CreatureType = L0329_ps_Group->_type]._graphicInfo;
	if (L0330_B_ProcessGroup = (creatureIndex < 0)) { /* If the creature index is negative then all creatures in the group are processed */
		creatureIndex = L0329_ps_Group->getCount();
	}
	do {
		AL0326_ui_Aspect = activeGroup->_aspect[creatureIndex];
		AL0326_ui_Aspect &= k0x0080_MaskActiveGroupIsAttacking | k0x0040_MaskActiveGroupFlipBitmap;
		if (L0328_i_Offset = ((L0327_ui_CreatureGraphicInfo >> 12) & 0x3)) {
			L0328_i_Offset = _vm->getRandomNumber(L0328_i_Offset);
			if (_vm->getRandomNumber(2)) {
				L0328_i_Offset = (-L0328_i_Offset) & 0x0007;
			}
			AL0326_ui_Aspect |= L0328_i_Offset;
		}
		if (L0328_i_Offset = ((L0327_ui_CreatureGraphicInfo >> 14) & 0x3)) {
			L0328_i_Offset = _vm->getRandomNumber(L0328_i_Offset);
			if (_vm->getRandomNumber(2)) {
				L0328_i_Offset = (-L0328_i_Offset) & 0x0007;
			}
			AL0326_ui_Aspect |= (L0328_i_Offset << 3);
		}
		if (isAttacking) {
			if (getFlag(L0327_ui_CreatureGraphicInfo, k0x0200_CreatureInfoGraphicMaskFlipAttack)) {
				if (getFlag(AL0326_ui_Aspect, k0x0080_MaskActiveGroupIsAttacking) && (L0331_ui_CreatureType == k18_CreatureTypeAnimatedArmourDethKnight)) {
					if (_vm->getRandomNumber(2)) {
						toggleFlag(AL0326_ui_Aspect, k0x0040_MaskActiveGroupFlipBitmap);
						_vm->f064_SOUND_RequestPlay_CPSD(k16_soundCOMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT, _g378_currentGroupMapX, _g379_currentGroupMapY, k1_soundModePlayIfPrioritized);
					}
				} else {
					if (!getFlag(AL0326_ui_Aspect, k0x0080_MaskActiveGroupIsAttacking) || !getFlag(L0327_ui_CreatureGraphicInfo, k0x0400_CreatureInfoGraphicMaskFlipDuringAttack)) {
						if (_vm->getRandomNumber(2)) {
							setFlag(AL0326_ui_Aspect, k0x0040_MaskActiveGroupFlipBitmap);
						} else {
							clearFlag(AL0326_ui_Aspect, k0x0040_MaskActiveGroupFlipBitmap);
						}
					}
				}
			} else {
				clearFlag(AL0326_ui_Aspect, k0x0040_MaskActiveGroupFlipBitmap);
			}
			setFlag(AL0326_ui_Aspect, k0x0080_MaskActiveGroupIsAttacking);
		} else {
			if (getFlag(L0327_ui_CreatureGraphicInfo, k0x0004_CreatureInfoGraphicMaskFlipNonAttack)) {
				if (L0331_ui_CreatureType == k13_CreatureTypeCouatl) {
					if (_vm->getRandomNumber(2)) {
						toggleFlag(AL0326_ui_Aspect, k0x0040_MaskActiveGroupFlipBitmap);
						L1635_ui_SoundIndex = _vm->_movsens->f514_getSound(k13_CreatureTypeCouatl);
						if (L1635_ui_SoundIndex <= k34_D13_soundCount) {
							_vm->f064_SOUND_RequestPlay_CPSD(L1635_ui_SoundIndex, _g378_currentGroupMapX, _g379_currentGroupMapY, k1_soundModePlayIfPrioritized);
						}
					}
				} else {
					if (_vm->getRandomNumber(2)) {
						setFlag(AL0326_ui_Aspect, k0x0040_MaskActiveGroupFlipBitmap);
					} else {
						clearFlag(AL0326_ui_Aspect, k0x0040_MaskActiveGroupFlipBitmap);
					}
				}
			} else {
				clearFlag(AL0326_ui_Aspect, k0x0040_MaskActiveGroupFlipBitmap);
			}
			clearFlag(AL0326_ui_Aspect, k0x0080_MaskActiveGroupIsAttacking);
		}
		activeGroup->_aspect[creatureIndex] = AL0326_ui_Aspect;
	} while (L0330_B_ProcessGroup && (creatureIndex--));
	AL0326_ui_AnimationTicks = g243_CreatureInfo[L0329_ps_Group->_type]._animationTicks;
	return _vm->_g313_gameTime + (isAttacking ? ((AL0326_ui_AnimationTicks >> 8) & 0xF) : ((AL0326_ui_AnimationTicks >> 4) & 0xF)) + _vm->getRandomNumber(2);
}

void GroupMan::f205_setDirection(ActiveGroup *activeGroup, int16 dir, int16 creatureIndex, bool twoHalfSquareSizedCreatures) {
	uint16 L0435_ui_GroupDirections;
	static long G0395_l_TwoHalfSquareSizedCreaturesGroupLastDirectionSetTime; /* These two variables are used to prevent setting direction of half square sized creatures twice at the same game time */
	static ActiveGroup *G0396_ps_TwoHalfSquareSizedCreaturesGroupLastDirectionSetActiveGroup;


	warning(false, "potentially dangerous cast to uint32 below");
	if (twoHalfSquareSizedCreatures && (_vm->_g313_gameTime == (uint32)G0395_l_TwoHalfSquareSizedCreaturesGroupLastDirectionSetTime) && (activeGroup == G0396_ps_TwoHalfSquareSizedCreaturesGroupLastDirectionSetActiveGroup)) {
		return;
	}
	if (M21_normalizeModulo4(_vm->_groupMan->M50_getCreatureValue(L0435_ui_GroupDirections = activeGroup->_directions, creatureIndex) - dir) == 2) { /* If current and new direction are opposites then change direction only one step at a time */
		L0435_ui_GroupDirections = f178_getGroupValueUpdatedWithCreatureValue(L0435_ui_GroupDirections, creatureIndex, dir = returnNextVal((_vm->getRandomNumber(65536) & 0x0002) + dir));
	} else {
		L0435_ui_GroupDirections = f178_getGroupValueUpdatedWithCreatureValue(L0435_ui_GroupDirections, creatureIndex, dir);
	}
	if (twoHalfSquareSizedCreatures) {
		L0435_ui_GroupDirections = f178_getGroupValueUpdatedWithCreatureValue(L0435_ui_GroupDirections, creatureIndex ^ 1, dir); /* Set direction of the second half square sized creature */
		G0395_l_TwoHalfSquareSizedCreaturesGroupLastDirectionSetTime = _vm->_g313_gameTime;
		G0396_ps_TwoHalfSquareSizedCreaturesGroupLastDirectionSetActiveGroup = activeGroup;
	}
	activeGroup->_directions = (direction)L0435_ui_GroupDirections;
}

void GroupMan::f208_groupAddEvent(TimelineEvent *event, uint32 time) {
	warning(false, "potentially dangerous cast to uint32 below");
	if (time < (uint32)M30_time(event->_mapTime)) {
		event->_type -= 5;
		event->_C._ticks = M30_time(event->_mapTime) - time;
		M32_setTime(event->_mapTime, time);
	} else {
		event->_C._ticks = time - M30_time(event->_mapTime);
	}
	_vm->_timeline->f238_addEventGetEventIndex(event);
}

int16 GroupMan::f201_getSmelledPartyPrimaryDirOrdinal(CreatureInfo *creatureInfo, int16 mapY, int16 mapX) {

	uint16 L0426_ui_SmellRange;
	int16 L0427_i_ScentOrdinal;


	if (!(L0426_ui_SmellRange = creatureInfo->M55_getSmellRange())) {
		return 0;
	}
	if ((((L0426_ui_SmellRange + 1) >> 1) >= _g381_currGroupDistanceToParty) && f199_getDistanceBetweenUnblockedSquares(mapY, mapX, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, &GroupMan::f198_isSmellPartyBlocked)) {
		_vm->_projexpl->_g363_secondaryDirToOrFromParty = _g383_currGroupSecondaryDirToParty;
		return _vm->M0_indexToOrdinal(_g382_currGroupPrimaryDirToParty);
	}
	if ((L0427_i_ScentOrdinal = _vm->_championMan->f315_getScentOrdinal(mapY, mapX)) && ((_vm->_championMan->_g407_party._scentStrengths[_vm->M1_ordinalToIndex(L0427_i_ScentOrdinal)] + _vm->getRandomNumber(4)) > (30 - (L0426_ui_SmellRange << 1)))) { /* If there is a fresh enough party scent on the group square */
		return _vm->M0_indexToOrdinal(f228_getDirsWhereDestIsVisibleFromSource(mapY, mapX, _vm->_championMan->_g407_party._scents[L0427_i_ScentOrdinal].getMapX(), _vm->_championMan->_g407_party._scents[L0427_i_ScentOrdinal].getMapY()));
	}
	return 0;
}

bool GroupMan::f198_isSmellPartyBlocked(uint16 mapX, uint16 mapY) {
	uint16 L0408_ui_Square;
	int16 L0409_i_SquareType;

	return ((L0409_i_SquareType = Square(L0408_ui_Square = _vm->_dungeonMan->_g271_currMapData[mapX][mapY]).getType()) == k0_ElementTypeWall) || ((L0409_i_SquareType == k6_ElementTypeFakeWall) && !getFlag(L0408_ui_Square, k0x0004_FakeWallOpen));
}

int16 GroupMan::f203_getFirstPossibleMovementDirOrdinal(CreatureInfo *info, int16 mapX, int16 mapY, bool allowMovementOverImaginaryPitsAndFakeWalls) {
	int16 L0434_i_Direction;


	for (L0434_i_Direction = kDirNorth; L0434_i_Direction <= kDirWest; L0434_i_Direction++) {
		if ((!_g384_groupMovementTestedDirections[L0434_i_Direction]) && f202_isMovementPossible(info, mapX, mapY, L0434_i_Direction, allowMovementOverImaginaryPitsAndFakeWalls)) {
			return _vm->M0_indexToOrdinal(L0434_i_Direction);
		}
	}
	return 0;
}

void GroupMan::f206_groupSetDirGroup(ActiveGroup *activeGroup, int16 dir, int16 creatureIndex, int16 creatureSize) {
	bool L0436_B_TwoHalfSquareSizedCreatures;


	if (L0436_B_TwoHalfSquareSizedCreatures = creatureIndex && (creatureSize == k1_MaskCreatureSizeHalf)) {
		creatureIndex--;
	}
	do {
		if (!creatureIndex || _vm->getRandomNumber(2)) {
			f205_setDirection(activeGroup, dir, creatureIndex, L0436_B_TwoHalfSquareSizedCreatures);
		}
	} while (creatureIndex--);
}

void GroupMan::f182_stopAttacking(ActiveGroup *group, int16 mapX, int16 mapY) {
	int16 L0337_i_CreatureIndex;

	for (L0337_i_CreatureIndex = 0; L0337_i_CreatureIndex < 4; clearFlag(group->_aspect[L0337_i_CreatureIndex++], k0x0080_MaskActiveGroupIsAttacking));
	f181_groupDeleteEvents(mapX, mapY);

}

bool GroupMan::f204_isArchenemyDoubleMovementPossible(CreatureInfo *info, int16 mapX, int16 mapY, uint16 dir) {
	if (_g385_fluxCages[dir]) {
		return false;
	}
	mapX += _vm->_dirIntoStepCountEast[dir], mapY += _vm->_dirIntoStepCountNorth[dir];
	return f202_isMovementPossible(info, mapX, mapY, dir, false);
}

bool GroupMan::f207_isCreatureAttacking(Group *group, int16 mapX, int16 mapY, uint16 creatureIndex) {
	static const uint8 G0244_auc_Graphic559_CreatureAttackSounds[11] = { 3, 7, 14, 15, 19, 21, 29, 30, 31, 4, 16 }; /* Atari ST: { 3, 7, 14, 15, 19, 21, 4, 16 } */

	uint16 L0437_ui_Multiple;
#define AL0437_ui_CreatureType L0437_ui_Multiple
#define AL0437_T_Thing         L0437_ui_Multiple
	int16 L0439_i_Multiple;
#define AL0439_i_GroupCells    L0439_i_Multiple
#define AL0439_i_TargetCell    L0439_i_Multiple
#define AL0439_i_ChampionIndex L0439_i_Multiple
	int16 L0440_i_Multiple;
#define AL0440_i_KineticEnergy      L0440_i_Multiple
#define AL0440_i_Counter            L0440_i_Multiple
#define AL0440_i_Damage             L0440_i_Multiple
#define AL0440_i_AttackSoundOrdinal L0440_i_Multiple

	_vm->_projexpl->_g361_lastCreatureAttackTime = _vm->_g313_gameTime;
	ActiveGroup L0443_s_ActiveGroup = _vm->_groupMan->_g375_activeGroups[group->getActiveGroupIndex()];
	CreatureInfo *L0441_ps_CreatureInfo = &g243_CreatureInfo[AL0437_ui_CreatureType = group->_type];
	uint16 L0438_ui_PrimaryDirectionToParty = _g382_currGroupPrimaryDirToParty;
	if ((AL0439_i_GroupCells = L0443_s_ActiveGroup._cells) == k255_CreatureTypeSingleCenteredCreature) {
		AL0439_i_TargetCell = _vm->getRandomNumber(2);
	} else {
		AL0439_i_TargetCell = ((_vm->_groupMan->M50_getCreatureValue(AL0439_i_GroupCells, creatureIndex) + 5 - L0438_ui_PrimaryDirectionToParty) & 0x0002) >> 1;
	}
	AL0439_i_TargetCell += L0438_ui_PrimaryDirectionToParty;
	AL0439_i_TargetCell &= 0x0003;
	if ((L0441_ps_CreatureInfo->M56_getAttackRange() > 1) && ((_g381_currGroupDistanceToParty > 1) || _vm->getRandomNumber(2))) {
		switch (AL0437_ui_CreatureType) {
		case k14_CreatureTypeVexirk:
		case k23_CreatureTypeLordChaos:
			if (_vm->getRandomNumber(2)) {
				AL0437_T_Thing = Thing::_explFireBall.toUint16();
			} else {
				switch (_vm->getRandomNumber(4)) {
				case 0:
					AL0437_T_Thing = Thing::_explHarmNonMaterial.toUint16();
					break;
				case 1:
					AL0437_T_Thing = Thing::_explLightningBolt.toUint16();
					break;
				case 2:
					AL0437_T_Thing = Thing::_explPoisonCloud.toUint16();
					break;
				case 3:
					AL0437_T_Thing = Thing::_explOpenDoor.toUint16();
				}
			}
			break;
		case k1_CreatureTypeSwampSlimeSlime:
			AL0437_T_Thing = Thing::_explSlime.toUint16();
			break;
		case k3_CreatureTypeWizardEyeFlyingEye:
			if (_vm->getRandomNumber(8)) {
				AL0437_T_Thing = Thing::_explLightningBolt.toUint16();
			} else {
				AL0437_T_Thing = Thing::_explOpenDoor.toUint16();
			}
			break;
		case k19_CreatureTypeMaterializerZytaz:
			if (_vm->getRandomNumber(2)) {
				AL0437_T_Thing = Thing::_explPoisonCloud.toUint16();
				break;
			}
		case k22_CreatureTypeDemon:
		case k24_CreatureTypeRedDragon:
			AL0437_T_Thing = Thing::_explFireBall.toUint16();
		} /* BUG0_13 The game may crash when 'Lord Order' or 'Grey Lord' cast spells. This cannot happen with the original dungeons as they do not contain any groups of these types. 'Lord Order' and 'Grey Lord' creatures can cast spells (attack range > 1) but no projectile type is defined for them in the code. If these creatures are present in a dungeon they will cast projectiles containing undefined things because the variable is not initialized */
		AL0440_i_KineticEnergy = (L0441_ps_CreatureInfo->_attack >> 2) + 1;
		AL0440_i_KineticEnergy += _vm->getRandomNumber(AL0440_i_KineticEnergy);
		AL0440_i_KineticEnergy += _vm->getRandomNumber(AL0440_i_KineticEnergy);
		_vm->f064_SOUND_RequestPlay_CPSD(k13_soundSPELL, mapX, mapY, k0_soundModePlayImmediately);
		_vm->_projexpl->f212_projectileCreate(Thing(AL0437_T_Thing), mapX, mapY, AL0439_i_TargetCell, (direction)_g382_currGroupPrimaryDirToParty, f26_getBoundedValue((int16)20, AL0440_i_KineticEnergy, (int16)255), L0441_ps_CreatureInfo->_dexterity, 8);
	} else {
		if (getFlag(L0441_ps_CreatureInfo->_attributes, k0x0010_MaskCreatureInfo_attackAnyChamp)) {
			AL0439_i_ChampionIndex = _vm->getRandomNumber(4);
			for (AL0440_i_Counter = 0; (AL0440_i_Counter < 4) && !_vm->_championMan->_gK71_champions[AL0439_i_ChampionIndex]._currHealth; AL0440_i_Counter++) {
				AL0439_i_ChampionIndex = returnNextVal(AL0439_i_ChampionIndex);
			}
			if (AL0440_i_Counter == 4) {
				return false;
			}
		} else {
			if ((AL0439_i_ChampionIndex = _vm->_championMan->f286_getTargetChampionIndex(mapX, mapY, AL0439_i_TargetCell)) < 0) {
				return false;
			}
		}
		if (AL0437_ui_CreatureType == k2_CreatureTypeGiggler) {
			f193_stealFromChampion(group, AL0439_i_ChampionIndex);
		} else {
			AL0440_i_Damage = f230_getChampionDamage(group, AL0439_i_ChampionIndex) + 1;
			Champion *L0442_ps_Champion = &_vm->_championMan->_gK71_champions[AL0439_i_ChampionIndex];
			if (AL0440_i_Damage > L0442_ps_Champion->_maximumDamageReceived) {
				L0442_ps_Champion->_maximumDamageReceived = AL0440_i_Damage;
				L0442_ps_Champion->_directionMaximumDamageReceived = returnOppositeDir((direction)L0438_ui_PrimaryDirectionToParty);
			}
		}
	}
	if (AL0440_i_AttackSoundOrdinal = L0441_ps_CreatureInfo->_attackSoundOrdinal) {
		_vm->f064_SOUND_RequestPlay_CPSD(G0244_auc_Graphic559_CreatureAttackSounds[--AL0440_i_AttackSoundOrdinal], mapX, mapY, k1_soundModePlayIfPrioritized);
	}
	return true;
}

void GroupMan::f229_setOrderedCellsToAttack(signed char *orderedCellsToAttack, int16 targetMapX, int16 targetMapY, int16 attackerMapX, int16 attackerMapY, uint16 cellSource) {
	static signed char g23_orderedCellsToAttack[8][4] = { // @ G0023_aac_Graphic562_OrderedCellsToAttack
		{0, 1, 3, 2},   /* Attack South from position Northwest or Southwest */
		{1, 0, 2, 3},   /* Attack South from position Northeast or Southeast */
		{1, 2, 0, 3},   /* Attack West from position Northwest or Northeast */
		{2, 1, 3, 0},   /* Attack West from position Southeast or Southwest */
		{3, 2, 0, 1},   /* Attack North from position Northwest or Southwest */
		{2, 3, 1, 0},   /* Attack North from position Southeast or Northeast */
		{0, 3, 1, 2},   /* Attack East from position Northwest or Northeast */
		{3, 0, 2, 1}}; /* Attack East from position Southeast or Southwest */
	uint16 L0557_ui_OrderedCellsToAttackIndex;


	if (!((L0557_ui_OrderedCellsToAttackIndex = f228_getDirsWhereDestIsVisibleFromSource(targetMapX, targetMapY, attackerMapX, attackerMapY) << 1) & 0x0002)) {
		cellSource++;
	}
	L0557_ui_OrderedCellsToAttackIndex += (cellSource >> 1) & 0x0001;
	for (uint16 i = 0; i < 4; ++i)
		orderedCellsToAttack[i] = g23_orderedCellsToAttack[L0557_ui_OrderedCellsToAttackIndex][i];
}

void GroupMan::f193_stealFromChampion(Group *group, uint16 championIndex) {
	int16 L0391_i_Percentage;
	uint16 L0392_ui_StealFromSlotIndex;
	uint16 L0393_ui_Counter;
	Thing L0394_T_Thing;
	Champion *L0395_ps_Champion;
	bool L0396_B_ObjectStolen;
	static unsigned char G0394_auc_StealFromSlotIndices[8]; /* Initialized with 0 bytes by C loader */


	L0396_B_ObjectStolen = false;
	L0391_i_Percentage = 100 - _vm->_championMan->f311_getDexterity(L0395_ps_Champion = &_vm->_championMan->_gK71_champions[championIndex]);
	L0393_ui_Counter = _vm->getRandomNumber(8);
	while ((L0391_i_Percentage > 0) && !_vm->_championMan->f308_isLucky(L0395_ps_Champion, L0391_i_Percentage)) {
		if ((L0392_ui_StealFromSlotIndex = G0394_auc_StealFromSlotIndices[L0393_ui_Counter]) == k13_ChampionSlotBackpackLine_1_1) {
			L0392_ui_StealFromSlotIndex += _vm->getRandomNumber(17); /* Select a random slot in the backpack */
		}
		if (((L0394_T_Thing = L0395_ps_Champion->_slots[L0392_ui_StealFromSlotIndex]) != Thing::_none)) {
			L0396_B_ObjectStolen = true;
			L0394_T_Thing = _vm->_championMan->f300_getObjectRemovedFromSlot(championIndex, L0392_ui_StealFromSlotIndex);
			if (group->_slot == Thing::_endOfList) {
				group->_slot = L0394_T_Thing; /* BUG0_12 An object is cloned and appears at two different locations in the dungeon and/or inventory. The game may crash when interacting with this object. If a Giggler with no possessions steals an object that was previously in a chest and was not the last object in the chest then the objects that followed it are cloned. In the chest, the object is part of a linked list of objects that is not reset when the object is removed from the chest and placed in the inventory (but not in the dungeon), nor when it is stolen and added as the first Giggler possession. If the Giggler already has a possession before stealing the object then this does not create a cloned object.
											 The following statement is missing: L0394_T_Thing->Next = Thing::_endOfList;
											 This creates cloned things if L0394_T_Thing->Next is not Thing::_endOfList which is the case when the object comes from a chest in which it was not the last object */
			} else {
				_vm->_dungeonMan->f163_linkThingToList(L0394_T_Thing, group->_slot, kM1_MapXNotOnASquare, 0);
			}
			_vm->_championMan->f292_drawChampionState((ChampionIndex)championIndex);
		}
		++L0393_ui_Counter;
		L0393_ui_Counter &= 0x0007;
		L0391_i_Percentage -= 20;
	}
	if (!_vm->getRandomNumber(8) || (L0396_B_ObjectStolen && _vm->getRandomNumber(2))) {
		_vm->_groupMan->_g375_activeGroups[group->getActiveGroupIndex()]._delayFleeingFromTarget = _vm->getRandomNumber(64) + 20;
		group->setBehaviour(k5_behavior_FLEE);
	}
}

int16 GroupMan::f230_getChampionDamage(Group *group, uint16 champIndex) {
	unsigned char g24_woundProbabilityIndexToWoundMask[4] = {32, 16, 8, 4}; // @ G0024_auc_Graphic562_WoundProbabilityIndexToWoundMask

	Champion *L0562_ps_Champion;
	int16 L0558_i_Multiple;
#define AL0558_i_Attack L0558_i_Multiple
#define AL0558_i_Damage L0558_i_Multiple
	uint16 L0559_ui_Multiple;
#define AL0559_ui_WoundTest          L0559_ui_Multiple
#define AL0559_ui_PoisonAttack       L0559_ui_Multiple
#define AL0559_ui_CreatureDifficulty L0559_ui_Multiple
	uint16 L0560_ui_WoundProbabilities;
	uint16 L0561_ui_Multiple;
#define AL0561_ui_WoundProbabilityIndex L0561_ui_Multiple
#define AL0561_ui_AllowedWound          L0561_ui_Multiple
	int16 L0563_i_DoubledMapDifficulty;
	CreatureInfo L0564_s_CreatureInfo;


	L0562_ps_Champion = &_vm->_championMan->_gK71_champions[champIndex];
	if (champIndex >= _vm->_championMan->_g305_partyChampionCount) {
		return 0;
	}
	if (!L0562_ps_Champion->_currHealth) {
		return 0;
	}
	if (_vm->_championMan->_g300_partyIsSleeping) {
		_vm->_championMan->f314_wakeUp();
	}
	L0563_i_DoubledMapDifficulty = _vm->_dungeonMan->_g269_currMap->_difficulty << 1;
	L0564_s_CreatureInfo = g243_CreatureInfo[group->_type];
	_vm->_championMan->f304_addSkillExperience(champIndex, k7_ChampionSkillParry, L0564_s_CreatureInfo.M58_getExperience());
	if (_vm->_championMan->_g300_partyIsSleeping || (((_vm->_championMan->f311_getDexterity(L0562_ps_Champion) < (_vm->getRandomNumber(32) + L0564_s_CreatureInfo._dexterity + L0563_i_DoubledMapDifficulty - 16)) || !_vm->getRandomNumber(4)) && !_vm->_championMan->f308_isLucky(L0562_ps_Champion, 60))) {
		if ((AL0559_ui_WoundTest = _vm->getRandomNumber(65536)) & 0x0070) {
			AL0559_ui_WoundTest &= 0x000F;
			L0560_ui_WoundProbabilities = L0564_s_CreatureInfo._woundProbabilities;
			for (AL0561_ui_WoundProbabilityIndex = 0; AL0559_ui_WoundTest > (L0560_ui_WoundProbabilities & 0x000F); L0560_ui_WoundProbabilities >>= 4) {
				AL0561_ui_WoundProbabilityIndex++;
			}
			AL0561_ui_AllowedWound = g24_woundProbabilityIndexToWoundMask[AL0561_ui_WoundProbabilityIndex];
		} else {
			AL0561_ui_AllowedWound = AL0559_ui_WoundTest & 0x0001; /* 0 (Ready hand) or 1 (action hand) */
		}
		if ((AL0558_i_Attack = (_vm->getRandomNumber(16) + L0564_s_CreatureInfo._attack + L0563_i_DoubledMapDifficulty) - (_vm->_championMan->f303_getSkillLevel(champIndex, k7_ChampionSkillParry) << 1)) <= 1) {
			if (_vm->getRandomNumber(2)) {
				goto T0230014;
			}
			AL0558_i_Attack = _vm->getRandomNumber(4) + 2;
		}
		AL0558_i_Attack >>= 1;
		AL0558_i_Attack += _vm->getRandomNumber(AL0558_i_Attack) + _vm->getRandomNumber(4);
		AL0558_i_Attack += _vm->getRandomNumber(AL0558_i_Attack);
		AL0558_i_Attack >>= 2;
		AL0558_i_Attack += _vm->getRandomNumber(4) + 1;
		if (_vm->getRandomNumber(2)) {
			AL0558_i_Attack -= _vm->getRandomNumber((AL0558_i_Attack >> 1) + 1) - 1;
		}
		if (AL0558_i_Damage = _vm->_championMan->f321_addPendingDamageAndWounds_getDamage(champIndex, AL0558_i_Attack, AL0561_ui_AllowedWound, L0564_s_CreatureInfo._attackType)) {
			_vm->f064_SOUND_RequestPlay_CPSD(k09_soundCHAMPION_0_DAMAGED + champIndex, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, k2_soundModePlayOneTickLater);
			if ((AL0559_ui_PoisonAttack = L0564_s_CreatureInfo._poisonAttack) && _vm->getRandomNumber(2) && ((AL0559_ui_PoisonAttack = _vm->_championMan->f307_getStatisticAdjustedAttack(L0562_ps_Champion, k4_ChampionStatVitality, AL0559_ui_PoisonAttack)) >= 0)) {
				_vm->_championMan->f322_championPoison(champIndex, AL0559_ui_PoisonAttack);
			}
			return AL0558_i_Damage;
		}
	}
T0230014:
	return 0;
}

void GroupMan::f187_dropMovingCreatureFixedPossession(Thing thing, int16 mapX, int16 mapY) {
	Group *L0363_ps_Group;
	int16 L0364_i_CreatureType;


	if (_g391_dropMovingCreatureFixedPossCellCount) {
		L0363_ps_Group = (Group *)_vm->_dungeonMan->f156_getThingData(thing);
		L0364_i_CreatureType = L0363_ps_Group->_type;
		while (_g391_dropMovingCreatureFixedPossCellCount) {
			f186_dropCreatureFixedPossessions(L0364_i_CreatureType, mapX, mapY, _g392_dropMovingCreatureFixedPossessionsCell[--_g391_dropMovingCreatureFixedPossCellCount], k2_soundModePlayOneTickLater);
		}
	}
}

void GroupMan::f180_startWanedring(int16 mapX, int16 mapY) {
	Group *L0332_ps_Group;
	TimelineEvent L0333_s_Event;


	L0332_ps_Group = (Group *)_vm->_dungeonMan->f156_getThingData(_vm->_groupMan->f175_groupGetThing(mapX, mapY));
	if (L0332_ps_Group->getBehaviour() >= k4_behavior_USELESS) {
		L0332_ps_Group->setBehaviour(k0_behavior_WANDER);
	}
	M33_setMapAndTime(L0333_s_Event._mapTime, _vm->_dungeonMan->_g272_currMapIndex, (_vm->_g313_gameTime + 1));
	L0333_s_Event._type = k37_TMEventTypeUpdateBehaviourGroup;
	L0333_s_Event._priority = 255 - g243_CreatureInfo[L0332_ps_Group->_type]._movementTicks; /* The fastest creatures (with small MovementTicks value) get higher event priority */
	L0333_s_Event._C._ticks = 0;
	L0333_s_Event._B._location._mapX = mapX;
	L0333_s_Event._B._location._mapY = mapY;
	_vm->_timeline->f238_addEventGetEventIndex(&L0333_s_Event);
}

void GroupMan::f183_addActiveGroup(Thing thing, int16 mapX, int16 mapY) {
	uint16 L0339_ui_CreatureIndex;
	Group *L0340_ps_Group;
	ActiveGroup *L0341_ps_ActiveGroup;
	int16 L0344_i_ActiveGroupIndex;


	L0341_ps_ActiveGroup = _vm->_groupMan->_g375_activeGroups;
	L0344_i_ActiveGroupIndex = 0;
	while (L0341_ps_ActiveGroup->_groupThingIndex >= 0) {
		if (++L0344_i_ActiveGroupIndex >= _vm->_groupMan->_g376_maxActiveGroupCount) {
			return;
		}
		L0341_ps_ActiveGroup++;
	}
	_g377_currActiveGroupCount++;

	warning(false, "Code differs from the original in GroupMan::f183_addActiveGroup");
	//L0340_ps_Group = ((Group *)_vm->_dungeonMan->_g284_thingData[k4_GroupThingType]) + (L0341_ps_ActiveGroup->_groupThingIndex = (thing).getType());
	L0341_ps_ActiveGroup->_groupThingIndex = thing.getType();
	L0340_ps_Group = (Group*)_vm->_dungeonMan->f156_getThingData(f175_groupGetThing(mapX, mapY));

	L0341_ps_ActiveGroup->_cells = L0340_ps_Group->_cells;
	L0340_ps_Group->getActiveGroupIndex() = L0344_i_ActiveGroupIndex;
	L0341_ps_ActiveGroup->_priorMapX = L0341_ps_ActiveGroup->_homeMapX = mapX;
	L0341_ps_ActiveGroup->_priorMapY = L0341_ps_ActiveGroup->_homeMapY = mapY;
	L0341_ps_ActiveGroup->_lastMoveTime = _vm->_g313_gameTime - 127;
	L0339_ui_CreatureIndex = L0340_ps_Group->getCount();
	do {
		L0341_ps_ActiveGroup->_directions = (direction)f178_getGroupValueUpdatedWithCreatureValue(L0341_ps_ActiveGroup->_directions, L0339_ui_CreatureIndex, L0340_ps_Group->getDir());
		L0341_ps_ActiveGroup->_aspect[L0339_ui_CreatureIndex] = 0;
	} while (L0339_ui_CreatureIndex--);
	f179_getCreatureAspectUpdateTime(L0341_ps_ActiveGroup, kM1_wholeCreatureGroup, false);
}

void GroupMan::f184_removeActiveGroup(uint16 activeGroupIndex) {
	ActiveGroup *L0347_ps_ActiveGroup;
	Group *L0348_ps_Group;


	if ((activeGroupIndex > _vm->_groupMan->_g376_maxActiveGroupCount) || (_vm->_groupMan->_g375_activeGroups[activeGroupIndex]._groupThingIndex < 0)) {
		return;
	}
	L0347_ps_ActiveGroup = &_vm->_groupMan->_g375_activeGroups[activeGroupIndex];
	L0348_ps_Group = &((Group *)_vm->_dungeonMan->_g284_thingData[k4_GroupThingType])[L0347_ps_ActiveGroup->_groupThingIndex];
	_g377_currActiveGroupCount--;
	L0348_ps_Group->_cells = L0347_ps_ActiveGroup->_cells;
	L0348_ps_Group->setDir(M21_normalizeModulo4(L0347_ps_ActiveGroup->_directions));
	if (L0348_ps_Group->getBehaviour() >= k4_behavior_USELESS) {
		L0348_ps_Group->setBehaviour(k0_behavior_WANDER);
	}
	L0347_ps_ActiveGroup->_groupThingIndex = -1;
}

void GroupMan::f194_removeAllActiveGroups() {
	for (int16 L0397_ui_ActiveGroupIndex = 0; _g377_currActiveGroupCount > 0; L0397_ui_ActiveGroupIndex++) {
		if (_vm->_groupMan->_g375_activeGroups[L0397_ui_ActiveGroupIndex]._groupThingIndex >= 0) {
			f184_removeActiveGroup(L0397_ui_ActiveGroupIndex);
		}
	}
}

void GroupMan::f195_addAllActiveGroups() {
	uint16 L0398_ui_MapX;
	uint16 L0399_ui_MapY;
	Thing L0400_T_Thing;
	byte *L0401_puc_Square;
	Thing *L0402_pT_SquareFirstThing;


	L0401_puc_Square = _vm->_dungeonMan->_g271_currMapData[0];
	L0402_pT_SquareFirstThing = &_vm->_dungeonMan->_g283_squareFirstThings[_vm->_dungeonMan->_g270_currMapColCumulativeSquareFirstThingCount[0]];
	for (L0398_ui_MapX = 0; L0398_ui_MapX < _vm->_dungeonMan->_g273_currMapWidth; L0398_ui_MapX++) {
		for (L0399_ui_MapY = 0; L0399_ui_MapY < _vm->_dungeonMan->_g274_currMapHeight; L0399_ui_MapY++) {
			if (getFlag(*L0401_puc_Square++, k0x0010_ThingListPresent)) {
				L0400_T_Thing = *L0402_pT_SquareFirstThing++;
				do {
					if (L0400_T_Thing.getType() == k4_GroupThingType) {
						f181_groupDeleteEvents(L0398_ui_MapX, L0399_ui_MapY);
						f183_addActiveGroup(L0400_T_Thing, L0398_ui_MapX, L0399_ui_MapY);
						f180_startWanedring(L0398_ui_MapX, L0399_ui_MapY);
						break;
					}
				} while ((L0400_T_Thing = _vm->_dungeonMan->f159_getNextThing(L0400_T_Thing)) != Thing::_endOfList);
			}
		}
	}
}

Thing GroupMan::f185_groupGetGenerated(int16 creatureType, int16 healthMultiplier, uint16 creatureCount, direction dir, int16 mapX, int16 mapY) {
	Thing L0349_T_GroupThing;
	uint16 L0350_ui_BaseHealth;
	uint16 L0351_ui_Cell = 0;
	uint16 L0352_ui_GroupCells = 0;
	Group* L0353_ps_Group;
	CreatureInfo* L0354_ps_CreatureInfo;
	bool L0355_B_SeveralCreaturesInGroup;


	if (((_g377_currActiveGroupCount >= (_vm->_groupMan->_g376_maxActiveGroupCount - 5)) && (_vm->_dungeonMan->_g272_currMapIndex == _vm->_dungeonMan->_g309_partyMapIndex)) || ((L0349_T_GroupThing = _vm->_dungeonMan->f166_getUnusedThing(k4_GroupThingType)) == Thing::_none)) {
		return Thing::_none;
	}
	L0353_ps_Group = (Group*)_vm->_dungeonMan->f156_getThingData(L0349_T_GroupThing);
	L0353_ps_Group->_slot = Thing::_endOfList;
	L0353_ps_Group->setDoNotDiscard(false);
	L0353_ps_Group->setDir(dir);
	L0353_ps_Group->setCount(creatureCount);
	if (L0355_B_SeveralCreaturesInGroup = creatureCount) {
		L0351_ui_Cell = _vm->getRandomNumber(4);
	} else {
		L0352_ui_GroupCells = k255_CreatureTypeSingleCenteredCreature;
	}
	L0354_ps_CreatureInfo = &g243_CreatureInfo[L0353_ps_Group->_type = creatureType];
	L0350_ui_BaseHealth = L0354_ps_CreatureInfo->_baseHealth;
	do {
		L0353_ps_Group->_health[creatureCount] = (L0350_ui_BaseHealth * healthMultiplier) + _vm->getRandomNumber((L0350_ui_BaseHealth >> 2) + 1);
		if (L0355_B_SeveralCreaturesInGroup) {
			L0352_ui_GroupCells = f178_getGroupValueUpdatedWithCreatureValue(L0352_ui_GroupCells, creatureCount, L0351_ui_Cell++);
			if (getFlag(L0354_ps_CreatureInfo->_attributes, k0x0003_MaskCreatureInfo_size) == k1_MaskCreatureSizeHalf) {
				L0351_ui_Cell++;
			}
			L0351_ui_Cell &= 0x0003;
		}
	} while (creatureCount--);
	L0353_ps_Group->_cells = L0352_ui_GroupCells;
	if (_vm->_movsens->f267_getMoveResult(L0349_T_GroupThing, kM1_MapXNotOnASquare, 0, mapX, mapY)) { /* If F0267_MOVE_GetMoveResult_CPSCE returns true then the group was either killed by a projectile impact (in which case the thing data was marked as unused) or the party is on the destination square and an event is created to move the creature into the dungeon later (in which case the thing is referenced in the event) */
		return Thing::_none;
	}
	_vm->f064_SOUND_RequestPlay_CPSD(k17_soundBUZZ, mapX, mapY, k1_soundModePlayIfPrioritized);
	return L0349_T_GroupThing;
}

bool GroupMan::f223_isSquareACorridorTeleporterPitOrDoor(int16 mapX, int16 mapY) {
	int16 L0544_i_SquareType;

	return (((L0544_i_SquareType = Square(_vm->_dungeonMan->f151_getSquare(mapX, mapY)).getType()) == k1_CorridorElemType)
			|| (L0544_i_SquareType == k5_ElementTypeTeleporter) || (L0544_i_SquareType == k2_ElementTypePit) || (L0544_i_SquareType == k4_DoorElemType));
}

int16 GroupMan::f177_getMeleeTargetCreatureOrdinal(int16 groupX, int16 groupY, int16 partyX, int16 partyY, uint16 champCell) {
	uint16 L0321_ui_Counter;
	int16 L0322_i_CreatureOrdinal;
	Thing L0323_T_GroupThing;
	Group* L0324_ps_Group;
	signed char L0325_auc_OrderedCellsToAttack[4];


	if ((L0323_T_GroupThing = _vm->_groupMan->f175_groupGetThing(groupX, groupY)) == Thing::_endOfList) {
		return 0;
	}
	L0324_ps_Group = (Group*)_vm->_dungeonMan->f156_getThingData(L0323_T_GroupThing);
	f229_setOrderedCellsToAttack(L0325_auc_OrderedCellsToAttack, groupX, groupY, partyX, partyY, champCell);
	L0321_ui_Counter = 0;
	for (;;) { /*_Infinite loop_*/
		if (L0322_i_CreatureOrdinal = _vm->_groupMan->f176_getCreatureOrdinalInCell(L0324_ps_Group, L0325_auc_OrderedCellsToAttack[L0321_ui_Counter])) {
			return L0322_i_CreatureOrdinal;
		}
		L0321_ui_Counter++;
	}
}

int16 GroupMan::f231_getMeleeActionDamage(Champion* champ, int16 champIndex, Group* group, int16 creatureIndex, int16 mapX, int16 mapY, uint16 actionHitProbability, uint16 actionDamageFactor, int16 skillIndex) {
	int16 L0565_i_Damage = 0;
	int16 L0566_i_Damage = 0;
	int16 L0567_i_DoubledMapDifficulty;
	int16 L0568_i_Defense;
	int16 L0569_i_Outcome;
	bool L0570_B_ActionHitsNonMaterialCreatures;
	int16 L0571_i_ActionHandObjectIconIndex;
	CreatureInfo* L0572_ps_CreatureInfo;

	if (champIndex >= _vm->_championMan->_g305_partyChampionCount) {
		return 0;
	}
	if (!champ->_currHealth) {
		return 0;
	}
	L0567_i_DoubledMapDifficulty = _vm->_dungeonMan->_g269_currMap->_difficulty << 1;
	L0572_ps_CreatureInfo = &g243_CreatureInfo[group->_type];
	L0571_i_ActionHandObjectIconIndex = _vm->_objectMan->f33_getIconIndex(champ->_slots[k1_ChampionSlotActionHand]);
	if (L0570_B_ActionHitsNonMaterialCreatures = getFlag(actionHitProbability, k0x8000_hitNonMaterialCreatures)) {
		clearFlag(actionHitProbability, k0x8000_hitNonMaterialCreatures);
	}
	if ((!getFlag(L0572_ps_CreatureInfo->_attributes, k0x0040_MaskCreatureInfo_nonMaterial) || L0570_B_ActionHitsNonMaterialCreatures) &&
		((_vm->_championMan->f311_getDexterity(champ) > (_vm->getRandomNumber(32) + L0572_ps_CreatureInfo->_dexterity + L0567_i_DoubledMapDifficulty - 16)) ||
		(!_vm->getRandomNumber(4)) ||
		 (_vm->_championMan->f308_isLucky(champ, 75 - actionHitProbability)))) {
		if (!(L0565_i_Damage = _vm->_championMan->f312_getStrength(champIndex, k1_ChampionSlotActionHand))) {
			goto T0231009;
		}
		L0565_i_Damage += _vm->getRandomNumber((L0565_i_Damage >> 1) + 1);
		L0565_i_Damage = ((long)L0565_i_Damage * (long)actionDamageFactor) >> 5;
		L0568_i_Defense = _vm->getRandomNumber(32) + L0572_ps_CreatureInfo->_defense + L0567_i_DoubledMapDifficulty;
		if (L0571_i_ActionHandObjectIconIndex == k39_IconIndiceWeaponDiamondEdge) {
			L0568_i_Defense -= L0568_i_Defense >> 2;
		} else {
			if (L0571_i_ActionHandObjectIconIndex == k43_IconIndiceWeaponHardcleaveExecutioner) {
				L0568_i_Defense -= L0568_i_Defense >> 3;
			}
		}
		if ((L0566_i_Damage = L0565_i_Damage = _vm->getRandomNumber(32) + L0565_i_Damage - L0568_i_Defense) <= 1) {
T0231009:
			if (!(L0565_i_Damage = _vm->getRandomNumber(4))) {
				goto T0231015;
			}
			L0565_i_Damage++;
			if (((L0566_i_Damage += _vm->getRandomNumber(16)) > 0) || (_vm->getRandomNumber(2))) {
				L0565_i_Damage += _vm->getRandomNumber(4);
				if (!_vm->getRandomNumber(4)) {
					L0565_i_Damage += MAX(0, L0566_i_Damage + _vm->getRandomNumber(16));
				}
			}
		}
		L0565_i_Damage >>= 1;
		L0565_i_Damage += _vm->getRandomNumber(L0565_i_Damage) + _vm->getRandomNumber(4);
		L0565_i_Damage += _vm->getRandomNumber(L0565_i_Damage);
		L0565_i_Damage >>= 2;
		L0565_i_Damage += _vm->getRandomNumber(4) + 1;
		if ((L0571_i_ActionHandObjectIconIndex == k40_IconIndiceWeaponVorpalBlade) && !getFlag(L0572_ps_CreatureInfo->_attributes, k0x0040_MaskCreatureInfo_nonMaterial) && !(L0565_i_Damage >>= 1))
			goto T0231015;
		if (_vm->getRandomNumber(64) < _vm->_championMan->f303_getSkillLevel(champIndex, skillIndex)) {
			L0565_i_Damage += L0565_i_Damage + 10;
		}
		L0569_i_Outcome = f190_groupGetDamageCreatureOutcome(group, creatureIndex, mapX, mapY, L0565_i_Damage, true);
		_vm->_championMan->f304_addSkillExperience(champIndex, skillIndex, (L0565_i_Damage * L0572_ps_CreatureInfo->M58_getExperience() >> 4) + 3);
		_vm->_championMan->f325_decrementStamine(champIndex, _vm->getRandomNumber(4) + 4);
		goto T0231016;
	}
T0231015:
	L0565_i_Damage = 0;
	L0569_i_Outcome = k0_outcomeKilledNoCreaturesInGroup;
	_vm->_championMan->f325_decrementStamine(champIndex, _vm->getRandomNumber(2) + 2);
T0231016:
	_vm->_championMan->f292_drawChampionState((ChampionIndex)champIndex);
	if (L0569_i_Outcome != k2_outcomeKilledAllCreaturesInGroup) {
		f209_processEvents29to41(mapX, mapY, kM1_TMEventTypeCreateReactionEvent31ParyIsAdjacent, 0);
	}
	return L0565_i_Damage;
}

void GroupMan::f224_fluxCageAction(int16 mapX, int16 mapY) {
	Thing L0545_T_Thing;
	int16 L0546_i_Multiple;
#define AL0546_i_SquareType    L0546_i_Multiple
#define AL0546_i_FluxcageCount L0546_i_Multiple
	TimelineEvent L0547_s_Event;


	if (((AL0546_i_SquareType = _vm->_dungeonMan->f151_getSquare(mapX, mapY).getType()) == k0_ElementTypeWall) || (AL0546_i_SquareType == k3_ElementTypeStairs)) {
		return;
	}
	if ((L0545_T_Thing = _vm->_dungeonMan->f166_getUnusedThing(k15_ExplosionThingType)) == Thing::_none) {
		return;
	}
	_vm->_dungeonMan->f163_linkThingToList(L0545_T_Thing, Thing(0), mapX, mapY);
	(((Explosion*)_vm->_dungeonMan->_g284_thingData[k15_ExplosionThingType])[L0545_T_Thing.getIndex()]).setType(k50_ExplosionType_Fluxcage);
	M33_setMapAndTime(L0547_s_Event._mapTime, _vm->_dungeonMan->_g272_currMapIndex, _vm->_g313_gameTime + 100);
	L0547_s_Event._type = k24_TMEventTypeRemoveFluxcage;
	L0547_s_Event._priority = 0;
	L0547_s_Event._C._slot = L0545_T_Thing.toUint16();
	L0547_s_Event._B._location._mapX = mapX;
	L0547_s_Event._B._location._mapY = mapY;
	L0547_s_Event._B._location._mapY = mapY;
	_vm->_timeline->f238_addEventGetEventIndex(&L0547_s_Event);
	if (f222_isLordChaosOnSquare(mapX, mapY - 1)) {
		mapY--;
		AL0546_i_FluxcageCount = f221_isFluxcageOnSquare(mapX + 1, mapY);
		goto T0224005;
	}
	if (f222_isLordChaosOnSquare(mapX - 1, mapY)) {
		mapX--;
		AL0546_i_FluxcageCount = f221_isFluxcageOnSquare(mapX, mapY + 1);
T0224005:
		AL0546_i_FluxcageCount += f221_isFluxcageOnSquare(mapX, mapY - 1) + f221_isFluxcageOnSquare(mapX - 1, mapY);
	} else {
		if (f222_isLordChaosOnSquare(mapX + 1, mapY)) {
			mapX++;
			AL0546_i_FluxcageCount = f221_isFluxcageOnSquare(mapX, mapY - 1);
			goto T0224008;
		}
		if (f222_isLordChaosOnSquare(mapX, mapY + 1)) {
			mapY++;
			AL0546_i_FluxcageCount = f221_isFluxcageOnSquare(mapX - 1, mapY);
T0224008:
			AL0546_i_FluxcageCount += f221_isFluxcageOnSquare(mapX, mapY + 1) + f221_isFluxcageOnSquare(mapX + 1, mapY);
		} else {
			AL0546_i_FluxcageCount = 0;
		}
	}
	if (AL0546_i_FluxcageCount == 2) {
		f209_processEvents29to41(mapX, mapY, kM3_TMEventTypeCreateReactionEvent29DangerOnSquare, 0);
	}
}

uint16 GroupMan::f222_isLordChaosOnSquare(int16 mapX, int16 mapY) {
	Thing L0542_T_Thing;
	Group* L0543_ps_Group;

	if ((L0542_T_Thing = _vm->_groupMan->f175_groupGetThing(mapX, mapY)) == Thing::_endOfList) {
		return 0;
	}
	L0543_ps_Group = (Group*)_vm->_dungeonMan->f156_getThingData(L0542_T_Thing);
	if (L0543_ps_Group->_type == k23_CreatureTypeLordChaos) {
		return L0542_T_Thing.toUint16();
	}
	return 0;
}

bool GroupMan::f221_isFluxcageOnSquare(int16 mapX, int16 mapY) {
	Thing L0540_T_Thing;
	int16 L0541_i_SquareType;

	if (((L0541_i_SquareType = _vm->_dungeonMan->f151_getSquare(mapX, mapY).getType()) == k0_ElementTypeWall) || (L0541_i_SquareType == k3_ElementTypeStairs)) {
		return false;
	}
	L0540_T_Thing = _vm->_dungeonMan->f161_getSquareFirstThing(mapX, mapY);
	while (L0540_T_Thing != Thing::_endOfList) {
		if ((L0540_T_Thing.getType() == k15_ExplosionThingType) && (((Explosion*)_vm->_dungeonMan->_g284_thingData[k15_ExplosionThingType])[L0540_T_Thing.getIndex()].getType() == k50_ExplosionType_Fluxcage)) {
			return true;
		}
		L0540_T_Thing = _vm->_dungeonMan->f159_getNextThing(L0540_T_Thing);
	}
	return false;
}

void GroupMan::f225_fuseAction(uint16 mapX, uint16 mapY) {
	int16 L0548_i_MapX;
	int16 L0549_i_MapY;
	uint16 L0551_ui_FluxcageCount;
	uint16 L0552_ui_FluxcageIndex;
	uint16 L0553_ui_Counter;
	bool L0554_aB_Fluxcages[4];
	Thing L0555_T_LordChaosThing;

	if ((mapX < 0) || (mapX >= _vm->_dungeonMan->_g273_currMapWidth) || (mapY < 0) || (mapY >= _vm->_dungeonMan->_g274_currMapHeight)) {
		return;
	}

	_vm->_projexpl->f213_explosionCreate(Thing::_explHarmNonMaterial, 255, mapX, mapY, k255_CreatureTypeSingleCenteredCreature); /* BUG0_17 The game crashes after the Fuse action is performed while looking at a wall on a map boundary. An explosion thing is created on the square in front of the party but there is no check to ensure the square coordinates are in the map bounds. This corrupts a memory location and leads to a game crash */
	if ((L0555_T_LordChaosThing = Thing(f222_isLordChaosOnSquare(mapX, mapY))).toUint16()) {
		L0551_ui_FluxcageCount = (L0554_aB_Fluxcages[0] = f221_isFluxcageOnSquare(mapX - 1, mapY)) +
			(L0554_aB_Fluxcages[1] = f221_isFluxcageOnSquare(mapX + 1, mapY)) +
			(L0554_aB_Fluxcages[2] = f221_isFluxcageOnSquare(mapX, mapY - 1)) +
			(L0554_aB_Fluxcages[3] = f221_isFluxcageOnSquare(mapX, mapY + 1));
		while (L0551_ui_FluxcageCount++ < 4) {
			L0548_i_MapX = mapX;
			L0549_i_MapY = mapY;
			L0552_ui_FluxcageIndex = _vm->getRandomNumber(4);
			for (L0553_ui_Counter = 5; --L0553_ui_Counter; L0552_ui_FluxcageIndex = returnNextVal(L0552_ui_FluxcageIndex)) {
				if (!L0554_aB_Fluxcages[L0552_ui_FluxcageIndex]) {
					L0554_aB_Fluxcages[L0552_ui_FluxcageIndex] = true;
					switch (L0552_ui_FluxcageIndex) {
					case 0:
						L0548_i_MapX--;
						break;
					case 1:
						L0548_i_MapX++;
						break;
					case 2:
						L0549_i_MapY--;
						break;
					case 3:
						L0549_i_MapY++;
					}
					break;
				}
			}
			if (f223_isSquareACorridorTeleporterPitOrDoor(L0548_i_MapX, L0549_i_MapY)) {
				if (!_vm->_movsens->f267_getMoveResult(L0555_T_LordChaosThing, mapX, mapY, L0548_i_MapX, L0549_i_MapY)) {
					f180_startWanedring(L0548_i_MapX, L0549_i_MapY);
				}
				return;
			}
		}
		warning(false, "F0446_STARTEND_FuseSequence()");
	}
}

void GroupMan::save1_ActiveGroupPart(Common::OutSaveFile* file) {
	for (uint16 i = 0; i < _g376_maxActiveGroupCount; ++i) {
		ActiveGroup *group = &_g375_activeGroups[i];
		file->writeUint16BE(group->_groupThingIndex);
		file->writeUint16BE(group->_directions);
		file->writeByte(group->_cells);
		file->writeByte(group->_lastMoveTime);
		file->writeByte(group->_delayFleeingFromTarget);
		file->writeByte(group->_targetMapX);
		file->writeByte(group->_targetMapY);
		file->writeByte(group->_priorMapX);
		file->writeByte(group->_priorMapY);
		file->writeByte(group->_homeMapX);
		file->writeByte(group->_homeMapY);
		for (uint16 j = 0; j < 4; ++j)
			file->writeByte(group->_aspect[j]);
	}
}

void GroupMan::load1_ActiveGroupPart(Common::InSaveFile* file) {
	for (uint16 i = 0; i < _g376_maxActiveGroupCount; ++i) {
		ActiveGroup *group = &_g375_activeGroups[i];
		group->_groupThingIndex = file->readUint16BE();
		group->_directions = (direction)file->readUint16BE();
		group->_cells = file->readByte();
		group->_lastMoveTime = file->readByte();
		group->_delayFleeingFromTarget = file->readByte();
		group->_targetMapX = file->readByte();
		group->_targetMapY = file->readByte();
		group->_priorMapX = file->readByte();
		group->_priorMapY = file->readByte();
		group->_homeMapX = file->readByte();
		group->_homeMapY = file->readByte();
		for (uint16 j = 0; j < 4; ++j)
			group->_aspect[j] = file->readByte();
	}
}
}
