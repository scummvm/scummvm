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

/* Start executing script 'script' with parameters 'a' and 'b' */
void Scumm::runScript(int script, int a, int b, int16 * lvarptr)
{
	byte *scriptPtr;
	uint32 scriptOffs;
	byte scriptType;
	int slot;
	ScriptSlot *s;

	if (script == 0)
		return;

	if (b == 0)
		stopScriptNr(script);

	if (script < _numGlobalScripts) {
		scriptPtr = getResourceAddress(rtScript, script);
		scriptOffs = _resourceHeaderSize;
		scriptType = WIO_GLOBAL;
	} else {
		scriptOffs = _localScriptList[script - _numGlobalScripts];
		if (scriptOffs == 0)
			error("Local script %d is not in room %d", script, _roomResource);
		scriptType = WIO_LOCAL;
	}

	slot = getScriptSlot();

	s = &vm.slot[slot];
	s->number = script;
	s->offs = scriptOffs;
	s->status = 2;
	s->where = scriptType;
	s->unk1 = a;
	s->unk2 = b;
	s->freezeCount = 0;

	initializeLocals(slot, lvarptr);

	runScriptNested(slot);
}

/* Stop script 'script' */
void Scumm::stopScriptNr(int script)
{
	ScriptSlot *ss;
	NestedScript *nest;
	int i, num;

	if (script == 0)
		return;

	ss = &vm.slot[1];

	for (i = 1; i < NUM_SCRIPT_SLOT; i++, ss++) {
		if (script != ss->number ||
				ss->where != WIO_GLOBAL && ss->where != WIO_LOCAL || ss->status == 0)
			continue;

		if (ss->cutsceneOverride)
			error("Script %d stopped with active cutscene/override", script);
		ss->number = 0;
		ss->status = ssDead;
		if (_currentScript == i)
			_currentScript = 0xFF;
	}

	if (_numNestedScripts == 0)
		return;

	nest = &vm.nest[0];
	num = _numNestedScripts;

	do {
		if (nest->number == script
				&& (nest->where == WIO_GLOBAL || nest->where == WIO_LOCAL)) {
			nest->number = 0xFF;
			nest->slot = 0xFF;
			nest->where = 0xFF;
		}
	} while (nest++, --num);
}

/* Stop an object script 'script'*/
void Scumm::stopObjectScript(int script)
{
	ScriptSlot *ss;
	NestedScript *nest;
	int i, num;

	if (script == 0)
		return;

	ss = &vm.slot[1];

	for (i = 1; i < NUM_SCRIPT_SLOT; i++, ss++) {
		if (script == ss->number && (ss->where == WIO_ROOM ||
																 ss->where == WIO_INVENTORY
																 || ss->where == WIO_FLOBJECT)
				&& ss->status != 0) {
			if (ss->cutsceneOverride)
				error("Object %d stopped with active cutscene/override", script);
			ss->number = 0;
			ss->status = ssDead;
			if (_currentScript == i)
				_currentScript = 0xFF;
		}
	}

	if (_numNestedScripts == 0)
		return;

	nest = &vm.nest[0];
	num = _numNestedScripts;

	do {
		if (nest->number == script &&
				(nest->where == WIO_ROOM || nest->where == WIO_FLOBJECT ||
				 nest->where == WIO_INVENTORY)) {
			nest->number = 0xFF;
			nest->slot = 0xFF;
			nest->where = 0xFF;
		}
	} while (nest++, --num);
}

/* Return a free script slot */
int Scumm::getScriptSlot()
{
	ScriptSlot *ss;
	int i;
	ss = &vm.slot[1];

	for (i = 1; i < NUM_SCRIPT_SLOT; i++, ss++) {
		if (ss->status == 0)
			return i;
	}
	error("Too many scripts running, %d max", NUM_SCRIPT_SLOT);
}

