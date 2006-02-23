/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "cine/cine.h"

uint32 crc;			// variable at 5C5A
uint32 bitbucket;			// dx:bx

uint16 swap16(uint16 r) {
	return (r >> 8) | (r << 8);
}

#define loadd(p, d) {\
    d = *(--p);\
    d |= (*(--p)) << 8;\
    d |= (*(--p)) << 16;\
    d |= (*(--p)) << 24;\
}
#define store(p, b) *(--p) = b
#define getbit(p, b) {\
    b = bitbucket & 1;\
    bitbucket >>= 1;\
    if (!bitbucket) {\
        loadd(p, bitbucket);\
	crc ^= bitbucket;\
	b = bitbucket & 1;\
	bitbucket >>= 1;\
	bitbucket |= 0x80000000;\
    }\
}

#define loadbits(p, b) {\
    b = 0;\
    do {\
	getbit(p, bit);\
	b <<= 1;\
	b |= bit;\
	nbits--;\
    } while (nbits);\
}

int decomp(uint8 *in, uint8 *out, int size) {
	uint8 bit;			// Carry flag
	uint8 nbits;		// cl
	uint8 byte = 0;		// ch
	uint16 counter;		// bp
	uint16 var = 0;		// variable at 5C58
	uint16 ptr;
	uint16 flags;

	enum {
		DO_COPY,
		DO_UNPACK
	} action;

	loadd(in, crc);
	loadd(in, bitbucket);
	crc ^= bitbucket;

	do {			// 5A4C
		getbit(in, bit);
		if (!bit) {	// 5A94
			getbit(in, bit);
			if (!bit) {	// 5AC8
				nbits = 3;
				byte = 0;
				action = DO_COPY;
			} else {	// 5ACA
				var = 1;
				nbits = 8;
				action = DO_UNPACK;
			}
		} else {	// 5B4F
			nbits = 2;
			loadbits(in, flags);
			if (flags < 2) {
				nbits = flags + 9;	// 5BC3
				var = flags + 2;
				action = DO_UNPACK;
			} else if (flags == 3) {
				nbits = 8;	// 5B4A
				byte = 8;
				action = DO_COPY;
			} else {
				nbits = 8;
				loadbits(in, var);
				nbits = 12;
				action = DO_UNPACK;
			}
		}

		switch (action) {
		case DO_COPY:
			{
				// 5AD1
				loadbits(in, counter);	// 5AFD
				counter += byte;
				counter++;
				size -= counter;
				do {
					nbits = 8;
					loadbits(in, byte);	// 5B3F
					store(out, byte);
					counter--;
				} while (counter);	// 5B45
				break;
			}

		case DO_UNPACK:

			// 5BD3
			loadbits(in, ptr);	// 5BFF
			counter = var + 1;
			size -= counter;
			do {
				byte = *(out + ptr - 1);
				store(out, byte);
				counter--;
			} while (counter);
		}
	} while (size > 0);

	// 5C32
	// ???
	if (crc) {
		return -1;
	} else {
		return 0;
	}
}
