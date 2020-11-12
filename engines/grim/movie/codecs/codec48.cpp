/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "common/endian.h"
#include "common/util.h"
#include "common/textconsole.h"

#include "engines/grim/movie/codecs/codec48.h"

namespace Grim {

Codec48Decoder::Codec48Decoder() {
	_frameSize = 640 * 480; // Yes, this is correct. Looks like the buffers are always this size

	_curBuf = 0;
	_deltaBuf[0] = new byte[_frameSize * 2];
	_deltaBuf[1] = _deltaBuf[0] + _frameSize;

	_offsetTable = new int16[255];
	_tableLastPitch = -1;
	_tableLastIndex = -1;

	_interTable = nullptr;
}

void Codec48Decoder::init(int width, int height) {
	if (_width == width && _height == height)
		return;
	deinit();
	_width = width;
	_height = height;

	_blockX = (_width + 7) / 8;
	_blockY = (_height + 7) / 8;
	_pitch = _blockX * 8;

	// don't support when this is not equal yet
	assert(_width == _pitch);
}

void Codec48Decoder::deinit() {
}

Codec48Decoder::~Codec48Decoder() {
	delete[] _deltaBuf[0];
	delete[] _offsetTable;
	delete[] _interTable;
}

bool Codec48Decoder::decode(byte *dst, const byte *src) {
	// The header is identical to codec 37, except the flags field is somewhat different

	const byte *gfxData = src + 0x10;

	makeTable(_pitch, src[1]);

	int16 seqNb = READ_LE_UINT16(src + 2);

	if (seqNb == 0)
		memset(_deltaBuf[0], 0, _frameSize * 2);

	if (src[12] & (1 << 3)) {
		// Interpolation table present
		if (!_interTable)
			_interTable = new byte[65536];

		byte *ptr = _interTable;

		for (int i = 0; i < 256; i++) {
			byte *ptr1 = ptr + i;
			byte *ptr2 = ptr + i;

			for (int j = 256 - i; j > 0; j--) {
				byte pixel = *gfxData++;
				*ptr2 = pixel;
				*ptr1++ = pixel;
				ptr2 += 256;
			}

			ptr += 256;
		}
	}

	switch (src[0]) {
	case 0:
		// Raw frame
		memcpy(_deltaBuf[_curBuf], gfxData, READ_LE_UINT32(src + 4));
		break;
	case 2:
		// Blast object
		bompDecodeLine(_deltaBuf[_curBuf], gfxData, _width * _height);
		break;
	case 3:
		// 8x8 block encoding
		if (!(seqNb && seqNb != _prevSeqNb + 1)) {
			if (seqNb & 1 || !(src[12] & 1) || src[12] & 0x10)
				_curBuf ^= 1;

			decode3(_deltaBuf[_curBuf], gfxData, _deltaBuf[_curBuf ^ 1] - _deltaBuf[_curBuf]);
		}
		break;
	case 5:
		// Some other encoding, but it's unused. (Good)
		warning("SmushDecoder::decode() codec 48 frame type 5 encountered! Please report!");
		break;
	default:
		warning("SmushDecoder::decode() Unknown codec 48 frame type %d", src[0]);
		break;
	}

	_prevSeqNb = seqNb;
	memcpy(dst, _deltaBuf[_curBuf], _pitch * _height);
	return true;
}

void Codec48Decoder::bompDecodeLine(byte *dst, const byte *src, int len) {
	while (len > 0) {
		byte code = *src++;
		byte num = (code >> 1) + 1;

		if (num > len)
			num = len;

		len -= num;

		if (code & 1) {
			byte color = *src++;
			memset(dst, color, num);
		} else {
			memcpy(dst, src, num);
			src += num;
		}

		dst += num;
	}
}

void Codec48Decoder::makeTable(int pitch, int index) {
	// codec48's table is codec47's table appended by the first
	// part of codec37's table

	// This is essentially Codec37Decoder::makeTable() with a different table

	static const int8 table[] = {
		0,   0,  -1, -43,   6, -43,  -9, -42,  13, -41,
		-16, -40,  19, -39, -23, -36,  26, -34,  -2, -33,
		4, -33, -29, -32,  -9, -32,  11, -31, -16, -29,
		32, -29,  18, -28, -34, -26, -22, -25,  -1, -25,
		3, -25,  -7, -24,   8, -24,  24, -23,  36, -23,
		-12, -22,  13, -21, -38, -20,   0, -20, -27, -19,
		-4, -19,   4, -19, -17, -18,  -8, -17,   8, -17,
		18, -17,  28, -17,  39, -17, -12, -15,  12, -15,
		-21, -14,  -1, -14,   1, -14, -41, -13,  -5, -13,
		5, -13,  21, -13, -31, -12, -15, -11,  -8, -11,
		8, -11,  15, -11,  -2, -10,   1, -10,  31, -10,
		-23,  -9, -11,  -9,  -5,  -9,   4,  -9,  11,  -9,
		42,  -9,   6,  -8,  24,  -8, -18,  -7,  -7,  -7,
		-3,  -7,  -1,  -7,   2,  -7,  18,  -7, -43,  -6,
		-13,  -6,  -4,  -6,   4,  -6,   8,  -6, -33,  -5,
		-9,  -5,  -2,  -5,   0,  -5,   2,  -5,   5,  -5,
		13,  -5, -25,  -4,  -6,  -4,  -3,  -4,   3,  -4,
		9,  -4, -19,  -3,  -7,  -3,  -4,  -3,  -2,  -3,
		-1,  -3,   0,  -3,   1,  -3,   2,  -3,   4,  -3,
		6,  -3,  33,  -3, -14,  -2, -10,  -2,  -5,  -2,
		-3,  -2,  -2,  -2,  -1,  -2,   0,  -2,   1,  -2,
		2,  -2,   3,  -2,   5,  -2,   7,  -2,  14,  -2,
		19,  -2,  25,  -2,  43,  -2,  -7,  -1,  -3,  -1,
		-2,  -1,  -1,  -1,   0,  -1,   1,  -1,   2,  -1,
		3,  -1,  10,  -1,  -5,   0,  -3,   0,  -2,   0,
		-1,   0,   1,   0,   2,   0,   3,   0,   5,   0,
		7,   0, -10,   1,  -7,   1,  -3,   1,  -2,   1,
		-1,   1,   0,   1,   1,   1,   2,   1,   3,   1,
		-43,   2, -25,   2, -19,   2, -14,   2,  -5,   2,
		-3,   2,  -2,   2,  -1,   2,   0,   2,   1,   2,
		2,   2,   3,   2,   5,   2,   7,   2,  10,   2,
		14,   2, -33,   3,  -6,   3,  -4,   3,  -2,   3,
		-1,   3,   0,   3,   1,   3,   2,   3,   4,   3,
		19,   3,  -9,   4,  -3,   4,   3,   4,   7,   4,
		25,   4, -13,   5,  -5,   5,  -2,   5,   0,   5,
		2,   5,   5,   5,   9,   5,  33,   5,  -8,   6,
		-4,   6,   4,   6,  13,   6,  43,   6, -18,   7,
		-2,   7,   0,   7,   2,   7,   7,   7,  18,   7,
		-24,   8,  -6,   8, -42,   9, -11,   9,  -4,   9,
		5,   9,  11,   9,  23,   9, -31,  10,  -1,  10,
		2,  10, -15,  11,  -8,  11,   8,  11,  15,  11,
		31,  12, -21,  13,  -5,  13,   5,  13,  41,  13,
		-1,  14,   1,  14,  21,  14, -12,  15,  12,  15,
		-39,  17, -28,  17, -18,  17,  -8,  17,   8,  17,
		17,  18,  -4,  19,   0,  19,   4,  19,  27,  19,
		38,  20, -13,  21,  12,  22, -36,  23, -24,  23,
		-8,  24,   7,  24,  -3,  25,   1,  25,  22,  25,
		34,  26, -18,  28, -32,  29,  16,  29, -11,  31,
		9,  32,  29,  32,  -4,  33,   2,  33, -26,  34,
		23,  36, -19,  39,  16,  40, -13,  41,   9,  42,
		-6,  43,   1,  43,   0,   0,   0,   0,   0,   0,
		0,   0,   1,   0,   2,   0,   3,   0,   5,   0,
		8,   0,  13,   0,  21,   0,  -1,   0,  -2,   0,
		-3,   0,  -5,   0,  -8,   0, -13,   0, -17,   0,
		-21,   0,   0,   1,   1,   1,   2,   1,   3,   1,
		5,   1,   8,   1,  13,   1,  21,   1,  -1,   1,
		-2,   1,  -3,   1,  -5,   1,  -8,   1, -13,   1,
		-17,   1, -21,   1,   0,   2,   1,   2,   2,   2,
		3,   2,   5,   2,   8,   2,  13,   2,  21,   2,
		-1,   2,  -2,   2,  -3,   2,  -5,   2,  -8,   2,
		-13,   2, -17,   2, -21,   2,   0,   3,   1,   3,
		2,   3,   3,   3,   5,   3,   8,   3,  13,   3,
		21,   3,  -1,   3,  -2,   3,  -3,   3,  -5,   3,
		-8,   3, -13,   3, -17,   3, -21,   3,   0,   5,
		1,   5,   2,   5,   3,   5,   5,   5,   8,   5,
		13,   5,  21,   5,  -1,   5,  -2,   5,  -3,   5,
		-5,   5,  -8,   5, -13,   5, -17,   5, -21,   5,
		0,   8,   1,   8,   2,   8,   3,   8,   5,   8,
		8,   8,  13,   8,  21,   8,  -1,   8,  -2,   8,
		-3,   8,  -5,   8,  -8,   8, -13,   8, -17,   8,
		-21,   8,   0,  13,   1,  13,   2,  13,   3,  13,
		5,  13,   8,  13,  13,  13,  21,  13,  -1,  13,
		-2,  13,  -3,  13,  -5,  13,  -8,  13, -13,  13,
		-17,  13, -21,  13,   0,  21,   1,  21,   2,  21,
		3,  21,   5,  21,   8,  21,  13,  21,  21,  21,
		-1,  21,  -2,  21,  -3,  21,  -5,  21,  -8,  21,
		-13,  21, -17,  21, -21,  21,   0,  -1,   1,  -1,
		2,  -1,   3,  -1,   5,  -1,   8,  -1,  13,  -1,
		21,  -1,  -1,  -1,  -2,  -1,  -3,  -1,  -5,  -1,
		-8,  -1, -13,  -1, -17,  -1, -21,  -1,   0,  -2,
		1,  -2,   2,  -2,   3,  -2,   5,  -2,   8,  -2,
		13,  -2,  21,  -2,  -1,  -2,  -2,  -2,  -3,  -2,
		-5,  -2,  -8,  -2, -13,  -2, -17,  -2, -21,  -2,
		0,  -3,   1,  -3,   2,  -3,   3,  -3,   5,  -3,
		8,  -3,  13,  -3,  21,  -3,  -1,  -3,  -2,  -3,
		-3,  -3,  -5,  -3,  -8,  -3, -13,  -3, -17,  -3,
		-21,  -3,   0,  -5,   1,  -5,   2,  -5,   3,  -5,
		5,  -5,   8,  -5,  13,  -5,  21,  -5,  -1,  -5,
		-2,  -5,  -3,  -5,  -5,  -5,  -8,  -5, -13,  -5,
		-17,  -5, -21,  -5,   0,  -8,   1,  -8,   2,  -8,
		3,  -8,   5,  -8,   8,  -8,  13,  -8,  21,  -8,
		-1,  -8,  -2,  -8,  -3,  -8,  -5,  -8,  -8,  -8,
		-13,  -8, -17,  -8, -21,  -8,   0, -13,   1, -13,
		2, -13,   3, -13,   5, -13,   8, -13,  13, -13,
		21, -13,  -1, -13,  -2, -13,  -3, -13,  -5, -13,
		-8, -13, -13, -13, -17, -13, -21, -13,   0, -17,
		1, -17,   2, -17,   3, -17,   5, -17,   8, -17,
		13, -17,  21, -17,  -1, -17,  -2, -17,  -3, -17,
		-5, -17,  -8, -17, -13, -17, -17, -17, -21, -17,
		0, -21,   1, -21,   2, -21,   3, -21,   5, -21,
		8, -21,  13, -21,  21, -21,  -1, -21,  -2, -21,
		-3, -21,  -5, -21,  -8, -21, -13, -21, -17, -21
	};

	if (_tableLastPitch == pitch && _tableLastIndex == index)
		return;

	_tableLastPitch = pitch;
	_tableLastIndex = index;
	index *= 255;
	assert(index + 254 < (int32)(sizeof(table) / 2));

	for (int32 i = 0; i < 255; i++) {
		int32 j = (i + index) * 2;
		_offsetTable[i] = table[j + 1] * pitch + table[j];
	}
}

void Codec48Decoder::decode3(byte *dst, const byte *src, int bufOffset) {
	for (int i = 0; i < _blockY; i++) {
		for (int j = 0; j < _blockX; j++) {
			byte opcode = *src++;
			

			switch (opcode) {
			case 0xFF: {
				// Interpolate a 4x4 block based on 1 pixel, then scale to 8x8
				byte scaleBuffer[16];
				scaleBuffer[15] = *src++;
				scaleBuffer[7] = _interTable[(dst[-_pitch + 7] << 8) | scaleBuffer[15]];
				scaleBuffer[3] = _interTable[(dst[-_pitch + 7] << 8) | scaleBuffer[7]];
				scaleBuffer[11] = _interTable[(scaleBuffer[15] << 8) | scaleBuffer[7]];

				scaleBuffer[1] = _interTable[(dst[-1] << 8) | scaleBuffer[3]];
				scaleBuffer[0] = _interTable[(dst[-1] << 8) | scaleBuffer[1]];
				scaleBuffer[2] = _interTable[(scaleBuffer[3] << 8) | scaleBuffer[1]];

				scaleBuffer[5] = _interTable[(dst[_pitch * 2 - 1] << 8) | scaleBuffer[7]];
				scaleBuffer[4] = _interTable[(dst[_pitch * 2 - 1] << 8) | scaleBuffer[5]];
				scaleBuffer[6] = _interTable[(scaleBuffer[7] << 8) | scaleBuffer[5]];

				scaleBuffer[9] = _interTable[(dst[_pitch * 3 - 1] << 8) | scaleBuffer[11]];
				scaleBuffer[8] = _interTable[(dst[_pitch * 3 - 1] << 8) | scaleBuffer[9]];
				scaleBuffer[10] = _interTable[(scaleBuffer[11] << 8) | scaleBuffer[9]];

				scaleBuffer[13] = _interTable[(dst[_pitch * 4 - 1] << 8) | scaleBuffer[15]];
				scaleBuffer[12] = _interTable[(dst[_pitch * 4 - 1] << 8) | scaleBuffer[13]];
				scaleBuffer[14] = _interTable[(scaleBuffer[15] << 8) | scaleBuffer[13]];

				scaleBlock(dst, scaleBuffer);
				break;
			}
			case 0xFE:
				// Copy a block using an absolute offset
				copyBlock(dst, bufOffset, (int16)READ_LE_UINT16(src));
				src += 2;
				break;
			case 0xFD: {
				// Interpolate a 4x4 block based on 4 pixels, then scale to 8x8
				byte scaleBuffer[16];
				scaleBuffer[5] = src[0];
				scaleBuffer[7] = src[1];
				scaleBuffer[13] = src[2];
				scaleBuffer[15] = src[3];

				scaleBuffer[1] = _interTable[(dst[-_pitch + 3] << 8) | scaleBuffer[5]];
				scaleBuffer[3] = _interTable[(dst[-_pitch + 7] << 8) | scaleBuffer[7]];
				scaleBuffer[11] = _interTable[(scaleBuffer[15] << 8) | scaleBuffer[7]];
				scaleBuffer[9] = _interTable[(scaleBuffer[13] << 8) | scaleBuffer[5]];

				scaleBuffer[0] = _interTable[(dst[-1] << 8) | scaleBuffer[1]];
				scaleBuffer[2] = _interTable[(scaleBuffer[3] << 8) | scaleBuffer[1]];
				scaleBuffer[4] = _interTable[(dst[_pitch * 2 - 1] << 8) | scaleBuffer[5]];
				scaleBuffer[6] = _interTable[(scaleBuffer[7] << 8) | scaleBuffer[5]];

				scaleBuffer[8] = _interTable[(dst[_pitch * 3 - 1] << 8) | scaleBuffer[9]];
				scaleBuffer[10] = _interTable[(scaleBuffer[11] << 8) | scaleBuffer[9]];
				scaleBuffer[12] = _interTable[(dst[_pitch * 4 - 1] << 8) | scaleBuffer[13]];
				scaleBuffer[14] = _interTable[(scaleBuffer[15] << 8) | scaleBuffer[13]];
				
				scaleBlock(dst, scaleBuffer);

				src += 4;
				break;
			}
			case 0xFC:
				// Copy 4 4x4 blocks using the offset table
				*((uint32 *)dst) = *((uint32 *)(dst + bufOffset + _offsetTable[src[0]]));
				*((uint32 *)(dst + _pitch)) = *((uint32 *)(dst + bufOffset + _offsetTable[src[0]] + _pitch));
				*((uint32 *)(dst + _pitch * 2)) = *((uint32 *)(dst + bufOffset + _offsetTable[src[0]] + _pitch * 2));
				*((uint32 *)(dst + _pitch * 3)) = *((uint32 *)(dst + bufOffset + _offsetTable[src[0]] + _pitch * 3));

				*((uint32 *)(dst + 4)) = *((uint32 *)(dst + bufOffset + _offsetTable[src[1]] + 4));
				*((uint32 *)(dst + _pitch + 4)) = *((uint32 *)(dst + bufOffset + _offsetTable[src[1]] + _pitch + 4));
				*((uint32 *)(dst + _pitch * 2 + 4)) = *((uint32 *)(dst + bufOffset + _offsetTable[src[1]] + _pitch * 2 + 4));
				*((uint32 *)(dst + _pitch * 3 + 4)) = *((uint32 *)(dst + bufOffset + _offsetTable[src[1]] + _pitch * 3 + 4));

				*((uint32 *)(dst + _pitch * 4)) = *((uint32 *)(dst + bufOffset + _offsetTable[src[2]] + _pitch * 4));
				*((uint32 *)(dst + _pitch * 5)) = *((uint32 *)(dst + bufOffset + _offsetTable[src[2]] + _pitch * 5));
				*((uint32 *)(dst + _pitch * 6)) = *((uint32 *)(dst + bufOffset + _offsetTable[src[2]] + _pitch * 6));
				*((uint32 *)(dst + _pitch * 7)) = *((uint32 *)(dst + bufOffset + _offsetTable[src[2]] + _pitch * 7));

				*((uint32 *)(dst + _pitch * 4 + 4)) = *((uint32 *)(dst + bufOffset + _offsetTable[src[3]] + _pitch * 4 + 4));
				*((uint32 *)(dst + _pitch * 5 + 4)) = *((uint32 *)(dst + bufOffset + _offsetTable[src[3]] + _pitch * 5 + 4));
				*((uint32 *)(dst + _pitch * 6 + 4)) = *((uint32 *)(dst + bufOffset + _offsetTable[src[3]] + _pitch * 6 + 4));
				*((uint32 *)(dst + _pitch * 7 + 4)) = *((uint32 *)(dst + bufOffset + _offsetTable[src[3]] + _pitch * 7 + 4));

				src += 4;
				break;
			case 0xFB:
				// Copy 4 4x4 blocks using absolute offsets
				*((uint32 *)dst) = *((uint32 *)(dst + bufOffset + (int16)READ_LE_UINT16(src)));
				*((uint32 *)(dst + _pitch)) = *((uint32 *)(dst + bufOffset + (int16)READ_LE_UINT16(src) + _pitch));
				*((uint32 *)(dst + _pitch * 2)) = *((uint32 *)(dst + bufOffset + (int16)READ_LE_UINT16(src) + _pitch * 2));
				*((uint32 *)(dst + _pitch * 3)) = *((uint32 *)(dst + bufOffset + (int16)READ_LE_UINT16(src) + _pitch * 3));

				*((uint32 *)(dst + 4)) = *((uint32 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 2) + 4));
				*((uint32 *)(dst + _pitch + 4)) = *((uint32 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 2) + _pitch + 4));
				*((uint32 *)(dst + _pitch * 2 + 4)) = *((uint32 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 2) + _pitch * 2 + 4));
				*((uint32 *)(dst + _pitch * 3 + 4)) = *((uint32 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 2) + _pitch * 3 + 4));

