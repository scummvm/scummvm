/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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
#include "scumm.h"
#include "actor.h"
#include "verbs.h"
#include "smush/player.h"
#include "smush/scumm_renderer.h"

#include "sound/mididrv.h"
#include "scumm/sound.h"
#include "scumm/imuse.h"

#include "dialogs.h"		// FIXME: This is just for the FT-INSANE warning. 
				// Remove when INSANE is implemented

#define OPCODE(x)	{ &Scumm::x, #x }

void Scumm::setupOpcodes2()
{
	static const OpcodeEntry opcodes[256] = {
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
		OPCODE(o6_zero),
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
		OPCODE(o6_kill),
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
		OPCODE(o6_startObject),
		OPCODE(o6_setObjectState),
		OPCODE(o6_setObjectXY),
		OPCODE(o6_drawBlastObject),
		/* 64 */
		OPCODE(o6_setBlastObjectWindow),
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_endCutscene),
		/* 68 */
		OPCODE(o6_cutScene),
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
		OPCODE(o6_getScriptRunning),
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
		OPCODE(o6_actorSet),
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
		OPCODE(o6_invalid),
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
		OPCODE(o6_delayLonger),
		OPCODE(o6_delayVeryLong),
		OPCODE(o6_stopSentence),
		/* B4 */
		OPCODE(o6_print_0),
		OPCODE(o6_print_1),
		OPCODE(o6_print_2),
		OPCODE(o6_print_3),
		/* B8 */
		OPCODE(o6_printActor),
		OPCODE(o6_printEgo),
		OPCODE(o6_talkActor),
		OPCODE(o6_talkEgo),
		/* BC */
		OPCODE(o6_dim),
		OPCODE(o5_dummy),
		OPCODE(o6_runVerbCodeQuick),
		OPCODE(o6_runScriptQuick),
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
		OPCODE(o6_kernelFunction),
		OPCODE(o6_miscOps),
		OPCODE(o6_delayFrames),
		OPCODE(o6_pickOneOf),
		/* CC */
		OPCODE(o6_pickOneOfDefault),
		OPCODE(o6_unknownCD),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* D0 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_getAnimateVariable),
		OPCODE(o6_invalid),
		/* D4 */
		OPCODE(o6_invalid),
		OPCODE(o6_jumpToScript),
		OPCODE(o6_band),
		OPCODE(o6_bor),
		/* D8 */
		OPCODE(o6_isRoomScriptRunning),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* DC */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* E0 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* E4 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* E8 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* EC */
		OPCODE(o6_getActorPriority),
		OPCODE(o6_getObjectNewDir),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* F0 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* F4 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* F8 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* FC */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
	};

	_opcodes = opcodes;
}
void Scumm::o6_setBlastObjectWindow()
{																// Set BOMP processing window
	int a, b, c, d;

	d = pop();
	c = pop();
	b = pop();
	a = pop();

	warning("o6_bompWindow(%d, %d, %d, %d)", a, b, c, d);
	// sub_274EF(a, b, c, d);
}

int Scumm::popRoomAndObj(int *room)
{
	int obj;

	if (_features & GF_HAS_ROOMTABLE) {
		obj = pop();
		*room = getObjectRoom(obj);
	} else {
		*room = pop();
		obj = pop();
	}

	return obj;
}


int Scumm::readArray(int array, int idx, int base)
{
	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(array));

	if (ah == NULL) {
		error("readArray: invalid array %d (%d)", array, readVar(array));
	}

	base += idx * ah->dim1_size;

	// FIXME: comment this for the time being as it was causing ft to crash
	// in the minefeild
	// assert(base >= 0 && base < ah->dim1_size * ah->dim2_size);

	if (ah->type == 4) {
		return ah->data[base];
	} else {
		return (int16)READ_LE_UINT16(ah->data + base * 2);
	}
}

void Scumm::writeArray(int array, int idx, int base, int value)
{
	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(array));
	if (!ah)
		return;
	base += idx * ah->dim1_size;

	assert(base >= 0 && base < ah->dim1_size * ah->dim2_size);

	if (ah->type == 4) {
		ah->data[base] = value;
	} else {
		((uint16 *)ah->data)[base] = TO_LE_16(value);
	}
}

int Scumm::getStackList(int16 *args, uint maxnum)
{
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

void Scumm::o6_pushByte()
{
	push(fetchScriptByte());
}

void Scumm::o6_pushWord()
{
	push((int16)fetchScriptWord());
}

void Scumm::o6_pushByteVar()
{
	push(readVar(fetchScriptByte()));
}

void Scumm::o6_pushWordVar()
{
	push(readVar(fetchScriptWord()));
}

void Scumm::o6_invalid()
{
	error("Invalid opcode '%x' at %x", _opcode, _scriptPointer - _scriptOrgPointer);
}

void Scumm::o6_byteArrayRead()
{
	int base = pop();
	push(readArray(fetchScriptByte(), 0, base));
}

void Scumm::o6_wordArrayRead()
{
	int base = pop();
	push(readArray(fetchScriptWord(), 0, base));
}

void Scumm::o6_byteArrayIndexedRead()
{
	int base = pop();
	int idx = pop();
	push(readArray(fetchScriptByte(), idx, base));
}

void Scumm::o6_wordArrayIndexedRead()
{
	int base = pop();
	int idx = pop();
	push(readArray(fetchScriptWord(), idx, base));
}

void Scumm::o6_dup()
{
	int a = pop();
	push(a);
	push(a);
}

void Scumm::o6_zero()
{
	push(pop() == 0);
}

void Scumm::o6_eq()
{
	push(pop() == pop());
}

void Scumm::o6_neq()
{
	push(pop() != pop());
}

void Scumm::o6_gt()
{
	int a = pop();
	push(pop() > a);
}

void Scumm::o6_lt()
{
	int a = pop();
	push(pop() < a);
}

void Scumm::o6_le()
{
	int a = pop();
	push(pop() <= a);
}

void Scumm::o6_ge()
{
	int a = pop();
	push(pop() >= a);
}

void Scumm::o6_add()
{
	int a = pop();
	push(pop() + a);
}

void Scumm::o6_sub()
{
	int a = pop();
	push(pop() - a);
}

void Scumm::o6_mul()
{
	int a = pop();
	push(pop() * a);
}

void Scumm::o6_div()
{
	int a = pop();
	if (a == 0)
		error("division by zero");
	push(pop() / a);
}

void Scumm::o6_land()	// Logical And
{
	int a = pop();
	push(pop() && a);
}

void Scumm::o6_lor()	// Logical Or
{
	int a = pop();
	push(pop() || a);
}

void Scumm::o6_bor()	// Bitwise Or
{
	int a = pop();
	push(pop() | a);
}

void Scumm::o6_band()	// Bitwise And
{
	int a = pop();
	push(pop() | a);
}

void Scumm::o6_kill()
{
	pop();
}

void Scumm::o6_writeByteVar()
{
	writeVar(fetchScriptByte(), pop());
}

void Scumm::o6_writeWordVar()
{
	writeVar(fetchScriptWord(), pop());
}

void Scumm::o6_byteArrayWrite()
{
	int a = pop();
	writeArray(fetchScriptByte(), 0, pop(), a);
}

void Scumm::o6_wordArrayWrite()
{
	int a = pop();
	writeArray(fetchScriptWord(), 0, pop(), a);
}

void Scumm::o6_byteArrayIndexedWrite()
{
	int val = pop();
	int base = pop();
	writeArray(fetchScriptByte(), pop(), base, val);
}

void Scumm::o6_wordArrayIndexedWrite()
{
	int val = pop();
	int base = pop();
	writeArray(fetchScriptWord(), pop(), base, val);
}

void Scumm::o6_byteVarInc()
{
	int var = fetchScriptByte();
	writeVar(var, readVar(var) + 1);
}

void Scumm::o6_wordVarInc()
{
	int var = fetchScriptWord();
	writeVar(var, readVar(var) + 1);
}

void Scumm::o6_byteArrayInc()
{
	int var = fetchScriptByte();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) + 1);
}

