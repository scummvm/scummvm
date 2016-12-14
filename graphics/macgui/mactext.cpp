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

MacText::MacText(Common::String s, Graphics::Font *font, int maxWidth) {
	_str = s;
	_font = font;
	_maxWidth = maxWidth;
	_interLinear = 2; // 2 pixels by default

	_textMaxWidth = -1;

	splitString();
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
}

} // End of namespace Graphics
