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
#include "common/util.h"
#include "graphics/palette.h"

namespace Sherlock {

Screen::Screen(SherlockEngine *vm) : Surface(SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT), _vm(vm),
		_backBuffer1(SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT),
		_backBuffer2(SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT),
		_backBuffer(&_backBuffer1) {
	_transitionSeed = 1;
	_fadeStyle = false;
	_font = nullptr;
	_fontHeight = 0;
	Common::fill(&_cMap[0], &_cMap[PALETTE_SIZE], 0);
	Common::fill(&_sMap[0], &_sMap[PALETTE_SIZE], 0);
	setFont(1);
}

Screen::~Screen() {
	delete _font;
}

void Screen::setFont(int fontNumber) {
	_fontNumber = fontNumber;
	Common::String fname = Common::String::format("FONT%d.VGS", fontNumber + 1);

	// Discard any previous font and read in new one
	delete _font;
	_font = new ImageFile(fname);

	// Iterate through the frames to find the tallest font character
	_fontHeight = 0;
	for (uint idx = 0; idx < _font->size(); ++idx)
		_fontHeight = MAX((uint16)_fontHeight, (*_font)[idx]._frame.h);
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
			*((byte *)getPixels() + offset) = *((const byte *)_backBuffer->getPixels() + offset);
	
		if (idx != 0 && (idx % 100) == 0) {
			// Ensure there's a full screen dirty rect for the next frame update
			if (_dirtyRects.empty())
				addDirtyRect(Common::Rect(0, 0, this->w, this->h));
			
			events.pollEvents();
			events.delay(1);
		}
	}

	// Make sure everything has been transferred
	blitFrom(_backBuffer1);
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
				blitFrom(_backBuffer1, Common::Point(xp, table[xp]),
					Common::Rect(xp, table[xp], xp + 1, table[xp] + temp));
				table[xp] += temp;
			}
		}

		events.delay(10);
	}
}

/**
 * Copies a section of the second back buffer into the main back buffer
 */
void Screen::restoreBackground(const Common::Rect &r) {
	if (r.width() > 0 && r.height() > 0) {
		Common::Rect tempRect = r;
		tempRect.clip(Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCENE_HEIGHT));

		if (tempRect.isValidRect())
			_backBuffer1.blitFrom(_backBuffer2, Common::Point(tempRect.left, tempRect.top), tempRect);
	}
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
	if (r.width() && r.height() > 0) {
		Common::Rect tempRect = r;
		tempRect.clip(Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));

		if (tempRect.isValidRect())
			blitFrom(*_backBuffer, Common::Point(tempRect.left, tempRect.top), tempRect);
	}
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
		Common::Rect mergedBounds = newBounds;
		mergedBounds.extend(oldBounds);
		mergedBounds.right += 1;
		mergedBounds.bottom += 1;

		slamRect(mergedBounds);
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

/**
 * Prints the text passed onto the back buffer at the given position and color.
 * The string is then blitted to the screen
 */
void Screen::print(const Common::Point &pt, int color, const char *format, ...) {
	// Create the string to display
	char buffer[100];
	va_list args;

	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);
	Common::String str(buffer);

	// Figure out area to draw text in
	Common::Point pos = pt;
	int width = stringWidth(str);
	pos.y--;		// Font is always drawing one line higher
	if (!pos.x)
		// Center text horizontally
		pos.x = (SHERLOCK_SCREEN_WIDTH - width) / 2;

	Common::Rect textBounds(pos.x, pos.y, pos.x + width, pos.y + _fontHeight);
	if (textBounds.right > SHERLOCK_SCREEN_WIDTH)
		textBounds.moveTo(SHERLOCK_SCREEN_WIDTH - width, textBounds.top);
	if (textBounds.bottom > SHERLOCK_SCREEN_HEIGHT)
		textBounds.moveTo(textBounds.left, SHERLOCK_SCREEN_HEIGHT - _fontHeight);

	// Write out the string at the given position
	writeString(str, Common::Point(textBounds.left, textBounds.top), color);

	// Copy the affected area to the screen
	slamRect(textBounds);
}

/**
 * Print a strings onto the back buffer without blitting it to the screen
 */
void Screen::gPrint(const Common::Point &pt, int color, const char *format, ...) {
	// Create the string to display
	char buffer[100];
	va_list args;

	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);
	Common::String str(buffer);

	// Print the text
	writeString(str, pt, color);
}


/**
 * Returns the width of a string in pixels
 */
int Screen::stringWidth(const Common::String &str) {
	int width = 0;

	for (const char *c = str.c_str(); *c; ++c) 
		width += charWidth(*c);

	return width;
}

/**
 * Returns the width of a character in pixels
 */
int Screen::charWidth(char c) {
	if (c == ' ')
		return 5;
	else if (c > ' ' && c <= '~')
		return (*_font)[c - 33]._frame.w + 1;
	else
		return 0;
}

/**
 * Draws the given string into the back buffer using the images stored in _font
 */
void Screen::writeString(const Common::String &str, const Common::Point &pt, int color) {
	Common::Point charPos = pt;

	for (const char *c = str.c_str(); *c; ++c) {
		if (*c == ' ')
			charPos.x += 5;
		else {
			assert(*c > ' ' && *c <= '~');
			ImageFrame &frame = (*_font)[*c - 33];
			_backBuffer->transBlitFrom(frame, charPos, false, color);
			charPos.x += frame._frame.w + 1;
		}
	}
}

/**
 * Fills an area on the back buffer, and then copies it to the screen
 */
void Screen::vgaBar(const Common::Rect &r, int color) {
	_backBuffer->fillRect(r, color);
	slamRect(r);
}

/**
 * Draws a button for use in the inventory, talk, and examine dialogs.
 */
void Screen::makeButton(const Common::Rect &bounds, int textX,
		const Common::String &str) {

	Surface &bb = *_backBuffer;
	bb.fillRect(Common::Rect(bounds.left, bounds.top, bounds.right, bounds.top + 1), BUTTON_TOP);
	bb.fillRect(Common::Rect(bounds.left, bounds.top, bounds.left + 1, bounds.bottom), BUTTON_TOP);
	bb.fillRect(Common::Rect(bounds.right - 1, bounds.top, bounds.right, bounds.bottom), BUTTON_BOTTOM);
	bb.fillRect(Common::Rect(bounds.left + 1, bounds.bottom - 1, bounds.right, bounds.bottom), BUTTON_BOTTOM);
	bb.fillRect(Common::Rect(bounds.left + 1, bounds.top + 1, bounds.right - 1, bounds.bottom - 1), BUTTON_MIDDLE);

	gPrint(Common::Point(textX, bounds.top), COMMAND_HIGHLIGHTED, "%c", str[0]);
	gPrint(Common::Point(textX + charWidth(str[0]), bounds.top), 
		COMMAND_FOREGROUND, "%s", str.c_str() + 1);
}

} // End of namespace Sherlock
