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

#include "glk/alan3/decode.h"
#include "glk/alan3/acode.h"
#include "glk/alan3/syserr.h"
#include "glk/alan3/exe.h"
#include "glk/alan3/memory.h"

namespace Glk {
namespace Alan3 {

/* PUBLIC DATA */
Aword *freq;            /* Cumulative character frequencies */


/* PRIVATE DATA */
/* Bit output */
static int decodeBuffer;    /* Bits to be input */
static int bitsToGo;        /* Bits still in buffer */
static int garbageBits;     /* Bits past EOD */


static int inputBit(void) {
	int bit;

	/* More bits available ? */
	if (!bitsToGo) {
		/* No, so get more */
		decodeBuffer = (textFile->pos() >= textFile->size()) ? EOD : textFile->readByte();
		if (decodeBuffer == (int)EOD) {
			garbageBits++;
			if (garbageBits > VALUEBITS - 2)
				syserr("Error in encoded data file.");
		} else
			bitsToGo = 8;     /* Another Char, 8 new bits */
	}
	bit = decodeBuffer & 1;   /* Get next bit */
	decodeBuffer = decodeBuffer >> 1; /* and remove it */
	bitsToGo--;
	return bit;
}


/* Current state of decoding */

static CodeValue value;         /* Currently seen code value */
static CodeValue low, high;     /* Current code region */


void startDecoding(void) {
	int i;

	bitsToGo = 0;
	garbageBits = 0;

	value = 0;
	for (i = 0; i < VALUEBITS; i++)
		value = 2 * value + inputBit();
	low = 0;
	high = TOPVALUE;
}


int decodeChar(void) {
	long range;
	int f;
	int symbol;

	range = (long)(high - low) + 1;
	f = (((long)(value - low) + 1) * freq[0] - 1) / range;

	/* Find the symbol */
	for (symbol = 1; (int)freq[symbol] > f; ++symbol) {}

	high = low + range * freq[symbol - 1] / freq[0] - 1;
	low = low + range * freq[symbol] / freq[0];

	for (;;) {
		if (high < HALF)
			;
		else if (low >= HALF) {
			value = value - HALF;
			low = low - HALF;
			high = high - HALF;
		} else if (low >= ONEQUARTER && high < THREEQUARTER) {
			value = value - ONEQUARTER;
			low = low - ONEQUARTER;
			high = high - ONEQUARTER;
		} else
			break;

		/* Scale up the range */
		low = 2 * low;
		high = 2 * high + 1;
		value = 2 * value + inputBit();
	}
	return symbol - 1;
}



/* Structure for saved decode info */
struct DecodeInfo {
	long fpos;
	int buffer;
	int bits;
	CodeValue value;
	CodeValue high;
	CodeValue low;
};


/*======================================================================

  pushDecode()

  Save so much info about the decoding process so it is possible to
  restore and continue later.

 */
void *pushDecode(void) {
	DecodeInfo *info;

	info = (DecodeInfo *)allocate(sizeof(DecodeInfo));
	info->fpos = textFile->pos();
	info->buffer = decodeBuffer;
	info->bits = bitsToGo;
	info->value = value;
	info->high = high;
	info->low = low;
	return (info);
}


/*======================================================================

  popDecode()

  Restore enough info about the decoding process so it is possible to
  continue after having decoded something else.

 */
void popDecode(void *i) {
	DecodeInfo *info = (DecodeInfo *) i;

	textFile->seek(info->fpos);
	decodeBuffer = info->buffer;
	bitsToGo = info->bits;
	value = info->value;
	high = info->high;
	low = info->low;

	free(info);
}

} // End of namespace Alan3
} // End of namespace Glk
