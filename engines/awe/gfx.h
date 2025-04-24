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

#ifndef AWE_GRAPHICS_H
#define AWE_GRAPHICS_H

#include "graphics/surface.h"
#include "awe/intern.h"

namespace Awe {

enum {
	FMT_CLUT,
	FMT_RGB555,
	FMT_RGB,
	FMT_RGBA,
};

enum {
	FIXUP_PALETTE_NONE,
	FIXUP_PALETTE_REDRAW, // redraw all primitives on setPal script call
};

enum {
	COL_ALPHA = 0x10, // transparent pixel (OR'ed with 0x8)
	COL_PAGE  = 0x11, // buffer 0 pixel
	COL_BMP   = 0xFF, // bitmap in buffer 0 pixel
};

enum {
	GRAPHICS_ORIGINAL,
	GRAPHICS_SOFTWARE,
	GRAPHICS_GL
};

enum {
	ALPHA_COLOR_INDEX = 12, // 3DO uses 0x0010 (RGB555) as the blend color, using color #12 matches Amiga/DOS graphics better
	GFX_W = 320,
	GFX_H = 200
};

struct SystemStub;

struct Gfx {
	static const uint8 FONT[];
	static bool _is1991; // draw graphics as in the original 1991 game release
	static Graphics::PixelFormat _format;
	static const uint16 SHAPES_MASK_OFFSET[];
	static const int SHAPES_MASK_COUNT;
	static const uint8 SHAPES_MASK_DATA[];

	int _fixUpPalette = 0;

	virtual ~Gfx() {};

	virtual void init(int targetW, int targetH) {}
	virtual void fini() {}

	virtual void setFont(const uint8 *src, int w, int h) = 0;
	virtual void setPalette(const Color *colors, int count) = 0;
	virtual void setSpriteAtlas(const uint8 *src, int w, int h, int xSize, int ySize) = 0;
	virtual void drawSprite(int buffer, int num, const Point *pt, uint8 color) = 0;
	virtual void drawBitmap(int buffer, const uint8 *data, int w, int h, int fmt) = 0;
	virtual void drawPoint(int buffer, uint8 color, const Point *pt) = 0;
	virtual void drawQuadStrip(int buffer, uint8 color, const QuadStrip *qs) = 0;
	virtual void drawStringChar(int buffer, uint8 color, char c, const Point *pt) = 0;
	virtual void clearBuffer(int num, uint8 color) = 0;
	virtual void copyBuffer(int dst, int src, int vscroll = 0) = 0;
	virtual void drawBuffer(int num, SystemStub *) = 0;
	virtual void drawRect(int num, uint8 color, const Point *pt, int w, int h) = 0;
	virtual void drawBitmapOverlay(const Graphics::Surface &src, int fmt, SystemStub *stub) = 0;
};

Gfx *GraphicsSoft_create();

} // namespace Awe

#endif
