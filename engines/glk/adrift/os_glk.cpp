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

#include "glk/adrift/adrift.h"
#include "glk/adrift/scare.h"
#include "glk/adrift/scprotos.h"
#include "glk/glk.h"
#include "glk/streams.h"
#include "glk/windows.h"
#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/translation.h"

namespace Glk {
namespace Adrift {

/*
 * Module notes:
 *
 * o The Glk interface makes no effort to set text colors, background colors,
 *   and so forth, and minimal effort to set fonts and other style effects.
 */

#undef _WIN32   /* Gargoyle */

/*
 * True and false definitions -- usually defined in glkstart.h, but we need
 * them early, so we'll define them here too.  We also need nullptr, but that's
 * normally from stdio.h or one of it's cousins.
 */
#ifndef FALSE
# define FALSE 0
#endif
#ifndef TRUE
# define TRUE (!FALSE)
#endif


/*---------------------------------------------------------------------*/
/*  Module variables, miscellaneous other stuff                        */
/*---------------------------------------------------------------------*/

/* Glk SCARE interface version number. */
static const glui32 GSC_PORT_VERSION = 0x00010310;

/* Two windows, one for the main text, and one for a status line. */
static winid_t gsc_main_window = nullptr, gsc_status_window = nullptr;

/*
 * Transcript stream and input log.  These are nullptr if there is no current
 * collection of these strings.
 */
static strid_t gsc_transcript_stream = nullptr, gsc_inputlog_stream = nullptr;

/* Input read log stream, for reading back an input log. */
static strid_t gsc_readlog_stream = nullptr;

/* Options that may be turned off or set by command line flags. */
static int gsc_commands_enabled = TRUE, gsc_abbreviations_enabled = TRUE,
	gsc_unicode_enabled = TRUE;

/* Adrift game to interpret. */
sc_game gsc_game = nullptr;

/* Special out-of-band os_confirm() options used locally with os_glk. */
static const sc_int GSC_CONF_SUBTLE_HINT = INTEGER_MAX,
                    GSC_CONF_UNSUBTLE_HINT = INTEGER_MAX - 1,
                   GSC_CONF_CONTINUE_HINTS = INTEGER_MAX - 2;

/* Forward declaration of event wait functions, and a short delay. */
static void gsc_event_wait_2(glui32 wait_type_1,
                             glui32 wait_type_2, event_t *event);
static void gsc_event_wait(glui32 wait_type, event_t *event);
static void gsc_short_delay();


/*---------------------------------------------------------------------*/
/*  Glk port utility functions                                         */
/*---------------------------------------------------------------------*/

/*
 * gsc_fatal()
 *
 * Fatal error handler.  The function returns, expecting the caller to
 * abort() or otherwise handle the error.
 */
static void gsc_fatal(const char *string) {
	/*
	 * If the failure happens too early for us to have a window, print
	 * the message to stderr.
	 */
	if (!gsc_main_window) {
		error("\n\nINTERNAL ERROR: %s\n", string);
	}

	/* Cancel all possible pending window input events. */
	g_vm->glk_cancel_line_event(gsc_main_window, nullptr);
	g_vm->glk_cancel_char_event(gsc_main_window);

	/* Print a message indicating the error, and exit. */
	g_vm->glk_set_window(gsc_main_window);
	g_vm->glk_set_style(style_Normal);
	g_vm->glk_put_string("\n\nINTERNAL ERROR: ");
	g_vm->glk_put_string((const char *)string);

	g_vm->glk_put_string("\n\nPlease record the details of this error, try to"
	                     " note down everything you did to cause it, and email"
	                     " this information to simon_baldwin@yahoo.com.\n\n");
}


/*
 * gsc_malloc()
 *
 * Non-failing malloc; call gsc_fatal and exit if memory allocation fails.
 */
static void *gsc_malloc(size_t size) {
	void *pointer;

	pointer = malloc(size > 0 ? size : 1);
	if (!pointer) {
		gsc_fatal("GLK: Out of system memory");
		g_vm->glk_exit();
	}

	return pointer;
}


/*---------------------------------------------------------------------*/
/*  Glk port locale data                                               */
/*---------------------------------------------------------------------*/

/* Unicode values up to 256 are equivalent to iso 8859-1. */
static const glui32 GSC_ISO_8859_EQUIVALENCE = 256;

/*
 * Lookup table pair for converting a given single character into unicode and
 * iso 8859-1 (the lower byte of the unicode representation, assuming an upper
 * byte of zero), and an ascii substitute should nothing else be available.
 * Tables are 256 elements; although the first 128 characters of a codepage
 * are usually standard ascii, making tables full-sized allows for support of
 * codepages where they're not (dingbats, for example).
 */
enum { GSC_TABLE_SIZE = 256 };
struct gsc_codepages_t {
	const glui32 unicode[GSC_TABLE_SIZE];
	const sc_char *const ascii[GSC_TABLE_SIZE];
};

/*
 * Locale contains a name and a pair of codepage structures, a main one and
 * an alternate.  The latter is intended for monospaced output.
 */
struct gsc_locale_t {
	const sc_char *const name;
	const gsc_codepages_t main;
	const gsc_codepages_t alternate;
};


/*
 * Locale for Latin1 -- cp1252 and cp850.
 *
 * The ascii representations of characters in this table are based on the
 * general look of the characters, rather than pronounciation.  Accented
 * characters are generally rendered unaccented, and box drawing, shading,
 * and other non-alphanumeric glyphs as either a similar shape, or as a
 * character that might be recognizable as what it's trying to emulate.
 */
static const gsc_locale_t GSC_LATIN1_LOCALE = {
	"Latin1",
	/* cp1252 to unicode. */
	{	{
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x000a, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x0021, 0x0022, 0x0023,
			0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c,
			0x002d, 0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035,
			0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e,
			0x003f, 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
			0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 0x0050,
			0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059,
			0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 0x0060, 0x0061, 0x0062,
			0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b,
			0x006c, 0x006d, 0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074,
			0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d,
			0x007e, 0x0000, 0x20ac, 0x0000, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020,
			0x2021, 0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0000, 0x017d, 0x0000,
			0x0000, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0x02dc,
			0x2122, 0x0161, 0x203a, 0x0153, 0x0000, 0x017e, 0x0178, 0x00a0, 0x00a1,
			0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x00aa,
			0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af, 0x00b0, 0x00b1, 0x00b2, 0x00b3,
			0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc,
			0x00bd, 0x00be, 0x00bf, 0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5,
			0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce,
			0x00cf, 0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
			0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df, 0x00e0,
			0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9,
			0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef, 0x00f0, 0x00f1, 0x00f2,
			0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x00f9, 0x00fa, 0x00fb,
			0x00fc, 0x00fd, 0x00fe, 0x00ff
		},
		/* cp1252 to ascii. */
		{
			nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  "        ",
			"\n",  nullptr,  nullptr,  "\n",  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
			nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
			" ",   "!",   "\"",  "#",   "$",   "%",   "&",   "'",   "(",   ")",   "*",
			"+",   ",",   "-",   ".",   "/",   "0",   "1",   "2",   "3",   "4",   "5",
			"6",   "7",   "8",   "9",   ":",   ";",   "<",   "=",   ">",   "?",   "@",
			"A",   "B",   "C",   "D",   "E",   "F",   "G",   "H",   "I",   "J",   "K",
			"L",   "M",   "N",   "O",   "P",   "Q",   "R",   "S",   "T",   "U",   "V",
			"W",   "X",   "Y",   "Z",   "[",   "\\",  "]",   "^",   "_",   "`",   "a",
			"b",   "c",   "d",   "e",   "f",   "g",   "h",   "i",   "j",   "k",   "l",
			"m",   "n",   "o",   "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
			"x",   "y",   "z",   "{",   "|",   "}",   "~",   nullptr,  "E",   nullptr,  ",",
			"f",   ",,",  "...", "+",   "#",   "^",   "%",   "S",   "<",   "OE",  nullptr,
			"Z",   nullptr,  nullptr,  "'",   "'",   "\"",  "\"",  "*",   "-",   "-",   "~",
			"[TM]", "s",   ">",   "oe",  nullptr,  "z",   "Y",   " ",   "!",   "c",   "GBP",
			"*",   "Y",   "|",   "S",   "\"",  "(C)", "a",   "<<",  "-",   "-",   "(R)",
			"-",   "o",   "+/-", "2",   "3",   "'",   "u",   "P",   "*",   ",",   "1",
			"o",   ">>",  "1/4", "1/2", "3/4", "?",   "A",   "A",   "A",   "A",   "A",
			"A",   "AE",  "C",   "E",   "E",   "E",   "E",   "I",   "I",   "I",   "I",
			"D",   "N",   "O",   "O",   "O",   "O",   "O",   "x",   "O",   "U",   "U",
			"U",   "U",   "Y",   "p",   "ss",  "a",   "a",   "a",   "a",   "a",   "a",
			"ae",  "c",   "e",   "e",   "e",   "e",   "i",   "i",   "i",   "i",   "d",
			"n",   "o",   "o",   "o",   "o",   "o",   "/",   "o",   "u",   "u",   "u",
			"u",   "y",   "P",   "y"
		}
	},
	/* cp850 to unicode. */
	{	{
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x000a, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x0021, 0x0022, 0x0023,
			0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c,
			0x002d, 0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035,
			0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e,
			0x003f, 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
			0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 0x0050,
			0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059,
			0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 0x0060, 0x0061, 0x0062,
			0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b,
			0x006c, 0x006d, 0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074,
			0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d,
			0x007e, 0x0000, 0x00c7, 0x00fc, 0x00e9, 0x00e2, 0x00e4, 0x00e0, 0x00e5,
			0x00e7, 0x00ea, 0x00eb, 0x00e8, 0x00ef, 0x00ee, 0x00ec, 0x00c4, 0x00c5,
			0x00c9, 0x00e6, 0x00c6, 0x00f4, 0x00f6, 0x00f2, 0x00fb, 0x00f9, 0x00ff,
			0x00d6, 0x00dc, 0x00f8, 0x00a3, 0x00d8, 0x00d7, 0x0192, 0x00e1, 0x00ed,
			0x00f3, 0x00fa, 0x00f1, 0x00d1, 0x00aa, 0x00ba, 0x00bf, 0x00ae, 0x00ac,
			0x00bd, 0x00bc, 0x00a1, 0x00ab, 0x00bb, 0x2591, 0x2592, 0x2593, 0x2502,
			0x2524, 0x00c1, 0x00c2, 0x00c0, 0x00a9, 0x2563, 0x2551, 0x2557, 0x255d,
			0x00a2, 0x00a5, 0x2510, 0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c,
			0x00e3, 0x00c3, 0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c,
			0x00a4, 0x00f0, 0x00d0, 0x00ca, 0x00cb, 0x00c8, 0x0131, 0x00cd, 0x00ce,
			0x00cf, 0x2518, 0x250c, 0x2588, 0x2584, 0x00a6, 0x00cc, 0x2580, 0x00d3,
			0x00df, 0x00d4, 0x00d2, 0x00f5, 0x00d5, 0x00b5, 0x00fe, 0x00de, 0x00da,
			0x00db, 0x00d9, 0x00fd, 0x00dd, 0x00af, 0x00b4, 0x00ad, 0x00b1, 0x2017,
			0x00be, 0x00b6, 0x00a7, 0x00f7, 0x00b8, 0x00b0, 0x00a8, 0x00b7, 0x00b9,
			0x00b3, 0x00b2, 0x25a0, 0x00a0
		},
		/* cp850 to ascii. */
		{
			nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  "        ",
			"\n",  nullptr,  nullptr,  "\n",  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
			nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
			" ",   "!",   "\"",  "#",   "$",   "%",   "&",   "'",   "(",   ")",   "*",
			"+",   ",",   "-",   ".",   "/",   "0",   "1",   "2",   "3",   "4",   "5",
			"6",   "7",   "8",   "9",   ":",   ";",   "<",   "=",   ">",   "?",   "@",
			"A",   "B",   "C",   "D",   "E",   "F",   "G",   "H",   "I",   "J",   "K",
			"L",   "M",   "N",   "O",   "P",   "Q",   "R",   "S",   "T",   "U",   "V",
			"W",   "X",   "Y",   "Z",   "[",   "\\",  "]",   "^",   "_",   "`",   "a",
			"b",   "c",   "d",   "e",   "f",   "g",   "h",   "i",   "j",   "k",   "l",
			"m",   "n",   "o",   "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
			"x",   "y",   "z",   "{",   "|",   "}",   "~",   nullptr,  "C",   "u",   "e",
			"a",   "a",   "a",   "a",   "c",   "e",   "e",   "e",   "i",   "i",   "i",
			"A",   "A",   "E",   "ae",  "AE",  "o",   "o",   "o",   "u",   "u",   "y",
			"O",   "U",   "o",   "GBP", "O",   "x",   "f",   "a",   "i",   "o",   "u",
			"n",   "N",   "a",   "o",   "?",   "(R)", "-",   "1/2", "1/4", "i",   "<<",
			">>",  "#",   "#",   "#",   "|",   "+",   "A",   "A",   "A",   "(C)", "+",
			"|",   "+",   "+",   "c",   "Y",   "+",   "+",   "+",   "+",   "+",   "-",
			"+",   "a",   "A",   "+",   "+",   "+",   "+",   "+",   "=",   "+",   "*",
			"d",   "D",   "E",   "E",   "E",   "i",   "I",   "I",   "I",   "+",   "+",
			".",   ".",   "|",   "I",   ".",   "O",   "ss",  "O",   "O",   "o",   "O",
			"u",   "p",   "P",   "U",   "U",   "U",   "y",   "Y",   "-",   "'",   "-",
			"+/-", "=",   "3/4", "P",   "S",   "/",   ",",   "deg", "\"",  "*",   "1",
			"3",   "2",   ".",   " "
		}
	}
};


/*
 * Locale for Cyrillic -- cp1251 and cp866.
 *
 * The ascii representations in this table, for alphabetic characters, follow
 * linguistic rather than appearance rules, the essence of gost 16876-71.
 * Capitalized cyrillic letters that translate to multiple ascii characters
 * have the first ascii character only of the sequence translated.  This gives
 * the best appearance in normal sentences, but is not optimal in a run of
 * all capitals (headings, for example).  For non-alphanumeric characters,
 * the general appearance and shape of the character being emulated is used.
 */
static const gsc_locale_t GSC_CYRILLIC_LOCALE = {
	"Cyrillic",
	/* cp1251 to unicode. */
	{	{
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x000a, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x0021, 0x0022, 0x0023,
			0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c,
			0x002d, 0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035,
			0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e,
			0x003f, 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
			0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 0x0050,
			0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059,
			0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 0x0060, 0x0061, 0x0062,
			0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b,
			0x006c, 0x006d, 0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074,
			0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d,
			0x007e, 0x0000, 0x0402, 0x0403, 0x201a, 0x0453, 0x201e, 0x2026, 0x2020,
			0x2021, 0x20ac, 0x2030, 0x0409, 0x2039, 0x040a, 0x040c, 0x040b, 0x040f,
			0x0452, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0x0000,
			0x2122, 0x0459, 0x203a, 0x045a, 0x045c, 0x045b, 0x045f, 0x00a0, 0x040e,
			0x045e, 0x0408, 0x00a4, 0x0490, 0x00a6, 0x00a7, 0x0401, 0x00a9, 0x0404,
			0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x0407, 0x00b0, 0x00b1, 0x0406, 0x0456,
			0x0491, 0x00b5, 0x00b6, 0x00b7, 0x0451, 0x2116, 0x0454, 0x00bb, 0x0458,
			0x0405, 0x0455, 0x0457, 0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415,
			0x0416, 0x0417, 0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e,
			0x041f, 0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
			0x0428, 0x0429, 0x042a, 0x042b, 0x042c, 0x042d, 0x042e, 0x042f, 0x0430,
			0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439,
			0x043a, 0x043b, 0x043c, 0x043d, 0x043e, 0x043f, 0x0440, 0x0441, 0x0442,
			0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044a, 0x044b,
			0x044c, 0x044d, 0x044e, 0x044f
		},
		/* cp1251 to gost 16876-71 ascii. */
		{
			nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  "        ",
			"\n",  nullptr,  nullptr,  "\n",  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
			nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
			" ",   "!",   "\"",  "#",   "$",   "%",   "&",   "'",   "(",   ")",   "*",
			"+",   ",",   "-",   ".",   "/",   "0",   "1",   "2",   "3",   "4",   "5",
			"6",   "7",   "8",   "9",   ":",   ";",   "<",   "=",   ">",   "?",   "@",
			"A",   "B",   "C",   "D",   "E",   "F",   "G",   "H",   "I",   "J",   "K",
			"L",   "M",   "N",   "O",   "P",   "Q",   "R",   "S",   "T",   "U",   "V",
			"W",   "X",   "Y",   "Z",   "[",   "\\",  "]",   "^",   "_",   "`",   "a",
			"b",   "c",   "d",   "e",   "f",   "g",   "h",   "i",   "j",   "k",   "l",
			"m",   "n",   "o",   "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
			"x",   "y",   "z",   "{",   "|",   "}",   "~",   nullptr,  nullptr,  nullptr,  ",",
			nullptr,  ",,",  "...", "+",   "#",   "E",   "%",   nullptr,  "<",   nullptr,  nullptr,
			nullptr,  nullptr,  nullptr,  "'",   "'",   "\"",  "\"",  "*",   "-",   "-",   nullptr,
			"[TM]", nullptr,  ">",   nullptr,  nullptr,  nullptr,  nullptr,  " ",   nullptr,  nullptr,  nullptr,
			"*",   "G",   "|",   "S",   "Jo",  "(C)", "Je",  "<<",  "-",   "-",   "(R)",
			"Ji",  "o",   "+/-", "I",   "i",   "g",   "u",   "P",   "*",   "jo",  nullptr,
			"je",  ">>",  "j",   "S",   "s",   "ji",  "A",   "B",   "V",   "G",   "D",
			"E",   "Zh",  "Z",   "I",   "Jj",  "K",   "L",   "M",   "N",   "O",   "P",
			"R",   "S",   "T",   "U",   "F",   "Kh",  "C",   "Ch",  "Sh",  "Shh", "\"",
			"Y",   "'",   "Eh",  "Ju",  "Ja",  "a",   "b",   "v",   "g",   "d",   "e",
			"zh",  "z",   "i",   "jj",  "k",   "l",   "m",   "n",   "o",   "p",   "r",
			"s",   "t",   "u",   "f",   "kh",  "c",   "ch",  "sh",  "shh", "\"",  "y",
			"'",   "eh",  "ju",  "ja"
		}
	},
	/* cp866 to unicode. */
	{	{
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x000a, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x0021, 0x0022, 0x0023,
			0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c,
			0x002d, 0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035,
			0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e,
			0x003f, 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
			0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 0x0050,
			0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059,
			0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 0x0060, 0x0061, 0x0062,
			0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b,
			0x006c, 0x006d, 0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074,
			0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d,
			0x007e, 0x0000, 0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416,
			0x0417, 0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e, 0x041f,
			0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428,
			0x0429, 0x042a, 0x042b, 0x042c, 0x042d, 0x042e, 0x042f, 0x0430, 0x0431,
			0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043a,
			0x043b, 0x043c, 0x043d, 0x043e, 0x043f, 0x2591, 0x2592, 0x2593, 0x2502,
			0x2524, 0x2561, 0x2562, 0x2556, 0x2555, 0x2563, 0x2551, 0x2557, 0x255d,
			0x255c, 0x255b, 0x2510, 0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c,
			0x255e, 0x255f, 0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c,
			0x2567, 0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256b,
			0x256a, 0x2518, 0x250c, 0x2588, 0x2584, 0x258c, 0x2590, 0x2580, 0x0440,
			0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449,
			0x044a, 0x044b, 0x044c, 0x044d, 0x044e, 0x044f, 0x0401, 0x0451, 0x0404,
			0x0454, 0x0407, 0x0457, 0x040e, 0x045e, 0x00b0, 0x2022, 0x00b7, 0x221a,
			0x2116, 0x00a4, 0x25a0, 0x00a0
		},
		/* cp866 to gost 16876-71 ascii. */
		{
			nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  "        ",
			"\n",  nullptr,  nullptr,  "\n",  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
			nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
			" ",   "!",   "\"",  "#",   "$",   "%",   "&",   "'",   "(",   ")",   "*",
			"+",   ",",   "-",   ".",   "/",   "0",   "1",   "2",   "3",   "4",   "5",
			"6",   "7",   "8",   "9",   ":",   ";",   "<",   "=",   ">",   "?",   "@",
			"A",   "B",   "C",   "D",   "E",   "F",   "G",   "H",   "I",   "J",   "K",
			"L",   "M",   "N",   "O",   "P",   "Q",   "R",   "S",   "T",   "U",   "V",
			"W",   "X",   "Y",   "Z",   "[",   "\\",  "]",   "^",   "_",   "`",   "a",
			"b",   "c",   "d",   "e",   "f",   "g",   "h",   "i",   "j",   "k",   "l",
			"m",   "n",   "o",   "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
			"x",   "y",   "z",   "{",   "|",   "}",   "~",   nullptr,  "A",   "B",   "V",
			"G",   "D",   "E",   "Zh",  "Z",   "I",   "Jj",  "K",   "L",   "M",   "N",
			"O",   "P",   "R",   "S",   "T",   "U",   "F",   "Kh",  "C",   "Ch",  "Sh",
			"Shh", "\"",  "Y",   "'",   "Eh",  "Ju",  "Ja",  "a",   "b",   "v",   "g",
			"d",   "e",   "zh",  "z",   "i",   "jj",  "k",   "l",   "m",   "n",   "o",
			"p",   "#",   "#",   "#",   "|",   "+",   "+",   "+",   "+",   "+",   "+",
			"|",   "+",   "+",   "+",   "+",   "+",   "+",   "+",   "+",   "+",   "-",
			"+",   "+",   "+",   "+",   "+",   "+",   "+",   "+",   "|",   "+",   "+",
			"+",   "+",   "+",   "+",   "+",   "+",   "+",   "+",   "+",   "+",   "+",
			"+",   ".",   ".",   ".",   ".",   "r",   "s",   "t",   "u",   "f",   "kh",
			"c",   "ch",  "sh",  "shh", "\"",  "y",   "'",   "eh",  "ju",  "ja",  "Jo",
			"jo",  "Je",  "je",  "Ji",  "ji",  nullptr,  nullptr,  "deg", "*",    "*",  nullptr,
			nullptr,  "*",   ".",   " "
		}
	}
};


/*---------------------------------------------------------------------*/
/*  Glk port locale control and conversion functions                   */
/*---------------------------------------------------------------------*/

#ifdef GLK_MODULE_UNICODE
static const sc_bool gsc_has_unicode = TRUE;
#else
static const sc_bool gsc_has_unicode = FALSE;

/* Gestalt selector and stubs for non-unicode capable libraries. */
static const glui32 gestalt_Unicode = 15;

static void glk_put_char_uni(glui32 ch) {
	glui32 unused;
	unused = ch;
	gsc_fatal("GLK: Stub unicode function called");
}

static void glk_request_line_event_uni(winid_t win,
                                       glui32 *buf, glui32 maxlen, glui32 initlen) {
	winid_t unused1;
	glui32 *unused2;
	glui32 unused3, unused4;
	unused1 = win;
	unused2 = buf;
	unused3 = maxlen;
	unused4 = initlen;
	gsc_fatal("GLK: Stub unicode function called");
}

#endif

/*
 * Known valid character printing range.  Some Glk libraries aren't accurate
 * about what will and what won't print when queried with g_vm->glk_gestalt(), so
 * we also make an explicit range check against guaranteed to print chars.
 */
static const glui32 GSC_MIN_PRINTABLE = ' ',
                    GSC_MAX_PRINTABLE = '~';


/* List of pointers to supported and available locales, nullptr terminated. */
static const gsc_locale_t *const GSC_AVAILABLE_LOCALES[] = {
	&GSC_LATIN1_LOCALE,
	&GSC_CYRILLIC_LOCALE,
	nullptr
};

/*
 * The locale for the game, set below explicitly or on game startup, and
 * a fallback locale to use in case none has been set.
 */
static const gsc_locale_t *gsc_locale = nullptr;
static const gsc_locale_t *const gsc_fallback_locale = &GSC_LATIN1_LOCALE;


/*
 * gsc_set_locale()
 *
 * Set a locale explicitly from the name passed in.
 */
static void gsc_set_locale(const sc_char *name) {
	const gsc_locale_t *matched = nullptr;
	const gsc_locale_t *const *iterator;
	assert(name);

	/*
	 * Search locales for a matching name, abbreviated if necessary.  Stop on
	 * the first match found.
	 */
	for (iterator = GSC_AVAILABLE_LOCALES; *iterator; iterator++) {
		const gsc_locale_t *const locale = *iterator;

		if (sc_strncasecmp(name, locale->name, strlen(name)) == 0) {
			matched = locale;
			break;
		}
	}

	/* If matched, set the global locale. */
	if (matched)
		gsc_locale = matched;
}


/*
 * gsc_put_char_uni()
 *
 * Wrapper around g_vm->glk_put_char_uni().  Handles, inelegantly, the problem of
 * having to write transcripts as ascii.
 */
static void gsc_put_char_uni(glui32 unicode, const char *ascii) {
	/* If there is an transcript stream, temporarily disconnect it. */
	if (gsc_transcript_stream)
		g_vm->glk_window_set_echo_stream(gsc_main_window, nullptr);

	g_vm->glk_put_char_uni(unicode);

	/* Print ascii to the transcript, then reattach it. */
	if (gsc_transcript_stream) {
		if (ascii)
			g_vm->glk_put_string_stream(gsc_transcript_stream, (const char *)ascii);
		else
			g_vm->glk_put_char_stream(gsc_transcript_stream, '?');

		g_vm->glk_window_set_echo_stream(gsc_main_window, gsc_transcript_stream);
	}
}


/*
 * gsc_put_char_locale()
 *
 * Write a single character using the supplied locale.  Select either the
 * main or the alternate codepage depending on the flag passed in.
 */
static void gsc_put_char_locale(sc_char ch, const gsc_locale_t *locale, sc_bool is_alternate) {
	const gsc_codepages_t *codepage;
	unsigned char character;
	glui32 unicode;
	const char *ascii;

	/*
	 * Select either the main or the alternate codepage for this locale, and
	 * retrieve the unicode and ascii representations of the character.
	 */
	codepage = is_alternate ? &locale->alternate : &locale->main;
	character = (unsigned char) ch;
	unicode = codepage->unicode[character];
	ascii = codepage->ascii[character];

	/*
	 * If a unicode representation exists, use for either iso 8859-1 or, if
	 * possible, direct unicode output.
	 */
	if (unicode > 0) {
		/*
		 * If unicode is in the range 1-255, this value is directly equivalent
		 * to the iso 8859-1 representation; otherwise the character has no
		 * direct iso 8859-1 glyph.
		 */
		if (unicode < GSC_ISO_8859_EQUIVALENCE) {
			/*
			 * If the iso 8859-1 character is one that this Glk library will
			 * print exactly, print and return.  We add a check here for the
			 * guaranteed printable characters, since some Glk libraries don't
			 * return the correct values for gestalt_CharOutput for these.
			 */
			if (unicode == '\n'
			        || (unicode >= GSC_MIN_PRINTABLE && unicode <= GSC_MAX_PRINTABLE)
			        || g_vm->glk_gestalt(gestalt_CharOutput,
			                             unicode) == gestalt_CharOutput_ExactPrint) {
				g_vm->glk_put_char((unsigned char) unicode);
				return;
			}
		}

		/*
		 * If no usable iso 8859-1 representation, see if unicode is enabled and
		 * if the Glk library can print the character exactly.  If yes, output
		 * the character that way.
		 *
		 * TODO Using unicode output currently disrupts transcript output.  Any
		 * echo stream connected for a transcript here will be a text rather than
		 * a unicode stream, so probably won't output the character correctly.
		 * For now, if there's a transcript, we try to write ascii output.
		 */
		if (gsc_unicode_enabled) {
			if (g_vm->glk_gestalt(gestalt_CharOutput,
			                      unicode) == gestalt_CharOutput_ExactPrint) {
				gsc_put_char_uni(unicode, ascii);
				return;
			}
		}
	}

	/*
	 * No success with iso 8859-1 or unicode, so try for an ascii substitute.
	 * Substitute strings use only 7-bit ascii, and so all are safe to print
	 * directly with Glk.
	 */
	if (ascii) {
		g_vm->glk_put_string((const char *)ascii);
		return;
	}

	/* No apparent way to output this character, so print a '?'. */
	g_vm->glk_put_char('?');
}


/*
 * gsc_put_char()
 * gsc_put_char_alternate()
 * gsc_put_buffer_using()
 * gsc_put_buffer()
 * gsc_put_string()
 * gsc_put_string_alternate()
 *
 * Public functions for writing using the current or fallback locale.
 */
static void gsc_put_char(const sc_char character) {
	const gsc_locale_t *locale;

	locale = gsc_locale ? gsc_locale : gsc_fallback_locale;
	gsc_put_char_locale(character, locale, FALSE);
}

static void gsc_put_char_alternate(const sc_char character) {
	const gsc_locale_t *locale;

	locale = gsc_locale ? gsc_locale : gsc_fallback_locale;
	gsc_put_char_locale(character, locale, TRUE);
}

static void gsc_put_buffer_using(const sc_char *buffer, sc_int length, void (*putchar_function)(sc_char)) {
	sc_int index_;

	for (index_ = 0; index_ < length; index_++)
		putchar_function(buffer[index_]);
}

static void gsc_put_buffer(const sc_char *buffer, sc_int length) {
	assert(buffer);

	gsc_put_buffer_using(buffer, length, gsc_put_char);
}

static void gsc_put_string(const sc_char *string) {
	assert(string);

	gsc_put_buffer_using(string, strlen(string), gsc_put_char);
}

static void gsc_put_string_alternate(const sc_char *string) {
	assert(string);

	gsc_put_buffer_using(string, strlen(string), gsc_put_char_alternate);
}


/*
 * gsc_unicode_to_locale()
 * gsc_unicode_buffer_to_locale()
 *
 * Convert a unicode character back to an sc_char through a locale.  Used for
 * reverse translations in line input.  Returns '?' if there is no translation
 * available.
 */
static sc_char gsc_unicode_to_locale(glui32 unicode, const gsc_locale_t *locale) {
	const gsc_codepages_t *codepage;
	sc_int character;

	/* Always use the main codepage for input. */
	codepage = &locale->main;

	/*
	 * Search the unicode table sequentially for the input character.  This is
	 * inefficient, but because game input is usually not copious, excusable.
	 */
	for (character = 0; character < GSC_TABLE_SIZE; character++) {
		if (codepage->unicode[character] == unicode)
			break;
	}

	/* Return the character translation, or '?' if none. */
	return character < GSC_TABLE_SIZE ? (sc_char) character : '?';
}

static void gsc_unicode_buffer_to_locale(const glui32 *unicode, sc_int length,
		sc_char *buffer, const gsc_locale_t *locale) {
	sc_int index_;

	for (index_ = 0; index_ < length; index_++)
		buffer[index_] = gsc_unicode_to_locale(unicode[index_], locale);
}


/*
 * gsc_read_line_locale()
 *
 * Read in a line and translate out of the given locale.  Returns the count
 * of characters placed in the buffer.
 */
static sc_int gsc_read_line_locale(sc_char *buffer, sc_int length, const gsc_locale_t *locale) {
	event_t event;

	/*
	 * If we have unicode, we have to use it to ensure that characters not in
	 * the Latin1 locale are properly translated.
	 */
	if (gsc_unicode_enabled) {
		uint32 *unicode;

		/*
		 * Allocate a unicode buffer long enough to hold all the characters,
		 * then read in a unicode line.
		 */
		unicode = (uint32 *)gsc_malloc(length * sizeof(*unicode));
		g_vm->glk_request_line_event_uni(gsc_main_window, unicode, length, 0);
		gsc_event_wait(evtype_LineInput, &event);

		/* Convert the unicode buffer out, then free it. */
		gsc_unicode_buffer_to_locale(unicode, event.val1, buffer, locale);
		free(unicode);

		/* Return the count of characters placed in the buffer. */
		return event.val1;
	}

	/* No success with unicode, so fall back to standard line input. */
	g_vm->glk_request_line_event(gsc_main_window, buffer, length, 0);
	gsc_event_wait(evtype_LineInput, &event);

	/* Return the count of characters placed in the buffer. */
	return event.val1;
}


/*
 * gsc_read_line()
 *
 * Public function for reading using the current or fallback locale.
 */
static sc_int gsc_read_line(sc_char *buffer, sc_int length) {
	const gsc_locale_t *locale;

	locale = gsc_locale ? gsc_locale : gsc_fallback_locale;
	return gsc_read_line_locale(buffer, length, locale);
}


/*---------------------------------------------------------------------*/
/*  Glk port status line functions                                     */
/*---------------------------------------------------------------------*/

/*
 * Slop for right-justification of status lines, as an attempt to compensate
 * for the fact that some characters in a games status line may use more than
 * one position when printed, a particular problem in gost 16876-71 Cyrillic.
 */
static const sc_int GSC_STATUS_SLOP = 10;

/* Size of saved status buffer used for non-windowing Glk status lines. */
enum { GSC_STATUS_BUFFER_LENGTH = 74 };

/* Whitespace characters, used to detect empty status elements. */
static const sc_char *const GSC_WHITESPACE = "\t\n\v\f\r ";


/*
 * gsc_is_string_usable()
 *
 * Return TRUE if string is non-null, not zero-length or contains characters
 * other than whitespace.
 */
static sc_bool gsc_is_string_usable(const sc_char *string) {
	/* If non-null, scan for any non-space character. */
	if (string) {
		sc_int index_;

		for (index_ = 0; string[index_] != '\0'; index_++) {
			if (!strchr(GSC_WHITESPACE, string[index_]))
				return TRUE;
		}
	}

	/* nullptr, or no characters other than whitespace. */
	return FALSE;
}


/*
 * gsc_status_update()
 *
 * Update the status line from the current game state.  This is for windowing
 * Glk libraries.
 */
static void gsc_status_update() {
	uint width, height;
	uint index;
	assert(gsc_status_window);

	g_vm->glk_window_get_size(gsc_status_window, &width, &height);
	if (height > 0) {
		const sc_char *room;

		g_vm->glk_window_clear(gsc_status_window);
		g_vm->glk_window_move_cursor(gsc_status_window, 0, 0);
		g_vm->glk_set_window(gsc_status_window);

		g_vm->glk_set_style(style_User1);
		for (index = 0; index < width; index++)
			g_vm->glk_put_char(' ');
		g_vm->glk_window_move_cursor(gsc_status_window, 0, 0);

		/* See if the game is indicating any current player room. */
		room = sc_get_game_room(gsc_game);
		if (!gsc_is_string_usable(room)) {
			/*
			 * Player location is indeterminate, so print out a generic status,
			 * showing the game name and author.
			 */
			g_vm->glk_window_move_cursor(gsc_status_window, 1, 0);
			gsc_put_string(sc_get_game_name(gsc_game));
			g_vm->glk_put_string(" | ");
			gsc_put_string(sc_get_game_author(gsc_game));
		} else {
			const sc_char *status;
			char score[64];

			/* Print the player location. */
			g_vm->glk_window_move_cursor(gsc_status_window, 1, 0);
			gsc_put_string(room);

			/* Get the game's status line, or if none, format score. */
			status = sc_get_game_status_line(gsc_game);
			if (!gsc_is_string_usable(status)) {
				sprintf(score, "Score: %ld", sc_get_game_score(gsc_game));
				status = score;
			}

			/* Print the status line or score at window right, if it fits. */
			if (width > strlen(status) + GSC_STATUS_SLOP + 1) {
				glui32 position;

				position = width - strlen(status) - GSC_STATUS_SLOP;
				g_vm->glk_window_move_cursor(gsc_status_window, position - 1, 0);
				gsc_put_string(status);
			}
		}

		g_vm->glk_set_window(gsc_main_window);
	}
}


/*
 * gsc_status_safe_strcat()
 *
 * Helper for gsc_status_print(), concatenates strings only up to the
 * available length.
 */
static void gsc_status_safe_strcat(char *dest, size_t length, const char *src) {
	size_t available, src_length;

	/* Append only as many characters as will fit. */
	src_length = strlen(src);
	available = length - strlen(dest) - 1;
	if (available > 0)
		strncat(dest, src, src_length < available ? src_length : available);
}


/*
 * gsc_status_print()
 *
 * Print the current contents of the completed status line buffer out in the
 * main window, if it has changed since the last call.  This is for non-
 * windowing Glk libraries.
 */
static void gsc_status_print() {
	static char current_status[GSC_STATUS_BUFFER_LENGTH + 1];

	const sc_char *room;

	/* Do nothing if the game isn't indicating any current player room. */
	room = sc_get_game_room(gsc_game);
	if (gsc_is_string_usable(room)) {
		char buffer[GSC_STATUS_BUFFER_LENGTH + 1];
		const sc_char *status;
		char score[64];

		/* Make an attempt at a status line, starting with player location. */
		strcpy(buffer, "");
		gsc_status_safe_strcat(buffer, sizeof(buffer), room);

		/* Get the game's status line, or if none, format score. */
		status = sc_get_game_status_line(gsc_game);
		if (!gsc_is_string_usable(status)) {
			sprintf(score, "Score: %ld", sc_get_game_score(gsc_game));
			status = score;
		}

		/* Append the status line or score. */
		gsc_status_safe_strcat(buffer, sizeof(buffer), " | ");
		gsc_status_safe_strcat(buffer, sizeof(buffer), status);

		/* If this matches the current saved status line, do nothing more. */
		if (strcmp(buffer, current_status) != 0) {
			/* Bracket, and output the status line buffer. */
			g_vm->glk_put_string("[ ");
			gsc_put_string(buffer);
			g_vm->glk_put_string(" ]\n");

			/* Save the details of the printed status buffer. */
			strcpy(current_status, buffer);
		}
	}
}


/*
 * gsc_status_notify()
 *
 * Front end function for updating status.  Either updates the status window
 * or prints the status line to the main window.
 */
static void gsc_status_notify() {
	if (gsc_status_window)
		gsc_status_update();
	else
		gsc_status_print();
}


/*
 * gsc_status_redraw()
 *
 * Redraw the contents of any status window with the constructed status string.
 * This function should be called on the appropriate Glk window resize and
 * arrange events.
 */
static void gsc_status_redraw() {
	if (gsc_status_window) {
		winid_t parent;

		/*
		 * Rearrange the status window, without changing its actual arrangement
		 * in any way.  This is a hack to work round incorrect window repainting
		 * in Xglk; it forces a complete repaint of affected windows on Glk
		 * window resize and arrange events, and works in part because Xglk
		 * doesn't check for actual arrangement changes in any way before
		 * invalidating its windows.  The hack should be harmless to Glk
		 * libraries other than Xglk, moreover, we're careful to activate it
		 * only on resize and arrange events.
		 */
		parent = g_vm->glk_window_get_parent(gsc_status_window);
		g_vm->glk_window_set_arrangement(parent,
		                                 winmethod_Above | winmethod_Fixed, 1, nullptr);
		gsc_status_update();
	}
}


/*---------------------------------------------------------------------*/
/*  Glk port output functions                                          */
/*---------------------------------------------------------------------*/

/*
 * Flag for if the user entered "help" as their last input, or if hints have
 * been silenced as a result of already using a Glk command.
 */
static int gsc_help_requested = FALSE,
           gsc_help_hints_silenced = FALSE;

/* Font descriptor type, encapsulating size and monospaced boolean. */
struct gsc_font_size_t {
	sc_bool is_monospaced;
	sc_int size;
};

/* Font stack and attributes for nesting tags. */
enum { GSC_MAX_STYLE_NESTING = 32 };
static gsc_font_size_t gsc_font_stack[GSC_MAX_STYLE_NESTING];
static glui32 gsc_font_index = 0;
static glui32 gsc_attribute_bold = 0,
              gsc_attribute_italic = 0,
              gsc_attribute_underline = 0,
              gsc_attribute_secondary_color = 0;

/* Notional default font size, and limit font sizes. */
static const sc_int GSC_DEFAULT_FONT_SIZE = 12,
                    GSC_MEDIUM_FONT_SIZE = 14,
                    GSC_LARGE_FONT_SIZE = 16;

/* Milliseconds per second and timeouts count for delay tags. */
static const glui32 GSC_MILLISECONDS_PER_SECOND = 1000;
static const glui32 GSC_TIMEOUTS_COUNT = 10;

/* Number of hints to refuse before offering to end hint display. */
static const sc_int GSC_HINT_REFUSAL_LIMIT = 5;

/* The keypresses used to cancel any <wait x.x> early. */
static const glui32 GSC_CANCEL_WAIT_1 = ' ',
                    GSC_CANCEL_WAIT_2 = keycode_Return;


/*
 * gsc_output_register_help_request()
 * gsc_output_silence_help_hints()
 * gsc_output_provide_help_hint()
 *
 * Register a request for help, and print a note of how to get Glk command
 * help from the interpreter unless silenced.
 */
static void gsc_output_register_help_request() {
	gsc_help_requested = TRUE;
}

static void gsc_output_silence_help_hints() {
	gsc_help_hints_silenced = TRUE;
}

static void gsc_output_provide_help_hint() {
	if (gsc_help_requested && !gsc_help_hints_silenced) {
		g_vm->glk_set_style(style_Emphasized);
		g_vm->glk_put_string("[Try 'glk help' for help on special interpreter"
		                     " commands]\n");

		gsc_help_requested = FALSE;
		g_vm->glk_set_style(style_Normal);
	}
}


/*
 * gsc_set_glk_style()
 *
 * Set a Glk style based on the top of the font stack and attributes.
 */
static void gsc_set_glk_style() {
	sc_bool is_monospaced;
	sc_int font_size;

	/* Get the current font stack top, or default value. */
	if (gsc_font_index > 0) {
		is_monospaced = gsc_font_stack[gsc_font_index - 1].is_monospaced;
		font_size = gsc_font_stack[gsc_font_index - 1].size;
	} else {
		is_monospaced = FALSE;
		font_size = GSC_DEFAULT_FONT_SIZE;
	}

	/*
	 * Map the font and current attributes into a Glk style.  Because Glk styles
	 * aren't cumulative this has to be done by precedences.
	 */
	if (is_monospaced) {
		/*
		 * No matter the size or attributes, if monospaced use Preformatted
		 * style, as it's all we have.
		 */
		g_vm->glk_set_style(style_Preformatted);
	} else {
		/*
		 * For large and medium point sizes, use Header or Subheader styles
		 * respectively.
		 */
		if (font_size >= GSC_LARGE_FONT_SIZE)
			g_vm->glk_set_style(style_Header);
		else if (font_size >= GSC_MEDIUM_FONT_SIZE)
			g_vm->glk_set_style(style_Subheader);
		else {
			/*
			 * For bold, use Subheader; for italics, underline, or secondary
			 * color, use Emphasized.
			 */
			if (gsc_attribute_bold > 0)
				g_vm->glk_set_style(style_Subheader);
			else if (gsc_attribute_italic > 0
			         || gsc_attribute_underline > 0
			         || gsc_attribute_secondary_color > 0)
				g_vm->glk_set_style(style_Emphasized);
			else {
				/*
				 * There's nothing special about this text, so drop down to
				 * Normal style.
				 */
				g_vm->glk_set_style(style_Normal);
			}
		}
	}
}


/*
 * gsc_handle_font_tag()
 * gsc_handle_endfont_tag()
 *
 * Push the settings of a font tag onto the font stack, and pop on end of
 * font tag.  Set the appropriate Glk style.
 */
static void gsc_handle_font_tag(const sc_char *argument) {
	/* Ignore the call on stack overrun. */
	if (gsc_font_index < GSC_MAX_STYLE_NESTING) {
		sc_char *lower, *face, *size;
		sc_bool is_monospaced;
		sc_int index_, font_size;

		/* Get the current top of stack, or default on empty stack. */
		if (gsc_font_index > 0) {
			is_monospaced = gsc_font_stack[gsc_font_index - 1].is_monospaced;
			font_size = gsc_font_stack[gsc_font_index - 1].size;
		} else {
			is_monospaced = FALSE;
			font_size = GSC_DEFAULT_FONT_SIZE;
		}

		/* Copy and convert argument to all lowercase. */
		lower = (sc_char *)gsc_malloc(strlen(argument) + 1);
		strcpy(lower, argument);
		for (index_ = 0; lower[index_] != '\0'; index_++)
			lower[index_] = g_vm->glk_char_to_lower(lower[index_]);

		/* Find any face= portion of the tag argument. */
		face = strstr(lower, "face=");
		if (face) {
			/*
			 * There may be plenty of monospaced fonts, but we do only courier
			 * and terminal.
			 */
			is_monospaced = strncmp(face, "face=\"courier\"", 14) == 0
			                || strncmp(face, "face=\"terminal\"", 15) == 0;
		}

		/* Find the size= portion of the tag argument. */
		size = strstr(lower, "size=");
		if (size) {
			sc_uint value;

			/* Deal with incremental and absolute sizes. */
			if (strncmp(size, "size=+", 6) == 0
			        && sscanf(size, "size=+%lu", &value) == 1)
				font_size += value;
			else if (strncmp(size, "size=-", 6) == 0
			         && sscanf(size, "size=-%lu", &value) == 1)
				font_size -= value;
			else if (sscanf(size, "size=%lu", &value) == 1)
				font_size = value;
		}

		/* Done with tag argument copy. */
		free(lower);

		/*
		 * Push the new font setting onto the font stack, and set Glk style.
		 */
		gsc_font_stack[gsc_font_index].is_monospaced = is_monospaced;
		gsc_font_stack[gsc_font_index++].size = font_size;
		gsc_set_glk_style();
	}
}

static void gsc_handle_endfont_tag() {
	/* Unless underrun, pop the font stack and set Glk style. */
	if (gsc_font_index > 0) {
		gsc_font_index--;
		gsc_set_glk_style();
	}
}


/*
 * gsc_handle_attribute_tag()
 *
 * Increment the required attribute nesting counter, or decrement on end
 * tag.  Set the appropriate Glk style.
 */
static void gsc_handle_attribute_tag(sc_int tag) {
	/*
	 * Increment the required attribute nesting counter, and set Glk style.
	 */
	switch (tag) {
	case SC_TAG_BOLD:
		gsc_attribute_bold++;
		break;
	case SC_TAG_ITALICS:
		gsc_attribute_italic++;
		break;
	case SC_TAG_UNDERLINE:
		gsc_attribute_underline++;
		break;
	case SC_TAG_COLOR:
		gsc_attribute_secondary_color++;
		break;
	default:
		break;
	}
	gsc_set_glk_style();
}

static void gsc_handle_endattribute_tag(sc_int tag) {
	/*
	 * Decrement the required attribute nesting counter, unless underrun, and
	 * set Glk style.
	 */
	switch (tag) {
	case SC_TAG_ENDBOLD:
		if (gsc_attribute_bold > 0)
			gsc_attribute_bold--;
		break;
	case SC_TAG_ENDITALICS:
		if (gsc_attribute_italic > 0)
			gsc_attribute_italic--;
		break;
	case SC_TAG_ENDUNDERLINE:
		if (gsc_attribute_underline > 0)
			gsc_attribute_underline--;
		break;
	case SC_TAG_ENDCOLOR:
		if (gsc_attribute_secondary_color > 0)
			gsc_attribute_secondary_color--;
		break;
	default:
		break;
	}
	gsc_set_glk_style();
}


/*
 * gsc_handle_wait_tag()
 *
 * If Glk offers timers, delay for the requested period.  Otherwise, this
 * function does nothing.
 */
static void gsc_handle_wait_tag(const sc_char *argument) {
	double delay = 0.0;

	/* Ignore the wait tag if the Glk doesn't have timers. */
	if (!g_vm->glk_gestalt(gestalt_Timer, 0))
		return;

	/* Determine the delay time, and convert to milliseconds. */
	if (sscanf(argument, "%lf", &delay) == 1 && delay > 0.0) {
		glui32 milliseconds, timeout;

		/*
		 * Work with timeouts at 1/10 of the wait period, to minimize Glk
		 * timer jitter.  Allow the timeout to be canceled by keypress, as a
		 * user convenience.
		 */
		milliseconds = (glui32)(delay * GSC_MILLISECONDS_PER_SECOND);
		timeout = milliseconds / GSC_TIMEOUTS_COUNT;
		if (timeout > 0) {
			glui32 delayed;
			sc_bool is_completed;

			/* Request timer events, and let a keypress cancel the wait. */
			g_vm->glk_request_char_event(gsc_main_window);
			g_vm->glk_request_timer_events(timeout);

			/* Loop until delay completed or canceled by a keypress. */
			is_completed = TRUE;
			for (delayed = 0; delayed < milliseconds; delayed += timeout) {
				event_t event;

				gsc_event_wait_2(evtype_CharInput, evtype_Timer, &event);
				if (event.type == evtype_CharInput) {
					/* Cancel the delay, or reissue the input request. */
					if (event.val1 == GSC_CANCEL_WAIT_1
					        || event.val1 == GSC_CANCEL_WAIT_2) {
						is_completed = FALSE;
						break;
					} else
						g_vm->glk_request_char_event(gsc_main_window);
				}
			}

			/* Cancel any pending character input, and stop timers. */
			if (is_completed)
				g_vm->glk_cancel_char_event(gsc_main_window);
			g_vm->glk_request_timer_events(0);
		}
	}
}


/*
 * gsc_reset_glk_style()
 *
 * Drop all stacked fonts and nested attributes, and return to normal Glk
 * style.
 */
static void gsc_reset_glk_style() {
	/* Reset the font stack and attributes, and set a normal style. */
	gsc_font_index = 0;
	gsc_attribute_bold = 0;
	gsc_attribute_italic = 0;
	gsc_attribute_underline = 0;
	gsc_attribute_secondary_color = 0;
	gsc_set_glk_style();
}


/*
 * os_print_tag()
 *
 * Interpret selected Adrift output control tags.  Not all are implemented
 * here; several are ignored.
 */
void os_print_tag(sc_int tag, const sc_char *argument) {
	event_t event;
	assert(argument);

	switch (tag) {
	case SC_TAG_CLS:
		/* Clear the main text display window. */
		g_vm->glk_window_clear(gsc_main_window);
		break;

	case SC_TAG_FONT:
		/* Handle with specific tag handler function. */
		gsc_handle_font_tag(argument);
		break;

	case SC_TAG_ENDFONT:
		/* Handle with specific endtag handler function. */
		gsc_handle_endfont_tag();
		break;

	case SC_TAG_BOLD:
	case SC_TAG_ITALICS:
	case SC_TAG_UNDERLINE:
	case SC_TAG_COLOR:
		/* Handle with common attribute tag handler function. */
		gsc_handle_attribute_tag(tag);
		break;

	case SC_TAG_ENDBOLD:
	case SC_TAG_ENDITALICS:
	case SC_TAG_ENDUNDERLINE:
	case SC_TAG_ENDCOLOR:
		/* Handle with common attribute endtag handler function. */
		gsc_handle_endattribute_tag(tag);
		break;

	case SC_TAG_CENTER:
	case SC_TAG_RIGHT:
	case SC_TAG_ENDCENTER:
	case SC_TAG_ENDRIGHT:
		/*
		 * We don't center or justify text, but so that things look right we do
		 * want a newline on starting or ending such a section.
		 */
		g_vm->glk_put_char('\n');
		break;

	case SC_TAG_WAIT:
		/*
		 * Update the status line now only if it has its own window, then
		 * handle with a specialized handler.
		 */
		if (gsc_status_window)
			gsc_status_notify();
		gsc_handle_wait_tag(argument);
		break;

	case SC_TAG_WAITKEY:
		/*
		 * If reading an input log, ignore; it disrupts replay.  Write a newline
		 * to separate off any unterminated game output instead.
		 */
		if (!gsc_readlog_stream) {
			/* Update the status line now only if it has its own window. */
			if (gsc_status_window)
				gsc_status_notify();

			/* Request a character event, and wait for it to be filled. */
			g_vm->glk_request_char_event(gsc_main_window);
			gsc_event_wait(evtype_CharInput, &event);
		} else
			g_vm->glk_put_char('\n');
		break;

	default:
		/* Ignore unimplemented and unknown tags. */
		break;
	}
}


/*
 * os_print_string()
 *
 * Print a text string to the main output window.
 */
void os_print_string(const sc_char *string) {
	sc_bool is_monospaced;
	assert(string);
	assert(g_vm->glk_stream_get_current());

	/*
	 * Get the monospace font setting from the current top of stack, or
	 * default on empty stack.  If set, we may need to use an alternative
	 * function to write this string.
	 */
	if (gsc_font_index > 0)
		is_monospaced = gsc_font_stack[gsc_font_index - 1].is_monospaced;
	else
		is_monospaced = FALSE;

	/*
	 * The main window should always be the currently set window at this point,
	 * so we never be attempting monospaced output to the status window.
	 * Nevertheless, check anyway.
	 */
	if (is_monospaced
	        && g_vm->glk_stream_get_current() == g_vm->glk_window_get_stream(gsc_main_window))
		gsc_put_string_alternate(string);
	else
		gsc_put_string(string);
}


/*
 * os_print_string_debug()
 *
 * Debugging output goes to the main Glk window -- no special effects or
 * dedicated debugging window attempted.
 */
void os_print_string_debug(const sc_char *string) {
	assert(string);
	assert(g_vm->glk_stream_get_current());

	gsc_put_string(string);
}


/*
 * gsc_styled_string()
 * gsc_styled_char()
 * gsc_standout_string()
 * gsc_standout_char()
 * gsc_normal_string()
 * gsc_normal_char()
 * gsc_header_string()
 *
 * Convenience functions to print strings in assorted styles.  A standout
 * string is one that hints that it's from the interpreter, not the game.
 */
static void gsc_styled_string(glui32 style, const char *message) {
	assert(message);

	g_vm->glk_set_style(style);
	g_vm->glk_put_string((const char *)message);
	g_vm->glk_set_style(style_Normal);
}

static void gsc_styled_char(glui32 style, char c) {
	char buffer[2];

	buffer[0] = c;
	buffer[1] = '\0';
	gsc_styled_string(style, buffer);
}

static void gsc_standout_string(const char *message) {
	gsc_styled_string(style_Emphasized, message);
}

static void gsc_standout_char(char c) {
	gsc_styled_char(style_Emphasized, c);
}

static void gsc_normal_string(const char *message) {
	gsc_styled_string(style_Normal, message);
}

static void gsc_normal_char(char c) {
	gsc_styled_char(style_Normal, c);
}

static void gsc_header_string(const char *message) {
	gsc_styled_string(style_Header, message);
}


/*
 * os_display_hints()
 *
 * This is a very basic hints display.  In mitigation, very few games use
 * hints at all, and those that do are usually sparse in what they hint at, so
 * it's sort of good enough for the moment.
 */
void os_display_hints(sc_game game) {
	sc_game_hint hint;
	sc_int refused;

	/* For each hint, print the question, and confirm hint display. */
	refused = 0;
	for (hint = sc_get_first_game_hint(game);
	        hint; hint = sc_get_next_game_hint(game, hint)) {
		const sc_char *hint_question, *hint_text;

		/* If enough refusals, offer a way out of the loop. */
		if (refused >= GSC_HINT_REFUSAL_LIMIT) {
			if (!os_confirm(GSC_CONF_CONTINUE_HINTS))
				break;
			refused = 0;
		}

		/* Pop the question. */
		hint_question = sc_get_game_hint_question(game, hint);
		gsc_normal_char('\n');
		gsc_standout_string(hint_question);
		gsc_normal_char('\n');

		/* Print the subtle hint, or on to the next hint. */
		hint_text = sc_get_game_subtle_hint(game, hint);
		if (hint_text) {
			if (!os_confirm(GSC_CONF_SUBTLE_HINT)) {
				refused++;
				continue;
			}
			gsc_normal_char('\n');
			gsc_standout_string(hint_text);
			gsc_normal_string("\n\n");
		}

		/* Print the less than subtle hint, or on to the next hint. */
		hint_text = sc_get_game_unsubtle_hint(game, hint);
		if (hint_text) {
			if (!os_confirm(GSC_CONF_UNSUBTLE_HINT)) {
				refused++;
				continue;
			}
			gsc_normal_char('\n');
			gsc_standout_string(hint_text);
			gsc_normal_string("\n\n");
		}
	}
}


/*---------------------------------------------------------------------*/
/*  Glk resource handling functions                                    */
/*---------------------------------------------------------------------*/

/*
 * os_play_sound()
 * os_stop_sound()
 *
 * Stub functions.  The unused variables defeat gcc warnings.
 */
void os_play_sound(const sc_char *filepath, sc_int offset, sc_int length, sc_bool is_looping) {
/*
	const sc_char *unused1;
	sc_int unused2, unused3;
	sc_bool unused4;
	unused1 = filepath;
	unused2 = offset;
	unused3 = length;
	unused4 = is_looping;
*/
}

void os_stop_sound() {
}


/*
 * os_show_graphic()
 *
 * For graphic-capable Glk libraries on Linux, attempt graphics using xv.  The
 * graphic capability test isn't really required, it's just a way of having
 * graphics behave without surprises; someone using a non-graphical Glk
 * probably won't expect graphics to pop up.
 *
 * For other cases, this is a stub function, with unused variables to defeat
 * gcc warnings.
 */
#ifdef LINUX_GRAPHICS
static int gsclinux_graphics_enabled = TRUE;
static char *gsclinux_game_file = nullptr;
void os_show_graphic(const sc_char *filepath, sc_int offset, sc_int length) {
	const sc_char *unused1;
	unused1 = filepath;

	if (length > 0
	        && gsclinux_graphics_enabled && g_vm->glk_gestalt(gestalt_Graphics, 0)) {
		sc_char *buffer;

		/*
		 * Try to extract data with dd.  Assuming that works, background xv to
		 * display the image, then background a job to delay ten seconds and
		 * then delete the temporary file containing the image.  Systems lacking
		 * xv can usually use a small script, named xv, to invoke eog or an
		 * alternative image display binary.  Not exactly finessed.
		 */
		assert(gsclinux_game_file);
		buffer = gsc_malloc(strlen(gsclinux_game_file) + 128);
		sprintf(buffer, "dd if=%s ibs=1c skip=%ld count=%ld obs=100k"
		        " of=/tmp/scare.jpg 2>/dev/null",
		        gsclinux_game_file, offset, length);
		system(buffer);
		free(buffer);
		system("xv /tmp/scare.jpg >/dev/null 2>&1 &");
		system("( sleep 10; rm /tmp/scare.jpg ) >/dev/null 2>&1 &");
	}
}
#else
void os_show_graphic(const sc_char *filepath, sc_int offset, sc_int length) {
/*
	const sc_char *unused1;
	sc_int unused2, unused3;
	unused1 = filepath;
	unused2 = offset;
	unused3 = length;
*/
}
#endif


/*---------------------------------------------------------------------*/
/*  Glk command escape functions                                       */
/*---------------------------------------------------------------------*/

/*
 * gsc_command_script()
 *
 * Turn game output scripting (logging) on and off.
 */
static void gsc_command_script(const char *argument) {
	assert(argument);

	if (sc_strcasecmp(argument, "on") == 0) {
		frefid_t fileref;

		if (gsc_transcript_stream) {
			gsc_normal_string("Glk transcript is already on.\n");
			return;
		}

		fileref = g_vm->glk_fileref_create_by_prompt(fileusage_Transcript
		          | fileusage_TextMode,
		          filemode_WriteAppend, 0);
		if (!fileref) {
			gsc_standout_string("Glk transcript failed.\n");
			return;
		}

		gsc_transcript_stream = g_vm->glk_stream_open_file(fileref,
		                        filemode_WriteAppend, 0);
		g_vm->glk_fileref_destroy(fileref);
		if (!gsc_transcript_stream) {
			gsc_standout_string("Glk transcript failed.\n");
			return;
		}

		g_vm->glk_window_set_echo_stream(gsc_main_window, gsc_transcript_stream);

		gsc_normal_string("Glk transcript is now on.\n");
	}

	else if (sc_strcasecmp(argument, "off") == 0) {
		if (!gsc_transcript_stream) {
			gsc_normal_string("Glk transcript is already off.\n");
			return;
		}

		g_vm->glk_stream_close(gsc_transcript_stream, nullptr);
		gsc_transcript_stream = nullptr;

		g_vm->glk_window_set_echo_stream(gsc_main_window, nullptr);

		gsc_normal_string("Glk transcript is now off.\n");
	}

	else if (strlen(argument) == 0) {
		gsc_normal_string("Glk transcript is ");
		gsc_normal_string(gsc_transcript_stream ? "on" : "off");
		gsc_normal_string(".\n");
	}

	else {
		gsc_normal_string("Glk transcript can be ");
		gsc_standout_string("on");
		gsc_normal_string(", or ");
		gsc_standout_string("off");
		gsc_normal_string(".\n");
	}
}


/*
 * gsc_command_inputlog()
 *
 * Turn game input logging on and off.
 */
static void gsc_command_inputlog(const char *argument) {
	assert(argument);

	if (sc_strcasecmp(argument, "on") == 0) {
		frefid_t fileref;

		if (gsc_inputlog_stream) {
			gsc_normal_string("Glk input logging is already on.\n");
			return;
		}

		fileref = g_vm->glk_fileref_create_by_prompt(fileusage_InputRecord
		          | fileusage_BinaryMode,
		          filemode_WriteAppend, 0);
		if (!fileref) {
			gsc_standout_string("Glk input logging failed.\n");
			return;
		}

		gsc_inputlog_stream = g_vm->glk_stream_open_file(fileref,
		                      filemode_WriteAppend, 0);
		g_vm->glk_fileref_destroy(fileref);
		if (!gsc_inputlog_stream) {
			gsc_standout_string("Glk input logging failed.\n");
			return;
		}

		gsc_normal_string("Glk input logging is now on.\n");
	}

	else if (sc_strcasecmp(argument, "off") == 0) {
		if (!gsc_inputlog_stream) {
			gsc_normal_string("Glk input logging is already off.\n");
			return;
		}

		g_vm->glk_stream_close(gsc_inputlog_stream, nullptr);
		gsc_inputlog_stream = nullptr;

		gsc_normal_string("Glk input log is now off.\n");
	}

	else if (strlen(argument) == 0) {
		gsc_normal_string("Glk input logging is ");
		gsc_normal_string(gsc_inputlog_stream ? "on" : "off");
		gsc_normal_string(".\n");
	}

	else {
		gsc_normal_string("Glk input logging can be ");
		gsc_standout_string("on");
		gsc_normal_string(", or ");
		gsc_standout_string("off");
		gsc_normal_string(".\n");
	}
}


/*
 * gsc_command_readlog()
 *
 * Set the game input log, to read input from a file.
 */
static void gsc_command_readlog(const char *argument) {
	assert(argument);

	if (sc_strcasecmp(argument, "on") == 0) {
		frefid_t fileref;

		if (gsc_readlog_stream) {
			gsc_normal_string("Glk read log is already on.\n");
			return;
		}

		fileref = g_vm->glk_fileref_create_by_prompt(fileusage_InputRecord
		          | fileusage_BinaryMode,
		          filemode_Read, 0);
		if (!fileref) {
			gsc_standout_string("Glk read log failed.\n");
			return;
		}

		if (!g_vm->glk_fileref_does_file_exist(fileref)) {
			g_vm->glk_fileref_destroy(fileref);
			gsc_standout_string("Glk read log failed.\n");
			return;
		}

		gsc_readlog_stream = g_vm->glk_stream_open_file(fileref, filemode_Read, 0);
		g_vm->glk_fileref_destroy(fileref);
		if (!gsc_readlog_stream) {
			gsc_standout_string("Glk read log failed.\n");
			return;
		}

		gsc_normal_string("Glk read log is now on.\n");
	}

	else if (sc_strcasecmp(argument, "off") == 0) {
		if (!gsc_readlog_stream) {
			gsc_normal_string("Glk read log is already off.\n");
			return;
		}

		g_vm->glk_stream_close(gsc_readlog_stream, nullptr);
		gsc_readlog_stream = nullptr;

		gsc_normal_string("Glk read log is now off.\n");
	}

	else if (strlen(argument) == 0) {
		gsc_normal_string("Glk read log is ");
		gsc_normal_string(gsc_readlog_stream ? "on" : "off");
		gsc_normal_string(".\n");
	}

	else {
		gsc_normal_string("Glk read log can be ");
		gsc_standout_string("on");
		gsc_normal_string(", or ");
		gsc_standout_string("off");
		gsc_normal_string(".\n");
	}
}


/*
 * gsc_command_abbreviations()
 *
 * Turn abbreviation expansions on and off.
 */
static void gsc_command_abbreviations(const char *argument) {
	assert(argument);

	if (sc_strcasecmp(argument, "on") == 0) {
		if (gsc_abbreviations_enabled) {
			gsc_normal_string("Glk abbreviation expansions are already on.\n");
			return;
		}

		gsc_abbreviations_enabled = TRUE;
		gsc_normal_string("Glk abbreviation expansions are now on.\n");
	}

	else if (sc_strcasecmp(argument, "off") == 0) {
		if (!gsc_abbreviations_enabled) {
			gsc_normal_string("Glk abbreviation expansions are already off.\n");
			return;
		}

		gsc_abbreviations_enabled = FALSE;
		gsc_normal_string("Glk abbreviation expansions are now off.\n");
	}

	else if (strlen(argument) == 0) {
		gsc_normal_string("Glk abbreviation expansions are ");
		gsc_normal_string(gsc_abbreviations_enabled ? "on" : "off");
		gsc_normal_string(".\n");
	}

	else {
		gsc_normal_string("Glk abbreviation expansions can be ");
		gsc_standout_string("on");
		gsc_normal_string(", or ");
		gsc_standout_string("off");
		gsc_normal_string(".\n");
	}
}


/*
 * gsc_command_print_version_number()
 * gsc_command_version()
 *
 * Print out the Glk library version number.
 */
static void gsc_command_print_version_number(glui32 version) {
	char buffer[64];

	sprintf(buffer, "%lu.%lu.%lu",
	        (unsigned long) version >> 16,
	        (unsigned long)(version >> 8) & 0xff,
	        (unsigned long) version & 0xff);
	gsc_normal_string(buffer);
}

static void gsc_command_version(const char *argument) {
	glui32 version;
	assert(argument);

	gsc_normal_string("This is version ");
	gsc_command_print_version_number(GSC_PORT_VERSION);
	gsc_normal_string(" of the Glk SCARE port.\n");

	version = g_vm->glk_gestalt(gestalt_Version, 0);
	gsc_normal_string("The Glk library version is ");
	gsc_command_print_version_number(version);
	gsc_normal_string(".\n");
}


/*
 * gsc_command_commands()
 *
 * Turn command escapes off.  Once off, there's no way to turn them back on.
 * Commands must be on already to enter this function.
 */
static void gsc_command_commands(const char *argument) {
	assert(argument);

	if (sc_strcasecmp(argument, "on") == 0) {
		gsc_normal_string("Glk commands are already on.\n");
	}

	else if (sc_strcasecmp(argument, "off") == 0) {
		gsc_commands_enabled = FALSE;
		gsc_normal_string("Glk commands are now off.\n");
	}

	else if (strlen(argument) == 0) {
		gsc_normal_string("Glk commands are ");
		gsc_normal_string(gsc_commands_enabled ? "on" : "off");
		gsc_normal_string(".\n");
	}

	else {
		gsc_normal_string("Glk commands can be ");
		gsc_standout_string("on");
		gsc_normal_string(", or ");
		gsc_standout_string("off");
		gsc_normal_string(".\n");
	}
}


/*
 * gsc_command_license()
 *
 * Print licensing terms.
 */
static void gsc_command_license(const char *argument) {
	assert(argument);

	gsc_normal_string("This program is free software; you can redistribute it"
	                  " and/or modify it under the terms of version 2 of the"
	                  " GNU General Public License as published by the Free"
	                  " Software Foundation.\n\n");

	gsc_normal_string("This program is distributed in the hope that it will be"
	                  " useful, but ");
	gsc_standout_string("WITHOUT ANY WARRANTY");
	gsc_normal_string("; without even the implied warranty of ");
	gsc_standout_string("MERCHANTABILITY");
	gsc_normal_string(" or ");
	gsc_standout_string("FITNESS FOR A PARTICULAR PURPOSE");
	gsc_normal_string(".  See the GNU General Public License for more"
	                  " details.\n\n");

	gsc_normal_string("You should have received a copy of the GNU General"
	                  " Public License along with this program; if not, write"
	                  " to the Free Software Foundation, Inc., 51 Franklin"
	                  " Street, Fifth Floor, Boston, MA 02110-1301 USA\n\n");

	gsc_normal_string("Please report any bugs, omissions, or misfeatures to ");
	gsc_standout_string("simon_baldwin@yahoo.com");
	gsc_normal_string(".\n");
}


/* Glk subcommands and handler functions. */
struct gsc_command_t {
	const char *const command;                      /* Glk subcommand. */
	void (* const handler)(const char *argument);   /* Subcommand handler. */
	const int takes_argument;                       /* Argument flag. */
};
typedef gsc_command_t *gsc_commandref_t;

static void gsc_command_summary(const char *argument);
static void gsc_command_help(const char *argument);

static gsc_command_t GSC_COMMAND_TABLE[] = {
	{"summary",        gsc_command_summary,        FALSE},
	{"script",         gsc_command_script,         TRUE},
	{"inputlog",       gsc_command_inputlog,       TRUE},
	{"readlog",        gsc_command_readlog,        TRUE},
	{"abbreviations",  gsc_command_abbreviations,  TRUE},
	{"version",        gsc_command_version,        FALSE},
	{"commands",       gsc_command_commands,       TRUE},
	{"license",        gsc_command_license,        FALSE},
	{"help",           gsc_command_help,           TRUE},
	{nullptr, nullptr, FALSE}
};


/*
 * gsc_command_summary()
 *
 * Report all current Glk settings.
 */
static void gsc_command_summary(const char *argument) {
	gsc_commandref_t entry;
	assert(argument);

	/*
	 * Call handlers that have status to report with an empty argument,
	 * prompting each to print its current setting.
	 */
	for (entry = GSC_COMMAND_TABLE; entry->command; entry++) {
		if (entry->handler == gsc_command_summary
		        || entry->handler == gsc_command_license
		        || entry->handler == gsc_command_help)
			continue;

		entry->handler("");
	}
}


/*
 * gsc_command_help()
 *
 * Document the available Glk commands.
 */
static void gsc_command_help(const char *command) {
	gsc_commandref_t entry, matched;
	assert(command);

	if (strlen(command) == 0) {
		gsc_normal_string("Glk commands are");
		for (entry = GSC_COMMAND_TABLE; entry->command; entry++) {
			gsc_commandref_t next;

			next = entry + 1;
			gsc_normal_string(next->command ? " " : " and ");
			gsc_standout_string(entry->command);
			gsc_normal_string(next->command ? "," : ".\n\n");
		}

		gsc_normal_string("Glk commands may be abbreviated, as long as"
		                  " the abbreviation is unambiguous.  Use ");
		gsc_standout_string("glk help");
		gsc_normal_string(" followed by a Glk command name for help on that"
		                  " command.\n");
		return;
	}

	matched = nullptr;
	for (entry = GSC_COMMAND_TABLE; entry->command; entry++) {
		if (sc_strncasecmp(command, entry->command, strlen(command)) == 0) {
			if (matched) {
				gsc_normal_string("The Glk command ");
				gsc_standout_string(command);
				gsc_normal_string(" is ambiguous.  Try ");
				gsc_standout_string("glk help");
				gsc_normal_string(" for more information.\n");
				return;
			}
			matched = entry;
		}
	}
	if (!matched) {
		gsc_normal_string("The Glk command ");
		gsc_standout_string(command);
		gsc_normal_string(" is not valid.  Try ");
		gsc_standout_string("glk help");
		gsc_normal_string(" for more information.\n");
		return;
	}

	if (matched->handler == gsc_command_summary) {
		gsc_normal_string("Prints a summary of all the current Glk SCARE"
		                  " settings.\n");
	}

	else if (matched->handler == gsc_command_script) {
		gsc_normal_string("Logs the game's output to a file.\n\nUse ");
		gsc_standout_string("glk script on");
		gsc_normal_string(" to begin logging game output, and ");
		gsc_standout_string("glk script off");
		gsc_normal_string(" to end it.  Glk SCARE will ask you for a file"
		                  " when you turn scripts on.\n");
	}

	else if (matched->handler == gsc_command_inputlog) {
		gsc_normal_string("Records the commands you type into a game.\n\nUse ");
		gsc_standout_string("glk inputlog on");
		gsc_normal_string(", to begin recording your commands, and ");
		gsc_standout_string("glk inputlog off");
		gsc_normal_string(" to turn off input logs.  You can play back"
		                  " recorded commands into a game with the ");
		gsc_standout_string("glk readlog");
		gsc_normal_string(" command.\n");
	}

	else if (matched->handler == gsc_command_readlog) {
		gsc_normal_string("Plays back commands recorded with ");
		gsc_standout_string("glk inputlog on");
		gsc_normal_string(".\n\nUse ");
		gsc_standout_string("glk readlog on");
		gsc_normal_string(".  Command play back stops at the end of the"
		                  " file.  You can also play back commands from a"
		                  " text file created using any standard editor.\n");
	}

	else if (matched->handler == gsc_command_abbreviations) {
		gsc_normal_string("Controls abbreviation expansion.\n\nGlk SCARE"
		                  " automatically expands several standard single"
		                  " letter abbreviations for you; for example, \"x\""
		                  " becomes \"examine\".  Use ");
		gsc_standout_string("glk abbreviations on");
		gsc_normal_string(" to turn this feature on, and ");
		gsc_standout_string("glk abbreviations off");
		gsc_normal_string(" to turn it off.  While the feature is on, you"
		                  " can bypass abbreviation expansion for an"
		                  " individual game command by prefixing it with a"
		                  " single quote.\n");
	}

	else if (matched->handler == gsc_command_version) {
		gsc_normal_string("Prints the version numbers of the Glk library"
		                  " and the Glk SCARE port.\n");
	}

	else if (matched->handler == gsc_command_commands) {
		gsc_normal_string("Turn off Glk commands.\n\nUse ");
		gsc_standout_string("glk commands off");
		gsc_normal_string(" to disable all Glk commands, including this one."
		                  "  Once turned off, there is no way to turn Glk"
		                  " commands back on while inside the game.\n");
	}

	else if (matched->handler == gsc_command_license) {
		gsc_normal_string("Prints Glk SCARE's software license.\n");
	}

	else if (matched->handler == gsc_command_help)
		gsc_command_help("");

	else
		gsc_normal_string("There is no help available on that Glk command."
		                  "  Sorry.\n");
}


/*
 * gsc_command_escape()
 *
 * This function is handed each input line.  If the line contains a specific
 * Glk port command, handle it and return TRUE, otherwise return FALSE.
 */
static int gsc_command_escape(const char *string) {
	int posn;
	char *string_copy, *command, *argument;
	assert(string);

	/*
	 * Return FALSE if the string doesn't begin with the Glk command escape
	 * introducer.
	 */
	posn = strspn(string, "\t ");
	if (sc_strncasecmp(string + posn, "glk", strlen("glk")) != 0)
		return FALSE;

	/* Take a copy of the string, without any leading space or introducer. */
	string_copy = (char *)gsc_malloc(strlen(string + posn) + 1 - strlen("glk"));
	strcpy(string_copy, string + posn + strlen("glk"));

	/*
	 * Find the subcommand; the first word in the string copy.  Find its end,
	 * and ensure it terminates with a NUL.
	 */
	posn = strspn(string_copy, "\t ");
	command = string_copy + posn;
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
	if (strlen(command) > 0) {
		gsc_commandref_t entry, matched;
		int matches;

		/*
		 * Search for the first unambiguous table command string matching
		 * the command passed in.
		 */
		matches = 0;
		matched = nullptr;
		for (entry = GSC_COMMAND_TABLE; entry->command; entry++) {
			if (sc_strncasecmp(command, entry->command, strlen(command)) == 0) {
				matches++;
				matched = entry;
			}
		}

		/* If the match was unambiguous, call the command handler. */
		if (matches == 1) {
			gsc_normal_char('\n');
			matched->handler(argument);

			if (!matched->takes_argument && strlen(argument) > 0) {
				gsc_normal_string("[The ");
				gsc_standout_string(matched->command);
				gsc_normal_string(" command ignores arguments.]\n");
			}
		}

		/* No match, or the command was ambiguous. */
		else {
			gsc_normal_string("\nThe Glk command ");
			gsc_standout_string(command);
			gsc_normal_string(" is ");
			gsc_normal_string(matches == 0 ? "not valid" : "ambiguous");
			gsc_normal_string(".  Try ");
			gsc_standout_string("glk help");
			gsc_normal_string(" for more information.\n");
		}
	} else {
		gsc_normal_char('\n');
		gsc_command_help("");
	}

	/* The string contained a Glk command; return TRUE. */
	free(string_copy);
	return TRUE;
}


/*---------------------------------------------------------------------*/
/*  Glk port input functions                                           */
/*---------------------------------------------------------------------*/

/* Quote used to suppress abbreviation expansion and local commands. */
static const char GSC_QUOTED_INPUT = '\'';


/* Table of single-character command abbreviations. */
typedef const struct {
	const char abbreviation;      /* Abbreviation character. */
	const char *const expansion;  /* Expansion string. */
} gsc_abbreviation_t;
typedef gsc_abbreviation_t *gsc_abbreviationref_t;

static gsc_abbreviation_t GSC_ABBREVIATIONS[] = {
	{'c', "close"},    {'g', "again"},  {'i', "inventory"},
	{'k', "attack"},   {'l', "look"},   {'p', "open"},
	{'q', "quit"},     {'r', "drop"},   {'t', "take"},
	{'x', "examine"},  {'y', "yes"},    {'z', "wait"},
	{'\0', nullptr}
};


/*
 * gsc_expand_abbreviations()
 *
 * Expand a few common one-character abbreviations commonly found in other
 * game systems.
 */
static void gsc_expand_abbreviations(char *buffer, int size) {
	char *command, abbreviation;
	const char *expansion;
	gsc_abbreviationref_t entry;
	assert(buffer);

	/* Ignore anything that isn't a single letter command. */
	command = buffer + strspn(buffer, "\t ");
	if (!(strlen(command) == 1
	        || (strlen(command) > 1 && Common::isSpace(command[1]))))
		return;

	/* Scan the abbreviations table for a match. */
	abbreviation = g_vm->glk_char_to_lower((unsigned char) command[0]);
	expansion = nullptr;
	for (entry = GSC_ABBREVIATIONS; entry->expansion; entry++) {
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
		if (strlen(buffer) + strlen(expansion) - 1 >= (unsigned int) size)
			return;

		memmove(command + strlen(expansion) - 1, command, strlen(command) + 1);
		memcpy(command, expansion, strlen(expansion));

		gsc_standout_string("[");
		gsc_standout_char(abbreviation);
		gsc_standout_string(" -> ");
		gsc_standout_string(expansion);
		gsc_standout_string("]\n");
	}
}


/*
 * os_read_line()
 *
 * Read and return a line of player input.
 */
sc_bool os_read_line(sc_char *buffer, sc_int length) {
	sc_int characters;
	assert(buffer && length > 0);

	/* If a help request is pending, provide a user hint. */
	gsc_output_provide_help_hint();

	/*
	 * Ensure normal style, update the status line, and issue an input prompt.
	 */
	gsc_reset_glk_style();
	gsc_status_notify();
	g_vm->glk_put_string(">");

	/*
	 * If we have an input log to read from, use that until it is exhausted.
	 * On end of file, close the stream and resume input from line requests.
	 */
	if (gsc_readlog_stream) {
		glui32 chars;

		/* Get the next line from the log stream. */
		chars = g_vm->glk_get_line_stream(gsc_readlog_stream, buffer, length);
		if (chars > 0) {
			/* Echo the line just read in input style. */
			g_vm->glk_set_style(style_Input);
			gsc_put_buffer(buffer, chars);
			g_vm->glk_set_style(style_Normal);

			/* Return this line as player input. */
			return TRUE;
		}

		/*
		 * We're at the end of the log stream.  Close it, and then continue
		 * on to request a line from Glk.
		 */
		g_vm->glk_stream_close(gsc_readlog_stream, nullptr);
		gsc_readlog_stream = nullptr;
	}

	/*
	 * No input log being read, or we just hit the end of file on one.  Revert
	 * to normal line input; start by getting a new line from Glk.
	 */
	characters = gsc_read_line(buffer, length - 1);
	assert(characters <= length);
	buffer[characters] = '\0';

	/*
	 * If neither abbreviations nor local commands are enabled, use the data
	 * read above without further massaging.
	 */
	if (gsc_abbreviations_enabled || gsc_commands_enabled) {
		char *command;

		/*
		 * If the first non-space input character is a quote, bypass all
		 * abbreviation expansion and local command recognition, and use the
		 * unadulterated input, less introductory quote.
		 */
		command = buffer + strspn(buffer, "\t ");
		if (command[0] == GSC_QUOTED_INPUT) {
			/* Delete the quote with memmove(). */
			memmove(command, command + 1, strlen(command));
		} else {
			/* Check for, and expand, and abbreviated commands. */
			if (gsc_abbreviations_enabled)
				gsc_expand_abbreviations(buffer, length);

			/*
			 * Check for standalone "help", then for Glk port special commands;
			 * suppress the interpreter's use of this input for Glk commands by
			 * returning FALSE.
			 */
			if (gsc_commands_enabled) {
				int posn;

				posn = strspn(buffer, "\t ");
				if (sc_strncasecmp(buffer + posn, "help", strlen("help")) == 0) {
					if (strspn(buffer + posn + strlen("help"), "\t ")
					        == strlen(buffer + posn + strlen("help"))) {
						gsc_output_register_help_request();
					}
				}

				if (gsc_command_escape(buffer)) {
					gsc_output_silence_help_hints();
					return FALSE;
				}
			}
		}
	}

	/*
	 * If there is an input log active, log this input string to it.  Note that
	 * by logging here we get any abbreviation expansions but we won't log glk
	 * special commands, nor any input read from a current open input log.
	 */
	if (gsc_inputlog_stream) {
		g_vm->glk_put_string_stream(gsc_inputlog_stream, buffer);
		g_vm->glk_put_char_stream(gsc_inputlog_stream, '\n');
	}

	return TRUE;
}


/*
 * os_read_line_debug()
 *
 * Read and return a debugger command line.  There's no dedicated debugging
 * window, so this is just a call to the normal readline, with an additional
 * prompt.
 */
sc_bool os_read_line_debug(sc_char *buffer, sc_int length) {
	gsc_output_silence_help_hints();
	gsc_reset_glk_style();
	g_vm->glk_put_string("[SCARE debug]");
	return os_read_line(buffer, length);
}


/*
 * os_confirm()
 *
 * Confirm a game action with a yes/no prompt.
 */
sc_bool os_confirm(sc_int type) {
	sc_char response;

	/*
	 * Always allow game saves and hint display, and if we're reading from an
	 * input log, allow everything no matter what, on the assumption that the
	 * user knows what they are doing.
	 */
	if (gsc_readlog_stream
	        || type == SC_CONF_SAVE || type == SC_CONF_VIEW_HINTS)
		return TRUE;

	/* Ensure back to normal style, and update status. */
	gsc_reset_glk_style();
	gsc_status_notify();

	/* Prompt for the confirmation, based on the type. */
	if (type == GSC_CONF_SUBTLE_HINT)
		g_vm->glk_put_string("View the subtle hint for this topic");
	else if (type == GSC_CONF_UNSUBTLE_HINT)
		g_vm->glk_put_string("View the unsubtle hint for this topic");
	else if (type == GSC_CONF_CONTINUE_HINTS)
		g_vm->glk_put_string("Continue with hints");
	else {
		g_vm->glk_put_string("Do you really want to ");
		switch (type) {
		case SC_CONF_QUIT:
			g_vm->glk_put_string("quit");
			break;
		case SC_CONF_RESTART:
			g_vm->glk_put_string("restart");
			break;
		case SC_CONF_SAVE:
			g_vm->glk_put_string("save");
			break;
		case SC_CONF_RESTORE:
			g_vm->glk_put_string("restore");
			break;
		case SC_CONF_VIEW_HINTS:
			g_vm->glk_put_string("view hints");
			break;
		default:
			g_vm->glk_put_string("do that");
			break;
		}
	}
	g_vm->glk_put_string("? ");

	/* Loop until 'yes' or 'no' returned. */
	do {
		event_t event;

		/* Wait for a standard key, ignoring Glk special keys. */
		do {
			g_vm->glk_request_char_event(gsc_main_window);
			gsc_event_wait(evtype_CharInput, &event);
		} while (event.val1 > BYTE_MAX);
		response = g_vm->glk_char_to_upper(event.val1);
	} while (response != 'Y' && response != 'N');

	/* Echo the confirmation response, and a new line. */
	g_vm->glk_set_style(style_Input);
	g_vm->glk_put_string(response == 'Y' ? "Yes" : "No");
	g_vm->glk_set_style(style_Normal);
	g_vm->glk_put_char('\n');

	/* Use a short delay on restarts, if confirmed. */
	if (type == SC_CONF_RESTART && response == 'Y')
		gsc_short_delay();

	/* Return TRUE if 'Y' was entered. */
	return (response == 'Y');
}


/*---------------------------------------------------------------------*/
/*  Glk port event functions                                           */
/*---------------------------------------------------------------------*/

/* Short delay before restarts; 1s, in 100ms segments. */
static const glui32 GSC_DELAY_TIMEOUT = 100;
static const glui32 GSC_DELAY_TIMEOUTS_COUNT = 10;

/*
 * gsc_short_delay()
 *
 * Delay for a short period; used before restarting a completed game, to
 * improve the display where 'r', or confirming restart, triggers an otherwise
 * immediate, and abrupt, restart.
 */
static void gsc_short_delay() {
	/* Ignore the call if the Glk doesn't have timers. */
	if (g_vm->glk_gestalt(gestalt_Timer, 0)) {
		glui32 timeout;

		/* Timeout in small chunks to minimize Glk jitter. */
		g_vm->glk_request_timer_events(GSC_DELAY_TIMEOUT);
		for (timeout = 0; timeout < GSC_DELAY_TIMEOUTS_COUNT; timeout++) {
			event_t event;

			gsc_event_wait(evtype_Timer, &event);
		}
		g_vm->glk_request_timer_events(0);
	}
}


/*
 * gsc_event_wait_2()
 * gsc_event_wait()
 *
 * Process Glk events until one of the expected type, or types, arrives.
 * Return the event of that type.
 */
static void gsc_event_wait_2(glui32 wait_type_1, glui32 wait_type_2, event_t *event) {
	assert(event);

	do {
		g_vm->glk_select(event);
		if (g_vm->shouldQuit()) {
			g_vm->glk_cancel_line_event(gsc_main_window, event);
			return;
		}

		switch (event->type) {
		case evtype_Arrange:
		case evtype_Redraw:
			/* Refresh any sensitive windows on size events. */
			gsc_status_redraw();
			break;

		default:
			break;
		}
	} while (!(event->type == (EvType)wait_type_1 || event->type == (EvType)wait_type_2));
}

static void gsc_event_wait(glui32 wait_type, event_t *event) {
	assert(event);

	gsc_event_wait_2(wait_type, evtype_None, event);
}


/*---------------------------------------------------------------------*/
/*  Glk port file functions                                            */
/*---------------------------------------------------------------------*/

/*
 * os_open_file()
 *
 * Open a file for save or restore, and return a Glk stream for the opened
 * file.
 */
void *os_open_file(sc_bool is_save) {
	glui32 usage, fmode;
	frefid_t fileref;
	strid_t stream;

	usage = fileusage_SavedGame | fileusage_BinaryMode;
	fmode = is_save ? filemode_Write : filemode_Read;

	fileref = g_vm->glk_fileref_create_by_prompt(usage, (FileMode)fmode, 0);
	if (!fileref)
		return nullptr;

	if (!is_save && !g_vm->glk_fileref_does_file_exist(fileref)) {
		g_vm->glk_fileref_destroy(fileref);
		return nullptr;
	}

	stream = g_vm->glk_stream_open_file(fileref, (FileMode)fmode, 0);
	g_vm->glk_fileref_destroy(fileref);

	return stream;
}


/*
 * os_write_file()
 * os_read_file()
 *
 * Write/read the given buffered data to/from the open Glk stream.
 */
void os_write_file(void *opaque, const sc_byte *buffer, sc_int length) {
	strid_t stream = (strid_t) opaque;
	assert(opaque && buffer);

	g_vm->glk_put_buffer_stream(stream, (const char *)buffer, length);
}

sc_int os_read_file(void *opaque, sc_byte *buffer, sc_int length) {
	strid_t stream = (strid_t) opaque;
	assert(opaque && buffer);

	return g_vm->glk_get_buffer_stream(stream, (char *)buffer, length);
}


/*
 * os_close_file()
 *
 * Close the opened Glk stream.
 */
void os_close_file(void *opaque) {
	strid_t stream = (strid_t) opaque;
	assert(opaque);

	g_vm->glk_stream_close(stream, nullptr);
}


/*---------------------------------------------------------------------*/
/*  main() and options parsing                                         */
/*---------------------------------------------------------------------*/

/* Loading message flush delay timeout. */
static const glui32 GSC_LOADING_TIMEOUT = 100;

/* Enumerated game end options. */
enum gsc_end_option { GAME_RESTART, GAME_UNDO, GAME_QUIT };

/*
 * The following value needs to be passed between the startup_code and main
 * functions.
 */
static const char *gsc_game_message;


/*
 * gsc_callback()
 *
 * Callback function for reading in game and restore file data; fills a
 * buffer with TAF or TAS file data from a Glk stream, and returns the byte
 * count.
 */
static sc_int gsc_callback(void *opaque, sc_byte *buffer, sc_int length) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)opaque;
	assert(stream);

