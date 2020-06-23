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

#ifndef GLK_ZCODE_CONFIG
#define GLK_ZCODE_CONFIG

#include "glk/zcode/frotz_types.h"

namespace Glk {
namespace ZCode {

/**
 * Configuration flags
 */
enum ConfigFlag {
	CONFIG_BYTE_SWAPPED = 0x01, ///< Story file is byte swapped         - V3 
	CONFIG_TIME         = 0x02, ///< Status line displays time          - V3 
	CONFIG_TWODISKS     = 0x04, ///< Story file occupied two disks      - V3 
	CONFIG_TANDY        = 0x08, ///< Tandy licensed game                - V3 
	CONFIG_NOSTATUSLINE = 0x10, ///< Interpr can't support status lines - V3 
	CONFIG_SPLITSCREEN  = 0x20, ///< Interpr supports split screen mode - V3 
	CONFIG_PROPORTIONAL = 0x40, ///< Interpr uses proportional font     - V3 
	
	CONFIG_COLOUR       = 0x01, ///< Interpr supports colour            - V5+
	CONFIG_PICTURES	    = 0x02, ///< Interpr supports pictures	        - V6 
	CONFIG_BOLDFACE     = 0x04, ///< Interpr supports boldface style    - V4+
	CONFIG_EMPHASIS     = 0x08, ///< Interpr supports emphasis style    - V4+
	CONFIG_FIXED        = 0x10, ///< Interpr supports fixed width style - V4+
	CONFIG_SOUND	    = 0x20, ///< Interpr supports sound             - V6 
	CONFIG_TIMEDINPUT   = 0x80, ///< Interpr supports timed input       - V4+
	
	SCRIPTING_FLAG	  = 0x0001, ///< Outputting to transscription file  - V1+
	FIXED_FONT_FLAG   = 0x0002, ///< Use fixed width font               - V3+
	REFRESH_FLAG 	  = 0x0004, ///< Refresh the screen                 - V6 
	GRAPHICS_FLAG	  = 0x0008, ///< Game wants to use graphics         - V5+
	OLD_SOUND_FLAG	  = 0x0010, ///< Game wants to use sound effects    - V3 
	UNDO_FLAG	  = 0x0010, ///< Game wants to use UNDO feature     - V5+
	MOUSE_FLAG	  = 0x0020, ///< Game wants to use a mouse          - V5+
	COLOUR_FLAG	  = 0x0040, ///< Game wants to use colours          - V5+
	SOUND_FLAG	  = 0x0080, ///< Game wants to use sound effects    - V5+
	MENU_FLAG	  = 0x0100  ///< Game wants to use menus            - V6 
};

/**
 * There are four error reporting modes: never report errors;
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
 * Enumeration of the game header byte indexes
 */
enum HeaderByte {
	H_VERSION             = 0,
	H_CONFIG              = 1,
	H_RELEASE             = 2,
	H_RESIDENT_SIZE       = 4,
	H_START_PC            = 6,
	H_DICTIONARY          = 8,
	H_OBJECTS             = 10,
	H_GLOBALS             = 12,
	H_DYNAMIC_SIZE        = 14,
	H_FLAGS               = 16,
	H_SERIAL              = 18,
	H_ABBREVIATIONS       = 24,
	H_FILE_SIZE           = 26,
	H_CHECKSUM            = 28,
	H_INTERPRETER_NUMBER  = 30,
	H_INTERPRETER_VERSION = 31,
	H_SCREEN_ROWS         = 32,
	H_SCREEN_COLS         = 33,
	H_SCREEN_WIDTH        = 34,
	H_SCREEN_HEIGHT       = 36,
	H_FONT_HEIGHT         = 38,		///< this is the font width in V5
	H_FONT_WIDTH          = 39,		///< this is the font height in V5
	H_FUNCTIONS_OFFSET    = 40,
	H_STRINGS_OFFSET      = 42,
	H_DEFAULT_BACKGROUND  = 44,
	H_DEFAULT_FOREGROUND  = 45,
	H_TERMINATING_KEYS    = 46,
	H_LINE_WIDTH          = 48,
	H_STANDARD_HIGH       = 50,
	H_STANDARD_LOW        = 51,
	H_ALPHABET            = 52,
	H_EXTENSION_TABLE     = 54,
	H_USER_NAME           = 56
};

/**
 * Header extension fields
 */
enum {
	HX_TABLE_SIZE    = 0,
	HX_MOUSE_X       = 1,
	HX_MOUSE_Y       = 2,
	HX_UNICODE_TABLE = 3,
	HX_FLAGS         = 4,
	HX_FORE_COLOUR   = 5,
	HX_BACK_COLOUR   = 6
};

/**
 * User options
 */
struct UserOptions {
	bool _attribute_assignment;
	bool _attribute_testing;
	bool _object_locating;
	bool _object_movement;
	bool _expand_abbreviations;
	bool _ignore_errors;
	bool _piracy;
	bool _quetzal;
	bool _sound;
	bool _tandyBit;
	int _left_margin;
	int _right_margin;
	int _undo_slots;
	int _script_cols;
	int _err_report_mode;
	uint _defaultForeground;
	uint _defaultBackground;
	bool _color_enabled;

	/**
	 * Constructor
	 */
	UserOptions();

	/**
	 * Initializes the options
	 */
	void initialize(uint hVersion, uint storyId);

	/**
	 * Returns true if the game being played is one of the original Infocom releases
	 */
	bool isInfocom() const;
};

/**
 * Story file header data
 */
struct Header {
private:
	struct StoryEntry {
		Story _storyId;
		zword _release;
		char _serial[7];
	};
	static const StoryEntry RECORDS[26];
public:
	zbyte h_version;
	zbyte h_config;
	zword h_release;
	zword h_resident_size;
	zword h_start_pc;
	zword h_dictionary;
	zword h_objects;
	zword h_globals;
	zword h_dynamic_size;
	zword h_flags;
	zbyte h_serial[6];
	zword h_abbreviations;
	zword h_file_size;
	zword h_checksum;
	zbyte h_interpreter_number;
	zbyte h_interpreter_version;
	zbyte h_screen_rows;
	zbyte h_screen_cols;
	zword h_screen_width;
	zword h_screen_height;
	zbyte h_font_height;
	zbyte h_font_width;
	zword h_functions_offset;
	zword h_strings_offset;
	zbyte h_default_background;
	zbyte h_default_foreground;
	zword h_terminating_keys;
	zword h_line_width;
	zbyte h_standard_high;
	zbyte h_standard_low;
	zword h_alphabet;
	zword h_extension_table;
	zbyte h_user_name[8];

	zword hx_table_size;
	zword hx_mouse_x;
	zword hx_mouse_y;
	zword hx_unicode_table;
	zword hx_flags;
	zword hx_fore_colour;
	zword hx_back_colour;

	Story _storyId;

	/**
	 * Constructor
	 */
	Header();

	/**
	 * Load the header
	 */
	void loadHeader(Common::SeekableReadStream &f);
};

} // End of namespace ZCode
} // End of namespace Glk

#endif
