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

#ifndef M4_M4_TYPES_H
#define M4_M4_TYPES_H

#include "common/scummsys.h"

namespace M4 {

#define MAX_STRING_LEN 144
#define MAX_FILENAME_SIZE 144
#define MIN_VIDEO_X 0
#define MIN_VIDEO_Y 0
#define MAX_VIDEO_X 639
#define MAX_VIDEO_Y 479
#define VIDEO_W 640
#define VIDEO_H 480

typedef void *Ptr;
typedef void *Handle;

/**
 * This needs to be intptr in ScummVM because series_stream in gr_series.cpp stores
 * a pointer as a global. On some systems size(ptr) > sizeof(int)
 */
typedef intptr frac16;

typedef uint32 ulong;

enum {
	TRIG_INV_CLICK = 32000,
	TRIG_RESTORE_GAME
};

struct Buffer {
	int32 w = 0;
	int32 h = 0;
	uint8 *data = nullptr;
	uint8 encoding = 0;
	int32 stride = 0;

	constexpr Buffer() = default;
	constexpr Buffer(int32 _w, int32 _h, uint8 *_data, uint8 _encoding, int32 _stride) : w(_w), h(_h), data(_data), encoding(_encoding), stride(_stride) {}

	uint8 *getBasePtr(int x, int y) {
		return data + y * w + x;
	}
	const uint8 *getBasePtr(int x, int y) const {
		return data + y * w + x;
	}
};

struct token {
	char *sym_name = nullptr;
	int32 tag = 0;
};

#include "common/pack-start.h"  // START STRUCT PACKING
struct RGB8 {
	byte r = 0, g = 0, b = 0;

	constexpr RGB8() = default;
	constexpr RGB8(const byte red, const byte green, const byte blue) : r(red), g(green), b(blue) {}
} PACKED_STRUCT;
#include "common/pack-end.h"	// END STRUCT PACKING

typedef void (*FuncPtr_v_vv)(void *, void *);

typedef FuncPtr_v_vv M4CALLBACK;

} // namespace M4

#endif
