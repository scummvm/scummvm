/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
#include <time.h>

#include "actor.h"
#include "charset.h"
#include "imuse.h"
#include "intern.h"
#include "scumm.h"
#include "sound.h"
#include "verbs.h"
#include "smush/smush_player.h"
#include "sound/mididrv.h"
#include "sound/mixer.h"

#include "dialogs.h"		// FIXME: This is just for the FT-INSANE warning. 
				// Remove when INSANE is implemented

#define OPCODE(x)	{ &Scumm_v6::x, #x }

void Scumm_v6::setupOpcodes() {
	static const OpcodeEntryV6 opcodes[256] = {
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
		OPCODE(o6_jumpTrue),
		OPCODE(o6_jumpFalse),
		OPCODE(o6_startScriptEx),
		OPCODE(o6_startScript),
		/* 60 */
		OPCODE(o6_startObjectEx),
		OPCODE(o6_setObjectState),
		OPCODE(o6_setObjectXY),
		OPCODE(o6_drawBlastObject),
		/* 64 */
		OPCODE(o6_setBlastObjectWindow),
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
		OPCODE(o6_setState),
		OPCODE(o6_setOwner),
		OPCODE(o6_getOwner),
		OPCODE(o6_jump),
		/* 74 */
		OPCODE(o6_startSound),
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
		OPCODE(o6_putActorInRoom),
		/* 80 */
		OPCODE(o6_putActorAtObject),
		OPCODE(o6_faceActor),
		OPCODE(o6_animateActor),
		OPCODE(o6_doSentence),
		/* 84 */
		OPCODE(o6_pickupObject),
		OPCODE(o6_loadRoomWithEgo),
		OPCODE(o6_invalid),
		OPCODE(o6_getRandomNumber),
		/* 88 */
		OPCODE(o6_getRandomNumberRange),
		OPCODE(o6_invalid),
		OPCODE(o6_getActorMoving),
		OPCODE(o6_isScriptRunning),
		/* 8C */
		OPCODE(o6_getActorRoom),
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
		OPCODE(o6_createBoxMatrix),
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
		OPCODE(o6_arrayOps),
		OPCODE(o6_saveRestoreVerbs),
		OPCODE(o6_drawBox),
		OPCODE(o6_pop),
		/* A8 */
		OPCODE(o6_getActorWidth),
		OPCODE(o6_wait),
		OPCODE(o6_getActorScaleX),
		OPCODE(o6_getActorAnimCounter1),
		/* AC */
		OPCODE(o6_soundKludge),
		OPCODE(o6_isAnyOf),
		OPCODE(o6_quitPauseRestart),
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
		OPCODE(o6_dim),
		OPCODE(o6_dummy),
		OPCODE(o6_startObjectQuick),
		OPCODE(o6_startScriptQuick),
		/* C0 */
		OPCODE(o6_dim2),
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
		OPCODE(o6_invalid),
		OPCODE(o6_findAllObjects),
		OPCODE(o6_deleteFile),
		OPCODE(o6_invalid),
		/* E0 */
		OPCODE(o6_unknownE0),
		OPCODE(o6_unknownE1),
		OPCODE(o6_localizeArray),
		OPCODE(o6_pickVarRandom),
		/* E4 */
		OPCODE(o6_unknownE4),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* E8 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_unknownEA),
		OPCODE(o6_invalid),
		/* EC */
		OPCODE(o6_getActorLayer),
		OPCODE(o6_getObjectNewDir),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* F0 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_readINI),
		/* F4 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* F8 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_unknownFA),
		OPCODE(o6_invalid),
		/* FC */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
	};

	_opcodesV6 = opcodes;
}

void Scumm_v6::executeOpcode(byte i) {
	OpcodeProcV6 op = _opcodesV6[i].proc;
	(this->*op) ();
}

const char *Scumm_v6::getOpcodeDesc(byte i) {
	return _opcodesV6[i].desc;
}

int Scumm_v6::popRoomAndObj(int *room) {
	int obj;

	if (_version >= 7) {
		obj = pop();
		*room = getObjectRoom(obj);
	} else {
		*room = pop();
		obj = pop();
	}

	return obj;
}


int Scumm::readArray(int array, int idx, int base) {
	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(array));

	if (ah == NULL || ah->data == NULL) {
		error("readArray: invalid array %d (%d)", array, readVar(array));
	}

	base += idx * ah->dim1_size;

	// FIXME: comment this for the time being as it was causing ft to crash
	// in the minefeild
	// FIX THE FIXME: fixing an assert by commenting out is bad. It's evil.
	// It's wrong. Find the proper cause, or at least, silently return
	// from the function, but don't just go on overwriting memory!
	assert(base >= 0 && base < ah->dim1_size * ah->dim2_size);

	if (ah->type == 4) {
		return ah->data[base];
	} else if (_version == 8) {
		return (int32)READ_LE_UINT32(ah->data + base * 4);
	} else {
		return (int16)READ_LE_UINT16(ah->data + base * 2);
	}
}

void Scumm::writeArray(int array, int idx, int base, int value) {
	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(array));
	if (!ah)
		return;
	base += idx * ah->dim1_size;

	assert(base >= 0 && base < ah->dim1_size * ah->dim2_size);

	if (ah->type == 4) {
		ah->data[base] = value;
	} else if (_version == 8) {
#if defined(SCUMM_NEED_ALIGNMENT)
		uint32 tmp = TO_LE_32(value);
		memcpy(&ah->data[base*4], &tmp, 4);
#else
		((uint32 *)ah->data)[base] = TO_LE_32(value);
#endif
	} else {
#if defined(SCUMM_NEED_ALIGNMENT)
		uint16 tmp = TO_LE_16(value);
		memcpy(&ah->data[base*2], &tmp, 2);
#else
		((uint16 *)ah->data)[base] = TO_LE_16(value);
#endif
	}
}

int Scumm_v6::getStackList(int *args, uint maxnum) {
	uint num, i;

	for (i = 0; i < maxnum; i++)
		args[i] = 0;

	num = pop();

	if (num > maxnum)
		error("Too many items %d in stack list, max %d", num, maxnum);

	i = num;
	while (((int)--i) >= 0) {
		args[i] = pop();
	}

	return num;
}

void Scumm_v6::o6_pushByte() {
	push(fetchScriptByte());
}

void Scumm_v6::o6_pushWord() {
	push(fetchScriptWordSigned());
}

void Scumm_v6::o6_pushByteVar() {
	push(readVar(fetchScriptByte()));
}

void Scumm_v6::o6_pushWordVar() {
	push(readVar(fetchScriptWord()));
}

void Scumm_v6::o6_invalid() {
	error("Invalid opcode '%x' at %x", _opcode, _scriptPointer - _scriptOrgPointer);
}

void Scumm_v6::o6_byteArrayRead() {
	int base = pop();
	push(readArray(fetchScriptByte(), 0, base));
}

void Scumm_v6::o6_wordArrayRead() {
	int base = pop();
	push(readArray(fetchScriptWord(), 0, base));
}

void Scumm_v6::o6_byteArrayIndexedRead() {
	int base = pop();
	int idx = pop();
	push(readArray(fetchScriptByte(), idx, base));
}

void Scumm_v6::o6_wordArrayIndexedRead() {
	int base = pop();
	int idx = pop();
	push(readArray(fetchScriptWord(), idx, base));
}

void Scumm_v6::o6_dup() {
	int a = pop();
	push(a);
	push(a);
}

void Scumm_v6::o6_not() {
	push(pop() == 0);
}

void Scumm_v6::o6_eq() {
	push(pop() == pop());
}

void Scumm_v6::o6_neq() {
	push(pop() != pop());
}

void Scumm_v6::o6_gt() {
	int a = pop();
	push(pop() > a);
}

void Scumm_v6::o6_lt() {
	int a = pop();
	push(pop() < a);
}

void Scumm_v6::o6_le() {
	int a = pop();
	push(pop() <= a);
}

void Scumm_v6::o6_ge() {
	int a = pop();
	push(pop() >= a);
}

void Scumm_v6::o6_add() {
	int a = pop();
	push(pop() + a);
}

void Scumm_v6::o6_sub() {
	int a = pop();
	push(pop() - a);
}

void Scumm_v6::o6_mul() {
	int a = pop();
	push(pop() * a);
}

void Scumm_v6::o6_div() {
	int a = pop();
	if (a == 0)
		error("division by zero");
	push(pop() / a);
}

void Scumm_v6::o6_land() {
	int a = pop();
	push(pop() && a);
}

void Scumm_v6::o6_lor() {
	int a = pop();
	push(pop() || a);
}

void Scumm_v6::o6_bor() {
	int a = pop();
	push(pop() | a);
}

void Scumm_v6::o6_band() {
	int a = pop();
	push(pop() & a);
}

void Scumm_v6::o6_pop() {
	pop();
}

void Scumm_v6::o6_writeByteVar() {
	writeVar(fetchScriptByte(), pop());
}

void Scumm_v6::o6_writeWordVar() {
	writeVar(fetchScriptWord(), pop());
}

void Scumm_v6::o6_byteArrayWrite() {
	int a = pop();
	writeArray(fetchScriptByte(), 0, pop(), a);
}

void Scumm_v6::o6_wordArrayWrite() {
	int a = pop();
	writeArray(fetchScriptWord(), 0, pop(), a);
}

void Scumm_v6::o6_byteArrayIndexedWrite() {
	int val = pop();
	int base = pop();
	writeArray(fetchScriptByte(), pop(), base, val);
}

