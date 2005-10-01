/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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

#include "common/stdafx.h"

#include "common/config-manager.h"
#include "common/util.h"

#include "scumm/actor.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/util.h"
#include "scumm/scumm.h"
#include "scumm/verbs.h"

namespace Scumm {

/* Start executing script 'script' with the given parameters */
void ScummEngine::runScript(int script, bool freezeResistant, bool recursive, int *lvarptr, int cycle) {
	ScriptSlot *s;
	byte *scriptPtr;
	uint32 scriptOffs;
	byte scriptType;
	int slot;

	if (!script)
		return;

	if (!recursive)
		stopScript(script);

	if (script < _numGlobalScripts) {
		scriptPtr = getResourceAddress(rtScript, script);
		scriptOffs = _resourceHeaderSize;
		scriptType = WIO_GLOBAL;

		debugC(DEBUG_SCRIPTS, "runScript(Global-%d) from %d-%d", script,
				       vm.slot[_currentScript].number, _roomResource);
	} else {
		scriptOffs = _localScriptOffsets[script - _numGlobalScripts];
		if (scriptOffs == 0)
			error("Local script %d is not in room %d", script, _roomResource);
		scriptType = WIO_LOCAL;

		debugC(DEBUG_SCRIPTS, "runScript(%d) from %d-%d", script,
				       vm.slot[_currentScript].number, _roomResource);
	}

	if (cycle == 0)
		cycle = (_heversion >= 90) ? VAR(VAR_SCRIPT_CYCLE) : 1;

	slot = getScriptSlot();

	s = &vm.slot[slot];
	s->number = script;
	s->offs = scriptOffs;
	s->status = ssRunning;
	s->where = scriptType;
	s->freezeResistant = freezeResistant;
	s->recursive = recursive;
	s->freezeCount = 0;
	s->delayFrameCount = 0;
	s->cycle = cycle;

	initializeLocals(slot, lvarptr);

	runScriptNested(slot);
}

void ScummEngine::runObjectScript(int object, int entry, bool freezeResistant, bool recursive, int *vars, int slot, int cycle) {
	ScriptSlot *s;
	uint32 obcd;
	int where, offs;

	if (!object)
		return;

	if (!recursive && (_version >= 3))
		stopObjectScript(object);

	where = whereIsObject(object);

	if (where == WIO_NOT_FOUND) {
		warning("Code for object %d not in room %d", object, _roomResource);
		return;
	}

	obcd = getOBCDOffs(object);

	// Find a free object slot, unless one was specified
	if (slot == -1)
		slot = getScriptSlot();

	offs = getVerbEntrypoint(object, entry);
	if (offs == 0)
		return;

	if (cycle == 0)
		cycle = (_heversion >= 90) ? VAR(VAR_SCRIPT_CYCLE) : 1;

	s = &vm.slot[slot];
	s->number = object;
	s->offs = obcd + offs;
	s->status = ssRunning;
	s->where = where;
	s->freezeResistant = freezeResistant;
	s->recursive = recursive;
	s->freezeCount = 0;
	s->delayFrameCount = 0;
	s->cycle = cycle;

	initializeLocals(slot, vars);

	runScriptNested(slot);
}

void ScummEngine::initializeLocals(int slot, int *vars) {
	int i;
	if (!vars) {
		for (i = 0; i < 25; i++)
			vm.localvar[slot][i] = 0;
	} else {
		for (i = 0; i < 25; i++)
			vm.localvar[slot][i] = vars[i];
	}
}

int ScummEngine::getVerbEntrypoint(int obj, int entry) {
	const byte *objptr, *verbptr;
	int verboffs;

	if (whereIsObject(obj) == WIO_NOT_FOUND)
		return 0;

	objptr = getOBCDFromObject(obj);
	assert(objptr);

	if (_platform == Common::kPlatformC64 && _gameId == GID_MANIAC)
		verbptr = objptr + 14;
	else if (_version <= 2)
		verbptr = objptr + 15;
	else if (_features & GF_OLD_BUNDLE)
		verbptr = objptr + 17;
	else if (_features & GF_SMALL_HEADER)
		verbptr = objptr + 19;
	else
		verbptr = findResource(MKID('VERB'), objptr);

	assert(verbptr);

	verboffs = verbptr - objptr;

	if (!(_features & GF_SMALL_HEADER))
		verbptr += _resourceHeaderSize;

	if (_version == 8) {
		const uint32 *ptr = (const uint32 *)verbptr;
		uint32 verb;
		do {
			verb = READ_LE_UINT32(ptr);
			if (!verb)
				return 0;
			if (verb == (uint32)entry || verb == 0xFFFFFFFF)
				break;
			ptr += 2;
		} while (1);
		return verboffs + 8 + READ_LE_UINT32(ptr + 1);
	} if (_version <= 2) {
		do {
			if (!*verbptr)
				return 0;
			if (*verbptr == entry || *verbptr == 0xFF)
				break;
			verbptr += 2;
		} while (1);

		return *(verbptr + 1);
	} else {
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
}

/* Stop script 'script' */
void ScummEngine::stopScript(int script) {
	ScriptSlot *ss;
	NestedScript *nest;
	int i, num;

	if (script == 0)
		return;

	ss = vm.slot;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++) {
		if (script == ss->number && ss->status != ssDead &&
			(ss->where == WIO_GLOBAL || ss->where == WIO_LOCAL)) {
			if (ss->cutsceneOverride)
				if (_version >= 5)
					error("Script %d stopped with active cutscene/override", script);
			ss->number = 0;
			ss->status = ssDead;
			nukeArrays(i);
			if (_currentScript == i)
				_currentScript = 0xFF;
		}
	}

	nest = vm.nest;
	num = vm.numNestedScripts;

	while (num > 0) {
		if (nest->number == script &&
				(nest->where == WIO_GLOBAL || nest->where == WIO_LOCAL)) {
			nukeArrays(nest->slot);
			nest->number = 0xFF;
			nest->slot = 0xFF;
			nest->where = 0xFF;
		}
		nest++;
		num--;
	}
}

/* Stop an object script 'script'*/
void ScummEngine::stopObjectScript(int script) {
	ScriptSlot *ss;
	NestedScript *nest;
	int i, num;

	if (script == 0)
		return;

	ss = vm.slot;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++) {
		if (script == ss->number && ss->status != ssDead &&
		    (ss->where == WIO_ROOM || ss->where == WIO_INVENTORY || ss->where == WIO_FLOBJECT)) {
			if (ss->cutsceneOverride)
				if (_version >= 5)
					error("Object %d stopped with active cutscene/override", script);
			ss->number = 0;
			ss->status = ssDead;
			nukeArrays(i);
			if (_currentScript == i)
				_currentScript = 0xFF;
		}
	}