	return stream->read(buffer, length);
}


/*
 * gsc_get_ending_option()
 *
 * Offer the option to restart, undo, or quit.  Returns the selected game
 * end option.  Called on game completion.
 */
static enum gsc_end_option gsc_get_ending_option() {
	sc_char response;

	/* Ensure back to normal style, and update status. */
	gsc_reset_glk_style();
	gsc_status_notify();

	/* Prompt for restart, undo, or quit. */
	g_vm->glk_put_string("\nWould you like to RESTART, UNDO a turn, or QUIT? ");

	/* Loop until 'restart', 'undo' or 'quit'. */
	do
	{
		event_t event;

		do
		{
			g_vm->glk_request_char_event(gsc_main_window);
			gsc_event_wait(evtype_CharInput, &event);
		} while (event.val1 > BYTE_MAX);
		response = g_vm->glk_char_to_upper(event.val1);
	} while (response != 'R' && response != 'U' && response != 'Q');

	/* Echo the confirmation response, and a new line. */
	g_vm->glk_set_style(style_Input);
	switch (response) {
	case 'R':
		g_vm->glk_put_string("Restart");
		break;
	case 'U':
		g_vm->glk_put_string("Undo");
		break;
	case 'Q':
		g_vm->glk_put_string("Quit");
		break;
	default:
		gsc_fatal("GLK: Invalid response encountered");
		g_vm->glk_exit();
	}
	g_vm->glk_set_style(style_Normal);
	g_vm->glk_put_char('\n');

