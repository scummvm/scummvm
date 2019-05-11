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
#include "glk/hugo/htokens.h"
#include "glk/hugo/hugo_defines.h"
#include "glk/hugo/hugo_types.h"
#include "glk/hugo/stringfn.h"

namespace Glk {
namespace Hugo {

/**
 * Hugo game interpreter
 */
class Hugo : public GlkAPI, public HTokens, public StringFunctions {
private:
	winid_t mainwin, currentwin;
	winid_t secondwin, auxwin;
	bool runtime_warnings;
	int dbnest;

	/**
	 * address_scale refers to the factor by which addresses are multiplied to
	 * get the "real" address.  In this way, a 16-bit integer can reference
	 * 64K * 16 = 1024K of memory.
	 */
	int address_scale;

	// heexpr
	int eval[MAX_EVAL_ELEMENTS];		///< expression components
	int evalcount;						///< # of expr. components
	int var[MAXLOCALS + MAXGLOBALS];	///< variables
	int incdec;							///< value is being incremented/dec.
	char getaddress;					///< true when finding &routine
	char inexpr;						///< true when in expression
	char inobj;							///< true when in object compound

	int last_precedence;


	// hemisc
	char gamefile[255];
	int game_version;
	int object_size;
	//HUGO_FILE game;
	Common::SeekableReadStream *game;
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
	bool skipping_more;
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

	// heobject
	int display_object;				///< i.e., non-existent (yet)
	char display_needs_repaint;		///< for display object
	int display_pointer_x, display_pointer_y;

	// heparse
	char buffer[MAXBUFFER + MAXWORDS];
	char errbuf[MAXBUFFER + 1];				///< last invalid input
	char line[1025];						///< line buffer

	int words;								///< parsed word count
	char *word[MAXWORDS + 1];				///< breakdown into words
	unsigned int wd[MAXWORDS + 1];			///<     "      "   dict. entries
	unsigned int parsed_number;				///< needed for numbers in input

	signed char remaining;					///< multiple commands in input
	char parseerr[MAXBUFFER + 1];			///< for passing to RunPrint, etc.
	char parsestr[MAXBUFFER + 1];			///< for passing quoted string
	char xverb;								///< flag; 0 = regular verb
	char starts_with_verb;					///< input line; 0 = no verb word
	unsigned int grammaraddr;				///< address in grammar
	char *obj_parselist;					///< objects with noun/adjective
	int domain, odomain;					///< of object(s)
	int objlist[MAXOBJLIST];				///< for objects of verb
	char objcount;							///< of objlist
	char parse_allflag;						///< for "all" in MatchObject
	pobject_structure pobjlist[MAXPOBJECTS];	///< for possible objects
	int pobjcount;							///< of pobjlist
	int pobj;								///< last remaining suspect
	int obj_match_state;					///< see MatchCommand() for details
	char objword_cache[MAXWORDS];			///< for MatchWord() xobject, etc.
	char object_is_number;					///< number used in player command
	unsigned int objgrammar;				///< for 2nd pass
	int objstart;							///<  "   "   "
	int objfinish;							///<  "   "   "
	char addflag;							///< true if adding to objlist[]
	int speaking;							///< if command is addressed to obj.

	char oops[MAXBUFFER + 1];				///< illegal word
	int oopscount;							///< # of corrections in a row

	char parse_called_twice;
	char reparse_everything;
	char punc_string[64];					///< punctuation string

	char full_buffer;
	/**
	 * to MatchObject()
	 * Necessary for proper disambiguation when addressing a character;
	 * i.e., when 'held' doesn't refer to held by the player, etc.
	 */
	char recursive_call;
	int parse_location;						///< usually var[location]

	// herun
	int passlocal[MAXLOCALS];		///< locals passed to routine
	int arguments_passed;			///< when calling routine
	int ret;						///< return value and returning flag
	char retflag;
	bool during_player_input;
	char override_full;
	bool game_reset;				///< for restore, undo, etc.

	CODE_BLOCK code_block[MAXSTACKDEPTH];
	int stack_depth;
	int tail_recursion;
	long tail_recursion_addr;

