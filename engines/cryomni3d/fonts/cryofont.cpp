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

#include "common/debug.h"
#include "common/file.h"
#include "graphics/managed_surface.h"

#include "cryomni3d/fonts/cryofont.h"

namespace CryOmni3D {

void CryoFont::load(const Common::String &fontFile) {
	Common::File crf;

	if (!crf.open(fontFile)) {
		error("can't open file %s", fontFile.c_str());
	}

	byte magic[8];

	crf.read(magic, sizeof(magic));
	if (memcmp(magic, "CRYOFONT", 8)) {
		error("Invalid font magic");
	}

	// 3 unknown uint16
	(void) crf.readUint16BE();
	(void) crf.readUint16BE();
	(void) crf.readUint16BE();

	_height = crf.readSint16BE();
	//debug("Max char height %d", _maxHeight);

	crf.read(_comment, sizeof(_comment));
	//debug("Comment %s", _comment);

	loadAll8bitGlyphs(crf);
}

void CryoFont::loadAll8bitGlyphs(Common::SeekableReadStream &font_fl) {
	for (uint i = 0; i < k8bitCharactersCount; i++) {
		// Cache maps a character to a glyph
		Glyph &glyph = _glyphs[i];
		uint16 h = font_fl.readUint16BE();
		uint16 w = font_fl.readUint16BE();
		uint sz = glyph.setup(w, h);
		//debug("Char %d sz %dx%d %d", i, w, h, sz);
		glyph.offX    = font_fl.readSint16BE();
		glyph.offY    = font_fl.readSint16BE();
		glyph.advance = font_fl.readUint16BE();
		//debug("Char %d offX %d offY %d PW %d", i, glyph.offX, glyph.offY, glyph.advance);

		font_fl.read(glyph.bitmap, sz);
		//debug("Char %d read %d", i, v);

		if (glyph.advance > _maxAdvance) {
			_maxAdvance = glyph.advance;
		}
	}
}

Common::Rect CryoFont::getBoundingBox(uint32 chr) const {
	const Glyph &glyph = _glyphs[mapGlyph(chr)];
	return Common::Rect(glyph.offX, glyph.offY,
	                    glyph.offX + glyph.w, glyph.offY + glyph.h);
}

int CryoFont::getCharWidth(uint32 chr) const {
	const Glyph &glyph = _glyphs[mapGlyph(chr)];
	return glyph.advance;
}

void CryoFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	assert(dst);
	assert(dst->format.bytesPerPixel == 1 || dst->format.bytesPerPixel == 2 ||
	       dst->format.bytesPerPixel == 4);

	const Glyph &glyph = _glyphs[mapGlyph(chr)];

	x += glyph.offX;
	y += glyph.offY + _height - 2;

	if (x > dst->w) {
		return;
	}
	if (y > dst->h) {
		return;
	}

	int w = glyph.w;
	int h = glyph.h;

	const uint8 *srcPos = (const uint8 *)glyph.bitmap;

	// Make sure we are not drawing outside the screen bounds
	if (x < 0) {
		// x is negative so srcPos will increase and w will decrease
		srcPos -= x;
		w += x;
		x = 0;
	}

	if (x + w > dst->w) {
		w = dst->w - x;
	}

	if (w <= 0) {
		return;
	}

	if (y < 0) {
		// y is negative so srcPos will increase and h will decrease
		srcPos -= y * glyph.w;
		h += y;
		y = 0;
	}

	if (y + h > dst->h) {
		h = dst->h - y;
	}

	if (h <= 0) {
		return;
	}

	for (uint16 i = 0; i < h; i++) {
		for (uint16 j = 0; j < w; j++) {
			if (srcPos[j]) {
				if (dst->format.bytesPerPixel == 1) {
					*((byte *)dst->getBasePtr(x + j, y + i)) = color;
				} else if (dst->format.bytesPerPixel == 2) {
					*((uint16 *)dst->getBasePtr(x + j, y + i)) = color;
				} else if (dst->format.bytesPerPixel == 4) {
					*((uint32 *)dst->getBasePtr(x + j, y + i)) = color;
				}
			}
		}
		// Next line
		srcPos += glyph.w;
	}
}

uint32 CryoFont::mapGlyph(uint32 chr) const {
	/* Placeholder for non printable and out of limit characters */
	if (chr < 0x20 || chr >= 0xff) {
		chr = '?';
	}
	chr -= 0x20;
	return chr;
}

CryoFont::Glyph::Glyph() : offX(0), offY(0), advance(0), bitmap(nullptr) {
}

CryoFont::Glyph::~Glyph() {
	delete[] bitmap;
}

uint CryoFont::Glyph::setup(uint16 width, uint16 height) {
	w = width;
	h = height;
	uint sz = w * h;
	bitmap = new byte[sz];
	return sz;
}

} // End of namespace CryOmni3D
