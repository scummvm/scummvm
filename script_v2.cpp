/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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
 * Change Log:
 * $Log$
 * Revision 1.1  2001/10/16 10:01:47  strigeus
 * preliminary DOTT support
 *
 */


#include "stdafx.h"
#include "scumm.h"

#if defined(DOTT)

void Scumm::setupOpcodes2() {
	static const OpcodeProc opcode_list[256] = {
	/* 00 */
	&Scumm::o2_pushByte,
	&Scumm::o2_pushWord,
	&Scumm::o2_pushByteVar,
	&Scumm::o2_pushWordVar,
	/* 04 */
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_byteArrayRead,
	&Scumm::o2_wordArrayRead,
	/* 08 */
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_byteArrayIndexedRead,
	&Scumm::o2_wordArrayIndexedRead,
	/* 0C */
	&Scumm::o2_dup,
	&Scumm::o2_zero,
	&Scumm::o2_eq,
	&Scumm::o2_neq,
	/* 10 */
	&Scumm::o2_gt,
	&Scumm::o2_lt,
	&Scumm::o2_le,
	&Scumm::o2_ge,
	/* 14 */
	&Scumm::o2_add,
	&Scumm::o2_sub,
	&Scumm::o2_mul,
	&Scumm::o2_div,
	/* 18 */
	&Scumm::o2_land,
	&Scumm::o2_lor,
	&Scumm::o2_kill,
	&Scumm::o2_invalid,
	/* 1C */
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* 20 */
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* 24 */
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* 28 */
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* 2C */
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* 30 */
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* 34 */
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* 38 */
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* 3C */
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* 40 */
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_writeByteVar,
	&Scumm::o2_writeWordVar,
	/* 44 */
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_byteArrayWrite,
	&Scumm::o2_wordArrayWrite,
	/* 48 */
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_byteArrayIndexedWrite,
	&Scumm::o2_wordArrayIndexedWrite,
	/* 4C */
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_byteVarInc,
	&Scumm::o2_wordVarInc,
	/* 50 */
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_byteArrayInc,
	&Scumm::o2_wordArrayInc,
	/* 54 */
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_byteVarDec,
	&Scumm::o2_wordVarDec,
	/* 58 */
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_byteArrayDec,
	&Scumm::o2_wordArrayDec,
	/* 5C */
	&Scumm::o2_jumpTrue,
	&Scumm::o2_jumpFalse,
	&Scumm::o2_startScriptEx,
	&Scumm::o2_startScript,
	/* 60 */
	&Scumm::o2_startObject,
	&Scumm::o2_setObjectState,
	&Scumm::o2_setObjectXY,
	&Scumm::o2_invalid,
	/* 64 */
	&Scumm::o2_invalid,
	&Scumm::o2_stopObjectCode,
	&Scumm::o2_stopObjectCode,
	&Scumm::o2_endCutscene,
	/* 68 */
	&Scumm::o2_cutScene,
	&Scumm::o2_stopMusic,
	&Scumm::o2_freezeUnfreeze,
	&Scumm::o2_cursorCommand,
	/* 6C */
	&Scumm::o2_breakHere,
	&Scumm::o2_ifClassOfIs,
	&Scumm::o2_setClass,
	&Scumm::o2_getState,
	/* 70 */
	&Scumm::o2_setState,
	&Scumm::o2_setOwner,
	&Scumm::o2_getOwner,
	&Scumm::o2_jump,
	/* 74 */
	&Scumm::o2_startSound,
	&Scumm::o2_stopSound,
	&Scumm::o2_startMusic,
	&Scumm::o2_stopObjectScript,
	/* 78 */
	&Scumm::o2_panCameraTo,
	&Scumm::o2_actorFollowCamera,
	&Scumm::o2_setCameraAt,
	&Scumm::o2_loadRoom,
	/* 7C */
	&Scumm::o2_stopScript,
	&Scumm::o2_walkActorToObj,
	&Scumm::o2_walkActorTo,
	&Scumm::o2_putActorInRoom,
	/* 80 */
	&Scumm::o2_putActorAtObject,
	&Scumm::o2_faceActor,
	&Scumm::o2_animateActor,
	&Scumm::o2_doSentence,
	/* 84 */
	&Scumm::o2_pickupObject,
	&Scumm::o2_loadRoomWithEgo,
	&Scumm::o2_invalid,
	&Scumm::o2_getRandomNumber,
	/* 88 */
	&Scumm::o2_getRandomNumberRange,
	&Scumm::o2_invalid,
	&Scumm::o2_getActorMoving,
	&Scumm::o2_getScriptRunning,
	/* 8C */
	&Scumm::o2_getActorRoom,
	&Scumm::o2_getObjectX,
	&Scumm::o2_getObjectY,
	&Scumm::o2_getObjectDir,
	/* 90 */
	&Scumm::o2_getActorWalkBox,
	&Scumm::o2_getActorCostume,
	&Scumm::o2_findInventory,
	&Scumm::o2_getInventoryCount,
	/* 94 */
	&Scumm::o2_getVerbFromXY,
	&Scumm::o2_beginOverride,
	&Scumm::o2_endOverride,
	&Scumm::o2_setObjectName,
	/* 98 */
	&Scumm::o2_isSoundRunning,
	&Scumm::o2_setBoxFlags,
	&Scumm::o2_createBoxMatrix,
	&Scumm::o2_resourceRoutines,
	/* 9C */
	&Scumm::o2_roomOps,
	&Scumm::o2_actorSet,
	&Scumm::o2_verbOps,
	&Scumm::o2_getActorFromXY,
	/* A0 */
	&Scumm::o2_findObject,
	&Scumm::o2_pseudoRoom,
	&Scumm::o2_getActorElevation,
	&Scumm::o2_getVerbEntrypoint,
	/* A4 */
	&Scumm::o2_arrayOps,
	&Scumm::o2_saveRestoreVerbs,
	&Scumm::o2_drawBox,
	&Scumm::o2_invalid,
	/* A8 */
	&Scumm::o2_getActorWidth,
	&Scumm::o2_wait,
	&Scumm::o2_getActorScaleX,
	&Scumm::o2_getActorAnimCounter1,
	/* AC */
	&Scumm::o2_soundKludge,
	&Scumm::o2_isAnyOf,
	&Scumm::o2_quitPauseRestart,
	&Scumm::o2_isActorInBox,
	/* B0 */
	&Scumm::o2_delay,
	&Scumm::o2_delayLonger,
	&Scumm::o2_delayVeryLong,
	&Scumm::o2_stopSentence,
	/* B4 */
	&Scumm::o2_print_0,
	&Scumm::o2_print_1,
	&Scumm::o2_print_2,
	&Scumm::o2_print_3,
	/* B8 */
	&Scumm::o2_printActor,
	&Scumm::o2_printEgo,
	&Scumm::o2_talkActor,
	&Scumm::o2_talkEgo,
	/* BC */
	&Scumm::o2_dim,
	&Scumm::o2_invalid,
	&Scumm::o2_runVerbCodeQuick,
	&Scumm::o2_runScriptQuick,
	/* C0 */
	&Scumm::o2_dim2,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* C4 */
	&Scumm::o2_abs,
	&Scumm::o2_distObjectObject,
	&Scumm::o2_distObjectPt,
	&Scumm::o2_distPtPt,
	/* C8 */
	&Scumm::o2_invalid,
	&Scumm::o2_miscOps,
	&Scumm::o2_breakMaybe,
	&Scumm::o2_pickOneOf,
	/* CC */
	&Scumm::o2_pickOneOfDefault,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* D0 */	
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* D4 */	
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* D8 */	
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* DC */	
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* E0 */	
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* E4 */	
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* E8 */	
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* EC */	
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* F0 */	
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* F4 */	
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* F8 */	
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	/* FC */	
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	&Scumm::o2_invalid,
	};

	_opcodes = opcode_list;
}

