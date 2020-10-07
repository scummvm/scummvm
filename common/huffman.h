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

// Based on xoreos' Huffman code

#ifndef COMMON_HUFFMAN_H
#define COMMON_HUFFMAN_H

#include "common/array.h"
#include "common/list.h"
#include "common/types.h"

namespace Common {

/**
 * @defgroup common_huffmann Huffman bit stream decoding
 * @ingroup common
 *
 * @brief API for operations related to Huffman bit stream decoding.
 * 
 * @details Used in engines:
 *          - SCUMM
 *		
 * @{
 */

inline uint32 REVERSEBITS(uint32 x) {
	x = (((x & ~0x55555555) >> 1) | ((x & 0x55555555) << 1));
	x = (((x & ~0x33333333) >> 2) | ((x & 0x33333333) << 2));
	x = (((x & ~0x0F0F0F0F) >> 4) | ((x & 0x0F0F0F0F) << 4));
	x = (((x & ~0x00FF00FF) >> 8) | ((x & 0x00FF00FF) << 8));

	return((x >> 16) | (x << 16));
}

/**
 * Huffman bit stream decoding.
 *
 */
template<class BITSTREAM>
class Huffman {
public:
	/** Construct a Huffman decoder.
	 *
	 *  @param maxLength Maximal code length. If 0, it is searched for.
	 *  @param codeCount Number of codes.
	 *  @param codes     The actual codes.
	 *  @param lengths   Lengths of the individual codes.
	 *  @param symbols   The symbols. If 0, assume they are identical to the code indices.
	 */
	Huffman(uint8 maxLength, uint32 codeCount, const uint32 *codes, const uint8 *lengths, const uint32 *symbols = nullptr);

	/** Return the next symbol in the bit stream. */
	uint32 getSymbol(BITSTREAM &bits) const;

private:
	struct Symbol {
		uint32 code;
		uint32 symbol;

		Symbol(uint32 c, uint32 s) : code(c), symbol(s) {}
	};

	typedef List<Symbol> CodeList;
	typedef Array<CodeList> CodeLists;

	/** Lists of codes and their symbols, sorted by code length. */
	CodeLists _codes;

	/** Prefix lookup table used to speed up the decoding of short codes. */
	struct PrefixEntry {
		uint32 symbol;
		uint8  length;

		PrefixEntry() : length(0xFF) {}
	};

	static const uint8 _prefixTableBits = 8;
	PrefixEntry _prefixTable[1 << _prefixTableBits];
};

template <class BITSTREAM>
Huffman<BITSTREAM>::Huffman(uint8 maxLength, uint32 codeCount, const uint32 *codes, const uint8 *lengths, const uint32 *symbols) {
	assert(codeCount > 0);

	assert(codes);
	assert(lengths);

	if (maxLength == 0)
		for (uint32 i = 0; i < codeCount; i++)
			maxLength = MAX(maxLength, lengths[i]);

	assert(maxLength <= 32);

	// Codes that do not fit in the prefix table are stored in the _codes array.
	_codes.resize(MAX(maxLength - _prefixTableBits, 0));

	for (uint i = 0; i < codeCount; i++) {
		uint8 length = lengths[i];

		// The symbol. If none was specified, assume it is identical to the code index.
		uint32 symbol = symbols ? symbols[i] : i;

		if (length <= _prefixTableBits) {
			// Short codes go in the prefix lookup table. Set all the entries in the table
			// with an index starting with the code to the symbol value.
			uint32 startIndex;
			if (BITSTREAM::isMSB2LSB()) {
				startIndex = codes[i] << (_prefixTableBits - length);
			} else {
				startIndex = REVERSEBITS(codes[i]) >> (32 - _prefixTableBits);
			}

			uint32 endIndex = startIndex | ((1 << (_prefixTableBits - length)) - 1);

			for (uint32 j = startIndex; j <= endIndex; j++) {
				uint32 index = BITSTREAM::isMSB2LSB() ? j : REVERSEBITS(j) >> (32 - _prefixTableBits);
				_prefixTable[index].symbol = symbol;
				_prefixTable[index].length = length;
			}
		} else {
			// Put the code and symbol into the correct list for the length.
			_codes[lengths[i] - 1 - _prefixTableBits].push_back(Symbol(codes[i], symbol));
		}
	}
}

template <class BITSTREAM>
uint32 Huffman<BITSTREAM>::getSymbol(BITSTREAM &bits) const {
	uint32 code = bits.peekBits(_prefixTableBits);

	uint8 length = _prefixTable[code].length;

	if (length != 0xFF) {
		bits.skip(length);
		return _prefixTable[code].symbol;
	} else {
		bits.skip(_prefixTableBits);

		for (uint32 i = 0; i < _codes.size(); i++) {
			bits.addBit(code, i + _prefixTableBits);

			for (typename CodeList::const_iterator cCode = _codes[i].begin(); cCode != _codes[i].end(); ++cCode)
				if (code == cCode->code)
					return cCode->symbol;
		}
	}

	error("Unknown Huffman code");
	return 0;
}

/** @} */

} // End of namespace Common

#endif // COMMON_HUFFMAN_H
