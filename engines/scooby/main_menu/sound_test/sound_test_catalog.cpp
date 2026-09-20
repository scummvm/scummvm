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

#include "sound_test_catalog.h"

namespace Scooby {
const Common::Array<uint8> SoundTestCatalog::kAudioCommands{
	0x00, 0x01, 0x03, 0x04, 0x05, 0x06, 0x07, 0x09, 0x0B, 0x0D, 0x0F, 0x13, 0x14, 0x15, 0x19, 0x1B,
	0x1C, 0x1D, 0x1E, 0x1F, 0x22, 0x24, 0x25, 0x27, 0x28, 0x2A, 0x2B, 0x2C, 0x2E, 0x2F, 0x31, 0x33,
	0x34, 0x37, 0x38, 0x3A, 0x3C, 0x3D, 0x3E, 0x41, 0x43, 0x44, 0x45, 0x46, 0x48, 0x4A, 0x4B, 0x4D,
	0x4E, 0x4F, 0x51, 0x52, 0x53, 0x55, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60,
	0x61, 0x62, 0x63, 0x64, 0x66, 0x67, 0x68, 0x69, 0x6B, 0x6C, 0x6F, 0x71, 0x72, 0x73, 0x74, 0x75,
	0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7F};

SoundTestCatalog::SoundTestCatalog(const ScoobyDooRom &rom) {
	_names.resize(static_cast<std::size_t>(kNameCount));
	int offset = kNameTableOffset;
	for (int index = 0; index < static_cast<int>(_names.size()); index++) {
		int endOffset = offset;
		while (rom.readByte(endOffset) != 0) {
			endOffset++;
		}

		Span<const uint8> nameBytes = rom.readBytes(offset, endOffset - offset);
		_names[static_cast<std::size_t>(index)] = Common::String(
			reinterpret_cast<const char *>(nameBytes.data()), static_cast<uint32>(nameBytes.size()));
		offset = endOffset + 1;
	}
}
} // namespace Scooby