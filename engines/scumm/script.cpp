/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/config-manager.h"
#include "common/util.h"
#include "common/system.h"

#include "scumm/actor.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/util.h"
#include "scumm/scumm_v0.h"
#include "scumm/scumm_v2.h"
#include "scumm/sound.h"
#include "scumm/verbs.h"

namespace Scumm {

/* Start executing script 'script' with the given parameters */
void ScummEngine::runScript(int script, bool freezeResistant, bool recursive, int *lvarptr, int cycle) {
	ScriptSlot *s;

	uint32 scriptOffs;
	byte scriptType;
	int slot;

	// WORKAROUND for crash (#15256) in Maniac Mansion C64 demo:
	// Attempting to, as any character, give the can of Pepsi to any other character
	// attempts to start script 43. Unfortunately script 43 does not exist in
	// the resources and crashes the game even on the original executable :-)
	if (enhancementEnabled(kEnhGameBreakingBugFixes) && _game.id == GID_MANIAC &&
		_game.version == 0 && (_game.features & GF_DEMO) && script == 43)
		return;

	// WORKAROUND for bug in v0/v1 Zak McKracken:
	// Picking up the yellow shard in the Mexican Temple while playing as Annie was not possible.
	// This was fixed in v2.
	if (enhancementEnabled(kEnhGameBreakingBugFixes) && _game.id == GID_ZAK &&
		_game.version < 2 && script == 119 && VAR(VAR_EGO) == 2) {
		addObjectToInventory(56, 14);
		putOwner(56, VAR(VAR_EGO));
	}

	if (!script)
		return;

	if (!recursive)
		stopScript(script);

	uint16 number = (_currentScript != 0xFF) ? vm.slot[_currentScript].number : 0;

	if (script < _numGlobalScripts) {
		// Call getResourceAddress to ensure the resource is loaded & its usage count reset
		/*scriptPtr =*/ getResourceAddress(rtScript, script);
		scriptOffs = _resourceHeaderSize;
		scriptType = WIO_GLOBAL;

		debugC(DEBUG_SCRIPTS, "runScript(Global-%d) from %d-%d", script,
				       number, _roomResource);
	} else {
		scriptOffs = _localScriptOffsets[script - _numGlobalScripts];
		if (scriptOffs == 0)
			error("Local script %d is not in room %d", script, _roomResource);
		scriptType = WIO_LOCAL;

		debugC(DEBUG_SCRIPTS, "runScript(%d) from %d-%d", script,
				       number, _roomResource);
	}

	if (cycle == 0)
		cycle = (_game.heversion >= 90) ? VAR(VAR_DEFAULT_SCRIPT_PRIORITY) : 1;

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

	if (!recursive && (_game.version >= 3))
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
		cycle = (_game.heversion >= 90) ? VAR(VAR_DEFAULT_SCRIPT_PRIORITY) : 1;

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
		for (i = 0; i < NUM_SCRIPT_LOCAL; i++)
			vm.localvar[slot][i] = 0;
	} else {
		for (i = 0; i < NUM_SCRIPT_LOCAL; i++)
			vm.localvar[slot][i] = vars[i];
	}
}

int ScummEngine::getVerbEntrypoint(int obj, int entry) {
	const byte *objptr, *verbptr;
	int verboffs;

	// WORKAROUND for bug #2826: Disallow pulling the rope if it's
	// already in the player's inventory.
	//
	// Doing so would cause fatal errors, such as "Object 1047 not
	// found in room 98" in (at least) the original DOS/English
	// release, if one loads the savegame in the bug ticket above,
	// and pulls the rope after moving to the first room on the
	// right. The same error happened with the original interpreter.
	//
	// Script 97-1047 was fixed in later releases, in different ways.
	// On Amiga, a getObjectOwner() check was added; the Macintosh
	// release completely disables pulling the rope, instead. We
	// choose to follow the latter, as it's simpler, and the former
	// made Guybrush silent when trying to trigger this action.
	//
	// (The Special Edition is based on the original release with
	// the buggy script, but it doesn't cause any fatal error,
	// although it does glitch, when playing in Classic Mode.)
	if (_game.id == GID_MONKEY2 && obj == 1047 && entry == 6 && whereIsObject(obj) == WIO_INVENTORY &&
		enhancementEnabled(kEnhGameBreakingBugFixes)) {
		return 0;
	}

	if (whereIsObject(obj) == WIO_NOT_FOUND)
		return 0;

	objptr = getOBCDFromObject(obj);
	assert(objptr);

	if (_game.version == 0)
		verbptr = objptr + 14;
	else if (_game.version <= 2)
		verbptr = objptr + 15;
	else if ((_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine) ||
		_game.features & GF_OLD_BUNDLE)
		verbptr = objptr + 17;
	else if (_game.features & GF_SMALL_HEADER)
		verbptr = objptr + 19;
	else
		verbptr = findResource(MKTAG('V','E','R','B'), objptr);

	assert(verbptr);

	verboffs = verbptr - objptr;

	if (!(_game.features & GF_SMALL_HEADER))
		verbptr += _resourceHeaderSize;

	if (_game.version == 8) {
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
	} else if (_game.version <= 2) {
		do {
			const int kFallbackEntry = (_game.version == 0 ? 0x0F : 0xFF);
			if (!*verbptr)
				return 0;
			if (*verbptr == entry || *verbptr == kFallbackEntry)
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

		if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine) {
			verbptr += READ_LE_UINT16(verbptr + 1) + 3;
			return verbptr - objptr;
		} else if (_game.features & GF_SMALL_HEADER)
			return READ_LE_UINT16(verbptr + 1);
		else
			return verboffs + READ_LE_UINT16(verbptr + 1);
	}
}