	/* Return the appropriate value for response. */
	switch (response) {
	case 'R':
		return GAME_RESTART;
	case 'U':
		return GAME_UNDO;
	case 'Q':
		return GAME_QUIT;
	default:
		gsc_fatal("GLK: Invalid response encountered");
		g_vm->glk_exit();
	}

	/* Unreachable; supplied to suppress compiler warning. */
	return GAME_QUIT;
}


/*
 * gsc_startup_code()
 * gsc_main
 *
 * Together, these functions take the place of the original main().  The
 * first one is called from the platform-specific startup_code(), to parse
 * and generally handle options.  The second is called from g_vm->glk_main, and
 * does the real work of running the game.
 */
static int gsc_startup_code(Common::SeekableReadStream *game_stream, int restore_slot,
		sc_uint trace_flags, sc_bool enable_debugger, sc_bool stable_random, const sc_char *locale) {
	winid_t window;
	assert(game_stream);

	/* Open a temporary Glk main window. */
	window = g_vm->glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	if (window) {
		/* Clear and initialize the temporary window. */
		g_vm->glk_window_clear(window);
		g_vm->glk_set_window(window);
		g_vm->glk_set_style(style_Normal);

		/*
		 * Display a brief loading game message; here we have to use a timeout
		 * to ensure that the text is flushed to Glk.
		 */
		g_vm->glk_put_string_uni(_("Loading game...\n").u32_str());
		if (g_vm->glk_gestalt(gestalt_Timer, 0)) {
			event_t event;

			g_vm->glk_request_timer_events(GSC_LOADING_TIMEOUT);
			do {
				g_vm->glk_select(&event);
			} while (!g_vm->shouldQuit() && event.type != evtype_Timer);
			g_vm->glk_request_timer_events(0);
		}

		if (g_vm->shouldQuit())
			return false;
	}

	/* If the Glk libarary does not support unicode, disable it. */
	if (!gsc_has_unicode || !g_vm->glk_gestalt(gestalt_Unicode, 0))
		gsc_unicode_enabled = FALSE;

	/*
	 * If a locale was requested, set it in the core interpreter now.  This
	 * locale will preempt any auto-detected one found from inspecting the
	 * game on creation.  After game creation, the Glk locale is synchronized
	 * to the core interpreter's locale.
	 */
	if (locale)
		sc_set_locale(locale);

	/*
	 * Set tracing flags, then try to create a SCARE game reference from the
	 * TAF file.  Since we need this in our call from g_vm->glk_main, we have to keep
	 * it in a module static variable.  If we can't open the TAF file, then
	 * we'll set the pointer to nullptr, and complain about it later in main.
	 * Passing the message string around like this is a nuisance...
	 */
	sc_set_trace_flags(trace_flags);
	gsc_game = sc_game_from_callback(gsc_callback, game_stream);
	if (!gsc_game) {
		gsc_game = nullptr;
		gsc_game_message = "Unable to load an Adrift game from the requested file.";
	} else {
		gsc_game_message = nullptr;
	}

	/*
	 * If the game was created successfully and there is a restore stream, try
	 * to immediately restore the game from that stream.
	 */
	if (gsc_game && restore_slot != -1) {
		if (g_vm->loadGameState(restore_slot).getCode() != Common::kNoError) {
			sc_free_game(gsc_game);
			gsc_game = nullptr;
			gsc_game_message = "Unable to restore this Adrift game from the requested file.";
		} else {
			gsc_game_message = nullptr;
		}
	}

	/* If successful, set game debugging and synchronize to the core's locale. */
	if (gsc_game) {
		sc_set_game_debugger_enabled(gsc_game, enable_debugger);
		gsc_set_locale(sc_get_locale());
	}

	/* Set portable and predictable random number generation if requested. */
	if (stable_random) {
		sc_set_portable_random(TRUE);
		sc_reseed_random_sequence(1);
	}

	/* Close the temporary window. */
	if (window)
		g_vm->glk_window_close(window, nullptr);

	/* Set title of game */
#ifdef GARGLK
	g_vm->garglk_set_story_name(sc_get_game_name(gsc_game));
#endif

	/* Game set up, perhaps successfully. */
	return TRUE;
}

