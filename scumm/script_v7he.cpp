/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */


#include "stdafx.h"

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
#include "scumm/smush/smush_player.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

namespace Scumm {

// Compatibility notes:
//
// FREDDEMO (freddemo)
//     stringLen is completely different
//     unknownF4 is completely different

#define OPCODE(x)	{ &ScummEngine_v7he::x, #x }

void ScummEngine_v7he::setupOpcodes() {
	static const OpcodeEntryV7he opcodes[256] = {
		/* 00 */
		OPCODE(o6_pushByte),
		OPCODE(o6_pushWord),
		OPCODE(o6_pushByteVar),
		OPCODE(o6_pushWordVar),
		/* 04 */
		OPCODE(o7_getString),
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
		OPCODE(o7_objectX),
		OPCODE(o7_objectY),
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
		OPCODE(o7_startScript),
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
		OPCODE(o7_cursorCommand),
		/* 6C */
		OPCODE(o6_breakHere),
		OPCODE(o6_ifClassOfIs),
		OPCODE(o6_setClass),
		OPCODE(o6_getState),
		/* 70 */
		OPCODE(o6_setState),
		OPCODE(o6_setOwner),
		OPCODE(o6_getOwner),
		OPCODE(o6_jump),
		/* 74 */
		OPCODE(o7_startSound),
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
		OPCODE(o7_pickupObject),
		OPCODE(o6_loadRoomWithEgo),
		OPCODE(o6_invalid),
		OPCODE(o6_getRandomNumber),
		/* 88 */
		OPCODE(o6_getRandomNumberRange),
		OPCODE(o6_invalid),
		OPCODE(o6_getActorMoving),
		OPCODE(o6_isScriptRunning),
		/* 8C */
		OPCODE(o7_getActorRoom),
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
		OPCODE(o6_resourceRoutines),
		/* 9C */
		OPCODE(o6_roomOps),
		OPCODE(o6_actorOps),
		OPCODE(o6_verbOps),
		OPCODE(o6_getActorFromXY),
		/* A0 */
		OPCODE(o6_findObject),
		OPCODE(o6_pseudoRoom),
		OPCODE(o6_getActorElevation),
		OPCODE(o6_getVerbEntrypoint),
		/* A4 */
		OPCODE(o7_arrayOps),
		OPCODE(o6_saveRestoreVerbs),
		OPCODE(o6_drawBox),
		OPCODE(o6_pop),
		/* A8 */
		OPCODE(o6_getActorWidth),
		OPCODE(o6_wait),
		OPCODE(o6_getActorScaleX),
		OPCODE(o6_getActorAnimCounter1),
		/* AC */
		OPCODE(o6_invalid),
		OPCODE(o6_isAnyOf),
		OPCODE(o7_quitPauseRestart),
		OPCODE(o6_isActorInBox),
		/* B0 */
		OPCODE(o6_delay),
		OPCODE(o6_delaySeconds),
		OPCODE(o6_delayMinutes),
		OPCODE(o6_stopSentence),
		/* B4 */
		OPCODE(o6_printLine),
		OPCODE(o6_printCursor),
		OPCODE(o6_printDebug),
		OPCODE(o6_printSystem),
		/* B8 */
		OPCODE(o6_printActor),
		OPCODE(o6_printEgo),
		OPCODE(o6_talkActor),
		OPCODE(o6_talkEgo),
		/* BC */
		OPCODE(o7_dimArray),
		OPCODE(o6_dummy),
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
		OPCODE(o6_kernelGetFunctions),
		OPCODE(o6_kernelSetFunctions),
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
		OPCODE(o6_closeFile),
		OPCODE(o6_openFile),
		OPCODE(o6_readFile),
		/* DC */
		OPCODE(o6_writeFile),
		OPCODE(o6_findAllObjects),
		OPCODE(o6_deleteFile),
		OPCODE(o6_rename),
		/* E0 */
		OPCODE(o6_soundOps),
		OPCODE(o6_getPixel),
		OPCODE(o6_localizeArray),
		OPCODE(o6_pickVarRandom),
		/* E4 */
		OPCODE(o6_setBoxSet),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* E8 */
		OPCODE(o6_invalid),
		OPCODE(o6_seekFilePos),
		OPCODE(o6_redimArray),
		OPCODE(o6_readFilePos),
		/* EC */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o7_stringLen),
		OPCODE(o6_invalid),
		/* F0 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o7_readINI),
		/* F4 */
		OPCODE(o7_unknownF4),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* F8 */
		OPCODE(o6_invalid),
		OPCODE(o7_unknownF9),
		OPCODE(o7_unknownFA),
		OPCODE(o7_unknownFB),
		/* FC */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
	};

