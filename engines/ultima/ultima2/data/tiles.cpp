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
#include "common/memstream.h"
#include "ultima/ultima2/data/tiles.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

// Ultima II has 64 tiles, each 16x16 pixels in CGA Linear format
const int TILE_DATA_OFFSET = 31808;
const int TILE2_DATA_OFFSET = 21640;

const Common::ArchiveMemberPtr Tiles::getMember(const Common::Path &path) const {
	if (!hasFile(path))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *Tiles::createReadStreamForMember(const Common::Path &path) const {
	if (!hasFile(path))
		return nullptr;

	Common::File f;
	if (!f.open("ultimaii.exe"))
		error("Could not open ultimaii.exe");

	// Seek to the tile data
	f.seek(TILE_DATA_OFFSET);

	Common::MemoryWriteStreamDynamic buf(DisposeAfterUse::NO);

	// Process each tile
	for (int tileNumber = 0; tileNumber < TILE_COUNT; tileNumber++) {
		if (tileNumber == (TILE_COUNT - 1))
			f.seek(TILE2_DATA_OFFSET);

		byte tileW = f.readByte() * 4;
		byte tileH = f.readByte();
		assert(tileW == TILE_WIDTH && tileH == TILE_HEIGHT);

		// Read the tile data (16 rows, 4 bytes per row)
		for (int y = 0; y < TILE_HEIGHT; y++) {
			for (int x = 0; x < TILE_WIDTH / 4; x++) {
				// Read one byte (contains 4 pixels in CGA Linear format)
				byte pixelByte = f.readByte();

				// Extract 4 pixels from this byte
				// CGA Linear: bits 7-6 = first pixel, 5-4 = second, 3-2 = third, 1-0 = fourth
				for (int offset = 0; offset < 4; offset++) {
					int colorBit = 7 - (offset * 2);  // Start at bit 7, then 5, 3, 1

					// Extract 2-bit color value
					int colorIndex = 0;
					if (pixelByte & (1 << colorBit))
						colorIndex |= 2;
					if (pixelByte & (1 << (colorBit - 1)))
						colorIndex |= 1;

					// Map the 2-bit value to palette index (0-3)
					byte paletteIndex = colorIndex;

					// Write the palette index to the buffer
					buf.writeByte(paletteIndex);
				}
			}
		}
	}

	f.close();

	return new Common::MemoryReadStream(buf.getData(), buf.size(), DisposeAfterUse::YES);
}

} // namespace Data
} // namespace Ultima2
} // namespace Ultima
