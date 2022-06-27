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

ScrollText::ScrollText() : ScrollView("ScrollText") {
	_font = &g_globals->_fontNormal;
}

void ScrollText::setBounds(const Common::Rect &r) {
	ScrollView::setBounds(_bounds);
	_innerBounds = _bounds;
	_innerBounds.grow(-FRAME_BORDER_SIZE);
	_rowCount = _innerBounds.height() / FONT_HEIGHT;
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
				pt.x = _innerBounds.right - strWidth;
			else
				pt.x = _innerBounds.left +
					(_innerBounds.width() + strWidth) / 2;
		}

		_lines.push_back(Line(str, pt, color));
	}
}

void ScrollText::addText(const Common::String &str,
		int lineNum, byte color, int xp) {
	_lines.push_back(Line(str,
		Common::Point(xp, lineNum * FONT_HEIGHT), color));
}

void ScrollText::draw() {
	ScrollView::draw();

	// Iterate through displaying any text
	for (Lines::const_iterator i = begin();
		i != end(); ++i) {
		setTextColor(i->_color);
		writeString(i->_pos.x + FRAME_BORDER_SIZE,
			i->_pos.y + FRAME_BORDER_SIZE,
			i->_str
		);
	}
}

bool ScrollText::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_ESCAPE) {
		close();
		return true;
	}

	return false;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
