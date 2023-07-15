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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/config-manager.h"
#include "common/system.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/he/intern_he.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/he/sound_he.h"
#include "scumm/verbs.h"

namespace Scumm {

#define OPCODE(i, x)	_opcodes[i]._OPCODE(ScummEngine_v70he, x)

void ScummEngine_v70he::setupOpcodes() {
	ScummEngine_v60he::setupOpcodes();

	OPCODE(0x74, o70_soundOps);
	OPCODE(0x84, o70_pickupObject);
	OPCODE(0x8c, o70_getActorRoom);
	OPCODE(0x9b, o70_resourceRoutines);
	OPCODE(0xae, o70_systemOps);
	OPCODE(0xee, o70_getStringLen);
	OPCODE(0xf2, o70_isResourceLoaded);
	OPCODE(0xf3, o70_readINI);
	OPCODE(0xf4, o70_writeINI);
	OPCODE(0xf9, o70_createDirectory);
	OPCODE(0xfa, o70_setSystemMessage);
}

void ScummEngine_v70he::o70_soundOps() {
	int var, value;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_SOFT:
		_heSndFlags |= HE_SND_SOFT_SOUND;
		break;
	case SO_VARIABLE:
		value = pop();
		var = pop();
		_heSndSoundId = pop();
		((SoundHE *)_sound)->setSoundVar(_heSndSoundId, var, value);
		break;
	case SO_SOUND_VOLUME:
		value = pop();
		_heSndSoundId = pop();
		_sound->startSound(_heSndSoundId, 0, 0, HE_SND_VOL, 0, 0, value);
		break;
	case SO_NOW:
		_heSndFlags |= HE_SND_QUICK_START;
		break;
	case SO_SOUND_ADD:
		_heSndFlags |= HE_SND_APPEND;
		break;
	case SO_SOUND_START_VOLUME:
		// WORKAROUND: For errors in room script 240 (room 4) of maze
		break;
	case SO_SOUND_FREQUENCY:
		_heSndFrequencyShift = pop();
		break;
	case SO_SOUND_CHANNEL:
		_heSndChannel = pop();
		break;
	case SO_AT:
		_heSndOffset = pop();
		break;
	case SO_SOUND_START:
		_heSndSoundId = pop();
		_heSndOffset = 0;
		_heSndFrequencyShift = 11025;
		_heSndChannel = VAR(VAR_SOUND_CHANNEL);
		break;
	case SO_SOUND_LOOPING:
		_heSndFlags |= HE_SND_LOOP;
		break;
	case SO_END:
		_sound->startSound(_heSndSoundId, _heSndOffset, _heSndChannel, _heSndFlags, _heSndFrequencyShift);
		_heSndFlags = 0;
		break;

	default:
		error("o70_soundOps invalid case %d", subOp);
	}
}

void ScummEngine_v70he::o70_pickupObject() {
	int obj, room;

	room = pop();
	obj = pop();
	if (room == 0)
		room = getObjectRoom(obj);

	addObjectToInventory(obj, room);
	putOwner(obj, VAR(VAR_EGO));
	if (_game.heversion <= 70) {
		putClass(obj, kObjectClassUntouchable, 1);
		putState(obj, 1);
		markObjectRectAsDirty(obj);
		clearDrawObjectQueue();
	}
	runInventoryScript(obj);									/* Difference */
}

void ScummEngine_v70he::o70_getActorRoom() {
	int act = pop();

	if (act < _numActors) {
		Actor *a = derefActor(act, "o70_getActorRoom");
		push(a->_room);
	} else
		push(getObjectRoom(act));
}

