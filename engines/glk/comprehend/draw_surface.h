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
 *
 */

#ifndef GLK_COMPREHEND_GRAPHICS_H
#define GLK_COMPREHEND_GRAPHICS_H

#include "common/scummsys.h"
#include "graphics/managed_surface.h"

namespace Glk {
namespace Comprehend {

#define G_RENDER_WIDTH  280
#define G_RENDER_HEIGHT 160

#define RGB(r, g, b)        (uint32)(((r) << 24) | ((g) << 16) | ((b) << 8) | 0xff)

#define G_COLOR_BLACK  0x000000ff
#define G_COLOR_WHITE  0xffffffff
#define G_COLOR_CYAN   0x3366ffff
#define G_COLOR_YELLOW 0xffff00ff
#define G_COLOR_RED    0xff0000ff

#define G_COLOR_GRAY0  0x202020ff
#define G_COLOR_GRAY1  0x404040ff
#define G_COLOR_GRAY2  0x808080ff
#define G_COLOR_GRAY3  0xc0c0c0ff

#define G_COLOR_LIGHT_ORANGE  0xff9966ff
#define G_COLOR_ORANGE        0xff9900ff
#define G_COLOR_DARK_PURPLE   0x666699ff
#define G_COLOR_DARK_BLUE     0x000099ff

#define G_COLOR_DARK_RED      0xcc0033ff
#define G_COLOR_DITHERED_PINK 0xff6699ff

#define G_COLOR_DARK_GREEN1   0x009966ff
#define G_COLOR_DARK_GREEN2   0x003300ff

#define G_COLOR_AQUA          0x33ccccff

#define G_COLOR_GREEN         0x33cc00ff

#define G_COLOR_BROWN1        0x7a5200ff
#define G_COLOR_BROWN2        0x663300ff

enum Shape {
	SHAPE_PIXEL = 0,
	SHAPE_BOX = 1,
	SHAPE_CIRCLE_TINY = 2,
	SHAPE_CIRCLE_SMALL = 3,
	SHAPE_CIRCLE_MED = 4,
	SHAPE_CIRCLE_LARGE = 5,
	SHAPE_A = 6,
	SHAPE_SPRAY = 7
};


class Surface : public Graphics::ManagedSurface {
private:
	static const uint32 PEN_COLORS[8];
	static const uint32 DEFAULT_COLOR_TABLE[256];
	static const uint32 COLOR_TABLE_1[256];
	static const uint32 *COLOR_TABLES[2];

public:
	const uint32 *_colorTable;
public:
	Surface() : _colorTable(DEFAULT_COLOR_TABLE) {
		reset();
	}

	/**
	 * Sets up the surface to the correct size and pixel format
	 */
	void reset();

	void setColorTable(uint index);
	uint getPenColor(uint8 param) const;
	uint32 getFillColor(uint8 index);

	void drawLine(int16 x1, int16 y1, int16 x2, int16 y2, uint32 color);
	void drawBox(int16 x1, int16 y1, int16 x2, int16 y2, uint32 color);
	void drawFilledBox(int16 x1, int16 y1, int16 x2, int16 y2, uint32 color);
	void drawShape(int16 x, int16 y, Shape shapeType, uint32 fill_color);
	void drawPixel(int16 x, int16 y, uint32 color);
	uint32 getPixelColor(int16 x, int16 y) const;
	void clearScreen(uint32 color);
	void drawCircle(int16 x, int16 y, int16 diameter, uint32 color);
	void drawCirclePoint(int16 x, int16 y);
};

class FloodFillSurface : public Surface {
private:
	bool isPixelWhite(int16 x, int16 y) const;

	void floodFillRow(int16 x, int16 y, uint32 fillColor);
public:
	void floodFill(int16 x, int16 y, uint32 fillColor);

	void dumpToScreen();
};

class DrawSurface : public FloodFillSurface {
};

} // namespace Comprehend
} // namespace Glk

#endif
