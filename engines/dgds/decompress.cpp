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

#include "common/debug.h"
#include "common/stream.h"
#include "common/util.h"
#include "common/textconsole.h"

#include "dgds/decompress.h"

namespace Dgds {

uint32 RleDecompressor::decompress(byte *dest, uint32 sz, Common::SeekableReadStream &input) {
	uint32 left = sz;

	uint32 lenR = 0, lenW = 0;
	while (left > 0 && !input.eos()) {
		lenR = input.readByte();

		if (lenR == 128) {
			lenW = 0;
		} else if (lenR <= 127) {
			lenW = MIN(lenR, left);
			for (uint32 j = 0; j < lenW; j++) {
				*dest++ = input.readByte();
			}
			for (; lenR > lenW; lenR--) {
				input.readByte();
			}
		} else {
			lenW = MIN(lenR & 0x7F, left);
			byte val = input.readByte();
			memset(dest, val, lenW);
			dest += lenW;
		}

		left -= lenW;
	}

	return sz - left;
}

LzwDecompressor::LzwDecompressor() {
	reset();
}

void LzwDecompressor::reset() {
	memset(&_codeTable, 0, sizeof(_codeTable));

	for (uint32 code = 0; code < 256; code++) {
		_codeTable[code].len = 1;
		_codeTable[code].str[0] = code;
	}

	_tableSize = 0x101;
	_tableMax = 0x200;
	_tableFull = false;

	_codeSize = 9;
	_codeLen = 0;

	_cacheBits = 0;
	_bitsData = 0;
	_bitsSize = 0;
}

uint32 LzwDecompressor::decompress(byte *dest, uint32 sz, Common::SeekableReadStream &input) {
	_bitsData = 0;
	_bitsSize = 0;

	reset();

	uint32 idx;
	idx = 0;
	_cacheBits = 0;

	do {
		uint32 code;

		code = getCode(_codeSize, input);
		if (code == 0xFFFFFFFF) break;

		_cacheBits += _codeSize;
		if (_cacheBits >= _codeSize * 8) {
			_cacheBits -= _codeSize * 8;
		}

		if (code == 0x100) {
			if (_cacheBits > 0)
				getCode(_codeSize * 8 - _cacheBits, input);
			reset();
		} else {
			if (code >= _tableSize && !_tableFull) {
				_codeCur[_codeLen++] = _codeCur[0];

				for (uint32 i = 0; i < _codeLen; i++) {
					if (idx >= sz)
						break;
					dest[idx++] = _codeCur[i];
				}
			} else {
				for (uint32 i = 0; i < _codeTable[code].len; i++) {
					if (idx >= sz)
						break;
					dest[idx++] = _codeTable[code].str[i];
				}

				_codeCur[_codeLen++] = _codeTable[code].str[0];
			}

			if (_codeLen >= 2) {
				if (!_tableFull) {
					uint32 i;

					if (_tableSize == _tableMax && _codeSize == 12) {
						_tableFull = true;
						i = _tableSize;
					} else {
						i = _tableSize++;
						_cacheBits = 0;
					}

					if (_tableSize == _tableMax && _codeSize < 12) {
						_codeSize++;
						_tableMax <<= 1;
					}

					for (uint32 j = 0; j < _codeLen; j++) {
						_codeTable[i].str[j] = _codeCur[j];
						_codeTable[i].len++;
					}
				}

				for (uint32 i = 0; i < _codeTable[code].len; i++)
					_codeCur[i] = _codeTable[code].str[i];

				_codeLen = _codeTable[code].len;
			}
		}
	} while (idx < sz);

	return idx;
}

uint32 LzwDecompressor::getCode(uint32 totalBits, Common::SeekableReadStream &input) {
	const byte bitMasks[9] = {
		0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF
	};

	uint32 numBits = totalBits;
	uint32 useBits = numBits;
	uint32 result = 0;

	while (numBits > 0) {
		if (input.pos() >= input.size())
			return 0xFFFFFFFF;

		if (_bitsSize == 0) {
			_bitsSize = 8;
			_bitsData = input.readByte();
		}

		useBits = numBits;
		if (useBits > 8)
			useBits = 8;
		if (useBits > _bitsSize)
			useBits = _bitsSize;

		result |= (_bitsData & bitMasks[useBits]) << (totalBits - numBits);

		numBits -= useBits;
		_bitsSize -= useBits;
		_bitsData >>= useBits;
	}

	return result;
}

Decompressor::Decompressor() {
}

Decompressor::~Decompressor() {
}

byte *Decompressor::decompress(Common::SeekableReadStream *input, int size, uint32 &uncompressedSize) {
	byte compression = input->readByte();
	uncompressedSize = input->readUint32LE();
	byte *data = new byte[uncompressedSize];

	uint32 result = 0;
	uint32 expectedSize = size;
	switch (compression) {
	case 0x00:
		result = input->read(data, size);
		break;
	case 0x01:
		result = _rleDecompressor.decompress(data, uncompressedSize, *input);
		expectedSize = uncompressedSize;
		break;
	case 0x02:
		result = _lzwDecompressor.decompress(data, uncompressedSize, *input);
		expectedSize = uncompressedSize;
		break;
	default:
		input->skip(size);
		result = size;
		warning("Unknown chunk compression: 0x%x", compression);
		break;
	}

	if (result != expectedSize) {
		warning("Loading resource with compression type %d - expected %d bytes, got %d",
			compression, expectedSize, result);
	}

	return data;
}

} // End of namespace Dgds

