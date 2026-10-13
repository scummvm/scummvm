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

void loadTiles(const char *filename, Graphics::ManagedSurface *tiles, int count, int tileSize) {
	const int BYTES_PER_ROW = tileSize / 2;		// 4 planes, so 2 pixels per byte
	const int BYTES_PER_PLANE_ROW = BYTES_PER_ROW / 4;	// bytes per row, within a single plane
	const int BYTES_PER_TILE = BYTES_PER_ROW * tileSize;

	Common::File f;
	if (!f.open(filename))
		error("Could not open %s", filename);

	byte *data = new byte[BYTES_PER_TILE];

	for (int tileNum = 0; tileNum < count; ++tileNum) {
		if (f.read(data, BYTES_PER_TILE) != (uint32)BYTES_PER_TILE)
			error("Unexpected end of %s", filename);

		Graphics::ManagedSurface &tile = tiles[tileNum];
		tile.create(tileSize, tileSize, Graphics::PixelFormat::createFormatCLUT8());

		for (int y = 0; y < tileSize; ++y) {
			const byte *row = &data[y * BYTES_PER_ROW];
			byte *destRow = (byte *)tile.getBasePtr(0, y);

			for (int x = 0; x < tileSize; ++x) {
				int byteIdx = x / 8;
				int bitMask = 0x80 >> (x % 8);

				byte colorIndex = 0;
				for (int plane = 0; plane < 4; ++plane) {
					if (row[plane * BYTES_PER_PLANE_ROW + byteIdx] & bitMask)
						colorIndex |= (1 << plane);
				}

				destRow[x] = colorIndex;
			}
		}
	}

	delete[] data;
}

constexpr int SHIP_TILE_HEIGHT = SPACE_SHIP_TILE_HEIGHT;
constexpr int SHIP_TILE_SHIFT_VARIANTS = 8;
constexpr byte SHIP_TILE_MASK_COLOR = 15;

void loadShipTiles(const char *filename, Graphics::ManagedSurface *tiles, int count, int width) {
	const int rowBytes = width / 8;
	const int tileBytes = rowBytes * SHIP_TILE_HEIGHT;
	const int slotBytes = tileBytes * SHIP_TILE_SHIFT_VARIANTS;

	Common::File f;
	if (!f.open(filename))
		error("Could not open %s", filename);

	byte *data = new byte[tileBytes];

	for (int tileNum = 0; tileNum < count; ++tileNum) {
		f.seek(tileNum * slotBytes);
		if (f.read(data, tileBytes) != (uint32)tileBytes)
			error("Unexpected end of %s", filename);

		Graphics::ManagedSurface &tile = tiles[tileNum];
		tile.create(width, SHIP_TILE_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
		tile.clear(0);

		for (int y = 0; y < SHIP_TILE_HEIGHT; ++y) {
			const byte *row = &data[y * rowBytes];
			byte *destRow = (byte *)tile.getBasePtr(0, y);

			for (int x = 0; x < width; ++x) {
				int byteIdx = x / 8;
				int bitMask = 0x80 >> (x % 8);
				if (row[byteIdx] & bitMask)
					destRow[x] = SHIP_TILE_MASK_COLOR;
			}
		}
	}

	delete[] data;
}

} // namespace Data
} // namespace Ultima1
} // namespace Ultima
