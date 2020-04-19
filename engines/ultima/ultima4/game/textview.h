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
 *
 */

#ifndef ULTIMA4_GAME_TEXTVIEW_H
#define ULTIMA4_GAME_TEXTVIEW_H

#include "ultima/ultima4/game/view.h"
#include "ultima/ultima4/gfx/image.h"

namespace Ultima {
namespace Ultima4 {

#define PRINTF_LIKE(x,y)

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8

/**
 * A view of a text area.  Keeps track of the cursor position.
 */
class TextView : public View {
public:
	TextView(int x, int y, int columns, int rows);
	virtual ~TextView();

	void reinit();

	int getCursorX() const {
		return _cursorX;
	}
	int getCursorY() const {
		return _cursorY;
	}
	bool getCursorEnabled() const {
		return _cursorEnabled;
	}
	int getWidth() const {
		return _columns;
	}

	/**
	 * Draw a character from the charset onto the view.
	 */
	void drawChar(int chr, int x, int y);

	/**
	 * Draw a character from the charset onto the view, but mask it with
	 * horizontal lines.  This is used for the avatar symbol in the
	 * statistics area, where a line is masked out for each virtue in
	 * which the player is not an avatar.
	 */
	void drawCharMasked(int chr, int x, int y, byte mask);
	void textAt(int x, int y, const char *fmt, ...) PRINTF_LIKE(4, 5);
	void scroll();

	void setCursorFollowsText(bool follows) {
		_cursorFollowsText = follows;
	}
	void setCursorPos(int x, int y, bool clearOld = true);
	void enableCursor();
	void disableCursor();
	void drawCursor();
	static void cursorTimer(void *data);

	// functions to modify the charset font palette
	void setFontColor(ColorFG fg, ColorBG bg);
	void setFontColorFG(ColorFG fg);
	void setFontColorBG(ColorBG bg);

	// functions to add color to strings
	/**
	 * Highlight the selected row using a background color
	 */
	void textSelectedAt(int x, int y, const char *text);

	/**
	 * Depending on the status type, apply colorization to the character
	 */
	Common::String colorizeStatus(char statustype);

	/**
	 * Depending on the status type, apply colorization to the character
	 */
	Common::String colorizeString(Common::String input, ColorFG color, uint colorstart, uint colorlength = 0);

protected:
	int _columns, _rows;         /**< size of the view in character cells  */
	bool _cursorEnabled;         /**< whether the cursor is enabled */
	bool _cursorFollowsText;     /**< whether the cursor is moved past the last character written */
	int _cursorX, _cursorY;      /**< current position of cursor */
	int _cursorPhase;            /**< the rotation state of the cursor */
	static Image *_charset;      /**< image containing font */
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
