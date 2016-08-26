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

bool MovesensMan::f275_sensorIsTriggeredByClickOnWall(int16 mapX, int16 mapY, uint16 cellParam) {
	ChampionMan &champMan = *_vm->_championMan;
	DungeonMan &dunMan = *_vm->_dungeonMan;
	ObjectMan &objMan = *_vm->_objectMan;


	bool atLeastOneSensorWasTriggered = false;
	Thing leaderHandObject = champMan._g414_leaderHandObject;
	int16 sensorCountToProcessPerCell[4];
	uint16 cell;
	for (cell = k0_CellNorthWest; cell < k3_CellSouthWest; ++cell) {
		sensorCountToProcessPerCell[cell] = 0;
	}
	Thing squareFirstThing;
	Thing thingBeingProcessed = squareFirstThing = dunMan.f161_getSquareFirstThing(mapX, mapY);
	ThingType thingType;
	while (thingBeingProcessed != Thing::_endOfList) {
		thingType = thingBeingProcessed.getType();
		if (thingType == k3_SensorThingType) {
			sensorCountToProcessPerCell[thingBeingProcessed.getCell()]++;
		} else if (thingType >= k4_GroupThingType) {
			break;
		}
		thingBeingProcessed = dunMan.f159_getNextThing(thingBeingProcessed);
	}
	Thing lastProcessedThing = thingBeingProcessed = squareFirstThing;

	while (thingBeingProcessed != Thing::_endOfList) {
		thingType = thingBeingProcessed.getType();
		if (thingType == k3_SensorThingType) {
			cell = thingBeingProcessed.getCell();
			sensorCountToProcessPerCell[cell]--;
			Sensor *sensor = (Sensor*)dunMan.f156_getThingData(thingBeingProcessed); // IF YOU CHECK ME, I'LL CALL THE COPS!
			SensorType sensorType = sensor->getType();
			if (sensorType == k0_SensorDisabled)
				goto T0275058_ProceedToNextThing;
			if ((champMan._g411_leaderIndex == kM1_ChampionNone) && (sensorType != k127_SensorWallChampionPortrait))
				goto T0275058_ProceedToNextThing;
			if (cell != cellParam)
				goto T0275058_ProceedToNextThing;
			int16 sensorData = sensor->getData();
			int16 sensorEffect = sensor->getEffectA();
			bool doNotTriggerSensor;
			switch (sensorType) {
			case k1_SensorWallOrnClick:
				doNotTriggerSensor = false;
				if (sensor->getEffectA() == k3_SensorEffHold) {
					goto T0275058_ProceedToNextThing;
				}
				break;
			case k2_SensorWallOrnClickWithAnyObj:
				doNotTriggerSensor = (champMan._g415_leaderEmptyHanded != sensor->getRevertEffectA());
				break;
			case k17_SensorWallOrnClickWithSpecObjRemovedSensor:
			case k11_SensorWallOrnClickWithSpecObjRemovedRotateSensors:
				if (sensorCountToProcessPerCell[cell])
					goto T0275058_ProceedToNextThing;
			case k3_SensorWallOrnClickWithSpecObj:
			case k4_SensorWallOrnClickWithSpecObjRemoved:
				doNotTriggerSensor = ((sensorData == objMan.f32_getObjectType(leaderHandObject)) == sensor->getRevertEffectA());
				if (!doNotTriggerSensor && (sensorType == k17_SensorWallOrnClickWithSpecObjRemovedSensor)) {
					if (lastProcessedThing == thingBeingProcessed)
						break;
					((Sensor*)dunMan.f156_getThingData(lastProcessedThing))->setNextThing(sensor->getNextThing());
					sensor->setNextThing(Thing::_none);
					thingBeingProcessed = lastProcessedThing;
				}
				if (!doNotTriggerSensor && (sensorType == k11_SensorWallOrnClickWithSpecObjRemovedRotateSensors)) {
					warning("MISSING CODE: F0270_SENSOR_TriggerLocalEffect");
				}
				break;
			case k12_SensorWallObjGeneratorRotateSensors:
				if (sensorCountToProcessPerCell[cell])
					goto T0275058_ProceedToNextThing;
				doNotTriggerSensor = !champMan._g415_leaderEmptyHanded;
				if (!doNotTriggerSensor) {
					warning("MISSING CODE: F0270_SENSOR_TriggerLocalEffect");
				}
				break;
			case k13_SensorWallSingleObjStorageRotateSensors:
				if (champMan._g415_leaderEmptyHanded) {
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
				if ((sensorEffect == k3_SensorEffHold) && !champMan._g415_leaderEmptyHanded) {
					doNotTriggerSensor = true;
				} else {
					doNotTriggerSensor = false;
				}
				break;
			case k16_SensorWallObjExchanger: {
				if (sensorCountToProcessPerCell[cell])
					goto T0275058_ProceedToNextThing;
				Thing thingOnSquare = dunMan.f161_getSquareFirstThing(mapX, mapY);
				if ((objMan.f32_getObjectType(leaderHandObject) != sensorData) || (thingOnSquare == Thing::_none))
					goto T0275058_ProceedToNextThing;
				warning("MISSING CODE: F0164_DUNGEON_UnlinkThingFromList");
				warning("MISSING CODE: F0298_CHAMPION_GetObjectRemovedFromLeaderHand");
				warning("MISSING CODE: F0163_DUNGEON_LinkThingToList");
				warning("MISSING CODE: F0297_CHAMPION_PutObjectInLeaderHand");
				doNotTriggerSensor = false;
				break;
			}
			case k127_SensorWallChampionPortrait:
				champMan.f280_addCandidateChampionToParty(sensorData);
				goto T0275058_ProceedToNextThing;
			default:
				goto T0275058_ProceedToNextThing;
			}

			if (sensorEffect == k3_SensorEffHold) {
				sensorEffect = doNotTriggerSensor ? k1_SensorEffClear : k0_SensorEffSet;
				doNotTriggerSensor = false;
			}

			if (!doNotTriggerSensor) {
				atLeastOneSensorWasTriggered = true;
				if (sensor->getAudibleA()) {
					warning("MISSING CODE: F0064_SOUND_RequestPlay_CPSD");
				}
				if (!champMan._g415_leaderEmptyHanded &&
					((sensorType == k4_SensorWallOrnClickWithSpecObjRemoved) ||
					(sensorType == k11_SensorWallOrnClickWithSpecObjRemovedRotateSensors) ||
					 (sensorType == k17_SensorWallOrnClickWithSpecObjRemovedSensor))) {

					*((Thing*)dunMan.f156_getThingData(leaderHandObject)) = Thing::_none;
					warning("MISSING CODE: F0298_CHAMPION_GetObjectRemovedFromLeaderHand");
					leaderHandObject = Thing::_none;
				} else {
					warning("MISSING CODE: (leaderHandObject = F0167_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator(sensorData)");
					if (champMan._g415_leaderEmptyHanded && (sensorType == k12_SensorWallObjGeneratorRotateSensors) && (leaderHandObject != Thing::_none)) {
						warning("MISSING CODE: F0297_CHAMPION_PutObjectInLeaderHand");
					}
				}
				warning("MISSING CODE: F0272_SENSOR_TriggerEffect");
			}
			goto T0275058_ProceedToNextThing;
		}
		if (thingType >= k4_GroupThingType)
			break;
T0275058_ProceedToNextThing:
		lastProcessedThing = thingBeingProcessed;
		thingBeingProcessed = dunMan.f159_getNextThing(thingBeingProcessed);
	}
	warning("MISSING CODE: F0271_SENSOR_ProcessRotationEffect");
	return atLeastOneSensorWasTriggered;
}

}
