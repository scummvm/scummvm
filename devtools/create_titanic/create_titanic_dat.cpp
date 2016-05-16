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
#define HEADER_SIZE 0x290

Common::File inputFile, outputFile;
Common::PEResources res;
uint headerOffset = 6;
uint dataOffset = HEADER_SIZE;
#define SEGMENT_OFFSET 0x401C00

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

void writeHeader() {
	// Write out magic string
	const char *MAGIC_STR = "SVTN";
	outputFile.write(MAGIC_STR, 4);

	// Write out version number
	outputFile.writeWord(VERSION_NUMBER);
}

void writeData() {
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

	writeStringArray("TEXT/ITEM_DESCRIPTIONS", ITEM_DESCRIPTIONS, 46);
	writeStringArray("TEXT/ITEM_NAMES", ITEM_NAMES, 46);
	writeStringArray("TEXT/ITEM_IDS", ITEM_IDS, 40);
	writeStringArray("TEXT/ROOM_NAMES", ROOM_NAMES, 34);

	writeStringArray("TEXT/PHRASES", 0x21B7C8, 376);
	writeStringArray("TEXT/REPLACEMENTS1", 0x21BDB0, 218);
	writeStringArray("TEXT/REPLACEMENTS2", 0x21C120, 1576);
	writeStringArray("TEXT/REPLACEMENTS3", 0x21D9C8, 82);
	writeNumbers();
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
