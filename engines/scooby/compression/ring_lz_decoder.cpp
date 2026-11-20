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

#include "scooby/compression/ring_lz_decoder.h"

#include "common/array.h"
#include "common/bitstream.h"

namespace Scooby {
namespace {
const int kWindowSize = 0x1000;
} // namespace

Common::Array<uint8> decompressRingLz(const ScoobyDooRom &rom, int sourceOffset) {
	Common::Array<uint8> window(kWindowSize);
	Common::fill(window.begin(), window.end(), 0);
	Common::Array<uint8> output;

	// Ghidra: InitializeRingLzBlock (0x000098F8). The original loads the first descriptor longword eagerly and
	// then refills one byte at a time; because that longword is big-endian and every field is consumed MSB
	// first, the block is a single MSB-first bit stream over the bytes starting at sourceOffset. Fields are
	// not byte-aligned. The ring cursor starts at index one.
	Span<const uint8> source = rom.readBytes(sourceOffset, rom.size() - sourceOffset);
	Common::BitStreamMemoryStream sourceStream(source.data(), static_cast<uint32>(source.size()));
	Common::BitStreamMemory8MSB reader(&sourceStream);
	int writeOffset = 1;

	auto writeByte = [&](uint8 value) {
		output.push_back(value);
		window[static_cast<size_t>(writeOffset)] = value;
		writeOffset = (writeOffset + 1) & (kWindowSize - 1);
	};

	while (true) {
		if (reader.getBit() != 0) {
			writeByte(static_cast<uint8>(reader.getBits<8>()));
			continue;
		}

		int copyOffset = static_cast<int>(reader.getBits<12>());
		if (copyOffset == 0) {
			return output;
		}

		int copyLength = static_cast<int>(reader.getBits<4>()) + 2;
		for (int index = 0; index < copyLength; ++index) {
			writeByte(window[static_cast<size_t>((copyOffset + index) & (kWindowSize - 1))]);
		}
	}
}
} // namespace Scooby
