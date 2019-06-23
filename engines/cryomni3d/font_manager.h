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

#ifndef CRYOMNI3D_FONT_MANAGER_H
#define CRYOMNI3D_FONT_MANAGER_H

#include "common/array.h"
#include "common/str.h"
#include "common/rect.h"

namespace Common {
class ReadStream;
}

namespace Graphics {
class ManagedSurface;
}

namespace CryOmni3D {

class FontManager {
public:
	FontManager();
	virtual ~FontManager();

	void loadFonts(const Common::Array<Common::String> &fontFiles);
	void setCurrentFont(int currentFont);
	uint getCurrentFont() { return _currentFontId; }
	void setTransparentBackground(bool transparent) { _transparentBackground = transparent; }
	void setSpaceWidth(uint additionalSpace);
	void setForeColor(byte color) { _foreColor = color; }
	void setLineHeight(int h) { _lineHeight = h; }
	int lineHeight() { return _lineHeight; }
	void setCharSpacing(uint w) { _charSpacing = w; }
	void setSurface(Graphics::ManagedSurface *surface) { _currentSurface = surface; }

	int getFontMaxHeight() { return _currentFont->maxHeight; }

	void displayInt(uint x, uint y, int value) const { displayStr_(x, y, Common::String::format("%d", value)); }
	void displayStr(uint x, uint y, const Common::String &text) const { displayStr_(x, y, text); }
	uint getStrWidth(const Common::String &text) const;

	uint getLinesCount(const Common::String &text, uint width);

	void setupBlock(const Common::Rect &block, bool justifyText = false) { _blockRect = block; _blockPos.x = block.left; _blockPos.y = block.top; _justifyText = justifyText; }
	bool displayBlockText(const Common::String &text) { return displayBlockText(text, text.begin()); }
	bool displayBlockText(const Common::String &text, Common::String::const_iterator begin);
	Common::String::const_iterator blockTextRemaining() { return _blockTextRemaining; }
	Common::Point blockTextLastPos() { return _blockPos; }

private:
	void loadFont(Common::ReadStream &font_fl);
	uint displayStr_(uint x, uint y, const Common::String &text) const;
	uint displayChar(uint x, uint y, unsigned char c) const;
	void calculateWordWrap(const Common::String &text, Common::String::const_iterator *position,
	                       uint *finalPos, bool *has_br, Common::Array<Common::String> &words) const;

	struct Character {
		uint16 h;
		uint16 w;
		int16 offX;
		int16 offY;
		uint16 printedWidth;

		byte *data;

		Character();
		~Character();

		uint setup(uint16 width, uint16 height);
	};

	struct Font {
		static const uint kCharactersCount = 223;

		uint16 maxHeight;
		byte comment[32];
		Character chars[kCharactersCount];
	};

	Common::Array<Font *> _fonts;
	const Font *_currentFont;
	uint _currentFontId;
	bool _transparentBackground;
	uint _spaceWidth;
	uint _charSpacing;

	byte _foreColor;

	Graphics::ManagedSurface *_currentSurface;

	Common::Rect _blockRect;
	Common::Point _blockPos;
	int _lineHeight;
	bool _justifyText;
	Common::String::const_iterator _blockTextRemaining;
};

} // End of namespace CryOmni3D

#endif
