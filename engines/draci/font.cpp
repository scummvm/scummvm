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

const Common::String kFontSmall("Small.fon");
const Common::String kFontBig("Big.fon"); 

Font::Font(const Common::String &filename) { 

	_fontHeight = 0;
	_maxCharWidth = 0;
	_charWidths = NULL;
	_charData = NULL;

	loadFont(filename);

	_currentFontColour = kFontColour1;
}

Font::~Font() {
	 freeFont(); 
}

/**
 * @brief Sets the varying font colour
 * @param colour The new font colour
 */

void Font::setColour(uint8 colour) {
	_currentFontColour = colour;
}

/**
 * @brief Loads fonts from a file
 * @param path Path to font file
 * @return true if the font was loaded successfully, false otherwise
 *
 * Loads fonts from a file into a Font instance. The original game uses two
 * fonts (located inside files "Small.fon" and "Big.fon"). The characters in the
 * font are indexed from the space character so an appropriate offset must be 
 * added to convert them to equivalent char values, i.e. kDraciIndexOffset.
 * Characters in the higher range are non-ASCII and vary between different
 * language versions of the game.
 *
 * font format: [1 byte] maximum character width
 *				[1 byte] font height
 *				[138 bytes] character widths of all 138 characters in the font
 *				[138 * fontHeight * maxWidth bytes] character data, stored row-wise 
 */

bool Font::loadFont(const Common::String &filename) {
	
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

void Font::freeFont() {
	delete[] _charWidths;
	delete[] _charData;
}

uint8 Font::getCharWidth(uint8 chr) const {
	return _charWidths[chr - kCharIndexOffset];
}

/**
 * @brief Draw a char to a Draci::Surface
 *
 * @param dst 	Pointer to the destination surface
 * @param chr 	Character to draw
 * @param tx  	Horizontal offset on the surface
 * @param ty  	Vertical offset on the surface
 */

void Font::drawChar(Surface *dst, uint8 chr, int tx, int ty, bool markDirty) const {
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

	int _transparent = dst->getTransparentColour();

	for (int y = 0; y < yPixelsToDraw; ++y) {
		for (int x = 0; x <= xPixelsToDraw; ++x) {

			int curr = y * _maxCharWidth + x;
			int colour = _charData[charOffset + curr];

			// Replace colour with font colours
			switch (colour) {

			case 254:
				colour = _currentFontColour;
				break;

			case 253:
				colour = kFontColour2;
				break;

			case 252:
				colour = kFontColour3;
				break;

			case 251:
				colour = kFontColour4;
				break;
			}
			
			// Paint pixel (if not transparent)
			if (colour != _transparent)			
				ptr[x] = colour;
		}

		// Advance to next row
		ptr += dst->pitch;	
	}

	if (markDirty) {
		Common::Rect r(tx, ty, tx + xPixelsToDraw, ty + yPixelsToDraw);
		dst->markDirtyRect(r);
	}
}

/**
 * @brief Draw a string to a Draci::Surface
 *
 * @param dst 		Pointer to the destination surface
 * @param str 		Buffer containing string data
 * @param len		Length of the data
 * @param x  		Horizontal offset on the surface
 * @param y  		Vertical offset on the surface
 * @param spacing 	Space to leave between individual characters. Defaults to 0. 
 */

void Font::drawString(Surface *dst, const byte *str, uint len, 
							int x, int y, int spacing, bool markDirty) const {
	assert(dst != NULL);
	assert(x >= 0);
	assert(y >= 0);

	int curx = x;

	for (unsigned int i = 0; i < len; ++i) {
		
		// Return early if there's no more space on the screen	
		if (curx >= dst->w) {
			return;
		}		
			
		drawChar(dst, str[i], curx, y, markDirty);
		curx += getCharWidth(str[i]) + spacing;
	}
}

/**
 * @brief Draw a string to a Draci::Surface
 *
 * @param dst 		Pointer to the destination surface
 * @param str 		String to draw
 * @param x  		Horizontal offset on the surface
 * @param y  		Vertical offset on the surface
 * @param spacing 	Space to leave between individual characters. Defaults to 0. 
 */

void Font::drawString(Surface *dst, const Common::String &str, 
							int x, int y, int spacing, bool markDirty) const {

	drawString(dst, (byte *) str.c_str(), str.size(), x, y, spacing, markDirty);
}

/**
 * @brief Calculate the width of a string when drawn in the current font
 *
 * @param str 		String to draw
 * @param spacing	Space to leave between individual characters. Defaults to 0. 
 *
 * @return The calculated width of the string 
 */

int Font::getStringWidth(const Common::String &str, int spacing) const {
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
