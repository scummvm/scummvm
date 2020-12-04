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

static const byte SHAPE_DATA[32][8] = {
	{    0,    0,    0,    0,    0,    0,    0,    0 },
	{    0,    0,    0,    0,    0,    0,    0,    0 },
	{    0,    0,    0,    0,    0,    0,    0, 0x80 },
	{    0,    0,    0,    0,    0,    0,    0,    0 },
	{    0,    0,    0,    0,    0,    0,    0,    1 },
	{    1,    0,    0,    0,    0,    0,    0,    0 },
	{    0,    0,    0,    0,    0,    0,    0, 0x80 },
	{ 0x80,    0,    0,    0,    0,    0,    0,    0 },
	{    0,    0,    0,    0,    0,    0,    1,    3 },
	{    3,    1,    0,    0,    0,    0,    0,    0 },
	{    0,    0,    0,    0,    0,    0, 0x80, 0xC0 },
	{ 0xC0, 0x80,    0,    0,    0,    0,    0,    0 },
	{    0,    0,    0,    0,    0,    3,    7,    7 },
	{    7,    7,    3,    0,    0,    0,    0,    0 },
	{    0,    0,    0,    0,    0, 0xC0, 0xE0, 0xE0 },
	{ 0xE0, 0xE0, 0xC0,    0,    0,    0,    0,    0 },
	{    0,    0,    0,    3, 0x0F, 0x0F, 0x1F, 0x1F },
	{ 0x1F, 0x1F, 0x0F, 0x0F,    3,    0,    0,    0 },
	{    0,    0,    0, 0xC0, 0xF0, 0xF0, 0xF8, 0xF8 },
	{ 0xF8, 0xF8, 0xF0, 0xF0, 0xC0,    0,    0,    0 },
	{    0,    3, 0x1F, 0x3F, 0x3F, 0x3F, 0x7F, 0x7F },
	{ 0x7F, 0x7F, 0x3F, 0x3F, 0x3F, 0x1F,    3,    0 },
	{    0, 0xC0, 0xF8, 0xFC, 0xFC, 0xFC, 0xFE, 0xFE },
	{ 0xFE, 0xFE, 0xFC, 0xFC, 0xFC, 0xF8, 0xC0,    0 },
	{    0,    0,    0,    0,    1,    8,    2,    0 },
	{ 0x0A,    0,    4,    0,    0,    0,    0,    0 },
	{    0,    0,    0,    0,    0, 0x20,    0, 0x90 },
	{    0, 0xA0,    0, 0x80,    0,    0,    0,    0 },
	{    0,    2,    8, 0x12,    1, 0x24, 0x0B,    3 },
	{ 0x23,    9, 0x22, 0x0A,    4,    1,    0,    0 },
	{    0, 0x20, 0x80, 0x28,    0, 0xD4, 0xC0, 0xE4 },
	{ 0xE8, 0x90, 0x44, 0xA8,    0, 0x50,    0,    0 }
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
	drawPixel(x1, y1, color);

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
	if (x1 > x2)
		SWAP(x1, x2);
	if (y1 > y2)
		SWAP(y1, y2);

	Common::Rect r(x1, y1, x2 + 1, y2 + 1);
	frameRect(r, color);
}

void Surface::drawFilledBox(int16 x1, int16 y1, int16 x2, int16 y2, uint32 color) {
	if (x1 > x2)
		SWAP(x1, x2);
	if (y1 > y2)
		SWAP(y1, y2);

	Common::Rect r(x1, y1, x2 + 1, y2 + 1);
	fillRect(r, color);
}

void Surface::drawShape(int16 x, int16 y, Shape shapeType, uint32 fillColor) {
	uint shapeNum = (uint)shapeType * 4;

	// Outer loop to draw the shape across a 2x2 grid of 8x8 sub-shapes
	for (int shapeX = 0; shapeX <= 8; shapeX += 8) {
		for (int shapeY = 0; shapeY <= 8; shapeY += 8, ++shapeNum) {
			// Inner loop for character
			for (int charY = 0; charY < 8; ++charY) {
				int yp = y + shapeY + charY;
				if (yp < 0 || yp >= this->h)
					continue;

				int xp = x + shapeX;
				uint32 *lineP = (uint32 *)getBasePtr(xp, yp);
				byte bits = SHAPE_DATA[shapeNum][charY];

				for (int charX = 0; charX < 8; ++lineP, ++charX, ++xp, bits <<= 1) {
					if (xp >= 0 && xp < this->w && (bits & 0x80) != 0)
						*lineP = fillColor;
				}
			}

		}
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
	const uint32 *ptr = (const uint32 *)getBasePtr(x, y);
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

void FloodFillSurface::dumpToScreen() {
	Graphics::ManagedSurface s(w * 2, h * 2, g_system->getScreenFormat());
	s.transBlitFrom(*this, Common::Rect(0, 0, w, h), Common::Rect(0, 0, w * 2, h * 2), 0x888888);

	g_system->copyRectToScreen(s.getPixels(), s.pitch, 0, 0, w * 2, h * 2);
	g_system->updateScreen();
}

void FloodFillSurface::floodFill(int16 x, int16 y, uint32 fillColor) {
	if (y == this->h)
		y = this->h - 1;
	else if (y > this->h)
		return;

	if (!isPixelWhite(x, y))
		return;

	floodFillRow(x, y, fillColor);
}

void FloodFillSurface::floodFillRow(int16 x, int16 y, uint32 fillColor) {
	int x1, x2, i;

	// Left end of scanline
	for (x1 = x; x1 > 0; x1--)
		if (!isPixelWhite(x1 - 1, y))
			break;

	// Right end of scanline
	for (x2 = x; x2 < this->w; x2++)
		if (!isPixelWhite(x2 + 1, y))
			break;

	drawLine(x1, y, x2, y, fillColor);

	//dumpToScreen();

	// Scanline above
	if (y > 0) {
		for (i = x1; i <= x2; i++)
			if (isPixelWhite(i, y - 1))
				floodFillRow(i, y - 1, fillColor);
	}

	// Scanline below
	if (y < (this->h - 1)) {
		for (i = x1; i <= x2; i++)
			if (isPixelWhite(i, y + 1))
				floodFillRow(i, y + 1, fillColor);
	}
}

} // namespace Comprehend
} // namespace Glk
