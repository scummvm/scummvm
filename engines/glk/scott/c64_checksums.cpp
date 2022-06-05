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

#include "glk/scott/scott.h"
#include "glk/scott/c64_checksums.h"
#include "glk/scott/definitions.h"

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

	{ BATON_C64,		0x5170,	 0xb240, TYPE_T64, 2 },		// The Golden Baton C64, T64
	{ BATON_C64,		0x2ab00, 0xbfbf, TYPE_D64, 2 },		// Mysterious Adventures C64 dsk 1 alt
	{ FEASIBILITY_C64,	0x2ab00, 0x9c18, TYPE_D64, 2 },		// Mysterious Adventures C64 dsk 2 alt
	{ TIME_MACHINE_C64,	0x5032,  0x5635, TYPE_T64, 1 },		// The Time Machine C64
	{ ARROW1_C64,		0x5b46,  0x92db, TYPE_T64, 1 },		// Arrow of Death part 1 C64
	{ ARROW2_C64,		0x5fe2,  0xe14f, TYPE_T64, 1 },		// Arrow of Death part 2 C64
	{ PULSAR7_C64,		0x46bf,  0x1679, TYPE_T64, 1 },		// Escape from Pulsar 7 C64
	{ CIRCUS_C64,		0x4269,  0xa449, TYPE_T64, 2 },		// Circus C64
	{ FEASIBILITY_C64,	0x5a7b,  0x0f48, TYPE_T64, 1 },		// Feasibility Experiment C64
	{ AKYRZ_C64,		0x2ab00, 0x6cca, TYPE_D64, 0 },		// The Wizard of Akyrz C64
	{ AKYRZ_C64,		0x4be1,  0x5a00, TYPE_T64, 1 },		// The Wizard of Akyrz C64, T64
	{ PERSEUS_C64,		0x502b,  0x913b, TYPE_T64, 1 },		// Perseus and Andromeda C64
	{ INDIANS_C64,		0x4f9f,  0xe6c8, TYPE_T64, 1 },		// Ten Little Indians C64
	{ WAXWORKS_C64,		0x4a11,  0xa37a, TYPE_T64, 1 },		// Waxworks C64

	{ ADVENTURELAND_C64, 0x6a10,  0x1910, TYPE_T64, 1 },												  // Adventureland C64 (T64) CruelCrunch v2.2
	{ ADVENTURELAND_C64, 0x6a10,  0x1b10, TYPE_T64, 1, nullptr, nullptr,	   0,	   0, 0 },            // Adventureland C64 (T64) alt CruelCrunch v2.2
	{ ADVENTURELAND_C64, 0x2ab00, 0x6638, TYPE_D64, 1, nullptr, nullptr,	   0,	   0, 0 },            // Adventureland C64 (D64) CruelCrunch v2.2
	{ ADVENTURELAND_C64, 0x2adab, 0x751f, TYPE_D64, 0, nullptr, nullptr,	   0,	   0, 0 },            // Adventureland C64 (D64) alt
	{ ADVENTURELAND_C64, 0x2adab, 0x64a4, TYPE_D64, 0, nullptr, "SAG1PIC",	   -0xa53, 0, 0, 0, 0x65af }, // Adventureland C64 (D64) alt 2

	{ SECRET_MISSION_C64, 0x88be,  0xa122, TYPE_T64, 1, nullptr, nullptr, 0, 0, 0 },             // Secret Mission  C64 (T64) Section8 Packer
	{ SECRET_MISSION_C64, 0x2ab00, 0x04d6, TYPE_D64, 0, nullptr, nullptr, 0, 0, 0, 0, -0x1bff }, // Secret Mission  C64 (D64)

	{ CLAYMORGUE_C64, 0x6ff7,  0xe4ed, TYPE_T64, 3, nullptr, nullptr, 0, 0x855, 0x7352, 0x20 },		// Sorcerer Of Claymorgue Castle C64 (T64), MasterCompressor / Relax
	// -> ECA Compacker -> MegaByte Cruncher v1.x Missing 17 pictures
	{ CLAYMORGUE_C64, 0x912f,  0xa69f, TYPE_T64, 1, nullptr, nullptr, 0, 0x855, 0x7352, 0x20 },		// Sorcerer Of Claymorgue Castle C64 (T64) alt, MegaByte Cruncher
	// v1.x Missing 17 pictures
	{ CLAYMORGUE_C64, 0xc0dd,  0x3701, TYPE_T64, 1, nullptr, nullptr, 0, 0,	    0,	    0, -0x7fe },// Sorcerer Of Claymorgue Castle C64 (T64) alt 2, Trilogic Expert
	// v2.7
	{ CLAYMORGUE_C64, 0xbc5f,  0x492c, TYPE_T64, 1, nullptr, nullptr, 0, 0x855, 0x7352, 0x20 },		// Sorcerer Of Claymorgue Castle C64 (T64) alt 3, , Section8 Packer
	{ CLAYMORGUE_C64, 0x2ab00, 0xfd67, TYPE_D64, 1, nullptr, nullptr, 0, 0x855, 0x7352, 0x20 },		// Sorcerer Of Claymorgue Castle C64 (D64), Section8 Packer

	{ HULK_C64,		 0x2ab00, 0xcdd8, TYPE_D64, 0, nullptr, nullptr, 0, 0x1806, 0xb801, 0x307 },		// Questprobe 1 - The Hulk C64 (D64)
	{ SPIDERMAN_C64, 0x2ab00, 0xde56, TYPE_D64, 0, nullptr, nullptr, 0, 0x1801, 0xa801, 0x2000 },		// Spiderman C64 (D64)
	{ SPIDERMAN_C64, 0x08e72, 0xb2f4, TYPE_T64, 3, nullptr, nullptr, 0, 0,	    0 },					// Spiderman C64 (T64) MasterCompressor / Relax -> ECA Compacker ->
	// Section8 Packer

	{ SAVAGE_ISLAND_C64,  0x2ab00, 0x8801, TYPE_D64, 1, "-f86 -d0x1793", "SAVAGEISLAND1+",   1, 0, 0 },		// Savage Island part 1 C64 (D64)
	{ SAVAGE_ISLAND2_C64, 0x2ab00, 0x8801, TYPE_D64, 1, "-f86 -d0x178b", "SAVAGEISLAND2+",   1, 0, 0 },		// Savage Island part 2 C64 (D64)
	{ SAVAGE_ISLAND_C64,  0x2ab00, 0xc361, TYPE_D64, 1, "-f86 -d0x1793", "SAVAGE ISLAND P1", 1, 0, 0 },		// Savage Island part 1 C64 (D64) alt
	{ SAVAGE_ISLAND2_C64, 0x2ab00, 0xc361, TYPE_D64, 1, nullptr,			"SAVAGE ISLAND P2", 0, 0, 0 },  // Savage Island part 2  C64 (D64) alt

	{ ROBIN_OF_SHERWOOD_C64, 0x2ab00, 0xcf9e, TYPE_D64, 1, nullptr, nullptr, 0, 0x1802, 0xbd27, 0x2000 }, // Robin Of Sherwood D64 * unknown packer
	{ ROBIN_OF_SHERWOOD_C64, 0xb2ef,  0x7c44, TYPE_T64, 1, nullptr, nullptr, 0, 0x9702, 0x9627, 0x2000 }, // Robin Of Sherwood C64 (T64) * TCS Cruncher v2.0
	{ ROBIN_OF_SHERWOOD_C64, 0xb690,  0x7b61, TYPE_T64, 1, nullptr, nullptr, 0, 0x9702, 0x9627, 0x2000 }, // Robin Of Sherwood C64 (T64) alt * TCS Cruncher v2.0
	{ ROBIN_OF_SHERWOOD_C64, 0x8db6,  0x7853, TYPE_T64, 1, nullptr, nullptr, 0, 0xd7fb, 0xbd20, 0x2000 }, // Robin Of Sherwood T64 alt 2 * PUCrunch

	{ GREMLINS_C64,			0xdd94,  0x25a8, TYPE_T64, 1, nullptr,	  nullptr, 0 },							 // Gremlins C64 (T64) version * Action Replay v4.x
	{ GREMLINS_C64,			0x2ab00, 0xc402, TYPE_D64, 0, nullptr,	  "G1",	   -0x8D },                      // Gremlins C64 (D64) version
	{ GREMLINS_C64,			0x2ab00, 0x3ccf, TYPE_D64, 0, nullptr,	  "G1",    -0x8D },                      // Gremlins C64 (D64) version 2
	{ GREMLINS_C64,			0x2ab00, 0xabf8, TYPE_D64, 2, "-e0x1255", nullptr,  2 },                         // Gremlins C64 (D64) version alt * ByteBoiler, Exomizer
	{ GREMLINS_C64,			0x2ab00, 0xa265, TYPE_D64, 2, "-e0x1255", nullptr,  2 },                         // Gremlins C64 (D64)  version alt 2 * ByteBoiler, Exomizer
	{ GREMLINS_GERMAN_C64,	0xc003,  0x558c, TYPE_T64, 1, nullptr,	  nullptr,  0, 0xd801, 0xc6c0, 0x1f00 }, // German Gremlins C64 (T64) version * TBC Multicompactor v2.x
	{ GREMLINS_GERMAN_C64,	0x2ab00, 0x6729, TYPE_D64, 2, nullptr,	  nullptr,  0, 0xdc02, 0xcac1, 0x1f00 }, // German Gremlins C64 (D64) version * Exomizer

	{ SUPERGRAN_C64,			0x726f,  0x0901, TYPE_T64, 1, nullptr,	  nullptr,  0, 0xd802, 0xc623, 0x1f00 }, // Super Gran C64 (T64) PUCrunch Generic Hack

	{ SEAS_OF_BLOOD_C64,		0xa209,  0xf115, TYPE_T64, 6, "-e0x1000", nullptr,  3, 0xd802, 0xb07c, 0x2000 }, // Seas of Blood C64 (T64) MasterCompressor / Relax -> ECA
	// Compacker -> Unknown -> MasterCompressor / Relax -> ECA
	// Compacker -> CCS Packer
	{ SEAS_OF_BLOOD_C64,		0x2ab00, 0x5c1d, TYPE_D64, 1, nullptr,	  nullptr,  0, 0xd802, 0xb07c, 0x2000 }, // Seas of Blood C64 (D64) CCS Packer
	{ SEAS_OF_BLOOD_C64,		0x2ab00, 0xe308, TYPE_D64, 1, nullptr,	  nullptr,  0, 0xd802, 0xb07c, 0x2000 }, // Seas of Blood C64 (D64) alt CCS Packer

	{ UNKNOWN_GAME, 0, 0, UNKNOWN_FILE_TYPE, 0, nullptr, nullptr, 0, 0, 0, 0 }
};

