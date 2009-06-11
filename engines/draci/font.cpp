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
 * $URL$
 * $Id$
 *
 */

#include "common/file.h"

#include "draci/draci.h"
#include "draci/font.h"

namespace Draci {

DraciFont::DraciFont(Common::String &filename) : 
	_fontHeight(0), _maxCharWidth(0), 
	_charWidths(NULL), _charData(0) { 
	setFont(filename); 
}

DraciFont::~DraciFont() {
	 freeFont(); 
}

bool DraciFont::setFont(Common::String &filename) {
	
	// If there is a font already loaded, free it
	if (_charData) {
		freeFont();
	}

	Common::File f;

	f.open(filename);
	if (f.isOpen()) {
		debugC(6, kDraciGeneralDebugLevel, "Opened font file %s", 
			filename.c_str());
	} else {
		debugC(6, kDraciGeneralDebugLevel, "Error opening font file %s", 
			filename.c_str());
		return false;
	}

	_maxCharWidth = f.readByte();
	_fontHeight = f.readByte();

	// Read in the widths of the glyphs	
	_charWidths = new uint8[kCharNum];
	for (unsigned int i = 0; i < kCharNum; ++i) {
		_charWidths[i] = f.readByte();
	}

	// Calculate size of font data
	unsigned int fontDataSize = kCharNum * _maxCharWidth * _fontHeight;

	// Read in all glyphs
	_charData = new byte[fontDataSize];
	f.read(_charData, fontDataSize);

	debugC(5, kDraciGeneralDebugLevel, "Font %s loaded", filename.c_str()); 

	return true;
}

void DraciFont::freeFont() {
	delete[] _charWidths;
	delete[] _charData;
}

uint8 DraciFont::getCharWidth(uint8 chr) const {
	return _charWidths[chr - kCharIndexOffset];
}

void DraciFont::drawChar(Graphics::Surface *dst, uint8 chr, int tx, int ty) const {
	assert(dst != NULL);
	byte *ptr = (byte *)dst->getBasePtr(tx, ty);
	uint8 charIndex = chr - kCharIndexOffset;
	int charOffset = charIndex * _fontHeight * _maxCharWidth;
	uint8 currentWidth = _charWidths[charIndex];

	for (uint8 y = 0; y < _fontHeight; ++y) {

		// Check for vertical overflow
		if (ty + y < 0 || ty + y >= dst->h) {
			continue;
		}

		for (uint8 x = 0; x <= currentWidth; ++x) {
			
			// Check for horizontal overflow
			if (tx + x < 0 || tx + x >= dst->w) {
				continue;
			}
			
			// Paint pixel
			int curr = ((int)y) * _maxCharWidth + x;
			ptr[x] = _charData[charOffset + curr];
		}

		// Advance to next row
		ptr += dst->pitch;	
	}
}

void DraciFont::drawString(Graphics::Surface *dst, Common::String str, 
							int x, int y, int spacing) const {
	assert(dst != NULL);	
	int curx = x;
	uint len = str.size();
	for (unsigned int i = 0; i < len; ++i) {
		drawChar(dst, str[i], curx, y);
		curx += getCharWidth(str[i]) + spacing;
	}
}

int DraciFont::getStringWidth(Common::String &str, int spacing) const {
	int width = 0;	
	uint len = str.size();
	for (unsigned int i = 0; i < len; ++i) {
		uint8 charIndex = str[i] - kCharIndexOffset;
		width += _charWidths[charIndex];
	}

	// Add width of spaces, if any
	width += (len - 1) * spacing;

	return width;
}

} // End of namespace Draci