/* Run script 'script' nested - eg, within the parent script.*/
void Scumm::runScriptNested(int script)
{
	NestedScript *nest;
	ScriptSlot *slot;

	updateScriptPtr();

	nest = &vm.nest[_numNestedScripts];

	if (_currentScript == 0xFF) {
		nest->number = 0xFF;
		nest->where = 0xFF;
	} else {
		slot = &vm.slot[_currentScript];
		nest->number = slot->number;
		nest->where = slot->where;
		nest->slot = _currentScript;
	}

	if (++_numNestedScripts > sizeof(vm.nest) / sizeof(vm.nest[0]))
		error("Too many nested scripts");

	_currentScript = script;

	getScriptBaseAddress();
	getScriptEntryPoint();
	executeScript();

	_numNestedScripts--;

	nest = &vm.nest[_numNestedScripts];

	if (nest->number != 0xFF) {
		slot = &vm.slot[nest->slot];
		if (slot->number == nest->number && slot->where == nest->where &&
				slot->status != 0 && slot->freezeCount == 0) {
			_currentScript = nest->slot;
			getScriptBaseAddress();
			getScriptEntryPoint();
			return;
		}
	}
	_currentScript = 0xFF;
}

void Scumm::updateScriptPtr()
{
	if (_currentScript == 0xFF)
		return;

	vm.slot[_currentScript].offs = _scriptPointer - _scriptOrgPointer;
}

/* Get the code pointer to a script */
void Scumm::getScriptBaseAddress()
{
	ScriptSlot *ss;
	int idx;

	if (_currentScript == 0xFF)
		return;

	ss = &vm.slot[_currentScript];
	switch (ss->where) {
	case WIO_INVENTORY:					/* inventory script * */
		idx = getObjectIndex(ss->number);
		_scriptOrgPointer = getResourceAddress(rtInventory, idx);
		_lastCodePtr = &_baseInventoryItems[idx];
		break;

	case 3:
	case WIO_ROOM:								/* room script */
		_scriptOrgPointer = getResourceAddress(rtRoom, _roomResource);
		_lastCodePtr = &_baseRooms[_roomResource];
		break;

	case WIO_GLOBAL:							/* global script */
		_scriptOrgPointer = getResourceAddress(rtScript, ss->number);
		_lastCodePtr = &_baseScripts[ss->number];
		break;

	case WIO_FLOBJECT:						/* flobject script */
		idx = getObjectIndex(ss->number);
		_scriptOrgPointer =
			getResourceAddress(rtFlObject, _objs[idx].fl_object_index);
		_lastCodePtr = &_baseFLObject[ss->number];
		break;
	default:
		error("Bad type while getting base address");
	}
}


void Scumm::getScriptEntryPoint()
{
	if (_currentScript == 0xFF)
		return;
	_scriptPointer = _scriptOrgPointer + vm.slot[_currentScript].offs;
}

/* Execute a script - Read opcode, and execute it from the table */
void Scumm::executeScript()
{
	OpcodeProc op;
	while (_currentScript != 0xFF) {
		_opcode = fetchScriptByte();
		_scriptPointerStart = _scriptPointer;
		vm.slot[_currentScript].didexec = 1;
		//debug(1, "Script %d: [%X] %s()", vm.slot[_currentScript].number, _opcode, _opcodes_lookup[_opcode]);
		op = getOpcode(_opcode);
		(this->*op) ();
	}
CHECK_HEAP}

byte Scumm::fetchScriptByte()
{
	if (*_lastCodePtr + sizeof(MemBlkHeader) != _scriptOrgPointer) {
		uint32 oldoffs = _scriptPointer - _scriptOrgPointer;
		getScriptBaseAddress();
		_scriptPointer = _scriptOrgPointer + oldoffs;
	}
	return *_scriptPointer++;
}

int Scumm::fetchScriptWord()
{
	int a;
	if (*_lastCodePtr + sizeof(MemBlkHeader) != _scriptOrgPointer) {
		uint32 oldoffs = _scriptPointer - _scriptOrgPointer;
		getScriptBaseAddress();
		_scriptPointer = _scriptOrgPointer + oldoffs;
	}
	a = READ_LE_UINT16(_scriptPointer);
	_scriptPointer += 2;
	return a;
}

