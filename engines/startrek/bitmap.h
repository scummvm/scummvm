/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 */

#ifndef STARTREK_BITMAP_H
#define STARTREK_BITMAP_H

#include "common/ptr.h"
#include "common/stream.h"
#include "common/memstream.h"

namespace StarTrek {

struct Bitmap {
	int16 xoffset;
	int16 yoffset;
	int16 width;
	int16 height;
	byte *pixels;

	Bitmap(Common::MemoryReadStreamEndian *stream, bool closeStream = true);
	Bitmap(const Bitmap &bitmap);
	Bitmap(int w, int h);
	~Bitmap();

protected:
	int32 pixelsArraySize;
	Bitmap() : xoffset(0), yoffset(0), width(0), height(0), pixels(nullptr), pixelsArraySize(0) {}
};


/**
 * TextBitmap is the same as Bitmap, except it stores character indices in its "pixels"
 * array instead of actual pixels.
 * A consequence of this is that the pixels array is smaller than otherwise expected
 * (since width/height still reflect the actual size when drawn).
 */
struct TextBitmap : Bitmap {
	TextBitmap(int w, int h);
};


/**
 * StubBitmap is a bitmap without any actual pixel data. Used as a stub for the
 * "starfield" sprite, which is always in draw mode 1 (invisible), so it never gets drawn;
 * however, it does trigger refreshes on the background in that area, so the game can draw
 * on the background layer manually.
 */
struct StubBitmap : Bitmap {
	StubBitmap(int w, int h);
};

} // End of namespace StarTrek

#endif