static uint16_t checksum(byte *sf, uint32_t extent) {
	return 0;
}

int decrunchC64(uint8_t **sf, size_t *extent, C64Rec entry);

uint8_t *getLargestFile(uint8_t *data, int length, int *newlength) {
	return nullptr;
}

uint8_t *getFileNamed(uint8_t *data, int length, int *newlength, const char *name) {
	return nullptr;
}

int savageIslandMenu(uint8_t **sf, size_t *extent, int recindex) {
	return 0;
}

int mysteriousMenu(uint8_t **sf, size_t *extent, int recindex) {
	return 0;
}

int mysteriousMenu2(uint8_t **sf, size_t *extent, int recindex) {
	return 0;
}

int detectC64(uint8_t **sf, size_t *extent) {
	if (*extent > MAX_LENGTH || *extent < MIN_LENGTH)
		return 0;

	uint16_t chksum = checksum(*sf, *extent);

	for (int i = 0; g_C64Registry[i]._length != 0; i++) {
		if (*extent == g_C64Registry[i]._length && chksum == g_C64Registry[i]._chk) {
			if (g_C64Registry[i]._id == SAVAGE_ISLAND_C64) {
				return savageIslandMenu(sf, extent, i);
			} else if (g_C64Registry[i]._id == BATON_C64 && (chksum == 0xc3fc || chksum == 0xbfbf)) {
				return mysteriousMenu(sf, extent, i);
			} else if (g_C64Registry[i]._id == FEASIBILITY_C64 && (chksum == 0x9eaa || chksum == 0x9c18)) {
				return mysteriousMenu2(sf, extent, i);
			}
			if (g_C64Registry[i]._type == TYPE_D64) {
				int newlength;
				uint8_t *largest_file = getLargestFile(*sf, *extent, &newlength);
				uint8_t *appendix = nullptr;
				int appendixlen = 0;

				if (g_C64Registry[i]._appendFile != nullptr) {
					appendix = getFileNamed(*sf, *extent, &appendixlen, g_C64Registry[i]._appendFile);
					if (appendix == nullptr)
						error("detectC64(): Appending file failed");
					appendixlen -= 2;
				}

				uint8_t *megabuf = new uint8_t[newlength + appendixlen];
				memcpy(megabuf, largest_file, newlength);
				if (appendix != nullptr) {
					memcpy(megabuf + newlength + g_C64Registry[i]._parameter, appendix + 2, appendixlen);
					newlength += appendixlen;
				}

				if (largest_file) {
					*sf = megabuf;
					*extent = newlength;
				}

			} else if (g_C64Registry[i]._type == TYPE_T64) {
				uint8_t *file_records = *sf + 64;
				int number_of_records = (*sf)[36] + (*sf)[37] * 0x100;
				int offset = file_records[8] + file_records[9] * 0x100;
				int start_addr = file_records[2] + file_records[3] * 0x100;
				int end_addr = file_records[4] + file_records[5] * 0x100;
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
			return decrunchC64(sf, extent, g_C64Registry[i]);
		}
	}
	return 0;
}

int decrunchC64(uint8_t **sf, size_t *extent, C64Rec entry) {
	return 0;
}

} // End of namespace Scott
} // End of namespace Glk
