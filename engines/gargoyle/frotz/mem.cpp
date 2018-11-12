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

#include "gargoyle/frotz/mem.h"
#include "gargoyle/frotz/frotz.h"
#include "common/memstream.h"
#include "common/textconsole.h"

namespace Gargoyle {
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

/*--------------------------------------------------------------------------*/

Mem::Mem() : story_fp(nullptr), blorb_ofs(0), blorb_len(0), story_size(0),
		first_undo(nullptr), last_undo(nullptr), curr_undo(nullptr),
		undo_mem(nullptr), prev_zmp(nullptr), undo_diff(nullptr),
		undo_count(0), reserve_mem(0) {
}

void Mem::initialize() {
	initializeStoryFile();
	initializeUndo();
	loadGameHeader();

	// Allocate memory for story data
	if ((zmp = (zbyte *)realloc(zmp, story_size)) == nullptr)
		error("Out of memory");

	// Load story file in chunks of 32KB
	uint n = 0x8000;

	for (uint size = 64; size < story_size; size += n) {
		if (story_size - size < 0x8000)
			n = story_size - size;

		setPC(size);

		if (story_fp->read(pcp, n) != n)
			error("Story file read error");

	}

	// Read header extension table
	hx_table_size = get_header_extension(HX_TABLE_SIZE);
	hx_unicode_table = get_header_extension(HX_UNICODE_TABLE);
	hx_flags = get_header_extension(HX_FLAGS);
}

void Mem::initializeStoryFile() {
	Common::SeekableReadStream *f = story_fp;
	giblorb_map_t *map;
	giblorb_result_t res;
	uint32 magic;

	magic = f->readUint32BE();

	if (magic == MKTAG('F', 'O', 'R', 'M')) {
		if (g_vm->giblorb_set_resource_map(f))
			error("This Blorb file seems to be invalid.");

		map = g_vm->giblorb_get_resource_map();

		if (g_vm->giblorb_load_resource(map, giblorb_method_FilePos, &res, giblorb_ID_Exec, 0))
			error("This Blorb file does not contain an executable chunk.");
		if (res.chunktype != MKTAG('Z', 'C', 'O', 'D'))
			error("This Blorb file contains an executable chunk, but it is not a Z-code file.");

		blorb_ofs = res.data.startpos;
		blorb_len = res.length;
	} else {
		blorb_ofs = 0;
		blorb_len = f->size();
	}

	if (blorb_len < 64)
		error("This file is too small to be a Z-code file.");
}

void Mem::initializeUndo() {
	void *reserved = nullptr;

	if (reserve_mem != 0) {
		if ((reserved = malloc(reserve_mem)) == NULL)
			return;
	}

	// Allocate h_dynamic_size bytes for previous dynamic zmp state
	// + 1.5 h_dynamic_size for Quetzal diff + 2.
	undo_mem = new zbyte[(h_dynamic_size * 5) / 2 + 2];
	if (undo_mem != nullptr) {
		prev_zmp = undo_mem;
		undo_diff = undo_mem + h_dynamic_size;
		memcpy(prev_zmp, zmp, h_dynamic_size);
	} else {
		_undo_slots = 0;
	}

	if (reserve_mem != 0)
		delete reserved;
}

void Mem::loadGameHeader() {
	// Load header
	zmp = new byte[64];
	story_fp->seek(blorb_ofs);
	story_fp->read(zmp, 64);

	Common::MemoryReadStream h(zmp, 64);
	loadHeader(h);

	// Calculate story file size in bytes
	if (h_file_size != 0) {
		story_size = (long)2 * h_file_size;

		if (h_version >= V4)
			story_size *= 2;
		if (h_version >= V6)
			story_size *= 2;
	} else {
		// Some old games lack the file size entry
		story_size = blorb_len;
	}
}

zword Mem::get_header_extension(int entry) {
	zword addr;
	zword val;

	if (h_extension_table == 0 || entry > hx_table_size)
		return 0;

	addr = h_extension_table + 2 * entry;
	LOW_WORD(addr, val);

	return val;   
}

} // End of namespace Scott
} // End of namespace Gargoyle