	// Used by RunWindow for setting current window dimensions:
	int last_window_top, last_window_bottom, last_window_left, last_window_right;
	int lowest_windowbottom,				///< in text lines
		physical_lowest_windowbottom;		///< in pixels or text lines
	bool just_left_window;

	// heset
	char game_title[MAX_GAME_TITLE];
	char arrexpr;							///< true when assigning array
	char multiprop;							///< true in multiple prop. assign.
	int set_value;

#if defined (DEBUGGER)
	char debug_line[MAX_DEBUG_LINE];
	bool debug_eval;
	bool debug_eval_error;
	bool debugger_step_over;
	bool debugger_finish;
	bool debugger_run;
	int currentroutine;
	bool complex_prop_breakpoint;
	bool trace_complex_prop_routine;
	char *objectname[MAX_OBJECT];
	char *propertyname[MAX_PROPERTY];
//	CODE code[999];
	CALL call[999];
	int properties;
	WINDOW window[99];
	int codeline[9][100];
	char localname[9][100];
	int current_locals;
#endif
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

	/**
	 * \defgroup hemisc
	 * @{
	 */

	/**
	 * The all-purpose printing routine that takes care of word-wrapping.
	 */
	void AP(const char *a);

	/**
	 * Used whenever a routine is called, assumes the routine address and begins
	 * with the arguments (if any).
	 */
	int CallRoutine(unsigned int addr);

	/**
	 * Adds a command to the context command list.  A zero value (i.e., an empty string)
	 * resets the list.
	 */
	void ContextCommand();

	/**
	 * Dynamically creates a new dictionary entry.
	 */
	unsigned int Dict();

	/**
	 * Generates a fatal error
	 */
	void FatalError(int n);

	void FileIO();

	void Flushpbuffer();

	void GetCommand();

	/**
	 * From any address <addr>; the segment must be defined prior to calling the function.
	 */
	char *GetString(long addr);

	/**
	 * Get text block from position <textaddr> in the text bank.  If the game was not fully loaded
	 * in memory, i.e., if loaded_in_memory is not true, the block is read from disk.
	 */
	char *GetText(long textaddr);

	/**
	 * From the dictionary table.
	 */
	char *GetWord(unsigned int w);

	void HandleTailRecursion(long addr);

	void InitGame();

	void LoadGame();

	/**
	 * Must be called before running every new routine, i.e. before calling RunRoutine().
	 * Unfortunately, the current locals must be saved in a temp array prior to calling.
	 * The argument n gives the number of arguments passed.
	 */
	void PassLocals(int n);

	inline unsigned char Peek(long a) {
		return MEM(defseg * 16L + a);
	}

	inline unsigned int PeekWord(long a) {
		return (unsigned char)MEM(defseg * 16L + a) + (unsigned char)MEM(defseg * 16L + a + 1) * 256;
	}

	inline void Poke(unsigned int a, unsigned char v) {
		SETMEM(defseg * 16L + a, v);
	}

	inline void PokeWord(unsigned int a, unsigned int v) {
		SETMEM(defseg * 16L + a, (char)(v % 256));
		SETMEM(defseg * 16L + a + 1, (char)(v / 256));
	}

	/**
	 * Returns <a> as a hex-number string in XXXXXX format.
	 */
	static const char *PrintHex(long a);

	/**
	 * Print to client display taking into account cursor relocation,
	 * font changes, color setting, and window scrolling.
	 */
	void Printout(char *a);

	int RecordCommands();

	/**
	 * Formats:
	 *
	 * end of turn:    (0, undoturn, 0, 0, 0)
	 * move obj.:      (MOVE_T, obj., parent, 0, 0)
	 * property:       (PROP_T, obj., prop., # or PROP_ROUTINE, val.)
	 * attribute:      (ATTR_T, obj., attr., 0 or 1, 0)
	 * variable:       (VAR_T, var., value, 0, 0)
	 * array:          (ARRAYDATA_T, array addr., element, val., 0)
	 * dict:           (DICT_T, entry length, 0, 0, 0)
	 * word setting:   (WORD_T, word number, new word, 0, 0)
	 */
	void SaveUndo(int a, int b, int c, int d, int e);

