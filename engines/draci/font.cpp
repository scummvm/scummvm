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

/**
 * @brief Loads fonts from a file
 * @param path Path to font file
 * @return true if the font was loaded successfully, false otherwise
 *
 * Loads fonts from a file into a DraciFont instance. The original game uses two
 * fonts (located inside files "Small.fon" and "Big.fon"). The characters in the
 * font are indexed from the ASCII space (decimal value 32) so an appropriate
 * offset must be added to convert them to equivalent char values, 
 * i.e. kDraciIndexOffset.
 *
 * font format: [1 byte] maximum character width
 *				[1 byte] font height
 *				[138 bytes] character widths of all 138 characters in the font
 *				[138 * fontHeight * maxWidth bytes] character data, stored row-wise 
 */

bool DraciFont::setFont(Common::String &filename) {
	
	// Free previously loaded font (if any)
	freeFont();

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
	// If there is a font already loaded, free it
	if (_charData) {	
		delete[] _charWidths;
		delete[] _charData;
	}
}

uint8 DraciFont::getCharWidth(uint8 chr) const {
	return _charWidths[chr - kCharIndexOffset];
}

/**
 * @brief Draw a char to a Graphics::Surface
 * @param dst 	Pointer to the destination surface
 * @param chr 	Character to draw (ASCII value)
 * @param tx  	Horizontal offset on the surface
 * @param ty  	Vertical offset on the surface
 */

void DraciFont::drawChar(Graphics::Surface *dst, uint8 chr, int tx, int ty) const {
	assert(dst != NULL);
	assert(tx >= 0);
	assert(ty >= 0);

	byte *ptr = (byte *)dst->getBasePtr(tx, ty);
	uint8 charIndex = chr - kCharIndexOffset;
	int charOffset = charIndex * _fontHeight * _maxCharWidth;
	uint8 currentWidth = _charWidths[charIndex];

	// Determine how many pixels to draw horizontally (to prevent overflow)
	int xSpaceLeft = dst->w - tx - 1;	
	int xPixelsToDraw = (currentWidth < xSpaceLeft) ? currentWidth : xSpaceLeft;

	// Determine how many pixels to draw vertically
	int ySpaceLeft = dst->h - ty - 1;	
	int yPixelsToDraw = (_fontHeight < ySpaceLeft) ? _fontHeight : ySpaceLeft;

	for (int y = 0; y < yPixelsToDraw; ++y) {
		for (int x = 0; x <= xPixelsToDraw; ++x) {

			// Paint pixel
			int curr = y * _maxCharWidth + x;
			ptr[x] = _charData[charOffset + curr];
		}

		// Advance to next row
		ptr += dst->pitch;	
	}
}

/**
 * @brief Draw a string to a Graphics::Surface
 *
 * @param dst 		Pointer to the destination surface
 * @param str 		String to draw
 * @param x  		Horizontal offset on the surface
 * @param y  		Vertical offset on the surface
 * @param spacing 	Space to leave between individual characters. Defaults to 0. 
 */

void DraciFont::drawString(Graphics::Surface *dst, Common::String &str, 
							int x, int y, int spacing) const {
	assert(dst != NULL);
	assert(x >= 0);
	assert(y >= 0);

	int curx = x;
	uint len = str.size();

	for (unsigned int i = 0; i < len; ++i) {
		drawChar(dst, str[i], curx, y);
		curx += getCharWidth(str[i]) + spacing;
	}
}

/**
 * @brief Calculate the width of a string when drawn in the current font
 *
 * @param str 		String to draw
 * @param spacing	Space to leave between individual characters. Defaults to 0. 
 *
 * @return The calculated width of the string 
 */

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