/* Stop script 'script' */
void ScummEngine::stopScript(int script) {
	ScriptSlot *ss;
	int i;

	if (script == 0)
		return;

	ss = vm.slot;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++) {
		if (script == ss->number && ss->status != ssDead &&
			(ss->where == WIO_GLOBAL || ss->where == WIO_LOCAL)) {
			if (ss->cutsceneOverride)
				if (_game.version >= 5)
					error("Script %d stopped with active cutscene/override", script);
			ss->number = 0;
			ss->status = ssDead;
			nukeArrays(i);
			if (_currentScript == i)
				_currentScript = 0xFF;
		}
	}

	for (i = 0; i < vm.numNestedScripts; ++i) {
		if (vm.nest[i].number == script &&
				(vm.nest[i].where == WIO_GLOBAL || vm.nest[i].where == WIO_LOCAL)) {
			nukeArrays(vm.nest[i].slot);
			vm.nest[i].number = 0;
			vm.nest[i].slot = 0xFF;
			vm.nest[i].where = 0xFF;
		}
	}
}

/* Stop an object script 'script'*/
void ScummEngine::stopObjectScript(int script) {
	ScriptSlot *ss;
	int i;

	if (script == 0)
		return;

	ss = vm.slot;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++) {
		if (script == ss->number && ss->status != ssDead &&
		    (ss->where == WIO_ROOM || ss->where == WIO_INVENTORY || ss->where == WIO_FLOBJECT)) {
			if (ss->cutsceneOverride)
				if (_game.version >= 5)
					error("Object %d stopped with active cutscene/override", script);
			ss->number = 0;
			ss->status = ssDead;
			nukeArrays(i);
			if (_currentScript == i)
				_currentScript = 0xFF;
		}
	}

	for (i = 0; i < vm.numNestedScripts; ++i) {
		if (vm.nest[i].number == script &&
				(vm.nest[i].where == WIO_ROOM || vm.nest[i].where == WIO_INVENTORY || vm.nest[i].where == WIO_FLOBJECT)) {
			nukeArrays(vm.nest[i].slot);
			vm.nest[i].number = 0;
			vm.nest[i].slot = 0xFF;
			vm.nest[i].where = 0xFF;
		}
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

	// Backyard Basketball is one of the games which requires more than 15 nested scripts
	if (vm.numNestedScripts >= (_game.heversion >= 99 ? kMaxScriptNestingHE : kMaxScriptNesting))
		error("Too many nested scripts");

	nest = &vm.nest[vm.numNestedScripts];

	if (_currentScript == 0xFF) {
		nest->number = 0;
		nest->where = 0xFF;
	} else {
		// Store information about the currently running script
		slot = &vm.slot[_currentScript];
		nest->number = slot->number;
		nest->where = slot->where;
		nest->slot = _currentScript;
	}

	vm.numNestedScripts++;

	_currentScript = script;
	getScriptBaseAddress();
	resetScriptPointer();
	executeScript();

	if (vm.numNestedScripts != 0)
		vm.numNestedScripts--;

	if (nest->number) {
		// Try to resume the script which called us, if its status has not changed
		// since it invoked us. In particular, we only resume it if it hasn't been
		// stopped in the meantime, and if it did not already move on.
		slot = &vm.slot[nest->slot];
		if (slot->number == nest->number && slot->where == nest->where &&
				slot->status != ssDead && slot->freezeCount == 0) {
			_currentScript = nest->slot;
			getScriptBaseAddress();
			resetScriptPointer();
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

	if (_game.heversion == 0 || scriptSlot == 0)
		return;

	for (i = 1; i < _numArray; i++) {
		if (_arraySlot[i] == scriptSlot) {
			_res->nukeResource(rtString, i);
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
		_lastCodePtr = &_res->_types[rtInventory][idx]._address;
		break;

	case WIO_LOCAL:
	case WIO_ROOM:								/* room script */
		if (_game.version == 8) {
			_scriptOrgPointer = getResourceAddress(rtRoomScripts, _roomResource);
			assert(_roomResource < (int)_res->_types[rtRoomScripts].size());
			_lastCodePtr = &_res->_types[rtRoomScripts][_roomResource]._address;
		} else {
			_scriptOrgPointer = getResourceAddress(rtRoom, _roomResource);
			assert(_roomResource < _numRooms);
			_lastCodePtr = &_res->_types[rtRoom][_roomResource]._address;
		}
		break;

	case WIO_GLOBAL:							/* global script */
		_scriptOrgPointer = getResourceAddress(rtScript, ss->number);
		assert(ss->number < _numScripts);
		_lastCodePtr = &_res->_types[rtScript][ss->number]._address;
		break;

	case WIO_FLOBJECT:						/* flobject script */
		idx = getObjectIndex(ss->number);
		assert(idx != -1);
		idx = _objs[idx].fl_object_index;
		_scriptOrgPointer = getResourceAddress(rtFlObject, idx);
		assert(idx < _numFlObject);
		_lastCodePtr = &_res->_types[rtFlObject][idx]._address;
		break;
	default:
		error("Bad type while getting base address");
	}

	// The following fixes bug #2028. Confirmed against disasm.
	if (_game.version <= 2 && _scriptOrgPointer == nullptr) {
		ss->status = ssDead;
		_currentScript = 0xFF;
	}
}

void ScummEngine::resetScriptPointer() {
	if (_currentScript == 0xFF)
		return;
	_scriptPointer = _scriptOrgPointer + vm.slot[_currentScript].offs;
}

/**
 * This method checks whether the resource that contains the active script
 * moved, and if so, updates the script pointer accordingly.
 *
 * The script resource may have moved because it might have been garbage
 * collected by ResourceManager::expireResources.
 */
void ScummEngine::refreshScriptPointer() {
	if (*_lastCodePtr != _scriptOrgPointer) {
		long oldoffs = _scriptPointer - _scriptOrgPointer;
		getScriptBaseAddress();
		_scriptPointer = _scriptOrgPointer + oldoffs;
	}
}

/** Execute a script - Read opcode, and execute it from the table */
void ScummEngine::executeScript() {
	int c;
	while (_currentScript != 0xFF) {

		if (_showStack == 1) {
			debugN("Stack:");
			for (c = 0; c < _scummStackPos; c++) {
				debugN(" %d", _vmStack[c]);
			}
			debugN("\n");
		}
		_opcode = fetchScriptByte();
		if (_game.version > 2) // V0-V2 games didn't use the didexec flag
			vm.slot[_currentScript].didexec = true;
		debugC(DEBUG_OPCODES, "Script %d, offset 0x%x: [%X] %s()",
				vm.slot[_currentScript].number,
				(uint)(_scriptPointer - _scriptOrgPointer),
				_opcode,
				getOpcodeDesc(_opcode));
		if (_hexdumpScripts == true) {
			for (c = -1; c < 15; c++) {
				debugN(" %02x", *(_scriptPointer + c));
			}
			debugN("\n");
		}

		executeOpcode(_opcode);

	}
}

void ScummEngine::executeOpcode(byte i) {
	if (_opcodes[i].proc && _opcodes[i].proc->isValid())
		(*_opcodes[i].proc)();
	else {
		error("Invalid opcode '%x' at %lx", i, (long)(_scriptPointer - _scriptOrgPointer));
	}
}

const char *ScummEngine::getOpcodeDesc(byte i) {
#ifndef REDUCE_MEMORY_USAGE
	return _opcodes[i].desc;
#else
	return "";
#endif
}

byte ScummEngine::fetchScriptByte() {
	refreshScriptPointer();
	return *_scriptPointer++;
}

uint ScummEngine::fetchScriptWord() {
	refreshScriptPointer();
	uint a = READ_LE_UINT16(_scriptPointer);
	_scriptPointer += 2;
	return a;
}

int ScummEngine::fetchScriptWordSigned() {
	return (int16)fetchScriptWord();
}

uint ScummEngine::fetchScriptDWord() {
	refreshScriptPointer();
	uint a = READ_LE_UINT32(_scriptPointer);
	_scriptPointer += 4;
	return a;
}

int ScummEngine::fetchScriptDWordSigned() {
	return (int32)fetchScriptDWord();
}

int ScummEngine::readVar(uint var) {
	int a;

	debugC(DEBUG_VARS, "readvar(%d)", var);

	if ((var & 0x2000) && (_game.version <= 5)) {
		a = fetchScriptWord();
		if (a & 0x2000)
			var += readVar(a & ~0x2000);
		else
			var += a & 0xFFF;
		var &= ~0x2000;
	}

	if (!(var & 0xF000)) {
		if (!_copyProtection) {
			if (var == 490 && _game.id == GID_MONKEY2) {
				var = 518;
			}
		}

		if (VAR_SUBTITLES != 0xFF && var == VAR_SUBTITLES) {
			return ConfMan.getBool("subtitles");
		}
		if (VAR_NOSUBTITLES != 0xFF && var == VAR_NOSUBTITLES) {
			return !ConfMan.getBool("subtitles");
		}

#if defined(USE_ENET) && defined(USE_LIBCURL)
		if (_enableHECompetitiveOnlineMods) {
			// HACK: If we're reading var586, competitive mods enabled, playing online,
			// successfully fetched custom teams, and we're not in one of the three scripts
			// that cause bugs if 263 is returned here, return 263.
			if (_game.id == GID_BASEBALL2001 && var == 586 && readVar(399) == 1 && readVar(747) == 1 &&
				!(_currentRoom == 4 && (vm.slot[_currentScript].number == 2150 || vm.slot[_currentScript].number == 2208 || vm.slot[_currentScript].number == 2210))) {
				return 263;
			}
			// Mod for Backyard Baseball 2001 online competitive play: allow random bounces
			// Normally they only happen offline; this script checks var399, here we tell this
			// script that we're not in online play even if we are
			if (_game.id == GID_BASEBALL2001 && vm.slot[_currentScript].number == 39 && var == 399) {
				return 0;
			}
		}
#endif
		assertRange(0, var, _numVariables - 1, "variable (reading)");
		return _scummVars[var];
	}

	if (var & 0x8000) {
		if (_game.heversion >= 80) {
			var &= 0xFFF;
			assertRange(0, var, _numRoomVariables - 1, "room variable (reading)");

#if defined(USE_ENET) && defined(USE_LIBCURL)
			if (_enableHECompetitiveOnlineMods) {
				// Mod for Backyard Baseball 2001 online competitive play: don't give powerups for double plays
				// Return true for this variable, which dictates whether powerups are disabled, but only in this script
				// that detects double plays (among other things)
				if (_game.id == GID_BASEBALL2001 && _currentRoom == 3 && vm.slot[_currentScript].number == 2099 && var == 32 && readVar(399) == 1) {
					return 1;
				}
			}
#endif

			return _roomVars[var];

		} else if (_game.version <= 3 && !(_game.id == GID_INDY3 && _game.platform == Common::kPlatformFMTowns) &&
			!(_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine)) {
			int bit = var & 0xF;
			var = (var >> 4) & 0xFF;

			if (!_copyProtection) {
				if (_game.id == GID_LOOM && (_game.platform == Common::kPlatformFMTowns) && var == 214 && bit == 15) {
					return 0;
				} else if (_game.id == GID_ZAK && (_game.platform == Common::kPlatformFMTowns) && var == 151 && bit == 8) {
					return 0;
				}
			}

			assertRange(0, var, _numVariables - 1, "variable (reading)");
			return (_scummVars[ var ] & ( 1 << bit ) ) ? 1 : 0;
		} else {
			var &= 0x7FFF;
			if (!_copyProtection) {
				if (_game.id == GID_INDY3 && (_game.platform == Common::kPlatformFMTowns) && var == 1508)
					return 0;
			}

			assertRange(0, var, _numBitVariables - 1, "variable (reading)");
			return (_bitVars[var >> 3] & (1 << (var & 7))) ? 1 : 0;
		}
	}

	if (var & 0x4000) {
		if (_game.features & GF_FEW_LOCALS) {
			var &= 0xF;
		} else {
			var &= 0xFFF;
		}

		if (_game.heversion >= 80)
			assertRange(0, var, 25, "local variable (reading)");
		else
			assertRange(0, var, 20, "local variable (reading)");
#if defined(USE_ENET) && defined(USE_LIBCURL)
		// Mod for Backyard Baseball 2001 online competitive play: change impact of
		// batter's power stat on hit power
		if (_enableHECompetitiveOnlineMods) {
			if (_game.id == GID_BASEBALL2001 &&
				_currentRoom == 4 && vm.slot[_currentScript].number == 2090  // The script that calculates hit power
				&& readVar(399) == 1  // Check that we're playing online
				&& var == 2  // Local var for batter's hitting power stat
			) {
				int swingType = vm.localvar[_currentScript][0];
				int powerStat, powerStatModified;
				switch (swingType) {
				case 2:  // Line drive or grounder swing
					powerStat = vm.localvar[_currentScript][var];
					powerStatModified = 20 + powerStat * 4 / 5;
					return powerStatModified;
				case 1:  // Power swing
					powerStat = vm.localvar[_currentScript][var];
					powerStatModified = 10 + powerStat * 17 / 20;;
					return powerStatModified;
				default:
					break;
				}
			}
		}
#endif
		return vm.localvar[_currentScript][var];
	}

	error("Illegal varbits (r)");
	return -1;
}

void ScummEngine::writeVar(uint var, int value) {
	debugC(DEBUG_VARS, "writeVar(%d, %d)", var, value);

	if (!(var & 0xF000)) {
		assertRange(0, var, _numVariables - 1, "variable (writing)");

		if (VAR_SUBTITLES != 0xFF && var == VAR_SUBTITLES) {
			// Ignore default setting in HE72-74 games
			if (_game.heversion <= 74 && vm.slot[_currentScript].number == 1)
				return;
			assert(value == 0 || value == 1);
			ConfMan.setBool("subtitles", (value != 0));
		}
		if (VAR_NOSUBTITLES != 0xFF && var == VAR_NOSUBTITLES) {
			// Ignore default setting in HE60-71 games
			if (_game.heversion >= 60 && vm.slot[_currentScript].number == 1)
				return;
			assert(value == 0 || value == 1);
			ConfMan.setBool("subtitles", !value);
		}

		if (var == VAR_CHARINC) {
			// Use the value specified by the game script, everywhere except
			// at game boot-up: if there was a user override, then use that.
			//
			// Note: To determine whether there was a user override, we only
			// look at the target specific settings, assuming that any global
			// value is likely to be bogus. See also bug #4008.
			if (_currentRoom == 0 && ConfMan.hasKey("talkspeed", _targetName)) {
				value = 9 - getTalkSpeed();
			} else {
				// Save the new talkspeed value to ConfMan
				setTalkSpeed(9 - value);
			}
		}

		// WORKAROUND bug #13378: For whatever reason, the German and
		// Italian talkie versions (I can't check the floppy versions)
		// set the game to run much too fast in some parts of the intro.
		// Some differences are natural because of the different lengths
		// of the spoken lines, but 1 or 2 is too fast.
		//
		// Any modifications here depend on knowing if the script will
		// set the timer value back to something sensible afterwards.

		if (_game.id == GID_SAMNMAX && vm.slot[_currentScript].number == 65 && var == VAR_TIMER_NEXT && enhancementEnabled(kEnhTimingChanges)) {
			// "Wirst Du brutzeln, wie eine grobe Bratwurst!"
			if (value == 1 && _language == Common::DE_DEU)
				value = 4;

			// Max beats up the scientist. This was probably to
			// match the subtitles to the speech better, but this
			// is just too much! The floppy version doesn't do this
			// but there's no need to explicitly test this since
			// the script never sets the value to 2 there.
			if (value == 2 && _language == Common::IT_ITA)
				value = 3;
		}

		// WORKAROUND: When the Loom messenger nymph flies to wake up
		// Bobbin, the whole game is sped up. Slow down the fire
		// animation so that it appears to run at constant speed
		// throughout the intro. This does not apply to the VGA talkie
		// version, because there the fire isn't animated.

		else if (_game.id == GID_LOOM && !(_game.features & GF_DEMO) && _game.version < 4 && vm.slot[_currentScript].number == 44 && var == VAR_TIMER_NEXT && enhancementEnabled(kEnhTimingChanges)) {
			Actor *a = derefActorSafe(4, "writeVar");
			if (a) {
				a->setAnimSpeed((value == 0) ? 6 : 0);
			}
		}

		_scummVars[var] = value;

		if ((_varwatch == (int)var || _varwatch == 0) && _currentScript < NUM_SCRIPT_SLOT) {
			if (vm.slot[_currentScript].number < 100)
				debug(1, "vars[%d] = %d (via script-%d)", var, value, vm.slot[_currentScript].number);
			else
				debug(1, "vars[%d] = %d (via room-%d-%d)", var, value, _currentRoom,
							vm.slot[_currentScript].number);
		}
		return;
	}

	if (var & 0x8000) {
		if (_game.heversion >= 80) {
			var &= 0xFFF;
			assertRange(0, var, _numRoomVariables - 1, "room variable (writing)");
			_roomVars[var] = value;

		} else if (_game.version <= 3 && !(_game.id == GID_INDY3 && _game.platform == Common::kPlatformFMTowns) &&
			!(_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine)) {
			// In the old games, the bit variables were using the same memory
			// as the normal variables!
			int bit = var & 0xF;
			var = (var >> 4) & 0xFF;
			assertRange(0, var, _numVariables - 1, "variable (writing)");
			if (value)
				_scummVars[var] |= ( 1 << bit );
			else
				_scummVars[var] &= ~( 1 << bit );
		} else {
			var &= 0x7FFF;
			assertRange(0, var, _numBitVariables - 1, "bit variable (writing)");

			if (value)
				_bitVars[var >> 3] |= (1 << (var & 7));
			else
				_bitVars[var >> 3] &= ~(1 << (var & 7));
		}
		return;
	}

	if (var & 0x4000) {
		if (_game.features & GF_FEW_LOCALS) {
			var &= 0xF;
		} else {
			var &= 0xFFF;
		}

		if (_game.heversion >= 80)
			assertRange(0, var, 25, "local variable (writing)");
		else
			assertRange(0, var, 20, "local variable (writing)");

		vm.localvar[_currentScript][var] = value;
		return;
	}

	error("Illegal varbits (w)");
}

void ScummEngine::push(int a) {
	assert(_scummStackPos >= 0 && _scummStackPos < ARRAYSIZE(_vmStack));
	_vmStack[_scummStackPos++] = a;
}

int ScummEngine::pop() {
	assert(_scummStackPos >= 1 && _scummStackPos <= ARRAYSIZE(_vmStack));
	return _vmStack[--_scummStackPos];
}

void ScummEngine::stopObjectCode() {
	ScriptSlot *ss;

	assert(_currentScript != 0xFF);
	ss = &vm.slot[_currentScript];
	if (_game.version <= 2) {
		if (ss->where == WIO_GLOBAL || ss->where == WIO_LOCAL) {
			stopScript(ss->number);
		} else {
			ss->number = 0;
			ss->status = ssDead;
		}
	} else if (_game.version <= 5) {
		if (ss->where != WIO_GLOBAL && ss->where != WIO_LOCAL) {
			stopObjectScript(ss->number);
		} else {
			if (_game.version >= 4 && ss->cutsceneOverride)
				error("Script %d ending with active cutscene/override (%d)", ss->number, ss->cutsceneOverride);

			ss->number = 0;
			ss->status = ssDead;
		}
	} else {
		if (ss->where != WIO_GLOBAL && ss->where != WIO_LOCAL) {
			if (ss->cutsceneOverride)
				error("Object %d ending with active cutscene/override (%d)", ss->number, ss->cutsceneOverride);
		} else {
			if (ss->cutsceneOverride)
				error("Script %d ending with active cutscene/override (%d)", ss->number, ss->cutsceneOverride);
		}
		ss->number = 0;
		ss->status = ssDead;
	}

	nukeArrays(_currentScript);
	_currentScript = 0xFF;
}

void ScummEngine::runInventoryScript(int i) {
	if (VAR(VAR_INVENTORY_SCRIPT)) {
		int args[NUM_SCRIPT_LOCAL];
		memset(args, 0, sizeof(args));
		args[0] = i;
		runScript(VAR(VAR_INVENTORY_SCRIPT), 0, 0, args);
	}
}

void ScummEngine::runInventoryScriptEx(int i) {
	runInventoryScript(i);
}

void ScummEngine::freezeScripts(int flag) {
	int i;

	if (_game.version <= 2) {
		for (i = 0; i < NUM_SCRIPT_SLOT; i++) {
			if (_currentScript != i && vm.slot[i].status != ssDead && !vm.slot[i].freezeResistant) {
				vm.slot[i].status |= 0x80;
			}
		}
		return;
	}

	bool flagCondition = _game.version >= 7 ? flag == 2 : flag >= 0x80;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++) {
		if (_currentScript != i && vm.slot[i].status != ssDead && (!vm.slot[i].freezeResistant || flagCondition)) {
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

	if (_game.version <= 2) {
		for (i = 0; i < NUM_SCRIPT_SLOT; i++) {
			vm.slot[i].status &= 0x7F;
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
		vm.slot[i].didexec = false;

	_currentScript = 0xFF;
	int numCycles = (_game.heversion >= 90) ? VAR(VAR_LAST_SCRIPT_PRIORITY) : 1;

	for (int cycle = 1; cycle <= numCycles; cycle++) {
		for (i = 0; i < NUM_SCRIPT_SLOT; i++) {
			if (vm.slot[i].cycle == cycle && vm.slot[i].status == ssRunning && !vm.slot[i].didexec) {
				_currentScript = (byte)i;
				getScriptBaseAddress();
				resetScriptPointer();
				executeScript();
			}
		}
	}
}

void ScummEngine::runExitScript() {
	if (VAR_EXIT_SCRIPT != 0xFF && VAR(VAR_EXIT_SCRIPT))
		runScript(VAR(VAR_EXIT_SCRIPT), 0, 0, nullptr);
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
		if (_game.id == GID_INDY3 && !(_game.features & GF_OLD_BUNDLE)) {
			byte *roomptr = getResourceAddress(rtRoom, _roomResource);
			const byte *excd = findResourceData(MKTAG('E','X','C','D'), roomptr) - _resourceHeaderSize;
			if (!excd || (getResourceDataSize(excd) < 1)) {
				debug(2, "Exit-%d is empty", _roomResource);
				return;
			}
		}

		initializeLocals(slot, nullptr);
		runScriptNested(slot);
	}
	if (VAR_EXIT_SCRIPT2 != 0xFF && VAR(VAR_EXIT_SCRIPT2))
		runScript(VAR(VAR_EXIT_SCRIPT2), 0, 0, nullptr);

	// WORKAROUND: Once the water has been diverted to the grate, but
	// before Maggie has been freed, the spider lair (room 44) will play
	// the sound of trickling water (sound 215). It doesn't seem to trigger
	// the first time you enter the room, only but if you leave and
	// re-enter it. Which is probably why it's so rarely noticed.
	//
	// The sound is not stopped when you leave the room, so it will keep
	// playing even where it makes no sense. This also happens with the
	// original interpreter.
	//
	// The same sound effect is also used in the underwater cavern (room
	// 33), so we do the same fade out as in that room's exit script.
	if (_game.id == GID_DIG && _currentRoom == 44 && enhancementEnabled(kEnhAudioChanges)) {
		int scriptCmds[] = { 14, 215, 0x600, 0, 30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		_sound->soundKludge(scriptCmds, ARRAYSIZE(scriptCmds));
	}
}

void ScummEngine::runEntryScript() {
	if (VAR_ENTRY_SCRIPT != 0xFF && VAR(VAR_ENTRY_SCRIPT))
		runScript(VAR(VAR_ENTRY_SCRIPT), 0, 0, nullptr);
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
		initializeLocals(slot, nullptr);
		runScriptNested(slot);
	}
	if (VAR_ENTRY_SCRIPT2 != 0xFF && VAR(VAR_ENTRY_SCRIPT2))
		runScript(VAR(VAR_ENTRY_SCRIPT2), 0, 0, nullptr);
}

void ScummEngine::runQuitScript() {
	if (VAR_QUIT_SCRIPT != 0xFF && VAR(VAR_QUIT_SCRIPT)) {
		int args[NUM_SCRIPT_LOCAL];

		memset(args, 0, sizeof(args));
		args[0] = 2;
		args[1] = 1003;

		runScript(VAR(VAR_QUIT_SCRIPT), 0, 0, args);
	}
}

void ScummEngine::killScriptsAndResources() {
	ScriptSlot *ss;
	int i;

	ss = vm.slot;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++) {
		if (ss->where == WIO_ROOM || ss->where == WIO_FLOBJECT) {
			if (ss->cutsceneOverride) {
				if (_game.version >= 5)
					warning("Object %d stopped with active cutscene/override in exit", ss->number);
				ss->cutsceneOverride = 0;
			}
			nukeArrays(i);
			ss->status = ssDead;
		} else if (ss->where == WIO_LOCAL) {
			if (ss->cutsceneOverride) {
				if (_game.version >= 5)
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
				const int owner = getOwner((_game.version != 0 ? obj : OBJECT_V0_ID(obj)));
				// We can delete custom name resources if either the object is
				// no longer in use (i.e. not owned by anyone anymore); or if
				// it is an object which is owned by a room.
				if (owner == 0 || (_game.version < 7 && owner == OF_OWNER_ROOM)) {
					// WORKAROUND for a problem mentioned in bug report #1607:
					// In FOA in the sentry room, in the chest plate of the statue,
					// the pegs may be renamed to mouth: this custom name is lost
					// when leaving the room; this hack prevents this).
					//
					// TODO: investigate this bug report from 2004, and see if the
					// issue appears with an original interpreter, and with our current
					// (much more accurate) SCUMMv5 implementation.
					if (owner == OF_OWNER_ROOM && _game.id == GID_INDY4 && 336 <= obj && obj <= 340)
						continue;

					_newNames[i] = 0;
					_res->nukeResource(rtObjectName, i);
				}
			}
		}
	}
}

void ScummEngine::killAllScriptsExceptCurrent() {
	for (int i = 0; i < NUM_SCRIPT_SLOT; i++) {
		if (i != _currentScript) {
			vm.slot[i].status = ssDead;
			vm.slot[i].cutsceneOverride = 0;
		}
	}
}

void ScummEngine::doSentence(int verb, int objectA, int objectB) {
	SentenceTab *st;

	if (_game.version >= 7) {

		if (objectA == objectB)
			return;

		if (_sentenceNum > 0) {
			st = &_sentence[_sentenceNum - 1];

			// Check if this doSentence request is identical to the previous one;
			// if yes, ignore this invocation.
			if (_sentenceNum && st->verb == verb && st->objectA == objectA && st->objectB == objectB)
				return;
		}

	}

	assert(_sentenceNum < NUM_SENTENCE);
	st = &_sentence[_sentenceNum++];

	st->verb = verb;
	st->objectA = objectA;
	st->objectB = objectB;
	st->preposition = (objectB != 0);
	st->freezeCount = 0;
}

void ScummEngine::checkAndRunSentenceScript() {
	int i;
	int localParamList[NUM_SCRIPT_LOCAL];
	const ScriptSlot *ss;
	int sentenceScript;

	if (_game.version <= 2)
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
	SentenceTab &st = _sentence[_sentenceNum];

	if (_game.version < 7)
		if (st.preposition && st.objectB == st.objectA)
			return;

	if (_game.version <= 2) {
		VAR(VAR_ACTIVE_VERB) = st.verb;
		VAR(VAR_ACTIVE_OBJECT1) = st.objectA;
		VAR(VAR_ACTIVE_OBJECT2) = st.objectB;
		VAR(VAR_VERB_ALLOWED) = (0 != getVerbEntrypoint(st.objectA, st.verb));
	} else {
		localParamList[0] = st.verb;
		localParamList[1] = st.objectA;
		localParamList[2] = st.objectB;


		if (_game.id == GID_FT && !isValidActor(localParamList[1]) && !isValidActor(localParamList[2])) {
			// WORKAROUND for bug #2466. The buggy script clearly
			// assumes that one of the two objects is an actor. If that's
			// not the case, fall back on the default sentence script.

			// FIXME: We do not yet have all necessary information, but the
			// following is known:
			//
			// * The US PC version uses scripts 28 and 103 and has 456 scripts.
			// * The French PC version uses scripts 29 and 104 and has 467 scripts.
			// * The German Mac version uses scripts 29 and 104 and has 469 scripts.
			// * The German, Italian, Portuguese and Spanish PC versions
			//   use script 29. The other script is not currently known.
			// * The US Mac demo uses script 28.
			//
			// For now we assume that if there are more than 460 scripts, then
			// the pair 29/104 is used, else the pair 28/103.

			if (_res->_types[rtScript].size() > 460) {
				if (sentenceScript == 104)
					sentenceScript = 29;
			} else {
				if (sentenceScript == 103)
					sentenceScript = 28;
			}
		}
	}
	_currentScript = 0xFF;
	if (sentenceScript)
		runScript(sentenceScript, 0, 0, localParamList);
}

void ScummEngine_v0::walkToActorOrObject(int object) {
	int x, y, dir;
	Actor_v0 *a = (Actor_v0 *)derefActor(VAR(VAR_EGO), "walkToObject");

	_walkToObject = object;
	_walkToObjectState = kWalkToObjectStateWalk;

	if (OBJECT_V0_TYPE(object) == kObjectV0TypeActor) {
		walkActorToActor(VAR(VAR_EGO), OBJECT_V0_ID(object), 4);
		x = a->getRealPos().x;
		y = a->getRealPos().y;
	} else {
		walkActorToObject(VAR(VAR_EGO), object);
		getObjectXYPos(object, x, y, dir);
	}

	VAR(6) = x;
	VAR(7) = y;

	// actor must not move if frozen
	if (a->_miscflags & kActorMiscFlagFreeze) {
		a->stopActorMoving();
		a->_newWalkBoxEntered = false;
	}
}

bool ScummEngine_v0::checkPendingWalkAction() {
	// before a sentence script is executed, it might be necessary to walk to
	// and pickup objects before. Check if such an action is pending and handle
	// it if available.
	if (_walkToObjectState == kWalkToObjectStateDone)
		return false;

	int actor = VAR(VAR_EGO);
	Actor_v0 *a = (Actor_v0 *)derefActor(actor, "checkPendingWalkAction");

	// wait until walking or turning action is finished
	if (a->_moving != 2)
		return true;

	// after walking and turning finally execute the script
	if (_walkToObjectState == kWalkToObjectStateTurn) {
		runSentenceScript();
	// change actor facing
	} else {
		int x, y, distX, distY;
		if (objIsActor(_walkToObject)) {
			Actor *b = derefActor(objToActor(_walkToObject), "checkPendingWalkAction(2)");
			x = b->getRealPos().x;
			y = b->getRealPos().y;
			if (x < a->getRealPos().x)
				x += 4;
			else
				x -= 4;
		} else {
			getObjectXYPos(_walkToObject, x, y);
		}
		AdjustBoxResult abr = a->adjustXYToBeInBox(x, y);
		distX = ABS(a->getRealPos().x - abr.x);
		distY = ABS(a->getRealPos().y - abr.y);

		if (distX <= 4 && distY <= 8) {
			if (objIsActor(_walkToObject)) { // walk to actor finished
				// make actors turn to each other
				a->faceToObject(_walkToObject);
				int otherActor = objToActor(_walkToObject);
				// ignore the plant
				if (otherActor != 19) {
					Actor *b = derefActor(otherActor, "checkPendingWalkAction(3)");
					b->faceToObject(actorToObj(actor));
				}
			} else { // walk to object finished
				int tmpX, tmpY, dir;
				getObjectXYPos(_walkToObject, tmpX, tmpY, dir);
				a->turnToDirection(dir);
			}
			_walkToObjectState = kWalkToObjectStateTurn;
			return true;
		}
	}

	_walkToObjectState = kWalkToObjectStateDone;
	return false;
}

void ScummEngine_v0::checkAndRunSentenceScript() {
	if (checkPendingWalkAction())
		return;

	if (!_sentenceNum || _sentence[_sentenceNum - 1].freezeCount)
		return;

	SentenceTab &st = _sentence[_sentenceNum - 1];

	if (st.preposition && st.objectB == st.objectA) {
		_sentenceNum--;
		return;
	}

	_currentScript = 0xFF;

	assert(st.objectA);

	// If two objects are involved, at least one must be in the actors inventory
	if (st.objectB &&
		(OBJECT_V0_TYPE(st.objectA) != kObjectV0TypeFG || _objectOwnerTable[st.objectA] != VAR(VAR_EGO)) &&
		(OBJECT_V0_TYPE(st.objectB) != kObjectV0TypeFG || _objectOwnerTable[st.objectB] != VAR(VAR_EGO)))
	{
		if (getVerbEntrypoint(st.objectA, kVerbPickUp))
			doSentence(kVerbPickUp, st.objectA, 0);
		else if (getVerbEntrypoint(st.objectB, kVerbPickUp))
			doSentence(kVerbPickUp, st.objectB, 0);
		else
			_sentenceNum--;
		return;
	}

	_cmdVerb = st.verb;
	_cmdObject = st.objectA;
	_cmdObject2 = st.objectB;
	_sentenceNum--;

	// abort sentence execution if the number of nested scripts is too high.
	// This might happen for instance if the sentence command depends on an
	// object that the actor has to pick-up in a nested doSentence() call.
	// If the actor is not able to pick-up the object (e.g. because it is not
	// reachable or pickupable) a nested pick-up command is triggered again
	// and again, so the actual sentence command will never be executed.
	// In this case the sentence command has to be aborted.
	_sentenceNestedCount++;
	if (_sentenceNestedCount > 6) {
		_sentenceNestedCount = 0;
		_sentenceNum = 0;
		return;
	}

	if (whereIsObject(st.objectA) != WIO_INVENTORY) {
		if (_currentMode != kModeKeypad) {
			walkToActorOrObject(st.objectA);
			return;
		}
	} else if (st.objectB && whereIsObject(st.objectB) != WIO_INVENTORY) {
		walkToActorOrObject(st.objectB);
		return;
	}

	runSentenceScript();
	if (_currentMode == kModeKeypad) {
		_walkToObjectState = kWalkToObjectStateDone;
	}
}

void ScummEngine_v0::runSentenceScript() {
	_redrawSentenceLine = true;

	if (getVerbEntrypoint(_cmdObject, _cmdVerb) != 0) {
		// do not read in the dark
		if (!(_cmdVerb == kVerbRead && _currentLights == 0)) {
			VAR(VAR_ACTIVE_OBJECT2) = OBJECT_V0_ID(_cmdObject2);
			runObjectScript(_cmdObject, _cmdVerb, false, false, nullptr);
			return;
		}
	} else {
		if (_cmdVerb == kVerbGive) {
			// no "give to"-script: give to other kid or ignore
			int actor = OBJECT_V0_ID(_cmdObject2);
			if (actor < 8)
				setOwnerOf(_cmdObject, actor);
			return;
		}
	}

	if (_cmdVerb != kVerbWalkTo) {
		// perform verb's fallback action
		VAR(VAR_ACTIVE_VERB) = _cmdVerb;
		runScript(3, 0, 0, nullptr);
	}
}

void ScummEngine_v2::runInputScript(int clickArea, int val, int mode) {
	int args[NUM_SCRIPT_LOCAL];
	int verbScript;

	verbScript = 4;
	VAR(VAR_CLICK_AREA) = clickArea;
	switch (clickArea) {
	case kVerbClickArea:		// Verb clicked
		VAR(VAR_CLICK_VERB) = val;
		break;
	case kInventoryClickArea:		// Inventory clicked
		VAR(VAR_CLICK_OBJECT) = val;
		break;
	default:
		break;
	}

	memset(args, 0, sizeof(args));
	args[0] = clickArea;
	args[1] = val;
	args[2] = mode;

	if (verbScript)
		runScript(verbScript, 0, 0, args);
}

void ScummEngine::runInputScript(int clickArea, int val, int mode) {
	int args[NUM_SCRIPT_LOCAL];
	int verbScript;

	verbScript = VAR(VAR_VERB_SCRIPT);

	memset(args, 0, sizeof(args));
	args[0] = clickArea;
	args[1] = val;
	args[2] = mode;
	// All HE 72+ games but only some HE 71 games.
	if (_game.heversion >= 71) {
		args[3] = VAR(VAR_VIRT_MOUSE_X);
		args[4] = VAR(VAR_VIRT_MOUSE_Y);
	}

	// Macintosh version of indy3ega used different interface, so adjust values.
	if (_game.id == GID_INDY3 && _game.platform == Common::kPlatformMacintosh) {
		// Clicks are handled differently in Indy3 mac: param 2 of the
		// input script is set to 0 for normal clicks, and to 1 for double clicks.
		// The EGA DOS version of Loom also checks that the second click happens
		// close enough to the first one, but that seems like overkill.
		uint32 time = _system->getMillis();
		args[2] = (time < _lastInputScriptTime + 500);	// 500 ms double click delay
		_lastInputScriptTime = time;
	} else if (_game.id == GID_LOOM && _game.platform == Common::kPlatformMacintosh) {
		uint32 time = _system->getMillis();
		VAR(52) = (time < _lastInputScriptTime + 500);	// 500 ms double click delay
		_lastInputScriptTime = time;
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
				if (_game.id == GID_INDY3 && _game.platform == Common::kPlatformMacintosh && ss->number == 134) {
					// Unlike the DOS version, there doesn't
					// appear to be anything in the credits
					// script to clear the credits between
					// the text screens. I don't know how
					// the original did it, but the only
					// reliable way I can think of is to
					// trigger on the end of each delay
					// throughout the script.
					//
					// Since this is at the very end of the
					// game, it should be safe enough.
					mac_undrawIndy3CreditsText();
				}
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
	memcpy(dst, _scriptPointer, len);
	_scriptPointer += len;
	dst += len;
	*dst = 0;
}

/**
 * Given a pointer to a Scumm string, this function returns the total
 * byte length of the string data in that resource. To do so it has to
 * understand certain special characters embedded into the string. The
 * reason for this function is that sometimes this embedded data
 * contains zero bytes, thus we can't just use strlen.
 */
int ScummEngine::resStrLen(const byte *src) {
	int num = 0;
	byte chr;
	if (src == nullptr) {
		refreshScriptPointer();
		src = _scriptPointer;
	}
	while ((chr = *src++) != 0) {
		num++;
		if (_game.version == 8 && chr == 0xFF) {
			src += 5;
			num += 5;
		} else if (_game.heversion <= 71 && chr == 0xFF) {
			chr = *src++;
			num++;

			// WORKAROUND for bugs #1675 and #2715, script bugs in German Indy3.
			// For more information, See the the corresponding workaround in
			// ScummEngine::convertMessageToString().
			//
			// While the twin workaround in convertMessageToString() can be optional,
			// our code diverges from the original just enough that we can't ignore the
			// invalid control code at least in INDY3 VGA DE, so the following has to
			// run regardless of the enhancement settings. Therefore, kEnhGameBreakingBugFixes
			// is the appropriate class here.
			if (enhancementEnabled(kEnhGameBreakingBugFixes) && _game.id == GID_INDY3 && _language == Common::DE_DEU &&
			    ((_roomResource == 23 && chr == 0x2E) ||
			     (_roomResource == 21 && chr == 0x20))) {
				num--;
				src--;
				continue;
			}

			if (chr != 1 && chr != 2 && chr != 3 && chr != 8) {
				src += 2;
				num += 2;
			}
		}
	}
	return num;
}

void ScummEngine::beginCutscene(int *args) {
	int scr = _currentScript;
	vm.slot[scr].cutsceneOverride++;

	++vm.cutSceneStackPointer;
	if (vm.cutSceneStackPointer >= kMaxCutsceneNum)
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
	int args[NUM_SCRIPT_LOCAL];

	if (ss->cutsceneOverride > 0)	// Only terminate if active
		ss->cutsceneOverride--;

	memset(args, 0, sizeof(args));
	args[0] = vm.cutSceneData[vm.cutSceneStackPointer];

	VAR(VAR_OVERRIDE) = 0;

	if (vm.cutScenePtr[vm.cutSceneStackPointer] && (ss->cutsceneOverride > 0))	// Only terminate if active
		ss->cutsceneOverride--;

	vm.cutSceneScript[vm.cutSceneStackPointer] = 0;
	vm.cutScenePtr[vm.cutSceneStackPointer] = 0;

	if (0 == vm.cutSceneStackPointer) {
		// WORKAROUND bug #5624: Due to poor translation of the v2 script to
		// v5 an if statement jumps in the middle of a cutscene causing a
		// endCutscene() without a begin cutscene()
		//
		// TODO: see what the original interpreter did when encountering this
		// script bug (with UNZ, DREAMM or TOWNSEMU), and decide which
		// Enhancement setting should be used in this case.
		if (_game.id == GID_ZAK && _game.platform == Common::kPlatformFMTowns &&
			vm.slot[_currentScript].number == 205 && _currentRoom == 185) {
			return;
		}
		error("Cutscene stack underflow");
	}
	vm.cutSceneStackPointer--;

	if (VAR(VAR_CUTSCENE_END_SCRIPT))
		runScript(VAR(VAR_CUTSCENE_END_SCRIPT), 0, 0, args);
}

void ScummEngine::abortCutscene() {
	const int idx = vm.cutSceneStackPointer;
	assert(0 <= idx && idx < kMaxCutsceneNum);

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
	}
}

void ScummEngine::beginOverride() {
	const int idx = vm.cutSceneStackPointer;
	assert(0 <= idx && idx < kMaxCutsceneNum);

	vm.cutScenePtr[idx] = _scriptPointer - _scriptOrgPointer;
	vm.cutSceneScript[idx] = _currentScript;

	// Skip the jump instruction following the override instruction
	// (the jump is responsible for "skipping" cutscenes, and the reason
	// why we record the current script position in vm.cutScenePtr).
	fetchScriptByte();
	fetchScriptWord();

	if (_game.version >= 5)
		VAR(VAR_OVERRIDE) = 0;
}

void ScummEngine::endOverride() {
	const int idx = vm.cutSceneStackPointer;
	assert(0 <= idx && idx < kMaxCutsceneNum);

	vm.cutScenePtr[idx] = 0;
	vm.cutSceneScript[idx] = 0;

	if (_game.version >= 4)
		VAR(VAR_OVERRIDE) = 0;
}

} // End of namespace Scumm
