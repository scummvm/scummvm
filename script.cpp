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
 * Revision 1.1  2001/10/09 14:30:13  strigeus
 * Initial revision
 *
 *
 */

#include "stdafx.h"
#include "scumm.h"

void Scumm::runScript(int script, int a, int b, int16 *lvarptr) {
	byte *scriptPtr;
	uint32 scriptOffs;
	byte scriptType;
	int slot,i;
	ScriptSlot *s;

	if (script==0)
		return;

	if (b==0)
		stopScriptNr(script);

	if (script < _numGlobalScriptsUsed) {
		scriptPtr = getResourceAddress(2, script);
		scriptOffs = 8;
		scriptType = 2;
	} else {
		scriptOffs = _localScriptList[script - _numGlobalScriptsUsed];
		if (scriptOffs == 0)
			error("Local script %d is not in room %d", script, _roomResource);
		scriptOffs += 9;
		scriptType = 3;
	}

	slot = getScriptSlot();

	s = &vm.slot[slot];
	s->number = script;
	s->offs = scriptOffs;
	s->status = 2;
	s->type = scriptType;
	s->unk1 = a;
	s->unk2 = b;
	s->freezeCount = 0;
	
	if (lvarptr==NULL) {
		for(i=0; i<16; i++)
			vm.localvar[slot * 0x11 + i] = 0;
	} else {
		for(i=0; i<16; i++)
			vm.localvar[slot * 0x11 + i] = lvarptr[i];
	}

	runScriptNested(slot);
}

void Scumm::stopScriptNr(int script) {
	ScriptSlot *ss;
	NestedScript *nest;
	int i,num;

	if (script==0)
		return;

	ss = &vm.slot[1];
	
	for (i=1; i<20; i++,ss++) {
		if (script!=ss->number || ss->type!=2 && ss->type!=3 || ss->status==0)
			continue;

		if (ss->cutsceneOverride)
			error("Script %d stopped with active cutscene/override", script);
		ss->number = 0;
		ss->status = 0;
		if (_currentScript == i)
			_currentScript = 0xFF;
	}

	if (_numNestedScripts==0)
		return;

	nest = &vm.nest[0];
	num = _numNestedScripts;

	do {
		if (nest->number == script && (nest->type==2 || nest->type==3)) {
			nest->number = 0xFFFF;
			nest->slot = 0xFF;
			nest->type = 0xFF;
		}
	} while(nest++,--num);
}

void Scumm::stopObjectScript(int script) {
	ScriptSlot *ss;
	NestedScript *nest;
	int i,num;

	if (script==0)
		return;

	ss = &vm.slot[1];
	
	for (i=1; i<20; i++,ss++) {
		if (script!=ss->number || ss->type!=1 && ss->type!=0 && ss->type!=4 || ss->status==0)
			continue;

		if (ss->cutsceneOverride)
			error("Object %d stopped with active cutscene/override", script);
		ss->number = 0;
		ss->status = 0;
		if (_currentScript == i)
			_currentScript = 0xFF;
	}

	if (_numNestedScripts==0)
		return;

	nest = &vm.nest[0];
	num = _numNestedScripts;

	do {
		if (nest->number == script && (nest->type==1 || nest->type==4 || nest->type==0)) {
			nest->number = 0xFFFF;
			nest->slot = 0xFF;
			nest->type = 0xFF;
		}
	} while(nest++,--num);
}

int Scumm::getScriptSlot() {
	ScriptSlot *ss;
	int i;
	ss = &vm.slot[1];
	
	for (i=1; i<20; i++,ss++) {
		if(ss->status==0)
			return i;
	}
	error("Too many scripts running, %d max", 20);
}

void Scumm::runScriptNested(int script) {
	NestedScript *nest;
	ScriptSlot *slot;

	updateScriptPtr();
	
	nest = &vm.nest[_numNestedScripts];

	if (_currentScript==0xFF) {
		nest->number = 0xFF;
		nest->type = 0xFF;
	} else {
		slot = &vm.slot[_currentScript];
		nest->number = slot->number;
		nest->type = slot->type;
		/* scumm is buggy here */
		nest->slot = _currentScript;
	}

	if (++_numNestedScripts>=0x10)
		error("Too many nested scripts");

	_currentScript = script;

	getScriptBaseAddress();
	getScriptEntryPoint();
	executeScript();

	_numNestedScripts--;

	nest = &vm.nest[_numNestedScripts];
	
	if (nest->number != 0xFF) {
		slot = &vm.slot[nest->slot];
		if (slot->number == nest->number && slot->type==nest->type &&
			slot->status != 0 && slot->freezeCount==0) {
			_currentScript = nest->slot;
			getScriptBaseAddress();
			getScriptEntryPoint();
			return;
		}
	}
	_currentScript = 0xFF;
}

void Scumm::updateScriptPtr() {
	if (_currentScript == 0xFF)
		return;

	vm.slot[_currentScript].offs = _scriptPointer - _scriptOrgPointer;
}

void Scumm::getScriptBaseAddress() {
	ScriptSlot *ss;
	int index;

	if (_currentScript == 0xFF)
		return;

	ss = &vm.slot[_currentScript];
	switch(ss->type) {
	case 0: /* inventory script **/
		index = getObjectIndex(ss->number);
		_scriptOrgPointer = getResourceAddress(5, index);
		_lastCodePtr = &_baseInventoryItems[index];
		break;

	case 3:
	case 1: /* room script */
		_scriptOrgPointer = getResourceAddress(1, _roomResource);
		_lastCodePtr = &_baseRooms[_roomResource];
		break;

	case 2: /* global script */
		_scriptOrgPointer = getResourceAddress(2, ss->number);
		_lastCodePtr = &_baseScripts[ss->number];
		break;

	case 4: /* flobject script */
		index = getObjectIndex(ss->number);
		_scriptOrgPointer = getResourceAddress(13,objs[index].fl_object_index);
		_lastCodePtr = &_baseFLObject[ss->number];
		break;
	default:
		error("Bad type while getting base address");
	}
}


void Scumm::getScriptEntryPoint() {
	if (_currentScript == 0xFF)
		return;
	_scriptPointer = _scriptOrgPointer + vm.slot[_currentScript].offs;
}