void Scumm_v6::o6_wordArrayIndexedWrite() {
	int val = pop();
	int base = pop();
	writeArray(fetchScriptWord(), pop(), base, val);
}

void Scumm_v6::o6_byteVarInc() {
	int var = fetchScriptByte();
	writeVar(var, readVar(var) + 1);
}

void Scumm_v6::o6_wordVarInc() {
	int var = fetchScriptWord();
	writeVar(var, readVar(var) + 1);
}

void Scumm_v6::o6_byteArrayInc() {
	int var = fetchScriptByte();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) + 1);
}

void Scumm_v6::o6_wordArrayInc() {
	int var = fetchScriptWord();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) + 1);
}

void Scumm_v6::o6_byteVarDec() {
	int var = fetchScriptByte();
	writeVar(var, readVar(var) - 1);
}

void Scumm_v6::o6_wordVarDec() {
	int var = fetchScriptWord();
	writeVar(var, readVar(var) - 1);
}

void Scumm_v6::o6_byteArrayDec() {
	int var = fetchScriptByte();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) - 1);
}

void Scumm_v6::o6_wordArrayDec() {
	int var = fetchScriptWord();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) - 1);
}

void Scumm_v6::o6_jumpTrue() {
	if (pop())
		o6_jump();
	else
		fetchScriptWord();
}

void Scumm_v6::o6_jumpFalse() {
	if (!pop())
		o6_jump();
	else
		fetchScriptWord();
}

void Scumm_v6::o6_jump() {
	_scriptPointer += fetchScriptWordSigned();
}

void Scumm_v6::o6_startScriptEx() {
	int args[16];
	int script, flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = pop();
	runScript(script, (flags & 1) != 0, (flags & 2) != 0, args);
}

void Scumm_v6::o6_jumpToScript() {
	int args[16];
	int script, flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = pop();
	stopObjectCode();
	runScript(script, (flags & 1) != 0, (flags & 2) != 0, args);
}

void Scumm_v6::o6_startScript() {
	int args[16];
	int script;
	getStackList(args, ARRAYSIZE(args));
	script = pop();
	runScript(script, 0, 0, args);
}

void Scumm_v6::o6_startScriptQuick() {
	int args[16];
	int script;
	getStackList(args, ARRAYSIZE(args));
	script = pop();
	runScript(script, 0, 1, args);
}

void Scumm_v6::o6_startObjectEx() {
	int args[16];
	int script, entryp;
	int flags;
	getStackList(args, ARRAYSIZE(args));
	entryp = pop();
	script = pop();
	flags = pop();
	runObjectScript(script, entryp, (flags & 1) != 0, (flags & 2) != 0, args);
}

void Scumm_v6::o6_startObjectQuick() {
	int args[16];
	int script, entryp;
	getStackList(args, ARRAYSIZE(args));
	entryp = pop();
	script = pop();
	runObjectScript(script, entryp, 0, 1, args);
}

void Scumm_v6::o6_setObjectState() {
	int a = pop();
	int b = pop();
	if (a == 0)
		a = 1;
//	debug(1, "setObjectState(%d,%d)", a, b);

	setObjectState(b, a, -1, -1);
}

void Scumm_v6::o6_setObjectXY() {
	int y = pop();
	int x = pop();
	int obj = pop();
//  debug(1, "setObjectXY(%d,%d,%d)", obj, x, y);
	setObjectState(obj, 1, x, y);
}

void Scumm_v6::o6_stopObjectCode() {
	stopObjectCode();
}

void Scumm_v6::o6_endCutscene() {
	endCutscene();
}

void Scumm_v6::o6_cutscene() {
	int args[16];
	getStackList(args, ARRAYSIZE(args));
	beginCutscene(args);
}

void Scumm_v6::o6_stopMusic() {
	_sound->stopAllSounds();
}

void Scumm_v6::o6_freezeUnfreeze() {
	int a = pop();
	if (a)
		freezeScripts(a);
	else
		unfreezeScripts();
}

void Scumm_v6::o6_cursorCommand() {
	int a, i;
	int args[16];
	int op = fetchScriptByte();

	switch (op) {
	case 0x90:
		_cursor.state = 1;
		verbMouseOver(0);
		break;
	case 0x91:
		_cursor.state = 0;
		verbMouseOver(0);
		break;
	case 0x92:
		_userPut = 1;
		break;
	case 0x93:
		_userPut = 0;
		break;
	case 0x94:
		_cursor.state++;
		if (_cursor.state > 1)
			error("Cursor state greater than 1 in script");
		verbMouseOver(0);
		break;
	case 0x95:
		_cursor.state--;
		verbMouseOver(0);
		break;
	case 0x96:
		_userPut++;
		break;
	case 0x97:
		_userPut--;
		break;
	case 0x99:{
			if (_features & GF_AFTER_HEV7) {
				warning("cursorCommand 0x99 PC_SetCursorToID(%d) stub", pop());
				break;
			}
			int room, obj = popRoomAndObj(&room);
			setCursorImg(obj, room, 1);
			break;
		}
	case 0x9A:
		a = pop();
		setCursorHotspot(pop(), a);
		break;
	case 0x9C:										/* init charset */
		initCharset(pop());
		break;
	case 0x9D:										/* set charset colors */
		getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < 16; i++)
			_charsetColorMap[i] = _charsetData[_string[1].t_charset][i] = (unsigned char)args[i];
		break;
	case 0xD6:
		makeCursorColorTransparent(pop());
		break;
	default:
		error("o6_cursorCommand: default case %x", op);
	}

	VAR(VAR_CURSORSTATE) = _cursor.state;
	VAR(VAR_USERPUT) = _userPut;
}

void Scumm_v6::o6_breakHere() {
	updateScriptPtr();
	_currentScript = 0xFF;
}

void Scumm_v6::o6_ifClassOfIs() {
	int args[16];
	int num, obj, cls;
	bool b;
	int cond = 1;

	num = getStackList(args, ARRAYSIZE(args));
	obj = pop();

	while (--num >= 0) {
		cls = args[num];
		b = getClass(obj, cls);
		if ((cls & 0x80 && !b) || (!(cls & 0x80) && b))
			cond = 0;
	}
	push(cond);
}

void Scumm_v6::o6_setClass() {
	int args[16];
	int num, obj, cls;

	num = getStackList(args, ARRAYSIZE(args));
	obj = pop();

	while (--num >= 0) {
		cls = args[num];
		if (cls == 0)
			_classData[num] = 0;
		else if (cls & 0x80)
			putClass(obj, cls, 1);
		else
			putClass(obj, cls, 0);
	}
}

void Scumm_v6::o6_getState() {
	push(getState(pop()));
}

void Scumm_v6::o6_setState() {
	int state = pop();
	int obj = pop();

	if ((_features & GF_HUMONGOUS) && (state & 0x8000)) {
		state = state & 0x7F00;
		putState(obj, state);
		return;
	}
//  debug(1, "setState(%d,%d)", obj, state);

	putState(obj, state);
	removeObjectFromRoom(obj);
	if (_BgNeedsRedraw)
		clearDrawObjectQueue();
}

void Scumm_v6::o6_setOwner() {
	int owner = pop();
	int obj = pop();

	setOwnerOf(obj, owner);
}

void Scumm_v6::o6_getOwner() {
	push(getOwner(pop()));
}

void Scumm_v6::o6_startSound() {
	if ((_features & GF_HUMONGOUS) && (_gameId != GID_PUTTDEMO))
		pop(); // offset which seems to always be zero
	_sound->addSoundToQueue(pop());
}

void Scumm_v6::o6_stopSound() {
	_sound->stopSound(pop());
}

void Scumm_v6::o6_startMusic() {
	_sound->addSoundToQueue(pop());
}

void Scumm_v6::o6_stopObjectScript() {
	stopObjectScript(pop());
}

void Scumm_v6::o6_panCameraTo() {
	if (_version >= 7) {
		int y = pop();
		int x = pop();
		panCameraTo(x, y);
	} else {
		panCameraTo(pop(), 0);
	}
}

void Scumm_v6::o6_actorFollowCamera() {
	if (_version >= 7)
		setCameraFollows(derefActor(pop(), "actorFollowCamera"));
	else
		actorFollowCamera(pop());
}

void Scumm_v6::o6_setCameraAt() {
	if (_version >= 7) {
		int x, y;

		camera._follows = 0;
		VAR(VAR_CAMERA_FOLLOWED_ACTOR) = 0;

		y = pop();
		x = pop();

		setCameraAt(x, y);
	} else {
		setCameraAtEx(pop());
	}
}

void Scumm_v6::o6_loadRoom() {
	int room = pop();
	startScene(room, 0, 0);
	_fullRedraw = 1;
}

void Scumm_v6::o6_stopScript() {
	int script = pop();
	if (script == 0)
		stopObjectCode();
	else
		stopScript(script);
}

