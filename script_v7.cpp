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
 * Revision 1.1.2.1  2001/11/12 16:24:29  yazoo
 * The dig and Full Throttle support
 *
 * Revision 1.4  2001/10/26 17:34:50  strigeus
 * bug fixes, code cleanup
 *
 * Revision 1.3  2001/10/24 20:12:52  strigeus
 * fixed some bugs related to string handling
 *
 * Revision 1.2  2001/10/23 19:51:50  strigeus
 * recompile not needed when switching games
 * debugger skeleton implemented
 *
 * Revision 1.1  2001/10/16 10:01:47  strigeus
 * preliminary DOTT support
 *
 */


#include "stdafx.h"
#include "scumm.h"

void Scumm::setupOpcodesV7() {
	static const OpcodeProc opcode_list[256] = {
	/* 00 */
	&Scumm::o7_pushByte,
	&Scumm::o7_pushWord,
	&Scumm::o7_pushByteVar,
	&Scumm::o7_pushWordVar,
	/* 04 */
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_byteArrayRead,
	&Scumm::o7_wordArrayRead,
	/* 08 */
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_byteArrayIndexedRead,
	&Scumm::o7_wordArrayIndexedRead,
	/* 0C */
	&Scumm::o7_dup,
	&Scumm::o7_zero,
	&Scumm::o7_eq,
	&Scumm::o7_neq,
	/* 10 */
	&Scumm::o7_gt,
	&Scumm::o7_lt,
	&Scumm::o7_le,
	&Scumm::o7_ge,
	/* 14 */
	&Scumm::o7_add,
	&Scumm::o7_sub,
	&Scumm::o7_mul,
	&Scumm::o7_div,
	/* 18 */
	&Scumm::o7_land,
	&Scumm::o7_lor,
	&Scumm::o7_kill,
	&Scumm::o7_invalid,
	/* 1C */
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* 20 */
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* 24 */
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* 28 */
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* 2C */
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* 30 */
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* 34 */
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* 38 */
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* 3C */
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* 40 */
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_writeByteVar,
	&Scumm::o7_writeWordVar,
	/* 44 */
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_byteArrayWrite,
	&Scumm::o7_wordArrayWrite,
	/* 48 */
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_byteArrayIndexedWrite,
	&Scumm::o7_wordArrayIndexedWrite,
	/* 4C */
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_byteVarInc,
	&Scumm::o7_wordVarInc,
	/* 50 */
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_byteArrayInc,
	&Scumm::o7_wordArrayInc,
	/* 54 */
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_byteVarDec,
	&Scumm::o7_wordVarDec,
	/* 58 */
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_byteArrayDec,
	&Scumm::o7_wordArrayDec,
	/* 5C */
	&Scumm::o7_jumpTrue,
	&Scumm::o7_jumpFalse,
	&Scumm::o7_startScriptEx,
	&Scumm::o7_startScript,
	/* 60 */
	&Scumm::o7_startObject,
	&Scumm::o7_setObjectState,
	&Scumm::o7_setObjectXY,
	&Scumm::o7_invalid,
	/* 64 */
	&Scumm::o7_invalid,
	&Scumm::o7_stopObjectCode,
	&Scumm::o7_stopObjectCode,
	&Scumm::o7_endCutscene,
	/* 68 */
	&Scumm::o7_cutScene,
	&Scumm::o7_stopMusic,
	&Scumm::o7_freezeUnfreeze,
	&Scumm::o7_cursorCommand,
	/* 6C */
	&Scumm::o7_breakHere,
	&Scumm::o7_ifClassOfIs,
	&Scumm::o7_setClass,
	&Scumm::o7_getState,
	/* 70 */
	&Scumm::o7_setState,
	&Scumm::o7_setOwner,
	&Scumm::o7_getOwner,
	&Scumm::o7_jump,
	/* 74 */
	&Scumm::o7_startSound,
	&Scumm::o7_stopSound,
	&Scumm::o7_startMusic,
	&Scumm::o7_stopObjectScript,
	/* 78 */
	&Scumm::o7_panCameraTo,
	&Scumm::o7_actorFollowCamera,
	&Scumm::o7_setCameraAt,
	&Scumm::o7_loadRoom,
	/* 7C */
	&Scumm::o7_stopScript,
	&Scumm::o7_walkActorToObj,
	&Scumm::o7_walkActorTo,
	&Scumm::o7_putActorInRoom,
	/* 80 */
	&Scumm::o7_putActorAtObject,
	&Scumm::o7_faceActor,
	&Scumm::o7_animateActor,
	&Scumm::o7_doSentence,
	/* 84 */
	&Scumm::o7_pickupObject,
	&Scumm::o7_loadRoomWithEgo,
	&Scumm::o7_invalid,
	&Scumm::o7_getRandomNumber,
	/* 88 */
	&Scumm::o7_getRandomNumberRange,
	&Scumm::o7_invalid,
	&Scumm::o7_getActorMoving,
	&Scumm::o7_getScriptRunning,
	/* 8C */
	&Scumm::o7_getActorRoom,
	&Scumm::o7_getObjectX,
	&Scumm::o7_getObjectY,
	&Scumm::o7_getObjectDir,
	/* 90 */
	&Scumm::o7_getActorWalkBox,
	&Scumm::o7_getActorCostume,
	&Scumm::o7_findInventory,
	&Scumm::o7_getInventoryCount,
	/* 94 */
	&Scumm::o7_getVerbFromXY,
	&Scumm::o7_beginOverride,
	&Scumm::o7_endOverride,
	&Scumm::o7_setObjectName,
	/* 98 */
	&Scumm::o7_isSoundRunning,
	&Scumm::o7_setBoxFlags,
	&Scumm::o7_createBoxMatrix,
	&Scumm::o7_resourceRoutines,
	/* 9C */
	&Scumm::o7_roomOps,
	&Scumm::o7_actorSet,
	&Scumm::o7_verbOps,
	&Scumm::o7_getActorFromXY,
	/* A0 */
	&Scumm::o7_findObject,
	&Scumm::o7_pseudoRoom,
	&Scumm::o7_getActorElevation,
	&Scumm::o7_getVerbEntrypoint,
	/* A4 */
	&Scumm::o7_arrayOps,
	&Scumm::o7_saveRestoreVerbs,
	&Scumm::o7_drawBox,
	&Scumm::o7_invalid,
	/* A8 */
	&Scumm::o7_getActorWidth,
	&Scumm::o7_wait,
	&Scumm::o7_getActorScaleX,
	&Scumm::o7_getActorAnimCounter1,
	/* AC */
	&Scumm::o7_soundKludge,
	&Scumm::o7_isAnyOf,
	&Scumm::o7_quitPauseRestart,
	&Scumm::o7_isActorInBox,
	/* B0 */
	&Scumm::o7_delay,
	&Scumm::o7_delayLonger,
	&Scumm::o7_delayVeryLong,
	&Scumm::o7_stopSentence,
	/* B4 */
	&Scumm::o7_print_0,
	&Scumm::o7_print_1,
	&Scumm::o7_print_2,
	&Scumm::o7_print_3,
	/* B8 */
	&Scumm::o7_printActor,
	&Scumm::o7_printEgo,
	&Scumm::o7_talkActor,
	&Scumm::o7_talkEgo,
	/* BC */
	&Scumm::o7_dim,
	&Scumm::o7_invalid,
	&Scumm::o7_runVerbCodeQuick,
	&Scumm::o7_runScriptQuick,
	/* C0 */
	&Scumm::o7_dim2,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* C4 */
	&Scumm::o7_abs,
	&Scumm::o7_distObjectObject,
	&Scumm::o7_distObjectPt,
	&Scumm::o7_distPtPt,
	/* C8 */
	&Scumm::o7_Kfunction,
	&Scumm::o7_miscOps,
	&Scumm::o7_breakMaybe,
	&Scumm::o7_pickOneOf,
	/* CC */
	&Scumm::o7_pickOneOfDefault,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* D0 */	
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_animateGetVariable,
	&Scumm::o7_invalid,
	/* D4 */	
	&Scumm::o7_invalid,
	&Scumm::o7_chainScript,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* D8 */	
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* DC */	
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* E0 */	
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* E4 */	
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* E8 */	
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* EC */	
	&Scumm::o7_invalid,
	&Scumm::o7_ED,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* F0 */	
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* F4 */	
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* F8 */	
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	/* FC */	
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	&Scumm::o7_invalid,
	};

	_opcodes = opcode_list;
}

