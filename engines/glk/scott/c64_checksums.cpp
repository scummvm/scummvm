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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
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
	int _imgOffset;
};

static C64Rec g_C64Registry[] = {
	{ BATON_C64,		0x2ab00, 0xc3fc, TYPE_D64, 0, nullptr, nullptr, 0, 0, 0, 0, 0 },		// Mysterious Adventures C64 dsk 1
	{ TIME_MACHINE_C64,	0x2ab00, 0xc3fc, TYPE_D64, 0, nullptr, nullptr, 0, 0, 0, 0, 0 },
	{ ARROW1_C64,		0x2ab00, 0xc3fc, TYPE_D64, 0, nullptr, nullptr, 0, 0, 0, 0, 0 },
	{ ARROW2_C64,		0x2ab00, 0xc3fc, TYPE_D64, 0, nullptr, nullptr, 0, 0, 0, 0, 0 },
	{ PULSAR7_C64,		0x2ab00, 0xc3fc, TYPE_D64, 0, nullptr, nullptr, 0, 0, 0, 0, 0 },
	{ CIRCUS_C64,		0x2ab00, 0xc3fc, TYPE_D64, 0, nullptr, nullptr, 0, 0, 0, 0, 0 },

	{ FEASIBILITY_C64,	0x2ab00, 0x9eaa, TYPE_D64, 0, nullptr, nullptr, 0, 0, 0, 0, 0 },		// Mysterious Adventures C64 dsk 2
	{ AKYRZ_C64,		0x2ab00, 0x9eaa, TYPE_D64, 0, nullptr, nullptr, 0, 0, 0, 0, 0 },
	{ PERSEUS_C64,		0x2ab00, 0x9eaa, TYPE_D64, 0, nullptr, nullptr, 0, 0, 0, 0, 0 },
	{ INDIANS_C64,		0x2ab00, 0x9eaa, TYPE_D64, 0, nullptr, nullptr, 0, 0, 0, 0, 0 },
	{ WAXWORKS_C64,		0x2ab00, 0x9eaa, TYPE_D64, 0, nullptr, nullptr, 0, 0, 0, 0, 0 },
	{ BATON_C64,		0x2ab00, 0x9dca, TYPE_D64, 2, nullptr, nullptr, 0, 0, 0, 0, 0 },

	{ ROBIN_OF_SHERWOOD_C64, 0x2ab00, 0xcf9e, TYPE_D64, 1, nullptr, nullptr, 0, 0x1802, 0xbd27, 0x2000, 0 }, // Robin Of Sherwood D64 * unknown packer
	{ ROBIN_OF_SHERWOOD_C64, 0xb2ef,  0x7c44, TYPE_T64, 1, nullptr, nullptr, 0, 0x9702, 0x9627, 0x2000, 0 }, // Robin Of Sherwood C64 (T64) * TCS Cruncher v2.0
	{ ROBIN_OF_SHERWOOD_C64, 0xb690,  0x7b61, TYPE_T64, 1, nullptr, nullptr, 0, 0x9702, 0x9627, 0x2000, 0 }, // Robin Of Sherwood C64 (T64) alt * TCS Cruncher v2.0
	{ ROBIN_OF_SHERWOOD_C64, 0x8db6,  0x7853, TYPE_T64, 1, nullptr, nullptr, 0, 0xd7fb, 0xbd20, 0x2000, 0 }, // Robin Of Sherwood T64 alt 2 * PUCrunch

	{ GREMLINS_C64,			0xdd94,		0x25a8, TYPE_T64, 1, nullptr,	nullptr, 0, 0, 0, 0, 0 },                                 // Gremlins C64 (T64) version * Action Replay v4.x
	{ GREMLINS_C64,			0x2ab00,	0xc402, TYPE_D64, 0, nullptr,	"G1",	-0x8D, 0, 0, 0, 0 },                               // Gremlins C64 (D64) version
	{ GREMLINS_C64,			0x2ab00,	0x3ccf, TYPE_D64, 0, nullptr,	"G1",	-0x8D, 0, 0, 0, 0 },                               // Gremlins C64 (D64) version 2
	{ GREMLINS_C64,			0x2ab00,	0xabf8, TYPE_D64, 2, "-e0x1255",nullptr, 2, 0, 0, 0, 0 },                             // Gremlins C64 (D64) version alt * ByteBoiler, Exomizer
	{ GREMLINS_C64,			0x2ab00,	0xa265, TYPE_D64, 2, "-e0x1255",nullptr, 2, 0, 0, 0, 0 },                             // Gremlins C64 (D64)  version alt 2 * ByteBoiler, Exomizer
	{ GREMLINS_GERMAN_C64,	0xc003,		0x558c, TYPE_T64, 1, nullptr,	nullptr, 0, 0xd801, 0xc6c0, 0x1f00, 0 },  // German Gremlins C64 (T64) version * TBC Multicompactor v2.x
	{ GREMLINS_GERMAN_C64,	0x2ab00,	0x6729, TYPE_D64, 2, nullptr,	nullptr, 0, 0xdc02, 0xcac1, 0x1f00, 0 }, // German Gremlins C64 (D64) version * Exomizer

	{ SEAS_OF_BLOOD_C64, 0xa209,  0xf115, TYPE_T64, 6, "-e0x1000", nullptr, 3, 0xd802, 0xb07c, 0x2000, 0 }, // Seas of Blood C64 (T64) MasterCompressor / Relax -> ECA
	// Compacker -> Unknown -> MasterCompressor / Relax -> ECA
	// Compacker -> CCS Packer
	{ SEAS_OF_BLOOD_C64, 0x2ab00, 0x5c1d, TYPE_D64, 1, nullptr,	   nullptr, 0, 0xd802, 0xb07c, 0x2000, 0 }, // Seas of Blood C64 (D64) CCS Packer
	{ SEAS_OF_BLOOD_C64, 0x2ab00, 0xe308, TYPE_D64, 1, nullptr,	   nullptr, 0, 0xd802, 0xb07c, 0x2000, 0 }, // Seas of Blood C64 (D64) alt CCS Packer

	{ CLAYMORGUE_C64, 0x6ff7,  0xe4ed, TYPE_T64, 3, nullptr, nullptr, 0, 0x855, 0x7352, 0x20, 0 },		// Sorcerer Of Claymorgue Castle C64 (T64), MasterCompressor / Relax
	// -> ECA Compacker -> MegaByte Cruncher v1.x Missing 17 pictures
	{ CLAYMORGUE_C64, 0x912f,  0xa69f, TYPE_T64, 1, nullptr, nullptr, 0, 0x855, 0x7352, 0x20, 0 },		// Sorcerer Of Claymorgue Castle C64 (T64) alt, MegaByte Cruncher
	// v1.x Missing 17 pictures
	{ CLAYMORGUE_C64, 0xc0dd,  0x3701, TYPE_T64, 1, nullptr, nullptr, 0, 0,		0,		0,	-0x7fe },// Sorcerer Of Claymorgue Castle C64 (T64) alt 2, Trilogic Expert
	// v2.7
	{ CLAYMORGUE_C64, 0xbc5f,  0x492c, TYPE_T64, 1, nullptr, nullptr, 0, 0x855, 0x7352, 0x20, 0 },		// Sorcerer Of Claymorgue Castle C64 (T64) alt 3, , Section8 Packer
	{ CLAYMORGUE_C64, 0x2ab00, 0xfd67, TYPE_D64, 1, nullptr, nullptr, 0, 0x855, 0x7352, 0x20, 0 },		// Sorcerer Of Claymorgue Castle C64 (D64), Section8 Packer

	{ ADVENTURELAND_C64, 0x6a10,  0x1910, TYPE_T64, 1, nullptr, nullptr,	0,		0, 0, 0, 0 },            // Adventureland C64 (T64) CruelCrunch v2.2
	{ ADVENTURELAND_C64, 0x6a10,  0x1b10, TYPE_T64, 1, nullptr, nullptr,	0,		0, 0, 0, 0 },            // Adventureland C64 (T64) alt CruelCrunch v2.2
	{ ADVENTURELAND_C64, 0x2ab00, 0x6638, TYPE_D64, 1, nullptr, nullptr,	0,		0, 0, 0, 0 },            // Adventureland C64 (D64) CruelCrunch v2.2
	{ ADVENTURELAND_C64, 0x2adab, 0x751f, TYPE_D64, 0, nullptr, nullptr,	0,		0, 0, 0, 0 },            // Adventureland C64 (D64) alt
	{ ADVENTURELAND_C64, 0x2adab, 0x64a4, TYPE_D64, 0, nullptr, "SAG1PIC",	-0xa53, 0, 0, 0, 0x65af }, // Adventureland C64 (D64) alt 2

	{ SAVAGE_ISLAND_C64,  0x2ab00, 0x8801, TYPE_D64, 1, "-f86 -d0x1793", "SAVAGEISLAND1+",   1, 0, 0, 0, 0 }, // Savage Island part 1 C64 (D64)
	{ SAVAGE_ISLAND2_C64, 0x2ab00, 0x8801, TYPE_D64, 1, "-f86 -d0x178b", "SAVAGEISLAND2+",   1, 0, 0, 0, 0 }, // Savage Island part 2 C64 (D64)
	{ SAVAGE_ISLAND_C64,  0x2ab00, 0xc361, TYPE_D64, 1, "-f86 -d0x1793", "SAVAGE ISLAND P1", 1, 0, 0, 0, 0 }, // Savage Island part 1 C64 (D64) alt
	{ SAVAGE_ISLAND2_C64, 0x2ab00, 0xc361, TYPE_D64, 1, nullptr,		 "SAVAGE ISLAND P2", 0, 0, 0, 0, 0 }, // Savage Island part 2  C64 (D64) alt

	{ HULK_C64,		 0x2ab00, 0xcdd8, TYPE_D64, 0, nullptr, nullptr, 0, 0x1806, 0xb801, 0x307, 0 },  // Questprobe 1 - The Hulk C64 (D64)
	{ SPIDERMAN_C64, 0x2ab00, 0xde56, TYPE_D64, 0, nullptr, nullptr, 0, 0x1801, 0xa801, 0x2000, 0 }, // Spiderman C64 (D64)
	{ SPIDERMAN_C64, 0x08e72, 0xb2f4, TYPE_T64, 3, nullptr, nullptr, 0, 0, 0, 0, 0 },                // Spiderman C64 (T64) MasterCompressor / Relax -> ECA Compacker -> Section8 Packer

	{ BATON_C64,		0x5170,		0xb240, TYPE_T64, 2, nullptr, nullptr, 0, 0, 0, 0, 0 }, // The Golden Baton C64, T64
	{ BATON_C64,		0x2ab00,	0xbfbf, TYPE_D64, 2, nullptr, nullptr, 0, 0, 0, 0, 0 }, // Mysterious Adventures C64 dsk 1 alt
	{ FEASIBILITY_C64,	0x2ab00,	0x9c18, TYPE_D64, 2, nullptr, nullptr, 0, 0, 0, 0, 0 }, // Mysterious Adventures C64 dsk 2 alt
	{ TIME_MACHINE_C64, 0x5032,		0x5635, TYPE_T64, 1, nullptr, nullptr, 0, 0, 0, 0, 0 }, // The Time Machine C64
	{ ARROW1_C64,		0x5b46,		0x92db, TYPE_T64, 1, nullptr, nullptr, 0, 0, 0, 0, 0 }, // Arrow of Death part 1 C64
	{ ARROW2_C64,		0x5fe2,		0xe14f, TYPE_T64, 1, nullptr, nullptr, 0, 0, 0, 0, 0 }, // Arrow of Death part 2 C64
	{ PULSAR7_C64,		0x46bf,		0x1679, TYPE_T64, 1, nullptr, nullptr, 0, 0, 0, 0, 0 }, // Escape from Pulsar 7 C64
	{ CIRCUS_C64,		0x4269,		0xa449, TYPE_T64, 2, nullptr, nullptr, 0, 0, 0, 0, 0 }, // Circus C64
	{ FEASIBILITY_C64,	0x5a7b,		0x0f48, TYPE_T64, 1, nullptr, nullptr, 0, 0, 0, 0, 0 }, // Feasibility Experiment C64
	{ AKYRZ_C64,		0x2ab00,	0x6cca, TYPE_D64, 0, nullptr, nullptr, 0, 0, 0, 0, 0 }, // The Wizard of Akyrz C64
	{ AKYRZ_C64,		0x4be1,		0x5a00, TYPE_T64, 1, nullptr, nullptr, 0, 0, 0, 0, 0 }, // The Wizard of Akyrz C64, T64
	{ PERSEUS_C64,		0x502b,		0x913b, TYPE_T64, 1, nullptr, nullptr, 0, 0, 0, 0, 0 }, // Perseus and Andromeda C64
	{ INDIANS_C64,		0x4f9f,		0xe6c8, TYPE_T64, 1, nullptr, nullptr, 0, 0, 0, 0, 0 }, // Ten Little Indians C64
	{ WAXWORKS_C64,		0x4a11,		0xa37a, TYPE_T64, 1, nullptr, nullptr, 0, 0, 0, 0, 0 }, // Waxworks C64

	{ SUPERGRAN_C64, 0x726f, 0x0901, TYPE_T64, 1, nullptr, nullptr, 0, 0xd802, 0xc623, 0x1f00, 0 }, // Super Gran C64 (T64) PUCrunch Generic Hack

	{ UNKNOWN_GAME, 0, 0, UNKNOWN_FILE_TYPE, 0, nullptr, nullptr, 0, 0, 0, 0, 0 }
};

