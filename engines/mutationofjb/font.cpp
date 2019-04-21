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

#include "mutationofjb/font.h"
#include "mutationofjb/encryptedfile.h"
#include "mutationofjb/util.h"
#include "common/debug.h"

namespace MutationOfJB {

Font::Font(const Common::String &fileName, int horizSpacing, int lineHeight) :
	_horizSpacing(horizSpacing),
	_lineHeight(lineHeight),
	_maxCharWidth(0) {

	load(fileName);
}

bool Font::load(const Common::String &fileName) {
	EncryptedFile file;
	file.open(fileName);
	if (!file.isOpen()) {
		reportFileMissingError(fileName.c_str());
		return false;
	}

	file.seek(0x02D6, SEEK_SET); // Skip header + unknown data (unused palette?).

	uint16 noGlyphs = 0;
	noGlyphs = file.readUint16LE();

	file.seek(7, SEEK_CUR); // Skip unknown data (0s).

	uint8 maxHeight = 0;

	while (noGlyphs--) {
		const uint8 character = file.readByte();
		const uint8 width = file.readByte();
		const uint8 height = file.readByte();

		Graphics::ManagedSurface &surf = _glyphs[character];
		surf.create(width, height);
		for (int h = 0; h < height; ++h) {
			file.read(surf.getBasePtr(0, h), width);
		}

		if (width > _maxCharWidth) {
			_maxCharWidth = width;
		}

		if (height > maxHeight) {
			maxHeight = height;
		}
	}

	if (_lineHeight == -1) {
		_lineHeight = maxHeight;
	}

	return true;
}

int Font::getFontHeight() const {
	return _lineHeight;
}

int Font::getMaxCharWidth() const {
	return _maxCharWidth;
}

int Font::getCharWidth(uint32 chr) const {
	GlyphMap::iterator it = _glyphs.find(chr);
	if (it == _glyphs.end()) {
		return 0;
	}
	return it->_value.w;
}

int Font::getKerningOffset(uint32 left, uint32 right) const {
	if (left == 0) {
		// Do not displace the first character.
		return 0;
	}

	if (_glyphs.find(left) == _glyphs.end()) {
		// Missing glyphs must not create extra displacement.
		// FIXME: This way is not completely correct, as if the last character is
		// missing a glyph, it will still create extra displacement. This should
		// not affect the visuals but it might affect getStringWidth() / getBoundingBox().
		return 0;
	}

	return _horizSpacing;
}

class FontBlitOperation {
public:
	FontBlitOperation(const Font &font, const byte baseColor)
		: _font(font),
		  _baseColor(baseColor) {}

	byte operator()(const byte srcColor, const byte destColor) {
		if (srcColor == 0) {
			// Transparency - keep destination color.
			return destColor;
		}

		// Replace destination with transformed source color.
		return _font.transformColor(_baseColor, srcColor);
	}

private:
	const Font &_font;
	const byte _baseColor;
};

void Font::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	GlyphMap::iterator it = _glyphs.find(chr);
	if (it == _glyphs.end()) {
		// Missing glyph is a common situation in the game and it's okay to ignore it.
		return;
	}

	Graphics::ManagedSurface &glyphSurface = it->_value;
	blit_if(glyphSurface, *dst, Common::Point(x, y), FontBlitOperation(*this, color));
}

uint8 Font::transformColor(uint8 baseColor, uint8 glyphColor) const {
	return baseColor + glyphColor - 0x10;
}

SystemFont::SystemFont() : Font("sysfnt.aft", 1, 7) {}

SpeechFont::SpeechFont() : Font("font1.aft", -1, -1) {}

uint8 SpeechFont::transformColor(uint8 baseColor, uint8 glyphColor) const {
	// Hack in original game.
	if (glyphColor == 0x11) {
		return 0xC0;
	}

	return Font::transformColor(baseColor, glyphColor);
}

}
