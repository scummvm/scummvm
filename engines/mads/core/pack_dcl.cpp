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

#include "common/textconsole.h"
#include "mads/core/pack_dcl.h"

namespace MADS {

/*
 * Format notes (from blast.c):
 *
 * - First byte is 0 if literals are uncoded or 1 if they are coded. Second
 *   byte is 4, 5, or 6 for the number of extra bits in the distance code.
 *   This is the base-2 logarithm of the dictionary size minus six.
 *
 * - Compressed data is a combination of literals and length/distance pairs
 *   terminated by an end code. Literals are either Huffman coded or
 *   uncoded bytes. A length/distance pair is a coded length followed by a
 *   coded distance to represent a string that occurs earlier in the
 *   uncompressed data that occurs again at the current location.
 *
 * - A bit preceding a literal or length/distance pair indicates which comes
 *   next: 0 for literals, 1 for length/distance.
 *
 * - If literals are uncoded, the next eight bits are the literal, in normal
 *   bit order (no bit-reversal). Length and distance extra bits are also
 *   read in normal bit order.
 *
 * - Bits are packed from the least significant bit to the most significant
 *   bit of each byte.
 *
 * - Huffman codes are bit-reversed relative to a simple integer ordering of
 *   codes of the same length, so decode() inverts each bit as it reads it
 *   to recover a naturally-ordered code value for comparison.
 */

#define DCL_MAXBITS 13
#define DCL_MAXWIN  4096

struct DclHuffman {
	short count[DCL_MAXBITS + 1]; /* number of symbols of each length */
	short *symbol;                 /* canonically ordered symbols */
};

// Bit lengths of literal codes 0..255, compactly encoded: each byte's low
// nibble is a code length and high nibble is (repeat count - 1).
static const byte kLitLenCompact[] = {
	11, 124, 8, 7, 28, 7, 188, 13, 76, 4, 10, 8, 12, 10, 12, 10, 8, 23, 8,
	9, 7, 6, 7, 8, 7, 6, 55, 8, 23, 24, 12, 11, 7, 9, 11, 12, 6, 7, 22, 5,
	7, 24, 6, 11, 9, 6, 7, 22, 7, 11, 38, 7, 9, 8, 25, 11, 8, 11, 9, 12,
	8, 12, 5, 38, 5, 38, 5, 11, 7, 5, 6, 21, 6, 10, 53, 8, 7, 24, 10, 27,
	44, 253, 253, 253, 252, 252, 252, 13, 12, 45, 12, 45, 12, 61, 12, 45,
	44, 173
};

// Bit lengths of length codes 0..15 (same compact encoding).
static const byte kLenLenCompact[] = {2, 35, 36, 53, 38, 23};

// Bit lengths of distance codes 0..63 (same compact encoding).
static const byte kDistLenCompact[] = {2, 20, 53, 230, 247, 151, 248};

// Base value and extra-bit count for each of the 16 length codes.
static const short kLenBase[16] = {
	3, 2, 4, 5, 6, 7, 8, 9, 10, 12, 16, 24, 40, 72, 136, 264
};
static const byte kLenExtra[16] = {
	0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8
};

struct DclState {
	Common::SeekableReadStream *src;

	uint32 bitbuf;
	int bitcnt;
	bool inputError;

	// Sliding window: doubles as LZ77 history and the last DCL_MAXWIN
	// bytes of output. windowPos wraps modulo DCL_MAXWIN, which is
	// equivalent to blast.c's own block-recycling scheme since distances
	// never exceed the dictionary size (max 4096).
	byte window[DCL_MAXWIN];
	int windowPos;
	bool first; // true until a full window's worth of output has been produced

