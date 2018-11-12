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

#ifndef GARGOYLE_FROTZ_MEM
#define GARGOYLE_FROTZ_MEM

#include "gargoyle/frotz/frotz_types.h"

namespace Gargoyle {
namespace Frotz {

#define SET_WORD(addr,v)  zmp[addr] = hi(v); zmp[addr+1] = lo(v)
#define LOW_WORD(addr,v)  v = READ_BE_UINT16(&zmp[addr])
#define HIGH_WORD(addr,v) v = READ_BE_UINT16(&zmp[addr])
#define HIGH_LONG(addr,v) v = READ_BE_UINT32(&zmp[addr])
#define SET_BYTE(addr,v)   zmp[addr] = v
#define LOW_BYTE(addr,v)   v = zmp[addr]

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
 * Stores undo information
 */
struct undo_struct {
	undo_struct *next;
	undo_struct *prev;
	long pc;
	long diff_size;
	zword frame_count;
	zword stack_size;
	zword frame_offset;
	// undo diff and stack data follow
};
typedef undo_struct undo_t;

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
	static const StoryEntry RECORDS[25];
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
	zbyte h_font_height = 1;
	zbyte h_font_width = 1;
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
	Header() : h_version(0), h_config(0), h_release(0), h_resident_size(0), h_start_pc(0),
			h_dictionary(0), h_objects(0), h_globals(0), h_dynamic_size(0), h_flags(0),
			h_abbreviations(0), h_file_size(0), h_checksum(0), h_interpreter_number(0),
			h_interpreter_version(0), h_screen_rows(0), h_screen_cols(0), h_screen_width(0),
			h_screen_height(0), h_font_height(1), h_font_width(1), h_functions_offset(0),
			h_strings_offset(0), h_default_background(0), h_default_foreground(0),
			h_terminating_keys(0), h_line_width(0), h_standard_high(1), h_standard_low(1),
			h_alphabet(0), h_extension_table(0),
			hx_table_size(0), hx_mouse_x(0), hx_mouse_y(0), hx_unicode_table(0),
			hx_flags(0), hx_fore_colour(0), hx_back_colour(0), _storyId(UNKNOWN) {
		Common::fill(&h_serial[0], &h_serial[6], '\0');
		Common::fill(&h_user_name[0], &h_user_name[8], '\0');
	}

	/**
	 * Load the header
	 */
	void loadHeader(Common::SeekableReadStream &f);
};

class Mem : public Header, public virtual UserOptions {
protected:
	Common::SeekableReadStream *story_fp;
	uint blorb_ofs, blorb_len;
	uint story_size;
	byte *pcp;
	byte *zmp;

	undo_t *first_undo, *last_undo, *curr_undo;
	zbyte *undo_mem, *prev_zmp, *undo_diff;
	int undo_count;
	int reserve_mem;
private:
	/**
	 * Handles setting the story file, parsing it if it's a Blorb file
	 */
	void initializeStoryFile();

	/**
	 * Setup undo data
	 */
	void initializeUndo();

	/**
	 * Handles loading the game header
	 */
	void loadGameHeader();
public:
	/**
	 * Constructor
	 */
	Mem();

	/**
	 * Initialize
	 */
	void initialize();

	/**
	 * Read a word
	 */
	zword readWord() {
		pcp += 2;
		return READ_BE_UINT16(pcp - 2);
	}

	/**
	 * Read a word at a given index relative to pcp
	 */
	zword readWord(size_t ofs) {
		return READ_BE_UINT16(pcp + ofs);
	}

	/**
	 * Get the PC
	 */
	uint getPC() const { return pcp - zmp; }

	/**
	 * Set the PC
	 */
	void setPC(uint ofs) { pcp = zmp + ofs; }

	/**
	 * Read a value from the header extension (former mouse table).
	 */
	zword get_header_extension(int entry);
};

} // End of namespace Frotz
} // End of namespace Gargoyle

#endif
