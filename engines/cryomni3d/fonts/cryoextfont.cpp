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

#include "cryomni3d/fonts/cryoextfont.h"

namespace CryOmni3D {

CryoExtFont::~CryoExtFont() {
	delete _crf;
}

void CryoExtFont::load(const Common::String &fontFile, Common::CodePage codepage) {
	// For now only CP950 is supported
	assert(codepage == Common::kWindows950);

	_codepage = codepage;

	Common::File *crf = new Common::File();

	if (!crf->open(fontFile)) {
		error("can't open file %s", fontFile.c_str());
	}

	_crf = crf;

	byte magic[8];

	_crf->read(magic, sizeof(magic));
	if (memcmp(magic, "CRYOFONT", 8)) {
		error("Invalid font magic");
	}

	// 3 unknown uint16
	(void) _crf->readUint16BE();
	(void) _crf->readUint16BE();
	(void) _crf->readUint16BE();

	_height = _crf->readSint16BE();
	//debug("Max char height %d", _maxHeight);

	_crf->read(_comment, sizeof(_comment));
	//debug("Comment %s", _comment);

	Common::String offsetsFile = fontFile;
	offsetsFile.setChar('I', offsetsFile.size() - 1);
	loadOffsets(offsetsFile);
}

void CryoExtFont::loadOffsets(const Common::String &offsetsFile) {
	Common::File cri;

	if (!cri.open(offsetsFile)) {
		error("can't open file %s", offsetsFile.c_str());
	}

	uint32 counts = cri.size() / sizeof(uint32);
	_offsets.reserve(counts);
	debug("Loading %u offsets", counts);

	for (; counts > 0; counts--) {
		uint32 offset = cri.readUint32BE();
		_offsets.push_back(offset);
	}
}

void CryoExtFont::assureCached(uint32 chr) const {
	if (_cache.contains(chr)) {
		return;
	}

	uint32 glyphId = mapGlyph(chr);

	if (glyphId >= _offsets.size()) {
		warning("Invalid glyph id: %u", glyphId);
		glyphId = 0;
	}

	uint32 offset = _offsets[glyphId];
	_crf->seek(offset);

	Glyph &glyph = _cache[chr];
	uint16 h = _crf->readUint16BE();
	uint16 w = _crf->readUint16BE();
	uint sz = glyph.setup(w, h);
	//debug("Char %u/%u sz %ux%u %u", chr, glyphId, w, h, sz);
	glyph.offX    = _crf->readSint16BE();
	glyph.offY    = _crf->readSint16BE();
	glyph.advance = _crf->readUint16BE();
	//debug("Char %u/%u offX %d offY %d PW %d", chr, glyphId, glyph.offX, glyph.offY, glyph.advance);

	_crf->read(glyph.bitmap, sz);
	//debug("Char %u/%u read %d", chr, glyphId, v);

	if ((_cache.size() % 10) == 0) {
		debug("Glyph cache size is now %u", _cache.size());
	}
}

Common::Rect CryoExtFont::getBoundingBox(uint32 chr) const {
	assureCached(chr);

	const Glyph &glyph = _cache[chr];
	return Common::Rect(glyph.offX, glyph.offY,
	                    glyph.offX + glyph.w, glyph.offY + glyph.h);
}

int CryoExtFont::getCharWidth(uint32 chr) const {
	assureCached(chr);

	const Glyph &glyph = _cache[chr];
	return glyph.advance;
}

void CryoExtFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	assert(dst);
	assert(dst->format.bytesPerPixel == 1 || dst->format.bytesPerPixel == 2 ||
	       dst->format.bytesPerPixel == 4);

	assureCached(chr);

	const Glyph &glyph = _cache[chr];

	x += glyph.offX;
	// In european versions of Versailles there was a -2 offset which is not present in Chinese one
	// Maybe this offset should be checked when adding new games
	y += glyph.offY + _height;

	if (x > dst->w) {
		return;
	}
	if (y > dst->h) {
		return;
	}

	int w = glyph.w;
	int h = glyph.h;

	const uint8 *srcPos = (const uint8 *)glyph.bitmap;

	const uint16 stride = (glyph.w + 7) / 8;
	const uint16 originalWidth = glyph.w;

	// Make sure we are not drawing outside the screen bounds
	if (y < 0) {
		// y is negative so srcPos will increase and h will decrease
		srcPos -= y * stride;
		h += y;
		y = 0;
	}

	if (y + h > dst->h) {
		h = dst->h - y;
	}

	if (h <= 0) {
		return;
	}

	int jStart = 0;
	if (x < 0) {
		// x is negative so jStart will be positive and w will decrease
		jStart = -x;
		w += x;
		x = 0;
	}

	if (x + w > dst->w) {
		w = dst->w - x;
	}

	if (w <= 0) {
		return;
	}

	const int jEnd = jStart + w - 1;

	for (uint16 i = 0; i < h; i++) {
		byte b = 0;
		for (uint16 j = 0; j < originalWidth; j++) {
			if ((j % 8) == 0) {
				b = *(srcPos++);
			}

			if (j >= jStart && j <= jEnd && b & 0x80) {
				if (dst->format.bytesPerPixel == 1) {
					*((byte *)dst->getBasePtr(x + j, y + i)) = color;
				} else if (dst->format.bytesPerPixel == 2) {
					*((uint16 *)dst->getBasePtr(x + j, y + i)) = color;
				} else if (dst->format.bytesPerPixel == 4) {
					*((uint32 *)dst->getBasePtr(x + j, y + i)) = color;
				}
			}

			b <<= 1;
		}
	}
}

uint32 CryoExtFont::mapGlyph(uint32 chr) const {
	switch (_codepage) {
	case Common::kWindows950:
		/* Nothing more than 0xffff */
		if (chr >> 16) {
			return 0;
		}
		/* No glyph for non printable */
		if (chr < 0x20) {
			return 0;
		}
		/* ASCII characters : like in 8bits case, 0x60 ones */
		if (chr < 0x80) {
			return chr - 0x20;
		}
		/* Invalid ranges */
		if (chr < 0x8000) {
			return 0;
		}
		if ((chr & 0xff) < 0x40) {
			return 0;
		}
		/* After the 0x60 ASCII characters
		 * a table of 0xC0 large (starting at 0x40) and
		 * 0x80 long (starting at 0x80) */
		chr = 0xC0 * ((chr >> 8) - 0x80) + ((chr & 0xff) - 0x40) + 0x60;
		return chr;
	default:
		error("Invalid encoding");
	}
}

CryoExtFont::Glyph::Glyph() : offX(0), offY(0), advance(0), bitmap(nullptr) {
}

CryoExtFont::Glyph::~Glyph() {
	delete[] bitmap;
}

uint CryoExtFont::Glyph::setup(uint16 width, uint16 height) {
	w = width;
	h = height;
	uint sz = h * ((w + 7) / 8);
	bitmap = new byte[sz];
	return sz;
}

} // End of namespace CryOmni3D
