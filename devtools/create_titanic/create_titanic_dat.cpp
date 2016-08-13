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
#include "script_preresponses.h"
#include "script_quotes.h"
#include "script_responses.h"
#include "script_ranges.h"
#include "script_states.h"
#include "tag_maps.h"

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
#define HEADER_SIZE 0xD00

Common::File inputFile, outputFile;
Common::PEResources res;
uint headerOffset = 6;
uint dataOffset = HEADER_SIZE;
#define SEGMENT_OFFSET 0x401C00

const int FILE_DIFF = 0x401C00;

static const char *const ITEM_NAMES[46] = {
	"LeftArmWith", "LeftArmWithout", "RightArmWith", "RightArmWithout", "BridgeRed",
	"BridgeYellow", "BridgeBlue", "BridgeGreen", "Parrot", "CentralCore", "BrainGreen",
	"BrainYellow", "BrainRed", "BrainBlue", "ChickenGreasy", "ChickenPlain", "ChickenPurple",
	"ChickenRed", "ChickenYellow", "CrushedTV", "Ear", "Ear1", "Eyeball", "Eyeball1",
	"Feather", "Lemon", "GlassEmpty", "GlassPurple", "GlassRed", "GlassYellow", "Hammer",
	"Hose", "HoseEnd", "LiftHead", "LongStick", "Magazine", "Mouth", "MusicKey", "Napkin",
	"Nose", "Perch", "PhonoCylinder", "PhonoCylinder1", "PhonoCylinder2", "PhonoCylinder3",
	"Photo"
};

static const char *const ITEM_DESCRIPTIONS[46] = {
	"The Maitre d'Bot's left arm holding a key", "The Maitre d'Bot's left arm",
	"The Maitre d'Bot's right arm holding Titania's auditory center",
	"The Maitre d'Bot's right arm", "Red Fuse", "Yellow Fuse", "Blue Fuse",
	"Green Fuse", "The Parrot", "Titania's central intelligence core",
	"Titania's auditory center", "Titania's olfactory center",
	"Titania's speech center", "Titania's vision center", "rather greasy chicken",
	"very plain chicken", "chicken smeared with starling pur$e",
	"chicken covered with tomato sauce", "chicken coated in mustard sauce",
	"A crushed television set", "Titania's ear", "Titania's ear", "Titania's eye",
	"Titania's eye", "A parrot feather", "A nice fat juicy lemon",
	"An empty beer glass", "A beer glass containing pur$ed flock of starlings",
	"A beer glass containing tomato sauce", "A beer glass containing mustard sauce",
	"A hammer", "A hose", "The other end of a hose", "The LiftBot's head",
	"A rather long stick", "A magazine", "Titania's mouth", "A key",
	"A super-absorbent napkin", "Titania's nose", "A perch", "A phonograph cylinder",
	"A phonograph cylinder", "A phonograph cylinder", "A phonograph cylinder",
	"A photograph"
};

static const char *const ITEM_IDS[40] = {
	"MaitreD Left Arm", "MaitreD Right Arm", "OlfactoryCentre", "AuditoryCentre",
	"SpeechCentre", "VisionCentre", "CentralCore", "Perch", "SeasonBridge",
	"FanBridge", "BeamBridge", "ChickenBridge", "CarryParrot", "Chicken",
	"CrushedTV", "Feathers", "Lemon", "BeerGlass", "BigHammer", "Ear1", "Ear 2",
	"Eye1", "Eye2", "Mouth", "Nose", "NoseSpare", "Hose", "DeadHoseSpare",
	"HoseEnd", "DeadHoseEndSpare", "BrokenLiftbotHead", "LongStick", "Magazine",
	"Napkin", "Phonograph Cylinder", "Phonograph Cylinder 1", "Phonograph Cylinder 2",
	"Phonograph Cylinder 3", "Photograph", "Music System Key"
};

