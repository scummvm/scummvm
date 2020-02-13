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
#include "common/stack.h"

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

/**
 * Indexes useable in function pointer array offsets
 */
enum FPOffset {
	FP_FP = 0,
	FP_PC = 1,
	FP_ARGS_SIZE = 2,
	FP_ARGS = 3
};

/**
 * Action flags
 */
enum ActionFlag {
	A_ACTOR = 1,		///< Actor
	A_DOBJECT = 2,		///< Direct object
	A_IOBJECT = 4		///< Indirect object
};


class VM;
typedef void (VM::*OpcodeMethod)();

/**
 * Fixed stack
 */
class FixedStack : public Common::FixedStack<int, 500> {
public:
	/**
	 * Resize the stack
	 */
	void resize(size_t newSize) {
		assert(newSize <= 500);
		_size = newSize;
	}

	/**
	 * Allocate extra space on the stack
	 */
	void allocate(size_t amount) {
		uint oldSize = _size;
		resize(_size + amount);
		Common::fill(&_stack[oldSize], &_stack[oldSize + amount], 0);
	}
};

/**
 * Implements a function pointer reference into the stack. It also allows
 * positive array indexing to reference the following:
 *		0  = Previous function pointer
 *		1  = Return PC
 *		2  = Size of argument block
 *		3+ = Any function call arguments
 */
class FunctionPointer {
private:
	FixedStack &_stack;
	int _index;
public:
	/**
	 * Constructor
	 */
	FunctionPointer(FixedStack &s) : _stack(s), _index(0) {}

	/**
	 * Array indexing
	 */
	int &operator[](int idx) { return _stack[_index - idx - 1]; }

	/**
	 * Sets the index in the stack of the function pointer
	 */
	FunctionPointer &operator=(int index) {
		_index = index;
		return *this;
	}

	/**
	 * Clear the function pointer
	 */
	void clear() { _index = 0; }

	/**
	 * Returns the index in the stack of the function pointer
	 */
	operator int() const { return _index; }

	/**
	 * Sets the function pointer to the top of the stack
	 */
	void set() {
		_index = _stack.size();
	}
};

/**
 * Main VM for AdvSys
 */
class VM : public GlkInterface, public Game {
	struct InputWord {
		Common::String _text;
		int _number;

		InputWord() : _number(0) {}
		operator int() const { return _number; }
	};
	struct AdjectiveEntry {
		int _list;
		int _word;

		AdjectiveEntry() : _list(0), _word(0) {}
	};
	struct Noun {
		int _noun;
		int _num;
		AdjectiveEntry *_adjective;

		Noun() : _noun(0), _num(0), _adjective(nullptr) {}
	};
private:
	// Execution fields
	static OpcodeMethod _METHODS[0x34];
	int _pc;
	ExecutionResult _status;
	FixedStack _stack;
	FunctionPointer _fp;

	// Parser fields
	int _actor;
	int _action;
	int _dObject;
	int _ndObjects;
	int _iObject;
	Common::Array<InputWord> _words;
	Common::Array<InputWord>::iterator _wordPtr;
	Common::Array<int> _verbs;
	Common::Array<AdjectiveEntry> _adjectiveList;
	Common::Array<Noun> _nouns;
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
		int v = getCodeWord(_pc);
		_pc += 2;
		return v;
	}

	/**
	 * Gets an input line and parse it
	 */
	bool parseInput();

	/**
	 * Gets an input line and splits it up into the words array
	 */
	bool getLine();

	/**
	 * Get the next word of a passed input line
	 * @param line		Input line
	 * @returns			True if a valid word was extracted
	 */
	bool getWord(Common::String &line);

	/**
	 * Get a noun phrase and return the object it refers to
	 */
	uint getNoun();

	/**
	 * Get a verb phrase and return the action it refers to
	 */
	bool getVerb();

	/**
	 * Match an object against a name and list of adjectives
	 */
	bool match(int obj, int noun, const AdjectiveEntry *adjectives);

	/**
	 * Called when a parsing error occurs
	 */
	void parseError();

	/**
	 * Returns true if a passed character is a skippable whitespace
	 */
	static bool isWhitespace(char c);

	/**
	 * Skips over spaces in a passed string
	 */
	static bool skipSpaces(Common::String &str);
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

	/**
	 * Get an input line and parse it
	 */
	bool getInput();

	/**
	 * Get the next command (next direct object)
	 */
	bool nextCommand();
};

} // End of namespace AdvSys
} // End of namespace Glk

#endif
