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

#ifndef WAYNESWORLD_GRAPHICS_H
#define WAYNESWORLD_GRAPHICS_H

#include "graphics/surface.h"

namespace WaynesWorld {

class WWSurface : public Graphics::Surface {
public:
	WWSurface(int width, int height);
	WWSurface(const Graphics::Surface *sourceSurface);
	~WWSurface();
	void drawSurfaceIntern(const Graphics::Surface *surface, int x, int y, bool transparent);
	void drawSurface(const Graphics::Surface *surface, int x, int y);
	void drawSurfaceTransparent(const Graphics::Surface *surface, int x, int y);
	void scaleSurface(const Graphics::Surface *surface);
	void frameRect(int x1, int y1, int x2, int y2, byte color);
	void borderSquare(int x, int y, int length, byte frameColor, byte fillColor);
	void fillRect(int x1, int y1, int x2, int y2, byte color);
	void clear(byte color);
};

class GFTFont {
public:
	GFTFont();
	~GFTFont();
	void loadFromFile(const char *filename);
	void drawText(Graphics::Surface *surface, const char *text, int x, int y, byte color);
	void drawWrappedText(Graphics::Surface *surface, const char *text, int x, int y, int maxWidth, byte color);
	int drawChar(Graphics::Surface *surface, byte ch, int x, int y, byte color);
	int getTextWidth(const char *text) const;
	int getCharWidth(byte ch) const;
protected:
	byte _firstChar;
	byte _lastChar;
	uint16 *_charTable;
	byte *_fontData;
	int16 _formWidth, _formHeight;
};

class Screen {
public:
	Screen();
	~Screen();
	void drawSurface(const Graphics::Surface *surface, int x, int y);
	void drawSurfaceTransparent(const Graphics::Surface *surface, int x, int y);
	void frameRect(int x1, int y1, int x2, int y2, byte color);
	void borderSquare(int x, int y, int length, byte frameColor, byte fillColor);
	void fillRect(int x1, int y1, int x2, int y2, byte color);
	void clear(byte color);
	void drawText(GFTFont *font, const char *text, int x, int y, byte color);
	void drawWrappedText(GFTFont *font, const char *text, int x, int y, int maxWidth, byte color);
protected:
	WWSurface *_surface;
	void updateScreen();
};

} // End of namespace WaynesWorld

#endif // WAYNESWORLD_GRAPHICS_H
