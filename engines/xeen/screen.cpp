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

/**
 * Constructor
 */
Screen::Screen(XeenEngine *vm) : _vm(vm) {
	_fadeMode = false;
	create(SCREEN_WIDTH, SCREEN_HEIGHT);
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
			srcP = (const byte *)_pages[1].getBasePtr(xp, y);
			Common::copy(srcP, srcP + SCREEN_WIDTH - xp, destP + xp);
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

void Screen::fade(int step) {
	_fadeMode = true;
	fadeInner(step);
}

void Screen::fade2(int step) {
	_fadeMode = false;
	fadeInner(step);
}

void Screen::fadeInner(int step) {
	for (int idx = 128; idx != 0 && !_vm->shouldQuit(); idx -= step) {
		int val = idx;
		bool flag = !_fadeMode;
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

} // End of namespace Xeen
