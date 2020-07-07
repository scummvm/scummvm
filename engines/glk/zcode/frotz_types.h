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

#ifndef GLK_ZCODE_FROTZ_TYPES
#define GLK_ZCODE_FROTZ_TYPES

#include "glk/glk_types.h"
#include "common/algorithm.h"

namespace Glk {
namespace ZCode {

#define MAX_UNDO_SLOTS 500
#define STACK_SIZE 32768

#define lo(v)	(v & 0xff)
#define hi(v)	(v >> 8)

/**
 * Character codes
 */
enum ZCodeKey {
	ZC_TIME_OUT      = 0x00,
	ZC_NEW_STYLE     = 0x01,
	ZC_NEW_FONT      = 0x02,
	ZC_BACKSPACE     = 0x08,
	ZC_INDENT        = 0x09,
	ZC_GAP           = 0x0b,
	ZC_RETURN        = 0x0d,
	ZC_HKEY_MIN      = 0x0e,
	ZC_HKEY_RECORD   = 0x0e,
	ZC_HKEY_PLAYBACK = 0x0f,
	ZC_HKEY_SEED     = 0x10,
	ZC_HKEY_UNDO     = 0x11,
	ZC_HKEY_RESTART  = 0x12,
	ZC_HKEY_QUIT     = 0x13,
	ZC_HKEY_DEBUG    = 0x14,
	ZC_HKEY_HELP     = 0x15,
	ZC_HKEY_MAX      = 0x15,
	ZC_ESCAPE        = 0x1b,
	ZC_ASCII_MIN     = 0x20,
	ZC_ASCII_MAX     = 0x7e,
	ZC_BAD           = 0x7f,
	ZC_ARROW_MIN     = 0x81,
	ZC_ARROW_UP      = 0x81,
	ZC_ARROW_DOWN    = 0x82,
	ZC_ARROW_LEFT    = 0x83,
	ZC_ARROW_RIGHT   = 0x84,
	ZC_ARROW_MAX     = 0x84,
	ZC_FKEY_MIN      = 0x85,
	ZC_FKEY_MAX      = 0x90,
	ZC_NUMPAD_MIN    = 0x91,
	ZC_NUMPAD_MAX    = 0x9a,
	ZC_SINGLE_CLICK  = 0x9b,
	ZC_DOUBLE_CLICK  = 0x9c,
	ZC_MENU_CLICK    = 0x9d,
	ZC_LATIN1_MIN    = 0xa0,
	ZC_LATIN1_MAX    = 0xff
};

enum Story {
	BEYOND_ZORK,
	SHERLOCK,
	ZORK_ZERO,
	SHOGUN,
	ARTHUR,
	JOURNEY,
	LURKING_HORROR,
	MILLIWAYS,
	UNKNOWN
};

enum Version {
	V1 = 1,
	V2 = 2,
	V3 = 3,
	V4 = 4,
	V5 = 5,
	V6 = 6,
	V7 = 7,
	V8 = 8,
	V9 = 9
};

enum {
	TRANSPARENT_FLAG = 0x0001 ///< Game wants to use transparency     - V6
};

enum ErrorCode {
	ERR_TEXT_BUF_OVF   = 1,		///< Text buffer overflow
	ERR_STORE_RANGE    = 2,		///< Store out of dynamic memory
	ERR_DIV_ZERO       = 3,		///< Division by zero
	ERR_ILL_OBJ        = 4,		///< Illegal object
	ERR_ILL_ATTR       = 5,		///< Illegal attribute
	ERR_NO_PROP        = 6,		///< No such property
	ERR_STK_OVF        = 7,		///< Stack overflow
	ERR_ILL_CALL_ADDR  = 8,		///< Call to illegal address
	ERR_CALL_NON_RTN   = 9,		///< Call to non-routine
	ERR_STK_UNDF       = 10,	///< Stack underflow
	ERR_ILL_OPCODE     = 11,	///< Illegal opcode
	ERR_BAD_FRAME      = 12,	///< Bad stack frame
	ERR_ILL_JUMP_ADDR  = 13,	///< Jump to illegal address
	ERR_SAVE_IN_INTER  = 14,	///< Can't save while in interrupt
	ERR_STR3_NESTING   = 15,	///< Nesting stream #3 too deep
	ERR_ILL_WIN        = 16,	///< Illegal window
	ERR_ILL_WIN_PROP   = 17,	///< Illegal window property
	ERR_ILL_PRINT_ADDR = 18,	///< Print at illegal address
	ERR_DICT_LEN       = 19,	///< Illegal dictionary word length
	ERR_MAX_FATAL      = 19,