int Scumm::readArrayV7(int array, int index, int base) {
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

void Scumm::writeArrayV7(int array, int index, int base, int value) {
	ArrayHeader *ah = (ArrayHeader*)getResourceAddress(7, readVar(array));
	assert(ah);
	base += index*ah->dim1_size;

//	assert(base>=0 && base < ah->dim1_size*ah->dim2_size); Dif

	if (ah->type==4) {
		ah->data[base] = value;
	} else {
		((uint16*)ah->data)[base] = TO_LE_16(value);
	}
}

int Scumm::getStackListV7(int16 *args, uint maxnum) {
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

void Scumm::o7_pushByte() {
	push(fetchScriptByte());
}

void Scumm::o7_pushWord() {
	push((int16)fetchScriptWord());
}

void Scumm::o7_pushByteVar() {
	push(readVar(fetchScriptByte()));
}

void Scumm::o7_pushWordVar() {
	push(readVar(fetchScriptWord()));
}

void Scumm::o7_invalid() {
	error("Invalid opcode '%x' at %x", _opcode, _scriptPointer - _scriptOrgPointer);
}

void Scumm::o7_byteArrayRead() {
	int base = pop();
	push(readArrayV7(fetchScriptByte(), 0, base));
}

void Scumm::o7_wordArrayRead() {
	int base = pop();
	push(readArrayV7(fetchScriptWord(), 0, base));
}

void Scumm::o7_byteArrayIndexedRead() {
	int base = pop();
	int index = pop();
	push(readArrayV7(fetchScriptByte(), index, base));
}

void Scumm::o7_wordArrayIndexedRead() {
	int base = pop();
	int index = pop();
	push(readArrayV7(fetchScriptWord(), index, base));
}

void Scumm::o7_dup() {
	int a = pop();
	push(a);
	push(a);
}

void Scumm::o7_zero() {
	push( pop() == 0 );
}

void Scumm::o7_eq() {
	push( pop() == pop() );
}

void Scumm::o7_neq() {
	push( pop() != pop() );
}

void Scumm::o7_gt() {
	int a = pop();
	push( pop() > a );
}

void Scumm::o7_lt() {
	int a = pop();
	push( pop() < a );
}

void Scumm::o7_le() {
	int a = pop();
	push( pop() <= a );
}

void Scumm::o7_ge() {
	int a = pop();
	push( pop() >= a );
}

void Scumm::o7_add() {
	int a = pop();
	push( pop() + a );
}

void Scumm::o7_sub() {
	int a = pop();
	push( pop() - a );
}

void Scumm::o7_mul() {
	int a = pop();
	push( pop() * a );
}

void Scumm::o7_div() {
	int a = pop();
	if (a==0) error("division by zero");
	push( pop() / a );
}

void Scumm::o7_land() {
	int a = pop();
	push( pop() && a );
}

void Scumm::o7_lor() {
	int a = pop();
	push( pop() || a );
}

void Scumm::o7_kill() {
	pop();
}

void Scumm::o7_writeByteVar() {
	writeVar(fetchScriptByte(), pop());
}

void Scumm::o7_writeWordVar() {
	writeVar(fetchScriptWord(), pop());
}

void Scumm::o7_byteArrayWrite() {
	int a = pop();
	writeArrayV7(fetchScriptByte(), 0, pop(), a);
}

void Scumm::o7_wordArrayWrite() {
	int a = pop();
	writeArrayV7(fetchScriptWord(), 0, pop(), a);
}

void Scumm::o7_byteArrayIndexedWrite() {
	int val = pop();
	int base = pop();
	writeArrayV7(fetchScriptByte(), pop(), base, val);
}

void Scumm::o7_wordArrayIndexedWrite() {
	int val = pop();
	int base = pop();
	writeArrayV7(fetchScriptWord(), pop(), base, val);
}

void Scumm::o7_byteVarInc() {
	int var = fetchScriptByte();
	writeVar(var,readVar(var)+1);
}

void Scumm::o7_wordVarInc() {
	int var = fetchScriptWord();
	writeVar(var,readVar(var)+1);
}

void Scumm::o7_byteArrayInc() {
	int var = fetchScriptByte();
	int base = pop();
	writeArrayV7(var, 0, base, readArrayV7(var, 0, base) + 1);
}

void Scumm::o7_wordArrayInc() {
	int var = fetchScriptWord();
	int base = pop();
	writeArrayV7(var, 0, base, readArrayV7(var, 0, base) + 1);
}


void Scumm::o7_byteVarDec() {
	int var = fetchScriptByte();
	writeVar(var,readVar(var)-1);
}

void Scumm::o7_wordVarDec() {
	int var = fetchScriptWord();
	writeVar(var,readVar(var)-1);
}

void Scumm::o7_byteArrayDec() {
	int var = fetchScriptByte();
	int base = pop();
	writeArrayV7(var, 0, base, readArrayV7(var, 0, base) - 1);
}

void Scumm::o7_wordArrayDec() {
	int var = fetchScriptWord();
	int base = pop();
	writeArrayV7(var, 0, base, readArrayV7(var, 0, base) - 1);
}

void Scumm::o7_jumpTrue() {
	if (pop())
		o7_jump();
	else
		fetchScriptWord();
}

void Scumm::o7_jumpFalse() {
	if (!pop())
		o7_jump();
	else
		fetchScriptWord();
}

void Scumm::o7_jump() {
	_scriptPointer += (int16)fetchScriptWord();
}

void Scumm::o7_startScriptEx() {
	int16 args[16];
	int script,flags;

	getStackListV7(args,sizeof(args)/sizeof(args[0]));
	script = pop();
	flags = pop();
	runScript(script, flags&1, flags&2, args);
}

void Scumm::o7_startScript() {
	int16 args[16];
	int script;
	getStackListV7(args,sizeof(args)/sizeof(args[0]));
	script = pop();
	runScript(script, 0, 0, args);
}

void Scumm::o7_startObject() {
	int16 args[16];
	int script,entryp;
	int flags;
	getStackListV7(args,sizeof(args)/sizeof(args[0]));
	entryp = pop();
	script = pop();
	flags = pop();
	runVerbCode(script, entryp, flags&1, flags&2, args);
}

void Scumm::o7_setObjectState() {
	int a = pop();
	if (a==0) a=1;
	setObjectState(pop(), a, -1, -1);
}

void Scumm::o7_setObjectXY() {
	int y = pop();
	int x = pop();
	setObjectState(pop(), 1, x, y);
}

void Scumm::o7_stopObjectCode() {
	stopObjectCode();
}

void Scumm::o7_endCutscene() {
	endCutscene();
}

void Scumm::o7_cutScene() {
	int16 args[16];
	getStackListV7(args,sizeof(args)/sizeof(args[0]));
	cutscene(args);
}

void Scumm::o7_stopMusic() {
	warning("o7_stopMusic: not implemented");
}

void Scumm::o7_freezeUnfreeze() {
	int a = pop();
	if (a)
		freezeScripts(a);
	else
		unfreezeScripts();
}

void Scumm::o7_cursorCommand() {
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
	//	setCursorImg(a, pop()); Dif with version 6 there
		break;
	case 0x9A:
		a = pop();
		setCursorHotspot2(pop(),a);
		break;
	case 0x9C: /* init charset */
		initCharset(pop());
		break;
	case 0x9D: /* set charset colors */
		getStackListV7(args,sizeof(args)/sizeof(args[0]));
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

void Scumm::o7_breakHere() {
	updateScriptPtr();
	_currentScript = 0xFF;
}

void Scumm::o7_ifClassOfIs() {
	int16 args[16];
	int num,obj,cls;
	bool b;
	int cond = 1;

	num = getStackListV7(args,sizeof(args)/sizeof(args[0]));
	obj = pop();

	while (--num>=0) {
		cls = args[num];
		b = getClass(obj, cls);
		if (cls&0x80 && !b || !(cls&0x80) && b)
			cond = 0;
	}
	push(cond);
}

void Scumm::o7_setClass() {
	int16 args[16];
	int num,obj,cls;

	num = getStackListV7(args,sizeof(args)/sizeof(args[0]));
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

void Scumm::o7_getState() {
	push(getState(pop()));
}

void Scumm::o7_setState() {
	int state = pop();
	int obj = pop();

	putState(obj, state);
	removeObjectFromRoom(obj);
	if (_BgNeedsRedraw)
		clearDrawObjectQueue();
}

void Scumm::o7_setOwner() {
	int owner = pop();
	int obj = pop();

	setOwnerOf(obj, owner);
}

void Scumm::o7_getOwner() {
	push(getOwner(pop()));
}

void Scumm::o7_startSound() {
	addSoundToQueue(pop());
}

void Scumm::o7_stopSound() {
	unkSoundProc1(pop());
}

void Scumm::o7_startMusic() {
	addSoundToQueue(pop());
}

void Scumm::o7_stopObjectScript() {
	stopObjectScript(pop());
}

void Scumm::o7_panCameraTo() {
	panCameraTo(pop());
}

void Scumm::o7_actorFollowCamera() {
	actorFollowCamera(pop());
}

void Scumm::o7_setCameraAt() {
	setCameraAtEx(pop());
}

void Scumm::o7_loadRoom() {
	int room = pop();
	debug(1,"Loading room %d", room);
	startScene(room, 0, 0);
	_fullRedraw = 1;
}

void Scumm::o7_stopScript() {
	int script = pop();
	if (script==0)
		stopObjectCode();
	else
		stopScriptNr(script);
}

void Scumm::o7_walkActorToObj() {
	int obj,dist;
	Actor *a, *a2;
	int x;

	dist = pop();
	obj = pop();
	a = derefActorSafe(pop(), "o7_walkActorToObj");

	if (obj >= 17) {
		if (whereIsObject(obj)==-1)
			return;
		getObjectXYPos(obj);
		startWalkActor(a, _xPos, _yPos, _dir);
	} else {
		a2 = derefActorSafe(obj, "o7_walkActorToObj(2)");
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

void Scumm::o7_walkActorTo() {
	int x,y;
	y = pop();
	x = pop();
	startWalkActor(derefActorSafe(pop(), "o7_walkActorTo"), x, y, 0xFF);
}

void Scumm::o7_putActorInRoom() {
	int room, x, y;
	Actor *a;

	room = pop();
	y = pop();
	x = pop();
	a = derefActorSafe(pop(), "o7_putActorInRoom");
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

void Scumm::o7_putActorAtObject() {
	int room,obj,x,y;
	Actor *a;

	room = pop();
	obj = pop();

	a = derefActorSafe(pop(), "o7_putActorAtObject");
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

void Scumm::o7_faceActor() {
	int act,obj;
	obj = pop();
	act = pop();
	faceActorToObj(act, obj);
}

void Scumm::o7_animateActor() {
	int anim = pop();
	int act = pop();
	animateActor(act, anim);
}

void Scumm::o7_doSentence() {
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

void Scumm::o7_pickupObject() {
	int obj, room;

	room = pop();
	obj = pop();
	
	if (room==0)
		room = _roomResource;
	addObjectToInventory(obj, room);
	putOwner(obj, _vars[VAR_EGO]);
	putClass(obj, 32, 1);
	putState(obj, 1);
	removeObjectFromRoom(obj);
	clearDrawObjectQueue();
	runHook(obj); /* Difference */
}

void Scumm::o7_loadRoomWithEgo() {
	Actor *a;
	int room,obj,x,y;

	y = pop();
	x = pop();
	room = pop();
	obj = pop();

	a = derefActorSafe(_vars[VAR_EGO], "o_loadRoomWithEgo");

	putActor(a, 0, 0, room);
	_egoPositioned = false;

	_vars[VAR_WALKTO_OBJ] = obj;
	startScene(a->room, a, obj);
	_vars[VAR_WALKTO_OBJ] = 0;

	/* startScene maybe modifies VAR_EGO, i hope not */
	camera._destPos = camera._curPos = a->x;
	setCameraFollows(a);
	_fullRedraw=1;
	if (x != -1) {
		startWalkActor(a, x, y, 0xFF);
	}
}

void Scumm::o7_getRandomNumber() {
	int rnd;
	rnd = getRandomNumber(pop()+1);
	_vars[VAR_V6_RANDOM_NR] = rnd;
	push(rnd);
}

void Scumm::o7_getRandomNumberRange() {
	int max = pop();
	int min = pop();
	int rnd = getRandomNumber(max-min+1) + min;
	_vars[VAR_V6_RANDOM_NR] = rnd;
	push(rnd);
}

void Scumm::o7_getActorMoving() {
	push(derefActorSafe(pop(),"o7_getActorMoving")->moving);
}

void Scumm::o7_getScriptRunning() {
	push(getScriptRunning(pop()));
}

void Scumm::o7_getActorRoom() {
	push(derefActorSafe(pop(),"o7_getActorRoom")->room);
}

void Scumm::o7_getObjectX() {
	push(getObjX(pop()));
}

void Scumm::o7_getObjectY() {
	push(getObjY(pop()));
}

void Scumm::o7_getObjectDir() {
	push(getObjDir(pop()));
}

void Scumm::o7_getActorWalkBox() {
	push(derefActorSafe(pop(),"o7_getActorWalkBox")->walkbox);
}

void Scumm::o7_getActorCostume() {
	push(derefActorSafe(pop(),"o7_getActorCostume")->costume);
}

void Scumm::o7_findInventory() {
	int index = pop();
	int owner = pop();
	push(findInventory(owner,index));
}

void Scumm::o7_getInventoryCount() {
	push(getInventoryCount(pop()));
}

void Scumm::o7_getVerbFromXY() {
	int y = pop();
	int x = pop();
	int over = checkMouseOver(x,y);
	if (over)
		over = _verbs[over].verbid;
	push(over);
}

void Scumm::o7_beginOverride() {
	beginOverride();
}

void Scumm::o7_endOverride() {
	endOverride();
}

void Scumm::o7_setObjectName() {
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

void Scumm::o7_isSoundRunning() {
	int snd = pop();
	if (snd)
		snd = unkSoundProc23(snd);
	push(snd);
}

void Scumm::o7_setBoxFlags() {
	int16 table[65];
	int num,value;

	value = pop();
	num = getStackListV7(table,sizeof(table)/sizeof(table[0]));

	while (--num>=0) {
		setBoxFlags(table[num], value);
	}
}

void Scumm::o7_createBoxMatrix() {
	createBoxMatrix();
}

void Scumm::o7_resourceRoutines() {
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
	//	unkResProc(pop(), res); Dif with version 6
		break;
	default:
		error("o7_resourceRoutines: default case");
	}
}

void Scumm::o7_roomOps() {
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
		darkenPalette(b,c,a,a,a);
		break;

	case 180:
		_saveLoadCompatible = true;
		_saveLoadSlot = pop();
		_saveLoadFlag = pop();
		warning("o7_roomops:180: partially unimplemented");
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
		checkRange(16, 1, a, "o7_roomOps: 187: color cycle out of range (%d)");
		_colorCycle[a-1].delay = (b!=0) ? 0x4000 / (b*0x4C) : 0;
		break;

	case 213: /* set palette */
		setPalette(pop());
		break;

	default:
		error("o7_roomOps: default case");
	}
}

void Scumm::o7_actorSet() {
	Actor *a;
	int i,j,k;
	int16 args[8];
	byte b;
	int var;

	b = fetchScriptByte();
	if (b==197) {
		_curActor = pop();
		return;
	}
	
	a = derefActorSafe(_curActor, "o7_actorSet");

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
		k = getStackListV7(args, sizeof(args)/sizeof(args[0]));
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

				//all cases bellow are V7 only I think.
	case 228:
		a->script=pop();
		break;
	case 230:  // Yet another unsupported function :(
		pop();
		break;
	case 231:
		pop();
		break;
	case 235:
		a->script2=pop();
		break;
	case 198:
		var=pop();
		if (var<0 || var>=15)
			error("o7_animateSetVariable: out of range");
		a->AnimateVar[var]=pop();
		break;
	default:
		error("o7_actorset: default case");
	}
}

void Scumm::o7_verbOps() {
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
			ptr = getStringAddress(a);
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
		error("o7_verbops: default case");
	}
}

void Scumm::o7_getActorFromXY() {
	int y = pop();
	int x = pop();
	push(getActorFromPos(x,y));
}

void Scumm::o7_findObject() {
	int y = pop();
	int x = pop();
	int r = findObject(x,y);
	push(r);
}

void Scumm::o7_pseudoRoom() {
	int16 list[100];
	int num,a,value;

	num = getStackListV7(list,sizeof(list)/sizeof(list[0]));
	value = pop();

	while (--num>=0) {
		a = list[num];
		if (a > 0x7F)
			_resourceMapper[a&0x7F] = value;
	}
}

void Scumm::o7_getActorElevation() {
	push(derefActorSafe(pop(),"o7_getActorElevation")->elevation);
}

void Scumm::o7_getVerbEntrypoint() {
	int e = pop();
	int v = pop();
	push(getVerbEntrypoint(v,e));
}

void Scumm::o7_arrayOps() {
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
			writeArrayV7(a, 0, b+c, pop());
		}
		break;
	case 212:
		a = fetchScriptWord();
		b = pop();
		num = getStackListV7(list,sizeof(list)/sizeof(list[0]));
		d = readVar(a);
		if (d==0)
			error("Must DIM a two dimensional array before assigning");
		c = pop();
		while (--num>=0) {
			writeArrayV7(a, c, b+num, list[num]);
		}
		break;
	default:
		error("o7_arrayOps: default case");
	}
}

void Scumm::o7_saveRestoreVerbs() {
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
		error("o7_saveRestoreVerbs: default case");
	}
}

void Scumm::o7_drawBox() {
	int x,y,x2,y2,color;
	color = pop();
	y2 = pop();
	x2 = pop();
	y = pop();
	x = pop();
	drawBox(x, y, x2, y2, color);
}

void Scumm::o7_getActorWidth() {
	push(derefActorSafe(pop(),"o7_getActorWidth")->width);
}

void Scumm::o7_wait() {
	byte oldaddr;

	int temp;
	temp=fetchScriptByte();
	
	switch(temp) {
	case 168: // I think this has been changed in V7
		if (derefActorSafe(pop(), "o7_wait")->moving) {
			_scriptPointer += (int16)fetchScriptWord();
			o7_breakHere();
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
		 // All cases bellow are V7...
	case 226: // This opcode refers to a yet unknown array...
		pop(); 
		fetchScriptWord();
		o7_breakHere();
		return;
	case 232: // Must be a bug somewhere in this..
		if(_currentRoom==0)
			return;
		if (derefActorSafe(pop(), "o7_wait")->moving!=4)
		{
			fetchScriptWord();
			return;
		}
		_scriptPointer += (int16)fetchScriptWord();
		o7_breakHere();
		return;
	default:
		error("o7_wait: default case");
	}

	_scriptPointer -= 2;
	o7_breakHere();
}

void Scumm::o7_getActorScaleX() {
	push(derefActorSafe(pop(),"o7_getActorScale")->scalex);
}

void Scumm::o7_getActorAnimCounter1() {
	push(derefActorSafe(pop(),"o7_getActorAnimCounter")->cost.animCounter1);
}

void Scumm::o7_soundKludge() {
	int16 list[8];
	getStackListV7(list,sizeof(list)/sizeof(list[0]));
	soundKludge(list);
}

void Scumm::o7_isAnyOf() {
	int16 list[100];
	int num;
	int16 val;

	num = getStackListV7(list,sizeof(list)/sizeof(list[0]));
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

void Scumm::o7_quitPauseRestart() {
	switch(fetchScriptByte()) {
	case 158:
		pauseGame(0);
		break;
	case 160:
		shutDown(0);
		break;
	default:
		error("o7_quitPauseRestart: invalid case");
	}
}

void Scumm::o7_isActorInBox() {
	int box = pop();
	Actor *a = derefActorSafe(pop(), "o7_isActorInBox");
	push(checkXYInBoxBounds(box, a->x, a->y));
}

void Scumm::o7_delay() {
	uint32 delay = (uint16)pop();
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = 1;
	o7_breakHere();
}

void Scumm::o7_delayLonger() {
	uint32 delay = (uint16)pop() * 60;
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = 1;
	o7_breakHere();
}

void Scumm::o7_delayVeryLong() {
	uint32 delay = (uint16)pop() * 3600;
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = 1;
	o7_breakHere();
}

void Scumm::o7_stopSentence() {
	_sentenceIndex = 0xFF;
	stopScriptNr(_vars[VAR_SENTENCE_SCRIPT]);
	clearClickedStatus();
}

void Scumm::o7_print_0() {
	_actorToPrintStrFor = 0xFF;
	decodeParseStringV7(0,0);
}

void Scumm::o7_print_1() {
	decodeParseStringV7(1,0);
}

void Scumm::o7_print_2() {
	decodeParseStringV7(2,0);
}

void Scumm::o7_print_3() {
	decodeParseStringV7(3,0);
}

void Scumm::o7_printActor() {
	decodeParseStringV7(0,1);
}

void Scumm::o7_printEgo() {
	push(_vars[VAR_EGO]);
	decodeParseStringV7(0,1);
}

void Scumm::o7_talkActor() {
	_actorToPrintStrFor = pop();
	_messagePtr = _scriptPointer;
	setStringVars(0);
	actorTalk();
	_scriptPointer = _messagePtr;
}

void Scumm::o7_talkEgo() {
	_actorToPrintStrFor = _vars[VAR_EGO];
	_messagePtr = _scriptPointer;
	setStringVars(0);
	actorTalk();
	_scriptPointer = _messagePtr;
}

void Scumm::o7_dim() {
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
		error("o7_dim: default case");
	}

	defineArray(fetchScriptWord(), data, 0, pop());
}

void Scumm::o7_runVerbCodeQuick() {
	int16 args[16];
	int script,entryp;
	getStackListV7(args,sizeof(args)/sizeof(args[0]));
	entryp = pop();
	script = pop();
	runVerbCode(script, entryp, 0, 1, args);
}

void Scumm::o7_runScriptQuick() {
	int16 args[16];
	int script;
	getStackListV7(args,sizeof(args)/sizeof(args[0]));
	script = pop();
	runScript(script, 0, 1, args);
}

void Scumm::o7_dim2() {
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
		error("o7_dim2: default case");
	}

	b = pop();
	a = pop();
	defineArray(fetchScriptWord(), data, a, b);
}

void Scumm::o7_abs() {
	push(abs(pop()));
}


void Scumm::o7_distObjectObject() {
	int a,b;
	b = pop();
	a = pop();
	push(getDistanceBetween(true, a, 0, true, b, 0));
}

void Scumm::o7_distObjectPt() {
	int a,b,c;
	c = pop();
	b = pop();
	a = pop();
	push(getDistanceBetween(true, a, 0, false, b, c));
}

void Scumm::o7_distPtPt() {
	int a,b,c,d;
	d = pop();
	c = pop();
	b = pop();
	a = pop();
	push(getDistanceBetween(false, a, b, false, c, d));
}

void Scumm::o7_dummy_stacklist() {
	error("opcode o7_dummy_stacklist invalid");
}

void Scumm::o7_miscOps() { // Require update for V7
	int16 args[30];
	int i;

	getStackListV7(args,sizeof(args)/sizeof(args[0]));
	switch(args[0]) {
	case 3:
		warning("o7_miscOps: nothing in 3");
		break;
	case 4:
		unkMiscOp4(args[1], args[2], args[3], args[4]);
		break;
	case 5:
		unkVirtScreen4(args[1]);
		break;
	case 6: // That's the start video
		startVideo();
		break;
	case 8:
		startManiac();
		break;
	case 9:
		unkMiscOp9();
		break;
	case 12: // That's a set cursor image
		break;
	case 14: // that launch the remap actor
		break;
	case 15:
		break;
	case 16:
		break;
	case 124:
		break;
	default:
		warning("MiscOps default case");
		break;
	}
}

void Scumm::o7_breakMaybe() {
	ScriptSlot *ss = &vm.slot[_currentScript];
	if (ss->newfield == 0) {
		ss->newfield = pop();
	} else {
		ss->newfield--;
	}
	if (ss->newfield) {
		_scriptPointer--;
		o7_breakHere();
	}
}

void Scumm::o7_pickOneOf() {
	int16 args[100];
	int i,num;

	num = getStackListV7(args,sizeof(args)/sizeof(args[0]));
	i = pop();
	if (i<0 || i>=num)
		error("o7_pickOneOf: out of range");
	push(args[i]);
}

void Scumm::o7_pickOneOfDefault() {
	int16 args[100];
	int i,num,def;

	def = pop();
	num = getStackListV7(args,sizeof(args)/sizeof(args[0]));
	i = pop();
	if (i<0 || i>=num)
		i = def;
	else
		i = args[i];
	push(i);
}

void Scumm::decodeParseStringV7(int m, int n) {
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
		error("decodeParseStringV7: case 73");
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

void Scumm::o7_animateGetVariable() {
	Actor *a;
	int var;

	_curActor = pop();
	a = derefActorSafe(_curActor, "o7_animateGetVariable");

	var=pop();

	if (var<=0 || var>=15)
		error("o7_animateGetVariable: out of range");

	push(a->AnimateVar[var]);
}

void Scumm::o7_ED() {
	int var;
	Actor *a;

	var=pop();

	if(var>30)
	{
		warning("ED!");
	}
	else
	{
		_curActor=var;
		a = derefActorSafe(_curActor, "o7_ED");
		push(a->facing);
	}
}

void Scumm::o7_Kfunction() {
	int16 args[30];
	int i;

	getStackListV7(args,sizeof(args)/sizeof(args[0]));

	switch(args[0]) {
	case 215:
		push(0);
		break;
	default:
		warning("Kfunction default case");
		break;
	}
}

void Scumm::o7_chainScript() { // Rechecked OK for V7
	int16 args[30];
	int i;
	int j;

	o7_stopObjectCode();

	getStackListV7(args,sizeof(args)/sizeof(args[0]));

	i=pop();
	j=pop(); // j contains the 2 params for the script start

	runScript(i,j&1,j&2,args);
}