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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

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
	kFileReadMode = 1,
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
		int pos = ftell(f);
		fseek (f, 0, SEEK_END);
		int end = ftell (f);
		fseek (f, pos, SEEK_SET);
	
		return end;
	}
};

File textFile, txxInp, txxNtp;

/*-------------------------------------------------------------------------*/

#define BUFFER_SIZE 32768

const byte tabdr[32] = {
	32, 101, 115,  97, 114, 105, 110,
	117, 116, 111, 108,  13, 100,  99,
	112, 109,  46, 118, 130,  39, 102,
	98,  44, 113, 104, 103,  33,  76,
	85, 106,  30,  31
};

const byte tab30[32] = {
	69,  67,  74, 138, 133, 120,  77, 122,
	121,  68,  65,  63,  73,  80,  83,  82,
	156,  45,  58,  79,  49,  86,  78,  84,
	71,  81,  64,  66, 135,  34, 136,  91
};

const byte tab31[32]= {
	93,  47,  48,  53,  50,  70, 124,  75,
	72, 147, 140, 150, 151,  57,  56,  51,
	107, 139,  55,  89, 131,  37,  54,  88,
	119,   0,   0,   0,   0,   0,   0,   0
};

/**
 * Extracts a single character from the game data
 */
static void extractCharacter(unsigned char &c, int &idx, int &pt, bool &the_end, const uint16 *strData) {
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

	switch (oct) {
	case 11 : {
		c = '$';
		the_end = true;
	}
	break;
	case 30:
	case 31 : {
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

		if (c == '\0') {
			the_end = true;
		}
	}
	break;
	default:
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
	txxInp.open("TXX.INP", kFileReadMode);
	txxNtp.open("TXX.NTP", kFileReadMode);
	textFile.open(textFilename, kFileWriteMode);

	// Read all the compressed string data into a buffer
	strData = (uint16 *)malloc(txxInp.size());
	txxInp.read(strData, txxInp.size());

	// Loop through getting each string
	for (unsigned int strIndex = 0; strIndex < (txxNtp.size() / 3); ++strIndex) {
		int indis = txxNtp.readWord();
		int point = txxNtp.readByte();

		// Extract the string
		int charIndex = 0;
		unsigned char ch;
		bool endFlag = false;
		do {
			extractCharacter(ch, indis, point, endFlag, strData);
			buffer[charIndex++] = ch;
			if (ch == BUFFER_SIZE) {
				printf("Extracted string exceeded allowed buffer size.\n");
				exit(1);
			}
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
	txxInp.open("TXX.INP", kFileWriteMode);
	txxNtp.open("TXX.NTP", kFileWriteMode);
	textFile.open(textFilename, kFileReadMode);

	// Set up a buffer for the output compressed strings
	uint16 strData[BUFFER_SIZE];
	memset(strData, 0, BUFFER_SIZE);
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
	txxInp.write(strData, indis);

	// Close the files
	txxInp.close();
	txxNtp.close();
	textFile.close();
}


int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Format: %s export|import output_file\n", argv[0]);
		printf("The program must be run from the directory with the Mortville Manor game files.\n");
		exit(0);
	}

	// Do the processing
	if (!strcmp(argv[1], "export"))
		export_strings(argv[2]);
	else if (!strcmp(argv[1], "import"))
		import_strings(argv[2]);
	else
		printf("Unknown operation specified\n");
}
