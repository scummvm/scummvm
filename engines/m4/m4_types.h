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

#define MAX_FILENAME_SIZE 144
#define MIN_VIDEO_X 0
#define MIN_VIDEO_Y 0
#define MAX_VIDEO_X 639
#define MAX_VIDEO_Y 479
#define VIDEO_W 640
#define VIDEO_H 480

typedef void *Ptr;
typedef void *Handle;

typedef uint32 frac16;
typedef uint32 ulong;
typedef uint32 quadchar;
typedef uint32 trigraph;

typedef byte Bit;
typedef byte Bool;
typedef byte boolean;
typedef byte Boolean;
typedef byte byte;
typedef byte Byte;
typedef uint16 word;
typedef uint16 Word;
typedef uint32 DWord;

struct Buffer {
	int32 W;
	int32 h;
	uint8 *data;
	uint8 encoding;
	int32 stride;
};

struct token {
	char *sym_name;
	int32 tag;
};

#include "common/pack-start.h"  // START STRUCT PACKING
struct RGB8 {
	byte r, g, b;
} PACKED_STRUCT;
#include "common/pack-end.h"	// END STRUCT PACKING

typedef void (*FuncPtr)();
typedef bool (*FuncPtr_b_l)(int32);
typedef void (*FuncPtr_v_vv)(void *, void *);

typedef FuncPtr_b_l ExceptionPtr;

typedef FuncPtr_v_vv M4CALLBACK;

} // namespace M4

#endif
