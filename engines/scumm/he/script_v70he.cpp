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
 * $URL$
 * $Id$
 *
 */

#include "common/config-manager.h"
#include "common/system.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/he/intern_he.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/he/resource_he.h"
#include "scumm/scumm.h"
#include "scumm/he/sound_he.h"
#include "scumm/verbs.h"

namespace Scumm {

#define OPCODE(i, x)	_opcodes[i]._OPCODE(ScummEngine_v70he, x)

void ScummEngine_v70he::setupOpcodes() {
	/* 00 */
	OPCODE(0x00, o6_pushByte);
	OPCODE(0x01, o6_pushWord);
	OPCODE(0x02, o6_pushByteVar);
	OPCODE(0x03, o6_pushWordVar);
	/* 04 */
	OPCODE(0x06, o6_byteArrayRead);
	OPCODE(0x07, o6_wordArrayRead);
	/* 08 */
	OPCODE(0x0a, o6_byteArrayIndexedRead);
	OPCODE(0x0b, o6_wordArrayIndexedRead);
	/* 0C */
	OPCODE(0x0c, o6_dup);
	OPCODE(0x0d, o6_not);
	OPCODE(0x0e, o6_eq);
	OPCODE(0x0f, o6_neq);
	/* 10 */
	OPCODE(0x10, o6_gt);
	OPCODE(0x11, o6_lt);
	OPCODE(0x12, o6_le);
	OPCODE(0x13, o6_ge);
	/* 14 */
	OPCODE(0x14, o6_add);
	OPCODE(0x15, o6_sub);
	OPCODE(0x16, o6_mul);
	OPCODE(0x17, o6_div);
	/* 18 */
	OPCODE(0x18, o6_land);
	OPCODE(0x19, o6_lor);
	OPCODE(0x1a, o6_pop);
	/* 1C */
	/* 20 */
	/* 24 */
	/* 28 */
	/* 2C */
	/* 30 */
	/* 34 */
	/* 38 */
	/* 3C */
	/* 40 */
	OPCODE(0x42, o6_writeByteVar);
	OPCODE(0x43, o6_writeWordVar);
	/* 44 */
	OPCODE(0x46, o6_byteArrayWrite);
	OPCODE(0x47, o6_wordArrayWrite);
	/* 48 */
	OPCODE(0x4a, o6_byteArrayIndexedWrite);
	OPCODE(0x4b, o6_wordArrayIndexedWrite);
	/* 4C */
	OPCODE(0x4e, o6_byteVarInc);
	OPCODE(0x4f, o6_wordVarInc);
	/* 50 */
	OPCODE(0x52, o6_byteArrayInc);
	OPCODE(0x53, o6_wordArrayInc);
	/* 54 */
	OPCODE(0x56, o6_byteVarDec);
	OPCODE(0x57, o6_wordVarDec);
	/* 58 */
	OPCODE(0x5a, o6_byteArrayDec);
	OPCODE(0x5b, o6_wordArrayDec);
	/* 5C */
	OPCODE(0x5c, o6_if);
	OPCODE(0x5d, o6_ifNot);
	OPCODE(0x5e, o6_startScript);
	OPCODE(0x5f, o6_startScriptQuick);
	/* 60 */
	OPCODE(0x60, o6_startObject);
	OPCODE(0x61, o6_drawObject);
	OPCODE(0x62, o6_drawObjectAt);
	/* 64 */
	OPCODE(0x65, o6_stopObjectCode);
	OPCODE(0x66, o6_stopObjectCode);
	OPCODE(0x67, o6_endCutscene);
	/* 68 */
	OPCODE(0x68, o6_cutscene);
	OPCODE(0x69, o6_stopMusic);
	OPCODE(0x6a, o6_freezeUnfreeze);
	OPCODE(0x6b, o6_cursorCommand);
	/* 6C */
	OPCODE(0x6c, o6_breakHere);
	OPCODE(0x6d, o6_ifClassOfIs);
	OPCODE(0x6e, o6_setClass);
	OPCODE(0x6f, o6_getState);
	/* 70 */
	OPCODE(0x70, o60_setState);
	OPCODE(0x71, o6_setOwner);
	OPCODE(0x72, o6_getOwner);
	OPCODE(0x73, o6_jump);
	/* 74 */
	OPCODE(0x74, o70_startSound);
	OPCODE(0x75, o6_stopSound);
	OPCODE(0x76, o6_startMusic);
	OPCODE(0x77, o6_stopObjectScript);
	/* 78 */
	OPCODE(0x78, o6_panCameraTo);
	OPCODE(0x79, o6_actorFollowCamera);
	OPCODE(0x7a, o6_setCameraAt);
	OPCODE(0x7b, o6_loadRoom);
	/* 7C */
	OPCODE(0x7c, o6_stopScript);
	OPCODE(0x7d, o6_walkActorToObj);
	OPCODE(0x7e, o6_walkActorTo);
	OPCODE(0x7f, o6_putActorAtXY);
	/* 80 */
	OPCODE(0x80, o6_putActorAtObject);
	OPCODE(0x81, o6_faceActor);
	OPCODE(0x82, o6_animateActor);
	OPCODE(0x83, o6_doSentence);
	/* 84 */
	OPCODE(0x84, o70_pickupObject);
	OPCODE(0x85, o6_loadRoomWithEgo);
	OPCODE(0x87, o6_getRandomNumber);
	/* 88 */
	OPCODE(0x88, o6_getRandomNumberRange);
	OPCODE(0x8a, o6_getActorMoving);
	OPCODE(0x8b, o6_isScriptRunning);
	/* 8C */
	OPCODE(0x8c, o70_getActorRoom);
	OPCODE(0x8d, o6_getObjectX);
	OPCODE(0x8e, o6_getObjectY);
	OPCODE(0x8f, o6_getObjectOldDir);
	/* 90 */
	OPCODE(0x90, o6_getActorWalkBox);
	OPCODE(0x91, o6_getActorCostume);
	OPCODE(0x92, o6_findInventory);
	OPCODE(0x93, o6_getInventoryCount);
	/* 94 */
	OPCODE(0x94, o6_getVerbFromXY);
	OPCODE(0x95, o6_beginOverride);
	OPCODE(0x96, o6_endOverride);
	OPCODE(0x97, o6_setObjectName);
	/* 98 */
	OPCODE(0x98, o6_isSoundRunning);
	OPCODE(0x99, o6_setBoxFlags);
	OPCODE(0x9b, o70_resourceRoutines);
	/* 9C */
	OPCODE(0x9c, o60_roomOps);
	OPCODE(0x9d, o60_actorOps);
	OPCODE(0x9e, o6_verbOps);
	OPCODE(0x9f, o6_getActorFromXY);
	/* A0 */
	OPCODE(0xa0, o6_findObject);
	OPCODE(0xa1, o6_pseudoRoom);
	OPCODE(0xa2, o6_getActorElevation);
	OPCODE(0xa3, o6_getVerbEntrypoint);
	/* A4 */
	OPCODE(0xa4, o6_arrayOps);
	OPCODE(0xa5, o6_saveRestoreVerbs);
	OPCODE(0xa6, o6_drawBox);
	OPCODE(0xa7, o6_pop);
	/* A8 */
	OPCODE(0xa8, o6_getActorWidth);
	OPCODE(0xa9, o6_wait);
	OPCODE(0xaa, o6_getActorScaleX);
	OPCODE(0xab, o6_getActorAnimCounter);
	/* AC */
	OPCODE(0xad, o6_isAnyOf);
	OPCODE(0xae, o70_systemOps);
	OPCODE(0xaf, o6_isActorInBox);
	/* B0 */
	OPCODE(0xb0, o6_delay);
	OPCODE(0xb1, o6_delaySeconds);
	OPCODE(0xb2, o6_delayMinutes);
	OPCODE(0xb3, o6_stopSentence);
	/* B4 */
	OPCODE(0xb4, o6_printLine);
	OPCODE(0xb5, o6_printText);
	OPCODE(0xb6, o6_printDebug);
	OPCODE(0xb7, o6_printSystem);
	/* B8 */
	OPCODE(0xb8, o6_printActor);
	OPCODE(0xb9, o6_printEgo);
	OPCODE(0xba, o6_talkActor);
	OPCODE(0xbb, o6_talkEgo);
	/* BC */
	OPCODE(0xbc, o6_dimArray);
	OPCODE(0xbd, o6_stopObjectCode);
	OPCODE(0xbe, o6_startObjectQuick);
	OPCODE(0xbf, o6_startScriptQuick2);
	/* C0 */
	OPCODE(0xc0, o6_dim2dimArray);
	/* C4 */
	OPCODE(0xc4, o6_abs);
	OPCODE(0xc5, o6_distObjectObject);
	OPCODE(0xc6, o6_distObjectPt);
	OPCODE(0xc7, o6_distPtPt);
	/* C8 */
	OPCODE(0xc8, o60_kernelGetFunctions);
	OPCODE(0xc9, o60_kernelSetFunctions);
	OPCODE(0xca, o6_delayFrames);
	OPCODE(0xcb, o6_pickOneOf);
	/* CC */
	OPCODE(0xcc, o6_pickOneOfDefault);
	OPCODE(0xcd, o6_stampObject);
	/* D0 */
	OPCODE(0xd0, o6_getDateTime);
	OPCODE(0xd1, o6_stopTalking);
	OPCODE(0xd2, o6_getAnimateVariable);
	/* D4 */
	OPCODE(0xd4, o6_shuffle);
	OPCODE(0xd5, o6_jumpToScript);
	OPCODE(0xd6, o6_band);
	OPCODE(0xd7, o6_bor);
	/* D8 */
	OPCODE(0xd8, o6_isRoomScriptRunning);
	OPCODE(0xd9, o60_closeFile);
	OPCODE(0xda, o60_openFile);
	OPCODE(0xdb, o60_readFile);
	/* DC */
	OPCODE(0xdc, o60_writeFile);
	OPCODE(0xdd, o6_findAllObjects);
	OPCODE(0xde, o60_deleteFile);
	OPCODE(0xdf, o60_rename);
	/* E0 */
	OPCODE(0xe0, o60_soundOps);
	OPCODE(0xe1, o6_getPixel);
	OPCODE(0xe2, o60_localizeArrayToScript);
	OPCODE(0xe3, o6_pickVarRandom);
	/* E4 */
	OPCODE(0xe4, o6_setBoxSet);
	/* E8 */
	OPCODE(0xe9, o60_seekFilePos);
	OPCODE(0xea, o60_redimArray);
	OPCODE(0xeb, o60_readFilePos);
	/* EC */
	OPCODE(0xee, o70_getStringLen);
	/* F0 */
	OPCODE(0xf2, o70_isResourceLoaded);
	OPCODE(0xf3, o70_readINI);
	/* F4 */
	OPCODE(0xf4, o70_writeINI);
	/* F8 */
	OPCODE(0xf9, o70_createDirectory);
	OPCODE(0xfa, o70_setSystemMessage);
	/* FC */
}

void ScummEngine_v70he::o70_startSound() {
	int var, value;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 9:
		_heSndFlags |= 4;
		break;
	case 23:
		value = pop();
		var = pop();
		_heSndSoundId = pop();
		((SoundHE *)_sound)->setSoundVar(_heSndSoundId, var, value);
		break;
	case 25:
		value = pop();
		_heSndSoundId = pop();
		_sound->addSoundToQueue(_heSndSoundId, 0, 0, 8);
	case 56:
		_heSndFlags |= 16;
		break;
	case 164:
		_heSndFlags |= 2;
		break;
	case 222:
		// WORKAROUND: For errors in room script 240 (room 4) of maze
		break;
	case 224:
		_heSndSoundFreq = pop();
		break;
	case 230:
		_heSndChannel = pop();
		break;
	case 231:
		_heSndOffset = pop();
		break;
	case 232:
		_heSndSoundId = pop();
		_heSndOffset = 0;
		_heSndSoundFreq = 11025;
		_heSndChannel = VAR(VAR_SOUND_CHANNEL);
		break;
	case 245:
		_heSndFlags |= 1;
		break;
	case 255:
		_sound->addSoundToQueue(_heSndSoundId, _heSndOffset, _heSndChannel, _heSndFlags);
		_heSndFlags = 0;
		break;

	default:
		error("o70_startSound invalid case %d", subOp);
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
	case 100:		// SO_LOAD_SCRIPT
		resid = pop();
		ensureResourceLoaded(rtScript, resid);
		break;
	case 101:		// SO_LOAD_SOUND
		resid = pop();
		ensureResourceLoaded(rtSound, resid);
		break;
	case 102:		// SO_LOAD_COSTUME
		resid = pop();
		ensureResourceLoaded(rtCostume, resid);
		break;
	case 103:		// SO_LOAD_ROOM
		resid = pop();
		ensureResourceLoaded(rtRoomImage, resid);
		ensureResourceLoaded(rtRoom, resid);
		break;
	case 104:		// SO_NUKE_SCRIPT
		resid = pop();
		_res->nukeResource(rtScript, resid);
		break;
	case 105:		// SO_NUKE_SOUND
		resid = pop();
		_res->nukeResource(rtSound, resid);
		break;
	case 106:		// SO_NUKE_COSTUME
		resid = pop();
		_res->nukeResource(rtCostume, resid);
		break;
	case 107:		// SO_NUKE_ROOM
		resid = pop();
		_res->nukeResource(rtRoom, resid);
		_res->nukeResource(rtRoomImage, resid);
		break;
	case 108:		// SO_LOCK_SCRIPT
		resid = pop();
		if (resid >= _numGlobalScripts)
			break;
		_res->lock(rtScript, resid);
		break;
	case 109:		// SO_LOCK_SOUND
		resid = pop();
		_res->lock(rtSound, resid);
		break;
	case 110:		// SO_LOCK_COSTUME
		resid = pop();
		_res->lock(rtCostume, resid);
		break;
	case 111:		// SO_LOCK_ROOM
		resid = pop();
		if (_game.heversion <= 71 && resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		_res->lock(rtRoom, resid);
		_res->lock(rtRoomImage, resid);
		break;
	case 112:		// SO_UNLOCK_SCRIPT
		resid = pop();
		if (resid >= _numGlobalScripts)
			break;
		_res->unlock(rtScript, resid);
		break;
	case 113:		// SO_UNLOCK_SOUND
		resid = pop();
		_res->unlock(rtSound, resid);
		break;
	case 114:		// SO_UNLOCK_COSTUME
		resid = pop();
		_res->unlock(rtCostume, resid);
		break;
	case 115:		// SO_UNLOCK_ROOM
		resid = pop();
		if (_game.heversion <= 71 && resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		_res->unlock(rtRoom, resid);
		_res->unlock(rtRoomImage, resid);
		break;
	case 116:
		// TODO: Clear Heap
		break;
	case 117:		// SO_LOAD_CHARSET
		resid = pop();
		loadCharset(resid);
		break;
	case 118:		// SO_NUKE_CHARSET
		resid = pop();
		nukeCharset(resid);
		break;
	case 119:		// SO_LOAD_OBJECT
		{
			int obj = pop();
			int room = getObjectRoom(obj);
			loadFlObject(obj, room);
			break;
		}
	case 120:
		resid = pop();
		if (resid >= _numGlobalScripts)
			break;
		//queueLoadResource(rtScript, resid);
		break;
	case 121:
		resid = pop();
		//queueLoadResource(rtSound, resid);
		break;
	case 122:
		resid = pop();
		//queueLoadResource(rtCostume, resid);
		break;
	case 123:
		resid = pop();
		//queueLoadResource(rtRoomImage, resid);
		break;
	case 159:
		resid = pop();
		_res->unlock(rtImage, resid);
		break;
	case 192:
		resid = pop();
		_res->nukeResource(rtImage, resid);
		break;
	case 201:
		resid = pop();
		ensureResourceLoaded(rtImage, resid);
		break;
	case 202:
		resid = pop();
		_res->lock(rtImage, resid);
		break;
	case 203:
		resid = pop();
		//queueLoadResource(rtImage, resid);
		break;
	case 233:
		resid = pop();
		objidx = getObjectIndex(resid);
		if (objidx == -1)
			break;
		_res->lock(rtFlObject, _objs[objidx].fl_object_index);
		break;
	case 235:
		resid = pop();
		objidx = getObjectIndex(resid);
		if (objidx == -1)
			break;
		_res->unlock(rtFlObject, _objs[objidx].fl_object_index);
		break;
	case 239:
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
	case 158:
		restart();
		break;
	case 160:
		// Confirm shutdown
		quitGame();
		break;
	case 244:
		quitGame();
		break;
	case 250:
		id = pop();
		src = getStringAddress(id);
		len = resStrLen(src) + 1;
		memcpy(string, src, len);
		debug(0, "Start executable (%s)", string);
		break;
	case 251:
		convertMessageToString(_scriptPointer, string, sizeof(string));
		len = resStrLen(_scriptPointer);
		_scriptPointer += len + 1;
		debug(0, "Start executable (%s)", string);
		break;
	case 252:
		convertMessageToString(_scriptPointer, string, sizeof(string));
		len = resStrLen(_scriptPointer);
		_scriptPointer += len + 1;
		debug(0, "Start game (%s)", string);
		break;
	case 253:
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
	int type;

	byte subOp = fetchScriptByte();
	/* int idx = */ pop();

	switch (subOp) {
	case 18:
		type = rtImage;
		break;
	case 226:
		type = rtRoom;
		break;
	case 227:
		type = rtCostume;
		break;
	case 228:
		type = rtSound;
		break;
	case 229:
		type = rtScript;
		break;
	default:
		error("o70_isResourceLoaded: default case %d", subOp);
	}

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
	case 240:
		debug(1,"o70_setSystemMessage: (%d) %s", subOp, name);
		break;
	case 241:  // Set Version
		debug(1,"o70_setSystemMessage: (%d) %s", subOp, name);
		break;
	case 242:
		debug(1,"o70_setSystemMessage: (%d) %s", subOp, name);
		break;
	case 243: // Set Window Caption
		// TODO: The 'name' string can contain non-ASCII data. This can lead to
		// problems, because (a) the encoding used for "name" is not clear,
		// (b) OSystem::setWindowCaption only supports ASCII. As a result, odd
		// behavior can occur, from strange wrong titles, up to crashes (happens
		// under Mac OS X).
		//
		// Possible fixes/workarounds:
		// - Simply stop using this. It's a rather unimportant "feature" anyway.
		// - Try to translate the text to ASCII.
		// - Refine OSystem to accept window captions that are non-ASCII, e.g.
		//   by enhancing all backends to deal with UTF-8 data. Of course, then
		//   one still would have to convert 'name' to the correct encoding.
		//_system->setWindowCaption((const char *)name);
		break;
	default:
		error("o70_setSystemMessage: default case %d", subOp);
	}
}

} // End of namespace Scumm
