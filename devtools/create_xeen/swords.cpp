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

 // Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include "file.h"
#include "swords.h"

#define MONSTERS_COUNT 150

void writeSwordsData(CCArchive &cc, const char *swordsDatName) {
	Common::File f;
	Common::MemFile monsters;
	const size_t size = MONSTERS_COUNT * 60;
	const int32 offset = 0x44200;
	byte buffer[size];

	if (!f.open(swordsDatName, Common::kFileReadMode))
		error("Could not open '%s'", swordsDatName);

	if (f.seek(offset) != 0)
		error("Failed to seek to 0x%x for '%s'", offset, swordsDatName);

	if (f.read(buffer, size) != size)
		error("Failed to read %zu bytes from '%s'", size, swordsDatName);

	if (strcmp((const char *)buffer + 0x33, "Slime"))
		error("Invalid '%s'", swordsDatName);

	monsters.write(buffer, size);
	cc.add("monsters.swd", monsters);
}