	byte *dest;
	long destSize;
	long destPos;
};

/**
 * Reads need bits from the input stream, least-significant-bit first.
 * Sets s.inputError and returns 0 if the stream runs out of data.
 */
static int dclBits(DclState &s, int need) {
	uint32 val = s.bitbuf;
	while (s.bitcnt < need) {
		byte b;
		if (s.src->eos() || s.src->read(&b, 1) != 1) {
			s.inputError = true;
			return 0;
		}
		val |= (uint32)b << s.bitcnt;
		s.bitcnt += 8;
	}
	s.bitbuf = val >> need;
	s.bitcnt -= need;
	return (int)(val & ((1u << need) - 1));
}

/**
 * Bit-reversed canonical Huffman decode; see the format notes above.
 */
static int dclDecode(DclState &s, const DclHuffman &h) {
	int code = 0, first = 0, index = 0, len = 1;
	const short *next = h.count + 1;

	for (;;) {
		int bit = dclBits(s, 1);
		if (s.inputError)
			return 0;

		code |= bit ^ 1; // invert to recover natural ordering

		int count = *next++;
		if (code < first + count)
			return h.symbol[index + (code - first)];

		index += count;
		first += count;
		first <<= 1;
		code <<= 1;
		len++;

		if (len > DCL_MAXBITS) {
			s.inputError = true;
			return 0;
		}
	}
}

/**
 * Expands a compact repeat-count/length byte list into a canonical Huffman
 * decode table (count[] and symbol[]).
 *
 * @param h         Huffman table to fill in (h.symbol must already point at
 *                  storage large enough for the expanded symbol count).
 * @param rep       Compact repeat-count/length bytes.
 * @param repLen    Number of bytes in rep.
 * @param scratch   Scratch buffer, must hold at least as many entries as the
 *                  expanded symbol count (256 is always sufficient).
 */
static void dclConstruct(DclHuffman &h, const byte *rep, int repLen, short *scratch) {
	int symbol = 0;
	for (int i = 0; i < repLen; ++i) {
		int len = rep[i] & 15;
		int left = (rep[i] >> 4) + 1;
		while (left-- > 0)
			scratch[symbol++] = len;
	}
	int n = symbol;

	for (int len = 0; len <= DCL_MAXBITS; ++len)
		h.count[len] = 0;
	for (int i = 0; i < n; ++i)
		h.count[scratch[i]]++;

	if (h.count[0] == n)
		return; // empty code; never referenced by a well-formed stream

	short offs[DCL_MAXBITS + 2];
	offs[1] = 0;
	for (int len = 1; len < DCL_MAXBITS; ++len)
		offs[len + 1] = offs[len] + h.count[len];

	for (int i = 0; i < n; ++i) {
		if (scratch[i] != 0)
			h.symbol[offs[scratch[i]]++] = i;
	}
}

bool pack_dcl_explode(Common::SeekableReadStream *src, byte *dest, long destSize) {
	DclState s;
	s.src = src;
	s.bitbuf = 0;
	s.bitcnt = 0;
	s.inputError = false;
	memset(s.window, 0, sizeof(s.window));
	s.windowPos = 0;
	s.first = true;
	s.dest = dest;
	s.destSize = destSize;
	s.destPos = 0;

	short litSym[256], lenSym[16], distSym[64];
	DclHuffman litCode, lenCode, distCode;
	litCode.symbol = litSym;
	lenCode.symbol = lenSym;
	distCode.symbol = distSym;

	short scratch[256];
	dclConstruct(litCode, kLitLenCompact, sizeof(kLitLenCompact), scratch);
	dclConstruct(lenCode, kLenLenCompact, sizeof(kLenLenCompact), scratch);
	dclConstruct(distCode, kDistLenCompact, sizeof(kDistLenCompact), scratch);

	int lit = dclBits(s, 8);
	if (s.inputError || lit > 1)
		return false;
	int dictBits = dclBits(s, 8);
	if (s.inputError || dictBits < 4 || dictBits > 6)
		return false;

	while (s.destPos < s.destSize) {
		int flag = dclBits(s, 1);
		if (s.inputError)
			return false;

		if (flag) {
			// Length/distance pair.
			int symbol = dclDecode(s, lenCode);
			if (s.inputError)
				return false;
			int len = kLenBase[symbol] + dclBits(s, kLenExtra[symbol]);
			if (s.inputError)
				return false;
			if (len == 519)
				break; // end code

			int distBits = (len == 2) ? 2 : dictBits;
			int distSymbol = dclDecode(s, distCode);
			if (s.inputError)
				return false;
			uint32 dist = ((uint32)distSymbol << distBits) + dclBits(s, distBits);
			if (s.inputError)
				return false;
			dist++;

			if (s.first && (long)dist > s.windowPos)
				return false; // distance points before the start of output

			while (len-- > 0) {
				byte b = s.window[(s.windowPos - (int)dist) & (DCL_MAXWIN - 1)];
				s.window[s.windowPos] = b;
				s.windowPos = (s.windowPos + 1) & (DCL_MAXWIN - 1);
				if (s.windowPos == 0)
					s.first = false;
				if (s.destPos < s.destSize)
					s.dest[s.destPos++] = b;
			}
		} else {
			// Literal.
			int symbol = lit ? dclDecode(s, litCode) : dclBits(s, 8);
			if (s.inputError)
				return false;

			byte b = (byte)symbol;
			s.window[s.windowPos] = b;
			s.windowPos = (s.windowPos + 1) & (DCL_MAXWIN - 1);
			if (s.windowPos == 0)
				s.first = false;
			if (s.destPos < s.destSize)
				s.dest[s.destPos++] = b;
		}
	}

	return s.destPos == s.destSize;
}

} // namespace MADS
