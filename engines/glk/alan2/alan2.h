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

#ifndef GLK_ALAN2
#define GLK_ALAN2

#include "common/scummsys.h"
#include "common/stack.h"
#include "glk/glk_api.h"
#include "glk/alan2/acode.h"
#include "glk/alan2/types.h"

namespace Glk {
namespace Alan2 {

typedef Common::FixedStack<Aptr, 100> Alan2Stack;
class Decode;
class Execute;
class Interpreter;
class SaveLoad;

#define N_EVTS 100

/**
 * Alan2 game interpreter
 */
class Alan2 : public GlkAPI {
public:
	bool vm_exited_cleanly;
private:
	/**
	 * Validates the game file, and if it's invalid, displays an error dialog
	 */
	bool is_gamefile_valid();
public:
	/**
	 * Constructor
	 */
	Alan2(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Run the game
	 */
	void runGame();

	/**
	 * Returns the running interpreter type
	 */
	virtual InterpreterType getInterpreterType() const override { return INTERPRETER_ALAN2; }

	/**
	 * Load a savegame from the passed stream
	 */
	virtual Common::Error loadGameData(strid_t file) override;

	/**
	 * Save the game to the passed stream
	 */
	virtual Common::Error saveGameData(strid_t file, const Common::String &desc) override;

	/**
	 * Output a string to the screen
	 */
	void output(const Common::String str);

	/**
	 * Print a message from the message table
	 */
	void printMessage(MsgKind msg);

	/**
	 * Print an error from the message table, force new player input and abort
	 */
	void printError(MsgKind msg);

	/**
	 * Make a new paragraph, i.e one empty line (one or two newlines)
	 */
	void paragraph();

	/**
	 * Print the the status line on the top of the screen
	 */
	void statusLine();

	/**
	 * Make a newline, but check for screen full
	 */
	void newLine();

	// Engine variables
	Alan2Stack *_stack;
	int pc;
	ParamElem *params;
	Aword *memory;	// The Amachine memory
	int memTop;		// Top of memory
	CurVars cur;	// Amachine variables
	int col;
	bool fail;
	int scores[100];	// FIXME: type + size
	AcdHdr *header;
	bool _needSpace;		// originally "needsp"

	EvtElem *evts;					// Event table pointer
	bool looking;					// LOOKING? flag
	int dscrstkp;   	            // Describe-stack pointer
	Common::File *_txtFile;
	bool _anyOutput;
	winid_t _bottomWindow;

	Decode *_decode;
	Execute *_execute;
	Interpreter *_interpreter;
	SaveLoad *_saveLoad;
};

extern Alan2 *_vm;

} // End of namespace Alan2
} // End of namespace Glk

#endif
