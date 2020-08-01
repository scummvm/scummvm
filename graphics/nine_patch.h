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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/* This code is based on Nine Patch code by Matthew Leverton
   taken from https://github.com/konforce/Allegro-Nine-Patch

   Copyright (C) 2011 Matthew Leverton

   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is furnished to do
   so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
 */

#ifndef GRAPHICS_NINE_PATCH_H
#define GRAPHICS_NINE_PATCH_H

#include "common/array.h"
#include "common/rect.h"
#include "common/hashmap.h"

namespace Graphics {

struct TransparentSurface;
struct Surface;
class MacWindowManager;

struct NinePatchMark {
	int offset;
	int length;
	int dest_offset;
	int dest_length;
	float ratio;
};

class NinePatchSide {
public:
	Common::Array<NinePatchMark *> _m;
	int _fix;

	NinePatchSide() : _fix(0) { _m.clear(); }
	~NinePatchSide();

	bool init(Graphics::TransparentSurface *bmp, bool vertical);
	void calcOffsets(int len);
};

class NinePatchBitmap {
	Graphics::TransparentSurface *_bmp;
	NinePatchSide _h, _v;
	Common::Rect _padding;
	bool _destroy_bmp;
	int _width, _height;
	int _cached_dw, _cached_dh;
	Common::HashMap<uint32, int> _cached_colors;

public:
	NinePatchBitmap(Graphics::TransparentSurface *bmp, bool owns_bitmap);
	~NinePatchBitmap();

	void blit(Graphics::Surface &target, int dx, int dy, int dw, int dh, byte *palette = NULL, int numColors = 0, MacWindowManager *wm = NULL);
	void blitClip(Graphics::Surface &target, Common::Rect clip, int dx, int dy, int dw, int dh);

	int getWidth() { return _width; }
	int getHeight() { return _height; }
	int getMinWidth() { return _h._fix; }
	int getMinHeight() { return _v._fix; }
	Graphics::TransparentSurface *getSource() { return _bmp; }
	Common::Rect &getPadding() { return _padding; }

private:

	void drawRegions(Graphics::Surface &target, int dx, int dy, int dw, int dh);

	// Assumes color is in the palette
	byte getColorIndex(uint32 target, byte *palette);
	uint32 grayscale(uint32 color);
	uint32 grayscale(byte r, byte g, byte b);
	byte closestGrayscale(uint32 color, byte* palette, int paletteLength);
};

} // end of namespace Graphics

#endif // GRAPHICS_NINE_PATCH_H
