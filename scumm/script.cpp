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
#include "resource.h"
#include "common/util.h"

/* Script status type (slot.status) */
enum {
	ssDead = 0,
	ssPaused = 1,
	ssRunning = 2
};


/* Start executing script 'script' with parameters 'a' and 'b' */
void Scumm::runScript(int script, int a, int b, int *lvarptr)
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
	s->status = ssRunning;
	s->where = scriptType;
	s->unk1 = a;
	s->unk2 = b;
	s->freezeCount = 0;

	s->delayFrameCount = 0;

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
		if (script != ss->number || ss->where != WIO_GLOBAL && ss->where != WIO_LOCAL || ss->status == ssDead)
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
		if (nest->number == script && (nest->where == WIO_GLOBAL || nest->where == WIO_LOCAL)) {
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
		if (script == ss->number && ss->status != ssDead &&
		    (ss->where == WIO_ROOM || ss->where == WIO_INVENTORY || ss->where == WIO_FLOBJECT)) {
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
				(nest->where == WIO_ROOM || nest->where == WIO_FLOBJECT || nest->where == WIO_INVENTORY)) {
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
		if (ss->status == ssDead)
			return i;
	}
	error("Too many scripts running, %d max", NUM_SCRIPT_SLOT);
	return -1;
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
				slot->status != ssDead && slot->freezeCount == 0) {
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
		if (_features & GF_AFTER_V8) {
			_scriptOrgPointer = getResourceAddress(rtRoomScripts, _roomResource);
			_lastCodePtr = &res.address[rtRoomScripts][_roomResource];
		} else {
			_scriptOrgPointer = getResourceAddress(rtRoom, _roomResource);
			_lastCodePtr = &_baseRooms[_roomResource];
		}
		break;

	case WIO_GLOBAL:							/* global script */
		_scriptOrgPointer = getResourceAddress(rtScript, ss->number);
		_lastCodePtr = &_baseScripts[ss->number];
		break;

	case WIO_FLOBJECT:						/* flobject script */
		idx = getObjectIndex(ss->number);
		_scriptOrgPointer = getResourceAddress(rtFlObject, _objs[idx].fl_object_index);
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
	while (_currentScript != 0xFF) {
		_opcode = fetchScriptByte();
		_scriptPointerStart = _scriptPointer;
		vm.slot[_currentScript].didexec = 1;
		debug(3, "Script %d, offset 0x%x: [%X] %s()",
				vm.slot[_currentScript].number,
				_scriptPointer - _scriptOrgPointer,
				_opcode,
				getOpcodeDesc(_opcode));
		executeOpcode(_opcode);
	}
	CHECK_HEAP;
}

byte Scumm::fetchScriptByte()
{
	if (*_lastCodePtr + sizeof(MemBlkHeader) != _scriptOrgPointer) {
		uint32 oldoffs = _scriptPointer - _scriptOrgPointer;
		getScriptBaseAddress();
		_scriptPointer = _scriptOrgPointer + oldoffs;
	}
	return *_scriptPointer++;
}

uint Scumm::fetchScriptWord()
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

int Scumm::fetchScriptWordSigned()
{
	return (int16)fetchScriptWord();
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
		if (var == 175)
			return 1;
#if defined(BYPASS_COPY_PROT)
		if (var == 490 && _gameId == GID_MONKEY2 && !copyprotbypassed) {
			copyprotbypassed = true;
			var = 518;
		}

		if (var == 179 && _gameId == GID_MONKEY_VGA && !copyprotbypassed) {
			copyprotbypassed = true;
			var = 266;
		}
#endif

		checkRange(_numVariables - 1, 0, var, "Variable %d out of range(r)");
		return _vars[var];
	}

	if (var & 0x2000 && !(_features & GF_NEW_OPCODES)) {
		a = fetchScriptWord();
		if (a & 0x2000)
			var += readVar(a & ~0x2000);
		else
			var += a & 0xFFF;
		var &= ~0x2000;
	}

	if (!(var & 0xF000))
		return _vars[var];

	if (var & 0x8000) {
		if (_gameId == GID_ZAK256) {
			// Emulate a wierd hack in Zak256 to read individual
			// bits of a normal global
			int b = (var & 0x000F);
			var &= 0x0FFF;
			var >>= 4;
			checkRange(_numVariables - 1, 0, var, "Variable %d out of range(rzb)");
			return (_vars[ var ] & ( 1 << b ) ) ? 1 : 0;
		}

		var &= 0x7FFF;
		checkRange(_numBitVariables - 1, 0, var, "Bit variable %d out of range(r)");
		return (_bitVars[var >> 3] & (1 << (var & 7))) ? 1 : 0;
	}

	if (var & 0x4000) {
		if (_gameId == GID_INDY3_256) {
			var &= 0xF;
		} else {
			var &= 0xFFF;
		}
		checkRange(0x10, 0, var, "Local variable %d out of range(r)");
		return vm.localvar[_currentScript][var];
	}

	error("Illegal varbits (r)");
	return -1;
}

