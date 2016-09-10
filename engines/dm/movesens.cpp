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
	bool atLeastOneSensorWasTriggered = false;
	Thing leaderHandObject = _vm->_championMan->_leaderHandObject;
	int16 sensorCountToProcessPerCell[4];
	for (int16 i = kDMCellNorthWest; i < kDMCellSouthWest + 1; i++)
		sensorCountToProcessPerCell[i] = 0;

	Thing squareFirstThing = _vm->_dungeonMan->getSquareFirstThing(mapX, mapY);
	Thing thingBeingProcessed = squareFirstThing;
	while (thingBeingProcessed != Thing::_endOfList) {
		ThingType thingType = thingBeingProcessed.getType();
		if (thingType == kDMThingTypeSensor)
			sensorCountToProcessPerCell[thingBeingProcessed.getCell()]++;
		else if (thingType >= kDMThingTypeGroup)
			break;

		thingBeingProcessed = _vm->_dungeonMan->getNextThing(thingBeingProcessed);
	}
	for (Thing thingBeingProcessed = squareFirstThing; thingBeingProcessed != Thing::_endOfList; thingBeingProcessed = _vm->_dungeonMan->getNextThing(thingBeingProcessed)) {
		Thing lastProcessedThing = thingBeingProcessed;
		uint16 ProcessedThingType = thingBeingProcessed.getType();
		if (ProcessedThingType == kDMThingTypeSensor) {
			int16 cellIdx = thingBeingProcessed.getCell();
			sensorCountToProcessPerCell[cellIdx]--;
			Sensor *currentSensor = (Sensor *)_vm->_dungeonMan->getThingData(thingBeingProcessed);
			SensorType processedSensorType = currentSensor->getType();
			if (processedSensorType == k0_SensorDisabled)
				continue;

			if ((_vm->_championMan->_leaderIndex == kDMChampionNone) && (processedSensorType != k127_SensorWallChampionPortrait))
				continue;

			if (cellIdx != cellParam)
				continue;

			bool doNotTriggerSensor;
			int16 sensorData = 0;
			int16 sensorEffect = 0;

			sensorData = currentSensor->getData();
			sensorEffect = currentSensor->getAttrEffectA();

			switch (processedSensorType) {
			case k1_SensorWallOrnClick:
				doNotTriggerSensor = false;
				if (currentSensor->getAttrEffectA() == k3_SensorEffHold)
					continue;
				break;
			case k2_SensorWallOrnClickWithAnyObj:
				doNotTriggerSensor = (_vm->_championMan->_leaderEmptyHanded != currentSensor->getAttrRevertEffectA());
				break;
			case k17_SensorWallOrnClickWithSpecObjRemovedSensor:
			case k11_SensorWallOrnClickWithSpecObjRemovedRotateSensors:
				if (sensorCountToProcessPerCell[cellIdx]) /* If the sensor is not the last one of its type on the cell */
					continue;
				// No break on purpose
			case k3_SensorWallOrnClickWithSpecObj:
			case k4_SensorWallOrnClickWithSpecObjRemoved:
				doNotTriggerSensor = ((sensorData == _vm->_objectMan->getObjectType(leaderHandObject)) == currentSensor->getAttrRevertEffectA());
				if (!doNotTriggerSensor && (processedSensorType == k17_SensorWallOrnClickWithSpecObjRemovedSensor)) {
					if (lastProcessedThing == thingBeingProcessed) /* If the sensor is the only one of its type on the cell */
						break;
					Sensor *lastSensor = (Sensor *)_vm->_dungeonMan->getThingData(lastProcessedThing);
					lastSensor->setNextThing(currentSensor->getNextThing());
					currentSensor->setNextThing(Thing::_none);
					thingBeingProcessed = lastProcessedThing;
				}

				if (!doNotTriggerSensor && (processedSensorType == k11_SensorWallOrnClickWithSpecObjRemovedRotateSensors))
					triggerLocalEffect(k2_SensorEffToggle, mapX, mapY, cellIdx); /* This will cause a rotation of the sensors at the specified cell on the specified square after all sensors have been processed */

				break;
			case k12_SensorWallObjGeneratorRotateSensors:
				if (sensorCountToProcessPerCell[cellIdx]) /* If the sensor is not the last one of its type on the cell */
					continue;

				doNotTriggerSensor = !_vm->_championMan->_leaderEmptyHanded;
				if (!doNotTriggerSensor)
					triggerLocalEffect(k2_SensorEffToggle, mapX, mapY, cellIdx); /* This will cause a rotation of the sensors at the specified cell on the specified square after all sensors have been processed */
				break;
			case k13_SensorWallSingleObjStorageRotateSensors:
				if (_vm->_championMan->_leaderEmptyHanded) {
					leaderHandObject = getObjectOfTypeInCell(mapX, mapY, cellIdx, sensorData);
					if (leaderHandObject == Thing::_none)
						continue;

					_vm->_dungeonMan->unlinkThingFromList(leaderHandObject, Thing(0), mapX, mapY);
					_vm->_championMan->putObjectInLeaderHand(leaderHandObject, true);
				} else {
					if ((_vm->_objectMan->getObjectType(leaderHandObject) != sensorData) || (getObjectOfTypeInCell(mapX, mapY, cellIdx, sensorData) != Thing::_none))
						continue;

					_vm->_championMan->getObjectRemovedFromLeaderHand();
					_vm->_dungeonMan->linkThingToList(thingWithNewCell(leaderHandObject, cellIdx), Thing(0), mapX, mapY);
					leaderHandObject = Thing::_none;
				}
				triggerLocalEffect(k2_SensorEffToggle, mapX, mapY, cellIdx); /* This will cause a rotation of the sensors at the specified cell on the specified square after all sensors have been processed */
				if ((sensorEffect == k3_SensorEffHold) && !_vm->_championMan->_leaderEmptyHanded)
					doNotTriggerSensor = true;
				else
					doNotTriggerSensor = false;

				break;
			case k16_SensorWallObjExchanger: {
				if (sensorCountToProcessPerCell[cellIdx]) /* If the sensor is not the last one of its type on the cell */
					continue;

				Thing thingOnSquare = _vm->_dungeonMan->getSquareFirstObject(mapX, mapY);
				if ((_vm->_objectMan->getObjectType(leaderHandObject) != sensorData) || (thingOnSquare == Thing::_none))
					continue;

				_vm->_dungeonMan->unlinkThingFromList(thingOnSquare, Thing(0), mapX, mapY);
				_vm->_championMan->getObjectRemovedFromLeaderHand();
				_vm->_dungeonMan->linkThingToList(thingWithNewCell(leaderHandObject, cellIdx), Thing(0), mapX, mapY);
				_vm->_championMan->putObjectInLeaderHand(thingOnSquare, true);
				doNotTriggerSensor = false;
				}
				break;
			case k127_SensorWallChampionPortrait:
				_vm->_championMan->addCandidateChampionToParty(sensorData);
				continue;
				break;
			default:
				continue;
				break;
			}

			if (sensorEffect == k3_SensorEffHold) {
				sensorEffect = doNotTriggerSensor ? k1_SensorEffClear : k0_SensorEffSet;
				doNotTriggerSensor = false;
			}
			if (!doNotTriggerSensor) {
				atLeastOneSensorWasTriggered = true;
				if (currentSensor->getAttrAudibleA())
					_vm->_sound->requestPlay(k01_soundSWITCH, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, k1_soundModePlayIfPrioritized);

				if (!_vm->_championMan->_leaderEmptyHanded && ((processedSensorType == k4_SensorWallOrnClickWithSpecObjRemoved) || (processedSensorType == k11_SensorWallOrnClickWithSpecObjRemovedRotateSensors) || (processedSensorType == k17_SensorWallOrnClickWithSpecObjRemovedSensor))) {
					Thing *leaderThing = (Thing *)_vm->_dungeonMan->getThingData(leaderHandObject);
					*leaderThing = Thing::_none;
					_vm->_championMan->getObjectRemovedFromLeaderHand();
					leaderHandObject = Thing::_none;
				} else if (_vm->_championMan->_leaderEmptyHanded
					&& (processedSensorType == k12_SensorWallObjGeneratorRotateSensors)) {
					leaderHandObject = _vm->_dungeonMan->getObjForProjectileLaucherOrObjGen(sensorData);
					if (leaderHandObject != Thing::_none)
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
	ThingType thingType = kDMThingTypeParty;
	int16 traversedPitCount = 0;
	uint16 moveGroupResult = 0;
	uint16 thingCell = 0;
	bool thingLevitates = false;

	if (thing != Thing::_party) {
		thingType = thing.getType();
		thingCell = thing.getCell();
		thingLevitates = isLevitating(thing);
	}
	/* If moving the party or a creature on the party map from a dungeon square then check for a projectile impact */
	if ((mapX >= 0) && ((thing == Thing::_party) || ((thingType == kDMThingTypeGroup) && (_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex)))) {
		if (moveIsKilledByProjectileImpact(mapX, mapY, destMapX, destMapY, thing))
			return true; /* The specified group thing cannot be moved because it was killed by a projectile impact */
	}

	uint16 mapIndexSource = 0;
	uint16 mapIndexDestination = 0;
	bool groupOnPartyMap = false;
	bool partySquare = false;
	bool audibleTeleporter = false;

	if (destMapX >= 0) {
		mapIndexSource = mapIndexDestination = _vm->_dungeonMan->_currMapIndex;
		groupOnPartyMap = (mapIndexSource == _vm->_dungeonMan->_partyMapIndex) && (mapX >= 0);
		uint16 direction = 0;
		bool fallKilledGroup = false;
		bool drawDungeonViewWhileFalling = false;
		bool destinationIsTeleporterTarget = false;
		int16 requiredTeleporterScope;
		if (thing == Thing::_party) {
			_vm->_dungeonMan->_partyMapX = destMapX;
			_vm->_dungeonMan->_partyMapY = destMapY;
			requiredTeleporterScope = k0x0002_TelepScopeObjOrParty;
			drawDungeonViewWhileFalling = !_vm->_inventoryMan->_inventoryChampionOrdinal && !_vm->_championMan->_partyIsSleeping;
			direction = _vm->_dungeonMan->_partyDir;
		} else if (thingType == kDMThingTypeGroup)
			requiredTeleporterScope = k0x0001_TelepScopeCreatures;
		else
			requiredTeleporterScope = (k0x0001_TelepScopeCreatures | k0x0002_TelepScopeObjOrParty);

		if (thingType == kDMThingTypeProjectile) {
			Teleporter *L0712_ps_Teleporter = (Teleporter *)_vm->_dungeonMan->getThingData(thing);
			_moveResultDir = (_vm->_timeline->_events[((Projectile *)L0712_ps_Teleporter)->_eventIndex])._Cu._projectile.getDir();
		}

		int16 destinationSquareData = 0;
		/* No more than 1000 chained moves at once (in a chain of teleporters and pits for example) */
		for (int16 chainedMoveCount = 1000; --chainedMoveCount; ) {
			destinationSquareData = _vm->_dungeonMan->_currMapData[destMapX][destMapY];
			SquareType destinationSquareType = Square(destinationSquareData).getType();
			if (destinationSquareType == k5_ElementTypeTeleporter) {
				if (!getFlag(destinationSquareData, k0x0008_TeleporterOpen))
					break;

				Teleporter *teleporter = (Teleporter *)_vm->_dungeonMan->getSquareFirstThingData(destMapX, destMapY);
				if ((teleporter->getScope() == k0x0001_TelepScopeCreatures) && (thingType != kDMThingTypeGroup))
					break;

				if ((requiredTeleporterScope != (k0x0001_TelepScopeCreatures | k0x0002_TelepScopeObjOrParty)) && !getFlag(teleporter->getScope(), requiredTeleporterScope))
					break;

				destinationIsTeleporterTarget = (destMapX == teleporter->getTargetMapX()) && (destMapY == teleporter->getTargetMapY()) && (mapIndexDestination == teleporter->getTargetMapIndex());
				destMapX = teleporter->getTargetMapX();
				destMapY = teleporter->getTargetMapY();
				audibleTeleporter = teleporter->isAudible();
				_vm->_dungeonMan->setCurrentMap(mapIndexDestination = teleporter->getTargetMapIndex());
				if (thing == Thing::_party) {
					_vm->_dungeonMan->_partyMapX = destMapX;
					_vm->_dungeonMan->_partyMapY = destMapY;
					if (teleporter->isAudible())
						_vm->_sound->requestPlay(k17_soundBUZZ, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, k0_soundModePlayImmediately);

					drawDungeonViewWhileFalling = true;
					if (teleporter->getAbsoluteRotation())
						_vm->_championMan->setPartyDirection(teleporter->getRotation());
					else
						_vm->_championMan->setPartyDirection(normalizeModulo4(_vm->_dungeonMan->_partyDir + teleporter->getRotation()));
				} else {
					if (thingType == kDMThingTypeGroup) {
						if (teleporter->isAudible())
							_vm->_sound->requestPlay(k17_soundBUZZ, destMapX, destMapY, k1_soundModePlayIfPrioritized);

						moveGroupResult = getTeleporterRotatedGroupResult(teleporter, thing, mapIndexSource);
					} else {
						if (thingType == kDMThingTypeProjectile)
							thing = getTeleporterRotatedProjectileThing(teleporter, thing);
						else if (!(teleporter->getAbsoluteRotation()) && (mapX != -2))
							thing = thingWithNewCell(thing, normalizeModulo4(thing.getCell() + teleporter->getRotation()));
					}
				}
				if (destinationIsTeleporterTarget)
					break;
			} else {
				if ((destinationSquareType == k2_ElementTypePit) && !thingLevitates && getFlag(destinationSquareData, k0x0008_PitOpen) && !getFlag(destinationSquareData, k0x0001_PitImaginary)) {
					if (drawDungeonViewWhileFalling && !_useRopeToClimbDownPit) {
						drawDungeonViewWhileFalling = true;
						if (traversedPitCount) {
							_vm->_dungeonMan->setCurrentMapAndPartyMap(mapIndexDestination);
							_vm->_displayMan->loadCurrentMapGraphics();
						}
						traversedPitCount++;
						_vm->_displayMan->drawDungeon(_vm->_dungeonMan->_partyDir, destMapX, destMapY); /* BUG0_28 When falling through multiple pits the dungeon view is updated to show each traversed map but the graphics used for creatures, wall and floor ornaments may not be correct. The dungeon view is drawn for each map by using the graphics loaded for the source map. Therefore the graphics for creatures, wall and floor ornaments may not look like what they should */
																												  /* BUG0_71 Some timings are too short on fast computers. When the party falls in a series of pits, the dungeon view is refreshed too quickly because the execution speed is not limited */
																												  /* BUG0_01 While drawing creatures the engine will read invalid ACTIVE_GROUP data in _vm->_groupMan->_g375_activeGroups because the data is for the creatures on the source map and not the map being drawn. The only consequence is that creatures may be drawn with incorrect bitmaps and/or directions */
					}
					mapIndexDestination = _vm->_dungeonMan->getLocationAfterLevelChange(mapIndexDestination, 1, &destMapX, &destMapY);
					_vm->_dungeonMan->setCurrentMap(mapIndexDestination);
					if (thing == Thing::_party) {
						_vm->_dungeonMan->_partyMapX = destMapX;
						_vm->_dungeonMan->_partyMapY = destMapY;
						if (_vm->_championMan->_partyChampionCount > 0) {
							if (_useRopeToClimbDownPit) {
								Champion *curChampion = _vm->_championMan->_champions;
								for (int16 championIdx = kDMChampionFirst; championIdx < _vm->_championMan->_partyChampionCount; championIdx++, curChampion++) {
									if (curChampion->_currHealth)
										_vm->_championMan->decrementStamina(championIdx, ((curChampion->_load * 25) / _vm->_championMan->getMaximumLoad(curChampion)) + 1);
								}
							} else if (_vm->_championMan->getDamagedChampionCount(20, kDMWoundLegs | kDMWoundFeet, kDMAttackTypeSelf))
								_vm->_sound->requestPlay(k06_soundSCREAM, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, k0_soundModePlayImmediately);
						}
						_useRopeToClimbDownPit = false;
					} else if (thingType == kDMThingTypeGroup) {
						_vm->_dungeonMan->setCurrentMap(mapIndexSource);
						uint16 outcome = _vm->_groupMan->getDamageAllCreaturesOutcome((Group *)_vm->_dungeonMan->getThingData(thing), mapX, mapY, 20, false);
						_vm->_dungeonMan->setCurrentMap(mapIndexDestination);
						fallKilledGroup = (outcome == k2_outcomeKilledAllCreaturesInGroup);
						if (fallKilledGroup)
							break;

						if (outcome == k1_outcomeKilledSomeCreaturesInGroup)
							_vm->_groupMan->dropMovingCreatureFixedPossession(thing, destMapX, destMapY);
					}
				} else if ((destinationSquareType == k3_ElementTypeStairs) && (thing != Thing::_party) && (thingType != kDMThingTypeProjectile)) {
					if (!getFlag(destinationSquareData, k0x0004_StairsUp)) {
						mapIndexDestination = _vm->_dungeonMan->getLocationAfterLevelChange(mapIndexDestination, 1, &destMapX, &destMapY);
						_vm->_dungeonMan->setCurrentMap(mapIndexDestination);
					}
					direction = _vm->_dungeonMan->getStairsExitDirection(destMapX, destMapY);
					destMapX += _vm->_dirIntoStepCountEast[direction], destMapY += _vm->_dirIntoStepCountNorth[direction];
					direction = returnOppositeDir((Direction)direction);
					uint16 thingCell = thing.getCell();
					thingCell = normalizeModulo4((((thingCell - direction + 1) & 0x0002) >> 1) + direction);
					thing = thingWithNewCell(thing, thingCell);
				} else
					break;
			}
		}
		if ((thingType == kDMThingTypeGroup) && (fallKilledGroup || !_vm->_dungeonMan->isCreatureAllowedOnMap(thing, mapIndexDestination))) {
			_vm->_groupMan->dropMovingCreatureFixedPossession(thing, destMapX, destMapY);
			_vm->_groupMan->dropGroupPossessions(destMapX, destMapY, thing, k2_soundModePlayOneTickLater);
			_vm->_dungeonMan->setCurrentMap(mapIndexSource);
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
			if (thing == Thing::_party) {
				if (_vm->_dungeonMan->_partyDir == direction)
					return false;
			} else if ((_moveResultCell == thingCell) && (thingType != kDMThingTypeProjectile))
				return false;
		} else {
			if ((thing == Thing::_party) && _vm->_championMan->_partyChampionCount) {
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
				_vm->_dungeonMan->setCurrentMap(mapIndexSource);
		}
	}
	if (mapX >= 0) {
		if (thing == Thing::_party)
			processThingAdditionOrRemoval(mapX, mapY, Thing::_party, partySquare, false);
		else if (thingLevitates)
			_vm->_dungeonMan->unlinkThingFromList(thing, Thing::_none, mapX, mapY);
		else
			processThingAdditionOrRemoval(mapX, mapY, thing, (_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) && (mapX == _vm->_dungeonMan->_partyMapX) && (mapY == _vm->_dungeonMan->_partyMapY), false);
	}
	if (destMapX >= 0) {
		if (thing == Thing::_party) {
			_vm->_dungeonMan->setCurrentMap(mapIndexDestination);
			if ((thing = _vm->_groupMan->groupGetThing(_vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY)) != Thing::_endOfList) { /* Delete group if party moves onto its square */
				_vm->_groupMan->dropGroupPossessions(_vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, thing, k1_soundModePlayIfPrioritized);
				_vm->_groupMan->groupDelete(_vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY);
			}

			if (mapIndexDestination == mapIndexSource)
				processThingAdditionOrRemoval(_vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, Thing::_party, partySquare, true);
			else {
				_vm->_dungeonMan->setCurrentMap(mapIndexSource);
				_vm->_newPartyMapIndex = mapIndexDestination;
			}
		} else {
			if (thingType == kDMThingTypeGroup) {
				_vm->_dungeonMan->setCurrentMap(mapIndexDestination);
				Teleporter *L0712_ps_Teleporter = (Teleporter *)_vm->_dungeonMan->getThingData(thing);
				int16 activeGroupIndex = ((Group *)L0712_ps_Teleporter)->getActiveGroupIndex();
				if (((mapIndexDestination == _vm->_dungeonMan->_partyMapIndex) && (destMapX == _vm->_dungeonMan->_partyMapX) && (destMapY == _vm->_dungeonMan->_partyMapY)) || (_vm->_groupMan->groupGetThing(destMapX, destMapY) != Thing::_endOfList)) { /* If a group tries to move to the party square or over another group then create an event to move the group later */
					_vm->_dungeonMan->setCurrentMap(mapIndexSource);
					if (mapX >= 0)
						_vm->_groupMan->groupDeleteEvents(mapX, mapY);

					if (groupOnPartyMap)
						_vm->_groupMan->removeActiveGroup(activeGroupIndex);

					createEventMoveGroup(thing, destMapX, destMapY, mapIndexDestination, audibleTeleporter);
					return true; /* The specified group thing cannot be moved because the party or another group is on the destination square */
				}
				uint16 movementSoundIndex = getSound(((Group *)_vm->_dungeonMan->_thingData[kDMThingTypeGroup])[thing.getIndex()]._type);
				if (movementSoundIndex < k34_D13_soundCount)
					_vm->_sound->requestPlay(movementSoundIndex, destMapX, destMapY, k1_soundModePlayIfPrioritized);

				if (groupOnPartyMap && (mapIndexDestination != _vm->_dungeonMan->_partyMapIndex)) { /* If the group leaves the party map */
					_vm->_groupMan->removeActiveGroup(activeGroupIndex);
					moveGroupResult = true;
				} else if ((mapIndexDestination == _vm->_dungeonMan->_partyMapIndex) && (!groupOnPartyMap)) { /* If the group arrives on the party map */
					_vm->_groupMan->addActiveGroup(thing, destMapX, destMapY);
					moveGroupResult = true;
				}
				if (thingLevitates)
					_vm->_dungeonMan->linkThingToList(thing, Thing(0), destMapX, destMapY);
				else
					processThingAdditionOrRemoval(destMapX, destMapY, thing, false, true);

				if (moveGroupResult || (mapX < 0)) /* If group moved from one map to another or if it was just placed on a square */
					_vm->_groupMan->startWandering(destMapX, destMapY);

				_vm->_dungeonMan->setCurrentMap(mapIndexSource);
				if (mapX >= 0) {
					if (moveGroupResult > 1) /* If the group behavior was C6_BEHAVIOR_ATTACK before being teleported from and to the party map */
						_vm->_groupMan->stopAttacking(&_vm->_groupMan->_activeGroups[moveGroupResult - 2], mapX, mapY);
					else if (moveGroupResult) /* If the group was teleported or leaved the party map or entered the party map */
						_vm->_groupMan->groupDeleteEvents(mapX, mapY);
				}
				return moveGroupResult;
			}
			_vm->_dungeonMan->setCurrentMap(mapIndexDestination);
			if (thingType == kDMThingTypeProjectile) /* BUG0_29 An explosion can trigger a floor sensor. Explosions do not trigger floor sensors on the square where they are created. However, if an explosion is moved by a teleporter (or by falling into a pit, see BUG0_26) after it was created, it can trigger floor sensors on the destination square. This is because explosions are not considered as levitating in the code, while projectiles are. The condition here should be (L0713_B_ThingLevitates) so that explosions would not start sensor processing on their destination square as they should be Levitating. This would work if F0264_MOVE_IsLevitating returned true for explosions (see BUG0_26) */
				_vm->_dungeonMan->linkThingToList(thing, Thing(0), destMapX, destMapY);
			else
				processThingAdditionOrRemoval(destMapX, destMapY, thing, (_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) && (destMapX == _vm->_dungeonMan->_partyMapX) && (destMapY == _vm->_dungeonMan->_partyMapY), true);

			_vm->_dungeonMan->setCurrentMap(mapIndexSource);
		}
	}
	return false;
}

bool MovesensMan::isLevitating(Thing thing) {
	ThingType thingType = thing.getType();
	bool retVal = false;
	if (thingType == kDMThingTypeGroup)
		retVal = getFlag(_vm->_dungeonMan->getCreatureAttributes(thing), k0x0020_MaskCreatureInfo_levitation);
	else if ((thingType == kDMThingTypeProjectile) || (thingType == kDMThingTypeExplosion))
	// Fix original bug involving explosions falling in pits
		retVal = true;

	return retVal;
}

bool MovesensMan::moveIsKilledByProjectileImpact(int16 srcMapX, int16 srcMapY, int16 destMapX, int16 destMapY, Thing thing) {
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

	SquareType impactType;
	if (thing == Thing::_party) {
		impactType = kM2_ChampionElemType;
		for (uint16 cellIdx = kDMCellNorthWest; cellIdx < kDMCellSouthWest + 1; cellIdx++) {
			if (_vm->_championMan->getIndexInCell((ViewCell)cellIdx) >= 0)
				championOrCreatureOrdinalInCell[cellIdx] = _vm->indexToOrdinal(cellIdx);
		}
	} else {
		impactType = kM1_CreatureElemType;
		Group *curGroup = (Group *)_vm->_dungeonMan->getThingData(thing);
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
		int16 secondaryDirection = returnNextVal(primaryDirection);
		for (int16 i = 0; i < 4; ++i)
			intermediaryChampionOrCreatureOrdinalInCell[i] = 0;

		intermediaryChampionOrCreatureOrdinalInCell[returnPrevVal(primaryDirection)] = championOrCreatureOrdinalInCell[primaryDirection];
		if (intermediaryChampionOrCreatureOrdinalInCell[returnPrevVal(primaryDirection)])
			checkDestinationSquareProjectileImpacts = true;

		intermediaryChampionOrCreatureOrdinalInCell[returnNextVal(secondaryDirection)] = championOrCreatureOrdinalInCell[secondaryDirection];
		if (intermediaryChampionOrCreatureOrdinalInCell[returnNextVal(secondaryDirection)])
			checkDestinationSquareProjectileImpacts = true;

		if (!championOrCreatureOrdinalInCell[primaryDirection])
			championOrCreatureOrdinalInCell[primaryDirection] = championOrCreatureOrdinalInCell[returnPrevVal(primaryDirection)];

		if (!championOrCreatureOrdinalInCell[secondaryDirection])
			championOrCreatureOrdinalInCell[secondaryDirection] = championOrCreatureOrdinalInCell[returnNextVal(secondaryDirection)];
	}
	uint16 projectileMapX = srcMapX; /* Check impacts with projectiles on the source square */
	uint16 projectileMapY = srcMapY;
T0266017_CheckProjectileImpacts:
	Thing curThing = _vm->_dungeonMan->getSquareFirstThing(projectileMapX, projectileMapY);
	while (curThing != Thing::_endOfList) {
		if ((curThing.getType() == kDMThingTypeProjectile) &&
			(_vm->_timeline->_events[(((Projectile *)_vm->_dungeonMan->_thingData[kDMThingTypeProjectile])[curThing.getIndex()])._eventIndex]._type != k48_TMEventTypeMoveProjectileIgnoreImpacts)) {
			int16 championOrCreatureOrdinal = championOrCreatureOrdinalInCell[curThing.getCell()];
			if (championOrCreatureOrdinal && _vm->_projexpl->hasProjectileImpactOccurred(impactType, srcMapX, srcMapY, _vm->ordinalToIndex(championOrCreatureOrdinal), curThing)) {
				_vm->_projexpl->projectileDeleteEvent(curThing);
				if (_vm->_projexpl->_creatureDamageOutcome == k2_outcomeKilledAllCreaturesInGroup)
					return true;

				goto T0266017_CheckProjectileImpacts;
			}
		}
		curThing = _vm->_dungeonMan->getNextThing(curThing);
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

void MovesensMan::addEvent(byte type, byte mapX, byte mapY, byte cell, byte effect, int32 time) {
	TimelineEvent newEvent;
	setMapAndTime(newEvent._mapTime, _vm->_dungeonMan->_currMapIndex, time);
	newEvent._type = type;
	newEvent._priority = 0;
	newEvent._Bu._location._mapX = mapX;
	newEvent._Bu._location._mapY = mapY;
	newEvent._Cu.A._cell = cell;
	newEvent._Cu.A._effect = effect;
	_vm->_timeline->addEventGetEventIndex(&newEvent);
}

int16 MovesensMan::getSound(byte creatureType) {
	if (_vm->_championMan->_partyIsSleeping)
		return 35;

	switch (creatureType) {
	case k3_CreatureTypeWizardEyeFlyingEye:
	case k8_CreatureTypeGhostRive:
	case k11_CreatureTypeBlackFlame:
	case k19_CreatureTypeMaterializerZytaz:
	case k23_CreatureTypeLordChaos:
	case k25_CreatureTypeLordOrder:
	case k26_CreatureTypeGreyLord:
		return 35;
	case k2_CreatureTypeGiggler:
	case k9_CreatureTypeStoneGolem:
	case k10_CreatureTypeMummy:
	case k14_CreatureTypeVexirk:
	case k16_CreatureTypeTrolinAntman:
	case k22_CreatureTypeDemon:
		return k24_soundMOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON;
	case k0_CreatureTypeGiantScorpionScorpion:
	case k4_CreatureTypePainRatHellHound:
	case k5_CreatureTypeRuster:
	case k6_CreatureTypeScreamer:
	case k7_CreatureTypeRockpile:
	case k15_CreatureTypeMagnetaWormWorm:
	case k21_CreatureTypeOitu:
		return k26_soundMOVE_SCREAMER_ROCK_ROCKPILE_MAGENTA_WORM_WORM_PAIN_RAT_HELLHOUND_RUSTER_GIANT_SCORPION_SCORPION_OITU;
	case k24_CreatureTypeRedDragon:
		return k32_soundMOVE_RED_DRAGON;
	case k12_CreatureTypeSkeleton:
		return k33_soundMOVE_SKELETON;
	case k18_CreatureTypeAnimatedArmourDethKnight:
		return k22_soundMOVE_ANIMATED_ARMOUR_DETH_KNIGHT;
	case k1_CreatureTypeSwampSlimeSlime:
	case k20_CreatureTypeWaterElemental:
		return k27_soundMOVE_SWAMP_SLIME_SLIME_DEVIL_WATER_ELEMENTAL;
	case k13_CreatureTypeCouatl:
	case k17_CreatureTypeGiantWaspMuncher:
		return k23_soundMOVE_COUATL_GIANT_WASP_MUNCHER;
	}

	return 35;
}

int16 MovesensMan::getTeleporterRotatedGroupResult(Teleporter *teleporter, Thing thing, uint16 mapIndex) {
	Group *group = (Group *)_vm->_dungeonMan->getThingData(thing);
	Direction rotation = teleporter->getRotation();
	uint16 groupDirections = _vm->_groupMan->getGroupDirections(group, mapIndex);

	bool absoluteRotation = teleporter->getAbsoluteRotation();
	uint16 updatedGroupDirections;
	if (absoluteRotation)
		updatedGroupDirections = rotation;
	else
		updatedGroupDirections = normalizeModulo4(groupDirections + rotation);

	uint16 updatedGroupCells = _vm->_groupMan->getGroupCells(group, mapIndex);
	if (updatedGroupCells != k255_CreatureTypeSingleCenteredCreature) {
		int16 groupCells = updatedGroupCells;
		int16 creatureSize = getFlag(_vm->_dungeonMan->_creatureInfos[group->_type]._attributes, k0x0003_MaskCreatureInfo_size);
		int16 relativeRotation = normalizeModulo4(4 + updatedGroupDirections - groupDirections);
		for (int16 creatureIdx = 0; creatureIdx <= group->getCount(); creatureIdx++) {
			updatedGroupDirections = _vm->_groupMan->getGroupValueUpdatedWithCreatureValue(updatedGroupDirections, creatureIdx, absoluteRotation ? rotation : normalizeModulo4(groupDirections + rotation));
			if (creatureSize == k0_MaskCreatureSizeQuarter) {
				relativeRotation = absoluteRotation ? 1 : 0;
				if (relativeRotation)
					relativeRotation = rotation;
			}
			if (relativeRotation)
				updatedGroupCells = _vm->_groupMan->getGroupValueUpdatedWithCreatureValue(updatedGroupCells, creatureIdx, normalizeModulo4(groupCells + relativeRotation));

			groupDirections >>= 2;
			groupCells >>= 2;
		}
	}
	_vm->_dungeonMan->setGroupDirections(group, updatedGroupDirections, mapIndex);
	_vm->_dungeonMan->setGroupCells(group, updatedGroupCells, mapIndex);
	if ((mapIndex == _vm->_dungeonMan->_partyMapIndex) && (group->setBehaviour(k6_behavior_ATTACK)))
		return group->getActiveGroupIndex() + 2;

	return 1;
}

Thing MovesensMan::getTeleporterRotatedProjectileThing(Teleporter *teleporter, Thing projectileThing) {
	int16 updatedDirection = _moveResultDir;
	int16 rotation = teleporter->getRotation();
	if (teleporter->getAbsoluteRotation())
		updatedDirection = rotation;
	else {
		updatedDirection = normalizeModulo4(updatedDirection + rotation);
		projectileThing = thingWithNewCell(projectileThing, normalizeModulo4(projectileThing.getCell() + rotation));
	}
	_moveResultDir = updatedDirection;
	return projectileThing;
}

void MovesensMan::processThingAdditionOrRemoval(uint16 mapX, uint16 mapY, Thing thing, bool partySquare, bool addThing) {
	int16 thingType;
	IconIndice objectType;
	if (thing != Thing::_party) {
		thingType = thing.getType();
		objectType = _vm->_objectMan->getObjectType(thing);
	} else {
		thingType = kDMThingTypeParty;
		objectType = kDMIconIndiceNone;
	}

	if ((!addThing) && (thingType != kDMThingTypeParty))
		_vm->_dungeonMan->unlinkThingFromList(thing, Thing(0), mapX, mapY);

	Square curSquare = Square(_vm->_dungeonMan->_currMapData[mapX][mapY]);
	int16 sensorTriggeredCell;
	if (curSquare.getType() == k0_WallElemType)
		sensorTriggeredCell = thing.getCell();
	else
		sensorTriggeredCell = kDMCellAny; // this will wrap around

	bool squareContainsObject = false;
	bool squareContainsGroup = false;
	bool squareContainsThingOfSameType = false;
	bool squareContainsThingOfDifferentType = false;
	Thing curThing = _vm->_dungeonMan->getSquareFirstThing(mapX, mapY);
	if (sensorTriggeredCell == kDMCellAny) {
		while (curThing != Thing::_endOfList) {
			uint16 curThingType = curThing.getType();
			if (curThingType == kDMThingTypeGroup)
				squareContainsGroup = true;
			else if ((curThingType == kDMstringTypeText) && (thingType == kDMThingTypeParty) && addThing && !partySquare) {
				_vm->_dungeonMan->decodeText(_vm->_stringBuildBuffer, curThing, k1_TextTypeMessage);
				_vm->_textMan->printMessage(k15_ColorWhite, _vm->_stringBuildBuffer);
			} else if ((curThingType > kDMThingTypeGroup) && (curThingType < kDMThingTypeProjectile)) {
				squareContainsObject = true;
				squareContainsThingOfSameType |= (_vm->_objectMan->getObjectType(curThing) == objectType);
				squareContainsThingOfDifferentType |= (_vm->_objectMan->getObjectType(curThing) != objectType);
			}
			curThing = _vm->_dungeonMan->getNextThing(curThing);
		}
	} else {
		while (curThing != Thing::_endOfList) {
			if ((sensorTriggeredCell == curThing.getCell()) && (curThing.getType() > kDMThingTypeGroup)) {
				squareContainsObject = true;
				squareContainsThingOfSameType |= (_vm->_objectMan->getObjectType(curThing) == objectType);
				squareContainsThingOfDifferentType |= (_vm->_objectMan->getObjectType(curThing) != objectType);
			}
			curThing = _vm->_dungeonMan->getNextThing(curThing);
		}
	}
	if (addThing && (thingType != kDMThingTypeParty))
		_vm->_dungeonMan->linkThingToList(thing, Thing(0), mapX, mapY);

	for (curThing = _vm->_dungeonMan->getSquareFirstThing(mapX, mapY); curThing != Thing::_endOfList; curThing = _vm->_dungeonMan->getNextThing(curThing)) {
		uint16 curThingType = curThing.getType();
		if (curThingType == kDMThingTypeSensor) {
			Sensor *curSensor = (Sensor *)_vm->_dungeonMan->getThingData(curThing);
			if (curSensor->getType() == k0_SensorDisabled)
				continue;

			int16 curSensorData = curSensor->getData();
			bool triggerSensor = addThing;
			if (sensorTriggeredCell == kDMCellAny) {
				switch (curSensor->getType()) {
				case k1_SensorFloorTheronPartyCreatureObj:
					if (partySquare || squareContainsObject || squareContainsGroup) /* BUG0_30 A floor sensor is not triggered when you put an object on the floor if a levitating creature is present on the same square. The condition to determine if the sensor should be triggered checks if there is a creature on the square but does not check whether the creature is levitating. While it is normal not to trigger the sensor if there is a non levitating creature on the square (because it was already triggered by the creature itself), a levitating creature should not prevent triggering the sensor with an object. */
						continue;
					break;
				case k2_SensorFloorTheronPartyCreature:
					if ((thingType > kDMThingTypeGroup) || partySquare || squareContainsGroup)
						continue;
					break;
				case k3_SensorFloorParty:
					if ((thingType != kDMThingTypeParty) || (_vm->_championMan->_partyChampionCount == 0))
						continue;

					if (curSensorData == 0) {
						if (partySquare)
							continue;
					} else if (!addThing)
						triggerSensor = false;
					else
						triggerSensor = (curSensorData == _vm->indexToOrdinal(_vm->_dungeonMan->_partyDir));
					break;
				case k4_SensorFloorObj:
					if ((curSensorData != _vm->_objectMan->getObjectType(thing)) || squareContainsThingOfSameType)
						continue;
					break;
				case k5_SensorFloorPartyOnStairs:
					if ((thingType != kDMThingTypeParty) || (curSquare.getType() != k3_StairsElemType))
						continue;
					break;
				case k6_SensorFloorGroupGenerator:
					continue;
					break;
				case k7_SensorFloorCreature:
					if ((thingType > kDMThingTypeGroup) || (thingType == kDMThingTypeParty) || squareContainsGroup)
						continue;
					break;
				case k8_SensorFloorPartyPossession:
					if (thingType != kDMThingTypeParty)
						continue;

					triggerSensor = isObjectInPartyPossession(curSensorData);
					break;
				case k9_SensorFloorVersionChecker:
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
				case k1_SensorWallOrnClick:
					if (squareContainsObject)
						continue;
					break;
				case k2_SensorWallOrnClickWithAnyObj:
					if (squareContainsThingOfSameType || (curSensor->getData() != _vm->_objectMan->getObjectType(thing)))
						continue;
					break;
				case k3_SensorWallOrnClickWithSpecObj:
					if (squareContainsThingOfDifferentType || (curSensor->getData() == _vm->_objectMan->getObjectType(thing)))
						continue;
					break;
				default:
					continue;
					break;
				}
			}

			triggerSensor ^= curSensor->getAttrRevertEffectA();
			int16 curSensorEffect = curSensor->getAttrEffectA();
			if (curSensorEffect == k3_SensorEffHold)
				curSensorEffect = triggerSensor ? k0_SensorEffSet : k1_SensorEffClear;
			else if (!triggerSensor)
				continue;

			if (curSensor->getAttrAudibleA())
				_vm->_sound->requestPlay(k01_soundSWITCH, mapX, mapY, k1_soundModePlayIfPrioritized);

			triggerEffect(curSensor, curSensorEffect, mapX, mapY, (uint16)kDMCellAny); // this will wrap around
			continue;
		}

		if (curThingType >= kDMThingTypeGroup)
			break;
	}
	processRotationEffect();
}

bool MovesensMan::isObjectInPartyPossession(int16 objectType) {
	bool leaderHandObjectProcessed = false;
	Champion *curChampion = _vm->_championMan->_champions;
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
					Container *container = (Container *)_vm->_dungeonMan->getThingData(curThing);
					curThing = container->getSlot();
					while (curThing != Thing::_endOfList) {
						if (_vm->_objectMan->getObjectType(curThing) == objectType)
							return true;

						curThing = _vm->_dungeonMan->getNextThing(curThing);
					}
				}
			}
		}
	}
	return false;
}

void MovesensMan::triggerEffect(Sensor *sensor, int16 effect, int16 mapX, int16 mapY, uint16 cell) {
	TimelineEventType squareTypeToEventTypeArray[7] = { // @ G0059_auc_Graphic562_SquareTypeToEventType
		k6_TMEventTypeWall,
		k5_TMEventTypeCorridor,
		k9_TMEventTypePit,
		k0_TMEventTypeNone,
		k10_TMEventTypeDoor,
		k8_TMEventTypeTeleporter,
		k7_TMEventTypeFakeWall
	};

	if (sensor->getAttrOnlyOnce())
		sensor->setTypeDisabled();

	int32 endTime = _vm->_gameTime + sensor->getAttrValue();
	if (sensor->getAttrLocalEffect())
		triggerLocalEffect(sensor->getActionLocalEffect(), mapX, mapY, cell);
	else {
		int16 targetMapX = sensor->getActionTargetMapX();
		int16 targetMapY = sensor->getActionTargetMapY();
		SquareType curSquareType = Square(_vm->_dungeonMan->_currMapData[targetMapX][targetMapY]).getType();
		uint16 targetCell;
		if (curSquareType == k0_ElementTypeWall)
			targetCell = sensor->getActionTargetCell();
		else
			targetCell = kDMCellNorthWest;

		addEvent(squareTypeToEventTypeArray[curSquareType], targetMapX, targetMapY, targetCell, effect, endTime);
	}
}

void MovesensMan::triggerLocalEffect(int16 localEffect, int16 effX, int16 effY, int16 effCell) {
	if (localEffect == k10_SensorEffAddExp) {
		addSkillExperience(kDMSkillSteal, 300, localEffect != kDMCellAny);
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
	if (_sensorRotationEffect == kM1_SensorEffNone)
		return;

	switch (_sensorRotationEffect) {
	case k1_SensorEffClear:
	case k2_SensorEffToggle:
		Thing firstSensorThing = _vm->_dungeonMan->getSquareFirstThing(_sensorRotationEffMapX, _sensorRotationEffMapY);
		while ((firstSensorThing.getType() != kDMThingTypeSensor)
			|| ((_sensorRotationEffCell != kDMCellAny) && (firstSensorThing.getCell() != _sensorRotationEffCell))) {
			firstSensorThing = _vm->_dungeonMan->getNextThing(firstSensorThing);
		}
		Sensor *firstSensor = (Sensor *)_vm->_dungeonMan->getThingData(firstSensorThing);
		Thing lastSensorThing = firstSensor->getNextThing();
		while ((lastSensorThing != Thing::_endOfList)
		    && ((lastSensorThing.getType() != kDMThingTypeSensor)
				|| ((_sensorRotationEffCell != kDMCellAny) && (lastSensorThing.getCell() != _sensorRotationEffCell)))) {
			lastSensorThing = _vm->_dungeonMan->getNextThing(lastSensorThing);
		}
		if (lastSensorThing == Thing::_endOfList)
			break;
		_vm->_dungeonMan->unlinkThingFromList(firstSensorThing, Thing(0), _sensorRotationEffMapX, _sensorRotationEffMapY);
		Sensor *lastSensor = (Sensor *)_vm->_dungeonMan->getThingData(lastSensorThing);
		lastSensorThing = _vm->_dungeonMan->getNextThing(lastSensorThing);
		while (((lastSensorThing != Thing::_endOfList) && (lastSensorThing.getType() == kDMThingTypeSensor))) {
			if ((_sensorRotationEffCell == kDMCellAny) || (lastSensorThing.getCell() == _sensorRotationEffCell))
				lastSensor = (Sensor *)_vm->_dungeonMan->getThingData(lastSensorThing);
			lastSensorThing = _vm->_dungeonMan->getNextThing(lastSensorThing);
		}
		firstSensor->setNextThing(lastSensor->getNextThing());
		lastSensor->setNextThing(firstSensorThing);
	}
	_sensorRotationEffect = kM1_SensorEffNone;
}

void MovesensMan::createEventMoveGroup(Thing groupThing, int16 mapX, int16 mapY, int16 mapIndex, bool audible) {
	TimelineEvent newEvent;
	setMapAndTime(newEvent._mapTime, mapIndex, _vm->_gameTime + 5);
	newEvent._type = audible ? k61_TMEventTypeMoveGroupAudible : k60_TMEventTypeMoveGroupSilent;
	newEvent._priority = 0;
	newEvent._Bu._location._mapX = mapX;
	newEvent._Bu._location._mapY = mapY;
	newEvent._Cu._slot = groupThing.toUint16();
	_vm->_timeline->addEventGetEventIndex(&newEvent);
}

Thing MovesensMan::getObjectOfTypeInCell(int16 mapX, int16 mapY, int16 cell, int16 objectType) {
	Thing curThing = _vm->_dungeonMan->getSquareFirstObject(mapX, mapY);
	while (curThing != Thing::_endOfList) {
		if (_vm->_objectMan->getObjectType(curThing) == objectType) {
			if ((cell == kDMCellAny) || (curThing.getCell() == cell))
				return curThing;
		}
		curThing = _vm->_dungeonMan->getNextThing(curThing);
	}
	return Thing::_none;
}
}
