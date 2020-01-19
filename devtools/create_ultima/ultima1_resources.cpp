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

#include "ultima1_resources.h"
#include "file.h"

#define FILE_BUFFER_SIZE 1024
#define MD5_COMPUTE_SIZE 1024

uint32 computeMD5(Common::File &f) {
	uint32 total = 0;
	f.seek(0);
	for (int idx = 0; idx < MD5_COMPUTE_SIZE; ++idx)
		total += f.readByte();

	f.seek(0);
	return total;
}

#define DATA_SEGMENT_OFFSET 0x40D0
#define LOGO_HEIGHT 64
#define LOGO_WIDTH1 199
#define LOGO_WIDTH2 76
#define LOGO_TABLE1 0x3262
#define LOGO_TABLE2 0x4320

void createLogo(Common::File &in, Common::MemFile &out) {
	int offsets[LOGO_HEIGHT][2];
	char buffer[LOGO_WIDTH1 + LOGO_WIDTH2 + 1];

	// Load in the tables
	in.seek(DATA_SEGMENT_OFFSET + LOGO_TABLE1);
	for (int y = 0; y < LOGO_HEIGHT; ++y)
		offsets[y][0] = in.readWord();
	in.seek(DATA_SEGMENT_OFFSET + LOGO_TABLE2);
	for (int y = 0; y < LOGO_HEIGHT; ++y)
		offsets[y][1] = in.readWord();

	// Convert the lines
	for (int y = 0; y < LOGO_HEIGHT; ++y) {
		in.seek(DATA_SEGMENT_OFFSET + offsets[y][0]);
		in.read(buffer, LOGO_WIDTH1 + 1);
		assert(buffer[LOGO_WIDTH1] == '\0');

		in.seek(DATA_SEGMENT_OFFSET + offsets[y][1]);
		in.read(buffer + LOGO_WIDTH1, LOGO_WIDTH2 + 1);
		assert(buffer[LOGO_WIDTH1 + LOGO_WIDTH2] == '\0');

		for (int x = 0; x < (LOGO_WIDTH1 + LOGO_WIDTH2); ++x)
			out.writeByte(buffer[x] == '*' ? 1 : 0);
	}
}

void writeUltima1Resources(Archive &a) {
	// Open up ultima1.exe for logo
	Common::File u1;
	if (!u1.open("Ultima 1/ultima.exe"))
		error("Could not find ultima.exe");
	if (computeMD5(u1) != 64620)
		error("Unknown version of Ultima 1 ultima.exe");

	// Add the Origin logo
	Common::MemFile logo;
	createLogo(u1, logo);
	a.add("ULTIMA1/LOGO", logo);

	u1.close();
}
