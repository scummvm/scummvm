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

#include "genesis_sprite_table_decoder.h"

namespace Scooby {
namespace {
const int kEntryByteCount = 8;
const int kVisibleCoordinateBias = 128;

inline uint16 ReadBigEndian16(Span<const uint8> entry, std::size_t offset) {
	return static_cast<uint16>((uint16(entry[offset]) << 8) | uint16(
																  entry[offset + 1]));
}
} // namespace

Common::Array<SpriteInstance> decode(Span<const uint8> packedEntries) {
	Common::Array<SpriteInstance> sprites;
	int entryIndex = 0;
	while (true) {
		Span<const uint8> entry =
			packedEntries.slice(
				static_cast<std::size_t>(entryIndex) * static_cast<std::size_t>(kEntryByteCount),
				static_cast<std::size_t>(kEntryByteCount));
		uint16 packedY = ReadBigEndian16(entry, 0);
		uint16 sizeAndLink = ReadBigEndian16(entry, 2);
		uint16 attributes = ReadBigEndian16(entry, 4);
		uint16 packedX = ReadBigEndian16(entry, 6);

		sprites.push_back(SpriteInstance(
			(static_cast<int32>(packedX & 0x01FF)) - kVisibleCoordinateBias,
			(static_cast<int32>(packedY & 0x03FF)) - kVisibleCoordinateBias,
			((sizeAndLink >> 10) & 3) + 1, ((sizeAndLink >> 8) & 3) + 1, attributes & 0x07FF,
			static_cast<uint8>((attributes >> 13) & 3), (attributes & 0x0800) != 0,
			(attributes & 0x1000) != 0, (attributes & 0x8000) != 0));

		entryIndex = sizeAndLink & 0x007F;
		if (entryIndex == 0) {
			return sprites;
		}
	}
}
} // namespace Scooby