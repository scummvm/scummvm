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

#include "password_display_builder.h"

#include "common/scummsys.h"
#include "common/algorithm.h"

#include "common/array.h"

#include "password_alphabet.h"

namespace Scooby {
namespace {
const int kBitsPerSymbol = 5;
const int kCharactersPerRow = 30;
const int kPasswordStateByteCount = 29;
const int kSymbolsPerGroup = 5;
const int kSymbolsPerRow = 25;

// Ghidra: TransferPasswordBitsToAccumulator (0x00008710). The complete body is
// 0x00008710-0x0000871D. Native ROXL.W passes each source MSB through X into the
// accumulator. Both callers transfer at most five bits into a freshly zeroed accumulator; the X bit
// inserted at the source LSB cannot become observable before that 16-bit source word is replaced.
void TransferPasswordBitsToAccumulator(uint16 &sourceBits, uint16 &symbolAccumulator,
									   int bitCount) {
	// Ghidra 0x00008710-0x00008713: zero bits leave both words unchanged.
	if (bitCount == 0) {
		return;
	}

	// Ghidra 0x00008714-0x0000871B: rotate one source MSB through X into the accumulator, then repeat
	// until the caller-owned count reaches zero. Logical shifts retain every observable bit under the
	// proved zero-through-five count and 16-bit source-replacement lifecycle.
	do {
		bool sourceHighBit = (sourceBits & 0x8000) != 0;
		sourceBits = static_cast<uint16>(sourceBits << 1);
		symbolAccumulator = static_cast<uint16>((symbolAccumulator << 1) | (sourceHighBit ? 1 : 0));
		bitCount--;
	} while (bitCount != 0);

	// Ghidra 0x0000871C-0x0000871D: return the shifted source and completed partial symbol.
}

// Ghidra: PackNextPasswordSymbol (0x0000870E). Its complete body is
// 0x0000870E-0x0000870F and falls through into TransferPasswordBitsToAccumulator at
// 0x00008710 without an intervening return.
void PackNextPasswordSymbol(uint16 &sourceBits, uint16 &symbolAccumulator) {
	// Ghidra 0x0000870E-0x0000870F: supply five bits, then preserve the native fallthrough as a direct
	// managed call to the separately recovered transfer owner.
	TransferPasswordBitsToAccumulator(sourceBits, symbolAccumulator, kBitsPerSymbol);
}

// Ghidra: BuildPasswordChecksumAndPrefixXor (0x0000873E). The complete body is
// 0x0000873E-0x0000876B. It mutates original scratch range 0xFF001E-0xFF003C in place:
// byte zero becomes the low five bits of checksum XOR room, byte one remains the checksum, and the
// remaining 29 bytes become the cumulative prefix-XOR chain.
void BuildPasswordChecksumAndPrefixXor(Span<uint8> packedPassword) {
	// Ghidra 0x0000873E-0x00008755: derive the low five-bit prefix from checksum XOR room and replace
	// the room scratch byte. Upper register bytes touched by the word mask have no stored consumer.
	uint8 prefixXor = static_cast<uint8>((packedPassword[1] ^ packedPassword[0]) &
										 0x1F);
	packedPassword[0] = prefixXor;

	// Ghidra 0x00008756-0x0000875F: select the 29-byte state range and its inclusive DBF bound.
	// Ghidra 0x00008760-0x00008769: XOR every source byte with the preceding encoded value in place.
	for (std::size_t index = 2; index < packedPassword.size(); index++) {
		prefixXor = static_cast<uint8>(prefixXor ^ packedPassword[index]);
		packedPassword[index] = prefixXor;
	}

	// Ghidra 0x0000876A-0x0000876B: return with the checksum byte unchanged.
}
} // namespace

Common::Array<Common::String> Build(int16 roomId, Span<const uint8> passwordStateBytes) {
	Common::Array<uint8> packedBytes(kPasswordStateByteCount + 2);
	uint8 roomByte = static_cast<uint8>(roomId);
	uint8 rollingByte = static_cast<uint8>((roomByte << 2) | (roomByte >> 7));
	bool carry = (roomByte & 0x40) != 0;
	for (int index = 0; index < kPasswordStateByteCount; index++) {
		uint8 previousRollingByte = rollingByte;
		rollingByte = static_cast<uint8>(((rollingByte << 1) | (carry ? 1 : 0)) ^
										 passwordStateBytes[static_cast<std::size_t>(index)]);
		carry = (previousRollingByte & 0x80) != 0;
	}

	packedBytes[0] = roomByte;
	packedBytes[1] = rollingByte;
	Common::copy(passwordStateBytes.begin(), passwordStateBytes.begin() + kPasswordStateByteCount,
			  packedBytes.begin() + 2);
	BuildPasswordChecksumAndPrefixXor(MakeSpan(packedBytes));

	int packedSourceOffset = 0;
	uint16 sourceBits = 0;
	int availableBitCount = 0;
	Common::Array<Common::String> rows(kRowCount);
	for (int rowIndex = 0; rowIndex < kRowCount; rowIndex++) {
		Common::String characters(static_cast<std::size_t>(kCharactersPerRow), ' ');
		for (int symbolIndex = 0; symbolIndex < kSymbolsPerRow; symbolIndex++) {
			uint16 symbolAccumulator = 0;

			// Ghidra 0x000086B2-0x000086BE: reset the symbol accumulator and reserve five source bits.
			availableBitCount -= kBitsPerSymbol;
			if (availableBitCount >= 0) {
				// Ghidra 0x000086D4: preserve the distinct five-bit fallthrough boundary.
				PackNextPasswordSymbol(sourceBits, symbolAccumulator);
			} else {
				// Ghidra 0x000086BA-0x000086C0: transfer every bit left in the current source word.
				int bitDeficit = availableBitCount;
				availableBitCount += kBitsPerSymbol;
				TransferPasswordBitsToAccumulator(sourceBits, symbolAccumulator, availableBitCount);

				// Ghidra 0x000086C4-0x000086CE: read the next big-endian word and derive the remaining count.
				// Original scratch byte 0xFF003D supplies the two zero bits beyond the 31-byte payload.
				sourceBits = static_cast<uint16>(packedBytes[static_cast<std::size_t>(
													 packedSourceOffset)]
												 << 8);
				if (packedSourceOffset + 1 < static_cast<int>(packedBytes.size())) {
					sourceBits = static_cast<uint16>(
						sourceBits | packedBytes[static_cast<std::size_t>(packedSourceOffset + 1)]);
				}

				packedSourceOffset += static_cast<int>(sizeof(uint16));
				availableBitCount = bitDeficit + 16;

				// Ghidra 0x000086D0-0x000086D2: complete the symbol from the new source word.
				TransferPasswordBitsToAccumulator(sourceBits, symbolAccumulator, -bitDeficit);
			}

			int characterIndex = symbolIndex + symbolIndex / kSymbolsPerGroup;
			characters[static_cast<std::size_t>(characterIndex)] =
				kSymbols[symbolAccumulator];
			if (symbolIndex % kSymbolsPerGroup == kSymbolsPerGroup - 1) {
				characters[static_cast<std::size_t>(characterIndex + 1)] = ' ';
			}
		}

		rows[static_cast<std::size_t>(rowIndex)] = characters;
	}

	return rows;
}
} // namespace Scooby