#ifndef BYPASS_COPY_PROT
#define BYPASS_COPY_PROT
#endif

int Scumm::readVar(uint var)
{
	int a;
#ifdef BYPASS_COPY_PROT
	static byte copyprotbypassed;
#endif
	debug(9, "readvar=%d", var);
	if (!(var & 0xF000)) {
#if defined(BYPASS_COPY_PROT)
		if (var == 490 && _gameId == GID_MONKEY2 && !copyprotbypassed) {
			copyprotbypassed = true;
			var = 518;
		}
#endif
		checkRange(_numVariables - 1, 0, var, "Variable %d out of range(r)");
		return _vars[var];
	}

	if (var & 0x2000 && !(_features & GF_NEW_OPCODES)) {
		a = fetchScriptWord();
		if (a & 0x2000)
			var = (var + readVar(a & ~0x2000)) & ~0x2000;
		else
			var = (var + (a & 0xFFF)) & ~0x2000;
	}

	if (!(var & 0xF000))
		return _vars[var];

	if (var & 0x8000) {
		var &= 0x7FFF;
		checkRange(_numBitVariables - 1, 0, var,
							 "Bit variable %d out of range(r)");
		return (_bitVars[var >> 3] & (1 << (var & 7))) ? 1 : 0;
	}

	if (var & 0x4000) {
		var &= 0xFFF;
		checkRange(0x10, 0, var, "Local variable %d out of range(r)");
		return vm.localvar[_currentScript][var];
	}

	error("Illegal varbits (r)");
}

void Scumm::writeVar(uint var, int value)
{
	if (!(var & 0xF000)) {
		checkRange(_numVariables - 1, 0, var, "Variable %d out of range(w)");
		_vars[var] = value;

		if ((_varwatch == (int)var) || (_varwatch == 0))
			printf("vars[%d] = %d (via script %d)\n", var, value,
						 vm.slot[_currentScript].number);
		return;
	}

	if (var & 0x8000) {
		var &= 0x7FFF;
		checkRange(_numBitVariables - 1, 0, var,
							 "Bit variable %d out of range(w)");
		if (value)
			_bitVars[var >> 3] |= (1 << (var & 7));
		else
			_bitVars[var >> 3] &= ~(1 << (var & 7));
		return;
	}

	if (var & 0x4000) {
		var &= 0xFFF;
		checkRange(0x10, 0, var, "Local variable %d out of range(w)");
		vm.localvar[_currentScript][var] = value;
		return;
	}

	error("Illegal varbits (w)");
}

void Scumm::getResultPos()
{
	int a;

	_resultVarNumber = fetchScriptWord();
	if (_resultVarNumber & 0x2000) {
		a = fetchScriptWord();
		if (a & 0x2000) {
			_resultVarNumber += readVar(a & ~0x2000);
		} else {
			_resultVarNumber += a & 0xFFF;
		}
		_resultVarNumber &= ~0x2000;
	}
}

void Scumm::setResult(int value)
{
	writeVar(_resultVarNumber, value);
}

void Scumm::drawBox(int x, int y, int x2, int y2, int color)
{
	int top, bottom, count;
	VirtScreen *vs;
	byte *backbuff, *bgbuff;

	if ((vs = findVirtScreen(y)) == NULL)
		return;

	top = vs->topline;
	bottom = top + vs->height;

	if (x > x2)
		SWAP(x, x2);

	if (y > y2)
		SWAP(y, y2);

	x2++;
	y2++;

	if (x > 319)
		return;
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	if (x2 < 0)
		return;
	if (x2 > 320)
		x2 = 320;
	if (y2 > bottom)
		y2 = bottom;

	updateDirtyRect(vs->number, x, x2, y - top, y2 - top, 0);

	backbuff = vs->screenPtr + vs->xstart + (y - top) * 320 + x;

	if (color == -1) {
		if (vs->number != 0)
			error("can only copy bg to main window");
		bgbuff =
			getResourceAddress(rtBuffer,
												 vs->number + 5) + vs->xstart + (y - top) * 320 + x;
		blit(backbuff, bgbuff, x2 - x, y2 - y);
	} else {
		count = y2 - y;
		while (count) {
			memset(backbuff, color, x2 - x);
			backbuff += 320;
			count--;
		}
	}
}