int Scumm::readArray(int array, int index, int base) {
	ArrayHeader *ah = (ArrayHeader*)getResourceAddress(7, readVar(array));

	assert(ah);

	base += index*ah->dim1_size;

	assert(base>=0 && base < ah->dim1_size*ah->dim2_size);

	if (ah->type==4) {
		return ah->data[base];
	} else {
		return (int16)READ_LE_UINT16(ah->data + base*2);
	}
}

void Scumm::writeArray(int array, int index, int base, int value) {
	ArrayHeader *ah = (ArrayHeader*)getResourceAddress(7, readVar(array));
	assert(ah);
	base += index*ah->dim1_size;

	assert(base>=0 && base < ah->dim1_size*ah->dim2_size);

	if (ah->type==4) {
		ah->data[base] = value;
	} else {
		((uint16*)ah->data)[base] = TO_LE_16(value);
	}
}

int Scumm::getStackList(int16 *args, uint maxnum) {
	uint num, i;

	for (i=0; i<maxnum; i++)
		args[i] = 0;

	num = pop();

	if (num > maxnum)
		error("Too many items %d in stack list, max %d", num, maxnum);

	i = num;
	while (((int)--i)>=0) {
		args[i] = pop();
	}

	return num;
}

void Scumm::o2_pushByte() {
	push(fetchScriptByte());
}

void Scumm::o2_pushWord() {
	push((int16)fetchScriptWord());
}

void Scumm::o2_pushByteVar() {
	push(readVar(fetchScriptByte()));
}

void Scumm::o2_pushWordVar() {
	push(readVar(fetchScriptWord()));
}

void Scumm::o2_invalid() {
	error("Invalid opcode '%x' at %x", _opcode, _scriptPointer - _scriptOrgPointer);
}

void Scumm::o2_byteArrayRead() {
	int base = pop();
	push(readArray(fetchScriptByte(), 0, base));
}

void Scumm::o2_wordArrayRead() {
	int base = pop();
	push(readArray(fetchScriptWord(), 0, base));
}

void Scumm::o2_byteArrayIndexedRead() {
	int base = pop();
	int index = pop();
	push(readArray(fetchScriptByte(), index, base));
}

void Scumm::o2_wordArrayIndexedRead() {
	int base = pop();
	int index = pop();
	push(readArray(fetchScriptWord(), index, base));
}

void Scumm::o2_dup() {
	int a = pop();
	push(a);
	push(a);
}

void Scumm::o2_zero() {
	push( pop() == 0 );
}

void Scumm::o2_eq() {
	push( pop() == pop() );
}

void Scumm::o2_neq() {
	push( pop() != pop() );
}

void Scumm::o2_gt() {
	int a = pop();
	push( pop() > a );
}

void Scumm::o2_lt() {
	int a = pop();
	push( pop() < a );
}

void Scumm::o2_le() {
	int a = pop();
	push( pop() <= a );
}

void Scumm::o2_ge() {
	int a = pop();
	push( pop() >= a );
}

void Scumm::o2_add() {
	int a = pop();
	push( pop() + a );
}

void Scumm::o2_sub() {
	int a = pop();
	push( pop() - a );
}

void Scumm::o2_mul() {
	int a = pop();
	push( pop() * a );
}

void Scumm::o2_div() {
	int a = pop();
	if (a==0) error("division by zero");
	push( pop() / a );
}

void Scumm::o2_land() {
	int a = pop();
	push( pop() && a );
}

void Scumm::o2_lor() {
	int a = pop();
	push( pop() || a );
}

void Scumm::o2_kill() {
	pop();
}

void Scumm::o2_writeByteVar() {
	writeVar(fetchScriptByte(), pop());
}

void Scumm::o2_writeWordVar() {
	writeVar(fetchScriptWord(), pop());
}

