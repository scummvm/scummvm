/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_PSX_PCDEFINES_H
#define ICB_PSX_PCDEFINES_H

#include "common/types.h"

namespace ICB {
// make our own equivalents
typedef struct MATRIX {
	int16 m[3][3]; /* 3x3 rotation matrix */
	int16 pad;
	int32 t[3]; /* transfer vector */
	MATRIX() { pad = 0; }
} MATRIX;

typedef struct MATRIXPC {
	int32 m[3][3]; /* 3x3 rotation matrix */
	int32 pad;
	int32 t[3]; /* transfer vector */
	MATRIXPC() { pad = 0; }
} MATRIXPC;

/* int32 word type 3D vector */
typedef struct VECTOR {
	int32 vx, vy;
	int32 vz, pad;
	VECTOR() { pad = 0; }
} VECTOR;

/* short word type 3D vector */
typedef struct SVECTOR {
	int16 vx, vy;
	int16 vz, pad;
} SVECTOR;

/* short word type 3D vector - PC version */
typedef struct SVECTORPC {
	int32 vx, vy;
	int32 vz, pad;
} SVECTORPC;

typedef struct CVECTOR {
	/* color type vector */
	uint8 r, g, b, cd;
} CVECTOR;

typedef struct {/* 2D short vector */
	short vx, vy;
} DVECTOR;

typedef uint8 PACKET;

//-=- Definitions -=-//
#ifndef ONE
#define ONE 4096
#endif
#define PSX_SCREEN_WIDTH 512
#define PSX_SCREEN_HEIGHT 240

//-=- Macros -=-//
#define __nint__(x) (((x) > 0) ? int((x) + 0.5) : int((x)-0.5))

#define getTPage(tp, abr, x, y) ((((tp)&0x3) << 7) | (((abr)&0x3) << 5) | (((y)&0x100) >> 4) | (((x)&0x3ff) >> 6) | (((y)&0x200) << 2))

#define getClut(x, y) ((y << 6) | ((x >> 4) & 0x3f))

} // End of namespace ICB

#endif // __PSX_PcDefines_H__