void Scumm::stopObjectCode()
{
	ScriptSlot *ss;

	ss = &vm.slot[_currentScript];
	if (ss->cutsceneOverride == 255) {	/* FIXME: What does this? */
		warning("Cutscene for script %d has overflown. Resetting.", ss->number);
		ss->cutsceneOverride = 0;
	}

	if (ss->where != WIO_GLOBAL && ss->where != WIO_LOCAL) {
		if (ss->cutsceneOverride) {
			warning("Object %d ending with active cutscene/override", ss->number);
			ss->cutsceneOverride = 0;
		}
	} else {
		if (ss->cutsceneOverride) {
			warning("Script %d ending with active cutscene/override (%d)",
							ss->number, ss->cutsceneOverride);
			ss->cutsceneOverride = 0;
		}
	}
	ss->number = 0;
	ss->status = 0;
	_currentScript = 0xFF;
}

bool Scumm::isScriptInUse(int script)
{
	ScriptSlot *ss;
	int i;

	ss = vm.slot;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++) {
		if (ss->number == script)
			return true;
	}
	return false;
}


void Scumm::runHook(int i)
{
	int16 tmp[16];
	tmp[0] = i;
	if (_vars[VAR_HOOK_SCRIPT]) {
		runScript(_vars[VAR_HOOK_SCRIPT], 0, 0, tmp);
	}
}

void Scumm::freezeScripts(int flag)
{
	int i;

	for (i = 1; i < NUM_SCRIPT_SLOT; i++) {
		if (_currentScript != i && vm.slot[i].status != ssDead
				&& (vm.slot[i].unk1 == 0 || flag >= 0x80)) {
			vm.slot[i].status |= 0x80;
			vm.slot[i].freezeCount++;
		}
	}

	for (i = 0; i < 6; i++)
		sentence[i].unk++;

	if (vm.cutSceneScriptIndex != 0xFF) {
		vm.slot[vm.cutSceneScriptIndex].status &= 0x7F;
		vm.slot[vm.cutSceneScriptIndex].freezeCount = 0;
	}
}

void Scumm::unfreezeScripts()
{
	int i;
	for (i = 1; i < NUM_SCRIPT_SLOT; i++) {
		if (vm.slot[i].status & 0x80) {
			if (!--vm.slot[i].freezeCount) {
				vm.slot[i].status &= 0x7F;
			}
		}
	}

	for (i = 0; i < 6; i++) {
		if (((int8)-- sentence[i].unk) < 0)
			sentence[i].unk = 0;
	}
}

void Scumm::runAllScripts()
{
	int i;

	for (i = 0; i < NUM_SCRIPT_SLOT; i++)
		vm.slot[i].didexec = 0;

	_currentScript = 0xFF;
	for (_curExecScript = 0; _curExecScript < NUM_SCRIPT_SLOT; _curExecScript++) {
		if (vm.slot[_curExecScript].status == ssRunning &&
				vm.slot[_curExecScript].didexec == 0) {
			_currentScript = (char)_curExecScript;
			getScriptBaseAddress();
			getScriptEntryPoint();
			executeScript();
		}
	}
}

