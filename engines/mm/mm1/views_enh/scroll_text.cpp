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
#define LINE_HEIGHT 10

ScrollText::ScrollText(const Common::String &name) :
		ScrollView(name) {
}

ScrollText::ScrollText(const Common::String &name, UIElement *owner) :
	ScrollView(name, owner) {
}

void ScrollText::setBounds(const Common::Rect &r) {
	ScrollView::setBounds(r);
	_rowCount = _innerBounds.height() / FONT_HEIGHT;

	if (_scrolling)
		setScrolling(true);
}

size_t ScrollText::visibleLineCount() const {
	const int height = _innerBounds.height();
	if (height < FONT_HEIGHT)
		return 0;

	return (height - FONT_HEIGHT) / LINE_HEIGHT + 1;
}

void ScrollText::setScrolling(bool enabled) {
	_scrolling = enabled;
	if (!_scrolling) {
		_topLine = 0;
		return;
	}

	const size_t visible = visibleLineCount();
	const size_t maxTop = (_lines.size() > visible) ?
		_lines.size() - visible : 0;
	if (_topLine > maxTop)
		_topLine = maxTop;
}

bool ScrollText::canScrollUp() const {
	return _scrolling && _topLine != 0;
}

bool ScrollText::canScrollDown() const {
	if (!_scrolling)
		return false;

	const size_t visible = visibleLineCount();
	return _topLine + visible < _lines.size();
}

bool ScrollText::scrollLines(int delta) {
	if (!_scrolling || !delta)
		return false;

	const size_t visible = visibleLineCount();
	const size_t maxTop = (_lines.size() > visible) ?
		_lines.size() - visible : 0;
	int newTop = (int)_topLine + delta;

	if (newTop < 0)
		newTop = 0;
	if ((size_t)newTop > maxTop)
		newTop = (int)maxTop;

	if ((size_t)newTop == _topLine)
		return false;

	_topLine = (size_t)newTop;
	redraw();
	return true;
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

	if (lineNum == -1)
		lineNum = (int)_lines.size();
	Common::Point pt(xp, lineNum * LINE_HEIGHT);

	// Split the lines
	Common::StringArray lines = splitLines(s);

	// Add them in
	for (uint i = 0; i < lines.size(); ++i, ++lineNum, pt.y += LINE_HEIGHT) {
		_lines.push_back(Line(lines[i], pt, color, align));
	}
}

void ScrollText::draw() {
	ScrollView::draw();

	if (!_scrolling) {
		// Existing behavior for ordinary ScrollText users.
		for (Lines::const_iterator i = begin(); i != end(); ++i) {
			setTextColor(i->_color);
			writeString(i->_pos.x, i->_pos.y, i->_str, i->_align);
		}
		return;
	}

	// Scrollable text is clipped by line: only lines whose complete glyph
	// height fits inside the message's inner bounds are drawn.
	const size_t visible = visibleLineCount();
	const size_t endLine = (_topLine + visible < _lines.size()) ?
		_topLine + visible : _lines.size();

	for (size_t idx = _topLine; idx < endLine; ++idx) {
		const Line &line = _lines[idx];
		Common::Point pt = line._pos;
		pt.y -= (int)_topLine * LINE_HEIGHT;
		setTextColor(line._color);
		writeString(pt.x, pt.y, line._str, line._align);
	}
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
