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

#include "xeen/window.h"
#include "xeen/xeen.h"

namespace Xeen {

Windows::Windows() {
	Window windows[40] = {
		Window(Common::Rect(0, 0, 320, 200), 0, 0, 0, 0, 320, 200),
		Window(Common::Rect(237, 9, 317, 74), 0, 0, 237, 12, 307, 68),
		Window(Common::Rect(225, 1, 319, 73), 1, 8, 225, 1, 319, 73),
		Window(Common::Rect(0, 0, 230, 149), 0, 0, 9, 8, 216, 140),
		Window(Common::Rect(235, 148, 309, 189), 2, 8, 0, 0, 0, 0),
		Window(Common::Rect(70, 20, 250, 183), 3, 8, 80, 38, 240, 166),
		Window(Common::Rect(52, 149, 268, 197), 4, 8, 0, 0, 0, 0),
		Window(Common::Rect(108, 0, 200, 200), 5, 0, 0, 0, 0, 0),
		Window(Common::Rect(232, 9, 312, 74), 0, 0, 0, 0, 0, 0),
		Window(Common::Rect(103, 156, 217, 186), 6, 8, 0, 0, 0, 0),
		Window(Common::Rect(226, 0, 319, 146), 7, 8, 0, 0, 0, 0),
		Window(Common::Rect(8, 8, 224, 140), 8, 8, 8, 8, 224, 200),
		Window(Common::Rect(0, 143, 320, 199), 9, 8, 0, 0, 0, 0),
		Window(Common::Rect(50, 103, 266, 139), 10, 8, 0, 0, 0, 0),
		Window(Common::Rect(0, 7, 320, 138), 11, 8, 0, 0, 0, 0),
		Window(Common::Rect(50, 71, 182, 129), 12, 8, 0, 0, 0, 0),
		Window(Common::Rect(228, 106, 319, 146), 13, 8, 0, 0, 0, 0),
		Window(Common::Rect(20, 142, 290, 199), 14, 8, 0, 0, 0, 0),
		Window(Common::Rect(0, 20, 320, 180), 15, 8, 0, 0, 0, 0),
		Window(Common::Rect(231, 48, 317, 141), 16, 8, 0, 0, 0, 0),
		Window(Common::Rect(72, 37, 248, 163), 17, 8, 0, 0, 0, 0),
		Window(Common::Rect(99, 59, 237, 141), 18, 8, 99, 59, 237, 0),
		Window(Common::Rect(65, 23, 250, 163), 19, 8, 75, 36, 245, 141),
		Window(Common::Rect(80, 28, 256, 148), 20, 8, 80, 28, 256, 172),
		Window(Common::Rect(0, 0, 320, 146), 21, 8, 0, 0, 320, 148),
		Window(Common::Rect(27, 6, 207, 142), 22, 8, 0, 0, 0, 146),
		Window(Common::Rect(15, 15, 161, 91), 23, 8, 0, 0, 0, 0),
		Window(Common::Rect(90, 45, 220, 157), 24, 8, 0, 0, 0, 0),
		Window(Common::Rect(0, 0, 320, 200), 25, 8, 0, 0, 0, 0),
		Window(Common::Rect(0, 101, 320, 146), 26, 8, 0, 101, 320, 0),
		Window(Common::Rect(0, 0, 320, 108), 27, 8, 0, 0, 0, 45),
		Window(Common::Rect(50, 112, 266, 148), 28, 8, 0, 0, 0, 0),
		Window(Common::Rect(12, 11, 164, 94), 0, 0, 0, 0, 52, 0),
		Window(Common::Rect(8, 147, 224, 192), 0, 8, 0, 0, 0, 94),
		Window(Common::Rect(232, 74, 312, 138), 29, 8, 0, 0, 0, 0),
		Window(Common::Rect(226, 26, 319, 146), 30, 8, 0, 0, 0, 0),
		Window(Common::Rect(225, 74, 319, 154), 31, 8, 0, 0, 0, 0),
		Window(Common::Rect(27, 6, 195, 142), 0, 8, 0, 0, 0, 0),
		Window(Common::Rect(225, 140, 319, 199), 0, 8, 0, 0, 0, 0)
	};

	_windows = Common::Array<Window>(windows, 40);
}

void Windows::closeAll() {
	for (int i = (int)_windowStack.size() - 1; i >= 0; --i)
		_windowStack[i]->close();
	assert(_windowStack.size() == 0);
}

void Windows::addToStack(Window *win) {
	_windowStack.push_back(win);
}

void Windows::removeFromStack(Window *win) {
	for (uint i = 0; i < _windowStack.size(); ++i) {
		if (_windowStack[i] == win) {
			_windowStack.remove_at(i);
			break;
		}
	}
}

/*------------------------------------------------------------------------*/

Window::Window() : XSurface(), _enabled(false),
	_a(0), _border(0), _xLo(0), _xHi(0), _ycL(0), _ycH(0) {
}

Window::Window(const Window &src) : XSurface(), _enabled(src._enabled),
		_a(src._a), _border(src._border), _xLo(src._xLo), _ycL(src._ycL),
		_xHi(src._xHi), _ycH(src._ycH) {

	setBounds(src._bounds);
	create(*g_vm->_screen, Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
}

Window::Window(const Common::Rect &bounds, int a, int border,
		int xLo, int ycL, int xHi, int ycH): XSurface(),
		_enabled(false), _a(a), _border(border),
		_xLo(xLo), _ycL(ycL), _xHi(xHi), _ycH(ycH) {
	setBounds(bounds);
	create(*g_vm->_screen, Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
}

void Window::setBounds(const Common::Rect &r) {
	_bounds = r;
	_innerBounds = r;
	_innerBounds.grow(-_border);
}

void Window::open() {
	if (!_enabled) {
		_enabled = true;
		g_vm->_windows->addToStack(this);
		open2();
	}

	if (g_vm->_mode == MODE_9) {
		warning("TODO: copyFileToMemory");
	}
}

void Window::open2() {
	Screen &screen = *g_vm->_screen;

	// Save a copy of the area under the window
	_savedArea.create(_bounds.width(), _bounds.height());
	_savedArea.copyRectToSurface(screen, 0, 0, _bounds);

	// Mark the area as dirty and fill it with a default background
	addDirtyRect(_bounds);
	frame();
	fill();

	screen._writePos.x = _bounds.right - 8;
	screen.writeSymbol(19);

	screen._writePos.x = _innerBounds.left;
	screen._writePos.y = _innerBounds.top;
	screen._fontJustify = JUSTIFY_NONE;
	screen._fontReduced = false;
}

void Window::frame() {
	Screen &screen = *g_vm->_screen;
	int xCount = (_bounds.width() - 9) / FONT_WIDTH;
	int yCount = (_bounds.height() - 9) / FONT_HEIGHT;

	// Write the top line
	screen._writePos = Common::Point(_bounds.left, _bounds.top);
	screen.writeSymbol(0);

	if (xCount > 0) {
		int symbolId = 1;
		for (int i = 0; i < xCount; ++i) {
			screen.writeSymbol(symbolId);
			if (++symbolId == 5)
				symbolId = 1;
		}
	}

	screen._writePos.x = _bounds.right - FONT_WIDTH;
	screen.writeSymbol(5);

	// Write the vertical edges
	if (yCount > 0) {
		int symbolId = 6;
		for (int i = 0; i < yCount; ++i) {
			screen._writePos.y += 8;

			screen._writePos.x = _bounds.left;
			screen.writeSymbol(symbolId);

			screen._writePos.x = _bounds.right - FONT_WIDTH;
			screen.writeSymbol(symbolId + 4);

			if (++symbolId == 10)
				symbolId = 6;
		}
	}

	// Write the bottom line
	screen._writePos = Common::Point(_bounds.left, _bounds.bottom - FONT_HEIGHT);
	screen.writeSymbol(14);

	if (xCount > 0) {
		int symbolId = 15;
		for (int i = 0; i < xCount; ++i) {
			screen.writeSymbol(symbolId);
			if (++symbolId == 19)
				symbolId = 15;
		}
	}

	screen._writePos.x = _bounds.right - FONT_WIDTH;
	screen.writeSymbol(19);
}

void Window::close() {
	Screen &screen = *g_vm->_screen;

	if (_enabled) {
		// Update the window
		update();

		// Restore the saved original content
		screen.copyRectToSurface(_savedArea, _bounds.left, _bounds.top,
			Common::Rect(0, 0, _bounds.width(), _bounds.height()));
		addDirtyRect(_bounds);

		// Remove the window from the stack and flag it as now disabled
		g_vm->_windows->removeFromStack(this);
		_enabled = false;
	}

	if (g_vm->_mode == MODE_9) {
		warning("TODO: copyFileToMemory");
	}
}

void Window::update() {
	// Since all window drawing is done on the screen surface anyway,
	// there's nothing that needs to be updated here
}

void Window::addDirtyRect(const Common::Rect &r) {
	g_vm->_screen->addDirtyRect(r);
}

void Window::fill() {
	fillRect(_innerBounds, g_vm->_screen->_bgColor);
}

const char *Window::writeString(const Common::String &s) {
	return g_vm->_screen->writeString(s, _innerBounds);
}

void Window::drawList(DrawStruct *items, int count) {
	for (int i = 0; i < count; ++i, ++items) {
		if (items->_frame == -1 || items->_scale == -1 || items->_sprites == nullptr)
			continue;

		Common::Point pt(items->_x, items->_y);
		pt.x += _innerBounds.left;
		pt.y += _innerBounds.top;

		items->_sprites->draw(*this, items->_frame, pt, items->_flags, items->_scale);
	}
}

} // End of namespace Xeen
