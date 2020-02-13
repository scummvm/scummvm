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
	Thing projectileThing = _vm->_dungeonMan->getUnusedThing(kDMThingTypeProjectile);
	if (projectileThing == _vm->_thingNone) /* BUG0_16 If the game cannot create a projectile thing because it has run out of such things (60 maximum) then the object being thrown/shot/launched is orphaned. If the game has run out of projectile things it will try to remove a projectile from elsewhere in the dungeon, except in an area of 11x11 squares centered around the party (to make sure the player cannot actually see the thing disappear on screen) */
		return;

	projectileThing = _vm->thingWithNewCell(projectileThing, cell);
	Projectile *projectilePtr = (Projectile *)_vm->_dungeonMan->getThingData(projectileThing);
	projectilePtr->_slot = thing;
	projectilePtr->_kineticEnergy = MIN((int16)kineticEnergy, (int16)255);
	projectilePtr->_attack = attack;
	_vm->_dungeonMan->linkThingToList(projectileThing, Thing(0), mapX, mapY); /* Projectiles are added on the square and not 'moved' onto the square. In the case of a projectile launcher sensor, this means that the new projectile traverses the square in front of the launcher without any trouble: there is no impact if it is a wall, the projectile direction is not changed if it is a teleporter. Impacts with creatures and champions are still processed */
	TimelineEvent newEvent;
	newEvent._mapTime = _vm->setMapAndTime(_vm->_dungeonMan->_currMapIndex, _vm->_gameTime + 1);
	if (_createLauncherProjectile)
		newEvent._type = kDMEventTypeMoveProjectile; /* Launcher projectiles can impact immediately */
	else
		newEvent._type = kDMEventTypeMoveProjectileIgnoreImpacts; /* Projectiles created by champions or creatures ignore impacts on their first movement */

	newEvent._priority = 0;
	newEvent._Bu._slot = projectileThing.toUint16();
	newEvent._Cu._projectile.setMapX(mapX);
	newEvent._Cu._projectile.setMapY(mapY);
	newEvent._Cu._projectile.setStepEnergy(stepEnergy);
	newEvent._Cu._projectile.setDir(dir);
	projectilePtr->_eventIndex = _vm->_timeline->addEventGetEventIndex(&newEvent);
}

