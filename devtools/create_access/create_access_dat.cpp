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
#include "create_access_dat.h"
#include "amazon_resources.h"
#include "martian_resources.h"

/**
 * Format of the access.dat file that will be created:
 * 4 Bytes - Magic string 'SVMA' to identify valid data file
 * 2 bytes - Version number
 * 2 Bytes - Number of different games data in the data file
 * Series of index entries identifying each game:
 * 1 byte  - Game type: 1 = Amazon, 2 = Martian Memorandum, 3 = Noctropolis
 * 1 byte  - disc type: 0 = Floppy, 1 = CD, 2 = Common data shared across
 *           all variations of the given game
 * 1 byte  - Is Demo: 0 = Full game, 1 = Demo
 * 1 byte  - Language (Common::Language)
 * 4 bytes - File offset for the data for the game
 */

File outputFile;

void writeHeader(int numExecutables);
void writeAmazonCommonData();
void writeMartianCommonData(int argc, char *argv[]);
bool processExecutable(int idx, const char *name);

void NORETURN_PRE error(const char *s, ...) {
	printf("%s\n", s);
	exit(1);
}

int main(int argc, char *argv[]) {
	const uint NUM_COMMON_ENTRIES = 2;

	if (argc < 3) {
		printf("Format: %s output_filename executable1 [executable2 ..]\n", argv[0]);
		exit(0);
	}

	// Create the new data file for output
	outputFile.open(argv[1], kFileWriteMode);
	writeHeader(argc - 2 + NUM_COMMON_ENTRIES);

	// Write out entries containing common data for the games
	writeAmazonCommonData();
	writeMartianCommonData(argc, argv);

	// Iterate through processing each specified executable
	outputFile.seek(0, SEEK_END);
	for (int idx = 2; idx < argc; ++idx) {
		if (!processExecutable(idx - 2 + NUM_COMMON_ENTRIES, argv[idx]))
			break;
	}

	// Close the output file
	outputFile.close();
}

void writeHeader(int numExecutables) {
	// Write out magic string
	const char *MAGIC_STR = "SVMA";
	outputFile.write(MAGIC_STR, 4);

	// Write out version number
	outputFile.writeWord(VERSION_NUMBER);

	// Write out the number of entries the data file will contain
	outputFile.writeWord(numExecutables);

	// Write out padding for index entries that will be written
	outputFile.writeByte(0, 8 * numExecutables);
}

void writeAmazonCommonData() {
	// Write out the header entry
	outputFile.seek(8);
	outputFile.writeByte(1);    // Amazon
	outputFile.writeByte(2);    // Common data
	outputFile.writeByte(0);
	outputFile.writeByte(0);
	outputFile.writeLong(outputFile.size());

	// Write out cursor list
	outputFile.seek(0, SEEK_END);
	outputFile.writeWord(AMAZON_NUM_CURSORS);

	for (uint idx = 0; idx < AMAZON_NUM_CURSORS; ++idx) {
		outputFile.writeWord(Amazon::CURSOR_SIZES[idx]);
		outputFile.write(Amazon::CURSORS[idx], Amazon::CURSOR_SIZES[idx]);
	}

	// Write out font data
	outputFile.writeWord(Amazon::FONT2_INDEX_SIZE);
	for (uint idx = 0; idx < Amazon::FONT2_INDEX_SIZE; ++idx)
		outputFile.writeWord(Amazon::FONT2_INDEX[idx]);

	outputFile.writeWord(Amazon::FONT2_DATA_SIZE);
	outputFile.write(Amazon::FONT2_DATA, Amazon::FONT2_DATA_SIZE);

	outputFile.writeWord(Amazon::FONT6x6_INDEX_SIZE);
	for (uint idx = 0; idx < Amazon::FONT6x6_INDEX_SIZE; ++idx)
		outputFile.writeWord(Amazon::FONT6x6_INDEX[idx]);

	outputFile.writeWord(Amazon::FONT6x6_DATA_SIZE);
	outputFile.write(Amazon::FONT2_DATA, Amazon::FONT6x6_DATA_SIZE);
}


