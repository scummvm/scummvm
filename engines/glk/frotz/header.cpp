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

#include "glk/frotz/header.h"
#include "common/textconsole.h"

namespace Glk {
namespace Frotz {

const Header::StoryEntry Header::RECORDS[25] = {
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
	{        UNKNOWN,   0, "------" }
};

void Header::loadHeader(Common::SeekableReadStream &f) {
	h_version = f.readByte();
	h_config = f.readByte();

	if (h_version < V1 || h_version > V8)
		error("Unknown Z-code version");

	if (h_version == V6)
		error("Cannot play Z-code version 6");

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

} // End of namespace Scott
} // End of namespace Glk