	_opcodesV7he = opcodes;
}

void ScummEngine_v7he::executeOpcode(byte i) {
	OpcodeProcV7he op = _opcodesV7he[i].proc;
	(this->*op) ();
}

const char *ScummEngine_v7he::getOpcodeDesc(byte i) {
	return _opcodesV7he[i].desc;
}


void ScummEngine_v7he::o7_objectX() {
	if (_heversion <= 71) {
		o6_invalid();
	}

	int object = pop();
	int objnum = getObjectIndex(object);

	if (objnum == -1) {
		push(0);
		return;
	}

	push(_objs[objnum].x_pos);
}


void ScummEngine_v7he::o7_objectY() {
	if (_heversion <= 71) {
		o6_invalid();
	}

	int object = pop();
	int objnum = getObjectIndex(object);

	if (objnum == -1) {
		push(0);
		return;
	}

	push(_objs[objnum].y_pos);
}

void ScummEngine_v7he::o7_getString() {
	int len;
	
	len = resStrLen(_scriptPointer);
	warning("stub o7_getString(\"%s\")", _scriptPointer);
	_scriptPointer += len;
	fetchScriptWord();
	fetchScriptWord();
}

void ScummEngine_v7he::o7_unknownFA() {
	int len, a = fetchScriptByte();
	
	if (_heversion <= 71) {
		len = resStrLen(_scriptPointer);
		warning("stub o7_unknownFA(%d, \"%s\")", a, _scriptPointer);
		_scriptPointer += len + 1;
	} else {
		warning("stub o7_unknownFA(%d)", a);
	}
}

void ScummEngine_v7he::o7_stringLen() {
	int a, len;
	byte *addr;

	a = pop();

	addr = getStringAddress(a);
	if (!addr) {
		// FIXME: should be error here
		warning("o7_stringLen: Reference to zeroed array pointer (%d)", a);
		push(0);
		return;
	}

	if (_gameId == GID_FREDDEMO) {
		len = strlen((char *)getStringAddress(a));
	} else { // FREDDI, PUTTMOON
		len = stringLen(addr);
	}
	push(len);
}

byte ScummEngine_v7he::stringLen(byte *ptr) {
	byte len;
	byte c;
	if (!ptr) {
		//ptr = _someGlobalPtr;
		error("ScummEngine_v7he::stringLen(): zero ptr. Undimplemented behaviour");
		return 1;
	}

	len = 0;
	c = *ptr++;

	if (len == c)
		return 0;

	do {
		len++;
		if (c == 0xff) {
			ptr += 3;
			len += 3;
		}
		c = *ptr++;
	} while (c);

	return len;
}

void ScummEngine_v7he::o7_readINI() {
	int len;
	int type;
	int retval;

	// we pretend that we don't have .ini file
	len = resStrLen(_scriptPointer);
	_scriptPointer += len + 1;
	type = pop();

	switch (type) {
	case 1: // number
		push(0);
		break;
	case 2: // string
		defineArray(0, kStringArray, 0, 0);
		retval = readVar(0);
		writeArray(0, 0, 0, 0);
		push(retval); // var ID string
		break;
	default:
		warning("o7_readINI(..., %d): read-ini string not implemented", type);
	}
}

void ScummEngine_v7he::o7_unknownF4() {
	if (_gameId == GID_FREDDEMO) {
		byte b;
		int len;
		b = fetchScriptByte();

		switch (b) {
		case 6:
			pop();
			len = resStrLen(_scriptPointer);
			_scriptPointer += len + 1;
			break;
		case 7:
			len = resStrLen(_scriptPointer);
			_scriptPointer += len + 1;
			len = resStrLen(_scriptPointer);
			_scriptPointer += len + 1;
			break;
		}
	} else { // FREDDI, PUTTMOON
		int a, b;
		byte filename1[256], filename2[256];
		int len;

		
		b = pop();
		a = pop();

		switch (b) {
		case 1:
			addMessageToStack(_scriptPointer, filename1, sizeof(filename1));

			len = resStrLen(_scriptPointer);
			_scriptPointer += len + 1;
			debug(1, "o7_unknownF4(%d, %d, \"%s\")", a, b, filename1);
			break;
		case 2:
			addMessageToStack(_scriptPointer, filename1, sizeof(filename1));

			len = resStrLen(_scriptPointer);
			_scriptPointer += len + 1;

			addMessageToStack(_scriptPointer, filename2, sizeof(filename2));

			len = resStrLen(_scriptPointer);
			_scriptPointer += len + 1;
			debug(1, "o7_unknownF4(%d, %d, \"%s\", \"%s\")", a, b, filename1, filename2);
			break;
		}
	}
	warning("o7_unknownF4 stub");
}