void Scumm_v6::o6_walkActorToObj() {
	int act, obj, dist;
	Actor *a, *a2;
	int x, y;

	dist = pop();
	obj = pop();
	act = pop();
	a = derefActor(act, "o6_walkActorToObj");

	if (obj >= _numActors) {
		int wio = whereIsObject(obj);

		if (wio != WIO_FLOBJECT && wio != WIO_ROOM)
			return;

		int dir;
		getObjectXYPos(obj, x, y, dir);
		a->startWalkActor(x, y, dir);
	} else {
		a2 = derefActorSafe(obj, "o6_walkActorToObj(2)");
		if (_gameId == GID_SAMNMAX && a2 == 0) {
			// FIXME: This is a hack to work around bug #742676 SAM: Fish Farm.
			// Note quite sure why it happens, though, if it's normal or due to
			// a bug in the ScummVM code.
			warning("o6_walkActorToObj: invalid actor %d", obj);
			return;
		}
		if (!a->isInCurrentRoom() || !a2->isInCurrentRoom())
			return;
		if (dist == 0) {
			dist = a2->scalex * a2->width / 0xFF;
			dist += dist / 2;
		}
		x = a2->x;
		y = a2->y;
		if (x < a->x)
			x += dist;
		else
			x -= dist;
		a->startWalkActor(x, y, -1);
	}
}

void Scumm_v6::o6_walkActorTo() {
	int x, y;
	y = pop();
	x = pop();
	Actor *a = derefActor(pop(), "o6_walkActorTo");
	a->startWalkActor(x, y, -1);
}

void Scumm_v6::o6_putActorInRoom() {
	int room, x, y, act;
	Actor *a;

	room = pop();
	y = pop();
	x = pop();
	act = pop();
	a = derefActor(act, "o6_putActorInRoom");

	if (room == 0xFF || room == 0x7FFFFFFF) {
		room = a->room;
	} else {
		if (a->visible && _currentRoom != room && VAR(VAR_TALK_ACTOR) == a->number) {
			clearMsgQueue();
		}
		if (room != 0)
			a->room = room;
	}
	a->putActor(x, y, room);
}


void Scumm_v6::o6_putActorAtObject() {
	int room, obj, x, y;
	Actor *a;

	obj = popRoomAndObj(&room);

	a = derefActor(pop(), "o6_putActorAtObject");
	if (whereIsObject(obj) != WIO_NOT_FOUND) {
		getObjectXYPos(obj, x, y);
	} else {
		x = 160;
		y = 120;
	}
	if (room == 0xFF)
		room = a->room;
	a->putActor(x, y, room);
}

void Scumm_v6::o6_faceActor() {
	int obj = pop();
	Actor *a = derefActor(pop(), "o6_faceActor");
	a->faceToObject(obj);
}

void Scumm_v6::o6_animateActor() {
	int anim = pop();
	int act = pop();
	if (_gameId == GID_TENTACLE && _roomResource == 57 &&
		vm.slot[_currentScript].number == 19 && act == 593) {
		// FIXME: This very odd case (animateActor(593,250)) occurs in DOTT, in the
		// cutscene after George cuts down the "cherry tree" and the tree Laverne
		// is trapped in vanishes... see bug #743363.
		// Not sure if this means animateActor somehow also must work for objects
		// (593 is the time machine in room 57), or if this is simply a script bug.
		act = 6;
	}
	Actor *a = derefActor(act, "o6_animateActor");
	a->animateActor(anim);
}

void Scumm_v6::o6_doSentence() {
	int verb, objectA, objectB, dummy = 0;

	objectB = pop();
	if (_version < 8)
		dummy = pop();	// dummy pop (in Sam&Max, seems to be always 0 or 130)
	objectA = pop();
	verb = pop();

	doSentence(verb, objectA, objectB);
}

void Scumm_v6::o6_pickupObject() {
	int obj, room;
	int i;

	obj = popRoomAndObj(&room);
	if (room == 0)
		room = _roomResource;

	for (i = 1; i < _maxInventoryItems; i++) {
		if (_inventory[i] == (uint16)obj) {
			putOwner(obj, VAR(VAR_EGO));
			runInventoryScript(obj);
			return;
		}
	}

	addObjectToInventory(obj, room);
	putOwner(obj, VAR(VAR_EGO));
	putClass(obj, kObjectClassUntouchable, 1);
	putState(obj, 1);
	removeObjectFromRoom(obj);
	clearDrawObjectQueue();
	runInventoryScript(obj);									/* Difference */
}

void Scumm_v6::o6_loadRoomWithEgo() {
	Actor *a;
	int obj, room, x, y;

	y = pop();
	x = pop();

	obj = popRoomAndObj(&room);

	a = derefActor(VAR(VAR_EGO), "o6_loadRoomWithEgo");
	a->putActor(0, 0, room);
	_egoPositioned = false;

	if (VAR_WALKTO_OBJ != 0xFF)
		VAR(VAR_WALKTO_OBJ) = obj;
	startScene(a->room, a, obj);
	if (VAR_WALKTO_OBJ != 0xFF)
		VAR(VAR_WALKTO_OBJ) = 0;

	/* startScene maybe modifies VAR_EGO, i hope not */

	if (_version == 6) {
		setCameraAt(a->x, a->y);
		setCameraFollows(a);
	}

	_fullRedraw = 1;

	if (x != -1 && x != 0x7FFFFFFF) {
		a->startWalkActor(x, y, -1);
	}
}

void Scumm_v6::o6_getRandomNumber() {
	int rnd;
	rnd = _rnd.getRandomNumber(pop());
	if (VAR_V6_RANDOM_NR != 0xFF)
		VAR(VAR_V6_RANDOM_NR) = rnd;
	push(rnd);
}

void Scumm_v6::o6_getRandomNumberRange() {
	int max = pop();
	int min = pop();
	int rnd = _rnd.getRandomNumberRng(min, max);
	if (VAR_V6_RANDOM_NR != 0xFF)
		VAR(VAR_V6_RANDOM_NR) = rnd;
	push(rnd);
}

void Scumm_v6::o6_isScriptRunning() {
	push(isScriptRunning(pop()));
}

void Scumm_v6::o6_isRoomScriptRunning() {
	push(isRoomScriptRunning(pop()));
}

void Scumm_v6::o6_getActorMoving() {
	Actor *a = derefActor(pop(), "o6_getActorMoving");
	push(a->moving);
}

void Scumm_v6::o6_getActorRoom() {
	int act = pop();

	if (act == 0) {
		// This case occurs at the very least in COMI. That's because in COMI's script 28,
		// there is a check which looks as follows:
		//   if (((VAR_TALK_ACTOR != 0) && (VAR_HAVE_MSG == 1)) &&
		//        (getActorRoom(VAR_TALK_ACTOR) == VAR_ROOM))
		// Due to the way this is represented in bytecode, the engine cannot
		// short circuit. Hence, even thought this would be perfectly fine code
		// in C/C++, here it can (and does) lead to getActorRoom(0) being
		// invoked. We silently ignore this.
		push(0);
		return;
	}

	if (act == 255) {
		// This case also occurs in COMI...
		push(0);
		return;
	}

	Actor *a = derefActor(act, "o6_getActorRoom");
	push(a->room);
}

void Scumm_v6::o6_getActorWalkBox() {
	Actor *a = derefActor(pop(), "o6_getActorWalkBox");
	push(a->ignoreBoxes ? 0 : a->walkbox);
}

void Scumm_v6::o6_getActorCostume() {
	Actor *a = derefActor(pop(), "o6_getActorCostume");
	push(a->costume);
}

void Scumm_v6::o6_getActorElevation() {
	Actor *a = derefActor(pop(), "o6_getActorElevation");
	push(a->elevation);
}

void Scumm_v6::o6_getActorWidth() {
	Actor *a = derefActor(pop(), "o6_getActorWidth");
	push(a->width);
}

void Scumm_v6::o6_getActorScaleX() {
	Actor *a = derefActor(pop(), "o6_getActorScale");
	push(a->scalex);
}

void Scumm_v6::o6_getActorAnimCounter1() {
	Actor *a = derefActor(pop(), "o6_getActorAnimCounter");
	push(a->cost.animCounter);
}

void Scumm_v6::o6_getAnimateVariable() {
	int var = pop();
	Actor *a = derefActor(pop(), "o6_getAnimateVariable");
	push(a->getAnimVar(var));
}

void Scumm_v6::o6_isActorInBox() {
	int box = pop();
	Actor *a = derefActor(pop(), "o6_isActorInBox");
	push(checkXYInBoxBounds(box, a->x, a->y));
}

void Scumm_v6::o6_getActorLayer() {
	Actor *a = derefActor(pop(), "getActorLayer");
	push(a->layer);
}

void Scumm_v6::o6_getObjectX() {
	push(getObjX(pop()));
}

void Scumm_v6::o6_getObjectY() {
	push(getObjY(pop()));
}

void Scumm_v6::o6_getObjectOldDir() {
	push(getObjOldDir(pop()));
}

void Scumm_v6::o6_getObjectNewDir() {
	push(getObjNewDir(pop()));
}

void Scumm_v6::o6_findInventory() {
	int idx = pop();
	int owner = pop();
	push(findInventory(owner, idx));
}

void Scumm_v6::o6_getInventoryCount() {
	push(getInventoryCount(pop()));
}

void Scumm_v6::o6_getVerbFromXY() {
	int y = pop();
	int x = pop();
	int over = checkMouseOver(x, y);
	if (over)
		over = _verbs[over].verbid;
	push(over);
}

void Scumm_v6::o6_beginOverride() {
	beginOverride();
}

void Scumm_v6::o6_endOverride() {
	endOverride();
}

