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

#include "dm.h"

namespace DM {
	class Sensor;
	class Teleporter;

	class MovesensMan {
	DMEngine *_vm;
public:
	int16 _g397_moveResultMapX; // @ G0397_i_MoveResultMapX
	int16 _g398_moveResultMapY; // @ G0398_i_MoveResultMapY
	uint16 _g399_moveResultMapIndex; // @ G0399_ui_MoveResultMapIndex
	int16 _g400_moveResultDir; // @ G0400_i_MoveResultDirection
	uint16 _g401_moveResultCell; // @ G0401_ui_MoveResultCell
	bool _g402_useRopeToClimbDownPit; // @ G0402_B_UseRopeToClimbDownPit
	int16 _g403_sensorRotationEffect; // @ G0403_i_SensorRotationEffect
	int16 _g404_sensorRotationEffMapX; // @ G0404_i_SensorRotationEffectMapX
	int16 _g405_sensorRotationEffMapY; // @ G0405_i_SensorRotationEffectMapY
	int16 _g406_sensorRotationEffCell; // @ G0406_i_SensorRotationEffectCell
	explicit MovesensMan(DMEngine *vm);
	bool f275_sensorIsTriggeredByClickOnWall(int16 mapX, int16 mapY, uint16 cellParam); // @ F0275_SENSOR_IsTriggeredByClickOnWall
	bool f267_getMoveResult(Thing thing, int16 mapX, int16 mapY, int16 destMapX, int16 destMapY); // @ F0267_MOVE_GetMoveResult_CPSCE
	bool f264_isLevitating(Thing thing); // @ F0264_MOVE_IsLevitating
	bool f266_moveIsKilledByProjectileImpact(int16 srcMapX, int16 srcMapY, int16 destMapX, int16 destMapY, Thing thing); // @ F0266_MOVE_IsKilledByProjectileImpact
	void f268_addEvent(byte type, byte mapX, byte mapY, byte cell, byte effect, int32 time); // @ F0268_SENSOR_AddEvent
	int16 f514_getSound(byte creatureType); // @ F0514_MOVE_GetSound
	int16 f262_getTeleporterRotatedGroupResult(Teleporter *teleporter, Thing thing, uint16 mapIndex);// @ F0262_MOVE_GetTeleporterRotatedGroupResult
	Thing f263_getTeleporterRotatedProjectileThing(Teleporter *teleporter, Thing projectileThing); // @ F0263_MOVE_GetTeleporterRotatedProjectileThing
	void f276_sensorProcessThingAdditionOrRemoval(uint16 mapX, uint16 mapY, Thing thing, bool partySquare, bool addThing);// @ F0276_SENSOR_ProcessThingAdditionOrRemoval
	bool f274_sensorIsObjcetInPartyPossession(int16 objectType); // @ F0274_SENSOR_IsObjectInPartyPossession
	void f272_sensorTriggerEffect(Sensor *sensor, int16 effect, int16 mapX, int16 mapY, uint16 cell); // @ F0272_SENSOR_TriggerEffect
	void f270_sensorTriggetLocalEffect(int16 localEffect, int16 effX, int16 effY, int16 effCell); // @ F0270_SENSOR_TriggerLocalEffect
	void f269_sensorAddSkillExperience(int16 skillIndex, uint16 exp, bool leaderOnly); // @ F0269_SENSOR_AddSkillExperience
	void f271_processRotationEffect();// @ F0271_SENSOR_ProcessRotationEffect
	void f265_createEvent60to61_moveGroup(Thing groupThing, int16 mapX, int16 mapY, int16 mapIndex, bool audible); // @ F0265_MOVE_CreateEvent60To61_MoveGroup
	Thing f273_sensorGetObjectOfTypeInCell(int16 mapX, int16 mapY, int16 cell, int16 objectType); // @ F0273_SENSOR_GetObjectOfTypeInCell




};

}

#endif