void Scumm::o2_byteArrayWrite() {
	int a = pop();
	writeArray(fetchScriptByte(), 0, pop(), a);
}

void Scumm::o2_wordArrayWrite() {
	int a = pop();
	writeArray(fetchScriptWord(), 0, pop(), a);
}

void Scumm::o2_byteArrayIndexedWrite() {
	int val = pop();
	int base = pop();
	writeArray(fetchScriptByte(), pop(), base, val);
}

void Scumm::o2_wordArrayIndexedWrite() {
	int val = pop();
	int base = pop();
	writeArray(fetchScriptWord(), pop(), base, val);
}

void Scumm::o2_byteVarInc() {
	int var = fetchScriptByte();
	writeVar(var,readVar(var)+1);
}

void Scumm::o2_wordVarInc() {
	int var = fetchScriptWord();
	writeVar(var,readVar(var)+1);
}

void Scumm::o2_byteArrayInc() {
	int var = fetchScriptByte();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) + 1);
}

void Scumm::o2_wordArrayInc() {
	int var = fetchScriptWord();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) + 1);
}


void Scumm::o2_byteVarDec() {
	int var = fetchScriptByte();
	writeVar(var,readVar(var)-1);
}

void Scumm::o2_wordVarDec() {
	int var = fetchScriptWord();
	writeVar(var,readVar(var)-1);
}

void Scumm::o2_byteArrayDec() {
	int var = fetchScriptByte();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) - 1);
}

void Scumm::o2_wordArrayDec() {
	int var = fetchScriptWord();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) - 1);
}

void Scumm::o2_jumpTrue() {
	if (pop())
		o2_jump();
	else
		fetchScriptWord();
}

void Scumm::o2_jumpFalse() {
	if (!pop())
		o2_jump();
	else
		fetchScriptWord();
}

void Scumm::o2_jump() {
	_scriptPointer += (int16)fetchScriptWord();
}

void Scumm::o2_startScriptEx() {
	int16 args[16];
	int script,flags;

	getStackList(args,sizeof(args)/sizeof(args[0]));
	script = pop();
	flags = pop();
	runScript(script, flags&1, flags&2, args);
}

void Scumm::o2_startScript() {
	int16 args[16];
	int script;
	getStackList(args,sizeof(args)/sizeof(args[0]));
	script = pop();
	runScript(script, 0, 0, args);
}

void Scumm::o2_startObject() {
	int16 args[16];
	int script,entryp;
	int flags;
	getStackList(args,sizeof(args)/sizeof(args[0]));
	entryp = pop();
	script = pop();
	flags = pop();
	runVerbCode(script, entryp, flags&1, flags&2, args);
}

void Scumm::o2_setObjectState() {
	int a = pop();
	if (a==0) a=1;
	setObjectState(pop(), a, -1, -1);
}

void Scumm::o2_setObjectXY() {
	int y = pop();
	int x = pop();
	setObjectState(pop(), 1, x, y);
}

void Scumm::o2_stopObjectCode() {
	stopObjectCode();
}

void Scumm::o2_endCutscene() {
	endCutscene();
}

void Scumm::o2_cutScene() {
	int16 args[16];
	getStackList(args,sizeof(args)/sizeof(args[0]));
	cutscene(args);
}

void Scumm::o2_stopMusic() {
	warning("o2_stopMusic: not implemented");
}

void Scumm::o2_freezeUnfreeze() {
	int a = pop();
	if (a)
		freezeScripts(a);
	else
		unfreezeScripts();
}

void Scumm::o2_cursorCommand() {
	int a,num,i;
	int16 args[16];

	switch(fetchScriptByte()) {
	case 0x90:
		_cursorState = 1;
		verbMouseOver(0);
		break;
	case 0x91:
		_cursorState = 0;
		verbMouseOver(0);
		break;
	case 0x92:
		_userPut = 1;
		break;
	case 0x93:
		_userPut = 0;
		break;
	case 0x94:
		_cursorState++;
		if (_cursorState > 1)
			error("Cursor state greater than 1 in script");
		verbMouseOver(0);
		break;
	case 0x95:
		_cursorState--;
		verbMouseOver(0);
		break;
	case 0x96:
		_userPut++;
		break;
	case 0x97:
		_userPut--;
		break;
	case 0x99:
		a = pop();
		setCursorImg(a, pop());
		break;
	case 0x9A:
		a = pop();
		setCursorHotspot2(pop(),a);
		break;
	case 0x9C: /* init charset */
		initCharset(pop());
		break;
	case 0x9D: /* set charset colors */
		getStackList(args,sizeof(args)/sizeof(args[0]));
		for (i=0; i<16; i++)
			charset._colorMap[i] = _charsetData[string[1].t_charset][i] = args[i];
		break;
	case 0xD6:
		new_unk_1(pop());
		break;
	}

	_vars[VAR_CURSORSTATE] = _cursorState;
	_vars[VAR_USERPUT] = _userPut;
}

void Scumm::o2_breakHere() {
	updateScriptPtr();
	_currentScript = 0xFF;
}

void Scumm::o2_ifClassOfIs() {
	int16 args[16];
	int num,obj,cls;
	bool b;
	int cond = 1;

	num = getStackList(args,sizeof(args)/sizeof(args[0]));
	obj = pop();

	while (--num>=0) {
		cls = args[num];
		b = getClass(obj, cls);
		if (cls&0x80 && !b || !(cls&0x80) && b)
			cond = 0;
	}
	push(cond);
}

void Scumm::o2_setClass() {
	int16 args[16];
	int num,obj,cls;

	num = getStackList(args,sizeof(args)/sizeof(args[0]));
	obj = pop();

	while (--num>=0) {
		cls = args[num];
		if (cls==0)
			_classData[num] = 0;
		else if (cls&0x80)
			putClass(obj, cls, 1);
		else
			putClass(obj, cls, 0);
	}
}

