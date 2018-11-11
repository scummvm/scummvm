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

#ifndef GARGOYLE_FROTZ_FROTZ_TYPES
#define GARGOYLE_FROTZ_FROTZ_TYPES

#include "gargoyle/glk_types.h"
#include "common/algorithm.h"

namespace Gargoyle {
namespace Frotz {

#define MAX_UNDO_SLOTS 500
#define STACK_SIZE 20

#define lo(v)	(v & 0xff)
#define hi(v)	(v >> 8)

#define SET_WORD(addr,v)  WRITE_BE_UINT16(addr,v)
#define LOW_WORD(addr,v)  READ_BE_UINT16(addr,v)
#define HIGH_WORD(addr,v) READ_BE_UINT16(addr,v)
#define HIGH_LONG(addr,v) READ_BE_UINT32(addr,v)
#define CODE_WORD(v)       v = g_vm->_mem.readWord()
#define CODE_IDX_WORD(v,i) v = g_vm->_mem.readWord(i)
#define GET_PC(v)          v = g_vm->_mem.getPC()
#define SET_PC(v)          g_vm->_mem.setPC(v);

/* There are four error reporting modes: never report errors;
 * report only the first time a given error type occurs;
 * report every time an error occurs;
 * or treat all errors as fatal errors, killing the interpreter.
 * I strongly recommend "report once" as the default. But you can compile in a
 * different default by changing the definition of ERR_DEFAULT_REPORT_MODE.
 */
enum ErrorReport {
	ERR_REPORT_NEVER  = 0,
	ERR_REPORT_ONCE   = 1,
	ERR_REPORT_ALWAYS = 2,
	ERR_REPORT_FATAL  = 3,

	ERR_DEFAULT_REPORT_MODE = ERR_REPORT_NEVER
};

/**
 * Character codes
 */
enum ZCode {
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
	UNKNOWN
};

typedef byte zbyte;
typedef uint zchar;
typedef uint16 zword;

/**
 * User options
 */
struct UserOptions {
	int _attribute_assignment;
	int _attribute_testing;
	int _context_lines;
	int _object_locating;
	int _object_movement;
	int _left_margin;
	int _right_margin;
	bool _ignore_errors;
	bool _piracy;
	int _undo_slots;
	int _expand_abbreviations;
	int _script_cols;
	bool _save_quetzal;
	int _err_report_mode;
	bool _sound;

	UserOptions() : _attribute_assignment(0), _attribute_testing(0),
		_context_lines(0), _object_locating(0), _object_movement(0),
		_left_margin(0), _right_margin(0), _ignore_errors(false), _piracy(false),
		_undo_slots(MAX_UNDO_SLOTS), _expand_abbreviations(0), _script_cols(80),
		_save_quetzal(true),
		_err_report_mode(ERR_DEFAULT_REPORT_MODE), _sound(true) {
	}
};

} // End of namespace Frotz
} // End of namespace Gargoyle

#endif
