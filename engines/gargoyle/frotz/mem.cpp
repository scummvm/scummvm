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
#include "common/textconsole.h"

namespace Gargoyle {
namespace Frotz {

const Mem::StoryEntry Mem::RECORDS[25] = {
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

Mem::Mem() : story_fp(nullptr), blorb_ofs(0), blorb_len(0) {
}

void Mem::initialize() {
/*
	long size;
	zword addr;
	unsigned n;
	int i, j;
	*/
	initializeStoryFile();

	// TODO: More stuff
}

void Mem::initializeStoryFile() {
	Common::SeekableReadStream *f = g_vm->_gameFile;
	giblorb_map_t *map;
	giblorb_result_t res;
	uint32 magic;

	story_fp = f;
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

} // End of namespace Scott
} // End of namespace Gargoyle