void Scumm::o2_getState() {
	push(getState(pop()));
}

void Scumm::o2_setState() {
	int state = pop();
	int obj = pop();

	putState(obj, state);
	removeObjectFromRoom(obj);
	if (_BgNeedsRedraw)
		clearDrawObjectQueue();
}

void Scumm::o2_setOwner() {
	int owner = pop();
	int obj = pop();

	setOwnerOf(obj, owner);
}

void Scumm::o2_getOwner() {
	push(getOwner(pop()));
}

void Scumm::o2_startSound() {
	addSoundToQueue(pop());
}

void Scumm::o2_stopSound() {
	unkSoundProc1(pop());
}

void Scumm::o2_startMusic() {
	addSoundToQueue(pop());
}

void Scumm::o2_stopObjectScript() {
	stopObjectScript(pop());
}

void Scumm::o2_panCameraTo() {
	panCameraTo(pop());
}

void Scumm::o2_actorFollowCamera() {
	actorFollowCamera(pop());
}

void Scumm::o2_setCameraAt() {
	setCameraAtEx(pop());
}

void Scumm::o2_loadRoom() {
	int room = pop();
	debug(1,"Loading room %d", room);
	startScene(room, 0, 0);
	_fullRedraw = 1;
}

void Scumm::o2_stopScript() {
	int script = pop();
	if (script==0)
		stopObjectCode();
	else
		stopScriptNr(script);
}

void Scumm::o2_walkActorToObj() {
	int obj,dist;
	Actor *a, *a2;
	int x;

	dist = pop();
	obj = pop();
	a = derefActorSafe(pop(), "o2_walkActorToObj");

	if (obj >= 17) {
		if (whereIsObject(obj)==-1)
			return;
		getObjectXYPos(obj);
		startWalkActor(a, _xPos, _yPos, _dir);
	} else {
		a2 = derefActorSafe(obj, "o2_walkActorToObj(2)");
		if (a2->room != _currentRoom ||
			  a->room != _currentRoom)
					return;
		if (dist==0) {
			dist = a2->scalex * a2->width / 0xFF;
			dist += dist>>1;
		}
		x = a2->x;
		if (x < a->x)
			x += dist;
		else
			x -= dist;
		startWalkActor(a, x, a2->y, 0xFF);
	}
}

void Scumm::o2_walkActorTo() {
	int x,y;
	y = pop();
	x = pop();
	startWalkActor(derefActorSafe(pop(), "o2_walkActorTo"), x, y, 0xFF);
}

void Scumm::o2_putActorInRoom() {
	int room, x, y;
	Actor *a;

	room = pop();
	y = pop();
	x = pop();
	a = derefActorSafe(pop(), "o2_putActorInRoom");
	if (room==0xFF) {
		room = a->room;
	} else {
		if (a->visible && _currentRoom != room && _vars[VAR_TALK_ACTOR]==a->number) {
			clearMsgQueue();
		}
		if (room != 0)
			a->room = room;
	}
	putActor(a, x, y, room);
}

void Scumm::o2_putActorAtObject() {
	int room,obj,x,y;
	Actor *a;

	room = pop();
	obj = pop();

	a = derefActorSafe(pop(), "o2_putActorAtObject");
	if (whereIsObject(obj)!=-1) {
		getObjectXYPos(obj);
		x = _xPos;
		y = _yPos;
	} else {
		x = 160;
		y = 120;
	}
	if (room == 0xFF)
		room = a->room;
	putActor(a, x, y, room);
}

void Scumm::o2_faceActor() {
	int act,obj;
	obj = pop();
	act = pop();
	faceActorToObj(act, obj);
}

void Scumm::o2_animateActor() {
	int anim = pop();
	int act = pop();
	animateActor(act, anim);
}

void Scumm::o2_doSentence() {
	int a,b,c;
	SentenceTab *st;

	a = pop();
	pop();  //dummy pop
	b = pop();
	c = pop();

	st = &sentence[++_sentenceIndex];

	st->unk5 = c;
	st->unk4 = b;
	st->unk3 = a;

	if (!(st->unk3&0xFF00))
		st->unk2 = 0;
	else
		st->unk2 = 1;

	st->unk = 0;
}

void Scumm::o2_pickupObject() {
	int obj, room;

	room = pop();
	obj = pop();
	
	if (room==0)
		room = _roomResource;
	addObjectToInventory(obj, room);
	putOwner(obj, _vars[VAR_UNK_ACTOR]);
	putClass(obj, 32, 1);
	putState(obj, 1);
	removeObjectFromRoom(obj);
	clearDrawObjectQueue();
	runHook(obj); /* Difference */
}

void Scumm::o2_loadRoomWithEgo() {
	Actor *a;
	int room,obj,x,y;

	y = pop();
	x = pop();
	room = pop();
	obj = pop();

	a = derefActorSafe(_vars[VAR_UNK_ACTOR], "o_loadRoomWithEgo");

	putActor(a, 0, 0, room);
	dseg_3A76 = 0;
	_vars[VAR_WALKTO_OBJ] = obj;
	startScene(a->room, a, obj);
	_vars[VAR_WALKTO_OBJ] = 0;

	/* startScene maybe modifies VAR_UNK_ACTOR, i hope not */
	camera._destPos = camera._curPos = a->x;
	setCameraFollows(a);
	_fullRedraw=1;
	if (x != -1) {
		startWalkActor(a, x, y, 0xFF);
	}
}

void Scumm::o2_getRandomNumber() {
	int rnd;
	rnd = getRandomNumber(pop()+1);
	_vars[VAR_RANDOM_NR] = rnd;
	push(rnd);
}

void Scumm::o2_getRandomNumberRange() {
	int max = pop();
	int min = pop();
	int rnd = getRandomNumber(max-min+1) + min;
	_vars[VAR_RANDOM_NR] = rnd;
	push(rnd);
}