OpcodeProc FORCEINLINE Scumm::getOpcode(int i) {
	static const OpcodeProc opcode_list[] = {
	/* 00 */
	&Scumm::o_stopObjectCode,
	&Scumm::o_putActor,
	&Scumm::o_startMusic,
	&Scumm::o_getActorRoom,
	/* 04 */
	&Scumm::o_isGreaterEqual, /* hmm, seems to be less or equal */
	&Scumm::o_drawObject,
	&Scumm::o_getActorElevation,
	&Scumm::o_setState,
	/* 08 */
	&Scumm::o_isNotEqual,
	&Scumm::o_faceActor,
	&Scumm::o_startScript,
	&Scumm::o_getVerbEntrypoint,
	/* 0C */
	&Scumm::o_resourceRoutines,
	&Scumm::o_walkActorToActor,
	&Scumm::o_putActorAtObject,
	&Scumm::o_getObjectState,
	/* 10 */
	&Scumm::o_getObjectOwner,
	&Scumm::o_animateActor,
	&Scumm::o_panCameraTo,
	&Scumm::o_actorSet,
	/* 14 */
	&Scumm::o_print,
	&Scumm::o_actorFromPos,
	&Scumm::o_getRandomNr,
	&Scumm::o_and,
	/* 18 */
	&Scumm::o_jumpRelative,
	&Scumm::o_doSentence,
	&Scumm::o_move,
	&Scumm::o_multiply,
	/* 1C */
	&Scumm::o_startSound,
	&Scumm::o_ifClassOfIs,
	&Scumm::o_walkActorTo,
	&Scumm::o_isActorInBox,
	/* 20 */
	&Scumm::o_stopMusic,
	&Scumm::o_putActor,
	&Scumm::o_getAnimCounter,
	&Scumm::o_getActorY,
	/* 24 */
	&Scumm::o_loadRoomWithEgo,
	&Scumm::o_pickupObject,
	&Scumm::o_setVarRange,
	&Scumm::o_stringOps,
	/* 28 */
	&Scumm::o_equalZero,
	&Scumm::o_setOwnerOf,
	&Scumm::o_startScript,
	&Scumm::o_delayVariable,
	/* 2C */
	&Scumm::o_cursorCommand,
	&Scumm::o_putActorInRoom,
	&Scumm::o_delay,
	&Scumm::o_badOpcode,
	/* 30 */
	&Scumm::o_matrixOps,
	&Scumm::o_getInventoryCount,
	&Scumm::o_setCameraAt,
	&Scumm::o_roomOps,
	/* 34 */
	&Scumm::o_getDist,
	&Scumm::o_findObject,
	&Scumm::o_walkActorToObject,
	&Scumm::o_startObject,
	/* 38 */
	&Scumm::o_lessOrEqual,
	&Scumm::o_doSentence,
	&Scumm::o_subtract,
	&Scumm::o_getActorScale,
	/* 3C */
	&Scumm::o_stopSound,
	&Scumm::o_findInventory,
	&Scumm::o_walkActorTo,
	&Scumm::o_drawBox,
	/* 40 */
	&Scumm::o_cutscene,
	&Scumm::o_putActor,
	&Scumm::o_chainScript,
	&Scumm::o_getActorX,
	/* 44 */
	&Scumm::o_isLess,
	&Scumm::o_badOpcode,
	&Scumm::o_increment,
	&Scumm::o_setState,
	/* 48 */
	&Scumm::o_isEqual,
	&Scumm::o_faceActor,
	&Scumm::o_startScript,
	&Scumm::o_getVerbEntrypoint,
	/* 4C */
	&Scumm::o_soundKludge,
	&Scumm::o_walkActorToActor,
	&Scumm::o_putActorAtObject,
	&Scumm::o_badOpcode,
	/* 50 */
	&Scumm::o_badOpcode,
	&Scumm::o_animateActor,
	&Scumm::o_actorFollowCamera,
	&Scumm::o_actorSet,
	/* 54 */
	&Scumm::o_setObjectName,
	&Scumm::o_actorFromPos,
	&Scumm::o_getActorMoving,
	&Scumm::o_or,
	/* 58 */
	&Scumm::o_overRide,
	&Scumm::o_doSentence,
	&Scumm::o_add,
	&Scumm::o_divide,
	/* 5C */
	&Scumm::o_badOpcode,
	&Scumm::o_actorSetClass,
	&Scumm::o_walkActorTo,
	&Scumm::o_isActorInBox,
	/* 60 */
	&Scumm::o_freezeScripts,
	&Scumm::o_putActor,
	&Scumm::o_stopScript,
	&Scumm::o_getActorFacing,
	/* 64 */
	&Scumm::o_loadRoomWithEgo,
	&Scumm::o_pickupObject,
	&Scumm::o_getClosestObjActor,
	&Scumm::o_dummy,
	/* 68 */
	&Scumm::o_getScriptRunning,
	&Scumm::o_setOwnerOf,
	&Scumm::o_startScript,
	&Scumm::o_debug,
	/* 6C */
	&Scumm::o_getActorWidth,
	&Scumm::o_putActorInRoom,
	&Scumm::o_stopObjectScript,
	&Scumm::o_badOpcode,
	/* 70 */
	&Scumm::o_lights,
	&Scumm::o_getActorCostume,
	&Scumm::o_loadRoom,
	&Scumm::o_roomOps,
	/* 74 */
	&Scumm::o_getDist,
	&Scumm::o_findObject,
	&Scumm::o_walkActorToObject,
	&Scumm::o_startObject,
	/* 78 */
	&Scumm::o_isGreater, /* less? */
	&Scumm::o_doSentence,
	&Scumm::o_verbOps,
	&Scumm::o_getActorWalkBox,
	/* 7C */
	&Scumm::o_isSoundRunning,
	&Scumm::o_findInventory,
	&Scumm::o_walkActorTo,
	&Scumm::o_drawBox,
	/* 80 */
	&Scumm::o_breakHere,
	&Scumm::o_putActor,
	&Scumm::o_startMusic,
	&Scumm::o_getActorRoom,
	/* 84 */
	&Scumm::o_isGreaterEqual, /* less equal? */
	&Scumm::o_drawObject,
	&Scumm::o_getActorElevation,
	&Scumm::o_setState,
	/* 88 */
	&Scumm::o_isNotEqual,
	&Scumm::o_faceActor,
	&Scumm::o_startScript,
	&Scumm::o_getVerbEntrypoint,
	/* 8C */
	&Scumm::o_resourceRoutines,
	&Scumm::o_walkActorToActor,
	&Scumm::o_putActorAtObject,
	&Scumm::o_getObjectState,
	/* 90 */
	&Scumm::o_getObjectOwner,
	&Scumm::o_animateActor,
	&Scumm::o_panCameraTo,
	&Scumm::o_actorSet,
	/* 94 */
	&Scumm::o_print,
	&Scumm::o_actorFromPos,
	&Scumm::o_getRandomNr,
	&Scumm::o_and,
	/* 98 */
	&Scumm::o_quitPauseRestart,
	&Scumm::o_doSentence,
	&Scumm::o_move,
	&Scumm::o_multiply,
	/* 9C */
	&Scumm::o_startSound,
	&Scumm::o_ifClassOfIs,
	&Scumm::o_walkActorTo,
	&Scumm::o_isActorInBox,
	/* A0 */
	&Scumm::o_stopObjectCode,
	&Scumm::o_putActor,
	&Scumm::o_getAnimCounter,
	&Scumm::o_getActorY,
	/* A4 */
	&Scumm::o_loadRoomWithEgo,
	&Scumm::o_pickupObject,
	&Scumm::o_setVarRange,
	&Scumm::o_dummy,
	/* A8 */
	&Scumm::o_notEqualZero,
	&Scumm::o_setOwnerOf,
	&Scumm::o_startScript,
	&Scumm::o_saveRestoreVerbs,
	/* AC */
	&Scumm::o_expression,
	&Scumm::o_putActorInRoom,
	&Scumm::o_wait,
	&Scumm::o_badOpcode,
	/* B0 */
	&Scumm::o_matrixOps,
	&Scumm::o_getInventoryCount,
	&Scumm::o_setCameraAt,
	&Scumm::o_roomOps,
	/* B4 */
	&Scumm::o_getDist,
	&Scumm::o_findObject,
	&Scumm::o_walkActorToObject,
	&Scumm::o_startObject,
	/* B8 */
	&Scumm::o_lessOrEqual,
	&Scumm::o_doSentence,
	&Scumm::o_subtract,
	&Scumm::o_getActorScale,
	/* BC */
	&Scumm::o_stopSound,
	&Scumm::o_findInventory,
	&Scumm::o_walkActorTo,
	&Scumm::o_drawBox,
	/* C0 */
	&Scumm::o_endCutscene,
	&Scumm::o_putActor,
	&Scumm::o_chainScript,
	&Scumm::o_getActorX,
	/* C4 */
	&Scumm::o_isLess,
	&Scumm::o_badOpcode,
	&Scumm::o_decrement,
	&Scumm::o_setState,
	/* C8 */
	&Scumm::o_isEqual,
	&Scumm::o_faceActor,
	&Scumm::o_startScript,
	&Scumm::o_getVerbEntrypoint,
	/* CC */
	&Scumm::o_pseudoRoom,
	&Scumm::o_walkActorToActor,
	&Scumm::o_putActorAtObject,
	&Scumm::o_badOpcode,
	/* D0 */
	&Scumm::o_badOpcode,
	&Scumm::o_animateActor,
	&Scumm::o_actorFollowCamera,
	&Scumm::o_actorSet,
	/* D4 */
	&Scumm::o_setObjectName,
	&Scumm::o_actorFromPos,
	&Scumm::o_getActorMoving,
	&Scumm::o_or,
	/* D8 */
	&Scumm::o_printEgo,
	&Scumm::o_doSentence,
	&Scumm::o_add,
	&Scumm::o_divide,
	/* DC */
	&Scumm::o_badOpcode,
	&Scumm::o_actorSetClass,
	&Scumm::o_walkActorTo,
	&Scumm::o_isActorInBox,
	/* E0 */
	&Scumm::o_freezeScripts,
	&Scumm::o_putActor,
	&Scumm::o_stopScript,
	&Scumm::o_getActorFacing,
	/* E4 */
	&Scumm::o_loadRoomWithEgo,
	&Scumm::o_pickupObject,
	&Scumm::o_getClosestObjActor,
	&Scumm::o_dummy,
	/* E8 */
	&Scumm::o_getScriptRunning,
	&Scumm::o_setOwnerOf,
	&Scumm::o_startScript,
	&Scumm::o_debug,
	/* EC */
	&Scumm::o_getActorWidth,
	&Scumm::o_putActorInRoom,
	&Scumm::o_stopObjectScript,
	&Scumm::o_badOpcode,
	/* F0 */
	&Scumm::o_lights,
	&Scumm::o_getActorCostume,
	&Scumm::o_loadRoom,
	&Scumm::o_roomOps,
	/* F4 */
	&Scumm::o_getDist,
	&Scumm::o_findObject,
	&Scumm::o_walkActorToObject,
	&Scumm::o_startObject,
	/* F8 */
	&Scumm::o_isGreater,
	&Scumm::o_doSentence,
	&Scumm::o_verbOps,
	&Scumm::o_getActorWalkBox,
	/* FC */
	&Scumm::o_isSoundRunning,
	&Scumm::o_findInventory,
	&Scumm::o_walkActorTo,
	&Scumm::o_drawBox
	};

	return opcode_list[i];
}


void Scumm::executeScript() {
	OpcodeProc op;
	while (_currentScript != 0xFF) {
		_opcode = fetchScriptByte();
		_scriptPointerStart = _scriptPointer;
		vm.slot[_currentScript].didexec = 1;
		debug(9, "%X", _opcode);
		op = getOpcode(_opcode);
		(this->*op)();
	}
	checkHeap();
}

byte Scumm::fetchScriptByte() {
	if (*_lastCodePtr != _scriptOrgPointer + 6) {
		uint32 oldoffs = _scriptPointer - _scriptOrgPointer;
		getScriptBaseAddress();
		_scriptPointer = _scriptOrgPointer + oldoffs;
	}
	return *_scriptPointer++;
}

int Scumm::fetchScriptWord() {
	int a;

	if (*_lastCodePtr != _scriptOrgPointer + 6) {
		uint32 oldoffs = _scriptPointer - _scriptOrgPointer;
		getScriptBaseAddress();
		_scriptPointer = _scriptOrgPointer + oldoffs;
	}
	
	a = READ_LE_UINT16(_scriptPointer);
	_scriptPointer += 2;

	debug(9, "fetchword=%d", a);
	return a;
}

void Scumm::ignoreScriptWord() {
	fetchScriptWord();
}

void Scumm::ignoreScriptByte() {
	fetchScriptByte();
}

int Scumm::getVarOrDirectWord(byte mask) {
	if (_opcode&mask)
		return readVar(fetchScriptWord());
	return (int16)fetchScriptWord();
}

int Scumm::getVarOrDirectByte(byte mask) {
	if (_opcode&mask)
		return readVar(fetchScriptWord());
	return fetchScriptByte();
}

int Scumm::readVar(uint var) {
	int a;
#ifdef BYPASS_COPY_PROT
	static byte copyprotbypassed;
#endif
	debug(9, "readvar=%d", var);
	if (!(var&0xF000)) {
		checkRange(0x31F, 0, var, "Variable %d out of range(r)");
		return vm.vars[var];
	}

	if (var&0x2000) {
		a = fetchScriptWord();
		if (a&0x2000)
			var = (var+readVar(a&~0x2000))&~0x2000;
		else
			var = (var+(a&0xFFF))&~0x2000;
	}

	if (!(var&0xF000))
		return vm.vars[var];

	if (var&0x8000) {
		var &= 0xFFF;
		checkRange(0x7FF, 0, var, "Bit variable %d out of range(r)");
		return (vm.bitvars[var>>3] & (1<<(var&7))) ? 1 : 0;
	}

	if (var&0x4000) {
		var &= 0xFFF;
		checkRange(0x10, 0, var, "Local variable %d out of range(r)");

#ifdef BYPASS_COPY_PROT
		if (!copyprotbypassed && _currentScript==1) {
			copyprotbypassed=1;
			return 1;
		}
#endif
		return vm.localvar[_currentScript * 17 + var];
	}

	error("Illegal varbits (r)");
}

