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
		write(s, strlen(s) + 1);
	}
	uint32 pos() {
		return ftell(f);
	}
};

File outputFile, mortCom;

/*-------------------------------------------------------------------------*/

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

void openOutputFile(const char *outFilename) {
	outputFile.open(outFilename, kFileWriteMode);

	// Write header
	outputFile.write("MORT", 4);
	outputFile.writeByte(VERSION_MAJOR);
	outputFile.writeByte(VERSION_MINOR);
}

/**
 * Write out the data for the font 
 */
void writeFontBlock() {
	byte checkBuffer[7];
	byte fontBuffer[121 * 6];

	// Move to just prior the font data and verify that we're reading the known mort.com
	mortCom.seek(0x36b0);
	mortCom.read(checkBuffer, 7);

	if ((checkBuffer[0] != 0x59) || (checkBuffer[1] != 0x5B) || (checkBuffer[2] != 0x58) ||
		(checkBuffer[3] != 0xC3) || (checkBuffer[4] != 0xE8) || (checkBuffer[5] != 0xD6) ||
		(checkBuffer[6] != 0x02)) {
		printf("Invalid mort.com input file");
		exit(0);
	}

	// Read in the data
	mortCom.read(fontBuffer, 121 * 6);

	// Write out a section header to the output file and the font data
	char fontHeader[4] = { 'F', 'O', 'N', 'T' };
	outputFile.write(fontHeader, 4);	// Section Id
	outputFile.writeWord(121 * 6);		// Section size

	outputFile.write(fontBuffer, 121 * 6);
}

void writeStaticStrings(const char **strings, int languageId) {
	// Write out a section header 
	char sStaticStrings[4] = { 'S', 'S', 'T', 'R' };
	outputFile.write(sStaticStrings, 4);

	// Figure out the block size
	int blockSize = 1;
	const char **s = &strings[0];
	while (*s) {
		blockSize += strlen(*s) + 1;
		++s;
	}

	outputFile.writeWord(blockSize);

	// Write out a byte indicating the language for this block
	outputFile.writeByte(languageId);

	// Write out each of the strings
	s = &strings[0];
	while (*s) {
		outputFile.writeString(*s);
		++s;
	}
}

/**
 * Write out the French strings previously hard-coded into the application
 */
void writeEnglishStaticStrings() {
	const char *string_list[] =  {
		"[2][ ][YES][NO]",
		"Go to",
		"Someone enters, looks surised, but says nothing",
		"Cool",
		" Lourde ",
		"Malsaine",
		"Idem",
		"You",
		"are",
		"Alone",

		"Gosh! You hear some noise...",
		" | You should have noticed,   |       ",
		"% of hints...",
		"Do you want to wake up?",
		"OK",
		"", 
		" Save",

		" Load",
		" Restart     ",
		"F3: More",
		"F8: Restart",
		"Hide self",
		"take",
		" probe    ",
		" raise    ",
		" -SUITE- ",
		" -STOP-  ",
		"[1] [So, use the DEP menu] [Ok]",
		"lift",
		"read",

		"look",
		"search",
		"open",
		"put",
		"turn",
		"tie",
		"close",
		"hit",
		"pose",
		"smash",

		"smell",
		"scratch",
		"probe",
		"[1] [ | Before, use the DEP menu...] [Ok]",
		"& day",
		NULL
	};

	writeStaticStrings(string_list, 1);
}

/**
 * Write out the French strings previously hard-coded into the application
 */
void writeFrenchStaticStrings() {
	const char *string_list[] = {
		"[2][ ][OUI][NON]",
		"aller",
		"quelqu'un entre, parait ‚tonn‚ mais ne dit rien",
		"Cool",
		" Lourde ",
		"Malsaine",
		"Idem",
		"Vous",
		"ˆtes",
		"SEUL",

		"Mince! Vous entendez du bruit...",
		" | Vous devriez avoir remarqu‚|       ",
		"% des indices...",
		"D‚sirez-vous vous r‚veiller?",
		"OK",
		"", 
		" Sauvegarde",

		" Chargement",
		" Recommence  ",
		"F3: Encore",
		"F8: Suite",
		"se cacher",

		"prendre",
		" sonder   ",
		"soulever",
		" -SUITE- ",
		" -STOP-  ",
		"[1][Alors, utilisez le menu DEP...][ok]",
		"soulever",
		"lire",

		"regarder",
		"fouiller",
		"ouvrir",
		"mettre",
		"tourner",
		"attacher",
		"fermer",
		"frapper",
		"poser",
		"d‚foncer",

		"sentir",
		"gratter",
		"sonder",
		"[1][ | Avant, utilisez le menu DEP...][ok]",
		"& jour",
		NULL
	};

	writeStaticStrings(string_list, 0);
}

void process() {
	writeFontBlock();
	writeEnglishStaticStrings();
	writeFrenchStaticStrings();
}

/**
 * Main method
 */
int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Format: input_filename output_filename\n", argv[0]);
		exit(0);
	}

	mortCom.open(argv[1], kFileReadMode);
	openOutputFile(argv[2]);

	process();

	mortCom.close();
	outputFile.close();
}
