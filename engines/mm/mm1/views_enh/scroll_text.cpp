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
#include "mm/shared/utils/strings.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define FONT_HEIGHT 8

ScrollText::ScrollText(const Common::String &name) :
		ScrollView(name) {
}

ScrollText::ScrollText(const Common::String &name, UIElement *owner) :
	ScrollView(name, owner) {
}

void ScrollText::setBounds(const Common::Rect &r) {
	ScrollView::setBounds(r);
	_rowCount = _innerBounds.height() / FONT_HEIGHT;
}

void ScrollText::addLine(const Common::String &str,
		TextAlign align, byte color) {
	if (_lines.size() < _rowCount) {
		switch (align) {
		case ALIGN_LEFT:
			addText(str, _lines.size(), color, align);
			break;
		case ALIGN_RIGHT:
			addText(str, _lines.size(), color, align);
			break;
		case ALIGN_MIDDLE:
			addText(str, _lines.size(), color, align);
			break;
		}
	}
}

void ScrollText::addText(const Common::String &s,
		int lineNum, byte color, TextAlign align, int xp) {
	const int LINE_HEIGHT = 10;
	Common::String str = s;
	Common::Point pt(xp, lineNum * LINE_HEIGHT);
	Graphics::Font &font = _fontReduced ?
		g_globals->_fontReduced : g_globals->_fontNormal;

	// Split the lines
	Common::StringArray lines = splitLines(s);

	// Add them in
	for (uint i = 0; i < lines.size(); ++i, ++lineNum, pt.y += LINE_HEIGHT)
		_lines.push_back(Line(lines[i], pt, color));
}

void ScrollText::draw() {
	ScrollView::draw();

	// Iterate through displaying any text
	for (Lines::const_iterator i = begin();
		i != end(); ++i) {
		setTextColor(i->_color);
		writeString(i->_pos.x, i->_pos.y, i->_str);
	}
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
