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

#include "dm/projexpl.h"
#include "dm/dungeonman.h"
#include "dm/timeline.h"
#include "dm/group.h"
#include "dm/objectman.h"
#include "dm/movesens.h"
#include "dm/sounds.h"

namespace DM {

ProjExpl::ProjExpl(DMEngine* vm) : _vm(vm) {
	_creatureDamageOutcome = 0;
	_secondaryDirToOrFromParty = 0;
	_lastCreatureAttackTime = -200;
	_createLauncherProjectile = false;
	_projectilePoisonAttack = 0;
	_projectileAttackType = 0;
	_lastPartyMovementTime = 0;
}

void ProjExpl::createProjectile(Thing thing, int16 mapX, int16 mapY, uint16 cell, Direction dir, byte kineticEnergy, byte attack, byte stepEnergy) {
	Thing projectileThing = _vm->_dungeonMan->getUnusedThing(k14_ProjectileThingType);
	if (projectileThing == Thing::_none) /* BUG0_16 If the game cannot create a projectile thing because it has run out of such things (60 maximum) then the object being thrown/shot/launched is orphaned. If the game has run out of projectile things it will try to remove a projectile from elsewhere in the dungeon, except in an area of 11x11 squares centered around the party (to make sure the player cannot actually see the thing disappear on screen) */
		return;

	projectileThing = thingWithNewCell(projectileThing, cell);
	Projectile *projectilePtr = (Projectile *)_vm->_dungeonMan->getThingData(projectileThing);
	projectilePtr->_slot = thing;
	projectilePtr->_kineticEnergy = MIN((int16)kineticEnergy, (int16)255);
	projectilePtr->_attack = attack;
	_vm->_dungeonMan->linkThingToList(projectileThing, Thing(0), mapX, mapY); /* Projectiles are added on the square and not 'moved' onto the square. In the case of a projectile launcher sensor, this means that the new projectile traverses the square in front of the launcher without any trouble: there is no impact if it is a wall, the projectile direction is not changed if it is a teleporter. Impacts with creatures and champions are still processed */
	TimelineEvent newEvent;
	setMapAndTime(newEvent._mapTime, _vm->_dungeonMan->_currMapIndex, _vm->_gameTime + 1);
	if (_createLauncherProjectile)
		newEvent._type = k49_TMEventTypeMoveProjectile; /* Launcher projectiles can impact immediately */
	else
		newEvent._type = k48_TMEventTypeMoveProjectileIgnoreImpacts; /* Projectiles created by champions or creatures ignore impacts on their first movement */

	newEvent._priority = 0;
	newEvent._B._slot = projectileThing.toUint16();
	newEvent._C._projectile.setMapX(mapX);
	newEvent._C._projectile.setMapY(mapY);
	newEvent._C._projectile.setStepEnergy(stepEnergy);
	newEvent._C._projectile.setDir(dir);
	projectilePtr->_eventIndex = _vm->_timeline->addEventGetEventIndex(&newEvent);
}

bool ProjExpl::hasProjectileImpactOccurred(int16 impactType, int16 mapXCombo, int16 mapYCombo, int16 cell, Thing projectileThing) {
	Projectile *projectileThingData = (Projectile *)_vm->_dungeonMan->getThingData(Thing(projectileThing));
	bool removePotion = false;
	int16 potionPower = 0;
	_creatureDamageOutcome = k0_outcomeKilledNoCreaturesInGroup;
	Thing projectileAssociatedThing = projectileThingData->_slot;
	int16 projectileAssociatedThingType = projectileAssociatedThing.getType();
	Potion *potion = nullptr;
	Thing explosionThing = Thing::_none;
	if (projectileAssociatedThingType == k8_PotionThingType) {
		Group *projectileAssociatedGroup = (Group *)_vm->_dungeonMan->getThingData(projectileAssociatedThing);
		PotionType potionType = ((Potion *)projectileAssociatedGroup)->getType();
		if ((potionType == k3_PotionTypeVen) || (potionType == k19_PotionTypeFulBomb)) {
			explosionThing = (potionType == k3_PotionTypeVen) ? Thing::_explPoisonCloud: Thing::_explFireBall;
			removePotion = true;
			potionPower = ((Potion *)projectileAssociatedGroup)->getPower();
			potion = (Potion *)projectileAssociatedGroup;
		}
	}
	bool createExplosionOnImpact = (projectileAssociatedThingType == k15_ExplosionThingType) && (projectileAssociatedThing != Thing::_explSlime) && (projectileAssociatedThing != Thing::_explPoisonBolt);
	Thing *curGroupSlot = nullptr;
	int16 projectileMapX;
	int16 projectileMapY;
	int16 projectileTargetMapX = mapXCombo;
	int16 projectileTargetMapY = mapYCombo;

	if (mapXCombo <= 255) {
		projectileMapX = mapXCombo;
		projectileMapY = mapYCombo;
	} else {
		projectileMapX = (mapXCombo >> 8) - 1;
		projectileMapY = (mapYCombo >> 8);
		projectileTargetMapX &= 0x00FF;
		projectileTargetMapY &= 0x00FF;
	}

	int16 championAttack = 0;
	int16 attack = 0;
	int16 championIndex = 0;
	switch (impactType) {
	case k4_DoorElemType: {
		byte curSquare = _vm->_dungeonMan->_currMapData[projectileTargetMapX][projectileTargetMapY];
		int16 curDoorState = Square(curSquare).getDoorState();
		Door *curDoor = (Door *)_vm->_dungeonMan->getSquareFirstThingData(projectileTargetMapX, projectileTargetMapY);
		if ((curDoorState != k5_doorState_DESTROYED) && (projectileAssociatedThing == Thing::_explOpenDoor)) {
			if (curDoor->hasButton())
				_vm->_moveSens->addEvent(k10_TMEventTypeDoor, projectileTargetMapX, projectileTargetMapY, 0, k2_SensorEffToggle, _vm->_gameTime + 1);
			break;
		}

		if ((curDoorState == k5_doorState_DESTROYED) || (curDoorState <= k1_doorState_FOURTH))
			return false;

		DoorInfo curDoorInfo = _vm->_dungeonMan->_currMapDoorInfo[curDoor->getType()];
		if (getFlag(curDoorInfo._attributes, k0x0002_MaskDoorInfo_ProjectilesCanPassThrough)) {
			if (projectileAssociatedThingType == k15_ExplosionThingType) {
				if (projectileAssociatedThing.toUint16() >= Thing::_explHarmNonMaterial.toUint16())
					return false;
			} else {
				int16 associatedThingIndex = _vm->_dungeonMan->getObjectInfoIndex(projectileAssociatedThing);
				uint16 associatedAllowedSlots = _vm->_dungeonMan->_objectInfos[associatedThingIndex].getAllowedSlots();
				int16 iconIndex = _vm->_objectMan->getIconIndex(projectileAssociatedThing);

				if ((projectileThingData->_attack > _vm->getRandomNumber(128))
				&& getFlag(associatedAllowedSlots, k0x0100_ObjectAllowedSlotPouchPassAndThroughDoors)
				&& (   (projectileAssociatedThingType != k10_JunkThingType)
					|| (iconIndex < k176_IconIndiceJunkIronKey)
					|| (iconIndex > k191_IconIndiceJunkMasterKey)
					)) {
					return false;
				}
			}
		}
		attack = getProjectileImpactAttack(projectileThingData, projectileAssociatedThing) + 1;
		_vm->_groupMan->groupIsDoorDestoryedByAttack(projectileTargetMapX, projectileTargetMapY, attack + _vm->getRandomNumber(attack), false, 0);
		}
		break;
	case kM2_ChampionElemType:
		championIndex = _vm->_championMan->getIndexInCell(cell);
		if (championIndex < 0)
			return false;

		championAttack = attack = getProjectileImpactAttack(projectileThingData, projectileAssociatedThing);
		break;
	case kM1_CreatureElemType: {
		Group *curGroup = (Group *)_vm->_dungeonMan->getThingData(_vm->_groupMan->groupGetThing(projectileTargetMapX, projectileTargetMapY));
		uint16 curCreatureIndex = _vm->_groupMan->getCreatureOrdinalInCell(curGroup, cell);
		if (!curCreatureIndex)
			return false;

		curCreatureIndex--;
		uint16 curCreatureType = curGroup->_type;
		CreatureInfo *curCreatureInfo = &_vm->_dungeonMan->_creatureInfos[curCreatureType];
		if ((projectileAssociatedThing == Thing::_explFireBall) && (curCreatureType == k11_CreatureTypeBlackFlame)) {
			uint16 *curCreatureHealth = &curGroup->_health[curCreatureIndex];
			*curCreatureHealth = MIN(1000, *curCreatureHealth + getProjectileImpactAttack(projectileThingData, projectileAssociatedThing));
			goto T0217044;
		}
		if (getFlag(curCreatureInfo->_attributes, k0x0040_MaskCreatureInfo_nonMaterial) && (projectileAssociatedThing != Thing::_explHarmNonMaterial))
			return false;

		attack = (uint16)((unsigned long)getProjectileImpactAttack(projectileThingData, projectileAssociatedThing) << 6) / curCreatureInfo->_defense;
		if (attack) {
			int16 outcome = _vm->_groupMan->groupGetDamageCreatureOutcome(curGroup, curCreatureIndex, projectileTargetMapX, projectileTargetMapY, attack + _vm->_groupMan->groupGetResistanceAdjustedPoisonAttack(curCreatureType, _projectilePoisonAttack), true);
			if (outcome != k0_outcomeKilledNoCreaturesInGroup)
				_vm->_groupMan->processEvents29to41(projectileTargetMapX, projectileTargetMapY, kM2_TMEventTypeCreateReactionEvent30HitByProjectile, 0);

			_creatureDamageOutcome = outcome;
			if (!createExplosionOnImpact && (outcome == k0_outcomeKilledNoCreaturesInGroup)
			&& (projectileAssociatedThingType == k5_WeaponThingType)
			&& getFlag(curCreatureInfo->_attributes, k0x0400_MaskCreatureInfo_keepThrownSharpWeapon)) {
				Weapon *weapon = (Weapon *)_vm->_dungeonMan->getThingData(projectileAssociatedThing);
				WeaponType weaponType = weapon->getType();
				if ((weaponType == k8_WeaponTypeDagger) || (weaponType == k27_WeaponTypeArrow)
				|| (weaponType == k28_WeaponTypeSlayer) || (weaponType == k31_WeaponTypePoisonDart)
				|| (weaponType == k32_WeaponTypeThrowingStar))
					curGroupSlot = &curGroup->_slot;
			}
		}
		}
		break;
	}
	if (championAttack && _projectilePoisonAttack && _vm->getRandomNumber(2)
	&& _vm->_championMan->addPendingDamageAndWounds_getDamage(championIndex, attack, k0x0004_ChampionWoundHead | k0x0008_ChampionWoundTorso, _projectileAttackType))
		_vm->_championMan->championPoison(championIndex, _projectilePoisonAttack);

	if (createExplosionOnImpact || removePotion) {
		uint16 explosionAttack;
		if (removePotion) {
			projectileAssociatedThing = explosionThing;
			explosionAttack = potionPower;
		} else {
			explosionAttack = projectileThingData->_kineticEnergy;
		}
		if ((projectileAssociatedThing == Thing::_explLightningBolt) && !(explosionAttack >>= 1))
			goto T0217044;
		createExplosion(projectileAssociatedThing, explosionAttack, mapXCombo, mapYCombo, (projectileAssociatedThing == Thing::_explPoisonCloud) ? k255_CreatureTypeSingleCenteredCreature : cell);
	} else {
		uint16 soundIndex;
		if ((projectileAssociatedThing).getType() == k5_WeaponThingType)
			soundIndex = k00_soundMETALLIC_THUD;
		else if (projectileAssociatedThing == Thing::_explPoisonBolt)
			soundIndex = k13_soundSPELL;
		else
			soundIndex = k04_soundWOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM;

		_vm->_sound->requestPlay(soundIndex, projectileMapX, projectileMapY, k1_soundModePlayIfPrioritized);
	}
T0217044:
	if (removePotion) {
		potion->_nextThing = Thing::_none;
		projectileThingData->_slot = explosionThing;
	}
	_vm->_dungeonMan->unlinkThingFromList(projectileThing, Thing(0), projectileMapX, projectileMapY);
	projectileDelete(projectileThing, curGroupSlot, projectileMapX, projectileMapY);
	return true;
}

uint16 ProjExpl::getProjectileImpactAttack(Projectile *projectile, Thing thing) {
	_projectilePoisonAttack = 0;
	_projectileAttackType = k3_attackType_BLUNT;

	uint16 kineticEnergy = projectile->_kineticEnergy;
	ThingType thingType = thing.getType();
	uint16 attack;
	if (thingType != k15_ExplosionThingType) {
		if (thingType == k5_WeaponThingType) {
			WeaponInfo *weaponInfo = _vm->_dungeonMan->getWeaponInfo(thing);
			attack = weaponInfo->_kineticEnergy;
			_projectileAttackType = k3_attackType_BLUNT;
		} else
			attack = _vm->getRandomNumber(4);

		attack += _vm->_dungeonMan->getObjectWeight(thing) >> 1;
	} else if (thing == Thing::_explSlime) {
		attack = _vm->getRandomNumber(16);
		_projectilePoisonAttack = attack + 10;
		attack += _vm->getRandomNumber(32);
	} else {
		if (thing.toUint16() >= Thing::_explHarmNonMaterial.toUint16()) {
			_projectileAttackType = k5_attackType_MAGIC;
			if (thing == Thing::_explPoisonBolt) {
				_projectilePoisonAttack = kineticEnergy;
				return 1;
			}
			return 0;
		}
		_projectileAttackType = k1_attackType_FIRE;
		attack = _vm->getRandomNumber(16) + _vm->getRandomNumber(16) + 10;
		if (thing == Thing::_explLightningBolt) {
			_projectileAttackType = k7_attackType_LIGHTNING;
			attack *= 5;
		}
	}
	attack = ((attack + kineticEnergy) >> 4) + 1;
	attack += _vm->getRandomNumber((attack >> 1) + 1) + _vm->getRandomNumber(4);
	attack = MAX(attack >> 1, attack - (32 - (projectile->_attack >> 3)));
	return attack;
}

void ProjExpl::createExplosion(Thing explThing, uint16 attack, uint16 mapXCombo, uint16 mapYCombo, uint16 cell) {
	Thing unusedThing = _vm->_dungeonMan->getUnusedThing(k15_ExplosionThingType);
	if (unusedThing == Thing::_none)
		return;

	Explosion *explosion = &((Explosion *)_vm->_dungeonMan->_thingData[k15_ExplosionThingType])[(unusedThing).getIndex()];
	int16 projectileTargetMapX;
	int16 projectileTargetMapY;
	uint16 projectileMapX = mapXCombo;
	uint16 projectileMapY = mapYCombo;

	if (mapXCombo <= 255) {
		projectileTargetMapX = mapXCombo;
		projectileTargetMapY = mapYCombo;
	} else {
		projectileTargetMapX = mapXCombo & 0x00FF;
		projectileTargetMapY = mapYCombo & 0x00FF;
		projectileMapX >>= 8;
		projectileMapX--;
		projectileMapY >>= 8;
	}

	if (cell == k255_CreatureTypeSingleCenteredCreature)
		explosion->setCentered(true);
	else {
		explosion->setCentered(false);
		unusedThing = thingWithNewCell(unusedThing, cell);
	}

	explosion->setType(explThing.toUint16() - Thing::_firstExplosion.toUint16());
	explosion->setAttack(attack);
	if (explThing.toUint16() < Thing::_explHarmNonMaterial.toUint16()) {
		uint16 soundIndex = (attack > 80) ? k05_soundSTRONG_EXPLOSION : k20_soundWEAK_EXPLOSION;
		_vm->_sound->requestPlay(soundIndex, projectileMapX, projectileMapY, k1_soundModePlayIfPrioritized);
	} else if (explThing != Thing::_explSmoke)
		_vm->_sound->requestPlay(k13_soundSPELL, projectileMapX, projectileMapY, k1_soundModePlayIfPrioritized);

	_vm->_dungeonMan->linkThingToList(unusedThing, Thing(0), projectileMapX, projectileMapY);
	TimelineEvent newEvent;
	setMapAndTime(newEvent._mapTime, _vm->_dungeonMan->_currMapIndex, _vm->_gameTime + ((explThing == Thing::_explRebirthStep1) ? 5 : 1));
	newEvent._type = k25_TMEventTypeExplosion;
	newEvent._priority = 0;
	newEvent._C._slot = unusedThing.toUint16();
	newEvent._B._location._mapX = projectileMapX;
	newEvent._B._location._mapY = projectileMapY;
	_vm->_timeline->addEventGetEventIndex(&newEvent);
	if ((explThing == Thing::_explLightningBolt) || (explThing == Thing::_explFireBall)) {
		projectileMapX = projectileTargetMapX;
		projectileMapY = projectileTargetMapY;
		attack = (attack >> 1) + 1;
		attack += _vm->getRandomNumber(attack) + 1;
		if ((explThing == Thing::_explFireBall) || (attack >>= 1)) {
			if ((_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) && (projectileMapX == _vm->_dungeonMan->_partyMapX) && (projectileMapY == _vm->_dungeonMan->_partyMapY)) {
				int16 wounds = k0x0001_ChampionWoundReadHand | k0x0002_ChampionWoundActionHand | k0x0004_ChampionWoundHead | k0x0008_ChampionWoundTorso | k0x0010_ChampionWoundLegs | k0x0020_ChampionWoundFeet;
				_vm->_championMan->getDamagedChampionCount(attack, wounds, k1_attackType_FIRE);
			} else {
				unusedThing = _vm->_groupMan->groupGetThing(projectileMapX, projectileMapY);
				if (unusedThing != Thing::_endOfList) {
					Group *creatureGroup = (Group *)_vm->_dungeonMan->getThingData(unusedThing);
					CreatureInfo *creatureInfo = &_vm->_dungeonMan->_creatureInfos[creatureGroup->_type];
					int16 creatureFireResistance = creatureInfo->getFireResistance();
					if (creatureFireResistance != k15_immuneToFire) {
						if (getFlag(creatureInfo->_attributes, k0x0040_MaskCreatureInfo_nonMaterial))
							attack >>= 2;

						if ((attack -= _vm->getRandomNumber((creatureFireResistance << 1) + 1)) > 0)
							_creatureDamageOutcome = _vm->_groupMan->getDamageAllCreaturesOutcome(creatureGroup, projectileMapX, projectileMapY, attack, true);

					}
				}
			}
		}
	}
}

int16 ProjExpl::projectileGetImpactCount(int16 impactType, int16 mapX, int16 mapY, int16 cell) {
	int16 impactCount = 0;
	_creatureDamageOutcome = k0_outcomeKilledNoCreaturesInGroup;
T0218001:
	Thing curThing = _vm->_dungeonMan->getSquareFirstThing(mapX, mapY);
	while (curThing != Thing::_endOfList) {
		if (((curThing).getType() == k14_ProjectileThingType) &&
			((curThing).getCell() == cell) &&
			hasProjectileImpactOccurred(impactType, mapX, mapY, cell, curThing)) {
			projectileDeleteEvent(curThing);
			impactCount++;
			if ((impactType == kM1_CreatureElemType) && (_creatureDamageOutcome == k2_outcomeKilledAllCreaturesInGroup))
				break;
			goto T0218001;
		}
		curThing = _vm->_dungeonMan->getNextThing(curThing);
	}
	return impactCount;
}

void ProjExpl::projectileDeleteEvent(Thing thing) {
	Projectile *projectile = (Projectile *)_vm->_dungeonMan->getThingData(thing);
	_vm->_timeline->deleteEvent(projectile->_eventIndex);
}

void ProjExpl::projectileDelete(Thing projectileThing, Thing *groupSlot, int16 mapX, int16 mapY) {
	Projectile *projectile = (Projectile *)_vm->_dungeonMan->getThingData(projectileThing);
	Thing projectileSlotThing = projectile->_slot;
	if (projectileSlotThing.getType() != k15_ExplosionThingType) {
		if (groupSlot != NULL) {
			Thing previousThing = *groupSlot;
			if (previousThing == Thing::_endOfList) {
				Thing *genericThing = (Thing *)_vm->_dungeonMan->getThingData(projectileSlotThing);
				*genericThing = Thing::_endOfList;
				*groupSlot = projectileSlotThing;
			} else
				_vm->_dungeonMan->linkThingToList(projectileSlotThing, previousThing, kM1_MapXNotOnASquare, 0);
		} else
			_vm->_moveSens->getMoveResult(Thing((projectileSlotThing).getTypeAndIndex() | getFlag(projectileThing.toUint16(), 0xC)), -2, 0, mapX, mapY);
	}
	projectile->_nextThing = Thing::_none;
}

void ProjExpl::processEvents48To49(TimelineEvent* event) {
	TimelineEvent* L0519_ps_Event;
	Projectile* L0520_ps_Projectile;
	Thing L0515_T_ProjectileThingNewCell;
	uint16 L0516_ui_Multiple;
#define AL0516_ui_StepEnergy L0516_ui_Multiple
#define AL0516_ui_Square     L0516_ui_Multiple
	Thing L0521_T_ProjectileThing;
	uint16 L0517_ui_ProjectileDirection;
	bool L0522_B_ProjectileMovesToOtherSquare;
	int16 L0523_i_DestinationMapX;
	int16 L0524_i_DestinationMapY;
	uint16 L0518_ui_Cell;
	int16 L0525_i_SourceMapX = -1;
	int16 L0526_i_SourceMapY = -1;
	TimelineEvent L0527_s_Event;


	L0527_s_Event = *event;
	L0519_ps_Event = &L0527_s_Event;
	L0520_ps_Projectile = (Projectile*)_vm->_dungeonMan->getThingData(L0521_T_ProjectileThing = L0515_T_ProjectileThingNewCell = Thing(L0519_ps_Event->_B._slot));
	L0523_i_DestinationMapX = L0519_ps_Event->_C._projectile.getMapX();
	L0524_i_DestinationMapY = L0519_ps_Event->_C._projectile.getMapY();
	if (L0519_ps_Event->_type == k48_TMEventTypeMoveProjectileIgnoreImpacts) {
		L0519_ps_Event->_type = k49_TMEventTypeMoveProjectile;
	} else {
		L0518_ui_Cell = (L0515_T_ProjectileThingNewCell).getCell();
		if ((_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) && (L0523_i_DestinationMapX == _vm->_dungeonMan->_partyMapX) && (L0524_i_DestinationMapY == _vm->_dungeonMan->_partyMapY) && hasProjectileImpactOccurred(kM2_ChampionElemType, L0523_i_DestinationMapX, L0524_i_DestinationMapY, L0518_ui_Cell, L0515_T_ProjectileThingNewCell)) {
			return;
		}
		if ((_vm->_groupMan->groupGetThing(L0523_i_DestinationMapX, L0524_i_DestinationMapY) != Thing::_endOfList) && hasProjectileImpactOccurred(kM1_CreatureElemType, L0523_i_DestinationMapX, L0524_i_DestinationMapY, L0518_ui_Cell, L0521_T_ProjectileThing)) {
			return;
		}
		if (L0520_ps_Projectile->_kineticEnergy <= (AL0516_ui_StepEnergy = L0519_ps_Event->_C._projectile.getStepEnergy())) {
			_vm->_dungeonMan->unlinkThingFromList(L0515_T_ProjectileThingNewCell = L0521_T_ProjectileThing, Thing(0), L0523_i_DestinationMapX, L0524_i_DestinationMapY);
			projectileDelete(L0515_T_ProjectileThingNewCell, NULL, L0523_i_DestinationMapX, L0524_i_DestinationMapY);
			return;
		}
		L0520_ps_Projectile->_kineticEnergy -= AL0516_ui_StepEnergy;
		if (L0520_ps_Projectile->_attack < AL0516_ui_StepEnergy) {
			L0520_ps_Projectile->_attack = 0;
		} else {
			L0520_ps_Projectile->_attack -= AL0516_ui_StepEnergy;
		}
	}
	L0517_ui_ProjectileDirection = L0519_ps_Event->_C._projectile.getDir();
	L0515_T_ProjectileThingNewCell = Thing(L0519_ps_Event->_B._slot);
	L0518_ui_Cell = L0515_T_ProjectileThingNewCell.getCell();
	L0522_B_ProjectileMovesToOtherSquare = (L0517_ui_ProjectileDirection == L0518_ui_Cell) || (returnNextVal(L0517_ui_ProjectileDirection) == L0518_ui_Cell);
	if (L0522_B_ProjectileMovesToOtherSquare) {
		L0525_i_SourceMapX = L0523_i_DestinationMapX;
		L0526_i_SourceMapY = L0524_i_DestinationMapY;
		L0523_i_DestinationMapX += _vm->_dirIntoStepCountEast[L0517_ui_ProjectileDirection], L0524_i_DestinationMapY += _vm->_dirIntoStepCountNorth[L0517_ui_ProjectileDirection];
		AL0516_ui_Square = _vm->_dungeonMan->getSquare(L0523_i_DestinationMapX, L0524_i_DestinationMapY).toByte();
		if ((Square(AL0516_ui_Square).getType() == k0_WallElemType) ||
			((Square(AL0516_ui_Square).getType() == k6_FakeWallElemType) && !getFlag(AL0516_ui_Square, (k0x0001_FakeWallImaginary | k0x0004_FakeWallOpen))) ||
			((Square(AL0516_ui_Square).getType() == k3_StairsElemType) && (Square(_vm->_dungeonMan->_currMapData[L0525_i_SourceMapX][L0526_i_SourceMapY]).getType() == k3_StairsElemType))) {
			if (hasProjectileImpactOccurred(Square(AL0516_ui_Square).getType(), L0525_i_SourceMapX, L0526_i_SourceMapY, L0518_ui_Cell, L0515_T_ProjectileThingNewCell)) {
				return;
			}
		}
	}
	if ((L0517_ui_ProjectileDirection & 0x0001) == (L0518_ui_Cell & 0x0001)) {
		L0518_ui_Cell--;
	} else {
		L0518_ui_Cell++;
	}
	L0515_T_ProjectileThingNewCell = thingWithNewCell(L0515_T_ProjectileThingNewCell, L0518_ui_Cell &= 0x0003);
	if (L0522_B_ProjectileMovesToOtherSquare) {
		_vm->_moveSens->getMoveResult(L0515_T_ProjectileThingNewCell, L0525_i_SourceMapX, L0526_i_SourceMapY, L0523_i_DestinationMapX, L0524_i_DestinationMapY);
		L0519_ps_Event->_C._projectile.setMapX(_vm->_moveSens->_moveResultMapX);
		L0519_ps_Event->_C._projectile.setMapY(_vm->_moveSens->_moveResultMapY);
		L0519_ps_Event->_C._projectile.setDir((Direction)_vm->_moveSens->_moveResultDir);
		L0515_T_ProjectileThingNewCell = thingWithNewCell(L0515_T_ProjectileThingNewCell, _vm->_moveSens->_moveResultCell);
		M31_setMap(L0519_ps_Event->_mapTime, _vm->_moveSens->_moveResultMapIndex);
	} else {
		if ((Square(_vm->_dungeonMan->getSquare(L0523_i_DestinationMapX, L0524_i_DestinationMapY)).getType() == k4_DoorElemType) && hasProjectileImpactOccurred(k4_DoorElemType, L0523_i_DestinationMapX, L0524_i_DestinationMapY, L0518_ui_Cell, L0521_T_ProjectileThing)) {
			return;
		}
		_vm->_dungeonMan->unlinkThingFromList(L0515_T_ProjectileThingNewCell, Thing(0), L0523_i_DestinationMapX, L0524_i_DestinationMapY);
		_vm->_dungeonMan->linkThingToList(L0515_T_ProjectileThingNewCell, Thing(0), L0523_i_DestinationMapX, L0524_i_DestinationMapY);
	}
	L0519_ps_Event->_mapTime += (_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) ? 1 : 3;
	//Strangerke: CHECKME: Maybe we should keep that piece of code too as it sounds like it's fixing a weird behavior of projectiles on different maps
#ifdef COMPILE42_CSB20EN_CSB21EN /* CHANGE7_20_IMPROVEMENT Projectiles now move at the same speed on all maps instead of moving slower on maps other than the party map */
	L0519_ps_Event->Map_Time++;
#endif
	L0519_ps_Event->_B._slot = L0515_T_ProjectileThingNewCell.toUint16();
	L0520_ps_Projectile->_eventIndex = _vm->_timeline->addEventGetEventIndex(L0519_ps_Event);
}

void ProjExpl::processEvent25(TimelineEvent* event) {
	Explosion* L0532_ps_Explosion;
	Group* L0533_ps_Group = nullptr;
	CreatureInfo* L0534_ps_CreatureInfo = nullptr;
	uint16 L0528_ui_MapX;
	uint16 L0529_ui_MapY;
	int16 L0530_i_Attack;
	int16 L0531_i_Multiple;
#define AL0531_i_SquareType    L0531_i_Multiple
#define AL0531_i_CreatureCount L0531_i_Multiple
	Thing L0535_T_GroupThing;
	Thing L0536_T_ExplosionThing;
	uint16 L0537_ui_Multiple = 0;
#define AL0537_ui_CreatureType                L0537_ui_Multiple
#define AL0537_ui_NonMaterialAdditionalAttack L0537_ui_Multiple
	bool L0538_B_ExplosionOnPartySquare;
	TimelineEvent L0539_s_Event;

	L0528_ui_MapX = event->_B._location._mapX;
	L0529_ui_MapY = event->_B._location._mapY;
	L0532_ps_Explosion = &((Explosion*)_vm->_dungeonMan->_thingData[k15_ExplosionThingType])[Thing((event->_C._slot)).getIndex()];
	AL0531_i_SquareType = Square(_vm->_dungeonMan->_currMapData[L0528_ui_MapX][L0529_ui_MapY]).getType();
	L0538_B_ExplosionOnPartySquare = (_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) && (L0528_ui_MapX == _vm->_dungeonMan->_partyMapX) && (L0529_ui_MapY == _vm->_dungeonMan->_partyMapY);
	if ((L0535_T_GroupThing = _vm->_groupMan->groupGetThing(L0528_ui_MapX, L0529_ui_MapY)) != Thing::_endOfList) {
		L0533_ps_Group = (Group*)_vm->_dungeonMan->getThingData(L0535_T_GroupThing);
		L0534_ps_CreatureInfo = &_vm->_dungeonMan->_creatureInfos[AL0537_ui_CreatureType = L0533_ps_Group->_type];
	}
	if ((L0536_T_ExplosionThing = Thing(Thing::_firstExplosion.toUint16() + L0532_ps_Explosion->getType())) == Thing::_explPoisonCloud) {
		L0530_i_Attack = MAX(1, MIN(L0532_ps_Explosion->getAttack() >> 5, 4) + _vm->getRandomNumber(2)); /* Value between 1 and 5 */
	} else {
		L0530_i_Attack = (L0532_ps_Explosion->getAttack() >> 1) + 1;
		L0530_i_Attack += _vm->getRandomNumber(L0530_i_Attack) + 1;
	}


	switch (L0536_T_ExplosionThing.toUint16()) {
	case 0xFF82:
		if (!(L0530_i_Attack >>= 1))
			break;
	case 0xFF80:
		if (AL0531_i_SquareType == k4_DoorElemType) {
			_vm->_groupMan->groupIsDoorDestoryedByAttack(L0528_ui_MapX, L0529_ui_MapY, L0530_i_Attack, true, 0);
		}
		break;
	case 0xFF83:
		if ((L0535_T_GroupThing != Thing::_endOfList) && getFlag(L0534_ps_CreatureInfo->_attributes, k0x0040_MaskCreatureInfo_nonMaterial)) {
			if ((AL0537_ui_CreatureType == k19_CreatureTypeMaterializerZytaz) && (_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex)) { /* ASSEMBLY_COMPILATION_DIFFERENCE jmp */
				L0530_i_Attack -= (AL0537_ui_NonMaterialAdditionalAttack = L0530_i_Attack >> 3);
				AL0537_ui_NonMaterialAdditionalAttack <<= 1;
				AL0537_ui_NonMaterialAdditionalAttack++;
				AL0531_i_CreatureCount = L0533_ps_Group->getCount();
				do {
					if (getFlag(_vm->_groupMan->_activeGroups[L0533_ps_Group->getActiveGroupIndex()]._aspect[AL0531_i_CreatureCount], k0x0080_MaskActiveGroupIsAttacking)) { /* Materializer / Zytaz can only be damaged while they are attacking */
						_vm->_groupMan->groupGetDamageCreatureOutcome(L0533_ps_Group, AL0531_i_CreatureCount, L0528_ui_MapX, L0529_ui_MapY, L0530_i_Attack + _vm->getRandomNumber(AL0537_ui_NonMaterialAdditionalAttack) + _vm->getRandomNumber(4), true);
					}
				} while (--AL0531_i_CreatureCount >= 0);
			} else {
				_vm->_groupMan->getDamageAllCreaturesOutcome(L0533_ps_Group, L0528_ui_MapX, L0529_ui_MapY, L0530_i_Attack, true);
			}
		}
		break;
	case 0xFFE4:
		L0532_ps_Explosion->setType(L0532_ps_Explosion->getType() + 1);
		_vm->_sound->requestPlay(k05_soundSTRONG_EXPLOSION, L0528_ui_MapX, L0529_ui_MapY, k1_soundModePlayIfPrioritized);
		goto T0220026;
	case 0xFFA8:
		if (L0532_ps_Explosion->getAttack() > 55) {
			L0532_ps_Explosion->setAttack(L0532_ps_Explosion->getAttack() - 40);
			goto T0220026;
		}
		break;
	case 0xFF87:
		if (L0538_B_ExplosionOnPartySquare) {
			_vm->_championMan->getDamagedChampionCount(L0530_i_Attack, k0x0000_ChampionWoundNone, k0_attackType_NORMAL);
		} else {
			if ((L0535_T_GroupThing != Thing::_endOfList) && (L0530_i_Attack = _vm->_groupMan->groupGetResistanceAdjustedPoisonAttack(AL0537_ui_CreatureType, L0530_i_Attack)) && (_vm->_groupMan->getDamageAllCreaturesOutcome(L0533_ps_Group, L0528_ui_MapX, L0529_ui_MapY, L0530_i_Attack, true) != k2_outcomeKilledAllCreaturesInGroup) && (L0530_i_Attack > 2)) {
				_vm->_groupMan->processEvents29to41(L0528_ui_MapX, L0529_ui_MapY, kM3_TMEventTypeCreateReactionEvent29DangerOnSquare, 0);
			}
		}
		if (L0532_ps_Explosion->getAttack() >= 6) {
			L0532_ps_Explosion->setAttack(L0532_ps_Explosion->getAttack() - 3);
T0220026:
			L0539_s_Event = *event;
			L0539_s_Event._mapTime++;
			_vm->_timeline->addEventGetEventIndex(&L0539_s_Event);
			return;
		}
	}
	_vm->_dungeonMan->unlinkThingFromList(Thing(event->_C._slot), Thing(0), L0528_ui_MapX, L0529_ui_MapY);
	L0532_ps_Explosion->setNextThing(Thing::_none);
}
}
