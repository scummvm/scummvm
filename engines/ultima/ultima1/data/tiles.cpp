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
#include "ultima/ultima1/data/tiles.h"

namespace Ultima {
namespace Ultima1 {
namespace Data {

constexpr int BYTES_PER_ROW = 8;	// 4 planes x 2 bytes/plane = 16 pixels
constexpr int BYTES_PER_TILE = BYTES_PER_ROW * TILE_HEIGHT;

void loadTiles(const char *filename, Graphics::ManagedSurface *tiles, int count) {
	Common::File f;
	if (!f.open("EgaTiles.Bin"))
		error("Could not open EgaTiles.Bin");

	byte data[BYTES_PER_TILE];

	for (int tileNum = 0; tileNum < count; ++tileNum) {
		if (f.read(data, BYTES_PER_TILE) != (uint32)BYTES_PER_TILE)
			error("Unexpected end of EgaTiles.Bin");

		Graphics::ManagedSurface &tile = tiles[tileNum];
		tile.create(TILE_WIDTH, TILE_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());

		for (int y = 0; y < TILE_HEIGHT; ++y) {
			const byte *row = &data[y * BYTES_PER_ROW];
			byte *destRow = (byte *)tile.getBasePtr(0, y);

			for (int x = 0; x < TILE_WIDTH; ++x) {
				int byteIdx = x / 8;
				int bitMask = 0x80 >> (x % 8);

				byte colorIndex = 0;
				for (int plane = 0; plane < 4; ++plane) {
					if (row[plane * 2 + byteIdx] & bitMask)
						colorIndex |= (1 << plane);
				}

				destRow[x] = colorIndex;
			}
		}
	}
}

} // namespace Data
} // namespace Ultima1
} // namespace Ultima
