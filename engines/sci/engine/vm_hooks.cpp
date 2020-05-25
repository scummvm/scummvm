/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/hashmap.h"
#include "common/array.h"
#include "common/language.h"
#include "sci/engine/vm_hooks.h"
#include "sci/engine/vm.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/scriptdebug.h"

namespace Sci {

/*****************************************************************************************************************
 *
 * This mechanism allows inserting new instructions, and not only replace existing code.
 * Note that when using hooks, the regular PC is frozen, and doesn't advance.
 * Therefore, 'jmp', 'bt' and 'bnt' are used to locally move around inside the patch.
 *
 ******************************************************************************************************************/


// solves the issue described at #9646:
// "
// When in room 58, and type "run", the hero will fall and his HP will decrease by 1 point. This can be repeated, but will never cause the hero to die.
// When typing "run" the ego will be assigned with the script egoRuns.
// egoRuns::changeState calls proc0_36 in script 0 which is deducing damage from the hero's HP.
// This procedure returns TRUE if the hero is still alive, but the return value is never observed in egoRuns.
// "
// we solve that by calling the hook before executing the opcode following proc0_36 call
// and check the return value. if the hero should die, we kill him

static const byte qfg1_die_after_running_on_ice[] = {
	// if shouldn't die, jump to end
	0x2f, 22,					   // bt +22

	// should die - done according to the code at main.sc, proc0_29:
	// 			(proc0_1 0 59 80 {Death from Overwork} 82 800 1 4)
	0x39, 0x08,                    // pushi 8		-- num of parameters
	0x39, 0x00,                    // pushi 0
	0x39, 59,					   // pushi 59
	0x39, 0,                       // pushi 0		-- modified, not using {Death from Overwork}
	0x36,                          // push
	0x39, 82,					   // pushi 82
	0x38, 32,  3,				   // push 800
	0x39, 1,                       // pushi 1
	0x39, 4,                       // pushi 4
	0x47, 0x00, 0x01, 0x10         // calle proc0_1
};


// SCI0 Hebrew translations need to modify and relocate the "Enter input:" prompt
// currently SQ3 is the only Hebrew SCI0 game, but this patch (or similar) should work for the future games as well

static const byte sci0_hebrew_input_prompt[] = {
	// in (procedure (Print ...
	// ...
	// 			(#edit
	// 				(++ paramCnt)
	// adding:
	//				(hDText moveTo: 104 4)
	//				(= hDText (DText new:))
	//				(hDText
	//					text: ''
	//					moveTo: 4 4
	//					font: gDefaultFont
	//					setSize:
	//				)
	//				(hDialog add: hDText)
	0x38, 0x92, 0,			// pushi    #moveTo
	0x39, 2,				// pushi    2
	0x38, 210, 0,			// pushi    220		;x
	0x39, 4, 				// pushi    4		;y
	0x85, 1,				// lat      temp1
	0x4a, 8,				// send     8
	0x38, 0x59, 0,			// pushi    #new
	0x39, 0, 				// pushi    0
	0x51, 0xd,				// class    DText
	0x4a, 4,				// send     4
	0xa5, 1,				// sat      temp1
	0x39, 0x1a,				// pushi    #text
	0x39, 1,				// pushi    1
	0x75, 25,				// lofss    ''	; that location has '\0'
	0x38, 146, 0,			// pushi    146
	0x39, 2,				// pushi    2
	0x39, 4,				// pushi    4		;x
	0x39, 14,				// pushi    12		;y
	0x39, 33,				// pushi    33
	0x39, 1,				// pushi    1
	0x89, 0x16,				// lsg      global22
	0x38, 144, 0,			// pushi    144
	0x39, 0,				// pushi    0
	0x85, 1,				// lat      temp1
	0x4a, 24,				// send     24
	0x38, 0x64, 0,			// pushi    #add
	0x39, 1, 				// pushi    1
	0x8d, 1,				// lst      temp1
	0x85, 0,				// lat      temp0
	0x4a, 6,				// send     6
	0x85, 5					// lat      temp5
};



/** Write here all games hooks
 *  From this we'll build _hooksMap, which contains only relevant hooks to current game
 *  The match is performed according to PC, script number, opcode (only opcode name, as seen in ScummVM debugger),
 *  and either:
 *  - objName and selector	(and then externID is -1)
 *  - external function ID  (and then selector is "")
 *		= in that case, if objName == "" it will be ignored, otherwise, it will be also used to match
 */

static const GeneralHookEntry allGamesHooks[] = {
	// GID, script, lang,        PC.offset, objName,  selector,  externID, opcode,  hook array
	{GID_QFG1, Common::UNK_LANG, {58,  0x144d}, {"egoRuns", "changeState", -1 , "push0", HOOKARRAY(qfg1_die_after_running_on_ice)}},
	{GID_SQ3,  Common::HE_ISR,   {255, 0x1103}, {"User",    "",            -1 , "pushi", HOOKARRAY(sci0_hebrew_input_prompt)}}
};


VmHooks::VmHooks() {
	// build _hooksMap
	for (uint i = 0; i < ARRAYSIZE(allGamesHooks); i++) {
		if (allGamesHooks[i].gameId == g_sci->getGameId())
			if (allGamesHooks[i].language == g_sci->getLanguage() || allGamesHooks[i].language == Common::UNK_LANG)
				_hooksMap.setVal(allGamesHooks[i].key, allGamesHooks[i].entry);
	}

	_lastPc = NULL_REG;
	_just_finished = false;
	_location = 0;
}

uint64 HookHashKey::hash() {
	return ((uint64)scriptNumber << 32) + offset;
}

//#ifndef REDUCE_MEMORY_USAGE
extern const char* opcodeNames[];
//#endif

// returns true if entry is matching to current state
bool hook_exec_match(Sci::EngineState *s, HookEntry entry) {
	Script *scr = s->_segMan->getScript(s->xs->addr.pc.getSegment());
	const char *objName = s->_segMan->getObjectName(s->xs->objp);
	Common::String selector;
	if (s->xs->debugSelector != -1)
		selector = g_sci->getKernel()->getSelectorName(s->xs->debugSelector);
	byte opcode = (scr->getBuf(s->xs->addr.pc.getOffset())[0]) >> 1;

	bool objMatch;
	if (entry.exportId != -1 && strcmp(entry.objName, "") == 0)
		objMatch = true;
	else
		objMatch = strcmp(objName, entry.objName) == 0;

	return objMatch && selector == entry.selector &&
		s->xs->debugExportId == entry.exportId && strcmp(entry.opcodeName, opcodeNames[opcode]) == 0;
}

void VmHooks::vm_hook_before_exec(Sci::EngineState *s) {
	if (_just_finished) {
		_just_finished = false;
		_lastPc = NULL_REG;
		return;
	}
	Script *scr = s->_segMan->getScript(s->xs->addr.pc.getSegment());
	int scriptNumber = scr->getScriptNumber();
	HookHashKey key = { scriptNumber, s->xs->addr.pc.getOffset() };
	if (_hookScriptData.empty() && _lastPc != s->xs->addr.pc && _hooksMap.contains(key)) {
		_lastPc = s->xs->addr.pc;
		HookEntry entry = _hooksMap[key];
		if (hook_exec_match(s, entry)) {
			debugC(kDebugLevelPatcher, "vm_hook: patching script: %d, PC: %04x:%04x, obj: %s, selector: %s, extern: %d, opcode: %s", scriptNumber, PRINT_REG(s->xs->addr.pc), entry.objName, entry.selector.c_str(), entry.exportId, entry.opcodeName);
			Common::Array<byte> buffer(entry.patch, entry.patchSize);

			_hookScriptData = buffer;
		} else {
			debugC(kDebugLevelPatcher, "vm_hook: failed to match! script: %d, PC: %04x:%04x, obj: %s, selector: %s, extern: %d, opcode: %s", scriptNumber, PRINT_REG(s->xs->addr.pc), entry.objName, entry.selector.c_str(), entry.exportId, entry.opcodeName);
		}
	}
}

byte *VmHooks::data() {
	return _hookScriptData.data() + _location;
}

bool VmHooks::isActive(Sci::EngineState *s) {
	// if PC has changed, then we're temporary inactive - went to some other call, or send, etc.
	return !_hookScriptData.empty() && _lastPc == s->xs->addr.pc;
}

void VmHooks::advance(int offset) {
	int newLocation = _location + offset;
	if (newLocation < 0)
		error("VmHooks: requested to change offset before start of patch");
	else if ((uint)newLocation > _hookScriptData.size())
		error("VmHooks: requested to change offset after end of patch");
	else if ((uint)newLocation == _hookScriptData.size()) {
		_hookScriptData.clear();
		_just_finished = true;
		_location = 0;
	} else
		_location = newLocation;
}


} // End of namespace Sci
