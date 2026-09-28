/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/memstream.h"
#include "ultima/ultima2/data/map.h"
#include "ultima/ultima2/data/secret_archive.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

namespace {

// Matches the compass entrances in the real MAPX22 - a 4-tile gap centered
// on each edge of the map
constexpr int ENTRANCE_START = 30;
constexpr int ENTRANCE_WIDTH = 4;

// A minimal 5x7 dot-matrix font, used to spell "SCUMMVM" out of wall tiles
// across the top half of the map. '#' is a lit pixel; anything else is
// left as the map's base grass
constexpr int FONT_WIDTH = 5;
constexpr int FONT_HEIGHT = 7;
constexpr int FONT_SPACING = 1;

const char *const FONT_C[FONT_HEIGHT] = {
	" ### ", "#    ", "#    ", "#    ", "#    ", "#    ", " ### "
};
const char *const FONT_M[FONT_HEIGHT] = {
	"#   #", "## ##", "# # #", "#   #", "#   #", "#   #", "#   #"
};
const char *const FONT_S[FONT_HEIGHT] = {
	" ### ", "#    ", "#    ", " ### ", "    #", "    #", " ### "
};
const char *const FONT_U[FONT_HEIGHT] = {
	"#   #", "#   #", "#   #", "#   #", "#   #", "#   #", " ### "
};
const char *const FONT_V[FONT_HEIGHT] = {
	"#   #", "#   #", "#   #", "#   #", "#   #", " # # ", "  #  "
};

const char *const *const SCUMMVM_WORD[] = {
	FONT_S, FONT_C, FONT_U, FONT_M, FONT_M, FONT_V, FONT_M
};
constexpr int SCUMMVM_LETTER_COUNT = ARRAYSIZE(SCUMMVM_WORD);

void setTile(byte *buffer, int x, int y, TileId tile) {
	buffer[y * MAP_WIDTH + x] = (byte)(tile * 4);
}

// The wall lettering re-uses the game's own A-Z tile glyphs, which run in
// alphabetic order except for a gap at Q (that slot is the moongate glyph)
TileId letterTile(char c) {
	return (TileId)(TILE_A + (c - 'A'));
}

void writeWallText(byte *buffer, int y, int x, const char *text) {
	for (int i = 0; text[i]; ++i)
		setTile(buffer, x + i, y, letterTile(text[i]));
}

Common::SeekableReadStream *buildTownMap() {
	byte *buffer = new byte[MAP_WIDTH * MAP_HEIGHT];

	for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT; ++i)
		buffer[i] = (byte)(TILE_GRASS * 4);

	// Walls around the edge of the town, with a gap at each compass entrance
	for (int x = 0; x < MAP_WIDTH; ++x) {
		if (x < ENTRANCE_START || x >= ENTRANCE_START + ENTRANCE_WIDTH) {
			setTile(buffer, x, 0, TILE_WALL);
			setTile(buffer, x, MAP_HEIGHT - 1, TILE_WALL);
		}
	}
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		if (y < ENTRANCE_START || y >= ENTRANCE_START + ENTRANCE_WIDTH) {
			setTile(buffer, 0, y, TILE_WALL);
			setTile(buffer, MAP_WIDTH - 1, y, TILE_WALL);
		}
	}

	// A crossroads connecting every compass entrance clear through the
	// town, like MAPX22's own main streets
	for (int y = ENTRANCE_START; y < ENTRANCE_START + ENTRANCE_WIDTH; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x)
			setTile(buffer, x, y, TILE_ROAD);
	}
	for (int x = ENTRANCE_START; x < ENTRANCE_START + ENTRANCE_WIDTH; ++x) {
		for (int y = 0; y < MAP_HEIGHT; ++y)
			setTile(buffer, x, y, TILE_ROAD);
	}

	// South wall lettering, mirroring MAPX22's own "TOWNE"/"LINDA" either
	// side of its south entrance
	writeWallText(buffer, MAP_HEIGHT - 1, 24, "TOWNE");
	writeWallText(buffer, MAP_HEIGHT - 1, 35, "SCUMMVM");

	// "SCUMMVM" spelled out in large wall-tile letters across the top half,
	// drawn over the north-south road so the lettering wins wherever the
	// two overlap. Letter spacing stays constant throughout; the word is
	// simply positioned so the gap between its two "M"s falls under the
	// north entrance, even where that means the letters' own wall tiles
	// spill out over the road on either side of that gap
	constexpr int MM_GAP_COLUMN = ENTRANCE_START + ENTRANCE_WIDTH / 2 - 1;
	constexpr int WORD_START_X = MM_GAP_COLUMN - (3 * (FONT_WIDTH + FONT_SPACING) + FONT_WIDTH);
	constexpr int WORD_START_Y = 3;

	for (int letter = 0; letter < SCUMMVM_LETTER_COUNT; ++letter) {
		int lx = WORD_START_X + letter * (FONT_WIDTH + FONT_SPACING);
		for (int row = 0; row < FONT_HEIGHT; ++row) {
			const char *bits = SCUMMVM_WORD[letter][row];
			for (int col = 0; col < FONT_WIDTH; ++col) {
				if (bits[col] != ' ')
					setTile(buffer, lx + col, WORD_START_Y + row, TILE_WALL);
			}
		}
	}

	return new Common::MemoryReadStream(buffer, MAP_WIDTH * MAP_HEIGHT, DisposeAfterUse::YES);
}

// MapMonsters::load reads eight parallel MAP_MONSTER_COUNT-byte arrays; an
// all-zero buffer means every slot's type is 0, i.e. inactive - no NPCs
constexpr int MONSTER_DATA_SIZE = MAP_MONSTER_COUNT * 8;

Common::SeekableReadStream *buildDummyMonsterData() {
	byte *buffer = new byte[MONSTER_DATA_SIZE]();
	return new Common::MemoryReadStream(buffer, MONSTER_DATA_SIZE, DisposeAfterUse::YES);
}

// Map::loadTalk reads this many bytes and splits them on zero bytes into
// dialogue lines; an all-zero buffer decodes to nothing but empty lines
constexpr int TALK_DATA_SIZE = 256;

Common::SeekableReadStream *buildDummyTalkData() {
	byte *buffer = new byte[TALK_DATA_SIZE]();
	return new Common::MemoryReadStream(buffer, TALK_DATA_SIZE, DisposeAfterUse::YES);
}

} // namespace

bool SecretMapArchive::hasFile(const Common::Path &path) const {
	Common::String name = path.baseName();
	return name.size() == 6 && name.hasSuffix("9") && (
		name.hasPrefixIgnoreCase("mapx") ||
		name.hasPrefixIgnoreCase("monx") ||
		name.hasPrefixIgnoreCase("tlkx")
	);
}

const Common::ArchiveMemberPtr SecretMapArchive::getMember(const Common::Path &path) const {
	if (!hasFile(path))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *SecretMapArchive::createReadStreamForMember(const Common::Path &path) const {
	if (!hasFile(path))
		return nullptr;

	Common::String name = path.baseName();

	if (name.hasPrefixIgnoreCase("mapx"))
		return buildTownMap();
	if (name.hasPrefixIgnoreCase("monx"))
		return buildDummyMonsterData();

	return buildDummyTalkData();
}

} // namespace Data
} // namespace Ultima2
} // namespace Ultima
