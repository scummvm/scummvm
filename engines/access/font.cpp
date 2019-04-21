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

#include "access/font.h"

namespace Access {

byte Font::_fontColors[4];

Font::Font(byte firstCharIndex) : _firstCharIndex(firstCharIndex), _bitWidth(0), _height(0) {
}

Font::~Font() {
	for (uint i = 0; i < _chars.size(); ++i)
		_chars[i].free();
}

int Font::charWidth(char c) {
	if (c < _firstCharIndex)
		return 0;

	return _chars[c - _firstCharIndex].w;
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
			s.clear();
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

void Font::drawString(BaseSurface *s, const Common::String &msg, const Common::Point &pt) {
	Common::Point currPt = pt;
	const char *msgP = msg.c_str();

	while (*msgP) {
		currPt.x += drawChar(s, *msgP, currPt);
		++msgP;
	}
}

int Font::drawChar(BaseSurface *s, char c, Common::Point &pt) {
	Graphics::Surface &ch = _chars[c - _firstCharIndex];
	Graphics::Surface dest = s->getSubArea(Common::Rect(pt.x, pt.y, pt.x + ch.w, pt.y + ch.h));

	// Loop through the lines of the character
	for (int y = 0; y < ch.h; ++y) {
		byte *pSrc = (byte *)ch.getBasePtr(0, y);
		byte *pDest = (byte *)dest.getBasePtr(0, y);

		// Loop through the horizontal pixels of the line
		for (int x = 0; x < ch.w; ++x, ++pSrc, ++pDest) {
			if (*pSrc != 0)
				*pDest = _fontColors[*pSrc];
		}
	}

	return ch.w;
}

/*------------------------------------------------------------------------*/

void AmazonFont::load(const int *fontIndex, const byte *fontData) {
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

/*------------------------------------------------------------------------*/

MartianFont::MartianFont(int height, Common::SeekableReadStream &s) : Font(0) {
	_height = height;
	load(s);
}

void MartianFont::load(Common::SeekableReadStream &s) {
	// Get the number of characters and the size of the raw font data
	size_t count = s.readUint16LE();
	size_t dataSize = s.readUint16LE();
	assert(count < 256);

	// Get the character widths
	Common::Array<byte> widths;
	widths.resize(count);
	s.read(&widths[0], count);

	// Get the character offsets
	Common::Array<int> offsets;
	offsets.resize(count);
	for (size_t idx = 0; idx < count; ++idx)
		offsets[idx] = s.readUint16LE();

	// Get the raw character data
	Common::Array<byte> data;
	data.resize(dataSize);
	s.read(&data[0], dataSize);

	// Iterate through decoding each character
	_chars.resize(count);
	for (size_t idx = 0; idx < count; ++idx) {
		Graphics::Surface &surface = _chars[idx];
		surface.create(widths[idx], _height, Graphics::PixelFormat::createFormatCLUT8());
		const byte *srcP = &data[offsets[idx]];
		int x1, y1, x2, y2;

		// Write horizontal lines
		while ((x1 = *srcP++) != 0xff) {
			x2 = *srcP++;
			y1 = *srcP++;
			surface.hLine(x1, y1, x2, 3);
		}

		// Write vertical lines
		while ((x1 = *srcP++) != 0xff) {
			y1 = *srcP++;
			y2 = *srcP++;
			surface.vLine(x1, y1, y2, 3);
		}
	}
}

/*------------------------------------------------------------------------*/

FontManager::FontManager() : _font1(nullptr), _font2(nullptr) {
	_printMaxX = 0;
	Common::fill(&Font::_fontColors[0], &Font::_fontColors[4], 0);
}

void FontManager::load(Font *font1, Font *font2) {
	_font1 = font1;
	_font2 = font2;
}


} // End of namespace Access