	nest = vm.nest;
	num = vm.numNestedScripts;

	while (num > 0) {
		if (nest->number == script &&
				(nest->where == WIO_ROOM || nest->where == WIO_INVENTORY || nest->where == WIO_FLOBJECT)) {
			nukeArrays(nest->slot);
			nest->number = 0xFF;
			nest->slot = 0xFF;
			nest->where = 0xFF;
		}
		nest++;
		num--;
	}
}

/* Return a free script slot */
int ScummEngine::getScriptSlot() {
	ScriptSlot *s;
	int i;

	for (i = 1; i < NUM_SCRIPT_SLOT; i++) {
		s = &vm.slot[i];
		if (s->status == ssDead)
			return i;
	}
	error("Too many scripts running, %d max", NUM_SCRIPT_SLOT);
	return -1;
}

/* Run script 'script' nested - eg, within the parent script.*/
void ScummEngine::runScriptNested(int script) {
	NestedScript *nest;
	ScriptSlot *slot;

	updateScriptPtr();

	nest = &vm.nest[vm.numNestedScripts];

	if (_currentScript == 0xFF) {
		nest->number = 0xFF;
		nest->where = 0xFF;
	} else {
		// Store information about the currently running script
		slot = &vm.slot[_currentScript];
		nest->number = slot->number;
		nest->where = slot->where;
		nest->slot = _currentScript;
	}

	vm.numNestedScripts++;

	if (vm.numNestedScripts > ARRAYSIZE(vm.nest))
		error("Too many nested scripts");

	_currentScript = script;
	getScriptBaseAddress();
	getScriptEntryPoint();
	executeScript();

	vm.numNestedScripts--;

	if (nest->number != 0xFF) {
		// Try to resume the script which called us, if its status has not changed
		// since it invoked us. In particular, we only resume it if it hasn't been
		// stopped in the meantime, and if it did not already move on.
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

void ScummEngine::updateScriptPtr() {
	if (_currentScript == 0xFF)
		return;

	vm.slot[_currentScript].offs = _scriptPointer - _scriptOrgPointer;
}

/* Nuke arrays based on script */
void ScummEngine::nukeArrays(byte scriptSlot) {
	int i;

	if (_heversion == 0 || scriptSlot == 0)
		return;

	for (i = 1; i < _numArray; i++) {
		if (_arraySlot[i] == scriptSlot) {
			res.nukeResource(rtString, i);
			_arraySlot[i] = 0;
		}
	}
}

/* Get the code pointer to a script */
void ScummEngine::getScriptBaseAddress() {
	ScriptSlot *ss;
	int idx;

	if (_currentScript == 0xFF)
		return;

	ss = &vm.slot[_currentScript];
	switch (ss->where) {
	case WIO_INVENTORY:					/* inventory script * */
		for (idx = 0; idx < _numInventory; idx++)
			if (_inventory[idx] == ss->number)
				break;
		_scriptOrgPointer = getResourceAddress(rtInventory, idx);
		assert(idx < _numInventory);
		_lastCodePtr = &res.address[rtInventory][idx];
		break;

	case WIO_LOCAL:
	case WIO_ROOM:								/* room script */
		if (_version == 8) {
			_scriptOrgPointer = getResourceAddress(rtRoomScripts, _roomResource);
			assert(_roomResource < res.num[rtRoomScripts]);
			_lastCodePtr = &res.address[rtRoomScripts][_roomResource];
		} else {
			_scriptOrgPointer = getResourceAddress(rtRoom, _roomResource);
			assert(_roomResource < _numRooms);
			_lastCodePtr = &res.address[rtRoom][_roomResource];
		}
		break;

	case WIO_GLOBAL:							/* global script */
		_scriptOrgPointer = getResourceAddress(rtScript, ss->number);
		assert(ss->number < _numScripts);
		_lastCodePtr = &res.address[rtScript][ss->number];
		break;

	case WIO_FLOBJECT:						/* flobject script */
		idx = getObjectIndex(ss->number);
		assert(idx != -1);
		idx = _objs[idx].fl_object_index;
		_scriptOrgPointer = getResourceAddress(rtFlObject, idx);
		assert(idx < _numFlObject);
		_lastCodePtr = &res.address[rtFlObject][idx];
		break;
	default:
		error("Bad type while getting base address");
	}

	// The following fixes bug #1202487. Confirmed against disasm.
	if (_version <= 2 && _scriptOrgPointer == NULL) {
		ss->status = ssDead;
		_currentScript = 0xFF;
	}
}


void ScummEngine::getScriptEntryPoint() {
	if (_currentScript == 0xFF)
		return;
	_scriptPointer = _scriptOrgPointer + vm.slot[_currentScript].offs;
}

/* Execute a script - Read opcode, and execute it from the table */
void ScummEngine::executeScript() {
	int c;
	while (_currentScript != 0xFF) {

		if (_showStack == 1) {
			printf("Stack:");
			for (c=0; c < _scummStackPos; c++) {
				printf(" %d", _vmStack[c]);
			}
			printf("\n");
		}
		_opcode = fetchScriptByte();
		vm.slot[_currentScript].didexec = 1;
		debugC(DEBUG_OPCODES, "Script %d, offset 0x%x: [%X] %s()",
				vm.slot[_currentScript].number,
				_scriptPointer - _scriptOrgPointer,
				_opcode,
				getOpcodeDesc(_opcode));
		if (_hexdumpScripts == true) {
			for (c= -1; c < 15; c++) {
				printf(" %02x", *(_scriptPointer + c));
			}
			printf("\n");
		}
		executeOpcode(_opcode);
	}
	CHECK_HEAP;
}

byte ScummEngine::fetchScriptByte() {
	if (*_lastCodePtr + sizeof(MemBlkHeader) != _scriptOrgPointer) {
		long oldoffs = _scriptPointer - _scriptOrgPointer;
		getScriptBaseAddress();
		_scriptPointer = _scriptOrgPointer + oldoffs;
	}
	return *_scriptPointer++;
}

uint ScummEngine::fetchScriptWord() {
	int a;
	if (*_lastCodePtr + sizeof(MemBlkHeader) != _scriptOrgPointer) {
		long oldoffs = _scriptPointer - _scriptOrgPointer;
		getScriptBaseAddress();
		_scriptPointer = _scriptOrgPointer + oldoffs;
	}
	a = READ_LE_UINT16(_scriptPointer);
	_scriptPointer += 2;
	return a;
}

int ScummEngine::fetchScriptWordSigned() {
	return (int16)fetchScriptWord();
}

int ScummEngine::readVar(uint var) {
	int a;

	debugC(DEBUG_VARS, "readvar(%d)", var);

	if ((var & 0x2000) && (_version <= 5)) {
		a = fetchScriptWord();
		if (a & 0x2000)
			var += readVar(a & ~0x2000);
		else
			var += a & 0xFFF;
		var &= ~0x2000;
	}

	if (!(var & 0xF000)) {
		if (!_copyProtection) {
			if (var == 490 && _gameId == GID_MONKEY2) {
				var = 518;
			}
		}

		if (VAR_SUBTITLES != 0xFF && var == VAR_SUBTITLES) {
			return ConfMan.getBool("subtitles");
		}
		if (VAR_NOSUBTITLES != 0xFF && var == VAR_NOSUBTITLES) {
			return !ConfMan.getBool("subtitles");
		}

		checkRange(_numVariables - 1, 0, var, "Variable %d out of range(r)");
		return _scummVars[var];
	}

	if (var & 0x8000) {
		if (_heversion >= 80) {
			var &= 0xFFF;
			checkRange(_numRoomVariables - 1, 0, var, "Room variable %d out of range(w)");
			return _roomVars[var];

		} else if ((_gameId == GID_ZAK256) || (_features & GF_OLD_BUNDLE) ||
			(_gameId == GID_LOOM && (_platform == Common::kPlatformFMTowns))) {
			int bit = var & 0xF;
			var = (var >> 4) & 0xFF;

			if (!_copyProtection) {
				if (_gameId == GID_LOOM && (_platform == Common::kPlatformFMTowns) && var == 214 && bit == 15) {
					return 0;
				} else if (_gameId == GID_ZAK256 && var == 151 && bit == 8) {
					return 0;
				}
			}

			checkRange(_numVariables - 1, 0, var, "Variable %d out of range(rzb)");
			return (_scummVars[ var ] & ( 1 << bit ) ) ? 1 : 0;
		} else {
			var &= 0x7FFF;
			if (!_copyProtection) {
				if (_gameId == GID_INDY3 && (_platform == Common::kPlatformFMTowns) && var == 1508)
					return 0;
			}

			checkRange(_numBitVariables - 1, 0, var, "Bit variable %d out of range(r)");
			return (_bitVars[var >> 3] & (1 << (var & 7))) ? 1 : 0;
		}
	}

	if (var & 0x4000) {
		if (_features & GF_FEW_LOCALS) {
			var &= 0xF;
		} else {
			var &= 0xFFF;
		}

		if (_heversion >= 80)
			checkRange(25, 0, var, "Local variable %d out of range(r)");
		else
			checkRange(20, 0, var, "Local variable %d out of range(r)");
		return vm.localvar[_currentScript][var];
	}

	error("Illegal varbits (r)");
	return -1;
}

void ScummEngine::writeVar(uint var, int value) {
	debugC(DEBUG_VARS, "writeVar(%d, %d)", var, value);

	if (!(var & 0xF000)) {
		checkRange(_numVariables - 1, 0, var, "Variable %d out of range(w)");

		if (VAR_SUBTITLES != 0xFF && var == VAR_SUBTITLES) {
			assert(value == 0 || value == 1);
			ConfMan.set("subtitles", value);
		}
		if (VAR_NOSUBTITLES != 0xFF && var == VAR_NOSUBTITLES) {
			// Ignore default setting in HE60/61 games
			if ((_heversion >= 60 && _heversion <= 61) && vm.slot[_currentScript].number == 1)
				return;
			assert(value == 0 || value == 1);
			ConfMan.set("subtitles", !value);
		}

		if (var == VAR_CHARINC && ConfMan.hasKey("talkspeed")) {
			uint talkspeed = ConfMan.getInt("talkspeed");
			if (talkspeed <= 9)
				VAR(VAR_CHARINC) = talkspeed;
		} else {
			_scummVars[var] = value;
		}

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
		if (_heversion >= 80) {
			var &= 0xFFF;
			checkRange(_numRoomVariables - 1, 0, var, "Room variable %d out of range(w)");
			_roomVars[var] = value;

		} else if ((_gameId == GID_ZAK256) || (_features & GF_OLD_BUNDLE) ||
			(_gameId == GID_LOOM && (_platform == Common::kPlatformFMTowns))) {
			// In the old games, the bit variables were using the same memory
			// as the normal variables!
			int bit = var & 0xF;
			var = (var >> 4) & 0xFF;
			checkRange(_numVariables - 1, 0, var, "Variable %d out of range(wzb)");
			if (value)
				_scummVars[var] |= ( 1 << bit );
			else
				_scummVars[var] &= ~( 1 << bit );
		} else {
			var &= 0x7FFF;
			checkRange(_numBitVariables - 1, 0, var, "Bit variable %d out of range(w)");

			if (value)
				_bitVars[var >> 3] |= (1 << (var & 7));
			else
				_bitVars[var >> 3] &= ~(1 << (var & 7));
		}
		return;
	}

	if (var & 0x4000) {
		if (_features & GF_FEW_LOCALS) {
			var &= 0xF;
		} else {
			var &= 0xFFF;
		}

		if (_heversion >= 80)
			checkRange(25, 0, var, "Local variable %d out of range(w)");
		else
			checkRange(20, 0, var, "Local variable %d out of range(w)");

		vm.localvar[_currentScript][var] = value;
		return;
	}

	error("Illegal varbits (w)");
}

void ScummEngine::getResultPos() {
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

void ScummEngine::setResult(int value) {
	writeVar(_resultVarNumber, value);
}

void ScummEngine::push(int a) {
	assert(_scummStackPos >= 0 && _scummStackPos < ARRAYSIZE(_vmStack));
	debug(9, "push %d", a);
	_vmStack[_scummStackPos++] = a;
}

int ScummEngine::pop() {
	if (_scummStackPos < 1 || _scummStackPos > ARRAYSIZE(_vmStack)) {
		error("No items on stack to pop() for %s (0x%X) at [%d-%d]", getOpcodeDesc(_opcode), _opcode, _roomResource, vm.slot[_currentScript].number);
	}
	--_scummStackPos;
	debug(9, "pop %d", _vmStack[_scummStackPos]);
	return _vmStack[_scummStackPos];
}

void ScummEngine::stopObjectCode() {
	ScriptSlot *ss;

	ss = &vm.slot[_currentScript];
	if (ss->cutsceneOverride == 255) {	/* FIXME: What does this? */
		warning("Cutscene for script %d has overflown. Resetting.", ss->number);
		ss->cutsceneOverride = 0;
	}

	if (ss->where != WIO_GLOBAL && ss->where != WIO_LOCAL) {
		if (ss->cutsceneOverride) {
			if (_version >= 5)
				warning("Object %d ending with active cutscene/override (%d)", ss->number, ss->cutsceneOverride);
			ss->cutsceneOverride = 0;
		}
	} else {
		if (ss->cutsceneOverride) {
			if (_version >= 5)
				warning("Script %d ending with active cutscene/override (%d)", ss->number, ss->cutsceneOverride);
			ss->cutsceneOverride = 0;
		}
	}
	nukeArrays(_currentScript);
	ss->number = 0;
	ss->status = ssDead;
	_currentScript = 0xFF;
}

void ScummEngine::runInventoryScript(int i) {
	int args[24];
	memset(args, 0, sizeof(args));
	args[0] = i;
	if (VAR(VAR_INVENTORY_SCRIPT)) {
		if (_gameId == GID_INDY3 && _platform == Common::kPlatformMacintosh) {
			inventoryScript(args);
		} else {
			runScript(VAR(VAR_INVENTORY_SCRIPT), 0, 0, args);
		}
	}
}

void ScummEngine::inventoryScript(int *args) {
	VerbSlot *vs;
	int j, slot;

	if (VAR(67) < 0) {
		VAR(67) = 0;
	}
	args[5] = getInventoryCount(VAR(VAR_EGO));
	if (args[5] <= 6) {
		VAR(67) = 0;
	}
	if (args[5] >= 6) {
		args[5] -= 6;
	}
	args[6] = 0;
	if (VAR(67) >= args[5]) {
		VAR(67) = args[5];
		args[4] = args[5];
		args[5] /= 2;
		args[5] *= 2;
		args[4] -= args[5];
		if (args[4]) {
			VAR(67)++;
		}
		args[6]++;
	}
	args[2] = 1;
	for (j = 1; j < 7; j++) {
		args[1] = (VAR(67) + args[2]);
		args[3] = findInventory(VAR(VAR_EGO),args[1]);
		VAR(82 + args[2]) = args[3];
		args[2]++;
	}

	byte tmp[6];

	tmp[0] = 0xFF;
	tmp[1] = 0x06;
	tmp[3] = 0x00;
	tmp[4] = 0x00;

	for (j = 0; j < 6; j++) {
		tmp[2] = 0x53 + j;

		slot = getVerbSlot(101 + j, 0);
		vs = &_verbs[slot];
		loadPtrToResource(rtVerb, slot, tmp);
		vs->type = kTextVerbType;
		vs->imgindex = 0;
		vs->curmode = 1;
		drawVerb(slot, 0);
	}

	args[5] = getInventoryCount(VAR(VAR_EGO));
	if (args[5] > 6) {
		slot = getVerbSlot(107, 0);
		if (VAR(67)) {
			vs = &_verbs[slot];
			vs->curmode = 1;
		} else {
			vs = &_verbs[slot];
			vs->curmode = 0;
		}
		drawVerb(slot, 0);
		slot = getVerbSlot(108, 0);
		if (!args[6]) {
			vs = &_verbs[slot];
			vs->curmode = 1;
		} else {
			vs = &_verbs[slot];
			vs->curmode = 0;
		}
		drawVerb(slot, 0);
	} else {
		slot = getVerbSlot(107, 0);
		vs = &_verbs[slot];
		vs->curmode = 0;
		drawVerb(slot, 0);
		verbMouseOver(0);
		slot = getVerbSlot(108, 0);
		vs = &_verbs[slot];
		vs->curmode = 0;
		drawVerb(slot, 0);
	}

	verbMouseOver(0);
}

void ScummEngine::freezeScripts(int flag) {
	int i;

	if (_version <= 2) {
		for (i = 0; i < NUM_SCRIPT_SLOT; i++) {
			if (_currentScript != i && vm.slot[i].status != ssDead && !vm.slot[i].freezeResistant) {
				vm.slot[i].status |= 0x80;
				vm.slot[i].freezeCount = 1;
			}
		}
		return;
	}

	for (i = 0; i < NUM_SCRIPT_SLOT; i++) {
		if (_currentScript != i && vm.slot[i].status != ssDead && (!vm.slot[i].freezeResistant || flag >= 0x80)) {
			vm.slot[i].status |= 0x80;
			vm.slot[i].freezeCount++;
		}
	}

	for (i = 0; i < NUM_SENTENCE; i++)
		_sentence[i].freezeCount++;

	if (vm.cutSceneScriptIndex != 0xFF) {
		vm.slot[vm.cutSceneScriptIndex].status &= 0x7F;
		vm.slot[vm.cutSceneScriptIndex].freezeCount = 0;
	}
}

void ScummEngine::unfreezeScripts() {
	int i;

	if (_version <= 2) {
		for (i = 0; i < NUM_SCRIPT_SLOT; i++) {
			vm.slot[i].status &= 0x7F;
			vm.slot[i].freezeCount = 0;
		}
		return;
	}

	for (i = 0; i < NUM_SCRIPT_SLOT; i++) {
		if (vm.slot[i].status & 0x80) {
			if (!--vm.slot[i].freezeCount) {
				vm.slot[i].status &= 0x7F;
			}
		}
	}

	for (i = 0; i < NUM_SENTENCE; i++) {
		if (_sentence[i].freezeCount > 0)
			_sentence[i].freezeCount--;
	}
}


void ScummEngine::runAllScripts() {
	int i;

	for (i = 0; i < NUM_SCRIPT_SLOT; i++)
		vm.slot[i].didexec = 0;

	// FIXME - why is _curExecScript?!? The only place it is ever set is here.
	// The outer world will only see it as consequence of the calls made in the following
	// for loop. But in that case, _curExecScript will be equal to _currentScript. Hence
	// it would seem we can replace all occurances of _curExecScript by _currentScript.
	_currentScript = 0xFF;
	int numCycles = (_heversion >= 90) ? VAR(VAR_NUM_SCRIPT_CYCLES) : 1;

	for (int cycle = 1; cycle <= numCycles; cycle++) {
		for (_curExecScript = 0; _curExecScript < NUM_SCRIPT_SLOT; _curExecScript++) {
			if (vm.slot[_curExecScript].cycle == cycle && vm.slot[_curExecScript].status == ssRunning && vm.slot[_curExecScript].didexec == 0) {
				_currentScript = (byte)_curExecScript;
				getScriptBaseAddress();
				getScriptEntryPoint();
				executeScript();
			}
		}
	}
}

void ScummEngine::runExitScript() {
	if (_version > 2 && VAR(VAR_EXIT_SCRIPT))
		runScript(VAR(VAR_EXIT_SCRIPT), 0, 0, 0);
	if (_EXCD_offs) {
		int slot = getScriptSlot();
		vm.slot[slot].status = ssRunning;
		vm.slot[slot].number = 10001;
		vm.slot[slot].where = WIO_ROOM;
		vm.slot[slot].offs = _EXCD_offs;
		vm.slot[slot].freezeResistant = false;
		vm.slot[slot].recursive = false;
		vm.slot[slot].freezeCount = 0;
		vm.slot[slot].delayFrameCount = 0;
		vm.slot[slot].cycle = 1;

		// FIXME: the exit script of room 7 in indy3 only seems to have a size
		// and tag not actual data not even a 00 (stop code). Maybe we should
		// be limiting ourselves to strictly reading the size from the header?
		if (_gameId == GID_INDY3 && !(_features & GF_OLD_BUNDLE)) {
			byte *roomptr = getResourceAddress(rtRoom, _roomResource);
			const byte *excd = findResourceData(MKID('EXCD'), roomptr) - _resourceHeaderSize;
			if (!excd || (getResourceDataSize(excd) < 1)) {
				debug(2, "Exit-%d is empty", _roomResource);
				return;
			}
		}

		initializeLocals(slot, 0);
		runScriptNested(slot);
	}
	if (_version > 2 && VAR(VAR_EXIT_SCRIPT2))
		runScript(VAR(VAR_EXIT_SCRIPT2), 0, 0, 0);
}

void ScummEngine::runEntryScript() {
	if (_version > 2 && VAR(VAR_ENTRY_SCRIPT))
		runScript(VAR(VAR_ENTRY_SCRIPT), 0, 0, 0);
	if (_ENCD_offs) {
		int slot = getScriptSlot();
		vm.slot[slot].status = ssRunning;
		vm.slot[slot].number = 10002;
		vm.slot[slot].where = WIO_ROOM;
		vm.slot[slot].offs = _ENCD_offs;
		vm.slot[slot].freezeResistant = false;
		vm.slot[slot].recursive = false;
		vm.slot[slot].freezeCount = 0;
		vm.slot[slot].delayFrameCount = 0;
		vm.slot[slot].cycle = 1;
		initializeLocals(slot, 0);
		runScriptNested(slot);
	}
	if (_version > 2 && VAR(VAR_ENTRY_SCRIPT2))
		runScript(VAR(VAR_ENTRY_SCRIPT2), 0, 0, 0);
}

void ScummEngine::killScriptsAndResources() {
	ScriptSlot *ss;
	int i;

	ss = vm.slot;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++) {
		if (ss->where == WIO_ROOM || ss->where == WIO_FLOBJECT) {
			if (ss->cutsceneOverride) {
				if (_version >= 5)
					warning("Object %d stopped with active cutscene/override in exit", ss->number);
				ss->cutsceneOverride = 0;
			}
			nukeArrays(i);
			ss->status = ssDead;
		} else if (ss->where == WIO_LOCAL) {
			if (ss->cutsceneOverride) {
				if (_version >= 5)
					warning("Script %d stopped with active cutscene/override in exit", ss->number);
				ss->cutsceneOverride = 0;
			}
			nukeArrays(i);
			ss->status = ssDead;
		}
	}

	/* Nuke local object names */
	if (_newNames) {
		for (i = 0; i < _numNewNames; i++) {
			const int obj = _newNames[i];
			if (obj) {
				const int owner = getOwner(obj);
				// We can delete custom name resources if either the object is
				// no longer in use (i.e. not owned by anyone anymore); or if
				// it is an object which is owned by a room.
				if (owner == 0 || (_version < 7 && owner == OF_OWNER_ROOM)) {
					// WORKAROUND for a problem mentioned in bug report #941275:
					// In FOA in the sentry room, in the chest plate of the statue,
					// the pegs may be renamed to mouth: this custom name is lost
					// when leaving the room; this hack prevents this).
					if (owner == OF_OWNER_ROOM && _gameId == GID_INDY4 && 336 <= obj && obj <= 340)
						continue;

					_newNames[i] = 0;
					res.nukeResource(rtObjectName, i);
				}
			}
		}
	}
}

void ScummEngine::killAllScriptsExceptCurrent() {
	for (int i = 0; i < NUM_SCRIPT_SLOT; i++) {
		if (i != _currentScript) {
			vm.slot[i].status = ssDead;
			if (_version == 6)
				vm.slot[i].cutsceneOverride = 0;
		}
	}
}

void ScummEngine::doSentence(int verb, int objectA, int objectB) {
	SentenceTab *st;

	if (_version >= 7) {

		if (objectA == objectB)
			return;

		if (_sentenceNum) {
			st = &_sentence[_sentenceNum - 1];

			// Check if this doSentence request is identical to the previous one;
			// if yes, ignore this invocation.
			if (_sentenceNum && st->verb == verb && st->objectA == objectA && st->objectB == objectB)
				return;
		}

	}

	st = &_sentence[_sentenceNum++];

	st->verb = verb;
	st->objectA = objectA;
	st->objectB = objectB;
	st->preposition = (objectB != 0);
	st->freezeCount = 0;
}

void ScummEngine::checkAndRunSentenceScript() {
	int i;
	int localParamList[24];
	const ScriptSlot *ss;
	int sentenceScript;
	if (_version <= 2)
		sentenceScript = 2;
	else
		sentenceScript = VAR(VAR_SENTENCE_SCRIPT);

	memset(localParamList, 0, sizeof(localParamList));
	if (isScriptInUse(sentenceScript)) {
		ss = vm.slot;
		for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++)
			if (ss->number == sentenceScript && ss->status != ssDead && ss->freezeCount == 0)
				return;
	}

	if (!_sentenceNum || _sentence[_sentenceNum - 1].freezeCount)
		return;

	_sentenceNum--;

	if (_version < 7)
		if (_sentence[_sentenceNum].preposition && _sentence[_sentenceNum].objectB == _sentence[_sentenceNum].objectA)
			return;

	if (_version <= 2) {
		_scummVars[VAR_ACTIVE_VERB] = _sentence[_sentenceNum].verb;
		_scummVars[VAR_ACTIVE_OBJECT1] = _sentence[_sentenceNum].objectA;
		_scummVars[VAR_ACTIVE_OBJECT2] = _sentence[_sentenceNum].objectB;
		_scummVars[VAR_VERB_ALLOWED] = (0 != getVerbEntrypoint(_sentence[_sentenceNum].objectA, _sentence[_sentenceNum].verb));
	} else {
		localParamList[0] = _sentence[_sentenceNum].verb;
		localParamList[1] = _sentence[_sentenceNum].objectA;
		localParamList[2] = _sentence[_sentenceNum].objectB;
	}
	_currentScript = 0xFF;
	if (sentenceScript)
		runScript(sentenceScript, 0, 0, localParamList);
}

void ScummEngine::runInputScript(int a, int cmd, int mode) {
	int args[24];
	int verbScript;

	if (_gameId == GID_MANIAC && _platform == Common::kPlatformC64) {
		verbScript = 3;
		//_scummVars[9] = cmd;

	} else if (_version <= 2) {
		verbScript = 4;
		_scummVars[VAR_CLICK_AREA] = a;
		switch (a) {
		case 1:		// Verb clicked
			_scummVars[33] = cmd;
			break;
		case 3:		// Inventory clicked
			_scummVars[35] = cmd;
			break;
		}
	} else {
		verbScript = VAR(VAR_VERB_SCRIPT);
	}

	memset(args, 0, sizeof(args));
	args[0] = a;
	args[1] = cmd;
	args[2] = mode;
	// All HE 72+ games but only some HE 71 games.
	if (_heversion >= 71) {
		args[3] = VAR(VAR_VIRT_MOUSE_X);
		args[4] = VAR(VAR_VIRT_MOUSE_Y);
	}
	if (verbScript)
		runScript(verbScript, 0, 0, args);
}

void ScummEngine::decreaseScriptDelay(int amount) {
	ScriptSlot *ss = vm.slot;
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

bool ScummEngine::isScriptInUse(int script) const {
	int i;
	const ScriptSlot *ss = vm.slot;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++)
		if (ss->number == script)
			return true;
	return false;
}

bool ScummEngine::isScriptRunning(int script) const {
	int i;
	const ScriptSlot *ss = vm.slot;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++)
		if (ss->number == script && (ss->where == WIO_GLOBAL || ss->where == WIO_LOCAL) && ss->status != ssDead)
			return true;
	return false;
}