void Scumm::o6_wordArrayInc()
{
	int var = fetchScriptWord();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) + 1);
}


void Scumm::o6_byteVarDec()
{
	int var = fetchScriptByte();
	writeVar(var, readVar(var) - 1);
}

void Scumm::o6_wordVarDec()
{
	int var = fetchScriptWord();
	writeVar(var, readVar(var) - 1);
}

void Scumm::o6_byteArrayDec()
{
	int var = fetchScriptByte();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) - 1);
}

void Scumm::o6_wordArrayDec()
{
	int var = fetchScriptWord();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) - 1);
}

void Scumm::o6_jumpTrue()
{
	if (pop())
		o6_jump();
	else
		fetchScriptWord();
}

void Scumm::o6_jumpFalse()
{
	if (!pop())
		o6_jump();
	else
		fetchScriptWord();
}

void Scumm::o6_jump()
{
	_scriptPointer += (int16)fetchScriptWord();
}

void Scumm::o6_startScriptEx()
{
	int16 args[16];
	int script, flags;

	getStackList(args, sizeof(args) / sizeof(args[0]));
	script = pop();
	flags = pop();
	runScript(script, flags & 1, flags & 2, args);
}

void Scumm::o6_jumpToScript()
{
	int16 args[16];
	int script, flags;

	getStackList(args, sizeof(args) / sizeof(args[0]));
	script = pop();
	flags = pop();
	o6_stopObjectCode();
	runScript(script, flags & 1, flags & 2, args);
}

void Scumm::o6_startScript()
{
	int16 args[16];
	int script;
	getStackList(args, sizeof(args) / sizeof(args[0]));
	script = pop();
	runScript(script, 0, 0, args);
}

void Scumm::o6_startObject()
{
	int16 args[16];
	int script, entryp;
	int flags;
	getStackList(args, sizeof(args) / sizeof(args[0]));
	entryp = pop();
	script = pop();
	flags = pop();
	runVerbCode(script, entryp, flags & 1, flags & 2, args);
}

void Scumm::o6_setObjectState()
{
	int a = pop();
	int b = pop();
	if (a == 0)
		a = 1;
//  debug(1, "setObjectState(%d,%d)", a, b);

	setObjectState(b, a, -1, -1);
}

void Scumm::o6_setObjectXY()
{
	int y = pop();
	int x = pop();
	int obj = pop();
//  debug(1, "setObjectXY(%d,%d,%d)", obj, x, y);
	setObjectState(obj, 1, x, y);
}

void Scumm::o6_stopObjectCode()
{
	stopObjectCode();
}

void Scumm::o6_endCutscene()
{
	endCutscene();
}

void Scumm::o6_cutScene()
{
	int16 args[16];
	getStackList(args, sizeof(args) / sizeof(args[0]));
	cutscene(args);
}

void Scumm::o6_stopMusic()
{
	_sound->stopAllSounds();
}

void Scumm::o6_freezeUnfreeze()
{
	int a = pop();
	if (a)
		freezeScripts(a);
	else
		unfreezeScripts();
}

void Scumm::o6_cursorCommand()
{
	int a, i;
	int16 args[16];

	switch (fetchScriptByte()) {
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
			int room, obj = popRoomAndObj(&room);
			setCursorImg(obj, room, 1);
			break;
		}
	case 0x9A:
		a = pop();
		setCursorHotspot2(pop(), a);
		break;
	case 0x9C:										/* init charset */
		initCharset(pop());
		break;
	case 0x9D:										/* set charset colors */
		getStackList(args, sizeof(args) / sizeof(args[0]));
		for (i = 0; i < 16; i++)
			charset._colorMap[i] = _charsetData[_string[1].t_charset][i] = (unsigned char)args[i];
		break;
	case 0xD6:
		makeCursorColorTransparent(pop());
		break;
	default:
		error("o6_cursorCommand: default case");
	}

	_vars[VAR_CURSORSTATE] = _cursor.state;
	_vars[VAR_USERPUT] = _userPut;
}

void Scumm::o6_breakHere()
{
	updateScriptPtr();
	_currentScript = 0xFF;
}

void Scumm::o6_ifClassOfIs()
{
	int16 args[16];
	int num, obj, cls;
	bool b;
	int cond = 1;

	num = getStackList(args, sizeof(args) / sizeof(args[0]));
	obj = pop();

	while (--num >= 0) {
		cls = args[num];
		b = getClass(obj, cls);
		if (cls & 0x80 && !b || !(cls & 0x80) && b)
			cond = 0;
	}
	push(cond);
}