void Scumm_v6::o6_setObjectName() {
	int obj = pop();
	int i;

	if (obj < _numActors)
		error("Can't set actor %d name with new-name-of", obj);

	if (_version < 7 && !getOBCDFromObject(obj))
		error("Can't set name of object %d", obj);

	for (i = 0; i < _numNewNames; i++) {
		if (_newNames[i] == obj) {
			nukeResource(rtObjectName, i);
			_newNames[i] = 0;
			break;
		}
	}

	for (i = 0; i < _numNewNames; i++) {
		if (_newNames[i] == 0) {
			loadPtrToResource(rtObjectName, i, NULL);
			_newNames[i] = obj;
			runInventoryScript(0);
			return;
		}
	}

	error("New name of %d overflows name table (max = %d)", obj, 50);
}

void Scumm_v6::o6_isSoundRunning() {
	int snd = pop();

	if (snd)
		snd = _sound->isSoundRunning(snd);

	push(snd);
}

void Scumm_v6::o6_setBoxFlags() {
	int table[65];
	int num, value;

	value = pop();
	num = getStackList(table, ARRAYSIZE(table));

	while (--num >= 0) {
		setBoxFlags(table[num], value);
	}
}

void Scumm_v6::o6_createBoxMatrix() {
	createBoxMatrix();
}

void Scumm_v6::o6_resourceRoutines() {
	int resid, op;
	op = fetchScriptByte();

	switch (op) {
	case 100:										/* load script */
		resid = pop();
		if (_version >= 7)
			if (resid >= _numGlobalScripts)
				break;
		ensureResourceLoaded(rtScript, resid);
		break;
	case 101:										/* load sound */
		resid = pop();
		ensureResourceLoaded(rtSound, resid);
		break;
	case 102:										/* load costume */
		resid = pop();
		ensureResourceLoaded(rtCostume, resid);
		break;
	case 103:										/* load room */
		resid = pop();
		ensureResourceLoaded(rtRoom, resid);
		break;
	case 104:										/* nuke script */
		resid = pop();
		if (_version >= 7)
			if (resid >= _numGlobalScripts)
				break;
		setResourceCounter(rtScript, resid, 0x7F);
		debug(5, "nuke script %d", resid);
		break;
	case 105:										/* nuke sound */
		resid = pop();
		setResourceCounter(rtSound, resid, 0x7F);
		break;
	case 106:										/* nuke costume */
		resid = pop();
		setResourceCounter(rtCostume, resid, 0x7F);
		break;
	case 107:										/* nuke room */
		resid = pop();
		setResourceCounter(rtRoom, resid, 0x7F);
		break;
	case 108:										/* lock script */
		resid = pop();
		if (resid >= _numGlobalScripts)
			break;
		lock(rtScript, resid);
		break;
	case 109:										/* lock sound */
		resid = pop();
		lock(rtSound, resid);
		break;
	case 110:										/* lock costume */
		resid = pop();
		lock(rtCostume, resid);
		break;
	case 111:										/* lock room */
		resid = pop();
		if (resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		lock(rtRoom, resid);
		break;
	case 112:										/* unlock script */
		resid = pop();
		if (resid >= _numGlobalScripts)
			break;
		unlock(rtScript, resid);
		break;
	case 113:										/* unlock sound */
		resid = pop();
		unlock(rtSound, resid);
		break;
	case 114:										/* unlock costume */
		resid = pop();
		unlock(rtCostume, resid);
		break;
	case 115:										/* unlock room */
		resid = pop();
		if (resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		unlock(rtRoom, resid);
		break;
	case 116:										/* clear heap */
		/* this is actually a scumm message */
		error("clear heap not working yet");
		break;
	case 117:										/* load charset */
		resid = pop();
		loadCharset(resid);
		break;
	case 118:										/* nuke charset */
		resid = pop();
		nukeCharset(resid);
		break;
	case 119:{										/* load fl object */
			int room, obj = popRoomAndObj(&room);
			loadFlObject(obj, room);
			break;
		}
	case 120:{										/* queue ? for load */
			 warning("stub queueload resource %d", pop());
			 // QL_QueGlobForLoad(2, pop(), 1);
			 break;
		
		}
	default:
		error("o6_resourceRoutines: default case %d", op);
	}
}


void Scumm_v6::o6_roomOps() {
	int a, b, c, d, e;
	byte op;

	op = fetchScriptByte();

	switch (op) {
	case 172:										/* room scroll */
		b = pop();
		a = pop();
		if (a < (_screenWidth / 2))
			a = (_screenWidth / 2);
		if (b < (_screenWidth / 2))
			b = (_screenWidth / 2);
		if (a > _roomWidth - (_screenWidth / 2))
			a = _roomWidth - (_screenWidth / 2);
		if (b > _roomWidth - (_screenWidth / 2))
			b = _roomWidth - (_screenWidth / 2);
		VAR(VAR_CAMERA_MIN_X) = a;
		VAR(VAR_CAMERA_MAX_X) = b;
		break;

	case 174:										/* set screen */
		b = pop();
		a = pop();
		initScreens(0, a, _screenWidth, b);
		break;

	case 175:										/* set palette color */
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		setPalColor(d, a, b, c);
		break;

	case 176:										/* shake on */
		setShake(1);
		break;

	case 177:										/* shake off */
		setShake(0);
		break;

	case 179:
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, a, a, b, c);
		break;

	case 180:
		_saveLoadCompatible = true;
		_saveLoadSlot = pop();
		_saveLoadFlag = pop();
		warning("o6_roomops:180: partially unimplemented");
		break;

	case 181:
		a = pop();
		if (a) {
			_switchRoomEffect = (byte)(a&0xFF);
			_switchRoomEffect2 = (byte)(a >> 8);
		} else {
			fadeIn(_newEffect);
		}
		break;

	case 182:
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, b, c, d, e);
		break;

	case 183:
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		setupShadowPalette(a, b, c, d, e);
		break;

	case 184:
		error("save string not implemented");
		break;

	case 185:
		error("load string not implemented");
		break;

	case 186:										/* palmanip? */
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		palManipulateInit(a, b, c, d);
		break;

	case 187:										/* color cycle delay */
		b = pop();
		a = pop();
		checkRange(16, 1, a, "o6_roomOps: 187: color cycle out of range (%d)");
		_colorCycle[a - 1].delay = (b != 0) ? 0x4000 / (b * 0x4C) : 0;
		break;

	case 213:										/* set palette */
		a = pop();

		// This opcode is used when turning off noir mode in Sam & Max,
		// but since our implementation of this feature doesn't change
		// the original palette there's no need to reload it. Doing it
		// this way, we avoid some graphics glitches that the original
		// interpreter had.

		if (_gameId == GID_SAMNMAX && vm.slot[_currentScript].number == 64)
			setDirtyColors(0, 255);
		else
			setPalette(a);
		break;

	default:
		error("o6_roomOps: default case %d", op);
	}
}

void Scumm_v6::o6_actorOps() {
	Actor *a;
	int i, j, k;
	int args[8];
	byte b;

	b = fetchScriptByte();
	if (b == 197) {
		_curActor = pop();
		return;
	}

	a = derefActorSafe(_curActor, "o6_actorOps");
	if (!a)
		return;

	switch (b) {
	case 76:											/* actor-costume */
		a->setActorCostume(pop());
		break;
	case 77:											/* actor-speed */
		j = pop();
		i = pop();
		a->setActorWalkSpeed(i, j);
		break;
	case 78:											/* actor-sound */
		k = getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < k; i++)
			a->sound[i] = args[i];
		break;
	case 79:											/* actor-walkframe */
		a->walkFrame = pop();
		break;
	case 80:											/* actor-talkframe */
		a->talkStopFrame = pop();
		a->talkStartFrame = pop();
		break;
	case 81:											/* actor-standframe */
		a->standFrame = pop();
		break;
	case 82:
		// FIXME: what does this opcode do ???
		pop();
		pop();
		pop();
		break;
	case 83:
		a->initActor(0);
		break;
	case 84:											/* actor-elevation */
		i = pop();
		if (i != a->elevation) {
			a->elevation = i;
			a->needRedraw = true;
		}
		break;
	case 85:											/* actor-defaultanims */
		a->initFrame = 1;
		a->walkFrame = 2;
		a->standFrame = 3;
		a->talkStartFrame = 4;
		a->talkStopFrame = 5;
		break;
	case 86:											/* actor-palette */
		j = pop();
		i = pop();
		checkRange(255, 0, i, "Illegal palette slot %d");
		a->palette[i] = j;
		a->needRedraw = true;
		break;
	case 87:											/* actor-talkcolor */
		a->talkColor = pop();
		break;
	case 88:											/* actor-name */
		loadPtrToResource(rtActorName, a->number, NULL);
		break;
	case 89:											/* actor-initframe */
		a->initFrame = pop();
		break;
	case 91:
		a->width = pop();
		break;
	case 92:
		a->scalex = a->scaley = pop();
		a->needRedraw = true;
		break;
	case 93:
		a->forceClip = 0;
		break;
	case 225:	// FIXME: Is subop 225 really identical to subop 94
	case 94:
		a->forceClip = pop();
		break;
	case 95:
		a->ignoreBoxes = 1;
		if (_version >= 7)
			a->forceClip = 100;
		else
			a->forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor(a->x, a->y, a->room);
		break;
	case 96:
		a->ignoreBoxes = 0;
		if (_version >= 7)
			a->forceClip = 100;
		else
			a->forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor(a->x, a->y, a->room);
		break;
	case 97:
		a->setAnimSpeed(pop());
		break;
	case 98:
		a->shadow_mode = pop();
		break;
	case 99:
		a->talkPosY = pop();
		a->talkPosX = pop();
		break;
	case 198:										/* set anim variable */
		i = pop();									/* value */
		a->setAnimVar(pop(), i);
		break;
	case 215:
		warning("ignoreTurns not yet implemented");
		a->ignoreTurns = true;
		break;
	case 216:
		a->ignoreTurns = false;
		break;
	case 217:
		a->initActor(2);
		break;
	case 218:
		{
			// TODO: this opcode is used in the putt-putt fun pack, in 'checkers" mini game
			warning("o6_actorOps(): unimplemented opcode 218");

			int top_actor = a->top;
			int bottom_actor = a->bottom;
//			a->_zbuf = 1; ???
			a->needRedraw = 1;
			a->drawActorCostume();
//			a->_zbuf = 0; ???
			a->needRedraw = 0;
			a->drawActorCostume();

			if (a->top > top_actor) {
				a->bottom = top_actor;
			}
			if (a->bottom > bottom_actor) {
				a->bottom = bottom_actor;
			}
		}
		break;
	case 227:										/* actor_layer */
		a->layer = pop();
		break;
	case 228:										/* walk script */
		a->walkScript = pop();
		break;
	case 229:										/* stand */
		a->stopActorMoving();
		a->startAnimActor(a->standFrame);
		break;
	case 230:										/* set direction */
		a->moving &= ~MF_TURN;
		a->setDirection(pop());
		break;
	case 231:										/* turn to direction */
		a->turnToDirection(pop());
		break;
	case 233:										/* freeze actor */
		a->moving |= MF_FROZEN;
		break;
	case 234:										/* unfreeze actor */
		a->moving &= ~MF_FROZEN;
		break;
	case 235:										/* talkScript */
		a->talkScript = pop();
		break;
	default:
		error("o6_actorOps: default case %d", b);
	}
}

