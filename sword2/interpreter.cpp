/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "common/util.h"
#include "sword2/sword2.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/memory.h"

namespace Sword2 {

#define STACK_SIZE 10

// The machine code table

#ifndef REDUCE_MEMORY_USAGE
#	define OPCODE(x)	{ &Logic::x, #x }
#else
#	define OPCODE(x)	{ &Logic::x, "" }
#endif

typedef int32 (Logic::*OpcodeProc)(int32 *);
struct OpcodeEntry {
	OpcodeProc proc;
	const char *desc;
};

static const OpcodeEntry opcodes[] = {
	/* 00 */
	OPCODE(fnTestFunction),
	OPCODE(fnTestFlags),
	OPCODE(fnRegisterStartPoint),
	OPCODE(fnInitBackground),
	/* 04 */
	OPCODE(fnSetSession),
	OPCODE(fnBackSprite),
	OPCODE(fnSortSprite),
	OPCODE(fnForeSprite),
	/* 08 */
	OPCODE(fnRegisterMouse),
	OPCODE(fnAnim),
	OPCODE(fnRandom),
	OPCODE(fnPreLoad),
	/* 0C */
	OPCODE(fnAddSubject),
	OPCODE(fnInteract),
	OPCODE(fnChoose),
	OPCODE(fnWalk),
	/* 10 */
	OPCODE(fnWalkToAnim),
	OPCODE(fnTurn),
	OPCODE(fnStandAt),
	OPCODE(fnStand),
	/* 14 */
	OPCODE(fnStandAfterAnim),
	OPCODE(fnPause),
	OPCODE(fnMegaTableAnim),
	OPCODE(fnAddMenuObject),
	/* 18 */
	OPCODE(fnStartConversation),
	OPCODE(fnEndConversation),
	OPCODE(fnSetFrame),
	OPCODE(fnRandomPause),
	/* 1C */
	OPCODE(fnRegisterFrame),
	OPCODE(fnNoSprite),
	OPCODE(fnSendSync),
	OPCODE(fnUpdatePlayerStats),
	/* 20 */
	OPCODE(fnPassGraph),
	OPCODE(fnInitFloorMouse),
	OPCODE(fnPassMega),
	OPCODE(fnFaceXY),
	/* 24 */
	OPCODE(fnEndSession),
	OPCODE(fnNoHuman),
	OPCODE(fnAddHuman),
	OPCODE(fnWeWait),
	/* 28 */
	OPCODE(fnTheyDoWeWait),
	OPCODE(fnTheyDo),
	OPCODE(fnWalkToTalkToMega),
	OPCODE(fnFadeDown),
	/* 2C */
	OPCODE(fnISpeak),
	OPCODE(fnTotalRestart),
	OPCODE(fnSetWalkGrid),
	OPCODE(fnSpeechProcess),
	/* 30 */
	OPCODE(fnSetScaling),
	OPCODE(fnStartEvent),
	OPCODE(fnCheckEventWaiting),
	OPCODE(fnRequestSpeech),
	/* 34 */
	OPCODE(fnGosub),
	OPCODE(fnTimedWait),
	OPCODE(fnPlayFx),
	OPCODE(fnStopFx),
	/* 38 */
	OPCODE(fnPlayMusic),
	OPCODE(fnStopMusic),
	OPCODE(fnSetValue),
	OPCODE(fnNewScript),
	/* 3C */
	OPCODE(fnGetSync),
	OPCODE(fnWaitSync),
	OPCODE(fnRegisterWalkGrid),
	OPCODE(fnReverseMegaTableAnim),
	/* 40 */
	OPCODE(fnReverseAnim),
	OPCODE(fnAddToKillList),
	OPCODE(fnSetStandbyCoords),
	OPCODE(fnBackPar0Sprite),
	/* 44 */
	OPCODE(fnBackPar1Sprite),
	OPCODE(fnForePar0Sprite),
	OPCODE(fnForePar1Sprite),
	OPCODE(fnSetPlayerActionEvent),
	/* 48 */
	OPCODE(fnSetScrollCoordinate),
	OPCODE(fnStandAtAnim),
	OPCODE(fnSetScrollLeftMouse),
	OPCODE(fnSetScrollRightMouse),
	/* 4C */
	OPCODE(fnColour),
	OPCODE(fnFlash),
	OPCODE(fnPreFetch),
	OPCODE(fnGetPlayerSaveData),
	/* 50 */
	OPCODE(fnPassPlayerSaveData),
	OPCODE(fnSendEvent),
	OPCODE(fnAddWalkGrid),
	OPCODE(fnRemoveWalkGrid),
	/* 54 */
	OPCODE(fnCheckForEvent),
	OPCODE(fnPauseForEvent),
	OPCODE(fnClearEvent),
	OPCODE(fnFaceMega),
	/* 58 */
	OPCODE(fnPlaySequence),
	OPCODE(fnShadedSprite),
	OPCODE(fnUnshadedSprite),
	OPCODE(fnFadeUp),
	/* 5C */
	OPCODE(fnDisplayMsg),
	OPCODE(fnSetObjectHeld),
	OPCODE(fnAddSequenceText),
	OPCODE(fnResetGlobals),
	/* 60 */
	OPCODE(fnSetPalette),
	OPCODE(fnRegisterPointerText),
	OPCODE(fnFetchWait),
	OPCODE(fnRelease),
	/* 64 */
	OPCODE(fnPrepareMusic),
	OPCODE(fnSoundFetch),
	OPCODE(fnPrepareMusic),	// Again, apparently
	OPCODE(fnSmackerLeadIn),
	/* 68 */
	OPCODE(fnSmackerLeadOut),
	OPCODE(fnStopAllFx),
	OPCODE(fnCheckPlayerActivity),
	OPCODE(fnResetPlayerActivityDelay),
	/* 6C */
	OPCODE(fnCheckMusicPlaying),
	OPCODE(fnPlayCredits),
	OPCODE(fnSetScrollSpeedNormal),
	OPCODE(fnSetScrollSpeedSlow),
	/* 70 */
	OPCODE(fnRemoveChooser),
	OPCODE(fnSetFxVolAndPan),
	OPCODE(fnSetFxVol),
	OPCODE(fnRestoreGame),
	/* 74 */
	OPCODE(fnRefreshInventory),
	OPCODE(fnChangeShadows)
};

#define push(value) \
do { \
	assert(stackPtr < ARRAYSIZE(stack)); \
	stack[stackPtr++] = (value); \
} while (false)

#define push_ptr(ptr) push(_vm->_memory->encodePtr(ptr))

#define pop() (assert(stackPtr < ARRAYSIZE(stack)), stack[--stackPtr])

uint32 *Logic::_scriptVars = NULL;

int Logic::runScript(char *scriptData, char *objectData, uint32 *offset) {
	// Interestingly, unlike our BASS engine the stack is a local variable.
	// This has some interesting implications which may or may not be
	// necessary to the BS2 engine.

	int32 stack[STACK_SIZE];
	int32 stackPtr = 0;

	StandardHeader *header = (StandardHeader *)scriptData;
	scriptData += sizeof(StandardHeader) + sizeof(ObjectHub);

	// The script data format:
	//	int32_TYPE	1		Size of variable space in bytes
	//	...				The variable space
	//	int32_TYPE	1		numberOfScripts
	//	int32_TYPE	numberOfScripts	The offsets for each script

	// Initialise some stuff

	uint32 ip = 0;			 // Code pointer
	int scriptNumber;

	// Get the start of variables and start of code

	uint32 *localVars = (uint32 *)(scriptData + sizeof(int32));
	char *code = scriptData + READ_LE_UINT32(scriptData) + sizeof(int32);
	uint32 noScripts = READ_LE_UINT32(code);

	code += sizeof(int32);

	const uint32 *offsetTable = (const uint32 *)code;

	if (*offset < noScripts) {
		ip = FROM_LE_32(offsetTable[*offset]);
		scriptNumber = *offset;
		debug(4, "Start script %d with offset %d", *offset, ip);
	} else {
		uint i;

		ip = *offset;

		for (i = 1; i < noScripts; i++) {
			if (FROM_LE_32(offsetTable[i]) >= ip)
				break;
		}

		scriptNumber = i - 1;
		debug(4, "Resume script %d with offset %d", scriptNumber, ip);
	}

	// WORKAROUND: The dreaded pyramid bug makes the torch untakeable when
	// you speak to Titipoco. This is because one of the conditions for the
	// torch to be takeable is that Titipoco isn't doing anything out of
	// the ordinary. Global variable 913 has to be 0 to signify that he is
	// in his "idle" state.
	//
	// Unfortunately, simply the act of speaking to him sets variable 913
	// to 1 (probably to stop him from turning around every now and then).
	// The script may then go on to set the variable to different values
	// to trigger various behaviours in him, but if you have run out of
	// these cases the script won't ever set it back to 0 again.
	//
	// So if his click hander (action script number 2) finishes, and
	// variable 913 is 1, we set it back to 0 manually.

	bool checkPyramidBug = false;

	// WORKAROUND for bug #1214168: The not-at-all dreaded mop bug.
	//
	// At the London Docks, global variable 1003 keeps track of Nico:
	//
	// 0: Hiding behind the first crate.
	// 1: Hiding behind the second crate.
	// 2: Standing in plain view on the deck.
	// 3: Hiding on the roof.
	//
	// The bug happens when trying to pick up the mop while hiding on the
	// roof. Nico climbs down, the mop is picked up, but the variable
	// remains set to 3. Visually, everything looks ok. But as far as the
	// scripts are concerned, she's still hiding up on the roof. This is
	// not fatal, but leads to a number of glitches until the state is
	// corrected. E.g. trying to climb back up the ladder will cause Nico
	// to climb down again.
	//
	// Global variable 1017 keeps track of the mop. Setting it to 2 means
	// that the mop has been picked up. We should be able to use that as
	// the signal that Nico's state needs to be updated as well. There are
	// a number of other possible workarounds, but this is the closest
	// point I've found to where Nico's state should have been updated, had
	// the script been correct.

	bool checkMopBug = false;

	if (scriptNumber == 2) {
		if (strcmp((char *)header->name, "titipoco_81") == 0)
			checkPyramidBug = true;
		else if (strcmp((char *)header->name, "mop_73") == 0)
			checkMopBug = true;
	}

	code += noScripts * sizeof(int32);

	// Code should now be pointing at an identifier and a checksum
	const char *checksumBlock = code;

	code += sizeof(int32) * 3;

	if (READ_LE_UINT32(checksumBlock) != 12345678) {
		error("Invalid script in object %s", header->name);
		return 0;
	}

	int32 codeLen = READ_LE_UINT32(checksumBlock + 4);
	int32 checksum = 0;

	for (int i = 0; i < codeLen; i++)
		checksum += (unsigned char) code[i];

	if (checksum != (int32) READ_LE_UINT32(checksumBlock + 8)) {
		debug(1, "Checksum error in object %s", header->name);
		// This could be bad, but there has been a report about someone
		// who had problems running the German version because of
		// checksum errors. Could there be a version where checksums
		// weren't properly calculated?
	}

	bool runningScript = true;

	int parameterReturnedFromMcodeFunction = 0;	// Allow scripts to return things
	int savedStartOfMcode = 0;	// For saving start of mcode commands

	while (runningScript) {
		int i;
		int32 a, b;
		int curCommand, parameter, value; // Command and parameter variables
		int retVal;
		int caseCount;
		bool foundCase;
		byte *ptr;

		curCommand = code[ip++];

		switch (curCommand) {

		// Script-related opcodes

		case CP_END_SCRIPT:
			// End the script
			runningScript = false;

			// WORKAROUND: Pyramid Bug. See explanation above.

			if (checkPyramidBug && _scriptVars[913] == 1) {
				warning("Working around Titipoco script bug (the \"Pyramid Bug\")");
				_scriptVars[913] = 0;
			}

			debug(9, "CP_END_SCRIPT");
			break;
		case CP_QUIT:
			// Quit out for a cycle
			*offset = ip;
			debug(9, "CP_QUIT");
			return 0;
		case CP_TERMINATE:
			// Quit out immediately without affecting the offset
			// pointer
			debug(9, "CP_TERMINATE");
			return 3;
		case CP_RESTART_SCRIPT:
			// Start the script again
			ip = FROM_LE_32(offsetTable[scriptNumber]);
			debug(9, "CP_RESTART_SCRIPT");
			break;

		// Stack-related opcodes

		case CP_PUSH_INT32:
			// Push a long word value on to the stack
			Read32ip(parameter);
			push(parameter);
			debug(9, "CP_PUSH_INT32: %d", parameter);
			break;
		case CP_PUSH_LOCAL_VAR32:
			// Push the contents of a local variable
			Read16ip(parameter);
			parameter /= 4;
			push(localVars[parameter]);
			debug(9, "CP_PUSH_LOCAL_VAR32: localVars[%d] => %d", parameter, localVars[parameter]);
			break;
		case CP_PUSH_GLOBAL_VAR32:
			// Push a global variable
			assert(_scriptVars);
			Read16ip(parameter);
			push(_scriptVars[parameter]);
			debug(9, "CP_PUSH_GLOBAL_VAR32: scriptVars[%d] => %d", parameter, _scriptVars[parameter]);
			break;
		case CP_PUSH_LOCAL_ADDR:
			// Push the address of a local variable

			// From what I understand, some scripts store data
			// (e.g. mouse pointers) in their local variable space
			// from the very beginning, and use this mechanism to
			// pass that data to the opcode function. I don't yet
			// know the conceptual difference between this and the
			// CP_PUSH_DEREFERENCED_STRUCTURE opcode.

			Read16ip(parameter);
			parameter /= 4;
			ptr = (byte *)&localVars[parameter];
			push_ptr(ptr);
			debug(9, "CP_PUSH_LOCAL_ADDR: &localVars[%d] => %p", parameter, ptr);
			break;
		case CP_PUSH_STRING:
			// Push the address of a string on to the stack
			// Get the string size
			Read8ip(parameter);

			// ip now points to the string
			ptr = (byte *)(code + ip);
			push_ptr(ptr);
			debug(9, "CP_PUSH_STRING: \"%s\"", ptr);
			ip += (parameter + 1);
			break;
		case CP_PUSH_DEREFERENCED_STRUCTURE:
			// Push the address of a dereferenced structure
			Read32ip(parameter);
			ptr = (byte *)(objectData + sizeof(int32) + sizeof(StandardHeader) + sizeof(ObjectHub) + parameter);
			push_ptr(ptr);
			debug(9, "CP_PUSH_DEREFERENCED_STRUCTURE: %d => %p", parameter, ptr);
			break;
		case CP_POP_LOCAL_VAR32:
			// Pop a value into a local word variable
			Read16ip(parameter);
			parameter /= 4;
			value = pop();
			localVars[parameter] = value;
			debug(9, "CP_POP_LOCAL_VAR32: localVars[%d] = %d", parameter, value);
			break;
		case CP_POP_GLOBAL_VAR32:
			// Pop a global variable
			Read16ip(parameter);
			value = pop();

			// WORKAROUND: Mop bug. See explanation above.

			if (checkMopBug && parameter == 1017 && _scriptVars[1003] != 2) {
				warning("Working around Mop script bug: Setting Nico state");
				_scriptVars[1003] = 2;
			}

			_scriptVars[parameter] = value;
			debug(9, "CP_POP_GLOBAL_VAR32: scriptsVars[%d] = %d", parameter, value);
			break;
		case CP_ADDNPOP_LOCAL_VAR32:
			Read16ip(parameter);
			parameter /= 4;
			value = pop();
			localVars[parameter] += value;
			debug(9, "CP_ADDNPOP_LOCAL_VAR32: localVars[%d] += %d => %d", parameter, value, localVars[parameter]);
			break;
		case CP_SUBNPOP_LOCAL_VAR32:
			Read16ip(parameter);
			parameter /= 4;
			value = pop();
			localVars[parameter] -= value;
			debug(9, "CP_SUBNPOP_LOCAL_VAR32: localVars[%d] -= %d => %d", parameter, value, localVars[parameter]);
			break;
		case CP_ADDNPOP_GLOBAL_VAR32:
			// Add and pop a global variable
			Read16ip(parameter);
			value = pop();
			_scriptVars[parameter] += value;
			debug(9, "CP_ADDNPOP_GLOBAL_VAR32: scriptVars[%d] += %d => %d", parameter, value, _scriptVars[parameter]);
			break;
		case CP_SUBNPOP_GLOBAL_VAR32:
			// Sub and pop a global variable
			Read16ip(parameter);
			value = pop();
			_scriptVars[parameter] -= value;
			debug(9, "CP_SUBNPOP_GLOBAL_VAR32: scriptVars[%d] -= %d => %d", parameter, value, _scriptVars[parameter]);
			break;

		// Jump opcodes

		case CP_SKIPONTRUE:
			// Skip if the value on the stack is true
			Read32ipLeaveip(parameter);
			value = pop();
			if (!value) {
				ip += sizeof(int32);
				debug(9, "CP_SKIPONTRUE: %d (IS FALSE (NOT SKIPPED))", parameter);
			} else {
				ip += parameter;
				debug(9, "CP_SKIPONTRUE: %d (IS TRUE (SKIPPED))", parameter);
			}
			break;
		case CP_SKIPONFALSE:
			// Skip if the value on the stack is false
			Read32ipLeaveip(parameter);
			value = pop();
			if (value) {
				ip += sizeof(int32);
				debug(9, "CP_SKIPONFALSE: %d (IS TRUE (NOT SKIPPED))", parameter);
			} else {
				ip += parameter;
				debug(9, "CP_SKIPONFALSE: %d (IS FALSE (SKIPPED))", parameter);
			}
			break;
		case CP_SKIPALWAYS:
			// skip a block
			Read32ipLeaveip(parameter);
			ip += parameter;
			debug(9, "CP_SKIPALWAYS: %d", parameter);
			break;
		case CP_SWITCH:
			// switch
			value = pop();
			Read32ip(caseCount);

			// Search the cases
			foundCase = false;
			for (i = 0; i < caseCount && !foundCase; i++) {
				if (value == (int32) READ_LE_UINT32(code + ip)) {
					// We have found the case, so lets
					// jump to it
					foundCase = true;
					ip += READ_LE_UINT32(code + ip + sizeof(int32));
				} else
					ip += sizeof(int32) * 2;
			}

			// If we found no matching case then use the default

			if (!foundCase)
				ip += READ_LE_UINT32(code + ip);

			debug(9, "CP_SWITCH: [SORRY, NO DEBUG INFO]");
			break;
		case CP_SAVE_MCODE_START:
			// Save the start position on an mcode instruction in
			// case we need to restart it again
			savedStartOfMcode = ip - 1;
			debug(9, "CP_SAVE_MCODE_START");
			break;
		case CP_CALL_MCODE:
			// Call an mcode routine
			Read16ip(parameter);
			assert(parameter < ARRAYSIZE(opcodes));
			// amount to adjust stack by (no of parameters)
			Read8ip(value);
			debug(9, "CP_CALL_MCODE: '%s', %d", opcodes[parameter].desc, value);
			stackPtr -= value;
			assert(stackPtr >= 0);
			retVal = (this->*opcodes[parameter].proc)(&stack[stackPtr]);

			switch (retVal & 7) {
			case IR_STOP:
				// Quit out for a cycle
				*offset = ip;
				return 0;
			case IR_CONT:
				// Continue as normal
				break;
			case IR_TERMINATE:
				// Return without updating the offset
				return 2;
			case IR_REPEAT:
				// Return setting offset to start of this
				// function call
				*offset = savedStartOfMcode;
				return 0;
			case IR_GOSUB:
				// that's really neat
				*offset = ip;
				return 2;
			default:
				error("Bad return code (%d) from '%s'", retVal & 7, opcodes[parameter].desc);
			}
			parameterReturnedFromMcodeFunction = retVal >> 3;
			break;
		case CP_JUMP_ON_RETURNED:
			// Jump to a part of the script depending on
			// the return value from an mcode routine

			// Get the maximum value
			Read8ip(parameter);
			debug(9, "CP_JUMP_ON_RETURNED: %d => %d",
				parameterReturnedFromMcodeFunction,
				READ_LE_UINT32(code + ip + parameterReturnedFromMcodeFunction * 4));
			ip += READ_LE_UINT32(code + ip + parameterReturnedFromMcodeFunction * 4);
			break;

		// Operators

		case OP_ISEQUAL:
			b = pop();
			a = pop();
			push(a == b);
			debug(9, "OP_ISEQUAL: RESULT = %d", a == b);
			break;
		case OP_NOTEQUAL:
			b = pop();
			a = pop();
			push(a != b);
			debug(9, "OP_NOTEQUAL: RESULT = %d", a != b);
			break;
		case OP_GTTHAN:
			b = pop();
			a = pop();
			push(a > b);
			debug(9, "OP_GTTHAN: RESULT = %d", a > b);
			break;
		case OP_LSTHAN:
			b = pop();
			a = pop();
			push(a < b);
			debug(9, "OP_LSTHAN: RESULT = %d", a < b);
			break;
		case OP_GTTHANE:
			b = pop();
			a = pop();
			push(a >= b);
			debug(9, "OP_GTTHANE: RESULT = %d", a >= b);
			break;
		case OP_LSTHANE:
			b = pop();
			a = pop();
			push(a <= b);
			debug(9, "OP_LSTHANE: RESULT = %d", a <= b);
			break;
		case OP_PLUS:
			b = pop();
			a = pop();
			push(a + b);
			debug(9, "OP_PLUS: RESULT = %d", a + b);
			break;
		case OP_MINUS:
			b = pop();
			a = pop();
			push(a - b);
			debug(9, "OP_MINUS: RESULT = %d", a - b);
			break;
		case OP_TIMES:
			b = pop();
			a = pop();
			push(a * b);
			debug(9, "OP_TIMES: RESULT = %d", a * b);
			break;
		case OP_DIVIDE:
			b = pop();
			a = pop();
			push(a / b);
			debug(9, "OP_DIVIDE: RESULT = %d", a / b);
			break;
		case OP_ANDAND:
			b = pop();
			a = pop();
			push(a && b);
			debug(9, "OP_ANDAND: RESULT = %d", a && b);
			break;
		case OP_OROR:
			b = pop();
			a = pop();
			push(a || b);
			debug(9, "OP_OROR: RESULT = %d", a || b);
			break;

		// Debugging opcodes, I think

		case CP_DEBUGON:
			debug(9, "CP_DEBUGON");
			break;
		case CP_DEBUGOFF:
			debug(9, "CP_DEBUGOFF");
			break;
		case CP_TEMP_TEXT_PROCESS:
			Read32ip(parameter);
			debug(9, "CP_TEMP_TEXT_PROCESS: %d", parameter);
			break;
		default:
			error("Invalid script command %d", curCommand);
			return 3;
		}
	}

	return 1;
}

} // End of namespace Sword2
