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

#ifndef GLK_HUGO_HUGO
#define GLK_HUGO_HUGO

#include "common/scummsys.h"
#include "glk/glk_api.h"
#include "glk/hugo/hugo_types.h"

namespace Glk {
namespace Hugo {

/**
 * Hugo game interpreter
 */
class Hugo : public GlkAPI {
private:
	winid_t mainwin, currentwin;

	/**
	 * address_scale refers to the factor by which addresses are multiplied to
	 * get the "real" address.  In this way, a 16-bit integer can reference
	 * 64K * 16 = 1024K of memory.
	 */
	int address_scale;

	int game_version;
	int object_size;
	HUGO_FILE game;
	HUGO_FILE script;
	HUGO_FILE save;
	HUGO_FILE playback;
	HUGO_FILE record;
	HUGO_FILE io; char ioblock; char ioerror;

	char id[3];
	char serial[9];
	unsigned int codestart;
	unsigned int objtable;
	unsigned int eventtable;
	unsigned int proptable;
	unsigned int arraytable;
	unsigned int dicttable;
	unsigned int syntable;
	unsigned int initaddr;
	unsigned int mainaddr;
	unsigned int parseaddr;
	unsigned int parseerroraddr;
	unsigned int findobjectaddr;
	unsigned int endgameaddr;
	unsigned int speaktoaddr;
	unsigned int performaddr;
	int objects;
	int events;
	int dictcount;
	int syncount;
#if !defined (COMPILE_V25)
	char context_command[MAX_CONTEXT_COMMANDS][64];
	int context_commands;
#endif
	unsigned char *mem;
	int loaded_in_memory;
	unsigned int defseg;
	unsigned int gameseg;
	long codeptr;
	long codeend;
	char pbuffer[MAXBUFFER * 2 + 1];
	int currentpos;
	int currentline;
	int full;
	signed char def_fcolor, def_bgcolor, def_slfcolor, def_slbgcolor;
	signed char fcolor, bgcolor, icolor, default_bgcolor;
	int currentfont;
	char capital;
	unsigned int textto;
	int SCREENWIDTH, SCREENHEIGHT;
	int physical_windowwidth, physical_windowheight,
		physical_windowtop, physical_windowleft,
		physical_windowbottom, physical_windowright;
	int inwindow;
	int charwidth, lineheight, FIXEDCHARWIDTH, FIXEDLINEHEIGHT;
	int current_text_x, current_text_y;
	int undostack[MAXUNDO][5];
	int undoptr;
	int undoturn;
	char undoinvalid;
	char undorecord;

	bool in_valid_window;
	int glk_fcolor, glk_bgcolor;
	int mainwin_bgcolor;
	int glk_current_font;
	bool just_cleared_screen;
private:
	/**
	 * \defgroup heglk
	 * @{
	 */

	 /**
	  * Does whatever has to be done to initially set up the display
	  */
	void hugo_init_screen();

	/**
	 * Does whatever has to be done to clean up the display pre-termination
	 */
	void hugo_cleanup_screen();

	void hugo_settextwindow(int left, int top, int right, int bottom);

	/**@}*/
private:
	/**
	 * Allocate memory block
	 */
	void *hugo_blockalloc(size_t num) { return new byte[num]; }

	void hugo_blockfree(void *block) { delete[] block; }
public:
	/**
	 * Constructor
	 */
	Hugo(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Run the game
	 */
	void runGame();

	/**
	 * Returns the running interpreter type
	 */
	virtual InterpreterType getInterpreterType() const override { return INTERPRETER_HUGO; }

	/**
	 * Load a savegame from the passed stream
	 */
	virtual Common::Error loadGameData(strid_t file) override;

	/**
	 * Save the game to the passed stream
	 */
	virtual Common::Error saveGameData(strid_t file, const Common::String &desc) override;
};

} // End of namespace Hugo
} // End of namespace Glk

#endif