static const char *const ROOM_NAMES[34] = {
	"1stClassLobby", "1stClassRestaurant", "1stClassState",
	"2ndClassLobby", "secClassState", "Arboretum", "FrozenArboretum",
	"Bar", "BilgeRoom", "BilgeRoomWith", "BottomOfWell", "Bridge",
	"CreatorsChamber", "CreatorsChamberOn", "Dome", "Home", "Lift",
	"EmbLobby", "MoonEmbLobby", "MusicRoomLobby", "MusicRoom",
	"ParrotLobby", "Pellerator", "PromenadeDeck", "SculptureChamber",
	"SecClassLittleLift", "ServiceElevator", "SGTLeisure", "SGTLittleLift",
	"SgtLobby", "SGTState", "Titania", "TopOfWell", "PlayersRoom"
};

struct NumberEntry {
	const char *_text;
	int _value;
	uint _flags;
};

const NumberEntry NUMBERS[76] = {
	{ "a", 1, 3 },
	{ "and", 0, 1 },
	{ "negative", 0, 10 },
	{ "minus", 0, 10 },
	{ "below zeor", 0, 8 },
	{ "degrees below zero", 0, 8 },
	{ "nil", 0, 2 },
	{ "zero", 0, 2 },
	{ "one", 1, 0x12 },
	{ "two", 2, 0x12 },
	{ "three", 3, 0x12 },
	{ "four", 4, 0x12 },
	{ "five", 5, 0x12 },
	{ "six", 6, 0x12 },
	{ "seven", 7, 0x12 },
	{ "eight", 8, 0x12 },
	{ "nine", 9, 0x12 },
	{ "0", 0, 2 },
	{ "1", 1, 2 },
	{ "2", 2, 2 },
	{ "3", 3, 2 },
	{ "4", 4, 2 },
	{ "5", 5, 2 },
	{ "6", 6, 2 },
	{ "7", 7, 2 },
	{ "8", 8, 2 },
	{ "9", 9, 2 },
	{ "first", 1, 2 },
	{ "second", 2, 2 },
	{ "third", 3, 2 },
	{ "fourth", 4, 2 },
	{ "fifth", 5, 2 },
	{ "sixth", 6, 2 },
	{ "seventh", 7, 2 },
	{ "eighth", 8, 2 },
	{ "ninth", 9, 2 },
	{ "ten", 10, 2 },
	{ "eleven", 11, 2 },
	{ "twelve", 12, 2 },
	{ "thirteen", 13, 2 },
	{ "fourteen", 14, 2 },
	{ "fifteen", 15, 2 },
	{ "sixteen", 16, 2 },
	{ "seventeen", 17, 2 },
	{ "eighteen", 18, 2 },
	{ "nineteen", 19, 2 },
	{ "tenth", 10, 2 },
	{ "eleventh", 11, 2 },
	{ "twelfth", 12, 2 },
	{ "thirteenth", 13, 2 },
	{ "fourteenth", 14, 2 },
	{ "fifteenth", 15, 2 },
	{ "sixteenth", 16, 2 },
	{ "seventeenth", 17, 2 },
	{ "eighteenth", 18, 2 },
	{ "nineteenth", 19, 2 },
	{ "twenty", 20, 0x12 },
	{ "thirty", 30, 0x12 },
	{ "forty", 40, 0x12 },
	{ "fourty", 40, 0x12 },
	{ "fifty", 50, 0x12 },
	{ "sixty", 60, 0x12 },
	{ "seventy", 70, 0x12 },
	{ "eighty", 80, 0x12 },
	{ "ninety", 90, 0x12 },
	{ "twentieth", 20, 2 },
	{ "thirtieth", 30, 2 },
	{ "fortieth", 40, 2 },
	{ "fiftieth", 50, 2 },
	{ "sixtieth", 60, 2 },
	{ "seventieth", 70, 2 },
	{ "eightieth", 80, 2 },
	{ "ninetieth", 90, 2 },
	{ "hundred", 100, 4 },
	{ "hundredth", 100, 6 }
};

struct CommonPhrase {
	const char *_str;
	uint _dialogueId;
	uint _roomNum;
	uint _val1;
};

