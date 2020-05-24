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

#ifndef CRYOMNI3D_FONTS_CRYOEXTFONT_H
#define CRYOMNI3D_FONTS_CRYOEXTFONT_H

#include "common/array.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "graphics/font.h"

namespace Common {
class SeekableReadStream;
}

namespace CryOmni3D {

class CryoExtFont : public Graphics::Font {
public:
	CryoExtFont() : _height(0), _maxAdvance(0), _crf(nullptr), _codepage(Common::kCodePageInvalid) { }
	virtual ~CryoExtFont();

	void load(const Common::String &fontFile, Common::CodePage encoding);

	virtual int getFontHeight() const { return _height; }
	virtual int getMaxCharWidth() const { return _maxAdvance; }

	virtual int getCharWidth(uint32 chr) const;

	virtual Common::Rect getBoundingBox(uint32 chr) const;

	virtual void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const;

private:
	void loadOffsets(const Common::String &offsetsFile);
	void assureCached(uint32 chr) const;
	uint32 mapGlyph(uint32 chr) const;

	struct Glyph {
		uint16 h;
		uint16 w;
		int16 offX;
		int16 offY;
		uint16 advance;

		byte *bitmap;

		Glyph();
		~Glyph();

		uint setup(uint16 width, uint16 height);
	};

	uint16 _height;
	uint16 _maxAdvance;
	byte _comment[32];

	Common::CodePage _codepage;

	mutable Common::SeekableReadStream *_crf;
	Common::Array<uint32> _offsets;

	typedef Common::HashMap<uint32, Glyph> GlyphCache;
	mutable GlyphCache _cache;
};

} // End of namespace CryOmni3D

#endif
