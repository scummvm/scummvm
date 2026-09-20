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

#include "ring_lz_decoder.h"

#include "common/array.h"

namespace Scooby {
namespace {
const int kWindowSize = 0x1000;

class BitReader {
public:
	// Continues after the eagerly loaded first descriptor block.
	BitReader(const ScoobyDooRom &rom, int sourceOffset, uint32 initialBits,
			  int initialBitCount)
		: _rom(rom), _sourceOffset(sourceOffset), _currentBits(initialBits),
		  _bitsRemaining(initialBitCount) {
	}

	// Consumes one MSB-first field without imposing byte alignment between
	// fields.
	int read(int bitCount) {
		int value = 0;
		for (int bit = 0; bit < bitCount; ++bit) {
			if (_bitsRemaining == 0) {
				_currentBits = uint32(_rom.readByte(_sourceOffset++)) << 24;
				_bitsRemaining = 8;
			}

			value = (value << 1) | static_cast<int>(_currentBits >> 31);
			_currentBits <<= 1;
			_bitsRemaining--;
		}

		return value;
	}

private:
	const ScoobyDooRom &_rom;
	int _sourceOffset;
	uint32 _currentBits;
	int _bitsRemaining;
};

struct InitializedBlock {
	BitReader reader;
	int writeOffset;
};

// Ghidra: InitializeRingLzBlock (0x000098F8). Starts a compressed block with
// its first descriptor longword and ring cursor.
InitializedBlock InitializeRingLzBlock(const ScoobyDooRom &rom, int sourceOffset) {
	const int initialBitCount = 32;
	const int initialWriteOffset = 1;
	uint32 initialBits = rom.readUInt32(sourceOffset);

	BitReader reader(rom, sourceOffset + static_cast<int>(sizeof(uint32)), initialBits,
					 initialBitCount);
	InitializedBlock block = {reader, initialWriteOffset};
	return block;
}
} // namespace

Common::Array<uint8> decompressRingLz(const ScoobyDooRom &rom, int sourceOffset) {
	Common::Array<uint8> window(kWindowSize);
	Common::fill(window.begin(), window.end(), 0);
	Common::Array<uint8> output;

	InitializedBlock block = InitializeRingLzBlock(rom, sourceOffset);
	BitReader reader = block.reader;
	int writeOffset = block.writeOffset;

	auto writeByte = [&](uint8 value) {
		output.push_back(value);
		window[static_cast<std::size_t>(writeOffset)] = value;
		writeOffset = (writeOffset + 1) & (kWindowSize - 1);
	};

	while (true) {
		if (reader.read(1) != 0) {
			writeByte(static_cast<uint8>(reader.read(8)));
			continue;
		}

		int copyOffset = reader.read(12);
		if (copyOffset == 0) {
			return output;
		}

		int copyLength = reader.read(4) + 2;
		for (int index = 0; index < copyLength; ++index) {
			writeByte(window[static_cast<std::size_t>((copyOffset + index) & (kWindowSize - 1))]);
		}
	}
}
} // namespace Scooby