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

#include "glk/agt/agility.h"
#include "glk/agt/interp.h"
#include "glk/agt/agt.h"

namespace Glk {
namespace AGT {

/*
 * Glk interface for AGiliTy 1.1.1.1
 * -------------------------------
 *
 * This module contains the the Glk porting layer for AGiliTy.  It
 * defines the Glk arguments list structure, the entry points for the
 * Glk library framework to use, and all platform-abstracted I/O to
 * link to Glk's I/O.
 *
 * The following items are omitted from this Glk port:
 *
 *  o Calls to g_vm->glk_tick().  The Glk documentation states that the
 *    interpreter should call g_vm->glk_tick() every opcode or so.  This is
 *    intrusive to code (it goes outside of this module), and since
 *    most Glk libraries do precisely nothing in g_vm->glk_tick(), there is
 *    little motivation to add it.
 *
 *  o Glk tries to assert control over _all_ file I/O.  It's just too
 *    disruptive to add it to existing code, so for now, the AGiliTy
 *    interpreter is still dependent on stdio and the like.
 */

/*
 * True and false definitions -- usually defined in glkstart.h, but we need
 * them early, so we'll define them here too.  We also need NULL, but that's
 * normally from stdio.h or one of it's cousins.
 */
#ifndef FALSE
# define FALSE 0
#endif
#ifndef TRUE
# define TRUE (!FALSE)
#endif


/*---------------------------------------------------------------------*/
/*  Module variables, miscellaneous externals not in header files      */
/*---------------------------------------------------------------------*/

/* Glk AGiliTy port version number. */
static const glui32 GAGT_PORT_VERSION = 0x00010701;

/* Forward declaration of event wait functions. */
static void gagt_event_wait(glui32 wait_type, event_t *event);
static void gagt_event_wait_2(glui32 wait_type_1,
                              glui32 wait_type_2,
                              event_t *event);

/*
 * Forward declaration of the g_vm->glk_exit() wrapper.  Normal functions in this
 * module should not to call g_vm->glk_exit() directly; they should always call it
 * through the wrapper instead.
 */
static void gagt_exit();


/*---------------------------------------------------------------------*/
/*  Glk port utility functions                                         */
/*---------------------------------------------------------------------*/

/*
 * gagt_fatal()
 *
 * Fatal error handler.  The function returns, expecting the caller to
 * abort() or otherwise handle the error.
 */
static void gagt_fatal(const char *string) {
	/*
	 * If the failure happens too early for us to have a window, print
	 * the message to stderr.
	 */
	if (!g_vm->gagt_main_window)
		error("INTERNAL ERROR: %s", string);

	/* Cancel all possible pending window input events. */
	g_vm->glk_cancel_line_event(g_vm->gagt_main_window, NULL);
	g_vm->glk_cancel_char_event(g_vm->gagt_main_window);

	/* Print a message indicating the error. */
	g_vm->glk_set_window(g_vm->gagt_main_window);
	g_vm->glk_set_style(style_Normal);
	g_vm->glk_put_string("\n\nINTERNAL ERROR: ");
	g_vm->glk_put_string(string);

	g_vm->glk_put_string("\n\nPlease record the details of this error, try to"
	                     " note down everything you did to cause it, and email"
	                     " this information to simon_baldwin@yahoo.com.\n\n");
}


/*
 * gagt_malloc()
 * gagt_realloc()
 *
 * Non-failing malloc and realloc; call gagt_fatal() and exit if memory
 * allocation fails.
 */
static void *gagt_malloc(size_t size) {
	void *pointer;

	pointer = malloc(size);
	if (!pointer) {
		gagt_fatal("GLK: Out of system memory");
		gagt_exit();
	}

	return pointer;
}

static void *gagt_realloc(void *ptr, size_t size) {
	void *pointer;

	pointer = realloc(ptr, size);
	if (!pointer) {
		gagt_fatal("GLK: Out of system memory");
		gagt_exit();
	}

	return pointer;
}


/*
 * gagt_strncasecmp()
 * gagt_strcasecmp()
 *
 * Strncasecmp and strcasecmp are not ANSI functions, so here are local
 * definitions to do the same jobs.
 */
static int gagt_strncasecmp(const char *s1, const char *s2, size_t n) {
	size_t index;

	for (index = 0; index < n; index++) {
		int diff;

		diff = g_vm->glk_char_to_lower(s1[index]) - g_vm->glk_char_to_lower(s2[index]);
		if (diff < 0 || diff > 0)
			return diff < 0 ? -1 : 1;
	}

	return 0;
}

static int gagt_strcasecmp(const char *s1, const char *s2) {
	size_t s1len, s2len;
	int result;

	s1len = strlen(s1);
	s2len = strlen(s2);

	result = gagt_strncasecmp(s1, s2, s1len < s2len ? s1len : s2len);
	if (result < 0 || result > 0)
		return result;
	else
		return s1len < s2len ? -1 : s1len > s2len ? 1 : 0;
}


/*
 * gagt_debug()
 *
 * Handler for module debug output.  If no debug, it ignores the call,
 * otherwise it prints a debug message, prefixed by the function name.
 */
static void gagt_debug(const char *function, const char *format, ...) {
	if (DEBUG_OUT) {
		Common::WriteStream *ws = dynamic_cast<Common::WriteStream *>(debugfile);
		assert(ws);

		ws->writeString(Common::String::format("%s (", function));
		if (format && strlen(format) > 0) {
			va_list va;

			va_start(va, format);
			Common::String data = Common::String::vformat(format, va);
			ws->writeString(data);
			va_end(va);
		}

		ws->writeString(")\n");
	}
}


/*---------------------------------------------------------------------*/
/*  Functions not ported - functionally unchanged from os_none.c       */
/*---------------------------------------------------------------------*/

/*
 * agt_tone()
 *
 * Produce a hz-Hertz sound for ms milliseconds.
 */
void agt_tone(int hz, int ms) {
	gagt_debug("agt_tone", "hz=%d, ms=%d", hz, ms);
}


/*
 * agt_rand()
 *
 * Return random number from a to b inclusive.  The random number generator
 * is seeded on the first call, to a reproducible sequence if stable_random,
 * otherwise using time().
 */
int agt_rand(int a, int b) {
	int result;

	result = a + g_vm->getRandomNumber(0x7fffffff) % (b - a + 1);
	gagt_debug("agt_rand", "a=%d, b=%d -> %d", a, b, result);
	return result;
}


/*---------------------------------------------------------------------*/
/*  Workrounds for bugs in core AGiliTy.                               */
/*---------------------------------------------------------------------*/

/*
 * gagt_workround_menus()
 *
 * Somewhere in AGiliTy's menu handling stuff is a condition that sets up
 * an eventual NULL dereference in rstrncpy(), called from num_name_func().
 * For some reason, perhaps memory overruns, perhaps something else, it
 * happens after a few turns have been made through agt_menu().  Replacing
 * agt_menu() won't avoid it.
 *
 * However, the menu stuff isn't too useful, or attractive, in a game, so one
 * solution is to simply disable it.  While not possible to do this directly,
 * there is a sneaky way, using our carnal knowledge of core AGiliTy.  In
 * runverb.c, there is code to prevent menu mode from being turned on where
 * verbmenu is NULL.  Verbmenu is set up in agil.c on loading the game, but,
 * crucially, is set up before agil.c calls start_interface().  So... here
 * we can free it, set it to NULL, set menu_mode to 0 (it probably is already)
 * and AGiliTy behaves as if the game prevents menu mode.
 */
static void gagt_workround_menus() {
	free(verbmenu);
	verbmenu = NULL;

	menu_mode = 0;
}


/*
 * gagt_workround_fileexist()
 *
 * This function verifies that the game file can be opened, in effect second-
 * guessing run_game().
 *
 * AGiliTy's fileexist() has in it either a bug, or a misfeature.  It always
 * passes a nofix value of 1 into try_open_file(), which defeats the code to
 * retry with both upper and lower cased filenames.  So here we have to go
 * round the houses, with readopen()/readclose().
 */
static int gagt_workround_fileexist(fc_type fc, filetype ft) {
	genfile file;
	const char *errstr;

	errstr = NULL;
	file = readopen(fc, ft, &errstr);

	if (file) {
		readclose(file);
		return TRUE;
	}
	return FALSE;
}


/*---------------------------------------------------------------------*/
/*  I/O interface start and stop functions.                            */
/*---------------------------------------------------------------------*/

/* AGiliTy font_status values that indicate what font may be used. */
enum {
	GAGT_FIXED_REQUIRED = 1, GAGT_PROPORTIONAL_OKAY = 2
};


/*
 * start_interface()
 * close_interface()
 *
 * Startup and shutdown callout points.  The start function for Glk looks
 * at the value of font_status that the game sets, to see if it has a strong
 * view of the font to use.  If it does, then we'll reflect that in the
 * module's font contol, perhaps overriding any command line options that the
 * user has passed in.
 */
void start_interface(fc_type fc) {
	switch (font_status) {
	case GAGT_FIXED_REQUIRED:
		g_vm->gagt_font_mode = FONT_FIXED_WIDTH;
		break;

	case GAGT_PROPORTIONAL_OKAY:
		g_vm->gagt_font_mode = FONT_PROPORTIONAL;
		break;

	default:
		break;
	}

	gagt_workround_menus();

	gagt_debug("start_interface", "fc=%p", fc);
}

void close_interface() {
	if (filevalid(scriptfile, fSCR))
		close_pfile(scriptfile, 0);

	gagt_debug("close_interface", "");
}


/*---------------------------------------------------------------------*/
/*  Code page 437 to ISO 8859 Latin-1 translations                     */
/*---------------------------------------------------------------------*/

/*
 * AGiliTy uses IBM code page 437 characters, and Glk works in ISO 8859
 * Latin-1.  There's some good news, in that a number of the characters,
 * especially international ones, in these two sets are the same.  The bad
 * news is that, for codes above 127 (that is, beyond 7-bit ASCII), or for
 * codes below 32, they live in different places.  So, here is a table of
 * conversions for codes not equivalent to 7-bit ASCII, and a pair of
 * conversion routines.
 *
 * Note that some code page 437 characters don't have ISO 8859 Latin-1
 * equivalents.  Predominantly, these are the box-drawing characters, which
 * is a pity, because these are the ones that are used the most.  Anyway,
 * in these cases, the table substitutes an approximated base ASCII char-
 * acter in its place.
 *
 * The first entry of table comments below is the character's UNICODE value,
 * just in case it's useful at some future date.
 */
typedef const struct {
	const unsigned char cp437;      /* Code page 437 character. */
	const unsigned char iso8859_1;  /* ISO 8859 Latin-1 character. */
} gagt_char_t;
typedef gagt_char_t *gagt_charref_t;

static gagt_char_t GAGT_CHAR_TABLE[] = {
	/*
	 * Low characters -- those below 0x20.   These are the really odd code
	 * page 437 characters, rarely used by AGT games.  Low characters are
	 * omitted from the reverse lookup, and participate only in the forwards
	 * lookup from code page 437 to ISO 8859 Latin-1.
	 */
	{0x01,  '@'},     /* 263a White smiling face */
	{0x02,  '@'},     /* 263b Black smiling face */
	{0x03,  '?'},     /* 2665 Black heart suit */
	{0x04,  '?'},     /* 2666 Black diamond suit */
	{0x05,  '?'},     /* 2663 Black club suit */
	{0x06,  '?'},     /* 2660 Black spade suit */
	{0x07, 0xb7},     /* 2022 Bullet */
	{0x08, 0xb7},     /* 25d8 Inverse bullet */
	{0x09, 0xb7},     /* 25e6 White bullet */
	{0x0a, 0xb7},     /* 25d9 Inverse white circle */
	{0x0b,  '?'},     /* 2642 Male sign */
	{0x0c,  '?'},     /* 2640 Female sign */
	{0x0d,  '?'},     /* 266a Eighth note */
	{0x0e,  '?'},     /* 266b Beamed eighth notes */
	{0x0f, 0xa4},     /* 263c White sun with rays */
	{0x10,  '>'},     /* 25b6 Black right-pointing triangle */
	{0x11,  '<'},     /* 25c0 Black left-pointing triangle */
	{0x12, 0xa6},     /* 2195 Up down arrow */
	{0x13,  '!'},     /* 203c Double exclamation mark */
	{0x14, 0xb6},     /* 00b6 Pilcrow sign */
	{0x15, 0xa7},     /* 00a7 Section sign */
	{0x16,  '#'},     /* 25ac Black rectangle */
	{0x17, 0xa6},     /* 21a8 Up down arrow with base */
	{0x18,  '^'},     /* 2191 Upwards arrow */
	{0x19,  'v'},     /* 2193 Downwards arrow */
	{0x1a,  '>'},     /* 2192 Rightwards arrow */
	{0x1b,  '<'},     /* 2190 Leftwards arrow */
	{0x1c,  '?'},     /* 2310 Reversed not sign */
	{0x1d,  '-'},     /* 2194 Left right arrow */
	{0x1e,  '^'},     /* 25b2 Black up-pointing triangle */
	{0x1f,  'v'},     /* 25bc Black down-pointing triangle */

	/*
	 * High characters -- those above 0x7f.  These are more often used by AGT
	 * games, particularly for box drawing.
	 */
	{0x80, 0xc7},     /* 00c7 Latin capital letter c with cedilla */
	{0x81, 0xfc},     /* 00fc Latin small letter u with diaeresis */
	{0x82, 0xe9},     /* 00e9 Latin small letter e with acute */
	{0x83, 0xe2},     /* 00e2 Latin small letter a with circumflex */
	{0x84, 0xe4},     /* 00e4 Latin small letter a with diaeresis */
	{0x85, 0xe0},     /* 00e0 Latin small letter a with grave */
	{0x86, 0xe5},     /* 00e5 Latin small letter a with ring above */
	{0x87, 0xe7},     /* 00e7 Latin small letter c with cedilla */
	{0x88, 0xea},     /* 00ea Latin small letter e with circumflex */
	{0x89, 0xeb},     /* 00eb Latin small letter e with diaeresis */
	{0x8a, 0xe8},     /* 00e8 Latin small letter e with grave */
	{0x8b, 0xef},     /* 00ef Latin small letter i with diaeresis */
	{0x8c, 0xee},     /* 00ee Latin small letter i with circumflex */
	{0x8d, 0xec},     /* 00ec Latin small letter i with grave */
	{0x8e, 0xc4},     /* 00c4 Latin capital letter a with diaeresis */
	{0x8f, 0xc5},     /* 00c5 Latin capital letter a with ring above */
	{0x90, 0xc9},     /* 00c9 Latin capital letter e with acute */
	{0x91, 0xe6},     /* 00e6 Latin small ligature ae */
	{0x92, 0xc6},     /* 00c6 Latin capital ligature ae */
	{0x93, 0xf4},     /* 00f4 Latin small letter o with circumflex */
	{0x94, 0xf6},     /* 00f6 Latin small letter o with diaeresis */
	{0x95, 0xf2},     /* 00f2 Latin small letter o with grave */
	{0x96, 0xfb},     /* 00fb Latin small letter u with circumflex */
	{0x97, 0xf9},     /* 00f9 Latin small letter u with grave */
	{0x98, 0xff},     /* 00ff Latin small letter y with diaeresis */
	{0x99, 0xd6},     /* 00d6 Latin capital letter o with diaeresis */
	{0x9a, 0xdc},     /* 00dc Latin capital letter u with diaeresis */
	{0x9b, 0xa2},     /* 00a2 Cent sign */
	{0x9c, 0xa3},     /* 00a3 Pound sign */
	{0x9d, 0xa5},     /* 00a5 Yen sign */
	{0x9e,  'p'},     /* 20a7 Peseta sign */
	{0x9f,  'f'},     /* 0192 Latin small letter f with hook */
	{0xa0, 0xe1},     /* 00e1 Latin small letter a with acute */
	{0xa1, 0xed},     /* 00ed Latin small letter i with acute */
	{0xa2, 0xf3},     /* 00f3 Latin small letter o with acute */
	{0xa3, 0xfa},     /* 00fa Latin small letter u with acute */
	{0xa4, 0xf1},     /* 00f1 Latin small letter n with tilde */
	{0xa5, 0xd1},     /* 00d1 Latin capital letter n with tilde */
	{0xa6, 0xaa},     /* 00aa Feminine ordinal indicator */
	{0xa7, 0xba},     /* 00ba Masculine ordinal indicator */
	{0xa8, 0xbf},     /* 00bf Inverted question mark */
	{0xa9,  '.'},     /* 2310 Reversed not sign */
	{0xaa, 0xac},     /* 00ac Not sign */
	{0xab, 0xbd},     /* 00bd Vulgar fraction one half */
	{0xac, 0xbc},     /* 00bc Vulgar fraction one quarter */
	{0xad, 0xa1},     /* 00a1 Inverted exclamation mark */
	{0xae, 0xab},     /* 00ab Left-pointing double angle quotation mark */
	{0xaf, 0xbb},     /* 00bb Right-pointing double angle quotation mark */
	{0xb0,  '#'},     /* 2591 Light shade */
	{0xb1,  '#'},     /* 2592 Medium shade */
	{0xb2,  '#'},     /* 2593 Dark shade */
	{0xb3,  '|'},     /* 2502 Box light vertical */
	{0xb4,  '+'},     /* 2524 Box light vertical and left */
	{0xb5,  '+'},     /* 2561 Box vertical single and left double */
	{0xb6,  '|'},     /* 2562 Box vertical double and left single */
	{0xb7,  '+'},     /* 2556 Box down double and left single */
	{0xb8,  '+'},     /* 2555 Box down single and left double */
	{0xb9,  '+'},     /* 2563 Box double vertical and left */
	{0xba,  '|'},     /* 2551 Box double vertical */
	{0xbb, '\\'},     /* 2557 Box double down and left */
	{0xbc,  '/'},     /* 255d Box double up and left */
	{0xbd,  '+'},     /* 255c Box up double and left single */
	{0xbe,  '+'},     /* 255b Box up single and left double */
	{0xbf, '\\'},     /* 2510 Box light down and left */
	{0xc0, '\\'},     /* 2514 Box light up and right */
	{0xc1,  '+'},     /* 2534 Box light up and horizontal */
	{0xc2,  '+'},     /* 252c Box light down and horizontal */
	{0xc3,  '+'},     /* 251c Box light vertical and right */
	{0xc4,  '-'},     /* 2500 Box light horizontal */
	{0xc5,  '+'},     /* 253c Box light vertical and horizontal */
	{0xc6,  '|'},     /* 255e Box vertical single and right double */
	{0xc7,  '|'},     /* 255f Box vertical double and right single */
	{0xc8, '\\'},     /* 255a Box double up and right */
	{0xc9,  '/'},     /* 2554 Box double down and right */
	{0xca,  '+'},     /* 2569 Box double up and horizontal */
	{0xcb,  '+'},     /* 2566 Box double down and horizontal */
	{0xcc,  '+'},     /* 2560 Box double vertical and right */
	{0xcd,  '='},     /* 2550 Box double horizontal */
	{0xce,  '+'},     /* 256c Box double vertical and horizontal */
	{0xcf,  '='},     /* 2567 Box up single and horizontal double */
	{0xd0,  '+'},     /* 2568 Box up double and horizontal single */
	{0xd1,  '='},     /* 2564 Box down single and horizontal double */
	{0xd2,  '+'},     /* 2565 Box down double and horizontal single */
	{0xd3,  '+'},     /* 2559 Box up double and right single */
	{0xd4,  '+'},     /* 2558 Box up single and right double */
	{0xd5,  '+'},     /* 2552 Box down single and right double */
	{0xd6,  '+'},     /* 2553 Box down double and right single */
	{0xd7,  '+'},     /* 256b Box vertical double and horizontal single */
	{0xd8,  '+'},     /* 256a Box vertical single and horizontal double */
	{0xd9,  '/'},     /* 2518 Box light up and left */
	{0xda,  '/'},     /* 250c Box light down and right */
	{0xdb,  '@'},     /* 2588 Full block */
	{0xdc,  '@'},     /* 2584 Lower half block */
	{0xdd,  '@'},     /* 258c Left half block */
	{0xde,  '@'},     /* 2590 Right half block */
	{0xdf,  '@'},     /* 2580 Upper half block */
	{0xe0,  'a'},     /* 03b1 Greek small letter alpha */
	{0xe1, 0xdf},     /* 00df Latin small letter sharp s */
	{0xe2,  'G'},     /* 0393 Greek capital letter gamma */
	{0xe3,  'p'},     /* 03c0 Greek small letter pi */
	{0xe4,  'S'},     /* 03a3 Greek capital letter sigma */
	{0xe5,  's'},     /* 03c3 Greek small letter sigma */
	{0xe6, 0xb5},     /* 00b5 Micro sign */
	{0xe7,  't'},     /* 03c4 Greek small letter tau */
	{0xe8,  'F'},     /* 03a6 Greek capital letter phi */
	{0xe9,  'T'},     /* 0398 Greek capital letter theta */
	{0xea,  'O'},     /* 03a9 Greek capital letter omega */
	{0xeb,  'd'},     /* 03b4 Greek small letter delta */
	{0xec,  '.'},     /* 221e Infinity */
	{0xed,  'f'},     /* 03c6 Greek small letter phi */
	{0xee,  'e'},     /* 03b5 Greek small letter epsilon */
	{0xef,  '^'},     /* 2229 Intersection */
	{0xf0,  '='},     /* 2261 Identical to */
	{0xf1, 0xb1},     /* 00b1 Plus-minus sign */
	{0xf2,  '>'},     /* 2265 Greater-than or equal to */
	{0xf3,  '<'},     /* 2264 Less-than or equal to */
	{0xf4,  'f'},     /* 2320 Top half integral */
	{0xf5,  'j'},     /* 2321 Bottom half integral */
	{0xf6, 0xf7},     /* 00f7 Division sign */
	{0xf7,  '='},     /* 2248 Almost equal to */
	{0xf8, 0xb0},     /* 00b0 Degree sign */
	{0xf9, 0xb7},     /* 2219 Bullet operator */
	{0xfa, 0xb7},     /* 00b7 Middle dot */
	{0xfb,  '/'},     /* 221a Square root */
	{0xfc,  'n'},     /* 207f Superscript latin small letter n */
	{0xfd, 0xb2},     /* 00b2 Superscript two */
	{0xfe,  '#'},     /* 25a0 Black square */
	{0xff, 0xa0},     /* 00a0 No-break space */
	{0, 0}            /* 0000 [END OF TABLE] */
};


/*
 * gagt_cp_to_iso()
 *
 * Convert a string from code page 437 into ISO 8859 Latin-1.  The input and
 * output buffers may be one and the same.
 */
static void gagt_cp_to_iso(const unsigned char *from_string, unsigned char *to_string) {
	static int is_initialized = FALSE;
	static unsigned char table[BYTE_MAX_VAL + 1];

	int index;
	unsigned char cp437, iso8859_1;
	assert(from_string && to_string);

	if (!is_initialized) {
		gagt_charref_t entry;

		/*
		 * Create a lookup entry for each code in the main table.  Fill in gaps
		 * for 7-bit characters with their ASCII equivalent values.  Any
		 * remaining codes not represented in the main table will map to zeroes
		 * in the lookup table, as static variables are initialized to zero.
		 */
		for (entry = GAGT_CHAR_TABLE; entry->cp437; entry++) {
			cp437 = entry->cp437;
			iso8859_1 = entry->iso8859_1;

//			assert(cp437 < 0x20 || (cp437 > INT8_MAX_VAL && cp437 <= BYTE_MAX_VAL));
			table[cp437] = iso8859_1;
		}
		for (index = 0; index <= INT8_MAX_VAL; index++) {
			if (table[index] == 0)
				table[index] = index;
		}

		is_initialized = TRUE;
	}

	for (index = 0; from_string[index] != '\0'; index++) {
		cp437 = from_string[index];
		iso8859_1 = table[cp437];

		to_string[index] = iso8859_1 ? iso8859_1 : cp437;
	}

	to_string[index] = '\0';
}


/*
 * gagt_iso_to_cp()
 *
 * Convert a string from ISO 8859 Latin-1 to code page 437.  The input and
 * output buffers may be one and the same.
 */
static void gagt_iso_to_cp(const unsigned char *from_string, unsigned char *to_string) {
	static int is_initialized = FALSE;
	static unsigned char table[BYTE_MAX_VAL + 1];

	int index;
	unsigned char iso8859_1, cp437;
	assert(from_string && to_string);

	if (!is_initialized) {
		gagt_charref_t entry;

		/*
		 * Create a reverse lookup entry for each code in the main table,
		 * overriding all of the low table entries (that is, anything under
		 * 128) with their ASCII no matter what the table contained.
		 *
		 * Any codes not represented in the main table will map to zeroes in
		 * the reverse lookup table, since static variables are initialized to
		 * zero.  The first 128 characters are equivalent to ASCII.  Moreover,
		 * some ISO 8859 Latin-1 entries are faked as base ASCII; where an
		 * entry is already occupied, the main table entry is skipped, so the
		 * match, which is n:1 in the reverse direction, works in first-found
		 * mode.
		 */
		for (entry = GAGT_CHAR_TABLE; entry->iso8859_1; entry++) {
			cp437 = entry->cp437;
			iso8859_1 = entry->iso8859_1;

			if (table[iso8859_1] == 0)
				table[iso8859_1] = cp437;
		}
		for (index = 0; index <= INT8_MAX_VAL; index++)
			table[index] = index;

		is_initialized = TRUE;
	}

	for (index = 0; from_string[index] != '\0'; index++) {
		iso8859_1 = from_string[index];
		cp437 = table[iso8859_1];

		to_string[index] = cp437 ? cp437 : iso8859_1;
	}

	to_string[index] = '\0';
}


/*---------------------------------------------------------------------*/
/*  Glk port status line functions                                     */
/*---------------------------------------------------------------------*/

/*
 * Buffered copy of the latest status line passed in by the interpreter.
 * Buffering it means it's readily available to print for Glk libraries
 * that don't support separate windows.  We also need a copy of the last
 * status buffer printed for non-windowing Glk libraries, for comparison.
 */
static char *gagt_status_buffer = NULL,
             *gagt_status_buffer_printed = NULL;

/*
 * Indication that we are in mid-delay.  The delay is silent, and can look
 * kind of confusing, so to try to make it less so, we'll have the status
 * window show something about it.
 */
static int gagt_inside_delay = FALSE;


/*
 * agt_statline()
 *
 * This function is called from our call to print_statline().  Here we'll
 * convert the string and buffer in an allocated area for later use.
 */
void agt_statline(const char *cp_string) {
	assert(cp_string);

	free(gagt_status_buffer);
	gagt_status_buffer = (char *)gagt_malloc(strlen(cp_string) + 1);
	gagt_cp_to_iso((const unsigned char *)cp_string, (unsigned char *)gagt_status_buffer);

	gagt_debug("agt_statline", "string='%s'", cp_string);
}


/*
 * gagt_status_update_extended()
 *
 * Helper for gagt_status_update() and gagt_status_in_delay().  This function
 * displays the second line of any extended status display, giving a list of
 * exits from the compass rose, and if in an AGT delay, a waiting indicator.
 */
static void gagt_status_update_extended() {
	uint width, height;
	assert(g_vm->gagt_status_window);

	g_vm->glk_window_get_size(g_vm->gagt_status_window, &width, &height);
	if (height > 1) {
		uint32 index;
		int exit;

		/* Clear the second status line only. */
		g_vm->glk_window_move_cursor(g_vm->gagt_status_window, 0, 1);
		g_vm->glk_set_window(g_vm->gagt_status_window);
		g_vm->glk_set_style(style_User1);
		for (index = 0; index < width; index++)
			g_vm->glk_put_char(' ');

		/*
		 * Check bits in the compass rose, and print out exit names from
		 * the exitname[] array.
		 */
		g_vm->glk_window_move_cursor(g_vm->gagt_status_window, 0, 1);
		g_vm->glk_put_string("  Exits: ");
		for (exit = 0; exit < (int)sizeof(exitname) / (int)sizeof(exitname[0]); exit++) {
			if (compass_rose & (1 << exit)) {
				g_vm->glk_put_string(exitname[exit]);
				g_vm->glk_put_char(' ');
			}
		}

		/* If the delay flag is set, print a waiting indicator at the right. */
		if (gagt_inside_delay) {
			g_vm->glk_window_move_cursor(g_vm->gagt_status_window,
			                             width - strlen("Waiting... "), 1);
			g_vm->glk_put_string("Waiting... ");
		}

		g_vm->glk_set_window(g_vm->gagt_main_window);
	}
}


/*
 * gagt_status_update()
 *
 *
 * This function calls print_statline() to prompt the interpreter into calling
 * our agt_statline(), then if we have a status window, displays the status
 * string, and calls gagt_status_update_extended() if necessary to handle the
 * second status line.  If we don't see a call to our agt_statline, we output
 * a default status string.
 */
static void gagt_status_update() {
	uint width, height;
	uint32 index;
	assert(g_vm->gagt_status_window);

	g_vm->glk_window_get_size(g_vm->gagt_status_window, &width, &height);
	if (height > 0) {
		g_vm->glk_window_clear(g_vm->gagt_status_window);
		g_vm->glk_window_move_cursor(g_vm->gagt_status_window, 0, 0);
		g_vm->glk_set_window(g_vm->gagt_status_window);

		g_vm->glk_set_style(style_User1);
		for (index = 0; index < width; index++)
			g_vm->glk_put_char(' ');
		g_vm->glk_window_move_cursor(g_vm->gagt_status_window, 0, 0);

		/* Call print_statline() to refresh status line buffer contents. */
		print_statline();

		/* See if we have a buffered status line available. */
		if (gagt_status_buffer) {
			glui32 print_width;

			/*
			 * Print the basic buffered status string, truncating to the
			 * current status window width if necessary, then try adding a
			 * second line if extended status enabled.
			 */
			print_width = width < strlen(gagt_status_buffer)
			              ? width : strlen(gagt_status_buffer);
			g_vm->glk_put_buffer(gagt_status_buffer, print_width);

			if (g_vm->gagt_extended_status_enabled)
				gagt_status_update_extended();
		} else {
			/*
			 * We don't (yet) have a status line.  Perhaps we're at the
			 * very start of a game.  Print a standard message.
			 */
			g_vm->glk_put_string("Glk AGiliTy version 1.1.1.1");
		}

		g_vm->glk_set_window(g_vm->gagt_main_window);
	}
}


/*
 * gagt_status_print()
 *
 * Print the current contents of the completed status line buffer out in the
 * main window, if it has changed since the last call.  This is for non-
 * windowing Glk libraries.
 *
 * Like gagt_status_update(), this function calls print_statline() to prompt
 * the interpreter into calling our agt_statline(), then if we have a new
 * status line, it prints it.
 */
static void gagt_status_print() {
	/* Call print_statline() to refresh status line buffer contents. */
	print_statline();

	/*
	 * Do no more if there is no status line to print, or if the status
	 * line hasn't changed since last printed.
	 */
	if (!gagt_status_buffer
	        || (gagt_status_buffer_printed
	            && strcmp(gagt_status_buffer, gagt_status_buffer_printed) == 0))
		return;

	/* Set fixed width font to try to preserve status line formatting. */
	g_vm->glk_set_style(style_Preformatted);

	/*
	 * Bracket, and output the status line buffer.  We don't need to put any
	 * spacing after the opening bracket or before the closing one, because
	 * AGiliTy puts leading/trailing spaces on its status lines.
	 */
	g_vm->glk_put_string("[");
	g_vm->glk_put_string(gagt_status_buffer);
	g_vm->glk_put_string("]\n");

	/* Save the details of the printed status buffer. */
	free(gagt_status_buffer_printed);
	gagt_status_buffer_printed = (char *)gagt_malloc(strlen(gagt_status_buffer) + 1);
	strcpy(gagt_status_buffer_printed, gagt_status_buffer);
}


/*
 * gagt_status_notify()
 *
 * Front end function for updating status.  Either updates the status window
 * or prints the status line to the main window.
 *
 * Functions interested in updating the status line should call either this
 * function, or gagt_status_redraw(), and not print_statline().
 */
static void gagt_status_notify() {
	if (!BATCH_MODE) {
		if (g_vm->gagt_status_window)
			gagt_status_update();
		else
			gagt_status_print();
	}
}


/*
 * gagt_status_redraw()
 *
 * Redraw the contents of any status window with the buffered status string.
 * This function handles window sizing, and updates the interpreter with
 * status_width, so may, and should, be called on resize and arrange events.
 *
 * Functions interested in updating the status line should call either this
 * function, or gagt_status_notify(), and not print_statline().
 */
static void gagt_status_redraw() {
	if (!BATCH_MODE) {
		if (g_vm->gagt_status_window) {
			uint width, height;
			winid_t parent;

			/*
			 * Measure the status window, and update the interpreter's
			 * status_width variable.
			 */
			g_vm->glk_window_get_size(g_vm->gagt_status_window, &width, &height);
			status_width = width;

			/*
			 * Rearrange the status window, without changing its actual
			 * arrangement in any way.  This is a hack to work round
			 * incorrect window repainting in Xglk; it forces a complete
			 * repaint of affected windows on Glk window resize and
			 * arrange events, and works in part because Xglk doesn't
			 * check for actual arrangement changes in any way before
			 * invalidating its windows.  The hack should be harmless to
			 * Glk libraries other than Xglk, moreover, we're careful to
			 * activate it only on resize and arrange events.
			 */
			parent = g_vm->glk_window_get_parent(g_vm->gagt_status_window);
			g_vm->glk_window_set_arrangement(parent,
			                                 winmethod_Above | winmethod_Fixed,
			                                 height, NULL);

			gagt_status_update();
		}
	}
}


/*
 * gagt_status_in_delay()
 *
 * Tells status line functions whether the game is delaying, or not.  This
 * function updates the extended status line, if present, automatically.
 */
static void gagt_status_in_delay(int inside_delay) {
	if (!BATCH_MODE) {
		/* Save the new delay status flag. */
		gagt_inside_delay = inside_delay;

		/*
		 * Update just the second line of the status window display, if
		 * extended status is being displayed.
		 */
		if (g_vm->gagt_status_window && g_vm->gagt_extended_status_enabled)
			gagt_status_update_extended();
	}
}


/*
 * gagt_status_cleanup()
 *
 * Free memory resources allocated by status line functions.  Called on game
 * end.
 */
static void gagt_status_cleanup() {
	free(gagt_status_buffer);
	gagt_status_buffer = NULL;

	free(gagt_status_buffer_printed);
	gagt_status_buffer_printed = NULL;
}


/*---------------------------------------------------------------------*/
/*  Glk port color and text attribute handling                         */
/*---------------------------------------------------------------------*/

/*
 * AGT color and character attribute definitions.  This is the range of
 * values passed in to agt_textcolor().
 */
enum {
	AGT_BLACK = 0,
	AGT_BLUE = 1,
	AGT_GREEN = 2,
	AGT_CYAN = 3,
	AGT_RED = 4,
	AGT_MAGENTA = 5,
	AGT_BROWN = 6,
	AGT_NORMAL = 7,
	AGT_BLINKING = 8,
	AGT_WHITE = 9,
	AGT_FIXED_FONT = 10,
	AGT_VARIABLE_FONT = 11,
	AGT_EMPHASIS = -1,
	AGT_DE_EMPHASIS = -2
};

/*
 * AGiliTy colors and text attributes seem a bit confused.  Let's see if we
 * can sort them out.  Sadly, once we have, it's often not possible to
 * render the full range in all Glk's anyway.  Nevertheless...
 */
struct gagt_attrset_t {
	int color;     /* Text color. */
	int blink;     /* Text blinking flag. */
	int fixed;     /* Text fixed font flag. */
	int emphasis;  /* Text emphasized flag. */
};

/*
 * Attributes as currently set by AGiliTy.  The default values set up here
 * correspond to AGT_NORMAL.
 */
static gagt_attrset_t gagt_current_attribute_set = { AGT_WHITE, FALSE,
                                                     FALSE, FALSE
                                                   };

/*
 * An extra flag to indicate if we have coerced fixed font override.  On
 * some occasions, we need to ensure that we get fixed font no matter what
 * the game says.
 */
static int gagt_coerced_fixed = FALSE;

/*
 * Bit masks for packing colors and attributes.  Normally, I don't like
 * bit-twiddling all that much, but for packing all of the above into a
 * single byte, that's what we need.  Stuff color into the low four bits,
 * convenient since color is from 0 to 9, then use three bits for the other
 * attributes.
 */
static const unsigned char GAGT_COLOR_MASK = 0x0f,
                           GAGT_BLINK_MASK = 1 << 4,
                           GAGT_FIXED_MASK = 1 << 5,
                           GAGT_EMPHASIS_MASK = 1 << 6;

/* Forward declaration of message function. */
static void gagt_standout_string(const char *message);


/*
 * agt_textcolor()
 *
 * The AGiliTy porting guide defines the use of this function as:
 *
 *   Set text color to color #c, where the colors are as follows:
 *    0=Black, 1=Blue,    2=Green, 3=Cyan,
 *    4=Red,   5=Magenta, 6=Brown,
 *    7=Normal("White"-- which may actually be some other color)
 *       This should turn off blinking, bold, color, etc. and restore
 *       the text mode to its default appearance.
 *    8=Turn on blinking.
 *    9= *Just* White (not neccessarily "normal" and no need to turn off
 *        blinking)
 *   10=Turn on fixed pitch font.
 *   11=Turn off fixed pitch font
 *   Also used to set other text attributes:
 *     -1=emphasized text, used (e.g.) for room titles
 *     -2=end emphasized text
 *
 * Here we try to make sense of all this.  Given an argument, we'll try to
 * update our separated color and text attributes flags to reflect the
 * expected text rendering.
 */
void agt_textcolor(int color) {
	switch (color) {
	case AGT_BLACK:
	case AGT_BLUE:
	case AGT_GREEN:
	case AGT_CYAN:
	case AGT_RED:
	case AGT_MAGENTA:
	case AGT_BROWN:
	case AGT_WHITE:
		gagt_current_attribute_set.color = color;
		break;

	case AGT_NORMAL:
		gagt_current_attribute_set.color = AGT_WHITE;
		gagt_current_attribute_set.blink = FALSE;
		gagt_current_attribute_set.fixed = FALSE;
		gagt_current_attribute_set.emphasis = FALSE;
		break;

	case AGT_BLINKING:
		gagt_current_attribute_set.blink = TRUE;
		break;

	case AGT_FIXED_FONT:
		gagt_current_attribute_set.fixed = TRUE;
		break;

	case AGT_VARIABLE_FONT:
		gagt_current_attribute_set.fixed = FALSE;
		break;

	case AGT_EMPHASIS:
		gagt_current_attribute_set.emphasis = TRUE;
		break;

	case AGT_DE_EMPHASIS:
		gagt_current_attribute_set.emphasis = FALSE;
		break;

	default:
		gagt_fatal("GLK: Unknown color encountered");
		gagt_exit();
	}

	gagt_debug("agt_textcolor", "color=% d -> %d%s%s%s",
	           color,
	           gagt_current_attribute_set.color,
	           gagt_current_attribute_set.blink ? " blink" : "",
	           gagt_current_attribute_set.fixed ? " fixed" : "",
	           gagt_current_attribute_set.emphasis ? " bold" : "");
}


/*
 * gagt_coerce_fixed_font()
 *
 * This coerces, or relaxes, a fixed font setting.  Used by box drawing, to
 * ensure that we get a temporary fixed font setting for known differenti-
 * ated parts of game output text.  Pass in TRUE to coerce fixed font, and
 * FALSE to relax it.
 */
static void gagt_coerce_fixed_font(int coerce) {
	gagt_coerced_fixed = coerce;
}


/*
 * gagt_pack_attributes()
 *
 * Pack a set of color and text rendering attributes into a single byte,
 * and return it.  This function is used so that a set of text attributes
 * can be encoded into a byte array that parallels the output strings that
 * we buffer from the interpreter.
 */
static unsigned char gagt_pack_attributes(const gagt_attrset_t *attribute_set, int coerced) {
	unsigned char packed;
	assert(attribute_set);

	/* Set the initial result to be color; these are the low bits. */
	assert((attribute_set->color & ~GAGT_COLOR_MASK) == 0);
	packed = attribute_set->color;

	/*
	 * Now OR in the text attributes settings, taking either the value for
	 * fixed or the coerced fixed font.
	 */
	packed |= attribute_set->blink ? GAGT_BLINK_MASK : 0;
	packed |= attribute_set->fixed || coerced ? GAGT_FIXED_MASK : 0;
	packed |= attribute_set->emphasis ? GAGT_EMPHASIS_MASK : 0;

	return packed;
}


/*
 * gagt_unpack_attributes()
 *
 * Unpack a set of packed current color and text rendering attributes from a
 * single byte, and return the result of unpacking.  This reconstitutes the
 * text attributes that were current at the time of packing.
 */
static void gagt_unpack_attributes(unsigned char packed, gagt_attrset_t *attribute_set) {
	assert(attribute_set);

	attribute_set->color = packed & GAGT_COLOR_MASK;
	attribute_set->blink = (packed & GAGT_BLINK_MASK) != 0;
	attribute_set->fixed = (packed & GAGT_FIXED_MASK) != 0;
	attribute_set->emphasis = (packed & GAGT_EMPHASIS_MASK) != 0;
}


/*
 * gagt_pack_current_attributes()
 *
 * Pack the current color and text rendering attributes into a single byte,
 * and return it.
 */
static unsigned char gagt_pack_current_attributes() {
	return gagt_pack_attributes(&gagt_current_attribute_set, gagt_coerced_fixed);
}


/*
 * gagt_init_user_styles()
 *
 * Attempt to set up two defined styles, User1 and User2, to represent
 * fixed font with AGT emphasis (rendered as Glk subheader), and fixed font
 * with AGT blink (rendered as Glk emphasis), respectively.
 *
 * The Glk stylehints here may not actually be honored by the Glk library.
 * We'll try to detect this later on.
 */
static void gagt_init_user_styles() {
	/*
	 * Set User1 to be fixed width, bold, and not italic.  Here we're sort of
	 * assuming that the style starts life equal to Normal.
	 */
	g_vm->glk_stylehint_set(wintype_TextBuffer, style_User1,
	                        stylehint_Proportional, 0);
	g_vm->glk_stylehint_set(wintype_TextBuffer, style_User1, stylehint_Weight, 1);
	g_vm->glk_stylehint_set(wintype_TextBuffer, style_User1, stylehint_Oblique, 0);

	/*
	 * Set User2 to be fixed width, normal, and italic, with the same
	 * assumptions.
	 */
	g_vm->glk_stylehint_set(wintype_TextBuffer, style_User2,
	                        stylehint_Proportional, 0);
	g_vm->glk_stylehint_set(wintype_TextBuffer, style_User2, stylehint_Weight, 0);
	g_vm->glk_stylehint_set(wintype_TextBuffer, style_User2, stylehint_Oblique, 1);
}


/*
 * gagt_confirm_appearance()
 *
 * Attempt to find out if a Glk style's on screen appearance matches a given
 * expectation.  There's a chance (often 100% with current Xglk) that we
 * can't tell, in which case we'll play safe, and say that it doesn't (our
 * caller is hoping it does).
 *
 * That is, when we return FALSE, we mean either it's not as expected, or we
 * don't know.
 */
static int gagt_confirm_appearance(glui32 style, glui32 stylehint, glui32 expected) {
	uint result;

	if (g_vm->glk_style_measure(g_vm->gagt_main_window, style, stylehint, &result)) {
		/*
		 * Measurement succeeded, so return TRUE if the result matches the
		 * caller's expectation.
		 */
		if (result == expected)
			return TRUE;
	}

	/* No straight answer, or the style's stylehint failed to match. */
	return FALSE;
}


/*
 * gagt_is_style_fixed()
 * gagt_is_style_bold()
 * gagt_is_style_oblique()
 *
 * Convenience functions for gagt_select_style().  A return of TRUE indicates
 * that the style has this attribute; FALSE indicates either that it hasn't,
 * or that it's not determinable.
 */
static int gagt_is_style_fixed(glui32 style) {
	return gagt_confirm_appearance(style, stylehint_Proportional, 0);
}

static int gagt_is_style_bold(glui32 style) {
	return gagt_confirm_appearance(style, stylehint_Weight, 1);
}

static int gagt_is_style_oblique(glui32 style) {
	return gagt_confirm_appearance(style, stylehint_Oblique, 1);
}


/*
 * gagt_select_style()
 *
 * Given a set of AGT text attributes, this function returns a Glk style that
 * is suitable (or more accurately, the best we can come up with) for render-
 * ing this set of attributes.
 *
 * For now, we ignore color totally, and just concentrate on the other attr-
 * ibutes.  This is because few, if any, games use color (no Photopia here),
 * few Glk libraries, at least on Linux, allow fine grained control over text
 * color, and even if you can get it, the scarcity of user-defined styles in
 * Glk makes it too painful to contemplate.
 */
static glui32 gagt_select_style(gagt_attrset_t *attribute_set) {
	glui32 style;
	assert(attribute_set);

	/*
	 * Glk styles are mutually exclusive, so here we'll work here by making a
	 * precedence selection: AGT emphasis take precedence over AGT blinking,
	 * which itself takes precedence over normal text.  Fortunately, few, if
	 * any, AGT games set both emphasis and blinking (not likely to be a
	 * pleasant combination).
	 *
	 * We'll try to map AGT emphasis to Glk Subheader, AGT blink to Glk
	 * Emphasized, and normal text to Glk Normal, with modifications to this
	 * for fixed width requests.
	 *
	 * First, then, see if emphasized text is requested in the attributes.
	 */
	if (attribute_set->emphasis) {
		/*
		 * Consider whether something requested a fixed width font or
		 * disallowed a proportional one.
		 *
		 * Glk Preformatted is boring, flat, and lifeless.  It often offers no
		 * fine grained control over emphasis, and so on.  So here we try to
		 * find something better.  However, not all Glk libraries implement
		 * stylehints, so we need to try to be careful to ensure that we get a
		 * fixed width font, no matter what else we may miss out on.
		 */
		if (attribute_set->fixed) {
			/*
			 * To start off, we'll see if User1, the font we set up for fixed
			 * width bold, really is fixed width and bold.  If it is, we'll
			 * use it.
			 *
			 * If it isn't, we'll check Subheader.  Our Glk library probably
			 * isn't implementing stylehints, but if Subheader is fixed width,
			 * it may provide a better look than Preformatted -- certainly
			 * it's worth a go.
			 *
			 * If Subheader isn't fixed width, we'll take another look at User1.
			 * It could be that the check for bold wasn't definitive, but it
			 * is nevertheless bold.  So check for fixed width -- if set, it's
			 * probably good enough to use this font, certainly no worse than
			 * Preformatted.
			 *
			 * If Subheader isn't guaranteed fixed width, nor is User1, we're
			 * cornered into Preformatted.
			 */
			if (gagt_is_style_fixed(style_User1)
			        && gagt_is_style_bold(style_User1))
				style = style_User1;

			else if (gagt_is_style_fixed(style_Subheader))
				style = style_Subheader;

			else if (gagt_is_style_fixed(style_User1))
				style = style_User1;

			else
				style = style_Preformatted;
		} else
			/* This is the easy case, use Subheader. */
			style = style_Subheader;
	} else if (attribute_set->blink) {
		/*
		 * Again, consider whether something requested a fixed width
		 * font or disallowed a proportional one.
		 */
		if (attribute_set->fixed) {
			/*
			 * As above, try to find something better than Preformatted, first
			 * trying User2, then Emphasized, then User2 again, and finally
			 * settling for Preformatted if neither of these two looks any
			 * better.
			 */
			if (gagt_is_style_fixed(style_User2)
			        && gagt_is_style_oblique(style_User2))
				style = style_User2;

			else if (gagt_is_style_fixed(style_Emphasized))
				style = style_Emphasized;

			else if (gagt_is_style_fixed(style_User2))
				style = style_User2;

			else
				style = style_Preformatted;
		} else
			/* This is the easy case, use Emphasized. */
			style = style_Emphasized;
	} else {
		/*
		 * There's no emphasis or blinking in the attributes.  In this case,
		 * use Preformatted for fixed width, and Normal for text that can be
		 * rendered proportionally.
		 */
		if (attribute_set->fixed)
			style = style_Preformatted;
		else
			style = style_Normal;
	}

	return style;
}


/*---------------------------------------------------------------------*/
/*  Glk port output buffering functions                                */
/*---------------------------------------------------------------------*/

/*
 * Buffering game output happens at two levels.  The first level is a single
 * line buffer, used to catch text sent to us with agt_puts().  In parallel
 * with the text strings, we keep and buffer the game text attributes, as
 * handed to agt_textcolor(), that are in effect at the time the string is
 * handed to us, packed for brevity.
 *
 * As each line is completed, by a call to agt_newline(), this single line
 * buffer is transferred to a main text page buffer.  The main page buffer
 * has places in it where we can assign paragraph, font hints, and perhaps
 * other marker information to a line.  Initially unset, they're filled in
 * at the point where we need to display the buffer.
 */

/*
 * Definition of font hints values.  Font hints may be:
 *   o none, for lines not in a definite paragraph;
 *   o proportional, for lines that can probably be safely rendered in a
 *     proportional font (if the AGT game text attributes allow it) and
 *     where the newline may be replaced by a space;
 *   o proportional_newline, for lines that may be rendered using a
 *     proportional font, but where the newline looks like it matters;
 *   o proportional_newline_standout, for proportional_newline lines that
 *     are also standout (for spacing in display functions);
 *   o fixed_width, for tables and other text that looks like it is a
 *     candidate for fixed font output.
 */
typedef enum {
	HINT_NONE,
	HINT_PROPORTIONAL,
	HINT_PROPORTIONAL_NEWLINE,
	HINT_PROPORTIONAL_NEWLINE_STANDOUT,
	HINT_FIXED_WIDTH
} gagt_font_hint_t;

/* Magic number used to ensure a pointer points to a page buffer line. */
static const unsigned int GAGT_LINE_MAGIC = 0x5bc14482;

/*
 * Definition of a single line buffer.  This is a growable string and a
 * parallel growable attributes array.  The string is buffered without any
 * null terminator -- not needed since we retain length.
 */
typedef struct {
	unsigned char *data;        /* Buffered character data. */
	unsigned char *attributes;  /* Parallel character attributes, packed. */
	int allocation;             /* Bytes allocated to each of the above. */
	int length;                 /* Amount of data actually buffered. */
} gagt_string_t;
typedef gagt_string_t *gagt_stringref_t;

/*
 * Definition of a page buffer entry.  This is a structure that holds the
 * the result of a single line buffer above, plus additional areas that
 * describe line text positioning, a blank line flag, a paragraph pointer
 * (NULL if not in a paragraph), and a font hint.
 */
typedef struct gagt_line_s *gagt_lineref_t;
typedef struct gagt_paragraph_s *gagt_paragraphref_t;

struct gagt_line_s {
	unsigned int magic;             /* Assertion check dog-tag. */

