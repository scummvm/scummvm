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
	int freq;
	short falseIdx;
	short trueIdx;
};

namespace {

template<typename Word>
Common::Array<byte> unpackStream(const byte *huff, uint huffSize, uint &offset, const HuffChar *table, int startEntry) {
	Common::Array<byte> decoded;
	decoded.reserve(huffSize * 2);
	assert((offset % sizeof(Word)) == 0);
	BitStream<Word> bs(reinterpret_cast<const Word *>(huff), huffSize / sizeof(Word), offset / sizeof(Word));
	while (true) {
		int value = startEntry;
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
	bs.alignToWord();
	offset = bs.getWordPos() * sizeof(Word);
	return decoded;
}

} // namespace

Common::Array<byte> unpackHuffman(const byte *huff, uint huffSize, byte wordSize) {
	HuffChar table[514] = {};
	uint offset = 0;
	uint8 freq_first = huff[offset++];
	do {
		uint8 freq_last = huff[offset++];
		if (freq_first <= freq_last) {
			for (auto idx = freq_first; idx <= freq_last; ++idx) {
				table[idx].freq = huff[offset++];
			}
		}
		freq_first = huff[offset++];
	} while (freq_first != 0);
	if (wordSize > 1 && (offset % wordSize) != 0) {
		offset += wordSize - (offset % wordSize);
	}
	table[256].freq = 1;
	table[513].freq = 0x7FFF;

	int startEntry;
	short codeIdx = 257;
	while (true) {
		short idx = 0;
		short smallest2 = 513, smallest1 = 513;
		while (idx < codeIdx) {
			auto freq = table[idx].freq;
			if (freq != 0) {
				if (freq >= table[smallest1].freq) {
					if (freq < table[smallest2].freq) {
						smallest2 = idx;
					}
				} else {
					smallest2 = smallest1;
					smallest1 = idx;
				}
			}
			++idx;
		}
		if (smallest2 == 513) {
			startEntry = codeIdx - 1;
			break;
		}
		table[codeIdx].freq = table[smallest1].freq + table[smallest2].freq;
		table[smallest1].freq = table[smallest2].freq = 0;
		table[codeIdx].falseIdx = smallest1;
		table[codeIdx].trueIdx = smallest2;
		++codeIdx;
	}
	assert(codeIdx < 513);
	Common::Array<byte> decoded;
	switch (wordSize) {
	case 1:
		decoded = unpackStream<byte>(huff, huffSize, offset, table, startEntry);
		break;
	case 4:
		decoded = unpackStream<uint32>(huff, huffSize, offset, table, startEntry);
		break;
	default:
		error("invalid word size");
	}
	debug("decoded %u bytes at %08x", decoded.size(), offset);
	if (wordSize == 1) {
		assert(offset == huffSize); // must decode to the end
	}
	return decoded;
}

} // namespace FourXM
} // namespace Video
