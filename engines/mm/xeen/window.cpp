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

#include "mm/xeen/window.h"
#include "mm/xeen/xeen.h"

namespace MM {
namespace Xeen {

Windows::Windows() {
	// Load font data for the game
	File f("fnt");
	byte *data = new byte[f.size()];
	f.read(data, f.size());
	_fontData = data;
	_fontWritePos = new Common::Point();

	Common::fill(&_textColors[0], &_textColors[4], 0);
	_bgColor = DEFAULT_BG_COLOR;
	_fontReduced = false;
	_fontJustify = JUSTIFY_NONE;

	Window windows[48] = {
		Window(Common::Rect(0, 0, 320, 200), 0, 0, 0, 0, 320, 200),
		Window(Common::Rect(223, 9, 317, 74), 0, 0, 237, 12, 307, 68),
		Window(Common::Rect(225, 1, 320, 73), 1, 8, 225, 1, 319, 73),
		Window(Common::Rect(0, 0, 230, 149), 0, 0, 9, 8, 216, 140),
		Window(Common::Rect(235, 148, 309, 189), 2, 8, 0, 0, 0, 0),
		Window(Common::Rect(70, 20, 250, 183), 3, 8, 80, 38, 240, 166),
		Window(Common::Rect(52, 149, 268, 198), 4, 8, 0, 0, 0, 0),
		Window(Common::Rect(108, 0, 200, 200), 5, 0, 0, 0, 0, 0),
		Window(Common::Rect(232, 9, 312, 74), 0, 0, 0, 0, 0, 0),
		Window(Common::Rect(103, 156, 217, 186), 6, 8, 0, 0, 0, 0),
		Window(Common::Rect(226, 0, 320, 146), 7, 8, 0, 0, 0, 0),
		Window(Common::Rect(8, 8, 224, 140), 8, 8, 8, 8, 224, 200),
		Window(Common::Rect(0, 143, 320, 199), 9, 8, 0, 0, 0, 0),
		Window(Common::Rect(50, 103, 266, 139), 10, 8, 0, 0, 0, 0),
		Window(Common::Rect(0, 7, 320, 138), 11, 8, 0, 0, 0, 0),
		Window(Common::Rect(50, 71, 182, 129), 12, 8, 0, 0, 0, 0),
		Window(Common::Rect(228, 106, 320, 146), 13, 8, 0, 0, 0, 0),
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
		Window(Common::Rect(226, 26, 320, 146), 30, 8, 0, 0, 0, 0),
		Window(Common::Rect(225, 74, 320, 154), 31, 8, 0, 0, 0, 0),
		Window(Common::Rect(27, 6, 195, 142), 0, 8, 0, 0, 0, 0),
		Window(Common::Rect(225, 140, 320, 199), 0, 8, 0, 0, 0, 0),
		Window(Common::Rect(12, 8, 162, 198), 0, 0, 128, 0, 119, 0),
		Window(Common::Rect(0, 0, 320, 200), 32, 8, 0, 0, 320, 190),
		Window(Common::Rect(0, 0, 320, 200), 33, 8, 0, 0, 320, 200)
	};

	_windows = Common::Array<Window>(windows, 42);

	Common::File pat;
	if (pat.open("TEXTPAT.FNT")) {
		_big5Font = new Graphics::Big5Font();
		_big5Font->loadPrefixedRaw(pat, 14);
	} else if (pat.open("CMM4.PAT")) {
		_big5Font = new Graphics::Big5Font();
		_big5Font->loadPrefixedRaw(pat, 15);
	} else if (g_vm->getLanguage() == Common::ZH_TWN) {
		error("Unable to find TEXTPAT.FNT or CMM4.PAT for Chinese version");
	}	

}

Windows::~Windows() {
	delete[] _fontData;
	delete _fontWritePos;
}

void Windows::closeAll() {
	for (int i = (int)_windowStack.size() - 1; i >= 0; --i)
		_windowStack[i]->close();
	assert(_windowStack.size() == 0);
}

void Windows::windowOpened(Window *win) {
	_windowStack.push_back(win);
}

void Windows::windowClosed(Window *win) {
	for (uint i = 0; i < _windowStack.size(); ++i) {
		if (_windowStack[i] == win) {
			_windowStack.remove_at(i);
			break;
		}
	}
}

/*------------------------------------------------------------------------*/

Window::Window() : FontSurface(), _enabled(false),
_a(0), _border(0), _xLo(0), _xHi(0), _ycL(0), _ycH(0) {
}

Window::Window(const Window &src) : FontSurface(), _enabled(src._enabled),
_a(src._a), _border(src._border), _xLo(src._xLo), _ycL(src._ycL),
_xHi(src._xHi), _ycH(src._ycH) {

	setBounds(src._bounds);
	create(*g_vm->_screen, Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
}

Window::Window(const Common::Rect &bounds, int a, int border,
	int xLo, int ycL, int xHi, int ycH) : FontSurface(),
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
	Screen &screen = *g_vm->_screen;

	if (!_enabled && !isFullScreen()) {
		// Save a copy of the area under the window
		_savedArea.create(_bounds.width(), _bounds.height());
		_savedArea.copyRectToSurface(screen, 0, 0, _bounds);

		// Mark the area as dirty and fill it with a default background
		addDirtyRect(_bounds);
		frame();
		fill();

		_writePos.x = _bounds.right - 8;
		writeSymbol(19);

		_writePos.x = _innerBounds.left;
		_writePos.y = _innerBounds.top;
		_fontJustify = JUSTIFY_NONE;
		_fontReduced = false;
		_enabled = true;

		// Signal that the window has opened
		g_vm->_windows->windowOpened(this);
	}
}

void Window::close() {
	Screen &screen = *g_vm->_screen;

	if (_enabled && !isFullScreen()) {
		// Update the window
		update();

		// Restore the saved original content
		screen.copyRectToSurface(_savedArea, _bounds.left, _bounds.top,
			Common::Rect(0, 0, _bounds.width(), _bounds.height()));
		addDirtyRect(_bounds);

		// Signal that the window has closed
		g_vm->_windows->windowClosed(this);
		_enabled = false;
	}
}

void Window::frame() {
	int xCount = (_bounds.width() - 9) / FONT_WIDTH;
	int yCount = (_bounds.height() - 9) / FONT_HEIGHT;

	// Write the top line
	_writePos = Common::Point(_bounds.left, _bounds.top);
	writeSymbol(0);

	if (xCount > 0) {
		int symbolId = 1;
		for (int i = 0; i < xCount; ++i) {
			writeSymbol(symbolId);
			if (++symbolId == 5)
				symbolId = 1;
		}
	}

	_writePos.x = _bounds.right - FONT_WIDTH;
	writeSymbol(5);

	// Write the vertical edges
	if (yCount > 0) {
		int symbolId = 6;
		for (int i = 0; i < yCount; ++i) {
			_writePos.y += 8;

			_writePos.x = _bounds.left;
			writeSymbol(symbolId);

			_writePos.x = _bounds.right - FONT_WIDTH;
			writeSymbol(symbolId + 4);

			if (++symbolId == 10)
				symbolId = 6;
		}
	}

	// Write the bottom line
	_writePos = Common::Point(_bounds.left, _bounds.bottom - FONT_HEIGHT);
	writeSymbol(14);

	if (xCount > 0) {
		int symbolId = 15;
		for (int i = 0; i < xCount; ++i) {
			writeSymbol(symbolId);
			if (++symbolId == 19)
				symbolId = 15;
		}
	}

	_writePos.x = _bounds.right - FONT_WIDTH;
	writeSymbol(19);
}

void Window::update() {
	// Since all window drawing is done on the screen surface anyway,
	// there's nothing that needs to be updated here
}

void Window::addDirtyRect(const Common::Rect &r) {
	g_vm->_screen->addDirtyRect(r);
}

void Window::fill() {
	fillRect(_innerBounds, _bgColor);
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

bool Window::isFullScreen() const {
	return _bounds.width() == SCREEN_WIDTH && _bounds.height() == SCREEN_HEIGHT;
}

} // End of namespace Xeen
} // End of namespace MM