static void gsc_main() {
	sc_bool is_running;
	Context context;

	/* Ensure SCARE internal types have the right sizes. */
	if (!(sizeof(sc_byte) == 1 && sizeof(sc_char) == 1
	        && sizeof(sc_uint) >= 4 && sizeof(sc_int) >= 4
	        && sizeof(sc_uint) <= 8 && sizeof(sc_int) <= 8)) {
		gsc_fatal("GLK: Types sized incorrectly, recompilation is needed");
		g_vm->glk_exit();
	}

	/* Create the Glk window, and set its stream as the current one. */
	gsc_main_window = g_vm->glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	if (!gsc_main_window) {
		gsc_fatal("GLK: Can't open main window");
		g_vm->glk_exit();
	}
	g_vm->glk_window_clear(gsc_main_window);
	g_vm->glk_set_window(gsc_main_window);
	g_vm->glk_set_style(style_Normal);

	/* If there's a problem with the game file, complain now. */
	if (!gsc_game) {
		assert(gsc_game_message);
		gsc_header_string("Glk SCARE Error\n\n");
		gsc_normal_string(gsc_game_message);
		gsc_normal_char('\n');
		g_vm->glk_exit();
	}

	/* Try to create a one-line status window.  We can live without it. */
	g_vm->glk_stylehint_set(wintype_TextGrid, style_User1, stylehint_ReverseColor, 1);
	gsc_status_window = g_vm->glk_window_open(gsc_main_window,
	                    winmethod_Above | winmethod_Fixed,
	                    1, wintype_TextGrid, 0);

	/* Repeat the game until no more restarts requested. */
	is_running = TRUE;
	while (is_running) {
		/* Run the game until it ends, or the user quits. */
		gsc_status_notify();

		if (!context._break)
			sc_interpret_game(context, gsc_game);

		// End point for any context long jump
		context.clear();

		/*
		 * If the game did not complete, the user quit explicitly, so leave the
		 * game repeat loop.
		 */
		if (!sc_has_game_completed(gsc_game)) {
			is_running = FALSE;
			break;
		}

		/*
		 * If reading from an input log, close it now.  We need to request a
		 * user selection, probably modal, and after that we probably don't
		 * want the follow-on readlog data being used as game input.
		 */
		if (gsc_readlog_stream) {
			g_vm->glk_stream_close(gsc_readlog_stream, nullptr);
			gsc_readlog_stream = nullptr;
		}

		/*
		 * Get user selection of restart, undo a turn, or quit completed game.
		 * If undo is unavailable (this should not be possible), degrade to
		 * restart.
		 */
		switch (gsc_get_ending_option()) {
		case GAME_RESTART:
			gsc_short_delay();
			sc_restart_game(context, gsc_game);
			break;

		case GAME_UNDO:
			if (sc_is_game_undo_available(gsc_game)) {
				sc_undo_game_turn(context, gsc_game);
				gsc_normal_string("The previous turn has been undone.\n");
			} else {
				gsc_normal_string("Sorry, no undo is available.\n");
				gsc_short_delay();
				sc_restart_game(context, gsc_game);
			}
			break;

		case GAME_QUIT:
		default:
			is_running = FALSE;
			break;
		}
	}

	/* All done -- release game resources. */
	sc_free_game(gsc_game);

	/* Close any open transcript, input log, and/or read log. */
	if (gsc_transcript_stream) {
		g_vm->glk_stream_close(gsc_transcript_stream, nullptr);
		gsc_transcript_stream = nullptr;
	}
	if (gsc_inputlog_stream) {
		g_vm->glk_stream_close(gsc_inputlog_stream, nullptr);
		gsc_inputlog_stream = nullptr;
	}
	if (gsc_readlog_stream) {
		g_vm->glk_stream_close(gsc_readlog_stream, nullptr);
		gsc_readlog_stream = nullptr;
	}
}