bool ScummEngine::isRoomScriptRunning(int script) const {
	int i;
	const ScriptSlot *ss = vm.slot;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++)
		if (ss->number == script && ss->where == WIO_ROOM && ss->status != ssDead)
			return true;
	return false;
}

void ScummEngine::copyScriptString(byte *dst) {
	int len = resStrLen(_scriptPointer) + 1;
	while (len--)
		*dst++ = fetchScriptByte();
	*dst = 0;
}

//
// Given a pointer to a Scumm string, this function returns the total byte length
// of the string data in that resource. To do so it has to understand certain
// special characters embedded into the string. The reason for this function is that
// sometimes this embedded data contains zero bytes, thus we can't just use strlen.
//
int ScummEngine::resStrLen(const byte *src) const {
	int num = 0;
	byte chr;
	if (src == NULL)
		src = _scriptPointer;
	while ((chr = *src++) != 0) {
		num++;
		if (chr == 255) {
			chr = *src++;
			num++;

			// WORKAROUND for bug #985948, a script bug in Indy3. See also
			// the corresponding code in ScummEngine::convertMessageToString().
			if (_gameId == GID_INDY3 && chr == 0x2E) {
				continue;
			}

			if (chr != 1 && chr != 2 && chr != 3 && chr != 8) {
				if (_version == 8) {
					src += 4;
					num += 4;
				} else {
					src += 2;
					num += 2;
				}
			}
		}
	}
	return num;
}