void Scumm_v6::o6_verbOps() {
	int slot, a, b;
	VerbSlot *vs;
	byte op;

	// Full Throttle implements conversation by creating new verbs, one
	// for each option, but it never tells when to actually draw them.

	if (_gameId == GID_FT)
		_verbRedraw = true;

	op = fetchScriptByte();
	if (op == 196) {
		_curVerb = pop();
		_curVerbSlot = getVerbSlot(_curVerb, 0);
		checkRange(_maxVerbs - 1, 0, _curVerbSlot, "Illegal new verb slot %d");
		return;
	}
	vs = &_verbs[_curVerbSlot];
	slot = _curVerbSlot;
	switch (op) {
	case 124:										/* load img */
		a = pop();
		if (_curVerbSlot) {
			setVerbObject(_roomResource, a, slot);
			vs->type = kImageVerbType;
		}
		break;
	case 125:
		loadPtrToResource(rtVerb, slot, NULL);
		vs->type = kTextVerbType;
		vs->imgindex = 0;
		break;
	case 126:
		vs->color = pop();
		break;
	case 127:
		vs->hicolor = pop();
		break;
	case 128:
		vs->y = pop();
		vs->x = pop();
		break;
	case 129:
		vs->curmode = 1;
		break;
	case 130:
		vs->curmode = 0;
		break;
	case 131:
		if (_features & GF_HUMONGOUS) {
			// delete verb supposedly...
			pop();
		}
		killVerb(slot);
		break;
	case 132:
		slot = getVerbSlot(_curVerb, 0);
		if (slot == 0) {
			for (slot = 1; slot < _maxVerbs; slot++) {
				if (_verbs[slot].verbid == 0)
					break;
			}
			if (slot == _maxVerbs)
				error("Too many verbs");
			_curVerbSlot = slot;
		}
		vs = &_verbs[slot];
		vs->verbid = _curVerb;
		vs->color = 2;
		vs->hicolor = 0;
		vs->dimcolor = 8;
		vs->type = kTextVerbType;
		vs->charset_nr = _string[0].t_charset;
		vs->curmode = 0;
		vs->saveid = 0;
		vs->key = 0;
		vs->center = 0;
		vs->imgindex = 0;
		break;
	case 133:
		vs->dimcolor = pop();
		break;
	case 134:
		vs->curmode = 2;
		break;
	case 135:
		vs->key = pop();
		break;
	case 136:
		vs->center = 1;
		break;
	case 137:
		a = pop();
		if (a == 0) {
			loadPtrToResource(rtVerb, slot, (const byte *)"");
		} else {
			loadPtrToResource(rtVerb, slot, getStringAddress(a));
		}
		vs->type = kTextVerbType;
		vs->imgindex = 0;
		break;
	case 139:
		b = pop();
		a = pop();
		if (slot && a != vs->imgindex) {
			setVerbObject(b, a, slot);
			vs->type = kImageVerbType;
			vs->imgindex = a;
		}
		break;
	case 140:
		vs->bkcolor = pop();
		break;
	case 255:
		drawVerb(slot, 0);
		verbMouseOver(0);
		break;
	default:
		error("o6_verbops: default case %d", op);
	}
}

void Scumm_v6::o6_getActorFromXY() {
	int y = pop();
	int x = pop();
	push(getActorFromPos(x, y));
}

void Scumm_v6::o6_findObject() {
	int y = pop();
	int x = pop();
	int r = findObject(x, y);
	push(r);
}

void Scumm_v6::o6_pseudoRoom() {
	int list[100];
	int num, a, value;

	num = getStackList(list, ARRAYSIZE(list));
	value = pop();

	while (--num >= 0) {
		a = list[num];
		if (a > 0x7F)
			_resourceMapper[a & 0x7F] = value;
	}
}

void Scumm_v6::o6_getVerbEntrypoint() {
	int e = pop();
	int v = pop();
	push(getVerbEntrypoint(v, e));
}

void Scumm_v6::o6_arrayOps() {
	byte subOp = fetchScriptByte();
	int array = fetchScriptWord();
	int b, c, d, len;
	ArrayHeader *ah;
	int list[128];

	switch (subOp) {
	case 205:
		b = pop();
		len = resStrLen(_scriptPointer);
		c = defineArray(array, 4, 0, len + 1);
		ah = (ArrayHeader *)getResourceAddress(rtString, c);
		copyScriptString(ah->data + b);
		break;
	case 208:
		b = pop();
		c = pop();
		d = readVar(array);
		if (d == 0) {
			defineArray(array, 5, 0, b + c);
		}
		while (c--) {
			writeArray(array, 0, b + c, pop());
		}
		break;
	case 212:
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
		error("o6_arrayOps: default case %d (array %d)", subOp, array);
	}
}

void Scumm_v6::o6_saveRestoreVerbs() {
	int a, b, c;
	int slot, slot2;

	c = pop();
	b = pop();
	a = pop();

	byte subOp = fetchScriptByte();
	if (_version == 8) {
		subOp = (subOp - 141) + 0xB4;
	}
	
	switch (subOp) {
	case 141:
		while (a <= b) {
			slot = getVerbSlot(a, 0);
			if (slot && _verbs[slot].saveid == 0) {
				_verbs[slot].saveid = c;
				drawVerb(slot, 0);
				verbMouseOver(0);
			}
			a++;
		}
		break;
	case 142:
		while (a <= b) {
			slot = getVerbSlot(a, c);
			if (slot) {
				slot2 = getVerbSlot(a, 0);
				if (slot2)
					killVerb(slot2);
				slot = getVerbSlot(a, c);
				_verbs[slot].saveid = 0;
				drawVerb(slot, 0);
				verbMouseOver(0);
			}
			a++;
		}
		break;
	case 143:
		while (a <= b) {
			slot = getVerbSlot(a, c);
			if (slot)
				killVerb(slot);
			a++;
		}
		break;
	default:
		error("o6_saveRestoreVerbs: default case");
	}
}

void Scumm_v6::o6_drawBox() {
	int x, y, x2, y2, color;
	color = pop();
	y2 = pop();
	x2 = pop();
	y = pop();
	x = pop();
	drawBox(x, y, x2, y2, color);
}

