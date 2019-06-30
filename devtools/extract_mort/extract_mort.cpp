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
 * This is a utility for extracting needed resource data from different language
 * version of the Lure of the Temptress lure.exe executable files into a new file
 * lure.dat - this file is required for the ScummVM  Lure of the Temptress module
 * to work properly
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/endian.h"

enum AccessMode {
	kFileReadMode  = 1,
	kFileWriteMode = 2
};

class File {
private:
	FILE *f;
public:
	bool open(const char *filename, AccessMode mode = kFileReadMode) {
		f = fopen(filename, (mode == kFileReadMode) ? "rb" : "wb");
		return (f != NULL);
	}
	void close() {
		fclose(f);
		f = NULL;
	}
	int seek(int32 offset, int whence = SEEK_SET) {
		return fseek(f, offset, whence);
	}
	long read(void *buffer, int len) {
		return fread(buffer, 1, len, f);
	}
	void write(const void *buffer, int len) {
		fwrite(buffer, 1, len, f);
	}
	byte readByte() {
		byte v;
		read(&v, sizeof(byte));
		return v;
	}
	uint16 readWord() {
		uint16 v;
		read(&v, sizeof(uint16));
		return FROM_LE_16(v);
	}
	uint32 readLong() {
		uint32 v;
		read(&v, sizeof(uint32));
		return FROM_LE_32(v);
	}
	void readString(char *sLine) {
		while ((*sLine = readByte()) != '\n')
			++sLine;

		*sLine = '\0';
	}
	void writeByte(byte v) {
		write(&v, sizeof(byte));
	}
	void writeWord(uint16 v) {
		uint16 vTemp = TO_LE_16(v);
		write(&vTemp, sizeof(uint16));
	}
	void writeLong(uint32 v) {
		uint32 vTemp = TO_LE_32(v);
		write(&vTemp, sizeof(uint32));
	}
	void writeString(const char *s) {
		fprintf(f, "%s", s);
	}
	uint32 pos() {
		return ftell(f);
	}
	uint32 size() {
		uint32 position = ftell(f);
		fseek (f, 0, SEEK_END);
		uint32 end = ftell(f);
		fseek (f, position, SEEK_SET);

		return end;
	}
};

File textFile, txxInp, txxNtp;
int _version;

/*-------------------------------------------------------------------------*/

#define BUFFER_SIZE 32768

const byte tabdrFr[32] = {
	32, 101, 115,  97, 114, 105, 110,
	117, 116, 111, 108,  13, 100,  99,
	112, 109,  46, 118, 130,  39, 102,
	98,  44, 113, 104, 103,  33,  76,
	85, 106,  30,  31
};

const byte tab30Fr[32] = {
	69,  67,  74, 138, 133, 120,  77, 122,
	121,  68,  65,  63,  73,  80,  83,  82,
	156,  45,  58,  79,  49,  86,  78,  84,
	71,  81,  64,  66, 135,  34, 136,  91
};

const byte tab31Fr[32]= {
	93,  47,  48,  53,  50,  70, 124,  75,
	72, 147, 140, 150, 151,  57,  56,  51,
	107, 139,  55,  89, 131,  37,  54,  88,
	119,   0,   0,   0,   0,   0,   0,   0
};

const byte tabdrDe[32] = {
	0x20, 0x65, 0x6E, 0x69, 0x73, 0x72, 0x74,
	0x68, 0x61, 0x75, 0x0D, 0x63, 0x6C, 0x64,
	0x6D, 0x6F, 0x67, 0x2E, 0x62, 0x66, 0x53,
	0x2C, 0x77, 0x45, 0x7A, 0x6B, 0x44, 0x76,
	0x9C, 0x47, 0x1E, 0x1F
};

const byte tab30De[32] = {
	0x49, 0x4D, 0x21, 0x42, 0x4C, 0x70, 0x41, 0x52,
	0x57, 0x4E, 0x48, 0x3F, 0x46, 0x50, 0x55, 0x4B,
	0x5A, 0x4A, 0x54, 0x31, 0x4F, 0x56, 0x79, 0x3A,
	0x6A, 0x5B, 0x5D, 0x40, 0x22, 0x2F, 0x30, 0x35
};