void Scumm::o2_getActorMoving() {
	push(derefActorSafe(pop(),"o2_getActorMoving")->moving);
}

void Scumm::o2_getScriptRunning() {
	push(getScriptRunning(pop()));
}

void Scumm::o2_getActorRoom() {
	push(derefActorSafe(pop(),"o2_getActorRoom")->room);
}

void Scumm::o2_getObjectX() {
	push(getObjX(pop()));
}

void Scumm::o2_getObjectY() {
	push(getObjY(pop()));
}

void Scumm::o2_getObjectDir() {
	push(getObjDir(pop()));
}

void Scumm::o2_getActorWalkBox() {
	push(derefActorSafe(pop(),"o2_getActorWalkBox")->walkbox);
}

void Scumm::o2_getActorCostume() {
	push(derefActorSafe(pop(),"o2_getActorCostume")->costume);
}

void Scumm::o2_findInventory() {
	int index = pop();
	int owner = pop();
	push(findInventory(owner,index));
}

void Scumm::o2_getInventoryCount() {
	push(getInventoryCount(pop()));
}

void Scumm::o2_getVerbFromXY() {
	int y = pop();
	int x = pop();
	int over = checkMouseOver(x,y);
	if (over)
		over = _verbs[over].verbid;
	push(over);
}

void Scumm::o2_beginOverride() {
	beginOverride();
}

void Scumm::o2_endOverride() {
	endOverride();
}

void Scumm::o2_setObjectName() {
	int obj = pop();
	int i;

	if (obj <= _vars[VAR_NUM_ACTOR])
		error("Can't set actor %d name with new-name-of", obj);

	if (!getObjectAddress(obj))
		error("Can't set name of object %d", obj);

	for (i=1; i<50; i++) {
		if (_newNames[i] == obj) {
			nukeResource(16, i);
			_newNames[i] = 0;
			break;
		}
	}

	for (i=1; i<50; i++) {
		if (_newNames[i] == 0) {
			loadPtrToResource(16, i, NULL);
			_newNames[i] = obj;
			runHook(0);
			return;
		}
	}

	error("New name of %d overflows name table (max = %d)", obj, 50);
}

void Scumm::o2_isSoundRunning() {
	int snd = pop();
	if (snd)
		snd = unkSoundProc23(snd);
	push(snd);
}

void Scumm::o2_setBoxFlags() {
	int16 table[65];
	int num,value;

	value = pop();
	num = getStackList(table,sizeof(table)/sizeof(table[0]));

	while (--num>=0) {
		setBoxFlags(table[num], value);
	}
}

void Scumm::o2_createBoxMatrix() {
	createBoxMatrix();
}

void Scumm::o2_resourceRoutines() {
	int res;

	switch(fetchScriptByte()) {
	case 100: /* load script */
		res = pop();
		ensureResourceLoaded(2, res);
		break;
	case 101: /* load sound */
		res = pop();
		ensureResourceLoaded(4, res);
		break;
	case 102: /* load costume */
		res = pop();
		ensureResourceLoaded(3, res);
		break;
	case 103: /* load room */
		res = pop();
		ensureResourceLoaded(1, res);
		break;
	case 104: /* nuke script */
		res = pop();
		setResourceFlags(2, res, 0x7F);
		break;
	case 105: /* nuke sound */
		res = pop();
		setResourceFlags(4, res, 0x7F);
		break;
	case 106: /* nuke costume */
		res = pop();
		setResourceFlags(3, res, 0x7F);
		break;
	case 107: /* nuke room */
		res = pop();
		setResourceFlags(1, res, 0x7F);
		break;
	case 108:  /* lock script */
		res = pop();
		if (res >= _numGlobalScripts)
			break;
		lock(2,res);
		break;
	case 109:/* lock sound */
		res = pop();
		lock(4,res);
		break;
	case 110:/* lock costume */
		res = pop();
		lock(3,res);
		break;
	case 111:/* lock room */
		res = pop();
		if (res > 0x7F)
			res = _resourceMapper[res&0x7F];
		lock(1,res);
		break;
	case 112:/* unlock script */
		res = pop();
		if (res >= _numGlobalScripts)
			break;
		unlock(2,res);
		break;
	case 113:/* unlock sound */
		res = pop();
		unlock(4,res);
		break;
	case 114:/* unlock costume */
		res = pop();
		unlock(3,res);
		break;
	case 115:/* unlock room */
		res = pop();
		if (res > 0x7F)
			res = _resourceMapper[res&0x7F];
		unlock(1,res);
		break;
	case 116:/* clear heap */
		/* this is actually a scumm message */
		error("clear heap not working yet");
		break;
	case 117:/* load charset */
		res = pop();
		loadCharset(res);
		break;
	case 118:/* nuke charset */
		warning("popping extra argument in nukeCharset");
		res = pop();
		nukeCharset(res);
		break;
	case 119:/* ? */
		res = pop();
		unkResProc(pop(), res);
		break;
	default:
		error("o2_resourceRoutines: default case");
	}
}

