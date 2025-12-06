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

#include "common/array.h"

namespace Video {
namespace FourXM {

class BitStream {
	const byte *_data;
	uint _bytePos;
	byte _bitMask;

public:
	BitStream(const byte *data, uint bytePos) : _data(data), _bytePos(bytePos), _bitMask(0x80) {}

	uint getBytePos() const {
		return _bytePos;
	}

	bool readBit() {
		bool bit = _data[_bytePos] & _bitMask;
		_bitMask >>= 1;
		if (_bitMask == 0) {
			_bitMask = 128;
			++_bytePos;
		}
		return bit;
	}

	int readUInt(byte n) {
		int value = 0;
		for (int i = 0; i != n; ++i) {
			if (readBit())
				value |= 1 << i;
		}
		return value;
	}

	int readInt(byte n) {
		int value = readUInt(n);
		if ((value & (1 << (n - 1))) == 0)
			value += 1 - (1 << n);
		return value;
	}

	void alignToByte() {
		if (_bitMask != 0x80) {
			_bitMask = 128;
			++_bytePos;
		}
	}
};
Common::Array<byte> unpackHuffman(const byte *huff, uint huffSize);

} // namespace FourXM
} // namespace Video