int decrunchC64(uint8_t **sf, size_t *extent, C64Rec entry);

uint8_t *getLargestFile(uint8_t *data, int length, int *newlength) {
	uint8_t *file = nullptr;
	*newlength = 0;
	DiskImage *d64 = diCreateFromData(data, length);
	if (d64) {
		RawDirEntry *largest = findLargestFileEntry(d64);
		if (largest) {
			ImageFile *c64file = diOpen(d64, largest->_rawname, largest->_type, "rb");
			if (c64file) {
				uint8_t *largeFile = new uint8_t[0xffff];
				*newlength = diRead(c64file, largeFile, 0xffff);
				file = new uint8_t[*newlength];
				memcpy(file, largeFile, *newlength);
			}
		}
		//di_free_image(d64);
	}
	return file;
}

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

int savageIslandMenu(uint8_t **sf, size_t *extent, int recIndex) {
	g_scott->output("This disk image contains two games. Select one.\n\n"
					"1. Savage Island part I\n"
					"2. Savage Island part II");

	g_scott->glk_request_char_event(_G(_bottomWindow));

	event_t ev;
	int result = 0;
	do {
		g_scott->glk_select(&ev);
		if (ev.type == evtype_CharInput) {
			if (ev.val1 == '1' || ev.val1 == '2') {
				result = ev.val1 - '0';
			} else {
				g_scott->glk_request_char_event(_G(_bottomWindow));
			}
		}

		if (g_vm->shouldQuit())
			return 0;
	} while (result == 0);

	g_scott->glk_window_clear(_G(_bottomWindow));

	recIndex += result - 1;

	C64Rec rec = g_C64Registry[recIndex];
	int length;
	uint8_t *file = getFileNamed(*sf, *extent, &length, rec._appendFile);

	if (file != nullptr) {
		if (rec._chk == 0xc361) {
			if (rec._switches != nullptr) {
				_G(_saveIslandAppendix1) = getFileNamed(*sf, *extent, &_G(_saveIslandAppendix1Length), "SI1PC1");
				_G(_saveIslandAppendix2) = getFileNamed(*sf, *extent, &_G(_saveIslandAppendix2Length), "SI1PC2");
			} else {
				_G(_saveIslandAppendix1) = getFileNamed(*sf, *extent, &_G(_saveIslandAppendix1Length), "SI2PIC");
			}
		}
		delete[] *sf;
		*sf = file;
		*extent = length;
		if (_G(_saveIslandAppendix1Length) > 2)
			_G(_saveIslandAppendix1Length) -= 2;
		if (_G(_saveIslandAppendix2Length) > 2)
			_G(_saveIslandAppendix2Length) -= 2;
		return decrunchC64(sf, extent, rec);
	} else {
		error("savageIslandMenu: Failed loading file %s\n", rec._appendFile);
	}
}

