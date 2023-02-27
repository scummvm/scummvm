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

#ifndef MM1_VIEWS_ENH_SCROLL_TEXT_H
#define MM1_VIEWS_ENH_SCROLL_TEXT_H

#include "common/rect.h"
#include "common/str.h"
#include "graphics/font.h"
#include "mm/mm1/views_enh/scroll_view.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

/**
 * Derived scroll class for displaying static text
 */
class ScrollText : public ScrollView {
	struct Line {
		Common::String _str;
		Common::Point _pos;
		byte _color = 0;
		TextAlign _align = ALIGN_LEFT;

		Line(const Common::String &str,
			const Common::Point &pos, byte color = 0) :
			_str(str), _pos(pos), _color(color) {
		}
	};
public:
	typedef Common::Array<Line> Lines;
private:
	size_t _rowCount = 0;
	Lines _lines;
public:
	ScrollText(const Common::String &name);
	ScrollText(const Common::String &name, UIElement *owner);

	/**
	 * Sets the element's bounds
	 */
	void setBounds(const Common::Rect &r) override;

	Lines::iterator begin() { return _lines.begin(); }
	Lines::iterator end() { return _lines.end(); }
	Lines::const_iterator begin() const { return _lines.begin(); }
	Lines::const_iterator end() const { return _lines.end(); }

	/**
	 * Clear the lines
	 */
	void clear() {
		_lines.clear();
	}

	/**
	 * Simplest form that adds lines one at a time
	 */
	void addLine(const Common::String &str,
		TextAlign align = ALIGN_LEFT, byte color = 0);

	/**
	 * Add a new line fragment for a given position
	 */
	void addText(const Common::String &str,
		int lineNum, byte color = 0,
		TextAlign align = ALIGN_LEFT, int xp = 0);
	void addText(const Common::String &str,
			int lineNum, TextAlign align = ALIGN_LEFT,
			int xp = 0) {
		addText(str, lineNum, 0, align, xp);
	}

	/**
	 * Draw the view
	 */
	void draw() override;
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