void Scumm_v6::o6_wait() {
	int actnum;
	int offs = -2;
	Actor *a;
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 168:
		offs = fetchScriptWordSigned();
		actnum = pop();
		a = derefActor(actnum, "o6_wait:168");
		if (a->isInCurrentRoom() && a->moving)
			break;
		return;
	case 169:
		if (VAR(VAR_HAVE_MSG))
			break;
		return;
	case 170:
		if (_version >= 7) {
			if (camera._dest != camera._cur)
				break;
		} else {
			if (camera._cur.x >> 3 != camera._dest.x >> 3)
				break;
		}

		return;
	case 171:
		if (_sentenceNum) {
			if (_sentence[_sentenceNum - 1].freezeCount && !isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
				return;
			break;
		}
		if (!isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
			return;
		break;
	case 226:										/* wait until actor drawn */
		offs = fetchScriptWordSigned();
		actnum = pop();
		a = derefActor(actnum, "o6_wait:226");
		if (a->isInCurrentRoom() && a->needRedraw)
			break;
		return;
	case 232:										/* wait until actor stops turning */
		// FIXME: This opcode is really odd. It's used a lot in The Dig.
		// But sometimes it receives the actor ID as params, and sometimes an
		// angle. However in (almost?) all cases, just before calling it, _curActor
		// is set, so we can use it. I tried to add code that detects if an angle
		// is passed, and if so, wait till that angle is reached, but that leads to hangs.
		// It would be very good if somebody could disassmble the original code
		// for this opcode so that we could figure out what's really going on here.
		//
		// For now, if the value passed in is divisible by 45, assume it is an
		// angle, and use _curActor as the actor to wait for.
		offs = fetchScriptWordSigned();
		actnum = pop();
		if (actnum % 45 == 0) {
			actnum = _curActor;
		}
		a = derefActor(actnum, "o6_wait:232b");
		if (a->isInCurrentRoom() && a->moving & MF_TURN)
			break;
		return;
	default:
		error("o6_wait: default case 0x%x", subOp);
	}

	_scriptPointer += offs;
	o6_breakHere();
}

void Scumm_v6::o6_soundKludge() {
	int list[16];
	int num = getStackList(list, ARRAYSIZE(list));

	_sound->soundKludge(list, num);
}

void Scumm_v6::o6_isAnyOf() {
	int list[100];
	int num;
	int16 val;

	num = getStackList(list, ARRAYSIZE(list));
	val = pop();

	while (--num >= 0) {
		if (list[num] == val) {
			push(1);
			return;
		}
	}
	push(0);
	return;
}

void Scumm_v6::o6_quitPauseRestart() {
	byte subOp = fetchScriptByte();
	switch (subOp) {
	case 158:		// Restart
		restart();
		break;
	case 159:		// Pause
		pauseGame();
		break;
	case 160:		// Quit
		shutDown();
		break;
	default:
		error("o6_quitPauseRestart invalid case %d", subOp);
	}
}

void Scumm_v6::o6_delay() {
	// FIXME - what exactly are we measuring here? In order for the other two
	// delay functions to be right, it should be 1/60th of a second. But for
	// CMI it would seem this should delay for 1/10th of a second...
	uint32 delay = (uint16)pop();
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = 1;
	o6_breakHere();
}

void Scumm_v6::o6_delaySeconds() {
	uint32 delay = (uint32)pop();
	if (_gameId != GID_CMI)
		// FIXME - are we really measuring minutes here?
		delay = delay * 60;
	else
		// FIXME - Is this the same in ComI? Seem to need a 1.5 minute 
		// multiplier for correct timing - see patch 664893
		delay = delay * 90;

	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = 1;
	o6_breakHere();
}

void Scumm_v6::o6_delayMinutes() {
	// FIXME - are we really measuring minutes here?
	uint32 delay = (uint16)pop() * 3600;
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = 1;
	o6_breakHere();
}

void Scumm_v6::o6_stopSentence() {
	_sentenceNum = 0;
	stopScript(VAR(VAR_SENTENCE_SCRIPT));
	clearClickedStatus();
}

void Scumm_v6::o6_printLine() {
	_actorToPrintStrFor = 0xFF;
	decodeParseString(0, 0);
}

void Scumm_v6::o6_printCursor() {
	decodeParseString(1, 0);
}

void Scumm_v6::o6_printDebug() {
	decodeParseString(2, 0);
}

void Scumm_v6::o6_printSystem() {
	decodeParseString(3, 0);
}

void Scumm_v6::o6_printActor() {
	decodeParseString(0, 1);
}

void Scumm_v6::o6_printEgo() {
	push(VAR(VAR_EGO));
	decodeParseString(0, 1);
}

void Scumm_v6::o6_talkActor() {
	_actorToPrintStrFor = pop();

	_messagePtr = translateTextAndPlaySpeech(_scriptPointer);
	_scriptPointer += resStrLen(_scriptPointer) + 1;

	setStringVars(0);
	actorTalk();
}

void Scumm_v6::o6_talkEgo() {
	push(VAR(VAR_EGO));
	o6_talkActor();
}

void Scumm_v6::o6_dim() {
	int data;

	switch (fetchScriptByte()) {
	case 199:
		data = 5;
		break;
	case 200:
		data = 1;
		break;
	case 201:
		data = 2;
		break;
	case 202:
		data = 3;
		break;
	case 203:
		data = 4;
		break;
	case 204:
		nukeArray(fetchScriptWord());
		return;
	default:
		error("o6_dim: default case");
	}

	defineArray(fetchScriptWord(), data, 0, pop());
}

void Scumm_v6::o6_dummy() {
	/* nothing */
	if (_features & GF_HUMONGOUS)
		stopObjectCode();
}

void Scumm_v6::o6_dim2() {
	int a, b, data;
	switch (fetchScriptByte()) {
	case 199:
		data = 5;
		break;
	case 200:
		data = 1;
		break;
	case 201:
		data = 2;
		break;
	case 202:
		data = 3;
		break;
	case 203:
		data = 4;
		break;
	default:
		error("o6_dim2: default case");
	}

	b = pop();
	a = pop();
	defineArray(fetchScriptWord(), data, a, b);
}

void Scumm_v6::o6_abs() {
	int a = pop();	// palmos: prevent multi pop if we use an abs function defined as : #define abs(a) ((a) < 0 ? -(a) : (a))
	push(abs(a));
}

void Scumm_v6::o6_distObjectObject() {
	int a, b;
	b = pop();
	a = pop();
	push(getDistanceBetween(true, a, 0, true, b, 0));
}

void Scumm_v6::o6_distObjectPt() {
	int a, b, c;
	c = pop();
	b = pop();
	a = pop();
	push(getDistanceBetween(true, a, 0, false, b, c));
}

void Scumm_v6::o6_distPtPt() {
	int a, b, c, d;
	d = pop();
	c = pop();
	b = pop();
	a = pop();
	push(getDistanceBetween(false, a, b, false, c, d));
}

void Scumm_v6::o6_drawBlastObject() {
	int args[16];
	int a, b, c, d, e;

	getStackList(args, ARRAYSIZE(args));
	e = pop();
	d = pop();
	c = pop();
	b = pop();
	a = pop();
	enqueueObject(a, b, c, d, e, 0xFF, 0xFF, 1, 0);
}

// Set BOMP processing window
void Scumm_v6::o6_setBlastObjectWindow() {
	// TODO - implement this
	int a, b, c, d;

	d = pop();
	c = pop();
	b = pop();
	a = pop();

	warning("o6_bompWindow(%d, %d, %d, %d)", a, b, c, d);
	// sub_274EF(a, b, c, d);
}

void Scumm_v6::o6_kernelSetFunctions() {
	int args[30];
	int num;
	Actor *a;

	num = getStackList(args, ARRAYSIZE(args));

	if (_version >= 7) {
		switch (args[0]) {
		case 4:
			grabCursor(args[1], args[2], args[3], args[4]);
			break;
		case 6: {
//				if (_imuseDigital) {
//					_imuseDigital->stopAllSounds();
//				}
				uint32 speed;
				assert(getStringAddressVar(VAR_VIDEONAME));
				if (strcmp((char *)getStringAddressVar(VAR_VIDEONAME), "sq3.san") == 0)
					speed = 71000;
				else {
					if (_smushFrameRate == 0) 
						_smushFrameRate = 14;
					speed = 1000000 / _smushFrameRate;
				}

				debug(1, "INSANE Arg: %d", args[1]);

				SmushPlayer *sp = new SmushPlayer(this, speed, !_noSubtitles);

				// INSANE mode 0: SMUSH movie playback
				if (args[1] == 0) {
					sp->play((char *)getStringAddressVar(VAR_VIDEONAME), getGameDataPath());
				} else if (_gameId == GID_FT) {
					int insaneMode = readArray(233,0,0);
					debug(1, "FT_INSANE Mode: %d", insaneMode);
						switch (insaneMode) {
						case 0:
							sp->play("minedriv.san", getGameDataPath());
							break;
						case 1:
							sp->play("tovista2.san", getGameDataPath());
							break;
						case 2:
							sp->play("tovista1.san", getGameDataPath());
							break;
						case 3: {
								if (readArray(233,0,50) == 0) {
									InfoDialog *dialog = new InfoDialog(_newgui, this,
														"Set MineRoad - You can now jump the gorge.");
									runDialog (dialog);
									delete dialog;

									writeArray(233, 0, 50, 1); // INSANE callback: Chain
									writeArray(233, 0, 51, 1); // INSANE callback: Chainsaw
									writeArray(233, 0, 52, 1); // INSANE callback: Mace
									writeArray(233, 0, 53, 1); // INSANE callback: 2x4
									writeArray(233, 0, 54, 1); // INSANE callback: Wrench
									writeArray(233, 0, 55, 1); // INSANE callback: Dust

									writeArray(233, 0, 8, 1);  // INSANE callback: Give Googles
									writeArray(233, 0, 7, 1);  // INSANE callback: Give nitro fuel

									putState(235, 1);	   // Cheat and activate Ramp
									writeVar(142 | 0x8000, 1); // Cheat and activate auto-booster (fan)
								}
//								smush->play("minefite.san", getGameDataPath());
							break;
						}
						case 4:
							sp->play("rottopen.san", getGameDataPath());
							break;
						case 5:
						case 6:
						case 7:
						case 8:
							warning("FT_INSANE mode %d: Stub", args[1]);
							break;
						case 9:
							sp->play("credits.san", getGameDataPath());
							break;
						default:
						// Other INSANE modes
							warning("Unknown FT_INSANE mode for %d", args[1]);
							sp->play((char *)getStringAddressVar(VAR_VIDEONAME), getGameDataPath());
					}
				} else {
					// Other INSANE modes
					warning("Unknown insane mode for %d", args[1]);
					sp->play((char *)getStringAddressVar(VAR_VIDEONAME), getGameDataPath());
				}
				delete sp;
			}
			break;
		case 12:
			setCursorImg(args[1], (uint) - 1, args[2]);
			break;
		case 13:
			derefActor(args[1], "o6_kernelSetFunctions:13")->remapActorPalette(args[2], args[3], args[4], -1);
			break;
		case 14:
			derefActor(args[1], "o6_kernelSetFunctions:14")->remapActorPalette(args[2], args[3], args[4], args[5]);
			break;
		case 15:
			_smushFrameRate = args[1];
			break;
		case 16:
		case 17:{
			const byte *message;
			byte buf_input[300], buf_output[300];
			_messagePtr = getStringAddressVar(VAR_STRING2DRAW);
			message = _msgPtrToAdd = buf_input;
			addMessageToStack(_messagePtr);
			if (_gameId == GID_DIG) {
				byte buf_trans[300];
				char *t_ptr = (char *)buf_input;
				buf_output[0] = 0;
				while (t_ptr != NULL) {
					if (*t_ptr == '/') {
						translateText((byte *)t_ptr, buf_trans);
						// hack 
						if (strstr((char *)buf_trans, "%___") != 0) {
							strcat((char *)buf_output, " ");
						} else {
							strcat((char *)buf_output, (char *)buf_trans);
						}
					}
					t_ptr = strchr((char *)t_ptr + 1, '/');
					if (t_ptr == NULL)
						break;
					t_ptr = strchr((char *)t_ptr + 1, '/');
				}
				message = buf_output;
			}
			enqueueText(message, args[3], args[4], args[2], args[1], true);
			break;}
		case 20:
			// Occurs in The Dig, at the alien pyramid. See bug #742979.
			// Also occurs in the first scene of The Dig.
			// Maybe this corresponds to setBoxScaleSlot from V8 ?!
			warning("o6_kernelSetFunctions: stub20(%d, %d)", args[1], args[2]);
			break;
		case 107:
			a = derefActor(args[1], "o6_kernelSetFunctions: 107");
			a->scalex = (unsigned char)args[2];
			a->needRedraw = true;
			break;
		case 108:
			setupShadowPalette(args[1], args[2], args[3], args[4], args[5], args[6]);
			break;
		case 109:
			setupShadowPalette(0, args[1], args[2], args[3], args[4], args[5]);
			break;
		case 114:
			warning("o6_kernelSetFunctions: stub114()");
			break;
		case 117:
			freezeScripts(2);
			break;
		case 118:
			enqueueObject(args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], 3);
			break;
		case 119:
			enqueueObject(args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], 0);
			break;
		case 124:
			_saveSound = args[1];
			break;
		default:
			error("o6_kernelSetFunctions: default case %d (param count %d)", args[0], num);
			break;
		}
	} else {
		switch (args[0]) {
		case 3:
			warning("o6_kernelSetFunctions: nothing in 3");
			break;
		case 4:
			grabCursor(args[1], args[2], args[3], args[4]);
			break;
		case 5:
			fadeOut(args[1]);
			break;
		case 6:
			_fullRedraw = 1;
			redrawBGAreas();
			setActorRedrawFlags();
			processActors();
			fadeIn(args[1]);
			break;
		case 8:
			startManiac();
			break;
		case 9:
			warning("o6_kernelSetFunctions: stub9()");
			break;
		case 104:									/* samnmax */
			nukeFlObjects(args[2], args[3]);
			break;
		case 107:									/* set actor scale */
			a = derefActor(args[1], "o6_kernelSetFunctions: 107");
			a->scalex = (unsigned char)args[2];
			a->needRedraw = true;
			break;
		case 108:									/* create proc_special_palette */
		case 109:
			// FIXME: are 108 and 109 really identical?! That seems unlikely.
			if (num != 6)
				warning("o6_kernelSetFunctions sub op %d: expected 6 params but got %d", args[0], num);
			createSpecialPalette(args[1], args[2], args[3], args[4], args[5], 0, 256);
			break;
		case 110:
			gdi.clearCharsetMask();
			break;
		case 111:
			a = derefActor(args[1], "o6_kernelSetFunctions: 111");
			a->shadow_mode = args[2] + args[3];
			break;
		case 112:									/* palette shift? */
			createSpecialPalette(args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
			break;
		case 114:
			// Sam & Max film noir mode
			if (_gameId == GID_SAMNMAX) {
				// At this point ScummVM will already have set
				// variable 0x8000 to indicate that the game is
				// in film noir mode. All we have to do here is
				// to mark the palette as "dirty", because
				// updatePalette() will desaturate the colors
				// as they are uploaded to the backend.
				//
				// This actually works better than the original
				// interpreter, where actors would sometimes
				// still be drawn in color.
				setDirtyColors(0, 255);
			} else
				warning("stub o6_kernelSetFunctions_114()");
			break;
		case 117:
			// Sam & Max uses this opcode in script-43, right
			// before a screensaver is selected. Perhaps it's
			// supposed to turn off all (other) animations, because
			// some of the screensavers look really stupid in some
			// rooms, e.g. outside the Tunnel of Love, when parts
			// of the screen are being redrawn all the time.
			//
			// But if so, how are the animations turn back on again
			// afterwards? My only possible clue is that when the
			// screensaver is turned off, the VAR_TIMER_NEXT
			// variable is set to 6. But maybe that's unrelated.
			//
			// Sam & Max uses variable 132 to specify the number of
			// minutes of inactivity (no mouse movements) before
			// starting the screensaver, so setting it to 0 will
			// help in debugging.
			warning("stub o6_kernelSetFunctions_117()");
			break;
		case 119:
			enqueueObject(args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], 0);
			break;
		case 120:
			swapPalColors(args[1], args[2]);
			break;
		case 122:
			VAR(VAR_SOUNDRESULT) =
				(short)_imuse->doCommand (num - 1, &args[1]);
			break;
		case 123:
			copyPalColor(args[2], args[1]);
			break;
		case 124:
			_saveSound = args[1];
			break;
		default:
			error("o6_kernelSetFunctions: default case %d (param count %d)", args[0], num);
			break;
		}
	}
}

