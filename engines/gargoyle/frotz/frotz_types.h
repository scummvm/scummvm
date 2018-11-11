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
	STORY_BEYOND_ZORK,
	STORY_SHERLOCK,
	STORY_ZORK_ZERO,
	STORY_SHOGUN,
	STORY_ARTHUR,
	STORY_JOURNEY,
	STORY_LURKING_HORROR,
	STORY_UNKNOWN
};

typedef byte zbyte;
typedef char zchar;
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

/**
 * Story file header data
 */
struct Header {
	zbyte _version;
	zbyte _config;
	zword _release;
	zword _resident_size;
	zword _start_pc;
	zword _dictionary;
	zword _objects;
	zword _globals;
	zword _dynamic_size;
	zword _flags;
	zbyte _serial[6];
	zword _abbreviations;
	zword _file_size;
	zword _checksum;
	zbyte _interpreter_number;
	zbyte _interpreter_version;
	zbyte _screen_rows;
	zbyte _screen_cols;
	zword _screen_width;
	zword _screen_height;
	zbyte _font_height = 1;
	zbyte _font_width = 1;
	zword _functions_offset;
	zword _strings_offset;
	zbyte _default_background;
	zbyte _default_foreground;
	zword _terminating_keys;
	zword _line_width;
	zbyte _standard_high;
	zbyte _standard_low;
	zword _alphabet;
	zword _extension_table;
	zbyte _user_name[8];

	zword _hx_table_size;
	zword _hx_mouse_x;
	zword _hx_mouse_y;
	zword _hx_unicode_table;
	zword _hx_flags;
	zword _hx_fore_colour;
	zword _hx_back_colour;

	Header() : _version(0), _config(0), _release(0), _resident_size(0), _start_pc(0),
			_dictionary(0), _objects(0), _globals(0), _dynamic_size(0), _flags(0),
			_abbreviations(0), _file_size(0), _checksum(0), _interpreter_number(0),
			_interpreter_version(0), _screen_rows(0), _screen_cols(0), _screen_width(0),
			_screen_height(0), _font_height(1), _font_width(1), _functions_offset(0),
			_strings_offset(0), _default_background(0), _default_foreground(0),
			_terminating_keys(0), _line_width(0), _standard_high(1), _standard_low(1),
			_alphabet(0), _extension_table(0),
			_hx_table_size(0), _hx_mouse_x(0), _hx_mouse_y(0), _hx_unicode_table(0),
			_hx_flags(0), _hx_fore_colour(0), _hx_back_colour(0) {
		Common::fill(&_serial[0], &_serial[6], '\0');
		Common::fill(&_user_name[0], &_user_name[8], '\0');
	}
};

} // End of namespace Frotz
} // End of namespace Gargoyle

#endif
