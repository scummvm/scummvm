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

/* VLC code
 *
 * Original copyright note: * Intel Indeo 4 (IV41, IV42, etc.) video decoder for ffmpeg
 * written, produced, and directed by Alan Smithee
 */

#include "image/codecs/indeo/vlc.h"
#include "image/codecs/indeo/mem.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Image {
namespace Indeo {

/**
 * Quicksort
 * This sort is fast, and fully inplace but not stable and it is possible
 * to construct input that requires O(n^2) time but this is very unlikely to
 * happen with non constructed input.
 */
#define AV_QSORT(p, num, type, cmp) do {\
	void *stack[64][2];\
	int sp = 1;\
	stack[0][0] = p;\
	stack[0][1] = (p)+(num)-1;\
	while(sp){\
		type *start = (type *)stack[--sp][0];\
		type *end   = (type *)stack[  sp][1];\
		while (start < end) {\
			if (start < end-1) {\
				int checksort = 0;\
				type *right = end - 2;\
				type *left  = start + 1;\
				type *mid = start + ((end - start) >> 1);\
				if(cmp(start, end) > 0) {\
					if(cmp(  end, mid) > 0) SWAP(*start, *mid);\
					else                    SWAP(*start, *end);\
				} else {\
					if(cmp(start, mid) > 0) SWAP(*start, *mid);\
					else checksort = 1;\
				}\
				if (cmp(mid, end) > 0) { \
					SWAP(*mid, *end);\
					checksort = 0;\
				}\
				if(start == end - 2) break;\
				SWAP(end[-1], *mid);\
				while (left <= right) {\
					while (left<=right && cmp(left, end - 1) < 0)\
						left++;\
					while (left<=right && cmp(right, end - 1) > 0)\
						right--;\
					if (left <= right) {\
						SWAP(*left, *right);\
						left++;\
						right--;\
					}\
				}\
				SWAP(end[-1], *left);\
				if(checksort && (mid == left - 1 || mid == left)){\
					mid= start;\
					while(mid<end && cmp(mid, mid+1) <= 0)\
						mid++;\
					if(mid==end)\
						break;\
				}\
				if (end - left < left - start){\
					stack[sp  ][0] = start;\
					stack[sp++][1] = right;\
					start = left + 1;\
				} else {\
					stack[sp  ][0] = left+1;\
					stack[sp++][1] = end;\
					end = right;\
				}\
			} else {\
				if (cmp(start, end) > 0)\
					SWAP(*start, *end);\
				break;\
			}\
		}\
	}\
} while (0)

#define COPY(condition)\
	for (i = 0; i < nbCodes; i++) {                                         \
		buf[j].bits = getData(p_bits, i, bitsWrap, bitsSize);               \
		if (!(condition))                                                   \
			continue;                                                       \
		if (buf[j].bits > (3 * nbBits) || buf[j].bits > 32) {               \
			warning("Too long VLC (%d) in init_vlc", buf[j].bits);          \
			if (!(flags & INIT_VLC_USE_NEW_STATIC))                         \
				free(buf);                                                  \
			return -1;                                                      \
		}                                                                   \
		buf[j].code = getData(codes, i, codesWrap, codesSize);              \
		if (buf[j].code >= (1LL << buf[j].bits)) {                            \
			warning("Invalid code %x for %d in init_vlc", buf[j].code, i);  \
			if (!(flags & INIT_VLC_USE_NEW_STATIC))                         \
				free(buf);                                                  \
			return -1;                                                      \
		}                                                                   \
		if (flags & INIT_VLC_LE)                                            \
			buf[j].code = bitswap32(buf[j].code);                           \
		else                                                                \
			buf[j].code <<= 32 - buf[j].bits;                               \
		if (symbols)                                                        \
			buf[j].symbol = getData(symbols, i, symbolsWrap, symbolsSize);  \
		else                                                                \
			buf[j].symbol = i;                                              \
		j++;                                                                \
	}

/*------------------------------------------------------------------------*/

VLC::VLC() : _bits(0), _tableSize(0), _tableAllocated(0), _table(nullptr) {
}

int VLC::init_vlc(int nbBits, int nbCodes, const void *bits, int bitsWrap, int bitsSize,
		const void *codes, int codesWrap, int codesSize, int flags) {
	return init_vlc(nbBits, nbCodes, bits, bitsWrap, bitsSize, codes, codesWrap,
		codesSize, nullptr, 0, 0, flags);
}

int VLC::init_vlc(int nbBits, int nbCodes, const void *p_bits, int bitsWrap,
		int bitsSize, const void *codes, int codesWrap, int codesSize,
		const void *symbols, int symbolsWrap, int symbolsSize, int flags) {
	VLCcode *buf;
	int i, j, ret;
	VLCcode localbuf[1500]; // the maximum currently needed is 1296 by rv34
	VLC localvlc, *vlc;

	vlc = this;
	vlc->_bits = nbBits;
	if (flags & INIT_VLC_USE_NEW_STATIC) {
		assert((nbCodes + 1) <= (int)FF_ARRAY_ELEMS(localbuf));
		buf = localbuf;
		localvlc = *this;
		vlc = &localvlc;
		vlc->_tableSize = 0;
	} else {
		vlc->_table = NULL;
		vlc->_tableAllocated = 0;
		vlc->_tableSize = 0;

		buf = (VLCcode *)malloc((nbCodes + 1) * sizeof(VLCcode));
		assert(buf);
	}

	assert(symbolsSize <= 2 || !symbols);
	j = 0;

	COPY(buf[j].bits > nbBits);

	// qsort is the slowest part of init_vlc, and could probably be improved or avoided
	AV_QSORT(buf, j, VLCcode, compareVlcSpec);
	COPY(buf[j].bits && buf[j].bits <= nbBits);
	nbCodes = j;

	ret = vlc->buildTable(nbBits, nbCodes, buf, flags);

	if (flags & INIT_VLC_USE_NEW_STATIC) {
		if (vlc->_tableSize != vlc->_tableAllocated)
			warning("needed %d had %d", vlc->_tableSize, vlc->_tableAllocated);

		assert(ret >= 0);
		*this = *vlc;
	} else {
		free(buf);
		if (ret < 0) {
			avFreeP(&vlc->_table);
			return -1;
		}
	}

	return 0;
}

