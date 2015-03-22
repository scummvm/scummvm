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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sherlock/screen.h"
#include "sherlock/sherlock.h"
#include "common/system.h"
#include "graphics/palette.h"

namespace Sherlock {

Screen::Screen(SherlockEngine *vm) : Surface(SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT), _vm(vm),
		_backBuffer(SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT),
		_backBuffer2(SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT) {
	_transitionSeed = 1;
	_fadeStyle = false;
	Common::fill(&_cMap[0], &_cMap[PALETTE_SIZE], 0);
	Common::fill(&_sMap[0], &_sMap[PALETTE_SIZE], 0);
	setFont(1);
}

void Screen::setFont(int fontNumber) {
	_fontNumber = fontNumber;
	Common::String fname = Common::String::format("FONT%d.VGS", fontNumber);
	Common::SeekableReadStream *stream = _vm->_res->load(fname);

	debug("TODO: Loading font %s, size - %d", fname.c_str(), stream->size());

	delete stream;
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

void Screen::getPalette(byte palette[PALETTE_SIZE]) {
	g_system->getPaletteManager()->grabPalette(palette, 0, PALETTE_COUNT);
}

void Screen::setPalette(const byte palette[PALETTE_SIZE]) {
	g_system->getPaletteManager()->setPalette(palette, 0, PALETTE_COUNT);
}

int Screen::equalizePalette(const byte palette[PALETTE_SIZE]) {
	int total = 0;
	byte tempPalette[PALETTE_SIZE];
	getPalette(tempPalette);

	// For any palette component that doesn't already match the given destination
	// palette, change by 1 towards the reference palette component
	for (int idx = 0; idx < PALETTE_SIZE; ++idx) {
		if (tempPalette[idx] > palette[idx])
		{
			tempPalette[idx] = MAX((int)palette[idx], (int)tempPalette[idx] - 4);
			++total;
		} else if (tempPalette[idx] < palette[idx]) {
			tempPalette[idx] = MIN((int)palette[idx], (int)tempPalette[idx] + 4);
			++total;
		}
	}

	if (total > 0)
		// Palette changed, so reload it
		setPalette(tempPalette);

	return total;
}

/**
 * Fade out the palette to black
 */
void Screen::fadeToBlack(int speed) {
	byte tempPalette[PALETTE_SIZE];
	Common::fill(&tempPalette[0], &tempPalette[PALETTE_SIZE], 0);

	while (equalizePalette(tempPalette)) {
		_vm->_events->delay(15 * speed);
	}

	setPalette(tempPalette);
}

/**
 * Fade in a given palette
 */
void Screen::fadeIn(const byte palette[PALETTE_SIZE], int speed) {
	int count = 50;
	while (equalizePalette(palette) && --count) {
		_vm->_events->delay(15 * speed);
	}

	setPalette(palette);
}

/**
 * Adds a rectangle to the list of modified areas of the screen during the
 * current frame
 */
void Screen::addDirtyRect(const Common::Rect &r) {
	_dirtyRects.push_back(r);
	assert(r.isValidRect() && r.width() > 0 && r.height() > 0);
}

/**
 * Merges together overlapping dirty areas of the screen
 */
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

/**
 * Returns the union of two dirty area rectangles
 */
bool Screen::unionRectangle(Common::Rect &destRect, const Common::Rect &src1, const Common::Rect &src2) {
	destRect = src1;
	destRect.extend(src2);

	return !destRect.isEmpty();
}

/**
 * Do a random pixel transition in from _backBuffer surface to the screen
 */
void Screen::randomTransition() {
	Events &events = *_vm->_events;
	const int TRANSITION_MULTIPLIER = 0x15a4e35;
	_dirtyRects.clear();

	for (int idx = 0; idx <= 65535; ++idx) {
		_transitionSeed = _transitionSeed * TRANSITION_MULTIPLIER + 1;
		int offset = _transitionSeed & 65535;

		if (offset < (SHERLOCK_SCREEN_WIDTH * SHERLOCK_SCREEN_HEIGHT))
			*((byte *)getPixels() + offset) = *((const byte *)_backBuffer.getPixels() + offset);
	
		if (idx != 0 && (idx % 100) == 0) {
			// Ensure there's a full screen dirty rect for the next frame update
			if (_dirtyRects.empty())
				addDirtyRect(Common::Rect(0, 0, this->w, this->h));
			
			events.pollEvents();
			events.delay(1);
		}
	}

	// Make sure everything has been transferred
	blitFrom(_backBuffer);
}

/**
 * Transition to the surface from _backBuffer using a vertical transition
 */
void Screen::verticalTransition() {
	Events &events = *_vm->_events;

	byte table[SHERLOCK_SCREEN_WIDTH];
	Common::fill(&table[0], &table[SHERLOCK_SCREEN_WIDTH], 0);
	
	for (int yp = 0; yp < SHERLOCK_SCREEN_HEIGHT; ++yp) {		
		for (int xp = 0; xp < SHERLOCK_SCREEN_WIDTH; ++xp) {
			int temp = (table[xp] >= 197) ? SHERLOCK_SCREEN_HEIGHT - table[xp] : 
				_vm->getRandomNumber(3) + 1;

			if (temp) {
				blitFrom(_backBuffer, Common::Point(xp, table[xp]),
					Common::Rect(xp, table[xp], xp + 1, table[xp] + temp));
				table[xp] += temp;
			}
		}

		events.delay(10);
	}
}

/**
 * Prints the text passed onto the back buffer at the given position and color.
 * The string is then blitted to the screen
 */
void Screen::print(const Common::Point &pt, int fgColor, int bgColor, const char *format, ...) {
	// TODO
}

/**
 * Copies a section of the second back buffer into the main back buffer
 */
void Screen::restoreBackground(const Common::Rect &r) {
	Common::Rect tempRect = r;
	tempRect.clip(Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCENE_HEIGHT));

	if (tempRect.isValidRect())
		_backBuffer.blitFrom(_backBuffer2, Common::Point(tempRect.left, tempRect.top), tempRect);
}

/**
 * Copies a given area to the screen
 */
void Screen::slamArea(int16 xp, int16 yp, int16 w, int16 h) {
	slamRect(Common::Rect(xp, yp, xp + w, yp + h));
}

/**
 * Copies a given area to the screen
 */
void Screen::slamRect(const Common::Rect &r) {
	Common::Rect tempRect = r;
	tempRect.clip(Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));

	if (tempRect.isValidRect())
		blitFrom(_backBuffer, Common::Point(tempRect.left, tempRect.top), tempRect);
}

/**
 * Copy an image from the back buffer to the screen, taking care of both the
 * new area covered by the shape as well as the old area, which must be restored
 */
void Screen::flushImage(ImageFrame *frame, const Common::Point &pt,
		int16 *xp, int16 *yp, int16 *w, int16 *h) {
	Common::Point imgPos = pt + frame->_offset;
	Common::Rect newBounds(imgPos.x, imgPos.y, imgPos.x + frame->_frame.w, imgPos.y + frame->_frame.h);
	Common::Rect oldBounds(*xp, *yp, *xp + *w, *yp + *h);

	// See if the areas of the old and new overlap, and if so combine the areas
	if (newBounds.intersects(oldBounds)) {
		newBounds.extend(oldBounds);
		slamRect(newBounds);
	} else {
		// The two areas are independent, so copy them both
		slamRect(newBounds);
		slamRect(oldBounds);
	}

	*xp = newBounds.left;
	*yp = newBounds.top;
	*w = newBounds.width();
	*h = newBounds.height();
}

} // End of namespace Sherlock
