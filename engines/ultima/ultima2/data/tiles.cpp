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

#include "common/file.h"
#include "ultima/ultima2/data/tiles.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

// Byte offset of the first tile's data within ULTIMAII.EXE
constexpr int32 TILE_DATA_OFFSET = 0x7C40;

// Byte offset of the attack-flash sprite's data within ULTIMAII.EXE
constexpr int32 ATTACK_SPRITE_OFFSET = 0x5488;

// 2-byte header (row width in bytes, row count -- always 4 and 16) plus
// 64 bytes of packed pixel data
constexpr int TILE_RECORD_SIZE = 66;

template<typename SurfaceT>
static void decodeTileRecord(const byte record[TILE_RECORD_SIZE], SurfaceT &surf) {
	surf.create(TILE_WIDTH, TILE_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	byte *dst = (byte *)surf.getPixels();

	for (int row = 0; row < TILE_HEIGHT; ++row) {
		for (int b = 0; b < TILE_WIDTH / 4; ++b) {
			byte v = record[2 + row * 4 + b];
			dst[row * TILE_WIDTH + b * 4 + 0] = (v >> 6) & 3;
			dst[row * TILE_WIDTH + b * 4 + 1] = (v >> 4) & 3;
			dst[row * TILE_WIDTH + b * 4 + 2] = (v >> 2) & 3;
			dst[row * TILE_WIDTH + b * 4 + 3] = v & 3;
		}
	}
}

// EGATILES-format files (see docs/enhanced-patch.md §3.8): 65 headerless
// 128-byte records (the 64 tiles, same order as TileId, plus the attack
// sprite as a 65th), 4 bits/pixel, 8 bytes/row x 16 rows, high nibble
// first. Pixel values are literal EGA_PALETTE16 indices, so they're
// offset by EGA_PALETTE_BASE to land in that palette's reserved range
constexpr int EGA_RECORD_SIZE = 128;
constexpr int EGA_ATTACK_SPRITE_RECORD = 64;

template<typename SurfaceT>
static void decodeEgaTileRecord(const byte record[EGA_RECORD_SIZE], SurfaceT &surf) {
	surf.create(TILE_WIDTH, TILE_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	byte *dst = (byte *)surf.getPixels();

	for (int row = 0; row < TILE_HEIGHT; ++row) {
		for (int b = 0; b < TILE_WIDTH / 2; ++b) {
			byte v = record[row * 8 + b];
			dst[row * TILE_WIDTH + b * 2 + 0] = EGA_PALETTE_BASE + ((v >> 4) & 0xF);
			dst[row * TILE_WIDTH + b * 2 + 1] = EGA_PALETTE_BASE + (v & 0xF);
		}
	}
}

// The base EGA tileset lives at the data root; each "theme" is an
// alternate EGATILES file in its own subfolder (confirmed pure data
// swaps, no code differences - see docs/enhanced-patch.md §3.6)
static Common::String egaTilesFilename(RenderMode mode) {
	switch (mode) {
	case RENDER_EGA_V1:
		return "egatheme.10/egatiles";
	case RENDER_EGA_WILTSHIRE:
		return "egatheme.alt/egatiles";
	case RENDER_EGA_C64:
		return "egatheme.c64/egatiles";
	default:
		return "egatiles";
	}
}

static void loadEgaTiles(Graphics::Surface tiles[TILE_COUNT], RenderMode mode) {
	Common::File f;
	Common::String filename = egaTilesFilename(mode);
	if (!f.open(filename.c_str()))
		error("Could not open %s", filename.c_str());

	for (int t = 0; t < TILE_COUNT; ++t) {
		byte record[EGA_RECORD_SIZE];
		if (f.read(record, EGA_RECORD_SIZE) != (uint32)EGA_RECORD_SIZE)
			error("Could not read tile %d from %s", t, filename.c_str());

		decodeEgaTileRecord(record, tiles[t]);
	}
}

void loadTiles(Graphics::Surface tiles[TILE_COUNT], RenderMode mode) {
	if (mode != RENDER_CGA) {
		loadEgaTiles(tiles, mode);
		return;
	}

	Common::File f;
	if (!f.open("ULTIMAII.EXE"))
		error("Could not open ULTIMAII.EXE");

	f.seek(TILE_DATA_OFFSET);

	for (int t = 0; t < TILE_COUNT; ++t) {
		byte record[TILE_RECORD_SIZE];
		if (f.read(record, TILE_RECORD_SIZE) != (uint32)TILE_RECORD_SIZE)
			error("Could not read tile %d", t);

		decodeTileRecord(record, tiles[t]);
	}
}

void loadAttackSprite(Graphics::ManagedSurface &sprite, RenderMode mode) {
	Common::File f;

	if (mode != RENDER_CGA) {
		Common::String filename = egaTilesFilename(mode);
		if (!f.open(filename.c_str()))
			error("Could not open %s", filename.c_str());

		f.seek(EGA_ATTACK_SPRITE_RECORD * EGA_RECORD_SIZE);
		byte record[EGA_RECORD_SIZE];
		if (f.read(record, EGA_RECORD_SIZE) != (uint32)EGA_RECORD_SIZE)
			error("Could not read attack sprite from %s", filename.c_str());

		decodeEgaTileRecord(record, sprite);
		return;
	}

	if (!f.open("ULTIMAII.EXE"))
		error("Could not open ULTIMAII.EXE");

	f.seek(ATTACK_SPRITE_OFFSET);

	byte record[TILE_RECORD_SIZE];
	if (f.read(record, TILE_RECORD_SIZE) != (uint32)TILE_RECORD_SIZE)
		error("Could not read attack sprite");

	decodeTileRecord(record, sprite);
}

void scrollTileRows(Graphics::Surface &tile, int rows) {
	byte copy[TILE_WIDTH * TILE_HEIGHT];
	byte *pixels = (byte *)tile.getPixels();
	memcpy(copy, pixels, sizeof(copy));

	for (int y = 0; y < TILE_HEIGHT; ++y)
		memcpy(pixels + y * TILE_WIDTH, copy + ((y + rows) % TILE_HEIGHT) * TILE_WIDTH, TILE_WIDTH);
}

} // namespace Data
} // namespace Ultima2
} // namespace Ultima