void Scumm::getResultPos() {
	int a;

	_resultVarNumber = fetchScriptWord();
	if (_resultVarNumber&0x2000) {
		a = fetchScriptWord();
		if (a&0x2000) {
			_resultVarNumber += readVar(a&~0x2000);
		} else {
			_resultVarNumber += a&0xFFF;
		}
		_resultVarNumber&=~0x2000;
	}

	debug(9, "getResultPos=%d", _resultVarNumber);
}

void Scumm::setResult(int value) {
	int var = _resultVarNumber;
	debug(9, "setResult %d,%d", var,value);

	if (!(var&0xF000)) {
		checkRange(0x31F, 0, var, "Variable %d out of range(w)");
		vm.vars[var] = value;

		if (var==518) {
			printf("The answer is %d\n", value);
		}
		return;
	}

	if(var&0x8000) {
		var&=0xFFF;
		checkRange(0x7FF, 0, var, "Bit variable %d out of range(w)");
		if (value)
			vm.bitvars[var>>3] |= (1<<(var&7));
		else
			vm.bitvars[var>>3] &= ~(1<<(var&7));
		return;
	}

	if (var&0x4000) {
		var &= 0xFFF;
		checkRange(0x10, 0, var, "Local variable %d out of range(w)");
		vm.localvar[_currentScript * 17 + var] = value;
		return;
	}
	error("Illegal varbits (w)");
}

void Scumm::o_actorFollowCamera() {
	int a = camera._follows;

	setCameraFollows(derefActorSafe(getVarOrDirectByte(0x80), "actorFollowCamera"));

	if (camera._follows != a) 
		runHook(0);

	camera._movingToActor = 0;
}

void Scumm::o_actorFromPos() {
	int x,y;
	getResultPos();
	x = getVarOrDirectWord(0x80);
	y = getVarOrDirectWord(0x40);
	setResult(getActorFromPos(x,y));
}

void Scumm::o_actorSet() {
	int act = getVarOrDirectByte(0x80);
	Actor *a = derefActorSafe(act, "actorSet");
	int i,j;

	while ( (_opcode = fetchScriptByte()) != 0xFF) {
		switch(_opcode&0x1F) {
		case 1: /* costume */
			setActorCostume(a, getVarOrDirectByte(0x80));
			break;
		case 2: /* walkspeed */
			i = getVarOrDirectByte(0x80);
			j = getVarOrDirectByte(0x40);
			setActorWalkSpeed(a, i, j);
			break;
		case 3: /* sound */
			a->sound = getVarOrDirectByte(0x80);
			break;
		case 4: /* walkanim */
			a->walkFrame = getVarOrDirectByte(0x80);
			break;
		case 5: /* talkanim */
			a->talkFrame1 = getVarOrDirectByte(0x80);
			a->talkFrame2 = getVarOrDirectByte(0x40);
			break;
		case 6: /* standanim */
			a->standFrame = getVarOrDirectByte(0x80);
			break;
		case 7: /* ignore */
			getVarOrDirectByte(0x80);
			getVarOrDirectByte(0x40);
			getVarOrDirectByte(0x20);
			break;
		case 8: /* init */
			initActor(a, 0);
			break;
		case 9: /* elevation */
			a->elevation = getVarOrDirectWord(0x80);
			a->needRedraw = true;
			a->needBgReset = true;
			break;
		case 10: /* defaultanims */
			a->initFrame = 1;
			a->walkFrame = 2;
			a->standFrame = 3;
			a->talkFrame1 = 4;
			a->talkFrame2 = 4;
			break;
		case 11: /* palette */
			i = getVarOrDirectByte(0x80);
			j = getVarOrDirectByte(0x40);
			checkRange(32, 0, i, "Illegal palet slot %d");
			a->palette[i] = j;
			a->needRedraw = 1;
			break;
		case 12: /* talk color */
			a->talkColor = getVarOrDirectByte(0x80);
			break;
		case 13: /* name */
			loadPtrToResource(9, a->number, NULL);
			break;
		case 14: /* initanim */
			a->initFrame = getVarOrDirectByte(0x80);
			break;
		case 15: /* unk */
			error("o_actorset:unk not implemented");
			break;
		case 16: /* width */
			a->width = getVarOrDirectByte(0x80);
			break;
		case 17: /* scale */
			a->scalex = getVarOrDirectByte(0x80);
			a->scaley = getVarOrDirectByte(0x40);
			break;
		case 18: /* neverzclip */
			a->neverZClip = 0;
			break;
		case 19: /* setzclip */
			a->neverZClip = getVarOrDirectByte(0x80);
			break;
		case 20: /* ignoreboxes */
			a->ignoreBoxes = 1;
			a->neverZClip = 0;
FixRoom:
			if (a->room==_currentRoom)
				putActor(a, a->x, a->y, a->room);
			break;
		case 21: /* followboxes */
			a->ignoreBoxes = 0;
			a->neverZClip = 0;
			goto FixRoom;

		case 22: /* animspeed */
			a->animSpeed = getVarOrDirectByte(0x80);
			break;
		case 23: /* unk2 */
			a->data8 = getVarOrDirectByte(0x80); /* unused */
			break;
		default:
			error("o_actorSet: default case");
		}
	}
}

void Scumm::o_actorSetClass() {
	int act = getVarOrDirectWord(0x80);
	int i;

	while ( (_opcode=fetchScriptByte()) != 0xFF) {
		i = getVarOrDirectWord(0x80);
		if (i==0) {
			_classData[act] = 0;
			continue;
		}
		if (i&0x80)
			putClass(act, i, 1);
		else
			putClass(act, i, 0);
	}
}

void Scumm::o_add() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) + a);
}

void Scumm::o_and() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) & a);
}

void Scumm::o_animateActor() {
	int anim,shr,dir;
	bool inRoom;
	Actor *a;

	a = derefActorSafe(getVarOrDirectByte(0x80), "animateActor");
	anim = getVarOrDirectByte(0x40);

	shr = anim>>2;
	dir = anim&3;

	inRoom = (a->room == _currentRoom);

	if (shr == 0x3F) {
		if (inRoom) {
			startAnimActor(a, a->standFrame, a->facing);
			a->moving = 0;
		}
		return;
	}

	if (shr == 0x3E) {
		if (inRoom) {
			startAnimActor(a, 0x3E, dir);
			a->moving &= ~4;
		}
		a->facing = dir;
		return;
	}

	if (shr == 0x3D) {
		if (inRoom) {
			turnToDirection(a, dir);
		} else {
			a->facing = dir;
		}
		return;
	}

	startAnimActor(a, anim, a->facing);
}

void Scumm::o_badOpcode() {
	error("Scumm opcode %d illegal", _opcode);
}

void Scumm::o_breakHere() {
	updateScriptPtr();
	_currentScript = 0xFF;
}

void Scumm::o_chainScript() {
	int16 vars[16];
	int data;
	int cur;

	data = getVarOrDirectByte(0x80);

	getWordVararg(vars);

	cur = _currentScript;

	if (vm.slot[cur].cutsceneOverride != 0) {
		error("Script %d chaining with active cutscene/override");
	}

	vm.slot[cur].number = 0;
	vm.slot[cur].status = 0;
	_currentScript = 0xFF;

	runScript(data, vm.slot[cur].unk1, vm.slot[cur].unk2, vars);
}

void Scumm::o_cursorCommand() {
	int i,j,k;
	int16 table[16];

	switch((_opcode=fetchScriptByte())&0x1F) {
	case 1: /* cursor show */
		_cursorState = 1;
		verbMouseOver(0);
		break;
	case 2: /* cursor hide */
		_cursorState = 0;
		verbMouseOver(0);
		break;
	case 3: /* userput on */
		_userPut = 1;
		break;
	case 4: /* userput off */
		_userPut = 0;
		break;
	case 5: /* cursor soft on */
		_cursorState++;
		if (_cursorState > 1) {
			error("Cursor state greater than 1 in script");
		}
		break;
	case 6: /* cursor soft off */
		_cursorState--;
		break;
	case 7: /* userput soft on */
		_userPut++;
		break;
	case 8: /* userput soft off */
		_userPut--;
		break;
	case 10: /* set cursor img */
		i = getVarOrDirectByte(0x80);
		j = getVarOrDirectByte(0x40);
		setCursorImg(i, j);
		break;
	case 11: /* set cursor hotspot */
		i = getVarOrDirectByte(0x80);
		j = getVarOrDirectByte(0x40);
		k = getVarOrDirectByte(0x20);
		setCursorHotspot(i, j, k);
		break;

	case 12: /* init cursor */
		setCursor(getVarOrDirectByte(0x80));
		break;
	case 13: /* init charset */
		initCharset(getVarOrDirectByte(0x80));
		break;
	case 14: /* unk */
		getWordVararg(table);
		for (i=0; i<16; i++)
			charset._colorMap[i] = _charsetData[textslot.charset[1]][i] = table[i];
		break;
	}

	vm.vars[VAR_CURSORSTATE] = _cursorState;
	vm.vars[VAR_USERPUT] = _userPut;
}

void Scumm::o_cutscene() {
	int scr = _currentScript;

	getWordVararg(_vararg_temp_pos);
	
	vm.slot[scr].cutsceneOverride++;
	
	if (++vm.cutSceneStackPointer > 5)
		error("Cutscene stack overflow");

	vm.cutSceneData[vm.cutSceneStackPointer] = _vararg_temp_pos[0];
	vm.cutSceneScript[vm.cutSceneStackPointer] = 0;
	vm.cutScenePtr[vm.cutSceneStackPointer] = 0;

	vm.cutSceneScriptIndex = scr;
	if (vm.vars[VAR_CUTSCENE_START_SCRIPT])
		runScript(vm.vars[VAR_CUTSCENE_START_SCRIPT], 0, 0, _vararg_temp_pos);
	vm.cutSceneScriptIndex = 0xFF;
}