void appendSIfiles(uint8_t **sf, size_t *extent) {
	//int totalLength = *extent + _G(_saveIslandAppendix1Length) + _G(_saveIslandAppendix2Length);

	uint8_t *megabuf = new uint8_t[0xFFFF];
	memcpy(megabuf, *sf, *extent);
	delete[] *sf;
	int offset = 0x6202;

	if (_G(_saveIslandAppendix1)) {
		memcpy(megabuf + offset, _G(_saveIslandAppendix1) + 2, _G(_saveIslandAppendix1Length));
		delete[] _G(_saveIslandAppendix1);
	}
	if (_G(_saveIslandAppendix2)) {
		memcpy(megabuf + offset + _G(_saveIslandAppendix1Length), _G(_saveIslandAppendix2) + 2, _G(_saveIslandAppendix2Length));
		delete[] _G(_saveIslandAppendix2);
	}
	*extent = offset + _G(_saveIslandAppendix1Length) + _G(_saveIslandAppendix2Length);
	*sf = new uint8_t[*extent];
	memcpy(*sf, megabuf, *extent);
	delete[] megabuf;
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

		if (g_vm->shouldQuit())
			return 0;
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
	}

	int length;
	uint8_t *file = getFileNamed(*sf, *extent, &length, filename);

	if (file != nullptr) {
		delete[] *sf;
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

		if (g_vm->shouldQuit())
			return 0;
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
	}

	int length;
	uint8_t *file = getFileNamed(*sf, *extent, &length, filename);

	if (file != nullptr) {
		delete[] *sf;
		*sf = file;
		*extent = length;
		C64Rec rec = g_C64Registry[recindex - 1 + result];
		return decrunchC64(sf, extent, rec);
	} else {
		error("mysteriousMenu2: Failed loading file %s", filename);
	}
}

