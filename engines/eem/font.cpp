/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/textconsole.h"

#include "graphics/fonts/macfont.h"
#include "graphics/surface.h"

#include "eem/detection.h"
#include "eem/font.h"
#include "eem/resource.h"

namespace EEM {

// CHR2FNT @ 29b6:0000. FONT.FNT layout:
//   0..26  : ' ' .. ':'
//   27..32 : ';' '<' '=' '>' '?' '@'
//   33..58 : A..Z
//   59..84 : a..z
// Original aliases uppercase to lowercase glyphs; we route uppercase
// ASCII to slots 33..58 for mixed-case rendering.
const byte kCharToGlyph[128] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // 0x20..0x27 ' '..'\''
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, // 0x28..0x2F '('..'/'
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, // 0x30..0x37 '0'..'7'
	0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, // 0x38..0x3F '8','9',':',';','<','=','>','?'
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, // 0x40..0x47 '@','A'..'G'
	0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, // 0x48..0x4F 'H'..'O'
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, // 0x50..0x57 'P'..'W'
	0x38, 0x39, 0x3A, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x58..0x5F 'X','Y','Z'..
	0x00, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, // 0x60..0x67 '`','a'..'g'
	0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, // 0x68..0x6F 'h'..'o'
	0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, // 0x70..0x77 'p'..'w'
	0x52, 0x53, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00  // 0x78..0x7F 'x','y','z'..
};

inline byte mapChar(uint32 c) {
	return c < 128 ? kCharToGlyph[c] : 0;
}

EEMFont::~EEMFont() {
	clear();
}

void EEMFont::clear() {
	_glyphs.clear();
	delete _macFont;
	_macFont = nullptr;
	_maxHeight = _maxWidth = _lineHeight = 0;
}

bool EEMFont::load(const Common::Path &path) {
	clear();

	Common::File f;
	if (!f.open(path)) {
		warning("EEMFont::load: cannot open %s", path.toString().c_str());
		return false;
	}

	const uint16 numChars = f.readUint16LE();
	if (numChars == 0 || numChars > 256) {
		warning("EEMFont::load: %s reports %u chars",
				path.toString().c_str(), numChars);
		return false;
	}
	_glyphs.resize(numChars);
	_maxHeight = _maxWidth = 0;
	_lineHeight = 0;

	for (uint i = 0; i < numChars; i++) {
		FontGlyph &g = _glyphs[i];
		g.height    = f.readByte();
		g.widthBits = f.readByte();
		g.sizeBytes = f.readByte();
		if (g.sizeBytes > 0) {
			g.bitmap.resize(g.sizeBytes);
			if (f.read(g.bitmap.data(), g.sizeBytes) != g.sizeBytes) {
				warning("EEMFont::load: short bitmap read at glyph %u", i);
				return false;
			}
		}
		if (g.height > _maxHeight)
			_maxHeight = g.height;
		if (g.widthBits > _maxWidth)
			_maxWidth = g.widthBits;
	}

	// _LoadFont @ 1b03:0220 sets line stride to the first glyph's height
	// Descenders ('g','j','p','q','y') intentionally overhang into
	// the next row.
	_lineHeight = !_glyphs.empty() ? _glyphs[0].height : _maxHeight;
	if (_lineHeight == 0)
		_lineHeight = _maxHeight;

	debugC(1, kDebugGfx, "Font %s loaded: %u glyphs, max %ux%u",
		   path.toString().c_str(), numChars, _maxWidth, _maxHeight);
	return true;
}

bool EEMFont::loadMacResource(const Common::Path &path, uint16 resourceId,
							  int size) {
	clear();

	Common::SeekableReadStream *stream =
		openMacResource(path, MKTAG('F', 'O', 'N', 'T'), resourceId);
	if (!stream)
		return false;

	Graphics::MacFONTFont *font = new Graphics::MacFONTFont();
	if (!font->loadFont(*stream, nullptr, size, 0)) {
		delete stream;
		delete font;
		return false;
	}
	delete stream;

	_macFont = font;
	debugC(1, kDebugGfx, "Mac FONT %u loaded from %s: %dx%d",
		   resourceId, path.toString().c_str(), _macFont->getMaxCharWidth(),
		   _macFont->getFontHeight());
	return true;
}

int EEMFont::getFontHeight() const {
	if (_macFont)
		return _macFont->getFontHeight();
	return _lineHeight ? _lineHeight : _maxHeight;
}

int EEMFont::getMaxCharWidth() const {
	if (_macFont)
		return _macFont->getMaxCharWidth();
	return _maxWidth;
}

int EEMFont::getCharWidth(uint32 chr) const {
	if (_macFont)
		return _macFont->getCharWidth(chr);

	const byte gi = mapChar(chr);
	if (gi >= _glyphs.size())
		return 0;
	return _glyphs[gi].widthBits;
}

int EEMFont::drawWordWrapped(Graphics::ManagedSurface *dst, int x, int y,
							 int width, const Common::String &s,
							 uint32 color) const {
	Common::Array<Common::String> lines;
	wordWrapText(s, width, lines);
	const int lineH = getFontHeight();
	for (uint i = 0; i < lines.size(); i++)
		drawString(dst, lines[i], x, y + (int)i * lineH, width, color);
	return (int)lines.size() * lineH;
}

void EEMFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y,
					   uint32 color) const {
	if (!dst)
		return;

	if (_macFont) {
		_macFont->drawChar(dst, chr, x, y, color);
		return;
	}

	const byte gi = mapChar(chr);
	if (gi >= _glyphs.size())
		return;
	const FontGlyph &g = _glyphs[gi];
	if (g.bitmap.empty())
		return;

	const uint bytesPerRow = (g.widthBits + 7) / 8;
	for (uint row = 0; row < g.height; row++) {
		const int dstY = y + (int)row;
		if (dstY < 0 || dstY >= dst->h)
			continue;
		const byte *srcRow = g.bitmap.data() + row * bytesPerRow;
		byte *dstRow = (byte *)dst->getBasePtr(0, dstY);
		for (uint bit = 0; bit < g.widthBits; bit++) {
			const int dstX = x + (int)bit;
			if (dstX < 0 || dstX >= dst->w)
				continue;
			const byte mask = (byte)(0x80 >> (bit & 7));
			if (srcRow[bit / 8] & mask)
				dstRow[dstX] = (byte)color;
		}
	}
}

} // End of namespace EEM
