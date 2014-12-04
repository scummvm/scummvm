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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "access/font.h"

namespace Access {

byte Font::_fontColors[4];

Font::Font() {
}

Font::~Font() {
	for (uint i = 0; i < _chars.size(); ++i)
		_chars[i].free();
}

void Font::load(const int *fontIndex, const byte *fontData) {
	assert(_chars.size() == 0);
	int count = fontIndex[0];
	_bitWidth = fontIndex[1];
	_height = fontIndex[2];

	_chars.resize(count);

	for (int i = 0; i < count; ++i) {
		const byte *pData = fontData + fontIndex[i + 3];
		_chars[i].create(*pData++, _height, Graphics::PixelFormat::createFormatCLUT8());

		for (int y = 0; y < _height; ++y) {
			int bitsLeft = 0;
			byte srcByte = 0;
			byte pixel;

			byte *pDest = (byte *)_chars[i].getBasePtr(0, y);
			for (int x = 0; x < _chars[i].w; ++x, ++pDest) {
				// Get the pixel
				pixel = 0;
				for (int pixelCtr = 0; pixelCtr < _bitWidth; ++pixelCtr, --bitsLeft) {
					// No bits in current byte left, so get next byte
					if (bitsLeft == 0) {
						bitsLeft = 8;
						srcByte = *pData++;
					}

					pixel = (pixel << 1) | (srcByte >> 7);
					srcByte <<= 1;
				}

				// Write out the pixel
				*pDest = pixel;
			}
		}
	}
}

int Font::charWidth(char c) {
	if (c < ' ')
		return 0;

	return _chars[c - ' '].w;
}

int Font::stringWidth(const Common::String &msg) {
	int total = 0;

	for (const char *c = msg.c_str(); *c != '\0'; ++c)
		total += charWidth(*c);

	return total;
}

bool Font::getLine(Common::String &s, int maxWidth, Common::String &line, int &width) {
	assert(maxWidth > 0);
	width = 0;
	const char *src = s.c_str();
	char c;

	while ((c = *src) != '\0') {
		if (c == '\r') {
			// End of line, so return calculated line
			line = Common::String(s.c_str(), src);
			s = Common::String(src + 1);
			return false;
		}

		++src;
		width += charWidth(c);
		if (width < maxWidth)
			continue;

		// Reached maximum allowed size
		// If this was the last character of the string, let it go
		if (*src == '\0') {
			line = Common::String(s.c_str(), src);
			s = "";
			return true;
		}

		// Work backwards to find space at the start of the current word
		// as a point to split the line on
		while (src >= s.c_str() && *src != ' ') {
			width -= charWidth(*src);
			--src;
		}
		if (src < s.c_str())
			error("Could not fit line");

		// Split the line around the space
		line = Common::String(s.c_str(), src);
		s = Common::String(src + 1);
		return false;
	}

	// Return entire string
	line = s;
	s = Common::String();
	return true;
}

void Font::drawString(ASurface *s, const Common::String &msg, const Common::Point &pt) {
	Common::Point currPt = pt;
	const char *msgP = msg.c_str();

	while (*msgP) {
		currPt.x += drawChar(s, *msgP, currPt);
		++msgP;
	}
}

int Font::drawChar(ASurface *s, char c, Common::Point &pt) {
	Graphics::Surface &ch = _chars[c - ' '];

	// Loop through the lines of the character
	for (int y = 0; y < ch.h; ++y) {
		byte *pSrc = (byte *)ch.getBasePtr(0, y);
		byte *pDest = (byte *)s->getBasePtr(pt.x, pt.y + y);

		// Loop through the horizontal pixels of the line
		for (int x = 0; x < ch.w; ++x, ++pSrc, ++pDest) {
			if (*pSrc != 0)
				*pDest = _fontColors[*pSrc];
		}
	}

	return ch.w;
}

/*------------------------------------------------------------------------*/

FontManager::FontManager() {
	_printMaxX = 0;
	Common::fill(&Font::_fontColors[0], &Font::_fontColors[4], 0);
}

} // End of namespace Access