void Scumm::runExitScript()
{
	if (_vars[VAR_EXIT_SCRIPT])
		runScript(_vars[VAR_EXIT_SCRIPT], 0, 0, 0);
	if (_EXCD_offs) {
		int slot = getScriptSlot();
		vm.slot[slot].status = ssRunning;
		vm.slot[slot].number = 10001;
		vm.slot[slot].where = WIO_ROOM;
		vm.slot[slot].offs = _EXCD_offs;
		vm.slot[slot].unk1 = 0;
		vm.slot[slot].unk2 = 0;
		vm.slot[slot].freezeCount = 0;
		runScriptNested(slot);
	}
	if (_vars[VAR_EXIT_SCRIPT2])
		runScript(_vars[VAR_EXIT_SCRIPT2], 0, 0, 0);
}

void Scumm::runEntryScript()
{
	if (_vars[VAR_ENTRY_SCRIPT])
		runScript(_vars[VAR_ENTRY_SCRIPT], 0, 0, 0);
	if (_ENCD_offs) {
		int slot = getScriptSlot();
		vm.slot[slot].status = ssRunning;
		vm.slot[slot].number = 10002;
		vm.slot[slot].where = WIO_ROOM;
		vm.slot[slot].offs = _ENCD_offs;
		vm.slot[slot].unk1 = 0;
		vm.slot[slot].unk2 = 0;
		vm.slot[slot].freezeCount = 0;
		runScriptNested(slot);
	}
	if (_vars[VAR_ENTRY_SCRIPT2])
		runScript(_vars[VAR_ENTRY_SCRIPT2], 0, 0, 0);
}

void Scumm::killScriptsAndResources()
{
	ScriptSlot *ss;
	int i;

	ss = &vm.slot[1];

	for (i = 1; i < NUM_SCRIPT_SLOT; i++, ss++) {
		if (ss->where == WIO_ROOM || ss->where == WIO_FLOBJECT) {
			if (ss->cutsceneOverride)
				error("Object %d stopped with active cutscene/override in exit",
							ss->number);
			ss->status = 0;
		} else if (ss->where == WIO_LOCAL) {
			if (ss->cutsceneOverride)
				error("Script %d stopped with active cutscene/override in exit",
							ss->number);
			ss->status = 0;
		}
	}

	/* Nuke FL objects */
	i = 0;
	do {
		if (_objs[i].fl_object_index)
			nukeResource(rtFlObject, _objs[i].fl_object_index);
	} while (++i <= _numObjectsInRoom);

	/* Nuke local object names */
	if (_newNames) {
		for (i = 0; i < _numNewNames; i++) {
			int j = _newNames[i];
			if (j && getOwner(j) == 0) {
				_newNames[i] = 0;
				nukeResource(rtObjectName, i);
			}
		}
	}
}

void Scumm::checkAndRunVar33()
{
	int i;
	ScriptSlot *ss;

	memset(_localParamList, 0, sizeof(_localParamList));
	if (isScriptInUse(_vars[VAR_SENTENCE_SCRIPT])) {
		ss = vm.slot;
		for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++)
			if (ss->number == _vars[VAR_SENTENCE_SCRIPT] && ss->status != 0
					&& ss->freezeCount == 0)
				return;
	}

	if (!_sentenceNum || sentence[_sentenceNum - 1].unk)
		return;

	_sentenceNum--;

	if (!(_features & GF_AFTER_V7))
		if (sentence[_sentenceNum].unk2 &&
				sentence[_sentenceNum].unk3 == sentence[_sentenceNum].unk4)
			return;

	_localParamList[0] = sentence[_sentenceNum].unk5;
	_localParamList[1] = sentence[_sentenceNum].unk4;
	_localParamList[2] = sentence[_sentenceNum].unk3;
	_currentScript = 0xFF;
	if (_vars[VAR_SENTENCE_SCRIPT])
		runScript(_vars[VAR_SENTENCE_SCRIPT], 0, 0, _localParamList);
}

void Scumm::runInputScript(int a, int cmd, int mode)
{
	int16 args[16];
	memset(args, 0, sizeof(args));
	args[0] = a;
	args[1] = cmd;
	args[2] = mode;
	if (_vars[VAR_VERB_SCRIPT])
		runScript(_vars[VAR_VERB_SCRIPT], 0, 0, args);
}