int detectC64(uint8_t **sf, size_t *extent) {
	if (*extent > MAX_LENGTH || *extent < MIN_LENGTH)
		return 0;

	Common::String md5 = g_vm->getGameMD5();
	int index = _G(_md5Index)[md5];
	if (g_C64Registry[index]._id == SAVAGE_ISLAND_C64) {
		return savageIslandMenu(sf, extent, index);
	} else if (g_C64Registry[index]._id == BATON_C64 && index == 0) {
		return mysteriousMenu(sf, extent, index);
	} else if (g_C64Registry[index]._id == FEASIBILITY_C64 && index == 6) {
		return mysteriousMenu2(sf, extent, index);
	}
	if (g_C64Registry[index]._type == TYPE_D64) {
		int newlength;
		uint8_t *largest_file = getLargestFile(*sf, *extent, &newlength);
		uint8_t *appendix = nullptr;
		int appendixlen = 0;

		if (g_C64Registry[index]._appendFile != nullptr) {
			appendix = getFileNamed(*sf, *extent, &appendixlen, g_C64Registry[index]._appendFile);
			if (appendix == nullptr)
				error("detectC64(): Appending file failed");
			appendixlen -= 2;
		}

		uint8_t *megabuf = new uint8_t[newlength + appendixlen];
		memcpy(megabuf, largest_file, newlength);
		if (appendix != nullptr) {
			memcpy(megabuf + newlength + g_C64Registry[index]._parameter, appendix + 2, appendixlen);
			newlength += appendixlen;
		}
		delete[] appendix;

		if (largest_file) {
			*sf = megabuf;
			*extent = newlength;
		}
		delete[] largest_file;

	} else if (g_C64Registry[index]._type == TYPE_T64) {
		uint8_t *file_records = *sf + 64;
		int number_of_records = READ_LE_UINT16(&(*sf)[36]);
		int offset = READ_LE_UINT16(&file_records[8]);
		int start_addr = READ_LE_UINT16(&file_records[2]);
		int end_addr = READ_LE_UINT16(&file_records[4]);
		int size;
		if (number_of_records == 1)
			size = *extent - offset;
		else
			size = end_addr - start_addr;
		uint8_t *first_file = new uint8_t[size + 2];
		memcpy(first_file + 2, *sf + offset, size);
		memcpy(first_file, file_records + 2, 2);
		*sf = first_file;
		*extent = size + 2;
	}
	return decrunchC64(sf, extent, g_C64Registry[index]);
}