void Scumm::o_endCutscene() {
	ScriptSlot *ss = &vm.slot[_currentScript];
	uint32 *csptr;
	
	ss->cutsceneOverride--;

	_vararg_temp_pos[0] = vm.cutSceneData[vm.cutSceneStackPointer];
	vm.vars[VAR_OVERRIDE] = 0;

	csptr = &vm.cutScenePtr[vm.cutSceneStackPointer];
	if (*csptr)
		ss->cutsceneOverride--;

	vm.cutSceneScript[vm.cutSceneStackPointer] = 0;
	*csptr = 0;
	vm.cutSceneStackPointer--;

	if (vm.vars[VAR_CUTSCENE_END_SCRIPT])
		runScript(vm.vars[VAR_CUTSCENE_END_SCRIPT], 0, 0, _vararg_temp_pos);
}


void Scumm::o_debug() {
	getVarOrDirectWord(0x80);
}

void Scumm::o_decrement() {
	getResultPos();
	setResult(readVar(_resultVarNumber)-1);
}

void Scumm::o_delay() {
	int delay = fetchScriptByte();
	delay |= fetchScriptByte()<<8;
	delay |= fetchScriptByte()<<16;
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = 1;
	o_breakHere();
}

void Scumm::o_delayVariable() {
	vm.slot[_currentScript].delay = readVar(fetchScriptWord());
	vm.slot[_currentScript].status = 1;
	o_breakHere();
}

void Scumm::o_divide() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	if(a==0) {
		error("Divide by zero");
		setResult(0);
	} else 
		setResult(readVar(_resultVarNumber) / a);
}

void Scumm::o_doSentence() {
	int a,b;
	_sentenceIndex++;

	a = getVarOrDirectByte(0x80);
	if (a==0xFE) {
		_sentenceIndex = 0xFF;
		stopScriptNr(vm.vars[VAR_SENTENCE_SCRIPT]);
		clearClickedStatus();
		return;
	}
	_sentenceTab5[_sentenceIndex] = a;
	_sentenceTab4[_sentenceIndex] = getVarOrDirectWord(0x40);
	b = _sentenceTab3[_sentenceIndex] = getVarOrDirectWord(0x20);
	if (b==0) {
		_sentenceTab2[_sentenceIndex] = 0;
	} else {
		_sentenceTab2[_sentenceIndex] = 1;
	}
	_sentenceTab[_sentenceIndex] = 0;
}

void Scumm::o_drawBox() {
	int x,y,x2,y2,color;

	x = getVarOrDirectWord(0x80);
	y = getVarOrDirectWord(0x40);

	_opcode = fetchScriptByte();
	x2 = getVarOrDirectWord(0x80);
	y2 = getVarOrDirectWord(0x40);
	color = getVarOrDirectByte(0x20);

	drawBox(x, y, x2, y2, color);
}

void Scumm::drawBox(int x, int y, int x2, int y2, int color) {
	int top,bottom,count;

	if (findVirtScreen(y)==-1)
		return;

	top = virtscr[gdi.virtScreen].topline;
	bottom = top + virtscr[gdi.virtScreen].height;

	if (x > x2)
		SWAP(x,x2);

	if (y > y2)
		SWAP(y,y2);

	x2++;
	y2++;

	if (x>319) return;
	if (x<0) x=0;
	if (x2<0) return;
	if (x2>320) x2=320;
	if (y2 > bottom) y2=bottom;

	updateDirtyRect(gdi.virtScreen, x, x2, y-top, y2-top, 0);

	gdi.bg_ptr = getResourceAddress(0xA, gdi.virtScreen+1) 
		+ virtscr[gdi.virtScreen].xstart
		+ (y-top)*320 + x;

	count = y2 - y;
	while (count) {
		memset(gdi.bg_ptr, color, x2 - x);
		gdi.bg_ptr += 320;
		count--;
	}
}

void Scumm::o_drawObject() {
	int state,obj,index,i;
	ObjectData *od;
	byte x,y,w,h;

	state = 1;
	_xPos = _yPos = 255;
	obj = getVarOrDirectWord(0x80);

	switch((_opcode = fetchScriptByte())&0x1F) {
	case 1: /* draw at */
		_xPos = getVarOrDirectWord(0x80);
		_yPos = getVarOrDirectWord(0x40);
		break;
	case 2: /* set state */
		state = getVarOrDirectWord(0x80);
		break;
	case 0x1F: /* neither */
		break;
	default:
		error("o_drawObject: default case");
	}
	index = getObjectIndex(obj);
	if (index==-1)
		return;
	od = &objs[index];
	if (_xPos!=0xFF) {
		od->cdhd_10 += (_xPos - od->x_pos)<<3;
		od->x_pos = _xPos;
		od->cdhd_12 += (_yPos - od->y_pos)<<3;
		od->y_pos = _yPos;
	}
	addObjectToDrawQue(index);

	x = od->x_pos;
	y = od->y_pos;
	w = od->numstrips;
	h = od->height;

	i = _numObjectsInRoom;
	do {
		if (objs[i].x_pos == x && objs[i].y_pos == y
			&& objs[i].numstrips == w && objs[i].height==h) 
			putState(objs[i].obj_nr, 0);
	} while (--i);

	putState(obj, state);
}

void Scumm::o_dummy() {
	/* nothing */
}


void Scumm::o_expression() {
	int dst, i;

	_scummStackPos = 0;
	getResultPos();
	dst = _resultVarNumber;

	while ((_opcode = fetchScriptByte())!=0xFF) {
		switch(_opcode&0x1F) {
		case 1: /* varordirect */
			stackPush(getVarOrDirectWord(0x80));
			break;
		case 2: /* add */
			i = stackPop();
			stackPush(i + stackPop());
			break;
		case 3: /* sub */
			i = stackPop();
			stackPush(stackPop() - i);
			break;
		case 4: /* mul */
			i = stackPop();
			stackPush(i * stackPop());
			break;
		case 5: /* div */
			i = stackPop();
			if (i==0)
				error("Divide by zero");
			stackPush(stackPop() / i);
			break;
		case 6: /* normal opcode */
			_opcode = fetchScriptByte();
			(this->*(getOpcode(_opcode)))();
			stackPush(vm.vars[0]);
			break;
		}
	}

	_resultVarNumber = dst;
	setResult(stackPop());
}

void Scumm::o_faceActor() {
	int act, obj;
	int x;
	byte dir;

	act = getVarOrDirectByte(0x80);
	obj = getVarOrDirectWord(0x40);

	if (getObjectOrActorXY(act)==-1)
		return;

	x = _xPos;

	if (getObjectOrActorXY(obj)==-1)
		return;
	
	dir = (_xPos > x) ? 1 : 0;
	turnToDirection(derefActorSafe(act, "o_faceActor"), dir);
}

void Scumm::o_findInventory() {
	int owner, b, count, i, obj;

	getResultPos();
	owner = getVarOrDirectByte(0x80);
	b = getVarOrDirectByte(0x40);
	count = 1;
	for (i=0; i!=_maxInventoryItems; i++) {
		obj = _inventory[i];
		if (obj && getOwner(obj)==owner && count++ == b) {
			setResult(obj);
			return;
		}
	}
	setResult(0);
}

void Scumm::o_findObject() {
	int t;
	getResultPos();
	t = getVarOrDirectWord(0x80);
	setResult(findObject(t, getVarOrDirectWord(0x40)));
}

void Scumm::o_freezeScripts() {
	int scr = getVarOrDirectByte(0x80);

	if (scr!=0)
		freezeScripts(scr);
	else
		unfreezeScripts();
}

void Scumm::o_getActorCostume() {
	getResultPos();
	setResult(derefActorSafe(getVarOrDirectByte(0x80),"o_getActorCostume")->costume);
}

void Scumm::o_getActorElevation() {
	getResultPos();
	setResult(derefActorSafe(getVarOrDirectByte(0x80),"o_getActorElevation")->elevation);
}

void Scumm::o_getActorFacing() {
	getResultPos();
	setResult(derefActorSafe(getVarOrDirectByte(0x80),"o_getActorFacing")->facing);
}

void Scumm::o_getActorMoving() {
	getResultPos();
	setResult(derefActorSafe(getVarOrDirectByte(0x80),"o_getActorMoving")->moving);
}

void Scumm::o_getActorRoom() {
	getResultPos();
	setResult(derefActorSafe(getVarOrDirectByte(0x80),"o_getActorRoom")->room);
}

void Scumm::o_getActorScale() {
	getResultPos();
	setResult(derefActorSafe(getVarOrDirectByte(0x80),"o_getActorScale")->scalex);
}

void Scumm::o_getActorWalkBox() {
	getResultPos();
	setResult(derefActorSafe(getVarOrDirectByte(0x80),"o_getActorWalkbox")->walkbox);
}

void Scumm::o_getActorWidth() {
	getResultPos();
	setResult(derefActorSafe(getVarOrDirectByte(0x80),"o_getActorWidth")->width);
}

void Scumm::o_getActorX() {
	int index;
	getResultPos();
	index = getVarOrDirectWord(0x80);
	if (index <= vm.vars[VAR_NUM_ACTOR]) {
		setResult(derefActorSafe(index,"o_getActorX")->x);
	} else {
		if (whereIsObject(index)==-1)
			setResult(-1);
		else {
			getObjectOrActorXY(index);
			setResult(_xPos);
		}
	}
}

void Scumm::o_getActorY() {
	int index;
	getResultPos();
	index = getVarOrDirectWord(0x80);
	if (index <= vm.vars[VAR_NUM_ACTOR]) {
		setResult(derefActorSafe(index,"o_getActorY")->y);
	} else {
		if (whereIsObject(index)==-1)
			setResult(-1);
		else {
			getObjectOrActorXY(index);
			setResult(_yPos);
		}
	}
}

void Scumm::o_getAnimCounter() {
	getResultPos();
	setResult(derefActorSafe(getVarOrDirectByte(0x80),"o_getActorAnimCounter")->cost.animCounter1);
}

