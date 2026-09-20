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

#include "genesis_asset_decoder.h"

namespace Scooby {
namespace {
const int kDecodedPixelsPerTile = 64;
const int kPackedBytesPerTile = 32;
} // namespace

Common::Array<uint8> decodeTiles(Span<const uint8> packedTiles) {
	std::size_t tileCount = packedTiles.size() / static_cast<std::size_t>(kPackedBytesPerTile);
	Common::Array<uint8> pixels(tileCount * static_cast<std::size_t>(kDecodedPixelsPerTile));
	for (std::size_t tile = 0; tile < tileCount; ++tile) {
		for (int packedIndex = 0; packedIndex < kPackedBytesPerTile; ++packedIndex) {
			uint8 packedPixels =
				packedTiles[tile * static_cast<std::size_t>(kPackedBytesPerTile) + static_cast<std::size_t>(
																					   packedIndex)];
			std::size_t pixelIndex =
				tile * static_cast<std::size_t>(kDecodedPixelsPerTile) + static_cast<std::size_t>(
																			 packedIndex) *
																			 2;
			pixels[pixelIndex] = static_cast<uint8>(packedPixels >> 4);
			pixels[pixelIndex + 1] = static_cast<uint8>(packedPixels & 0x0F);
		}
	}

	return pixels;
}

TileCell decodeTileCell(Span<const uint8> packedCells, int byteOffset) {
	uint16 entry = static_cast<uint16>(
		(uint16(packedCells[static_cast<std::size_t>(byteOffset)]) << 8) |
		uint16(packedCells[static_cast<std::size_t>(byteOffset) + 1]));
	return decodeTileCell(entry);
}

TileCell decodeTileCell(uint16 entry) {
	return TileCell(static_cast<int16>(entry & 0x07FF), static_cast<uint8>((entry >> 13) & 3),
					(entry & 0x0800) != 0, (entry & 0x1000) != 0, (entry & 0x8000) != 0);
}

uint16 encodeTileCell(const TileCell &cell) {
	uint16 value = static_cast<uint16>(
		(static_cast<uint16>(cell.tileIndex)) | (static_cast<uint16>(cell.paletteIndex) << 13) |
		(cell.flipHorizontally ? 0x0800 : 0) | (cell.flipVertically ? 0x1000 : 0) |
		(cell.highPriority ? 0x8000 : 0));
	return value;
}

Common::Array<PaletteColor> readPalette(const ScoobyDooRom &rom, int romOffset, int colorCount) {
	Common::Array<PaletteColor> palette(static_cast<std::size_t>(colorCount));
	for (std::size_t index = 0; index < palette.size(); ++index) {
		uint16 packedColor = rom.readUInt16(
			romOffset + static_cast<int>(index) * static_cast<int>(sizeof(uint16)));
		palette[index] = PaletteColor(static_cast<uint8>((packedColor >> 1) & 7),
									  static_cast<uint8>((packedColor >> 5) & 7),
									  static_cast<uint8>((packedColor >> 9) & 7));
	}

	return palette;
}
} // namespace Scooby