void ScummEngine_v7he::o7_unknownF9() {
	// File related
	int len, r;
	byte filename[100];

	addMessageToStack(_scriptPointer, filename, sizeof(filename));

	len = resStrLen(_scriptPointer);
	_scriptPointer += len + 1;

	for (r = strlen((char*)filename); r != 0; r--) {
		if (filename[r - 1] == '\\')
			break;
	}

	warning("stub o7_unknownF9(\"%s\")", filename + r);
}

void ScummEngine_v7he::o7_unknownFB() {
	byte b;
	b = fetchScriptByte();

	switch (b) {
	case 246:
	case 248:
		pop();
		pop();
		pop();
		pop();
		pop();
		pop();
		pop();
		pop();
		pop();
		break;
	case 247:
		pop();
		pop();
		break;
	}
	warning("o7_unknownFB stub");
}

void ScummEngine_v7he::o7_quitPauseRestart() {
	byte subOp = fetchScriptByte();
	int par1;

	switch (subOp & 0xff) {
	case 158:		// SO_RESTART
		restart();
		break;
	case 160:
		// FIXME: check
		shutDown();
		break;
	case 250:
		par1 = pop();
		warning("stub: o7_quitPauseRestart subOpcode %d", subOp);
		break;
	case 253:
		par1 = pop();
		warning("stub: o7_quitPauseRestart subOpcode %d", subOp);
	case 244:		// SO_QUIT
		shutDown();
		break;
	case 251:
	case 252:
		warning("stub: o7_quitPauseRestart subOpcode %d", subOp);
		break;
	default:
		error("o7_quitPauseRestart invalid case %d", subOp);
	}
}

void ScummEngine_v7he::o7_pickupObject() {
	int obj, room;

	room = pop();
	obj = pop();
	if (room == 0)
		room = getObjectRoom(obj);

	addObjectToInventory(obj, room);
	putOwner(obj, VAR(VAR_EGO));
	putClass(obj, kObjectClassUntouchable, 1);
	putState(obj, 1);
	markObjectRectAsDirty(obj);
	clearDrawObjectQueue();
	runInventoryScript(obj);									/* Difference */
}


void ScummEngine_v7he::o7_getActorRoom() {
	int act = pop();

	if (act < _numActors) {
		Actor *a = derefActor(act, "o7_getActorRoom");
		push(a->room);
	} else
		push(getObjectRoom(act));
}

void ScummEngine_v7he::o7_dimArray() {
	if (_heversion <= 71) {
		ScummEngine_v6::o6_dimArray();
		return;
	}

	int data;
	int type = fetchScriptByte();

	switch (type) {
	case 5:		// SO_INT_ARRAY
		data = kIntArray;
		break;
	case 2:		// SO_BIT_ARRAY
		data = kBitArray;
		break;
	case 3:		// SO_NIBBLE_ARRAY
		data = kNibbleArray;
		break;
	case 4:		// SO_BYTE_ARRAY
		data = kByteArray;
		break;
	case 7:		// SO_STRING_ARRAY
		data = kStringArray;
		break;
	case 204:		// SO_UNDIM_ARRAY
		nukeArray(fetchScriptWord());
		return;
	default:
		error("o7_dimArray: default case %d", type);
	}

	defineArray(fetchScriptWord(), data, 0, pop());
}

