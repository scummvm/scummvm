/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>

#include "eden.h"
#include "eden_icons.h"
#include "eden_rooms.h"
#include "eden_static.h"

#define CRYO_DAT_VER 1	// 32-bit integer

template <typename T>
static void writeLE(FILE *f, T value) {
	for (unsigned int i = 0; i < sizeof(value); i++, value >>= 8) {
		unsigned char b = value & 0xFF;
		fwrite(&b, 1, 1, f);
	}
}

static void writeByte(FILE *f, byte value) {
	fwrite(&value, 1, 1, f);
}

struct _icon_t : icon_t {
	void write(FILE *f) const {
		writeLE<int16>(f, sx);
		writeLE<int16>(f, sy);
		writeLE<int16>(f, ex);
		writeLE<int16>(f, ey);
		writeLE<uint16>(f, cursor_id);
		writeLE<unsigned int>(f, action_id);
		writeLE<unsigned int>(f, object_id);
	}
};

static void emitIcons(FILE *f) {
	const _icon_t *icons = (const _icon_t*)gameIcons;
	for (int i = 0; i < kNumIcons; i++)
		icons[i].write(f);
}

struct _room_t : room_t {
	void write(FILE *f) const {
		writeByte(f, ff_0);
		writeByte(f, exits[0]);
		writeByte(f, exits[1]);
		writeByte(f, exits[2]);
		writeByte(f, exits[3]);
		writeByte(f, flags);
		writeLE<uint16>(f, bank);
		writeLE<uint16>(f, party);
		writeByte(f, level);
		writeByte(f, video);
		writeByte(f, location);
		writeByte(f, background);
	}
};

static void emitRooms(FILE *f) {
	const _room_t *rooms = (const _room_t*)gameRooms;
	for (int i = 0; i < kNumRooms; i++)
		rooms[i].write(f);
}

static void emitStatic(FILE *f) {
	const int kNumFollowers = 15;
	const int kNumLabyrinthPath = 70;
	const int kNumDinoSpeedForCitaLevel = 16;
	const int kNumTabletView = 12;
	const int kNumPersoRoomBankTable = 84;
	const int kNumGotos = 130;
	const int kNumObjects = 42;
	const int kNumObjectLocations = 45;
	const int kNumPersons = 58;
	const int kNumCitadel = 7;
	const int kNumCharacterRects = 19;
	const int kNumCharacters = 20;
	const int kNumActionCursors = 299;
	const int kNumAreas = 12;

	for (int i = 0; i < kNumFollowers; i++) {
		writeByte(f, followerList[i]._id);
		writeByte(f, followerList[i]._spriteNum);
		writeLE<int16>(f, followerList[i].sx);
		writeLE<int16>(f, followerList[i].sy);
		writeLE<int16>(f, followerList[i].ex);
		writeLE<int16>(f, followerList[i].ey);
		writeLE<int16>(f, followerList[i]._spriteBank);
		writeLE<int16>(f, followerList[i].ff_C);
		writeLE<int16>(f, followerList[i].ff_E);
	}

	fwrite(kLabyrinthPath, 1, kNumLabyrinthPath, f);
	fwrite(kDinoSpeedForCitaLevel, 1, kNumDinoSpeedForCitaLevel, f);
	fwrite(kTabletView, 1, kNumTabletView, f);
	fwrite(kPersoRoomBankTable, 1, kNumPersoRoomBankTable, f);
	fwrite(gotos, 5, kNumGotos, f);	// sizeof(Goto)

	for (int i = 0; i < kNumObjects; i++) {
		writeByte(f, _objects[i]._id);
		writeByte(f, _objects[i]._flags);
		writeLE<int>(f, _objects[i]._locations);
		writeLE<uint16>(f, _objects[i]._itemMask);
		writeLE<uint16>(f, _objects[i]._powerMask);
		writeLE<int16>(f, _objects[i]._count);
	}

	for (int i = 0; i < kNumObjectLocations; i++) {
		writeLE<uint16>(f, kObjectLocations[i]);
	}

	for (int i = 0; i < kNumPersons; i++) {
		writeLE<uint16>(f, kPersons[i]._roomNum);
		writeLE<uint16>(f, kPersons[i]._actionId);
		writeLE<uint16>(f, kPersons[i]._partyMask);
		writeByte(f, kPersons[i]._id);
		writeByte(f, kPersons[i]._flags);
		writeByte(f, kPersons[i]._roomBankId);
		writeByte(f, kPersons[i]._spriteBank);
		writeLE<uint16>(f, kPersons[i]._items);
		writeLE<uint16>(f, kPersons[i]._powers);
		writeByte(f, kPersons[i]._targetLoc);
		writeByte(f, kPersons[i]._lastLoc);
		writeByte(f, kPersons[i]._speed);
		writeByte(f, kPersons[i]._steps);
	}

	for (int i = 0; i < kNumCitadel; i++) {
		writeLE<int16>(f, _citadelList[i]._id);
		for (int j = 0; j < 8; j++)
			writeLE<int16>(f, _citadelList[i]._bank[j]);
		for (int j = 0; j < 8; j++)
			writeLE<int16>(f, _citadelList[i]._video[j]);
	}

	for (int i = 0; i < kNumCharacterRects; i++) {
		writeLE<int16>(f, _characterRects[i].left);
		writeLE<int16>(f, _characterRects[i].top);
		writeLE<int16>(f, _characterRects[i].right);
		writeLE<int16>(f, _characterRects[i].bottom);
	}

	fwrite(_characterArray, 5, kNumCharacters, f);

	for (int i = 0; i < kNumAreas; i++) {
		writeByte(f, kAreasTable[i]._num);
		writeByte(f, kAreasTable[i]._type);
		writeLE<uint16>(f, kAreasTable[i]._flags);
		writeLE<uint16>(f, kAreasTable[i]._firstRoomIdx);
		writeByte(f, kAreasTable[i]._citadelLevel);
		writeByte(f, kAreasTable[i]._placeNum);
		// pointer to _citadelRoomPtr is always initialized to null
		writeLE<int16>(f, kAreasTable[i]._visitCount);
	}

	for (int i = 0; i < 64; i++) {
		writeLE<uint16>(f, tab_2CEF0[i]);
	}

	for (int i = 0; i < 64; i++) {
		writeLE<uint16>(f, tab_2CF70[i]);
	}

	fwrite(kActionCursors, 1, kNumActionCursors, f);
	fwrite(mapMode, 1, 12, f);
	fwrite(cubeTextureCoords, 6 * 2 * 3 * 2, 3, f);
}

static int emitData(char *outputFilename) {
	FILE *f = fopen(outputFilename, "w+b");
	if (!f) {
		printf("ERROR: Unable to create output file %s\n", outputFilename);
		return 1;
	}

	printf("Generating %s...\n", outputFilename);

	fwrite("CRYODATA", 8, 1, f);
	writeLE<uint32>(f, CRYO_DAT_VER);

	emitIcons(f);
	emitRooms(f);
	emitStatic(f);

	fclose(f);

	printf("Done!\n");

	return 0;
}

int main(int argc, char **argv) {

	if (argc > 1)
		return emitData(argv[1]);
	else
		printf("Usage: %s <output.dat>\n", argv[0]);

	return 0;
}
