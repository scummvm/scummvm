/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

#ifndef WSAMOVIES_H
#define WSAMOVIES_H

#include "kyra/resource.h"

namespace Kyra {

enum WSAFlags {
	WF_OFFSCREEN_DECODE = 0x10,
	WF_NO_FIRST_FRAME = 0x40,
	WF_HAS_PALETTE = 0x100
};

struct WSAMovieV1 {
	uint16 currentFrame;
	uint16 numFrames;
	uint16 width;
	uint16 height;
	uint16 flags;
	uint8 *deltaBuffer;
	uint32 deltaBufferSize;
	uint8 *offscreenBuffer;
	uint32 *frameOffsTable;
	uint8 *frameData;
};

} // end of namespace Kyra

#endif
