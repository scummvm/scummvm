/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
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

/***************************************************************************
** decomp.c
**
** Routines that deal with AGI version 3 specific features.
** The original LZW code is from DJJ, October 1989, p.86.
** It has been modified to handle AGI compression.
**
** (c) 1997  Lance Ewing
***************************************************************************/

#include "common/stdafx.h"

#include "agi/agi.h"
#include "agi/lzw.h"

namespace Agi {

#define MAXBITS		12
#define TABLE_SIZE	18041	/* strange number */
#define START_BITS	9

static int32 BITS, MAX_VALUE, MAX_CODE;
static uint32 *prefix_code;
static uint8 *append_character;
static uint8 *decode_stack;
static int32 input_bit_count = 0;	/* Number of bits in input bit buffer */
static uint32 input_bit_buffer = 0L;

static void initLZW() {
	decode_stack = (uint8 *)calloc(1, 8192);
	prefix_code = (uint32 *)malloc(TABLE_SIZE * sizeof(uint32));
	append_character = (uint8 *)malloc(TABLE_SIZE * sizeof(uint8));
	input_bit_count = 0;	/* Number of bits in input bit buffer */
	input_bit_buffer = 0L;
}

static void closeLZW() {
	free(decode_stack);
	free(prefix_code);
	free(append_character);
}

/***************************************************************************
** setBITS
**
** Purpose: To adjust the number of bits used to store codes to the value
** passed in.
***************************************************************************/
int setBITS(int32 value) {
	if (value == MAXBITS)
		return true;

	BITS = value;
	MAX_VALUE = (1 << BITS) - 1;
	MAX_CODE = MAX_VALUE - 1;

	return false;
}

/***************************************************************************
** decode_string
**
** Purpose: To return the string that the code taken from the input buffer
** represents. The string is returned as a stack, i.e. the characters are
** in reverse order.
***************************************************************************/
static uint8 *decode_string(uint8 *buffer, uint32 code) {
	uint32 i;

	for (i = 0; code > 255;) {
		*buffer++ = append_character[code];
		code = prefix_code[code];
		if (i++ >= 4000) {
			fprintf(stderr, "lzw: error in code expansion.\n");
			abort();
		}
	}
	*buffer = code;

	return buffer;
}

/***************************************************************************
** input_code
**
** Purpose: To return the next code from the input buffer.
***************************************************************************/
static uint32 input_code(uint8 **input) {
	uint32 r;

	while (input_bit_count <= 24) {
		input_bit_buffer |= (uint32) * (*input)++ << input_bit_count;
		input_bit_count += 8;
	}
	r = (input_bit_buffer & 0x7FFF) % (1 << BITS);
	input_bit_buffer >>= BITS;
	input_bit_count -= BITS;

	return r;
}

/***************************************************************************
** expand
**
** Purpose: To uncompress the data contained in the input buffer and store
** the result in the output buffer. The fileLength parameter says how
** many bytes to uncompress. The compression itself is a form of LZW that
** adjusts the number of bits that it represents its codes in as it fills
** up the available codes. Two codes have special meaning:
**
**  code 256 = start over
**  code 257 = end of data
***************************************************************************/
void LZW_expand(uint8 *in, uint8 *out, int32 len) {
	int32 c, lzwnext, lzwnew, lzwold;
	uint8 *s, *end;

	initLZW();

	setBITS(START_BITS);	/* Starts at 9-bits */
	lzwnext = 257;		/* Next available code to define */

	end = (unsigned char *)((long)out + (long)len);

	lzwold = input_code(&in);	/* Read in the first code */
	c = lzwold;
	lzwnew = input_code(&in);

	while ((out < end) && (lzwnew != 0x101)) {
		if (lzwnew == 0x100) {
			/* Code to "start over" */
			lzwnext = 258;
			setBITS(START_BITS);
			lzwold = input_code(&in);
			c = lzwold;
			*out++ = (char)c;
			lzwnew = input_code(&in);
		} else {
			if (lzwnew >= lzwnext) {
				/* Handles special LZW scenario */
				*decode_stack = c;
				s = decode_string(decode_stack + 1, lzwold);
			} else
				s = decode_string(decode_stack, lzwnew);

			/* Reverse order of decoded string and
			 * store in out buffer
			 */
			c = *s;
			while (s >= decode_stack)
				*out++ = *s--;

			if (lzwnext > MAX_CODE)
				setBITS(BITS + 1);

			prefix_code[lzwnext] = lzwold;
			append_character[lzwnext] = c;
			lzwnext++;
			lzwold = lzwnew;

			lzwnew = input_code(&in);
		}
	}
	closeLZW();
}

}                             // End of namespace Agi