void Scumm::o2_roomOps() {
	int a,b,c,d,e;

	switch(fetchScriptByte()) {
	case 172: /* room scroll */
		b = pop();
		a = pop();
		if (a < 160) a=160;
		if (a > ((_scrWidthIn8Unit-20)<<3)) a=((_scrWidthIn8Unit-20)<<3);
		if (b < 160) b=160;
		if (b > ((_scrWidthIn8Unit-20)<<3)) b=((_scrWidthIn8Unit-20)<<3);
		_vars[VAR_CAMERA_MIN] = a;
		_vars[VAR_CAMERA_MAX] = b;
		break;

	case 174: /* set screen */
		b = pop();
		a = pop();
		initScreens(0,a,320,b);
		break;

	case 175: /* set palette color */
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		setPalColor(d, a, b, c);
		break;

	case 176: /* shake on */
		setShake(1);
		break;

	case 177: /* shake off */
		setShake(0);
		break;

	case 179:
		c = pop();
		b = pop();
		a = pop();
		unkRoomFunc2(b,c,a,a,a);
		break;

	case 180:
		_saveLoadData = pop();
		_saveLoadFlag = pop();
		warning("o2_roomops:180: partially unimplemented");
		break;

	case 181:
		a = pop();
		if (a) {
			_switchRoomEffect = (byte)(a);
			_switchRoomEffect2 = (byte)(a>>8);
		} else {
			screenEffect(_newEffect);
		}
		break;

	case 182:
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		unkRoomFunc2(d, e, a, b, c);
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

	case 186: /* palmanip? */
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		unkRoomFunc4(a, b, c, d, 1);
		break;

	case 187: /* color cycle delay */
		b = pop();
		a = pop();
		if (b!=0)
			_colorCycleDelays[a] = 0x4000 / (b*0x4C);
		else
			_colorCycleDelays[a] = 0;
		break;

	case 213: /* set palette */
		setPalette(pop());
		break;

	default:
		error("o2_roomOps: default case");
	}
}

void Scumm::o2_actorSet() {
	Actor *a;
	int i,j,k;
	int16 args[8];
	byte b;

	b = fetchScriptByte();
	if (b==197) {
		_curActor = pop();
		return;
	}
	
	a = derefActorSafe(_curActor, "o2_actorSet");

	switch(b) {
	case 76: /* actor-costume */
		setActorCostume(a, pop());
		break;
	case 77: /* actor-speed */
		j = pop();
		i = pop();
		setActorWalkSpeed(a, i, j);
		break;
	case 78: /* actor-sound */
		k = getStackList(args, sizeof(args)/sizeof(args[0]));
		for (i=0; i<k; i++)
			a->sound[i] = args[i];
		break;
	case 79: /* actor-walkframe */
		a->walkFrame = pop();
		break;
	case 80: /* actor-talkframe */
		a->talkFrame2 = pop();
		a->talkFrame1 = pop();
		break;
	case 81: /* actor-standframe */
		a->standFrame = pop();
		break;
	case 82:
		pop();
		pop();
		pop();
		break;
	case 83:
		initActor(a, 0);
		break;
	case 84: /* actor-elevation */
		a->elevation = pop();
		a->needRedraw = true;
		a->needBgReset = true;
		break;
	case 85: /* actor-defaultanims */
		a->initFrame = 1;
		a->walkFrame = 2;
		a->standFrame = 3;
		a->talkFrame1 = 4;
		a->talkFrame2 = 5;
		break;
	case 86: /* actor-palette */
		j = pop();
		i = pop();
		checkRange(31, 0, i, "Illegal palet slot %d");
		a->palette[i] = j;
		a->needRedraw = true;
		break;
	case 87: /* actor-talkcolor */
		a->talkColor = pop();
		break;
	case 88: /* actor-name */
		loadPtrToResource(9, a->number, NULL);
		break;
	case 89: /* actor-initframe */
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
		a->neverZClip = 0;
		break;
	case 94:
		a->neverZClip = pop();
		break;
	case 95:
		a->ignoreBoxes = 1;
		a->neverZClip = 0;
FixRooms:;
		if (a->room==_currentRoom)
			putActor(a, a->x, a->y, a->room);
		break;
	case 96:
		a->ignoreBoxes = 0;
		a->neverZClip = 0;
		goto FixRooms;
	case 97:
		a->animSpeed = pop();
		a->animProgress = 0;
		break;
	case 98:
		a->data8 = pop();
		break;
	case 99:
		a->new_1 = pop();
		a->new_2 = pop();
		break;
	case 215:
		a->new_3 = 1;
		break;	
	case 216:
		a->new_3 = 0;
		break;
	case 217:
		initActor(a, 2);
		break;	
	default:
		error("o2_actorset: default case");
	}
}