				*((uint32 *)(dst + _pitch * 4)) = *((uint32 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 4) + _pitch * 4));
				*((uint32 *)(dst + _pitch * 5)) = *((uint32 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 4) + _pitch * 5));
				*((uint32 *)(dst + _pitch * 6)) = *((uint32 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 4) + _pitch * 6));
				*((uint32 *)(dst + _pitch * 7)) = *((uint32 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 4) + _pitch * 7));

				*((uint32 *)(dst + _pitch * 4 + 4)) = *((uint32 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 6) + _pitch * 4 + 4));
				*((uint32 *)(dst + _pitch * 5 + 4)) = *((uint32 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 6) + _pitch * 5 + 4));
				*((uint32 *)(dst + _pitch * 6 + 4)) = *((uint32 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 6) + _pitch * 6 + 4));
				*((uint32 *)(dst + _pitch * 7 + 4)) = *((uint32 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 6) + _pitch * 7 + 4));
				src += 8;
				break;
			case 0xFA:
				// Scale a 4x4 block to an 8x8 block
				scaleBlock(dst, src);
				src += 16;
				break;
			case 0xF9:
				// Copy 16 2x2 blocks using the offset table
				*((uint16 *)dst) = *((uint16 *)(dst + bufOffset + _offsetTable[src[0]]));
				*((uint16 *)(dst + _pitch)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[0]] + _pitch));

				*((uint16 *)(dst + 2)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[1]] + 2));
				*((uint16 *)(dst + _pitch + 2)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[1]] + _pitch + 2));

				*((uint16 *)(dst + 4)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[2]] + 4));
				*((uint16 *)(dst + _pitch + 4)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[2]] + _pitch + 4));

				*((uint16 *)(dst + 6)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[3]] + 6));
				*((uint16 *)(dst + _pitch + 6)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[3]] + _pitch + 6));

				*((uint16 *)(dst + _pitch * 2)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[4]] + _pitch * 2));
				*((uint16 *)(dst + _pitch * 3)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[4]] + _pitch * 3));

				*((uint16 *)(dst + _pitch * 2 + 2)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[5]] + _pitch * 2 + 2));
				*((uint16 *)(dst + _pitch * 3 + 2)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[5]] + _pitch * 3 + 2));

				*((uint16 *)(dst + _pitch * 2 + 4)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[6]] + _pitch * 2 + 4));
				*((uint16 *)(dst + _pitch * 3 + 4)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[6]] + _pitch * 3 + 4));

				*((uint16 *)(dst + _pitch * 2 + 6)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[7]] + _pitch * 2 + 6));
				*((uint16 *)(dst + _pitch * 3 + 6)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[7]] + _pitch * 3 + 6));

				*((uint16 *)(dst + _pitch * 4)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[8]] + _pitch * 4));
				*((uint16 *)(dst + _pitch * 5)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[8]] + _pitch * 5));

				*((uint16 *)(dst + _pitch * 4 + 2)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[9]] + _pitch * 4 + 2));
				*((uint16 *)(dst + _pitch * 5 + 2)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[9]] + _pitch * 5 + 2));

				*((uint16 *)(dst + _pitch * 4 + 4)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[10]] + _pitch * 4 + 4));
				*((uint16 *)(dst + _pitch * 5 + 4)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[10]] + _pitch * 5 + 4));

				*((uint16 *)(dst + _pitch * 4 + 6)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[11]] + _pitch * 4 + 6));
				*((uint16 *)(dst + _pitch * 5 + 6)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[11]] + _pitch * 5 + 6));

				*((uint16 *)(dst + _pitch * 6)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[12]] + _pitch * 6));
				*((uint16 *)(dst + _pitch * 7)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[12]] + _pitch * 7));

				*((uint16 *)(dst + _pitch * 6 + 2)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[13]] + _pitch * 6 + 2));
				*((uint16 *)(dst + _pitch * 7 + 2)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[13]] + _pitch * 7 + 2));

				*((uint16 *)(dst + _pitch * 6 + 4)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[14]] + _pitch * 6 + 4));
				*((uint16 *)(dst + _pitch * 7 + 4)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[14]] + _pitch * 7 + 4));

				*((uint16 *)(dst + _pitch * 6 + 6)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[15]] + _pitch * 6 + 6));
				*((uint16 *)(dst + _pitch * 7 + 6)) = *((uint16 *)(dst + bufOffset + _offsetTable[src[15]] + _pitch * 7 + 6));
				src += 16;
				break;
			case 0xF8:
				// Copy 16 2x2 blocks using absolute offsets
				*((uint16 *)dst) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src)));
				*((uint16 *)(dst + _pitch)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src) + _pitch));
		
				*((uint16 *)(dst + 2)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 2) + 2));
				*((uint16 *)(dst + _pitch + 2)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 2) + _pitch + 2));

				*((uint16 *)(dst + 4)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 4) + 4));
				*((uint16 *)(dst + _pitch + 4)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 4) + _pitch + 4));

				*((uint16 *)(dst + 6)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 6) + 6));
				*((uint16 *)(dst + _pitch + 6)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 6) + _pitch + 6));

				*((uint16 *)(dst + _pitch * 2)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 8) + _pitch * 2));
				*((uint16 *)(dst + _pitch * 3)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 8) + _pitch * 3));

				*((uint16 *)(dst + _pitch * 2 + 2)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 10) + _pitch * 2 + 2));
				*((uint16 *)(dst + _pitch * 3 + 2)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 10) + _pitch * 3 + 2));

				*((uint16 *)(dst + _pitch * 2 + 4)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 12) + _pitch * 2 + 4));
				*((uint16 *)(dst + _pitch * 3 + 4)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 12) + _pitch * 3 + 4));

				*((uint16 *)(dst + _pitch * 2 + 6)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 14) + _pitch * 2 + 6));
				*((uint16 *)(dst + _pitch * 3 + 6)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 14) + _pitch * 3 + 6));

				*((uint16 *)(dst + _pitch * 4)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 16) + _pitch * 4));
				*((uint16 *)(dst + _pitch * 5)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 16) + _pitch * 5));

				*((uint16 *)(dst + _pitch * 4 + 2)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 18) + _pitch * 4 + 2));
				*((uint16 *)(dst + _pitch * 5 + 2)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 18) + _pitch * 5 + 2));

				*((uint16 *)(dst + _pitch * 4 + 4)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 20) + _pitch * 4 + 4));
				*((uint16 *)(dst + _pitch * 5 + 4)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 20) + _pitch * 5 + 4));

				*((uint16 *)(dst + _pitch * 4 + 6)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 22) + _pitch * 4 + 6));
				*((uint16 *)(dst + _pitch * 5 + 6)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 22) + _pitch * 5 + 6));

				*((uint16 *)(dst + _pitch * 6)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 24) + _pitch * 6));
				*((uint16 *)(dst + _pitch * 7)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 24) + _pitch * 7));

				*((uint16 *)(dst + _pitch * 6 + 2)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 26) + _pitch * 6 + 2));
				*((uint16 *)(dst + _pitch * 7 + 2)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 26) + _pitch * 7 + 2));

				*((uint16 *)(dst + _pitch * 6 + 4)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 28) + _pitch * 6 + 4));
				*((uint16 *)(dst + _pitch * 7 + 4)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 28) + _pitch * 7 + 4));

				*((uint16 *)(dst + _pitch * 6 + 6)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 30) + _pitch * 6 + 6));
				*((uint16 *)(dst + _pitch * 7 + 6)) = *((uint16 *)(dst + bufOffset + (int16)READ_LE_UINT16(src + 30) + _pitch * 7 + 6));

				src += 32;
				break;
			case 0xF7:
				// Raw 8x8 block
				*((uint32 *)dst) = *((const uint32 *)src);
				*((uint32 *)(dst + 4)) = *((const uint32 *)(src + 4));
				*((uint32 *)(dst + _pitch)) = *((const uint32 *)(src + 8));
				*((uint32 *)(dst + _pitch + 4)) = *((const uint32 *)(src + 12));
				*((uint32 *)(dst + _pitch * 2)) = *((const uint32 *)(src + 16));
				*((uint32 *)(dst + _pitch * 2 + 4)) = *((const uint32 *)(src + 20));
				*((uint32 *)(dst + _pitch * 3)) = *((const uint32 *)(src + 24));
				*((uint32 *)(dst + _pitch * 3 + 4)) = *((const uint32 *)(src + 28));
				*((uint32 *)(dst + _pitch * 4)) = *((const uint32 *)(src + 32));
				*((uint32 *)(dst + _pitch * 4 + 4)) = *((const uint32 *)(src + 36));
				*((uint32 *)(dst + _pitch * 5)) = *((const uint32 *)(src + 40));
				*((uint32 *)(dst + _pitch * 5 + 4)) = *((const uint32 *)(src + 44));
				*((uint32 *)(dst + _pitch * 6)) = *((const uint32 *)(src + 48));
				*((uint32 *)(dst + _pitch * 6 + 4)) = *((const uint32 *)(src + 52));
				*((uint32 *)(dst + _pitch * 7)) = *((const uint32 *)(src + 56));
				*((uint32 *)(dst + _pitch * 7 + 4)) = *((const uint32 *)(src + 60));

				src += 64;
				break;
			default:
				// Copy a block using the offset table
				copyBlock(dst, bufOffset, _offsetTable[opcode]);
				break;
			}

			dst += 8;
		}

		dst += _pitch * 7;
	}
}