void Scumm::writeVar(uint var, int value)
{
	if (!(var & 0xF000)) {
		checkRange(_numVariables - 1, 0, var, "Variable %d out of range(w)");

		// FIXME: Find some better place to put this.
		if (var == VAR_CHARINC)
			_vars[VAR_CHARINC] = _defaultTalkDelay / 20;
		else
			_vars[var] = value;

		if ((_varwatch == (int)var) || (_varwatch == 0)) {
			if (vm.slot[_currentScript].number < 100)
				debug(1, "vars[%d] = %d (via script-%d)", var, value, vm.slot[_currentScript].number);
			else
				debug(1, "vars[%d] = %d (via room-%d-%d)", var, value, _currentRoom,
							vm.slot[_currentScript].number);
		}
		return;
	}

	if (var & 0x8000) {
		if (_gameId == GID_ZAK256) {
			// Emulate a wierd hack in Zak256 to read individual
			// bits of a normal global
			int b = (var & 0x000F);
			var &= 0x7FFF;
			var >>= 4;
			checkRange(_numVariables - 1, 0, var, "Variable %d out of range(wzb)");
			if(value)
 				_vars[ var ] |= ( 1 << b );
			else
				_vars[ var ] &= ~( 1 << b );
			return;
		}
		var &= 0x7FFF;
		checkRange(_numBitVariables - 1, 0, var, "Bit variable %d out of range(w)");

		/* FIXME: Enable Indy4 mousefighting by default. 
		   is there a better place to put this? */
		if (_gameId == GID_INDY4 && var == 107 && vm.slot[_currentScript].number == 1)
			value = 1;

		if (value)
			_bitVars[var >> 3] |= (1 << (var & 7));
		else
			_bitVars[var >> 3] &= ~(1 << (var & 7));
		return;
	}

	if (var & 0x4000) {
		if (_gameId == GID_INDY3_256) {
			var &= 0xF;
		} else {
			var &= 0xFFF;
		}

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

void Scumm::push(int a)
{
	assert(_scummStackPos >= 0 && (unsigned int)_scummStackPos <= ARRAYSIZE(_scummStack));
	_scummStack[_scummStackPos++] = a;
}

int Scumm::pop()
{
	if ((_scummStackPos < 1) || ((unsigned int)_scummStackPos > ARRAYSIZE(_scummStack))) {
		error("No items on stack to pop() for %s (0x%X) at [%d-%d]\n", getOpcodeDesc(_opcode), _opcode, _roomResource, vm.slot[_currentScript].number);
	}

	return _scummStack[--_scummStackPos];
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

	if (x > _realWidth - 1)
		return;
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	if (x2 < 0)
		return;
	if (x2 > _realWidth)
		x2 = _realWidth;
	if (y2 > bottom)
		y2 = bottom;

	updateDirtyRect(vs->number, x, x2, y - top, y2 - top, 0);

	backbuff = vs->screenPtr + vs->xstart + (y - top) * _realWidth + x;

	if (color == -1) {
		if (vs->number != 0)
			error("can only copy bg to main window");
		bgbuff = getResourceAddress(rtBuffer, vs->number + 5) + vs->xstart + (y - top) * _realWidth + x;
		blit(backbuff, bgbuff, x2 - x, y2 - y);
	} else {
		count = y2 - y;
		while (count) {
			memset(backbuff, color, x2 - x);
			backbuff += _realWidth;
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
			warning("Script %d ending with active cutscene/override (%d)", ss->number, ss->cutsceneOverride);
			ss->cutsceneOverride = 0;
		}
	}
	ss->number = 0;
	ss->status = ssDead;
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
	int tmp[16];
	tmp[0] = i;
	if (_vars[VAR_HOOK_SCRIPT]) {
		runScript(_vars[VAR_HOOK_SCRIPT], 0, 0, tmp);
	}
}

void Scumm::freezeScripts(int flag)
{
	int i;

	for (i = 1; i < NUM_SCRIPT_SLOT; i++) {
		if (_currentScript != i && vm.slot[i].status != ssDead && (vm.slot[i].unk1 == 0 || flag >= 0x80)) {
			vm.slot[i].status |= 0x80;
			vm.slot[i].freezeCount++;
		}
	}

	for (i = 0; i < 6; i++)
		_sentence[i].freezeCount++;

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
		if (_sentence[i].freezeCount > 0)
			_sentence[i].freezeCount--;
	}
}

void Scumm::runAllScripts()
{
	int i;

	for (i = 0; i < NUM_SCRIPT_SLOT; i++)
		vm.slot[i].didexec = 0;

	_currentScript = 0xFF;
	for (_curExecScript = 0; _curExecScript < NUM_SCRIPT_SLOT; _curExecScript++) {
		if (vm.slot[_curExecScript].status == ssRunning && vm.slot[_curExecScript].didexec == 0) {
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

		vm.slot[slot].delayFrameCount = 0;

		// FIXME: the exit script of room 7 in indy3 only seems to have a size and tag
		// not actual data not even a 00 (stop code)
		// maybe we should be limiting ourselves to strictly reading the size 
		// described in the header?
		if (_gameId == GID_INDY3_256) {
			// FIXME: Oddly, Indy3 seems to contain exit scripts with only a size
			// and a tag - not even a terminating NULL!
			byte *roomptr = getResourceAddress(rtRoom, _roomResource);
			byte *excd = findResourceData(MKID('EXCD'), roomptr) - _resourceHeaderSize;
			if (!excd || (getResourceDataSize(excd) < 1)) {
				debug(2, "Exit-%d is empty", _roomResource);
				return;
			}
		}

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
		vm.slot[slot].delayFrameCount = 0;
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
				error("Object %d stopped with active cutscene/override in exit", ss->number);
			ss->status = ssDead;
		} else if (ss->where == WIO_LOCAL) {
			if (ss->cutsceneOverride)
				error("Script %d stopped with active cutscene/override in exit", ss->number);
			ss->status = ssDead;
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

void Scumm::killAllScriptsExceptCurrent()
{
	for (int i = 1; i < NUM_SCRIPT_SLOT; i++) {
		if (i != _currentScript)
			vm.slot[i].status = ssDead;
	}
}

void Scumm::checkAndRunSentenceScript()
{
	int i;
	ScriptSlot *ss;

	memset(_localParamList, 0, sizeof(_localParamList));
	if (isScriptInUse(_vars[VAR_SENTENCE_SCRIPT])) {
		ss = vm.slot;
		for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++)
			if (ss->number == _vars[VAR_SENTENCE_SCRIPT] && ss->status != ssDead && ss->freezeCount == 0)
				return;
	}

	if (!_sentenceNum || _sentence[_sentenceNum - 1].freezeCount)
		return;

	_sentenceNum--;

	if (!(_features & GF_AFTER_V7))
		if (_sentence[_sentenceNum].unk2 && _sentence[_sentenceNum].unk3 == _sentence[_sentenceNum].unk4)
			return;

	_localParamList[0] = _sentence[_sentenceNum].unk5;
	_localParamList[1] = _sentence[_sentenceNum].unk4;
	_localParamList[2] = _sentence[_sentenceNum].unk3;
	_currentScript = 0xFF;
	if (_vars[VAR_SENTENCE_SCRIPT])
		runScript(_vars[VAR_SENTENCE_SCRIPT], 0, 0, _localParamList);
}

void Scumm::runInputScript(int a, int cmd, int mode)
{
	int args[16];
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
		if (ss->status == ssPaused) {
			ss->delay -= amount;
			if (ss->delay < 0) {
				ss->status = ssRunning;
				ss->delay = 0;
			}
		}
	}
}

void Scumm::runVerbCode(int object, int entry, int a, int b, int *vars)
{
	uint32 obcd;
	int slot, where, offs;

	if (!object)
		return;
	if (!b)
		stopObjectScript(object);

	where = whereIsObject(object);

	if (where == WIO_NOT_FOUND) {
		warning("Code for object %d not in room %d", object, _roomResource);
		return;
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
	vm.slot[slot].delayFrameCount = 0;

	initializeLocals(slot, vars);

	runScriptNested(slot);
}

void Scumm::initializeLocals(int slot, int *vars)
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

void Scumm::endCutscene()
{
	ScriptSlot *ss = &vm.slot[_currentScript];
	uint32 *csptr;
	int args[16];

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

void Scumm::cutscene(int *args)
{
	int scr = _currentScript;
	vm.slot[scr].cutsceneOverride++;

	if (++vm.cutSceneStackPointer > sizeof(vm.cutSceneData) / sizeof(vm.cutSceneData[0]))
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
	int x, x2, y, dir;

	if (getObjectOrActorXY(act, x, y) == -1)
		return;

	if (getObjectOrActorXY(obj, x2, y) == -1)
		return;

	dir = (x2 > x) ? 90 : 270;
	derefActorSafe(act, "faceActorToObj")->turnToDirection(dir);
}

bool Scumm::isScriptRunning(int script)
{
	int i;
	ScriptSlot *ss = vm.slot;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++)
		if (ss->number == script && (ss->where == WIO_GLOBAL || ss->where == WIO_LOCAL) && ss->status != ssDead)
			return true;
	return false;
}

bool Scumm::isRoomScriptRunning(int script)
{
	int i;
	ScriptSlot *ss = vm.slot;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++)
		if (ss->number == script && ss->where == WIO_ROOM && ss->status != ssDead)
			return true;
	return false;

}


void Scumm::beginOverride()
{
	int idx;
	uint32 *ptr;

	idx = vm.cutSceneStackPointer;
	ptr = &vm.cutScenePtr[idx];

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

	if (_features & GF_AFTER_V8) {
		if (array & 0x40000000) {
			_arrays[id] = (char)vm.slot[_currentScript].number;
		}
	
		if (array & 0x80000000) {
			error("Can't define bit variable as array pointer");
		}

		size = (type == 5) ? 32 : 8;
	} else {
		if (array & 0x4000) {
			_arrays[id] = (char)vm.slot[_currentScript].number;
		}
	
		if (array & 0x8000) {
			error("Can't define bit variable as array pointer");
		}

		size = (type == 5) ? 16 : 8;
	}

	writeVar(array, id);

	size *= dim2 + 1;
	size *= dim1 + 1;
	size >>= 3;

	ah = (ArrayHeader *)createResource(rtString, id, size + sizeof(ArrayHeader));

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
	return -1;
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
		ss->status = ssRunning;
		ss->freezeCount = 0;

		if (ss->cutsceneOverride > 0)
			ss->cutsceneOverride--;

		_vars[VAR_OVERRIDE] = 1;
		vm.cutScenePtr[vm.cutSceneStackPointer] = 0;
	}
}
void Scumm::doSentence(int c, int b, int a)
{
	SentenceTab *st;

	if (_features & GF_AFTER_V7) {

		if (b == a)
			return;

		st = &_sentence[_sentenceNum - 1];
		
		
		// Check if this doSentence request is identical to the previous one;
		// if yes, ignore this invocation.
		if (_sentenceNum && st->unk5 == c && st->unk4 == b && st->unk3 == a)
			return;

		_sentenceNum++;
		st++;

		warning("doSentence(%d,%d,%d)", c, b, a);

	} else {

		st = &_sentence[_sentenceNum++];

		if (!(st->unk3 & 0xFF00))
			st->unk2 = 0;
		else
			st->unk2 = 1;

	}

	st->unk5 = c;
	st->unk4 = b;
	st->unk3 = a;
	st->freezeCount = 0;
}