void Scumm_v6::o6_kernelGetFunctions() {
	int args[30];
	int i;
	Actor *a;

	getStackList(args, ARRAYSIZE(args));

	switch (args[0]) {
	case 113:
		// This is used for the Sam & Max paint-by-numbers mini-game
		// to find out what color to change. I think that what we have
		// the virtual mouse coordinates, because that's what used
		// everywhere else in the script.

		if (args[1] != -1 && args[2] != -1) {
			VirtScreen *vs = &virtscr[0];
			push(vs->screenPtr[args[1] + args[2] * vs->width]);
		} else
			push(0);
		break;
	case 115:
		push(getSpecialBox(args[1], args[2]));
		break;
	case 116:
		push(checkXYInBoxBounds(args[3], args[1], args[2]));
		break;
	case 206:
		push(remapPaletteColor(args[1], args[2], args[3], (uint) - 1));
		break;
	case 207:
		i = getObjectIndex(args[1]);
		push(_objs[i].x_pos);
		break;
	case 208:
		i = getObjectIndex(args[1]);
		push(_objs[i].y_pos);
		break;
	case 209:
		i = getObjectIndex(args[1]);
		push(_objs[i].width);
		break;
	case 210:
		i = getObjectIndex(args[1]);
		push(_objs[i].height);
		break;
	case 211:
		/*
		   13 = thrust
		   336 = thrust
		   328 = thrust
		   27 = abort
		   97 = left
		   331 = left
		   115 = right
		   333 = right
		 */


		if ((args[1] == 27) && (_lastKeyHit == 27)) {
			push(1); // abort
			return;
		}

		if ( ((args[1] == 328) || (args[1] == 336) || (args[1] == 13)) &&
			((VAR(VAR_LEFTBTN_HOLD)) || (_lastKeyHit == 13) || (_lastKeyHit == 274) ||
			(_lastKeyHit == 273)) ) {
			push(1); // thrust
			return;
		}

		if (((args[1] == 97) || (args[1] == 331)) && (_lastKeyHit == 276)) {
			push(1); // left
			return;
		}
		
		if (((args[1] == 115) || (args[1] == 333)) && (_lastKeyHit == 275)) {
			push(1); // right
			return;
		}

		push(0);
		break;
	case 212:
		a = derefActor(args[1], "o6_kernelGetFunctions:212");
		// This is used by walk scripts
		push(a->frame);
		break;
	case 215:
		if ((_extraBoxFlags[args[1]] & 0x00FF) == 0x00C0) {
			push(_extraBoxFlags[args[1]]);
		} else {
			push(getBoxFlags(args[1]));
		}
		break;
	default:
		error("o6_kernelGetFunctions: default case %d", args[0]);
	}
}

void Scumm_v6::o6_delayFrames() {
	ScriptSlot *ss = &vm.slot[_currentScript];
	if (ss->delayFrameCount == 0) {
		ss->delayFrameCount = pop();
	} else {
		ss->delayFrameCount--;
	}
	if (ss->delayFrameCount) {
		_scriptPointer--;
		o6_breakHere();
	}
}

void Scumm_v6::o6_pickOneOf() {
	int args[100];
	int i, num;

	num = getStackList(args, ARRAYSIZE(args));
	i = pop();
	if (i < 0 || i > num)
		error("o6_pickOneOf: %d out of range (0, %d)", i, num - 1);
	push(args[i]);
}

void Scumm_v6::o6_pickOneOfDefault() {
	int args[100];
	int i, num, def;

	def = pop();
	num = getStackList(args, ARRAYSIZE(args));
	i = pop();
	if (i < 0 || i >= num)
		i = def;
	else
		i = args[i];
	push(i);
}

