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

#include "create_ultima.h"
#include "file.h"

#define FILE_BUFFER_SIZE 1024

typedef unsigned int uint32;

#define DATA_SEGMENT_OFFSET 0x40D0
#define LOGO_HEIGHT 64
#define LOGO_WIDTH1 199
#define LOGO_WIDTH2 76
#define LOGO_TABLE1 0x3262
#define LOGO_TABLE2 0x4320
#define FLAG_WIDTH 8
#define FLAG_HEIGHT 8
#define NUM_FLAGS 3

// Creates the logo.bmp file
void createLogo(File &in) {
	int offsets[LOGO_HEIGHT][2];
	char buffer[LOGO_WIDTH1 + LOGO_WIDTH2 + 1];

	// Load in the tables
	in.seek(DATA_SEGMENT_OFFSET + LOGO_TABLE1);
	for (int y = 0; y < LOGO_HEIGHT; ++y)
		offsets[y][0] = in.readWord();
	in.seek(DATA_SEGMENT_OFFSET + LOGO_TABLE2);
	for (int y = 0; y < LOGO_HEIGHT; ++y)
		offsets[y][1] = in.readWord();

	// Set up a surface
	Surface out(LOGO_WIDTH1 + LOGO_WIDTH2, LOGO_HEIGHT);
	out.setPaletteEntry(0, 0, 0, 0);
	out.setPaletteEntry(1, 0xff, 0xff, 0xff);

	// Convert the lines
	for (int y = 0; y < LOGO_HEIGHT; ++y) {
		in.seek(DATA_SEGMENT_OFFSET + offsets[y][0]);
		in.read(buffer, LOGO_WIDTH1 + 1);
		assert(buffer[LOGO_WIDTH1] == '\0');

		in.seek(DATA_SEGMENT_OFFSET + offsets[y][1]);
		in.read(buffer + LOGO_WIDTH1, LOGO_WIDTH2 + 1);
		assert(buffer[LOGO_WIDTH1 + LOGO_WIDTH2] == '\0');

		byte *line = out.getBasePtr(0, y);
		for (int x = 0; x < (LOGO_WIDTH1 + LOGO_WIDTH2); ++x, ++line)
			*line = buffer[x] == '*' ? 1 : 0;
	}

	out.saveToFile("logo.bmp");
}

// Creates the flags.bmp file
void createFlags(File &in) {
	Surface out(FLAG_WIDTH * NUM_FLAGS, FLAG_HEIGHT);
	out.setPaletteEntry(10, 0, 0, 0);
	out.setPaletteEntry(11, 0xff, 0xff, 0xff);

	in.seek(DATA_SEGMENT_OFFSET + 0x124);

	for (int flagNum = 0; flagNum < NUM_FLAGS; ++flagNum) {
		for (int yp = 0; yp < FLAG_HEIGHT; ++yp) {
			byte *line = out.getBasePtr(flagNum * FLAG_WIDTH, yp);
			for (int xp = 0; xp < FLAG_WIDTH; ++xp)
				*line++ = in.readByte() ? 10 : 11;
		}
	}

	out.saveToFile("flags.bmp");
}

void extractUltima1Resources() {
	// Open up ultima1.exe for logo
	File u1("ultima.exe");
	if (u1.computeMD5() != 64620)
		error("Unknown version of Ultima 1 ultima.exe");

	// Extract the Origin logo and flag animation data
	createLogo(u1);
	createFlags(u1);
}
