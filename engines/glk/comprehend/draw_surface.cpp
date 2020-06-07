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

#include "glk/comprehend/draw_surface.h"
#include "glk/comprehend/comprehend.h"
#include "glk/comprehend/pics.h"
#include "glk/window_graphics.h"

namespace Glk {
namespace Comprehend {

const uint32 Surface::PEN_COLORS[8] = {
	G_COLOR_BLACK,
	RGB(0x00, 0x66, 0x00),
	RGB(0x00, 0xff, 0x00),
	G_COLOR_WHITE,
	G_COLOR_BLACK,
	RGB(0x00, 0xff, 0xff),
	RGB(0xff, 0x00, 0xff),
	RGB(0xff, 0x00, 0x00),
};

/* Used by Transylvania and Crimson Crown */
const uint32 Surface::DEFAULT_COLOR_TABLE[256] = {
	G_COLOR_WHITE,     // 00
	G_COLOR_DARK_BLUE, // 01
	G_COLOR_GRAY1,     // 02
	G_COLOR_DARK_RED,  // 03
	G_COLOR_GRAY2,     // 04
	0, G_COLOR_GRAY3, 0, 0, 0, 0, 0, 0,
	G_COLOR_BROWN1, G_COLOR_DARK_PURPLE, 0,

	0, 0, G_COLOR_DARK_RED, G_COLOR_BROWN2, 0, 0, 0,
	G_COLOR_DARK_BLUE, G_COLOR_BLACK, 0, 0, 0, 0, 0, 0, G_COLOR_DARK_PURPLE,

	G_COLOR_DARK_PURPLE, 0, G_COLOR_DARK_RED, 0, 0, 0, 0, 0,
	0, 0, 0, G_COLOR_DARK_PURPLE, 0, 0, 0, 0,

	0, 0, 0, 0, G_COLOR_WHITE, G_COLOR_GRAY0, RGB(0xb5, 0x6c, 0x47),
	0, 0, 0, 0, 0, G_COLOR_CYAN, G_COLOR_DARK_RED,
	G_COLOR_DARK_GREEN1, G_COLOR_DARK_GREEN2,

	G_COLOR_DARK_PURPLE, 0, G_COLOR_DITHERED_PINK, 0, 0,
	G_COLOR_BROWN2, G_COLOR_DARK_RED, G_COLOR_DARK_BLUE,
	G_COLOR_DARK_BLUE, G_COLOR_DARK_BLUE, 0, 0, 0,
	G_COLOR_WHITE, G_COLOR_BROWN2, G_COLOR_BROWN2,

	G_COLOR_BLACK, G_COLOR_DARK_PURPLE, 0, G_COLOR_GRAY2,
	G_COLOR_BROWN2, 0, 0, G_COLOR_AQUA, 0, 0, G_COLOR_GREEN,
	G_COLOR_DARK_BLUE, G_COLOR_DARK_PURPLE, G_COLOR_BROWN1,
	G_COLOR_BROWN2, 0,

	G_COLOR_DARK_PURPLE, G_COLOR_LIGHT_ORANGE, 0, 0,
	G_COLOR_ORANGE, G_COLOR_RED, G_COLOR_DARK_RED, 0, 0, 0,
	G_COLOR_DARK_BLUE, G_COLOR_DARK_PURPLE, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	G_COLOR_BLACK, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* Used by OO-topos */
/* FIXME - incomplete */
const uint32 Surface::COLOR_TABLE_1[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,
	0,
	0,
	0,
	0,
	RGB(0x80, 0x00, 0x00),
	0,
	RGB(0xe6, 0xe6, 0x00),
	0,
	0,
	0,
	0,
	RGB(0xc0, 0x00, 0x00),
	RGB(0x80, 0x00, 0x00),
	G_COLOR_ORANGE,
	0,

	0,
	G_COLOR_BROWN1,
	RGB(0x00, 0x00, 0x66),
	RGB(0x33, 0x99, 0xff),
	0,
	RGB(0xe8, 0xe8, 0xe8),
	RGB(0x99, 0xcc, 0xff),
	0,
	RGB(0x99, 0x33, 0x33),
	RGB(0xcc, 0x66, 0x00),
	0,
	0,
	0,
	0,
	0,
	0,

	G_COLOR_GRAY3,
	0,
	0,
	0,
	0,
	0,
	0,
	RGB(0x99, 0x33, 0x00),
	G_COLOR_CYAN,
	0,
	0,
	RGB(0x66, 0x00, 0x33),
	0,
	0,
	0,
	0,

	G_COLOR_AQUA,
	G_COLOR_GRAY2,
	0,
	0,
	0,
	G_COLOR_DARK_BLUE,
	0,
	0,
	0,
	0,
	G_COLOR_GRAY1,
	0,
	0,
	0,
	0,
	0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const uint32 *Surface::COLOR_TABLES[2] = {
	DEFAULT_COLOR_TABLE,
	COLOR_TABLE_1,
};

/*-------------------------------------------------------*/

void Surface::reset() {
	create(G_RENDER_WIDTH, G_RENDER_HEIGHT,
	       Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
}

void Surface::setColorTable(uint index) {
	if (index >= ARRAY_SIZE(COLOR_TABLES)) {
		warning("Bad color table %d - using default", index);
		_colorTable = DEFAULT_COLOR_TABLE;
	}

	_colorTable = COLOR_TABLES[index];
}

uint Surface::getPenColor(uint8 param) const {
	return PEN_COLORS[param];
}

uint32 Surface::getFillColor(uint8 index) {
	unsigned color;

	color = _colorTable[index];
	if (!color) {
		/* Unknown color - use ugly purple */
		debugC(kDebugGraphics, "Unknown color %.2x", index);
		return RGB(0xff, 0x00, 0xff);
	}

	return color;
}

void Surface::drawLine(int16 x1, int16 y1, int16 x2, int16 y2, uint32 color) {
#if 1
	Graphics::ManagedSurface::drawLine(x1, y1, x2, y2, color);
#else
	bool swapped = false;
	int deltaX = -1, deltaY = -1;
	int xDiff = x1 - x2, yDiff = y1 - y2;

	// Draw pixel at starting point
	drawPixel(x1, y1);

	// Figure out the deltas movement for creating the line
	if (xDiff < 0) {
		deltaX = 1;
		xDiff = -xDiff;
	}
	if (yDiff < 0) {
		deltaY = 1;
		yDiff = -yDiff;
	}

	if (xDiff < yDiff) {
		swapped = true;
		SWAP(xDiff, yDiff);
		SWAP(deltaX, deltaY);
		SWAP(x1, y1);
	}

	int temp1 = yDiff;
	int temp2 = yDiff - xDiff;
	int temp3 = temp2;

	// Iterate to draw the remaining pixels of the line
	for (int ctr = xDiff; ctr > 0; --ctr) {
		x1 += deltaX;

		if (temp3 >= 0) {
			y1 += deltaY;
			temp3 += temp2;
		} else {
			temp3 += temp1;
		}

		int xp = x1, yp = y1;
		if (swapped)
			SWAP(xp, yp);

		drawPixel(xp, yp, color);
	}
#endif
}

void Surface::drawBox(int16 x1, int16 y1, int16 x2, int16 y2, uint32 color) {
	Common::Rect r(x1, y1, x2 + 1, y2 + 1);
	frameRect(r, color);
}

void Surface::drawFilledBox(int16 x1, int16 y1, int16 x2, int16 y2, uint32 color) {
	Common::Rect r(x1, y1, x2 + 1, y2 + 1);
	fillRect(r, color);
}

void Surface::drawShape(int16 x, int16 y, int shape_type, uint32 fill_color) {
	int i, j;

	switch (shape_type) {
	case SHAPE_PIXEL:
		x += 7;
		y += 7;
		drawPixel(x, y, fill_color);
		break;

	case SHAPE_BOX:
		x += 6;
		y += 7;
		drawFilledBox(x, y, x + 2, y + 2, fill_color);
		break;

	case SHAPE_CIRCLE_TINY:
		x += 5;
		y += 5;
		drawFilledBox(x + 1, y, x + 3, y + 4, fill_color);
		drawFilledBox(x, y + 1, x + 4, y + 3, fill_color);
		break;

	case SHAPE_CIRCLE_SMALL:
		x += 4;
		y += 4;
		drawFilledBox(x + 1, y, x + 5, y + 6, fill_color);
		drawFilledBox(x, y + 1, x + 6, y + 5, fill_color);
		break;

	case SHAPE_CIRCLE_MED:
		x += 1;
		y += 1;
		drawFilledBox(x + 1,
		              y + 1,
		              x + 1 + (2 + 4 + 2),
		              y + 1 + (2 + 4 + 2),
		              fill_color);
		drawFilledBox(x + 3,
		              y,
		              x + 3 + 4,
		              y + (1 + 2 + 4 + 2 + 1),
		              fill_color);
		drawFilledBox(x,
		              y + 3,
		              x + (1 + 2 + 4 + 2 + 1),
		              y + 3 + 4,
		              fill_color);
		break;

	case SHAPE_CIRCLE_LARGE:
		drawFilledBox(x + 2,
		              y + 1,
		              x + 2 + (3 + 4 + 3),
		              y + 1 + (1 + 3 + 4 + 3 + 1),
		              fill_color);
		drawFilledBox(x + 1,
		              y + 2,
		              x + 1 + (1 + 3 + 4 + 3 + 1),
		              y + 2 + (3 + 4 + 3),
		              fill_color);
		drawFilledBox(x + 5,
		              y,
		              x + 5 + 4,
		              y + 1 + 1 + 3 + 4 + 3 + 1 + 1,
		              fill_color);
		drawFilledBox(x,
		              y + 5,
		              x + 1 + 1 + 3 + 4 + 3 + 1 + 1,
		              y + 5 + 4,
		              fill_color);
		break;

	case SHAPE_A:
		/* FIXME - very large circle? */
		break;

	case SHAPE_SPRAY: {
		char spray[13][13] = {
			{0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0},
			{0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
			{0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1},
			{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
			{1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0},
			{0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0},
			{1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0},
			{0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0},
			{1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0},
			{0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0},
			{0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0},
		};
		for (i = 0; i < 13; i++)
			for (j = 0; j < 13; j++)
				if (spray[i][j])
					drawPixel(x + i, y + j, fill_color);
		break;
	}

	default:
		/* Unknown shape */
		break;
	}
}

void Surface::drawPixel(int16 x, int16 y, uint32 color) {
	if (x >= 0 && y >= 0 && x < this->w && y < this->h) {
		uint32 *ptr = (uint32 *)getBasePtr(x, y);
		*ptr = color;
	}
}

uint32 Surface::getPixelColor(int16 x, int16 y) const {
	assert(x >= 0 && y >= 0 && x < this->w && y < this->h);
	const uint32 *ptr = (uint32 *)getBasePtr(x, y);
	return *ptr;
}

void Surface::clearScreen(uint32 color) {
	fillRect(Common::Rect(0, 0, this->w, this->h), color);
}

void Surface::drawCircle(int16 x, int16 y, int16 diameter, uint32 color) {
	int invert = -diameter;
	int delta = 0;

	do {
		drawPixel(x - delta, y - diameter, color);
		drawPixel(x + delta, y - diameter, color);
		drawPixel(x + delta, y + diameter, color);
		drawPixel(x - delta, y + diameter, color);

		drawPixel(x + diameter, y - delta, color);
		drawPixel(x - diameter, y - delta, color);
		drawPixel(x - diameter, y + delta, color);
		drawPixel(x + diameter, y + delta, color);

		invert += (delta * 2) + 1;
		++delta;
		if (!((uint)invert & 0x80)) {
			invert += 2;
			diameter <<= 1;
			invert -= diameter;
			diameter >>= 1;
			--diameter;
		}
	} while (diameter >= delta);
}

/*--------------------------------------------------------------------------*/

bool FloodFillSurface::isPixelWhite(int16 x, int16 y) const {
	if (x < 0 || y < 0 || x >= this->w || y >= this->h) {
		return false;
	} else {
		byte r, g, b;
		format.colorToRGB(getPixelColor(x, y), r, g, b);
		return r == 255 && g == 255 && b == 255;
	}
}

void FloodFillSurface::floodFill(int16 x, int16 y, uint32 fillColor) {
	int x1, x2, i;

	if (y == this->h)
		y = this->h - 1;
	else if (y > this->h)
		return;

	if (!isPixelWhite(x, y))
		return;

	// Left end of scanline
	for (x1 = x; x1 > 0; x1--)
		if (!isPixelWhite(x1 - 1, y))
			break;

	// Right end of scanline
	for (x2 = x; x2 < this->w; x2++)
		if (!isPixelWhite(x2 + 1, y))
			break;

	drawLine(x1, y, x2, y, fillColor);

	// Scanline above
	if (y > 0) {
		for (i = x1; i < x2; i++)
			if (isPixelWhite(i, y - 1))
				floodFill(i, y - 1, fillColor);
	}

	// Scanline below
	if (y < (this->h - 1)) {
		for (i = x1; i < x2; i++)
			if (isPixelWhite(i, y + 1))
				floodFill(i, y + 1, fillColor);
	}
}

} // namespace Comprehend
} // namespace Glk