void Scumm::o6_setClass()
{
	int16 args[16];
	int num, obj, cls;

	num = getStackList(args, sizeof(args) / sizeof(args[0]));
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

void Scumm::o6_getState()
{
	push(getState(pop()));
}

void Scumm::o6_setState()
{
	int state = pop();
	int obj = pop();

//  debug(1, "setState(%d,%d)", obj, state);

	putState(obj, state);
	removeObjectFromRoom(obj);
	if (_BgNeedsRedraw)
		clearDrawObjectQueue();
}

void Scumm::o6_setOwner()
{
	int owner = pop();
	int obj = pop();

	setOwnerOf(obj, owner);
}

void Scumm::o6_getOwner()
{
	push(getOwner(pop()));
}

void Scumm::o6_startSound()
{
	_sound->addSoundToQueue(pop());
}

void Scumm::o6_stopSound()
{
	_sound->stopSound(pop());
}

void Scumm::o6_startMusic()
{
	_sound->addSoundToQueue(pop());
}

void Scumm::o6_stopObjectScript()
{
	stopObjectScript(pop());
}

void Scumm::o6_panCameraTo()
{
	if (_features & GF_AFTER_V7) {
		int y = pop();
		int x = pop();
		panCameraTo(x, y);
	} else {
		panCameraTo(pop(), 0);
	}
}

void Scumm::o6_actorFollowCamera()
{
	if (_features & GF_AFTER_V7)
		setCameraFollows(derefActorSafe(pop(), "actorFollowCamera"));
	else
		actorFollowCamera(pop());

}

void Scumm::o6_setCameraAt()
{
	if (_features & GF_AFTER_V7) {
		int x, y;

		camera._follows = 0;

		y = pop();
		x = pop();

		setCameraAt(x, y);
	} else {
		setCameraAtEx(pop());
	}
}

void Scumm::o6_loadRoom()
{
	int room;
	/* Begin: Autosave 
	   _saveLoadSlot = 0;
	   sprintf(_saveLoadName, "Autosave", _saveLoadSlot);
	   _saveLoadFlag = 1;
	   _saveLoadCompatible = false;
	   End: Autosave */

	room = pop();
	startScene(room, 0, 0);
	_fullRedraw = 1;
}

void Scumm::o6_stopScript()
{
	int script = pop();
	if (script == 0)
		stopObjectCode();
	else
		stopScriptNr(script);
}

void Scumm::o6_walkActorToObj()
{
	int obj, dist;
	Actor *a, *a2;
	int x;

	dist = pop();
	obj = pop();
	a = derefActorSafe(pop(), "o6_walkActorToObj");
	if (!a)
		return;

	if (obj >= NUM_ACTORS) {
		if (whereIsObject(obj) == WIO_NOT_FOUND)
			return;
		int y, dir;
		getObjectXYPos(obj, x, y, dir);
		a->startWalkActor(x, y, dir);
	} else {
		a2 = derefActorSafe(obj, "o6_walkActorToObj(2)");
		if (!a2)
			return;
		if (!a->isInCurrentRoom() || !a2->isInCurrentRoom())
			return;
		if (dist == 0) {
			dist = a2->scalex * a2->width / 0xFF;
			dist += dist >> 1;
		}
		x = a2->x;
		if (x < a->x)
			x += dist;
		else
			x -= dist;
		a->startWalkActor(x, a2->y, -1);
	}
}

void Scumm::o6_walkActorTo()
{
	int x, y;
	y = pop();
	x = pop();
	derefActorSafe(pop(), "o6_walkActorTo")->startWalkActor(x, y, -1);
}

void Scumm::o6_putActorInRoom()
{
	int room, x, y;
	Actor *a;

	room = pop();
	y = pop();
	x = pop();
	a = derefActorSafe(pop(), "o6_putActorInRoom");
	if (!a)
		return;

	if (room == 0xFF) {
		room = a->room;
	} else {
		if (a->visible && _currentRoom != room && _vars[VAR_TALK_ACTOR] == a->number) {
			clearMsgQueue();
		}
		if (room != 0)
			a->room = room;
	}
	a->putActor(x, y, room);
}


void Scumm::o6_putActorAtObject()
{
	int room, obj, x, y;
	Actor *a;

	obj = popRoomAndObj(&room);

	a = derefActorSafe(pop(), "o6_putActorAtObject");
	if (whereIsObject(obj) != WIO_NOT_FOUND) {
		getObjectXYPos(obj, x, y);
	} else {
		x = (_realWidth / 2);
		y = 120;
	}
	if (room == 0xFF)
		room = a->room;
	a->putActor(x, y, room);
}

void Scumm::o6_faceActor()
{
	int act, obj;
	obj = pop();
	act = pop();
	faceActorToObj(act, obj);
}

void Scumm::o6_animateActor()
{
	int anim = pop();
	int act = pop();

	Actor *a = derefActorSafe(act, "o6_animateActor");
	if (!a)
		return;

	a->animateActor(anim);
}

void Scumm::o6_doSentence()
{
	int a, b, c;

	a = pop();
	pop();												//dummy pop
	b = pop();
	c = pop();

	doSentence(c, b, a);
}

void Scumm::o6_pickupObject()
{
	int obj, room;
	int i;

	obj = popRoomAndObj(&room);
	if (room == 0)
		room = _roomResource;

	for (i = 1; i < _maxInventoryItems; i++) {
		if (_inventory[i] == (uint16)obj) {
			putOwner(obj, _vars[VAR_EGO]);
			runHook(obj);
			return;
		}
	}

	addObjectToInventory(obj, room);
	putOwner(obj, _vars[VAR_EGO]);
	putClass(obj, 32, 1);
	putState(obj, 1);
	removeObjectFromRoom(obj);
	clearDrawObjectQueue();
	runHook(obj);									/* Difference */
}

void Scumm::o6_loadRoomWithEgo()
{
	Actor *a;
	int obj, room, x, y;

	y = pop();
	x = pop();

	obj = popRoomAndObj(&room);

	a = derefActorSafe(_vars[VAR_EGO], "o6_loadRoomWithEgo");

	a->putActor(0, 0, room);
	_egoPositioned = false;

	_vars[VAR_WALKTO_OBJ] = obj;
	startScene(a->room, a, obj);
	_vars[VAR_WALKTO_OBJ] = 0;

	/* startScene maybe modifies VAR_EGO, i hope not */

	if (!(_features & GF_AFTER_V7)) {
		setCameraAt(a->x, a->y);
		setCameraFollows(a);
	}

	_fullRedraw = 1;

	if (x != -1) {
		a->startWalkActor(x, y, -1);
	}
}

void Scumm::o6_getRandomNumber()
{
	int rnd;
	rnd = _rnd.getRandomNumber(pop());
	_vars[VAR_V6_RANDOM_NR] = rnd;
	push(rnd);
}

void Scumm::o6_getRandomNumberRange()
{
	int max = pop();
	int min = pop();
	int rnd = _rnd.getRandomNumberRng(min, max);
	_vars[VAR_V6_RANDOM_NR] = rnd;
	push(rnd);
}

void Scumm::o6_getActorMoving()
{
	push(derefActorSafe(pop(), "o6_getActorMoving")->moving);
}

void Scumm::o6_getScriptRunning()
{
	push(isScriptRunning(pop()));
}

void Scumm::o6_isRoomScriptRunning()
{
	push(isRoomScriptRunning(pop()));
}

void Scumm::o6_getActorRoom()
{
	push(derefActorSafe(pop(), "o6_getActorRoom")->room);
}

void Scumm::o6_getObjectX()
{
	push(getObjX(pop()));
}

void Scumm::o6_getObjectY()
{
	push(getObjY(pop()));
}

void Scumm::o6_getObjectOldDir()
{
	push(getObjOldDir(pop()));
}

void Scumm::o6_getObjectNewDir()
{
	push(getObjNewDir(pop()));
}

void Scumm::o6_getActorWalkBox()
{
	Actor *a = derefActorSafe(pop(), "o6_getActorWalkBox");
	push(a->ignoreBoxes ? 0 : a->walkbox);
}

void Scumm::o6_getActorCostume()
{
	push(derefActorSafe(pop(), "o6_getActorCostume")->costume);
}

void Scumm::o6_findInventory()
{
	int idx = pop();
	int owner = pop();
	push(findInventory(owner, idx));
}

void Scumm::o6_getInventoryCount()
{
	push(getInventoryCount(pop()));
}

void Scumm::o6_getVerbFromXY()
{
	int y = pop();
	int x = pop();
	int over = checkMouseOver(x, y);
	if (over)
		over = _verbs[over].verbid;
	push(over);
}

void Scumm::o6_beginOverride()
{
	beginOverride();
}

void Scumm::o6_endOverride()
{
	endOverride();
}

void Scumm::o6_setObjectName()
{
	int obj = pop();
	int i;

	if (obj < NUM_ACTORS)
		error("Can't set actor %d name with new-name-of", obj);

	if (!(_features & GF_AFTER_V7) && !getOBCDFromObject(obj))
		error("Can't set name of object %d", obj);

	for (i = 1; i < 50; i++) {
		if (_newNames[i] == obj) {
			nukeResource(rtObjectName, i);
			_newNames[i] = 0;
			break;
		}
	}

	for (i = 1; i < 50; i++) {
		if (_newNames[i] == 0) {
			loadPtrToResource(rtObjectName, i, NULL);
			_newNames[i] = obj;
			runHook(0);
			return;
		}
	}

	error("New name of %d overflows name table (max = %d)", obj, 50);
}

void Scumm::o6_isSoundRunning()
{
	int snd = pop();

	if (snd)
		snd = _sound->isSoundRunning(snd);

	push(snd);
}

void Scumm::o6_setBoxFlags()
{
	int16 table[65];
	int num, value;

	value = pop();
	num = getStackList(table, sizeof(table) / sizeof(table[0]));

	while (--num >= 0) {
		setBoxFlags(table[num], value);
	}
}

void Scumm::o6_createBoxMatrix()
{
	createBoxMatrix();
}

void Scumm::o6_resourceRoutines()
{
	int resid;

	switch (fetchScriptByte()) {
	case 100:										/* load script */
		resid = pop();
		if (_features & GF_AFTER_V7)
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
		if (_features & GF_AFTER_V7)
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
	default:
		error("o6_resourceRoutines: default case");
	}
}

void Scumm::o6_roomOps()
{
	int a, b, c, d, e;

	switch (fetchScriptByte()) {
	case 172:										/* room scroll */
		b = pop();
		a = pop();
		if (a < (_realWidth / 2))
			a = (_realWidth / 2);
		if (b < (_realWidth / 2))
			b = (_realWidth / 2);
		if (a > _scrWidth - (_realWidth / 2))
			a = _scrWidth - (_realWidth / 2);
		if (b > _scrWidth - (_realWidth / 2))
			b = _scrWidth - (_realWidth / 2);
		_vars[VAR_CAMERA_MIN_X] = a;
		_vars[VAR_CAMERA_MAX_X] = b;
		break;

	case 174:										/* set screen */
		b = pop();
		a = pop();
		initScreens(0, a, _realWidth, b);
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
		darkenPalette(b, c, a, a, a);
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
			_switchRoomEffect = (byte)(a);
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
		darkenPalette(d, e, a, b, c);
		break;

	case 183:
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		unkRoomFunc3(d, e, a, b, c);
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
		// One case where this is used is to turn off Sam & Max film
		// noir mode. Unfortunately it only restores color to the
		// palette, it doesn't take palette manipulation (darkening,
		// etc.) into account. So, for instance, if you turn on film
		// noir mode in Sam & Max's office, turn off the light and turn
		// off film noir mode, the room will no longer look dark.
		//
		// This bug is present in the original interpreter, so it may
		// not be worth the trouble fixing it.
		setPalette(pop());
		break;

	default:
		error("o6_roomOps: default case");
	}
}

void Scumm::o6_actorSet()
{
	Actor *a;
	int i, j, k;
	int16 args[8];
	byte b;

	b = fetchScriptByte();
	if (b == 197) {
		_curActor = pop();
		return;
	}

	a = derefActorSafe(_curActor, "o6_actorSet");
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
		k = getStackList(args, sizeof(args) / sizeof(args[0]));
		for (i = 0; i < k; i++)
			a->sound[i] = args[i];
		break;
	case 79:											/* actor-walkframe */
		a->walkFrame = pop();
		break;
	case 80:											/* actor-talkframe */
		a->talkFrame2 = pop();
		a->talkFrame1 = pop();
		break;
	case 81:											/* actor-standframe */
		a->standFrame = pop();
		break;
	case 82:
		pop();
		pop();
		pop();
		break;
	case 83:
		a->initActor(0);
		break;
	case 84:											/* actor-elevation */
		a->elevation = pop();
		a->needRedraw = true;
		a->needBgReset = true;
		break;
	case 85:											/* actor-defaultanims */
		a->initFrame = 1;
		a->walkFrame = 2;
		a->standFrame = 3;
		a->talkFrame1 = 4;
		a->talkFrame2 = 5;
		break;
	case 86:											/* actor-palette */
		j = pop();
		i = pop();
		checkRange(31, 0, i, "Illegal palet slot %d");
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
		a->needBgReset = true;
		break;
	case 93:
		a->forceClip = 0;
		break;
	case 225:
	case 94:
		a->forceClip = pop();
		break;
	case 95:
		a->ignoreBoxes = 1;
		if (_features & GF_AFTER_V7)	// yazoo: I don't know if it's supposed to be 100 in other games too...
			a->forceClip = 100;
		else
			a->forceClip = 0;
	FixRooms:;
		if (a->isInCurrentRoom())
			a->putActor(a->x, a->y, a->room);
		break;
	case 96:
		a->ignoreBoxes = 0;
		if (_features & GF_AFTER_V7)	// yazoo: I don't know if it's supposed to be 100 in other games too...
			a->forceClip = 100;
		else
			a->forceClip = 0;
		goto FixRooms;
	case 97:
		a->animSpeed = pop();
		a->animProgress = 0;
		break;
	case 98:
		a->shadow_mode = pop();
		break;
	case 99:
		a->new_1 = pop();
		a->new_2 = pop();
		break;
	case 198:										/* set anim variable */
		i = pop();									/* value */
		a->setAnimVar(pop(), i);
		break;
	case 215:
		a->new_3 = 1;
		break;
	case 216:
		a->new_3 = 0;
		break;
	case 217:
		a->initActor(2);
		break;
	case 227:										/* actor_layer */
		a->layer = pop();
		break;
	case 228:										/* walk script */
		a->walk_script = pop();
		break;
	case 235:										/* talk_script */
		a->talk_script = pop();
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
		a->moving |= 0x80;
		break;
	case 234:										/* unfreeze actor */
		a->moving &= ~0x7f;
		break;
	default:
		error("o6_actorset: default case %d", b);
	}
}