void ScummEngine_v70he::o70_resourceRoutines() {
	int objidx, resid;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_LOAD_SCRIPT:
		resid = pop();
		ensureResourceLoaded(rtScript, resid);
		break;
	case SO_LOAD_SOUND:
		resid = pop();
		ensureResourceLoaded(rtSound, resid);
		break;
	case SO_LOAD_COSTUME:
		resid = pop();
		ensureResourceLoaded(rtCostume, resid);
		break;
	case SO_LOAD_ROOM:
		resid = pop();
		ensureResourceLoaded(rtRoomImage, resid);
		ensureResourceLoaded(rtRoom, resid);
		break;
	case SO_NUKE_SCRIPT:
		resid = pop();
		_res->nukeResource(rtScript, resid);
		break;
	case SO_NUKE_SOUND:
		resid = pop();
		_res->nukeResource(rtSound, resid);
		break;
	case SO_NUKE_COSTUME:
		resid = pop();
		_res->nukeResource(rtCostume, resid);
		break;
	case SO_NUKE_ROOM:
		resid = pop();
		_res->nukeResource(rtRoom, resid);
		_res->nukeResource(rtRoomImage, resid);
		break;
	case SO_LOCK_SCRIPT:
		resid = pop();
		if (resid >= _numGlobalScripts)
			break;
		_res->lock(rtScript, resid);
		break;
	case SO_LOCK_SOUND:
		resid = pop();
		_res->lock(rtSound, resid);
		break;
	case SO_LOCK_COSTUME:
		resid = pop();
		_res->lock(rtCostume, resid);
		break;
	case SO_LOCK_ROOM:
		resid = pop();
		if (_game.heversion <= 71 && resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		_res->lock(rtRoom, resid);
		_res->lock(rtRoomImage, resid);
		break;
	case SO_UNLOCK_SCRIPT:
		resid = pop();
		if (resid >= _numGlobalScripts)
			break;
		_res->unlock(rtScript, resid);
		break;
	case SO_UNLOCK_SOUND:
		resid = pop();
		_res->unlock(rtSound, resid);
		break;
	case SO_UNLOCK_COSTUME:
		resid = pop();
		_res->unlock(rtCostume, resid);
		break;
	case SO_UNLOCK_ROOM:
		resid = pop();
		if (_game.heversion <= 71 && resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		_res->unlock(rtRoom, resid);
		_res->unlock(rtRoomImage, resid);
		break;
	case SO_CLEAR_HEAP:
		// TODO: Clear Heap
		break;
	case SO_LOAD_CHARSET:
		resid = pop();
		loadCharset(resid);
		break;
	case SO_NUKE_CHARSET:
		resid = pop();
		nukeCharset(resid);
		break;
	case SO_LOAD_OBJECT:
		{
			int obj = pop();
			int room = getObjectRoom(obj);
			loadFlObject(obj, room);
			break;
		}
	case SO_PRELOAD_SCRIPT:
		resid = pop();
		if (resid >= _numGlobalScripts)
			break;
		//queueLoadResource(rtScript, resid);
		break;
	case SO_PRELOAD_SOUND:
		resid = pop();
		//queueLoadResource(rtSound, resid);
		break;
	case SO_PRELOAD_COSTUME:
		resid = pop();
		//queueLoadResource(rtCostume, resid);
		break;
	case SO_PRELOAD_ROOM:
		resid = pop();
		//queueLoadResource(rtRoomImage, resid);
		break;
	case SO_UNLOCK_IMAGE:
		resid = pop();
		_res->unlock(rtImage, resid);
		break;
	case SO_NUKE_IMAGE:
		resid = pop();
		_res->nukeResource(rtImage, resid);
		break;
	case SO_LOAD_IMAGE:
		resid = pop();
		ensureResourceLoaded(rtImage, resid);
		break;
	case SO_LOCK_IMAGE:
		resid = pop();
		_res->lock(rtImage, resid);
		break;
	case SO_PRELOAD_IMAGE:
		resid = pop();
		//queueLoadResource(rtImage, resid);
		break;
	case SO_LOCK_FLOBJECT:
		resid = pop();
		objidx = getObjectIndex(resid);
		if (objidx == -1)
			break;
		_res->lock(rtFlObject, _objs[objidx].fl_object_index);
		break;
	case SO_UNLOCK_FLOBJECT:
		resid = pop();
		objidx = getObjectIndex(resid);
		if (objidx == -1)
			break;
		_res->unlock(rtFlObject, _objs[objidx].fl_object_index);
		break;
	case SO_PRELOAD_FLUSH:
		// Used in airport
		break;
	default:
		error("o70_resourceRoutines: default case %d", subOp);
	}
}

void ScummEngine_v70he::o70_systemOps() {
	byte *src, string[256];
	int id, len;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_RESTART:
		restart();
		break;
	case SO_QUIT:
		// Confirm shutdown
		confirmExitDialog();
		break;
	case SO_QUIT_QUIT:
		quitGame();
		break;
	case SO_START_SYSTEM:
		id = pop();
		src = getStringAddress(id);
		len = resStrLen(src) + 1;
		memcpy(string, src, len);
		debug(0, "Start executable (%s)", string);
		break;
	case SO_START_SYSTEM_STRING:
		convertMessageToString(_scriptPointer, string, sizeof(string));
		len = resStrLen(_scriptPointer);
		_scriptPointer += len + 1;
		debug(0, "Start executable (%s)", string);
		break;
	case SO_RESTART_STRING:
		convertMessageToString(_scriptPointer, string, sizeof(string));
		len = resStrLen(_scriptPointer);
		_scriptPointer += len + 1;
		debug(0, "Start game (%s)", string);
		break;
	case SO_RESTART_ARRAY:
		id = pop();
		src = getStringAddress(id);
		len = resStrLen(src) + 1;
		memcpy(string, src, len);
		debug(0, "Start game (%s)", string);
		break;
	default:
		error("o70_systemOps invalid case %d", subOp);
	}
}