static const CommonPhrase BELLBOT_COMMON_PHRASES[] = {
	{ "what is wrong with her", 0x30FF9, 0x7B, 0 },
	{ "what is wrong with titania", 0x30FF9, 0x7B, 0 },
	{ "something for the weekend", 0x30D8B, 0x00, 0 },
	{ "other food", 0x30E1D, 0x00, 3 },
	{ "different food", 0x30E1D, 0x00, 3 },
	{ "alternative food", 0x30E1D, 0x00, 3 },
	{ "decent food", 0x30E1D, 0x00, 3 },
	{ "nice food", 0x30E1D, 0x00, 3 },
	{ "nicer food", 0x30E1D, 0x00, 3 },
	{ "make me happy", 0x31011, 0x00, 0 },
	{ "cheer me up", 0x31011, 0x00, 0 },
	{ "help me if im unhappy", 0x31011, 0x00, 0 },
	{ "i obtain a better room", 0x30E8A, 0x00, 3 },
	{ "i obtain a better room", 0x30E8A, 0x00, 2 },
	{ "i get a better room", 0x30E8A, 0x00, 3 },
	{ "i get a better room", 0x30E8A, 0x00, 2 },
	{ "i want a better room", 0x30E8A, 0x00, 3 },
	{ "i want a better room", 0x30E8A, 0x00, 2 },
	{ "i understood", 0x30D75, 0x6D, 0 },
	{ "i knew", 0x30D75, 0x6D, 0 },
	{ "i know", 0x30D75, 0x6D, 0 },
	{ "not stupid", 0x30D75, 0x6D, 0 },
	{ "cheeky", 0x30D75, 0x6D, 0 },
	{ "not help", 0x30D6F, 0x6D, 0 },
	{ "not helpful", 0x30D6F, 0x6D, 0 },
	{ "dont help", 0x30D6F, 0x6D, 0 },
	{ "no help", 0x30D6F, 0x6D, 0 },
	{ "sorry", 0x30D76, 0x6D, 0 },
	{ "not mean that", 0x30D76, 0x6D, 0 },
	{ "didnt mean that", 0x30D76, 0x6D, 0 },
	{ "apologise", 0x30D76, 0x6D, 0 },
	{ "play golf", 0x313B6, 0x00, 0 },
	{ "is not the captain meant to go down with the ship", 0x31482, 0x00, 0 },
	{ "is not the captain supposed to go down with the ship", 0x31482, 0x00, 0 },
	{ "sauce sticks to the chicken", 0x3156B, 0x00, 0 },
	{ "sauce gets stuck to the chicken", 0x3156B, 0x00, 0 },
	{ nullptr, 0, 0, 0 }
};

struct FrameRange {
	int _startFrame;
	int _endFrame;
};

static const FrameRange BARBOT_FRAME_RANGES[60] = {
	{ 558, 585 }, { 659, 692 }, { 802, 816 }, { 1941, 1977 }, { 1901, 1941 },
	{ 810, 816 }, { 857, 865}, { 842, 857 }, { 821, 842 }, { 682, 692 },
	{ 1977, 2018 }, { 2140, 2170 }, { 2101, 2139 }, { 2018, 2099}, { 1902, 2015 },
	{ 1811, 1901 }, { 1751, 1810 }, { 1703, 1750 }, { 1681, 1702 }, { 1642, 1702 },
	{ 1571, 1641 }, { 1499, 1570 }, { 1403, 1463 }, { 1464, 1499 }, { 1288, 1295 },
	{ 1266, 1287 }, { 1245, 1265 }, { 1208, 1244 }, { 1171, 1207 }, { 1120, 1170 },
	{ 1092, 1120 }, { 1092, 1092 }, { 1044, 1091 }, { 1011, 1043 }, { 1001, 1010 },
	{ 985, 1001 }, { 927, 984 }, { 912, 926 }, { 898, 906 }, { 802, 896 },
	{ 865, 896 }, { 842, 865 }, { 816, 842 }, { 802, 842 }, { 740, 801 },
	{ 692, 740 }, { 610, 692 }, { 558, 610 }, { 500, 558 }, { 467, 500 },
	{ 421, 466 }, { 349, 420 }, { 306, 348 }, { 305, 306 }, { 281, 305 },
	{ 202, 281 }, { 182, 202 }, { 165, 182 }, { 96, 165 }, { 0, 95 }
};

