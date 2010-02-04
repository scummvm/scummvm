/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include <stdint.h>
#include <cstdio>
#include <cstring>


typedef unsigned char byte;
typedef unsigned char uint8;
typedef signed char int8;
typedef signed short int16;
typedef unsigned short uint16;

uint32_t get_be_uint32(char *p) {
	unsigned char *pos = reinterpret_cast<unsigned char *>(p);
	return (pos[0] << 24) | (pos[1] << 16) | (pos[2] << 8) | pos[3];
}

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

static uint16 destTable[5786];
byte *sourceBuffer;

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

void decompressVima(byte *src, int16 *dest, int destLen, uint16 *destTable) {
	int numChannels = 1;
	byte sBytes[2];
	int16 sWords[2];

	sBytes[0] = *(uint8 *)(src++);
	if (sBytes[0] & 0x80) {
		sBytes[0] = ~sBytes[0];
		numChannels = 2;
	}
	sWords[0] = (src[0] << 8) | src[1];
	src += 2;
	if (numChannels > 1) {
		sBytes[1] = *(uint8 *)(src++);
		sWords[1] = (src[0] << 8) | src[1];
		src += 2;
	}

	int numSamples = destLen / (numChannels * 2);
	int bits = (src[0] << 8) | src[1];
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
				bits = ((bits & 0xff) << 8) | *(uint8 *)(src++);
				bitPtr -= 8;
			}

			if (val & highBit)
				val ^= highBit;
			else
				highBit = 0;

			if (val == lowBits) {
				outputWord = ((int16)(bits << bitPtr) & 0xffffff00);
				bits = ((bits & 0xff) << 8) | *(uint8 *)(src++);
				outputWord |= ((bits >> (8 - bitPtr)) & 0xff);
				bits = ((bits & 0xff) << 8) | *(uint8 *)(src++);
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

			byte *b = (byte *)destPos;
			b[0] = (byte)(outputWord >> 0);
			b[1] = (byte)(outputWord >> 8);
			destPos += numChannels;

			currTablePos += offsets[numBits - 2][val];

			if (currTablePos < 0)
				currTablePos = 0;
			else if (currTablePos > 88)
				currTablePos = 88;
		}
	}
}

int main(int /* argc */, char *argv[]) {
	vimaInit(destTable);

	FILE *f = fopen(argv[1], "rb");
	if (f == NULL) {
		perror(argv[1]);
		return 1;
	}

	char magic[4];
	fread(magic, 4, 1, f);
	if (memcmp(magic, "MCMP", 4) != 0) {
		fprintf(stderr, "Not a valid file\n");
		return 1;
	}
	uint16_t numBlocks = getc(f) << 8;
	numBlocks |= getc(f);
	char *blocks = new char[9 * numBlocks];
	fread(blocks, 9, numBlocks, f);

	uint16_t numCodecs = getc(f) << 8;
	numCodecs |= getc(f);
	numCodecs /= 5;
	char *codecs = new char[5 * numCodecs];
	fread(codecs, 5, numCodecs, f);

	for (int i = 0; i < numBlocks; i++) {
		int codec = blocks[9 * i];
		int uncompSize = get_be_uint32(blocks + 9 * i + 1);
		int compSize = get_be_uint32(blocks + 9 * i + 5);

		sourceBuffer = new byte[compSize];
		fread(sourceBuffer, 1, compSize, f);

		if (strcmp(codecs + 5 * codec, "NULL") == 0)
			fwrite(sourceBuffer, 1, uncompSize, stdout);
		else if (strcmp(codecs + 5 * codec, "VIMA") == 0) {
			char *buffer = new char[uncompSize];
			decompressVima(sourceBuffer, (int16 *)buffer, uncompSize, destTable);
			fwrite(buffer, 1, uncompSize, stdout);
			delete[] buffer;
		} else {
			fprintf(stderr, "Unrecognized codec %s\n", codecs + 5 * codec);
			return 1;
		}
		delete[] sourceBuffer;
	}

	delete[] blocks;
	delete[] codecs;
	fclose(f);
	return 0;
}
