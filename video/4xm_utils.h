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

template<typename Type, bool BigEndian>
class BitStream {
	const Type *_data;
	uint _size;
	uint _wordPos;
	Type _bitMask;
	static constexpr Type InitialMask = Type(1) << (sizeof(Type) * 8 - 1);
	static constexpr uint WordSize = sizeof(Type);

public:
	BitStream(const Type *data, uint size, uint wordPos) : _data(data), _size(size), _wordPos(wordPos), _bitMask(InitialMask) {}

	uint getWordPos() const {
		return _wordPos;
	}

	bool readBit() {
		assert(_wordPos < _size);
		bool bit = _data[_wordPos] & _bitMask;
		_bitMask >>= 1;
		if (_bitMask == 0) {
			_bitMask = InitialMask;
			++_wordPos;
		}
		return bit;
	}

	int readUInt(byte n) {
		int value = 0;
		if (BigEndian) {
			for (int i = 0; i != n; ++i) {
				value <<= 1;
				if (readBit())
					value |= 1;
			}
		} else {
			for (int i = 0; i != n; ++i) {
				if (readBit())
					value |= 1 << i;
			}
		}
		return value;
	}

	int readInt(byte n) {
		int value = readUInt(n);
		if ((value & (1 << (n - 1))) == 0)
			value += 1 - (1 << n);
		return value;
	}

	void alignToWord() {
		if (_bitMask != InitialMask) {
			_bitMask = InitialMask;
			++_wordPos;
		}
	}
};
using LEByteBitStream = BitStream<byte, false>;
using BEByteBitStream = BitStream<byte, true>;
using LEWordBitStream = BitStream<uint32, false>;

class HuffmanDecoder {
	static constexpr uint kMaxTableSize = 514;
	static constexpr uint kLastEntry = kMaxTableSize - 1;
	struct HuffChar {
		uint freq = 0;
		ushort falseIdx = kMaxTableSize;
		ushort trueIdx = kMaxTableSize;
	};
	HuffChar _table[kMaxTableSize] = {};
	uint _startEntry = 0;
	uint _numCodes = 0;

public:
	uint loadStatistics(const byte *huff, uint huffSize);
	void initStatistics(const std::initializer_list<uint> &freqs);

	Common::Array<byte> unpack(const byte *huff, uint huffSize, uint &offset, byte wordSize);

	static Common::Array<byte> unpack(const byte *huff, uint huffSize, byte wordSize);

	template<typename BitStreamType>
	uint next(BitStreamType &bs);

	void dump() const;

private:
	template<typename Word>
	Common::Array<byte> unpackStream(const byte *huff, uint huffSize, uint &offset);

	void buildTable(uint numCodes);
	void dumpImpl(uint code, uint size, uint index, uint ch) const;
};

void idct(int16_t block[64]);

} // namespace FourXM
} // namespace Video
