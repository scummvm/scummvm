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
    int sp= 1;\
    stack[0][0] = p;\
    stack[0][1] = (p)+(num)-1;\
    while(sp){\
        type *start= (type *)stack[--sp][0];\
        type *end  = (type *)stack[  sp][1];\
        while(start < end){\
            if(start < end-1) {\
                int checksort=0;\
                type *right = end-2;\
                type *left  = start+1;\
                type *mid = start + ((end-start)>>1);\
                if(cmp(start, end) > 0) {\
                    if(cmp(  end, mid) > 0) FFSWAP(type, *start, *mid);\
                    else                    FFSWAP(type, *start, *end);\
                }else{\
                    if(cmp(start, mid) > 0) FFSWAP(type, *start, *mid);\
                    else checksort= 1;\
                }\
                if(cmp(mid, end) > 0){ \
                    FFSWAP(type, *mid, *end);\
                    checksort=0;\
                }\
                if(start == end-2) break;\
                FFSWAP(type, end[-1], *mid);\
                while(left <= right){\
                    while(left<=right && cmp(left, end-1) < 0)\
                        left++;\
                    while(left<=right && cmp(right, end-1) > 0)\
                        right--;\
                    if(left <= right){\
                        FFSWAP(type, *left, *right);\
                        left++;\
                        right--;\
                    }\
                }\
                FFSWAP(type, end[-1], *left);\
                if(checksort && (mid == left-1 || mid == left)){\
                    mid= start;\
                    while(mid<end && cmp(mid, mid+1) <= 0)\
                        mid++;\
                    if(mid==end)\
                        break;\
                }\
                if(end-left < left-start){\
                    stack[sp  ][0]= start;\
                    stack[sp++][1]= right;\
                    start = left+1;\
                }else{\
                    stack[sp  ][0]= left+1;\
                    stack[sp++][1]= end;\
                    end = right;\
                }\
            }else{\
                if(cmp(start, end) > 0)\
                    FFSWAP(type, *start, *end);\
                break;\
            }\
        }\
    }\
} while (0)

#define COPY(condition)\
    for (i = 0; i < nb_codes; i++) {                                        \
        buf[j].bits = getData(p_bits, i, bits_wrap, bits_size);             \
        if (!(condition))                                                   \
            continue;                                                       \
        if (buf[j].bits > 3*nb_bits || buf[j].bits>32) {                    \
            warning("Too long VLC (%d) in init_vlc", buf[j].bits);          \
            if (!(flags & INIT_VLC_USE_NEW_STATIC))                         \
                free(buf);                                                  \
            return -1;                                                      \
        }                                                                   \
        buf[j].code = getData(codes, i, codes_wrap, codes_size);            \
        if (buf[j].code >= (1LL<<buf[j].bits)) {                            \
            warning("Invalid code %x for %d in init_vlc", buf[j].code, i);  \
            if (!(flags & INIT_VLC_USE_NEW_STATIC))                         \
                free(buf);                                                  \
            return -1;                                                      \
        }                                                                   \
        if (flags & INIT_VLC_LE)                                            \
            buf[j].code = bitswap_32(buf[j].code);                          \
        else                                                                \
            buf[j].code <<= 32 - buf[j].bits;                               \
        if (symbols)                                                        \
            buf[j].symbol = getData(symbols, i, symbols_wrap, symbols_size); \
        else                                                                \
            buf[j].symbol = i;                                              \
        j++;                                                                \
    }

/*------------------------------------------------------------------------*/

VLC::VLC() : _bits(0), _table_size(0), _table_allocated(0), _table(nullptr) {
}

int VLC::init_vlc(int nb_bits, int nb_codes, const void *bits, int bits_wrap, int bits_size,
		const void *codes, int codes_wrap, int codes_size, int flags) {
	return init_vlc(nb_bits, nb_codes, bits, bits_wrap, bits_size, codes, codes_wrap,
		codes_size, nullptr, 0, 0, flags);
}

