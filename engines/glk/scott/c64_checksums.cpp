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

#include "common/str.h"
#include "common/scummsys.h"
#include "glk/scott/scott.h"
#include "glk/scott/globals.h"
#include "glk/scott/c64_checksums.h"
#include "glk/scott/definitions.h"
#include "glk/scott/disk_image.h"
#include "glk/scott/game_info.h"
#include "glk/scott/resource.h"
#include "glk/scott/saga_draw.h"
#include "glk/scott/unp64/unp64_interface.h"

namespace Glk {
namespace Scott {

#define MAX_LENGTH 300000
#define MIN_LENGTH 24

enum FileType {
	UNKNOWN_FILE_TYPE,
	TYPE_D64,
	TYPE_T64
};

struct C64Rec {
	GameIDType _id;
	size_t _length;
	uint16_t _chk;
	FileType _type;
	int _decompressIterations;
	const char *_switches;
	const char *_appendFile;
	int _parameter;
	size_t _copySource;
	size_t _copyDest;
	size_t _copySize;
	size_t _imgOffset;
};

static C64Rec g_C64Registry[] = {
	{ BATON_C64,		0x2ab00, 0xc3fc, TYPE_D64, 0 },		// Mysterious Adventures C64 dsk 1
	{ TIME_MACHINE_C64,	0x2ab00, 0xc3fc, TYPE_D64, 0 },
	{ ARROW1_C64,		0x2ab00, 0xc3fc, TYPE_D64, 0 },
	{ ARROW2_C64,		0x2ab00, 0xc3fc, TYPE_D64, 0 },
	{ PULSAR7_C64,		0x2ab00, 0xc3fc, TYPE_D64, 0 },
	{ CIRCUS_C64,		0x2ab00, 0xc3fc, TYPE_D64, 0 },

	{ FEASIBILITY_C64,	0x2ab00, 0x9eaa, TYPE_D64, 0 },		// Mysterious Adventures C64 dsk 2
	{ AKYRZ_C64,		0x2ab00, 0x9eaa, TYPE_D64, 0 },
	{ PERSEUS_C64,		0x2ab00, 0x9eaa, TYPE_D64, 0 },
	{ INDIANS_C64,		0x2ab00, 0x9eaa, TYPE_D64, 0 },
	{ WAXWORKS_C64,		0x2ab00, 0x9eaa, TYPE_D64, 0 },
	{ BATON_C64,		0x2ab00, 0x9dca, TYPE_D64, 2 },

	{ UNKNOWN_GAME, 0, 0, UNKNOWN_FILE_TYPE, 0, nullptr, nullptr, 0, 0, 0, 0 }
};

int decrunchC64(uint8_t **sf, size_t *extent, C64Rec entry);

uint8_t *getFileNamed(uint8_t* data, int length, int* newLength, const char* name) {
	uint8_t *file = nullptr;
	*newLength = 0;
	DiskImage *d64 = diCreateFromData(data, length);
	byte rawname[100];
	diRawnameFromName(rawname, name);
	if (d64) {
		ImageFile *c64file = diOpen(d64, rawname, 0xC2, "rb");
		if (c64file) {
			uint8_t *buf = new uint8_t[0xffff];
			*newLength = diRead(c64file, buf, 0xffff);
			file = new uint8_t[*newLength];
			memcpy(file, buf, *newLength);
			delete[] buf;
		}
	}
	return file;
}

int mysteriousMenu(uint8_t **sf, size_t *extent, int recindex) {
	recindex = 0;

	g_scott->output("This disk image contains six games. Select one.\n\n"
					"1. The Golden Baton\n"
					"2. The Time Machine\n"
					"3. Arrow of Death part 1\n"
					"4. Arrow of Death part 2\n"
					"5. Escape from Pulsar 7\n"
					"6. Circus");

	g_scott->glk_request_char_event(_G(_bottomWindow));

	event_t ev;
	int result = 0;
	do {
		g_scott->glk_select(&ev);
		if (ev.type == evtype_CharInput) {
			if (ev.val1 >= '1' && ev.val1 <= '6') {
				result = ev.val1 - '0';
			} else {
				g_scott->glk_request_char_event(_G(_bottomWindow));
			}
		}
	} while (result == 0);

	g_scott->glk_window_clear(_G(_bottomWindow));

	const char *filename = nullptr;
	switch (result) {
	case 1:
		filename = "BATON";
		break;
	case 2:
		filename = "TIME MACHINE";
		break;
	case 3:
		filename = "ARROW I";
		break;
	case 4:
		filename = "ARROW II";
		break;
	case 5:
		filename = "PULSAR 7";
		break;
	case 6:
		filename = "CIRCUS";
		break;
	default:
		error("mysteriousMenu: Unknown Game");
		break;
	}

	int length;
	uint8_t *file = getFileNamed(*sf, *extent, &length, filename);

	if (file != nullptr) {
		delete[] * sf;
		*sf = file;
		*extent = length;
		C64Rec rec = g_C64Registry[recindex - 1 + result];
		return decrunchC64(sf, extent, rec);
	} else {
		error("mysteriousMenu: Failed loading file %s", filename);
		return 0;
	}
}

int mysteriousMenu2(uint8_t **sf, size_t *extent, int recindex) {
	recindex = 6;

	g_scott->output("This disk image contains five games. Select one.\n\n"
					"1. Feasibility Experiment\n"
					"2. The Wizard of Akyrz\n"
					"3. Perseus and Andromeda\n"
					"4. Ten Little Indians\n"
					"5. Waxworks");

	g_scott->glk_request_char_event(_G(_bottomWindow));

	event_t ev;
	int result = 0;
	do {
		g_scott->glk_select(&ev);
		if (ev.type == evtype_CharInput) {
			if (ev.val1 >= '1' && ev.val1 <= '5') {
				result = ev.val1 - '0';
			} else {
				g_scott->glk_request_char_event(_G(_bottomWindow));
			}
		}
	} while (result == 0);

	g_scott->glk_window_clear(_G(_bottomWindow));

	const char *filename = nullptr;
	switch (result) {
	case 1:
		filename = "EXPERIMENT";
		break;
	case 2:
		filename = "WIZARD OF AKYRZ";
		break;
	case 3:
		filename = "PERSEUS";
		break;
	case 4:
		filename = "INDIANS";
		break;
	case 5:
		filename = "WAXWORKS";
		break;
	default:
		error("mysteriousMenu2: Unknown Game");
		break;
	}

	int length;
	uint8_t *file = getFileNamed(*sf, *extent, &length, filename);

	if (file != nullptr) {
		delete[] * sf;
		*sf = file;
		*extent = length;
		C64Rec rec = g_C64Registry[recindex - 1 + result];
		return decrunchC64(sf, extent, rec);
	} else {
		error("mysteriousMenu2: Failed loading file %s", filename);
		return 0;
	}
}

int detectC64(uint8_t **sf, size_t *extent) {
	if (*extent > MAX_LENGTH || *extent < MIN_LENGTH)
		return 0;

	Common::String md5 = g_vm->getGameMD5();
	int index = _G(_md5Index)[md5];
	if (g_C64Registry[index]._id == BATON_C64) {
		return mysteriousMenu(sf, extent, index);
	} else if (g_C64Registry[index]._id == FEASIBILITY_C64) {
		return mysteriousMenu2(sf, extent, index);
	}
	return decrunchC64(sf, extent, g_C64Registry[index]);
}

size_t copyData(size_t dest, size_t source, uint8_t** data, size_t dataSize, size_t bytesToMove) {
	return 0;
}

int decrunchC64(uint8_t **sf, size_t *extent, C64Rec record) {
	uint8_t *uncompressed = nullptr;
	_G(_fileLength) = *extent;

	size_t decompressedLength = *extent;

	uncompressed = new uint8_t[0xffff];

	char *switches[3];
	int numSwitches = 0;

	if (record._switches != nullptr) {
		char string[100];
		strcpy(string, record._switches);
		switches[numSwitches] = strtok(string, " ");

		while (switches[numSwitches] != nullptr)
			switches[++numSwitches] = strtok(nullptr, " ");
	}

	size_t result = 0;

	for (int i = 1; i <= record._decompressIterations; i++) {
		/* We only send switches on the iteration specified by parameter */
		if (i == record._parameter && record._switches != nullptr) {
			result = unp64(_G(_entireFile), _G(_fileLength), uncompressed, &decompressedLength, switches, numSwitches);
		} else
			result = unp64(_G(_entireFile), _G(_fileLength), uncompressed, &decompressedLength, nullptr, 0);
		if (result) {
			if (_G(_entireFile) != nullptr)
				delete[] _G(_entireFile);
			_G(_entireFile) = new uint8_t[decompressedLength];
			memcpy(_G(_entireFile), uncompressed, decompressedLength);
			_G(_fileLength) = decompressedLength;
		} else {
			delete[] uncompressed;
			break;
		}
	}

	for (int i = 0; i < NUMGAMES; i++) {
		if (g_games[i]._gameID == record._id) {
			delete _G(_game);
			_G(_game) = &g_games[i];
			break;
		}
	}

	if (_G(_game)->_title == nullptr) {
		error("decrunchC64: Game not found");
	}

	int offset;

	DictionaryType dictype = getId(&offset);
	if (dictype != _G(_game)->_dictionary) {
		error("decrunchC64: Wrong game?");
	}

	if (!tryLoading(*_G(_game), offset, 0)) {
		error("decrunchC64: Game could not be read");
	}

	if (record._copySource != 0) {
		result = copyData(record._copyDest, record._copySource, sf, *extent, record._copySize);
		if (result) {
			*extent = result;
		}
	}

	if (!(_G(_game)->_subType & MYSTERIOUS))
		sagaSetup(record._imgOffset);

	return CURRENT_GAME;
}

} // End of namespace Scott
} // End of namespace Glk
