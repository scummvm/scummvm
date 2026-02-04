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

class HuffmanDecoder {
	static constexpr uint kMaxTableSize = 514;
	static constexpr uint kLastEntry = kMaxTableSize - 1;
	struct HuffChar {
		uint freq = 0;
		uint16 falseIdx = kMaxTableSize;
		uint16 trueIdx = kMaxTableSize;
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

void idct(int16_t block[64], int shift = 6);

inline int readInt(int value, unsigned n) {
	if ((value & (1 << (n - 1))) == 0)
		value += 1 - (1 << n);
	return value;
}

template<typename BitStream>
inline int readInt(BitStream &bs, size_t n) {
	return readInt(bs.getBits(n), n);
}

} // namespace FourXM
} // namespace Video
