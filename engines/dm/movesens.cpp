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


#include "movesens.h"
#include "champion.h"
#include "inventory.h"
#include "dungeonman.h"
#include "objectman.h"


namespace DM {

MovesensMan::MovesensMan(DMEngine* vm) : _vm(vm) {}

bool MovesensMan::sensorIsTriggeredByClickOnWall(int16 mapX, int16 mapY, uint16 cellParam) {
	ChampionMan &champMan = *_vm->_championMan;
	DungeonMan &dunMan = *_vm->_dungeonMan;
	ObjectMan &objMan = *_vm->_objectMan;


	bool atLeastOneSensorWasTriggered = false;
	Thing leaderHandObject = champMan._leaderHandObject;
	int16 sensorCountToProcessPerCell[4];
	uint16 cell;
	for (cell = kCellNorthWest; cell < kCellSouthWest; ++cell) {
		sensorCountToProcessPerCell[cell] = 0;
	}
	Thing squareFirstThing;
	Thing thingBeingProcessed = squareFirstThing = dunMan.getSquareFirstThing(mapX, mapY);
	ThingType thingType;
	while (thingBeingProcessed != Thing::_endOfList) {
		thingType = thingBeingProcessed.getType();
		if (thingType == kSensorThingType) {
			sensorCountToProcessPerCell[thingBeingProcessed.getCell()]++;
		} else if (thingType >= kGroupThingType) {
			break;
		}
		thingBeingProcessed = dunMan.getNextThing(thingBeingProcessed);
	}
	Thing lastProcessedThing = thingBeingProcessed = squareFirstThing;

	while (thingBeingProcessed != Thing::_endOfList) {
		thingType = thingBeingProcessed.getType();
		if (thingType == kSensorThingType) {
			cell = thingBeingProcessed.getCell();
			sensorCountToProcessPerCell[cell]--;
			Sensor *sensor = (Sensor*)dunMan.getThingData(thingBeingProcessed); // IF YOU CHECK ME, I'LL CALL THE COPS!
			SensorType sensorType = sensor->getType();
			if (sensorType == kSensorDisabled)
				goto T0275058_ProceedToNextThing;
			if ((champMan._leaderIndex == kChampionNone) && (sensorType != kSensorWallChampionPortrait))
				goto T0275058_ProceedToNextThing;
			if (cell != cellParam)
				goto T0275058_ProceedToNextThing;
			int16 sensorData = sensor->getData();
			int16 sensorEffect = sensor->getEffectA();
			bool doNotTriggerSensor;
			switch (sensorType) {
			case kSensorWallOrnClick:
				doNotTriggerSensor = false;
				if (sensor->getEffectA() == kSensorEffHold) {
					goto T0275058_ProceedToNextThing;
				}
				break;
			case kSensorWallOrnClickWithAnyObj:
				doNotTriggerSensor = (champMan._leaderEmptyHanded != sensor->getRevertEffectA());
				break;
			case kSensorWallOrnClickWithSpecObjRemovedSensor:
			case kSensorWallOrnClickWithSpecObjRemovedRotateSensors:
				if (sensorCountToProcessPerCell[cell])
					goto T0275058_ProceedToNextThing;
			case kSensorWallOrnClickWithSpecObj:
			case kSensorWallOrnClickWithSpecObjRemoved:
				doNotTriggerSensor = ((sensorData == objMan.getObjectType(leaderHandObject)) == sensor->getRevertEffectA());
				if (!doNotTriggerSensor && (sensorType == kSensorWallOrnClickWithSpecObjRemovedSensor)) {
					if (lastProcessedThing == thingBeingProcessed)
						break;
					((Sensor*)dunMan.getThingData(lastProcessedThing))->setNextThing(sensor->getNextThing());
					sensor->setNextThing(Thing::_none);
					thingBeingProcessed = lastProcessedThing;
				}
				if (!doNotTriggerSensor && (sensorType == kSensorWallOrnClickWithSpecObjRemovedRotateSensors)) {
					warning("MISSING CODE: F0270_SENSOR_TriggerLocalEffect");
				}
				break;
			case kSensorWallObjGeneratorRotateSensors:
				if (sensorCountToProcessPerCell[cell])
					goto T0275058_ProceedToNextThing;
				doNotTriggerSensor = !champMan._leaderEmptyHanded;
				if (!doNotTriggerSensor) {
					warning("MISSING CODE: F0270_SENSOR_TriggerLocalEffect");
				}
				break;
			case kSensorWallSingleObjStorageRotateSensors:
				if (champMan._leaderEmptyHanded) {
					warning("MISSING CODE: F0273_SENSOR_GetObjectOfTypeInCell");
					warning("MISSING CODE: F0164_DUNGEON_UnlinkThingFromList");
					warning("MISSING CODE: F0297_CHAMPION_PutObjectInLeaderHand");
				} else {
					warning("MISSING CODE: F0273_SENSOR_GetObjectOfTypeInCell");
					warning(("MISSING CODE: F0298_CHAMPION_GetObjectRemovedFromLeaderHand"));
					warning("MISSING CODE: F0163_DUNGEON_LinkThingToList");
					leaderHandObject = Thing::_none;
				}
				warning("MISSING CODE: F0270_SENSOR_TriggerLocalEffect");
				if ((sensorEffect == kSensorEffHold) && !champMan._leaderEmptyHanded) {
					doNotTriggerSensor = true;
				} else {
					doNotTriggerSensor = false;
				}
				break;
			case kSensorWallObjExchanger: {
				if (sensorCountToProcessPerCell[cell])
					goto T0275058_ProceedToNextThing;
				Thing thingOnSquare = dunMan.getSquareFirstThing(mapX, mapY);
				if ((objMan.getObjectType(leaderHandObject) != sensorData) || (thingOnSquare == Thing::_none))
					goto T0275058_ProceedToNextThing;
				warning("MISSING CODE: F0164_DUNGEON_UnlinkThingFromList");
				warning("MISSING CODE: F0298_CHAMPION_GetObjectRemovedFromLeaderHand");
				warning("MISSING CODE: F0163_DUNGEON_LinkThingToList");
				warning("MISSING CODE: F0297_CHAMPION_PutObjectInLeaderHand");
				doNotTriggerSensor = false;
				break;
			}
			case kSensorWallChampionPortrait:
				champMan.addCandidateChampionToParty(sensorData);
				goto T0275058_ProceedToNextThing;
			default:
				goto T0275058_ProceedToNextThing;
			}

			if (sensorEffect == kSensorEffHold) {
				sensorEffect = doNotTriggerSensor ? kSensorEffClear : kSensorEffSet;
				doNotTriggerSensor = false;
			}

			if (!doNotTriggerSensor) {
				atLeastOneSensorWasTriggered = true;
				if (sensor->getAudibleA()) {
					warning("MISSING CODE: F0064_SOUND_RequestPlay_CPSD");
				}
				if (!champMan._leaderEmptyHanded &&
					((sensorType == kSensorWallOrnClickWithSpecObjRemoved) ||
					(sensorType == kSensorWallOrnClickWithSpecObjRemovedRotateSensors) ||
					 (sensorType == kSensorWallOrnClickWithSpecObjRemovedSensor))) {

					*((Thing*)dunMan.getThingData(leaderHandObject)) = Thing::_none;
					warning("MISSING CODE: F0298_CHAMPION_GetObjectRemovedFromLeaderHand");
					leaderHandObject = Thing::_none;
				} else {
					warning("MISSING CODE: (leaderHandObject = F0167_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator(sensorData)");
					if (champMan._leaderEmptyHanded && (sensorType == kSensorWallObjGeneratorRotateSensors) && (leaderHandObject != Thing::_none)) {
						warning("MISSING CODE: F0297_CHAMPION_PutObjectInLeaderHand");
					}
				}
				warning("MISSING CODE: F0272_SENSOR_TriggerEffect");
			}
			goto T0275058_ProceedToNextThing;
		}
		if (thingType >= kGroupThingType)
			break;
T0275058_ProceedToNextThing:
		lastProcessedThing = thingBeingProcessed;
		thingBeingProcessed = dunMan.getNextThing(thingBeingProcessed);
	}
	warning("MISSING CODE: F0271_SENSOR_ProcessRotationEffect");
	return atLeastOneSensorWasTriggered;
}

}
