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

#ifndef IGOR_FLOPPY_FONT_H
#define IGOR_FLOPPY_FONT_H

#include "graphics/font.h"

namespace Igor {

/**
 * Common part of the two Spanish floppy fonts: the shared character map and
 * the drawing code. Glyph pixels are 1 (text color) or 2 (shadow color).
 */
class FloppyFont : public Graphics::Font {
public:
	int getFontHeight() const override { return kFontHeight; }
	int getMaxCharWidth() const override { return _glyphWidth; }
	int getCharWidth(uint32 chr) const override;
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;

	// Engine-specific: raw buffer rendering with shadow color
	using Graphics::Font::drawChar;
	void drawString(uint8 *dst, const char *s, int x, int y, int color1, int color2) const;
	void drawChar(uint8 *dst, int chr, int x, int y, int color1, int color2) const;
	int getStringWidth(const char *s) const;

protected:
	FloppyFont(const uint8 *widths, const uint32 *glyphs, int glyphWidth);

private:
	static const int kFontHeight = 11;
	static const int kGlyphCount = 91;
	static const int kSpaceWidth = 5;

	int getGlyph(uint32 chr) const;

	const uint8 *_widths;
	const uint32 *_glyphs;
	int _glyphWidth;

	static const uint8 _charIndex[];
};

/**
 * Spanish floppy dialogue font, 9x11 glyphs.
 */
class FloppyDialogueFont : public FloppyFont {
public:
	FloppyDialogueFont();

private:
	static const uint8 _charWidth[];
	static const uint32 _charData[];
};

/**
 * Spanish floppy action sentence font, 7x11 glyphs.
 */
class FloppySentenceFont : public FloppyFont {
public:
	FloppySentenceFont();

private:
	static const uint8 _charWidth[];
	static const uint32 _charData[];
};

} // End of namespace Igor

#endif // IGOR_FLOPPY_FONT_H
