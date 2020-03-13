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

// John_Doe's implementation

#include "prince/decompress.h"

namespace Prince {

static const uint16 table1[] = {
	0x8000, 0x0002,
	0x4000, 0x0004,
	0x2000, 0x0008,
	0x1000, 0x0010,
	0x0800, 0x0020,
	0x0400, 0x0040,
	0x0200, 0x0080,
	0x0100, 0x0100,
	0x0080, 0x0200,
	0x0040, 0x0400
};

static const uint32 table2[] = {
	0x0000F000,
	0x0020FC00,
	0x00A0FF00,
	0x02A0FF80,
	0x06A0FFC0,
	0x0EA0FFE0,
	0x1EA0FFF0,
	0x3EA0FFF8
};

static const uint16 table3[] = {
	0x8000, 0x0000,
	0x4000, 0x0002,
	0x2000, 0x0006,
	0x1000, 0x000E,
	0x0800, 0x001E,
	0x0400, 0x003E,
	0x0200, 0x007E,
	0x0100, 0x00FE,
	0x0080, 0x01FE,
	0x0040, 0x03FE,
	0x0020, 0x07FE,
	0x0010, 0x0FFE,
	0x0008, 0x1FFE,
	0x0004, 0x3FFE,
	0x0002, 0x7FFE,
	0x0001, 0xFFFE
};

void Decompressor::decompress(byte *source, byte *dest, uint32 destSize) {
	byte *destEnd = dest + destSize;
	int more;
	_src = source;
	_dst = dest;
	_bitBuffer = 0x80;
	while (_dst < destEnd) {
		uint32 ebp;
		uint16 offset, length;
		if (getBit()) {
			if (getBit()) {
				if (getBit()) {
					if (getBit()) {
						if (getBit()) {
							if (getBit()) {
								uint32 tableIndex = 0;
								while (getBit())
									tableIndex++;
								length = table3[tableIndex * 2 + 0];
								do {
									more = !(length & 0x8000);
									length = (length << 1) | getBit();
								} while (more);
								length += table3[tableIndex * 2 + 1];
								length++;
								memcpy(_dst, _src, length);
								_src += length;
								_dst += length;
							}
							*_dst++ = *_src++;
						}
						*_dst++ = *_src++;
					}
					*_dst++ = *_src++;
				}
				*_dst++ = *_src++;
			}
			*_dst++ = *_src++;
		}
		if (!getBit()) {
			if (getBit()) {
				uint32 tableIndex = getBit();
				tableIndex = (tableIndex << 1) | getBit();
				tableIndex = (tableIndex << 1) | getBit();
				ebp = table2[tableIndex];
				length = 1;
			} else {
				ebp = 0x0000FF00;
				length = 0;
			}
		} else {
			uint32 tableIndex = 0;
			while (getBit())
				tableIndex++;
			length = table1[tableIndex * 2 + 0];
			do {
				more = !(length & 0x8000);
				length = (length << 1) | getBit();
			} while (more);
			length += table1[tableIndex * 2 + 1];
			tableIndex = getBit();
			tableIndex = (tableIndex << 1) | getBit();
			tableIndex = (tableIndex << 1) | getBit();
			ebp = table2[tableIndex];
		}
		offset = ebp & 0xFFFF;
		do {
			if (_bitBuffer == 0x80) {
				if (offset >= 0xFF00) {
					offset = (offset << 8) | *_src++;
				}
			}
			more = offset & 0x8000;
			offset = (offset << 1) | getBit();
		} while (more);
		offset += (ebp >> 16);
		length += 2;
		while (length--) {
			if (_dst >= destEnd) {
				return;
			}

			if (_dst - offset >= dest)
				*_dst = *(_dst - offset);
			_dst++;
		}
	}
}

int Decompressor::getBit() {
	int bit = (_bitBuffer & 0x80) >> 7;
	_bitBuffer <<= 1;
	if (_bitBuffer == 0) {
		_bitBuffer = *_src++;
		bit = (_bitBuffer & 0x80) >> 7;
		_bitBuffer <<= 1;
		_bitBuffer |= 1;
	}
	return bit;
}

} // End of namespace Prince
