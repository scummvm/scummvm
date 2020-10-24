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

#ifndef DGDS_FONT_H
#define DGDS_FONT_H

#include "common/scummsys.h"
#include "graphics/font.h"

namespace Graphics {
class Font;
struct Surface;
}

namespace Common {
class SeekableReadStream;
}

namespace Dgds {

class Decompressor;

class Font : public Graphics::Font {
public:
	int getFontHeight() const { return _h; }
	int getMaxCharWidth() const { return _w; }
	virtual int getCharWidth(uint32 chr) const = 0;
    void drawChar(Graphics::Surface* dst, int pos, int bit, int x, int y, uint32 color) const;

protected:
	byte _w;
	byte _h;
	byte _start;
	byte _count;
	byte *_data;

	bool hasChar(byte chr) const;
};

class PFont : public Font {
public:
	int getCharWidth(uint32 chr) const { return _widths[chr - _start]; }
	void drawChar(Graphics::Surface* dst, uint32 chr, int x, int y, uint32 color) const;
	static PFont *load(Common::SeekableReadStream &input, Decompressor *decompressor);

protected:
	uint16 *_offsets;
	byte *_widths;

	void mapChar(byte chr, int &pos, int &bit) const;
};

class FFont : public Font {
public:
	int getCharWidth(uint32 chr) const { return _w; }
	void drawChar(Graphics::Surface* dst, uint32 chr, int x, int y, uint32 color) const;
	static FFont *load(Common::SeekableReadStream &input);

protected:
	void mapChar(byte chr, int &pos, int &bit) const;
};

} // End of namespace Dgds

#endif // DGDS_FONT_H
