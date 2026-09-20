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

#include "binary_mask_tile_generator.h"

namespace Scooby {
namespace {
const int kSourceOffset = 0x30F12;
const int kSourceWordCount = 0x5F0;
} // namespace

Common::Array<uint8> generate(const ScoobyDooRom &rom, uint8 nonzeroPaletteIndex,
							  uint8 zeroPaletteIndex) {
	Span<const uint8> source =
		rom.readBytes(kSourceOffset, kSourceWordCount * static_cast<int>(sizeof(uint16)));
	Common::Array<uint8> output = source.toArray();
	for (std::size_t index = 0; index < output.size(); ++index) {
		uint8 packedPixels = output[index];
		uint8 highPaletteIndex = (packedPixels & 0xF0) == 0 ? zeroPaletteIndex : nonzeroPaletteIndex;
		uint8 lowPaletteIndex = (packedPixels & 0x0F) == 0 ? zeroPaletteIndex : nonzeroPaletteIndex;
		output[index] = static_cast<uint8>((highPaletteIndex << 4) | lowPaletteIndex);
	}

	return output;
}
} // namespace Scooby