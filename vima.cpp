// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "stdafx.h"
#include "bits.h"
#include "debug.h"

static uint16 imcTable1[] = {
  0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e,
  0x0010, 0x0011, 0x0013, 0x0015, 0x0017, 0x0019, 0x001c, 0x001f,
  0x0022, 0x0025, 0x0029, 0x002d, 0x0032, 0x0037, 0x003c, 0x0042,
  0x0049, 0x0050, 0x0058, 0x0061, 0x006b, 0x0076, 0x0082, 0x008f,
  0x009d, 0x00ad, 0x00be, 0x00d1, 0x00e6, 0x00fd, 0x0117, 0x0133,
  0x0151, 0x0173, 0x0198, 0x01c1, 0x01ee, 0x0220, 0x0256, 0x0292,
  0x02d4, 0x031c, 0x036c, 0x03c3, 0x0424, 0x048e, 0x0502, 0x0583,
  0x0610, 0x06ab, 0x0756, 0x0812, 0x08e0, 0x09c3, 0x0abd, 0x0bd0,
  0x0cff, 0x0e4c, 0x0fba, 0x114c, 0x1307, 0x14ee, 0x1706, 0x1954,
  0x1bdc, 0x1ea5, 0x21b6, 0x2515, 0x28ca, 0x2cdf, 0x315b, 0x364b,
  0x3bb9, 0x41b2, 0x4844, 0x4f7e, 0x5771, 0x602f, 0x69ce, 0x7462,
  0x7fff
};

static int8 imcTable2[] = {
  0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05,
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06,
  0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
  0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
  0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07
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

static uint16 destTable[5786];

void vimaInit() {
	int destTableStartPos, incer;
	for (destTableStartPos = 0, incer = 0; destTableStartPos < 64; destTableStartPos++, incer++) {
	unsigned int destTablePos, imcTable1Pos;
		for (imcTable1Pos = 0, destTablePos = destTableStartPos;
		imcTable1Pos < sizeof(imcTable1) / sizeof(imcTable1[0]);
		imcTable1Pos++, destTablePos += 64) {
			int put = 0, count, tableValue;
		for (count = 32, tableValue = imcTable1[imcTable1Pos]; count != 0;
			count >>= 1, tableValue >>= 1) {
		if ((incer & count) != 0)
			put += tableValue;
		}
		destTable[destTablePos] = put;
		}
	}
}

void decompressVima(const char *src, int16 *dest, int destLen) {
	int numChannels = 1;
	uint8 sBytes[2];
	int16 sWords[2];

	sBytes[0] = *(uint8*)(src++);
	if (sBytes[0] & 0x80) {
		sBytes[0] = ~sBytes[0];
		numChannels = 2;
	}
	sWords[0] = READ_BE_UINT16(src);
	src += 2;
	if (numChannels > 1) {
		sBytes[1] = *(uint8*)(src++);
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
				bits = ((bits & 0xff) << 8) | *(uint8*)(src++);
				bitPtr -= 8;
			}

			if (val & highBit)
				val ^= highBit;
			else
				highBit = 0;

			if (val == lowBits) {
				outputWord = ((signed short) (bits << bitPtr) & 0xffffff00);
				bits = ((bits & 0xff) << 8) | *(uint8*)(src++);
				outputWord |= ((bits >> (8 - bitPtr)) & 0xff);
				bits = ((bits & 0xff) << 8) | *(uint8*)(src++);
			} else {
				int index = (val << (7 - numBits)) | (currTablePos << 6);
				int delta = destTable[index];

				if (val != 0)
					delta += (imcTable1[currTablePos] >> (numBits - 1));
				if (highBit != 0)
					delta = -delta;

				outputWord += delta;
				if (outputWord < -0x8000)
					outputWord = -0x8000;
				else if (outputWord > 0x7fff)
					outputWord = 0x7fff;
			}

			*destPos = outputWord;
			destPos += numChannels;

			currTablePos += offsets[numBits - 2][val];

			if (currTablePos < 0)
				currTablePos = 0;
			else if (currTablePos > 88)
				currTablePos = 88;
		}
	}
}
