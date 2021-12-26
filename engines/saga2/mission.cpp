/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/mission.h"
#include "saga2/actor.h"

namespace Saga2 {

ActiveMission       activeMissions[32];

//-----------------------------------------------------------------------
//	Active mission creator

ActiveMission *ActiveMission::newMission(ObjectID genID, uint16 script) {
	int             i;
	ActiveMission   *ms = nullptr;

	for (i = 0; i < ARRAYSIZE(activeMissions); i++) {
		if (!(activeMissions[i]._data.missionFlags & inUse)) {
			ms = &activeMissions[i];
			break;
		}
	}

	if (!ms) return nullptr;

	ms->_data.missionID   = i;
	ms->_data.generatorID = genID;
	ms->_data.missionScript = script;
	ms->_data.missionFlags |= inUse;

	ms->_data.numKnowledgeIDs = ms->_data.numObjectIDs = 0;

	memset(ms->_data.missionVars, 0, ARRAYSIZE(ms->_data.missionVars));

	ms->_data.aMission = ms;

	return ms;
}

//-----------------------------------------------------------------------
//	Search for a specific mission

int ActiveMission::findMission(ObjectID genID) {
	int             i;

	for (i = 0; i < ARRAYSIZE(activeMissions); i++) {
		if (activeMissions[i]._data.missionFlags & inUse
		        &&  activeMissions[i]._data.generatorID == genID) {
			return i;
		}
	}

	return -1;
}

ActiveMission *ActiveMission::missionAddress(int index) {
	assert(index >= 0);
	assert(index < ARRAYSIZE(activeMissions));

	return &activeMissions[index];
}

//-----------------------------------------------------------------------
//	Add record of object creation to mission

bool ActiveMission::addObjectID(ObjectID objID) {
	if (_data.numObjectIDs < ARRAYSIZE(_data.missionObjectList)) {
		_data.missionObjectList[_data.numObjectIDs++] = objID;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------
//	Add record of object creation to mission

bool ActiveMission::removeObjectID(ObjectID objID) {
	bool            found = false;

	for (int i = 0; i < _data.numObjectIDs; i++) {
		if (found) {
			_data.missionObjectList[i - 1] = _data.missionObjectList[i];
		} else {
			if (_data.missionObjectList[i] == objID)
				found = true;
		}
	}

	if (found)
		_data.numObjectIDs--;

	return found;
}

//-----------------------------------------------------------------------
//	Add record of knowledge creation to mission

bool ActiveMission::addKnowledgeID(ObjectID actor, uint16 knowledgeID) {
	if (!isActor(actor)) return false;

	if (_data.numKnowledgeIDs < ARRAYSIZE(_data.missionKnowledgeList)) {
		Actor       *a = (Actor *)GameObject::objectAddress(actor);

		if (!a->addKnowledge(knowledgeID)) return false;

		_data.missionKnowledgeList[_data.numKnowledgeIDs].id = actor;
		_data.missionKnowledgeList[_data.numKnowledgeIDs++].kID = knowledgeID;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------
//	Add record of knowledge creation to mission

bool ActiveMission::removeKnowledgeID(ObjectID actor, uint16 knowledgeID) {
	bool            found = false;

	for (int i = 0; i < _data.numKnowledgeIDs; i++) {
		if (found) {
			_data.missionKnowledgeList[i - 1] = _data.missionKnowledgeList[i];
		} else {
			if (_data.missionKnowledgeList[i].id  == actor
			        &&  _data.missionKnowledgeList[i].kID == knowledgeID) {
				found = true;
			}
		}
	}

	if (found)
		_data.numKnowledgeIDs--;

	return found;
}

//-----------------------------------------------------------------------
//	Add record of knowledge creation to mission

void ActiveMission::read(Common::InSaveFile *in) {
	_data.missionID = in->readUint16LE();
	_data.generatorID = in->readUint16LE();
	_data.missionScript = in->readUint16LE();
	_data.missionFlags = in->readUint16LE();

	debugC(4, kDebugSaveload, "... missionID = %d", _data.missionID);
	debugC(4, kDebugSaveload, "... generatorID = %d", _data.generatorID);
	debugC(4, kDebugSaveload, "... missionScript = %d", _data.missionScript);
	debugC(4, kDebugSaveload, "... missionFlags = %d", _data.missionFlags);

	for (int i = 0; i < ARRAYSIZE(_data.missionVars); ++i) {
		_data.missionVars[i] = in->readByte();
		debugC(5, kDebugSaveload, "... missionVars[%d] = %d", i, _data.missionVars[i]);
	}

	for (int i = 0; i < ARRAYSIZE(_data.missionObjectList); ++i) {
		_data.missionObjectList[i] = in->readUint16LE();
		debugC(5, kDebugSaveload, "... missionObjectList[%d] = %d", i, _data.missionObjectList[i]);
	}

	for (int i = 0; i < ARRAYSIZE(_data.missionKnowledgeList); ++i) {
		_data.missionKnowledgeList[i].id = in->readUint16LE();

		_data.missionKnowledgeList[i].kID = in->readUint16LE();

		debugC(5, kDebugSaveload, "... missionKnowledgeList[%d].id = %d", i, _data.missionKnowledgeList[i].id);
		debugC(5, kDebugSaveload, "... missionKnowledgeList[%d].kID = %d", i, _data.missionKnowledgeList[i].kID);
	}

	_data.numObjectIDs = in->readUint16LE();
	_data.numKnowledgeIDs = in->readUint16LE();

	_data.aMission = this;

	debugC(4, kDebugSaveload, "... numObjectIDs = %d", _data.numObjectIDs);
	debugC(4, kDebugSaveload, "... numKnowledgeIDs = %d", _data.numKnowledgeIDs);
}

void ActiveMission::write(Common::MemoryWriteStreamDynamic *out) {
	out->writeUint16LE(_data.missionID);
	out->writeUint16LE(_data.generatorID);
	out->writeUint16LE(_data.missionScript);
	out->writeUint16LE(_data.missionFlags);

	debugC(4, kDebugSaveload, "... missionID = %d", _data.missionID);
	debugC(4, kDebugSaveload, "... generatorID = %d", _data.generatorID);
	debugC(4, kDebugSaveload, "... missionScript = %d", _data.missionScript);
	debugC(4, kDebugSaveload, "... missionFlags = %d", _data.missionFlags);

	for (int i = 0; i < ARRAYSIZE(_data.missionVars); ++i) {
		out->writeByte(_data.missionVars[i]);
		debugC(5, kDebugSaveload, "... missionVars[%d] = %d", i, _data.missionVars[i]);
	}

	for (int i = 0; i < ARRAYSIZE(_data.missionObjectList); ++i) {
		out->writeUint16LE(_data.missionObjectList[i]);
		debugC(5, kDebugSaveload, "... missionObjectList[%d] = %d", i, _data.missionObjectList[i]);
	}

	for (int i = 0; i < ARRAYSIZE(_data.missionKnowledgeList); ++i) {
		out->writeUint16LE(_data.missionKnowledgeList[i].id);
		out->writeUint16LE(_data.missionKnowledgeList[i].kID);

		debugC(5, kDebugSaveload, "... missionKnowledgeList[%d].id = %d", i, _data.missionKnowledgeList[i].id);
		debugC(5, kDebugSaveload, "... missionKnowledgeList[%d].kID = %d", i, _data.missionKnowledgeList[i].kID);
	}

	out->writeUint16LE(_data.numObjectIDs);
	out->writeUint16LE(_data.numKnowledgeIDs);

	debugC(4, kDebugSaveload, "... numObjectIDs = %d", _data.numObjectIDs);
	debugC(4, kDebugSaveload, "... numKnowledgeIDs = %d", _data.numKnowledgeIDs);
}

void ActiveMission::cleanup() {
	int             i;

	for (i = 0; i < _data.numKnowledgeIDs; i++) {
		Actor       *a;

		a = (Actor *)GameObject::objectAddress(_data.missionKnowledgeList[i].id);

		a->removeKnowledge(_data.missionKnowledgeList[i].kID);
	}

	for (i = 0; i < _data.numObjectIDs; i++) {
		GameObject      *obj;

		obj = GameObject::objectAddress(_data.missionObjectList[i]);

		obj->deleteObjectRecursive();
	}

	_data.numKnowledgeIDs = _data.numObjectIDs = 0;
	_data.missionFlags &= ~inUse;
}

//-----------------------------------------------------------------------
//	Initialize the active mission list

void initMissions() {
	int     i;

	for (i = 0; i < ARRAYSIZE(activeMissions); i++)
		activeMissions[i]._data.missionFlags &= ~inUse;
}

void saveMissions(Common::OutSaveFile *outS) {
	debugC(2, kDebugSaveload, "Saving Missions");

	outS->write("MISS", 4);
	CHUNK_BEGIN;
	for (int i = 0; i < ARRAYSIZE(activeMissions); ++i) {
		debugC(3, kDebugSaveload, "Saving Mission %d", i);
		activeMissions[i].write(out);
	}
	CHUNK_END;
}

void loadMissions(Common::InSaveFile *in) {
	debugC(2, kDebugSaveload, "Loading Missions");

	for (int i = 0; i < ARRAYSIZE(activeMissions); ++i) {
		activeMissions[i].read(in);
		debugC(3, kDebugSaveload, "Loading Mission %d", i);
	}
}

} // end if namespace Saga2
