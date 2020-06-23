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

#include "glk/zcode/config.h"
#include "glk/zcode/detection.h"
#include "glk/glk.h"
#include "common/config-manager.h"
#include "common/textconsole.h"

namespace Glk {
namespace ZCode {

const Header::StoryEntry Header::RECORDS[26] = {
	{       SHERLOCK,  21, "871214" },
	{       SHERLOCK,  26, "880127" },
	{    BEYOND_ZORK,  47, "870915" },
	{    BEYOND_ZORK,  49, "870917" },
	{    BEYOND_ZORK,  51, "870923" },
	{    BEYOND_ZORK,  57, "871221" },
	{      ZORK_ZERO, 296, "881019" },
	{      ZORK_ZERO, 366, "890323" },
	{      ZORK_ZERO, 383, "890602" },
	{      ZORK_ZERO, 393, "890714" },
	{         SHOGUN, 292, "890314" },
	{         SHOGUN, 295, "890321" },
	{         SHOGUN, 311, "890510" },
	{         SHOGUN, 322, "890706" },
	{         ARTHUR,  54, "890606" },
	{         ARTHUR,  63, "890622" },
	{         ARTHUR,  74, "890714" },
	{        JOURNEY,  26, "890316" },
	{        JOURNEY,  30, "890322" },
	{        JOURNEY,  77, "890616" },
	{        JOURNEY,  83, "890706" },
	{ LURKING_HORROR, 203, "870506" },
	{ LURKING_HORROR, 219, "870912" },
	{ LURKING_HORROR, 221, "870918" },
	{      MILLIWAYS, 184, "890412" },
	{        UNKNOWN,   0, "------" }
};

static uint getConfigBool(const Common::String &profileName, bool defaultVal = false) {
	return ConfMan.hasKey(profileName) ? ConfMan.getBool(profileName) : defaultVal;
}

static uint getConfigInt(const Common::String &profileName, uint defaultVal, uint maxVal) {
	uint val = ConfMan.hasKey(profileName) ? ConfMan.getInt(profileName) : defaultVal;
	if (val > maxVal)
		error("Invalid value for configuration value %s", profileName.c_str());

	return val;
}

Header::Header() : h_version(0), h_config(0), h_release(0), h_resident_size(0), h_start_pc(0),
		h_dictionary(0), h_objects(0), h_globals(0), h_dynamic_size(0), h_flags(0),
		h_abbreviations(0), h_file_size(0), h_checksum(0),
		h_interpreter_version(0), h_screen_rows(0), h_screen_cols(0), h_screen_width(0),
		h_screen_height(0), h_font_height(1), h_font_width(1), h_functions_offset(0),
		h_strings_offset(0), h_default_background(0), h_default_foreground(0),
		h_terminating_keys(0), h_line_width(0), h_standard_high(1), h_standard_low(1),
		h_alphabet(0), h_extension_table(0),
		hx_table_size(0), hx_mouse_x(0), hx_mouse_y(0), hx_unicode_table(0),
		hx_flags(0), hx_fore_colour(0), hx_back_colour(0), _storyId(UNKNOWN) {
	Common::fill(&h_serial[0], &h_serial[6], '\0');
	Common::fill(&h_user_name[0], &h_user_name[8], '\0');

	h_interpreter_number = getConfigInt("interpreter_number", INTERP_AMIGA, INTERP_TANDY);
	

	if (ConfMan.hasKey("username")) {
		Common::String username = ConfMan.get("username");
		strncpy((char *)h_user_name, username.c_str(), 7);
	}
}

void Header::loadHeader(Common::SeekableReadStream &f) {
	f.seek(0);
	h_version = f.readByte();
	h_config = f.readByte();

	if (h_version < V1 || h_version > V8)
		error("Unknown Z-code version");

	if (h_version == V3 && (h_config & CONFIG_BYTE_SWAPPED))
		error("Byte swapped story file");

	h_release = f.readUint16BE();
	h_resident_size = f.readUint16BE();
	h_start_pc = f.readUint16BE();
	h_dictionary = f.readUint16BE();
	h_objects = f.readUint16BE();
	h_globals = f.readUint16BE();
	h_dynamic_size = f.readUint16BE();
	h_flags = f.readUint16BE();
	f.read(h_serial, 6);
	
	/* Auto-detect buggy story files that need special fixes */
	_storyId = UNKNOWN;

	for (int i = 0; RECORDS[i]._storyId != UNKNOWN; ++i) {
		if (h_release == RECORDS[i]._release) {
			if (!strncmp((const char *)h_serial, RECORDS[i]._serial, 6)) {
				_storyId = RECORDS[i]._storyId;
				break;
			}
		}
	}

	h_abbreviations = f.readUint16BE();
	h_file_size = f.readUint16BE();
	h_checksum = f.readUint16BE();
	
	f.seek(H_FUNCTIONS_OFFSET);
	h_functions_offset = f.readUint16BE();
	h_strings_offset = f.readUint16BE();
	f.seek(H_TERMINATING_KEYS);
	h_terminating_keys = f.readUint16BE();
	f.seek(H_ALPHABET);
	h_alphabet = f.readUint16BE();
	h_extension_table = f.readUint16BE();


	// Zork Zero Macintosh doesn't have the graphics flag set
	if (_storyId == ZORK_ZERO && h_release == 296)
		h_flags |= GRAPHICS_FLAG;
}

/*--------------------------------------------------------------------------*/

UserOptions::UserOptions() : _undo_slots(MAX_UNDO_SLOTS), _sound(true), _quetzal(true), _color_enabled(false),
	_err_report_mode(ERR_REPORT_ONCE), _ignore_errors(false), _expand_abbreviations(false), _tandyBit(false),
	_piracy(false), _script_cols(0), _left_margin(0), _right_margin(0), _defaultBackground(0), _defaultForeground(0) {
}

void UserOptions::initialize(uint hVersion, uint storyId) {
	_err_report_mode = getConfigInt("err_report_mode", ERR_REPORT_ONCE, ERR_REPORT_FATAL);
	_ignore_errors = getConfigBool("ignore_errors");
	_expand_abbreviations = getConfigBool("expand_abbreviations");
	_tandyBit = getConfigBool("tandy_bit");
	_piracy = getConfigBool("piracy");
	_script_cols = getConfigInt("wrap_script_lines", 80, 999);
	_left_margin = getConfigInt("left_margin", 0, 999);
	_right_margin = getConfigInt("right_margin", 0, 999);

	// Debugging flags
	_attribute_assignment = getConfigBool("attribute_assignment");
	_attribute_testing = getConfigBool("attribute_testing");
	_object_locating = getConfigBool("object_locating");
	_object_movement = getConfigBool("object_movement");

	int defaultFg = hVersion == V6 ? 0 : 0xffffff;
	int defaultBg = hVersion == V6 ? 0xffffff : 0x80;
	if (storyId == BEYOND_ZORK)
		defaultBg = 0;

	defaultFg = getConfigInt("foreground", defaultFg, 0xffffff);
	defaultBg = getConfigInt("background", defaultBg, 0xffffff);

	Graphics::PixelFormat format = g_system->getScreenFormat();
	_defaultForeground = format.RGBToColor((defaultFg >> 16) & 0xff, (defaultFg >> 8) & 0xff, defaultFg & 0xff);
	_defaultBackground = format.RGBToColor((defaultBg >> 16) & 0xff, (defaultBg >> 8) & 0xff, defaultBg & 0xff);
}

bool UserOptions::isInfocom() const {
	return g_vm->getOptions() & OPTION_INFOCOM;
}

} // End of namespace ZCode
} // End of namespace Glk
