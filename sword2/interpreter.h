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

// Interpreter return codes

#define IR_STOP		0
#define IR_CONT		1
#define IR_TERMINATE	2	
#define IR_REPEAT	3
#define IR_GOSUB	4


#ifdef INSIDE_LINC			// Are we running in linc?

extern int g_debugFlag;

#ifdef _DEBUG

#define DEBUG1(x,y)		if(g_debugFlag){engine.AddTextLine(CVString(x,y),VS_COL_DEBUG);}
#define DEBUG2(x,y,z)	if(g_debugFlag){engine.AddTextLine(CVString(x,y,z),VS_COL_DEBUG);}
#define DEBUG3(x,y,z,a)	if(g_debugFlag){engine.AddTextLine(CVString(x,y,z,a),VS_COL_DEBUG);}

#else //_DEBUG

#define DEBUG1
#define DEBUG2
#define DEBUG3

#endif //_DEBUG

#else //INSIDE_LINC

//#include "src\driver96.h"
#include "debug.h"
#include "header.h"

#define DEBUG1				if(g_debugFlag)Zdebug
#define DEBUG2				if(g_debugFlag)Zdebug
#define DEBUG3				if(g_debugFlag)Zdebug

#define ASSERT(x) {if(!(x)){Zdebug("Interpreter ASSERT %s,%d",__FILE__,__LINE__);Con_fatal_error("Assert error in interpreter");}}


#endif


	// Get parameter fix so that the playstation version can handle words not on word boundaries
#define Read16ip(var)			{var = *((int16 *)(code+ip));ip+=sizeof(int16);}
#define Read32ip(var)			{var = *((int32 *)(code+ip));ip+=sizeof(int32);}
#define Read32ipLeaveip(var)		{var = *((int32 *)(code+ip));}

void SetGlobalInterpreterVariables(int32 *vars);

#ifdef INSIDE_LINC			// Are we running in linc?
int RunScript ( MCBOVirtualSword &engine , const char * scriptData , char * /*objectData*/ , uint32 *offset );
#else
int RunScript ( const char * scriptData , char * /*objectData*/ , uint32 *offset );
#endif



//		Command tokens

#define	CT_COMMENT						1				// A program comment
#define	CT_IF							2				// An if statement
#define	CT_OPENBRACKET					3				// (
#define	CT_CLOSEBRACKET					4				// )
#define	CT_VAR							5				// Define a variable
#define	CT_SEMICOLON					6				// ;
#define	CT_COMMA						7				// ,
#define	CT_OPENBRACE					8				// {
#define	CT_CLOSEBRACE					9				// }
#define	CT_STRUCT						10				// Struct
#define	CT_SWITCH						11				// Switch
#define	CT_CASE							12				// Case
#define	CT_BREAK						13				// break
#define	CT_DEFAULT						14				// default
#define	CT_ASSIGN						14				// =
#define	CT_PLUSEQ						15				// '+='
#define	CT_MINUSEQ						16				// '-='
#define	CT_FOR							17				// for
#define	CT_DO							18				// do
#define	CT_WHILE						19				// while
#define	CT_DEBUGON						20				// Turn debugging on
#define	CT_DEBUGOFF						21				// Turn debugging off
#define	CT_QUIT							22				// Quit for a cycle
#define	CT_ENDIF						23				// Endif
#define	CT_TEXTOBJECT					24				// Speaker: text line
#define	CT_ANIM							25				// An animation
#define	CT_ELSE							26				// else to an if
#define	CT_CHOOSE						27				// Start a chooser
#define	CT_END							28				// end, usually followed by something else
#define	CT_END_CHOICE					29				// end choice
#define	CT_TERMINATE					30				// Terminate
#define	CT_PAUSE						31				// Pause
#define	CT_RESTART						32				// Restart script
#define	CT_START						33				// Start conversation
#define	CT_CALL							34				// Call a character
#define	CT_ACTORSCOMMENT				35				// A comment for an actor
#define	CT_TALKER						36				// A set talker command

//		Special functions

#define	SF_RUNLIST						1
#define	SF_DOUBLEQUOTE					2
#define	SF_BACKGROUND					3
#define	SF_SCALEA						4
#define	SF_SCALEB						5
#define	SF_SPEECHSCRIPT					6

//		Compiled tokens

#define	CP_END_SCRIPT					0
#define	CP_PUSH_LOCAL_VAR32				1				// Push a local variable on to the stack
#define	CP_PUSH_GLOBAL_VAR32			2				// Push a global variable
#define	CP_POP_LOCAL_VAR32				3				// Pop a local variable from the stack
#define	CP_CALL_MCODE					4				// Call a machine code function
#define	CP_PUSH_LOCAL_ADDR				5				// Push the address of a local variable
#define	CP_PUSH_INT32					6				// Adjust the stack after calling an fn function
#define	CP_SKIPONFALSE					7				// Skip if the bottom value on the stack is false
#define	CP_SKIPALLWAYS					8				// Skip a block of code
#define	CP_SWITCH						9				// Switch on last stack value
#define	CP_ADDNPOP_LOCAL_VAR32			10				// Add to a local varible
#define	CP_SUBNPOP_LOCAL_VAR32			11				// Subtract to a local variable
#define	CP_SKIPONTRUE					12				// Skip if the bottom value on the stack is true
#define	CP_POP_GLOBAL_VAR32				13				// Pop a global variable
#define	CP_ADDNPOP_GLOBAL_VAR32			14
#define	CP_SUBNPOP_GLOBAL_VAR32			15
#define	CP_DEBUGON						16				// Turn debugging on
#define	CP_DEBUGOFF						17				// Turn debugging off
#define	CP_QUIT							18				// Quit for a cycle
#define	CP_TERMINATE					19				// Quit script completely

//		Operators

#define	OP_ISEQUAL						20				// '=='
#define	OP_PLUS							21				// '+'
#define	OP_MINUS						22				// '-'
#define	OP_TIMES						23				// '*'
#define	OP_DEVIDE						24				// '/'
#define	OP_NOTEQUAL						25				// '=='
#define	OP_ANDAND						26				// &&
#define	OP_GTTHAN						27				// >
#define	OP_LSTHAN						28				// <

//		More tokens, mixed types

#define	CP_JUMP_ON_RETURNED				29				// Use table of jumps with value returned from fn_mcode
#define	CP_TEMP_TEXT_PROCESS			30				// A dummy text process command for me
#define	CP_SAVE_MCODE_START				31				// Save the mcode code start for restarting when necessary
#define	CP_RESTART_SCRIPT				32				// Start the script from the beginning
#define	CP_PUSH_STRING					33				// Push a pointer to a string on the stack
#define	CP_PUSH_DEREFERENCED_STRUCTURE	34				// Push the address of a structure thing

#define	OP_GTTHANE						35				// >=
#define	OP_LSTHANE						36				// <=
#define	OP_OROR							37				// || or OR