const byte tab31De[32]= {
	0x78, 0x2D, 0x32, 0x82, 0x43, 0x39, 0x33, 0x38,
	0x7C, 0x27, 0x37, 0x3B, 0x25, 0x28, 0x29, 0x36,
	0x51, 0x59, 0x71, 0x81, 0x87, 0x88, 0x93, 0,
	0,    0,    0,    0,    0,    0,    0,    0
};

const byte *tabdr, *tab30, *tab31;
uint16 ctrlChar;

/**
 * Extracts a single character from the game data
 */
static void extractCharacter(unsigned char &c, uint &idx, uint &pt, bool &the_end, const uint16 *strData) {
	uint16 oct, ocd;

	/* 5-8 */
	oct = FROM_LE_16(strData[idx]);

	oct = ((uint16)(oct << (16 - pt))) >> (16 - pt);
	if (pt < 6) {
		idx = idx + 1;
		oct = oct << (5 - pt);
		pt = pt + 11;
		oct = oct | (FROM_LE_16(strData[idx]) >> pt);
	} else {
		pt = pt - 5;
		oct = (uint)oct >> pt;
	}

	if (oct == ctrlChar) {
		c = '$';
		the_end = true;
	} else if (oct == 30 || oct == 31) {
		ocd = FROM_LE_16(strData[idx]);
		ocd = (uint16)(ocd << (16 - pt)) >> (16 - pt);
		if (pt < 6) {
			idx = idx + 1;
			ocd = ocd << (5 - pt);
			pt = pt + 11;
			ocd = ocd | (FROM_LE_16(strData[idx]) >> pt);
		} else {
			pt = pt - 5;
			ocd = (uint)ocd >> pt;
		}
		if (oct == 30)
			c = (char)tab30[ocd];
		else
			c = (char)tab31[ocd];

		if (c == '\0')
			the_end = true;
	} else {
		c = (char)tabdr[oct];
	}
}

/**
 * Puts a compressed 5-bit value into the string data buffer
 */
static void addCompressedValue(int oct, int &indis, int &point, uint16 *strData) {
	// Write out the part of the value that fits into the current word
	if (point < 5)
		strData[indis] |= oct >> (5 - point);
	else
		strData[indis] |= oct << (point - 5);

	// Handling of there's any overlap into the next word
	if (point < 5) {
		// Overlapping into next word
		++indis;

		// Get the bits that fall into the next word and set it
		int remainder = oct & ((1 << (5 - point)) - 1);
		strData[indis] |= remainder << (16 - (5 - point));

		point += -5 + 16;
	} else {
		point -= 5;
		if (point == 0) {
			point = 16;
			++indis;
		}
	}
}

/**
 * Compresses a single passed character and stores it in the compressed strings buffer
 */
static void compressCharacter(unsigned char ch, int &indis, int &point, uint16 *strData) {
	if (ch == '$') {
		// End of string
		addCompressedValue(11, indis, point, strData);
		return;
	}

	// Scan through the tabdr array for a match
	for (int idx = 0; idx < 30; ++idx) {
		if ((idx != 11) && (tabdr[idx] == ch)) {
			addCompressedValue(idx, indis, point, strData);
			return;
		}
	}

	// Scan through the tab30 array
	for (int idx = 0; idx < 32; ++idx) {
		if (tab30[idx] == ch) {
			addCompressedValue(30, indis, point, strData);
			addCompressedValue(idx, indis, point, strData);
			return;
		}
	}

	// Scan through the tab31 array
	for (int idx = 0; idx < 32; ++idx) {
		if (tab31[idx] == ch) {
			addCompressedValue(31, indis, point, strData);
			addCompressedValue(idx, indis, point, strData);
			return;
		}
	}

	printf("Encountered invalid character '%c' when compressing strings\n", ch);
	exit(1);
}

/**
 * string extractor
 */
