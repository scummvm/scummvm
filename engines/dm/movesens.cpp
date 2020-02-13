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

#include "dm/movesens.h"
#include "dm/champion.h"
#include "dm/inventory.h"
#include "dm/dungeonman.h"
#include "dm/objectman.h"
#include "dm/timeline.h"
#include "dm/group.h"
#include "dm/projexpl.h"
#include "dm/text.h"
#include "dm/sounds.h"

namespace DM {

MovesensMan::MovesensMan(DMEngine *vm) : _vm(vm) {
	_moveResultMapX = 0;
	_moveResultMapY = 0;
	_moveResultMapIndex = 0;
	_moveResultDir = 0;
	_moveResultCell = 0;
	_useRopeToClimbDownPit = false;
	_sensorRotationEffect = 0;
	_sensorRotationEffMapX = 0;
	_sensorRotationEffMapY = 0;
	_sensorRotationEffCell = 0;
}

bool MovesensMan::sensorIsTriggeredByClickOnWall(int16 mapX, int16 mapY, uint16 cellParam) {
	DungeonMan &dungeon = *_vm->_dungeonMan;

	bool atLeastOneSensorWasTriggered = false;
	Thing leaderHandObject = _vm->_championMan->_leaderHandObject;
	int16 sensorCountToProcessPerCell[4];
	for (int16 i = kDMCellNorthWest; i < kDMCellSouthWest + 1; i++)
		sensorCountToProcessPerCell[i] = 0;

	Thing squareFirstThing = dungeon.getSquareFirstThing(mapX, mapY);
	Thing thingBeingProcessed = squareFirstThing;
	while (thingBeingProcessed != _vm->_thingEndOfList) {
		ThingType thingType = thingBeingProcessed.getType();
		if (thingType == kDMThingTypeSensor)
			sensorCountToProcessPerCell[thingBeingProcessed.getCell()]++;
		else if (thingType >= kDMThingTypeGroup)
			break;

		thingBeingProcessed = dungeon.getNextThing(thingBeingProcessed);
	}
	for (thingBeingProcessed = squareFirstThing; thingBeingProcessed != _vm->_thingEndOfList; thingBeingProcessed = dungeon.getNextThing(thingBeingProcessed)) {
		Thing lastProcessedThing = thingBeingProcessed;
		uint16 ProcessedThingType = thingBeingProcessed.getType();
		if (ProcessedThingType == kDMThingTypeSensor) {
			int16 cellIdx = thingBeingProcessed.getCell();
			sensorCountToProcessPerCell[cellIdx]--;
			Sensor *currentSensor = (Sensor *)dungeon.getThingData(thingBeingProcessed);
			SensorType processedSensorType = currentSensor->getType();
			if (processedSensorType == kDMSensorDisabled)
				continue;

			if ((_vm->_championMan->_leaderIndex == kDMChampionNone) && (processedSensorType != kDMSensorWallChampionPortrait))
				continue;

			if (cellIdx != cellParam)
				continue;

			bool doNotTriggerSensor;
			int16 sensorData = currentSensor->getData();
			SensorEffect sensorEffect = (SensorEffect)currentSensor->getAttrEffectA();

			switch (processedSensorType) {
			case kDMSensorWallOrnClick:
				doNotTriggerSensor = false;
				if (currentSensor->getAttrEffectA() == kDMSensorEffectHold)
					continue;
				break;
			case kDMSensorWallOrnClickWithAnyObj:
				doNotTriggerSensor = (_vm->_championMan->_leaderEmptyHanded != currentSensor->getAttrRevertEffectA());
				break;
			case kDMSensorWallOrnClickWithSpecObjRemovedSensor:
			case kDMSensorWallOrnClickWithSpecObjRemovedRotateSensors:
				if (sensorCountToProcessPerCell[cellIdx]) /* If the sensor is not the last one of its type on the cell */
					continue;
				// fall through
			case kDMSensorWallOrnClickWithSpecObj:
			case kDMSensorWallOrnClickWithSpecObjRemoved:
				doNotTriggerSensor = ((sensorData == _vm->_objectMan->getObjectType(leaderHandObject)) == currentSensor->getAttrRevertEffectA());
				if (!doNotTriggerSensor && (processedSensorType == kDMSensorWallOrnClickWithSpecObjRemovedSensor)) {
					if (lastProcessedThing == thingBeingProcessed) /* If the sensor is the only one of its type on the cell */
						break;
					Sensor *lastSensor = (Sensor *)dungeon.getThingData(lastProcessedThing);
					lastSensor->setNextThing(currentSensor->getNextThing());
					currentSensor->setNextThing(_vm->_thingNone);
					thingBeingProcessed = lastProcessedThing;
				}

				if (!doNotTriggerSensor && (processedSensorType == kDMSensorWallOrnClickWithSpecObjRemovedRotateSensors))
					triggerLocalEffect(kDMSensorEffectToggle, mapX, mapY, cellIdx); /* This will cause a rotation of the sensors at the specified cell on the specified square after all sensors have been processed */

				break;
			case kDMSensorWallObjGeneratorRotateSensors:
				if (sensorCountToProcessPerCell[cellIdx]) /* If the sensor is not the last one of its type on the cell */
					continue;

				doNotTriggerSensor = !_vm->_championMan->_leaderEmptyHanded;
				if (!doNotTriggerSensor)
					triggerLocalEffect(kDMSensorEffectToggle, mapX, mapY, cellIdx); /* This will cause a rotation of the sensors at the specified cell on the specified square after all sensors have been processed */
				break;
			case kDMSensorWallSingleObjStorageRotateSensors:
				if (_vm->_championMan->_leaderEmptyHanded) {
					leaderHandObject = getObjectOfTypeInCell(mapX, mapY, cellIdx, sensorData);
					if (leaderHandObject == _vm->_thingNone)
						continue;

					dungeon.unlinkThingFromList(leaderHandObject, Thing(0), mapX, mapY);
					_vm->_championMan->putObjectInLeaderHand(leaderHandObject, true);
				} else {
					if ((_vm->_objectMan->getObjectType(leaderHandObject) != sensorData) || (getObjectOfTypeInCell(mapX, mapY, cellIdx, sensorData) != _vm->_thingNone))
						continue;

					_vm->_championMan->getObjectRemovedFromLeaderHand();
					dungeon.linkThingToList(_vm->thingWithNewCell(leaderHandObject, cellIdx), Thing(0), mapX, mapY);
					leaderHandObject = _vm->_thingNone;
				}
				triggerLocalEffect(kDMSensorEffectToggle, mapX, mapY, cellIdx); /* This will cause a rotation of the sensors at the specified cell on the specified square after all sensors have been processed */
				if ((sensorEffect == kDMSensorEffectHold) && !_vm->_championMan->_leaderEmptyHanded)
					doNotTriggerSensor = true;
				else
					doNotTriggerSensor = false;

				break;
			case kDMSensorWallObjExchanger: {
				if (sensorCountToProcessPerCell[cellIdx]) /* If the sensor is not the last one of its type on the cell */
					continue;

				Thing thingOnSquare = dungeon.getSquareFirstObject(mapX, mapY);
				if ((_vm->_objectMan->getObjectType(leaderHandObject) != sensorData) || (thingOnSquare == _vm->_thingNone))
					continue;

				dungeon.unlinkThingFromList(thingOnSquare, Thing(0), mapX, mapY);
				_vm->_championMan->getObjectRemovedFromLeaderHand();
				dungeon.linkThingToList(_vm->thingWithNewCell(leaderHandObject, cellIdx), Thing(0), mapX, mapY);
				_vm->_championMan->putObjectInLeaderHand(thingOnSquare, true);
				doNotTriggerSensor = false;
				}
				break;
			case kDMSensorWallChampionPortrait:
				_vm->_championMan->addCandidateChampionToParty(sensorData);
				continue;
				break;
			default:
				continue;
				break;
			}

			if (sensorEffect == kDMSensorEffectHold) {
				sensorEffect = doNotTriggerSensor ? kDMSensorEffectClear : kDMSensorEffectSet;
				doNotTriggerSensor = false;
			}
			if (!doNotTriggerSensor) {
				atLeastOneSensorWasTriggered = true;
				if (currentSensor->getAttrAudibleA())
					_vm->_sound->requestPlay(kDMSoundIndexSwitch, dungeon._partyMapX, dungeon._partyMapY, kDMSoundModePlayIfPrioritized);

				if (!_vm->_championMan->_leaderEmptyHanded && ((processedSensorType == kDMSensorWallOrnClickWithSpecObjRemoved) || (processedSensorType == kDMSensorWallOrnClickWithSpecObjRemovedRotateSensors) || (processedSensorType == kDMSensorWallOrnClickWithSpecObjRemovedSensor))) {
					Thing *leaderThing = (Thing *)dungeon.getThingData(leaderHandObject);
					*leaderThing = _vm->_thingNone;
					_vm->_championMan->getObjectRemovedFromLeaderHand();
					leaderHandObject = _vm->_thingNone;
				} else if (_vm->_championMan->_leaderEmptyHanded
					&& (processedSensorType == kDMSensorWallObjGeneratorRotateSensors)) {
					leaderHandObject = dungeon.getObjForProjectileLaucherOrObjGen(sensorData);
					if (leaderHandObject != _vm->_thingNone)
						_vm->_championMan->putObjectInLeaderHand(leaderHandObject, true);
				}
				triggerEffect(currentSensor, sensorEffect, mapX, mapY, cellIdx);
			}
			continue;
		}
		if (ProcessedThingType >= kDMThingTypeGroup)
			break;
	}
	processRotationEffect();
	return atLeastOneSensorWasTriggered;
}

bool MovesensMan::getMoveResult(Thing thing, int16 mapX, int16 mapY, int16 destMapX, int16 destMapY) {
	DungeonMan &dungeon = *_vm->_dungeonMan;
	DisplayMan &display = *_vm->_displayMan;

	ThingType thingType = kDMThingTypeParty;
	int16 traversedPitCount = 0;
	uint16 moveGroupResult = 0;
	uint16 thingCell = 0;
	bool thingLevitates = false;

	if (thing != _vm->_thingParty) {
		thingType = thing.getType();
		thingCell = thing.getCell();
		thingLevitates = isLevitating(thing);
	}
	/* If moving the party or a creature on the party map from a dungeon square then check for a projectile impact */
	if ((mapX >= 0) && ((thing == _vm->_thingParty) || ((thingType == kDMThingTypeGroup) && (dungeon._currMapIndex == dungeon._partyMapIndex)))) {
		if (moveIsKilledByProjectileImpact(mapX, mapY, destMapX, destMapY, thing))
			return true; /* The specified group thing cannot be moved because it was killed by a projectile impact */
	}

	uint16 mapIndexSource = 0;
	uint16 mapIndexDestination = 0;
	bool groupOnPartyMap = false;
	bool partySquare = false;
	bool audibleTeleporter = false;

	if (destMapX >= 0) {
		mapIndexSource = mapIndexDestination = dungeon._currMapIndex;
		groupOnPartyMap = (mapIndexSource == dungeon._partyMapIndex) && (mapX >= 0);
		uint16 direction = 0;
		bool fallKilledGroup = false;
		bool drawDungeonViewWhileFalling = false;
		bool destinationIsTeleporterTarget = false;
		int16 requiredTeleporterScope;
		if (thing == _vm->_thingParty) {
			dungeon._partyMapX = destMapX;
			dungeon._partyMapY = destMapY;
			requiredTeleporterScope = kDMTeleporterScopeObjectsOrParty;
			drawDungeonViewWhileFalling = !_vm->_inventoryMan->_inventoryChampionOrdinal && !_vm->_championMan->_partyIsSleeping;
			direction = dungeon._partyDir;
		} else if (thingType == kDMThingTypeGroup)
			requiredTeleporterScope = kDMTeleporterScopeCreatures;
		else
			requiredTeleporterScope = (kDMTeleporterScopeCreatures | kDMTeleporterScopeObjectsOrParty);

		if (thingType == kDMThingTypeProjectile) {
			Teleporter *L0712_ps_Teleporter = (Teleporter *)dungeon.getThingData(thing);
			_moveResultDir = (_vm->_timeline->_events[((Projectile *)L0712_ps_Teleporter)->_eventIndex])._Cu._projectile.getDir();
		}

		int16 destinationSquareData = 0;
		/* No more than 1000 chained moves at once (in a chain of teleporters and pits for example) */
		for (int16 chainedMoveCount = 1000; --chainedMoveCount; ) {
			destinationSquareData = dungeon._currMapData[destMapX][destMapY];
			ElementType destinationSquareType = Square(destinationSquareData).getType();
			if (destinationSquareType == (int)kDMElementTypeTeleporter) {
				if (!getFlag(destinationSquareData, kDMSquareMaskTeleporterOpen))
					break;

				Teleporter *teleporter = (Teleporter *)dungeon.getSquareFirstThingData(destMapX, destMapY);
				if ((teleporter->getScope() == kDMTeleporterScopeCreatures) && (thingType != kDMThingTypeGroup))
					break;

				if ((requiredTeleporterScope != (kDMTeleporterScopeCreatures | kDMTeleporterScopeObjectsOrParty)) && !getFlag(teleporter->getScope(), requiredTeleporterScope))
					break;

				destinationIsTeleporterTarget = (destMapX == teleporter->getTargetMapX()) && (destMapY == teleporter->getTargetMapY()) && (mapIndexDestination == teleporter->getTargetMapIndex());
				destMapX = teleporter->getTargetMapX();
				destMapY = teleporter->getTargetMapY();
				audibleTeleporter = teleporter->isAudible();
				dungeon.setCurrentMap(mapIndexDestination = teleporter->getTargetMapIndex());
				if (thing == _vm->_thingParty) {
					dungeon._partyMapX = destMapX;
					dungeon._partyMapY = destMapY;
					if (teleporter->isAudible())
						_vm->_sound->requestPlay(kDMSoundIndexBuzz, dungeon._partyMapX, dungeon._partyMapY, kDMSoundModePlayImmediately);

					drawDungeonViewWhileFalling = true;
					if (teleporter->getAbsoluteRotation())
						_vm->_championMan->setPartyDirection(teleporter->getRotation());
					else
						_vm->_championMan->setPartyDirection(_vm->normalizeModulo4(dungeon._partyDir + teleporter->getRotation()));
				} else {
					if (thingType == kDMThingTypeGroup) {
						if (teleporter->isAudible())
							_vm->_sound->requestPlay(kDMSoundIndexBuzz, destMapX, destMapY, kDMSoundModePlayIfPrioritized);

						moveGroupResult = getTeleporterRotatedGroupResult(teleporter, thing, mapIndexSource);
					} else {
						if (thingType == kDMThingTypeProjectile)
							thing = getTeleporterRotatedProjectileThing(teleporter, thing);
						else if (!(teleporter->getAbsoluteRotation()) && (mapX != -2))
							thing = _vm->thingWithNewCell(thing, _vm->normalizeModulo4(thing.getCell() + teleporter->getRotation()));
					}
				}
				if (destinationIsTeleporterTarget)
					break;
			} else {
				if ((destinationSquareType == (int)kDMElementTypePit) && !thingLevitates && getFlag(destinationSquareData, kDMSquareMaskPitOpen) && !getFlag(destinationSquareData, kDMSquareMaskPitImaginary)) {
					if (drawDungeonViewWhileFalling && !_useRopeToClimbDownPit) {
						drawDungeonViewWhileFalling = true;
						if (traversedPitCount) {
							dungeon.setCurrentMapAndPartyMap(mapIndexDestination);
							display.loadCurrentMapGraphics();
						}
						traversedPitCount++;
						display.drawDungeon(dungeon._partyDir, destMapX, destMapY); /* BUG0_28 When falling through multiple pits the dungeon view is updated to show each traversed map but the graphics used for creatures, wall and floor ornaments may not be correct. The dungeon view is drawn for each map by using the graphics loaded for the source map. Therefore the graphics for creatures, wall and floor ornaments may not look like what they should */
																					/* BUG0_71 Some timings are too short on fast computers. When the party falls in a series of pits, the dungeon view is refreshed too quickly because the execution speed is not limited */
																					/* BUG0_01 While drawing creatures the engine will read invalid ACTIVE_GROUP data in _vm->_groupMan->_g375_activeGroups because the data is for the creatures on the source map and not the map being drawn. The only consequence is that creatures may be drawn with incorrect bitmaps and/or directions */
					}
					mapIndexDestination = dungeon.getLocationAfterLevelChange(mapIndexDestination, 1, &destMapX, &destMapY);
					dungeon.setCurrentMap(mapIndexDestination);
					if (thing == _vm->_thingParty) {
						dungeon._partyMapX = destMapX;
						dungeon._partyMapY = destMapY;
						if (_vm->_championMan->_partyChampionCount > 0) {
							if (_useRopeToClimbDownPit) {
								Champion *curChampion = _vm->_championMan->_champions;
								for (int16 championIdx = kDMChampionFirst; championIdx < _vm->_championMan->_partyChampionCount; championIdx++, curChampion++) {
									if (curChampion->_currHealth)
										_vm->_championMan->decrementStamina(championIdx, ((curChampion->_load * 25) / _vm->_championMan->getMaximumLoad(curChampion)) + 1);
								}
							} else if (_vm->_championMan->getDamagedChampionCount(20, kDMWoundLegs | kDMWoundFeet, kDMAttackTypeSelf))
								_vm->_sound->requestPlay(kDMSoundIndexScream, dungeon._partyMapX, dungeon._partyMapY, kDMSoundModePlayImmediately);
						}
						_useRopeToClimbDownPit = false;
					} else if (thingType == kDMThingTypeGroup) {
						dungeon.setCurrentMap(mapIndexSource);
						uint16 outcome = _vm->_groupMan->getDamageAllCreaturesOutcome((Group *)dungeon.getThingData(thing), mapX, mapY, 20, false);
						dungeon.setCurrentMap(mapIndexDestination);
						fallKilledGroup = (outcome == kDMKillOutcomeAllCreaturesInGroup);
						if (fallKilledGroup)
							break;

						if (outcome == kDMKillOutcomeSomeCreaturesInGroup)
							_vm->_groupMan->dropMovingCreatureFixedPossession(thing, destMapX, destMapY);
					}
				} else if ((destinationSquareType == (int)kDMElementTypeStairs) && (thing != _vm->_thingParty) && (thingType != kDMThingTypeProjectile)) {
					if (!getFlag(destinationSquareData, kDMSquareMaskStairsUp)) {
						mapIndexDestination = dungeon.getLocationAfterLevelChange(mapIndexDestination, 1, &destMapX, &destMapY);
						dungeon.setCurrentMap(mapIndexDestination);
					}
					direction = dungeon.getStairsExitDirection(destMapX, destMapY);
					destMapX += _vm->_dirIntoStepCountEast[direction];
					destMapY += _vm->_dirIntoStepCountNorth[direction];
					direction = _vm->returnOppositeDir((Direction)direction);
					thingCell = thing.getCell();
					thingCell = _vm->normalizeModulo4((((thingCell - direction + 1) & 0x0002) >> 1) + direction);
					thing = _vm->thingWithNewCell(thing, thingCell);
				} else
					break;
			}
		}
		if ((thingType == kDMThingTypeGroup) && (fallKilledGroup || !dungeon.isCreatureAllowedOnMap(thing, mapIndexDestination))) {
			_vm->_groupMan->dropMovingCreatureFixedPossession(thing, destMapX, destMapY);
			_vm->_groupMan->dropGroupPossessions(destMapX, destMapY, thing, kDMSoundModePlayOneTickLater);
			dungeon.setCurrentMap(mapIndexSource);
			if (mapX >= 0)
				_vm->_groupMan->groupDelete(mapX, mapY);

			return true; /* The specified group thing cannot be moved because it was killed by a fall or because it is not allowed on the destination map */
		}
		_moveResultMapX = destMapX;
		_moveResultMapY = destMapY;
		_moveResultMapIndex = mapIndexDestination;
		_moveResultCell = thing.getCell();
		partySquare = (mapIndexDestination == mapIndexSource) && (destMapX == mapX) && (destMapY == mapY);
		if (partySquare) {
			if (thing == _vm->_thingParty) {
				if (dungeon._partyDir == direction)
					return false;
			} else if ((_moveResultCell == thingCell) && (thingType != kDMThingTypeProjectile))
				return false;
		} else {
			if ((thing == _vm->_thingParty) && _vm->_championMan->_partyChampionCount) {
				uint16 oldDestinationSquare = destinationSquareData;
				int16 scentIndex = _vm->_championMan->_party._scentCount;
				while (scentIndex >= 24) {
					_vm->_championMan->deleteScent(0);
					scentIndex--;
				}

				if (scentIndex)
					_vm->_championMan->addScentStrength(mapX, mapY, (int)(_vm->_gameTime - _vm->_projexpl->_lastPartyMovementTime));

				_vm->_projexpl->_lastPartyMovementTime = _vm->_gameTime;
				_vm->_championMan->_party._scentCount++;
				if (_vm->_championMan->_party._event79Count_Footprints)
					_vm->_championMan->_party._lastScentIndex = _vm->_championMan->_party._scentCount;

				_vm->_championMan->_party._scents[scentIndex].setMapX(destMapX);
				_vm->_championMan->_party._scents[scentIndex].setMapY(destMapY);
				_vm->_championMan->_party._scents[scentIndex].setMapIndex(mapIndexDestination);
				_vm->_championMan->_party._scentStrengths[scentIndex] = 0;
				_vm->_championMan->addScentStrength(destMapX, destMapY, kDMMaskMergeCycles | 24);
				destinationSquareData = oldDestinationSquare;
			}
			if (mapIndexDestination != mapIndexSource)
				dungeon.setCurrentMap(mapIndexSource);
		}
	}
	if (mapX >= 0) {
		if (thing == _vm->_thingParty)
			processThingAdditionOrRemoval(mapX, mapY, _vm->_thingParty, partySquare, false);
		else if (thingLevitates)
			dungeon.unlinkThingFromList(thing, _vm->_thingNone, mapX, mapY);
		else
			processThingAdditionOrRemoval(mapX, mapY, thing, (dungeon._currMapIndex == dungeon._partyMapIndex) && (mapX == dungeon._partyMapX) && (mapY == dungeon._partyMapY), false);
	}
	if (destMapX >= 0) {
		if (thing == _vm->_thingParty) {
			dungeon.setCurrentMap(mapIndexDestination);
			if ((thing = _vm->_groupMan->groupGetThing(dungeon._partyMapX, dungeon._partyMapY)) != _vm->_thingEndOfList) { /* Delete group if party moves onto its square */
				_vm->_groupMan->dropGroupPossessions(dungeon._partyMapX, dungeon._partyMapY, thing, kDMSoundModePlayIfPrioritized);
				_vm->_groupMan->groupDelete(dungeon._partyMapX, dungeon._partyMapY);
			}

			if (mapIndexDestination == mapIndexSource)
				processThingAdditionOrRemoval(dungeon._partyMapX, dungeon._partyMapY, _vm->_thingParty, partySquare, true);
			else {
				dungeon.setCurrentMap(mapIndexSource);
				_vm->_newPartyMapIndex = mapIndexDestination;
			}
		} else {
			if (thingType == kDMThingTypeGroup) {
				dungeon.setCurrentMap(mapIndexDestination);
				Teleporter *L0712_ps_Teleporter = (Teleporter *)dungeon.getThingData(thing);
				int16 activeGroupIndex = ((Group *)L0712_ps_Teleporter)->getActiveGroupIndex();
				if (((mapIndexDestination == dungeon._partyMapIndex) && (destMapX == dungeon._partyMapX) && (destMapY == dungeon._partyMapY)) || (_vm->_groupMan->groupGetThing(destMapX, destMapY) != _vm->_thingEndOfList)) { /* If a group tries to move to the party square or over another group then create an event to move the group later */
					dungeon.setCurrentMap(mapIndexSource);
					if (mapX >= 0)
						_vm->_groupMan->groupDeleteEvents(mapX, mapY);

					if (groupOnPartyMap)
						_vm->_groupMan->removeActiveGroup(activeGroupIndex);

					createEventMoveGroup(thing, destMapX, destMapY, mapIndexDestination, audibleTeleporter);
					return true; /* The specified group thing cannot be moved because the party or another group is on the destination square */
				}
				Group *tmpGroup = (Group *)dungeon._thingData[kDMThingTypeGroup];
				uint16 movementSoundIndex = getSound((CreatureType)tmpGroup[thing.getIndex()]._type);
				if (movementSoundIndex < kDMSoundCount)
					_vm->_sound->requestPlay(movementSoundIndex, destMapX, destMapY, kDMSoundModePlayIfPrioritized);

				if (groupOnPartyMap && (mapIndexDestination != dungeon._partyMapIndex)) { /* If the group leaves the party map */
					_vm->_groupMan->removeActiveGroup(activeGroupIndex);
					moveGroupResult = true;
				} else if ((mapIndexDestination == dungeon._partyMapIndex) && (!groupOnPartyMap)) { /* If the group arrives on the party map */
					_vm->_groupMan->addActiveGroup(thing, destMapX, destMapY);
					moveGroupResult = true;
				}
				if (thingLevitates)
					dungeon.linkThingToList(thing, Thing(0), destMapX, destMapY);
				else
					processThingAdditionOrRemoval(destMapX, destMapY, thing, false, true);

				if (moveGroupResult || (mapX < 0)) /* If group moved from one map to another or if it was just placed on a square */
					_vm->_groupMan->startWandering(destMapX, destMapY);

				dungeon.setCurrentMap(mapIndexSource);
				if (mapX >= 0) {
					if (moveGroupResult > 1) /* If the group behavior was C6_BEHAVIOR_ATTACK before being teleported from and to the party map */
						_vm->_groupMan->stopAttacking(&_vm->_groupMan->_activeGroups[moveGroupResult - 2], mapX, mapY);
					else if (moveGroupResult) /* If the group was teleported or leaved the party map or entered the party map */
						_vm->_groupMan->groupDeleteEvents(mapX, mapY);
				}
				return moveGroupResult;
			}
			dungeon.setCurrentMap(mapIndexDestination);
			if (thingType == kDMThingTypeProjectile) /* BUG0_29 An explosion can trigger a floor sensor. Explosions do not trigger floor sensors on the square where they are created. However, if an explosion is moved by a teleporter (or by falling into a pit, see BUG0_26) after it was created, it can trigger floor sensors on the destination square. This is because explosions are not considered as levitating in the code, while projectiles are. The condition here should be (L0713_B_ThingLevitates) so that explosions would not start sensor processing on their destination square as they should be Levitating. This would work if F0264_MOVE_IsLevitating returned true for explosions (see BUG0_26) */
				dungeon.linkThingToList(thing, Thing(0), destMapX, destMapY);
			else
				processThingAdditionOrRemoval(destMapX, destMapY, thing, (dungeon._currMapIndex == dungeon._partyMapIndex) && (destMapX == dungeon._partyMapX) && (destMapY == dungeon._partyMapY), true);

			dungeon.setCurrentMap(mapIndexSource);
		}
	}
	return false;
}

bool MovesensMan::isLevitating(Thing thing) {
	ThingType thingType = thing.getType();
	bool retVal = false;
	if (thingType == kDMThingTypeGroup)
		retVal = getFlag(_vm->_dungeonMan->getCreatureAttributes(thing), kDMCreatureMaskLevitation);
	else if ((thingType == kDMThingTypeProjectile) || (thingType == kDMThingTypeExplosion))
	// Fix original bug involving explosions falling in pits
		retVal = true;

	return retVal;
}

bool MovesensMan::moveIsKilledByProjectileImpact(int16 srcMapX, int16 srcMapY, int16 destMapX, int16 destMapY, Thing thing) {
	DungeonMan &dungeon = *_vm->_dungeonMan;

	/* This array is used only when moving between two adjacent squares and is used to test projectile
	impacts when the party or group is in the 'intermediary' step between the two squares. Without
	this test, in the example below no impact would be detected. In this example, the party moves from
	the source square on the left (which contains a single champion at cell 2) to the destination square
	on the right (which contains a single projectile at cell 3).
	Party:      Projectiles on target square:   Incorrect result without the test for the intermediary step (the champion would have passed through the projectile without impact):
	00    ->    00                         00
	01          P0                         P1 */
	byte intermediaryChampionOrCreatureOrdinalInCell[4];

	/* This array has an entry for each cell on the source square, containing the ordinal of the champion
	or creature (0 if there is no champion or creature at this cell) */
	byte championOrCreatureOrdinalInCell[4];

	bool checkDestinationSquareProjectileImpacts = false;
	for (int16 i = 0; i < 4; ++i)
		championOrCreatureOrdinalInCell[i] = 0;

	ElementType impactType;
	if (thing == _vm->_thingParty) {
		impactType = kDMElementTypeChampion;
		for (uint16 cellIdx = kDMCellNorthWest; cellIdx < kDMCellSouthWest + 1; cellIdx++) {
			if (_vm->_championMan->getIndexInCell((ViewCell)cellIdx) >= 0)
				championOrCreatureOrdinalInCell[cellIdx] = _vm->indexToOrdinal(cellIdx);
		}
	} else {
		impactType = kDMElementTypeCreature;
		Group *curGroup = (Group *)dungeon.getThingData(thing);
		int16 creatureAlive = 0;
		for (uint16 cellIdx = kDMCellNorthWest; cellIdx < kDMCellSouthWest + 1; cellIdx++) {
			creatureAlive |= curGroup->_health[cellIdx];
			if (_vm->_groupMan->getCreatureOrdinalInCell(curGroup, cellIdx))
				championOrCreatureOrdinalInCell[cellIdx] = _vm->indexToOrdinal(cellIdx);
		}
		if (!creatureAlive)
			return false;
	}
	if ((destMapX >= 0) && ((abs(srcMapX - destMapX) + abs(srcMapY - destMapY)) == 1)) {
		/* If source and destination squares are adjacent (if party or group is not being teleported) */
		int16 primaryDirection = _vm->_groupMan->getDirsWhereDestIsVisibleFromSource(srcMapX, srcMapY, destMapX, destMapY);
		int16 secondaryDirection = _vm->turnDirRight(primaryDirection);
		for (int16 i = 0; i < 4; ++i)
			intermediaryChampionOrCreatureOrdinalInCell[i] = 0;

		intermediaryChampionOrCreatureOrdinalInCell[_vm->turnDirLeft(primaryDirection)] = championOrCreatureOrdinalInCell[primaryDirection];
		if (intermediaryChampionOrCreatureOrdinalInCell[_vm->turnDirLeft(primaryDirection)])
			checkDestinationSquareProjectileImpacts = true;

		intermediaryChampionOrCreatureOrdinalInCell[_vm->turnDirRight(secondaryDirection)] = championOrCreatureOrdinalInCell[secondaryDirection];
		if (intermediaryChampionOrCreatureOrdinalInCell[_vm->turnDirRight(secondaryDirection)])
			checkDestinationSquareProjectileImpacts = true;

		if (!championOrCreatureOrdinalInCell[primaryDirection])
			championOrCreatureOrdinalInCell[primaryDirection] = championOrCreatureOrdinalInCell[_vm->turnDirLeft(primaryDirection)];

		if (!championOrCreatureOrdinalInCell[secondaryDirection])
			championOrCreatureOrdinalInCell[secondaryDirection] = championOrCreatureOrdinalInCell[_vm->turnDirRight(secondaryDirection)];
	}
	uint16 projectileMapX = srcMapX; /* Check impacts with projectiles on the source square */
	uint16 projectileMapY = srcMapY;
T0266017_CheckProjectileImpacts:
	Thing curThing = dungeon.getSquareFirstThing(projectileMapX, projectileMapY);
	while (curThing != _vm->_thingEndOfList) {
		if ((curThing.getType() == kDMThingTypeProjectile) &&
			(_vm->_timeline->_events[(((Projectile *)dungeon._thingData[kDMThingTypeProjectile])[curThing.getIndex()])._eventIndex]._type != kDMEventTypeMoveProjectileIgnoreImpacts)) {
			int16 championOrCreatureOrdinal = championOrCreatureOrdinalInCell[curThing.getCell()];
			if (championOrCreatureOrdinal && _vm->_projexpl->hasProjectileImpactOccurred(impactType, srcMapX, srcMapY, _vm->ordinalToIndex(championOrCreatureOrdinal), curThing)) {
				_vm->_projexpl->projectileDeleteEvent(curThing);
				if (_vm->_projexpl->_creatureDamageOutcome == kDMKillOutcomeAllCreaturesInGroup)
					return true;

				goto T0266017_CheckProjectileImpacts;
			}
		}
		curThing = dungeon.getNextThing(curThing);
	}
	if (checkDestinationSquareProjectileImpacts) {
		srcMapX |= ((projectileMapX = destMapX) + 1) << 8; /* Check impacts with projectiles on the destination square */
		srcMapY |= (projectileMapY = destMapY) << 8;
		for (uint16 i = 0; i < 4; ++i)
			championOrCreatureOrdinalInCell[i] = intermediaryChampionOrCreatureOrdinalInCell[i];
		checkDestinationSquareProjectileImpacts = false;
		goto T0266017_CheckProjectileImpacts;
	}
	return false;
}

void MovesensMan::addEvent(TimelineEventType type, byte mapX, byte mapY, Cell cell, SensorEffect effect, int32 time) {
	TimelineEvent newEvent;
	newEvent._mapTime = _vm->setMapAndTime(_vm->_dungeonMan->_currMapIndex, time);
	newEvent._type = type;
	newEvent._priority = 0;
	newEvent._Bu._location._mapX = mapX;
	newEvent._Bu._location._mapY = mapY;
	newEvent._Cu.A._cell = cell;
	newEvent._Cu.A._effect = effect;
	_vm->_timeline->addEventGetEventIndex(&newEvent);
}

int16 MovesensMan::getSound(CreatureType creatureType) {
	if (_vm->_championMan->_partyIsSleeping)
		return 35;

	switch (creatureType) {
	case kDMCreatureTypeWizardEye:
	case kDMCreatureTypeGhostRive:
	case kDMCreatureTypeBlackFlame:
	case kDMCreatureTypeMaterializerZytaz:
	case kDMCreatureTypeLordChaos:
	case kDMCreatureTypeLordOrder:
	case kDMCreatureTypeGreyLord:
	default:
		return 35;
	case kDMCreatureTypeGiggler:
	case kDMCreatureTypeStoneGolem:
	case kDMCreatureTypeMummy:
	case kDMCreatureTypeVexirk:
	case kDMCreatureTypeAntman:
	case kDMCreatureTypeDemon:
		return kDMSoundIndexMoveMummyTrolinAntmanStoneGolemGiggleVexirkDemon;
	case kDMCreatureTypeGiantScorpion:
	case kDMCreatureTypePainRat:
	case kDMCreatureTypeRuster:
	case kDMCreatureTypeScreamer:
	case kDMCreatureTypeRockpile:
	case kDMCreatureTypeMagentaWorm:
	case kDMCreatureTypeOitu:
		return kDMSoundIndexMoveScreamerRocksWormPainRatHellHoundRusterScorpionsOitu;
	case kDMCreatureTypeRedDragon:
		return kDMSoundIndexMoveRedDragon;
	case kDMCreatureTypeSkeleton:
		return kDMSoundIndexMoveSkeletton;
	case kDMCreatureTypeAnimatedArmour:
		return kDMSoundIndexMoveAnimatedArmorDethKnight;
	case kDMCreatureTypeSwampSlime:
	case kDMCreatureTypeWaterElemental:
		return kDMSoundIndexMoveSlimesDevilWaterElemental;
	case kDMCreatureTypeCouatl:
	case kDMCreatureTypeGiantWasp:
		return kDMSoundIndexMoveCouatlGiantWaspMuncher;
	}

	return 35;
}

int16 MovesensMan::getTeleporterRotatedGroupResult(Teleporter *teleporter, Thing thing, uint16 mapIndex) {
	DungeonMan &dungeon = *_vm->_dungeonMan;
	Group *group = (Group *)dungeon.getThingData(thing);
	Direction rotation = teleporter->getRotation();
	uint16 groupDirections = _vm->_groupMan->getGroupDirections(group, mapIndex);

	bool absoluteRotation = teleporter->getAbsoluteRotation();
	uint16 updatedGroupDirections;
	if (absoluteRotation)
		updatedGroupDirections = rotation;
	else
		updatedGroupDirections = _vm->normalizeModulo4(groupDirections + rotation);

	uint16 updatedGroupCells = _vm->_groupMan->getGroupCells(group, mapIndex);
	if (updatedGroupCells != kDMCreatureTypeSingleCenteredCreature) {
		int16 groupCells = updatedGroupCells;
		int16 creatureSize = getFlag(dungeon._creatureInfos[group->_type]._attributes, kDMCreatureMaskSize);
		int16 relativeRotation = _vm->normalizeModulo4(4 + updatedGroupDirections - groupDirections);
		for (int16 creatureIdx = 0; creatureIdx <= group->getCount(); creatureIdx++) {
			updatedGroupDirections = _vm->_groupMan->getGroupValueUpdatedWithCreatureValue(updatedGroupDirections, creatureIdx, absoluteRotation ? (uint16)rotation : _vm->normalizeModulo4(groupDirections + rotation));
			if (creatureSize == kDMCreatureSizeQuarter) {
				relativeRotation = absoluteRotation ? 1 : 0;
				if (relativeRotation)
					relativeRotation = rotation;
			}
			if (relativeRotation)
				updatedGroupCells = _vm->_groupMan->getGroupValueUpdatedWithCreatureValue(updatedGroupCells, creatureIdx, _vm->normalizeModulo4(groupCells + relativeRotation));

			groupDirections >>= 2;
			groupCells >>= 2;
		}
	}
	dungeon.setGroupDirections(group, updatedGroupDirections, mapIndex);
	dungeon.setGroupCells(group, updatedGroupCells, mapIndex);
	if ((mapIndex == dungeon._partyMapIndex) && (group->setBehaviour(kDMBehaviorAttack)))
		return group->getActiveGroupIndex() + 2;

	return 1;
}

Thing MovesensMan::getTeleporterRotatedProjectileThing(Teleporter *teleporter, Thing projectileThing) {
	int16 updatedDirection = _moveResultDir;
	int16 rotation = teleporter->getRotation();
	if (teleporter->getAbsoluteRotation())
		updatedDirection = rotation;
	else {
		updatedDirection = _vm->normalizeModulo4(updatedDirection + rotation);
		projectileThing = _vm->thingWithNewCell(projectileThing, _vm->normalizeModulo4(projectileThing.getCell() + rotation));
	}
	_moveResultDir = updatedDirection;
	return projectileThing;
}

void MovesensMan::processThingAdditionOrRemoval(uint16 mapX, uint16 mapY, Thing thing, bool partySquare, bool addThing) {
	DungeonMan &dungeon = *_vm->_dungeonMan;
	TextMan &txtMan = *_vm->_textMan;

	int16 thingType;
	IconIndice objectType;
	if (thing != _vm->_thingParty) {
		thingType = thing.getType();
		objectType = _vm->_objectMan->getObjectType(thing);
	} else {
		thingType = kDMThingTypeParty;
		objectType = kDMIconIndiceNone;
	}

	if ((!addThing) && (thingType != kDMThingTypeParty))
		dungeon.unlinkThingFromList(thing, Thing(0), mapX, mapY);

	Square curSquare = Square(dungeon._currMapData[mapX][mapY]);
	int16 sensorTriggeredCell;
	if (curSquare.getType() == kDMElementTypeWall)
		sensorTriggeredCell = thing.getCell();
	else
		sensorTriggeredCell = kDMCellAny; // this will wrap around

	bool squareContainsObject = false;
	bool squareContainsGroup = false;
	bool squareContainsThingOfSameType = false;
	bool squareContainsThingOfDifferentType = false;
	Thing curThing = dungeon.getSquareFirstThing(mapX, mapY);
	if (sensorTriggeredCell == kDMCellAny) {
		while (curThing != _vm->_thingEndOfList) {
			uint16 curThingType = curThing.getType();
			if (curThingType == kDMThingTypeGroup)
				squareContainsGroup = true;
			else if ((curThingType == kDMstringTypeText) && (thingType == kDMThingTypeParty) && addThing && !partySquare) {
				dungeon.decodeText(_vm->_stringBuildBuffer, curThing, kDMTextTypeMessage);
				txtMan.printMessage(kDMColorWhite, _vm->_stringBuildBuffer);
			} else if ((curThingType > kDMThingTypeGroup) && (curThingType < kDMThingTypeProjectile)) {
				squareContainsObject = true;
				squareContainsThingOfSameType |= (_vm->_objectMan->getObjectType(curThing) == objectType);
				squareContainsThingOfDifferentType |= (_vm->_objectMan->getObjectType(curThing) != objectType);
			}
			curThing = dungeon.getNextThing(curThing);
		}
	} else {
		while (curThing != _vm->_thingEndOfList) {
			if ((sensorTriggeredCell == curThing.getCell()) && (curThing.getType() > kDMThingTypeGroup)) {
				squareContainsObject = true;
				squareContainsThingOfSameType |= (_vm->_objectMan->getObjectType(curThing) == objectType);
				squareContainsThingOfDifferentType |= (_vm->_objectMan->getObjectType(curThing) != objectType);
			}
			curThing = dungeon.getNextThing(curThing);
		}
	}
	if (addThing && (thingType != kDMThingTypeParty))
		dungeon.linkThingToList(thing, Thing(0), mapX, mapY);

	for (curThing = dungeon.getSquareFirstThing(mapX, mapY); curThing != _vm->_thingEndOfList; curThing = dungeon.getNextThing(curThing)) {
		uint16 curThingType = curThing.getType();
		if (curThingType == kDMThingTypeSensor) {
			Sensor *curSensor = (Sensor *)dungeon.getThingData(curThing);
			if (curSensor->getType() == kDMSensorDisabled)
				continue;

			int16 curSensorData = curSensor->getData();
			bool triggerSensor = addThing;
			if (sensorTriggeredCell == kDMCellAny) {
				switch (curSensor->getType()) {
				case kDMSensorFloorTheronPartyCreatureObj:
					if (partySquare || squareContainsObject || squareContainsGroup) /* BUG0_30 A floor sensor is not triggered when you put an object on the floor if a levitating creature is present on the same square. The condition to determine if the sensor should be triggered checks if there is a creature on the square but does not check whether the creature is levitating. While it is normal not to trigger the sensor if there is a non levitating creature on the square (because it was already triggered by the creature itself), a levitating creature should not prevent triggering the sensor with an object. */
						continue;
					break;
				case kDMSensorFloorTheronPartyCreature:
					if ((thingType > kDMThingTypeGroup) || partySquare || squareContainsGroup)
						continue;
					break;
				case kDMSensorFloorParty:
					if ((thingType != kDMThingTypeParty) || (_vm->_championMan->_partyChampionCount == 0))
						continue;

					if (curSensorData == 0) {
						if (partySquare)
							continue;
					} else if (!addThing)
						triggerSensor = false;
					else
						triggerSensor = (curSensorData == _vm->indexToOrdinal(dungeon._partyDir));
					break;
				case kDMSensorFloorObj:
					if ((curSensorData != _vm->_objectMan->getObjectType(thing)) || squareContainsThingOfSameType)
						continue;
					break;
				case kDMSensorFloorPartyOnStairs:
					if ((thingType != kDMThingTypeParty) || (curSquare.getType() != kDMElementTypeStairs))
						continue;
					break;
				case kDMSensorFloorGroupGenerator:
					continue;
					break;
				case kDMSensorFloorCreature:
					if ((thingType > kDMThingTypeGroup) || (thingType == kDMThingTypeParty) || squareContainsGroup)
						continue;
					break;
				case kDMSensorFloorPartyPossession:
					if (thingType != kDMThingTypeParty)
						continue;

					triggerSensor = isObjectInPartyPossession(curSensorData);
					break;
				case kDMSensorFloorVersionChecker:
					if ((thingType != kDMThingTypeParty) || !addThing || partySquare)
						continue;

					// Strangerke: 20 is a hardcoded version of the game. later version uses 21. Not present in the original dungeons anyway.
					triggerSensor = (curSensorData <= 20);
					break;
				default:
					continue;
					break;
				}
			} else {
				if (sensorTriggeredCell != curThing.getCell())
					continue;

				switch (curSensor->getType()) {
				case kDMSensorWallOrnClick:
					if (squareContainsObject)
						continue;
					break;
				case kDMSensorWallOrnClickWithAnyObj:
					if (squareContainsThingOfSameType || (curSensor->getData() != _vm->_objectMan->getObjectType(thing)))
						continue;
					break;
				case kDMSensorWallOrnClickWithSpecObj:
					if (squareContainsThingOfDifferentType || (curSensor->getData() == _vm->_objectMan->getObjectType(thing)))
						continue;
					break;
				default:
					continue;
					break;
				}
			}

			triggerSensor ^= curSensor->getAttrRevertEffectA();
			SensorEffect curSensorEffect = (SensorEffect)curSensor->getAttrEffectA();
			if (curSensorEffect == kDMSensorEffectHold)
				curSensorEffect = triggerSensor ? kDMSensorEffectSet : kDMSensorEffectClear;
			else if (!triggerSensor)
				continue;

			if (curSensor->getAttrAudibleA())
				_vm->_sound->requestPlay(kDMSoundIndexSwitch, mapX, mapY, kDMSoundModePlayIfPrioritized);

			triggerEffect(curSensor, curSensorEffect, mapX, mapY, (uint16)kDMCellAny); // this will wrap around
			continue;
		}

		if (curThingType >= kDMThingTypeGroup)
			break;
	}
	processRotationEffect();
}

bool MovesensMan::isObjectInPartyPossession(int16 objectType) {
	DungeonMan &dungeon = *_vm->_dungeonMan;
	Champion *curChampion = _vm->_championMan->_champions;

	bool leaderHandObjectProcessed = false;
	int16 championIdx;
	uint16 slotIdx = 0;
	Thing curThing;
	Thing *curSlotThing = nullptr;
	for (championIdx = kDMChampionFirst; championIdx < _vm->_championMan->_partyChampionCount; championIdx++, curChampion++) {
		if (curChampion->_currHealth) {
			curSlotThing = curChampion->_slots;
			for (slotIdx = kDMSlotReadyHand; (slotIdx < kDMSlotChest1) || !leaderHandObjectProcessed; slotIdx++) {
				if (slotIdx < kDMSlotChest1)
					curThing = *curSlotThing++;
				else {
					leaderHandObjectProcessed = true;
					curThing = _vm->_championMan->_leaderHandObject;
				}

				int16 curObjectType = _vm->_objectMan->getObjectType(curThing);
				if (curObjectType == objectType)
					return true;

				if (curObjectType == kDMIconIndiceContainerChestClosed) {
					Container *container = (Container *)dungeon.getThingData(curThing);
					curThing = container->getSlot();
					while (curThing != _vm->_thingEndOfList) {
						if (_vm->_objectMan->getObjectType(curThing) == objectType)
							return true;

						curThing = dungeon.getNextThing(curThing);
					}
				}
			}
		}
	}
	return false;
}

void MovesensMan::triggerEffect(Sensor *sensor, SensorEffect effect, int16 mapX, int16 mapY, uint16 cell) {
	static const TimelineEventType squareTypeToEventTypeArray[7] = { // @ G0059_auc_Graphic562_SquareTypeToEventType
		kDMEventTypeWall,
		kDMEventTypeCorridor,
		kDMEventTypePit,
		kDMEventTypeNone,
		kDMEventTypeDoor,
		kDMEventTypeTeleporter,
		kDMEventTypeFakeWall
	};

	if (sensor->getAttrOnlyOnce())
		sensor->setTypeDisabled();

	int32 endTime = _vm->_gameTime + sensor->getAttrValue();
	if (sensor->getAttrLocalEffect())
		triggerLocalEffect((SensorEffect)sensor->getActionLocalEffect(), mapX, mapY, cell);
	else {
		int16 targetMapX = sensor->getActionTargetMapX();
		int16 targetMapY = sensor->getActionTargetMapY();
		ElementType curSquareType = Square(_vm->_dungeonMan->_currMapData[targetMapX][targetMapY]).getType();
		Cell targetCell;
		if (curSquareType == kDMElementTypeWall)
			targetCell = sensor->getActionTargetCell();
		else
			targetCell = kDMCellNorthWest;

		addEvent(squareTypeToEventTypeArray[curSquareType], targetMapX, targetMapY, targetCell, effect, endTime);
	}
}

void MovesensMan::triggerLocalEffect(SensorEffect localEffect, int16 effX, int16 effY, int16 effCell) {
	if (localEffect == kDMSensorEffectAddExperience) {
		addSkillExperience(kDMSkillSteal, 300, localEffect != kDMSensorEffectNone);
		return;
	}
	_sensorRotationEffect = localEffect;
	_sensorRotationEffMapX = effX;
	_sensorRotationEffMapY = effY;
	_sensorRotationEffCell = effCell;
}

void MovesensMan::addSkillExperience(int16 skillIndex, uint16 exp, bool leaderOnly) {
	if (leaderOnly) {
		if (_vm->_championMan->_leaderIndex != kDMChampionNone)
			_vm->_championMan->addSkillExperience(_vm->_championMan->_leaderIndex, skillIndex, exp);
	} else {
		exp /= _vm->_championMan->_partyChampionCount;
		Champion *curChampion = _vm->_championMan->_champions;
		for (int16 championIdx = kDMChampionFirst; championIdx < _vm->_championMan->_partyChampionCount; championIdx++, curChampion++) {
			if (curChampion->_currHealth)
				_vm->_championMan->addSkillExperience(championIdx, skillIndex, exp);
		}
	}
}

void MovesensMan::processRotationEffect() {
	if (_sensorRotationEffect == kDMSensorEffectNone)
		return;

	DungeonMan &dungeon = *_vm->_dungeonMan;

	switch (_sensorRotationEffect) {
	case kDMSensorEffectClear:
	case kDMSensorEffectToggle:
		{
			Thing firstSensorThing = dungeon.getSquareFirstThing(_sensorRotationEffMapX, _sensorRotationEffMapY);
			while ((firstSensorThing.getType() != kDMThingTypeSensor)
				|| ((_sensorRotationEffCell != kDMCellAny) && (firstSensorThing.getCell() != _sensorRotationEffCell))) {
				firstSensorThing = dungeon.getNextThing(firstSensorThing);
			}
			Sensor *firstSensor = (Sensor *)dungeon.getThingData(firstSensorThing);
			Thing lastSensorThing = firstSensor->getNextThing();
			while ((lastSensorThing != _vm->_thingEndOfList)
				&& ((lastSensorThing.getType() != kDMThingTypeSensor)
				|| ((_sensorRotationEffCell != kDMCellAny) && (lastSensorThing.getCell() != _sensorRotationEffCell)))) {
				lastSensorThing = dungeon.getNextThing(lastSensorThing);
			}
			if (lastSensorThing == _vm->_thingEndOfList)
				break;
			dungeon.unlinkThingFromList(firstSensorThing, Thing(0), _sensorRotationEffMapX, _sensorRotationEffMapY);
			Sensor *lastSensor = (Sensor *)dungeon.getThingData(lastSensorThing);
			lastSensorThing = dungeon.getNextThing(lastSensorThing);
			while (((lastSensorThing != _vm->_thingEndOfList) && (lastSensorThing.getType() == kDMThingTypeSensor))) {
				if ((_sensorRotationEffCell == kDMCellAny) || (lastSensorThing.getCell() == _sensorRotationEffCell))
					lastSensor = (Sensor *)dungeon.getThingData(lastSensorThing);
				lastSensorThing = dungeon.getNextThing(lastSensorThing);
			}
			firstSensor->setNextThing(lastSensor->getNextThing());
			lastSensor->setNextThing(firstSensorThing);
		}
		break;
	default:
		break;
	}
	_sensorRotationEffect = kDMSensorEffectNone;
}

void MovesensMan::createEventMoveGroup(Thing groupThing, int16 mapX, int16 mapY, int16 mapIndex, bool audible) {
	TimelineEvent newEvent;
	newEvent._mapTime = _vm->setMapAndTime(mapIndex, _vm->_gameTime + 5);
	newEvent._type = audible ? kDMEventTypeMoveGroupAudible : kDMEventTypeMoveGroupSilent;
	newEvent._priority = 0;
	newEvent._Bu._location._mapX = mapX;
	newEvent._Bu._location._mapY = mapY;
	newEvent._Cu._slot = groupThing.toUint16();
	_vm->_timeline->addEventGetEventIndex(&newEvent);
}

Thing MovesensMan::getObjectOfTypeInCell(int16 mapX, int16 mapY, int16 cell, int16 objectType) {
	DungeonMan &dungeon = *_vm->_dungeonMan;
	Thing curThing = dungeon.getSquareFirstObject(mapX, mapY);
	while (curThing != _vm->_thingEndOfList) {
		if ((_vm->_objectMan->getObjectType(curThing) == objectType) && ((cell == kDMCellAny) || (curThing.getCell() == cell)))
			return curThing;

		curThing = dungeon.getNextThing(curThing);
	}
	return _vm->_thingNone;
}
}
