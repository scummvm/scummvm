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
#include "glk/hugo/hugo_defines.h"
#include "glk/hugo/hugo_types.h"

namespace Glk {
namespace Hugo {

/**
 * Hugo game interpreter
 */
class Hugo : public GlkAPI {
private:
	winid_t mainwin, currentwin;
	winid_t secondwin, auxwin;

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
	char context_command[MAX_CONTEXT_COMMANDS][64];
	int context_commands;
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
	int secondwin_bottom;

	// heparse
	char buffer[MAXBUFFER + MAXWORDS];

	// heexpr
	int var[MAXLOCALS + MAXGLOBALS];
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
	void hugo_cleanup_screen() {
		// No implementation
	}

	int hugo_getkey() const {
		// Not needed here--single-character events are handled solely by hugo_waitforkey(), below
		return 0;
	}

	/**
	 * Gets a line of input from the keyboard, storing it in <buffer>.
	 */
	void hugo_getline(const char *prmpt);

	/**
	 * Provided to be replaced by multitasking systems where cycling while waiting
	 * for a keystroke may not be such a hot idea.
	 */
	int hugo_waitforkey();

	/**
	 * Returns true if a keypress is waiting to be retrieved.
	 */
	int hugo_iskeywaiting();

	/**
	 * Waits for 1/n seconds.  Returns false if waiting is unsupported.
	 */
	int hugo_timewait(int n);

	/**
	 * Clears everything on the screen, moving the cursor to the top-left corner of the screen
	 */
	void hugo_clearfullscreen();

	/**
	 * Clears the currently defined window, moving the cursor to the top-left corner of the window
	 */
	void hugo_clearwindow();

	/**
	 * This function does whatever is necessary to set the system up for a standard text display
	 */
	void hugo_settextmode();

	void hugo_settextwindow(int left, int top, int right, int bottom);

	/**
	 * Specially accommodated in GetProp() While the engine thinks that the linelength is 0x7fff,
	this tells things like the display object the actual length.  (Defined as ACTUAL_LINELENGTH)
	*/
	int heglk_get_linelength();

	/**
	 * Similar to heglk_get_linelength().  (Defined as ACTUAL_SCREENHEIGHT)
	 */
	int heglk_get_screenheight();

	void hugo_settextpos(int x, int y);

	/**
	 * Essentially the same as printf() without formatting, since printf() generally doesn't take
	 * into account color setting, font changes, windowing, etc.
	 *
	 * The newline character '\n' must be explicitly included at the end of a line in order to
	 * produce a linefeed.  The new cursor position is set to the end of this printed text.
	 * Upon hitting the right edge of the screen, the printing position wraps to the start
	 * of the next line.
	 */
	void hugo_print(const char *a);

	/**
	 * Scroll the text window
	 */
	void hugo_scrollwindowup() {
		// No implementation. Glk takes care of it
	}

	/**
	 * Set the font
	 * @param f		The <f> argument is a mask containing any or none of:
	 *				BOLD_FONT, UNDERLINE_FONT, ITALIC_FONT, PROP_FONT.
	 */
	void hugo_font(int f);

	/**
	 * Set the foreground (print) color
	 */
	void hugo_settextcolor(int c);

	/**
	 * Set the background color
	 */
	void hugo_setbackcolor(int c);

	/**
	 * Color-setting functions should always pass the color through hugo_color()
	 * in order to properly set default fore/background colors
	 */
	int hugo_color(int c);

	/**
	 * Get the width of a character
	 * @remarks		As given here, this function works only for non-proportional printing.
	 * For proportional printing, hugo_charwidth() should return the width of the supplied
	 * character in the current font and style.
	*/
	int hugo_charwidth(char a) const;

	/**
	 * Return the width of a string
	 */
	int hugo_textwidth(const char *a) const;

	/**
	 * Return the length of a string
	 */
	int hugo_strlen(const char *a) const;

	void hugo_setgametitle(const char *t);

	int hugo_hasvideo() const;

	int hugo_playvideo(HUGO_FILE infile, long reslength, char loop_flag, char background, int volume);

	void hugo_stopvideo();

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
