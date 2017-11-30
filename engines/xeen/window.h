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
#include "xeen/sprites.h"
#include "xeen/xsurface.h"

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

class Windows {
	friend class Window;
private:
	Common::Array<Window> _windows;
	Common::Array<Window *> _windowStack;
private:
	/**
	 * Adds a window to the stack of currently open ones
	 */
	void addToStack(Window *win);

	/**
	 * Removes a window from the currently active stack
	 */
	void removeFromStack(Window *win);
public:
	Windows();

	/**
	 * Returns a specified window
	 */
	Window &operator[](int index) { return _windows[index]; }

	/**
	 * Close all currently open windows
	 */
	void closeAll();
};

class Window: public XSurface {
private:
	Common::Rect _bounds;
	Common::Rect _innerBounds;
	XSurface _savedArea;
	int _a;
	int _border;
	int _xLo, _xHi;
	int _ycL, _ycH;

	void open2();
public:
	bool _enabled;
public:
	Window();
	Window(const Window &src);
	Window(const Common::Rect &bounds, int a, int border,
		int xLo, int ycL, int xHi, int ycH);

	virtual void addDirtyRect(const Common::Rect &r);

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

	const char *writeString(const Common::String &s);

	void drawList(DrawStruct *items, int count);

	int getString(Common::String &line, uint maxLen, int maxWidth);
};

} // End of namespace Xeen

#endif /* XEEN_WINDOW_H */