void writeMartianCommonData(int argc, char *argv[]) {
	// Write out the header entry
	outputFile.seek(16);
	outputFile.writeByte(2);    // Martian
	outputFile.writeByte(2);    // Common data
	outputFile.writeByte(0);
	outputFile.writeByte(0);
	outputFile.writeLong(outputFile.size());

	// Write out cursor list
	outputFile.seek(0, SEEK_END);
	outputFile.writeWord(MARTIAN_NUM_CURSORS);

	for (uint idx = 0; idx < MARTIAN_NUM_CURSORS; ++idx) {
		outputFile.writeWord(Martian::CURSOR_SIZES[idx]);
		outputFile.write(Martian::CURSORS[idx], Martian::CURSOR_SIZES[idx]);
	}

	// Check for the presence of a Martian Memorandum executable
	for (int idx = 2; idx < argc; ++idx) {
		File exeFile;
		if (!exeFile.open(argv[idx]))
			continue;

		// Total up the first 256 bytes of the executable as a simplified checksum
		uint fileChecksum = 0;
		for (int i = 0; i < 256; ++i)
			fileChecksum += exeFile.readByte();

		if (fileChecksum == 10454) {
			// Write out font data
			const int DATA_SEGMENT = 0x9600;
			#define FONT_COUNT 119
			const int FONT_WIDTHS[2] = { 0x47E6, 0x4C9C };
			const int FONT_CHAR_OFFSETS[2] = { 0x46F8, 0x4BAE };
			const uint FONT_DATA_SIZE[2] = { 849, 907 };
			int dataOffset;

			for (int fontNum = 0; fontNum < 2; ++fontNum) {
				// Write out sizes
				outputFile.writeWord(FONT_COUNT);
				outputFile.writeWord(FONT_DATA_SIZE[fontNum]);

				// Write out character widths
				exeFile.seek(DATA_SEGMENT + FONT_WIDTHS[fontNum]);
				outputFile.write(exeFile, FONT_COUNT);

				// Write out character offsets
				uint offsets[FONT_COUNT];
				exeFile.seek(DATA_SEGMENT + FONT_CHAR_OFFSETS[fontNum]);
				for (int i = 0; i < FONT_COUNT; ++i) {
					offsets[i] = exeFile.readWord();
					if (i == 0)
						dataOffset = offsets[0];
					offsets[i] -= dataOffset;
					assert(offsets[i] < FONT_DATA_SIZE[fontNum]);

					outputFile.writeWord(offsets[i]);
				}

				// Write out character data
				exeFile.seek(DATA_SEGMENT + dataOffset);
				outputFile.write(exeFile, FONT_DATA_SIZE[fontNum]);
			}

			return;
		}
	}

	// No executable found, so store 0 size fonts
	outputFile.writeWord(0);
	outputFile.writeWord(0);
	outputFile.writeWord(0);
	outputFile.writeWord(0);
}

