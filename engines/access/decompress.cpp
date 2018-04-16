/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/debug.h"
#include "common/endian.h"
#include "common/util.h"

#include "access/decompress.h"

namespace Access {

void LzwDecompressor::decompress(byte *source, byte *dest) {

	_source = source;

	byte litByte = 0;
	uint16 oldCode = 0;
	uint16 copyLength, maxCodeValue, code, nextCode, lastCode;

	byte *copyBuf = new byte[8192];

	struct { uint16 code; byte value; } codeTable[8192];
	memset(codeTable, 0, sizeof(codeTable));

	_codeLength = 9;
	nextCode = 258;
	maxCodeValue = 512;

	copyLength = 0;
	_sourceBitsLeft = 8;

	while (1) {

		code = getCode();

		if (code == 257)
			break;

		if (code == 256) {
			_codeLength = 9;
			nextCode = 258;
			maxCodeValue = 512;
			lastCode = getCode();
			oldCode = lastCode;
			litByte = lastCode;
			*dest++ = litByte;
		} else {
			lastCode = code;
			if (code >= nextCode) {
				lastCode = oldCode;
				copyBuf[copyLength++] = litByte;
			}
			while (lastCode > 255) {
				copyBuf[copyLength++] = codeTable[lastCode].value;
				lastCode = codeTable[lastCode].code;
			}
			litByte = lastCode;
			copyBuf[copyLength++] = lastCode;
			while (copyLength > 0)
				*dest++ = copyBuf[--copyLength];
			codeTable[nextCode].value = lastCode;
			codeTable[nextCode].code = oldCode;
			nextCode++;
			oldCode = code;
			if (nextCode >= maxCodeValue && _codeLength <= 12) {
				_codeLength++;
				maxCodeValue <<= 1;
			}
		}

	}

	delete[] copyBuf;

}

uint16 LzwDecompressor::getCode() {
	const byte bitMasks[9] = {
		0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0x0FF
	};

	byte   resultBitsLeft = _codeLength;
	byte   resultBitsPos = 0;
	uint16 result = 0;
	byte   currentByte = *_source;
	byte   currentBits = 0;

	// Get bits of current byte
	while (resultBitsLeft) {
		if (resultBitsLeft < _sourceBitsLeft) {
			// we need less than we have left
			currentBits = (currentByte >> (8 - _sourceBitsLeft)) & bitMasks[resultBitsLeft];
			result |= (currentBits << resultBitsPos);
			_sourceBitsLeft -= resultBitsLeft;
			resultBitsLeft = 0;

		} else {
			// we need as much as we have left or more
			resultBitsLeft -= _sourceBitsLeft;
			currentBits = currentByte >> (8 - _sourceBitsLeft);
			result |= (currentBits << resultBitsPos);
			resultBitsPos += _sourceBitsLeft;

			// Go to next byte
			_source++;

			_sourceBitsLeft = 8;
			if (resultBitsLeft) {
				currentByte = *_source;
			}
		}
	}
	return result;
}

uint32 decompressDBE(byte *source, byte **dest) {

	uint32 destSize = READ_LE_UINT32(source + 4);
	*dest = new byte[destSize];

	debug(1, "decompressDBE() destSize = %d", destSize);

	LzwDecompressor dec;
	dec.decompress(source + 16, *dest);

	debug(1, "decompressDBE() ok");

	return destSize;
}

} // End of namespace Access
