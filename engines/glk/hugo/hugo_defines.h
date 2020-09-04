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

#ifndef GLK_HUGO_DEFINES
#define GLK_HUGO_DEFINES

#include "common/scummsys.h"

namespace Glk {
namespace Hugo {

#define HEVERSION 3
#define HEREVISION 3
#define HEINTERIM ".0"

#define GLK
#define GRAPHICS_SUPPORTED
#define SOUND_SUPPORTED
#define SETTITLE_SUPPORTED
#define SAVEGAMEDATA_REPLACED
#define RESTOREGAMEDATA_REPLACED

// There's a bunch of debugging code in the original Hugo sources behind DEBUGGER defines,
// but doesn't actually have any implementation of them. I've put in some stub methods,
// with the idea that debugger code could eventually be hooked up to the ScummVM debugger.
// So for now the debugger defined is commented out, since with debugger enabled the games
// don't work properly
//#define DEBUGGER 1


#define MAXOBJLIST 32
#define MAX_CONTEXT_COMMANDS	32
#define MAX_EVAL_ELEMENTS 256
#define MAX_GAME_TITLE 64
#define MAX_DEBUG_LINE 256
#define MAX_OBJECT 999
#define MAX_PROPERTY 999
// maximum number of matchable object words
#define MAX_MOBJ 16
// Larger than normal since Glk doesn't break up paragraphs (1024+256)
#define MAXBUFFER 1280
#define MAXUNDO 1024
#define MAXCALLS 99
#define MAXBREAKPOINTS 99
#define MAX_CODE_HISTORY 99
#define MAX_RES_PATH 255
#define MAXRES 1024
#define CHARWIDTH 1

#define HUGO_FILE	strid_t
#define MAXPATH         256
#define MAXFILENAME     256
#define MAXDRIVE        256
#define MAXDIR          256
#define MAXEXT          256

#define DEF_PRN         ""
#define DEF_FCOLOR      0
#define DEF_BGCOLOR     15
#define DEF_SLFCOLOR	15
#define DEF_SLBGCOLOR	1

/* These static values are not changeable--they depend largely on internals of the Engine. */
#define MAXATTRIBUTES    128
#define MAXGLOBALS       240
#define MAXLOCALS         16
#define MAXPOBJECTS      256    /* contenders for disambiguation */
#define MAXWORDS          32    /* in an input line              */
#define MAXSTACKDEPTH    256	/* for nesting {...}		 */


/* The positions of various data in the header: */
#define H_GAMEVERSION	0x00
#define H_ID		0x01
#define H_SERIAL	0x03
#define H_CODESTART	0x0B

#define H_OBJTABLE	0x0D           /* data tables */
#define H_PROPTABLE	0x0F
#define H_EVENTTABLE	0x11
#define H_ARRAYTABLE	0x13
#define H_DICTTABLE	0x15
#define H_SYNTABLE	0x17

#define H_INIT		0x19           /* junction routines */
#define H_MAIN		0x1B
#define H_PARSE		0x1D
#define H_PARSEERROR	0x1F
#define H_FINDOBJECT	0x21
#define H_ENDGAME	0x23
#define H_SPEAKTO	0x25
#define H_PERFORM	0x27

#define H_TEXTBANK	0x29

/* additional debugger header information */
#define H_DEBUGGABLE     0x3A
#define H_DEBUGDATA      0x3B
#define H_DEBUGWORKSPACE 0x3E

/* Printing control codes--embedded in strings printed by AP(). */
#define FONT_CHANGE       1
#define COLOR_CHANGE      2
#define NO_CONTROLCHAR    3
#define NO_NEWLINE        30
#define FORCED_SPACE      31	/* Can't be <= # colors/font codes + 1
(See AP() for the reason) */

/* Font control codes--these bitmasks follow FONT_CHANGE codes. */
#define NORMAL_FONT	  0
#define BOLD_FONT         1
#define ITALIC_FONT       2
#define UNDERLINE_FONT    4
#define PROP_FONT         8

/* CHAR_TRANSLATION is simply a value that is added to an ASCII character
in order to encode the text, i.e., make it unreadable to casual
browsing.
*/
#define CHAR_TRANSLATION  0x14

/* Passed to GetWord() */
#define PARSE_STRING_VAL  0xFFF0
#define SERIAL_STRING_VAL 0xFFF1

/* Returned by FindWord() */
#define UNKNOWN_WORD      0xFFFF

/* Bitmasks for certain qualities of properties */
#define ADDITIVE_FLAG   1
#define COMPLEX_FLAG    2

/* Property-table indicators */
#define PROP_END          255
#define PROP_ROUTINE      255

#define MEM(addr) (mem[addr])
#define SETMEM(addr, n) (mem[addr] = n)
#define GETMEMADDR(addr) (&mem[addr])
#define HUGO_PTR

#define RESET_STACK_DEPTH (-1)

#define RUNROUTINE_BLOCK  1
#define CONDITIONAL_BLOCK 2
#define DOWHILE_BLOCK     3

#define TAIL_RECURSION_ROUTINE          (-1)
#define TAIL_RECURSION_PROPERTY         (-2)

/* For system_status: */
#define STAT_UNAVAILABLE	((short)-1)
#define STAT_NOFILE 		101
#define STAT_NORESOURCE		102
#define STAT_LOADERROR		103

#define PRINTFATALERROR(a)	error("%s", a)

#define PIC 0
#define SND 1

#if defined (DEBUGGER)
#define VIEW_CALLS 0
#define VIEW_LOCALS 1
#define CODE_WINDOW 2
#define VIEW_BREAKPOINTS 3
#define VIEW_WATCH 4

#define FORCE_REDRAW 1

#endif

} // End of namespace Hugo
} // End of namespace Glk

#endif
