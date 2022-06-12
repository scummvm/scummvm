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
#include "glk/scott/scott.h"
#include "glk/scott/globals.h"
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

	{ UNKNOWN_GAME, 0, 0, UNKNOWN_FILE_TYPE, 0, nullptr, nullptr, 0, 0, 0, 0 }
};

int decrunchC64(uint8_t **sf, size_t *extent, C64Rec entry);

int mysteriousMenu(uint8_t **sf, size_t *extent, int recindex) {
	return 0;
}

int mysteriousMenu2(uint8_t **sf, size_t *extent, int recindex) {
	return 0;
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

int decrunchC64(uint8_t **sf, size_t *extent, C64Rec entry) {
	return 0;
}

} // End of namespace Scott
} // End of namespace Glk
