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

#include "common/system.h"
#include "graphics/palette.h"
#include "graphics/surface.h"
#include "xeen/screen.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

Window::Window() : _vm(nullptr),  _enabled(false), _a(0), _border(0),
	_xLo(0), _xHi(0), _ycL(0), _ycH(0) {
}

Window::Window(XeenEngine *vm, const Common::Rect &bounds, int a, int border, 
		int xLo, int ycL, int xHi, int ycH): 
		_vm(vm), _enabled(false), _a(a), _border(border), 
		_xLo(xLo), _ycL(ycL), _xHi(xHi), _ycH(ycH) {
	setBounds(bounds);
	create(_vm->_screen, Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
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

/**
 * Update the window
 */
void Window::update() {
	// Since all window drawing is done on the screen surface anyway,
	// there's nothing that needs to be updated here
}

/**
 * Adds an area that requires redrawing on the next frame update
 */
void Window::addDirtyRect(const Common::Rect &r) {
	_vm->_screen->addDirtyRect(r);
}

/**
 * Fill the content area of a window with the current background color 
 */
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

/*------------------------------------------------------------------------*/

/**
 * Constructor
 */
Screen::Screen(XeenEngine *vm) : _vm(vm) {
	_fadeIn = false;
	create(SCREEN_WIDTH, SCREEN_HEIGHT);
	Common::fill(&_tempPalette[0], &_tempPalette[PALETTE_SIZE], 0);
	Common::fill(&_mainPalette[0], &_mainPalette[PALETTE_SIZE], 0);

	// Load font data for the screen
	File f("fnt");
	byte *data = new byte[f.size()];
	f.read(data, f.size());
	_fontData = data;
}

Screen::~Screen() {
	delete[] _fontData;
}

void Screen::setupWindows() {
	Window windows[40] = {
		Window(_vm, Common::Rect(0, 0, 320, 200), 0, 0, 0, 0, 320, 200),
		Window(_vm, Common::Rect(237, 9, 317, 74), 0, 0, 237, 12, 307, 68),
		Window(_vm, Common::Rect(225, 1, 319, 73), 1, 8, 225, 1, 319, 73),
		Window(_vm, Common::Rect(0, 0, 230, 149), 0, 0, 9, 8, 216, 140),
		Window(_vm, Common::Rect(235, 148, 309, 189), 2, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(70, 20, 250, 183), 3, 8, 80, 38, 240, 166),
		Window(_vm, Common::Rect(52, 149, 268, 197), 4, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(108, 0, 200, 200), 5, 0, 0, 0, 0, 0),
		Window(_vm, Common::Rect(232, 9, 312, 74), 0, 0, 0, 0, 0, 0),
		Window(_vm, Common::Rect(103, 156, 217, 186), 6, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(226, 0, 319, 146), 7, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(8, 8, 224, 140), 8, 8, 8, 8, 224, 200),
		Window(_vm, Common::Rect(0, 143, 320, 199), 9, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(50, 103, 266, 139), 10, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(0, 7, 320, 138), 11, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(50, 71, 182, 129), 12, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(228, 106, 319, 146), 13, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(20, 142, 290, 199), 14, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(0, 20, 320, 180), 15, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(231, 48, 317, 141), 16, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(72, 37, 248, 163), 17, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(99, 59, 237, 141), 18, 8, 99, 59, 237, 0),
		Window(_vm, Common::Rect(65, 23, 250, 163), 19, 8, 75, 36, 245, 141),
		Window(_vm, Common::Rect(80, 28, 256, 148), 20, 8, 80, 28, 256, 172),
		Window(_vm, Common::Rect(0, 0, 320, 146), 21, 8, 0, 0, 320, 148),
		Window(_vm, Common::Rect(27, 6, 207, 142), 22, 8, 0, 0, 0, 146),
		Window(_vm, Common::Rect(15, 15, 161, 91), 23, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(90, 45, 220, 157), 24, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(0, 0, 320, 200), 25, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(0, 101, 320, 146), 26, 8, 0, 101, 320, 0),
		Window(_vm, Common::Rect(0, 0, 320, 108), 27, 8, 0, 0, 0, 45),
		Window(_vm, Common::Rect(50, 112, 266, 148), 28, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(12, 11, 164, 94), 0, 0, 0, 0, 52, 0),
		Window(_vm, Common::Rect(8, 147, 224, 192), 0, 8, 0, 0, 0, 94),
		Window(_vm, Common::Rect(232, 74, 312, 138), 29, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(226, 26, 319, 146), 30, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(225, 74, 319, 154), 31, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(27, 6, 195, 142), 0, 8, 0, 0, 0, 0),
		Window(_vm, Common::Rect(225, 140, 319, 199), 0, 8, 0, 0, 0, 0)
	};

	_windows = Common::Array<Window>(windows, 40);
}

void Screen::closeWindows() {
	for (int i = (int)_windowStack.size() - 1; i >= 0; --i)
		_windowStack[i]->close();
	assert(_windowStack.size() == 0);
}

void Screen::update() {
	// Merge the dirty rects
	mergeDirtyRects();

	// Loop through copying dirty areas to the physical screen
	Common::List<Common::Rect>::iterator i;
	for (i = _dirtyRects.begin(); i != _dirtyRects.end(); ++i) {
		const Common::Rect &r = *i;
		const byte *srcP = (const byte *)getBasePtr(r.left, r.top);
		g_system->copyRectToScreen(srcP, this->pitch, r.left, r.top,
			r.width(), r.height());
	}

	// Signal the physical screen to update
	g_system->updateScreen();
	_dirtyRects.clear();
}

void Screen::addDirtyRect(const Common::Rect &r) {
	assert(r.isValidRect() && r.width() > 0 && r.height() > 0 
		&& r.left >= 0 && r.top >= 0
		&& r.right <= SCREEN_WIDTH && r.bottom <= SCREEN_HEIGHT);
	_dirtyRects.push_back(r);
}

void Screen::mergeDirtyRects() {
	Common::List<Common::Rect>::iterator rOuter, rInner;

	// Ensure dirty rect list has at least two entries
	rOuter = _dirtyRects.begin();
	for (int i = 0; i < 2; ++i, ++rOuter) {
		if (rOuter == _dirtyRects.end())
			return;
	}

	// Process the dirty rect list to find any rects to merge
	for (rOuter = _dirtyRects.begin(); rOuter != _dirtyRects.end(); ++rOuter) {
		rInner = rOuter;
		while (++rInner != _dirtyRects.end()) {

			if ((*rOuter).intersects(*rInner)) {
				// these two rectangles overlap or
				// are next to each other - merge them

				unionRectangle(*rOuter, *rOuter, *rInner);

				// remove the inner rect from the list
				_dirtyRects.erase(rInner);

				// move back to beginning of list
				rInner = rOuter;
			}
		}
	}
}

bool Screen::unionRectangle(Common::Rect &destRect, const Common::Rect &src1, const Common::Rect &src2) {
	destRect = src1;
	destRect.extend(src2);

	return !destRect.isEmpty();
}

/**
 * Load a palette resource into the temporary palette
 */
void Screen::loadPalette(const Common::String &name) {
	File f(name);
	for (int i = 0; i < PALETTE_SIZE; ++i)
		_tempPalette[i] = f.readByte() << 2;
}

/**
 * Load a background resource into memory
 */
void Screen::loadBackground(const Common::String &name) {
	File f(name);

	assert(f.size() == (SCREEN_WIDTH * SCREEN_HEIGHT));
	f.read((byte *)getPixels(), SCREEN_WIDTH * SCREEN_HEIGHT);
	addDirtyRect(Common::Rect(0, 0, this->w, this->h));
}

/**
 * Copy a loaded background into a display page
 */
void Screen::loadPage(int pageNum) {
	assert(pageNum == 0 || pageNum == 1);
	if (_pages[0].empty()) {
		_pages[0].create(SCREEN_WIDTH, SCREEN_HEIGHT);
		_pages[1].create(SCREEN_WIDTH, SCREEN_HEIGHT);
	}

	blitTo(_pages[pageNum]);
}

void Screen::freePages() {
	_pages[0].free();
	_pages[1].free();
}

/**
 * Merge the two pages along a horizontal split point
 */
void Screen::horizMerge(int xp) {
	if (_pages[0].empty())
		return;

	for (int y = 0; y < SCREEN_HEIGHT; ++y) {
		byte *destP = (byte *)getBasePtr(0, y);
		const byte *srcP = (const byte *)_pages[0].getBasePtr(0, y);
		Common::copy(srcP, srcP + SCREEN_WIDTH - xp, destP);

		if (xp != 0) {
			srcP = (const byte *)_pages[1].getBasePtr(0, y);
			Common::copy(srcP + SCREEN_WIDTH - xp, srcP + SCREEN_WIDTH, destP + SCREEN_WIDTH - xp);
		}
	}
}

/**
 * Merge the two pages along a vertical split point
 */
void Screen::vertMerge(int yp) {
	if (_pages[0].empty())
		return;

	for (int y = 0; y < SCREEN_HEIGHT - yp; ++y) {
		const byte *srcP = (const byte *)_pages[0].getBasePtr(0, y);
		byte *destP = (byte *)getBasePtr(0, y);
		Common::copy(srcP, srcP + SCREEN_WIDTH, destP);
	}

	for (int y = SCREEN_HEIGHT - yp; y < SCREEN_HEIGHT; ++y) {
		const byte *srcP = (const byte *)_pages[1].getBasePtr(0, y);
		byte *destP = (byte *)getBasePtr(0, y);
		Common::copy(srcP, srcP + SCREEN_WIDTH, destP);
	}
}

void Screen::draw(void *data) {
	// TODO: Figure out data structure that can be passed to method
	assert(!data);
	drawScreen();
}

/**
 * Mark the entire screen for drawing
 */
void Screen::drawScreen() {
	addDirtyRect(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
}

void Screen::fadeIn(int step) {
	_fadeIn = true;
	fadeInner(step);
}

void Screen::fadeOut(int step) {
	_fadeIn = false;
	fadeInner(step);
}

void Screen::fadeInner(int step) {
	for (int idx = 128; idx >= 0 && !_vm->shouldQuit(); idx -= step) {
		int val = MAX(idx, 0);
		bool flag = !_fadeIn;
		if (!flag) {
			val = -(val - 128);
			flag = step != 0x81;
		}

		if (!flag) {
			step = 0x80;
		} else {
			// Create a scaled palette from the temporary one
			for (int i = 0; i < PALETTE_SIZE; ++i) {
				_mainPalette[i] = (_tempPalette[i] * val * 2) >> 8;
			}

			updatePalette();
		}

		_vm->_events->pollEventsAndWait();
	}
}

void Screen::updatePalette() {
	updatePalette(_mainPalette, 0, 16);
}

void Screen::updatePalette(const byte *pal, int start, int count16) {
	g_system->getPaletteManager()->setPalette(pal, start, count16 * 16);
}

void Screen::saveBackground(int slot) {
	assert(slot > 0 && slot < 10);
	_savedScreens[slot - 1].copyFrom(*this);
}

void Screen::restoreBackground(int slot) {
	assert(slot > 0 && slot < 10);

	_savedScreens[slot - 1].blitTo(*this);
}

void Screen::frameWindow(uint bgType) {
	if (bgType >= 4)
		return;

	if (bgType == 0) {
		// Totally black background
		_vm->_screen->fillRect(Common::Rect(8, 8, 224, 140), 0);
	} else {
		const byte *lookup = BACKGROUND_XLAT + bgType;
		for (int yp = 8; yp < 140; ++yp) {
			byte *destP = (byte *)_vm->_screen->getBasePtr(8, yp);
			for (int xp = 8; xp < 224; ++xp, ++destP)
				*destP = lookup[*destP];
		}
	}
}

} // End of namespace Xeen
