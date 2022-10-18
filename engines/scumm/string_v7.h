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

#ifndef SCUMM_STRING_V7_H
#define SCUMM_STRING_V7_H

#ifdef ENABLE_SCUMM_7_8

#include "common/scummsys.h"
#include "scumm/charset_v7.h"

namespace Scumm {

class ScummEngine;

class TextRenderer_v7 {
public:
	TextRenderer_v7(ScummEngine *vm, GlyphRenderer_v7 *gr);
	~TextRenderer_v7() {}

	void drawString(const char *str, byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, TextStyleFlags flags);
	void drawStringWrap(const char *str, byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, TextStyleFlags flags);

	int getStringWidth(const char *str) { return getStringWidth(str, 100000); }
	int getStringHeight(const char *str) { return getStringHeight(str, 100000); }

	Common::Rect calcStringDimensions(const char *str, int x, int y, TextStyleFlags flags);

	static inline bool is2ByteCharacter(Common::Language lang, byte c) {
		if (lang == Common::JA_JPN)
			return (c >= 0x80 && c <= 0x9F) || (c >= 0xE0 && c <= 0xFD);
		else if (lang == Common::KO_KOR)
			return (c >= 0xB0 && c <= 0xD0);
		else if (lang == Common::ZH_TWN || lang == Common::ZH_CHN)
			return (c >= 0x80);
		return false;
	}

private:
	int getStringWidth(const char *str, uint numBytesMax);
	int getStringHeight(const char *str, uint numBytesMax);
	void drawSubstring(const char *str, uint numBytesMax, byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 &col, TextStyleFlags flags);

	const Common::Language _lang;
	const byte _gameId;
	const bool _useCJKMode;
	const int _direction;
	const int _rtlCenteredOffset;
	const int _spacing;
	const byte _2byteCharWidth;
	const byte _lineBreakMarker;
	const bool _newStyle;
	const uint16 _screenWidth;
	GlyphRenderer_v7 *_gr;
};

} // End of namespace Scumm

#endif
#endif
