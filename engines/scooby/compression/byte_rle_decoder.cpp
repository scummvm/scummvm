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

#include "byte_rle_decoder.h"

#include "scooby/common/span.h"

namespace Scooby {
Common::Array<uint8> decompressByteRle(const ScoobyDooRom &rom, int sourceOffset) {
	Common::Array<uint8> output(rom.readUInt16(sourceOffset + static_cast<int>(sizeof(uint16))));
	sourceOffset += static_cast<int>(sizeof(uint32));
	std::size_t outputOffset = 0;
	while (outputOffset < output.size()) {
		uint8 control = rom.readByte(sourceOffset++);
		if (static_cast<int8>(control) >= 0) {
			int literalCount = static_cast<int>(control) + 1;
			Span<const uint8> literals = rom.readBytes(sourceOffset, literalCount);
			for (int i = 0; i < literalCount; ++i) {
				output[outputOffset + static_cast<std::size_t>(i)] = literals[static_cast<std::size_t>(i)];
			}
			sourceOffset += literalCount;
			outputOffset += static_cast<std::size_t>(literalCount);
			continue;
		}

		int repeatCount = 0x101 - static_cast<int>(control);
		uint8 repeatedByte = rom.readByte(sourceOffset++);
		for (int i = 0; i < repeatCount; ++i) {
			output[outputOffset + static_cast<std::size_t>(i)] = repeatedByte;
		}
		outputOffset += static_cast<std::size_t>(repeatCount);
	}

	return output;
}
} // namespace Scooby