void NORETURN_PRE error(const char *s, ...) {
	printf("%s\n", s);
	exit(1);
}

void writeEntryHeader(const char *name, uint offset, uint size) {
	assert(headerOffset < HEADER_SIZE);
	outputFile.seek(headerOffset);
	outputFile.writeLong(offset);
	outputFile.writeLong(size);
	outputFile.writeString(name);

	headerOffset += 8 + strlen(name) + 1;
}

void writeFinalEntryHeader() {
	assert(headerOffset <= (HEADER_SIZE - 8));
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

void writeStringArray(const char *name, const char *const *strings, int count) {
	outputFile.seek(dataOffset);

	// Iterate through writing each string
	for (int idx = 0; idx < count; ++idx) {
		outputFile.writeString(strings[idx]);
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader(name, dataOffset, size);
	dataOffset += size;
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

void writeResource(const char *sectionStr, uint32 resId) {
	char nameBuffer[256];
	sprintf(nameBuffer, "%s/%u", sectionStr, resId);
	
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

void writeBitmap(const char *name, Common::File *file) {
	outputFile.seek(dataOffset);

	// Write out the necessary bitmap header so that the ScummVM
	// BMP decoder can properly handle decoding the bitmaps
	outputFile.write("BM", 2);
	outputFile.writeLong(file->size() + 14);	// Filesize
	outputFile.writeLong(0);					// res1 & res2
	outputFile.writeLong(0x436);				// image offset

	outputFile.write(*file, file->size() + 14);

	writeEntryHeader(name, dataOffset, file->size() + 14);
	dataOffset += file->size() + 14;
	delete file;
}

void writeBitmap(const char *sectionStr, const char *resId) {
	char nameBuffer[256];
	sprintf(nameBuffer, "%s/%s", sectionStr, resId);

	Common::File *file = res.getResource(getResId(sectionStr),
		Common::WinResourceID(resId));
	assert(file);
	writeBitmap(nameBuffer, file);
}

void writeBitmap(const char *sectionStr, uint32 resId) {
	char nameBuffer[256];
	sprintf(nameBuffer, "%s/%u", sectionStr, resId);

	Common::File *file = res.getResource(getResId(sectionStr),
		Common::WinResourceID(resId));
	assert(file);
	writeBitmap(nameBuffer, file);
}

void writeNumbers() {
	outputFile.seek(dataOffset);

	// Iterate through writing each string
	for (int idx = 0; idx < 76; ++idx) {
		outputFile.writeString(NUMBERS[idx]._text);
		outputFile.writeLong(NUMBERS[idx]._value);
		outputFile.writeLong(NUMBERS[idx]._flags);
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader("TEXT/NUMBERS", dataOffset, size);
	dataOffset += size;
}

void writeString(uint offset) {
	inputFile.seek(offset - FILE_DIFF);
	char c;
	do {
		c = inputFile.readByte();
		outputFile.writeByte(c);
	} while (c);
}

void writeResponseTree() {
	outputFile.seek(dataOffset);
	
	inputFile.seek(0x619500 - FILE_DIFF);
	char buffer[32];
	inputFile.read(buffer, 32);
	if (strcmp(buffer, "ReadInt(): No number to read")) {
		printf("Could not find tree data at expected position\n");
		exit(1);
	}

	for (int idx = 0; idx < 1022; ++idx) {
		inputFile.seek(0x619520 - FILE_DIFF + idx * 8);
		uint id = inputFile.readLong();
		uint offset = inputFile.readLong();

		outputFile.writeLong(id);
		if (!id) {
			// An end of list id
		} else if (offset >= 0x619520 && offset <= 0x61B510) {
			// Offset to another table
			outputFile.writeByte(0);
			outputFile.writeLong((offset - 0x619520) / 8);
		} else {
			// Offset to ASCIIZ string
			outputFile.writeByte(1);
			writeString(offset);
		}
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader("TEXT/TREE", dataOffset, size);
	dataOffset += size;
}

void writeSentenceEntries(const char *name, uint tableOffset) {
	outputFile.seek(dataOffset);

	uint v1, v2, v4, v9, v11, v12, v13;
	uint offset3, offset5, offset6, offset7, offset8, offset10;

	for (uint idx = 0; ; ++idx) {
		inputFile.seek(tableOffset - FILE_DIFF + idx * 0x34);
		v1 = inputFile.readLong();
		if (!v1)
			// Reached end of list
			break;

		// Read data fields
		v2 = inputFile.readLong();
		offset3 = inputFile.readLong();
		v4 = inputFile.readLong();
		offset5 = inputFile.readLong();
		offset6 = inputFile.readLong();
		offset7 = inputFile.readLong();
		offset8 = inputFile.readLong();
		v9 = inputFile.readLong();
		offset10 = inputFile.readLong();
		v11 = inputFile.readLong();
		v12 = inputFile.readLong();
		v13 = inputFile.readLong();

		outputFile.writeLong(v1);
		outputFile.writeLong(v2);
		writeString(offset3);
		outputFile.writeLong(v1);
		writeString(offset5);
		writeString(offset6);
		writeString(offset7);
		writeString(offset8);
		outputFile.writeLong(v9);
		writeString(offset10);
		outputFile.writeLong(v11);
		outputFile.writeLong(v12);
		outputFile.writeLong(v13);
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader(name, dataOffset, size);
	dataOffset += size;
}

void writeWords(const char *name, uint tableOffset, int recordCount = 2) {
	outputFile.seek(dataOffset);
	int recordSize = recordCount * 4;

	uint val, strOffset;
	for (uint idx = 0; ; ++idx) {
		inputFile.seek(tableOffset - FILE_DIFF + idx * recordSize);
		val = inputFile.readLong();
		strOffset = inputFile.readLong();

		if (!val)
			// Reached end of list
			break;

		outputFile.writeLong(val);
		writeString(strOffset);
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader(name, dataOffset, size);
	dataOffset += size;
}

void writeSentenceMappings(const char *name, uint offset, int numValues) {
	inputFile.seek(offset - FILE_DIFF);
	outputFile.seek(dataOffset);

	uint id;
	while ((id = inputFile.readLong()) != 0) {
		outputFile.writeLong(id);

		for (int ctr = 0; ctr < numValues; ++ctr)
			outputFile.writeLong(inputFile.readLong());
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader(name, dataOffset, size);
	dataOffset += size;
}

void writeStarfieldPoints() {
	outputFile.seek(dataOffset);

	inputFile.seek(0x59DE4C - FILE_DIFF);
	uint size = 876 * 12;

	outputFile.write(inputFile, size);
	writeEntryHeader("STARFIELD/POINTS", dataOffset, size);
	dataOffset += size;
}

void writeStarfieldPoints2() {
	outputFile.seek(dataOffset);

	for (int rootCtr = 0; rootCtr < 80; ++rootCtr) {
		inputFile.seek(0x5A2F28 - FILE_DIFF + rootCtr * 8);
		uint offset = inputFile.readUint32LE();
		uint count = inputFile.readUint32LE();

		outputFile.writeLong(count);
		inputFile.seek(offset - FILE_DIFF);
		outputFile.write(inputFile, count * 4 * 4);
	}

	uint size = outputFile.size() - dataOffset;
	outputFile.write(inputFile, size);
	writeEntryHeader("STARFIELD/POINTS2", dataOffset, size);
	dataOffset += size;
}

void writePhrases(const char *name, const CommonPhrase *phrases) {
	for (uint idx = 0; phrases->_str; ++idx, ++phrases) {
		outputFile.seek(dataOffset + idx * 4);
		outputFile.writeString(phrases->_str);
		outputFile.writeLong(phrases->_dialogueId);
		outputFile.writeLong(phrases->_roomNum);
		outputFile.writeLong(phrases->_val1);
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader("Phrases/Bellbot", dataOffset, size);
	dataOffset += size;
}

void writeBarbotFrameRanges() {
	outputFile.seek(dataOffset);

	for (int idx = 0; idx < 60; ++idx) {
		outputFile.writeLong(BARBOT_FRAME_RANGES[idx]._startFrame);
		outputFile.writeLong(BARBOT_FRAME_RANGES[idx]._endFrame);
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader("FRAMES/BARBOT", dataOffset, size);
	dataOffset += size;
}

void writeHeader() {
	// Write out magic string
	const char *MAGIC_STR = "SVTN";
	outputFile.write(MAGIC_STR, 4);

	// Write out version number
	outputFile.writeWord(VERSION_NUMBER);
}

void writeData() {
	writeBitmap("Bitmap", "BACKDROP");
	writeBitmap("Bitmap", "EVILTWIN");
	writeBitmap("Bitmap", "RESTORED");
	writeBitmap("Bitmap", "RESTOREF");
	writeBitmap("Bitmap", "RESTOREU");
	writeBitmap("Bitmap", "STARTD");
	writeBitmap("Bitmap", "STARTF");
	writeBitmap("Bitmap", "STARTU");
	writeBitmap("Bitmap", "TITANIC");
	writeBitmap("Bitmap", 133);
	writeBitmap("Bitmap", 164);
	writeBitmap("Bitmap", 165);

	writeResource("STFONT", 149);
	writeResource("STFONT", 151);
	writeResource("STFONT", 152);
	writeResource("STFONT", 153);

	writeResource("STARFIELD", 132);
	writeStarfieldPoints();
	writeStarfieldPoints2();

	writeResource("TEXT", "STVOCAB.TXT");
	writeResource("TEXT", "JRQUOTES.TXT");
	writeResource("TEXT", 155);

	writeStringArray("TEXT/ITEM_DESCRIPTIONS", ITEM_DESCRIPTIONS, 46);
	writeStringArray("TEXT/ITEM_NAMES", ITEM_NAMES, 46);
	writeStringArray("TEXT/ITEM_IDS", ITEM_IDS, 40);
	writeStringArray("TEXT/ROOM_NAMES", ROOM_NAMES, 34);

	writeStringArray("TEXT/PHRASES", 0x21B7C8, 376);
	writeStringArray("TEXT/REPLACEMENTS1", 0x21BDB0, 218);
	writeStringArray("TEXT/REPLACEMENTS2", 0x21C120, 1576);
	writeStringArray("TEXT/REPLACEMENTS3", 0x21D9C8, 82);
	writeStringArray("TEXT/PRONOUNS", 0x22F718, 15);

	writeSentenceEntries("Sentences/Default", 0x5C0130);
	writeSentenceEntries("Sentences/Barbot", 0x5ABE60);
	writeSentenceEntries("Sentences/Barbot2", 0x5BD4E8);
	writeSentenceEntries("Sentences/Bellbot", 0x5C2230);
	writeSentenceEntries("Sentences/Bellbot/1", 0x5D1670);
	writeSentenceEntries("Sentences/Bellbot/2", 0x5D1A80);
	writeSentenceEntries("Sentences/Bellbot/3", 0x5D1AE8);
	writeSentenceEntries("Sentences/Bellbot/4", 0x5D1B88);
	writeSentenceEntries("Sentences/Bellbot/5", 0x5D2A60);
	writeSentenceEntries("Sentences/Bellbot/6", 0x5D2CD0);
	writeSentenceEntries("Sentences/Bellbot/7", 0x5D3488);
	writeSentenceEntries("Sentences/Bellbot/8", 0x5D3900);
	writeSentenceEntries("Sentences/Bellbot/9", 0x5D3968);
	writeSentenceEntries("Sentences/Bellbot/10", 0x5D4668);
	writeSentenceEntries("Sentences/Bellbot/11", 0x5D47A0);
	writeSentenceEntries("Sentences/Bellbot/12", 0x5D4EC0);
	writeSentenceEntries("Sentences/Bellbot/13", 0x5D5100);
	writeSentenceEntries("Sentences/Bellbot/14", 0x5D5370);
	writeSentenceEntries("Sentences/Bellbot/15", 0x5D5548);
	writeSentenceEntries("Sentences/Bellbot/16", 0x5D56B8);
	writeSentenceEntries("Sentences/Bellbot/17", 0x5D57C0);
	writeSentenceEntries("Sentences/Bellbot/18", 0x5D5B38);
	writeSentenceEntries("Sentences/Bellbot/19", 0x5D61B8);

	writeSentenceEntries("Sentences/Deskbot", 0x5DCD10);
	writeSentenceEntries("Sentences/Deskbot/2", 0x5E8E18);
	writeSentenceEntries("Sentences/Deskbot/3", 0x5E8BA8);
	
	writeSentenceEntries("Sentences/Doorbot", 0x5EC110);
	writeSentenceEntries("Sentences/Doorbot/2", 0x5FD930);
	writeSentenceEntries("Sentences/Doorbot/100", 0x5FD930);
	writeSentenceEntries("Sentences/Doorbot/101", 0x5FE668);
	writeSentenceEntries("Sentences/Doorbot/102", 0x5FDD40);
	writeSentenceEntries("Sentences/Doorbot/107", 0x5FFF08);
	writeSentenceEntries("Sentences/Doorbot/110", 0x5FE3C0);
	writeSentenceEntries("Sentences/Doorbot/111", 0x5FF0C8);
	writeSentenceEntries("Sentences/Doorbot/124", 0x5FF780);
	writeSentenceEntries("Sentences/Doorbot/129", 0x5FFAC0);
	writeSentenceEntries("Sentences/Doorbot/131", 0x5FFC30);
	writeSentenceEntries("Sentences/Doorbot/132", 0x6000E0);

	writeSentenceEntries("Sentences/Liftbot", 0x6026B0);
	writeSentenceEntries("Sentences/MaitreD", 0x60CFD8);
	writeSentenceEntries("Sentences/MaitreD/1", 0x614288);
	writeSentenceEntries("Sentences/Parrot", 0x615858);
	writeSentenceEntries("Sentences/SuccUBus", 0x616698);
	writeSentenceMappings("Mappings/Barbot", 0x5B28A0, 8);
	writeSentenceMappings("Mappings/Bellbot", 0x5CD830, 1);
	writeSentenceMappings("Mappings/Deskbot", 0x5E2BB8, 4);
	writeSentenceMappings("Mappings/Doorbot", 0x5F7950, 4);
	writeSentenceMappings("Mappings/Liftbot", 0x608660, 4);
	writeSentenceMappings("Mappings/MaitreD", 0x6125C8, 1);
	writeSentenceMappings("Mappings/Parrot", 0x615B68, 1);
	writeSentenceMappings("Mappings/SuccUBus", 0x6189F0, 1);
	writeWords("Words/Barbot", 0x5BE2E0);
	writeWords("Words/Bellbot", 0x5D8230);
	writeWords("Words/Deskbot", 0x5EAAA8);
	writeWords("Words/Doorbot", 0x601098, 3);
	writeWords("Words/Liftbot", 0x60C788);
	writePhrases("Phrases/Bellbot", BELLBOT_COMMON_PHRASES);

	writeResponseTree();
	writeNumbers();
	writeAllScriptQuotes();
	writeAllScriptResponses();
	writeAllScriptRanges();
	writeAllTagMappings();
	writeAllUpdateStates();
	writeAllScriptPreResponses();
	writeBarbotFrameRanges();
}

void createScriptMap() {
	Common::File inFile;
	char line[80];
	char c[2];
	c[0] = c[1] = '\0';
	int counter = 0;

	inFile.open("d:\\temp\\map.txt");
	printf("static const TagMapping xxxx_ID_MAP[] = {\n");

	do {
		strcpy(line, "");

		while (!inFile.eof()) {
			c[0] = inFile.readByte();
			if (c[0] == '\n')
				c[0] = ' ';
			else if (c[0] == '\r')
				continue;
			strcat(line, c);
			if (inFile.eof() || strlen(line) == (2 * 9))
				break;
		}

		int v1, v2;
		sscanf(line, "%x %x", &v1, &v2);

		if (counter != 0 && (counter % 3) == 0)
			printf("\r\n");
		if ((counter % 3) == 0)
			printf("\t");

		printf("{ 0x%.5x, 0x%.5x }, ", v1, v2);
		++counter;
	} while (!inFile.eof());

	printf("};\r\n");
	inFile.close();
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
