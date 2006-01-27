/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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
 * $Header$
 *
 */

#include "common/stdafx.h"

#include "common/config-manager.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/resource_v7he.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/verbs.h"

namespace Scumm {

#define OPCODE(x)	_OPCODE(ScummEngine_v70he, x)

void ScummEngine_v70he::setupOpcodes() {
	static const OpcodeEntryv70he opcodes[256] = {
		/* 00 */
		OPCODE(o6_pushByte),
		OPCODE(o6_pushWord),
		OPCODE(o6_pushByteVar),
		OPCODE(o6_pushWordVar),
		/* 04 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteArrayRead),
		OPCODE(o6_wordArrayRead),
		/* 08 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteArrayIndexedRead),
		OPCODE(o6_wordArrayIndexedRead),
		/* 0C */
		OPCODE(o6_dup),
		OPCODE(o6_not),
		OPCODE(o6_eq),
		OPCODE(o6_neq),
		/* 10 */
		OPCODE(o6_gt),
		OPCODE(o6_lt),
		OPCODE(o6_le),
		OPCODE(o6_ge),
		/* 14 */
		OPCODE(o6_add),
		OPCODE(o6_sub),
		OPCODE(o6_mul),
		OPCODE(o6_div),
		/* 18 */
		OPCODE(o6_land),
		OPCODE(o6_lor),
		OPCODE(o6_pop),
		OPCODE(o6_invalid),
		/* 1C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 20 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 24 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 28 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 2C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 30 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 34 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 38 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 3C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 40 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_writeByteVar),
		OPCODE(o6_writeWordVar),
		/* 44 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteArrayWrite),
		OPCODE(o6_wordArrayWrite),
		/* 48 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteArrayIndexedWrite),
		OPCODE(o6_wordArrayIndexedWrite),
		/* 4C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteVarInc),
		OPCODE(o6_wordVarInc),
		/* 50 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteArrayInc),
		OPCODE(o6_wordArrayInc),
		/* 54 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteVarDec),
		OPCODE(o6_wordVarDec),
		/* 58 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_byteArrayDec),
		OPCODE(o6_wordArrayDec),
		/* 5C */
		OPCODE(o6_if),
		OPCODE(o6_ifNot),
		OPCODE(o6_startScript),
		OPCODE(o6_startScriptQuick),
		/* 60 */
		OPCODE(o6_startObject),
		OPCODE(o6_drawObject),
		OPCODE(o6_drawObjectAt),
		OPCODE(o6_invalid),
		/* 64 */
		OPCODE(o6_invalid),
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_endCutscene),
		/* 68 */
		OPCODE(o6_cutscene),
		OPCODE(o6_stopMusic),
		OPCODE(o6_freezeUnfreeze),
		OPCODE(o6_cursorCommand),
		/* 6C */
		OPCODE(o6_breakHere),
		OPCODE(o6_ifClassOfIs),
		OPCODE(o6_setClass),
		OPCODE(o6_getState),
		/* 70 */
		OPCODE(o60_setState),
		OPCODE(o6_setOwner),
		OPCODE(o6_getOwner),
		OPCODE(o6_jump),
		/* 74 */
		OPCODE(o70_startSound),
		OPCODE(o6_stopSound),
		OPCODE(o6_startMusic),
		OPCODE(o6_stopObjectScript),
		/* 78 */
		OPCODE(o6_panCameraTo),
		OPCODE(o6_actorFollowCamera),
		OPCODE(o6_setCameraAt),
		OPCODE(o6_loadRoom),
		/* 7C */
		OPCODE(o6_stopScript),
		OPCODE(o6_walkActorToObj),
		OPCODE(o6_walkActorTo),
		OPCODE(o6_putActorAtXY),
		/* 80 */
		OPCODE(o6_putActorAtObject),
		OPCODE(o6_faceActor),
		OPCODE(o6_animateActor),
		OPCODE(o6_doSentence),
		/* 84 */
		OPCODE(o70_pickupObject),
		OPCODE(o6_loadRoomWithEgo),
		OPCODE(o6_invalid),
		OPCODE(o6_getRandomNumber),
		/* 88 */
		OPCODE(o6_getRandomNumberRange),
		OPCODE(o6_invalid),
		OPCODE(o6_getActorMoving),
		OPCODE(o6_isScriptRunning),
		/* 8C */
		OPCODE(o70_getActorRoom),
		OPCODE(o6_getObjectX),
		OPCODE(o6_getObjectY),
		OPCODE(o6_getObjectOldDir),
		/* 90 */
		OPCODE(o6_getActorWalkBox),
		OPCODE(o6_getActorCostume),
		OPCODE(o6_findInventory),
		OPCODE(o6_getInventoryCount),
		/* 94 */
		OPCODE(o6_getVerbFromXY),
		OPCODE(o6_beginOverride),
		OPCODE(o6_endOverride),
		OPCODE(o6_setObjectName),
		/* 98 */
		OPCODE(o6_isSoundRunning),
		OPCODE(o6_setBoxFlags),
		OPCODE(o6_invalid),
		OPCODE(o70_resourceRoutines),
		/* 9C */
		OPCODE(o60_roomOps),
		OPCODE(o60_actorOps),
		OPCODE(o6_verbOps),
		OPCODE(o6_getActorFromXY),
		/* A0 */
		OPCODE(o6_findObject),
		OPCODE(o6_pseudoRoom),
		OPCODE(o6_getActorElevation),
		OPCODE(o6_getVerbEntrypoint),
		/* A4 */
		OPCODE(o6_arrayOps),
		OPCODE(o6_saveRestoreVerbs),
		OPCODE(o6_drawBox),
		OPCODE(o6_pop),
		/* A8 */
		OPCODE(o6_getActorWidth),
		OPCODE(o60_wait),
		OPCODE(o6_getActorScaleX),
		OPCODE(o6_getActorAnimCounter1),
		/* AC */
		OPCODE(o6_invalid),
		OPCODE(o6_isAnyOf),
		OPCODE(o70_systemOps),
		OPCODE(o6_isActorInBox),
		/* B0 */
		OPCODE(o6_delay),
		OPCODE(o6_delaySeconds),
		OPCODE(o6_delayMinutes),
		OPCODE(o6_stopSentence),
		/* B4 */
		OPCODE(o6_printLine),
		OPCODE(o6_printText),
		OPCODE(o6_printDebug),
		OPCODE(o6_printSystem),
		/* B8 */
		OPCODE(o6_printActor),
		OPCODE(o6_printEgo),
		OPCODE(o6_talkActor),
		OPCODE(o6_talkEgo),
		/* BC */
		OPCODE(o6_dimArray),
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_startObjectQuick),
		OPCODE(o6_startScriptQuick2),
		/* C0 */
		OPCODE(o6_dim2dimArray),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* C4 */
		OPCODE(o6_abs),
		OPCODE(o6_distObjectObject),
		OPCODE(o6_distObjectPt),
		OPCODE(o6_distPtPt),
		/* C8 */
		OPCODE(o60_kernelGetFunctions),
		OPCODE(o70_kernelSetFunctions),
		OPCODE(o6_delayFrames),
		OPCODE(o6_pickOneOf),
		/* CC */
		OPCODE(o6_pickOneOfDefault),
		OPCODE(o6_stampObject),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* D0 */
		OPCODE(o6_getDateTime),
		OPCODE(o6_stopTalking),
		OPCODE(o6_getAnimateVariable),
		OPCODE(o6_invalid),
		/* D4 */
		OPCODE(o6_shuffle),
		OPCODE(o6_jumpToScript),
		OPCODE(o6_band),
		OPCODE(o6_bor),
		/* D8 */
		OPCODE(o6_isRoomScriptRunning),
		OPCODE(o60_closeFile),
		OPCODE(o60_openFile),
		OPCODE(o60_readFile),
		/* DC */
		OPCODE(o60_writeFile),
		OPCODE(o6_findAllObjects),
		OPCODE(o60_deleteFile),
		OPCODE(o60_rename),
		/* E0 */
		OPCODE(o60_soundOps),
		OPCODE(o6_getPixel),
		OPCODE(o60_localizeArrayToScript),
		OPCODE(o6_pickVarRandom),
		/* E4 */
		OPCODE(o6_setBoxSet),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* E8 */
		OPCODE(o6_invalid),
		OPCODE(o70_seekFilePos),
		OPCODE(o60_redimArray),
		OPCODE(o60_readFilePos),
		/* EC */
		OPCODE(o70_copyString),
		OPCODE(o70_getStringWidth),
		OPCODE(o70_getStringLen),
		OPCODE(o70_appendString),
		/* F0 */
		OPCODE(o70_concatString),
		OPCODE(o70_compareString),
		OPCODE(o70_isResourceLoaded),
		OPCODE(o70_readINI),
		/* F4 */
		OPCODE(o70_writeINI),
		OPCODE(o70_getStringLenForWidth),
		OPCODE(o70_getCharIndexInString),
		OPCODE(o6_invalid),
		/* F8 */
		OPCODE(o6_invalid),
		OPCODE(o70_setFilePath),
		OPCODE(o70_setWindowCaption),
		OPCODE(o70_polygonOps),
		/* FC */
		OPCODE(o70_polygonHit),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
	};

	_opcodesv70he = opcodes;
}

