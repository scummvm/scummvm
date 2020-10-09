/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "common/endian.h"

namespace Grim {

static int16 imcTable1[] = {
	  7,     8,     9,    10,    11,    12,    13,    14,    16,    17,
	 19,    21,    23,    25,    28,    31,    34,    37,    41,    45,
	 50,    55,    60,    66,    73,    80,    88,    97,   107,   118,
	130,   143,   157,   173,   190,   209,   230,   253,   279,   307,
	337,   371,   408,   449,   494,   544,   598,   658,   724,   796,
	876,   963,  1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,
	2272,  2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,
	5894,  6484,  7132,  7845,  8630,  9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static int8 imcTable2[] = {
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

static int8 imcOtherTable1[] = {
	-1, 4, -1, 4
};

static int8 imcOtherTable2[] = {
	-1, -1, 2, 6, -1, -1, 2, 6
};

static int8 imcOtherTable3[] = {
	-1, -1, -1, -1, 1, 2, 4, 6,
	-1, -1, -1, -1, 1, 2, 4, 6
};

static int8 imcOtherTable4[] = {
	-1, -1, -1, -1, -1, -1, -1, -1,
	1, 1, 1, 2, 2, 4, 5, 6,
	-1, -1, -1, -1, -1, -1, -1, -1,
	1, 1, 1, 2, 2, 4, 5, 6
};

static int8 imcOtherTable5[] = {
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	 1, 1, 1, 1, 1, 2, 2, 2,
	 2, 4, 4, 4, 5, 5, 6, 6,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	 1, 1, 1, 1, 1, 2, 2, 2,
	 2, 4, 4, 4, 5, 5, 6, 6
};

static int8 imcOtherTable6[] = {
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	 1, 1, 1, 1, 1, 1, 1, 1,
	 1, 1, 2, 2, 2, 2, 2, 2,
	 2, 2, 4, 4, 4, 4, 4, 4,
	 5, 5, 5, 5, 6, 6, 6, 6,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	 1, 1, 1, 1, 1, 1, 1, 1,
	 1, 1, 2, 2, 2, 2, 2, 2,
	 2, 2, 4, 4, 4, 4, 4, 4,
	 5, 5, 5, 5, 6, 6, 6, 6
};

static int8 *offsets[] = {
	imcOtherTable1, imcOtherTable2, imcOtherTable3,
	imcOtherTable4, imcOtherTable5, imcOtherTable6
};

void vimaInit(uint16 *destTable) {
	int destTableStartPos, incer;

	for (destTableStartPos = 0, incer = 0; destTableStartPos < 64; destTableStartPos++, incer++) {
		unsigned int destTablePos, imcTable1Pos;
		for (imcTable1Pos = 0, destTablePos = destTableStartPos;
				imcTable1Pos < sizeof(imcTable1) / sizeof(imcTable1[0]); imcTable1Pos++, destTablePos += 64) {
			int put = 0, count, tableValue;
			for (count = 32, tableValue = imcTable1[imcTable1Pos]; count != 0; count >>= 1, tableValue >>= 1) {
				if (incer & count) {
					put += tableValue;
				}
			}
			destTable[destTablePos] = put;
		}
	}
}

void decompressVima(const byte *src, int16 *dest, int destLen, uint16 *destTable) {
	int numChannels = 1;
	byte sBytes[2];
	int16 sWords[2];

	sBytes[0] = *src++;
	if (sBytes[0] & 0x80) {
		sBytes[0] = ~sBytes[0];
		numChannels = 2;
	}
	sWords[0] = READ_BE_UINT16(src);
	src += 2;
	if (numChannels > 1) {
		sBytes[1] = *src++;
		sWords[1] = READ_BE_UINT16(src);
		src += 2;
	}

	int numSamples = destLen / (numChannels * 2);
	int bits = READ_BE_UINT16(src);
	int bitPtr = 0;
	src += 2;

	for (int channel = 0; channel < numChannels; channel++) {
		int16 *destPos = dest + channel;
		int currTablePos = sBytes[channel];
		int outputWord = sWords[channel];

		for (int sample = 0; sample < numSamples; sample++) {
			int numBits = imcTable2[currTablePos];
			bitPtr += numBits;
			int highBit = 1 << (numBits - 1);
			int lowBits = highBit - 1;
			int val = (bits >> (16 - bitPtr)) & (highBit | lowBits);

			if (bitPtr > 7) {
				bits = ((bits & 0xff) << 8) | *src++;
				bitPtr -= 8;
			}

			if (val & highBit)
				val ^= highBit;
			else
				highBit = 0;

			if (val == lowBits) {
				outputWord = ((int16)(bits << bitPtr) & 0xffffff00);
				bits = ((bits & 0xff) << 8) | *src++;
				outputWord |= ((bits >> (8 - bitPtr)) & 0xff);
				bits = ((bits & 0xff) << 8) | *src++;
			} else {
				int index = (val << (7 - numBits)) | (currTablePos << 6);
				int delta = destTable[index];

				if (val)
					delta += (imcTable1[currTablePos] >> (numBits - 1));
				if (highBit)
					delta = -delta;

				outputWord += delta;
				if (outputWord < -0x8000)
					outputWord = -0x8000;
				else if (outputWord > 0x7fff)
					outputWord = 0x7fff;
			}

			WRITE_BE_UINT16(destPos, outputWord);
			destPos += numChannels;

			currTablePos += offsets[numBits - 2][val];

			if (currTablePos < 0)
				currTablePos = 0;
			else if (currTablePos > 88)
				currTablePos = 88;
		}
	}
}

} // end of namespace Grim
