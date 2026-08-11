
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

#ifndef BAGEL_BOFLIB_STDINC_H
#define BAGEL_BOFLIB_STDINC_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "bagel/afxwin.h"

namespace Bagel {

/*
 * Normal types
 */
typedef void *(*BofCallback)(int, void *);

typedef long Fixed;

/*
 * Extended types
 */
struct StSize {
	int cx;
	int cy;
};

struct StPoint {
	int x;
	int y;
};

struct Vector {
	double x;
	double y;
	double z;
};

struct WindowPos {
	void *hwnd = nullptr;
	void *hwndinsertAfter = nullptr;
	int x = 0;
	int y = 0;
	int cx = 0;
	int cy = 0;
	uint32 flags = 0;
};

#define MAKE_WORD(a, b) ((uint16)(((byte)(a)) | ((uint16)((byte)(b))) << 8))
#define MAKE_LONG(low, high) ((int32)(((uint16)(low)) | (((uint32)((uint16)(high))) << 16)))

/* For big-endian platforms (i.e. MAC) */
#define SWAPWORD(x) MAKE_WORD(HIBYTE(x), LOBYTE(x))
#define SWAPLONG(x) MAKE_LONG(SWAPWORD(HIWORD(x)), SWAPWORD(LOWORD(x)))

#ifndef MAX_FNAME
	#define MAX_FNAME 256
#endif

#define MAX_DIRPATH 256

#ifndef PDFT
	#define PDFT(VALUE) = VALUE
#endif

/*
* normal types
*/
#define VIRTUAL virtual
#define STATIC static
#define CDECL
#define INLINE inline

} // namespace Bagel

#endif