	gagt_string_t buffer;           /* Buffered line string data. */

	int indent;                     /* Line indentation. */
	int outdent;                    /* Trailing line whitespace. */
	int real_length;                /* Real line length. */
	int is_blank;                   /* Line blank flag. */
	int is_hyphenated;              /* Line hyphenated flag. */

	gagt_paragraphref_t paragraph;  /* Paragraph containing the line. */
	gagt_font_hint_t font_hint;     /* Line's font hint. */

	gagt_lineref_t next;            /* List next element. */
	gagt_lineref_t prior;           /* List prior element. */
};

/*
 * Definition of the actual page buffer.  This is a doubly-linked list of
 * lines, with a tail pointer to facilitate adding entries at the end.
 */
static gagt_lineref_t gagt_page_head = NULL,
                      gagt_page_tail = NULL;

/*
 * Definition of the current output line; this one is appended to on
 * agt_puts(), and transferred into the page buffer on agt_newline().
 */
static gagt_string_t gagt_current_buffer = { NULL, NULL, 0, 0 };

/*
 * gagt_string_append()
 * gagt_string_transfer()
 * gagt_string_free()
 *
 * String append, move, and allocation free for string_t buffers.
 */
static void gagt_string_append(gagt_stringref_t buffer, const char *string,
                               unsigned char packed_attributes) {
	int length, bytes;

	/*
	 * Find the size we'll need from the line buffer to add this string,
	 * and grow buffer if necessary.
	 */
	length = strlen(string);
	for (bytes = buffer->allocation; bytes < buffer->length + length;)
		bytes = bytes == 0 ? 1 : bytes << 1;

	if (bytes > buffer->allocation) {
		buffer->data = (uchar *)gagt_realloc(buffer->data, bytes);
		buffer->attributes = (uchar *)gagt_realloc(buffer->attributes, bytes);

		buffer->allocation = bytes;
	}

	/* Add string to the line buffer, and store packed text attributes. */
	memcpy(buffer->data + buffer->length, string, length);
	memset(buffer->attributes + buffer->length, packed_attributes, length);

	buffer->length += length;
}

static void gagt_string_transfer(gagt_stringref_t from, gagt_stringref_t to) {
	*to = *from;
	from->data = from->attributes = NULL;
	from->allocation = from->length = 0;
}

static void gagt_string_free(gagt_stringref_t buffer) {
	free(buffer->data);
	free(buffer->attributes);
	buffer->data = buffer->attributes = NULL;
	buffer->allocation = buffer->length = 0;
}


/*
 * gagt_get_string_indent()
 * gagt_get_string_outdent()
 * gagt_get_string_real_length()
 * gagt_is_string_blank()
 * gagt_is_string_hyphenated()
 *
 * Metrics functions for string_t buffers.
 */
static int gagt_get_string_indent(const gagt_stringref_t buffer) {
	int indent, index;

	indent = 0;
	for (index = 0;
	        index < buffer->length && isspace(buffer->data[index]);
	        index++)
		indent++;

	return indent;
}

static int gagt_get_string_outdent(const gagt_stringref_t buffer) {
	int outdent, index;

	outdent = 0;
	for (index = buffer->length - 1;
	        index >= 0 && isspace(buffer->data[index]); index--)
		outdent++;

	return outdent;
}


static int gagt_get_string_real_length(const gagt_stringref_t buffer) {
	int indent, outdent;

	indent = gagt_get_string_indent(buffer);
	outdent = gagt_get_string_outdent(buffer);

	return indent == buffer->length ? 0 : buffer->length - indent - outdent;
}

static int gagt_is_string_blank(const gagt_stringref_t buffer) {
	return gagt_get_string_indent(buffer) == buffer->length;
}

static int gagt_is_string_hyphenated(const gagt_stringref_t buffer) {
	int is_hyphenated;

	is_hyphenated = FALSE;

	if (!gagt_is_string_blank(buffer)
	        && gagt_get_string_real_length(buffer) > 1) {
		int last;

		last = buffer->length - gagt_get_string_outdent(buffer) - 1;

		if (buffer->data[last] == '-') {
			if (isalpha(buffer->data[last - 1]))
				is_hyphenated = TRUE;
		}
	}

	return is_hyphenated;
}


/*
 * gagt_output_delete()
 *
 * Delete all buffered page and line text.  Free all malloc'ed buffer memory,
 * and return the buffer variables to their initial values.
 */
static void gagt_output_delete() {
	gagt_lineref_t line, next_line;

	for (line = gagt_page_head; line; line = next_line) {
		assert(line->magic == GAGT_LINE_MAGIC);
		next_line = line->next;

		gagt_string_free(&line->buffer);

		memset(line, 0, sizeof(*line));
		free(line);
	}

	gagt_page_head = gagt_page_tail = NULL;

	gagt_string_free(&gagt_current_buffer);
}


/*
 * agt_puts()
 *
 * Buffer the string passed in into our current single line buffer.  The
 * function converts to ISO 8859 Latin-1 encoding before buffering.
 */
void agt_puts(const char *cp_string) {
	assert(cp_string);

	if (!BATCH_MODE) {
		char *iso_string;
		unsigned char packed;
		int length;

		/* Update the apparent (virtual) window x position. */
		length = strlen(cp_string);
		curr_x += length;

		/*
		 * Convert the buffer from IBM cp 437 to Glk's ISO 8859 Latin-1, and
		 * add string and packed text attributes to the current line buffer.
		 */
		iso_string = (char *)gagt_malloc(length + 1);
		gagt_cp_to_iso((const uchar *)cp_string, (uchar *)iso_string);
		packed = gagt_pack_current_attributes();
		gagt_string_append(&gagt_current_buffer, iso_string, packed);

		/* Add the string to any script file. */
		if (script_on)
			textputs(scriptfile, iso_string);

		free(iso_string);
		gagt_debug("agt_puts", "string='%s'", cp_string);
	}
}


/*
 * agt_newline()
 *
 * Accept a newline to the main window.  Our job here is to append the
 * current line buffer to the page buffer, and clear the line buffer to
 * begin accepting new text.
 */
void agt_newline() {
	if (!BATCH_MODE) {
		gagt_lineref_t line;

		/* Update the apparent (virtual) window x position. */
		curr_x = 0;

		/* Create a new line entry for the page buffer. */
		line = (gagt_lineref_t)gagt_malloc(sizeof(*line));
		line->magic = GAGT_LINE_MAGIC;

		/* Move the line from the line buffer into the page buffer. */
		gagt_string_transfer(&gagt_current_buffer, &line->buffer);

		/* Fill in the line buffer metrics. */
		line->indent = gagt_get_string_indent(&line->buffer);
		line->outdent = gagt_get_string_outdent(&line->buffer);
		line->real_length = gagt_get_string_real_length(&line->buffer);
		line->is_blank = gagt_is_string_blank(&line->buffer);
		line->is_hyphenated = gagt_is_string_hyphenated(&line->buffer);

		/* For now, default the remaining page buffer fields for the line. */
		line->paragraph = NULL;
		line->font_hint = HINT_NONE;

		/* Add to the list, creating a new list if necessary. */
		line->next = NULL;
		line->prior = gagt_page_tail;
		if (gagt_page_head)
			gagt_page_tail->next = line;
		else
			gagt_page_head = line;
		gagt_page_tail = line;

		/* Add a newline to any script file. */
		if (script_on)
			textputs(scriptfile, "\n");

		gagt_debug("agt_newline", "");
	}
}


/*
 * gagt_get_first_page_line()
 * gagt_get_next_page_line()
 * gagt_get_prior_page_line()
 *
 * Iterator functions for the page buffer.  These functions return the first
 * line from the page buffer, the next line, or the previous line, given a
 * line, respectively.  They return NULL if no lines, or no more lines, are
 * available.
 */
static gagt_lineref_t gagt_get_first_page_line() {
	gagt_lineref_t line;

	line = gagt_page_head;
	assert(!line || line->magic == GAGT_LINE_MAGIC);
	return line;
}

static gagt_lineref_t gagt_get_next_page_line(const gagt_lineref_t line) {
	gagt_lineref_t next_line;
	assert(line && line->magic == GAGT_LINE_MAGIC);

	next_line = line->next;
	assert(!next_line || next_line->magic == GAGT_LINE_MAGIC);
	return next_line;
}

static gagt_lineref_t gagt_get_prior_page_line(const gagt_lineref_t line) {
	gagt_lineref_t prior_line;
	assert(line && line->magic == GAGT_LINE_MAGIC);

	prior_line = line->prior;
	assert(!prior_line || prior_line->magic == GAGT_LINE_MAGIC);
	return prior_line;
}


/*---------------------------------------------------------------------*/
/*  Glk port paragraphing functions and data                           */
/*---------------------------------------------------------------------*/

/* Magic number used to ensure a pointer points to a paragraph. */
static const unsigned int GAGT_PARAGRAPH_MAGIC = 0xb9a2297b;

/* Forward definition of special paragraph reference. */
typedef const struct gagt_special_s *gagt_specialref_t;

/*
 * Definition of a paragraph entry.  This is a structure that holds a
 * pointer to the first line buffer in the paragraph.
 */
struct gagt_paragraph_s {
	unsigned int magic;             /* Assertion check dog-tag. */