void Scumm::o_getClosestObjActor() {
	int obj;
	int act;
	int closobj=-1, closnum=-1;
	int dist;

	getResultPos();

	act = getVarOrDirectWord(0x80);
	obj = vm.vars[VAR_OBJECT_HI];

	do {
		dist = getObjActToObjActDist(obj,act);
		if (dist < closnum) {
			closnum = dist;
			closobj = obj;
		}
	} while (--obj >= vm.vars[VAR_OBJECT_LO]);

	setResult(closnum);
}

void Scumm::o_getDist() {
	int o1,o2;
	getResultPos();
	o1 = getVarOrDirectWord(0x80);
	o2 = getVarOrDirectWord(0x40);
	setResult(getObjActToObjActDist(o1,o2));
}

void Scumm::o_getInventoryCount() {
	int owner, count, i, obj;

	getResultPos();

	owner = getVarOrDirectByte(0x80);
	count = 0;
	for (i=0; i!=_maxInventoryItems; i++) {
		obj = _inventory[i];
		if (obj && getOwner(obj)==owner)
			count++;
	}
	setResult(count);
}

void Scumm::o_getObjectOwner() {
	getResultPos();
	setResult(getOwner(getVarOrDirectWord(0x80)));
}

void Scumm::o_getObjectState() {
	getResultPos();
	setResult(getState(getVarOrDirectWord(0x80)));
}

void Scumm::o_getRandomNr() {
	getResultPos();
	setResult(getRandomNumber(getVarOrDirectByte(0x80)));
}

void Scumm::o_getScriptRunning() {
	int i;
	ScriptSlot *ss;
	int script;

	getResultPos();
	script = getVarOrDirectByte(0x80);

	ss = vm.slot;
	for (i=0; i<20; i++,ss++) {
		if (ss->number==script && (ss->type==2 || ss->type==3) && ss->status) {
			setResult(1);
			return;
		}
	}
	setResult(0);
}

void Scumm::o_getVerbEntrypoint() {
	int a,b;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	b = getVarOrDirectWord(0x40);
	setResult(getVerbEntrypoint(a, b));
}

void Scumm::o_ifClassOfIs() {
	int act,cls;
	bool cond = true, b;

	act = getVarOrDirectWord(0x80);
	while ( (_opcode = fetchScriptByte()) != 0xFF) {
		cls = getVarOrDirectWord(0x80);
		b = getClass(act, cls);

		if (cls&0x80 && !b)
			cond = false;
		if (!(cls&0x80) && b)
			cond = false;
	}
	if (cond)
		ignoreScriptWord();
	else
		o_jumpRelative();
}

void Scumm::o_increment() {
	getResultPos();
	setResult(readVar(_resultVarNumber)+1);
}

void Scumm::o_isActorInBox() {
	int box;
	Actor *a;

	a = derefActorSafe(getVarOrDirectByte(0x80), "o_isActorInBox");
	box = getVarOrDirectByte(0x40);

	if (!checkXYInBoxBounds(box, a->x, a->y))
		o_jumpRelative();
	else
		ignoreScriptWord();
}

void Scumm::o_isEqual() {
	int16 a = readVar(fetchScriptWord());
	int16 b = getVarOrDirectWord(0x80);
	if (b == a) ignoreScriptWord();
	else o_jumpRelative();

}

void Scumm::o_isGreater() {
	int16 a = readVar(fetchScriptWord());
	int16 b = getVarOrDirectWord(0x80);
	if (b > a) ignoreScriptWord();
	else o_jumpRelative();
}

void Scumm::o_isGreaterEqual() {
	int16 a = readVar(fetchScriptWord());
	int16 b = getVarOrDirectWord(0x80);
	if (b >= a) ignoreScriptWord();
	else o_jumpRelative();
}

void Scumm::o_isLess() {
	int16 a = readVar(fetchScriptWord());
	int16 b = getVarOrDirectWord(0x80);
	if (b < a) ignoreScriptWord();
	else o_jumpRelative();
}

void Scumm::o_lessOrEqual() {
	int16 a = readVar(fetchScriptWord());
	int16 b = getVarOrDirectWord(0x80);
	if (b <= a) ignoreScriptWord();
	else o_jumpRelative();
}

void Scumm::o_isNotEqual() {
	int16 a = readVar(fetchScriptWord());
	int16 b = getVarOrDirectWord(0x80);
	if (b != a) ignoreScriptWord();
	else o_jumpRelative();
}

void Scumm::o_notEqualZero() {
	int a = readVar(fetchScriptWord());
	if (a != 0) ignoreScriptWord();
	else o_jumpRelative();
}

void Scumm::o_equalZero() {
	int a = readVar(fetchScriptWord());
	if (a == 0) ignoreScriptWord();
	else o_jumpRelative();
}

void Scumm::o_isSoundRunning() {
	int snd;
	getResultPos();
	snd = getVarOrDirectByte(0x80);
	if (snd)
		setResult(unkSoundProc23(snd));
	else
		setResult(0);
}

void Scumm::o_jumpRelative() {
	_scriptPointer += (int16)fetchScriptWord();
}

void Scumm::o_lights() {
	int a,b,c;

	a = getVarOrDirectByte(0x80);
	b = fetchScriptByte();
	c = fetchScriptByte();
	if (c==0)
		vm.vars[VAR_DRAWFLAGS] = a;
	else if (c==1) {
		_lightsValueA = a;
		_lightsValueB = b;
	}
	_fullRedraw=1;
}

void Scumm::o_loadRoom() {
	int room = getVarOrDirectByte(0x80);
	debug(1,"Loading room %d", room);
	startScene(room, 0, 0);
	_fullRedraw = 1;
}

void Scumm::o_loadRoomWithEgo() {
	int obj, room, x,y;
	Actor *a;

	obj = getVarOrDirectWord(0x80);
	room = getVarOrDirectByte(0x40);

	a = derefActorSafe(vm.vars[VAR_UNK_ACTOR], "o_loadRoomWithEgo");

	/* Warning: uses _xPos, _yPos from a previous update of those */
	putActor(a, _xPos, _yPos, room);

	x = (int16)fetchScriptWord();
	y = (int16)fetchScriptWord();

	dseg_3A76 = 0;

	vm.vars[VAR_WALKTO_OBJ] = obj;

	startScene(a->room, a, obj);

	vm.vars[VAR_WALKTO_OBJ] = 0;
	camera._destPos = camera._curPos = a->x;
	setCameraFollows(a);
	_fullRedraw=1;

	if (x != -1) {
		startWalkActor(a, x, y, 0xFF);
	}
}

void Scumm::o_matrixOps() {
	int a,b;

	_opcode = fetchScriptByte();
	switch(_opcode & 0x1F) {
	case 1:
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		setBoxFlags(a,b);
		break;
	case 2:
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		setBoxScale(a,b);
		break;
	case 3:
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		setBoxScale(a,(b-1)|0x8000);
		break;
	case 4:
		createBoxMatrix();
		break;
	}
}

void Scumm::o_move() {
	getResultPos();
	setResult(getVarOrDirectWord(0x80));
}

void Scumm::o_multiply() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) * a);
}


void Scumm::o_or() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) | a);
}

void Scumm::o_overRide() {
	byte b;
	int index;
	uint32 *ptr;

	b = fetchScriptByte();
	if(b!=0) {
		index = vm.cutSceneStackPointer;
		ptr = &vm.cutScenePtr[index];
		if (!*ptr) {
			vm.slot[_currentScript].cutsceneOverride++;
		}
		*ptr = _scriptPointer - _scriptOrgPointer;
		vm.cutSceneScript[index] = _currentScript;

		ignoreScriptByte();
		ignoreScriptWord();
	} else {
		index = vm.cutSceneStackPointer;
		ptr = &vm.cutScenePtr[index];
		if (*ptr) {
			vm.slot[_currentScript].cutsceneOverride--;
		}
		*ptr = 0;
		vm.cutSceneScript[index] = 0;
	}
	vm.vars[VAR_OVERRIDE] = 0;
}

void Scumm::o_panCameraTo() {
	CameraData *cd = &camera;
	cd->_destPos = getVarOrDirectWord(0x80);
	cd->_mode = 3;
	cd->_movingToActor = 0;
}

void Scumm::o_pickupObject() {
	int obj, room;

	obj = getVarOrDirectWord(0x80);
	room = getVarOrDirectByte(0x40);
	if (room==0)
		room = _roomResource;
	addObjectToInventory(obj, room);
	putOwner(obj, vm.vars[VAR_UNK_ACTOR]);
	putClass(obj, 32, 1);
	putState(obj, 1);
	removeObjectFromRoom(obj);
	clearDrawObjectQueue();
	runHook(1);
}

void Scumm::o_print() {
	_actorToPrintStrFor = getVarOrDirectByte(0x80);
	decodeParseString();
}

void Scumm::o_printEgo() {
	_actorToPrintStrFor = vm.vars[VAR_UNK_ACTOR];
	decodeParseString();
}

void Scumm::o_pseudoRoom() {
	int i = fetchScriptByte(), j;
	while ((j = fetchScriptByte()) != 0) {
		if (j >= 0x80) {
			_resourceMapper[j&0x7F] = i;
		}
	}
}

void Scumm::o_putActor() {
	int x,y;
	Actor *a;

	a = derefActorSafe(getVarOrDirectByte(0x80), "o_putActor");
	x = getVarOrDirectWord(0x40);
	y = getVarOrDirectWord(0x20);
	
	putActor(a, x, y, a->room);
}


void Scumm::o_putActorAtObject() {
	int obj;
	Actor *a;

	a = derefActorSafe(getVarOrDirectByte(0x80), "o_putActorAtObject");
	obj = getVarOrDirectWord(0x40);
	if (whereIsObject(obj)!=-1)
		getObjectXYPos(obj);
	else {
		_xPos = 240;
		_yPos = 120;
	}
	putActor(a, _xPos, _yPos, a->room);
}

