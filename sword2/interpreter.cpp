/* Copyright (C) 1994-2004 Revolution Software Ltd
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

namespace Sword2 {

// The machine code table

#define OPCODE(x)	{ &Logic::x, #x }

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

#define pop() (assert(stackPtr < ARRAYSIZE(stack)), stack[--stackPtr])

void Logic::setGlobalInterpreterVariables(int32 *vars) {
	_globals = vars;
}

int Logic::runScript(char *scriptData, char *objectData, uint32 *offset) {
	// Interestingly, unlike our BASS engine the stack is a local variable.
	// This has some interesting implications which may or may not be
	// necessary to the BS2 engine.

	int32 stack[STACK_SIZE];
	int32 stackPtr = 0;

	StandardHeader *header = (StandardHeader *) scriptData;
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

	uint32 *variables = (uint32 *) (scriptData + sizeof(int32));
	const char *code = scriptData + (int32) READ_LE_UINT32(scriptData) + sizeof(int32);
	uint32 noScripts = (int32) READ_LE_UINT32(code);

	code += sizeof(int32);

	const uint32 *offsetTable = (const uint32 *) code;

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

	// WORKAROUND: The dreaded pyramid makes the torch untakeable when you
	// speak to Titipoco. This is because one of the conditions for the
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

	bool checkPyramidBug = scriptNumber == 2 && strcmp((char *) header->name, "titipoco_81") == 0;

	code += noScripts * sizeof(int32);

	// Code should nop be pointing at an identifier and a checksum
	const int *checksumBlock = (const int *) code;

	code += sizeof(int32) * 3;

	if (READ_LE_UINT32(checksumBlock) != 12345678) {
		error("Invalid script in object %s", header->name);
		return 0;
	}

	int codeLen = READ_LE_UINT32(checksumBlock + 1);
	int checksum = 0;

	for (int i = 0; i < codeLen; i++)
		checksum += (unsigned char) code[i];

	if (checksum != (int32) READ_LE_UINT32(checksumBlock + 2)) {
		error("Checksum error in object %s", header->name);
		return 0;
	}

	bool runningScript = true;

	int parameterReturnedFromMcodeFunction = 0;	// Allow scripts to return things
	int savedStartOfMcode = 0;	// For saving start of mcode commands

	while (runningScript) {
		int32 a, b;
		int curCommand, parameter, value; // Command and parameter variables
		int retVal;
		int caseCount;
		bool foundCase;
		int32 ptrval;
		int i;

		curCommand = code[ip++];

		switch (curCommand) {

		// Script-related opcodes

		case CP_END_SCRIPT:
			// End the script
			debug(5, "End script");
			runningScript = false;

			// WORKAROUND: Pyramid Bug. See explanation above.

			if (checkPyramidBug && _globals[913] == 1) {
				warning("Working around Titipoco script bug (the \"Pyramid Bug\")");
				_globals[913] = 0;
			}

			break;
		case CP_QUIT:
			// Quit out for a cycle
			debug(5, "Quit script for a cycle");
			*offset = ip;
			return 0;
		case CP_TERMINATE:
			// Quit out immediately without affecting the offset
			// pointer
			debug(5, "Terminate script");
			return 3;
		case CP_RESTART_SCRIPT:
			// Start the script again
			debug(5, "Restart script");
			ip = FROM_LE_32(offsetTable[scriptNumber]);
			break;

		// Stack-related opcodes

		case CP_PUSH_INT32:
			// Push a long word value on to the stack
			Read32ip(parameter);
			debug(5, "Push int32 %d", parameter);
			push(parameter);
			break;
		case CP_PUSH_LOCAL_VAR32:
			// Push the contents of a local variable
			Read16ip(parameter);
			parameter /= 4;
			debug(5, "Push local var %d (%d)", parameter, variables[parameter]);
			push(variables[parameter]);
			break;
		case CP_PUSH_GLOBAL_VAR32:
			// Push a global variable
			Read16ip(parameter);
			assert(_globals);
			debug(5, "Push global var %d (%d)", parameter, _globals[parameter]);
			push(_globals[parameter]);
			break;
		case CP_PUSH_LOCAL_ADDR:
			// push the address of a local variable
			Read16ip(parameter);
			parameter /= 4;
			ptrval = _vm->_memory->ptrToInt((const uint8 *) &variables[parameter]);
			debug(5, "Push address of local variable %d (%x)", parameter, ptrval);
			push(ptrval);
			break;
		case CP_PUSH_STRING:
			// Push the address of a string on to the stack
			// Get the string size
			Read8ip(parameter);

			// ip points to the string
			ptrval = _vm->_memory->ptrToInt((const uint8 *) (code + ip));
			debug(5, "Push address of string (%x)\n", ptrval);
			push(ptrval);
			ip += (parameter + 1);
			break;
		case CP_PUSH_DEREFERENCED_STRUCTURE:
			// Push the address of a dereferenced structure
			Read32ip(parameter);
			ptrval = _vm->_memory->ptrToInt((const uint8 *) (objectData + sizeof(int32) + sizeof(StandardHeader) + sizeof(ObjectHub) + parameter));
			debug(5, "Push address of far variable (%x)", ptrval);
			push(ptrval);
			break;
		case CP_POP_LOCAL_VAR32:
			// Pop a value into a local word variable
			Read16ip(parameter);
			parameter /= 4;
			value = pop();
			debug(5, "Pop %d into local var %d", value, parameter);
			variables[parameter] = value;
			break;
		case CP_POP_GLOBAL_VAR32:
			// Pop a global variable
			Read16ip(parameter);
			value = pop();
			debug(5, "Pop %d into global var %d", value, parameter);
			_globals[parameter] = value;
			break;
		case CP_ADDNPOP_LOCAL_VAR32:
			Read16ip(parameter);
			parameter /= 4;
			value = pop();
			variables[parameter] += value;
			debug(5, "+= %d into local var %d -> %d", value, parameter, variables[parameter]);
			break;
		case CP_SUBNPOP_LOCAL_VAR32:
			Read16ip(parameter);
			parameter /= 4;
			value = pop();
			variables[parameter] -= value;
			debug(5, "-= %d into local var %d -> %d", value, parameter, variables[parameter]);
			break;
		case CP_ADDNPOP_GLOBAL_VAR32:
			// Add and pop a global variable
			Read16ip(parameter);
			value = pop();
			_globals[parameter] += value;
			debug(5, "+= %d into global var %d -> %d", value, parameter, _globals[parameter]);
			break;
		case CP_SUBNPOP_GLOBAL_VAR32:
			// Sub and pop a global variable
			Read16ip(parameter);
			value = pop();
			_globals[parameter] -= value;
			debug(5, "-= %d into global var %d -> %d", value, parameter, _globals[parameter]);
			break;

		// Jump opcodes

		case CP_SKIPONTRUE:
			// Skip if the value on the stack is true
			Read32ipLeaveip(parameter);
			value = pop();
			debug(5, "Skip %d if %d is false", parameter, value);
			if (!value)
				ip += sizeof(int32);
			else
				ip += parameter;
			break;
		case CP_SKIPONFALSE:
			// Skip if the value on the stack is false
			Read32ipLeaveip(parameter);
			value = pop();
			debug(5, "Skip %d if %d is false", parameter, value);
			if (value)
				ip += sizeof(int32);
			else
				ip += parameter;
			break;
		case CP_SKIPALWAYS:
			// skip a block
			Read32ipLeaveip(parameter);
			debug(5, "Skip %d", parameter);
			ip += parameter;
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

			break;
		case CP_SAVE_MCODE_START:
			// Save the start position on an mcode instruction in
			// case we need to restart it again
			savedStartOfMcode = ip - 1;
			break;
		case CP_CALL_MCODE:
			// Call an mcode routine
			Read16ip(parameter);
			assert(parameter < ARRAYSIZE(opcodes));
			// amount to adjust stack by (no of parameters)
			Read8ip(value);
			debug(5, "Calling '%s' with %d parameters", opcodes[parameter].desc, value);
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
				error("Bad return code (%d) from '%s'", opcodes[parameter].desc, retVal & 7);
			}
			parameterReturnedFromMcodeFunction = retVal >> 3;
			break;
		case CP_JUMP_ON_RETURNED:
			// Jump to a part of the script depending on
			// the return value from an mcode routine

			// Get the maximum value
			Read8ip(parameter);
			ip += READ_LE_UINT32(code + ip + parameterReturnedFromMcodeFunction * 4);
			break;

		// Operators

		case OP_ISEQUAL:
			b = pop();
			a = pop();
			push(a == b);
			debug(5, "operation %d == %d", a, b);
			break;
		case OP_NOTEQUAL:
			b = pop();
			a = pop();
			push(a != b);
			debug(5, "operation %d != %d", a, b);
			break;
		case OP_GTTHAN:
			b = pop();
			a = pop();
			push(a > b);
			debug(5, "operation %d > %d", a, b);
			break;
		case OP_LSTHAN:
			b = pop();
			a = pop();
			push(a < b);
			debug(5, "operation %d < %d", a, b);
			break;
		case OP_GTTHANE:
			b = pop();
			a = pop();
			push(a >= b);
			debug(5, "operation %d >= %d", a, b);
			break;
		case OP_LSTHANE:
			b = pop();
			a = pop();
			push(a <= b);
			debug(5, "operation %d <= %d", a, b);
			break;
		case OP_PLUS:
			b = pop();
			a = pop();
			push(a + b);
			debug(5, "operation %d + %d", a, b);
			break;
		case OP_MINUS:
			b = pop();
			a = pop();
			push(a - b);
			debug(5, "operation %d - %d", a, b);
			break;
		case OP_TIMES:
			b = pop();
			a = pop();
			push(a * b);
			debug(5, "operation %d * %d", a, b);
			break;
		case OP_DIVIDE:
			b = pop();
			a = pop();
			push(a / b);
			debug(5, "operation %d / %d", a, b);
			break;
		case OP_ANDAND:
			b = pop();
			a = pop();
			push(a && b);
			debug(5, "operation %d && %d", a, b);
			break;
		case OP_OROR:
			b = pop();
			a = pop();
			push(a || b);
			debug(5, "operation %d || %d", a, b);
			break;

		// Debugging opcodes, I think

		case CP_DEBUGON:
			// Turn debugging on
			_debugFlag = true;
			break;
		case CP_DEBUGOFF:
			// Turn debugging on
			_debugFlag = false;
			break;
		case CP_TEMP_TEXT_PROCESS:
			// Process a text line
			Read32ip(parameter);
			debug(5, "Process text id %d", parameter);
			break;
		default:
			error("Invalid script command %d", curCommand);
			return 3;
		}
	}

	return 1;
}

} // End of namespace Sword2
