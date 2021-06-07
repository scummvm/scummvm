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

#include "saga2/std.h"
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

	for (i = 0; i < elementsof(activeMissions); i++) {
		if (!(activeMissions[i].missionFlags & inUse)) {
			ms = &activeMissions[i];
			break;
		}
	}

	if (!ms) return NULL;

	GameObject      *generator = GameObject::objectAddress(genID);

	ms->missionID   = i;
	ms->generatorID = genID;
	ms->missionScript = script;
	ms->missionFlags |= inUse;

	ms->numKnowledgeIDs = ms->numObjectIDs = 0;

	memset(ms->missionVars, 0, elementsof(ms->missionVars));

	return ms;
}

//-----------------------------------------------------------------------
//	Search for a specific mission

int ActiveMission::findMission(ObjectID genID) {
	int             i;

	for (i = 0; i < elementsof(activeMissions); i++) {
		if (activeMissions[i].missionFlags & inUse
		        &&  activeMissions[i].generatorID == genID) {
			return i;
		}
	}

	return -1;
}

ActiveMission *ActiveMission::missionAddress(int index) {
	assert(index >= 0);
	assert(index < elementsof(activeMissions));

	return &activeMissions[index];
}

//-----------------------------------------------------------------------
//	Add record of object creation to mission

bool ActiveMission::addObjectID(ObjectID objID) {
	if (numObjectIDs < elementsof(missionObjectList)) {
		missionObjectList[numObjectIDs++] = objID;
		return TRUE;
	}
	return FALSE;
}

//-----------------------------------------------------------------------
//	Add record of object creation to mission

bool ActiveMission::removeObjectID(ObjectID objID) {
	bool            found = FALSE;

	for (int i = 0; i < numObjectIDs; i++) {
		if (found) {
			missionObjectList[i - 1] = missionObjectList[i];
		} else {
			if (missionObjectList[i] == objID) found = TRUE;
		}
	}

	if (found) numObjectIDs--;

	return found;
}

//-----------------------------------------------------------------------
//	Add record of knowledge creation to mission

bool ActiveMission::addKnowledgeID(ObjectID actor, uint16 knowledgeID) {
	if (!isActor(actor)) return FALSE;

	if (numKnowledgeIDs < elementsof(missionKnowledgeList)) {
		Actor       *a = (Actor *)GameObject::objectAddress(actor);

		if (!a->addKnowledge(knowledgeID)) return FALSE;

		missionKnowledgeList[numKnowledgeIDs].id = actor;
		missionKnowledgeList[numKnowledgeIDs++].kID = knowledgeID;
		return TRUE;
	}
	return FALSE;
}

//-----------------------------------------------------------------------
//	Add record of knowledge creation to mission

bool ActiveMission::removeKnowledgeID(ObjectID actor, uint16 knowledgeID) {
	bool            found = FALSE;

	for (int i = 0; i < numKnowledgeIDs; i++) {
		if (found) {
			missionKnowledgeList[i - 1] = missionKnowledgeList[i];
		} else {
			if (missionKnowledgeList[i].id  == actor
			        &&  missionKnowledgeList[i].kID == knowledgeID) {
				found = TRUE;
			}
		}
	}

	if (found) numKnowledgeIDs--;

	return found;
}

//-----------------------------------------------------------------------
//	Add record of knowledge creation to mission

void ActiveMission::cleanup(void) {
	int             i;

	for (i = 0; i < numKnowledgeIDs; i++) {
		Actor       *a;

		a = (Actor *)GameObject::objectAddress(missionKnowledgeList[i].id);

		a->removeKnowledge(missionKnowledgeList[i].kID);
	}

	for (i = 0; i < numObjectIDs; i++) {
		GameObject      *obj;

		obj = GameObject::objectAddress(missionObjectList[i]);

		obj->deleteObjectRecursive();
	}

	numKnowledgeIDs = numObjectIDs = 0;
	missionFlags &= ~inUse;
}

//-----------------------------------------------------------------------
//	Initialize the active mission list

void initMissions(void) {
	int     i;

	for (i = 0; i < elementsof(activeMissions); i++)
		activeMissions[i].missionFlags &= ~ActiveMission::inUse;
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
