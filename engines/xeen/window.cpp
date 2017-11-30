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

XeenEngine *Window::_vm;

void Window::init(XeenEngine *vm) {
	_vm = vm;
}

Window::Window() : XSurface(), _enabled(false),
	_a(0), _border(0), _xLo(0), _xHi(0), _ycL(0), _ycH(0) {
}

Window::Window(const Window &src) : XSurface(), _enabled(src._enabled),
		_a(src._a), _border(src._border), _xLo(src._xLo), _ycL(src._ycL),
		_xHi(src._xHi), _ycH(src._ycH) {

	setBounds(src._bounds);
	create(*_vm->_screen, Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
}

Window::Window(const Common::Rect &bounds, int a, int border,
		int xLo, int ycL, int xHi, int ycH): XSurface(),
		_enabled(false), _a(a), _border(border),
		_xLo(xLo), _ycL(ycL), _xHi(xHi), _ycH(ycH) {
	setBounds(bounds);
	create(*_vm->_screen, Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
}

void Window::setBounds(const Common::Rect &r) {
	_bounds = r;
	_innerBounds = r;
	_innerBounds.grow(-_border);
}

void Window::open() {
	if (!_enabled) {
		_enabled = true;
		_vm->_screen->_windowStack.push_back(this);
		open2();
	}

	if (_vm->_mode == MODE_9) {
		warning("TODO: copyFileToMemory");
	}
}

void Window::open2() {
	Screen &screen = *_vm->_screen;

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
	Screen &screen = *_vm->_screen;
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
	Screen &screen = *_vm->_screen;

	if (_enabled) {
		// Update the window
		update();

		// Restore the saved original content
		screen.copyRectToSurface(_savedArea, _bounds.left, _bounds.top,
			Common::Rect(0, 0, _bounds.width(), _bounds.height()));
		addDirtyRect(_bounds);

		// Remove the window from the stack and flag it as now disabled
		for (uint i = 0; i < _vm->_screen->_windowStack.size(); ++i) {
			if (_vm->_screen->_windowStack[i] == this)
				_vm->_screen->_windowStack.remove_at(i);
		}

		_enabled = false;
	}

	if (_vm->_mode == MODE_9) {
		warning("TODO: copyFileToMemory");
	}
}

void Window::update() {
	// Since all window drawing is done on the screen surface anyway,
	// there's nothing that needs to be updated here
}

void Window::addDirtyRect(const Common::Rect &r) {
	_vm->_screen->addDirtyRect(r);
}

void Window::fill() {
	fillRect(_innerBounds, _vm->_screen->_bgColor);
}

const char *Window::writeString(const Common::String &s) {
	return _vm->_screen->writeString(s, _innerBounds);
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