size_t copyData(size_t dest, size_t source, uint8_t** data, size_t dataSize, size_t bytesToMove) {
	if (source > dataSize || *data == nullptr)
		return 0;

	size_t newSize = MAX(dest + bytesToMove, dataSize);
	uint8_t *megaBuf = new uint8_t[newSize];
	memcpy(megaBuf, *data, dataSize);
	memcpy(megaBuf + dest, *data + source, bytesToMove);
	delete[] *data;
	*data = megaBuf;
	return newSize;
}

int decrunchC64(uint8_t **sf, size_t *extent, C64Rec record) {
	uint8_t *uncompressed = nullptr;
	_G(_fileLength) = *extent;

	size_t decompressedLength = *extent;

	uncompressed = new uint8_t[0xffff];

	size_t result = 0;

	for (int i = 1; i <= record._decompressIterations; i++) {
		/* We only send switches on the iteration specified by parameter */
		if (i == record._parameter && record._switches != nullptr) {
			result = unp64(_G(_entireFile), _G(_fileLength), uncompressed, &decompressedLength, record._switches);
		} else
			result = unp64(_G(_entireFile), _G(_fileLength), uncompressed, &decompressedLength, nullptr);
		if (result) {
			if (_G(_entireFile) != nullptr)
				delete[] _G(_entireFile);
			_G(_entireFile) = new uint8_t[decompressedLength];
			memcpy(_G(_entireFile), uncompressed, decompressedLength);
			_G(_fileLength) = decompressedLength;
		} else {
			delete[] uncompressed;
			uncompressed = nullptr;
			break;
		}
	}

	if (uncompressed != nullptr)
		delete[] uncompressed;

	for (int i = 0; i < NUMGAMES; i++) {
		if (_G(_games)[i]._gameID == record._id) {
			_G(_game) = &_G(_games)[i];
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

	if (_G(_saveIslandAppendix1) != nullptr) {
		appendSIfiles(sf, extent);
	}

	if (record._copySource != 0) {
		result = copyData(record._copyDest, record._copySource, sf, *extent, record._copySize);
		if (result) {
			*extent = result;
		}
	}

	if (CURRENT_GAME == CLAYMORGUE_C64 && record._copySource == 0x855) {
		result = copyData(0x1531a, 0x2002, sf, *extent, 0x2000);
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
