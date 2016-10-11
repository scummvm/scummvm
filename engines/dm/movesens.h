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


#ifndef DM_MOVESENS_H
#define DM_MOVESENS_H

#include "dm/dm.h"
#include "dm/group.h"
enum SensorEffect {
	kDMSensorEffectNone = -1, // @ CM1_EFFECT_NONE
	kDMSensorEffectSet = 0, // @ C00_EFFECT_SET
	kDMSensorEffectClear = 1, // @ C01_EFFECT_CLEAR
	kDMSensorEffectToggle = 2, // @ C02_EFFECT_TOGGLE
	kDMSensorEffectHold = 3, // @ C03_EFFECT_HOLD
	kDMSensorEffectAddExperience = 10 // @ C10_EFFECT_ADD_EXPERIENCE
};

namespace DM {
	class Sensor;
	class Teleporter;

	class MovesensMan {
	DMEngine *_vm;
public:
	explicit MovesensMan(DMEngine *vm);

	int16 _moveResultMapX; // @ G0397_i_MoveResultMapX
	int16 _moveResultMapY; // @ G0398_i_MoveResultMapY
	uint16 _moveResultMapIndex; // @ G0399_ui_MoveResultMapIndex
	int16 _moveResultDir; // @ G0400_i_MoveResultDirection
	uint16 _moveResultCell; // @ G0401_ui_MoveResultCell
	bool _useRopeToClimbDownPit; // @ G0402_B_UseRopeToClimbDownPit
	int16 _sensorRotationEffect; // @ G0403_i_SensorRotationEffect
	int16 _sensorRotationEffMapX; // @ G0404_i_SensorRotationEffectMapX
	int16 _sensorRotationEffMapY; // @ G0405_i_SensorRotationEffectMapY
	int16 _sensorRotationEffCell; // @ G0406_i_SensorRotationEffectCell

	bool sensorIsTriggeredByClickOnWall(int16 mapX, int16 mapY, uint16 cellParam); // @ F0275_SENSOR_IsTriggeredByClickOnWall
	bool getMoveResult(Thing thing, int16 mapX, int16 mapY, int16 destMapX, int16 destMapY); // @ F0267_MOVE_GetMoveResult_CPSCE
	bool isLevitating(Thing thing); // @ F0264_MOVE_IsLevitating
	bool moveIsKilledByProjectileImpact(int16 srcMapX, int16 srcMapY, int16 destMapX, int16 destMapY, Thing thing); // @ F0266_MOVE_IsKilledByProjectileImpact
	void addEvent(TimelineEventType type, byte mapX, byte mapY, Cell cell, SensorEffect effect, int32 time); // @ F0268_SENSOR_AddEvent
	int16 getSound(CreatureType creatureType); // @ F0514_MOVE_GetSound
	int16 getTeleporterRotatedGroupResult(Teleporter *teleporter, Thing thing, uint16 mapIndex);// @ F0262_MOVE_GetTeleporterRotatedGroupResult
	Thing getTeleporterRotatedProjectileThing(Teleporter *teleporter, Thing projectileThing); // @ F0263_MOVE_GetTeleporterRotatedProjectileThing
	void processThingAdditionOrRemoval(uint16 mapX, uint16 mapY, Thing thing, bool partySquare, bool addThing);// @ F0276_SENSOR_ProcessThingAdditionOrRemoval
	bool isObjectInPartyPossession(int16 objectType); // @ F0274_SENSOR_IsObjectInPartyPossession
	void triggerEffect(Sensor *sensor, SensorEffect effect, int16 mapX, int16 mapY, uint16 cell); // @ F0272_SENSOR_TriggerEffect
	void triggerLocalEffect(SensorEffect localEffect, int16 effX, int16 effY, int16 effCell); // @ F0270_SENSOR_TriggerLocalEffect
	void addSkillExperience(int16 skillIndex, uint16 exp, bool leaderOnly); // @ F0269_SENSOR_AddSkillExperience
	void processRotationEffect();// @ F0271_SENSOR_ProcessRotationEffect
	void createEventMoveGroup(Thing groupThing, int16 mapX, int16 mapY, int16 mapIndex, bool audible); // @ F0265_MOVE_CreateEvent60To61_MoveGroup
	Thing getObjectOfTypeInCell(int16 mapX, int16 mapY, int16 cell, int16 objectType); // @ F0273_SENSOR_GetObjectOfTypeInCell
};
}

#endif
