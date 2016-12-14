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
 */

#include "graphics/macgui/mactext.h"
#include "graphics/font.h"

namespace Graphics {

MacText::MacText(Common::String s, Graphics::Font *font, int fgcolor, int bgcolor, int maxWidth) {
	_str = s;
	_font = font;
	_fgcolor = fgcolor;
	_bgcolor = bgcolor;
	_maxWidth = maxWidth;

	_interLinear = 0; // 0 pixels between the lines by default

	_textMaxWidth = -1;

	splitString();

	_fullRefresh = true;
}

void MacText::splitString() {
	const char *s = _str.c_str();

	Common::String tmp;
	bool prevCR;

	while (*s) {
		if (*s == '\n' && prevCR) {	// trean \r\n as one
			prevCR = false;
			continue;
		}

		if (*s == '\r')
			prevCR = true;

		if (*s == '\r' || *s == '\n') {
			_text.push_back(tmp);
			_widths.push_back(_font->getStringWidth(tmp));

			tmp.clear();

			continue;
		}

		tmp += *s;
	}

	if (_text.size())
		_text.push_back(tmp);

	calcMaxWidth();
}

void MacText::calcMaxWidth() {
	int max = -1;

	for (uint i = 0; i < _widths.size(); i++)
		if (max < _widths[i])
			max = _widths[i];

	_textMaxWidth = max;
}

void MacText::render() {
	if (_fullRefresh) {
		_surface.create(_textMaxWidth, _text.size() * (_font->getFontHeight() + _interLinear));

		_surface.clear(_bgcolor);

		int y = 0;

		for (uint i = 0; i < _text.size(); i++) {
			_font->drawString(&_surface, _text[i], 0, y, _textMaxWidth, _fgcolor);

			y += _font->getFontHeight() + _interLinear;
		}

		_fullRefresh = false;
	}
}

} // End of namespace Graphics