/*---------------------------------------------------------------------*/
/*  Linkage between Glk entry/exit calls and the real interpreter      */
/*---------------------------------------------------------------------*/

/*
 * Safety flags, to ensure we always get startup before main, and that
 * we only get a call to main once.
 */
static int gsc_startup_called = FALSE,
           gsc_main_called = FALSE;

/*
 * adrift_main()
 *
 * Main entry point for Glk.  Here, all startup is done, and we call our
 * function to run the game, or to report errors if gsc_game_message is set.
 */
void adrift_main() {
	assert(gsc_startup_called && !gsc_main_called);
	gsc_main_called = TRUE;

	/* Call the generic interpreter main function. */
	gsc_main();
}


/*---------------------------------------------------------------------*/
/*  Glk linkage relevant only to the UNIX platform                     */
/*---------------------------------------------------------------------*/

#if 0
/*
 * Glk arguments for UNIX versions of the Glk interpreter.
 */
glkunix_argumentlist_t glkunix_arguments[] = {
	{
		(const char *)"-nc", glkunix_arg_NoValue,
		(const char *)"-nc        No local handling for Glk special commands"
	},
	{
		(const char *)"-na", glkunix_arg_NoValue,
		(const char *)"-na        Turn off abbreviation expansions"
	},
	{
		(const char *)"-nu", glkunix_arg_NoValue,
		(const char *)"-nu        Turn off any use of Unicode output"
	},
#ifdef LINUX_GRAPHICS
	{
		(const char *)"-ng", glkunix_arg_NoValue,
		(const char *)"-ng        Turn off attempts at game graphics"
	},
#endif
	{
		(const char *)"-r", glkunix_arg_ValueFollows,
		(const char *)"-r FILE    Restore from FILE on starting the game"
	},
	{
		(const char *)"", glkunix_arg_ValueCanFollow,
		(const char *)"filename   game to run"
	},
	{nullptr, glkunix_arg_End, nullptr}
};
#endif

/*
 * winglk_startup_code()
 *
 * Startup entry point for Windows versions of Glk interpreter.
 */
bool adrift_startup_code(Common::SeekableReadStream *gameFile) {
	const char *locale;
	sc_uint trace_flags;
	sc_bool enable_debugger, stable_random;
	assert(!gsc_startup_called);
	gsc_startup_called = TRUE;

	assert(gameFile);
	trace_flags = 0;
	enable_debugger = gDebugLevel > 0;
	stable_random = gDebugLevel > 0;
	locale = nullptr;

	// Check for savegame to load immediate
	int saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;

	// Use the generic startup code to complete startup
	return gsc_startup_code(gameFile, saveSlot, trace_flags, enable_debugger, stable_random, locale);
}

} // End of namespace Adrift
} // End of namespace Glk