void Scumm::o_putActorInRoom() {
	int room;
	Actor *a;

	a = derefActorSafe(getVarOrDirectByte(0x80), "o_putActorInRoom");
	room = getVarOrDirectByte(0x40);
	if (a->visible && _currentRoom!=room && vm.vars[VAR_TALK_ACTOR]==a->number) {
		clearMsgQueue();
	}
	a->room = room;
	if (!room)
		putActor(a, 0, 0, 0);
}

void Scumm::o_quitPauseRestart() {
	switch(fetchScriptByte()) {
	case 1:
		pauseGame(0);
		break;
	case 3:
		shutDown(0);
		break;
	}
}

void Scumm::o_resourceRoutines() {
	int res;

	_opcode = fetchScriptByte();
	if (_opcode != 17)
		res = getVarOrDirectByte(0x80);
	switch(_opcode&0x1F) {
	case 1: /* load script */
		ensureResourceLoaded(2, res);
		break;
	case 2: /* load sound */
		ensureResourceLoaded(4, res);
		break;
	case 3: /* load costume */
		ensureResourceLoaded(3, res);
		break;
	case 4: /* load room */
		ensureResourceLoaded(1, res);
		break;
	case 5: /* nuke script */
		setResourceFlags(2, res, 0x7F);
		break;
	case 6: /* nuke sound */
		setResourceFlags(4, res, 0x7F);
		break;
	case 7: /* nuke costume */
		setResourceFlags(3, res, 0x7F);
		break;
	case 8: /* nuke room */
		setResourceFlags(1, res, 0x7F);
		break;
	case 9:  /* lock script */
		if (res >= _numGlobalScriptsUsed)
			break;
		lock(2,res);
		break;
	case 10:/* lock sound */
		lock(4,res);
		break;
	case 11:/* lock costume */
		lock(3,res);
		break;
	case 12:/* lock room */
		if (res > 0x7F)
			res = _resourceMapper[res&0x7F];
		lock(1,res);
		break;
	case 13:/* unlock script */
		if (res >= _numGlobalScriptsUsed)
			break;
		unlock(2,res);
		break;
	case 14:/* unlock sound */
		unlock(4,res);
		break;
	case 15:/* unlock costume */
		unlock(3,res);
		break;
	case 16:/* unlock room */
		if (res > 0x7F)
			res = _resourceMapper[res&0x7F];
		unlock(1,res);
		break;
	case 17:/* clear heap */
		heapClear(0);
		unkHeapProc2(0,0);
		break;
	case 18:/* load charset */
		loadCharset(res);
		break;
	case 19:/* nuke charset */
		nukeCharset(res);
		break;
	case 20:/* ? */
		unkResProc(getVarOrDirectWord(0x40), res);
		break;
	}
}

void Scumm::o_roomOps() {
	int a,b,c,d,e;

	_opcode = fetchScriptByte();

	switch(_opcode & 0x1F) {
	case 1: /* room scroll */
		a = getVarOrDirectWord(0x80);
		b = getVarOrDirectWord(0x40);
		if (a < 160) a=160;
		if (a > ((_scrWidthIn8Unit-20)<<3)) a=((_scrWidthIn8Unit-20)<<3);
		if (b < 160) b=160;
		if (b > ((_scrWidthIn8Unit-20)<<3)) b=((_scrWidthIn8Unit-20)<<3);
		vm.vars[VAR_CAMERA_MIN] = a;
		vm.vars[VAR_CAMERA_MAX] = b;
		break;
	case 2: /* room color */
		error("room-color is no longer a valid command");
		break;

	case 3: /* set screen */
		a = getVarOrDirectWord(0x80);
		b = getVarOrDirectWord(0x40);
		initScreens(0,a,320,b);
		break;
	case 4: /* set palette color */
		a = getVarOrDirectWord(0x80);
		b = getVarOrDirectWord(0x40);
		c = getVarOrDirectWord(0x20);
		_opcode = fetchScriptByte();
		d = getVarOrDirectByte(0x80);
		setPalColor(d, a, b, c); /* index, r, g, b */
		break;
	case 5: /* shake on */
		setShake(1);
		break;
	case 6: /* shake off */
		setShake(0);
		break;
	case 8: /* room scale? */
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		c = getVarOrDirectByte(0x20);
		unkRoomFunc2(b, c, a, a, a);
		break;
	case 9: /* ? */
		_saveLoadFlag = getVarOrDirectByte(0x80);
		_saveLoadData = getVarOrDirectByte(0x40);
		_saveLoadData = 0; /* TODO: weird behaviour */
		break;
	case 10: /* ? */
		a = getVarOrDirectWord(0x80);
		if (a) {
			_switchRoomEffect = (byte)(a);
			_switchRoomEffect2 = (byte)(a>>8);
		} else {
			screenEffect(_newEffect);
		}
		break;
	case 11: /* ? */
		a = getVarOrDirectWord(0x80);
		b = getVarOrDirectWord(0x40);
		c = getVarOrDirectWord(0x20);
		_opcode = fetchScriptByte();
		d = getVarOrDirectByte(0x80);
		e = getVarOrDirectByte(0x40);
		unkRoomFunc2(d, e, a, b, c);
		break;
	case 12: /* ? */
		a = getVarOrDirectWord(0x80);
		b = getVarOrDirectWord(0x40);
		c = getVarOrDirectWord(0x20);
		_opcode = fetchScriptByte();
		d = getVarOrDirectByte(0x80);
		e = getVarOrDirectByte(0x40);
		unkRoomFunc3(d, e, a, b, c);
		break;

	case 13: /* ? */
		error("roomops:13 not implemented");
		break;
	case 14: /* ? */
		error("roomops:14 not implemented");
		break;
	case 15: /* palmanip? */
		a = getVarOrDirectByte(0x80);
		_opcode = fetchScriptByte();
		b = getVarOrDirectByte(0x80);
		c = getVarOrDirectByte(0x40);
		_opcode = fetchScriptByte();
		d = getVarOrDirectByte(0x80);
		unkRoomFunc4(b, c, a, d, 1);
		break;

	case 16: /* ? */
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		if (b!=0)
			_colorCycleDelays[a] = 0x4000 / (b*0x4C);
		else
			_colorCycleDelays[a] = 0;
		break;
	}
}

void Scumm::o_saveRestoreVerbs() {
	int a,b,c,slot, slot2;

	_opcode = fetchScriptByte();
	
	a = getVarOrDirectByte(0x80);
	b = getVarOrDirectByte(0x40);
	c = getVarOrDirectByte(0x20);

	switch(_opcode) {
	case 1: /* hide verbs */
		while (a<=b) {
			slot = getVerbSlot(a,0);
			if (slot && verbs[slot].saveid==0) {
				verbs[slot].saveid = c;
				drawVerb(slot, 0);
				verbMouseOver(0);
			}
			a++;
		}
		break;
	case 2: /* show verbs */
		while (a<=b) {
			slot = getVerbSlot(a, c);
			if (slot) {
				slot2 = getVerbSlot(a,0);
				if (slot2)
					killVerb(slot2);
				slot = getVerbSlot(a,c);
				verbs[slot].saveid = 0;
				drawVerb(slot, 0);
				verbMouseOver(0);
			}
			a++;
		}
		break;
	case 3: /* kill verbs */
		while (a<=b) {
			slot = getVerbSlot(a,c);
			if (slot)
				killVerb(slot);
			a++;
		}
		break;
	default:
		error("o_saveRestoreVerbs: invalid opcode");
	}
}

void Scumm::o_setCameraAt() {
	CameraData *cd = &camera;
	cd->_curPos = getVarOrDirectWord(0x80);
	cd->_mode = 1;
	setCameraAt(cd->_curPos);
	cd->_movingToActor = 0;
}

void Scumm::o_setObjectName() {
	int act = getVarOrDirectWord(0x80);
	int size;
	int a;
	int i;

	if (vm.vars[VAR_NUM_ACTOR] >= act)
		error("Can't set actor %d name with new-name-of", act);

	if (!getObjectAddress(act))
		error("Can't set name of object %d", act);

	size = READ_BE_UINT32_UNALIGNED(getObjOrActorName(act) - 4)-9;
	i = 0;

	while ((a = fetchScriptByte()) != 0) {
		 getObjOrActorName(act)[i++] = a;

		if (a==0xFF) {
			getObjOrActorName(act)[i++] = fetchScriptByte();
			getObjOrActorName(act)[i++] = fetchScriptByte();
			getObjOrActorName(act)[i++] = fetchScriptByte();
		}

		if (i > size)
			error("New name of object %d too long", act);
	}

	getObjOrActorName(act)[i] = 0;
	runHook(0);
}

void Scumm::o_setOwnerOf() {
	int obj, owner;
	ScriptSlot *ss;

	obj = getVarOrDirectWord(0x80);
	owner = getVarOrDirectByte(0x40);

	if (owner==0) {
		clearOwnerOf(obj);
		ss = &vm.slot[_currentScript];
		if (ss->type==0 && _inventory[ss->number]==obj) {
			putOwner(obj, owner);
			runHook(0);
			stopObjectCode();
			return;
		}
	}
	putOwner(obj, owner);
	runHook(0);
}

void Scumm::o_setState() {
	int obj, state;
	obj = getVarOrDirectWord(0x80);
	state = getVarOrDirectByte(0x40);
	putState(obj, state);
	removeObjectFromRoom(obj);
	if (_BgNeedsRedraw)
		clearDrawObjectQueue();
}

void Scumm::o_setVarRange() {
	int a,b;

	getResultPos();
	a=fetchScriptByte();
	do {
		if (_opcode&0x80)
			b=fetchScriptWord();
		else
			b=fetchScriptByte();
		
		setResult(b);
		_resultVarNumber++;
	} while (--a);
}

void Scumm::o_soundKludge() {
	int16 items[15];
	int i;
	int16 *ptr;

	for (i=0; i<15; i++)
		items[i] = 0;

	getWordVararg(items);
	if (items[0]==-1)
		unkSoundProc22();
	else {
		_soundQue[_soundQuePos++] = 8;

		ptr = _soundQue + _soundQuePos;
		_soundQuePos += 8;

		for (i=0; i<8; i++)
			*ptr++ = items[i];
		if (_soundQuePos > 0x100)
			error("Sound que buffer overflow");
	}
}

