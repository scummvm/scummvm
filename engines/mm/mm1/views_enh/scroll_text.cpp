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

#include "mm/mm1/views_enh/scroll_text.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define FONT_HEIGHT 8

ScrollText::ScrollText() {
	_font = &g_globals->_fontNormal;
}

void ScrollText::setSize(int w, int h) {
	_size.x = w;
	_size.y = h;
	_rowCount = h / 8;
}

void ScrollText::setReduced(bool flag) {
	_font = flag ? &g_globals->_fontReduced :
		&g_globals->_fontReduced;
}

void ScrollText::addLine(const Common::String &str,
		TextAlignment align, byte color) {
	if (_lines.size() < _rowCount) {
		Common::Point pt(0, _lines.size() * 8);

		if (align != ALIGN_LEFT) {
			size_t strWidth = _font->getStringWidth(str);
			if (align == ALIGN_RIGHT)
				pt.x = _size.x - strWidth;
			else
				pt.x = (_size.x - strWidth) / 2;
		}

		_lines.push_back(Line(str, pt, color));
	}
}

void ScrollText::addText(const Common::String &str,
		int lineNum, byte color, int xp) {
	_lines.push_back(Line(str,
		Common::Point(xp, lineNum * FONT_HEIGHT), color));
}


} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
