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
#include "common/compression/huffman.h"
#include "common/debug.h"

namespace Video {
namespace FourXM {

template<typename HuffmanType>
HuffmanType loadStatistics(const byte *&huff, uint &offset) {
	Common::Array<uint32> freqs, symbols;

	uint8 freq_first = huff[offset++];
	do {
		uint8 freq_last = huff[offset++];
		if (freq_first <= freq_last) {
			for (auto idx = freq_first; idx <= freq_last; ++idx) {
				auto freq = huff[offset++];
				if (freq != 0) {
					freqs.push_back(freq);
					symbols.push_back(idx);
				}
			}
		}
		freq_first = huff[offset++];
	} while (freq_first != 0);

	freqs.push_back(1);
	symbols.push_back(256);

	return HuffmanType::fromFrequencies(freqs.size(), freqs.data(), symbols.data());
}

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