	/**
	 * Properly sets up the code_block structure for the current stack depth depending
	 * on if this is a called block (RUNROUTINE_BLOCK) or otherwise.
	 */
	void SetStackFrame(int depth, int type, long brk, long returnaddr);

	void SetupDisplay();

	/**
	 * The method is passed <a> as the string and <*i> as the position in the string.
	 * The character(s) at a[*i], a[*(i+1)], etc. are converted into a single Latin-1
	 * (i.e., greater than 127) character value.
	 *
	 * Assume that the AP() has already encountered a control character ('\'),
	 * and that a[*i]... is one of:
	 *
	 * `a	accent grave on following character (e.g., 'a')
	 * 'a	accent acute on following character (e.g., 'a')
	 * ~n	tilde on following (e.g., 'n' or 'N')
	 * :a	umlaut on following (e.g., 'a')
	 * ^a	circumflex on following (e.g., 'a')
	 * ,c	cedilla on following (e.g., 'c' or 'C')
	 * <	Spanish left quotation marks
	 * >	Spanish right quotation marks
	 * !	upside-down exclamation mark
	 * ?	upside-down question mark
	 * ae	ae ligature
	 * AE	AE ligature
	 * c	cents symbol
	 * L	British pound
	 * Y	Japanese Yen
	 * -	em (long) dash
	 * #nnn	character value given by nnn
	 *
	 * Note that the return value is a single character--which will be either unchanged
	 * or a Latin-1 character value.
	 */
	char SpecialChar(const char *a, int *i);

	int Undo();

	/**@}*/

	/**
	 * \defgroup heobject - Object/property/attribute management functions
	 * @{
	 */

#if defined (DEBUGGER)
	int CheckinRange(uint v1, uint v2, const char *v3) {
		// TODO: Where the heck is this actualy implemented in Gargoyle
		return 1;
	}

	 /**
	  * Shorthand since many of these object functions may call CheckinRange() if the debugger
	  * is running and runtime_warnings is set.
	 */
	int CheckObjectRange(int obj);

	void DebugRunRoutine(long addr) {}

	void RuntimeWarning(const char *msg) {}

	void DebugMessageBox(const char *title, const char *msg) {}

	bool IsBreakpoint(long loc) const { return false; }

	const char *RoutineName(long loc) { return "Routine"; }

	void AddStringtoCodeWindow(const char *str) {}
#endif

	int Child(int obj);

	int Children(int obj);

	int Elder(int obj);

	/**
	 * Returns one of four sets of 32 attributes.
	 */
	unsigned long GetAttributes(int obj, int attribute_set);

	/**
	 * Returns the value of '<obj>.<p> #<n>'  If <s> is true, the self global
	 * is not set to <obj> in order to facilitate <obj>..<p> calls.
	 */
	int GetProp(int obj, int p, int n, char s);

	/**
	 * Returns the value of the last object above <obj> in the tree before object 0.
	 */
	int GrandParent(int obj);

	void MoveObj(int obj, int p);

	char *Name(int obj);

	int Parent(int obj);

	/**
	 * Returns address of <obj>.<p> (with <offset> provided for additive properties--
	 * i.e. subsequent calls with the same <obj> and <p>.
	 */
	unsigned int PropAddr(int obj, int p, unsigned int offset);

	/**
	 * Writes (puts) one of four sets of 32 attributes.
	 */
	void PutAttributes(int obj, unsigned long a, int attribute_set);

	/**
	 * Set an attribute
	 * c = 1 for set, 0 for clear
	 */
	void SetAttribute(int obj, int attr, int c);

	int Sibling(int obj);

	int TestAttribute(int obj, int attr, int nattr);

	int Youngest(int obj);

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

	// TODO: Stubs to be Properly implemented
	int GetValue() { return 0; }
	void PlayGame() {}
	void hugo_closefiles() {}
	void RunRoutine(long v) {}
	unsigned int FindWord(const char *a) { return 0; }
	void PromptMore() {}
	void hugo_stopsample() {}
	void hugo_stopmusic() {}
	int hugo_hasgraphics() { return 0; }
};

} // End of namespace Hugo
} // End of namespace Glk

#endif
