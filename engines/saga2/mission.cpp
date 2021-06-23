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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/saga2.h"
#include "saga2/mission.h"
#include "saga2/actor.h"
#include "saga2/savefile.h"

namespace Saga2 {

ActiveMission       activeMissions[32];

//-----------------------------------------------------------------------
//	Active mission creator

ActiveMission *ActiveMission::newMission(ObjectID genID, uint16 script) {
	int             i;
	ActiveMission   *ms = NULL;

	for (i = 0; i < ARRAYSIZE(activeMissions); i++) {
		if (!(activeMissions[i]._data.missionFlags & inUse)) {
			ms = &activeMissions[i];
			break;
		}
	}

	if (!ms) return NULL;

	ms->_data.missionID   = i;
	ms->_data.generatorID = genID;
	ms->_data.missionScript = script;
	ms->_data.missionFlags |= inUse;

	ms->_data.numKnowledgeIDs = ms->_data.numObjectIDs = 0;

	memset(ms->_data.missionVars, 0, ARRAYSIZE(ms->_data.missionVars));

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

void ActiveMission::cleanup(void) {
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

void initMissions(void) {
	int     i;

	for (i = 0; i < ARRAYSIZE(activeMissions); i++)
		activeMissions[i]._data.missionFlags &= ~inUse;
}

//-----------------------------------------------------------------------
//	Save the active missions

void saveMissions(SaveFileConstructor &saveGame) {
	saveGame.writeChunk(
	    MakeID('M', 'I', 'S', 'S'),
	    &activeMissions,
	    sizeof(activeMissions));
}

//-----------------------------------------------------------------------
//	Restore the active missions

void loadMissions(SaveFileReader &saveGame) {
	saveGame.read(&activeMissions, sizeof(activeMissions));
}

} // end if namespace Saga2