void Scumm::o_startMusic() {
	addSoundToQueue(getVarOrDirectByte(0x80));
}

void Scumm::o_startObject() {
	int obj, script;
	int16 data[16];

	obj = getVarOrDirectWord(0x80);
	script = getVarOrDirectByte(0x40);

	getWordVararg(data);
	runVERBCode(obj, script, 0, 0, data);
}

void Scumm::o_startScript() {
	int op,script;
	int16 data[16];
	int a,b;
	
	op = _opcode;
	script = getVarOrDirectByte(0x80);

	getWordVararg(data);

	a = b = 0;
	if (op&0x40) b=1;
	if (op&0x20) a=1;

	runScript(script, a, b, data);
}

void Scumm::o_startSound() {
	addSoundToQueue(getVarOrDirectByte(0x80));
}

void Scumm::o_stopMusic() {
	/* TODO: not implemented */
	warning("o_stopMusic: not implemented");
}

void Scumm::o_stopObjectCode() {
	stopObjectCode();
}

void Scumm::o_stopObjectScript() {
	stopObjectScript(getVarOrDirectWord(0x80));
}

void Scumm::o_stopScript() {
	int script;

	script = getVarOrDirectByte(0x80);
	if (script==0)
		stopObjectCode();
	else
		stopScriptNr(script);
}

void Scumm::o_stopSound() {
	unkSoundProc1(getVarOrDirectByte(0x80));
}

void Scumm::o_stringOps() {
	int a,b,c,i;
	byte *ptr;

	_opcode = fetchScriptByte();
	switch(_opcode&0x1F) {
	case 1: /* loadstring */
		loadPtrToResource(7, getVarOrDirectByte(0x80), NULL);
		break;
	case 2: /* copystring */
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		nukeResource(7, a);
		ptr = getResourceAddress(7, b);
		if (ptr) loadPtrToResource(7, a, ptr);
		break;
	case 3: /* set string char */
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		ptr = getResourceAddress(7, a);
		if (ptr==NULL) error("String %d does not exist", a);
		c = getVarOrDirectByte(0x20);
		ptr[b] = c;
		break;

	case 4: /* get string char */
		getResultPos();
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		ptr = getResourceAddress(7, a);
		if (ptr==NULL) error("String %d does not exist", a);
		setResult(ptr[b]);
		break;
		
	case 5: /* create empty string */
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		nukeResource(7, a);
		if (b) {
			ptr = createResource(7, a, b);
			if (ptr) {
				for(i=0; i<b; i++)
					ptr[i] = 0;
			}
		}
		break;
	}
}

void Scumm::o_subtract() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) - a);
}

void Scumm::o_verbOps() {
	int verb,slot;
	VerbSlot *vs;
	int a,b;
	byte *ptr;

	verb = getVarOrDirectByte(0x80);

	slot = getVerbSlot(verb,0);
	checkRange(_maxVerbs-1, 0, slot, "Illegal new verb slot %d");

	vs = &verbs[slot];
	vs->verbid = verb;

	while ((_opcode=fetchScriptByte()) != 0xFF) {
		switch(_opcode&0x1F) {
		case 1: /* load image */
			a = getVarOrDirectWord(0x80);
			if (verb) {
				setVerbObject(_roomResource, a, verb);
				vs->type = 1;
			}
			break;
		case 2: /* load from code */
			loadPtrToResource(8, slot, NULL);
			if (slot==0)
				nukeResource(8, slot);
			vs->type = 0;
			vs->imgindex = 0;
			break;
		case 3: /* color */
			vs->color = getVarOrDirectByte(0x80);
			break;
		case 4: /* set hi color */
			vs->hicolor = getVarOrDirectByte(0x80);
			break;
		case 5: /* set xy */
			vs->x = getVarOrDirectWord(0x80);
			vs->y = getVarOrDirectWord(0x40);
			break;
		case 6: /* set on */
			vs->curmode=1;
			break;
		case 7: /* set off */
			vs->curmode=0;
			break;
		case 8: /* delete */
			killVerb(slot);
			break;
		case 9: /* new */
			slot = getVerbSlot(verb, 0);
			if (slot==0) {
				for (slot=1; slot<_maxVerbs; slot++) {
					if(verbs[slot].verbid==0)
						break;
				}
				if (slot==_maxVerbs)
					error("Too many verbs");
			}
			vs = &verbs[slot];
			vs->verbid = verb;
			vs->color = 2;
			vs->hicolor = 0;
			vs->dimcolor = 8;
			vs->type = 0;
			vs->charset_nr = textslot.charset[0];
			vs->curmode = 0;
			vs->saveid = 0;
			vs->key = 0;
			vs->center = 0;
			vs->imgindex = 0;
			break;

		case 16: /* set dim color */
			vs->dimcolor = getVarOrDirectByte(0x80);
			break;
		case 17: /* dim */
			vs->curmode = 2;
			break;
		case 18: /* set key */
			vs->key = getVarOrDirectByte(0x80);
			break;
		case 19: /* set center */
			vs->center = 1;
			break;
		case 20: /* set to string */
			ptr = getResourceAddress(7, getVarOrDirectWord(0x80));
			if (!ptr)
				nukeResource(8, slot);
			else {
				loadPtrToResource(8, slot, ptr);
			}
			if (slot==0)
				nukeResource(8, slot);
			vs->type = 0;
			vs->imgindex = 0;
			break;
		case 22: /* assign object */
			a = getVarOrDirectWord(0x80);
			b = getVarOrDirectByte(0x40);
			if (slot && vs->imgindex!=a) {
				setVerbObject(b, a, slot);
				vs->type = 1;
				vs->imgindex = a;
			}
			break;
		case 23: /* set back color */
			vs->bkcolor = getVarOrDirectByte(0x80);
			break;
		}
	}
	drawVerb(slot, 0);
	verbMouseOver(0);
}

void Scumm::o_wait() {
	byte *oldaddr;

	oldaddr = _scriptPointer - 1;
	
	_opcode = fetchScriptByte();
	switch(_opcode&0x1F) {
	case 1: /* wait for actor */
		if (derefActorSafe(getVarOrDirectByte(0x80), "o_wait")->moving)
			break;
		return;
	case 2: /* wait for message */
		if (vm.vars[VAR_HAVE_MSG])
			break;
		return;
	case 3: /* wait for camera */
		if (camera._curPos>>3 != camera._destPos>>3)
			break;
		return;
	case 4: /* wait for sentence */
		if (_sentenceIndex!=0xFF) {
			if (_sentenceTab[_sentenceIndex] &&
				!isScriptLoaded(vm.vars[VAR_SENTENCE_SCRIPT]) )
				return;
			break;
		}
		if (!isScriptLoaded(vm.vars[VAR_SENTENCE_SCRIPT]))
			return;
		break;

	default:
		return;
	}

	_scriptPointer = oldaddr;
	o_breakHere();
}

void Scumm::o_walkActorTo() {
	int x, y;
	Actor *a;
	a = derefActorSafe(getVarOrDirectByte(0x80), "o_walkActorTo");
	x = getVarOrDirectWord(0x40);
	y = getVarOrDirectWord(0x20);
	startWalkActor(a, x, y, 0xFF);
}

void Scumm::o_walkActorToActor() {
	int b,x,y;
	Actor *a, *a2;

	a = derefActorSafe(getVarOrDirectByte(0x80), "o_walkActorToActor");
	if (a->room != _currentRoom) {
		getVarOrDirectByte(0x40);
		fetchScriptByte();
		return;
	}

	a2 = derefActorSafe(getVarOrDirectByte(0x40), "o_walkActorToActor(2)");
	if (a2->room != _currentRoom) {
		fetchScriptByte();
		return;
	}
	b = fetchScriptByte(); /* distance from actor */
	if (b==0xFF) {
		b = a2->scalex * a->width / 0xFF;
		b = b + b/2;
	}
	y = a2->x;
	x = a2->y;
	if (x < a->x)
		x += b;
	else
		x -= b;
	
	startWalkActor(a, x, y, 0xFF);
}

void Scumm::o_walkActorToObject() {
	int obj;
	Actor *a;

	a = derefActorSafe(getVarOrDirectByte(0x80), "o_walkActorToObject");
	obj = getVarOrDirectWord(0x40);
	if (whereIsObject(obj)!=-1) {
		getObjectXYPos(obj);
		startWalkActor(a, _xPos, _yPos, _dir);
	}
}

void Scumm::stopObjectCode() {
	ScriptSlot *ss;

	ss = &vm.slot[_currentScript];

	if (ss->type!=2 && ss->type!=3) {
		stopObjectScript(ss->number);
	} else {
		if (ss->cutsceneOverride)
			error("Script %d ending with active cutscene/override", ss->number);
		ss->number = 0;
		ss->status = 0;
	}
	_currentScript = 0xFF;
}

int Scumm::getWordVararg(int16 *ptr) {
	int i;
	for (i=0; i<16; i++)
		ptr[i] = 0;

	i = 0;
	while ((_opcode = fetchScriptByte()) != 0xFF) {
		ptr[i++] = getVarOrDirectWord(0x80);
	}
	return i;
}

bool Scumm::isScriptLoaded(int script) {
	ScriptSlot *ss;
	int i;

	ss = vm.slot;
	for (i=0; i<20; i++,ss++) {
		if (ss->number == script)
			return true;
	}
	return false;
}

void Scumm::runHook(int i) {
	int16 tmp[16];
	tmp[0] = i;
	if (vm.vars[VAR_HOOK_SCRIPT]) {
		runScript(vm.vars[VAR_HOOK_SCRIPT], 0, 0, tmp);
	}
}

void Scumm::freezeScripts(int flag) {
	int i;

	for(i=1; i<20; i++) {
		if (_currentScript!=i && vm.slot[i].status!=0 && (vm.slot[i].unk1==0 || flag>=0x80)) {
			vm.slot[i].status |= 0x80;
			vm.slot[i].freezeCount++;
		}
	}

	for (i=0; i<6; i++)
		_sentenceTab[i]++;

	if(vm.cutSceneScriptIndex != 0xFF) {
		vm.slot[vm.cutSceneScriptIndex].status&=0x7F;
		vm.slot[vm.cutSceneScriptIndex].freezeCount=0;
	}
}

