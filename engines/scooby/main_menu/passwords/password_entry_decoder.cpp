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

#include "password_entry_decoder.h"

#include "common/scummsys.h"

namespace Scooby {
namespace {
const int kPasswordStateByteCount = 29;

// Ghidra: ReadNextPasswordBit (0x00009392). The complete contiguous body is
// 0x00009392-0x000093A1. The original consumes undefined adjacent scratch only while building
// the ignored byte at 0xFF003D; returning zero for that six-bit tail avoids preserving
// unobservable RAM state.
bool ReadNextPasswordBit(Span<const uint8> symbols, int &bitOffset) {
	// Ghidra 0x00009392-0x0000939B: reload each five-bit symbol and align its first bit to the high side.
	int symbolIndex = bitOffset / 5;
	int bitIndex = 4 - bitOffset % 5;

	// Ghidra 0x0000939C-0x000093A1: advance the cursor and return the shifted bit through managed state.
	bitOffset++;
	return symbolIndex < static_cast<int>(symbols.size()) &&
		   (symbols[static_cast<std::size_t>(symbolIndex)] & (1 << bitIndex)) != 0;
}
} // namespace

void DecodePasswordSymbolXorChain(Span<uint8> packedPassword) {
	// Ghidra 0x0000876C-0x00008783: recover the five-bit room from checksum XOR original prefix.
	uint8 previousEncodedByte = packedPassword[0];
	packedPassword[0] = static_cast<uint8>((packedPassword[1] ^ previousEncodedByte) & 0x1F);

	// Ghidra 0x00008784-0x0000879D: invert all 29 chained state bytes using encoded predecessors.
	for (int index = 0; index < kPasswordStateByteCount; index++) {
		std::size_t packedIndex = static_cast<std::size_t>(index + 2);
		uint8 encodedByte = packedPassword[packedIndex];
		packedPassword[packedIndex] = static_cast<uint8>(encodedByte ^ previousEncodedByte);
		previousEncodedByte = encodedByte;
	}
}

uint8 ReadPasswordByteFromSymbols(Span<const uint8> symbols, int &bitOffset) {
	// Ghidra 0x00009382-0x00009385: clear D2 and arm eight DBF iterations.
	uint8 result = 0;

	// Ghidra 0x00009386-0x00009391: read and rotate eight extend bits into D2, most-significant first.
	for (int bit = 0; bit < 8; bit++) {
		result = static_cast<uint8>((result << 1) | (ReadNextPasswordBit(symbols, bitOffset)
														 ? 1
														 : 0));
	}

	return result;
}
} // namespace Scooby