void Scumm::decreaseScriptDelay(int amount)
{
	ScriptSlot *ss = &vm.slot[0];
	int i;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++) {
		if (ss->status == 1) {
			ss->delay -= amount;
			if (ss->delay < 0) {
				ss->status = 2;
				ss->delay = 0;
			}
		}
	}
}

void Scumm::runVerbCode(int object, int entry, int a, int b, int16 * vars)
{
	uint32 obcd;
	int slot, where, offs;

	if (!object)
		return;
	if (!b)
		stopObjectScript(object);

	where = whereIsObject(object);

	if (where == WIO_NOT_FOUND) {
		error("Code for object %d not in room %d", object, _roomResource);
	}

	obcd = getOBCDOffs(object);
	slot = getScriptSlot();

	offs = getVerbEntrypoint(object, entry);
	if (offs == 0)
		return;

	vm.slot[slot].number = object;
	vm.slot[slot].offs = obcd + offs;
	vm.slot[slot].status = ssRunning;
	vm.slot[slot].where = where;
	vm.slot[slot].unk1 = a;
	vm.slot[slot].unk2 = b;
	vm.slot[slot].freezeCount = 0;
	vm.slot[slot].newfield = 0;

	initializeLocals(slot, vars);

	runScriptNested(slot);
}

void Scumm::initializeLocals(int slot, int16 * vars)
{
	int i;
	if (!vars) {
		for (i = 0; i < 16; i++)
			vm.localvar[slot][i] = 0;
	} else {
		for (i = 0; i < 16; i++)
			vm.localvar[slot][i] = vars[i];
	}
}

int Scumm::getVerbEntrypoint(int obj, int entry)
{
	byte *objptr, *verbptr;
	int verboffs;

	if (whereIsObject(obj) == WIO_NOT_FOUND)
		return 0;

	objptr = getOBCDFromObject(obj);
	assert(objptr);

	if (_features & GF_SMALL_HEADER)
		verbptr = objptr + 19;
	else
		verbptr = findResource(MKID('VERB'), objptr);

	if (verbptr == NULL)
		error("No verb block in object %d", obj);

	verboffs = verbptr - objptr;

	if (!(_features & GF_SMALL_HEADER))
		verbptr += _resourceHeaderSize;

	do {
		if (!*verbptr)
			return 0;
		if (*verbptr == entry || *verbptr == 0xFF)
			break;
		verbptr += 3;
	} while (1);

	if (_features & GF_SMALL_HEADER)
		return READ_LE_UINT16(verbptr + 1);
	else
		return verboffs + READ_LE_UINT16(verbptr + 1);
}


void Scumm::push(int a)
{
	assert(_scummStackPos >= 0
				 && (unsigned int)_scummStackPos <= ARRAYSIZE(_scummStack));
	_scummStack[_scummStackPos++] = a;
}

int Scumm::pop()
{
	assert(_scummStackPos > 0
				 && (unsigned int)_scummStackPos <= ARRAYSIZE(_scummStack));
	return _scummStack[--_scummStackPos];
}


void Scumm::endCutscene()
{
	ScriptSlot *ss = &vm.slot[_currentScript];
	uint32 *csptr;
	int16 args[16];

	memset(args, 0, sizeof(args));

	if (ss->cutsceneOverride > 0)	// Only terminate if active
		ss->cutsceneOverride--;

	args[0] = vm.cutSceneData[vm.cutSceneStackPointer];
	_vars[VAR_OVERRIDE] = 0;

	csptr = &vm.cutScenePtr[vm.cutSceneStackPointer];
	if (*csptr && (ss->cutsceneOverride > 0))	// Only terminate if active
		ss->cutsceneOverride--;

	vm.cutSceneScript[vm.cutSceneStackPointer] = 0;
	*csptr = 0;
	vm.cutSceneStackPointer--;

	if (_vars[VAR_CUTSCENE_END_SCRIPT])
		runScript(_vars[VAR_CUTSCENE_END_SCRIPT], 0, 0, args);
}