void ScummEngine::beginCutscene(int *args) {
	int scr = _currentScript;
	vm.slot[scr].cutsceneOverride++;

	if (++vm.cutSceneStackPointer > ARRAYSIZE(vm.cutSceneData))
		error("Cutscene stack overflow");

	vm.cutSceneData[vm.cutSceneStackPointer] = args[0];
	vm.cutSceneScript[vm.cutSceneStackPointer] = 0;
	vm.cutScenePtr[vm.cutSceneStackPointer] = 0;

	vm.cutSceneScriptIndex = scr;
	if (VAR(VAR_CUTSCENE_START_SCRIPT))
		runScript(VAR(VAR_CUTSCENE_START_SCRIPT), 0, 0, args);
	vm.cutSceneScriptIndex = 0xFF;
}

void ScummEngine::endCutscene() {
	ScriptSlot *ss = &vm.slot[_currentScript];
	int args[16];

	if (ss->cutsceneOverride > 0)	// Only terminate if active
		ss->cutsceneOverride--;

	memset(args, 0, sizeof(args));
	args[0] = vm.cutSceneData[vm.cutSceneStackPointer];

	VAR(VAR_OVERRIDE) = 0;

	if (vm.cutScenePtr[vm.cutSceneStackPointer] && (ss->cutsceneOverride > 0))	// Only terminate if active
		ss->cutsceneOverride--;

	vm.cutSceneScript[vm.cutSceneStackPointer] = 0;
	vm.cutScenePtr[vm.cutSceneStackPointer] = 0;
	vm.cutSceneStackPointer--;

	if (VAR(VAR_CUTSCENE_END_SCRIPT))
		runScript(VAR(VAR_CUTSCENE_END_SCRIPT), 0, 0, args);
}