void VLC::freeVlc() {
	free(_table);
}

int VLC::compareVlcSpec(const void *a, const void *b) {
	const VLCcode *sa = (const VLCcode *)a, *sb = (const VLCcode *)b;
	return (sa->code >> 1) - (sb->code >> 1);
}

int VLC::buildTable(int tableNbBits, int nbCodes,
		VLCcode *codes, int flags) {
	VLC *vlc = this;
	int tableSize, tableIndex, index, codePrefix, symbol, subtableBits;
	int i, j, k, n, nb, inc;
	uint32 code;
	VLC_TYPE (*table)[2];

	tableSize = 1 << tableNbBits;
	if (tableNbBits > 30)
		return -1;
	tableIndex = allocTable(tableSize, flags & INIT_VLC_USE_NEW_STATIC);
	//warning("new table index=%d size=%d", tableIndex, tableSize);
	if (tableIndex < 0)
		return tableIndex;
	table = &vlc->_table[tableIndex];

	// first pass: map codes and compute auxiliary table sizes
	for (i = 0; i < nbCodes; i++) {
		n = codes[i].bits;
		code = codes[i].code;
		symbol = codes[i].symbol;
		//warning("i=%d n=%d code=0x%x", i, n, code);

		if (n <= tableNbBits) {
			// no need to add another table
			j = code >> (32 - tableNbBits);
			nb = 1 << (tableNbBits - n);
			inc = 1;
			if (flags & INIT_VLC_LE) {
				j = bitswap32(code);
				inc = 1 << n;
			}
			for (k = 0; k < nb; k++) {
				int bits = table[j][1];
				//warning("%4x: code=%d n=%d", j, i, n);

				if (bits != 0 && bits != n) {
					warning("incorrect codes");
					return -1;
				}

				table[j][1] = n; //bits
				table[j][0] = symbol;
				j += inc;
			}
		} else {
			// fill auxiliary table recursively
			n -= tableNbBits;
			codePrefix = code >> (32 - tableNbBits);
			subtableBits = n;
			codes[i].bits = n;
			codes[i].code = code << tableNbBits;
			for (k = i + 1; k < nbCodes; k++) {
				n = codes[k].bits - tableNbBits;
				if (n <= 0)
					break;
				code = codes[k].code;
				if (code >> (32 - tableNbBits) != (uint)codePrefix)
					break;
				codes[k].bits = n;
				codes[k].code = code << tableNbBits;
				subtableBits = MAX(subtableBits, n);
			}
			subtableBits = MIN(subtableBits, tableNbBits);
			j = (flags & INIT_VLC_LE) ? bitswap32(codePrefix) >> (32 - tableNbBits) : codePrefix;
			table[j][1] = -subtableBits;
			//warning("%4x: n=%d (subtable)", j, codes[i].bits + tableNbBits);
			index = vlc->buildTable(subtableBits, k - i, codes + i, flags);
			if (index < 0)
				return index;

			// note: realloc has been done, so reload tables
			table = (VLC_TYPE (*)[2])&vlc->_table[tableIndex];
			table[j][0] = index; //code
			i = k - 1;
		}
	}

	for (i = 0; i < tableSize; i++) {
		if (table[i][1] == 0) //bits
			table[i][0] = -1; //codes
	}

	return tableIndex;
}

int VLC::allocTable(int size, int useStatic) {
	VLC *vlc = this;
	int index = vlc->_tableSize;

	vlc->_tableSize += size;
	if (vlc->_tableSize > vlc->_tableAllocated) {
		// cannot do anything, init_vlc() is used with too little memory
		assert(!useStatic);

		vlc->_tableAllocated += (1 << vlc->_bits);
		vlc->_table = (int16(*)[2])avReallocF(vlc->_table, vlc->_tableAllocated, sizeof(VLC_TYPE) * 2);
		if (!vlc->_table) {
			vlc->_tableAllocated = 0;
			vlc->_tableSize = 0;
			return -2;
		}

		memset(vlc->_table + vlc->_tableAllocated - (1 << vlc->_bits), 0, sizeof(VLC_TYPE) * 2 << vlc->_bits);
	}

	return index;
}

uint VLC::getData(const void *table, uint idx, uint wrap, uint size) {
	const uint8 *ptr = (const uint8 *)table + idx * wrap;

	switch(size) {
	case 1:
		return *(const uint8 *)ptr;

	case 2:
		return *(const uint16 *)ptr;

	default:
		return *(const uint32 *)ptr;
	}
}

} // End of namespace Indeo
} // End of namespace Image
