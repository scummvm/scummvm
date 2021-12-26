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

#ifndef TOON_FONT_H
#define TOON_FONT_H

#include "toon/toon.h"

namespace Toon {

class DemoFont;

class FontRenderer {
public:
	FontRenderer(ToonEngine *vm);
	~FontRenderer();

	void setFont(Animation *font);
	bool loadDemoFont(const Common::String &filename);
	void computeSize(const Common::String &origText, int16 *retX, int16 *retY);
	void renderText(int16 x, int16 y, const Common::String &origText, int32 mode);
	void renderMultiLineText(int16 x, int16 y, const Common::String &origText, int32 mode, Graphics::Surface &frame);
	void setFontColorByCharacter(int32 characterId);
	void setFontColor(int32 fontColor1, int32 fontColor2, int32 fontColor3);
protected:
	Animation *_currentFont;
	DemoFont *_currentDemoFont;
	ToonEngine *_vm;
	byte _currentFontColor[4];
	byte textToFont(byte c);
};

struct GlyphDimensions {
	uint8 width;
	uint8 heightOffset; // # lines from top
	uint8 height;
};

// The font format used by the English demo.
class DemoFont {
public:
	DemoFont(uint8 glyphWidth, uint8 glyphHeight, uint16 numGlyphs);
	~DemoFont();

	uint8 *getGlyphData();
	uint8 *getGlyphData(uint8 glyphNum);

	uint8 getGlyphWidth(uint8 glyphNum);
	uint8 getHeight();
	void setGlyphDimensions(uint8 glyphNum, GlyphDimensions &glyphDimensions);

	void drawGlyph(Graphics::Surface &surface, int32 glyphNum, int16 xx, int16 yy, byte *colorMap);

protected:
	uint16 _numGlyphs;
	uint8 _glyphWidth;
	uint8 _glyphHeight;

	uint8 *_glyphData;
	GlyphDimensions *_glyphDimensions;
};

} // End of namespace Toon

#endif