int VLC::init_vlc(int nb_bits, int nb_codes, const void *p_bits, int bits_wrap,
		int bits_size, const void *codes, int codes_wrap, int codes_size,
		const void *symbols, int symbols_wrap, int symbols_size, int flags) {
	VLCcode *buf;
	int i, j, ret;
	VLCcode localbuf[1500]; // the maximum currently needed is 1296 by rv34
	VLC localvlc, *vlc;

	vlc = this;
	vlc->_bits = nb_bits;
	if (flags & INIT_VLC_USE_NEW_STATIC) {
		assert((nb_codes + 1) <= FF_ARRAY_ELEMS(localbuf));
		buf = localbuf;
		localvlc = *this;
		vlc = &localvlc;
		vlc->_table_size = 0;
	} else {
		vlc->_table = NULL;
		vlc->_table_allocated = 0;
		vlc->_table_size = 0;

		buf = (VLCcode *)av_malloc_array((nb_codes + 1), sizeof(VLCcode));
		assert(buf);
	}

	assert(symbols_size <= 2 || !symbols);
	j = 0;

	COPY(buf[j].bits > nb_bits);

	// qsort is the slowest part of init_vlc, and could probably be improved or avoided
	AV_QSORT(buf, j, VLCcode, compare_vlcspec);
	COPY(buf[j].bits && buf[j].bits <= nb_bits);
	nb_codes = j;

	ret = vlc->build_table(nb_bits, nb_codes, buf, flags);

	if (flags & INIT_VLC_USE_NEW_STATIC) {
		if (vlc->_table_size != vlc->_table_allocated)
			warning("needed %d had %d", vlc->_table_size, vlc->_table_allocated);

		assert(ret >= 0);
		*this = *vlc;
	} else {
		free(buf);
		if (ret < 0) {
			av_freep(&vlc->_table);
			return -1;
		}
	}

	return 0;
}

void VLC::ff_free_vlc() {
	free(_table);
}

int VLC::compare_vlcspec(const void *a, const void *b) {
	const VLCcode *sa = (VLCcode *)a, *sb = (VLCcode *)b;
	return (sa->code >> 1) - (sb->code >> 1);
}

int VLC::build_table(int table_nb_bits, int nb_codes,
		VLCcode *codes, int flags) {
	VLC *vlc = this;
	int table_size, table_index, index, code_prefix, symbol, subtable_bits;
	int i, j, k, n, nb, inc;
	uint32 code;
	VLC_TYPE (*table)[2];

	table_size = 1 << table_nb_bits;
	if (table_nb_bits > 30)
		return -1;
	table_index = alloc_table(table_size, flags & INIT_VLC_USE_NEW_STATIC);
	//warning("new table index=%d size=%d", table_index, table_size);
	if (table_index < 0)
		return table_index;
	table = &vlc->_table[table_index];

	// first pass: map codes and compute auxiliary table sizes
	for (i = 0; i < nb_codes; i++) {
		n = codes[i].bits;
		code = codes[i].code;
		symbol = codes[i].symbol;
		//warning("i=%d n=%d code=0x%x", i, n, code);

		if (n <= table_nb_bits) {
			// no need to add another table
			j = code >> (32 - table_nb_bits);
			nb = 1 << (table_nb_bits - n);
			inc = 1;
			if (flags & INIT_VLC_LE) {
				j = bitswap_32(code);
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
			n -= table_nb_bits;
			code_prefix = code >> (32 - table_nb_bits);
			subtable_bits = n;
			codes[i].bits = n;
			codes[i].code = code << table_nb_bits;
			for (k = i + 1; k < nb_codes; k++) {
				n = codes[k].bits - table_nb_bits;
				if (n <= 0)
					break;
				code = codes[k].code;
				if (code >> (32 - table_nb_bits) != (uint)code_prefix)
					break;
				codes[k].bits = n;
				codes[k].code = code << table_nb_bits;
				subtable_bits = MAX(subtable_bits, n);
			}
			subtable_bits = MIN(subtable_bits, table_nb_bits);
			j = (flags & INIT_VLC_LE) ? bitswap_32(code_prefix) >> (32 - table_nb_bits) : code_prefix;
			table[j][1] = -subtable_bits;
			//warning("%4x: n=%d (subtable)", j, codes[i].bits + table_nb_bits);
			index = vlc->build_table(subtable_bits, k - i, codes + i, flags);
			if (index < 0)
				return index;

			// note: realloc has been done, so reload tables
			table = (VLC_TYPE (*)[2])&vlc->_table[table_index];
			table[j][0] = index; //code
			i = k - 1;
		}
	}

	for (i = 0; i < table_size; i++) {
		if (table[i][1] == 0) //bits
			table[i][0] = -1; //codes
	}

	return table_index;
}

int VLC::alloc_table(int size, int use_static) {
	VLC *vlc = this;
	int index = vlc->_table_size;

	vlc->_table_size += size;
	if (vlc->_table_size > vlc->_table_allocated) {
		// cannot do anything, init_vlc() is used with too little memory
		assert(!use_static);

		vlc->_table_allocated += (1 << vlc->_bits);
		vlc->_table = (int16(*)[2])av_realloc_f(vlc->_table, vlc->_table_allocated, sizeof(VLC_TYPE) * 2);
		if (!vlc->_table) {
			vlc->_table_allocated = 0;
			vlc->_table_size = 0;
			return -2;
		}

		memset(vlc->_table + vlc->_table_allocated - (1 << vlc->_bits), 0, sizeof(VLC_TYPE) * 2 << vlc->_bits);
	}
	return index;
}

/**
* VLC decoding
*/
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