bool processExecutable(int exeIdx, const char *name) {
	uint dataSegmentOffset;
	uint filenamesOffset, numFilenames;
	uint charsStart, charsEnd;
	uint roomsStart, roomsEnd, numRooms;
	uint travelPosOffset;
	const char *const *roomDescs;
	const byte *deathScreens;
	const char *const *deathText;
	uint numDeaths;
	uint numItems;
	const char *const *itemNames;
	const int *comboTable;
	byte gameId = 0, discType = 0, demoType = 0;
	byte language = Common::EN_ANY;

	// Open up the file for access
	File exeFile;
	if (!exeFile.open(name)) {
		printf("Could not open file - %s\n", name);
		return false;
	}

	// Total up the first 256 bytes of the executable as a simplified
	// means of identifying the different executables we support
	uint fileChecksum = 0;
	for (int idx = 0; idx < 256; ++idx)
		fileChecksum += exeFile.readByte();

	switch (fileChecksum) {
	case 11899:
		// Amazon English floppy
		gameId = 1;
		dataSegmentOffset = 0xC8C0;
		filenamesOffset = dataSegmentOffset + 0x3628;
		numFilenames = 100;
		charsStart = dataSegmentOffset + 0x4234;
		charsEnd = dataSegmentOffset + 0x49c6;
		roomsStart = dataSegmentOffset + 0x35a8;
		roomsEnd = dataSegmentOffset + 0x4234;
		travelPosOffset = dataSegmentOffset + 0x5ff7;
		numRooms = 64;
		roomDescs = &Amazon::ROOM_DESCR[0];
		deathScreens = Amazon::DEATH_SCREENS_ENG;
		deathText = &Amazon::DEATH_TEXT_ENG[0];
		numDeaths = sizeof(Amazon::DEATH_SCREENS_ENG);
		numItems = 85;
		itemNames = &Amazon::INVENTORY_NAMES_ENG[0];
		comboTable = &Amazon::COMBO_TABLE[0][0];
		break;

	case 12360:
		// Amazon CD English
		gameId = 1;
		discType = 1;
		dataSegmentOffset = 0xd370;
		filenamesOffset = dataSegmentOffset + 0x3EA0;
		numFilenames = 116;
		charsStart = dataSegmentOffset + 0x4BDC;
		charsEnd = dataSegmentOffset + 0x5AF4;
		roomsStart = dataSegmentOffset + 0x3E20;
		roomsEnd = dataSegmentOffset + 0x4BDC;
		travelPosOffset = dataSegmentOffset + 0x7125;
		numRooms = 64;
		roomDescs = &Amazon::ROOM_DESCR[0];
		deathScreens = Amazon::DEATH_SCREENS_ENG;
		deathText = &Amazon::DEATH_TEXT_ENG[0];
		numDeaths = sizeof(Amazon::DEATH_SCREENS_ENG);
		numItems = 85;
		itemNames = &Amazon::INVENTORY_NAMES_ENG[0];
		comboTable = &Amazon::COMBO_TABLE[0][0];
		break;

	case 11748:
		// Amazon English Demo
		gameId = 1;
		discType = 0;
		demoType = 1;
		dataSegmentOffset = 0xa2a0;
		filenamesOffset = dataSegmentOffset + 0x242C;
		numFilenames = 100;
		charsStart = dataSegmentOffset + 0x2F1A;
		charsEnd = dataSegmentOffset + 0x34FB;
		roomsStart = dataSegmentOffset + 0x23AC;
		roomsEnd = dataSegmentOffset + 0x2F1A;
		travelPosOffset = dataSegmentOffset + 0x494E;
		numRooms = 64;
		roomDescs = &Amazon::ROOM_DESCR[0];
		deathScreens = Amazon::DEATH_SCREENS_ENG;
		deathText = &Amazon::DEATH_TEXT_ENG[0];
		numDeaths = sizeof(Amazon::DEATH_SCREENS_ENG);
		numItems = 85;
		itemNames = &Amazon::INVENTORY_NAMES_ENG[0];
		comboTable = &Amazon::COMBO_TABLE[0][0];
		break;

	case 1224:
		// Martian Memorandum English packed
		printf("Martian Memorandum provided that's packed with EXEPACK.\n");
		printf("It needs to be first unpacked before it can be used with this tool.\n");
		return false;

	case 10454:
		// Martian Memorandum English decompressed
		gameId = 2;
		dataSegmentOffset = 0x9600;
		filenamesOffset = dataSegmentOffset + 0x373A;
		numFilenames = 80;
		charsStart = dataSegmentOffset + 0x40F2;
		charsEnd = dataSegmentOffset + 0x46F8;
		roomsStart = dataSegmentOffset + 0x36DA;
		roomsEnd = dataSegmentOffset + 0x40F2;
		travelPosOffset = dataSegmentOffset + 0x58E9;
		numRooms = 48;
		roomDescs = &Martian::ROOM_DESCR[0];
		deathScreens = Martian::DEATH_SCREENS_ENG;
		deathText = &Martian::DEATH_TEXT_ENG[0];
		numDeaths = sizeof(Martian::DEATH_SCREENS_ENG);
		numItems = 55;
		itemNames = &Martian::INVENTORY_NAMES_ENG[0];
		comboTable = nullptr;
		break;

	default:
		printf("Unknown game executable specified - %s\n", name);
		exeFile.close();
		return false;
	}

	// Write out header entry
	uint outputOffset = outputFile.size();
	outputFile.seek(8 + exeIdx * 8);
	outputFile.writeByte(gameId);
	outputFile.writeByte(discType);
	outputFile.writeByte(demoType);
	outputFile.writeByte(language);
	outputFile.writeLong(outputOffset);
	outputFile.seek(0, SEEK_END);

	// Write out list of AP filenames
	outputFile.writeWord(numFilenames);
	for (uint idx = 0; idx < numFilenames; ++idx) {
		exeFile.seek(filenamesOffset + idx * 2);
		uint nameOffset = exeFile.readWord();

		exeFile.seek(dataSegmentOffset + nameOffset);
		outputFile.writeString(exeFile);
	}

	// Write out the character list
	exeFile.seek(charsStart);
	Common::Array<uint> charOffsets;
	charOffsets.push_back(exeFile.readWord());
	assert((dataSegmentOffset + charOffsets[0] - exeFile.pos()) < 512);

	while (exeFile.pos() < (dataSegmentOffset + charOffsets[0]))
		charOffsets.push_back(exeFile.readWord());

	outputFile.writeWord(charOffsets.size());
	charOffsets.push_back(charsEnd);
	for (uint idx = 0; idx < charOffsets.size() - 1; ++idx) {
		if (charOffsets[idx] == 0) {
			outputFile.writeWord(0);
		} else {
			uint nextOffset = 0xffff;
			for (uint idx2 = 0; idx2 < charOffsets.size(); ++idx2) {
				if (charOffsets[idx2] && charOffsets[idx2] > charOffsets[idx] && charOffsets[idx2] < nextOffset)
					nextOffset = charOffsets[idx2];
			}
			uint size = nextOffset - charOffsets[idx];

			exeFile.seek(dataSegmentOffset + charOffsets[idx]);
			outputFile.writeWord(size);
			outputFile.write(exeFile, size);
		}
	}

	// Write out the room data
	Common::Array<uint> roomOffsets;
	Common::Array<Common::Point> travelPos;

	exeFile.seek(roomsStart);
	for (uint idx = 0; idx < numRooms; ++idx)
		roomOffsets.push_back(exeFile.readWord());
	roomOffsets.push_back(roomsEnd);

	exeFile.seek(travelPosOffset);
	for (uint idx = 0; idx < numRooms; ++idx) {
		int16 xp = (int16)exeFile.readWord();
		int16 yp = (int16)exeFile.readWord();
		travelPos.push_back(Common::Point(xp, yp));
	}

	outputFile.writeWord(numRooms);
	for (uint idx = 0; idx < numRooms; ++idx) {
		uint dataSize = 0;

		if (roomOffsets[idx] == 0) {
			dataSize = 0;
		} else {
			// Find the offset of the next higher entry that's non-zero
			uint offset = 0;
			for (uint idx2 = idx + 1; !offset; ++idx2)
				offset = roomOffsets[idx2];
			dataSize = offset - roomOffsets[idx];
			exeFile.seek(dataSegmentOffset + roomOffsets[idx]);
		}

		// Write out the room description (used only by the debugger)
		outputFile.writeString(roomDescs[idx]);

		// Write out travel position
		outputFile.writeWord((uint16)travelPos[idx].x);
		outputFile.writeWord((uint16)travelPos[idx].y);

		// Write out the data for the room
		outputFile.writeWord(dataSize);
		if (dataSize > 0)
			outputFile.write(exeFile, dataSize);
	}

	// Write out the deaths list
	outputFile.writeWord(numDeaths);
	for (uint idx = 0; idx < numDeaths; ++idx) {
		// Write out the screen number and text
		outputFile.writeByte(deathScreens[idx]);
		outputFile.writeString(deathText[idx]);
	}

	// Write out inventory data
	outputFile.writeWord(numItems);
	for (uint idx = 0; idx < numItems; ++idx) {
		outputFile.writeString(itemNames[idx]);

		if (comboTable == nullptr) {
			for (uint cIdx = 0; cIdx < 4; ++cIdx)
				outputFile.writeWord(0);
		} else {
			for (uint cIdx = 0; cIdx < 4; ++cIdx, ++comboTable)
				outputFile.writeWord((uint16)*comboTable);
		}
	}

	// Write out game specific strings and other data
	if (gameId == 1) {
		// Write out miscellaneous strings
		outputFile.writeString(Amazon::NO_HELP_MESSAGE_ENG);
		outputFile.writeString(Amazon::NO_HINTS_MESSAGE_ENG);
		outputFile.writeString(Amazon::RIVER_HIT1_ENG);
		outputFile.writeString(Amazon::RIVER_HIT2_ENG);
		outputFile.writeString(Amazon::BAR_MESSAGE_ENG);

		for (int idx = 0; idx < 3; ++idx)
			outputFile.writeString(Amazon::HELPLVLTXT_ENG[idx]);
		for (int idx = 0; idx < 9; ++idx)
			outputFile.writeString(Amazon::IQLABELS_ENG[idx]);

		outputFile.writeString(Amazon::CANT_GET_THERE_ENG);
	}

	// Do final padding to the next paragraph boundary
	if ((outputFile.size() % 16) != 0)
		outputFile.writeByte(0, 16 - (outputFile.size() % 16));

	// Close the executable and signal that it was processed successfully
	exeFile.close();
	return true;
}
