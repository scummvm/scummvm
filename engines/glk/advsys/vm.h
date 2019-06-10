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

#ifndef GLK_ADVSYS_VM
#define GLK_ADVSYS_VM

#include "glk/advsys/glk_interface.h"
#include "glk/advsys/game.h"
#include "common/array.h"

namespace Glk {
namespace AdvSys {

/**
 * Execution result states
 */
enum ExecutionResult {
	IN_PROGRESS = 0,    ///< Default state whilst script is in progress
	FINISH = 1,         ///< Script was finished
	CHAIN = 2,          ///< Another script is being chained to
	ABORT = 3           ///< Script was aborted
};

/**
 * Opcode list
 */
enum Opcode {
	OP_BRT     = 0x01,          ///< Branch on true
	OP_BRF     = 0x02,          ///< Branch on false
	OP_BR      = 0x03,          ///< Branch unconditionally
	OP_T       = 0x04,          ///< Load top of stack with t
	OP_NIL     = 0x05,          ///< Load top of stack with nil
	OP_PUSH    = 0x06,          ///< Push nil onto stack
	OP_NOT     = 0x07,          ///< Logical negate top of stack
	OP_ADD     = 0x08,          ///< Add two numeric expressions
	OP_SUB     = 0x09,          ///< Subtract two numeric expressions
	OP_MUL     = 0x0A,          ///< Multiply two numeric expressions
	OP_DIV     = 0x0B,          ///< Divide two numeric expressions
	OP_REM     = 0x0C,          ///< Remainder of two numeric expressions
	OP_BAND    = 0x0D,          ///< Bitwise and of two numeric expressions
	OP_BOR     = 0x0E,          ///< Bitwise or of two numeric expressions
	OP_BNOT    = 0x0F,          ///< Bitwise not
	OP_LT      = 0x10,          ///< Less than
	OP_EQ      = 0x11,          ///< Equal to
	OP_GT      = 0x12,          ///< Greater than
	OP_LIT     = 0x13,          ///< Load literal
	OP_VAR     = 0x14,          ///< Load a variable value
	OP_GETP    = 0x15,          ///< Get the value of an object property
	OP_SETP    = 0x16,          ///< Set the value of an object property
	OP_SET     = 0x17,          ///< Set the value of a variable
	OP_PRINT   = 0x18,          ///< Print messages
	OP_TERPRI  = 0x19,          ///< Terminate the print line
	OP_PNUMBER = 0x1A,          ///< Print a number
	OP_FINISH  = 0x1B,          ///< Finish handling this command
	OP_CHAIN   = 0x1C,          ///< Chain to the next handler
	OP_ABORT   = 0x1D,          ///< Abort this command
	OP_EXIT    = 0x1E,          ///< Exit the game
	OP_RETURN  = 0x1F,          ///< Return from function
	OP_CALL    = 0x20,          ///< Call subroutine
	OP_SVAR    = 0x21,          ///< Short load a variable
	OP_SSET    = 0x22,          ///< Short set a variable
	OP_SPLIT   = 0x23,          ///< Short load a positive literal
	OP_SNLIT   = 0x24,          ///< Short load a negative literal
	OP_YORN    = 0x25,          ///< Yes or No predicate
	OP_SAVE    = 0x26,          ///< Save data structures
	OP_RESTORE = 0x27,          ///< Restore data structures
	OP_ARG     = 0x28,          ///< Load an argument value
	OP_ASET    = 0x29,          ///< Set an argument value
	OP_TMP     = 0x2A,          ///< Load a temporary variable value
	OP_TSET    = 0x2B,          ///< Set a temporary variable
	OP_TSPACE  = 0x2C,          ///< Allocate temporary variable space
	OP_CLASS   = 0x2D,          ///< Get the class of an object
	OP_MATCH   = 0x2E,          ///< Match a noun phrase with an object
	OP_PNOUN   = 0x2F,          ///< Print a noun phrase
	OP_RESTART = 0x30,          ///< Restart the current game
	OP_RAND    = 0x31,          ///< Generate a random number
	OP_RNDMIZE = 0x32,          ///< Seed the random number generator
	OP_SEND    = 0x33,          ///< Send a message to an object
	OP_VOWEL   = 0x34,          ///< Check for vowel beginning string

	OP_XVAR    = 0x40,          ///< Extra short load a variable
	OP_XSET    = 0x60,          ///< Extra short set a variable
	OP_XPLIT   = 0x80,          ///< Extra short load a positive literal
	OP_XNLIT   = 0xC0           ///< Extra short load a negative literal
};

class VM;
typedef void (VM::*OpcodeMethod)();

/**
 * Main VM for AdvSys
 */
class VM : public GlkInterface, public Game {
	class ArrayStack : public Common::Array<int> {
	public:
		/**
		 * Push a value onto the stack
		 */
		void push(int v) {
			push_back(v);
		}

		/**
		 * Pop a value from the stack
		 */
		int pop() {
			int v = back();
			pop_back();
			return v;
		}

		/**
		 * Returns the top of the stack (the most recently added value
		 */
		int &top() { return back(); }
	};
private:
	static OpcodeMethod _METHODS[0x34];
	int _pc;
	ExecutionResult _status;
	ArrayStack _stack;
private:
	/**
	 * Execute a single opcode within the script
	 */
	void executeOpcode();

	/**
	 * Get the next code byte and increments the PC counter
	 */
	int readCodeByte() {
		return getCodeByte(_pc++);
	}

	/**
	 * Gets the next code word and increases the PC counter to after it
	 */
	int readCodeWord() {
		return getCodeWord(_pc += 2);
	}
private:
	void opBRT();
	void opBRF();
	void opBR();
	void opT();
	void opNIL();
	void opPUSH();
	void opNOT();
	void opADD();
	void opSUB();
	void opMUL();
	void opDIV();
	void opREM();
	void opBAND();
	void opBOR();
	void opBNOT();
	void opLT();
	void opEQ();
	void opGT();
	void opLIT();
	void opVAR();
	void opGETP();
	void opSETP();
	void opSET();
	void opPRINT();
	void opTERPRI();
	void opPNUMBER();
	void opFINISH();
	void opCHAIN();
	void opABORT();
	void opEXIT();
	void opRETURN();
	void opCALL();
	void opSVAR();
	void opSSET();
	void opSPLIT();
	void opSNLIT();
	void opYORN();
	void opSAVE();
	void opRESTORE();
	void opARG();
	void opASET();
	void opTMP();
	void opTSET();
	void opTSPACE();
	void opCLASS();
	void opMATCH();
	void opPNOUN();
	void opRESTART();
	void opRAND();
	void opRNDMIZE();
	void opSEND();
	void opVOWEL();
public:
	/**
	 * Constructor
	 */
	VM(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Exeecute a script
	 * @param offset    Script offset
	 * @returns         Script result code
	 */
	ExecutionResult execute(int offset);
};

} // End of namespace AdvSys
} // End of namespace Glk

#endif
