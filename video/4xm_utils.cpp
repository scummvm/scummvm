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

#include "video/4xm_utils.h"
#include "common/debug.h"

namespace Video {
namespace FourXM {

struct HuffChar {
	short freq;
	short falseIdx;
	short trueIdx;
};

Common::Array<byte> unpackHuffman(const byte *huff, uint huffSize, bool alignedStart) {
	HuffChar table[514] = {};
	uint offset = 0;
	uint8 codebyte = huff[offset++];
	do {
		uint8 freq = huff[offset++];
		if (codebyte <= freq) {
			for (auto idx = codebyte; idx <= freq; ++idx) {
				table[idx].freq = huff[offset++];
			}
		}
		codebyte = huff[offset++];
	} while (codebyte != 0);
	if (alignedStart && (offset % 4) != 0) {
		offset += 4 - (offset % 4);
	}
	table[256].freq = 1;
	table[513].freq = 0x7FFF;

	short startEntry;
	short codeIdx = 257, nIdx = 257;
	while (true) {
		short idx = 0, dstIdx = 0;
		short trueIdx = 513, falseIdx = 513;
		short nextLo = 513, nextHi = 513;
		while (idx < nIdx) {
			auto freq = table[dstIdx].freq;
			if (freq != 0) {
				if (freq >= table[nextLo].freq) {
					if (freq < table[nextHi].freq) {
						trueIdx = idx;
						nextHi = dstIdx;
					}
				} else {
					trueIdx = falseIdx;
					nextHi = nextLo;
					falseIdx = idx;
					nextLo = dstIdx;
				}
			}
			++idx;
			++dstIdx;
		}
		if (trueIdx == 513) {
			startEntry = nIdx - 1;
			break;
		}
		table[codeIdx].freq = table[falseIdx].freq + table[trueIdx].freq;
		table[falseIdx].freq = table[trueIdx].freq = 0;
		table[codeIdx].falseIdx = falseIdx;
		table[codeIdx].trueIdx = trueIdx;
		++codeIdx;
		++nIdx;
	}
	Common::Array<byte> decoded;
	decoded.reserve(huffSize * 2);
	{
		BitStream bs(huff, huffSize, offset);
		while (true) {
			short value = startEntry;
			while (value > 256) {
				auto bit = bs.readBit();
				if (bit)
					value = table[value].trueIdx;
				else
					value = table[value].falseIdx;
			}
			if (value == 256)
				break;
			decoded.push_back(static_cast<byte>(value));
		}
		bs.alignToByte();
		offset = bs.getBytePos();
	}
	debug("decoded %u bytes at %08x", decoded.size(), offset);
	assert(offset == huffSize); // must decode to the end
	return decoded;
}

} // namespace FourXM
} // namespace Video