void Scumm::o6_verbOps()
{
	int slot, a, b;
	VerbSlot *vs;
	byte *ptr, op;

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
			ptr = (byte *)"";
		} else {
			ptr = getStringAddress(a);
		}
		loadPtrToResource(rtVerb, slot, ptr);
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
		error("o6_verbops: default case");
	}
}

void Scumm::o6_getActorFromXY()
{
	int y = pop();
	int x = pop();
	push(getActorFromPos(x, y));
}

void Scumm::o6_findObject()
{
	int y = pop();
	int x = pop();
	int r = findObject(x, y);
	push(r);
}

void Scumm::o6_pseudoRoom()
{
	int16 list[100];
	int num, a, value;

	num = getStackList(list, sizeof(list) / sizeof(list[0]));
	value = pop();

	while (--num >= 0) {
		a = list[num];
		if (a > 0x7F)
			_resourceMapper[a & 0x7F] = value;
	}
}

void Scumm::o6_getActorElevation()
{
	push(derefActorSafe(pop(), "o6_getActorElevation")->elevation);
}

void Scumm::o6_getVerbEntrypoint()
{
	int e = pop();
	int v = pop();
	push(getVerbEntrypoint(v, e));
}

void Scumm::o6_arrayOps()
{
	int a, b, c, d, num;
	int16 list[128];

	switch (fetchScriptByte()) {
	case 205:
		a = fetchScriptWord();
		pop();
		arrayop_1(a, NULL);
		break;
	case 208:
		a = fetchScriptWord();
		b = pop();
		c = pop();
		d = readVar(a);
		if (d == 0) {
			defineArray(a, 5, 0, b + c);
		}
		while (c--) {
			writeArray(a, 0, b + c, pop());
		}
		break;
	case 212:
		a = fetchScriptWord();
		b = pop();
		num = getStackList(list, sizeof(list) / sizeof(list[0]));
		d = readVar(a);
		if (d == 0)
			error("Must DIM a two dimensional array before assigning");
		c = pop();
		while (--num >= 0) {
			writeArray(a, c, b + num, list[num]);
		}
		break;
	default:
		error("o6_arrayOps: default case");
	}
}