void ScummEngine_v70he::o70_getStringLen() {
	int id, len;
	byte *addr;

	id = pop();

	addr = getStringAddress(id);
	if (!addr)
		error("o70_getStringLen: Reference to zeroed array pointer (%d)", id);

	len = resStrLen(getStringAddress(id));
	push(len);
}

void ScummEngine_v70he::o70_isResourceLoaded() {
	// Reports percentage of resource loaded by queue
	ResType type;

	byte subOp = fetchScriptByte();
	int idx = pop();

	switch (subOp) {
	case SO_IMAGE_LOADED:
		type = rtImage;
		break;
	case SO_ROOM_LOADED:
		type = rtRoom;
		break;
	case SO_COSTUME_LOADED:
		type = rtCostume;
		break;
	case SO_SOUND_LOADED:
		type = rtSound;
		break;
	case SO_SCRIPT_LOADED:
		type = rtScript;
		break;
	default:
		error("o70_isResourceLoaded: default case %d", subOp);
	}
	debug(7, "o70_isResourceLoaded(%d,%d)", type, idx);

	push(100);
}

void ScummEngine_v70he::o70_readINI() {
	byte option[256];
	byte *data;
	const char *entry;
	int len, type;

	convertMessageToString(_scriptPointer, option, sizeof(option));
	len = resStrLen(_scriptPointer);
	_scriptPointer += len + 1;

	type = pop();
	switch (type) {
	case 1: // number
		if (!strcmp((char *)option, "NoPrinting")) {
			push(1);
		} else if (!strcmp((char *)option, "TextOn")) {
			push(ConfMan.getBool("subtitles"));
		} else {
			push(ConfMan.getInt((char *)option));
		}
		break;
	case 2: // string
		entry = (ConfMan.get((char *)option).c_str());

		writeVar(0, 0);
		len = resStrLen((const byte *)entry);
		data = defineArray(0, kStringArray, 0, len);
		memcpy(data, entry, len);

		push(readVar(0));
		break;
	default:
		error("o70_readINI: default type %d", type);
	}
	debug(1, "o70_readINI: Option %s", option);
}

void ScummEngine_v70he::o70_writeINI() {
	int type, value;
	byte option[256], string[256];
	int len;

	type = pop();
	value = pop();

	convertMessageToString(_scriptPointer, option, sizeof(option));
	len = resStrLen(_scriptPointer);
	_scriptPointer += len + 1;

	switch (type) {
	case 1: // number
		ConfMan.setInt((char *)option, value);
		debug(1, "o70_writeINI: Option %s Value %d", option, value);
		break;
	case 2: // string
		convertMessageToString(_scriptPointer, string, sizeof(string));
		len = resStrLen(_scriptPointer);
		_scriptPointer += len + 1;
		ConfMan.set((char *)option, (char *)string);
		debug(1, "o70_writeINI: Option %s String %s", option, string);
		break;
	default:
		error("o70_writeINI: default type %d", type);
	}
}

void ScummEngine_v70he::o70_createDirectory() {
	int len;
	byte directoryName[100];

	convertMessageToString(_scriptPointer, directoryName, sizeof(directoryName));
	len = resStrLen(_scriptPointer);
	_scriptPointer += len + 1;

	debug(1,"stub o70_createDirectory(%s)", directoryName);
}

void ScummEngine_v70he::o70_setSystemMessage() {
	int len;
	byte name[255];

	byte subOp = fetchScriptByte();

	convertMessageToString(_scriptPointer, name, sizeof(name));
	len = resStrLen(_scriptPointer);
	_scriptPointer += len + 1;

	switch (subOp) {
	case SO_PAUSE_MESSAGE:
		debug(1,"o70_setSystemMessage: (%d) %s", subOp, name);
		break;
	case SO_PAUSE_TITLE: // Set Version
		debug(1,"o70_setSystemMessage: (%d) %s", subOp, name);
		break;
	case SO_PAUSE_OPTION:
		debug(1,"o70_setSystemMessage: (%d) %s", subOp, name);
		break;
	case SO_TITLE_BAR: // Set Window Caption
		// TODO: The 'name' string can contain non-ASCII data. This can lead to
		// problems, because the encoding used for "name" is not clear.
		//
		// Possible fixes/workarounds:
		// - Simply stop using this. It's a rather unimportant "feature" anyway.
		// - Try to translate the text to UTF-32.
		//_system->setWindowCaption(Common::U32String((const char *)name));
		break;
	default:
		error("o70_setSystemMessage: default case %d", subOp);
	}
}

} // End of namespace Scumm
