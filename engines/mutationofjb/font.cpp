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

Font::Font(const Common::String &fileName, int horizSpacing, int vertSpacing) :
	_horizSpacing(horizSpacing),
	_vertSpacing(vertSpacing) {

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

		if (height > maxHeight) {
			maxHeight = height;
		}
	}

	if (_vertSpacing == -1) {
		_vertSpacing = maxHeight;
	}

	return true;
}


void Font::drawGlyph(uint8 glyph, uint8 baseColor, int16 &x, int16 &y, Graphics::ManagedSurface &surf) {
	GlyphMap::iterator it = _glyphs.find(glyph);
	if (it == _glyphs.end()) {
		warning("Glyph %d not found", glyph);
		return;
	}

	Graphics::ManagedSurface &glyphSurface = it->_value;

	Graphics::ManagedSurface tmp(glyphSurface);
	for (int h = 0; h < tmp.h; ++h) {
		uint8 *ptr = reinterpret_cast<uint8 *>(tmp.getBasePtr(0, h));
		for (int w = 0; w < tmp.w; ++w) {
			if (*ptr != 0) {
				*ptr = transformColor(baseColor, *ptr);
			}
			ptr++;
		}
	}
	surf.transBlitFrom(tmp.rawSurface(), Common::Point(x, y));

	x += glyphSurface.w + _horizSpacing;
}

void Font::drawString(const Common::String &str, uint8 baseColor, int16 x, int16 y, Graphics::ManagedSurface &surf) {
	for (uint i = 0; i < str.size(); ++i) {
		drawGlyph(str[i], baseColor, x, y, surf); // "x" is updated.
	}
}

uint8 Font::transformColor(uint8 baseColor, uint8 glyphColor) {
	return baseColor + glyphColor - 0x10;
}

SystemFont::SystemFont() : Font("sysfnt.aft", 1, 7) {}

SpeechFont::SpeechFont() : Font("font1.aft", -1, -1) {}

uint8 SpeechFont::transformColor(uint8 baseColor, uint8 glyphColor) {
	// Hack in original game.
	if (glyphColor == 0x11) {
		return 0xC0;
	}

	return Font::transformColor(baseColor, glyphColor);
}

}
