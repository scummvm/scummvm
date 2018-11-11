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


/**
 * Story file header data
 */
struct Header {
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

	Header() : h_version(0), h_config(0), h_release(0), h_resident_size(0), h_start_pc(0),
			h_dictionary(0), h_objects(0), h_globals(0), h_dynamic_size(0), h_flags(0),
			h_abbreviations(0), h_file_size(0), h_checksum(0), h_interpreter_number(0),
			h_interpreter_version(0), h_screen_rows(0), h_screen_cols(0), h_screen_width(0),
			h_screen_height(0), h_font_height(1), h_font_width(1), h_functions_offset(0),
			h_strings_offset(0), h_default_background(0), h_default_foreground(0),
			h_terminating_keys(0), h_line_width(0), h_standard_high(1), h_standard_low(1),
			h_alphabet(0), h_extension_table(0),
			hx_table_size(0), hx_mouse_x(0), hx_mouse_y(0), hx_unicode_table(0),
			hx_flags(0), hx_fore_colour(0), hx_back_colour(0) {
		Common::fill(&h_serial[0], &h_serial[6], '\0');
		Common::fill(&h_user_name[0], &h_user_name[8], '\0');
	}
};

class Mem : public Header {
	struct StoryEntry {
		Story _storyId;
		zword _release;
		char _serial[7];
	};
	static const StoryEntry RECORDS[25];
private:
	Common::SeekableReadStream *story_fp;
	uint blorb_ofs, blorb_len;
	byte *pcp;
	byte *zmp;
private:
	/**
	 * Handles setting the story file, parsing it if it's a Blorb file
	 */
	void initializeStoryFile();
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
};

} // End of namespace Frotz
} // End of namespace Gargoyle

#endif
