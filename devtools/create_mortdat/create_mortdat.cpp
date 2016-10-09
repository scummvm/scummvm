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

#include "common/endian.h"
#include "create_mortdat.h"
#include "enginetext.h"
#include "gametext.h"
#include "menudata.h"


bool File::open(const char *filename, AccessMode mode) {
	f = fopen(filename, (mode == kFileReadMode) ? "rb" : "wb");
	return (f != NULL);
}

void File::close() {
	fclose(f);
	f = NULL;
}

int File::seek(int32 offset, int whence) {
	return fseek(f, offset, whence);
}

long File::read(void *buffer, int len) {
	return fread(buffer, 1, len, f);
}
void File::write(const void *buffer, int len) {
	fwrite(buffer, 1, len, f);
}

byte File::readByte() {
	byte v;
	read(&v, sizeof(byte));
	return v;
}

uint16 File::readWord() {
	uint16 v;
	read(&v, sizeof(uint16));
	return FROM_LE_16(v);
}

uint32 File::readLong() {
	uint32 v;
	read(&v, sizeof(uint32));
	return FROM_LE_32(v);
}

void File::writeByte(byte v) {
	write(&v, sizeof(byte));
}

void File::writeWord(uint16 v) {
	uint16 vTemp = TO_LE_16(v);
	write(&vTemp, sizeof(uint16));
}

void File::writeLong(uint32 v) {
	uint32 vTemp = TO_LE_32(v);
	write(&vTemp, sizeof(uint32));
}

void File::writeString(const char *s) {
	write(s, strlen(s) + 1);
}

uint32 File::pos() {
	return ftell(f);
}

/*-------------------------------------------------------------------------*/

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
	const int knownAddr[3] = {0x30cd, 0x36b0, 0x36c0};
	byte checkBuffer[7];
	byte fontBuffer[121 * 6];

	// Move to just prior the font data and verify that we're reading the known mort.com
	for (int i = 0; i <= 3; ++i) {
		if ( i == 3) {
			printf("Invalid mort.com input file");
			exit(0);
		}

		mortCom.seek(knownAddr[i]);
		mortCom.read(checkBuffer, 7);

		if ((checkBuffer[0] == 0x59) && (checkBuffer[1] == 0x5B) && (checkBuffer[2] == 0x58) &&
			(checkBuffer[3] == 0xC3) && (checkBuffer[4] == 0xE8) && (checkBuffer[5] == 0xD6) &&
			(checkBuffer[6] == 0x02)) {
			break;
		}
	}

	// Read in the data
	mortCom.read(fontBuffer, 121 * 6);

	// Write out a section header to the output file and the font data
	const char fontHeader[4] = { 'F', 'O', 'N', 'T' };
	outputFile.write(fontHeader, 4);	// Section Id
	outputFile.writeWord(121 * 6);		// Section size

	outputFile.write(fontBuffer, 121 * 6);
}

void writeStaticStrings(const char **strings, DataType dataType, int languageId) {
	// Write out a section header
	const char sStaticStrings[4] = { 'S', 'S', 'T', 'R' };
	const char sGameStrings[4] = { 'G', 'S', 'T', 'R' };

	if (dataType == kStaticStrings)
		outputFile.write(sStaticStrings, 4);
	else if (dataType == kGameStrings)
		outputFile.write(sGameStrings, 4);

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
 * Write out the strings previously hard-coded into the engine
 */
void writeEngineStrings() {
	writeStaticStrings(engineDataEn, kStaticStrings, 1);
	writeStaticStrings(engineDataFr, kStaticStrings, 0);
	writeStaticStrings(engineDataDe, kStaticStrings, 2);
}

/**
 * Write out the strings used in the game
 */
void writeGameStrings() {
	writeStaticStrings(gameDataEn, kGameStrings, 1);
	writeStaticStrings(gameDataFr, kGameStrings, 0);
	writeStaticStrings(gameDataDe, kGameStrings, 2);
}

/**
 * Write out the data for the English menu
 */
void writeMenuData(const char *menuData, int languageId) {
	// Write out a section header to the output file and the menu data
	const char menuHeader[4] = { 'M', 'E', 'N', 'U' };
	outputFile.write(menuHeader, 4);				// Section Id
	int size = strlen(menuData) / 8 + 1; // Language code + Menu data size
	outputFile.writeWord(size);

	outputFile.writeByte(languageId);
	// Write each 8-characters block as a byte (one bit per character)
	// ' ' -> 0, anything else -> 1
	byte value = 0;
	int valueCpt = 0;
	const char* str = menuData;
	while (*str != 0) {
		if (*(str++) != ' ')
			value |= (1 << (7 - valueCpt));
		++valueCpt;
		if (valueCpt == 8) {
			outputFile.writeByte(value);
			value = 0;
			valueCpt = 0;
		}
	}
}

void writeMenuBlock() {
	writeMenuData(menuDataEn, 1);
	writeMenuData(menuDataFr, 0);
	writeMenuData(menuDataDe, 2);
}

void writeVerbNums(const int *verbs, int languageId) {
	// Write out a section header to the output file
	const char menuHeader[4] = { 'V', 'E', 'R', 'B' };
	outputFile.write(menuHeader, 4);				// Section Id
	int size = 52 + 1; // Language code + 26 words
	outputFile.writeWord(size);

	outputFile.writeByte(languageId);
	for (int i = 0; i < 26; i++)
		outputFile.writeWord(verbs[i]);
}

void writeMenuVerbs() {
	writeVerbNums(verbsEn, 1);
	writeVerbNums(verbsFr, 0);
	writeVerbNums(verbsDe, 2);
}

void process() {
	writeFontBlock();
	writeGameStrings();
	writeEngineStrings();
	writeMenuVerbs();
	writeMenuBlock();
}

/**
 * Main method
 */
int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage:\n%s input_filename\nWhere input_filename is the name of the Mortevielle DOS executable.\n", argv[0]);
		exit(0);
	}

	mortCom.open(argv[1], kFileReadMode);
	openOutputFile(MORT_DAT);

	process();

	mortCom.close();
	outputFile.close();
}