	gagt_lineref_t first_line;      /* First line in the paragraph. */
	gagt_specialref_t special;      /* Special paragraph entry. */

	int line_count;                 /* Number of lines in the paragraph. */
	int id;                         /* Paragraph id, sequence, for debug only. */

	gagt_paragraphref_t next;       /* List next element. */
	gagt_paragraphref_t prior;      /* List prior element. */
};

/*
 * A doubly-linked list of paragraphs, with a tail pointer to facilitate
 * adding entries at the end.
 */
static gagt_paragraphref_t gagt_paragraphs_head = NULL,
                           gagt_paragraphs_tail = NULL;

/*
 * gagt_paragraphs_delete()
 *
 * Delete paragraphs held in the list.  This function doesn't delete the
 * page buffer lines, just the paragraphs describing the page.
 */
static void gagt_paragraphs_delete() {
	gagt_paragraphref_t paragraph, next_paragraph;

	for (paragraph = gagt_paragraphs_head; paragraph; paragraph = next_paragraph) {
		assert(paragraph->magic == GAGT_PARAGRAPH_MAGIC);
		next_paragraph = paragraph->next;

		memset(paragraph, 0, sizeof(*paragraph));
		free(paragraph);
	}

	gagt_paragraphs_head = gagt_paragraphs_tail = NULL;
}


/*
 * gagt_find_paragraph_start()
 *
 * Find and return the next non-blank line in the page buffer, given a start
 * point.  Returns NULL if there are no more blank lines.
 */
static gagt_lineref_t gagt_find_paragraph_start(const gagt_lineref_t begin) {
	gagt_lineref_t line, match;

	/*
	 * Advance line to the beginning of the next paragraph, stopping on the
	 * first non-blank line, or at the end of the page buffer.
	 */
	match = NULL;
	for (line = begin; line; line = gagt_get_next_page_line(line)) {
		if (!line->is_blank) {
			match = line;
			break;
		}
	}

	return match;
}


/*
 * gagt_find_block_end()
 * gagt_find_blank_line_block_end()
 *
 * Find and return the apparent end of a paragraph from the page buffer,
 * given a start point, and an indentation reference.  The end is either
 * the point where indentation returns to the reference indentation, or
 * the next blank line.
 *
 * Indentation reference can be -1, indicating that only the next blank
 * line will end the paragraph.  Indentation references less than 1 are
 * also ignored.
 */
static gagt_lineref_t gagt_find_block_end(const gagt_lineref_t begin, int indent) {
	gagt_lineref_t line, match;

	/*
	 * Initialize the match to be the start of the block, then advance line
	 * until we hit a blank line or the end of the page buffer.  At this point,
	 * match contains the last line checked.
	 */
	match = begin;
	for (line = begin; line; line = gagt_get_next_page_line(line)) {
		/*
		 * Found if we reach a blank line, or when given an indentation to
		 * check for, we find it.
		 */
		if (line->is_blank || (indent > 0 && line->indent == indent))
			break;

		match = line;
	}

	return match;
}

static gagt_lineref_t gagt_find_blank_line_block_end(const gagt_lineref_t begin) {
	return gagt_find_block_end(begin, -1);
}


/*
 * gagt_find_paragraph_end()
 *
 * Find and return the apparent end of a paragraph from the page buffer,
 * given a start point.  The function attempts to recognize paragraphs by
 * the "shape" of indentation.
 */
static gagt_lineref_t gagt_find_paragraph_end(const gagt_lineref_t first_line) {
	gagt_lineref_t second_line;

	/*
	 * If the start line is the last line in the buffer, or if the next line
	 * is a blank line, return the start line as also being the end of the
	 * paragraph.
	 */
	second_line = gagt_get_next_page_line(first_line);
	if (!second_line || second_line->is_blank) {
		return first_line;
	}

	/*
	 * Time to look at line indentations...
	 *
	 * If either line is grossly indented, forget about trying to infer
	 * anything from this, and just break the paragraph on the next blank line.
	 */
	if (first_line->indent > screen_width / 4
	        || second_line->indent > screen_width / 4) {
		return gagt_find_blank_line_block_end(second_line);
	}

	/*
	 * If the first line is indented more than the second, end the paragraph
	 * on a blank line, or on a return in indentation to the level of the
	 * first line.  Here we're looking for paragraphs with the shape
	 *
	 *     aksjdj jfkasjd fjkasjd ajksdj fkaj djf akjsd fkjas dfs
	 * kasjdlkfjkj fj aksd jfjkasj dlkfja skjdk flaks dlf jalksdf
	 * ksjdf kjs kdf lasjd fkjalks jdfkjasjd flkjasl djfkasjfdkl
	 */
	else if (first_line->indent > second_line->indent) {
		return gagt_find_block_end(second_line, first_line->indent);
	}

	/*
	 * If the second line is more indented than the first, this may indicate
	 * a title line, followed by normal indented paragraphing.  In this case,
	 * use the second line indentation as the reference, and begin searching
	 * at the next line.  This finds
	 *
	 * ksjdkfjask ksadf
	 *     kajskd fksjkfj jfkj jfkslaj fksjlfj jkjskjlfa j fjksal
	 * sjkkdjf sj fkjkajkdlfj lsjak dfjk djkfjskl dklf alks dfll
	 * fjksja jkj dksja kjdk kaj dskfj aksjdf aksjd kfjaks fjks
	 *
	 * and
	 *
	 * asdfj kjsdf kjs
	 *     akjsdkj fkjs kdjfa lskjdl fjalsj dlfjksj kdj fjkd jlsjd
	 *     jalksj jfk slj lkfjsa lkjd lfjlaks dlfkjals djkj alsjd
	 *     kj jfksj fjksjl alkjs dlkjf lakjsd fkjas ldkj flkja fsd
	 */
	else if (second_line->indent > first_line->indent) {
		gagt_lineref_t third_line;

		/*
		 * See if we have a third buffer line to look at.  If we don't, or if
		 * we do but it's blank, the paragraph ends here.
		 */
		third_line = gagt_get_next_page_line(second_line);
		if (!third_line || third_line->is_blank) {
			return second_line;
		}

		/* As above, give up on gross indentation. */
		if (second_line->indent > screen_width / 4
		        || third_line->indent > screen_width / 4) {
			return gagt_find_blank_line_block_end(third_line);
		}

		/*
		 * If the second line indentation exceeds the third, this is probably
		 * a paragraph with a title line.  In this case, end the paragraph on
		 * a return to the indentation of the second line.  If not, just find
		 * the next blank line.
		 */
		else if (second_line->indent > third_line->indent) {
			return gagt_find_block_end(third_line, second_line->indent);
		} else {
			return gagt_find_blank_line_block_end(third_line);
		}
	}

	/*
	 * Otherwise, the first and second line indentations are the same, so
	 * break only on the next empty line.  This finds the simple
	 *
	 * ksd kjal jdljf lakjsd lkj lakjsdl jfla jsldj lfaksdj fksj
	 * lskjd fja kjsdlk fjlakjs ldkjfksj lkjdf kjalskjd fkjklal
	 * skjd fkaj djfkjs dkfjal sjdlkfj alksjdf lkajs ldkjf alljjf
	 */
	else {
		assert(second_line->indent == first_line->indent);
		return gagt_find_blank_line_block_end(second_line);
	}
}


/*
 * gagt_paragraph_page()
 *
 * This function breaks the page buffer into what appear to be paragraphs,
 * based on observations of indentation and blank separator lines.
 */
static void gagt_paragraph_page() {
	gagt_lineref_t start;

	assert(!gagt_paragraphs_head && !gagt_paragraphs_tail);

	/* Find the start of the first paragraph. */
	start = gagt_find_paragraph_start(gagt_get_first_page_line());
	while (start) {
		gagt_paragraphref_t paragraph;
		gagt_lineref_t end, line;

		/* Create a new paragraph entry. */
		paragraph = (gagt_paragraphref_t)gagt_malloc(sizeof(*paragraph));
		paragraph->magic = GAGT_PARAGRAPH_MAGIC;
		paragraph->first_line = start;
		paragraph->special = NULL;
		paragraph->line_count = 1;
		paragraph->id = gagt_paragraphs_tail ? gagt_paragraphs_tail->id + 1 : 0;

		/* Add to the list, creating a new list if necessary. */
		paragraph->next = NULL;
		paragraph->prior = gagt_paragraphs_tail;
		if (gagt_paragraphs_head)
			gagt_paragraphs_tail->next = paragraph;
		else
			gagt_paragraphs_head = paragraph;
		gagt_paragraphs_tail = paragraph;

		/* From the start, identify the paragraph end. */
		end = gagt_find_paragraph_end(start);

		/*
		 * Set paragraph in each line identified as part of this paragraph,
		 * and increment the paragraph's line count.
		 */
		for (line = start;
		        line != end; line = gagt_get_next_page_line(line)) {
			line->paragraph = paragraph;
			paragraph->line_count++;
		}
		end->paragraph = paragraph;

		/*
		 * If there's another line, look for the next paragraph there,
		 * otherwise we're done.
		 */
		line = gagt_get_next_page_line(end);
		if (line)
			start = gagt_find_paragraph_start(line);
		else
			start = NULL;
	}
}


/*
 * gagt_get_first_paragraph()
 * gagt_get_next_paragraph()
 *
 * Iterator functions for the paragraphs list.
 */
static gagt_paragraphref_t gagt_get_first_paragraph() {
	gagt_paragraphref_t paragraph;

	paragraph = gagt_paragraphs_head;
	assert(!paragraph || paragraph->magic == GAGT_PARAGRAPH_MAGIC);
	return paragraph;
}

static gagt_paragraphref_t gagt_get_next_paragraph(const gagt_paragraphref_t paragraph) {
	gagt_paragraphref_t next_paragraph;
	assert(paragraph && paragraph->magic == GAGT_PARAGRAPH_MAGIC);

	next_paragraph = paragraph->next;
	assert(!next_paragraph || next_paragraph->magic == GAGT_PARAGRAPH_MAGIC);
	return next_paragraph;
}


/*
 * gagt_get_first_paragraph_line()
 * gagt_get_next_paragraph_line()
 * gagt_get_prior_paragraph_line()
 *
 * Iterator functions for the page buffer.  These functions implement a
 * paragraph-based view of the page buffer.
 *
 * The functions find the first line of a given paragraph; given a line,
 * the next line in the same paragraph, or NULL if line is the last para-
 * graph line (or the last line in the page buffer); and given a line,
 * the previous line in the same paragraph, or NULL if line is the first
 * paragraph line (or the first line in the page buffer).
 */
static gagt_lineref_t gagt_get_first_paragraph_line(const gagt_paragraphref_t paragraph) {
	assert(paragraph && paragraph->magic == GAGT_PARAGRAPH_MAGIC);

	/* Return the first line for the requested paragraph. */
	return paragraph->first_line;
}

static gagt_lineref_t gagt_get_next_paragraph_line(const gagt_lineref_t line) {
	gagt_lineref_t next_line;

	/* Get the next line; return it if the paragraph matches, else NULL. */
	next_line = gagt_get_next_page_line(line);
	if (next_line && next_line->paragraph == line->paragraph)
		return next_line;
	else
		return NULL;
}

static gagt_lineref_t gagt_get_prior_paragraph_line(const gagt_lineref_t line) {
	gagt_lineref_t prior_line;

	/* Get the previous line; return it if the paragraph matches, else NULL. */
	prior_line = gagt_get_prior_page_line(line);
	if (prior_line && prior_line->paragraph == line->paragraph)
		return prior_line;
	else
		return NULL;
}


/*
 * gagt_get_paragraph_line_count()
 *
 * Return the count of lines contained in the paragraph.
 */
static int gagt_get_paragraph_line_count(const gagt_paragraphref_t paragraph) {
	assert(paragraph && paragraph->magic == GAGT_PARAGRAPH_MAGIC);

	return paragraph->line_count;
}


/*---------------------------------------------------------------------*/
/*  Glk port page buffer analysis functions                            */
/*---------------------------------------------------------------------*/

/*
 * Threshold for consecutive punctuation/spaces before we decide that a line
 * is in fact part of a table, and a small selection of characters to apply
 * a somewhat larger threshold to when looking for punctuation (typically,
 * characters that appear together multiple times in non-table text).
 */
static const int GAGT_THRESHOLD = 4,
                 GAGT_COMMON_THRESHOLD = 8;
static const char *const GAGT_COMMON_PUNCTUATION = ".!?";


/*
 * gagt_line_is_standout()
 *
 * Return TRUE if a page buffer line appears to contain "standout" text.
 * This is one of:
 *    - a line where all characters have some form of AGT text attribute
 *      set (blinking, fixed width font, or emphasis),
 *    - a line where each alphabetical character is uppercase.
 * Typically, this describes room and other miscellaneous header lines.
 */
static int gagt_line_is_standout(const gagt_lineref_t line) {
	int index, all_formatted, upper_count, lower_count;

	/*
	 * Look at the line, for cases where all characters in it have AGT font
	 * attributes, and counting the upper and lower case characters.  Iterate
	 * over only the significant characters in the string.
	 */
	all_formatted = TRUE;
	upper_count = lower_count = 0;
	for (index = line->indent;
	        index < line->buffer.length - line->outdent; index++) {
		gagt_attrset_t attribute_set;
		unsigned char character;

		gagt_unpack_attributes(line->buffer.attributes[index], &attribute_set);
		character = line->buffer.data[index];

		/*
		 * If no AGT attribute is set for this character, then not all of the
		 * line is standout text.  In this case, reset the all_formatted flag.
		 */
		if (!(attribute_set.blink
		        || attribute_set.fixed || attribute_set.emphasis))
			all_formatted = FALSE;

		/* Count upper and lower case characters. */
		if (islower(character))
			lower_count++;
		else if (isupper(character))
			upper_count++;
	}

	/*
	 * Consider standout if every character was formatted, or if the string
	 * is all uppercase.
	 */
	return all_formatted || (upper_count > 0 && lower_count == 0);
}


/*
 * gagt_set_font_hint_proportional()
 * gagt_set_font_hint_proportional_newline()
 * gagt_set_font_hint_fixed_width()
 *
 * Helpers for assigning font hints.  Font hints have strengths, and these
 * functions ensure that gagt_assign_paragraph_font_hints() only increases
 * strengths, and doesn't need to worry about checking before setting.  In
 * the case of newline, the function also adds standout to the font hint if
 * appropriate.
 */
static void gagt_set_font_hint_proportional(gagt_lineref_t line) {
	/* The only weaker hint than proportional is none. */
	if (line->font_hint == HINT_NONE)
		line->font_hint = HINT_PROPORTIONAL;
}

static void gagt_set_font_hint_proportional_newline(gagt_lineref_t line) {
	/*
	 * Proportional and none are weaker than newline.  Because of the way we
	 * set font hints, this function can't be called with a current line hint
	 * of proportional newline.
	 */
	if (line->font_hint == HINT_NONE || line->font_hint == HINT_PROPORTIONAL) {
		if (gagt_line_is_standout(line))
			line->font_hint = HINT_PROPORTIONAL_NEWLINE_STANDOUT;
		else
			line->font_hint = HINT_PROPORTIONAL_NEWLINE;
	}
}

static void gagt_set_font_hint_fixed_width(gagt_lineref_t line) {
	/* Fixed width font is the strongest hint. */
	if (line->font_hint == HINT_NONE
	        || line->font_hint == HINT_PROPORTIONAL
	        || line->font_hint == HINT_PROPORTIONAL_NEWLINE
	        || line->font_hint == HINT_PROPORTIONAL_NEWLINE_STANDOUT)
		line->font_hint = HINT_FIXED_WIDTH;
}


/*
 * gagt_assign_paragraph_font_hints()
 *
 * For a given paragraph in the page buffer, this function looks at the text
 * style used, and assigns a font hint value to each line.  Font hints
 * indicate whether the line probably requires fixed width font, or may be
 * okay in variable width, and for lines that look like they might be okay
 * in variable width, whether the newline should probably be rendered at the
 * end of the line, or if it might be omitted.
 */
static void gagt_assign_paragraph_font_hints(const gagt_paragraphref_t paragraph) {
	static int is_initialized = FALSE;
	static int threshold[BYTE_MAX_VAL + 1];

	gagt_lineref_t line, first_line;
	int is_table, in_list;
	assert(paragraph);

	/* On first call, set up the table on punctuation run thresholds. */
	if (!is_initialized) {
		int character;

		for (character = 0; character <= BYTE_MAX_VAL; character++) {
			/*
			 * Set the threshold, either a normal value, or a larger one for
			 * punctuation characters that tend to have consecutive runs in
			 * non-table text.
			 */
			if (ispunct(character)) {
				threshold[character] = strchr(GAGT_COMMON_PUNCTUATION, character)
				                       ? GAGT_COMMON_THRESHOLD : GAGT_THRESHOLD;
			}
		}

		is_initialized = TRUE;
	}

	/*
	 * Note the first paragraph line.  This value is commonly used, and under
	 * certain circumstances, it's also modified later on.
	 */
	first_line = gagt_get_first_paragraph_line(paragraph);
	assert(first_line);

	/*
	 * Phase 1 -- look for pages that consist of just one paragraph,
	 *            itself consisting of only one line.
	 *
	 * There is no point in attempting alignment of text in a one paragraph,
	 * one line page.  This would be, for example, an error message from the
	 * interpreter parser.  In this case, set the line for proportional with
	 * newline, and return immediately.
	 */
	if (gagt_get_first_paragraph() == paragraph
	        && !gagt_get_next_paragraph(paragraph)
	        && !gagt_get_next_paragraph_line(first_line)) {
		/*
		 * Set the first paragraph line for proportional with a newline, and
		 * return.
		 */
		gagt_set_font_hint_proportional_newline(first_line);
		return;
	}

	/*
	 * Phase 2 -- try to identify paragraphs that are tables, based on
	 *            looking for runs of punctuation.
	 *
	 * Search for any string that has a run of apparent line drawing or other
	 * formatting characters in it.  If we find one, we'll consider the
	 * paragraph to be a "table", that is, it has some quality that we might
	 * destroy if we used a proportional font.
	 */
	is_table = FALSE;
	for (line = first_line;
	        line && !is_table; line = gagt_get_next_paragraph_line(line)) {
		int index, counts[BYTE_MAX_VAL + 1], total_counts;

		/*
		 * Clear the initial counts.  Using memset() here is an order of
		 * magnitude or two faster than a for-loop.  Also there's a total count
		 * to detect when counts needs to be recleared, or is already clear.
		 */
		memset(counts, 0, sizeof(counts));
		total_counts = 0;

		/*
		 * Count consecutive punctuation in the line, excluding the indentation
		 * and outdent.
		 */
		for (index = line->indent;
		        index < line->buffer.length - line->outdent && !is_table; index++) {
			int character;
			character = line->buffer.data[index];

			/* Test this character for punctuation. */
			if (ispunct(character)) {
				/*
				 * Increment the count for this character, and note that
				 * counts are no longer empty, then compare against threshold.
				 */
				counts[character]++;
				total_counts++;

				is_table = (counts[character] >= threshold[character]);
			} else {
				/*
				 * Re-clear all counts, again with memset() for speed, but only
				 * if they need clearing.  As they often won't, this optimization
				 * saves quite a bit of work.
				 */
				if (total_counts > 0) {
					memset(counts, 0, sizeof(counts));
					total_counts = 0;
				}
			}
		}
	}

	/*
	 * Phase 3 -- try again to identify paragraphs that are tables, based
	 *            this time on looking for runs of whitespace.
	 *
	 * If no evidence found so far, look again, this time searching for any
	 * run of four or more spaces on the line (excluding any lead-in or
	 * trailing spaces).
	 */
	if (!is_table) {
		for (line = first_line;
		        line && !is_table; line = gagt_get_next_paragraph_line(line)) {
			int index, count;

			/*
			 * Count consecutive spaces in the line, excluding the indentation
			 * and outdent.
			 */
			count = 0;
			for (index = line->indent;
			        index < line->buffer.length - line->outdent && !is_table;
			        index++) {
				int character;
				character = line->buffer.data[index];

				if (isspace(character)) {
					count++;
					is_table = (count >= GAGT_THRESHOLD);
				} else
					count = 0;
			}
		}
	}

	/*
	 * If the paragraph appears to be a table, and if it consists of more than
	 * just a single line, mark all lines as fixed font output and return.
	 */
	if (is_table && gagt_get_next_paragraph_line(first_line)) {
		for (line = first_line;
		        line; line = gagt_get_next_paragraph_line(line)) {
			gagt_set_font_hint_fixed_width(line);
		}

		/* Nothing more to do. */
		return;
	}

	/*
	 * Phase 4 -- consider separating the first line from the rest of
	 *            the paragraph.
	 *
	 * Not a table, so the choice is between proportional rendering with a
	 * newline, and proportional rendering without...
	 *
	 * If the first paragraph line is standout or short, render it pro-
	 * portionally with a newline, and don't consider it as a further part of
	 * the paragraph.
	 */
	if (gagt_line_is_standout(first_line)
	        || first_line->real_length < screen_width / 2) {
		/* Set the first paragraph line for a newline. */
		gagt_set_font_hint_proportional_newline(first_line);

		/*
		 * Disassociate this line from the rest of the paragraph by moving on
		 * the value of the first_line variable.  If it turns out that there
		 * is no next paragraph line, then we have a one-line paragraph, and
		 * there's no more to do.
		 */
		first_line = gagt_get_next_paragraph_line(first_line);
		if (!first_line)
			return;
	}

	/*
	 * Phase 5 -- try to identify lists by a simple initial look at line
	 *            indentations.
	 *
	 * Look through the paragraph for apparent lists, and decide for each
	 * line whether it's appropriate to output a newline, and render
	 * proportionally, or just render proportionally.
	 *
	 * After this loop, each line will have some form of font hint assigned
	 * to it.
	 */
	in_list = FALSE;
	for (line = first_line;
	        line; line = gagt_get_next_paragraph_line(line)) {
		gagt_lineref_t next_line;

		next_line = gagt_get_next_paragraph_line(line);

		/*
		 * Special last-iteration processing.  The newline is always output at
		 * the end of a paragraph, so if there isn't a next line, then this
		 * line is the last paragraph line.  Set its font hint appropriately,
		 * and do no more for the line.
		 */
		if (!next_line) {
			gagt_set_font_hint_proportional_newline(line);
			continue;
		}

		/*
		 * If the next line's indentation is deeper that that of the first
		 * line, this paragraph looks like it is trying to be some form of a
		 * list.  In this case, make newline significant for the current line,
		 * and set the in_list flag so we can delay the return to proportional
		 * by one line.  On return to first line indentation, make newline
		 * significant for the return line.
		 */
		if (next_line->indent > first_line->indent) {
			gagt_set_font_hint_proportional_newline(line);
			in_list = TRUE;
		} else {
			if (in_list)
				gagt_set_font_hint_proportional_newline(line);
			else
				gagt_set_font_hint_proportional(line);
			in_list = FALSE;
		}
	}

	/*
	 * Phase 6 -- look again for lines that look like they are supposed
	 *            to stand out from their neighbors.
	 *
	 * Now rescan the paragraph, looking this time for lines that stand out
	 * from their neighbours.  Make newline significant for each such line,
	 * and the line above, if there is one.
	 *
	 * Here we split the loop on lines so that we avoid looking at the prior
	 * line of the current first line -- because of "adjustments", it may not
	 * be the real paragraph first line.
	 *
	 * So, deal with the current first line...
	 */
	if (gagt_line_is_standout(first_line)) {
		/* Make newline significant for this line. */
		gagt_set_font_hint_proportional_newline(first_line);
	}

	/* ... then deal with the rest of the lines, looking for standouts. */
	for (line = gagt_get_next_paragraph_line(first_line);
	        line; line = gagt_get_next_paragraph_line(line)) {
		if (gagt_line_is_standout(line)) {
			gagt_lineref_t prior_line;

			/* Make newline significant for this line. */
			gagt_set_font_hint_proportional_newline(line);

			/*
			 * Make newline significant for the line above.  There will always
			 * be one because we start the loop past the first line.
			 */
			prior_line = gagt_get_prior_paragraph_line(line);
			gagt_set_font_hint_proportional_newline(prior_line);
		}
	}

	/*
	 * Phase 7 -- special case short lines at the paragraph start.
	 *
	 * Make a special case of lines that begin a paragraph, and are short and
	 * followed by a much longer line.  This should catch games which output
	 * room titles above descriptions without using AGT fonts/bold/whatever.
	 * Without this trap, room titles and their descriptions are run together.
	 * This is more programmatic guesswork than heuristics.
	 */
	if (gagt_get_next_paragraph_line(first_line)) {
		gagt_lineref_t next_line;

		next_line = gagt_get_next_paragraph_line(first_line);

		/*
		 * See if the first line is less than half width, and the second line
		 * is more than three quarters width.  If it is, set newline as
		 * significant for the first paragraph line.
		 */
		if (first_line->real_length < screen_width / 2
		        && next_line->real_length > screen_width * 3 / 4) {
			gagt_set_font_hint_proportional_newline(first_line);
		}
	}

	/*
	 * Phase 8 -- special case paragraphs of only short lines.
	 *
	 * Make a special case out of paragraphs where all lines are short.  This
	 * catches elements like indented addresses.
	 */
	if (gagt_get_next_paragraph_line(first_line)) {
		int all_short;

		all_short = TRUE;
		for (line = first_line;
		        line; line = gagt_get_next_paragraph_line(line)) {
			/* Clear flag if this line isn't 'short'. */
			if (line->real_length >= screen_width / 2) {
				all_short = FALSE;
				break;
			}
		}

		/*
		 * If all lines were short, mark the complete paragraph as having
		 * significant newlines.
		 */
		if (all_short) {
			for (line = first_line;
			        line; line = gagt_get_next_paragraph_line(line)) {
				gagt_set_font_hint_proportional_newline(line);
			}
		}
	}
}


/*
 * gagt_assign_font_hints()
 *
 *
 * Sets a font hint for each line of each page buffer paragraph that is not
 * a special paragraph.
 */
static void gagt_assign_font_hints() {
	gagt_paragraphref_t paragraph;

	for (paragraph = gagt_get_first_paragraph();
	        paragraph; paragraph = gagt_get_next_paragraph(paragraph)) {
		if (!paragraph->special)
			gagt_assign_paragraph_font_hints(paragraph);
	}
}


/*---------------------------------------------------------------------*/
/*  Glk port special paragraph functions and data                      */
/*---------------------------------------------------------------------*/

/*
 * It's helpful to handle some AGiliTy interpreter output specially, to im-
 * prove the look of the text where Glk fonts and styles are available.  We
 * build a table of paragraphs the interpreter can come out with, and the
 * replacement text we'll use when we see this paragraph.  Note that matches
 * are made after factoring out indentation, and replacement lines do not
 * automatically print with a newline.  All clear, then?  Here's the table
 * entry definition.
 */
enum { GAGT_SPECIAL_MATCH_MAX = 5 };

typedef const struct gagt_special_s {
	const int line_count;
	const char *const compare[GAGT_SPECIAL_MATCH_MAX + 1];
	const char *const replace;
} gagt_special_t;

/*
 * Table of special AGiliTy interpreter strings and paragraphs -- where one
 * appears in game output, we'll print out its replacement instead.  Be
 * warned; these strings are VERY specific to AGiliTy 1.1.1.1, and are extre-
 * mely likely to change with any future interpreter releases.  They also
 * omit initializers with abandon, expecting the compiler to default these
 * to NULL/zero.  Replacement strings embed style encoding as |x, where x is
 * E(mphasized), S(ubheader), or N(ormal) for convenience.
 */
static gagt_special_t GAGT_SPECIALS[] = {

	/* Initial screen AGT game type line. */
	{
		1,
		{"[Created with Malmberg and Welch's Adventure Game Toolkit]"},
		"|ECreated with Malmberg and Welch's Adventure Game Toolkit|N\n"
	},

	/* Normal version of initial interpreter information block. */
	{
		4,
		{
			"This game is being executed by",
			"AGiliTy: The (Mostly) Universal AGT Interpreter  version 1.1.1.1",
			"Copyright (C) 1996-99,2001 by Robert Masenten",
			"Glk version"
		},
		"This game is being executed by:\n\n"
		"    |SAGiliTy, The (Mostly) Universal AGT Interpreter, Version 1.1.1.1|N\n"
		"    |ECopyright (C) 1996-1999,2001 by Robert Masenten|N\n"
		"    |EGlk version|N\n"
	},

	/* AGiliTy "information" screen header block. */
	{
		5,
		{
			"AGiliTy",
			"The (Mostly) Universal AGT Interpreter, version 1.1.1.1",
			"Copyright (C) 1996-1999,2001 by Robert Masenten",
			"[Glk version]",
			"-----------------------------------------------------------"
		},
		"|SAGiliTy, The (Mostly) Universal AGT Interpreter, Version 1.1.1.1|N\n"
		"|ECopyright (C) 1996-1999,2001 by Robert Masenten|N\n"
		"|EGlk version|N\n"
	},

	/* "HIT ANY KEY" message, usually displayed after a game's introduction. */
	{
		1,
		{"--- HIT ANY KEY ---"},
		"|E[Press any key...]|N"
	},

	/* Alternative, shrunken version of initial interpreter information block. */
	{
		2,
		{
			"Being run by AGiliTy  version 1.1.1.1, Copyright (C) 1996-99,2001"
			" Robert Masenten",
			"Glk version"
		},
		"This game is being executed by:\n\n"
		"    |SAGiliTy, The (Mostly) Universal AGT Interpreter, Version 1.1.1.1|N\n"
		"    |ECopyright (C) 1996-1999,2001 by Robert Masenten|N\n"
		"    |EGlk version|N\n"
	},

	/* Alternative, minimal version of initial interpreter information block. */
	{
		1,
		{
			"Being run by AGiliTy  version 1.1.1.1, Copyright (C) 1996-99,2001"
			" Robert Masenten"
		},
		"This game is being executed by:\n\n"
		"    |SAGiliTy, The (Mostly) Universal AGT Interpreter, Version 1.1.1.1|N\n"
		"    |ECopyright (C) 1996-1999,2001 by Robert Masenten|N\n"
		"    |EGlk version|N\n"
	},

	/* Lengthy version of the "Created with..." message. */
	{
		2,
		{
			"This game was created with Malmberg and Welch's Adventure Game Toolkit;"
			" it is",
			"being executed by"
		},
		"|ECreated with Malmberg and Welch's Adventure Game Toolkit|N\n"
	},

	/* Three-line version of initial interpreter information block. */
	{
		3,
		{
			"AGiliTy: The (Mostly) Universal AGT Interpreter  version 1.1.1.1",
			"Copyright (C) 1996-99,2001 by Robert Masenten",
			"Glk version"
		},
		"This game is being executed by:\n\n"
		"    |SAGiliTy, The (Mostly) Universal AGT Interpreter, Version 1.1.1.1|N\n"
		"    |ECopyright (C) 1996-1999,2001 by Robert Masenten|N\n"
		"    |EGlk version|N\n"
	},

	/*
	 * Assorted special verb output messages, with the extra icky quality that
	 * we have to spot messages that wrap because we forced screen_width to 80.
	 */
	{
		2,
		{
			"[Now in BRIEF mode (room descriptions will only be printed"
			" when they are entered",
			"the first time)]"
		},
		"|E[Now in BRIEF mode: Room descriptions will only be printed"
		" when rooms are entered for the first time.]|N\n"
	},

	{
		2,
		{
			"[Now in VERBOSE mode (room descriptions will be printed"
			" every time you enter a",
			"room)]"
		},
		"|E[Now in VERBOSE mode: Room descriptions will be printed"
		" every time you enter a room.]|N\n"
	},

	{
		1,
		{"[LISTEXIT mode on: room exits will be listed.]"},
		"|E[LISTEXIT mode on: Room exits will be listed.]|N\n"
	},

	{
		1,
		{"[LISTEXIT mode off: room exits will not be listed.]"},
		"|E[LISTEXIT mode off: Room exits will not be listed.]|N\n"
	},

	/* End of table sentinel entry.  Do not delete. */
	{0, {NULL}, NULL}
};


/*
 * gagt_compare_special_line()
 * gagt_compare_special_paragraph()
 *
 * Helpers for gagt_find_equivalent_special().  Compare line data case-
 * insensitively, taking care to use lengths rather than relying on line
 * buffer data being NUL terminated (which it's not); and iterate a complete
 * special paragraph comparison.
 */
static int gagt_compare_special_line(const char *compare, const gagt_lineref_t line) {
	/*
	 * Return true if the lengths match, and the real line data (excluding
	 * indent and outdent) also matches, ignoring case.
	 */
	return (int)strlen(compare) == line->real_length
	       && gagt_strncasecmp(compare,
	                           (const char *)line->buffer.data + line->indent,
	                           line->real_length) == 0;
}

static int gagt_compare_special_paragraph(const gagt_specialref_t special,
        const gagt_paragraphref_t paragraph) {
	/* If the line counts match, compare line by line. */
	if (special->line_count == gagt_get_paragraph_line_count(paragraph)) {
		gagt_lineref_t line;
		int index, is_match;

		is_match = TRUE;
		for (index = 0, line = gagt_get_first_paragraph_line(paragraph);
		        index < special->line_count && line;
		        index++, line = gagt_get_next_paragraph_line(line)) {
			if (!gagt_compare_special_line(special->compare[index], line)) {
				is_match = FALSE;
				break;
			}
		}

		return is_match;
	}

	/* Line count mismatch; return FALSE. */
	return FALSE;
}


/*
 * gagt_find_equivalent_special()
 *
 * Given a paragraph, see if it matches any of the special ones set up in
 * our array.  Returns the special, or NULL if no match.
 */
static gagt_specialref_t gagt_find_equivalent_special(gagt_paragraphref_t paragraph) {
	gagt_specialref_t special, match;

	/* Check each special paragraph entry for a match against this paragraph. */
	match = NULL;
	for (special = GAGT_SPECIALS; special->replace; special++) {
		if (gagt_compare_special_paragraph(special, paragraph)) {
			match = special;
			break;
		}
	}

	return match;
}


/*
 * gagt_mark_specials()
 *
 * Search for and mark any lines that match special paragraphs.
 */
static void gagt_mark_specials() {
	static int is_verified = FALSE;

	/*
	 * Verify special paragraphs table contents.  This checks that each entry
	 * ends with a NULL comparison, has a replacement, and that the line count
	 * matches.
	 */
	if (!is_verified) {
		gagt_specialref_t special;

		for (special = GAGT_SPECIALS; special->replace; special++) {
			int line_count, index;

			line_count = 0;
			for (index = 0; special->compare[index]; index++)
				line_count++;

			assert(special->line_count == line_count);
			assert(special->replace);
			assert(!special->compare[GAGT_SPECIAL_MATCH_MAX]);
		}

		is_verified = TRUE;
	}

	/*
	 * Search all paragraphs for special matches, if enabled.  When a special
	 * match is found, mark the paragraph with a pointer to the matching entry.
	 */
	if (g_vm->gagt_replacement_enabled) {
		gagt_paragraphref_t paragraph;

		for (paragraph = gagt_get_first_paragraph();
		        paragraph; paragraph = gagt_get_next_paragraph(paragraph)) {
			paragraph->special = gagt_find_equivalent_special(paragraph);
		}
	}
}


/*
 * gagt_display_special()
 *
 * Display the replacement text for the specified special table entry.  The
 * current Glk style in force is passed in; we return the Glk style in force
 * after we've done.
 */
static glui32 gagt_display_special(const gagt_specialref_t special, glui32 current_style) {
	glui32 set_style;
	int index, marker, length;
	const char *string;
	assert(special);

	/* Extract replacement string and length. */
	string = special->replace;
	assert(string);
	length = strlen(string);

	set_style = current_style;

	/*
	 * Iterate each character in replacement string, looking for style escapes,
	 * and flushing delayed output when one is found.
	 */
	marker = 0;
	for (index = 0; index < length; index++) {
		if (string[index] == '|') {
			glui32 style;

			/* Flush delayed output accumulated so far, excluding escape. */
			g_vm->glk_put_buffer(string + marker, index - marker);
			marker = index + 2;

			/* Determine any new text style. */
			style = set_style;
			switch (string[++index]) {
			case 'E':
				style = style_Emphasized;
				break;

			case 'S':
				style = style_Subheader;
				break;

			case 'N':
				style = style_Normal;
				break;

			default:
				gagt_fatal("GLK: Invalid replacement style escape");
				gagt_exit();
			}

			/* If style changed, update Glk's style setting. */
			if (style != set_style) {
				g_vm->glk_set_style(style);
				set_style = style;
			}
		}
	}

	/* Output any remaining delayed characters. */
	if (marker < length)
		g_vm->glk_put_buffer(string + marker, length - marker);

	return set_style;
}


/*---------------------------------------------------------------------*/
/*  Glk port output functions                                          */
/*---------------------------------------------------------------------*/

/*
 * Flag for if the user entered "help" as their last input, or if hints have
 * been silenced as a result of already using a Glk command.
 */
static int gagt_help_requested = FALSE,
           gagt_help_hints_silenced = FALSE;

/*
 * gagt_display_register_help_request()
 * gagt_display_silence_help_hints()
 * gagt_display_provide_help_hint()
 *
 * Register a request for help, and print a note of how to get Glk command
 * help from the interpreter unless silenced.
 */
static void gagt_display_register_help_request() {
	gagt_help_requested = TRUE;
}

static void gagt_display_silence_help_hints() {
	gagt_help_hints_silenced = TRUE;
}

static glui32 gagt_display_provide_help_hint(glui32 current_style) {
	if (gagt_help_requested && !gagt_help_hints_silenced) {
		g_vm->glk_set_style(style_Emphasized);
		g_vm->glk_put_string("[Try 'glk help' for help on special interpreter"
		                     " commands]\n");

		gagt_help_requested = FALSE;
		return style_Emphasized;
	}

	return current_style;
}


/*
 * gagt_display_text_element()
 *
 * Display an element of a buffer string using matching packed attributes.
 * The currently set Glk style is supplied, and the function returns the
 * new currently set Glk style.
 *
 * The function handles a flag to coerce fixed width font.
 */
static glui32 gagt_display_text_element(const char *string, const unsigned char *attributes,
                                        int length, glui32 current_style, int fixed_width) {
	int marker, index;
	glui32 set_style;
	assert(g_vm->glk_stream_get_current());

	set_style = current_style;

	/*
	 * Iterate each character in the line range.  We actually delay output
	 * until we see a change in style; that way, we can send a buffer of
	 * characters to Glk, rather than sending them just one at a time.
	 */
	marker = 0;
	for (index = 0; index < length; index++) {
		gagt_attrset_t attribute_set;
		glui32 style;
		assert(attributes && string);

		/*
		 * Unpack the AGT font attributes for this character, and add fixed
		 * width font coercion.
		 */
		gagt_unpack_attributes(attributes[index], &attribute_set);
		attribute_set.fixed |= fixed_width;

		/*
		 * Decide on any applicable new Glk text styling.  If it's different
		 * to the current style, output the delayed characters, and update
		 * Glk's style setting.
		 */
		style = gagt_select_style(&attribute_set);
		if (style != set_style) {
			g_vm->glk_put_buffer(string + marker, index - marker);
			marker = index;

			g_vm->glk_set_style(style);
			set_style = style;
		}
	}

	/* Output any remaining delayed characters. */
	if (marker < length)
		g_vm->glk_put_buffer(string + marker, length - marker);

	return set_style;
}


/*
 * gagt_display_line()
 *
 * Display a page buffer line, starting in the current Glk style, and
 * returning the new current Glk style.
 *
 * The function takes additional flags to force fixed width font, skip over
 * indentation and trailing line whitespace, and trim hyphens (if skipping
 * trailing whitespace).
 */
static glui32 gagt_display_line(const gagt_lineref_t line, glui32 current_style,
                                int fixed_width, int skip_indent, int skip_outdent,
                                int trim_hyphen) {
	int start, length;
	glui32 set_style;

	/*
	 * Check the skip indent flag to find the first character to display, and
	 * the count of characters to display.
	 */
	start = 0;
	length = line->buffer.length;
	if (skip_indent) {
		start += line->indent;
		length -= line->indent;
	}

	/* Adjust length for skipping outdent and trimming hyphens. */
	if (skip_outdent) {
		length -= line->outdent;
		if (trim_hyphen && line->is_hyphenated)
			length--;
	}

	/* Display this line segment. */
	set_style = gagt_display_text_element((const char *)line->buffer.data + start,
	                                      line->buffer.attributes + start,
	                                      length, current_style, fixed_width);

	return set_style;
}


/*
 * gagt_display_hinted_line()
 *
 * Display a page buffer line, starting in the current Glk style, and
 * returning the new current Glk style.  The function uses the font hints
 * from the line, and receives the font hint of the prior line.
 */
static glui32 gagt_display_hinted_line(const gagt_lineref_t line, glui32 current_style,
                                       gagt_font_hint_t prior_hint) {
	glui32 style;

	style = current_style;
	switch (line->font_hint) {
	case HINT_FIXED_WIDTH:
		/* Force fixed width font on the line. */
		style = gagt_display_line(line, style, TRUE, FALSE, FALSE, FALSE);

		g_vm->glk_put_char('\n');
		break;

	case HINT_PROPORTIONAL:
		/*
		 * Permit proportional font, and suppress outdent.  Suppress indent
		 * too if this line follows a line that suppressed newline, or is the
		 * first line in the paragraph.  For all cases, trim the hyphen from
		 * hyphenated lines.
		 */
		if (prior_hint == HINT_PROPORTIONAL || prior_hint == HINT_NONE)
			style = gagt_display_line(line, style, FALSE, TRUE, TRUE, TRUE);
		else
			style = gagt_display_line(line, style, FALSE, FALSE, TRUE, TRUE);

		/*
		 * Where the line is not hyphenated, output a space in place of newline.
		 * This lets paragraph text to flow to the full display width.
		 */
		if (!line->is_hyphenated)
			g_vm->glk_put_char(' ');
		break;

	case HINT_PROPORTIONAL_NEWLINE:
	case HINT_PROPORTIONAL_NEWLINE_STANDOUT:
		/*
		 * As above, permit proportional font, suppress outdent, and suppress
		 * indent too under certain conditions; in this case, only when the
		 * prior line suppressed newline.
		 */
		if (prior_hint == HINT_PROPORTIONAL)
			style = gagt_display_line(line, style, FALSE, TRUE, TRUE, FALSE);
		else
			style = gagt_display_line(line, style, FALSE, FALSE, TRUE, FALSE);

		g_vm->glk_put_char('\n');
		break;

	case HINT_NONE:
		gagt_fatal("GLK: Page buffer line with no font hint");
		gagt_exit();
		break;

	default:
		gagt_fatal("GLK: Invalid font hint encountered");
		gagt_exit();
		break;
	}

	return style;
}


/*
 * gagt_display_auto()
 *
 * Display buffered output text to the Glk main window using a bunch of
 * occasionally rather dodgy heuristics to try to automatically set a suitable
 * font for the way the text is structured, while replacing special paragraphs
 * with altered text.
 */
static void gagt_display_auto() {
	gagt_paragraphref_t paragraph;
	glui32 style;

	style = style_Normal;
	g_vm->glk_set_style(style);

	/* Handle each paragraph. */
	for (paragraph = gagt_get_first_paragraph();
	        paragraph; paragraph = gagt_get_next_paragraph(paragraph)) {
		/* If a special paragraph, output replacement text instead. */
		if (paragraph->special) {
			style = gagt_display_special(paragraph->special, style);
			g_vm->glk_put_char('\n');
		} else {
			gagt_lineref_t line;
			gagt_font_hint_t prior_hint;

			/* Get the first line of the paragraph. */
			line = gagt_get_first_paragraph_line(paragraph);

			/*
			 * Output a blank line where the first line of the first paragraph
			 * is standout; this sets it apart from the prompt.
			 */
			if (paragraph == gagt_get_first_paragraph()
			        && line == gagt_get_first_paragraph_line(paragraph)) {
				if (line->font_hint == HINT_PROPORTIONAL_NEWLINE_STANDOUT)
					g_vm->glk_put_char('\n');
			}

			/* Handle each line of the paragraph. */
			prior_hint = HINT_NONE;
			for (; line; line = gagt_get_next_paragraph_line(line)) {
				/*
				 * Print this line according to its font hint, noting any change
				 * of style and the line's font hint for use next iteration as
				 * the prior hint.
				 */
				style = gagt_display_hinted_line(line, style, prior_hint);
				prior_hint = line->font_hint;
			}

			/* Output the newline for the end of the paragraph. */
			g_vm->glk_put_char('\n');
		}
	}

	/* If no paragraphs at all, but a current buffer, output a newline. */
	if (!gagt_get_first_paragraph() && gagt_current_buffer.length > 0)
		g_vm->glk_put_char('\n');

	/* Output any help hint and unterminated line from the line buffer. */
	style = gagt_display_provide_help_hint(style);
	style = gagt_display_text_element((const char *)gagt_current_buffer.data,
	                                  gagt_current_buffer.attributes,
	                                  gagt_current_buffer.length, style, FALSE);
}


/*
 * gagt_display_manual()
 *
 * Display buffered output text in the Glk main window, with either a fixed
 * width or a proportional font.
 */
static void gagt_display_manual(int fixed_width) {
	gagt_lineref_t line;
	glui32 style;

	style = style_Normal;
	g_vm->glk_set_style(style);

	for (line = gagt_get_first_page_line();
	        line; line = gagt_get_next_page_line(line)) {
		gagt_paragraphref_t paragraph;

		paragraph = line->paragraph;

		/*
		 * If this is a special paragraph, display the replacement text on
		 * its first line and ignore remaining special lines.  Otherwise,
		 * display the page buffer line using either fixed or proportional
		 * font, as requested.
		 */
		if (paragraph && paragraph->special) {
			if (gagt_get_first_paragraph_line(paragraph) == line)
				style = gagt_display_special(paragraph->special, style);
		} else {
			style = gagt_display_line(line, style, fixed_width,
			                          FALSE, FALSE, FALSE);
			g_vm->glk_put_char('\n');
		}
	}

	/* Output any help hint and unterminated line from the line buffer. */
	style = gagt_display_provide_help_hint(style);
	style = gagt_display_text_element((const char *)gagt_current_buffer.data,
	                                  gagt_current_buffer.attributes,
	                                  gagt_current_buffer.length,
	                                  style, fixed_width);
}


/*
 * gagt_display_debug()
 *
 * Display the analyzed page buffer in a form that shows all of its gory
 * detail.
 */
static void gagt_display_debug() {
	gagt_lineref_t line;
	char buffer[256];

	g_vm->glk_set_style(style_Preformatted);
	for (line = gagt_get_first_page_line();
	        line; line = gagt_get_next_page_line(line)) {
		gagt_paragraphref_t paragraph;

		paragraph = line->paragraph;
		sprintf(buffer,
		        "%2d:%2d->%2ld A=%-3d L=%-2d I=%-2d O=%-2d R=%-2d %c%c| ",
		        paragraph ? paragraph->id + 1 : 0,
		        paragraph ? paragraph->line_count : 0,
		        paragraph && paragraph->special
		        ? paragraph->special - GAGT_SPECIALS + 1 : 0,
		        line->buffer.allocation, line->buffer.length,
		        line->indent, line->outdent,
		        line->real_length,
		        line->is_hyphenated ? 'h' : '_',
		        line->is_blank ? 'b' :
		        line->font_hint == HINT_PROPORTIONAL ? 'P' :
		        line->font_hint == HINT_PROPORTIONAL_NEWLINE ? 'N' :
		        line->font_hint == HINT_PROPORTIONAL_NEWLINE_STANDOUT ? 'S' :
		        line->font_hint == HINT_FIXED_WIDTH ? 'F' : '_');
		g_vm->glk_put_string(buffer);

		g_vm->glk_put_buffer((const char *)line->buffer.data, line->buffer.length);
		g_vm->glk_put_char('\n');
	}

	if (gagt_current_buffer.length > 0) {
		sprintf(buffer,
		        "__,__->__ A=%-3d L=%-2d I=__ O=__ R=__ %s| ",
		        gagt_current_buffer.allocation, gagt_current_buffer.length,
		        gagt_help_requested ? "HR" : "__");
		g_vm->glk_put_string(buffer);

		g_vm->glk_put_buffer((const char *)gagt_current_buffer.data, gagt_current_buffer.length);
	}

	gagt_help_requested = FALSE;
}


/*
 * gagt_output_flush()
 *
 * Flush any buffered output text to the Glk main window, and clear the
 * buffer ready for new output text.  The function concerns itself with
 * both the page buffer and any unterminated line in the line buffer.
 */
static void gagt_output_flush() {
	/*
	 * Run the analysis of page buffer contents.  This will fill in the
	 * paragraph and font hints fields, any any applicable special pointer,
	 * for every line held in the buffer.
	 */
	gagt_paragraph_page();
	gagt_mark_specials();
	gagt_assign_font_hints();

	/*
	 * Select the appropriate display routine to use, and call it.  The display
	 * routines present somewhat different output, and are responsible for
	 * displaying both the page buffer _and_ any buffered current line text.
	 */
	switch (g_vm->gagt_font_mode) {
	case FONT_AUTOMATIC:
		gagt_display_auto();
		break;

	case FONT_FIXED_WIDTH:
		gagt_display_manual(TRUE);
		break;

	case FONT_PROPORTIONAL:
		gagt_display_manual(FALSE);
		break;

	case FONT_DEBUG:
		gagt_display_debug();
		break;

	default:
		gagt_fatal("GLK: Invalid font mode encountered");
		gagt_exit();
	}

	/* Empty the buffer, ready for new game strings. */
	gagt_paragraphs_delete();
	gagt_output_delete();
}


/*
 * agt_clrscr()
 *
 * Clear the main playing area window.  Although there may be little point
 * in flushing (rather than emptying) the buffers, nevertheless that is
 * what we do.
 */
void agt_clrscr() {
	if (!BATCH_MODE) {
		/* Update the apparent (virtual) window x position. */
		curr_x = 0;

		/* Flush any pending buffered output, and clear the main window. */
		gagt_output_flush();
		g_vm->glk_window_clear(g_vm->gagt_main_window);

		/* Add a series of newlines to any script file. */
		if (script_on)
			textputs(scriptfile, "\n\n\n\n");

		gagt_debug("agt_clrscr", "");
	}
}


/*
 * gagt_styled_string()
 * gagt_styled_char()
 * gagt_standout_string()
 * gagt_standout_char()
 * gagt_normal_string()
 * gagt_normal_char()
 * gagt_header_string()
 *
 * Convenience functions to print strings in assorted styles.  A standout
 * string is one that hints that it's from the interpreter, not the game.
 */
static void gagt_styled_string(glui32 style, const char *message) {
	assert(message);

	g_vm->glk_set_style(style);
	g_vm->glk_put_string(message);
	g_vm->glk_set_style(style_Normal);
}

static void gagt_styled_char(glui32 style, char c) {
	char buffer[2];

	buffer[0] = c;
	buffer[1] = '\0';
	gagt_styled_string(style, buffer);
}

static void gagt_standout_string(const char *message) {
	gagt_styled_string(style_Emphasized, message);
}

static void gagt_standout_char(char c) {
	gagt_styled_char(style_Emphasized, c);
}

static void gagt_normal_string(const char *message) {
	gagt_styled_string(style_Normal, message);
}

static void gagt_normal_char(char c) {
	gagt_styled_char(style_Normal, c);
}

static void gagt_header_string(const char *message) {
	gagt_styled_string(style_Header, message);
}


/*---------------------------------------------------------------------*/
/*  Glk port delay functions                                           */
/*---------------------------------------------------------------------*/

/* Number of milliseconds in a second (traditionally, 1000). */
static const int GAGT_MS_PER_SEC = 1000;

/*
 * Number of milliseconds to timeout.  Because of jitter in the way Glk
 * generates timeouts, it's worthwhile implementing a delay using a number
 * of shorter timeouts.  This minimizes inaccuracies in the actual delay.
 */
static const glui32 GAGT_DELAY_TIMEOUT = 50;

/* The character key that can be pressed to cancel, and suspend, delays. */
static const char GAGT_DELAY_SUSPEND = ' ';

/*
 * Flag to temporarily turn off all delays.  This is set when the user
 * cancels a delay with a keypress, and remains set until the next time
 * that AGiliTy requests user input.  This way, games that call agt_delay()
 * sequentially don't require multiple keypresses to jump out of delay
 * sections.
 */
static int gagt_delays_suspended = FALSE;


/*
 * agt_delay()
 *
 * Delay for the specified number of seconds.  The delay can be canceled
 * by a user keypress.
 */
void agt_delay(int seconds) {
	glui32 milliseconds, delayed;
	int delay_completed;

	/* Suppress delay if in fast replay or batch mode. */
	if (fast_replay || BATCH_MODE)
		return;

	/*
	 * Do nothing if Glk doesn't have timers, if the delay state is set to
	 * ignore delays, if a zero or negative delay was specified, or if delays
	 * are currently temporarily suspended.
	 */
	if (!g_vm->glk_gestalt(gestalt_Timer, 0)
	        || g_vm->gagt_delay_mode == DELAY_OFF
	        || seconds <= 0 || gagt_delays_suspended)
		return;

	/* Flush any pending buffered output, and refresh status to show waiting. */
	gagt_output_flush();
	gagt_status_in_delay(TRUE);

	/* Calculate the number of milliseconds to delay. */
	milliseconds = (seconds * GAGT_MS_PER_SEC)
	               / (g_vm->gagt_delay_mode == DELAY_SHORT ? 2 : 1);

	/* Request timer events, and let a keypress cancel the delay. */
	g_vm->glk_request_char_event(g_vm->gagt_main_window);
	g_vm->glk_request_timer_events(GAGT_DELAY_TIMEOUT);

	/*
	 * Implement the delay using a sequence of shorter Glk timeouts, with an
	 * option to cancel the delay with a keypress.
	 */
	delay_completed = TRUE;
	for (delayed = 0; delayed < milliseconds; delayed += GAGT_DELAY_TIMEOUT) {
		event_t event;

		/* Wait for the next timeout, or a character. */
		gagt_event_wait_2(evtype_CharInput, evtype_Timer, &event);
		if (event.type == evtype_CharInput) {
			/*
			 * If suspend requested, stop the delay, and set the delay
			 * suspension flag, and a note that the delay loop didn't complete.
			 * Otherwise, reissue the character input request.
			 */
			if (event.val1 == GAGT_DELAY_SUSPEND) {
				gagt_delays_suspended = TRUE;
				delay_completed = FALSE;
				break;
			} else
				g_vm->glk_request_char_event(g_vm->gagt_main_window);
		}
	}

	/* Cancel any pending character input, and timer events. */
	if (delay_completed)
		g_vm->glk_cancel_char_event(g_vm->gagt_main_window);
	g_vm->glk_request_timer_events(0);

	/* Clear the waiting indicator. */
	gagt_status_in_delay(FALSE);

	gagt_debug("agt_delay", "seconds=%d [%lu mS] -> %s", seconds, milliseconds,
	           delay_completed ? "completed" : "canceled");
}


/*
 * gagt_delay_resume()
 *
 * Unsuspend delays.  This function should be called by agt_input() and
 * agt_getkey(), to re-enable delays when the interpreter next requests
 * user input.
 */
static void gagt_delay_resume() {
	gagt_delays_suspended = FALSE;
}


/*---------------------------------------------------------------------*/
/*  Glk port box drawing functions                                     */
/*---------------------------------------------------------------------*/

/* Saved details of any current box dimensions and flags. */
static unsigned long gagt_box_flags = 0;
static int gagt_box_busy = FALSE,
           gagt_box_width = 0,
           gagt_box_height = 0,
           gagt_box_startx = 0;


/*
 * gagt_box_rule()
 * gagt_box_position()
 *
 * Draw a line at the top or bottom of a box, and position the cursor
 * with a box indent.
 */
static void gagt_box_rule(int width) {
	char *ruler;

	/* Write a +--...--+ ruler to delimit a box. */
	ruler = (char *)gagt_malloc(width + 2 + 1);
	memset(ruler + 1, '-', width);
	ruler[0] = ruler[width + 1] = '+';
	ruler[width + 2] = '\0';
	agt_puts(ruler);
	free(ruler);
}

static void gagt_box_position(int indent) {
	char *spaces;

	/* Write a newline before the indent. */
	agt_newline();

	/* Write the indent to the start of box text. */
	spaces = (char *)gagt_malloc(indent + 1);
	memset(spaces, ' ', indent);
	spaces[indent] = '\0';
	agt_puts(spaces);
	free(spaces);
}


/*
 * agt_makebox()
 * agt_qnewline()
 * agt_endbox()
 *
 * Start a box of given width, height, and with given flags.  Write a new
 * line in the box.  And end the box.
 */
void agt_makebox(int width, int height, unsigned long flags) {
	assert(!gagt_box_busy);

	gagt_box_busy = TRUE;
	gagt_box_flags = flags;
	gagt_box_width = width;
	gagt_box_height = height;

	/* If no centering requested, set the indent to zero. */
	if (gagt_box_flags & TB_NOCENT)
		gagt_box_startx = 0;
	else {
		int centering_width;

		/*
		 * Calculate the indent for centering, adding 4 characters for borders.
		 * Here, since screen_width is artificial, we'll center off status_width
		 * if it is less than screen width, otherwise we'll center by using
		 * screen_width.  The reason for shrinking to screen_width is that if
		 * we don't, we could drive curr_x to beyond screen_width with our box
		 * indentations, and that confuses AGiliTy.
		 */
		if (status_width < screen_width)
			centering_width = status_width;
		else
			centering_width = screen_width;
		if (gagt_box_flags & TB_BORDER)
			gagt_box_startx = (centering_width - gagt_box_width - 4) / 2;
		else
			gagt_box_startx = (centering_width - gagt_box_width) / 2;

		/* If the box turns out wider than the window, abandon centering. */
		if (gagt_box_startx < 0)
			gagt_box_startx = 0;
	}

	/*
	 * When in a box, we'll coerce fixed width font by setting it in the AGT
	 * font attributes.  This ensures that the box displays as accurately as
	 * we're able to achieve.
	 */
	gagt_coerce_fixed_font(TRUE);

	/* Position the cursor for the box, and if bordered, write the rule. */
	gagt_box_position(gagt_box_startx);
	if (gagt_box_flags & TB_BORDER) {
		gagt_box_rule(gagt_box_width + 2);
		gagt_box_position(gagt_box_startx);
		agt_puts("| ");
	}

	gagt_debug("agt_makebox", "width=%d, height=%d, flags=0x%lx",
	           width, height, flags);
}

void agt_qnewline() {
	assert(gagt_box_busy);

	/* Write box characters for the current and next line. */
	if (gagt_box_flags & TB_BORDER) {
		agt_puts(" |");
		gagt_box_position(gagt_box_startx);
		agt_puts("| ");
	} else
		gagt_box_position(gagt_box_startx);

	gagt_debug("agt_qnewline", "");
}

void agt_endbox() {
	assert(gagt_box_busy);

	/* Finish off the current box. */
	if (gagt_box_flags & TB_BORDER) {
		agt_puts(" |");
		gagt_box_position(gagt_box_startx);
		gagt_box_rule(gagt_box_width + 2);
	}
	agt_newline();

	/* An extra newline here improves the appearance. */
	agt_newline();

	/* Back to allowing proportional font output again. */
	gagt_coerce_fixed_font(FALSE);

	gagt_box_busy = FALSE;
	gagt_box_flags = gagt_box_width = gagt_box_startx = 0;

	gagt_debug("agt_endbox", "");
}


/*---------------------------------------------------------------------*/
/*  Glk command escape functions                                       */
/*---------------------------------------------------------------------*/

/*
 * gagt_command_script()
 *
 * Turn game output scripting (logging) on and off.
 */
static void gagt_command_script(const char *argument) {
	assert(argument);

	if (gagt_strcasecmp(argument, "on") == 0) {
		frefid_t fileref;

		if (g_vm->gagt_transcript_stream) {
			gagt_normal_string("Glk transcript is already on.\n");
			return;
		}

		fileref = g_vm->glk_fileref_create_by_prompt(fileusage_Transcript
		          | fileusage_TextMode,
		          filemode_WriteAppend, 0);
		if (!fileref) {
			gagt_standout_string("Glk transcript failed.\n");
			return;
		}

		g_vm->gagt_transcript_stream = g_vm->glk_stream_open_file(fileref,
		                         filemode_WriteAppend, 0);
		g_vm->glk_fileref_destroy(fileref);
		if (!g_vm->gagt_transcript_stream) {
			gagt_standout_string("Glk transcript failed.\n");
			return;
		}

		g_vm->glk_window_set_echo_stream(g_vm->gagt_main_window, g_vm->gagt_transcript_stream);

		gagt_normal_string("Glk transcript is now on.\n");
	}

	else if (gagt_strcasecmp(argument, "off") == 0) {
		if (!g_vm->gagt_transcript_stream) {
			gagt_normal_string("Glk transcript is already off.\n");
			return;
		}

		g_vm->glk_stream_close(g_vm->gagt_transcript_stream, NULL);
		g_vm->gagt_transcript_stream = NULL;

		g_vm->glk_window_set_echo_stream(g_vm->gagt_main_window, NULL);

		gagt_normal_string("Glk transcript is now off.\n");
	}

	else if (strlen(argument) == 0) {
		gagt_normal_string("Glk transcript is ");
		gagt_normal_string(g_vm->gagt_transcript_stream ? "on" : "off");
		gagt_normal_string(".\n");
	}

	else {
		gagt_normal_string("Glk transcript can be ");
		gagt_standout_string("on");
		gagt_normal_string(", or ");
		gagt_standout_string("off");
		gagt_normal_string(".\n");
	}
}


/*
 * gagt_command_inputlog()
 *
 * Turn game input logging on and off.
 */
static void gagt_command_inputlog(const char *argument) {
	assert(argument);

	if (gagt_strcasecmp(argument, "on") == 0) {
		frefid_t fileref;

		if (g_vm->gagt_inputlog_stream) {
			gagt_normal_string("Glk input logging is already on.\n");
			return;
		}

		fileref = g_vm->glk_fileref_create_by_prompt(fileusage_InputRecord
		          | fileusage_BinaryMode,
		          filemode_WriteAppend, 0);
		if (!fileref) {
			gagt_standout_string("Glk input logging failed.\n");
			return;
		}

		g_vm->gagt_inputlog_stream = g_vm->glk_stream_open_file(fileref,
		                       filemode_WriteAppend, 0);
		g_vm->glk_fileref_destroy(fileref);
		if (!g_vm->gagt_inputlog_stream) {
			gagt_standout_string("Glk input logging failed.\n");
			return;
		}

		gagt_normal_string("Glk input logging is now on.\n");
	}

	else if (gagt_strcasecmp(argument, "off") == 0) {
		if (!g_vm->gagt_inputlog_stream) {
			gagt_normal_string("Glk input logging is already off.\n");
			return;
		}

		g_vm->glk_stream_close(g_vm->gagt_inputlog_stream, NULL);
		g_vm->gagt_inputlog_stream = NULL;

		gagt_normal_string("Glk input log is now off.\n");
	}

	else if (strlen(argument) == 0) {
		gagt_normal_string("Glk input logging is ");
		gagt_normal_string(g_vm->gagt_inputlog_stream ? "on" : "off");
		gagt_normal_string(".\n");
	}

	else {
		gagt_normal_string("Glk input logging can be ");
		gagt_standout_string("on");
		gagt_normal_string(", or ");
		gagt_standout_string("off");
		gagt_normal_string(".\n");
	}
}


/*
 * gagt_command_readlog()
 *
 * Set the game input log, to read input from a file.
 */
static void gagt_command_readlog(const char *argument) {
	assert(argument);

	if (gagt_strcasecmp(argument, "on") == 0) {
		frefid_t fileref;

		if (g_vm->gagt_readlog_stream) {
			gagt_normal_string("Glk read log is already on.\n");
			return;
		}

		fileref = g_vm->glk_fileref_create_by_prompt(fileusage_InputRecord
		          | fileusage_BinaryMode,
		          filemode_Read, 0);
		if (!fileref) {
			gagt_standout_string("Glk read log failed.\n");
			return;
		}

		if (!g_vm->glk_fileref_does_file_exist(fileref)) {
			g_vm->glk_fileref_destroy(fileref);
			gagt_standout_string("Glk read log failed.\n");
			return;
		}

		g_vm->gagt_readlog_stream = g_vm->glk_stream_open_file(fileref, filemode_Read, 0);
		g_vm->glk_fileref_destroy(fileref);
		if (!g_vm->gagt_readlog_stream) {
			gagt_standout_string("Glk read log failed.\n");
			return;
		}

		gagt_normal_string("Glk read log is now on.\n");
	}

	else if (gagt_strcasecmp(argument, "off") == 0) {
		if (!g_vm->gagt_readlog_stream) {
			gagt_normal_string("Glk read log is already off.\n");
			return;
		}

		g_vm->glk_stream_close(g_vm->gagt_readlog_stream, NULL);
		g_vm->gagt_readlog_stream = NULL;

		gagt_normal_string("Glk read log is now off.\n");
	}

	else if (strlen(argument) == 0) {
		gagt_normal_string("Glk read log is ");
		gagt_normal_string(g_vm->gagt_readlog_stream ? "on" : "off");
		gagt_normal_string(".\n");
	}

	else {
		gagt_normal_string("Glk read log can be ");
		gagt_standout_string("on");
		gagt_normal_string(", or ");
		gagt_standout_string("off");
		gagt_normal_string(".\n");
	}
}


/*
 * gagt_command_abbreviations()
 *
 * Turn abbreviation expansions on and off.
 */
static void gagt_command_abbreviations(const char *argument) {
	assert(argument);

	if (gagt_strcasecmp(argument, "on") == 0) {
		if (g_vm->gagt_abbreviations_enabled) {
			gagt_normal_string("Glk abbreviation expansions are already on.\n");
			return;
		}

		g_vm->gagt_abbreviations_enabled = TRUE;
		gagt_normal_string("Glk abbreviation expansions are now on.\n");
	}

	else if (gagt_strcasecmp(argument, "off") == 0) {
		if (!g_vm->gagt_abbreviations_enabled) {
			gagt_normal_string("Glk abbreviation expansions are already off.\n");
			return;
		}

		g_vm->gagt_abbreviations_enabled = FALSE;
		gagt_normal_string("Glk abbreviation expansions are now off.\n");
	}

	else if (strlen(argument) == 0) {
		gagt_normal_string("Glk abbreviation expansions are ");
		gagt_normal_string(g_vm->gagt_abbreviations_enabled ? "on" : "off");
		gagt_normal_string(".\n");
	}

	else {
		gagt_normal_string("Glk abbreviation expansions can be ");
		gagt_standout_string("on");
		gagt_normal_string(", or ");
		gagt_standout_string("off");
		gagt_normal_string(".\n");
	}
}


/*
 * gagt_command_fonts()
 *
 * Set the value for g_vm->gagt_font_mode depending on the argument from the
 * user's command escape.
 *
 * Despite our best efforts, font control may still be wrong in some games.
 * This command gives us a chance to correct that.
 */
static void gagt_command_fonts(const char *argument) {
	assert(argument);

	if (gagt_strcasecmp(argument, "fixed") == 0) {
		if (g_vm->gagt_font_mode == FONT_FIXED_WIDTH) {
			gagt_normal_string("Glk font control is already 'fixed'.\n");
			return;
		}

		g_vm->gagt_font_mode = FONT_FIXED_WIDTH;
		gagt_normal_string("Glk font control is now 'fixed'.\n");
	}

	else if (gagt_strcasecmp(argument, "variable") == 0
	         || gagt_strcasecmp(argument, "proportional") == 0) {
		if (g_vm->gagt_font_mode == FONT_PROPORTIONAL) {
			gagt_normal_string("Glk font control is already 'proportional'.\n");
			return;
		}

		g_vm->gagt_font_mode = FONT_PROPORTIONAL;
		gagt_normal_string("Glk font control is now 'proportional'.\n");
	}

	else if (gagt_strcasecmp(argument, "auto") == 0
	         || gagt_strcasecmp(argument, "automatic") == 0) {
		if (g_vm->gagt_font_mode == FONT_AUTOMATIC) {
			gagt_normal_string("Glk font control is already 'automatic'.\n");
			return;
		}

		g_vm->gagt_font_mode = FONT_AUTOMATIC;
		gagt_normal_string("Glk font control is now 'automatic'.\n");
	}

	else if (gagt_strcasecmp(argument, "debug") == 0) {
		if (g_vm->gagt_font_mode == FONT_DEBUG) {
			gagt_normal_string("Glk font control is already 'debug'.\n");
			return;
		}

		g_vm->gagt_font_mode = FONT_DEBUG;
		gagt_normal_string("Glk font control is now 'debug'.\n");
	}

	else if (strlen(argument) == 0) {
		gagt_normal_string("Glk font control is set to '");
		switch (g_vm->gagt_font_mode) {
		case FONT_AUTOMATIC:
			gagt_normal_string("automatic");
			break;

		case FONT_FIXED_WIDTH:
			gagt_normal_string("fixed");
			break;

		case FONT_PROPORTIONAL:
			gagt_normal_string("proportional");
			break;

		case FONT_DEBUG:
			gagt_normal_string("debug");
			break;

		default:
			gagt_fatal("GLK: Invalid font mode encountered");
			gagt_exit();
		}
		gagt_normal_string("'.\n");
	}

	else {
		/* Avoid mentioning the debug setting. */
		gagt_normal_string("Glk font control can be ");
		gagt_standout_string("fixed");
		gagt_normal_string(", ");
		gagt_standout_string("proportional");
		gagt_normal_string(", or ");
		gagt_standout_string("automatic");
		gagt_normal_string(".\n");
	}
}


/*
 * gagt_command_delays()
 *
 * Set a value for g_vm->gagt_delay_mode depending on the argument from
 * the user's command escape.
 */
static void gagt_command_delays(const char *argument) {
	assert(argument);

	if (!g_vm->glk_gestalt(gestalt_Timer, 0)) {
		gagt_normal_string("Glk delays are not available.\n");
		return;
	}

	if (gagt_strcasecmp(argument, "full") == 0
	        || gagt_strcasecmp(argument, "on") == 0) {
		if (g_vm->gagt_delay_mode == DELAY_FULL) {
			gagt_normal_string("Glk delay mode is already 'full'.\n");
			return;
		}

		g_vm->gagt_delay_mode = DELAY_FULL;
		gagt_normal_string("Glk delay mode is now 'full'.\n");
	}

	else if (gagt_strcasecmp(argument, "short") == 0
	         || gagt_strcasecmp(argument, "half") == 0) {
		if (g_vm->gagt_delay_mode == DELAY_SHORT) {
			gagt_normal_string("Glk delay mode is already 'short'.\n");
			return;
		}

		g_vm->gagt_delay_mode = DELAY_SHORT;
		gagt_normal_string("Glk delay mode is now 'short'.\n");
	}

	else if (gagt_strcasecmp(argument, "none") == 0
	         || gagt_strcasecmp(argument, "off") == 0) {
		if (g_vm->gagt_delay_mode == DELAY_OFF) {
			gagt_normal_string("Glk delay mode is already 'none'.\n");
			return;
		}

		g_vm->gagt_delay_mode = DELAY_OFF;
		gagt_normal_string("Glk delay mode is now 'none'.\n");
	}

	else if (strlen(argument) == 0) {
		gagt_normal_string("Glk delay mode is set to '");
		switch (g_vm->gagt_delay_mode) {
		case DELAY_FULL:
			gagt_normal_string("full");
			break;

		case DELAY_SHORT:
			gagt_normal_string("short");
			break;

		case DELAY_OFF:
			gagt_normal_string("none");
			break;

		default:
			gagt_fatal("GLK: Invalid delay mode encountered");
			gagt_exit();
		}
		gagt_normal_string("'.\n");
	}

	else {
		gagt_normal_string("Glk delay mode can be ");
		gagt_standout_string("full");
		gagt_normal_string(", ");
		gagt_standout_string("short");
		gagt_normal_string(", or ");
		gagt_standout_string("none");
		gagt_normal_string(".\n");
	}
}


/*
 * gagt_command_width()
 *
 * Print out the (approximate) display width, from status_width.  It's
 * approximate because the main window might include a scrollbar that
 * the status window doesn't have, may use a different size font, and so
 * on.  But the main window won't tell us a width at all - it always
 * returns zero.  If we don't happen to have a status window available
 * to us, there's not much we can say.
 *
 * Note that this function uses the interpreter variable status_width,
 * so it's important to keep this updated with the current window size at
 * all times.
 */
static void gagt_command_width(const char *argument) {
	char buffer[16];
	assert(argument);

	if (!g_vm->gagt_status_window) {
		gagt_normal_string("Glk's current display width is unknown.\n");
		return;
	}

	gagt_normal_string("Glk's current display width is approximately ");
	sprintf(buffer, "%d", status_width);
	gagt_normal_string(buffer);
	gagt_normal_string(status_width == 1 ? " character" : " characters");
	gagt_normal_string(".\n");
}


/*
 * gagt_command_replacements()
 *
 * Turn Glk special paragraph replacement on and off.
 */
static void gagt_command_replacements(const char *argument) {
	assert(argument);

	if (gagt_strcasecmp(argument, "on") == 0) {
		if (g_vm->gagt_replacement_enabled) {
			gagt_normal_string("Glk replacements are already on.\n");
			return;
		}

		g_vm->gagt_replacement_enabled = TRUE;
		gagt_normal_string("Glk replacements are now on.\n");
	}

	else if (gagt_strcasecmp(argument, "off") == 0) {
		if (!g_vm->gagt_replacement_enabled) {
			gagt_normal_string("Glk replacements are already off.\n");
			return;
		}

		g_vm->gagt_replacement_enabled = FALSE;
		gagt_normal_string("Glk replacements are now off.\n");
	}

	else if (strlen(argument) == 0) {
		gagt_normal_string("Glk replacements are ");
		gagt_normal_string(g_vm->gagt_replacement_enabled ? "on" : "off");
		gagt_normal_string(".\n");
	}

	else {
		gagt_normal_string("Glk replacements can be ");
		gagt_standout_string("on");
		gagt_normal_string(", or ");
		gagt_standout_string("off");
		gagt_normal_string(".\n");
	}
}


/*
 * gagt_command_statusline()
 *
 * Turn the extended status line on and off.
 */
static void gagt_command_statusline(const char *argument) {
	assert(argument);

	if (!g_vm->gagt_status_window) {
		gagt_normal_string("Glk status window is not available.\n");
		return;
	}

	if (gagt_strcasecmp(argument, "extended") == 0
	        || gagt_strcasecmp(argument, "full") == 0) {
		if (g_vm->gagt_extended_status_enabled) {
			gagt_normal_string("Glk status line mode is already 'extended'.\n");
			return;
		}

		/* Expand the status window down to a second line. */
		g_vm->glk_window_set_arrangement(g_vm->glk_window_get_parent(g_vm->gagt_status_window),
		                                 winmethod_Above | winmethod_Fixed, 2, NULL);
		g_vm->gagt_extended_status_enabled = TRUE;

		gagt_normal_string("Glk status line mode is now 'extended'.\n");
	}

	else if (gagt_strcasecmp(argument, "short") == 0
	         || gagt_strcasecmp(argument, "normal") == 0) {
		if (!g_vm->gagt_extended_status_enabled) {
			gagt_normal_string("Glk status line mode is already 'short'.\n");
			return;
		}

		/* Shrink the status window down to one line. */
		g_vm->glk_window_set_arrangement(g_vm->glk_window_get_parent(g_vm->gagt_status_window),
		                                 winmethod_Above | winmethod_Fixed, 1, NULL);
		g_vm->gagt_extended_status_enabled = FALSE;

		gagt_normal_string("Glk status line mode is now 'short'.\n");
	}

	else if (strlen(argument) == 0) {
		gagt_normal_string("Glk status line mode is set to '");
		gagt_normal_string(g_vm->gagt_extended_status_enabled ? "extended" : "short");
		gagt_normal_string("'.\n");
	}

	else {
		gagt_normal_string("Glk status line can be ");
		gagt_standout_string("extended");
		gagt_normal_string(", or ");
		gagt_standout_string("short");
		gagt_normal_string(".\n");
	}
}


/*
 * gagt_command_print_version_number()
 * gagt_command_version()
 *
 * Print out the Glk library version number.
 */
static void gagt_command_print_version_number(glui32 version) {
	char buffer[64];

	sprintf(buffer, "%u.%u.%u",
	        version >> 16, (version >> 8) & 0xff, version & 0xff);
	gagt_normal_string(buffer);
}

static void gagt_command_version(const char *argument) {
	glui32 version;
	assert(argument);

	gagt_normal_string("This is version ");
	gagt_command_print_version_number(GAGT_PORT_VERSION);
	gagt_normal_string(" of the Glk AGiliTy port.\n");

	version = g_vm->glk_gestalt(gestalt_Version, 0);
	gagt_normal_string("The Glk library version is ");
	gagt_command_print_version_number(version);
	gagt_normal_string(".\n");
}


/*
 * gagt_command_commands()
 *
 * Turn command escapes off.  Once off, there's no way to turn them back on.
 * Commands must be on already to enter this function.
 */
static void gagt_command_commands(const char *argument) {
	assert(argument);

	if (gagt_strcasecmp(argument, "on") == 0) {
		gagt_normal_string("Glk commands are already on.\n");
	}

	else if (gagt_strcasecmp(argument, "off") == 0) {
		g_vm->gagt_commands_enabled = FALSE;
		gagt_normal_string("Glk commands are now off.\n");
	}

	else if (strlen(argument) == 0) {
		gagt_normal_string("Glk commands are ");
		gagt_normal_string(g_vm->gagt_commands_enabled ? "on" : "off");
		gagt_normal_string(".\n");
	}

	else {
		gagt_normal_string("Glk commands can be ");
		gagt_standout_string("on");
		gagt_normal_string(", or ");
		gagt_standout_string("off");
		gagt_normal_string(".\n");
	}
}

/* Glk subcommands and handler functions. */
struct gagt_command_t {
	const char *const command;                      /* Glk subcommand. */
	void (* const handler)(const char *argument);   /* Subcommand handler. */
	const int takes_argument;                       /* Argument flag. */
} ;
typedef const gagt_command_t *gagt_commandref_t;

static void gagt_command_summary(const char *argument);
static void gagt_command_help(const char *argument);

static gagt_command_t GAGT_COMMAND_TABLE[] = {
	{"summary",        gagt_command_summary,        FALSE},
	{"script",         gagt_command_script,         TRUE},
	{"inputlog",       gagt_command_inputlog,       TRUE},
	{"readlog",        gagt_command_readlog,        TRUE},
	{"abbreviations",  gagt_command_abbreviations,  TRUE},
	{"fonts",          gagt_command_fonts,          TRUE},
	{"delays",         gagt_command_delays,         TRUE},
	{"width",          gagt_command_width,          FALSE},
	{"replacements",   gagt_command_replacements,   TRUE},
	{"statusline",     gagt_command_statusline,     TRUE},
	{"version",        gagt_command_version,        FALSE},
	{"commands",       gagt_command_commands,       TRUE},
	{"help",           gagt_command_help,           TRUE},
	{NULL, NULL, FALSE}
};


/*
 * gagt_command_summary()
 *
 * Report all current Glk settings.
 */
static void gagt_command_summary(const char *argument) {
	gagt_commandref_t entry;
	assert(argument);

	/*
	 * Call handlers that have status to report with an empty argument,
	 * prompting each to print its current setting.
	 */
	for (entry = GAGT_COMMAND_TABLE; entry->command; entry++) {
		if (entry->handler == gagt_command_summary
		        || entry->handler == gagt_command_help)
			continue;

		entry->handler("");
	}
}


/*
 * gagt_command_help()
 *
 * Document the available Glk cmds.
 */
static void gagt_command_help(const char *cmd) {
	gagt_commandref_t entry, matched;
	assert(cmd);

	if (strlen(cmd) == 0) {
		gagt_normal_string("Glk cmds are");
		for (entry = GAGT_COMMAND_TABLE; entry->command; entry++) {
			gagt_commandref_t next;

			next = entry + 1;
			gagt_normal_string(next->command ? " " : " and ");
			gagt_standout_string(entry->command);
			gagt_normal_string(next->command ? "," : ".\n\n");
		}

		gagt_normal_string("Glk cmds may be abbreviated, as long as"
		                   " the abbreviation is unambiguous.  Use ");
		gagt_standout_string("glk help");
		gagt_normal_string(" followed by a Glk cmd name for help on that"
		                   " cmd.\n");
		return;
	}

	matched = NULL;
	for (entry = GAGT_COMMAND_TABLE; entry->command; entry++) {
		if (gagt_strncasecmp(cmd, entry->command, strlen(cmd)) == 0) {
			if (matched) {
				gagt_normal_string("The Glk cmd ");
				gagt_standout_string(cmd);
				gagt_normal_string(" is ambiguous.  Try ");
				gagt_standout_string("glk help");
				gagt_normal_string(" for more information.\n");
				return;
			}
			matched = entry;
		}
	}
	if (!matched) {
		gagt_normal_string("The Glk cmd ");
		gagt_standout_string(cmd);
		gagt_normal_string(" is not valid.  Try ");
		gagt_standout_string("glk help");
		gagt_normal_string(" for more information.\n");
		return;
	}

	if (matched->handler == gagt_command_summary) {
		gagt_normal_string("Prints a summary of all the current Glk AGiliTy"
		                   " settings.\n");
	}

	else if (matched->handler == gagt_command_script) {
		gagt_normal_string("Logs the game's output to a file.\n\nUse ");
		gagt_standout_string("glk script on");
		gagt_normal_string(" to begin logging game output, and ");
		gagt_standout_string("glk script off");
		gagt_normal_string(" to end it.  Glk AGiliTy will ask you for a file"
		                   " when you turn scripts on.\n");
	}

	else if (matched->handler == gagt_command_inputlog) {
		gagt_normal_string("Records the cmds you type into a game.\n\nUse ");
		gagt_standout_string("glk inputlog on");
		gagt_normal_string(", to begin recording your cmds, and ");
		gagt_standout_string("glk inputlog off");
		gagt_normal_string(" to turn off input logs.  You can play back"
		                   " recorded cmds into a game with the ");
		gagt_standout_string("glk readlog");
		gagt_normal_string(" cmd.\n");
	}

	else if (matched->handler == gagt_command_readlog) {
		gagt_normal_string("Plays back cmds recorded with ");
		gagt_standout_string("glk inputlog on");
		gagt_normal_string(".\n\nUse ");
		gagt_standout_string("glk readlog on");
		gagt_normal_string(".  cmd play back stops at the end of the"
		                   " file.  You can also play back cmds from a"
		                   " text file created using any standard editor.\n");
	}

	else if (matched->handler == gagt_command_abbreviations) {
		gagt_normal_string("Controls abbreviation expansion.\n\nGlk AGiliTy"
		                   " automatically expands several standard single"
		                   " letter abbreviations for you; for example, \"x\""
		                   " becomes \"examine\".  Use ");
		gagt_standout_string("glk abbreviations on");
		gagt_normal_string(" to turn this feature on, and ");
		gagt_standout_string("glk abbreviations off");
		gagt_normal_string(" to turn it off.  While the feature is on, you"
		                   " can bypass abbreviation expansion for an"
		                   " individual game cmd by prefixing it with a"
		                   " single quote.\n");
	}

	else if (matched->handler == gagt_command_fonts) {
		gagt_normal_string("Controls the way Glk AGiliTy uses fonts.\n\n"
		                   "AGT games normally assume 80x25 monospaced font"
		                   " displays.  Glk can often use proportional fonts."
		                   "  To try to improve text display, Glk AGiliTy will"
		                   " attempt to automatically detect when game text"
		                   " can be displayed safely in a proportional font,"
		                   " and when fixed width fonts are required.  For"
		                   " some games, however, you may need to override"
		                   " it.  Use ");
		gagt_standout_string("glk fonts automatic");
		gagt_normal_string(", ");
		gagt_standout_string("glk fonts proportional");
		gagt_normal_string(", and ");
		gagt_standout_string("glk fonts fixed");
		gagt_normal_string(" to switch between Glk AGiliTy font modes.\n");
	}

	else if (matched->handler == gagt_command_delays) {
		gagt_normal_string("Shortens, or eliminates, AGT game delays.\n\nUse ");
		gagt_standout_string("glk delays full");
		gagt_normal_string(", ");
		gagt_standout_string("glk delays short");
		gagt_normal_string(", or ");
		gagt_standout_string("glk delays none");
		gagt_normal_string(".  In Glk AGiliTy, you can also end an AGT game's"
		                   " delay early, by pressing Space while the game is"
		                   " delaying.\n");
	}

	else if (matched->handler == gagt_command_width) {
		gagt_normal_string("Prints the screen width available for fixed font"
		                   " display.\n\nEven though Glk AGiliTy tries to handle"
		                   " issues surrounding proportional font displays for"
		                   " you automatically, some game elements may still"
		                   " need to display in fixed width fonts.  These"
		                   " elements will be happiest if the available screen"
		                   " width is at least 80 columns.\n");
	}

	else if (matched->handler == gagt_command_replacements) {
		gagt_normal_string("Controls game text scanning and replacement.\n\n"
		                   "Glk AGiliTy can monitor the game's output, and"
		                   " replace a few selected standard messages with"
		                   " equivalents, printed using a style that stands"
		                   " out better in Glk displays.  Use ");
		gagt_standout_string("glk replacements on");
		gagt_normal_string(" to turn this feature on, and ");
		gagt_standout_string("glk replacements off");
		gagt_normal_string(" to turn it off.\n");
	}

	else if (matched->handler == gagt_command_statusline) {
		gagt_normal_string("Controls the Glk AGiliTy status line display.\n\n"
		                   "Use ");
		gagt_standout_string("glk statusline extended");
		gagt_normal_string(" to display a full, two line status display, and ");
		gagt_standout_string("glk statusline short");
		gagt_normal_string(" for a single line status display.\n");
	}

	else if (matched->handler == gagt_command_version) {
		gagt_normal_string("Prints the version numbers of the Glk library"
		                   " and the Glk AGiliTy port.\n");
	}

	else if (matched->handler == gagt_command_commands) {
		gagt_normal_string("Turn off Glk cmds.\n\nUse ");
		gagt_standout_string("glk cmds off");
		gagt_normal_string(" to disable all Glk cmds, including this one."
		                   "  Once turned off, there is no way to turn Glk"
		                   " cmds back on while inside the game.\n");
	}

	else if (matched->handler == gagt_command_help)
		gagt_command_help("");

	else
		gagt_normal_string("There is no help available on that Glk cmd."
		                   "  Sorry.\n");
}


/*
 * gagt_command_escape()
 *
 * This function is handed each input line.  If the line contains a specific
 * Glk port command, handle it and return TRUE, otherwise return FALSE.
 */
static int gagt_command_escape(const char *string) {
	int posn;
	char *string_copy, *cmd, *argument;
	assert(string);

	/*
	 * Return FALSE if the string doesn't begin with the Glk command escape
	 * introducer.
	 */
	posn = strspn(string, "\t ");
	if (gagt_strncasecmp(string + posn, "glk", strlen("glk")) != 0)
		return FALSE;

	/* Take a copy of the string, without any leading space or introducer. */
	string_copy = (char *)gagt_malloc(strlen(string + posn) + 1 - strlen("glk"));
	strcpy(string_copy, string + posn + strlen("glk"));

	/*
	 * Find the subcommand; the first word in the string copy.  Find its end,
	 * and ensure it terminates with a NUL.
	 */
	posn = strspn(string_copy, "\t ");
	cmd = string_copy + posn;
	posn += strcspn(string_copy + posn, "\t ");
	if (string_copy[posn] != '\0')
		string_copy[posn++] = '\0';

	/*
	 * Now find any argument data for the command, ensuring it too terminates
	 * with a NUL.
	 */
	posn += strspn(string_copy + posn, "\t ");
	argument = string_copy + posn;
	posn += strcspn(string_copy + posn, "\t ");
	string_copy[posn] = '\0';

	/*
	 * Try to handle the command and argument as a Glk subcommand.  If it
	 * doesn't run unambiguously, print command usage.  Treat an empty command
	 * as "help".
	 */
	if (strlen(cmd) > 0) {
		gagt_commandref_t entry, matched;
		int matches;

		/*
		 * Search for the first unambiguous table cmd string matching
		 * the cmd passed in.
		 */
		matches = 0;
		matched = NULL;
		for (entry = GAGT_COMMAND_TABLE; entry->command; entry++) {
			if (gagt_strncasecmp(cmd, entry->command, strlen(cmd)) == 0) {
				matches++;
				matched = entry;
			}
		}

		/* If the match was unambiguous, call the command handler. */
		if (matches == 1) {
			gagt_normal_char('\n');
			matched->handler(argument);

			if (!matched->takes_argument && strlen(argument) > 0) {
				gagt_normal_string("[The ");
				gagt_standout_string(matched->command);
				gagt_normal_string(" cmd ignores arguments.]\n");
			}
		}

		/* No match, or the cmd was ambiguous. */
		else {
			gagt_normal_string("\nThe Glk cmd ");
			gagt_standout_string(cmd);
			gagt_normal_string(" is ");
			gagt_normal_string(matches == 0 ? "not valid" : "ambiguous");
			gagt_normal_string(".  Try ");
			gagt_standout_string("glk help");
			gagt_normal_string(" for more information.\n");
		}
	} else {
		gagt_normal_char('\n');
		gagt_command_help("");
	}

	/* The string contained a Glk cmd; return TRUE. */
	free(string_copy);
	return TRUE;
}


/*---------------------------------------------------------------------*/
/*  Glk port input functions                                           */
/*---------------------------------------------------------------------*/

/* Longest line we're going to buffer for input. */
enum { GAGT_INPUTBUFFER_LENGTH = 256 };

/* Table of single-character command abbreviations. */
typedef const struct {
	const char abbreviation;       /* Abbreviation character. */
	const char *const expansion;   /* Expansion string. */
} gagt_abbreviation_t;
typedef gagt_abbreviation_t *gagt_abbreviationref_t;

static gagt_abbreviation_t GAGT_ABBREVIATIONS[] = {
	{'c', "close"},    {'g', "again"},  {'i', "inventory"},
	{'k', "attack"},   {'l', "look"},   {'p', "open"},
	{'q', "quit"},     {'r', "drop"},   {'t', "take"},
	{'x', "examine"},  {'y', "yes"},    {'z', "wait"},
	{'\0', NULL}
};


/*
 * gagt_expand_abbreviations()
 *
 * Expand a few common one-character abbreviations commonly found in other
 * game systems, but not always normal in AGT games.
 */
static void gagt_expand_abbreviations(char *buffer, int size) {
	char *command_, abbreviation;
	const char *expansion;
	gagt_abbreviationref_t entry;
	assert(buffer);

	/* Ignore anything that isn't a single letter command_. */
	command_ = buffer + strspn(buffer, "\t ");
	if (!(strlen(command_) == 1
	        || (strlen(command_) > 1 && isspace(command_[1]))))
		return;

	/* Scan the abbreviations table for a match. */
	abbreviation = g_vm->glk_char_to_lower((unsigned char) command_[0]);
	expansion = NULL;
	for (entry = GAGT_ABBREVIATIONS; entry->expansion; entry++) {
		if (entry->abbreviation == abbreviation) {
			expansion = entry->expansion;
			break;
		}
	}

	/*
	 * If a match found, check for a fit, then replace the character with the
	 * expansion string.
	 */
	if (expansion) {
		if ((int)strlen(buffer) + (int)strlen(expansion) - 1 >= size)
			return;

		memmove(command_ + strlen(expansion) - 1, command_, strlen(command_) + 1);
		memcpy(command_, expansion, strlen(expansion));

		gagt_standout_string("[");
		gagt_standout_char(abbreviation);
		gagt_standout_string(" -> ");
		gagt_standout_string(expansion);
		gagt_standout_string("]\n");
	}
}


/*
 * agt_input()
 *
 * Read a line from the keyboard, allocating space for it using malloc.
 * AGiliTy defines the following for the in_type argument:
 *
 *   in_type: 0=command, 1=number, 2=question, 3=userstr, 4=filename,
 *               5=RESTART,RESTORE,UNDO,QUIT
 *   Negative values are for internal use by the interface (i.e. this module)
 *   and so are free to be defined by the porter.
 *
 * Since it's unclear what use we can make of this information in Glk,
 * for the moment the argument is ignored.  It seems that no-one else
 * uses it, either.
 */
char *agt_input(int in_type) {
	event_t event;
	int length;
	char *buffer;

	/*
	 * Update the current status line display, and flush any pending buffered
	 * output.  Release any suspension of delays.
	 */
	gagt_status_notify();
	gagt_output_flush();
	gagt_delay_resume();

	/* Reset current x, as line input implies a newline. */
	curr_x = 0;

	/* Allocate a line input buffer, allowing 256 characters and a NUL. */
	length = GAGT_INPUTBUFFER_LENGTH + 1;
	buffer = (char *)gagt_malloc(length);

	/*
	 * If we have an input log to read from, use that until it is exhausted.
	 * On end of file, close the stream and resume input from line requests.
	 */
	if (g_vm->gagt_readlog_stream) {
		glui32 chars;

		/* Get the next line from the log stream. */
		chars = g_vm->glk_get_line_stream(g_vm->gagt_readlog_stream, buffer, length);
		if (chars > 0) {
			/* Echo the line just read in input style. */
			g_vm->glk_set_style(style_Input);
			g_vm->glk_put_buffer(buffer, chars);
			g_vm->glk_set_style(style_Normal);

			/*
			 * Convert the string from Glk's ISO 8859 Latin-1 to IBM cp 437,
			 * add to any script, and return it.
			 */
			gagt_iso_to_cp((const uchar *)buffer, (uchar *)buffer);
			if (script_on)
				textputs(scriptfile, buffer);
			return buffer;
		}

		/*
		 * We're at the end of the log stream.  Close it, and then continue
		 * on to request a line from Glk.
		 */
		g_vm->glk_stream_close(g_vm->gagt_readlog_stream, NULL);
		g_vm->gagt_readlog_stream = NULL;
	}

	/* Set this up as a read buffer for the main window, and wait. */
	g_vm->glk_request_line_event(g_vm->gagt_main_window, buffer, length - 1, 0);
	gagt_event_wait(evtype_LineInput, &event);
	if (g_vm->shouldQuit()) {
		g_vm->glk_cancel_line_event(g_vm->gagt_main_window, &event);
		return nullptr;
	}

	/* Terminate the input line with a NUL. */
	assert((int)event.val1 < length);
	buffer[event.val1] = '\0';

	/*
	 * If neither abbreviations nor local commands are enabled, use the data
	 * read above without further massaging.
	 */
	if (g_vm->gagt_abbreviations_enabled || g_vm->gagt_commands_enabled) {
		char *cmd;

		/*
		 * If the first non-space input character is a quote, bypass all
		 * abbreviation expansion and local command recognition, and use the
		 * unadulterated input, less introductory quote.
		 */
		cmd = buffer + strspn(buffer, "\t ");
		if (cmd[0] == '\'') {
			/* Delete the quote with memmove(). */
			memmove(cmd, cmd + 1, strlen(cmd));
		} else {
			/* Check for, and expand, any abbreviated commands. */
			if (g_vm->gagt_abbreviations_enabled)
				gagt_expand_abbreviations(buffer, length);

			/*
			 * Check for standalone "help", then for Glk port special commands;
			 * suppress the interpreter's use of this input for Glk commands.
			 */
			if (g_vm->gagt_commands_enabled) {
				int posn;

				posn = strspn(buffer, "\t ");
				if (gagt_strncasecmp(buffer + posn, "help", strlen("help")) == 0) {
					if (strspn(buffer + posn + strlen("help"), "\t ")
					        == strlen(buffer + posn + strlen("help"))) {
						gagt_display_register_help_request();
					}
				}

				if (gagt_command_escape(buffer)) {
					gagt_display_silence_help_hints();
					buffer[0] = '\0';
					return buffer;
				}
			}
		}
	}

	/*
	 * If there is an input log active, log this input string to it.  Note that
	 * by logging here we get any abbreviation expansions but we won't log glk
	 * special commands, nor any input read from a current open input log.
	 */
	if (g_vm->gagt_inputlog_stream) {
		g_vm->glk_put_string_stream(g_vm->gagt_inputlog_stream, buffer);
		g_vm->glk_put_char_stream(g_vm->gagt_inputlog_stream, '\n');
	}

	/*
	 * Convert from Glk's ISO 8859 Latin-1 to IBM cp 437, and add to any script.
	 */
	gagt_iso_to_cp((const uchar *)buffer, (uchar *)buffer);
	if (script_on)
		textputs(scriptfile, buffer);

	gagt_debug("agt_input", "in_type=%d -> '%s'", in_type, buffer);
	return buffer;
}


/*
 * agt_getkey()
 *
 * Read a single character and return it.  AGiliTy defines the echo_char
 * argument as:
 *
 *   If echo_char=1, echo character. If 0, then the character is not
 *   required to be echoed (and ideally shouldn't be).
 *
 * However, I've found that not all other ports really do this, and in
 * practice it doesn't always look right.  So for Glk, the character is
 * always echoed to the main window.
 */
char agt_getkey(rbool echo_char) {
	event_t event;
	char buffer[3];
	assert(g_vm->glk_stream_get_current());

	/*
	 * Update the current status line display, and flush any pending buffered
	 * output.  Release any suspension of delays.
	 */
	gagt_status_notify();
	gagt_output_flush();
	gagt_delay_resume();

	/* Reset current x, as echoed character input implies a newline. */
	curr_x = 0;

	/*
	 * If we have an input log to read from, use that as above until it is
	 * exhausted.  We take just the first character of a given line.
	 */
	if (g_vm->gagt_readlog_stream) {
		glui32 chars;
		char logbuffer[GAGT_INPUTBUFFER_LENGTH + 1];

		/* Get the next line from the log stream. */
		chars = g_vm->glk_get_line_stream(g_vm->gagt_readlog_stream,
		                                  logbuffer, sizeof(logbuffer));
		if (chars > 0) {
			/* Take just the first character, adding a newline if necessary. */
			buffer[0] = logbuffer[0];
			buffer[1] = buffer[0] == '\n' ? '\0' : '\n';
			buffer[2] = '\0';

			/* Echo the character just read in input style. */
			g_vm->glk_set_style(style_Input);
			g_vm->glk_put_string(buffer);
			g_vm->glk_set_style(style_Normal);

			/*
			 * Convert from Glk's ISO 8859 Latin-1 to IBM cp 437, add to any
			 * script, and return the character.
			 */
			gagt_iso_to_cp((const uchar *)buffer, (uchar *)buffer);
			if (script_on)
				textputs(scriptfile, buffer);
			return buffer[0];
		}

		/*
		 * We're at the end of the log stream.  Close it, and then continue
		 * on to request a character from Glk.
		 */
		g_vm->glk_stream_close(g_vm->gagt_readlog_stream, NULL);
		g_vm->gagt_readlog_stream = NULL;
	}

	/*
	 * Request a single character from main window, and wait.  Ignore non-
	 * ASCII codes that Glk returns for special function keys; we just want
	 * one ASCII return value.  (Glk does treat Return as a special key,
	 * though, and we want to pass that back as ASCII return.)
	 */
	do {
		g_vm->glk_request_char_event(g_vm->gagt_main_window);
		gagt_event_wait(evtype_CharInput, &event);
	} while (event.val1 > BYTE_MAX_VAL && event.val1 != keycode_Return);

	/*
	 * Save the character into a short string buffer, converting Return
	 * to newline, and adding a newline if not Return.
	 */
	buffer[0] = event.val1 == keycode_Return ? '\n' : event.val1;
	buffer[1] = buffer[0] == '\n' ? '\0' : '\n';
	buffer[2] = '\0';

	/* If there is an input log active, log this input string to it. */
	if (g_vm->gagt_inputlog_stream)
		g_vm->glk_put_string_stream(g_vm->gagt_inputlog_stream, buffer);

	/*
	 * No matter what echo_char says, as it happens, the output doesn't look
	 * great if we don't write out the character, and also a newline (c.f.
	 * the "Yes/No" confirmation of the QUIT command)...
	 */
	g_vm->glk_set_style(style_Input);
	g_vm->glk_put_string(buffer);
	g_vm->glk_set_style(style_Normal);

	/*
	 * Convert from Glk's ISO 8859 Latin-1 to IBM cp 437, and add to any
	 * script.
	 */
	gagt_iso_to_cp((const uchar *)buffer, (uchar *)buffer);
	if (script_on)
		textputs(scriptfile, buffer);

	gagt_debug("agt_getkey", "echo_char=%d -> '%c'",
	           echo_char, buffer[0] == '\n' ? '$' : buffer[0]);
	return buffer[0];
}


/*---------------------------------------------------------------------*/
/*  Glk port event functions                                           */
/*---------------------------------------------------------------------*/

/*
 * We have some clever atexit() finalizer handling for exit() calls that
 * come from the core interpreter.  However, an exit() call could also come
 * from Glk; Xkill for example.  To tell the difference, we'll have the
 * event wait functions set a flag to indicate when g_vm->glk_select() is active.
 */
static int gagt_in_glk_select = FALSE;

/*
 * gagt_event_wait_2()
 * gagt_event_wait()
 *
 * Process Glk events until one of the expected type, or types, arrives.
 * Return the event of that type.
 */
static void gagt_event_wait_2(glui32 wait_type_1, glui32 wait_type_2, event_t *event) {
	assert(event);

	do {
		gagt_in_glk_select = TRUE;
		g_vm->glk_select(event);
		gagt_in_glk_select = FALSE;

		switch (event->type) {
		case evtype_Arrange:
		case evtype_Redraw:
			gagt_status_redraw();
			break;
		case evtype_Quit:
			return;
		default:
			break;
		}
	} while (!(event->type == (EvType)wait_type_1 || event->type == (EvType)wait_type_2));
}

static void gagt_event_wait(glui32 wait_type, event_t *event) {
	assert(event);
	gagt_event_wait_2(wait_type, evtype_None, event);
}


/*
 * gagt_event_in_glk_select()
 *
 * Return TRUE if we're currently awaiting an event in g_vm->glk_select().  Used
 * by the finalizer to distinguish interpreter and glk exit() calls.
 */
static int gagt_event_in_glk_select() {
	return gagt_in_glk_select;
}


/*---------------------------------------------------------------------*/
/*  Miscellaneous Glk port startup and options functions               */
/*---------------------------------------------------------------------*/

/*
 * Default screen height and width, and also a default status width for
 * use with Glk libraries that don't support separate windows.
 */
static const int GAGT_DEFAULT_SCREEN_WIDTH = 80,
                 GAGT_DEFAULT_SCREEN_HEIGHT = 25,
                 GAGT_DEFAULT_STATUS_WIDTH = 76;


/*
 * agt_option()
 *
 * Platform-specific setup and options handling.  AGiliTy defines the
 * arguments and options as:
 *
 *   If setflag is 0, then the option was prefixed with NO_. Return 1 if
 *   the option is recognized.
 *
 * The Glk port has no options file handling, so none of this is
 * implemented here.
 */
rbool agt_option(int optnum, char *optstr[], rbool setflag) {
	gagt_debug("agt_option", "optnum=%d, optstr=%s, setflag=%d",
	           optnum, optstr[0], setflag);
	return 0;
}


/*
 * agt_globalfile()
 *
 * Global options file handle handling.  For now, this is a stub, since
 * there is no .agilrc for this port.
 */
genfile agt_globalfile(int fid) {
	gagt_debug("agt_globalfile", "fid=%d", fid);
	return badfile(fCFG);
}


/*
 * init_interface()
 *
 * General initialization for the module; sets some variables, and creates
 * the Glk windows to work in.  Called from the AGiliTy main().
 */
void init_interface() {
	glui32 status_height;

	/*
	 * Begin with some default values for global variables that this module
	 * is somehow responsible for.
	 */
	script_on = center_on = par_fill_on = FALSE;
	scriptfile = badfile(fSCR);
	debugfile = nullptr; // stderr;

	/*
	 * Set up AGT-specific Glk styles.  This needs to be done before any Glk
	 * window is opened.
	 */
	gagt_init_user_styles();

	/*
	 * Create the main game window.  The main game window creation must succeed.
	 * If it fails, we'll return, and the caller can detect this by looking
	 * for a NULL main window.
	 */
	g_vm->gagt_main_window = g_vm->glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	if (!g_vm->gagt_main_window)
		return;

	/*
	 * Set the main window to be the default window, for convenience.  We do
	 * this again in glk_main() -- this call is here just in case this version
	 * of init_interface() is ever called by AGiliTy's main.
	 */
	g_vm->glk_set_window(g_vm->gagt_main_window);

	/*
	 * Screen height is something we don't use.  Linux Xglk returns dimensions
	 * of 0x0 for text buffer windows, so we can't measure the main window
	 * height anyway.  But... the height does come into play in AGiliTy's
	 * agil.c, when the interpreter is deciding how to output game titles, and
	 * how much of its own subsequent verbiage to output.  This gives us a
	 * problem, since this "verbiage" is stuff we look for and replace with
	 * our own special text.  So... sigh, set 25, and try to cope in the
	 * special text we've set up with all the variations that ensue.
	 *
	 * Screen width does get used, but so, so many games, and for that matter
	 * the interpreter itself, assume 80 chars, so it's simplest just to set,
	 * and keep, this, and put up with the minor odd effects (making it match
	 * status_width, or making it something like MAX_INT to defeat the game's
	 * own wrapping, gives a lot of odder effects, trust me on this one...).
	 */
	screen_width = GAGT_DEFAULT_SCREEN_WIDTH;
	screen_height = GAGT_DEFAULT_SCREEN_HEIGHT;

	/*
	 * Create a status window, with one or two lines as selected by user
	 * options or flags.  We can live without a status window if we have to.
	 */
	status_height = g_vm->gagt_extended_status_enabled ? 2 : 1;
	g_vm->glk_stylehint_set(wintype_TextGrid, style_User1, stylehint_ReverseColor, 1);
	g_vm->gagt_status_window = g_vm->glk_window_open(g_vm->gagt_main_window,
	                     winmethod_Above | winmethod_Fixed,
	                     status_height, wintype_TextGrid, 0);
	if (g_vm->gagt_status_window) {
		/*
		 * Call gagt_status_redraw() to set the interpreter's status_width
		 * variable initial value.
		 */
		gagt_status_redraw();
	} else {
		/*
		 * No status window, so set a suitable default status width.  In this
		 * case, we're using a value four characters less than the set screen
		 * width.  AGiliTy's status line code will fill to this width with
		 * justified text, and we add two characters of bracketing when
		 * displaying status lines for Glks that don't support separate windows,
		 * making a total of 78 characters, which should be fairly standard.
		 */
		status_width = GAGT_DEFAULT_STATUS_WIDTH;
	}

	agt_clrscr();
}


/*---------------------------------------------------------------------*/
/*  Replacement interface.c functions                                  */
/*---------------------------------------------------------------------*/

/* Get_user_file() type codes. */
enum {
	AGT_SCRIPT = 0,
	AGT_SAVE = 1,
	AGT_RESTORE = 2,
	AGT_LOG_READ = 3,
	AGT_LOG_WRITE = 4
};

/* Longest acceptable filename. */
enum { GAGT_MAX_PATH = 1024 };


#ifdef GLK_ANSI_ONLY
/*
 * gagt_confirm()
 *
 * Print a confirmation prompt, and read a single input character, taking
 * only [YyNn] input.  If the character is 'Y' or 'y', return TRUE.
 *
 * This function is only required for the ANSI version of get_user_file().
 */
static int
gagt_confirm(const char *prompt) {
	event_t event;
	unsigned char response;
	assert(prompt);

	/*
	 * Print the confirmation prompt, in a style that hints that it's from the
	 * interpreter, not the game.
	 */
	gagt_standout_string(prompt);

	/* Wait for a single 'Y' or 'N' character response. */
	response = ' ';
	do {
		g_vm->glk_request_char_event(g_vm->gagt_main_window);
		gagt_event_wait(evtype_CharInput, &event);

		if (event.val1 <= BYTE_MAX_VAL)
			response = g_vm->glk_char_to_upper(event.val1);
	} while (!(response == 'Y' || response == 'N'));

	/* Echo the confirmation response, and a blank line. */
	g_vm->glk_set_style(style_Input);
	g_vm->glk_put_string(response == 'Y' ? "Yes" : "No");
	g_vm->glk_set_style(style_Normal);
	g_vm->glk_put_string("\n");

	return response == 'Y';
}
#endif


/*
 * gagt_get_user_file()
 *
 * Alternative versions of functions to get a file name from the user, and
 * return a file stream structure.  These functions are front-ended by the
 * main get_user_file() function, which first converts the AGT file type
 * into Glk usage and filemode, and also a mode for fopen()/fdopen().
 *
 * The ANSI version of the function prompts for the file using the simple
 * method of querying the user through input in the main window.  It then
 * constructs a file stream around the path entered, and returns it.
 *
 * The non-ANSI, Glk version is more sneaky.  It prompts for a file using
 * Glk's functions to get filenames by prompt, file selection dialog, or
 * whatever.  Then it attempts to uncover which file descriptor Glk opened
 * its file on, dup's it, closes the Glk stream, and returns a file stream
 * built on this file descriptor.  This is all highly non-ANSI, requiring
 * dup() and fdopen(), and making some assumptions about the way that dup,
 * open, and friends work.  It works on Linux, and on Mac (CodeWarrior).
 * It may also work for you, but if it doesn't, or if your system lacks
 * things like dup or fdopen, define g_vm->glk_ANSI_ONLY and use the safe version.
 *
 * If GARGLK is used, non-ansi version calls garglk_fileref_get_name()
 * instead, and opens a file the highly portable way, but still with a
 * Glkily nice prompt dialog.
 */
#ifdef GLK_ANSI_ONLY
static genfile
gagt_get_user_file(glui32 usage, glui32 fmode, const char *fdtype) {
	char filepath[GAGT_MAX_PATH];
	event_t event;
	int index, all_spaces;
	genfile retfile;
	assert(fdtype);

	/* Prompt in a similar way to Glk. */
	switch (usage) {
	case fileusage_SavedGame:
		gagt_normal_string("Enter saved game");
		break;

	case fileusage_Transcript:
		gagt_normal_string("Enter transcript file");
		break;

	case fileusage_InputRecord:
		gagt_normal_string("Enter command record file");
		break;
	}
	switch (fmode) {
	case filemode_Read:
		gagt_normal_string(" to load: ");
		break;

	case filemode_Write:
		gagt_normal_string(" to store: ");
		break;
	}

	/* Get the path to the file from the user. */
	g_vm->glk_request_line_event(g_vm->gagt_main_window, filepath, sizeof(filepath) - 1, 0);
	gagt_event_wait(evtype_LineInput, &event);

	/* Terminate the file path with a NUL. */
	assert(event.val1 < sizeof(filepath));
	filepath[event.val1] = '\0';

	/* Reject file paths that only contain any whitespace characters. */
	all_spaces = TRUE;
	for (index = 0; index < strlen(filepath); index++) {
		if (!isspace(filepath[index])) {
			all_spaces = FALSE;
			break;
		}
	}
	if (all_spaces)
		return badfile(fSAV);

	/* Confirm overwrite of any existing file. */
	if (fmode == filemode_Write) {
		genfile file;

		file = fopen(filepath, "r");
		if (file) {
			fclose(file);

			if (!gagt_confirm("Overwrite existing file? [y/n] "))
				return badfile(fSAV);
		}
	}

	/* Open and return a FILE* stream, or badfile if this fails. */
	retfile = fopen(filepath, fdtype);
	return retfile ? retfile : badfile(fSAV);
}
#endif

#ifndef GLK_ANSI_ONLY
static genfile gagt_get_user_file(glui32 usage, glui32 fmode, const char *fdtype) {
	frefid_t fileref;
	genfile retfile;
	assert(fdtype);

	/* Try to get a Glk file reference with these attributes. */
	fileref = g_vm->glk_fileref_create_by_prompt(usage, (FileMode)fmode, 0);
	if (!fileref)
		return badfile(fSAV);

	/*
	 * Reject the file reference if we're expecting to read from it,
	 * and the referenced file doesn't exist.
	 */
	if (fmode == filemode_Read && !g_vm->glk_fileref_does_file_exist(fileref)) {
		g_vm->glk_fileref_destroy(fileref);
		return badfile(fSAV);
	}

	/*
	 * Now, it gets ugly.  Glk assumes that the interpreter will do all of
	 * its reading and writing using the Glk streams read/write functions.
	 * It won't; at least, not without major surgery.  So here we're going
	 * to do some dangerous stuff...
	 *
	 * Since a Glk stream is opaque, it's hard to tell what the underlying
	 * file descriptor is for it.  We can get it if we want to play around
	 * in the internals of the strid_t structure, but it's unpleasant.
	 * The alternative is, arguably, no more pleasant, but it makes for
	 * (perhaps) more portable code.  What we'll do is to dup a file, then
	 * immediately close it, and call g_vm->glk_stream_open_file().  The open()
	 * in g_vm->glk_stream_open_file() will return the same file descriptor number
	 * that we just close()d (in theory...).  This makes the following two
	 * major assumptions:
	 *
	 *  1) g_vm->glk_stream_open_file() opens precisely one file with open()
	 *  2) open() always uses the lowest available file descriptor number,
	 *     like dup()
	 *
	 * Believe it or not, this is better than the alternatives.  There is
	 * no Glk function to return the filename from a frefid_t, and it
	 * moves about in different Glk libraries so we can't just take it
	 * from a given offset.  And there is no Glk function to return the
	 * underlying file descriptor or FILE* from a Glk stream either. :-(
	 */

#ifdef GARGLK
	retfile = fopen(g_vm->garglk_fileref_get_name(fileref), fdtype);
#else
	strid_t stream;
	int tryfd, glkfd, dupfd, retfd;

	/* So, start by dup()'ing the first file descriptor we can, ... */
	glkfd = -1;
	for (tryfd = 0; tryfd < FD_SETSIZE; tryfd++) {
		glkfd = fcntl(tryfd, F_DUPFD, 0);
		if (glkfd != -1)
			break;
	}
	if (tryfd >= FD_SETSIZE) {
		g_vm->glk_fileref_destroy(fileref);
		return badfile(fSAV);
	}

	/* ...then closing it, ... */
	close(glkfd);

	/* ...now open the Glk stream, assuming it opens on file 'glkfd', ... */
	stream = g_vm->glk_stream_open_file(fileref, fmode, 0);
	if (!stream) {
		g_vm->glk_fileref_destroy(fileref);
		return badfile(fSAV);
	}

	/* ...dup() the Glk file onto another file descriptor, ... */
	dupfd = fcntl(glkfd, F_DUPFD, 0);
	assert(dupfd != -1);

	/* ...close and destroy the Glk edifice for this file, ... */
	g_vm->glk_stream_close(stream, NULL);
	g_vm->glk_fileref_destroy(fileref);

	/* ...for neatness, dup() back to the old Glk file descriptor, ... */
	retfd = fcntl(dupfd, F_DUPFD, 0);
	assert(retfd != -1 && retfd == glkfd);
	close(dupfd);

	/* ...and finally, open a FILE* stream onto the return descriptor. */
	retfile = fdopen(retfd, fdtype);
	if (!retfile)
		return badfile(fSAV);
#endif /* GARGLK */

	/*
	 * The result of all of this should now be that retfile is a FILE* wrapper
	 * round a file descriptor open on a file indicated by the user through Glk.
	 * Return it.
	 */
	return retfile;
}
#endif


/*
 * get_user_file()
 *
 * Get a file name from the user, and return the file stream structure.
 * This is a front-end to ANSI and non-ANSI variants of the function.
 */
genfile get_user_file(int type) {
	glui32 usage = 0, fmode = 0;
	const char *fdtype;
	genfile retfile;

	gagt_output_flush();

	/* Map AGiliTy type to Glk usage and filemode. */
	switch (type) {
	case AGT_SCRIPT:
		usage = fileusage_Transcript;
		fmode = filemode_Write;
		break;

	case AGT_SAVE:
		usage = fileusage_SavedGame;
		fmode = filemode_Write;
		break;

	case AGT_RESTORE:
		usage = fileusage_SavedGame;
		fmode = filemode_Read;
		break;

	case AGT_LOG_READ:
		usage = fileusage_InputRecord;
		fmode = filemode_Read;
		break;

	case AGT_LOG_WRITE:
		usage = fileusage_InputRecord;
		fmode = filemode_Write;
		break;

	default:
		gagt_fatal("GLK: Unknown file type encountered");
		gagt_exit();
	}

	/* From these, determine a mode type for the f[d]open() call. */
	if (fmode == filemode_Write)
		fdtype = usage == fileusage_SavedGame ? "wb" : "w";
	else
		fdtype = usage == fileusage_SavedGame ? "rb" : "r";

	/* Get a file stream from these using the appropriate function. */
	retfile = gagt_get_user_file(usage, fmode, fdtype);

	gagt_debug("get_user_file", "type=%d -> %p", type, retfile);
	return retfile;
}


/*
 * set_default_filenames()
 *
 * Set defaults for last save, log, and script filenames.
 */
void set_default_filenames(fc_type fc) {
	/*
	 * There is nothing to do in this function, since Glk has its own ideas on
	 * default names for files obtained with a prompt.
	 */
	gagt_debug("set_default_filenames", "fc=%p", fc);
}


/*---------------------------------------------------------------------*/
/*  Functions intercepted by link-time wrappers                        */
/*---------------------------------------------------------------------*/

/*
 * __wrap_toupper()
 * __wrap_tolower()
 *
 * Wrapper functions around toupper(), tolower(), and fatal().  The Linux
 * linker's --wrap option will convert calls to mumble() to __wrap_mumble()
 * if we give it the right options.  We'll use this feature to translate
 * all toupper() and tolower() calls in the interpreter code into calls to
 * Glk's versions of these functions.
 *
 * It's not critical that we do this.  If a linker, say a non-Linux one,
 * won't do --wrap, then just do without it.  It's unlikely that there
 * will be much noticeable difference.
 */
int __wrap_toupper(int ch) {
	unsigned char uch;

	uch = g_vm->glk_char_to_upper((unsigned char) ch);
	return (int) uch;
}

int __wrap_tolower(int ch) {
	unsigned char lch;

	lch = g_vm->glk_char_to_lower((unsigned char) ch);
	return (int) lch;
}


/*---------------------------------------------------------------------*/
/*  Replacements for AGiliTy main() and options parsing                */
/*---------------------------------------------------------------------*/

/* External declaration of interface.c's set default options function. */
extern void set_default_options();


/*
 * gagt_startup_code()
 * gagt_main()
 *
 * Together, these functions take the place of the original AGiliTy main().
 * The first one is called from glkunix_startup_code().  The second is called
 * from glk_main(), and does the real work of running the game.
 */
bool gagt_startup_code() {
	/* Make the mandatory call for initialization. */
	set_default_options();

	/* All startup options were handled successfully. */
	return TRUE;
}

static void gagt_main() {
	fc_type fc;

	/*
	 * Initialize the interface.  As it happens, init_interface() is in our
	 * module here (above), and ignores argc and argv, but since the main() in
	 * AGiliTy passes them, we'll do so here, just in case we ever want to go
	 * back to using AGiliTy's main() function.
	 *
	 * init_interface() can fail if there is a problem creating the main
	 * window.  As it doesn't return status, we have to detect this by checking
	 * that g_vm->gagt_main_window is not NULL.
	 */
	init_interface();
	if (!g_vm->gagt_main_window) {
		gagt_fatal("GLK: Can't open main window");
		gagt_exit();
	}
	g_vm->glk_window_clear(g_vm->gagt_main_window);
	g_vm->glk_set_window(g_vm->gagt_main_window);
	g_vm->glk_set_style(style_Normal);

	/*
	 * Create a game file context, and try to ensure it will open successfully
	 * in run_game().
	 */
	fc = init_file_context(g_vm->gagt_gamefile, fDA1);
	if (!(gagt_workround_fileexist(fc, fAGX)
	        || gagt_workround_fileexist(fc, fDA1))) {
		if (g_vm->gagt_status_window)
			g_vm->glk_window_close(g_vm->gagt_status_window, NULL);
		gagt_header_string("Glk AGiliTy Error\n\n");
		gagt_normal_string("Can't find or open game '");
		gagt_normal_string(g_vm->gagt_gamefile);
		gagt_normal_char('\'');
		gagt_normal_char('\n');
		gagt_exit();
	}

	/*
	 * Run the game interpreter in AGiliTy.  run_game() releases the file
	 * context, so we don't have to, don't want to, and shouldn't.
	 */
	run_game(fc);

	/*
	 * Handle any updated status, and flush all remaining buffered output;
	 * this also frees all malloc'ed memory in the buffers.
	 */
	gagt_status_notify();
	gagt_output_flush();

	/*
	 * Free any temporary memory that may have been used by status line
	 * functions.
	 */
	gagt_status_cleanup();

	/* Close any open transcript, input log, and/or read log. */
	if (g_vm->gagt_transcript_stream) {
		g_vm->glk_stream_close(g_vm->gagt_transcript_stream, NULL);
		g_vm->gagt_transcript_stream = NULL;
	}
	if (g_vm->gagt_inputlog_stream) {
		g_vm->glk_stream_close(g_vm->gagt_inputlog_stream, NULL);
		g_vm->gagt_inputlog_stream = NULL;
	}
	if (g_vm->gagt_readlog_stream) {
		g_vm->glk_stream_close(g_vm->gagt_readlog_stream, NULL);
		g_vm->gagt_readlog_stream = NULL;
	}
}


/*---------------------------------------------------------------------*/
/*  Linkage between Glk entry/exit calls and the AGiliTy interpreter   */
/*---------------------------------------------------------------------*/

/*
 * Safety flags, to ensure we always get startup before main, and that
 * we only get a call to main once.
 */
static int gagt_startup_called = FALSE,
           gagt_main_called = FALSE;

/*
 * We try to catch calls to exit() from the interpreter, and redirect them
 * to g_vm->glk_exit().  To help tell these calls from a call to exit() from
 * g_vm->glk_exit() itself, we need to monitor when interpreter code is running,
 * and when not.
 */
static int gagt_agility_running = FALSE;


/*
 * gagt_finalizer()
 *
 * ANSI atexit() handler.  This is the first part of trying to catch and re-
 * direct the calls the core AGiliTy interpreter makes to exit() -- we really
 * want it to call g_vm->glk_exit(), but it's hard to achieve.  There are three
 * basic approaches possible, and all have drawbacks:
 *
 *   o #define exit to gagt_something, and provide the gagt_something()
 *     function.  This type of macro definition is portable for the most
 *     part, but tramples the code badly, and messes up the build of the
 *     non-interpreter "support" binaries.
 *   o Use ld's --wrap to wrapper exit.  This only works with Linux's linker
 *     and so isn't at all portable.
 *   o Register an exit handler with atexit(), and try to cope in it after
 *     exit() has been called.
 *
 * Here we try the last of these.  The one sticky part of it is that in our
 * exit handler we'll want to call g_vm->glk_exit(), which will in all likelihood
 * call exit().  And multiple calls to exit() from a program are "undefined".
 *
 * In practice, C runtimes tend to do one of three things: they treat the
 * exit() call from the exit handler as if it was a return; they recurse
 * indefinitely through the hander; or they do something ugly (abort, for
 * example).  The first of these is fine, ideal in fact, and seems to be the
 * Linux and SVR4 behavior.  The second we can avoid with a flag.  The last
 * is the problem case, seen only with SVR3 (and even then, it occurs only
 * on program exit, after everything's cleaned up, and for that matter only
 * on abnormal exit).
 *
 * Note that here we're not expecting to get a call to this routine, and if
 * we do, and interpreter code is still running, it's a sign that we need
 * to take actions we'd hoped not to have to take.
 */
void gagt_finalizer() {
	/*
	 * If interpreter code is still active, and we're not in a g_vm->glk_select(),
	 * the core interpreter code called exit().  Handle cleanup.
	 */
	if (gagt_agility_running && !gagt_event_in_glk_select()) {
		event_t event;

		/*
		 * If we have a main window, try to update status (which may go to the
		 * status window, or to the main window) and flush any pending buffered
		 * output.
		 */
		if (g_vm->gagt_main_window) {
			gagt_status_notify();
			gagt_output_flush();
		}

		/*
		 * Clear the flag to avoid recursion, and call g_vm->glk_exit() to clean up
		 * Glk and terminate.  This is the call that probably re-calls exit(),
		 * and thus prods "undefined" bits of the C runtime, so we'll make it
		 * configurable and overrideable for problem cases.
		 */
		gagt_agility_running = FALSE;

		/*
		 * We've decided not to take the dangerous route.
		 *
		 * In that case, providing we have a main window, fake a Glk-like-ish
		 * hit-any-key-and-wait message using a simple string in the main
		 * window.  Not great, but usable where we're forced into bypassing
		 * g_vm->glk_exit().  If we have no main window, there's no point in doing
		 * anything more.
		 */
		if (g_vm->gagt_main_window) {
			g_vm->glk_cancel_char_event(g_vm->gagt_main_window);
			g_vm->glk_cancel_line_event(g_vm->gagt_main_window, NULL);

			g_vm->glk_set_style(style_Alert);
			g_vm->glk_put_string("\n\nHit any key to exit.\n");
			g_vm->glk_request_char_event(g_vm->gagt_main_window);
			gagt_event_wait(evtype_CharInput, &event);
		}
	}
}


/*
 * gagt_exit()
 *
 * g_vm->glk_exit() local wrapper.  This is the second part of trying to catch
 * and redirect calls to exit().  g_vm->glk_finalizer() above needs to know that
 * we called g_vm->glk_exit() already from here, so it doesn't try to do it again.
 */
static void gagt_exit() {
	assert(gagt_agility_running);

	/*
	 * Clear the running flag to neutralize gagt_finalizer(), throw out any
	 * buffered output data, and then call the real g_vm->glk_exit().
	 */
	gagt_agility_running = FALSE;
	gagt_output_delete();
	g_vm->glk_exit();
}


/*
 * __wrap_exit()
 *
 * Exit() wrapper where a linker does --wrap.  This is the third part of
 * trying to catch and redirect calls to exit().
 *
 * This function is for use only with IFP, and avoids a nasty attempt at
 * reusing a longjmp buffer.   IFP will redirect calls to exit() into
 * g_vm->glk_exit() as a matter of course.  It also handles atexit(), and we've
 * registered a function with atexit() that calls g_vm->glk_exit(), and
 * IFP redirects g_vm->glk_exit() to be an effective return from glk_main().  At
 * that point it calls finalizers.  So without doing something special for
 * IFP, we'll find ourselves calling g_vm->glk_exit() twice -- once as the IFP
 * redirected exit(), and once from our finalizer.  Two returns from the
 * function glk_main() is a recipe for unpleasantness.
 *
 * As IFP is Linux-only, at present, --wrap will always be available to IFP
 * plugin builds.  So here, we'll wrap exit() before IFP can get to it, and
 * handle it safely.  For non-IFP/non-wrap links, this is just an unused
 * function definition, and can be safely ignored...
 */
void __wrap_exit(int status) {
	assert(gagt_agility_running);

	/*
	 * In an IFP plugin, only the core interpreter code could have called exit()
	 * here -- we don't, and IFP redirects g_vm->glk_exit(), the only other potential
	 * caller of exit().  (It also redirects exit() if we don't get to it here
	 * first.)
	 *
	 * So, if we have a main window, flush it.  This is the same cleanup as
	 * done by the finalizer.
	 */
	if (g_vm->gagt_main_window) {
		gagt_status_notify();
		gagt_output_flush();
	}

	/* Clear the running flag, and transform exit() into a g_vm->glk_exit(). */
	gagt_agility_running = FALSE;
	g_vm->glk_exit();
}


/*
 * glk_main)
 *
 * Main entry point for Glk.  Here, all startup is done, and we call our
 * function to run the game.
 */
void glk_main() {
	assert(gagt_startup_called && !gagt_main_called);
	gagt_main_called = TRUE;

	/*
	 * If we're testing for a clean exit, deliberately call exit() to see what
	 * happens.  We're hoping for a clean process termination, but our exit
	 * code explores "undefined" ANSI.  If we get something ugly, like a core
	 * dump, we'll want to set GLK[AGIL]_CLEAN_EXIT.
	 */
	if (g_vm->gagt_clean_exit_test) {
		gagt_agility_running = TRUE;
		return;
	}

	/*
	 * The final part of trapping exit().  Set the running flag, and call the
	 * interpreter main function.  Clear the flag when the main function returns.
	 */
	gagt_agility_running = TRUE;
	gagt_main();
	gagt_agility_running = FALSE;
}


/*---------------------------------------------------------------------*/
/*  Glk linkage relevant only to the UNIX platform                     */
/*---------------------------------------------------------------------*/

/*
 * Glk arguments for UNIX versions of the Glk interpreter.
 */
/*
glkunix_argumentlist_t glkunix_arguments[] = {
 {(char *) "-gf", glkunix_arg_NoValue,
  (char *) "-gf        Force Glk to use only a fixed width font"},
 {(char *) "-gp", glkunix_arg_NoValue,
  (char *) "-gp        Allow Glk to use only a proportional font"},
 {(char *) "-ga", glkunix_arg_NoValue,
  (char *) "-ga        Try to use a suitable Glk font automatically"},
 {(char *) "-gd", glkunix_arg_NoValue,
  (char *) "-gd        Delay for the full period in Glk"},
 {(char *) "-gh", glkunix_arg_NoValue,
  (char *) "-gh        Delay for approximately half the period in Glk"},
 {(char *) "-gn", glkunix_arg_NoValue,
  (char *) "-gn        Turn off all game delays in Glk"},
 {(char *) "-gr", glkunix_arg_NoValue,
  (char *) "-gr        Turn off Glk text replacement"},
 {(char *) "-gx", glkunix_arg_NoValue,
  (char *) "-gx        Turn off Glk abbreviation expansions"},
 {(char *) "-gs", glkunix_arg_NoValue,
  (char *) "-gs        Display a short status window in Glk"},
 {(char *) "-gl", glkunix_arg_NoValue,
  (char *) "-gl        Display an extended status window in Glk"},
 {(char *) "-gc", glkunix_arg_NoValue,
  (char *) "-gc        Turn off Glk command escapes in games"},
 {(char *) "-gD", glkunix_arg_NoValue,
  (char *) "-gD        Turn on Glk port module debug tracing"},
 {(char *) "-g#", glkunix_arg_NoValue,
  (char *) "-g#        Test for clean exit (Glk module debugging only)"},
 {(char *) "-1", glkunix_arg_NoValue,
  (char *) "-1         IRUN Mode: Print messages in first person"},
 {(char *) "-d", glkunix_arg_NoValue,
  (char *) "-d         Debug metacommand execution"},
 {(char *) "-t", glkunix_arg_NoValue,
  (char *) "-t         Test mode"},
 {(char *) "-c", glkunix_arg_NoValue,
  (char *) "-c         Create test file"},
 {(char *) "-m", glkunix_arg_NoValue,
  (char *) "-m         Force descriptions to be loaded from disk"},
#ifdef OPEN_AS_TEXT
 {(char *) "-b", glkunix_arg_NoValue,
  (char *) "-b         Open data files as binary files"},
#endif
 {(char *) "-p", glkunix_arg_NoValue,
  (char *) "-p         Debug parser"},
 {(char *) "-x", glkunix_arg_NoValue,
  (char *) "-x         Debug verb execution loop"},
 {(char *) "-a", glkunix_arg_NoValue,
  (char *) "-a         Debug disambiguation system"},
 {(char *) "-s", glkunix_arg_NoValue,
  (char *) "-s         Debug STANDARD message handler"},
#ifdef MEM_INFO
 {(char *) "-M", glkunix_arg_NoValue,
  (char *) "-M         Debug memory allocation"},
#endif
 {(char *) "", glkunix_arg_ValueCanFollow,
  (char *) "filename   game to run"},
 {NULL, glkunix_arg_End, NULL}
};

*/
/*
 * glkunix_startup_code()
 *
 * Startup entry point for UNIX versions of Glk AGiliTy.  Glk will call
 * glkunix_startup_code() to pass in arguments.  On startup, we call our
 * function to parse arguments and generally set stuff up.
 */

int glk_startup_code() {
	assert(!gagt_startup_called);
	gagt_startup_called = TRUE;

	return gagt_startup_code();
}

} // End of namespace AGT
} // End of namespace Glk