void ScummEngine::abortCutscene() {
	const int idx = vm.cutSceneStackPointer;
	assert(0 <= idx && idx < 5);

	uint32 offs = vm.cutScenePtr[idx];
	if (offs) {
		ScriptSlot *ss = &vm.slot[vm.cutSceneScript[idx]];
		ss->offs = offs;
		ss->status = ssRunning;
		ss->freezeCount = 0;

		if (ss->cutsceneOverride > 0)
			ss->cutsceneOverride--;

		VAR(VAR_OVERRIDE) = 1;
		vm.cutScenePtr[idx] = 0;

		// HACK to fix issues with SMUSH and the way it does keyboard handling.
		// In particular, normally abortCutscene() is being called while no
		// scripts are active. But SMUSH runs from *inside* the script engine.
		// And it calls abortCutscene() if ESC is pressed... not good.
		// Proper fix might be to let SMUSH/INSANE run from outside the script
		// engine but that would require lots of changes and may actually have
		// negative effects, too. So we cheat here, to fix bug #751670.
		if (_version == 7)
			getScriptEntryPoint();

	}
}

void ScummEngine::beginOverride() {
	const int idx = vm.cutSceneStackPointer;
	assert(0 <= idx && idx < 5);

	vm.cutScenePtr[idx] = _scriptPointer - _scriptOrgPointer;
	vm.cutSceneScript[idx] = _currentScript;

	// Skip the jump instruction following the override instruction
	// (the jump is responsible for "skipping" cutscenes, and the reason
	// why we record the current script position in vm.cutScenePtr).
	fetchScriptByte();
	fetchScriptWord();

	// This is based on disassembly
	VAR(VAR_OVERRIDE) = 0;
}

void ScummEngine::endOverride() {
	const int idx = vm.cutSceneStackPointer;
	assert(0 <= idx && idx < 5);

	vm.cutScenePtr[idx] = 0;
	vm.cutSceneScript[idx] = 0;

	if (_version > 3)
		VAR(VAR_OVERRIDE) = 0;
}

} // End of namespace Scumm
