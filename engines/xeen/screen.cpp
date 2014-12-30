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
#include "xeen/screen.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

const int WX[48] = {
	0, 237, 225, 0,
	235, 70, 52, 108, 232, 103, 226, 8,
	0, 50, 0, 50,
	228, 20, 0, 231,
	72, 99, 65, 80,
	0, 27, 15, 90,
	0, 0, 0, 50,
	12, 8, 232, 226,
	225, 27, 225, 0,
	0, 0, 0, 0,
	0, 0, 0, 0
};

 const int WY[48] = {
		0, 9, 1, 0,
		148, 20, 149, 0,
		9, 156, 0, 8,
		143, 103, 7, 71,
		106, 142, 20, 48,
		37, 59, 23, 28,
		0, 6, 15, 45,
		0, 101, 0, 112,
		11, 147, 74, 26,
		74, 6, 140, 0,
		0, 0, 0, 0,
		0, 0, 0, 0
	};

 const int WW[48] = {
	 320, 80, 94, 230,
	 74, 180, 216, 92,
	 80, 114, 93, 216,
	 320, 216, 320, 132,
	 91, 270, 320, 86,
	 176, 138, 185, 176,
	 320, 180, 146, 130,
	 320, 320, 320, 216,
	 152, 216, 80, 93,
	 94, 168, 94, 0,
	 0, 0, 0, 0,
	 0, 0, 0, 0
 };

const int WH[48] = {
200, 65, 72, 149,
41, 163, 48, 200,
65, 30, 146, 132,
56, 36, 131, 58,
40, 57, 160, 93,
126, 82, 140, 120,
146, 136, 76, 112,
200, 45, 108, 36,
83, 45, 64, 120,
80, 136, 59, 0,
0, 0, 0, 0,
0, 0, 0, 0
};

	 const int WA[48] = {
		 0, 0, 1, 0,
		 2, 3, 4, 5,
		 0, 6, 7, 8,
		 9, 10, 11, 12,
		 13, 14, 15, 16,
		 17, 18, 19, 20,
		 21, 22, 23, 24,
		 25, 26, 27, 28,
		 0, 0, 29, 30,
		 31, 0, 0, 0,
		 0, 0, 0, 0,
		 0, 0, 0, 0
	 };

	 const int WB[48] = {
		 0, 0, 8, 0,
		 8, 8, 8, 0,
		 0, 8, 8, 8,
		 8, 8, 8, 8,
		 8, 8, 8, 8,
		 8, 8, 8, 8,
		 8, 8, 8, 8,
		 8, 8, 8, 8,
		 0, 8, 8, 8,
		 8, 8, 8, 8,
		 8, 8, 8, 8,
		 8, 8, 8, 8
	 };

	 const int WXlo[48] = { 0, 237, 225, 9,
		 0, 80, 0, 0,
		 0, 0, 0, 8,
		 0, 0, 0, 0,
		 0, 0, 0, 0,
		 0, 99, 75, 80,
		 0, 0, 0, 0,
		 0, 0, 0, 0,
		 0, 0, 0, 0,
		 0, 0, 0, 0,
		 0, 0, 0, 0,
		 0, 0, 0, 0
	 };

	 const int WYcl[48] = {
		 0, 12, 1, 8,
		 0, 38, 0, 0,
		 0, 0, 0, 8,
		 0, 0, 0, 0,
		 0, 0, 0, 0,
		 0, 59, 36, 28,
		 0, 0, 0, 0,
		 0, 101, 0, 0,
		 0, 0, 0, 0,
		 0, 0, 0, 0,
		 0, 0, 0, 0,
		 0, 0, 0, 0
	 };

	 const int WXhi[48] = {
		 320, 307, 319, 216,
		 0, 240, 0, 0,
		 0, 0, 0, 224,
		 0, 0, 0, 0,
		 0, 0, 0, 0,
		 0, 237, 245, 256,
		 320, 0, 0, 0,
		 0, 320, 0, 0,
		 52, 0, 0, 0,
		 0, 0, 0, 0,
		 0, 0, 0, 0,
		 0, 0, 0, 0
	 };

	 const int WYch[48] = {
		 200, 68, 73, 140,
		 0, 166, 0, 0,
		 0, 0, 0, 200,
		 0, 0, 0, 0,
		 0, 0, 0, 0,
		 0, 0, 141, 172,
		 148, 146, 0, 0,
		 0, 0, 45, 0,
		 0, 94, 0, 0,
		 0, 0, 0, 0,
		 0, 0, 0, 0,
		 0, 0, 0, 0
	 };

Window::Window(const Common::Rect &bounds, int a, int border, int xLo, int ycL, int xHi, int ycH) {
	_bounds = bounds;
	_a = a;
	_border = border;
	_xLo = xLo;
	_ycL = ycL;
	_xHi = xHi;
	_ycH = ycH;
}

/*------------------------------------------------------------------------*/

/**
 * Constructor
 */
Screen::Screen(XeenEngine *vm) : _vm(vm) {
	_fadeIn = false;
	create(SCREEN_WIDTH, SCREEN_HEIGHT);

	for (int i = 0; i < 48; ++i) {
		warning("Window(Common::Rect(%d, %d, %d, %d), %d, %d, %d, %d, %d, %d);",
			WX[i], WY[i], WX[i] + WW[i], WY[i] + WH[i], WA[i], WB[i], WXlo[i],
			WYcl[i], WXhi[i], WYch[i]);
	}
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
		_tempPaltte[i] = f.readByte() << 2;
}

/**
 * Load a background resource into memory
 */
void Screen::loadBackground(const Common::String &name) {
	File f(name);

	assert(f.size() == (SCREEN_WIDTH * SCREEN_HEIGHT));
	f.read((byte *)getPixels(), SCREEN_WIDTH * SCREEN_HEIGHT);
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
				_mainPalette[i] = (_tempPaltte[i] * val * 2) >> 8;
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
	_savedScreens[slot - 1].free();
}


} // End of namespace Xeen
