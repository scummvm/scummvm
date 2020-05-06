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

void extractStringTable(File &src, const char *filename, int offset, int count) {
	if (offset != -1)
		src.seek(offset);

	WriteFile dest(filename);
	char c;

	for (int idx = 0; idx < count; ++idx) {
		if (offset == 87754 && idx == 19) {
			// String entry #19 for Lord British is dodgy in the original data
			const char *STR = "\n\n\n\n\nHe says:\nThe Great Stygian Abyss is the darkest pocket of evil "
				"remaining in Britannia!\n\n\n\n\nIt is said that in the deepest recesses of "
				"the Abyss is the Chamber of the Codex!\n\n\n\nIt is also said that only one "
				"of highest Virtue may enter this Chamber, one such as an Avatar!!!\n";
			dest.write(STR, (int)strlen(STR));
			dest.writeByte(0);

			// Skip to next line
			while (src.readByte()) {}
			while (src.readByte()) {}
			continue;
		}

		do {
			c = src.readByte();
			dest.writeByte(c);
		} while (c != '\0');
	}
}

void extractData(File &src, const char *filename, int offset, int size) {
	if (offset != -1)
		src.seek(offset);

	WriteFile dest(filename);
	dest.write(src, size);
}

void extractUltima4Resources() {
	// Extract string tables from avatar.exe
	File f2("avatar.exe");
	if (f2.computeMD5() != 55967)
		error("Unknown version of Ultima 4 avatar.exe");

	// Extract string tables
	extractStringTable(f2, "hawkwind.dat", 74729, 53);
	extractStringTable(f2, "lb_keywords.dat", 87581, 24);
	extractStringTable(f2, "lb_text.dat", 87754, 24);
	extractStringTable(f2, "virtue.dat", 0x0fc7b, 11);
	extractStringTable(f2, "endgame1.dat", 0x0fee4, 7);
	extractStringTable(f2, "endgame2.dat", 0x10187, 5);
	extractStringTable(f2, "shrines.dat", 93682, 24);

	// Extract string tables from title.exe
	File f1("title.exe");
	if (f1.computeMD5() != 115195)
		error("Unknown version of Ultima 4 title.exe");

	const int INTRO_TEXT_OFFSET = 17445 - 1;  // (start at zero)
	extractStringTable(f1, "intro_questions.dat", INTRO_TEXT_OFFSET, 28);
	extractStringTable(f1, "intro_text.dat", -1, 24);
	extractStringTable(f1, "intro_gypsy.dat", -1, 15);

	// Extract miscellaneous data from the title.exe
	#define INTRO_MAP_OFFSET 30339
	#define INTRO_FIXUPDATA_OFFSET 29806
	#define INTRO_SCRIPT_TABLE_SIZE 548
	#define INTRO_SCRIPT_TABLE_OFFSET 30434
	#define INTRO_BASETILE_TABLE_SIZE 15
	#define INTRO_BASETILE_TABLE_OFFSET 16584
	#define BEASTIE1_FRAMES 0x80
	#define BEASTIE2_FRAMES 0x40
	#define BEASTIE_FRAME_TABLE_OFFSET 0x7380
	#define BEASTIE1_FRAMES_OFFSET 0
	#define BEASTIE2_FRAMES_OFFSET 0x78
	#define INTRO_MAP_HEIGHT 5
	#define INTRO_MAP_WIDTH 19

	extractData(f1, "intro_sig.dat", INTRO_FIXUPDATA_OFFSET, 533);
	extractData(f1, "intro_map.dat", INTRO_MAP_OFFSET,
		INTRO_MAP_WIDTH * INTRO_MAP_HEIGHT);
	extractData(f1, "intro_script.dat", INTRO_SCRIPT_TABLE_OFFSET, INTRO_SCRIPT_TABLE_SIZE);
	extractData(f1, "intro_base_tile.dat", INTRO_BASETILE_TABLE_OFFSET, INTRO_BASETILE_TABLE_SIZE);
	extractData(f1, "intro_beastie1.dat", BEASTIE_FRAME_TABLE_OFFSET + BEASTIE1_FRAMES_OFFSET,
		BEASTIE1_FRAMES);
	extractData(f1, "intro_beastie2.dat", BEASTIE_FRAME_TABLE_OFFSET + BEASTIE2_FRAMES_OFFSET,
		BEASTIE2_FRAMES);
}