void Scumm::unfreezeScripts() {
	int i;
	for (i=1; i<20; i++) {
		if (vm.slot[i].status&0x80) {
			if (!--vm.slot[i].freezeCount) {
				vm.slot[i].status&=0x7F;
			}
		}
	}

	for (i=0; i<6; i++) {
		if (((int8)--_sentenceTab[i])<0)
			_sentenceTab[i] = 0;
	}
}

void Scumm::runAllScripts() {
	int i;

	for (i=0; i<20; i++)
		vm.slot[i].didexec = 0;
	
	_currentScript = 0xFF;
	for(_curExecScript = 0; _curExecScript<20; _curExecScript++) {
		if (vm.slot[_curExecScript].status == 2 &&
			vm.slot[_curExecScript].didexec == 0) {
			_currentScript = _curExecScript;
			getScriptBaseAddress();
			getScriptEntryPoint();
			executeScript();
		}
	}
}

void Scumm::runExitScript() {
	if (vm.vars[VAR_EXIT_SCRIPT])
		runScript(vm.vars[VAR_EXIT_SCRIPT], 0, 0, 0);
	if (_EXCD_offs) {
		int slot = getScriptSlot();
		vm.slot[slot].status = 2;
		vm.slot[slot].number = 10001;
		vm.slot[slot].type = 1;
		vm.slot[slot].offs = _EXCD_offs + 8;
		vm.slot[slot].unk1 = 0;
		vm.slot[slot].unk2 = 0;
		vm.slot[slot].freezeCount = 0;
		runScriptNested(slot);
	}
	if (vm.vars[VAR_EXIT_SCRIPT2])
		runScript(vm.vars[VAR_EXIT_SCRIPT2], 0, 0, 0);
}

void Scumm::runEntryScript() {
	if (vm.vars[VAR_ENTRY_SCRIPT])
		runScript(vm.vars[VAR_ENTRY_SCRIPT], 0, 0, 0);
	if (_ENCD_offs) {
		int slot = getScriptSlot();
		vm.slot[slot].status = 2;
		vm.slot[slot].number = 10002;
		vm.slot[slot].type = 1;
		vm.slot[slot].offs = _ENCD_offs + 8;
		vm.slot[slot].unk1 = 0;
		vm.slot[slot].unk2 = 0;
		vm.slot[slot].freezeCount = 0;
		runScriptNested(slot);
	}
	if (vm.vars[VAR_ENTRY_SCRIPT2])
		runScript(vm.vars[VAR_ENTRY_SCRIPT2], 0, 0, 0);
}

void Scumm::killScriptsAndResources() {
	ScriptSlot *ss;
	int i;

	ss = &vm.slot[1];
	
	for (i=1; i<20; i++,ss++) {
		if (ss->type==1 || ss->type==4) {
			if(ss->cutsceneOverride)
				error("Object %d stopped with active cutscene/override in exit", ss->number);
			ss->status = 0;
		} else if (ss->type==3) {
			if(ss->cutsceneOverride)
				error("Script %d stopped with active cutscene/override in exit", ss->number);
			ss->status = 0;
		}
	}
	
	i = 0;
	do {
		if (objs[i].fl_object_index)
			nukeResource(0xD, objs[i].fl_object_index);
	} while (++i <= _numObjectsInRoom);
}

void Scumm::checkAndRunVar33() {
	int i;
	ScriptSlot *ss;

	memset(_localParamList, 0, sizeof(_localParamList));
	if (isScriptLoaded(vm.vars[VAR_SENTENCE_SCRIPT])) {
		ss = vm.slot;
		for (i=0; i<20; i++,ss++)
			if (ss->number==vm.vars[VAR_SENTENCE_SCRIPT] && ss->status!=0 && ss->freezeCount==0)
				return;
	}

	if (_sentenceIndex > 0x7F || _sentenceTab[_sentenceIndex])
		return;
	
	if (_sentenceTab2[_sentenceIndex] && 
		_sentenceTab3[_sentenceIndex]==_sentenceTab4[_sentenceIndex]) {
		_sentenceIndex--;
		return;
	}

	_localParamList[0] = _sentenceTab5[_sentenceIndex];
	_localParamList[1] = _sentenceTab4[_sentenceIndex];
	_localParamList[2] = _sentenceTab3[_sentenceIndex];
	_sentenceIndex--;
	_currentScript = 0xFF;
	if (vm.vars[VAR_SENTENCE_SCRIPT])
		runScript(vm.vars[VAR_SENTENCE_SCRIPT], 0, 0, _localParamList);
}

void Scumm::runInputScript(int a, int cmd, int mode) {
	memset(_localParamList, 0, sizeof(_localParamList));
	_localParamList[0] = a;
	_localParamList[1] = cmd;
	_localParamList[2] = mode;
	if (vm.vars[VAR_VERB_SCRIPT])
		runScript(vm.vars[VAR_VERB_SCRIPT], 0, 0, _localParamList);
}

void Scumm::decreaseScriptDelay(int amount) {
	ScriptSlot *ss = &vm.slot[0];
	int i;
	for (i=0; i<20; i++,ss++) {
		if(ss->status==1) {
			ss->delay -= amount;
			if (ss->delay < 0){
				ss->status = 2;
				ss->delay = 0;
			}
		}
	}
}

void Scumm::decodeParseString() {
	int textSlot;

	switch(_actorToPrintStrFor) {
	case 252:
		textSlot = 3;
		break;
	case 253:
		textSlot = 2;
		break;
	case 254:
		textSlot = 1;
		break;
	default:
		textSlot = 0;
	}

	_stringXpos[textSlot] = textslot.x[textSlot];
	_stringYpos[textSlot] = textslot.y[textSlot];
	_stringCenter[textSlot] = textslot.center[textSlot];
	_stringOverhead[textSlot] = textslot.overhead[textSlot];
	_stringRight[textSlot] = textslot.right[textSlot];
	_stringColor[textSlot] = textslot.color[textSlot];
	_stringCharset[textSlot] = textslot.charset[textSlot];

	while((_opcode=fetchScriptByte()) != 0xFF) {
		switch(_opcode&0xF) {
		case 0: /* set string xy */
			_stringXpos[textSlot] = getVarOrDirectWord(0x80);
			_stringYpos[textSlot] = getVarOrDirectWord(0x40);
			_stringOverhead[textSlot] = 0;
			break;
		case 1: /* color */
			_stringColor[textSlot] = getVarOrDirectByte(0x80);
			break;
		case 2: /* right */
			_stringRight[textSlot] = getVarOrDirectWord(0x80);
			break;
		case 4:	/* center*/
			_stringCenter[textSlot] = 1;
			_stringOverhead[textSlot] = 0;
			break;
		case 6: /* left */
			_stringCenter[textSlot] = 0;
			_stringOverhead[textSlot] = 0;
			break;
		case 7: /* overhead */
			_stringOverhead[textSlot] = 1;
			break;
		case 8: /* ignore */
			getVarOrDirectWord(0x80);
			getVarOrDirectWord(0x40);
			break;
		case 15:
			_messagePtr = _scriptPointer;
			switch(textSlot) {
			case 0: actorTalk(); break;
			case 1: drawString(1); break;
			case 2: unkMessage1(); break;
			case 3: unkMessage2(); break;
			}
			_scriptPointer = _messagePtr;
			return;
		default:
			return;
		}
	}

	textslot.x[textSlot] = _stringXpos[textSlot];
	textslot.y[textSlot] = _stringYpos[textSlot];
	textslot.center[textSlot] = _stringCenter[textSlot];
	textslot.overhead[textSlot] = _stringOverhead[textSlot];
	textslot.right[textSlot] = _stringRight[textSlot];
	textslot.color[textSlot] = _stringColor[textSlot];
	textslot.charset[textSlot] = _stringCharset[textSlot];
}


void Scumm::runVERBCode(int object, int entry, int a, int b, int16 *vars) {
	uint32 obcd;
	int slot, where, offs,i;

	if (!object)
		return;
	if (!b)	
		stopObjectScript(object);

	where = whereIsObject(object);

	if (where == -1) {
		error("Code for object %d not in room %d", object, _roomResource);
	}

	obcd = getOBCDOffs(object);
	slot = getScriptSlot();

	offs = getVerbEntrypoint(object, entry);
	if (offs==0)
		return;

	vm.slot[slot].number = object;
	vm.slot[slot].offs = obcd + offs;
	vm.slot[slot].status = 2;
	vm.slot[slot].type = where;
	vm.slot[slot].unk1 = a;
	vm.slot[slot].unk2 = b;
	vm.slot[slot].freezeCount = 0;

	if (!vars) {
		for(i=0; i<16; i++)
			vm.localvar[slot * 17 + i] = 0;
	} else {
		for (i=0; i<16; i++)
			vm.localvar[slot * 17 + i] = vars[i];
	}

	runScriptNested(slot);
}

void Scumm::stackPush(int a) {
	assert(_scummStackPos >=0 && _scummStackPos < sizeof(_scummStack)-1);
	_scummStack[_scummStackPos++] = a;	
}

int Scumm::stackPop() {
	assert(_scummStackPos >0 && _scummStackPos < sizeof(_scummStack));
	return _scummStack[--_scummStackPos];
}

int Scumm::getVerbEntrypoint(int obj, int entry) {
	byte *objptr, *verbptr;
	int verboffs;

	if (whereIsObject(obj)==-1)
		return 0;

	objptr = getObjectAddress(obj);

	verbptr = findResource(MKID('VERB'), objptr);
	if (verbptr==NULL)
		error("No verb block in object %d", obj);

	verboffs = verbptr - objptr;

	verbptr += 8;
	do {
		if (!*verbptr)
			return 0;
		if (*verbptr==entry || *verbptr==0xFF)
			break;
		verbptr += 3;
	} while (1);

	return verboffs + READ_LE_UINT16(verbptr+1);
}