void Scumm::cutscene(int16 * args)
{
	int scr = _currentScript;
	vm.slot[scr].cutsceneOverride++;

	if (++vm.cutSceneStackPointer >
			sizeof(vm.cutSceneData) / sizeof(vm.cutSceneData[0]))
		error("Cutscene stack overflow");

	vm.cutSceneData[vm.cutSceneStackPointer] = args[0];
	vm.cutSceneScript[vm.cutSceneStackPointer] = 0;
	vm.cutScenePtr[vm.cutSceneStackPointer] = 0;

	vm.cutSceneScriptIndex = scr;
	if (_vars[VAR_CUTSCENE_START_SCRIPT])
		runScript(_vars[VAR_CUTSCENE_START_SCRIPT], 0, 0, args);
	vm.cutSceneScriptIndex = 0xFF;
}

void Scumm::faceActorToObj(int act, int obj)
{
	int x, dir;

	if (getObjectOrActorXY(act) == -1)
		return;

	x = _xPos;

	if (getObjectOrActorXY(obj) == -1)
		return;

	dir = (_xPos > x) ? 90 : 270;
	turnToDirection(derefActorSafe(act, "faceActorToObj"), dir);
}

void Scumm::animateActor(int act, int anim)
{
	if (_features & GF_AFTER_V7) {
		int cmd, dir;
		Actor *a;

		a = derefActorSafe(act, "animateActor");

		if (anim == 0xFF)
			anim = 2000;

		cmd = anim / 1000;
		dir = anim % 1000;

		/* temporary code */
//    dir = newDirToOldDir(dir);

		switch (cmd) {
		case 2:
			stopActorMoving(a);
			startAnimActor(a, a->standFrame);
			break;
		case 3:
			a->moving &= ~4;
			fixActorDirection(a, dir);
			break;
		case 4:
			turnToDirection(a, dir);
			break;
		default:
			startAnimActor(a, anim);
		}

	} else {
		int dir;
		Actor *a;

		a = derefActorSafe(act, "animateActor");
		if (!a)
			return;

		dir = anim & 3;

		switch (anim >> 2) {
		case 0x3F:
			stopActorMoving(a);
			startAnimActor(a, a->standFrame);
			break;
		case 0x3E:
			a->moving &= ~4;
			fixActorDirection(a, oldDirToNewDir(dir));
			break;
		case 0x3D:
			turnToDirection(a, oldDirToNewDir(dir));
			break;
		default:
			startAnimActor(a, anim);
		}

	}
}

bool Scumm::isScriptRunning(int script)
{
	int i;
	ScriptSlot *ss = vm.slot;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++)
		if (ss->number == script && (ss->where == WIO_GLOBAL ||
																 ss->where == WIO_LOCAL) && ss->status)
			return true;
	return false;
}

bool Scumm::isRoomScriptRunning(int script)
{
	int i;
	ScriptSlot *ss = vm.slot;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++)
		if (ss->number == script && ss->where == WIO_ROOM && ss->status)
			return true;
	return false;

}



void Scumm::beginOverride()
{
	int idx;
	uint32 *ptr;

	idx = vm.cutSceneStackPointer;
	ptr = &vm.cutScenePtr[idx];
/*	if (!*ptr) {		// ENDER - FIXME - We don't need this?
		vm.slot[_currentScript].cutsceneOverride++;
	} 
*/
	*ptr = _scriptPointer - _scriptOrgPointer;
	vm.cutSceneScript[idx] = _currentScript;

	fetchScriptByte();
	fetchScriptWord();
	_vars[VAR_OVERRIDE] = 0;
}

