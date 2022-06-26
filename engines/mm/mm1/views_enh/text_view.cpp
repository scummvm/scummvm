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

namespace MM {
namespace MM1 {
namespace ViewsEnh {

TextView::TextView(const Common::String &name) :
		UIElement(name, g_engine) {
}

TextView::TextView(const Common::String &name, UIElement *owner) :
		UIElement(name, owner) {
}

void TextView::setTextColor(byte col) {
	_colorsNum = col;
}

void TextView::writeChar(char c) {
	XeenFont::setColors(_colorsNum);
	Graphics::Font &font = _fontReduced ?
		g_globals->_fontReduced : g_globals->_fontNormal;

	if (c == '\r' || c == '\n') {
		_textPos.x = 0;
		_textPos.y++;
	} else {
		Graphics::ManagedSurface s = getSurface();
		font.drawChar(&s, c, _textPos.x, _textPos.y, 0xff);

		_textPos.x += font.getCharWidth(c);
		if (_textPos.x >= s.w) {
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
	for (const unsigned char *s = (const unsigned char *)str.c_str(); *s; ++s) {
		if (*s & 0x80)
			warning("TODO: Unknown high-bit set in string character");

		char c = (char)(*s & 0x7f);
		writeChar(c);
	}
}

void TextView::writeString(int x, int y, const Common::String &str) {
	_textPos.x = x;
	_textPos.y = y;
	writeString(str);
}

void TextView::writeNumber(int val) {
	writeString(Common::String::format("%d", val));
}

void TextView::writeNumber(int x, int y, int val) {
	_textPos.x = x;
	_textPos.y = y;
	writeNumber(val);
}

void TextView::newLine() {
	_textPos.x = 0;
	_textPos.y++;
}

void TextView::clearSurface() {
	UIElement::clearSurface();
	_textPos.x = _textPos.y = 0;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