void Scumm::o6_saveRestoreVerbs()
{
	int a, b, c;
	int slot, slot2;

	c = pop();
	b = pop();
	a = pop();

	switch (fetchScriptByte()) {
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

void Scumm::o6_drawBox()
{
	int x, y, x2, y2, color;
	color = pop();
	y2 = pop();
	x2 = pop();
	y = pop();
	x = pop();
	drawBox(x, y, x2, y2, color);
}

void Scumm::o6_getActorWidth()
{
	push(derefActorSafe(pop(), "o6_getActorWidth")->width);
}

void Scumm::o6_wait()
{
	switch (fetchScriptByte()) {
	case 168:{
			int offs = (int16)fetchScriptWord();
			if (derefActorSafe(pop(), "o6_wait")->moving) {
				_scriptPointer += offs;
				o6_breakHere();
			}
			return;
		}
	case 169:
		// HACK: For Conroy Bumpus' song in Sam & Max.
		// During the song three calls to o6_wait() appear,
		// but because of the way in which the actor talk is done
		// and the fact that the song extends beyond the span
		// of each individual line, no end-of-talk is ever
		// detected and the script hangs on o6_wait().
		if (_gameId == GID_SAMNMAX && vm.slot[_currentScript].number == 99 && _sound->isSoundRunning (64)) {
			debug (2, "Sam & Max hack: Bypassing o6_wait() during Conroy's song.");
			return;
		}

		if (_vars[VAR_HAVE_MSG])
			break;
		return;
	case 170:
		if (!(_features & GF_AFTER_V7)) {
			if (camera._cur.x >> 3 != camera._dest.x >> 3)
				break;
		} else {
			if (camera._dest.x != camera._cur.x || camera._dest.y != camera._cur.y)
				break;
		}

		return;
	case 171:
		printf("wait for sentence");
		if (_sentenceNum) {
			if (_sentence[_sentenceNum - 1].freezeCount && !isScriptInUse(_vars[VAR_SENTENCE_SCRIPT]))
				return;
			break;
		}
		if (!isScriptInUse(_vars[VAR_SENTENCE_SCRIPT]))
			return;
		break;
	case 226:{										/* wait until actor drawn */
			int actnum = pop();
			Actor *a = derefActorSafe(actnum, "o6_wait:226");
			int offs = (int16)fetchScriptWord();
			
			if (a && a->isInCurrentRoom() && a->needRedraw) {
				_scriptPointer += offs;
				o6_breakHere();
			}
			return;
		}
	case 232:{										/* wait until actor stops turning */
			Actor *a = derefActorSafe(pop(), "o6_wait:226");
			int offs = (int16)fetchScriptWord();
			if (a && a->isInCurrentRoom() && a->moving & MF_TURN) {
				_scriptPointer += offs;
				o6_breakHere();
			}
			return;
		}
	default:
		error("o6_wait: default case");
	}

	_scriptPointer -= 2;
	o6_breakHere();
}

void Scumm::o6_getActorScaleX()
{
	push(derefActorSafe(pop(), "o6_getActorScale")->scalex);
}

void Scumm::o6_getActorAnimCounter1()
{
	push(derefActorSafe(pop(), "o6_getActorAnimCounter")->cost.animCounter1);
}

void Scumm::o6_getAnimateVariable()
{
	int var = pop();
	push(derefActorSafe(pop(), "o6_getAnimateVariable")->getAnimVar(var));
}

void Scumm::o6_soundKludge()
{
	int16 list[16];
	getStackList(list, sizeof(list) / sizeof(list[0]));

	_sound->soundKludge(list);
}

void Scumm::o6_isAnyOf()
{
	int16 list[100];
	int num;
	int16 val;

	num = getStackList(list, sizeof(list) / sizeof(list[0]));
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

void Scumm::o6_quitPauseRestart()
{
	switch (fetchScriptByte()) {
	case 158:
		pauseGame(false);
		break;
	case 160:
		shutDown(0);
		break;
	default:
		error("o6_quitPauseRestart: invalid case");
	}
}

void Scumm::o6_isActorInBox()
{
	int box = pop();
	Actor *a = derefActorSafe(pop(), "o6_isActorInBox");
	push(checkXYInBoxBounds(box, a->x, a->y));
}

void Scumm::o6_delay()
{
	uint32 delay = (uint16)pop();
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = 1;
	o6_breakHere();
}

void Scumm::o6_delayLonger()
{
	uint32 delay = (uint16)pop() * 60;
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = 1;
	o6_breakHere();
}

void Scumm::o6_delayVeryLong()
{
	uint32 delay = (uint16)pop() * 3600;
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = 1;
	o6_breakHere();
}

void Scumm::o6_stopSentence()
{
	_sentenceNum = 0;
	stopScriptNr(_vars[VAR_SENTENCE_SCRIPT]);
	clearClickedStatus();
}

void Scumm::o6_print_0()
{
	_actorToPrintStrFor = 0xFF;
	decodeParseString2(0, 0);
}

void Scumm::o6_print_1()
{
	decodeParseString2(1, 0);
}

void Scumm::o6_print_2()
{
	decodeParseString2(2, 0);
}

void Scumm::o6_print_3()
{
	decodeParseString2(3, 0);
}

void Scumm::o6_printActor()
{
	decodeParseString2(0, 1);
}

void Scumm::o6_printEgo()
{
	push(_vars[VAR_EGO]);
	decodeParseString2(0, 1);
}

void Scumm::o6_talkActor()
{
	_actorToPrintStrFor = pop();
	_messagePtr = _scriptPointer;

	if ((_gameId == GID_DIG) && (_messagePtr[0] == '/')) {
		char pointer[20];
		int i, j;

		_scriptPointer += resStrLen((char*)_scriptPointer) + 1;
		translateText(_messagePtr, _transText);
		for (i = 0, j = 0; (_messagePtr[i] != '/' || j == 0) && j < 19; i++) {
			if (_messagePtr[i] != '/')
				pointer[j++] = _messagePtr[i];
		}
		pointer[j] = 0;

		// Stop any talking that's still going on
		if (_sound->_talkChannel > -1)
			_mixer->stop(_sound->_talkChannel);		

		_sound->_talkChannel = _sound->playBundleSound(pointer);
		_messagePtr = _transText;
		setStringVars(0);
		actorTalk();
	} else {
		setStringVars(0);
		actorTalk();
		_scriptPointer = _messagePtr;
	}
}

void Scumm::o6_talkEgo()
{
	_actorToPrintStrFor = (unsigned char)_vars[VAR_EGO];
	_messagePtr = _scriptPointer;

	if ((_gameId == GID_DIG) && (_messagePtr[0] == '/')) {
		char pointer[20];
		int i, j;

		_scriptPointer += resStrLen((char*)_scriptPointer) + 1;
		translateText(_messagePtr, _transText);
		for (i = 0, j = 0; (_messagePtr[i] != '/' || j == 0) && j < 19; i++) {
			if (_messagePtr[i] != '/')
				pointer[j++] = _messagePtr[i];
		}
		pointer[j] = 0;

		// Stop any talking that's still going on
		if (_sound->_talkChannel > -1)
			_mixer->stop(_sound->_talkChannel);

		_sound->_talkChannel = _sound->playBundleSound(pointer);
		_messagePtr = _transText;
		setStringVars(0);
		actorTalk();
	} else {
		setStringVars(0);
		actorTalk();
		_scriptPointer = _messagePtr;
	}
}

void Scumm::o6_dim()
{
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
		error("o6_dim	: default case");
	}

	defineArray(fetchScriptWord(), data, 0, pop());
}

void Scumm::o6_runVerbCodeQuick()
{
	int16 args[16];
	int script, entryp;
	getStackList(args, sizeof(args) / sizeof(args[0]));
	entryp = pop();
	script = pop();
	runVerbCode(script, entryp, 0, 1, args);
}

void Scumm::o6_runScriptQuick()
{
	int16 args[16];
	int script;
	getStackList(args, sizeof(args) / sizeof(args[0]));
	script = pop();
	runScript(script, 0, 1, args);
}

void Scumm::o6_dim2()
{
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

void Scumm::o6_abs()
{
	push(abs(pop()));
}


void Scumm::o6_distObjectObject()
{
	int a, b;
	b = pop();
	a = pop();
	push(getDistanceBetween(true, a, 0, true, b, 0));
}

void Scumm::o6_distObjectPt()
{
	int a, b, c;
	c = pop();
	b = pop();
	a = pop();
	push(getDistanceBetween(true, a, 0, false, b, c));
}

void Scumm::o6_distPtPt()
{
	int a, b, c, d;
	d = pop();
	c = pop();
	b = pop();
	a = pop();
	push(getDistanceBetween(false, a, b, false, c, d));
}

void Scumm::o6_dummy_stacklist()
{
	error("opcode o6_dummy_stacklist invalid");
}

void Scumm::o6_drawBlastObject()
{
	int16 args[16];
	int a, b, c, d, e;

	getStackList(args, sizeof(args) / sizeof(args[0]));
	e = pop();
	d = pop();
	c = pop();
	b = pop();
	a = pop();
	enqueueObject(a, b, c, d, e, 0xFF, 0xFF, 1, 0);
}

void Scumm::o6_miscOps()
{
	int16 args[30];
	int i;
	Actor *a;

	getStackList(args, sizeof(args) / sizeof(args[0]));

	if (_features & GF_AFTER_V7) {
		switch (args[0]) {
		case 4:
			grabCursor(args[1], args[2], args[3], args[4]);
			break;
		case 6: {
//				if (_imuseDigital) {
//					_imuseDigital->stopAll();
//				}
				uint32 speed;
				if (strcmp((char*)getStringAddressVar(VAR_VIDEONAME), "sq3.san") == 0)
	 				speed = 71;
				else
					speed = 1000 / _insaneFlag;

				debug(1, "INSANE Arg: %d", args[1]);

				ScummRenderer * sr = new ScummRenderer(this, speed);
 				SmushPlayer * sp = new SmushPlayer(sr);

				// INSANE mode 0: SMUSH movie playback
				if (args[1] == 0) {
 					sp->play((char*)getStringAddressVar(VAR_VIDEONAME), getGameDataPath());
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
							InfoDialog* dialog = new InfoDialog(_newgui, this,
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
// 						sp->play("minefite.san", getGameDataPath());
					 }
						break;
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
					 default:
						// Other INSANE modes
						warning("Unknown FT_INSANE mode for %d", args[1]);
 						sp->play((char*)getStringAddressVar(VAR_VIDEONAME), getGameDataPath());
					 }
				} else {
					// Other INSANE modes
					warning("Unknown insane mode for %d", args[1]);
 					sp->play((char*)getStringAddressVar(VAR_VIDEONAME), getGameDataPath());
				}
				delete sp;
				delete sr;
 			}
			break;
		case 7:
			warning("o6_miscOps: stub7()");
			break;
		case 10:
			warning("o6_miscOps: stub10(%d,%d,%d,%d)", args[1], args[2], args[3], args[4]);
			break;
		case 11:
			warning("o6_miscOps: stub11(%d)", args[1]);
			break;
		case 12:
			setCursorImg(args[1], (uint) - 1, args[2]);
			break;
		case 13:
			derefActorSafe(args[1], "o6_miscOps:14")->remapActorPalette(args[2], args[3], args[4], -1);
			break;
		case 14:
			derefActorSafe(args[1], "o6_miscOps:14")->remapActorPalette(args[2], args[3], args[4], args[5]);
			break;
		case 15:
			_insaneFlag = args[1];
			break;
		case 16:
			if (_gameId == GID_DIG) {
				byte buf[200];
				_msgPtrToAdd = buf;
				setStringVars(0);
				addMessageToStack(getStringAddressVar(VAR_STRING2DRAW));
				if (strncmp("/SYSTEM.007/ /", (char *)buf, 14) == 0) {
					translateText(buf + 13, charset._buffer);
					//description();
				}	else if (strncmp("/SYSTEM.007/ ", (char *)buf, 13) == 0) {
					strcpy((char *)charset._buffer, (char *)buf + 13);
					//description();
				}
			} else { 
				setStringVars(0);

				_string[0].charset = (byte)args[1];
				_string[0].color = (byte)args[2];
				_string[0].xpos = args[3];
				_string[0].ypos = args[4];
				drawDescString(getStringAddressVar(VAR_STRING2DRAW));
			}
			break;
		case 17:
			warning("o6_miscOps: stub17(%d,%d,%d,%d)", args[1], args[2], args[3], args[4]);
			break;
		case 18:
			warning("o6_miscOps: stub18(%d,%d)", args[1], args[2]);
			break;
		case 107:
			a = derefActorSafe(args[1], "o6_miscops: 107");
			a->scalex = (unsigned char)args[2];
			a->needBgReset = true;
			a->needRedraw = true;
			break;
		case 108:
			setupShadowPalette(args[1], args[2], args[3], args[4], args[5], args[6]);
			break;
		case 109:
			setupShadowPalette(0, args[1], args[2], args[3], args[4], args[5]);
			break;
		case 114:
			warning("o6_miscOps: stub114()");
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
		case 120:
			warning("o6_miscOps: stub120(%d,%d)", args[1], args[2]);
			break;
		case 124:
			warning("o6_miscOps: stub124(%d)", args[1]);
			break;
		}
	} else {
		switch (args[0]) {
		case 3:
			warning("o6_miscOps: nothing in 3");
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
			for (i = 0; i < NUM_ACTORS; i++)
				derefActor(i)->needRedraw = true;
			processActors();
			fadeIn(args[1]);
			break;
		case 8:
			startManiac();
			break;
		case 9:
			unkMiscOp9();
			break;

		case 104:									/* samnmax */
			nukeFlObjects(args[2], args[3]);
			break;

		case 106:
			error("stub o6_miscOps_106()");
			break;

		case 107:									/* set actor scale */
			a = derefActorSafe(args[1], "o6_miscops: 107");
			a->scalex = (unsigned char)args[2];
			a->needBgReset = true;
			a->needRedraw = true;
			break;

		case 108:									/* create proc_special_palette */
		case 109:

			createSpecialPalette(args[1], args[2], args[3], args[4], args[5], 0, 256);
			break;

		case 110:
			gdi.clearUpperMask();
			break;

		case 111:
			a = derefActorSafe(args[1], "o6_miscops: 111");
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
				warning("stub o6_miscOps_114()");
			break;

		case 117:
			warning("stub o6_miscOps_117()");
			break;

		case 118:
			error("stub o6_miscOps_118(%d,%d,%d,%d,%d,%d,%d)",
						args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
			break;

		case 119:
			enqueueObject(args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], 0);
			break;

		case 120:
			swapPalColors(args[1], args[2]);
			break;

		case 121:
			error("stub o6_miscOps_121(%d)", args[1]);
			break;

		case 122:

			_vars[VAR_SOUNDRESULT] =
				(short)_imuse->do_command(args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]
				);
			break;

		case 123:
			copyPalColor(args[2], args[1]);
			break;

		case 124:									/* samnmax */
			warning("o6_miscOps: _saveSound=%d", args[1]);
			_saveSound = args[1];
			break;

		default:
			error("o6_miscOps: default case %d", args[0]);
		}
	}
}

void Scumm::o6_kernelFunction()
{
	int16 args[30];
	int i;
	Actor *a;

	getStackList(args, sizeof(args) / sizeof(args[0]));

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
		   27 = abord
		   97 = left
		   331 = left
		   115 = right
		   333 = tight
		 */

		//FIXME: this is hack to allow turn on light bridge, 
		//How to check is left button pressed ?
		if ((_gameId == GID_DIG) && (args[1] == 13)) {
			push(1);
			break;
		}

		warning("o6_kernelFunction: getInput(%d) against %d", args[1], _lastKeyHit);
		push(0);
		break;
	case 212:
		a = derefActorSafe(args[1], "o6_kernelFunction:212");
		// FIXME - frame is never set and thus always 0! See actor.h comment
		push(a->frame);
		break;
	case 215:
		if ((_extraBoxFlags[args[1]] & 0x00FF) == 0x00C0) {
			push(_extraBoxFlags[args[1]]);
		} else {
			byte *temp = (byte *)getBoxBaseAddr(args[1]);
			push((byte)(*(temp + 17)));
		}
		break;
	default:
		error("o6_kernelFunction: default case %d", args[0]);
	}
}

