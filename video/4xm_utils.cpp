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
	short next;
	short falseIdx;
	short trueIdx;
};

Common::Array<byte> unpackHuffman(const byte *huff, uint huffSize) {
	HuffChar table[514] = {};
	uint offset = 0;
	uint8 codebyte = huff[offset++];
	do {
		uint8 next = huff[offset++];
		if (codebyte <= next) {
			for (auto idx = codebyte; idx <= next; ++idx) {
				table[idx].next = huff[offset++];
			}
		}
		codebyte = huff[offset++];
	} while (codebyte != 0);
	table[256].next = 1;
	table[513].next = 0x7FFF;

	short startEntry;
	short codeIdx = 257, nIdx = 257;
	while (true) {
		short idx = 0, dstIdx = 0;
		short trueIdx = 513, falseIdx = 513;
		short nextLo = 513, nextHi = 513;
		while (idx < nIdx) {
			auto next = table[dstIdx].next;
			if (next != 0) {
				if (next >= table[nextLo].next) {
					if (next < table[nextHi].next) {
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
		table[codeIdx].next = table[falseIdx].next + table[trueIdx].next;
		table[falseIdx].next = table[trueIdx].next = 0;
		table[codeIdx].falseIdx = falseIdx;
		table[codeIdx].trueIdx = trueIdx;
		++codeIdx;
		++nIdx;
	}
	Common::Array<byte> decoded;
	decoded.reserve(huffSize);
	{
		BitStream bs(huff, offset);
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
	return decoded;
}

} // namespace FourXM
} // namespace Video
