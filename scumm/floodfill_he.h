/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#if !defined(FLOODFILL_HE_H) && !defined(DISABLE_HE)
#define FLOODFILL_HE_H

#include "common/rect.h"

namespace Scumm {

struct FloodFillParameters {
	Common::Rect box;
	int32 x;
	int32 y;
	int32 flags;
	int32 unk1C; /* unused */
};

struct FloodFillLine {
	int y;
	int x1;
	int x2;
	int inc;
};

struct FloodFillState {
	FloodFillLine *fillLineTable;
	FloodFillLine *fillLineTableEnd;
	FloodFillLine *fillLineTableCur;
	Common::Rect dstBox;
	Common::Rect srcBox;
	uint8 *dst;
	int dst_w;
	int dst_h;
	int color1;
	int color2;
	int fillLineTableCount;
};

class ScummEngine_v90he;

typedef bool (*FloodFillPixelCheckCallback)(int x, int y, const FloodFillState *ffs);

void floodFill(FloodFillParameters *ffp, ScummEngine_v90he *vm);

} // End of namespace Scumm

#endif