void ScummEngine_v7he::o7_arrayOps() {
	byte subOp = fetchScriptByte();
	int array = fetchScriptWord();
	int b, c, d, len;
	ArrayHeader *ah;
	int list[128];

	switch (subOp) {
	case 7:			// SO_ASSIGN_STRING
		len = resStrLen(_scriptPointer);
		ah = defineArray(array, kStringArray, 0, len + 1);
		copyScriptString(ah->data);
		break;
	case 205:		// SO_ASSIGN_STRING
		b = pop();
		len = resStrLen(_scriptPointer);
		ah = defineArray(array, kStringArray, 0, len + 1);
		copyScriptString(ah->data + b);
		break;
	case 208:		// SO_ASSIGN_INT_LIST
		b = pop();
		c = pop();
		d = readVar(array);
		if (d == 0) {
			defineArray(array, kIntArray, 0, b + c);
		}
		while (c--) {
			writeArray(array, 0, b + c, pop());
		}
		break;
	case 212:		// SO_ASSIGN_2DIM_LIST
		b = pop();
		len = getStackList(list, ARRAYSIZE(list));
		d = readVar(array);
		if (d == 0)
			error("Must DIM a two dimensional array before assigning");
		c = pop();
		while (--len >= 0) {
			writeArray(array, c, b + len, list[len]);
		}
		break;
	default:
		error("o7_arrayOps: default case %d (array %d)", subOp, array);
	}
}

void ScummEngine_v7he::o7_startScript() {
	if (_heversion <= 71) {
		ScummEngine_v6::o6_startScript();
		return;
	}

	int args[16];
	int script, flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = fetchScriptByte();
	
	runScript(script, (flags == 199 || flags == 200), (flags == 195 || flags == 200), args);
}

void ScummEngine_v7he::o7_startSound() {
	byte op;
	op = fetchScriptByte();

	switch (op) {
	case 224:
		_heSndSoundFreq = pop();
		break;

	case 230:
		_heSndTimer = pop();
		break;

	case 231:
		_heSndOffset = pop();
		break;

	case 232:
		_heSndSoundId = pop();
		_heSndOffset = 0;
		_heSndSoundFreq = 11025;
		_heSndTimer = VAR(VAR_MUSIC_TIMER);
		break;

	case 245:
		_heSndLoop |= 1;
		break;

	case 255:
		// _sound->addSoundToQueue(_heSndSoundId, _heSndOffset, _heSndTimer, _heSndLoop);
		_sound->addSoundToQueue(_heSndSoundId, _heSndOffset);
		debug(2, "o7_startSound stub (%d, %d, %d, %d)", _heSndSoundId, _heSndOffset, _heSndTimer, _heSndLoop);
		_heSndLoop = 0;
		break;

	default:
		break;
	}
}


void ScummEngine_v7he::o7_cursorCommand() {
	int a, i;
	int args[16];
	int subOp = fetchScriptByte();

	switch (subOp) {
	case 0x90:		// SO_CURSOR_ON Turn cursor on
		_cursor.state = 1;
		verbMouseOver(0);
		break;
	case 0x91:		// SO_CURSOR_OFF Turn cursor off
		_cursor.state = 0;
		verbMouseOver(0);
		break;
	case 0x92:		// SO_USERPUT_ON
		_userPut = 1;
		break;
	case 0x93:		// SO_USERPUT_OFF
		_userPut = 0;
		break;
	case 0x94:		// SO_CURSOR_SOFT_ON Turn soft cursor on
		_cursor.state++;
		if (_cursor.state > 1)
			error("Cursor state greater than 1 in script");
		verbMouseOver(0);
		break;
	case 0x95:		// SO_CURSOR_SOFT_OFF Turn soft cursor off
		_cursor.state--;
		verbMouseOver(0);
		break;
	case 0x96:		// SO_USERPUT_SOFT_ON
		_userPut++;
		break;
	case 0x97:		// SO_USERPUT_SOFT_OFF
		_userPut--;
		break;
	case 0x99: 		// SO_CURSOR_IMAGE Set cursor image
		_Win32ResExtractor->setCursor(pop()); 				/* Difference */
		break;
	case 0x9A:		// SO_CURSOR_HOTSPOT Set cursor hotspot
		a = pop();
		setCursorHotspot(pop(), a);
		break;
	case 0x9C:		// SO_CHARSET_SET
		initCharset(pop());
		break;
	case 0x9D:		// SO_CHARSET_COLOR
		getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < 16; i++)
			_charsetColorMap[i] = _charsetData[_string[1]._default.charset][i] = (unsigned char)args[i];
		break;
	case 0xD6:		// SO_CURSOR_TRANSPARENT Set cursor transparent color
		setCursorTransparency(pop());
		break;
	default:
		error("o6_cursorCommand: default case %x", subOp);
	}

	VAR(VAR_CURSORSTATE) = _cursor.state;
	VAR(VAR_USERPUT) = _userPut;
}


} // End of namespace Scumm
