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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/**
 * @brief GPL2 bytecode disassembler
 * @param gplcode A pointer to the bytecode
 *        len Length of the bytecode
 *
 * GPL2 is short for Game Programming Language 2 which is the script language
 * used by Draci Historie. This is a simple disassembler for the language.
 *
 * A compiled GPL2 program consists of a stream of bytes representing commands
 * and their parameters. The syntax is as follows:
 *
 * Syntax of a command:
 *	<name of the command> <number> <sub-number> <list of parameters...>
 *
 *	Syntax of a parameter:
 *	- 1: integer number literally passed to the program
 *	- 2-1: string stored in the reservouir of game strings (i.e. something to be
 *	  displayed) and stored as an index in this list
 *	- 2-2: string resolved by the compiler (i.e., a path to another file) and
 *	  replaced by an integer index of this entity in the appropriate namespace
 *	  (e.g., the index of the palette, location, ...)
 *	- 3-0: relative jump to a label defined in this code.  Each label must be
 *	  first declared in the beginning of the program.
 *	- 3-1 .. 3-9: index of an entity in several namespaces, defined in file ident
 *	- 4: mathematical expression compiled into a postfix format
 *
 * 	In the compiled program, parameters of type 1..3 are represented by a single
 *	16-bit integer.  The called command knows by its definition what namespace the
 *	value comes from.
 */

#include "common/debug.h"
#include "common/stream.h"
#include "common/stack.h"

#include "draci/gpldisasm.h"
#include "draci/draci.h"

