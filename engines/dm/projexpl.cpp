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

ProjExpl::ProjExpl(DMEngine *vm) : _vm(vm) {
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
					|| (iconIndex < kDMIconIndiceJunkIronKey)
					|| (iconIndex > kDMIconIndiceJunkMasterKey)
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
	&& _vm->_championMan->addPendingDamageAndWounds_getDamage(championIndex, attack, kDMChampionWoundHead | kDMChampionWoundTorso, _projectileAttackType))
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
				int16 wounds = kDMChampionWoundReadHand | kDMChampionWoundActionHand | kDMChampionWoundHead | kDMChampionWoundTorso | kDMChampionWoundLegs | kDMChampionWoundFeet;
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
	
	for (Thing curThing = _vm->_dungeonMan->getSquareFirstThing(mapX, mapY); curThing != Thing::_endOfList; ) {
		if (((curThing).getType() == k14_ProjectileThingType) && ((curThing).getCell() == cell) &&
			hasProjectileImpactOccurred(impactType, mapX, mapY, cell, curThing)) {
			projectileDeleteEvent(curThing);
			impactCount++;
			if ((impactType == kM1_CreatureElemType) && (_creatureDamageOutcome == k2_outcomeKilledAllCreaturesInGroup))
				break;

			Thing curThing = _vm->_dungeonMan->getSquareFirstThing(mapX, mapY);
		} else
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

void ProjExpl::processEvents48To49(TimelineEvent *event) {
	int16 sourceMapX = -1;
	int16 sourceMapY = -1;
	TimelineEvent firstEvent = *event;
	TimelineEvent *curEvent = &firstEvent;
	Thing projectileThingNewCell = Thing(curEvent->_B._slot);
	Thing projectileThing  = projectileThingNewCell;
	Projectile *projectile = (Projectile *)_vm->_dungeonMan->getThingData(projectileThing);
	int16 destinationMapX = curEvent->_C._projectile.getMapX();
	int16 destinationMapY = curEvent->_C._projectile.getMapY();

	if (curEvent->_type == k48_TMEventTypeMoveProjectileIgnoreImpacts)
		curEvent->_type = k49_TMEventTypeMoveProjectile;
	else {
		uint16 projectileCurCell = projectileThingNewCell.getCell();
		if ((_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) && (destinationMapX == _vm->_dungeonMan->_partyMapX) && (destinationMapY == _vm->_dungeonMan->_partyMapY) && hasProjectileImpactOccurred(kM2_ChampionElemType, destinationMapX, destinationMapY, projectileCurCell, projectileThingNewCell))
			return;

		if ((_vm->_groupMan->groupGetThing(destinationMapX, destinationMapY) != Thing::_endOfList) && hasProjectileImpactOccurred(kM1_CreatureElemType, destinationMapX, destinationMapY, projectileCurCell, projectileThing))
			return;

		uint16 stepEnergy = curEvent->_C._projectile.getStepEnergy();
		if (projectile->_kineticEnergy <= stepEnergy) {
			_vm->_dungeonMan->unlinkThingFromList(projectileThingNewCell = projectileThing, Thing(0), destinationMapX, destinationMapY);
			projectileDelete(projectileThingNewCell, NULL, destinationMapX, destinationMapY);
			return;
		}
		projectile->_kineticEnergy -= stepEnergy;
		if (projectile->_attack < stepEnergy)
			projectile->_attack = 0;
		else
			projectile->_attack -= stepEnergy;
	}
	uint16 projectileDirection = curEvent->_C._projectile.getDir();
	projectileThingNewCell = Thing(curEvent->_B._slot);
	uint16 projectileNewCell = projectileThingNewCell.getCell();
	bool projectileMovesToOtherSquare = (projectileDirection == projectileNewCell) || (returnNextVal(projectileDirection) == projectileNewCell);
	if (projectileMovesToOtherSquare) {
		sourceMapX = destinationMapX;
		sourceMapY = destinationMapY;
		destinationMapX += _vm->_dirIntoStepCountEast[projectileDirection], destinationMapY += _vm->_dirIntoStepCountNorth[projectileDirection];
		Square destSquare = _vm->_dungeonMan->getSquare(destinationMapX, destinationMapY);
		SquareType destSquareType = destSquare.getType();
		if ((destSquareType == k0_WallElemType) ||
			((destSquareType == k6_FakeWallElemType) && !getFlag(destSquare.toByte(), (k0x0001_FakeWallImaginary | k0x0004_FakeWallOpen))) ||
			((destSquareType == k3_StairsElemType) && (Square(_vm->_dungeonMan->_currMapData[sourceMapX][sourceMapY]).getType() == k3_StairsElemType))) {
			if (hasProjectileImpactOccurred(destSquare.getType(), sourceMapX, sourceMapY, projectileNewCell, projectileThingNewCell)) {
				return;
			}
		}
	}

	if ((projectileDirection & 0x0001) == (projectileNewCell & 0x0001))
		projectileNewCell--;
	else
		projectileNewCell++;

	projectileThingNewCell = thingWithNewCell(projectileThingNewCell, projectileNewCell &= 0x0003);
	if (projectileMovesToOtherSquare) {
		_vm->_moveSens->getMoveResult(projectileThingNewCell, sourceMapX, sourceMapY, destinationMapX, destinationMapY);
		curEvent->_C._projectile.setMapX(_vm->_moveSens->_moveResultMapX);
		curEvent->_C._projectile.setMapY(_vm->_moveSens->_moveResultMapY);
		curEvent->_C._projectile.setDir((Direction)_vm->_moveSens->_moveResultDir);
		projectileThingNewCell = thingWithNewCell(projectileThingNewCell, _vm->_moveSens->_moveResultCell);
		M31_setMap(curEvent->_mapTime, _vm->_moveSens->_moveResultMapIndex);
	} else {
		if ((Square(_vm->_dungeonMan->getSquare(destinationMapX, destinationMapY)).getType() == k4_DoorElemType) && hasProjectileImpactOccurred(k4_DoorElemType, destinationMapX, destinationMapY, projectileNewCell, projectileThing))
			return;

		_vm->_dungeonMan->unlinkThingFromList(projectileThingNewCell, Thing(0), destinationMapX, destinationMapY);
		_vm->_dungeonMan->linkThingToList(projectileThingNewCell, Thing(0), destinationMapX, destinationMapY);
	}

	// This code is from CSB20. The projectiles move at the same speed on all maps instead of moving slower on maps other than the party map */
	curEvent->_mapTime++;

	curEvent->_B._slot = projectileThingNewCell.toUint16();
	projectile->_eventIndex = _vm->_timeline->addEventGetEventIndex(curEvent);
}

void ProjExpl::processEvent25(TimelineEvent *event) {
	uint16 mapX = event->_B._location._mapX;
	uint16 mapY = event->_B._location._mapY;
	Explosion *explosion = &((Explosion *)_vm->_dungeonMan->_thingData[k15_ExplosionThingType])[Thing((event->_C._slot)).getIndex()];
	int16 curSquareType = Square(_vm->_dungeonMan->_currMapData[mapX][mapY]).getType();
	bool explosionOnPartySquare = (_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) && (mapX == _vm->_dungeonMan->_partyMapX) && (mapY == _vm->_dungeonMan->_partyMapY);
	Thing groupThing = _vm->_groupMan->groupGetThing(mapX, mapY);

	Group *group = nullptr;
	CreatureInfo *creatureInfo = nullptr;

	uint16 creatureType = 0;
	if (groupThing != Thing::_endOfList) {
		group = (Group *)_vm->_dungeonMan->getThingData(groupThing);
		creatureType = group->_type;
		creatureInfo = &_vm->_dungeonMan->_creatureInfos[creatureType];
	}

	Thing explosionThing = Thing(Thing::_firstExplosion.toUint16() + explosion->getType());
	int16 attack;
	if (explosionThing == Thing::_explPoisonCloud)
		attack = MAX(1, MIN(explosion->getAttack() >> 5, 4) + _vm->getRandomNumber(2)); /* Value between 1 and 5 */
	else {
		attack = (explosion->getAttack() >> 1) + 1;
		attack += _vm->getRandomNumber(attack) + 1;
	}

	bool AddEventFl = false;

	switch (explosionThing.toUint16()) {
	case 0xFF82:
		if (!(attack >>= 1))
			break;
	case 0xFF80:
		if (curSquareType == k4_DoorElemType)
			_vm->_groupMan->groupIsDoorDestoryedByAttack(mapX, mapY, attack, true, 0);

		break;
	case 0xFF83:
		if ((groupThing != Thing::_endOfList) && getFlag(creatureInfo->_attributes, k0x0040_MaskCreatureInfo_nonMaterial)) {
			if ((creatureType == k19_CreatureTypeMaterializerZytaz) && (_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex)) {
				int16 nonMaterialAdditionalAttack = attack >> 3;
				attack -= nonMaterialAdditionalAttack;
				nonMaterialAdditionalAttack <<= 1;
				nonMaterialAdditionalAttack++;
				int16 creatureCount = group->getCount();
				do {
					if (getFlag(_vm->_groupMan->_activeGroups[group->getActiveGroupIndex()]._aspect[creatureCount], k0x0080_MaskActiveGroupIsAttacking)) /* Materializer / Zytaz can only be damaged while they are attacking */
						_vm->_groupMan->groupGetDamageCreatureOutcome(group, creatureCount, mapX, mapY, attack + _vm->getRandomNumber(nonMaterialAdditionalAttack) + _vm->getRandomNumber(4), true);
				} while (--creatureCount >= 0);
			} else
				_vm->_groupMan->getDamageAllCreaturesOutcome(group, mapX, mapY, attack, true);
		}
		break;
	case 0xFFE4:
		explosion->setType(explosion->getType() + 1);
		_vm->_sound->requestPlay(k05_soundSTRONG_EXPLOSION, mapX, mapY, k1_soundModePlayIfPrioritized);
		AddEventFl = true;
		break;
	case 0xFFA8:
		if (explosion->getAttack() > 55) {
			explosion->setAttack(explosion->getAttack() - 40);
			AddEventFl = true;
		}
		break;
	case 0xFF87:
		if (explosionOnPartySquare)
			_vm->_championMan->getDamagedChampionCount(attack, kDMChampionWoundNone, k0_attackType_NORMAL);
		else if ((groupThing != Thing::_endOfList)
			&& (attack = _vm->_groupMan->groupGetResistanceAdjustedPoisonAttack(creatureType, attack))
			&& (_vm->_groupMan->getDamageAllCreaturesOutcome(group, mapX, mapY, attack, true) != k2_outcomeKilledAllCreaturesInGroup)
			&& (attack > 2)) {
			_vm->_groupMan->processEvents29to41(mapX, mapY, kM3_TMEventTypeCreateReactionEvent29DangerOnSquare, 0);
		}
		if (explosion->getAttack() >= 6) {
			explosion->setAttack(explosion->getAttack() - 3);
			AddEventFl = true;
		}
		break;
	}
	if (AddEventFl) {
		TimelineEvent newEvent;
		newEvent = *event;
		newEvent._mapTime++;
		_vm->_timeline->addEventGetEventIndex(&newEvent);
	} else {
		_vm->_dungeonMan->unlinkThingFromList(Thing(event->_C._slot), Thing(0), mapX, mapY);
		explosion->setNextThing(Thing::_none);
	}
}
}
