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

class ResourceManager;
class Decompressor;

class Font : public Graphics::Font {
public:
	Font(byte w, byte h, byte start, byte count, byte *data);
	virtual ~Font();
	int getFontHeight() const { return _h; }
	int getMaxCharWidth() const { return _w; }
	virtual int getCharWidth(uint32 chr) const = 0;
    void drawChar(Graphics::Surface* dst, int pos, int bit, int x, int y, uint32 color) const;
	static Font *load(const Common::String &filename, ResourceManager *resourceManager, Decompressor *decompressor);

protected:
	byte _w;
	byte _h;
	byte _start;
	byte _count;
	byte *_data;

	bool hasChar(byte chr) const;
};

/* Proportional font (each char has its own width and so data is a different size) */
class PFont : public Font {
public:
	PFont(byte w, byte h, byte start, byte count, byte *data, const uint16 *offsets, const byte *widths);
	int getCharWidth(uint32 chr) const { return _widths[chr - _start]; }
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const;
	static PFont *load(Common::SeekableReadStream &input, Decompressor *decompressor);

protected:
	const uint16 *_offsets;
	const byte *_widths;

	void mapChar(byte chr, int &pos, int &bit) const;
};

#if DGDS_SUPPORT_FIXED_WIDTH
/* Fixed-width font */
class FFont : public Font {
public:
	FFont(byte w, byte h, byte start, byte count, byte *data);
	int getCharWidth(uint32 chr) const { return _w; }
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const;
	static FFont *load(Common::SeekableReadStream &input);

protected:
	void mapChar(byte chr, int &pos, int &bit) const;
};
#endif

} // End of namespace Dgds

#endif // DGDS_FONT_H
