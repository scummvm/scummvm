/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_GAME_ENGINE_SCRIPTENGINE_H
#define ICB_GAME_ENGINE_SCRIPTENGINE_H

#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_string.h"
#include "engines/icb/common/px_game_object.h"
#include "engines/icb/common/px_globalvariables.h"

namespace ICB {

#define SCRIPT_CHECK_TEXT "SDS>"
#define SCRIPT_CHECK_TEXT_LEN 4

enum scriptInterpreterReturnCodes {
	IR_RET_END_THE_CYCLE = 0,   // done enough this cycle
	IR_RET_SCRIPT_FINISHED = 1, // current script has finished and hit closing brace
	IR_RET_CONT_THIS_CYCLE = 2  // FN_ returned an IR_TERMINATE to interpretter so we just go around - new script or gosub
};

scriptInterpreterReturnCodes RunScript(const char *&scriptData, // A pointer to the script data that can be modified
                                       c_game_object *object,   // A pointer to the object that owns this object
                                       int *engineReturnValue = NULL,
                                       const char *scriptSourceName = NULL); // A value to return to the game engine

void SetScriptDebugging(bool8 f); // Set script debugging flag

extern CpxGlobalScriptVariables g_globalScriptVariables;

#define CP_END_SCRIPT 0               // Terminate a script
#define CP_PUSH_INT32 1               // Push a number on to the stack
#define CP_PUSH_ADDRESS_LOCAL_VAR32 2 // Push the address of a local variable

//#define   CP_CALL_SCRIPT_ON_TRUE           3              // Call a script if the expression result is true
// Not sure where this is used

#define CP_SKIPONFALSE 4       // Skip if the bottom value on the stack is false
#define CP_SKIPALLWAYS 5       // Skip a block of code
#define CP_SKIPONTRUE 6        // Skip if the bottom value on the stack is true
#define CP_RETURN 7            // return the value on the stack to the game engine
#define CP_PUSH_GLOBAL_VAR32 8 // Set a variable to 1
#define CP_POP_GLOBAL_VAR32 9  // Pop a global variable
#define CP_CALL_MCODE 10       // Call a machine code function
#define CP_QUIT 11             // Quit for a cycle
#define CP_PUSH_STRING 12      // Push a pointer to a string
//#define   CP_LINE_NUMBER                  13              // Notify a change of script line number
#define CP_CALL_VSCRIPT_ON_TRUE 14 // Call a virtual script if the expression result is true
// The script name is taken from the object virtual
// script table
#define CP_SAVE_MCODE_START 15     // Save the mcode code start for restarting when necessary
#define CP_PUSH_LOCAL_VAR32 16     // Push a local variable on to the stack
#define CP_POP_LOCAL_VAR32 17      // Pop a local variable from the stack
#define CP_PUSH_LOCAL_VARSTRING 18 // Push a local variable on to the stack
#define CP_DEBUG 19                // A debug command
#define CP_INITIALISEGLOBAL 20     // Initialise a global variable

#define CP_SWITCH 21 // takes value of the stack and uses with switch table...

#define CP_PUSH_0 22 // push 0 on the stack (a slight speed advantage and a space saving)
#define CP_PUSH_1 23 // push 1 on the stack (a slight speed advantage and a space saving)

// It was decided that the following fix to the return code of fn routines
// in expressions was not to be applied, but the code has been kept just in
// case

// special push variations to save space
#define CP_PUSH_INT16 26
#define CP_PUSH_INT8 27
#define CP_PUSH_STRING_REFERENCE 28

// An invalid token
#define CP_INVALID_TOKEN 29 // For functions where a token is required, but not appropriate

// Binary Operators

#define OP_ISEQUAL 30  // '=='
#define OP_PLUS 31     // '+'
#define OP_TIMES 32    // '*'
#define OP_MINUS 33    // '-'
#define OP_DEVIDE 34   // '/'
#define OP_LSTHAN 35   // <
#define OP_NOTEQUAL 36 // '!='
#define OP_ANDAND 37   // &&
#define OP_OROR 38     // || or OR
#define OP_GTTHAN 39   // >
#define OP_GTTHANE 41  // >=
#define OP_LSTHANE 42  // <=

// Unary Operators

#define TK_UNARY_NOT 50   // !    Invert a boolean
#define TK_UNARY_MINUS 51 // -    Negate a number

// a special op which pushes the string "player" onto the stack
#define CP_PUSH_STRING_PLAYER 52

// a new version of call_mcode which sets inside expression to true (saves 1 byte per fn_call)
#define CP_CALL_MCODE_EXPR 53

//      Command tokens, only ever used internally
#define CT_IF 100               // An if statement
#define CT_CLOSEBRACKET 101     // )
#define CT_SEMICOLON 102        // ;
#define CT_ONCE 103             // the 'once' command
#define CT_CLOSEBRACE 104       // }
#define CT_DOUBLECLOSEBRACE 105 // Two tab indent reduction
#define CT_DO 106               // the 'do' command
#define CT_RETURN 107           // return a value to the engine
#define CT_SWITCH 108           // Switch
#define CT_QUIT 109             // Quit for a cycle
#define CT_COMMA 110            // ,
#define CT_OPENBRACE 111        // {
#define CT_DEBUG 112            // Debug commands
#define CT_INITIALISEGLOBAL 113 // Initialise a global variable
#define CT_WHILE 114            // the 'while' command

} // End of namespace ICB

#endif
