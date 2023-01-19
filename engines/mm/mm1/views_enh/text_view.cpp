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

#include "mm/mm1/globals.h"
#include "mm/mm1/gfx/gfx.h"
#include "mm/mm1/views_enh/text_view.h"
#include "mm/mm1/mm1.h"
#include "mm/utils/strings.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

TextView::TextView(const Common::String &name) :
		UIElement(name, g_engine) {
}

TextView::TextView(const Common::String &name, UIElement *owner) :
		UIElement(name, owner) {
}

byte TextView::setTextColor(byte col) {
	byte oldColor = _colorsNum;
	_colorsNum = col;
	return oldColor;
}

Graphics::Font *TextView::getFont() const {
	return _fontReduced ? &g_globals->_fontReduced :
		&g_globals->_fontNormal;
}

void TextView::writeChar(char c) {
	assert((unsigned char)c < 0x80);
	XeenFont::setColors(_colorsNum);
	Graphics::Font &font = *getFont();

	if (c == '\r' || c == '\n') {
		_textPos.x = 0;
		_textPos.y += font.getFontHeight();
	} else {
		Graphics::ManagedSurface s = getSurface();
		if (c != ' ')
			font.drawChar(&s, c,
				_bounds.borderSize() + _textPos.x,
				_bounds.borderSize() + _textPos.y,
				0xff);

		_textPos.x += font.getCharWidth(c);
		if (_textPos.x >= _innerBounds.width()) {
			_textPos.x = 0;
			_textPos.y += font.getFontHeight();
		}
	}
}

void TextView::writeChar(int x, int y, char c) {
	_textPos.x = x;
	_textPos.y = y;
	writeChar(c);
}

void TextView::writeString(const Common::String &str) {
	for (const char *s = (const char *)str.c_str(); *s; ++s) {
		char c = *s;

		if (c == '\x01') {
			// Highlight next character for buttons
			int colNum = atoi(Common::String(s + 1, s + 3).c_str());
			byte oldCol = setTextColor(colNum);
			s += 3;
			writeChar(*s);
			setTextColor(oldCol);

		} else {
			writeChar(c);
		}
	}
}

void TextView::writeString(int x, int y, const Common::String &str,
		TextAlign align) {
	_textPos.x = x;
	_textPos.y = y;

	Common::StringArray lines = splitLines(str);

	for (auto line : lines) {
		if (line != lines.front())
			newLine();

		if (align != ALIGN_LEFT && x == 0) {
			int strWidth = getFont()->getStringWidth(line);

			if (align == ALIGN_MIDDLE)
				_textPos.x = MAX((_innerBounds.width() - strWidth) / 2, 0);
			else
				_textPos.x = MAX(_innerBounds.width() - strWidth, 0);
		}

		writeString(line);
	}
}

void TextView::writeNumber(int val) {
	writeString(Common::String::format("%d", val));
}

void TextView::writeNumber(int x, int y, int val) {
	_textPos.x = x;
	_textPos.y = y;
	writeNumber(val);
}

void TextView::writeLine(int lineNum, const Common::String &str,
		TextAlign align, int xp) {
	writeString(xp, lineNum * 8, str, align);
}

void TextView::newLine() {
	_textPos.x = 0;
	_textPos.y += 8;
}

void TextView::clearSurface() {
	UIElement::clearSurface();
	_textPos.x = _textPos.y = 0;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
