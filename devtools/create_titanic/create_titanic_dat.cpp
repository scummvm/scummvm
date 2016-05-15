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
#include "common/language.h"
#include "common/rect.h"
#include "winexe_pe.h"
#include "file.h"

/**
 * Format of the access.dat file that will be created:
 * 4 Bytes - Magic string 'SVTN' to identify valid data file
 * 2 bytes - Version number
 *
 * Following is a series of index entries with the following fields:
 * 4 bytes - offset in file of entry
 * 4 bytes - size of entry in the file
 * ASCIIZ  - name of the resource
 */

#define VERSION_NUMBER 1

Common::File inputFile, outputFile;
Common::PEResources res;
uint headerOffset = 6;
uint dataOffset = 0x200;
#define SEGMENT_OFFSET 0x401C00

void NORETURN_PRE error(const char *s, ...) {
	printf("%s\n", s);
	exit(1);
}

void writeEntryHeader(const char *name, uint offset, uint size) {
	assert(headerOffset < 0x200);
	outputFile.seek(headerOffset);
	outputFile.writeLong(offset);
	outputFile.writeLong(size);
	outputFile.writeString(name);

	headerOffset += 8 + strlen(name) + 1;
}

void writeFinalEntryHeader() {
	assert(headerOffset <= 0x1F8);
	outputFile.seek(headerOffset);
	outputFile.writeLong(0);
	outputFile.writeLong(0);
}

void writeStringArray(const char *name, uint offset, int count) {
	outputFile.seek(dataOffset);

	inputFile.seek(offset);
	uint *offsets = new uint[count];
	for (int idx = 0; idx < count; ++idx)
		offsets[idx] = inputFile.readLong();

	// Iterate through reading each string
	for (int idx = 0; idx < count; ++idx) {
		if (offsets[idx]) {
			inputFile.seek(offsets[idx] - SEGMENT_OFFSET);
			outputFile.writeString(inputFile);
		} else {
			outputFile.writeString("");
		}
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader(name, dataOffset, size);
	dataOffset += size;

	delete[] offsets;
}

Common::WinResourceID getResId(uint id) {
	return Common::WinResourceID(id);
}

Common::WinResourceID getResId(const char *id) {
	if (!strcmp(id, "Bitmap"))
		return Common::WinResourceID(2);

	return Common::WinResourceID(id);
}

void writeResource(const char *name, Common::File *file) {
	outputFile.seek(dataOffset);
	outputFile.write(*file, file->size());

	writeEntryHeader(name, dataOffset, file->size());
	dataOffset += file->size();
	delete file;
}

void writeResource(const char *sectionStr, const uint32 resId) {
	char nameBuffer[256];
	sprintf(nameBuffer, "%s/%d", sectionStr, resId);
	
	Common::File *file = res.getResource(getResId(sectionStr), resId);
	assert(file);
	writeResource(nameBuffer, file);
}

void writeResource(const char *sectionStr, const char *resId) {
	char nameBuffer[256];
	sprintf(nameBuffer, "%s/%s", sectionStr, resId);

	Common::File *file = res.getResource(getResId(sectionStr), 
		Common::WinResourceID(resId));
	assert(file);
	writeResource(nameBuffer, file);
}

void writeHeader() {
	// Write out magic string
	const char *MAGIC_STR = "SVTN";
	outputFile.write(MAGIC_STR, 4);

	// Write out version number
	outputFile.writeWord(VERSION_NUMBER);
}

void writeData() {
	writeStringArray("TEXT/STRINGS1", 0x21B7C8, 376);
	writeStringArray("TEXT/STRINGS2", 0x21BDB0, 218);
	writeStringArray("TEXT/STRINGS3", 0x21C120, 1576);
	writeStringArray("TEXT/STRINGS4", 0x21D9C8, 82);

	writeResource("Bitmap", "BACKDROP");
	writeResource("Bitmap", "EVILTWIN");
	writeResource("Bitmap", "RESTORED");
	writeResource("Bitmap", "RESTOREF");
	writeResource("Bitmap", "RESTOREU");
	writeResource("Bitmap", "STARTD");
	writeResource("Bitmap", "STARTF");
	writeResource("Bitmap", "STARTU");
	writeResource("Bitmap", "TITANIC");
	writeResource("Bitmap", 133);
	writeResource("Bitmap", 164);
	writeResource("Bitmap", 165);

	writeResource("STFONT", 149);
	writeResource("STFONT", 151);
	writeResource("STFONT", 152);
	writeResource("STFONT", 153);

	writeResource("TEXT", "STVOCAB.TXT");
	writeResource("TEXT", "JRQUOTES.TXT");
	writeResource("TEXT", 155);
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Format: %s ST.exe titanic.dat\n", argv[0]);
		exit(0);
	}

	if (!inputFile.open(argv[1])) {
		error("Could not open input file");
	}
	res.loadFromEXE(argv[1]);

	if (!outputFile.open(argv[2], Common::kFileWriteMode)) {
		error("Could not open output file");
	}

	writeHeader();
	writeData();
	writeFinalEntryHeader();

	inputFile.close();
	outputFile.close();
	return 0;
}
