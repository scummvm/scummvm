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

#ifndef MM1_VIEWS_ENH_TEXT_VIEW_H
#define MM1_VIEWS_ENH_TEXT_VIEW_H

#include "graphics/font.h"
#include "mm/mm1/events.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

enum TextAlign {
	ALIGN_LEFT, ALIGN_RIGHT, ALIGN_MIDDLE
};

class TextView : public UIElement {
private:
	Graphics::Font *getFont() const;

	/**
	 * Raw write string
	 */
	void rawWriteString(const Common::String &str);

protected:
	Common::Point _textPos;
	int _colorsNum = 0;
	bool _fontReduced = false;

	/**
	 * Set the text color
	 */
	byte setTextColor(byte col);

	/**
	 * Write a character
	 */
	void writeChar(char c);
	void writeChar(int x, int y, char c);

	/**
	 * Write some text
	 */
	void writeString(const Common::String &str,
		TextAlign align = ALIGN_LEFT);
	void writeString(int x, int y, const Common::String &str,
		TextAlign align = ALIGN_LEFT);

	/**
	 * Write a number
	 */
	void writeNumber(int val);
	void writeNumber(int x, int y, int val);

	/**
	 * Write a line
	 */
	void writeLine(int lineNum, const Common::String &str,
		TextAlign align = ALIGN_LEFT, int xp = 0);

	/**
	 * Move the text position to the next line
	 */
	void newLine();

	/**
	 * Split lines
	 */
	Common::StringArray splitLines(const Common::String &str,
		int firstLineWidth = -1);

	/**
	 * Clear the surface
	 */
	void clearSurface() override;

public:
	TextView(const Common::String &name);
	TextView(const Common::String &name, UIElement *owner);
	virtual ~TextView() {}

	/**
	 * Set whether to use the standard large sized font or reduced one
	 */
	void setReduced(bool flag) {
		_fontReduced = flag;
	}
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
