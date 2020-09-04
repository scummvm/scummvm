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

/* Based on the Hugo interpreter version 3.3.0 */

#ifndef GLK_HUGO_HUGO
#define GLK_HUGO_HUGO

#include "common/scummsys.h"
#include "common/str.h"
#include "glk/glk_api.h"
#include "glk/hugo/htokens.h"
#include "glk/hugo/hugo_defines.h"
#include "glk/hugo/hugo_types.h"
#include "glk/hugo/stringfn.h"

namespace Glk {
namespace Hugo {

class ResourceArchive;

/**
 * Hugo game interpreter
 */
class Hugo : public GlkAPI, public HTokens, public StringFunctions {
	friend class ResourceArchive;
private:
	int _savegameSlot;
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

	// hemedia
	schanid_t mchannel;
	schanid_t schannel;
	long resids[2][MAXRES];
	int numres[2];

	// hemisc
	char gamefile[255];
	int game_version;
	int object_size;
	Common::SeekableReadStream *game;
	HUGO_FILE script;
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
	bool loaded_in_memory;
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
	bool addflag;							///< true if adding to objlist[]
	int speaking;							///< if command is addressed to obj.

	char oops[MAXBUFFER + 1];				///< illegal word
	int oopscount;							///< # of corrections in a row

	char parse_called_twice;
	char reparse_everything;
	char punc_string[64];					///< punctuation string
	byte full_buffer;

	/**
	 * to MatchObject()
	 * Necessary for proper disambiguation when addressing a character;
	 * i.e., when 'held' doesn't refer to held by the player, etc.
	 */
	char recursive_call;
	int parse_location;						///< usually var[location]

	// heres
	HUGO_FILE resource_file;
	int extra_param;
	char loaded_filename[MAX_RES_PATH];
	char loaded_resname[MAX_RES_PATH];
	char resource_type;

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
	bool debugger_interrupt;
	bool debugger_skip;
	bool runtime_error;
	uint currentroutine;
	bool complex_prop_breakpoint;
	bool trace_complex_prop_routine;
	char *objectname[MAX_OBJECT];
	char *propertyname[MAX_PROPERTY];
	CALL call[MAXCALLS];
	int routines;
	int properties;
	WINDOW window[9];
	int codeline[9][100];
	char localname[9][100];
	int current_locals;
	long this_codeptr;
	int debug_workspace;
	int attributes;
	int original_dictcount;
	int buffered_code_lines;
	bool debugger_has_stepped_back;
	bool debugger_step_back;
	int debugger_collapsing;
	int runaway_counter;
	int history_count;
	int active_screen;
	int step_nest;
	BREAKPOINT breakpoint[MAXBREAKPOINTS];
	BREAKPOINT watch[MAXBREAKPOINTS];
	int code_history[MAX_CODE_HISTORY];
	int dbnest_history[MAX_CODE_HISTORY];
	int history_last;
#endif
private:
	/**
	 * \defgroup heexpr
	 * @{
	 */

	 /**
	  * The new-and-improved expression evaluator.  Evaluates the current expression
	  * (or sub-expression therein) beginning at eval[p].
	  */
	int EvalExpr(int p);

	/**
	 * Called by GetValue(); does the actual dirty work of returning a value from a
	 * simple data type.
	 */
	int GetVal();

	/**
	 * Does any reckoning for more sophisticated constructions.
	 */
	int GetValue();

	/**
	 * Actually performs the increment given below by IsIncrement.
	 */
	int Increment(int a, char inctype);

	/**
	 * If an increment/decrement is next up (i.e. ++, --, or +=, *=, etc.),
	 * then sets incdec equal to the increment/decrement and repositions codeptr.
	 * Returns the token number of the operation, if any.
	 */
	char IsIncrement(long addr);

	/**
	 *  Returns the precedence ranking of the operator represented by token[t].
	 * The lower the return value, the higher the rank in terms of processing order.
	 */
	int Precedence(int t);

	/**
	 * Reads the current expression from the current code position into eval[],
	 * using the following key:
	 *
	 * if eval[n] is 0, eval[n+1] is a value
	 * if eval[n] is 1, eval[n+1] is a token
	 *
	 * <inexpr> is used in various routines to keep track of whether or not we're currently
	 * reading an expression.  If <inexpr> is 1, we're in an expression; if 2, we may have
	 * to step back one code position if encountering a closing parentheses.
	 */
	void SetupExpr();

	/**
	 * Cuts off straggling components of eval[] after an expression or sub-expression
	 * has been successfully evaluated.
	 */
	void TrimExpr(int ptr);

	/**@}*/

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

