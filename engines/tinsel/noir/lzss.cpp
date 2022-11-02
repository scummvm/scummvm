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
 * Prototypes of actor functions
 */

#include "common/textconsole.h"
#include "tinsel/noir/lzss.h"

namespace Tinsel {

static byte HIGH_BITS(byte byteValue, int numBits) {
	unsigned int mask = ((1 << numBits) - 1) << (8 - numBits);
	return (byteValue & mask) >> (8 - numBits);
}

static byte LOW_BITS(byte byteValue, int numBits) {
	unsigned int mask = ((1 << numBits) - 1);
	return byteValue & mask;
}

int decompressLZSS(Common::SeekableReadStream &input, byte *output) {
	static const int kDictionarySize = 4096;
	byte dictionary[kDictionarySize] = {};
	int dictionaryOffset = 1;
	int outputOffset = 0;

	byte *data = new byte[input.size()];
	input.read(data, input.size());
	unsigned int offset = 0;

	int bitShift = 0;
	int bytesWritten = 0;

	while (true) {
		byte value = data[offset];
		byte bitMask = 0x80 >> bitShift++;
		// First bit:
		// 0 -> Copy data from dictionary
		// 1 -> Copy raw byte from input
		bool useRawByte = value & bitMask;
		if (bitShift == 8) {
			bitShift = 0;
			offset++;
		}
		if (!useRawByte) {
			unsigned int bitsFromFirst = 8 - bitShift;
			unsigned int bitsFromLast = 16 - 8 - bitsFromFirst;

			// The dictionary lookup is 16 bit:
			// 12 bits for the offset
			// 4 bits for the run-length

			// Combined with the first bit this makes for 17 bits,
			// So we will be reading from three bytes, except when
			// the first bit was read from the end of a byte, then
			// bitShift will be 0, and bitsFromLast will be 8.

			// We make the assumption that we can dereference the third byte
			// even if we aren't using it. We will check "offset" after decompression
			// to verify this assumption.
			unsigned int byte1 = LOW_BITS(data[offset], bitsFromFirst);
			unsigned int byte2 = data[offset + 1];
			unsigned int byte3 = HIGH_BITS(data[offset + 2], bitsFromLast);

			unsigned int lookup = (byte1 << (8 + bitsFromLast)) | (byte2 << bitsFromLast) | byte3;

			int lookupOffset = (lookup >> 4) & 0xFFF;
			if (lookupOffset == 0) {
				break;
			}
			int lookupRunLength = (lookup & 0xF) + 2;
			for (int j = 0; j < lookupRunLength; j++) {
				output[outputOffset++] = dictionary[(lookupOffset + j) % kDictionarySize];
				dictionary[dictionaryOffset++] = dictionary[(lookupOffset + j) % kDictionarySize];
				dictionaryOffset %= kDictionarySize;
			}

			offset += 2;
			bytesWritten += lookupRunLength;
		} else {
			// Raw byte, but since we spent a bit first,
			// we must reassemble it from potentially two bytes.
			unsigned int bitsFromFirst = 8 - bitShift;
			unsigned int bitsFromLast = 8 - bitsFromFirst;

			byte byteValue = LOW_BITS(data[offset], bitsFromFirst) << bitsFromLast;
			byteValue |= HIGH_BITS(data[offset + 1], bitsFromLast);

			offset++;

			output[outputOffset++] = byteValue;
			dictionary[dictionaryOffset++] = byteValue;
			dictionaryOffset %= kDictionarySize;

			bytesWritten++;
		}

	}
	delete[] data;

	if ((int32)offset > input.size()) {
		error("Read too far during decompression");
	}

	return bytesWritten;
}

}