void Scumm::o2_verbOps() {
	int slot,a,b;
	VerbSlot *vs;
	byte *ptr, op;
	
	op = fetchScriptByte();
	if(op==196) {
		_curVerb = pop();
		_curVerbSlot = getVerbSlot(_curVerb, 0);
		checkRange(_maxVerbs-1,0,_curVerbSlot,"Illegal new verb slot %d");
		return;
	}
	vs = &_verbs[_curVerbSlot];
	slot = _curVerbSlot;
	switch(op) {
	case 124: /* load img */
		a = pop();
		if (_curVerbSlot) {
			setVerbObject(_roomResource,a,slot);
			vs->type=1;
		}
		break;
	case 125:
		loadPtrToResource(8, slot, NULL);
		vs->type = 0;
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
		if (slot==0) {
			for (slot=1; slot<_maxVerbs; slot++) {
				if(_verbs[slot].verbid==0)
					break;
			}
			if (slot==_maxVerbs)
				error("Too many verbs");
			_curVerbSlot = slot;
		}
		vs = &_verbs[slot];
		vs->verbid = _curVerb;
		vs->color = 2;
		vs->hicolor = 0;
		vs->dimcolor = 8;
		vs->type = 0;
		vs->charset_nr = string[0].t_charset;
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
		if (a==0) {
			ptr = (byte*)"";
		} else {
			ptr = getResourceAddress(7, a);
		}
		loadPtrToResource(8, slot, ptr);
		vs->type = 0;
		vs->imgindex = 0;
		break;
	case 139:
		b = pop();
		a = pop();
		if (slot && a != vs->imgindex) {
			setVerbObject(b, a, slot);
			vs->type = 1;
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
		error("o2_verbops: default case");
	}
}

void Scumm::o2_getActorFromXY() {
	int y = pop();
	int x = pop();
	push(getActorFromPos(x,y));
}

void Scumm::o2_findObject() {
	int y = pop();
	int x = pop();
	int r = findObject(x,y);
	push(r);
}

void Scumm::o2_pseudoRoom() {
	int16 list[100];
	int num,a,value;

	num = getStackList(list,sizeof(list)/sizeof(list[0]));
	value = pop();

	while (--num>=0) {
		a = list[num];
		if (a > 0x7F)
			_resourceMapper[a&0x7F] = value;
	}
}

void Scumm::o2_getActorElevation() {
	push(derefActorSafe(pop(),"o2_getActorElevation")->elevation);
}

void Scumm::o2_getVerbEntrypoint() {
	int e = pop();
	int v = pop();
	push(getVerbEntrypoint(v,e));
}

void Scumm::o2_arrayOps() {
	int a,b,c,d,num;
	int16 list[128];

	switch(fetchScriptByte()) {
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
		if (d==0) {
			defineArray(a, 5, 0, b+c);
		}
		while (c--) {
			writeArray(a, 0, b+c, pop());
		}
		break;
	case 212:
		a = fetchScriptWord();
		b = pop();
		num = getStackList(list,sizeof(list)/sizeof(list[0]));
		d = readVar(a);
		if (d==0)
			error("Must DIM a two dimensional array before assigning");
		c = pop();
		while (--num>=0) {
			writeArray(a, c, b+num, list[num]);
		}
		break;
	default:
		error("o2_arrayOps: default case");
	}
}

void Scumm::o2_saveRestoreVerbs() {
	int a,b,c;
	int slot,slot2;

	c = pop();
	b = pop();
	a = pop();

	switch(fetchScriptByte()) {
	case 141:
		while (a<=b) {
			slot = getVerbSlot(a,0);
			if (slot && _verbs[slot].saveid==0) {
				_verbs[slot].saveid = c;
				drawVerb(slot, 0);
				verbMouseOver(0);
			}
			a++;
		}
		break;
	case 142:
		while (a<=b) {
			slot = getVerbSlot(a, c);
			if (slot) {
				slot2 = getVerbSlot(a,0);
				if (slot2)
					killVerb(slot2);
				slot = getVerbSlot(a,c);
				_verbs[slot].saveid = 0;
				drawVerb(slot, 0);
				verbMouseOver(0);
			}
			a++;
		}
		break;
	case 143:
		while (a<=b) {
			slot = getVerbSlot(a,c);
			if (slot)
				killVerb(slot);
			a++;
		}
		break;
	default:
		error("o2_saveRestoreVerbs: default case");
	}
}

void Scumm::o2_drawBox() {
	int x,y,x2,y2,color;
	color = pop();
	y2 = pop();
	x2 = pop();
	y = pop();
	x = pop();
	drawBox(x, y, x2, y2, color);
}

void Scumm::o2_getActorWidth() {
	push(derefActorSafe(pop(),"o2_getActorWidth")->width);
}

void Scumm::o2_wait() {
	byte oldaddr;
	
	switch(fetchScriptByte()) {
	case 168:
		if (derefActorSafe(pop(), "o2_wait")->moving) {
			_scriptPointer += (int16)fetchScriptWord();
			o2_breakHere();
		} else {
			fetchScriptWord();
		}
		return;
	case 169:
		if (_vars[VAR_HAVE_MSG])
			break;
		return;
	case 170:
		if (camera._curPos>>3 != camera._destPos>>3)
			break;
		return;
	case 171:
		if (_sentenceIndex!=0xFF) {
			if (sentence[_sentenceIndex].unk &&
				!isScriptLoaded(_vars[VAR_SENTENCE_SCRIPT]) )
				return;
			break;
		}
		if (!isScriptLoaded(_vars[VAR_SENTENCE_SCRIPT]))
			return;
		break;
	default:
		error("o2_wait: default case");
	}

	_scriptPointer -= 2;
	o2_breakHere();
}

void Scumm::o2_getActorScaleX() {
	push(derefActorSafe(pop(),"o2_getActorScale")->scalex);
}

void Scumm::o2_getActorAnimCounter1() {
	push(derefActorSafe(pop(),"o2_getActorAnimCounter")->cost.animCounter1);
}

void Scumm::o2_soundKludge() {
	int16 list[8];
	getStackList(list,sizeof(list)/sizeof(list[0]));
	soundKludge(list);
}

void Scumm::o2_isAnyOf() {
	int16 list[100];
	int num;
	int16 val;

	num = getStackList(list,sizeof(list)/sizeof(list[0]));
	val = pop();

	while (--num>=0) {
		if (list[num] == val) {
			push(1);
			return;
		}
	}
	push(0);
	return;
}

void Scumm::o2_quitPauseRestart() {
	switch(fetchScriptByte()) {
	case 158:
		pauseGame(0);
		break;
	case 160:
		shutDown(0);
		break;
	default:
		error("o2_quitPauseRestart: invalid case");
	}
}

void Scumm::o2_isActorInBox() {
	int box = pop();
	Actor *a = derefActorSafe(pop(), "o2_isActorInBox");
	push(checkXYInBoxBounds(box, a->x, a->y));
}

void Scumm::o2_delay() {
	uint32 delay = (uint16)pop();
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = 1;
	o2_breakHere();
}

void Scumm::o2_delayLonger() {
	uint32 delay = (uint16)pop() * 60;
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = 1;
	o2_breakHere();
}

void Scumm::o2_delayVeryLong() {
	uint32 delay = (uint16)pop() * 3600;
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = 1;
	o2_breakHere();
}

void Scumm::o2_stopSentence() {
	_sentenceIndex = 0xFF;
	stopScriptNr(_vars[VAR_SENTENCE_SCRIPT]);
	clearClickedStatus();
}

void Scumm::o2_print_0() {
	_actorToPrintStrFor = 0xFF;
	decodeParseString2(0,0);
}

void Scumm::o2_print_1() {
	decodeParseString2(1,0);
}

void Scumm::o2_print_2() {
	decodeParseString2(2,0);
}

void Scumm::o2_print_3() {
	decodeParseString2(3,0);
}

void Scumm::o2_printActor() {
	decodeParseString2(0,1);
}

void Scumm::o2_printEgo() {
	push(_vars[VAR_UNK_ACTOR]);
	decodeParseString2(0,1);
}

void Scumm::o2_talkActor() {
	_actorToPrintStrFor = pop();
	_messagePtr = _scriptPointer;
	setStringVars(0);
	actorTalk();
	_scriptPointer = _messagePtr;
}

void Scumm::o2_talkEgo() {
	_actorToPrintStrFor = _vars[VAR_UNK_ACTOR];
	_messagePtr = _scriptPointer;
	setStringVars(0);
	actorTalk();
	_scriptPointer = _messagePtr;
}

void Scumm::o2_dim() {
	byte b;
	int data;

	switch(fetchScriptByte()) {
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
		error("o2_dim: default case");
	}

	defineArray(fetchScriptWord(), data, 0, pop());
}

void Scumm::o2_runVerbCodeQuick() {
	int16 args[16];
	int script,entryp;
	getStackList(args,sizeof(args)/sizeof(args[0]));
	entryp = pop();
	script = pop();
	runVerbCode(script, entryp, 0, 1, args);
}

void Scumm::o2_runScriptQuick() {
	int16 args[16];
	int script;
	getStackList(args,sizeof(args)/sizeof(args[0]));
	script = pop();
	runScript(script, 0, 1, args);
}

void Scumm::o2_dim2() {
	int a,b,data;
	switch(fetchScriptByte()) {
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
		error("o2_dim2: default case");
	}

	b = pop();
	a = pop();
	defineArray(fetchScriptWord(), data, a, b);
}

void Scumm::o2_abs() {
	push(abs(pop()));
}


void Scumm::o2_distObjectObject() {
	int a,b;
	b = pop();
	a = pop();
	push(getDistanceBetween(true, a, 0, true, b, 0));
}

void Scumm::o2_distObjectPt() {
	int a,b,c;
	c = pop();
	b = pop();
	a = pop();
	push(getDistanceBetween(true, a, 0, false, b, c));
}

void Scumm::o2_distPtPt() {
	int a,b,c,d;
	d = pop();
	c = pop();
	b = pop();
	a = pop();
	push(getDistanceBetween(false, a, b, false, c, d));
}

void Scumm::o2_dummy_stacklist() {
	error("opcode o2_dummy_stacklist invalid");
}

void Scumm::o2_miscOps() {
	int16 args[30];
	int i;

	getStackList(args,sizeof(args)/sizeof(args[0]));
	switch(args[0]) {
	case 3:
		warning("o2_miscOps: nothing in 3");
		break;
	case 4:
		unkMiscOp4(args[1], args[2], args[3], args[4]);
		break;
	case 5:
		unkVirtScreen4(args[1]);
		break;
	case 6:
		_fullRedraw = 1;
		redrawBGAreas();
		for (i=0; i<13; i++)
			derefActor(i)->needRedraw = true;
		processActors();
		screenEffect(args[1]);
		break;
	case 8:
		startManiac();
		break;
	case 9:
		unkMiscOp9();
		break;
	}
}

void Scumm::o2_breakMaybe() {
	ScriptSlot *ss = &vm.slot[_currentScript];
	if (ss->newfield == 0) {
		ss->newfield = pop();
	} else {
		ss->newfield--;
	}
	if (ss->newfield) {
		_scriptPointer--;
		o2_breakHere();
	}
}

void Scumm::o2_pickOneOf() {
	int16 args[100];
	int i,num;

	num = getStackList(args,sizeof(args)/sizeof(args[0]));
	i = pop();
	if (i<0 || i>=num)
		error("o2_pickOneOf: out of range");
	push(args[i]);
}

void Scumm::o2_pickOneOfDefault() {
	int16 args[100];
	int i,num,def;

	def = pop();
	num = getStackList(args,sizeof(args)/sizeof(args[0]));
	i = pop();
	if (i<0 || i>=num)
		i = def;
	else
		i = args[i];
	push(i);
}

void Scumm::decodeParseString2(int m, int n) {
	byte b;

	b = fetchScriptByte();
	
	switch(b) {
	case 65:
		string[m].ypos = pop();
		string[m].xpos = pop();
		string[m].overhead = 0;
		break;
	case 66:
		string[m].color = pop();
		break;
	case 67:
		string[m].right = pop();
		break;
	case 69:
		string[m].center = 1;
		string[m].overhead = 0;
		break;
	case 71:
		string[m].center = 0;
		string[m].overhead = 0;
		break;
	case 72:
		string[m].overhead = 1;
		string[m].new_3 = 0;
		break;
	case 73:
		error("decodeParseString2: case 73");
		break;
	case 74:
		string[m].new_3 = 1;
		break;
	case 75:
		_messagePtr = _scriptPointer;
		switch(m) {
		case 0: actorTalk(); break;
		case 1: drawString(1); break;
		case 2: unkMessage1(); break;
		case 3: unkMessage2(); break;
		}
		_scriptPointer = _messagePtr;
		return;

	case 0xFE:
		setStringVars(m);
		if (n)
			_actorToPrintStrFor = pop();
		return;
	case 0xFF:
		string[m].t_xpos = string[m].xpos;
		string[m].t_ypos = string[m].ypos;
		string[m].t_center = string[m].center;
		string[m].t_overhead = string[m].overhead;
		string[m].t_new3 = string[m].new_3;
		string[m].t_right = string[m].right;
		string[m].t_color = string[m].color;
		string[m].t_charset = string[m].charset;
		return;
	default:
		error("decodeParseString: default case");
	}
}

#endif