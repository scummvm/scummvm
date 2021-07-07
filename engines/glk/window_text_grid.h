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

#ifndef GLK_WINDOW_TEXT_GRID_H
#define GLK_WINDOW_TEXT_GRID_H

#include "glk/windows.h"
#include "glk/conf.h"
#include "glk/speech.h"

namespace Glk {

/**
 * Text Grid window
 */
class TextGridWindow : public TextWindow, Speech {
	/**
	 * Structure for a row within the grid window
	 */
	struct TextGridRow {
		Common::Array<uint32> _chars;
		Common::Array<Attributes> _attrs;
		bool dirty;

		/**
		 * Constructor
		 */
		TextGridRow() : dirty(false) {}

		/**
		 * Resize the row
		 */
		void resize(size_t newSize);
	};
	typedef Common::Array<TextGridRow> TextGridRows;
private:
	MonoFontInfo &_font;
private:
	/**
	 * Mark a given text row as modified
	 */
	void touch(int line);

	/**
	 * Return or enter, during line input. Ends line input.
	 */
	void acceptLine(uint32 keycode);
public:
	int _width, _height;
	TextGridRows _lines;

	int _curX, _curY;    ///< the window cursor position

	///< for line input
	void *_inBuf;        ///< unsigned char* for latin1, uint32* for unicode
	int _inOrgX, _inOrgY;
	int _inMax;
	int _inCurs, _inLen;
	Attributes _origAttr;
	gidispatch_rock_t _inArrayRock;
	uint32 *_lineTerminators;

	WindowStyle _styles[style_NUMSTYLES]; ///< style hints and settings
public:
	/**
	 * Constructor
	 */
	TextGridWindow(Windows *windows, uint rock);

	/**
	 * Destructor
	 */
	~TextGridWindow() override;

	/**
	 * Get the font info structure associated with the window
	 */
	FontInfo *getFontInfo() override { return &_font; }

	/**
	 * Set the size of a window
	 */
	void setSize(const Point &newSize) override {
		Window::setSize(newSize);
		_curX = CLIP((int16)_curX, _bbox.left, _bbox.right);
		_curY = CLIP((int16)_curY, _bbox.top, _bbox.bottom);
	}

	/**
	 * Sets the position of a window
	 */
	void setPosition(const Point &newPos) override {
		_bbox.moveTo(newPos);
		_curX = CLIP((int16)_curX, _bbox.left, _bbox.right);
		_curY = CLIP((int16)_curY, _bbox.top, _bbox.bottom);
	}

	/**
	 * Rearranges the window
	 */
	void rearrange(const Rect &box) override;

	/**
	 * Get window split size within parent pair window
	 */
	uint getSplit(uint size, bool vertical) const override;

	/**
	 * Write a unicode character
	 */
	void putCharUni(uint32 ch) override;

	/**
	 * Unput a unicode character
	 */
	bool unputCharUni(uint32 ch) override;

	/**
	 * Get the cursor position
	 */
	Point getCursor() const override { return Point(_curX, _curY); }

	/**
	 * Move the cursor
	 */
	void moveCursor(const Point &newPos) override;

	/**
	 * Clear the window
	 */
	void clear() override;

	/**
	 * Click the window
	 */
	void click(const Point &newPos) override;

	/**
	 * Cancel a hyperlink event
	 */
	void cancelHyperlinkEvent() override {
		_hyperRequest = false;
	}

	/**
	 * Redraw the window
	 */
	void redraw() override;

	void acceptReadLine(uint32 arg) override;

	void acceptReadChar(uint arg) override;

	void getSize(uint *width, uint *height) const override;

	void requestCharEvent() override;

	void requestCharEventUni() override;

	/**
	 * Prepare for inputing a line
	 */
	void requestLineEvent(char *buf, uint maxlen, uint initlen) override;

	/**
	 * Prepare for inputing a line
	 */
	void requestLineEventUni(uint32 *buf, uint maxlen, uint initlen) override;

	/**
	 * Cancel an input line event
	 */
	void cancelLineEvent(Event *ev) override;

	/**
	 * Cancel a mouse event
	 */
	void cancelMouseEvent() override {
		_mouseRequest = false;
	}

	void requestMouseEvent() override {
		_mouseRequest = true;
	}

	void requestHyperlinkEvent() override {
		_hyperRequest = true;
	}

	void cancelCharEvent() override {
		_charRequest = _charRequestUni = false;
	}

	/**
	 * Returns a pointer to the styles for the window
	 */
	const WindowStyle *getStyles() const override {
		return _styles;
	}
};

} // End of namespace Glk

#endif
