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

void loadTiles(Graphics::Surface tiles[TILE_COUNT]) {
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

void loadAttackSprite(Graphics::ManagedSurface &sprite) {
	Common::File f;
	if (!f.open("ULTIMAII.EXE"))
		error("Could not open ULTIMAII.EXE");

	f.seek(ATTACK_SPRITE_OFFSET);

	byte record[TILE_RECORD_SIZE];
	if (f.read(record, TILE_RECORD_SIZE) != (uint32)TILE_RECORD_SIZE)
		error("Could not read attack sprite");

	decodeTileRecord(record, sprite);
}

} // namespace Data
} // namespace Ultima2
} // namespace Ultima