	void hugo_closefiles() {
		// Glk closes all files on exit
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
	void Printout(char *a, int no_scrollback_linebreak);

	void PromptMore();

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
	 * \defgroup hemedia
	 * @{
	 */

	int loadres(HUGO_FILE infile, int reslen, int type);

	int hugo_hasgraphics();

	void initsound();

	void initmusic();

	int hugo_playmusic(HUGO_FILE infile, long reslen, char loop_flag);

	void hugo_musicvolume(int vol);

	void hugo_stopmusic();

	int hugo_playsample(HUGO_FILE infile, long reslen, char loop_flag);

	void hugo_samplevolume(int vol);

	void hugo_stopsample();

	/**@}*/

	/**
	 * \defgroup heobject - Object/property/attribute management functions
	 * @{
	 */
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

	const char *Name(int obj);

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

	/**
	 * \defgroup heparse
	 * @{
	 */

	void AddAllObjects(int loc);

	/**
	 * Adds the object <obj> to objlist[], making all related adjustments.
	 */
	void AddObj(int obj);

	/**
	 * Adds <obj> as a contender to the possible object list, noting that it was referred
	 * to as either a noun or an adjective.
	 */
	void AddPossibleObject(int obj, char type, unsigned int w);

	/**
	 * Move the address in the grammar table past the current token.
	 */
	void AdvanceGrammar();

	/**
	 * For when it's only necessary to know if word[wn] is an object word for any object,
	 * not a particular object.  Returns 1 for an object word or -1 for a non-object word.
	 */
	int AnyObjWord(int wn);

	/**
	 * The non_grammar argument is true when called from a non-grammar function such as RunEvents().
	 */
	int Available(int obj, char non_grammar);

	void CallLibraryParse();

	/**
	 * Takes into account the preset domain for checking an object's presence;
	 * <domain> is 0, -1, or an object number..
	 */
	int DomainObj(int obj);

	/**
	 * Returns the dictionary address of <a>.
	 */
	unsigned int FindWord(const char *a);

	/**
	 * Checks to see if <obj> is in objlist[].
	 */
	int InList(int obj);

	/**
	 * Deletes word[a].
	 */
	void KillWord(int a);

	/**
	 * Here, briefly, is how MatchCommand() works:
	 *
	 * 1.  Match the verb.
	 *
	 * 2.  If no match, check to see if the line begins with an object (character)
	 * and try to match it.
	 *
	 * 3.  If found, try to match a syntax for that verb, including objects, dictionary words,
	 * numbers, attributes, and routines.  If any objects are specified, skip over them for now,
	 * marking the start and finish.  This is done mostly in MatchWord().
	 *
	 * 4.  Match the xobject, if there is one--via MatchObject().
	 *
	 * 5.  If all is well, return to match the objects that were previously skipped over,
	 * loading them into objlist[]. Once again, this is done by MatchObject().
	 *
	 * (The reason the objects are initially skipped is because it may be necessary to know
	 * where to look for them--this may require knowing what the xobject is, if the syntax
	 * is something like:
	 *
	 * "get" <object> "from" <xobject>)
	 *
	 * The variable <obj_match_state> is the indicator of what stage object-matching is at:
	 *
	 * obj_match_state = 0  - haven't matched anything yet
	 *
	 * obj_match_state = 1  - xobject has been matched
	 *
	 * obj_match_state = 2  - matching object(s), loading objlist[]
	 *
	 * obj_match_state = 5  - matching first word/name, i.e., "Bob, <do something>"
	 */
	int MatchCommand();

	/**
	 * The argument is the word number we're starting matching on.
	 *
	 * NOTE:  recusive_call is set to 0 if this is the first call. MatchObject() sets it to 1
	 * when calling itself when asking for clarification as to which object is meant.
	 *
	 * Return true on a recursive call to allow parsing to continue.
	 */
	bool MatchObject(int *wordnum);

	int MatchWord(int *wordnum);

	/**
	 * Returns true if the specified object has the specified word as an adjective or noun
	 * (as specified by type).
	 */
	int ObjWordType(int obj, unsigned int w, int type);

	/**
	 * Returns <adjective> if the word at dictionary address <w> is an adjective of <obj>,
	 * or <noun> if it is a noun.
	 */
	int ObjWord(int obj, unsigned int w);

	/**
	 * Turns word[] into dictionary addresses stored in wd[].  Takes care of fingering illegal
	 * (unknown) words and doing alterations such as compounds, removals, and synonyms.
	 */
	int Parse();

	void ParseError(int e, int a);

	/**
	 * Deletes wd[a].
	 */
	void RemoveWord(int a);

	/**
	 * Call FindObject(0, 0) to reset library's disambiguation mechanism.
	 */
	void ResetFindObject();

	/**
	 * Splits <buffer> into the word[] array.  Also does nifty things such as turning time
	 * values such as hh:mm into a single number (representing minutes from midnight).
	 */
	void SeparateWords();

	/**
	 * Removes object <obj> from objlist[], making all related adjustments.
	 */
	void SubtractObj(int obj);

	/**
	 * Removes <obj> as a possible contender for object disambiguation.
	 */
	void SubtractPossibleObject(int obj);

	/**
	 * Called by MatchObject() to see if <obj> is available, and add it to or subtract
	 * it from objlist[] accordingly.
	 */
	void TryObj(int obj);

	/**
	 * Checks first of all to see if an object is available, then checks if it meets
	 * all the qualifications demanded by the grammar syntax.
	 */
	int ValidObj(int obj);

	/**@}*/

	/**
	 * \defgroup heres
	 * @{
	 */

	void DisplayPicture();

	void PlayMusic();

	void PlaySample();

	void PlayVideo();

	/**
	 * Assumes that filename/resname contain a resourcefile name and a resource name.
	 * If resname is "", filename contains the path of the resource on disk.
	 * Returns the length of the resource if if the named resource is found.
	 *
	 * If FindResource() returns non-zero, the file is hot, i.e., it is open and positioned
	 * to the start of the resource.
	 *
	 * Note that resourcefiles are expected to be in (if not the current directory) "object" or "games",
	 * and on-disk resources in (if not the given directory) "source" or "resource" (where these are the
	 * environment variables "HUGO_...", not actual on-disk directories).
	 */
	long FindResource(const char *filename, const char *resname);

	/**
	 * Processes resourcefile/filename (and resource, if applicable).
	 * Returns 0 if a valid 0 parameter is passed as in "music 0" or "sound 0".
	 */
	int GetResourceParameters(char *filename, char *resname, int restype);

	/**@}*/

	/**
	* \defgroup herun
	* @{
	*/

	void RunDo();

	void RunEvents();

	void playGame();

	void RunIf(char override);

	void RunInput();

	/**
	 * (All the debugger range-checking is important because invalid memory writes due
	 * to invalid object location calculations are a good way to crash the system.)
	 */
	void RunMove();

	void RunPrint();

	int RunRestart();

	int RestoreGameData();

	int RunRestore();

	/**
	 * This is the main loop for running each line of code in sequence;
	 * the main switch statement is based on the first token in each line.
	 *
	 * This routine is relatively complex, especially given the addition of debugger control.
	 * Basically it is structured like this:
	 *
	 * 1.  If this is the debugger build, see what debugger information has to be set up upon
	 * calling this block of code
	 *
	 * 2.  Get the next token, and as long as it isn't CLOSE_BRACE_T ('}')...
	 *
	 * 3.  ...If this is the debugger build, see if there is a standing debugger_interrupt
	 * to pass control back to the debugger, and perform all operations for stepping
	 * tracing, breakpoints, etc.
	 *
	 * 4.  ...See what token we're dealing with and execute accordingly
	 *
	 * 5.  ...Loop back to (2)
	 *
	 * 6.  If this is the debugger build, do whatever is necessary to tidy up after finishing
	 * this block of code
	 *
	 * There's a bit of a trick involved since the original language design uses "{...}"
	 * structures for both conditionals and blocks that necessitate another (i.e., nested) call
	 * to RunRoutine(). The call_block structure array and stack_depth variable are the
	 * navigation guides.
	 */
	void RunRoutine(long addr);

	int RunSave();

	int RunScriptSet();

	/**
	 * As in 'x = string(<array>, "<string>"[, maxlen]'.
	 */
	int RunString();

	int RunSystem();

	void SaveWindowData(SAVED_WINDOW_DATA *spw);

	void RestoreWindowData(SAVED_WINDOW_DATA *spw);

	void RunWindow();

	/**@}*/

	/**
	 * \defgroup heglk
	 * @{
	 */

	/**
	 * If gotvalue is passed as -1, then no value has already been as the (potential) object, etc.
	 * comprising the first part of the object.property, for example, to be set.
	 */
	void RunSet(int gotvalue);

	unsigned int GetAnonymousFunction(long addr);

	int SetCompound(int t);

	/**@}*/

	/**
	* \defgroup Miscellaneous
	* @{
	*/

	int hugo_fseek(Common::SeekableReadStream *s, long int offset, int whence) {
		return !s->seek(offset, whence);
	}
	int hugo_fseek(strid_t s, long int offset, int whence) {
		Common::SeekableReadStream *rs = *s;
		return hugo_fseek(rs, offset, whence);
	}

	int hugo_fgetc(Common::SeekableReadStream *s) {
		return s->readByte();
	}
	int hugo_fgetc(strid_t s) {
		Common::SeekableReadStream *ws = *s;
		return hugo_fgetc(ws);
	}

	int hugo_fputc(int c, Common::WriteStream *s) {
		s->writeByte(c);
		return s->err() ? EOF : 0;
	}
	int hugo_fputc(int c, strid_t s) {
		Common::WriteStream *ws = *s;
		return hugo_fputc(c, ws);
	}

	char *hugo_fgets(char *buf, int max, Common::SeekableReadStream *s) {
		char *ptr = buf;
		char c;
		while (s->pos() < s->size() && --max > 0) {
			c = hugo_fgetc(s);
			if (c == '\n' || c == '\0')
				break;
			*ptr++ = c;
		}
		*ptr++ = '\0';
		return buf;
	}
	char *hugo_fgets(char *buf, int max, strid_t s) {
		Common::SeekableReadStream *rs = *s;
		return hugo_fgets(buf, max, rs);
	}

	size_t hugo_fread(void *ptr, size_t size, size_t count, Common::SeekableReadStream *s) {
		return s->read(ptr, size * count);
	}
	size_t hugo_fread(void *ptr, size_t size, size_t count, strid_t s) {
		Common::SeekableReadStream *rs = *s;
		return hugo_fread(ptr, size, count, rs);
	}

	int hugo_fprintf(Common::WriteStream *s, const char *fmt, ...) {
		va_list va;
		va_start(va, fmt);
		Common::String text = Common::String::vformat(fmt, va);
		va_end(va);

		s->write(text.c_str(), text.size());
		return s->err() ? -1 : 0;
	}
	int hugo_fprintf(strid_t s, const char *fmt, ...) {
		va_list va;
		va_start(va, fmt);
		Common::String text = Common::String::vformat(fmt, va);
		va_end(va);

		Common::WriteStream *str = *s;
		str->write(text.c_str(), text.size());
		return str->err() ? -1 : 0;
	}

	int hugo_fputs(const char *str, Common::WriteStream *s) {
		return s->write(str, strlen(str)) == strlen(str) ? 0 : -1;
	}
	int hugo_fputs(const char *str, strid_t s) {
		Common::WriteStream *ws = *s;
		return hugo_fputs(str, ws);
	}

	bool hugo_ferror(Common::SeekableReadStream *s) const {
		return s->err();
	}
	bool hugo_ferror(strid_t s) const {
		Common::SeekableReadStream *rs = *s;
		return hugo_ferror(rs);
	}

	long hugo_ftell(Common::SeekableReadStream *s) {
		return s->pos();
	}
	long hugo_ftell(strid_t s) {
		Common::SeekableReadStream *rs = *s;
		return hugo_ftell(rs);
	}

	int hugo_fclose(strid_t f) {
		delete f;
		return 0;
	}

	void hugo_exit(const char *msg) {
		error("%s", line);
	}

	uint hugo_rand() {
		return _random.getRandomNumber(0xffffff);
	}

	char *itoa(int value, char *str, int base) {
		assert(base == 10);
		sprintf(str, "%d", value);
		return str;
	}

	/**@}*/
private:
	/**
	 * Allocate memory block
	 */
	void *hugo_blockalloc(size_t num) { return malloc(num); }

	void hugo_blockfree(void *block) { free(block); }

#if defined (DEBUGGER)
	int CheckinRange(uint v1, uint v2, const char *v3) { return 1; }

	/**
	* Shorthand since many of these object functions may call CheckinRange() if the debugger
	* is running and runtime_warnings is set.
	*/
	int CheckObjectRange(int obj);

	void DebugRunRoutine(long addr) { RunRoutine(addr); }

	void RuntimeWarning(const char *msg) {}

	void DebugMessageBox(const char *title, const char *msg) {}

	bool IsBreakpoint(long loc) const { return false; }

	const char *RoutineName(long loc) { return "Routine"; }

	void AddStringtoCodeWindow(const char *str) {}

	void SwitchtoDebugger() {}

	void Debugger() {}

	void UpdateDebugScreen() {}

	void SwitchtoGame() {}

	void DebuggerFatal(DEBUGGER_ERROR err) { error("Debugger error"); }

	void AddLinetoCodeWindow(int lineNum) {}

	void RecoverLastGood() {}

	void SetupWatchEval(int num) {}

	bool EvalWatch() { return false; }
#endif
public:
	/**
	 * Constructor
	 */
	Hugo(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Destructor
	 */
	~Hugo();

	/**
	 * Run the game
	 */
	void runGame() override;

	/**
	 * Returns the running interpreter type
	 */
	InterpreterType getInterpreterType() const override { return INTERPRETER_HUGO; }

	/**
	 * Load a savegame from the passed Quetzal file chunk stream
	 */
	Common::Error readSaveData(Common::SeekableReadStream *rs) override;

	/**
	 * Save the game. The passed write stream represents access to the UMem chunk
	 * in the Quetzal save file that will be created
	 */
	Common::Error writeGameData(Common::WriteStream *ws) override;
};

extern Hugo *g_vm;

} // End of namespace Hugo
} // End of namespace Glk

#endif