void Codec48Decoder::copyBlock(byte *dst, int deltaBufOffset, int offset) {
	const byte *src = dst + deltaBufOffset + offset;

	for (int i = 0; i < 8; i++) {
		*((uint32 *)(dst + _pitch * i)) = *((const uint32 *)(src + _pitch * i));
		*((uint32 *)(dst + _pitch * i + 4)) = *((const uint32 *)(src + _pitch * i + 4));
	}
}

void Codec48Decoder::scaleBlock(byte *dst, const byte *src) {
	// This is doing a 2x scale of data

	for (int i = 0; i < 4; i++) {
		uint16 pixels = src[0];
		pixels = (pixels << 8) | pixels;
		*((uint16 *)dst) = pixels;
		*((uint16 *)(dst + _pitch)) = pixels;
		pixels = src[1];
		pixels = (pixels << 8) | pixels;
		*((uint16 *)(dst + 2)) = pixels;
		*((uint16 *)(dst + _pitch + 2)) = pixels;
		pixels = src[2];
		pixels = (pixels << 8) | pixels;
		*((uint16 *)(dst + 4)) = pixels;
		*((uint16 *)(dst + _pitch + 4)) = pixels;
		pixels = src[3];
		pixels = (pixels << 8) | pixels;
		*((uint16 *)(dst + 6)) = pixels;
		*((uint16 *)(dst + _pitch + 6)) = pixels;
		src += 4;
		dst += _pitch * 2;
	}
}

} // end of namespace Grim
