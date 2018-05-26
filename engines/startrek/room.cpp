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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "startrek/filestream.h"
#include "startrek/iwfile.h"
#include "startrek/room.h"
#include "startrek/startrek.h"

#include "rooms/function_map.h"

namespace StarTrek {

Room::Room(StarTrekEngine *vm, const Common::String &name) : _vm(vm) {
	SharedPtr<FileStream> rdfFile = _vm->loadFile(name + ".RDF");

	int size = rdfFile->size();
	_rdfData = new byte[size];
	rdfFile->read(_rdfData, size);

	// Find room-specific code table
	if (name == "DEMON0") {
		_roomActionList = demon0ActionList;
		_numRoomActions = sizeof(demon0ActionList) / sizeof(RoomAction);
	}
	else if (name == "DEMON1") {
		_roomActionList = demon1ActionList;
		_numRoomActions = sizeof(demon1ActionList) / sizeof(RoomAction);
	}
	else if (name == "DEMON2") {
		_roomActionList = demon2ActionList;
		_numRoomActions = sizeof(demon2ActionList) / sizeof(RoomAction);
	}
	else if (name == "DEMON3") {
		_roomActionList = demon3ActionList;
		_numRoomActions = sizeof(demon3ActionList) / sizeof(RoomAction);
	}
	else {
		warning("Room \"%s\" unimplemented", name.c_str());
		_numRoomActions = 0;
	}

	memset(&_roomVar, 0, sizeof(_roomVar));
}

Room::~Room() {
	delete[] _rdfData;
}

uint16 Room::readRdfWord(int offset) {
	return _rdfData[offset] | (_rdfData[offset+1]<<8);
}

bool Room::actionHasCode(const Action &action) {
	RoomAction *roomActionPtr = _roomActionList;
	int n = _numRoomActions;

	while (n-- > 0) {
		if (action == roomActionPtr->action)
			return true;
		roomActionPtr++;
	}
	return false;
}

bool Room::handleAction(const Action &action) {
	RoomAction *roomActionPtr = _roomActionList;
	int n = _numRoomActions;

	while (n-- > 0) {
		if (action == roomActionPtr->action) {
			_vm->_awayMission.rdfStillDoDefaultAction = false;
			(this->*(roomActionPtr->funcPtr))();
			if (!_vm->_awayMission.rdfStillDoDefaultAction)
				return true;
		}
		roomActionPtr++;
	}
	return false;
}

bool Room::handleActionWithBitmask(const Action &action) {
	RoomAction *roomActionPtr = _roomActionList;
	int n = _numRoomActions;

	while (n-- > 0) {
		uint32 bitmask = action.getBitmask();
		if ((action.toUint32() & bitmask) == (roomActionPtr->action.toUint32() & bitmask)) {
			_vm->_awayMission.rdfStillDoDefaultAction = false;
			(this->*(roomActionPtr->funcPtr))();
			if (!_vm->_awayMission.rdfStillDoDefaultAction)
				return true;
		}
		roomActionPtr++;
	}
	return false;
}

Common::Point Room::getBeamInPosition(int crewmanIndex) {
	int base = 0xaa + crewmanIndex * 4;
	return Common::Point(readRdfWord(base), readRdfWord(base + 2));
}


// Interface for room-specific code

void Room::loadActorAnim(int actorIndex, Common::String anim, int16 x, int16 y, uint16 finishedAnimActionParam) {
	Actor *actor = &_vm->_actorList[actorIndex];

	if (x == -1 || y == -1) {
		x = actor->sprite.pos.x;
		y = actor->sprite.pos.y;
	}

	if (actorIndex >= 0 && actorIndex < SCALED_ACTORS_END)
		_vm->loadActorAnimWithRoomScaling(actorIndex, anim, x, y);
	else
		_vm->loadActorAnim(actorIndex, anim, x, y, 256);

	if (finishedAnimActionParam != 0) {
		actor->triggerActionWhenAnimFinished = true;
		actor->finishedAnimActionParam = finishedAnimActionParam;
	}
}

void Room::loadActorStandAnim(int actorIndex) {
	if (_vm->_awayMission.redshirtDead && actorIndex == OBJECT_REDSHIRT)
		_vm->removeActorFromScreen(actorIndex);
	else {
		Actor *actor = &_vm->_actorList[actorIndex];
		if (actor->animationString[0] == '\0')
			_vm->removeActorFromScreen(actorIndex);
		else
			_vm->initStandAnim(actorIndex);
	}
}

/**
 * This is exactly the same as "loadActorAnim", but the game calls it at different times?
 */
void Room::loadActorAnim2(int actorIndex, Common::String anim, int16 x, int16 y, uint16 finishedAnimActionParam) {
	loadActorAnim(actorIndex, anim, x, y, finishedAnimActionParam);
}

// TODO: replace "rdfOffset" with a pointer, so we no longer read from RDF files? (This
// may be necessary to support other platforms; can't leave offsets hardcoded.)
int Room::showRoomSpecificText(const char **array) {
	Common::String speaker;
	byte textColor;

	if (array[0] != nullptr && array[0][0] != '\0') { // TODO
		speaker = Common::String(array[0]);
		if (speaker.equalsIgnoreCase("Capt. Kirk"))
			textColor = TEXTCOLOR_YELLOW;
		else if (speaker.equalsIgnoreCase("Mr. Spock"))
			textColor = TEXTCOLOR_BLUE;
		else if (speaker.equalsIgnoreCase("Dr. McCoy"))
			textColor = TEXTCOLOR_BLUE;
		else if (speaker.equalsIgnoreCase("Mr. Chekov"))
			textColor = TEXTCOLOR_YELLOW;
		else if (speaker.equalsIgnoreCase("Mr. Scott"))
			textColor = TEXTCOLOR_RED;
		else if (speaker.hasPrefixIgnoreCase("Lt"))
			textColor = TEXTCOLOR_RED;
		else if (speaker.hasPrefixIgnoreCase("Ensign"))
			textColor = TEXTCOLOR_RED;
		else
			textColor = TEXTCOLOR_GREY;
	}
	else
		textColor = TEXTCOLOR_YELLOW;

	return _vm->showText(&StarTrekEngine::readTextFromArray, (uintptr)array, 20, 20, textColor, true, false, false);
}

void Room::giveItem(int item) {
	_vm->_itemList[item - ITEMS_START].have = true;
}

void Room::loadRoomIndex(int roomIndex, int spawnIndex) {
	if (_vm->_awayMission.crewDownBitset != 0)
		return;

	_vm->loadRoomIndex(roomIndex, spawnIndex);

	// This room has now been deleted, don't do anything else here.
	// FIXME: this could a bit dangerous since this is generally called from room-specific
	// code, which isn't guaranteed to do nothing afterward. Original game would
	// manipulate the stack to jump directly back to the start of "runAwayMission"...
}

void Room::walkCrewman(int actorIndex, int16 destX, int16 destY, uint16 finishedAnimActionParam) {
	if (!(actorIndex >= OBJECT_KIRK && actorIndex <= OBJECT_REDSHIRT))
		error("Tried to walk a non PC");

	Actor *actor = &_vm->_actorList[actorIndex];
	Common::String anim = _vm->getCrewmanAnimFilename(actorIndex, "walk");
	bool success = _vm->actorWalkToPosition(actorIndex, anim, actor->pos.x, actor->pos.y, destX, destY);

	if (success && finishedAnimActionParam != 0) {
		actor->triggerActionWhenAnimFinished = true;
		actor->finishedAnimActionParam = finishedAnimActionParam;
	}
}

/**
 * Loads a pair of .map and .iw files to change the room's collisions and pathfinding.
 */
void Room::loadMapFile(const Common::String &name) {
	_vm->_mapFilename = name;
	_vm->_iwFile.reset();
	_vm->_mapFile.reset();
	_vm->_iwFile = SharedPtr<IWFile>(new IWFile(_vm, name + ".iw"));
	_vm->_mapFile = _vm->loadFile(name + ".map");
}

Common::Point Room::getActorPos(int actorIndex) {
	return _vm->_actorList[actorIndex].pos;
}

void Room::playSoundEffectIndex(int soundEffect) {
	_vm->playSoundEffectIndex(soundEffect);
}

void Room::playMidiMusicTracks(int startTrack, int loopTrack) {
	_vm->playMidiMusicTracks(startTrack, loopTrack);
}

void Room::showGameOverMenu() {
	_vm->showGameOverMenu();
}

void Room::playVoc(Common::String filename) {
	_vm->_sound->playVoc(filename);
}

}