static void export_strings(const char *textFilename) {
	char buffer[BUFFER_SIZE];
	uint16 *strData;

	// Open input and output files
	if (!txxInp.open("TXX.INP", kFileReadMode)) {
		if (!txxInp.open("TXX.MOR", kFileReadMode)) {
			printf("Missing TXX.INP/MOR");
			exit(-1);
		}
	}
	if (!txxNtp.open("TXX.NTP", kFileReadMode)) {
		if (!txxNtp.open("TXX.IND", kFileReadMode)) {
			printf("Missing TXX.NTP/IND");
			exit(-1);
		}
	}
	textFile.open(textFilename, kFileWriteMode);

	// Read all the compressed string data into a buffer
	printf("%d %d", txxInp.size(), txxNtp.size());
	strData = (uint16 *)malloc(txxInp.size());
	txxInp.read(strData, txxInp.size());

	// Loop through getting each string
	for (unsigned int strIndex = 0; strIndex < (txxNtp.size() / 3); ++strIndex) {
		uint indis = txxNtp.readWord();
		uint point = txxNtp.readByte();

		// Extract the string
		int charIndex = 0;
		unsigned char ch;
		bool endFlag = false;
		do {
			extractCharacter(ch, indis, point, endFlag, strData);
			buffer[charIndex++] = ch;
			if (charIndex == BUFFER_SIZE) {
				printf("Extracted string exceeded allowed buffer size.\n");
				exit(1);
			}

			if (indis >= (txxInp.size() / 2))
				endFlag = true;
		} while (!endFlag);

		// Write out the string
		buffer[charIndex++] = '\n';
		buffer[charIndex] = '\0';
		textFile.writeString(buffer);
	}

	// Close the files and free the buffer
	free(strData);
	txxInp.close();
	txxNtp.close();
	textFile.close();
}

/**
 * string importer
 */
static void import_strings(const char *textFilename) {
	// Open input and output files
	if (!txxInp.open("TXX.INP", kFileWriteMode)) {
		printf("Missing TXX data file");
		exit(-1);
	}
	if (!txxNtp.open("TXX.NTP", kFileWriteMode)) {
		printf("Missing TXX index file");
		exit(-1);
	}
	textFile.open(textFilename, kFileReadMode);

	// Set up a buffer for the output compressed strings
	uint16 strData[BUFFER_SIZE];
	memset(strData, 0, BUFFER_SIZE*sizeof(uint16));
	char sLine[BUFFER_SIZE];

	int indis = 0;
	int point = 16;

	while (textFile.pos() < textFile.size()) {
		// Read in the next source line
		textFile.readString(sLine);

		// Write out the index entry for the string
		txxNtp.writeWord(indis);
		txxNtp.writeByte(point);

		// Loop through writing out the characters to the compressed data buffer
		char *s = sLine;
		while (*s) {
			compressCharacter(*s, indis, point, strData);
			++s;
		}
	}

	// Write out the compressed data
	if (point != 16)
		++indis;
	txxInp.write(strData, indis * 2);

	// Close the files
	txxInp.close();
	txxNtp.close();
	textFile.close();
}


int main(int argc, char *argv[]) {
	if (argc != 4) {
		printf("Format: %s export|import v1|v2 output_file\n", argv[0]);
		printf("where:\nv1: French DOS version\nv2: German DOS version\n");
		printf("The program must be run from the directory with the Mortville Manor game files.\n");
		exit(0);
	}

	if (!strcmp(argv[2], "v1")) {
		tab30 = tab30Fr;
		tab31 = tab31Fr;
		tabdr = tabdrFr;
		ctrlChar = 11;
	} else if (!strcmp(argv[2], "v2")) {
		tab30 = tab30De;
		tab31 = tab31De;
		tabdr = tabdrDe;
		ctrlChar = 10;
	} else {
		printf("Unknown version");
		exit(-1);
	}

	// Do the processing
	if (!strcmp(argv[1], "export"))
		export_strings(argv[3]);
	else if (!strcmp(argv[1], "import"))
		import_strings(argv[3]);
	else
		printf("Unknown operation specified\n");
}
