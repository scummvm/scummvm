/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/common/ptr_util.h"
#include "engines/icb/p4.h"
#include "engines/icb/debug_pc.h"
#include "engines/icb/common/px_scriptengine.h"
#include "engines/icb/common/px_common.h"          // common defs for tools & engine
#include "engines/icb/common/px_globalvariables.h" // The global variable class
#include "engines/icb/fn_routines.h"
#include "engines/icb/icb.h"

namespace ICB {

const char *playerString = "player";

CpxGlobalScriptVariables *g_globalScriptVariables;

// Information for the script program stack
#define STACK_SIZE 10  // The size of the stack
int32 stack[STACK_SIZE]; // The current stack
int32 stackPointer = 0;  // Position within stack

#define _SCRIPT_ENGINE_ERROR(mess) Fatal_error("Script engine error\nObject %s\nScript %s\nMessage %s", CGameObject::GetName(object), scriptSourceName, mess)

// Check the stack pointer is within bounds
#define CheckStackPointer                                                                                                                                                          \
	if (!((stackPointer >= 0) && (stackPointer < STACK_SIZE)))                                                                                                                 \
	_SCRIPT_ENGINE_ERROR("Out of stack")

// Push a value on to the stack
#define PushOnStack(x)                                                                                                                                                             \
	{                                                                                                                                                                          \
		stack[stackPointer] = (x);                                                                                                                                         \
		stackPointer++;                                                                                                                                                    \
		CheckStackPointer;                                                                                                                                                 \
	}

// Pop a value from the stack
#define PopOffStack(x)                                                                                                                                                             \
	{                                                                                                                                                                          \
		x = stack[stackPointer - 1];                                                                                                                                       \
		stackPointer--;                                                                                                                                                    \
		CheckStackPointer;                                                                                                                                                 \
	}

// Do a stack based two operator operation
#define DoBinaryOperation(x)                                                                                                                                                       \
	{                                                                                                                                                                          \
		stack[stackPointer - 2] = (x);                                                                                                                                     \
		stackPointer--;                                                                                                                                                    \
		CheckStackPointer;                                                                                                                                                 \
	}

// Do a stack based single operator operation
#define DoUnaryOperation(x)                                                                                                                                                        \
	{ stack[stackPointer - 1] = (x); }

// Report an operation to the log file
#define TraceOperation(x, y) ScriptTrace("%d %s %d -> %d", stack[stackPointer - 2], x, stack[stackPointer - 1], stack[stackPointer - 2] y stack[stackPointer - 1]);

#define TraceUnaryOperation(x, y) ScriptTrace("%s %d -> %d", x, stack[stackPointer - 1], y stack[stackPointer - 1]);

// Macros for fetching data from the script file
#define Fetch8(var)                                                                                                                                                                \
	{                                                                                                                                                                          \
		var = *((char *)const_cast<char *>(actualScript));                                                                                                                                     \
		actualScript += sizeof(char);                                                                                                                                      \
	}

#define Fetch32(param)                                                                                                                                                             \
	{                                                                                                                                                                          \
		param = *((int32 *)const_cast<char *>(actualScript));                                                                                                                                  \
		actualScript += sizeof(int32);                                                                                                                                     \
	}
#define Read32ipLeaveip(var)                                                                                                                                                       \
	{ var = *((int32 *)const_cast<char *>(actualScript)); }
#define Fetch16(param)                                                                                                                                                             \
	{                                                                                                                                                                          \
		param = *((int16 *)const_cast<char *>(actualScript));                                                                                                                                  \
		actualScript += sizeof(int16);                                                                                                                                     \
	}
#define Read16ipLeaveip(var)                                                                                                                                                       \
	{ var = *((int16 *)const_cast<char *>(actualScript)); }

#define UpdatePC                                                                                                                                                                   \
	{ scriptData = actualScript; }

#define ScriptTrace Zdebug

void SetScriptDebugging(bool8) {}

extern mcodeFunctionReturnCodes fn_context_chosen_logic(int32 &, int32 *);
extern mcodeFunctionReturnCodes fn_start_player_interaction(int32 &, int32 *);

scriptInterpreterReturnCodes RunScript(const char *&scriptData, // A pointer to the script data that can be modified
									   CGame *object,   // A pointer to the object that owns this object
									   int32 *engineReturnValue2,
									   const char *scriptSourceName) { // A value to return to the game engine
	// Run a script
	ScriptTrace("Run script");

	// Default return value is 0
	if (engineReturnValue2)
		*engineReturnValue2 = 0;

	// Check if the script data begins with the check text, and skip it if it does
	if (strncmp(scriptData, SCRIPT_CHECK_TEXT, SCRIPT_CHECK_TEXT_LEN) == 0)
		scriptData += SCRIPT_CHECK_TEXT_LEN;

	const char *actualScript = scriptData;

	// Some variables to hold parameters and values
	int32 parameter1, parameter2, value;
	bool8 isInExpression = FALSE8;

	// Variable to prevent infinite loops
	uint32 infiniteLoopCounter = 0;

	while (*actualScript) {
		// Quick check for infinite loops
		infiniteLoopCounter++;
		if (infiniteLoopCounter > 10000) {
			// Oh dear, what do we do now.
			Fatal_error("Internal script loop in object %s", CGameObject::GetName(object));
		}

		int32 command = *(actualScript++);

		// so that call_mcode and call_mcode_expr can share code...
		isInExpression = FALSE8;

		switch (command) {
		case CP_PUSH_INT32: { // 1:   Push a 32 bit integer
			Fetch32(parameter1);
			ScriptTrace("Push number(32 bit) %d", parameter1);
			PushOnStack(parameter1);
		} break;

		case CP_PUSH_INT16: { // 1:   Push a 16 bit integer
			int16 sixteenBit;
			Fetch16(sixteenBit);
			ScriptTrace("Push number(16 bit) %d", sixteenBit);
			PushOnStack((int32)sixteenBit);
		} break;

		case CP_PUSH_INT8: { // 1:   Push an 8 bit integer
			int8 eightBit;
			Fetch8(eightBit);
			ScriptTrace("Push number(8 bit) %d", eightBit);
			PushOnStack((int32)eightBit);
		} break;


		case CP_PUSH_ADDRESS_LOCAL_VAR32: {
			Fetch8(value); // Get the local variable number
			if (!((value >= 0) && (value < (int32)CGameObject::GetNoLvars(object))))
				_SCRIPT_ENGINE_ERROR("Local variable out of range");
			ScriptTrace("Push address of local integer variable %d = %d", value, CGameObject::GetIntegerVariable(object, value));
			PushOnStack(MemoryUtil::encodePtr((uint8 *)CGameObject::GetIntegerVariablePtr(object, value)));
		} break;

		case CP_SKIPONFALSE: { // 4 :  Skip a chunk if a result if false
			Read32ipLeaveip(parameter1) PopOffStack(value);
			ScriptTrace("Skip %d if %d is false", parameter1, value);
			if (value)
				actualScript += sizeof(int32);
			else
				actualScript += parameter1;
		} break;

		case CP_SKIPALLWAYS: { // 5 :  Skip a chunk
			Read32ipLeaveip(parameter1) ScriptTrace("Skip %d", parameter1);
			actualScript += parameter1;
		} break;

		case CP_SKIPONTRUE: { // 6 :  Skip a chunk if a result if true
			Read32ipLeaveip(parameter1) PopOffStack(value);
			ScriptTrace("Skip %d if %d is true", parameter1, value);
			if (value)
				actualScript -= parameter1;
			else
				actualScript += sizeof(int32);
		} break;

		case CP_RETURN: { // po value off stack and return it to the game engine
			if (!engineReturnValue2)
				_SCRIPT_ENGINE_ERROR("No return value");
			PopOffStack(*engineReturnValue2);
			ScriptTrace("Return %d to game engine", *engineReturnValue2);
			return (IR_RET_END_THE_CYCLE);
		} break;

		case CP_PUSH_GLOBAL_VAR32: {
			Fetch32(parameter1);                                          // hash of global
			parameter2 = g_globalScriptVariables->GetVariable(parameter1); // value of global
			PushOnStack(parameter2);                                      // push on stack
			                                                              // printf("push global 0x%08x = %d",parameter1,parameter2);
		} break;

		case CP_POP_GLOBAL_VAR32: {
			Fetch32(parameter1);                                         // hash of global
			PopOffStack(parameter2);                                     // value from stack
			g_globalScriptVariables->SetVariable(parameter1, parameter2); // set value
			                                                             // printf("pop global 0x%08x = %d",parameter1,parameter2);
		} break;

		// isInExpression starts off at false as it is set every loop of the while...
		case CP_CALL_MCODE_EXPR:
			isInExpression = TRUE8; // set to true and carry on running this code...
			// falls through
		case CP_CALL_MCODE: {           // 10:  Call an mcode routine
			// Get the mcode number
			int16 fnNumber;

			Fetch16(fnNumber);

			const int16 numApiRoutines = (g_icb->getGameType() == GType_ELDORADO) ? NO_API_ROUTINES_ELDORADO : NO_API_ROUTINES_ICB;

			if (!((fnNumber >= 0) && (fnNumber < numApiRoutines)))
				_SCRIPT_ENGINE_ERROR("fnNumber out of range?");

			// Get the number of parameters
			Fetch8(value); // amount to adjust stack by (no of parameters)

			ScriptTrace("Call mcode %d (%sin expression)", fnNumber, isInExpression ? "" : "not ");

			int32 routineReturnParameter = 0; // The value returned by the mcode routine
			mcodeFunctionReturnCodes mcodeRetVal;

			if (g_icb->getGameType() == GType_ICB) {
				mcodeRetVal = McodeTableICB[fnNumber](routineReturnParameter, stack + (stackPointer - value));
			} else if (g_icb->getGameType() == GType_ELDORADO) {
				mcodeRetVal = McodeTableED[fnNumber](routineReturnParameter, stack + (stackPointer - value));
			} else {
				error("unknown game type");
			}

			ScriptTrace("api returned %d(%d)", mcodeRetVal, routineReturnParameter);

			// Correct the stack for the parameters pushed on
			stackPointer -= value;

			// If this is part of an expression then we want to
			// push the return value on to the stack
			// Otherwise we may want to pause the script here

			if (isInExpression) {
				// Push the fn_routine return value
				PushOnStack(routineReturnParameter);
				// save the mcode return value
			} else {
				// Check return value in case we want to pause the script

				switch (mcodeRetVal) {
				case IR_STOP:
					UpdatePC;
					ScriptTrace("Script returns IR_RET_END_THE_CYCLE");
					return (IR_RET_END_THE_CYCLE);

				case IR_CONT:
					// Continue the script
					break;

				case IR_TERMINATE:
					ScriptTrace("Script returns IR_RET_CONT_THIS_CYCLE");
					return (IR_RET_CONT_THIS_CYCLE);

				case IR_REPEAT:
					ScriptTrace("Script returns IR_RET_END_THE_CYCLE");
					return (IR_RET_END_THE_CYCLE);

				case IR_GOSUB:
					UpdatePC;
					ScriptTrace("Script returns IR_RET_END_THE_CYCLE");
					return (IR_RET_END_THE_CYCLE);
				}
			}
		} break;

		case CP_QUIT: {
			// Quit for a cycle
			UpdatePC;
			return (IR_RET_END_THE_CYCLE);
		} break;

		case CP_PUSH_STRING:
			Fetch8(value); // the length of the string
			ScriptTrace("Push string \"%s\"", actualScript);
			// printf("push \"%s\"\n",actualScript);
			PushOnStack(MemoryUtil::encodePtr((uint8 *)const_cast<char *>(actualScript))); // The pointer to the string
			actualScript += value;
			break;

		case CP_PUSH_STRING_REFERENCE:
			Fetch32(parameter1); // lookup (backwards)
			ScriptTrace("Push string reference \"%s\"", actualScript + parameter1 - 4);
			// printf("push reference \"%s\"\n",actualScript+parameter1-4);
			PushOnStack(MemoryUtil::encodePtr((uint8 *)const_cast<char *>(actualScript + parameter1 - 4)));
			break;

		case CP_PUSH_STRING_PLAYER:
			ScriptTrace("Push special string \"player\"");
			// printf("push special \"player\"\n");
			PushOnStack(MemoryUtil::encodePtr((uint8 *)const_cast<char *>(playerString)));
			break;

		case CP_CALL_VSCRIPT_ON_TRUE: { // 14:  Call a virtual script if a result is true
			// Get the value to check
			PopOffStack(value);

			// Get the script index
			Fetch32(parameter1);

			if (!((parameter1 >= 0) && (parameter1 < (int32)CGameObject::GetNoScripts(object))))
				_SCRIPT_ENGINE_ERROR("Virtual script call out of range");

			// Get the type
			Fetch8(parameter2);

			ScriptTrace("if (%d) call virtual script %d (%d)", value, parameter1, CGameObject::GetScriptNameFullHash(object, parameter1));

			if (value) {
				parameter1 &= 0xffff;

				int32 dummyReturnValue;
				ScriptTrace("param2 = %d", parameter2);
				int32 scriptHash = CGameObject::GetScriptNameFullHash(object, parameter1);
				if (parameter2) {
					ScriptTrace("interact");
					fn_start_player_interaction(dummyReturnValue, &scriptHash);
				} else {
					ScriptTrace("chosen logic");
					fn_context_chosen_logic(dummyReturnValue, &scriptHash);
				}

				// Update and finish
				UpdatePC;
				return (IR_RET_CONT_THIS_CYCLE);
			}
		} break;

		case CP_SAVE_MCODE_START: //  15: Update the script pc, usually before doing an mcode routine
			UpdatePC;
			ScriptTrace("Update pc");
			break;

		case CP_PUSH_LOCAL_VAR32: //  16: Push a local variable on to the stack
			Fetch8(value);    // Get the local variable number

			if (!((value >= 0) && (value < (int32)CGameObject::GetNoLvars(object))))
				_SCRIPT_ENGINE_ERROR("Unknown variable??");

			ScriptTrace("Push local integer variable %d = %d", value, CGameObject::GetIntegerVariable(object, value));
			PushOnStack(CGameObject::GetIntegerVariable(object, value));
			break;

		case CP_POP_LOCAL_VAR32: //  17              // Pop a local variable from the stack
			Fetch8(value);   // Get the local variable number

			if (!(value >= 0) && (value < (int32)CGameObject::GetNoLvars(object)))
				_SCRIPT_ENGINE_ERROR("Unknown variable??");

			ScriptTrace("Pop local variable %d", value);
			{
				int32 varValue;
				PopOffStack(varValue);
				CGameObject::SetIntegerVariable(object, value, varValue);
			}
			break;

		case CP_PUSH_LOCAL_VARSTRING: //  18: Push a local string variable on to the stack
			Fetch8(value);        // Get the local variable number

			if (!((value >= 0) && (value < (int32)CGameObject::GetNoLvars(object))))
				_SCRIPT_ENGINE_ERROR("Unknown variable (string)??");

			ScriptTrace("Push local string variable %d = \"%s\"", value, CGameObject::GetStringVariable(object, value));
			PushOnStack(MemoryUtil::encodePtr((uint8 *)const_cast<char *>(CGameObject::GetStringVariable(object, value))));
			break;

		case CP_DEBUG: {       // 19: Debug options
			Fetch8(value); // Get the debug type
		} break;

		case CP_INITIALISEGLOBAL: {
			// Initialise a global
			_SCRIPT_ENGINE_ERROR("Can't initialise a variable here anymore");
		} break;

		case CP_SWITCH: {
			int32 i, size, found;

			PopOffStack(parameter1); // value
			Fetch32(parameter2);     // 4 byte offset to table

			ScriptTrace("Start of switch statement - value: %d", parameter1);

			actualScript += parameter2 - sizeof(int32);

			Fetch8(size); // 1 byte number of cases

			i = 0;
			found = 0;
			while (!found) {
				Fetch32(value);        // case value (or offset if default case)
				if (i == (size - 1)) { // default case
					actualScript += value - sizeof(int32);
					found = 1;
				} else {
					Fetch32(parameter2);                                // case code offset
					if (value == parameter1) {                          // if switch value = this case value
						actualScript += parameter2 - sizeof(int32); // do the jump
						found = 1;
					}
				}
				i++;
			}

		} break;

		case CP_PUSH_0: {
			ScriptTrace("Push 0");
			PushOnStack(0);
		} break;

		case CP_PUSH_1: {
			ScriptTrace("Push 1");
			PushOnStack(1);
		} break;

		case OP_TIMES: { //  32              // '*'
			ScriptTrace("*");
			TraceOperation("*", *)DoBinaryOperation((stack[stackPointer - 2] * stack[stackPointer - 1]));
		} break;

		case OP_DIVIDE: { //  34              // '/'
			ScriptTrace("/");
			TraceOperation("/", / ) DoBinaryOperation((stack[stackPointer - 2] / stack[stackPointer - 1]));
		} break;

		case OP_PLUS: { //  31              // '+'
			ScriptTrace("+");
			TraceOperation("+", +) DoBinaryOperation((stack[stackPointer - 2] + stack[stackPointer - 1]));
		} break;

		case OP_MINUS: { //  33              // '-'
			ScriptTrace("-");
			TraceOperation("-", -) DoBinaryOperation((stack[stackPointer - 2] - stack[stackPointer - 1]));
		} break;

		case OP_LSTHAN: { //  35              // '<'
			ScriptTrace("<");
			TraceOperation("<", < ) DoBinaryOperation((stack[stackPointer - 2] < stack[stackPointer - 1]));
		} break;

		case OP_GTTHAN: { //  39              // '>'
			ScriptTrace(">");
			TraceOperation(">", > ) DoBinaryOperation((stack[stackPointer - 2] > stack[stackPointer - 1]));
		} break;

		case OP_LSTHANE: { //  42              // '<='
			ScriptTrace("<=");
			TraceOperation("<=", <= ) DoBinaryOperation((stack[stackPointer - 2] <= stack[stackPointer - 1]));
		} break;

		case OP_GTTHANE: { //  41              // '>='
			ScriptTrace(">=");
			TraceOperation(">=", >= ) DoBinaryOperation((stack[stackPointer - 2] >= stack[stackPointer - 1]));
		} break;

		case OP_ISEQUAL: { // 30:  '=='
			ScriptTrace("==");
			TraceOperation("==", == ) DoBinaryOperation((stack[stackPointer - 2] == stack[stackPointer - 1]));
		} break;

		case OP_NOTEQUAL: { // 36:  '!='
			ScriptTrace("!=");
			TraceOperation("!=", != ) DoBinaryOperation((stack[stackPointer - 2] != stack[stackPointer - 1]));
		} break;

		case OP_ANDAND: { //  37              // '&&'
			ScriptTrace("&&");
			TraceOperation("&&", &&) DoBinaryOperation((stack[stackPointer - 2] && stack[stackPointer - 1]));
		} break;

		case OP_OROR: { //  38              // '||'
			ScriptTrace("||");
			TraceOperation("||", || ) DoBinaryOperation((stack[stackPointer - 2] || stack[stackPointer - 1]));
		} break;

		case TK_UNARY_NOT: { //  50              //  '!'
			// _SCRIPT_ENGINE_ERROR("I aint doing that!");
			ScriptTrace("!");
			TraceUnaryOperation("!", !) DoUnaryOperation(!stack[stackPointer - 1]);
		} break;

		case TK_UNARY_MINUS: { //  51              //  '-'
			ScriptTrace("-");
			TraceUnaryOperation("-", -) DoUnaryOperation(-stack[stackPointer - 1]);
		} break;

		default: { _SCRIPT_ENGINE_ERROR(pxVString("Invalid script token %d", command)); }
		}
	}

	ScriptTrace("Script Done");
	return (IR_RET_SCRIPT_FINISHED);
}

} // End of namespace ICB
