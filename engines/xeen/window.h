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

#ifndef XEEN_WINDOW_H
#define XEEN_WINDOW_H

#include "common/rect.h"
#include "common/str.h"
#include "xeen/font.h"
#include "xeen/sprites.h"

namespace Xeen {

#define GAME_WINDOW 28

class XeenEngine;
class Window;

struct DrawStruct {
	SpriteResource *_sprites;
	int _frame;
	int _x;
	int _y;
	int _scale;
	int _flags;

	DrawStruct(int frame, int x, int y, int scale = 0, int flags = 0) :
		_sprites(nullptr), _frame(frame), _x(x), _y(y), _scale(scale), _flags(flags) {}
	DrawStruct(): _sprites(nullptr), _frame(0), _x(0), _y(0), _scale(0), _flags(0) {}
};

class Windows : public FontData {
private:
	Common::Array<Window> _windows;
	Common::Array<Window *> _windowStack;
public:
	Windows();
	~Windows();

	/**
	 * Returns a specified window
	 */
	Window &operator[](int index) { return _windows[index]; }

	/**
	 * Close all currently open windows
	 */
	void closeAll();

	/**
	 * Called when a window has been opened
	 */
	void windowOpened(Window *win);

	/**
	 * Called when a window has been closed
	 */
	void windowClosed(Window *win);
};

class Window: public FontSurface {
private:
	Common::Rect _bounds;
	Common::Rect _innerBounds;
	XSurface _savedArea;
	int _a;
	int _border;
	int _xLo, _xHi;
	int _ycL, _ycH;
private:
	/**
	 * Returns true if the window is covering the entire screen
	 */
	bool isFullScreen() const;
public:
	bool _enabled;
public:
	Window();
	Window(const Window &src);
	Window(const Common::Rect &bounds, int a, int border,
		int xLo, int ycL, int xHi, int ycH);
	~Window() override {}

	void addDirtyRect(const Common::Rect &r) override;

	void setBounds(const Common::Rect &r);

	const Common::Rect &getBounds() { return _bounds; }

	void open();

	void close();

	/**
	 * Update the window
	 */
	void update();

	void frame();

	/**
	 * Fill the content area of a window with the current background color
	 */
	void fill();

	void drawList(DrawStruct *items, int count);

	int getString(Common::String &line, uint maxLen, int maxWidth);

	/**
	 * Write a string to the window
	 * @param s			String to display
	 * @param clipRect	Window bounds to display string within
	 * @returns			Any string remainder that couldn't be displayed
	 * @remarks		Note that bounds is just used for wrapping purposes. Unless
	 *		justification is set, the message will be written at _writePos
	 */
	const char *writeString(const Common::String &s, const Common::Rect &clipRect) {
		return FontSurface::writeString(s, clipRect);
	}

	/**
	 * Write a string to the window
	 * @param s			String to display
	 * @returns			Any string remainder that couldn't be displayed
	 * @remarks		Note that bounds is just used for wrapping purposes. Unless
	 *		justification is set, the message will be written at _writePos
	 */
	const char *writeString(const Common::String &s) {
		return FontSurface::writeString(s, _innerBounds);
	}

	/**
	 * Write a charcter to the window
	 * @param c			Character
	 */
	void writeCharacter(char c) {
		FontSurface::writeCharacter(c, _innerBounds);
	}
};

} // End of namespace Xeen

#endif /* XEEN_WINDOW_H */