void Scumm::o6_delayFrames()
{
	ScriptSlot *ss = &vm.slot[_currentScript];
	if (ss->delayFrameCount== 0) {
		ss->delayFrameCount = pop();
	} else {
		ss->delayFrameCount--;
	}
	if (ss->delayFrameCount) {
		_scriptPointer--;
		o6_breakHere();
	}
}

void Scumm::o6_pickOneOf()
{
	int16 args[100];
	int i, num;

	num = getStackList(args, sizeof(args) / sizeof(args[0]));
	i = pop();
	if (i < 0 || i >= num)
		error("o6_pickOneOf: out of range");
	push(args[i]);
}

void Scumm::o6_pickOneOfDefault()
{
	int16 args[100];
	int i, num, def;

	def = pop();
	num = getStackList(args, sizeof(args) / sizeof(args[0]));
	i = pop();
	if (i < 0 || i >= num)
		i = def;
	else
		i = args[i];
	push(i);
}

void Scumm::o6_getActorPriority()
{
	Actor *a;

	a = derefActorSafe(pop(), "getActorPriority");

	push(a->layer);
}

void Scumm::o6_unknownCD() {
	// Full Throttle: FIXME
	//		  Opcode is used when placing the bunny into the
	//		  mine field. Some kind of drawBlastObject() wrapper
	int a, b, c, d;
	a = pop();
	b = pop();
	c = pop();
	d = pop();
	warning("o6_unknownCD: stub(%d, %d, %d, %d)", a, b, c, d);
}