namespace Draci {

// FIXME: Change parameter types to names once I figure out what they are exactly
GPL2Command gplCommands[] = {
	{ 0,  0, "gplend",				0, { 0 } },
	{ 0,  1, "exit",				0, { 0 } },
	{ 1,  1, "goto", 				1, { 3 } },
	{ 2,  1, "Let", 				2, { 3, 4 } },
	{ 3,  1, "if", 					2, { 4, 3 } },
	{ 4,  1, "Start", 				2, { 3, 2 } },
	{ 5,  1, "Load", 				2, { 3, 2 } },
	{ 5,  2, "StartPlay", 			2, { 3, 2 } },
	{ 5,  3, "JustTalk", 			0, { 0 } },
	{ 5,  4, "JustStay", 			0, { 0 } },
	{ 6,  1, "Talk", 				2, { 3, 2 } },
	{ 7,  1, "ObjStat", 			2, { 3, 3 } },
	{ 7,  2, "ObjStat_On", 			2, { 3, 3 } },
	{ 8,  1, "IcoStat", 			2, { 3, 3 } },
	{ 9,  1, "Dialogue", 			1, { 2 } },
	{ 9,  2, "ExitDialogue", 		0, { 0 } },
	{ 9,  3, "ResetDialogue", 		0, { 0 } },
	{ 9,  4, "ResetDialogueFrom", 	0, { 0 } },
	{ 9,  5, "ResetBlock", 			1, { 3 } },
	{ 10, 1, "WalkOn", 				3, { 1, 1, 3 } },
	{ 10, 2, "StayOn", 				3, { 1, 1, 3 } },
	{ 10, 3, "WalkOnPlay", 			3, { 1, 1, 3 } },
	{ 11, 1, "LoadPalette", 		1, { 2 } },
	{ 12, 1, "SetPalette", 			0, { 0 } },
	{ 12, 2, "BlackPalette", 		0, { 0 } },
	{ 13, 1, "FadePalette", 		3, { 1, 1, 1 } },
	{ 13, 2, "FadePalettePlay", 	3, { 1, 1, 1 } },
	{ 14, 1, "NewRoom", 			2, { 3, 1 } },
	{ 15, 1, "ExecInit", 			1, { 3 } },
	{ 15, 2, "ExecLook", 			1, { 3 } },
	{ 15, 3, "ExecUse", 			1, { 3 } },
	{ 16, 1, "RepaintInventory", 	0, { 0 } },
	{ 16, 2, "ExitInventory", 		0, { 0 } },
	{ 17, 1, "ExitMap", 			0, { 0 } },
	{ 18, 1, "LoadMusic", 			1, { 2 } },
	{ 18, 2, "StartMusic", 			0, { 0 } },
	{ 18, 3, "StopMusic", 			0, { 0 } },
	{ 18, 4, "FadeOutMusic",		1, { 1 } },
	{ 18, 5, "FadeInMusic", 		1, { 1 } },
	{ 19, 1, "Mark", 				0, { 0 } },
	{ 19, 2, "Release", 			0, { 0 } },
	{ 20, 1, "Play", 				0, { 0 } },
	{ 21, 1, "LoadMap", 			1, { 2 } },
	{ 21, 2, "RoomMap", 			0, { 0 } },
	{ 22, 1, "DisableQuickHero", 	0, { 0 } },
	{ 22, 2, "EnableQuickHero", 	0, { 0 } },
	{ 23, 1, "DisableSpeedText", 	0, { 0 } },
	{ 23, 2, "EnableSpeedText", 	0, { 0 } },
	{ 24, 1, "QuitGame", 			0, { 0 } },
	{ 25, 1, "PushNewRoom", 		0, { 0 } },
	{ 25, 2, "PopNewRoom", 			0, { 0 } },
	{ 26, 1, "ShowCheat", 			0, { 0 } },
	{ 26, 2, "HideCheat", 			0, { 0 } },
	{ 26, 3, "ClearCheat", 			1, { 1 } },
	{ 27, 1, "FeedPassword", 		3, { 1, 1, 1 } }
};

Common::String operators[] = {
	"oper_and",
	"oper_or",
	"oper_xor",
	"oper_equals",
	"oper_not_equal",
	"oper_less_than",
	"oper_greater_than",
	"oper_less_or_equal",
	"oper_greater_or_equal",
	"oper_multiply",
	"oper_divide",
	"oper_remainder",
	"oper_plus",
	"oper_minus"
};

Common::String functions[] = {
	"F_Not",
	"F_Random",
	"F_IsIcoOn",
	"F_IsIcoAct",
	"F_IcoStat",
	"F_ActIco",
	"F_IsObjOn",
	"F_IsObjOff",
	"F_IsObjAway",
	"F_ObjStat",
	"F_LastBlock",
	"F_AtBegin",
	"F_BlockVar",
	"F_HasBeen",
	"F_MaxLine",
	"F_ActPhase",
	"F_Cheat"
};

const unsigned int kNumCommands = sizeof gplCommands / sizeof gplCommands[0];

enum mathExpressionObject {
	kMathEnd,
	kMathNumber,
	kMathOperator,
	kMathFunctionCall,
	kMathVariable
};

// FIXME: The evaluator is now complete but I still need to implement callbacks
void handleMathExpression(Common::MemoryReadStream &reader) {
	Common::Stack<uint16> stk;
	mathExpressionObject obj;

	// Read in initial math object
	obj = (mathExpressionObject) reader.readUint16LE();

	uint16 value;
	while (1) {
		if (obj == kMathEnd) {
			break;
		}

		switch (obj) {

		// If the object type is not known, assume that it's a number
		default:
		case kMathNumber:
			value = reader.readUint16LE();
			stk.push(value);
			debugC(3, kDraciBytecodeDebugLevel, "\t\t-number %hu", value);
			break;

		case kMathOperator:
			value = reader.readUint16LE();
			stk.pop();
			stk.pop();
			debugC(3, kDraciBytecodeDebugLevel, "\t\t-operator %s",
				operators[value-1].c_str());
			break;

		case kMathVariable:
			value = reader.readUint16LE();
			stk.push(value);
			debugC(3, kDraciBytecodeDebugLevel, "\t\t-variable %hu", value);
			break;

		case kMathFunctionCall:
			value = reader.readUint16LE();

			// FIXME: Pushing dummy value for now, but should push return value
			stk.push(0);

			debugC(3, kDraciBytecodeDebugLevel, "\t\t-functioncall %s",
				functions[value-1].c_str());
			break;
		}

		obj = (mathExpressionObject) reader.readUint16LE();
	}

	return;
}

GPL2Command *findCommand(byte num, byte subnum) {
	unsigned int i = 0;
	while (1) {

		// Command not found
		if (i >= kNumCommands) {
			break;
		}

		// Return found command
		if (gplCommands[i]._number == num &&
			gplCommands[i]._subNumber == subnum) {
			return &gplCommands[i];
		}

		++i;
	}

	return NULL;
}

int gpldisasm(byte *gplcode, uint16 len) {
	Common::MemoryReadStream reader(gplcode, len);

	while (!reader.eos()) {
		// read in command pair
		uint16 cmdpair = reader.readUint16BE();

		// extract high byte, i.e. the command number
		byte num = (cmdpair >> 8) & 0xFF;

		// extract low byte, i.e. the command subnumber
		byte subnum = cmdpair & 0xFF;

		GPL2Command *cmd;
		if ((cmd = findCommand(num, subnum))) {

			// Print command name
			debugC(2, kDraciBytecodeDebugLevel, "%s", cmd->_name.c_str());

			for (uint16 i = 0; i < cmd->_numParams; ++i) {
				if (cmd->_paramTypes[i] == 4) {
					debugC(3, kDraciBytecodeDebugLevel, "\t<MATHEXPR>");
					handleMathExpression(reader);
				}
				else {
					debugC(3, kDraciBytecodeDebugLevel, "\t%hu", reader.readUint16LE());
				}
			}
		}
		else {
			debugC(2, kDraciBytecodeDebugLevel, "Unknown opcode %hu, %hu",
				num, subnum);
		}


	}

	return 0;
}

}

