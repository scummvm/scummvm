/* Copyright (C) 1994-2003 Revolution Software Ltd
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

#ifndef _INTERPRETER
#define _INTERPRETER

#include "debug.h"
#include "header.h"

// Interpreter return codes

#define IR_STOP				0
#define IR_CONT				1
#define IR_TERMINATE			2	
#define IR_REPEAT			3
#define IR_GOSUB			4

#define DEBUG				if (g_debugFlag) Zdebug

#define ASSERT(x)		 	{ if (!(x)) { Zdebug("Interpreter ASSERT %s,%d", __FILE__, __LINE__); Con_fatal_error("Assert error in interpreter"); } }

// Get parameter fix so that the playstation version can handle words not on
// word boundaries

#define Read8ip(var)			{ var = *((const int8 *) (code + ip)); ip++; }
#define Read16ip(var)			{ var = (int16) READ_LE_UINT16(code + ip); ip += sizeof(int16); }
#define Read32ip(var)			{ var = (int32) READ_LE_UINT32(code + ip); ip += sizeof(int32); }
#define Read32ipLeaveip(var)		{ var = (int32) READ_LE_UINT32(code + ip); }

void SetGlobalInterpreterVariables(int32 *vars);
int RunScript (char *scriptData, char *objectData, uint32 *offset);

// Compiled tokens

#define	CP_END_SCRIPT			0
#define	CP_PUSH_LOCAL_VAR32		1	// Push a local variable on to the stack
#define	CP_PUSH_GLOBAL_VAR32		2	// Push a global variable
#define	CP_POP_LOCAL_VAR32		3	// Pop a local variable from the stack
#define	CP_CALL_MCODE			4	// Call a machine code function
#define	CP_PUSH_LOCAL_ADDR		5	// Push the address of a local variable
#define	CP_PUSH_INT32			6	// Adjust the stack after calling an fn function
#define	CP_SKIPONFALSE			7	// Skip if the bottom value on the stack is false
#define	CP_SKIPALWAYS			8	// Skip a block of code
#define	CP_SWITCH			9	// Switch on last stack value
#define	CP_ADDNPOP_LOCAL_VAR32		10	// Add to a local varible
#define	CP_SUBNPOP_LOCAL_VAR32		11	// Subtract to a local variable
#define	CP_SKIPONTRUE			12	// Skip if the bottom value on the stack is true
#define	CP_POP_GLOBAL_VAR32		13	// Pop a global variable
#define	CP_ADDNPOP_GLOBAL_VAR32		14
#define	CP_SUBNPOP_GLOBAL_VAR32		15
#define	CP_DEBUGON			16	// Turn debugging on
#define	CP_DEBUGOFF			17	// Turn debugging off
#define	CP_QUIT				18	// Quit for a cycle
#define	CP_TERMINATE			19	// Quit script completely

// Operators

#define	OP_ISEQUAL			20	// '=='
#define	OP_PLUS				21	// '+'
#define	OP_MINUS			22	// '-'
#define	OP_TIMES			23	// '*'
#define	OP_DIVIDE			24	// '/'
#define	OP_NOTEQUAL			25	// '=='
#define	OP_ANDAND			26	// '&&'
#define	OP_GTTHAN			27	// '>'
#define	OP_LSTHAN			28	// '<'

// More tokens, mixed types

#define	CP_JUMP_ON_RETURNED		29	// Use table of jumps with value returned from fn_mcode
#define	CP_TEMP_TEXT_PROCESS		30	// A dummy text process command for me
#define	CP_SAVE_MCODE_START		31	// Save the mcode code start for restarting when necessary
#define	CP_RESTART_SCRIPT		32	// Start the script from the beginning
#define	CP_PUSH_STRING			33	// Push a pointer to a string on the stack
#define	CP_PUSH_DEREFERENCED_STRUCTURE	34	// Push the address of a structure thing

#define	OP_GTTHANE			35	// >=
#define	OP_LSTHANE			36	// <=
#define	OP_OROR				37	// || or OR

#endif
