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
#include "common/str-enc.h"
#include "common/ustr.h"
#include "common/rect.h"

#include "graphics/font.h"

namespace Graphics {
class ManagedSurface;
}

namespace CryOmni3D {

// This is a special simplified U32String which can contain either conforming UTF-32 data, 16-bits CJK codepoints or 8-bits characters
// We must use this because depending on the localization, all of those were used and we don't always know which encoding is which
// This class is only used when displaying stuff, hence its location
class CryoString : public Common::BaseString<Common::u32char_type_t> {
public:
	CryoString() : Common::BaseString<Common::u32char_type_t>() {}

	CryoString(const CryoString &str) : Common::BaseString<Common::u32char_type_t>(str) {}
	CryoString &operator=(const CryoString &str) {
		assign(str);
		return *this;
	}

	// Constructor to build UTF-32 based strings
	CryoString(const Common::U32String &ustr) : Common::BaseString<Common::u32char_type_t>(ustr) {}

	// Constructors to build 8-bits strings
	CryoString(const char *str, uint32 len) {
		initWithChars(str, len);
	}
	explicit CryoString(const char *str) {
		if (str == nullptr) {
			return;
		}
		uint32 len = 0;
		const char *s = str;
		while (*s++) {
			++len;
		}
		initWithChars(str, len);
	}
	CryoString(const Common::String &str) {
		initWithChars(str.c_str(), str.size());
	}

	// Used when building 16-bits CJK strings
	CryoString &operator+=(value_type c) {
		assignAppend(c);
		return *this;
	}

	// Various constructors
	CryoString(const value_type *beginP, const value_type *endP) :
		Common::BaseString<Common::u32char_type_t>(beginP, endP) {}
	CryoString(const value_type *str, uint32 len) :
		Common::BaseString<Common::u32char_type_t>(str, len) {}
#ifdef USE_CXX11
	explicit CryoString(const uint32 *str) :
		Common::BaseString<Common::u32char_type_t>((const value_type *) str) {}
	CryoString(const uint32 *str, uint32 len) :
		Common::BaseString<Common::u32char_type_t>((const value_type *) str, len) {}
	CryoString(const uint32 *beginP, const uint32 *endP) :
		Common::BaseString<Common::u32char_type_t>((const value_type *) beginP,
		        (const value_type *) endP) {}
#endif
private:
	void initWithChars(const char *str, uint32 len) {
		ensureCapacity(len, false);
		for (; len > 0; str++, len--) {
			_str[_size++] = (uint8)(*str);
		}
		_str[_size] = 0;
	}
};

class FontManager {
public:
	FontManager();
	virtual ~FontManager();

	void loadFonts(const Common::Array<Common::String> &fontFiles, Common::CodePage codepage);
	void loadTTFList(const Common::String &listFile, Common::CodePage codepage);
	void setCurrentFont(int currentFont);
	uint getCurrentFont() { return _currentFontId; }
	void setTransparentBackground(bool transparent) { _transparentBackground = transparent; }
	void setSpaceWidth(uint additionalSpace);
	void setForeColor(byte color) { _foreColor = color; }
	void setLineHeight(int h) { _lineHeight = h; }
	int lineHeight() { return _lineHeight; }
	void setCharSpacing(uint w) { _charSpacing = w; }
	void setSurface(Graphics::ManagedSurface *surface) { _currentSurface = surface; }

	int getFontMaxHeight() { return _currentFont->getFontHeight(); }

	void displayInt(uint x, uint y, int value) const {
		displayStr_(x, y,
		            toU32(Common::String::format("%d", value)));
	}
	void displayStr(uint x, uint y, const Common::String &text) const { displayStr_(x, y, toU32(text)); }
	void displayStr(uint x, uint y, const CryoString &text) const { displayStr_(x, y, text); }
	uint getStrWidth(const Common::String &text) const { return getStrWidth(toU32(text)); }
	uint getStrWidth(const CryoString &text) const;

	uint getLinesCount(const Common::String &text, uint width) { return getLinesCount(toU32(text), width); }

	void setupBlock(const Common::Rect &block, bool justifyText = false) {
		_blockRect = block;
		_blockPos.x = block.left;
		_blockPos.y = block.top;
		_justifyText = justifyText;
	}
	bool displayBlockText(const Common::String &text) {
		_blockTextStr = toU32(text);
		return displayBlockText(_blockTextStr, _blockTextStr.begin());
	}
	bool displayBlockTextContinue() { return displayBlockText(_blockTextStr, _blockTextRemaining); }
	Common::Point blockTextLastPos() { return _blockPos; }

private:
	CryoString toU32(const Common::String &text) const;

	uint displayStr_(uint x, uint y, const CryoString &text) const;
	uint getLinesCount(const CryoString &text, uint width);
	bool displayBlockText(const CryoString &text, CryoString::const_iterator begin);
	void calculateWordWrap(const CryoString &text, CryoString::const_iterator *position,
	                       uint *finalPos, bool *has_br, Common::Array<CryoString> &words) const;

	Common::CodePage _codepage;
	bool _toUnicode;
	Common::Array<Graphics::Font *> _fonts;
	const Graphics::Font *_currentFont;
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
	CryoString _blockTextStr;
	CryoString::const_iterator _blockTextRemaining;

	// Specific parameters for non alphabetic languages
	void setupWrapParameters();
	bool _useSpaceDelimiter;
	bool _keepASCIIjoined;
};

} // End of namespace CryOmni3D

#endif