	// Less serious errors
	ERR_JIN_0            = 20,	///< @jin called with object 0
	ERR_GET_CHILD_0      = 21,	///< @get_child called with object 0
	ERR_GET_PARENT_0     = 22,	///< @get_parent called with object 0
	ERR_GET_SIBLING_0    = 23,	///< @get_sibling called with object 0
	ERR_GET_PROP_ADDR_0  = 24,	///< @get_prop_addr called with object 0
	ERR_GET_PROP_0       = 25,	///< @get_prop called with object 0
	ERR_PUT_PROP_0       = 26,	///< @put_prop called with object 0
	ERR_CLEAR_ATTR_0     = 27,	///< @clear_attr called with object 0
	ERR_SET_ATTR_0       = 28,	///< @set_attr called with object 0
	ERR_TEST_ATTR_0      = 29,	///< @test_attr called with object 0
	ERR_MOVE_OBJECT_0    = 30,	///< @move_object called moving object 0
	ERR_MOVE_OBJECT_TO_0 = 31,	///< @move_object called moving into object 0
	ERR_REMOVE_OBJECT_0  = 32,	///< @remove_object called with object 0
	ERR_GET_NEXT_PROP_0  = 33,	///< @get_next_prop called with object 0
	ERR_NUM_ERRORS       = 33
};

enum FrotzInterp {
	INTERP_DEFAULT    =  0,
	INTERP_DEC_20     =  1,
	INTERP_APPLE_IIE  =  2,
	INTERP_MACINTOSH  =  3,
	INTERP_AMIGA      =  4,
	INTERP_ATARI_ST   =  5,
	INTERP_MSDOS      =  6,
	INTERP_CBM_128    =  7,
	INTERP_CBM_64     =  8,
	INTERP_APPLE_IIC  =  9,
	INTERP_APPLE_IIGS = 10,
	INTERP_TANDY      = 11
};

enum Colour {
	BLACK_COLOUR       = 2,
	RED_COLOUR         = 3,
	GREEN_COLOUR       = 4,
	YELLOW_COLOUR      = 5,
	BLUE_COLOUR        = 6,
	MAGENTA_COLOUR     = 7,
	CYAN_COLOUR        = 8,
	WHITE_COLOUR       = 9,
	GREY_COLOUR        = 10,	///< INTERP_MSDOS only
	LIGHTGREY_COLOUR   = 10, 	///< INTERP_AMIGA only
	MEDIUMGREY_COLOUR  = 11, 	///< INTERP_AMIGA only
	DARKGREY_COLOUR    = 12,	///< INTERP_AMIGA only
	TRANSPARENT_COLOUR = 15		///< ZSpec 1.1
};

enum Style {
	REVERSE_STYLE     = 1,
	BOLDFACE_STYLE    = 2,
	EMPHASIS_STYLE    = 4,
	FIXED_WIDTH_STYLE = 8
};

enum FontStyle {
	PREVIOUS_FONT    = 0,
	TEXT_FONT        = 1,
	PICTURE_FONT     = 2,
	GRAPHICS_FONT    = 3,
	FIXED_WIDTH_FONT = 4
};

/*** Constants for os_beep */

#define BEEP_HIGH       1
#define BEEP_LOW        2

/*** Constants for os_menu */

#define MENU_NEW 0
#define MENU_ADD 1
#define MENU_REMOVE 2

typedef byte zbyte;
typedef uint32 zchar;
typedef uint16 zword;

#define MAX_NESTING 16
struct Redirect {
	zword _xSize;
	zword _table;
	zword _width;
	zword _total;

	Redirect() : _xSize(0), _table(0), _width(0), _total(0) {}
	Redirect(zword xSize, zword table, zword width = 0, zword total = 0) :
		_xSize(xSize), _table(table), _width(width), _total(total) {}
};

} // End of namespace ZCode
} // End of namespace Glk

#endif
