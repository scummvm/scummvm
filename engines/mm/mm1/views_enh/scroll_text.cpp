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
	Common::String str = s;
	Common::Point pt(xp, lineNum * 8);
	Graphics::Font &font = _fontReduced ?
		g_globals->_fontReduced : g_globals->_fontNormal;

	int strWidth = font.getStringWidth(str);
	char *startP = const_cast<char *>(str.c_str());
	char *endP;

	switch (align) {
	case ALIGN_LEFT:
		// We have extra logic for standard left aligned strings to
		// insert extra newlines as necessary to word-wrap any text
		// that would go over the edge of the dialog
		while (*startP && strWidth > _innerBounds.width()) {
			// Find the last space before a full line
			endP = startP + strlen(startP) - 1;
			while (strWidth > _innerBounds.width()) {
				// Move back to a prior space
				for (--endP; endP > startP && *endP != ' '; --endP) {
				}
				assert(endP > startP);

				strWidth = font.getStringWidth(
					Common::String(startP, endP));
			}

			if (strWidth == _innerBounds.width()) {
				// Word break exactly at the line end.
				// So simply get rid of the space
				uint i = (const char *)endP - str.c_str();
				str.deleteChar(i);
				startP = const_cast<char *>(str.c_str() + i);
			} else {
				// Add a newline
				*endP = '\n';
				startP = endP + 1;
			}

			strWidth = font.getStringWidth(startP);
		}
		break;

	case ALIGN_MIDDLE:
		// Middle alignment
		if (xp == 0)
			xp = _innerBounds.width() / 2;
		pt.x = xp - strWidth / 2;
		break;

	case ALIGN_RIGHT:
		// Right alignment
		if (xp == 0)
			xp = _innerBounds.width();
		pt.x = xp - strWidth;
		break;
	}

	if (!str.empty())
		_lines.push_back(Line(str, pt, color));
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
