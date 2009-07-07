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

#include "common/debug.h"
#include "common/stream.h"
#include "common/stack.h"
#include "common/queue.h"

#include "draci/draci.h"
#include "draci/script.h"
#include "draci/game.h"

namespace Draci {

// FIXME: Change parameter types to names once I figure out what they are exactly

void Script::setupCommandList() {
	/** A table of all the commands the game player uses */
	static const GPL2Command gplCommands[] = {
		{ 0,  0, "gplend",				0, { 0 }, NULL },
		{ 0,  1, "exit",				0, { 0 }, NULL },
		{ 1,  1, "goto", 				1, { 3 }, NULL },
		{ 2,  1, "Let", 				2, { 3, 4 }, NULL },
		{ 3,  1, "if", 					2, { 4, 3 }, NULL },
		{ 4,  1, "Start", 				2, { 3, 2 }, &Script::start },
		{ 5,  1, "Load", 				2, { 3, 2 }, &Script::load },
		{ 5,  2, "StartPlay", 			2, { 3, 2 }, NULL },
		{ 5,  3, "JustTalk", 			0, { 0 }, NULL },
		{ 5,  4, "JustStay", 			0, { 0 }, NULL },
		{ 6,  1, "Talk", 				2, { 3, 2 }, NULL },
		{ 7,  1, "ObjStat", 			2, { 3, 3 }, NULL },
		{ 7,  2, "ObjStat_On", 			2, { 3, 3 }, NULL },
		{ 8,  1, "IcoStat", 			2, { 3, 3 }, NULL },
		{ 9,  1, "Dialogue", 			1, { 2 }, NULL },
		{ 9,  2, "ExitDialogue", 		0, { 0 }, NULL },
		{ 9,  3, "ResetDialogue", 		0, { 0 }, NULL },
		{ 9,  4, "ResetDialogueFrom", 	0, { 0 }, NULL },
		{ 9,  5, "ResetBlock", 			1, { 3 }, NULL },
		{ 10, 1, "WalkOn", 				3, { 1, 1, 3 }, NULL },
		{ 10, 2, "StayOn", 				3, { 1, 1, 3 }, NULL },
		{ 10, 3, "WalkOnPlay", 			3, { 1, 1, 3 }, NULL },
		{ 11, 1, "LoadPalette", 		1, { 2 }, NULL },
		{ 12, 1, "SetPalette", 			0, { 0 }, NULL },
		{ 12, 2, "BlackPalette", 		0, { 0 }, NULL },
		{ 13, 1, "FadePalette", 		3, { 1, 1, 1 }, NULL },
		{ 13, 2, "FadePalettePlay", 	3, { 1, 1, 1 }, NULL },
		{ 14, 1, "NewRoom", 			2, { 3, 1 }, NULL },
		{ 15, 1, "ExecInit", 			1, { 3 }, NULL },
		{ 15, 2, "ExecLook", 			1, { 3 }, NULL },
		{ 15, 3, "ExecUse", 			1, { 3 }, NULL },
		{ 16, 1, "RepaintInventory", 	0, { 0 }, NULL },
		{ 16, 2, "ExitInventory", 		0, { 0 }, NULL },
		{ 17, 1, "ExitMap", 			0, { 0 }, NULL },
		{ 18, 1, "LoadMusic", 			1, { 2 }, NULL },
		{ 18, 2, "StartMusic", 			0, { 0 }, NULL },
		{ 18, 3, "StopMusic", 			0, { 0 }, NULL },
		{ 18, 4, "FadeOutMusic",		1, { 1 }, NULL },
		{ 18, 5, "FadeInMusic", 		1, { 1 }, NULL },
		{ 19, 1, "Mark", 				0, { 0 }, NULL },
		{ 19, 2, "Release", 			0, { 0 }, NULL },
		{ 20, 1, "Play", 				0, { 0 }, NULL },
		{ 21, 1, "LoadMap", 			1, { 2 }, NULL },
		{ 21, 2, "RoomMap", 			0, { 0 }, NULL },
		{ 22, 1, "DisableQuickHero", 	0, { 0 }, NULL },
		{ 22, 2, "EnableQuickHero", 	0, { 0 }, NULL },
		{ 23, 1, "DisableSpeedText", 	0, { 0 }, NULL },
		{ 23, 2, "EnableSpeedText", 	0, { 0 }, NULL },
		{ 24, 1, "QuitGame", 			0, { 0 }, NULL },
		{ 25, 1, "PushNewRoom", 		0, { 0 }, NULL },
		{ 25, 2, "PopNewRoom", 			0, { 0 }, NULL },
		{ 26, 1, "ShowCheat", 			0, { 0 }, NULL },
		{ 26, 2, "HideCheat", 			0, { 0 }, NULL },
		{ 26, 3, "ClearCheat", 			1, { 1 }, NULL },
		{ 27, 1, "FeedPassword", 		3, { 1, 1, 1 }, NULL }
	};

	/** Operators used by the mathematical evaluator */
	static const GPL2Operator gplOperators[] = {
		{"&", 	&Script::operAnd 			},
		{"|",	&Script::operOr 			},
		{"^", 	&Script::operXor 			},
		{"==",	&Script::operEqual 			},
		{"!=", 	&Script::operNotEqual		},
		{"<", 	&Script::operLess			},
		{">", 	&Script::operGreater		},
		{"<=",	&Script::operLessOrEqual	},
		{">=",	&Script::operGreaterOrEqual	},
		{"*",	&Script::operMul			},
		{"/", 	&Script::operDiv			},
		{"%",	&Script::operMod			},
		{"+", 	&Script::operAdd			},
		{"-",	&Script::operSub			}
	};

	/** Functions used by the mathematical evaluator */
	static const GPL2Function gplFunctions[] = {
		{ "Not", 		NULL },
		{ "Random", 	&Script::funcRandom },
		{ "IsIcoOn", 	NULL },
		{ "IsIcoAct", 	NULL },
		{ "IcoStat", 	NULL },
		{ "ActIco", 	NULL },
		{ "IsObjOn", 	NULL },
		{ "IsObjOff", 	NULL },
		{ "IsObjAway", 	NULL },
		{ "ObjStat", 	NULL },
		{ "LastBlock", 	NULL },
		{ "AtBegin", 	NULL },
		{ "BlockVar", 	NULL },
		{ "HasBeen", 	NULL },
		{ "MaxLine", 	NULL },
		{ "ActPhase", 	NULL },
		{ "Cheat",  	NULL },
	};

	_commandList = gplCommands;
	_operatorList = gplOperators;
	_functionList = gplFunctions;
}

/** Type of mathematical object */
enum mathExpressionObject {
	kMathEnd,
	kMathNumber,
	kMathOperator,
	kMathFunctionCall,
	kMathVariable
};

/* GPL operators */

int Script::operAnd(int op1, int op2) {
	return op1 & op2;
}

int Script::operOr(int op1, int op2) {
	return op1 | op2;
}

int Script::operXor(int op1, int op2) {
	return op1 ^ op2;
}

int Script::operEqual(int op1, int op2) {
	return op1 == op2;
}

int Script::operNotEqual(int op1, int op2) {
	return op1 != op2;
}

int Script::operLess(int op1, int op2) {
	return op1 < op2;
}

int Script::operGreater(int op1, int op2) {
	return op1 > op2;
}

int Script::operGreaterOrEqual(int op1, int op2) {
	return op1 >= op2;
}

int Script::operLessOrEqual(int op1, int op2) {
	return op1 <= op2;
}

int Script::operMul(int op1, int op2) {
	return op1 * op2;
}

int Script::operAdd(int op1, int op2) {
	return op1 + op2;
}

int Script::operSub(int op1, int op2) {
	return op1 - op2;
}

int Script::operDiv(int op1, int op2) {
	return op1 / op2;
}

int Script::operMod(int op1, int op2) {
	return op1 % op2;
}

/* GPL functions */

int Script::funcRandom(int n) {

// The function needs to return numbers in the [0..n-1] range so we need to deduce 1
// (RandomSource::getRandomNumber returns a number in the range [0..n])

	n -= 1;
	return _vm->_rnd.getRandomNumber(n);
}

/* GPL commands */

void Script::load(Common::Queue<int> &params) {
	int objID = params.pop() - 1;
	int animID = params.pop() - 1;

	GameObject *obj = _vm->_game->getObject(objID);

	obj->_seqTab[animID - obj->_idxSeq] = _vm->_game->loadAnimation(animID);
}

void Script::start(Common::Queue<int> &params) {
	int objID = params.pop() - 1;
	int animID = params.pop() - 1;

	GameObject *obj = _vm->_game->getObject(objID);
	
	bool visible = (objID == kDragonObject || obj->_visible);

	if (visible && (obj->_location == _vm->_game->getRoomNum()))
		_vm->_anims->play(animID);
}

/**
 * @brief Evaluates mathematical expressions
 * @param reader Stream reader set to the beginning of the expression
 */

int Script::handleMathExpression(Common::MemoryReadStream &reader) {
	Common::Stack<int> stk;
	mathExpressionObject obj;
	GPL2Operator oper;
	GPL2Function func;

	// Read in initial math object
	obj = (mathExpressionObject)reader.readUint16LE();

	int value;
	int arg1, arg2, res;

	while (1) {
		if (obj == kMathEnd) {
			// Check whether the expression was evaluated correctly
			// The stack should contain only one value after the evaluation
			// i.e. the result of the expression
			assert(stk.size() == 1 && "Mathematical expression error");	
			break;
		}

		switch (obj) {

		// If the object type is not known, assume that it's a number
		default:
		case kMathNumber:
			value = reader.readUint16LE();
			stk.push(value);
			debugC(3, kDraciBytecodeDebugLevel, "\t\tnumber: %d", value);
			break;

		case kMathOperator:
			value = reader.readUint16LE();
			arg1 = stk.pop();
			arg2 = stk.pop();

			// Fetch operator
			oper = _operatorList[value-1];

			// Calculate result
			res = (this->*(oper._handler))(arg1, arg2);
			
			// Push result
			stk.push(res);

			debugC(3, kDraciBytecodeDebugLevel, "\t\t%d %s %d (res: %d)",
				arg1, oper._name.c_str(), arg2, res);
			break;

		case kMathVariable:
			value = reader.readUint16LE();
			stk.push(value);
			debugC(3, kDraciBytecodeDebugLevel, "\t\tvariable: %d", value);
			break;

		case kMathFunctionCall:
			value = reader.readUint16LE();

			// Fetch function
			func = _functionList[value-1];
					
			// If not yet implemented	
			if (func._handler == NULL) {
				stk.pop();

				// FIXME: Pushing dummy value for now, but should push return value
				stk.push(0);

				debugC(3, kDraciBytecodeDebugLevel, "\t\tcall: %s (not implemented)",
					func._name.c_str());
			} else {
				arg1 = stk.pop();

				// Calculate result
				res = (this->*(func._handler))(arg1);
			
				debugC(3, kDraciBytecodeDebugLevel, "\t\tcall: %s(%d) (res: %d)",
					func._name.c_str(), arg1, res);
			}

			break;
		}

		obj = (mathExpressionObject) reader.readUint16LE();
	}

	return stk.pop();
}

/**
 * @brief Find the current command in the internal table
 *
 * @param num 		Command number
 * @param subnum 	Command subnumer
 *
 * @return NULL if command is not found. Otherwise, a pointer to a GPL2Command
 *         struct representing the command.
 */
const GPL2Command *Script::findCommand(byte num, byte subnum) {
	unsigned int i = 0;
	while (1) {

		// Command not found
		if (i >= kNumCommands) {
			break;
		}

		// Return found command
		if (_commandList[i]._number == num &&
			_commandList[i]._subNumber == subnum) {
			return &_commandList[i];
		}

		++i;
	}

	return NULL;
}

/**
 * @brief GPL2 bytecode disassembler
 * @param program GPL program in the form of a GPL2Program struct
 *        offset Offset into the program where execution should begin
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

int Script::run(GPL2Program program, uint16 offset) {

	// Stream reader for the whole program
	Common::MemoryReadStream reader(program._bytecode, program._length);
	
	// Parameter queue that is passed to each command
	Common::Queue<int> params;

	// Offset is given as number of 16-bit integers so we need to convert
	// it to a number of bytes  
	offset -= 1;
	offset *= 2;

	// Seek to the requested part of the program
	reader.seek(offset);
	
	const GPL2Command *cmd;
	do {

		// Clear any parameters left on the stack from the previous command
		// This likely won't be needed once all commands are implemented
		params.clear();

		// read in command pair
		uint16 cmdpair = reader.readUint16BE();

		// extract high byte, i.e. the command number
		byte num = (cmdpair >> 8) & 0xFF;

		// extract low byte, i.e. the command subnumber
		byte subnum = cmdpair & 0xFF;

		if ((cmd = findCommand(num, subnum))) {
			int tmp;

			// Print command name
			debugC(2, kDraciBytecodeDebugLevel, "%s", cmd->_name.c_str());

			for (int i = 0; i < cmd->_numParams; ++i) {
				if (cmd->_paramTypes[i] == 4) {
					debugC(3, kDraciBytecodeDebugLevel, "\t<MATHEXPR>");
					params.push(handleMathExpression(reader));
				}
				else {
					tmp = reader.readUint16LE();
					params.push(tmp);
					debugC(3, kDraciBytecodeDebugLevel, "\t%hu", tmp);
				}
			}
		}
		else {
			debugC(2, kDraciBytecodeDebugLevel, "Unknown opcode %hu, %hu",
				num, subnum);
		}

		GPLHandler handler = cmd->_handler;

		if (handler != NULL) {
			// Call the handler for the current command
			(this->*(cmd->_handler))(params);
		}

	} while (cmd->_name != "gplend" || cmd->_name != "exit");

	return 0;
}

}