void ScummEngine_v70he::executeOpcode(byte i) {
	OpcodeProcv70he op = _opcodesv70he[i].proc;
	(this->*op) ();
}

const char *ScummEngine_v70he::getOpcodeDesc(byte i) {
	return _opcodesv70he[i].desc;
}

int ScummEngine_v70he::getStringCharWidth(byte chr) {
	int charset = _string[0]._default.charset;

	byte *ptr = getResourceAddress(rtCharset, charset);
	assert(ptr);
	ptr += 29;

	int spacing = 0;

	int offs = READ_LE_UINT32(ptr + chr * 4 + 4);
	if (offs) {
		spacing = ptr[offs] + (signed char)ptr[offs + 2];
	}

	return spacing;
}

int ScummEngine_v70he::setupStringArray(int size) {
	writeVar(0, 0);
	defineArray(0, kStringArray, 0, size + 1);
	writeArray(0, 0, 0, 0);
	return readVar(0);
}

void ScummEngine_v70he::appendSubstring(int dst, int src, int srcOffs, int len) {
	int dstOffs, value;
	int i = 0;

	if (len == -1) {
		len = resStrLen(getStringAddress(src));
		srcOffs = 0;
	}

	dstOffs = resStrLen(getStringAddress(dst));

	len -= srcOffs;
	len++;

	while (i < len) {
		writeVar(0, src);
		value = readArray(0, 0, srcOffs + i);
		writeVar(0, dst);
		writeArray(0, 0, dstOffs + i, value);
		i++;
	}

	writeArray(0, 0, dstOffs + i, 0);
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
		_sound->setSoundVar(_heSndSoundId, var, value);
		debug(0,"o70_startSound: case 23 (snd %d, var %d, value %d)", _heSndSoundId, var, value);
		break;
	case 25:
		value = pop();
		_heSndSoundId = pop();
		debug(0, "o70_startSound: case 25 (ID %d, Offset 0, Channel 0, Flags 8)", _heSndSoundId);
		_sound->addSoundToQueue(_heSndSoundId, 0, 0, 8);
	case 56:
		_heSndFlags |= 16;
		break;
	case 164:
		_heSndFlags |= 2;
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
		debug(1, "o70_startSound (ID %d, Offset %d, Channel %d, Flags %d)", _heSndSoundId, _heSndOffset, _heSndChannel, _heSndFlags);
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
	if (_heversion <= 70) {
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
	int objidx, resid, subOp;

	subOp = fetchScriptByte();

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
		res.nukeResource(rtScript, resid);
		break;
	case 105:		// SO_NUKE_SOUND
		resid = pop();
		res.nukeResource(rtSound, resid);
		break;
	case 106:		// SO_NUKE_COSTUME
		resid = pop();
		res.nukeResource(rtCostume, resid);
		break;
	case 107:		// SO_NUKE_ROOM
		resid = pop();
		res.nukeResource(rtRoom, resid);
		res.nukeResource(rtRoomImage, resid);
		break;
	case 108:		// SO_LOCK_SCRIPT
		resid = pop();
		if (resid >= _numGlobalScripts)
			break;
		res.lock(rtScript, resid);
		break;
	case 109:		// SO_LOCK_SOUND
		resid = pop();
		res.lock(rtSound, resid);
		break;
	case 110:		// SO_LOCK_COSTUME
		resid = pop();
		res.lock(rtCostume, resid);
		break;
	case 111:		// SO_LOCK_ROOM
		resid = pop();
		if (_heversion <= 71 && resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		res.lock(rtRoom, resid);
		res.lock(rtRoomImage, resid);
		break;
	case 112:		// SO_UNLOCK_SCRIPT
		resid = pop();
		if (resid >= _numGlobalScripts)
			break;
		res.unlock(rtScript, resid);
		break;
	case 113:		// SO_UNLOCK_SOUND
		resid = pop();
		res.unlock(rtSound, resid);
		break;
	case 114:		// SO_UNLOCK_COSTUME
		resid = pop();
		res.unlock(rtCostume, resid);
		break;
	case 115:		// SO_UNLOCK_ROOM
		resid = pop();
		if (_heversion <= 71 && resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		res.unlock(rtRoom, resid);
		res.unlock(rtRoomImage, resid);
		break;
	case 116:
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
		res.unlock(rtImage, resid);
		break;
	case 192:
		resid = pop();
		res.setUnModified(rtImage, resid);
		res.nukeResource(rtImage, resid);
		break;
	case 201:
		resid = pop();
		ensureResourceLoaded(rtImage, resid);
		break;
	case 202:
		resid = pop();
		res.lock(rtImage, resid);
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
		res.lock(rtFlObject, _objs[objidx].fl_object_index);
		break;
	case 235:
		resid = pop();
		objidx = getObjectIndex(resid);
		if (objidx == -1)
			break;
		res.unlock(rtFlObject, _objs[objidx].fl_object_index);
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
		shutDown();
		break;
	case 244:
		shutDown();
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

void ScummEngine_v70he::o70_seekFilePos() {
	int mode, offset, slot;
	mode = pop();
	offset = pop();
	slot = pop();

	if (slot == -1)
		return;

	switch (mode) {
	case 1:
		_hFileTable[slot].seek(offset, SEEK_SET);
		break;
	case 2:
		_hFileTable[slot].seek(offset, SEEK_CUR);
		break;
	case 3:
		_hFileTable[slot].seek(offset, SEEK_END);
		break;
	default:
		error("o70_seekFilePos: default case 0x%x", mode);
	}
}

void ScummEngine_v70he::o70_copyString() {
	int dst, size;
	int src = pop();

	size = resStrLen(getStringAddress(src)) + 1;
	dst = setupStringArray(size);

	appendSubstring(dst, src, -1, -1);

	push(dst);
}

void ScummEngine_v70he::o70_getStringWidth() {
	int array, pos, len;
	int chr, width = 0;

	len = pop();
	pos = pop();
	array = pop();

	if (len == -1) {
		pos = 0;
		len = resStrLen(getStringAddress(array));
	}

	writeVar(0, array);
	while (pos <= len) {
		chr = readArray(0, 0, pos);
		if (chr == 0)
			break;
		width += getStringCharWidth(chr);
		pos++;
	}

	push(width);
}

void ScummEngine_v70he::o70_kernelSetFunctions() {
	int args[29];
	int num;
	Actor *a;

	num = getStackList(args, ARRAYSIZE(args));

	debug(1, "o70_kernelSetFunctions: case %d (param count %d)", args[0], num);
	switch (args[0]) {
	case 1:
		// Used to restore images when decorating cake in
		// Fatty Bear's Birthday Surprise
		virtScreenLoad(args[1], args[2], args[3], args[4], args[5]);
		break;
	case 20: // HE72+
		a = derefActor(args[1], "o70_kernelSetFunctions: 20");
		((ScummEngine_v71he *)this)->queueAuxBlock(a);
		break;
	case 21:
		_skipDrawObject = 1;
		break;
	case 22:
		_skipDrawObject = 0;
		break;
	case 23:
		_charset->clearCharsetMask();
		_fullRedraw = true;
		break;
	case 24:
		_skipProcessActors = 1;
		redrawAllActors();
		break;
	case 25:
		_skipProcessActors = 0;
		redrawAllActors();
		break;
	case 26:
		a = derefActor(args[1], "o70_kernelSetFunctions: 26");
		a->_auxBlock.r.left = 0;
		a->_auxBlock.r.top = 0;
		a->_auxBlock.r.right = -1;
		a->_auxBlock.r.bottom = -2;
		break;
	case 30:
		a = derefActor(args[1], "o70_kernelSetFunctions: 30");
		a->_clipOverride.bottom = args[2];
		break;
	case 42:
		_wiz->_rectOverrideEnabled = true;
		_wiz->_rectOverride.left = args[1];
		_wiz->_rectOverride.top = args[2];
		_wiz->_rectOverride.right = args[3];
		_wiz->_rectOverride.bottom = args[4];
		break;
	case 43:
		_wiz->_rectOverrideEnabled = false;
		break;
	default:
		error("o70_kernelSetFunctions: default case %d (param count %d)", args[0], num);
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

void ScummEngine_v70he::o70_appendString() {
	int dst, size;

	int len = pop();
	int srcOffs = pop();
	int src = pop();

	size = len - srcOffs + 2;
	dst = setupStringArray(size);

	appendSubstring(dst, src, srcOffs, len);

	push(dst);
}

void ScummEngine_v70he::o70_concatString() {
	int dst, size;

	int src2 = pop();
	int src1 = pop();

	size = resStrLen(getStringAddress(src1));
	size += resStrLen(getStringAddress(src2)) + 1;
	dst = setupStringArray(size);

	appendSubstring(dst, src1, 0, -1);
	appendSubstring(dst, src2, 0, -1);

	push(dst);
}

void ScummEngine_v70he::o70_compareString() {
	int result;

	int array1 = pop();
	int array2 = pop();

	byte *string1 = getStringAddress(array1);
	if (!string1)
		error("o70_compareString: Reference to zeroed array pointer (%d)", array1);

	byte *string2 = getStringAddress(array2);
	if (!string2)
		error("o70_compareString: Reference to zeroed array pointer (%d)", array2);

	while (*string1 == *string2) {
		if (*string2 == 0) {
			push(0);
			return;
		}

		string1++;
		string2++;
	}

	result = (*string1 > *string2) ? -1 : 1;
	push(result);
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
	ArrayHeader *ah;
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
		ah = defineArray(0, kStringArray, 0, len);
		memcpy(ah->data, entry, len);

		push(readVar(0));
		break;
	default:
		error("o70_readINI: default type %d", type);
	}
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
		ConfMan.set((char *)option, value);
		break;
	case 2: // string
		convertMessageToString(_scriptPointer, string, sizeof(string));
		len = resStrLen(_scriptPointer);
		_scriptPointer += len + 1;
		ConfMan.set((char *)option, (char *)string);
		break;
	default:
		error("o70_writeINI: default type %d", type);
	}
}

void ScummEngine_v70he::o70_getStringLenForWidth() {
	int chr, max;
	int array, len, pos, width = 0;

	max = pop();
	pos = pop();
	array = pop();

	len = resStrLen(getStringAddress(array));

	writeVar(0, array);
	while (pos <= len) {
		chr = readArray(0, 0, pos);
		width += getStringCharWidth(chr);
		if (width >= max) {
			push(pos);
			return;
		}
		pos++;
	}

	push(len);
}

void ScummEngine_v70he::o70_getCharIndexInString() {
	int array, end, len, pos, value;

	value = pop();
	end = pop();
	pos = pop();
	array = pop();

	if (end >= 0) {
		len = resStrLen(getStringAddress(array));
		if (len < end)
			end = len;
	} else {
		end = 0;
	}

	if (pos < 0)
		pos = 0;

	writeVar(0, array);
	if (end > pos) {
		while (end >= pos) {
			if (readArray(0, 0, pos) == value) {
				push(pos);
				return;
			}
			pos++;
		}
	} else {
		while (end <= pos) {
			if (readArray(0, 0, pos) == value) {
				push(pos);
				return;
			}
			pos--;
		}
	}

	push(-1);
}

void ScummEngine_v70he::o70_setFilePath() {
	// File related
	int len;
	byte filename[100];

	convertMessageToString(_scriptPointer, filename, sizeof(filename));

	len = resStrLen(_scriptPointer);
	_scriptPointer += len + 1;

	debug(1,"stub o70_setFilePath(%s)", filename);
}

void ScummEngine_v70he::o70_setWindowCaption() {
	int num = fetchScriptByte();
	int len = resStrLen(_scriptPointer);
	debug(1,"stub o70_setWindowCaption(%d, \"%s\")", num, _scriptPointer);
	_scriptPointer += len + 1;
}

void ScummEngine_v70he::o70_polygonOps() {
	int vert1x, vert1y, vert2x, vert2y, vert3x, vert3y, vert4x, vert4y;
	int id, fromId, toId;
	bool flag;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 68: // HE 100
	case 69: // HE 100
	case 246:
	case 248:
		vert4y = pop();
		vert4x = pop();
		vert3y = pop();
		vert3x = pop();
		vert2y = pop();
		vert2x = pop();
		vert1y = pop();
		vert1x = pop();
		flag = (subOp == 69 || subOp == 248);
		id = pop();
		_wiz->polygonStore(id, flag, vert1x, vert1y, vert2x, vert2y, vert3x, vert3y, vert4x, vert4y);
		break;
	case 28: // HE 100
	case 247:
		toId = pop();
		fromId = pop();
		_wiz->polygonErase(fromId, toId);
		break;
	default:
		error("o70_polygonOps: default case %d", subOp);
	}
}

void ScummEngine_v70he::o70_polygonHit() {
	int y = pop();
	int x = pop();
	push(_wiz->polygonHit(0, x, y));
}

} // End of namespace Scumm