void Scumm_v6::o6_stampObject() {
	int object, x, y, state;

	state = pop();
	y = pop();
	x = pop();
	object = pop();
	if (object < 30) {
		if (state == 0) {
			state = 255;
		}
		warning("o6_stampObject: (%d at (%d,%d) scale %d)", object, x, y, state);
		Actor *a = derefActor(object, "o6_stampObject");
		a->scalex = state;
		a->scaley = state;
		a->putActor(x, y, _currentRoom); // TODO
		a->drawActorCostume();
//		drawActor(object, maskBufferPtr, x_y, scale_x_y);
//		drawActor(object, mainBufferPtr, x_y, scale_x_y);
		return;
	}
	
	if (object == 0) {
		state = 1;
	}

	if (x != -1) {
		setObjectState(object, state, x, y);
		drawObject(getObjectIndex(object), 0);
		warning("o6_stampObject: (%d at (%d,%d) state %d)", object, x, y, state);
	}
}

void Scumm_v6::o6_stopTalking() {
	stopTalk();
}

void Scumm_v6::o6_openFile() {
	int a, len;
	a = pop();
	len = resStrLen(_scriptPointer);
	warning("stub o6_openFile(\"%s\", %d)", _scriptPointer, a);
	_scriptPointer += len + 1;
	// -1 open failed, otherwise file handle?
	push(0);
}

void Scumm_v6::o6_closeFile() {
	// pop'd variable should be that pushed in o6_openFile()
	warning("stub o6_closeFile(%d)", pop());
}

void Scumm_v6::o6_deleteFile() {
	int len;
	len = resStrLen(_scriptPointer);
	warning("stub o6_deleteFile(\"%s\")", _scriptPointer);
	_scriptPointer += len + 1;
}

void Scumm_v6::o6_readFile() {
	error("stub o6_readFile(%d, %d)", pop(), pop());
}

void Scumm_v6::o6_findAllObjects() {
	// FIXME is this even remotely correct?
	// see http://users.bigpond.net.au/tgray2/findallobjects.txt
	// for asm
	int a = pop();
	int i = 1;

	warning("stub o6_findAllObjects(%d)", a);
	if (a != _currentRoom)
		warning("o6_findAllObjects: current room is not %d", a);
	writeVar(0, 0);
	defineArray(0, 5, 0, _numLocalObjects + 1);
	writeArray(0, 0, 0, _numLocalObjects);
	
	while (i < _numLocalObjects) {
		writeArray(0, 0, i, _objs[i].obj_nr);
		i++;
	}
	
	push(readVar(0));
}

void Scumm_v6::shuffleArray(int num, int minIdx, int maxIdx) {
	int range = maxIdx - minIdx;
	int count = range * 2;

	// Shuffle the array 'num'
	while (count--) {
		// Determine two random elements...
		int rand1 = _rnd.getRandomNumber(range) + minIdx;
		int rand2 = _rnd.getRandomNumber(range) + minIdx;
		
		// ...and swap them
		int val1 = readArray(num, 0, rand1);
		int val2 = readArray(num, 0, rand2);
		writeArray(num, 0, rand1, val2);
		writeArray(num, 0, rand2, val1);
	}
}

void Scumm_v6::o6_shuffle() {
	shuffleArray(fetchScriptWord(), pop(), pop());
}

void Scumm_v6::o6_pickVarRandom() {
	int num;
	int args[100];
	int var_C, var_A;

	num = getStackList(args, ARRAYSIZE(args));
	int value = fetchScriptWord();

	if (readVar(value) == 0) {
		defineArray(value, 5, 0, num);
		if (num > 0) {
			int16 counter = 0;
			do {
				writeArray(value, 0, counter + 1, args[counter]);
			} while (++counter < num);
		}

		shuffleArray(value, 1, num);
		writeArray(value, 0, 0, 2);
		push(readArray(value, 0, 1));
		return;
	}

	num = readArray(value, 0, 0);

	byte *ptr = getResourceAddress(rtString, num);
	if (_version >= 7) {
		var_A = READ_LE_UINT32(ptr + 4);
		var_C = READ_LE_UINT32(ptr + 8);
	} else {
		var_A = READ_LE_UINT16(ptr + 2);
		var_C = READ_LE_UINT16(ptr + 4);
	}

	if (var_A <= num) {
		int16 var_2 = readArray(value, 0, num - 1);
		shuffleArray(value, 1, var_A - 1);
		if (readArray(value, 0, 1) == var_2) {
			num = 2;
		} else {
			num = 1;
		}
	}

	writeArray(value, 0, 0, num + 1);
	push(readArray(value, 0, num));
}

void Scumm_v6::o6_getDateTime() {
	struct tm *t;
	time_t now = time(NULL);
	
	t = localtime(&now);

	VAR(VAR_TIMEDATE_YEAR) = t->tm_year;
	VAR(VAR_TIMEDATE_MONTH) = t->tm_mon;
	VAR(VAR_TIMEDATE_DAY) = t->tm_mday;
	VAR(VAR_TIMEDATE_HOUR) = t->tm_hour;
	VAR(VAR_TIMEDATE_MINUTE) = t->tm_min;
	
	if (_version == 8)
		VAR(VAR_TIMEDATE_SECOND) = t->tm_sec;
}

void Scumm_v6::o6_unknownE1() {
	// this opcode check ground area in minigame "Asteroid Lander" in the dig
	int y = pop();
	int x = pop();

	if (x > _screenWidth - 1) {
		push(-1);
		return;
	}
	if (x < 0) {
		push(-1);
		return;
	}

	if (y < 0) {
		push(-1);
		return;
	}
	
	VirtScreen *vs = findVirtScreen(y);

	if (vs == NULL) {
		push(-1);
		return;
	}

	int offset = (y - vs->topline) * _screenWidth + x + _screenLeft;

	byte area = *(getResourceAddress(rtBuffer, vs->number + 1) + offset);
	push(area);
}

void Scumm_v6::o6_unknownE0() {
	int a = fetchScriptByte();
	a -= 222;
	if (a != 0) {
		a -= 2;
		if (a != 0) 
			return;
		warning("o6_unknownE0(%d) stub", pop());
	} else {
		warning("o6_uknownE0, sound volume %d stub", pop());
	}
}

void Scumm_v6::o6_unknownE4() {
	warning("o6_unknownE4(%d) stub", pop());
}

void Scumm_v6::o6_unknownFA() {
	int len, a = fetchScriptByte();
	
	len = resStrLen(_scriptPointer);
	warning("stub o6_unknownFA(%d, \"%s\")", a, _scriptPointer);
	_scriptPointer += len + 1;
}

void Scumm_v6::o6_unknownEA() {
	int edi, esi, eax;
	edi = pop();
	esi = pop();

	if ((edi | esi) ==  0) {
		eax = esi;
		esi = edi;
		edi = eax;
	}

	eax = fetchScriptByte();
	switch (eax) {
		case 197:
			unknownEA_func(5, esi, edi, fetchScriptWord(), eax);
			break;
		case 202:
			unknownEA_func(3, esi, edi, fetchScriptWord(), eax);
			break;
		default:
			break;
	}
}

void Scumm_v6::unknownEA_func(int a, int b, int c, int d, int e) {
	warning("unknownEA_func(%d, %d, %d, %d, %d) stub", a, b, c, d, e);
}

void Scumm_v6::o6_readINI() {
	int len;

	len = resStrLen(_scriptPointer);
	warning("stub o6_readINI(\"%s\")", _scriptPointer);
	_scriptPointer += len + 1;
	pop();
	push(0);
	
}

void Scumm_v6::o6_localizeArray() {
	warning("stub localizeArray(%d)", pop());
}

void Scumm_v6::decodeParseString(int m, int n) {
	byte b;

	b = fetchScriptByte();

	switch (b) {
	case 65:
		_string[m].ypos = pop();
		_string[m].xpos = pop();
		_string[m].overhead = false;
		break;
	case 66:
		_string[m].color = pop();
		break;
	case 67:
		_string[m].right = pop();
		break;
	case 69:
		_string[m].center = true;
		_string[m].overhead = false;
		break;
	case 71:
		_string[m].center = false;
		_string[m].overhead = false;
		break;
	case 72:
		_string[m].overhead = true;
		_string[m].no_talk_anim = false;
		break;
	case 73:
		error("decodeParseString: case 73");
		break;
	case 74:
		_string[m].no_talk_anim = true;
		break;
	case 75:
		_messagePtr = translateTextAndPlaySpeech(_scriptPointer);
		_scriptPointer += resStrLen(_scriptPointer)+ 1;

		switch (m) {
		case 0:
			actorTalk();
			break;
		case 1:
			drawString(1);
			break;
		case 2:
			unkMessage1();
			break;
		case 3:
			unkMessage2();
			break;
		}
		return;
	case 0xF9:
		error("decodeParseString case 0xF9 stub");
		return;
	case 0xFE:
		setStringVars(m);
		if (n)
			_actorToPrintStrFor = pop();
		return;
	case 0xFF:
		_string[m].t_xpos = _string[m].xpos;
		_string[m].t_ypos = _string[m].ypos;
		_string[m].t_center = _string[m].center;
		_string[m].t_overhead = _string[m].overhead;
		_string[m].t_no_talk_anim = _string[m].no_talk_anim;
		_string[m].t_right = _string[m].right;
		_string[m].t_color = _string[m].color;
		_string[m].t_charset = _string[m].charset;
		return;
	default:
		error("decodeParseString: default case 0x%x", b);
	}
}