void Scumm::decodeParseString2(int m, int n)
{
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
		error("decodeParseString2: case 73");
		break;
	case 74:
		_string[m].no_talk_anim = true;
		break;
	case 75:{
			_messagePtr = _scriptPointer;

			if ((_messagePtr[0] == '/') && (_gameId == GID_DIG)) {
				char pointer[20];
				int i, j;

				_scriptPointer += resStrLen((char*)_scriptPointer)+ 1;
				translateText(_messagePtr, _transText);
				for (i = 0, j = 0; (_messagePtr[i] != '/' || j == 0) && j < 19; i++) {
				if (_messagePtr[i] != '/')
					pointer[j++] = _messagePtr[i];
				}
				pointer[j] = 0;

				// Stop any talking that's still going on
				if (_sound->_talkChannel > -1)
					_mixer->stop(_sound->_talkChannel);

				_sound->_talkChannel = _sound->playBundleSound(pointer);
				_messagePtr = _transText;

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
			} else {
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
				_scriptPointer = _messagePtr;
				return;
			}
		}
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
		error("decodeParseString: default case");
	}
}

void Scumm::setupShadowPalette(int slot, int rfact, int gfact, int bfact, int from, int to)
{
	byte *table;
	int i, num;
	byte *curpal;

	if (slot < 0 || slot > 7)
		error("setupShadowPalette: invalid slot %d", slot);

	if (from < 0 || from > 255 || to < 0 || from > 255 || to < from)
		error("setupShadowPalette: invalid range from %d to %d", from, to);

	table = _shadowPalette + slot * 256;
	for (i = 0; i < 256; i++)
		table[i] = i;

	table += from;
	curpal = _currentPalette + from * 3;
	num = to - from + 1;
	do {
		*table++ = remapPaletteColor((curpal[0] * rfact) >> 8,
																 curpal[1] * gfact >> 8, curpal[2] * bfact >> 8, (uint) - 1);
		curpal += 3;
	} while (--num);
}
