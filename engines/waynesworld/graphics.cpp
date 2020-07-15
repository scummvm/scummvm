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

#include "waynesworld/graphics.h"
#include "common/rect.h"
#include "common/system.h"

namespace WaynesWorld {

// WWSurface

WWSurface::WWSurface(int width, int height) {
	create(width, height, Graphics::PixelFormat::createFormatCLUT8());
}

WWSurface::WWSurface(const Graphics::Surface *sourceSurface) {
	copyFrom(*sourceSurface);
}

WWSurface::~WWSurface() {
	free();
}

void WWSurface::drawSurfaceIntern(const Graphics::Surface *surface, int x, int y, bool transparent) {
	int width, height, skipX = 0, skipY = 0;

	if (x >= w || y >= h)
		return;

	height = surface->h;
	if (y < 0) {
		height += y;
		if (height <= 0)
			return;
		skipY = -y;
		y = 0;
	}
	if (y + height > h)
		height = h - y;

	width = surface->w;
	if (x < 0) {
		width += x;
		if (width <= 0)
			return;
		skipX = -x;
		x = 0;
	}
	if (x + width >= w)
		width = w - x;

	// debug(6, "drawSurfaceIntern() (%d, %d, %d, %d); skipX: %d; skipY: %d", x, y, width, height, skipX, skipY);

	if (transparent) {
		for (int yc = 0; yc < height; ++yc) {
			const byte *source = (const byte*)surface->getBasePtr(skipX, skipY + yc);
			byte *dest = (byte*)getBasePtr(x, y + yc);
			for (int xc = 0; xc < width; ++xc) {
				byte value = *source++;
				if (value != 0) {
					*dest = value;
				}
				++dest;
			}
		}
	} else {
		for (int yc = 0; yc < height; ++yc) {
			const byte *source = (const byte*)surface->getBasePtr(skipX, skipY + yc);
			byte *dest = (byte*)getBasePtr(x, y + yc);
			memcpy(dest, source, width);
		}
	}
}

void WWSurface::drawSurface(const Graphics::Surface *surface, int x, int y) {
	drawSurfaceIntern(surface, x, y, false);
}

void WWSurface::drawSurfaceTransparent(const Graphics::Surface *surface, int x, int y) {
	drawSurfaceIntern(surface, x, y, true);
}

void WWSurface::scaleSurface(const Graphics::Surface *surface) {
	const int xIncr = surface->w / w;
	const int xIncrErr = surface->w % w;
	const int yIncr = (surface->h / h) * surface->pitch;
	const int yIncrErr = surface->h % h;
	int errY = 0;
	byte *source = (byte*)surface->getBasePtr(0, 0);
	for (int yc = 0; yc < h; yc++) {
		byte *sourceRow = source;
		byte *destRow = (byte*)getBasePtr(0, yc);
		int errX = 0;
		for (int xc = 0; xc < w; xc++) {
			*destRow++ = *sourceRow;
			sourceRow += xIncr;
			errX += xIncrErr;
			if (errX >= w) {
				errX -= w;
				sourceRow++;
			}
		}
		source += yIncr;
		errY += yIncrErr;
		if (errY >= h) {
			errY -= h;
			source += surface->pitch;
		}
	}
}

void WWSurface::frameRect(int x1, int y1, int x2, int y2, byte color) {
	// TODO
}

void WWSurface::borderSquare(int x, int y, int length, byte frameColor, byte fillColor) {
	// TODO
}

void WWSurface::fillRect(int x1, int y1, int x2, int y2, byte color) {
	// TODO
}

void WWSurface::clear(byte color) {
	Graphics::Surface::fillRect(Common::Rect(w, h), color);
}

// Screen

Screen::Screen() {
	_surface = new WWSurface(320, 200);
}

Screen::~Screen() {
	delete _surface;
}

void Screen::drawSurface(const Graphics::Surface *surface, int x, int y) {
	_surface->drawSurface(surface, x, y);
	updateScreen();
}

void Screen::drawSurfaceTransparent(const Graphics::Surface *surface, int x, int y) {
	_surface->drawSurfaceTransparent(surface, x, y);
	updateScreen();
}

void Screen::frameRect(int x1, int y1, int x2, int y2, byte color) {
	_surface->frameRect(x1, y1, x2, y2, color);
	updateScreen();
}

void Screen::borderSquare(int x, int y, int length, byte frameColor, byte fillColor) {
	_surface->borderSquare(x, y, length, frameColor, fillColor);
	updateScreen();
}

void Screen::fillRect(int x1, int y1, int x2, int y2, byte color) {
	_surface->fillRect(x1, y1, x2, y2, color);
	updateScreen();
}

void Screen::clear(byte color) {
	_surface->clear(color);
	updateScreen();
}

void Screen::updateScreen() {
	// TODO Use dirty rectangles or similar
	g_system->copyRectToScreen(_surface->getPixels(), _surface->pitch, 0, 0, _surface->w, _surface->h);
	g_system->updateScreen();
}

} // End of namespace WaynesWorld