bool ProjExpl::hasProjectileImpactOccurred(int16 impactType, int16 mapXCombo, int16 mapYCombo, int16 cell, Thing projectileThing) {
	Projectile *projectileThingData = (Projectile *)_vm->_dungeonMan->getThingData(Thing(projectileThing));
	bool removePotion = false;
	int16 potionPower = 0;
	_creatureDamageOutcome = kDMKillOutcomeNoCreaturesInGroup;
	Thing projectileAssociatedThing = projectileThingData->_slot;
	int16 projectileAssociatedThingType = projectileAssociatedThing.getType();
	Potion *potion = nullptr;
	Thing explosionThing = _vm->_thingNone;
	if (projectileAssociatedThingType == kDMThingTypePotion) {
		Group *projectileAssociatedGroup = (Group *)_vm->_dungeonMan->getThingData(projectileAssociatedThing);
		PotionType potionType = ((Potion *)projectileAssociatedGroup)->getType();
		if ((potionType == kDMPotionTypeVen) || (potionType == kDMPotionTypeFulBomb)) {
			explosionThing = (potionType == kDMPotionTypeVen) ? _vm->_thingExplPoisonCloud: _vm->_thingExplFireBall;
			removePotion = true;
			potionPower = ((Potion *)projectileAssociatedGroup)->getPower();
			potion = (Potion *)projectileAssociatedGroup;
		}
	}
	bool createExplosionOnImpact = (projectileAssociatedThingType == kDMThingTypeExplosion) && (projectileAssociatedThing != _vm->_thingExplSlime) && (projectileAssociatedThing != _vm->_thingExplPoisonBolt);
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
	case kDMElementTypeDoor: {
		byte curSquare = _vm->_dungeonMan->_currMapData[projectileTargetMapX][projectileTargetMapY];
		int16 curDoorState = Square(curSquare).getDoorState();
		Door *curDoor = (Door *)_vm->_dungeonMan->getSquareFirstThingData(projectileTargetMapX, projectileTargetMapY);
		if ((curDoorState != kDMDoorStateDestroyed) && (projectileAssociatedThing == _vm->_thingExplOpenDoor)) {
			if (curDoor->hasButton())
				_vm->_moveSens->addEvent(kDMEventTypeDoor, projectileTargetMapX, projectileTargetMapY, kDMCellNorthWest, kDMSensorEffectToggle, _vm->_gameTime + 1);
			break;
		}

		if ((curDoorState == kDMDoorStateDestroyed) || (curDoorState <= kDMDoorStateOneFourth))
			return false;

		DoorInfo curDoorInfo = _vm->_dungeonMan->_currMapDoorInfo[curDoor->getType()];
		if (getFlag(curDoorInfo._attributes, kDMMaskDoorInfoProjectilesCanPassThrough)) {
			if (projectileAssociatedThingType == kDMThingTypeExplosion) {
				if (projectileAssociatedThing.toUint16() >= _vm->_thingExplHarmNonMaterial.toUint16())
					return false;
			} else {
				int16 associatedThingIndex = _vm->_dungeonMan->getObjectInfoIndex(projectileAssociatedThing);
				uint16 associatedAllowedSlots = _vm->_dungeonMan->_objectInfos[associatedThingIndex].getAllowedSlots();
				int16 iconIndex = _vm->_objectMan->getIconIndex(projectileAssociatedThing);

				if ((projectileThingData->_attack > _vm->getRandomNumber(128))
				&& getFlag(associatedAllowedSlots, kDMMaskPouchPassAndThroughDoors)
				&& (   (projectileAssociatedThingType != kDMThingTypeJunk)
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
	case kDMElementTypeChampion:
		championIndex = _vm->_championMan->getIndexInCell(cell);
		if (championIndex < 0)
			return false;

		championAttack = attack = getProjectileImpactAttack(projectileThingData, projectileAssociatedThing);
		break;
	case kDMElementTypeCreature: {
		Group *curGroup = (Group *)_vm->_dungeonMan->getThingData(_vm->_groupMan->groupGetThing(projectileTargetMapX, projectileTargetMapY));
		uint16 curCreatureIndex = _vm->_groupMan->getCreatureOrdinalInCell(curGroup, cell);
		if (!curCreatureIndex)
			return false;

		curCreatureIndex--;
		CreatureType curCreatureType = curGroup->_type;
		CreatureInfo *curCreatureInfo = &_vm->_dungeonMan->_creatureInfos[curCreatureType];
		if ((projectileAssociatedThing == _vm->_thingExplFireBall) && (curCreatureType == kDMCreatureTypeBlackFlame)) {
			uint16 *curCreatureHealth = &curGroup->_health[curCreatureIndex];
			*curCreatureHealth = MIN(1000, *curCreatureHealth + getProjectileImpactAttack(projectileThingData, projectileAssociatedThing));
			goto T0217044;
		}
		if (getFlag(curCreatureInfo->_attributes, kDMCreatureMaskNonMaterial) && (projectileAssociatedThing != _vm->_thingExplHarmNonMaterial))
			return false;

		attack = (uint16)((unsigned long)getProjectileImpactAttack(projectileThingData, projectileAssociatedThing) << 6) / curCreatureInfo->_defense;
		if (attack) {
			int16 outcome = _vm->_groupMan->groupGetDamageCreatureOutcome(curGroup, curCreatureIndex, projectileTargetMapX, projectileTargetMapY, attack + _vm->_groupMan->groupGetResistanceAdjustedPoisonAttack(curCreatureType, _projectilePoisonAttack), true);
			if (outcome != kDMKillOutcomeNoCreaturesInGroup)
				_vm->_groupMan->processEvents29to41(projectileTargetMapX, projectileTargetMapY, kDMEventTypeCreateReactionHitByProjectile, 0);

			_creatureDamageOutcome = outcome;
			if (!createExplosionOnImpact && (outcome == kDMKillOutcomeNoCreaturesInGroup)
			&& (projectileAssociatedThingType == kDMThingTypeWeapon)
			&& getFlag(curCreatureInfo->_attributes, kDMCreatureMaskKeepThrownSharpWeapon)) {
				Weapon *weapon = (Weapon *)_vm->_dungeonMan->getThingData(projectileAssociatedThing);
				WeaponType weaponType = weapon->getType();
				if ((weaponType == kDMWeaponDagger) || (weaponType == kDMWeaponArrow)
				|| (weaponType == kDMWeaponSlayer) || (weaponType == kDMWeaponPoisonDart)
				|| (weaponType == kDMWeaponThrowingStar))
					curGroupSlot = &curGroup->_slot;
			}
		}
		}
		break;
	default:
		break;
	}
	if (championAttack && _projectilePoisonAttack && _vm->getRandomNumber(2)
	&& _vm->_championMan->addPendingDamageAndWounds_getDamage(championIndex, attack, kDMWoundHead | kDMWoundTorso, _projectileAttackType))
		_vm->_championMan->championPoison(championIndex, _projectilePoisonAttack);

	if (createExplosionOnImpact || removePotion) {
		uint16 explosionAttack;
		if (removePotion) {
			projectileAssociatedThing = explosionThing;
			explosionAttack = potionPower;
		} else {
			explosionAttack = projectileThingData->_kineticEnergy;
		}
		if ((projectileAssociatedThing == _vm->_thingExplLightningBolt) && !(explosionAttack >>= 1))
			goto T0217044;
		createExplosion(projectileAssociatedThing, explosionAttack, mapXCombo, mapYCombo, (projectileAssociatedThing == _vm->_thingExplPoisonCloud) ? (uint16)kDMCreatureTypeSingleCenteredCreature : cell);
	} else {
		uint16 soundIndex;
		if ((projectileAssociatedThing).getType() == kDMThingTypeWeapon)
			soundIndex = kDMSoundIndexMetallicThud;
		else if (projectileAssociatedThing == _vm->_thingExplPoisonBolt)
			soundIndex = kDMSoundIndexSpell;
		else
			soundIndex = kDMSoundIndexWoodenThudAttackTrolinAntmanStoneGolem;

		_vm->_sound->requestPlay(soundIndex, projectileMapX, projectileMapY, kDMSoundModePlayIfPrioritized);
	}
T0217044:
	if (removePotion) {
		potion->_nextThing = _vm->_thingNone;
		projectileThingData->_slot = explosionThing;
	}
	_vm->_dungeonMan->unlinkThingFromList(projectileThing, Thing(0), projectileMapX, projectileMapY);
	projectileDelete(projectileThing, curGroupSlot, projectileMapX, projectileMapY);
	return true;
}

uint16 ProjExpl::getProjectileImpactAttack(Projectile *projectile, Thing thing) {
	_projectilePoisonAttack = 0;
	_projectileAttackType = kDMAttackTypeBlunt;

	uint16 kineticEnergy = projectile->_kineticEnergy;
	ThingType thingType = thing.getType();
	uint16 attack;
	if (thingType != kDMThingTypeExplosion) {
		if (thingType == kDMThingTypeWeapon) {
			WeaponInfo *weaponInfo = _vm->_dungeonMan->getWeaponInfo(thing);
			attack = weaponInfo->_kineticEnergy;
			_projectileAttackType = kDMAttackTypeBlunt;
		} else
			attack = _vm->getRandomNumber(4);

		attack += _vm->_dungeonMan->getObjectWeight(thing) >> 1;
	} else if (thing == _vm->_thingExplSlime) {
		attack = _vm->getRandomNumber(16);
		_projectilePoisonAttack = attack + 10;
		attack += _vm->getRandomNumber(32);
	} else {
		if (thing.toUint16() >= _vm->_thingExplHarmNonMaterial.toUint16()) {
			_projectileAttackType = kDMAttackTypeMagic;
			if (thing == _vm->_thingExplPoisonBolt) {
				_projectilePoisonAttack = kineticEnergy;
				return 1;
			}
			return 0;
		}
		_projectileAttackType = kDMAttackTypeFire;
		attack = _vm->getRandomNumber(16) + _vm->getRandomNumber(16) + 10;
		if (thing == _vm->_thingExplLightningBolt) {
			_projectileAttackType = kDMAttackTypeLightning;
			attack *= 5;
		}
	}
	attack = ((attack + kineticEnergy) >> 4) + 1;
	attack += _vm->getRandomNumber((attack >> 1) + 1) + _vm->getRandomNumber(4);
	attack = MAX(attack >> 1, attack - (32 - (projectile->_attack >> 3)));
	return attack;
}

void ProjExpl::createExplosion(Thing explThing, uint16 attack, uint16 mapXCombo, uint16 mapYCombo, uint16 cell) {
	Thing unusedThing = _vm->_dungeonMan->getUnusedThing(kDMThingTypeExplosion);
	if (unusedThing == _vm->_thingNone)
		return;

	Explosion *explosion = &((Explosion *)_vm->_dungeonMan->_thingData[kDMThingTypeExplosion])[(unusedThing).getIndex()];
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

	if (cell == kDMCreatureTypeSingleCenteredCreature)
		explosion->setCentered(true);
	else {
		explosion->setCentered(false);
		unusedThing = _vm->thingWithNewCell(unusedThing, cell);
	}

	explosion->setType(explThing.toUint16() - _vm->_thingFirstExplosion.toUint16());
	explosion->setAttack(attack);
	if (explThing.toUint16() < _vm->_thingExplHarmNonMaterial.toUint16()) {
		uint16 soundIndex = (attack > 80) ? kDMSoundIndexStrongExplosion : kDMSoundIndexWeakExplosion;
		_vm->_sound->requestPlay(soundIndex, projectileMapX, projectileMapY, kDMSoundModePlayIfPrioritized);
	} else if (explThing != _vm->_thingExplSmoke)
		_vm->_sound->requestPlay(kDMSoundIndexSpell, projectileMapX, projectileMapY, kDMSoundModePlayIfPrioritized);

	_vm->_dungeonMan->linkThingToList(unusedThing, Thing(0), projectileMapX, projectileMapY);
	TimelineEvent newEvent;
	newEvent._mapTime = _vm->setMapAndTime(_vm->_dungeonMan->_currMapIndex, _vm->_gameTime + ((explThing == _vm->_thingExplRebirthStep1) ? 5 : 1));
	newEvent._type = kDMEventTypeExplosion;
	newEvent._priority = 0;
	newEvent._Cu._slot = unusedThing.toUint16();
	newEvent._Bu._location._mapX = projectileMapX;
	newEvent._Bu._location._mapY = projectileMapY;
	_vm->_timeline->addEventGetEventIndex(&newEvent);
	if ((explThing == _vm->_thingExplLightningBolt) || (explThing == _vm->_thingExplFireBall)) {
		projectileMapX = projectileTargetMapX;
		projectileMapY = projectileTargetMapY;
		attack = (attack >> 1) + 1;
		attack += _vm->getRandomNumber(attack) + 1;
		if ((explThing == _vm->_thingExplFireBall) || (attack >>= 1)) {
			if ((_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) && (projectileMapX == _vm->_dungeonMan->_partyMapX) && (projectileMapY == _vm->_dungeonMan->_partyMapY)) {
				int16 wounds = kDMWoundReadHand | kDMWoundActionHand | kDMWoundHead | kDMWoundTorso | kDMWoundLegs | kDMWoundFeet;
				_vm->_championMan->getDamagedChampionCount(attack, wounds, kDMAttackTypeFire);
			} else {
				unusedThing = _vm->_groupMan->groupGetThing(projectileMapX, projectileMapY);
				if (unusedThing != _vm->_thingEndOfList) {
					Group *creatureGroup = (Group *)_vm->_dungeonMan->getThingData(unusedThing);
					CreatureInfo *creatureInfo = &_vm->_dungeonMan->_creatureInfos[creatureGroup->_type];
					int16 creatureFireResistance = creatureInfo->getFireResistance();
					if (creatureFireResistance != kDMImmuneToFire) {
						if (getFlag(creatureInfo->_attributes, kDMCreatureMaskNonMaterial))
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
	_creatureDamageOutcome = kDMKillOutcomeNoCreaturesInGroup;

	for (Thing curThing = _vm->_dungeonMan->getSquareFirstThing(mapX, mapY); curThing != _vm->_thingEndOfList; ) {
		if (((curThing).getType() == kDMThingTypeProjectile) && ((curThing).getCell() == cell) &&
			hasProjectileImpactOccurred(impactType, mapX, mapY, cell, curThing)) {
			projectileDeleteEvent(curThing);
			impactCount++;
			if ((impactType == kDMElementTypeCreature) && (_creatureDamageOutcome == kDMKillOutcomeAllCreaturesInGroup))
				break;

			curThing = _vm->_dungeonMan->getSquareFirstThing(mapX, mapY);
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
	if (projectileSlotThing.getType() != kDMThingTypeExplosion) {
		if (groupSlot != NULL) {
			Thing previousThing = *groupSlot;
			if (previousThing == _vm->_thingEndOfList) {
				Thing *genericThing = (Thing *)_vm->_dungeonMan->getThingData(projectileSlotThing);
				*genericThing = _vm->_thingEndOfList;
				*groupSlot = projectileSlotThing;
			} else
				_vm->_dungeonMan->linkThingToList(projectileSlotThing, previousThing, kDMMapXNotOnASquare, 0);
		} else
			_vm->_moveSens->getMoveResult(Thing((projectileSlotThing).getTypeAndIndex() | getFlag(projectileThing.toUint16(), 0xC)), -2, 0, mapX, mapY);
	}
	projectile->_nextThing = _vm->_thingNone;
}

void ProjExpl::processEvents48To49(TimelineEvent *event) {
	int16 sourceMapX = -1;
	int16 sourceMapY = -1;
	TimelineEvent firstEvent = *event;
	TimelineEvent *curEvent = &firstEvent;
	Thing projectileThingNewCell = Thing(curEvent->_Bu._slot);
	Thing projectileThing  = projectileThingNewCell;
	Projectile *projectile = (Projectile *)_vm->_dungeonMan->getThingData(projectileThing);
	int16 destinationMapX = curEvent->_Cu._projectile.getMapX();
	int16 destinationMapY = curEvent->_Cu._projectile.getMapY();

	if (curEvent->_type == kDMEventTypeMoveProjectileIgnoreImpacts)
		curEvent->_type = kDMEventTypeMoveProjectile;
	else {
		uint16 projectileCurCell = projectileThingNewCell.getCell();
		if ((_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) && (destinationMapX == _vm->_dungeonMan->_partyMapX) && (destinationMapY == _vm->_dungeonMan->_partyMapY) && hasProjectileImpactOccurred(kDMElementTypeChampion, destinationMapX, destinationMapY, projectileCurCell, projectileThingNewCell))
			return;

		if ((_vm->_groupMan->groupGetThing(destinationMapX, destinationMapY) != _vm->_thingEndOfList) && hasProjectileImpactOccurred(kDMElementTypeCreature, destinationMapX, destinationMapY, projectileCurCell, projectileThing))
			return;

		uint16 stepEnergy = curEvent->_Cu._projectile.getStepEnergy();
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
	uint16 projectileDirection = curEvent->_Cu._projectile.getDir();
	projectileThingNewCell = Thing(curEvent->_Bu._slot);
	uint16 projectileNewCell = projectileThingNewCell.getCell();
	bool projectileMovesToOtherSquare = (projectileDirection == projectileNewCell) || (_vm->turnDirRight(projectileDirection) == projectileNewCell);
	if (projectileMovesToOtherSquare) {
		sourceMapX = destinationMapX;
		sourceMapY = destinationMapY;
		destinationMapX += _vm->_dirIntoStepCountEast[projectileDirection];
		destinationMapY += _vm->_dirIntoStepCountNorth[projectileDirection];
		Square destSquare = _vm->_dungeonMan->getSquare(destinationMapX, destinationMapY);
		ElementType destSquareType = destSquare.getType();
		if ((destSquareType == kDMElementTypeWall) ||
			((destSquareType == kDMElementTypeFakeWall) && !getFlag(destSquare.toByte(), (kDMSquareMaskFakeWallImaginary | kDMSquareMaskFakeWallOpen))) ||
			((destSquareType == kDMElementTypeStairs) && (Square(_vm->_dungeonMan->_currMapData[sourceMapX][sourceMapY]).getType() == kDMElementTypeStairs))) {
			if (hasProjectileImpactOccurred(destSquare.getType(), sourceMapX, sourceMapY, projectileNewCell, projectileThingNewCell)) {
				return;
			}
		}
	}

	if ((projectileDirection & 0x0001) == (projectileNewCell & 0x0001))
		projectileNewCell--;
	else
		projectileNewCell++;

	projectileThingNewCell = _vm->thingWithNewCell(projectileThingNewCell, projectileNewCell &= 0x0003);
	if (projectileMovesToOtherSquare) {
		_vm->_moveSens->getMoveResult(projectileThingNewCell, sourceMapX, sourceMapY, destinationMapX, destinationMapY);
		curEvent->_Cu._projectile.setMapX(_vm->_moveSens->_moveResultMapX);
		curEvent->_Cu._projectile.setMapY(_vm->_moveSens->_moveResultMapY);
		curEvent->_Cu._projectile.setDir((Direction)_vm->_moveSens->_moveResultDir);
		projectileThingNewCell = _vm->thingWithNewCell(projectileThingNewCell, _vm->_moveSens->_moveResultCell);
		_vm->setMap(curEvent->_mapTime, _vm->_moveSens->_moveResultMapIndex);
	} else {
		if ((Square(_vm->_dungeonMan->getSquare(destinationMapX, destinationMapY)).getType() == kDMElementTypeDoor) && hasProjectileImpactOccurred(kDMElementTypeDoor, destinationMapX, destinationMapY, projectileNewCell, projectileThing))
			return;

		_vm->_dungeonMan->unlinkThingFromList(projectileThingNewCell, Thing(0), destinationMapX, destinationMapY);
		_vm->_dungeonMan->linkThingToList(projectileThingNewCell, Thing(0), destinationMapX, destinationMapY);
	}

	// This code is from CSB20. The projectiles move at the same speed on all maps instead of moving slower on maps other than the party map */
	curEvent->_mapTime++;

	curEvent->_Bu._slot = projectileThingNewCell.toUint16();
	projectile->_eventIndex = _vm->_timeline->addEventGetEventIndex(curEvent);
}

void ProjExpl::processEvent25(TimelineEvent *event) {
	uint16 mapX = event->_Bu._location._mapX;
	uint16 mapY = event->_Bu._location._mapY;
	Explosion *explosion = &((Explosion *)_vm->_dungeonMan->_thingData[kDMThingTypeExplosion])[Thing((event->_Cu._slot)).getIndex()];
	int16 curSquareType = Square(_vm->_dungeonMan->_currMapData[mapX][mapY]).getType();
	bool explosionOnPartySquare = (_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) && (mapX == _vm->_dungeonMan->_partyMapX) && (mapY == _vm->_dungeonMan->_partyMapY);
	Thing groupThing = _vm->_groupMan->groupGetThing(mapX, mapY);

	Group *group = nullptr;
	CreatureInfo *creatureInfo = nullptr;

	CreatureType creatureType;
	creatureType = kDMCreatureTypeGiantScorpion; // Value of 0 as default to avoid possible uninitialized usage
	if (groupThing != _vm->_thingEndOfList) {
		group = (Group *)_vm->_dungeonMan->getThingData(groupThing);
		creatureType = group->_type;
		creatureInfo = &_vm->_dungeonMan->_creatureInfos[creatureType];
	}

	Thing explosionThing = Thing(_vm->_thingFirstExplosion.toUint16() + explosion->getType());
	int16 attack;
	if (explosionThing == _vm->_thingExplPoisonCloud)
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
		// fall through
	case 0xFF80:
		if (curSquareType == kDMElementTypeDoor)
			_vm->_groupMan->groupIsDoorDestoryedByAttack(mapX, mapY, attack, true, 0);
		break;
	case 0xFF83:
		if ((groupThing != _vm->_thingEndOfList) && getFlag(creatureInfo->_attributes, kDMCreatureMaskNonMaterial)) {
			if ((creatureType == kDMCreatureTypeMaterializerZytaz) && (_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex)) {
				int16 nonMaterialAdditionalAttack = attack >> 3;
				attack -= nonMaterialAdditionalAttack;
				nonMaterialAdditionalAttack <<= 1;
				nonMaterialAdditionalAttack++;
				int16 creatureCount = group->getCount();
				do {
					if (getFlag(_vm->_groupMan->_activeGroups[group->getActiveGroupIndex()]._aspect[creatureCount], kDMAspectMaskActiveGroupIsAttacking)) /* Materializer / Zytaz can only be damaged while they are attacking */
						_vm->_groupMan->groupGetDamageCreatureOutcome(group, creatureCount, mapX, mapY, attack + _vm->getRandomNumber(nonMaterialAdditionalAttack) + _vm->getRandomNumber(4), true);
				} while (--creatureCount >= 0);
			} else
				_vm->_groupMan->getDamageAllCreaturesOutcome(group, mapX, mapY, attack, true);
		}
		break;
	case 0xFFE4:
		explosion->setType(explosion->getType() + 1);
		_vm->_sound->requestPlay(kDMSoundIndexStrongExplosion, mapX, mapY, kDMSoundModePlayIfPrioritized);
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
			_vm->_championMan->getDamagedChampionCount(attack, kDMWoundNone, kDMAttackTypeNormal);
		else if ((groupThing != _vm->_thingEndOfList)
			&& (attack = _vm->_groupMan->groupGetResistanceAdjustedPoisonAttack(creatureType, attack))
			&& (_vm->_groupMan->getDamageAllCreaturesOutcome(group, mapX, mapY, attack, true) != kDMKillOutcomeAllCreaturesInGroup)
			&& (attack > 2)) {
			_vm->_groupMan->processEvents29to41(mapX, mapY, kDMEventTypeCreateReactionDangerOnSquare, 0);
		}
		if (explosion->getAttack() >= 6) {
			explosion->setAttack(explosion->getAttack() - 3);
			AddEventFl = true;
		}
		break;
	default:
		break;
	}
	if (AddEventFl) {
		TimelineEvent newEvent;
		newEvent = *event;
		newEvent._mapTime++;
		_vm->_timeline->addEventGetEventIndex(&newEvent);
	} else {
		_vm->_dungeonMan->unlinkThingFromList(Thing(event->_Cu._slot), Thing(0), mapX, mapY);
		explosion->setNextThing(_vm->_thingNone);
	}
}
}