void Scumm::endOverride()
{
	int idx;
	uint32 *ptr;

	idx = vm.cutSceneStackPointer;
	ptr = &vm.cutScenePtr[idx];
/*	if (!*ptr) {		// ENDER - FIXME - We don't need this?
		// vm.slot[_currentScript].cutsceneOverride--;
		//printf("ending override: %d on script %d\n", vm.slot[_currentScript].cutsceneOverride, _currentScript);
	} */
	*ptr = 0;
	vm.cutSceneScript[idx] = 0;
	_vars[VAR_OVERRIDE] = 0;
}


int Scumm::defineArray(int array, int type, int dim2, int dim1)
{
	int id;
	int size;
	ArrayHeader *ah;

	if (type != 5 && type != 4)
		type = 5;

	nukeArray(array);

	id = getArrayId();

	if (array & 0x4000) {
		_arrays[id] = (char)vm.slot[_currentScript].number;
	}

	if (array & 0x8000) {
		error("Can't define bit variable as array pointer");
	}

	writeVar(array, id);

	size = (type == 5) ? 16 : 8;
	size *= dim2 + 1;
	size *= dim1 + 1;
	size >>= 3;

	ah =
		(ArrayHeader *)createResource(rtString, id, size + sizeof(ArrayHeader));

	ah->type = type;
	ah->dim1_size = dim1 + 1;
	ah->dim2_size = dim2 + 1;

	return id;
}

void Scumm::nukeArray(int a)
{
	int data;

	data = readVar(a);

	if (data)
		nukeResource(rtString, data);
	_arrays[data] = 0;

	writeVar(a, 0);
}

int Scumm::getArrayId()
{
	byte **addr = _baseArrays;
	int i;

	for (i = 1; i < _numArray; i++) {
		if (!addr[i])
			return i;
	}
	error("Out of array pointers, %d max", _numArray);
}

void Scumm::arrayop_1(int a, byte *ptr)
{
	ArrayHeader *ah;
	int r;
	int len = getStringLen(ptr);

	r = defineArray(a, 4, 0, len);
	ah = (ArrayHeader *)getResourceAddress(rtString, r);
	copyString(ah->data, ptr, len);
}

void Scumm::copyString(byte *dst, byte *src, int len)
{
	if (!src) {
		while (--len >= 0)
			*dst++ = fetchScriptByte();
	} else {
		while (--len >= 0)
			*dst++ = *src++;
	}
}

int Scumm::getStringLen(byte *ptr)
{
	int len;
	byte c;
	if (!ptr)
		ptr = _scriptPointer;
	len = 0;
	do {
		c = *ptr++;
		if (!c)
			break;
		len++;
		if (c == 0xFF)
			ptr += 3, len += 3;
	} while (1);
	return len + 1;
}

void Scumm::exitCutscene()
{
	uint32 offs = vm.cutScenePtr[vm.cutSceneStackPointer];
	if (offs) {
		ScriptSlot *ss = &vm.slot[vm.cutSceneScript[vm.cutSceneStackPointer]];
		ss->offs = offs;
		ss->status = 2;
		ss->freezeCount = 0;

		if (ss->cutsceneOverride > 0)
			ss->cutsceneOverride--;

		_vars[VAR_OVERRIDE] = 1;
		vm.cutScenePtr[vm.cutSceneStackPointer] = 0;
	}
}
void Scumm::doSentence(int c, int b, int a)
{
	if (_features & GF_AFTER_V7) {
		SentenceTab *st;

		if (b == a)
			return;

		st = &sentence[_sentenceNum - 1];

		if (_sentenceNum && st->unk5 == c && st->unk4 == b && st->unk3 == a)
			return;

		_sentenceNum++;
		st++;

		st->unk5 = c;
		st->unk4 = b;
		st->unk3 = a;
		st->unk = 0;

		warning("dosentence(%d,%d,%d)", c, b, a);

	} else {

		SentenceTab *st;

		st = &sentence[_sentenceNum++];

		st->unk5 = c;
		st->unk4 = b;
		st->unk3 = a;

		if (!(st->unk3 & 0xFF00))
			st->unk2 = 0;
		else
			st->unk2 = 1;

		st->unk = 0;
	}
}
