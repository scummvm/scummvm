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

#include "sherlock/screen.h"
#include "sherlock/sherlock.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/palette.h"

namespace Sherlock {

Screen::Screen(SherlockEngine *vm) : Surface(g_system->getWidth(), g_system->getHeight()), _vm(vm),
		_backBuffer1(g_system->getWidth(), g_system->getHeight()),
		_backBuffer2(g_system->getWidth(), g_system->getHeight()),
		_backBuffer(&_backBuffer1) {
	_transitionSeed = 1;
	_fadeStyle = false;
	_font = nullptr;
	_fontHeight = 0;
	Common::fill(&_cMap[0], &_cMap[PALETTE_SIZE], 0);
	Common::fill(&_sMap[0], &_sMap[PALETTE_SIZE], 0);
	Common::fill(&_tMap[0], &_tMap[PALETTE_SIZE], 0);
	setFont(1);

	// Rose Tattoo specific fields
	_fadeBytesRead = _fadeBytesToRead = 0;
	_oldFadePercent = 0;
	_scrollSize = 0;
	_currentScroll = 0;
	_targetScroll = 0;
}

Screen::~Screen() {
	delete _font;
}

void Screen::setFont(int fontNumb) {
	// Interactive demo doesn't use fonts
	if (!_vm->_interactiveFl)
		return;

	_fontNumber = fontNumb;
	Common::String fname = Common::String::format("FONT%d.VGS", fontNumb + 1);

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
		g_system->copyRectToScreen(srcP, _surface.pitch, r.left, r.top,
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
		if (tempPalette[idx] > palette[idx]) {
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

void Screen::fadeToBlack(int speed) {
	byte tempPalette[PALETTE_SIZE];
	Common::fill(&tempPalette[0], &tempPalette[PALETTE_SIZE], 0);

	while (equalizePalette(tempPalette)) {
		_vm->_events->delay(15 * speed);
	}

	setPalette(tempPalette);
	fillRect(Common::Rect(0, 0, _surface.w, _surface.h), 0);
}

void Screen::fadeIn(const byte palette[PALETTE_SIZE], int speed) {
	int count = 50;
	while (equalizePalette(palette) && --count) {
		_vm->_events->delay(15 * speed);
	}

	setPalette(palette);
}

void Screen::addDirtyRect(const Common::Rect &r) {
	_dirtyRects.push_back(r);
	assert(r.width() > 0 && r.height() > 0);
}

void Screen::mergeDirtyRects() {
	Common::List<Common::Rect>::iterator rOuter, rInner;

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

void Screen::randomTransition() {
	Events &events = *_vm->_events;
	const int TRANSITION_MULTIPLIER = 0x15a4e35;
	_dirtyRects.clear();

	for (int idx = 0; idx <= 65535 && !_vm->shouldQuit(); ++idx) {
		_transitionSeed = _transitionSeed * TRANSITION_MULTIPLIER + 1;
		int offset = _transitionSeed & 0xFFFF;

		if (offset < (this->w() * this->h()))
			*((byte *)getPixels() + offset) = *((const byte *)_backBuffer->getPixels() + offset);

		if (idx != 0 && (idx % 300) == 0) {
			// Ensure there's a full screen dirty rect for the next frame update
			if (_dirtyRects.empty())
				addDirtyRect(Common::Rect(0, 0, _surface.w, _surface.h));

			events.pollEvents();
			events.delay(1);
		}
	}

	// Make sure everything has been transferred
	blitFrom(*_backBuffer);
}

void Screen::verticalTransition() {
	Events &events = *_vm->_events;

	byte table[640];
	Common::fill(&table[0], &table[640], 0);

	for (int yp = 0; yp < this->h(); ++yp) {
		for (int xp = 0; xp < this->w(); ++xp) {
			int temp = (table[xp] >= (this->h() - 3)) ? this->h() - table[xp] :
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

void Screen::restoreBackground(const Common::Rect &r) {
	if (r.width() > 0 && r.height() > 0) {
		Common::Rect tempRect = r;
		tempRect.clip(Common::Rect(0, 0, this->w(), SHERLOCK_SCENE_HEIGHT));

		if (tempRect.isValidRect())
			_backBuffer1.blitFrom(_backBuffer2, Common::Point(tempRect.left, tempRect.top), tempRect);
	}
}

void Screen::slamArea(int16 xp, int16 yp, int16 width, int16 height) {
	slamRect(Common::Rect(xp, yp, xp + width, yp + height));
}

void Screen::slamRect(const Common::Rect &r) {
	if (r.width() && r.height() > 0) {
		Common::Rect tempRect = r;
		tempRect.clip(Common::Rect(0, 0, this->w(), this->h()));

		if (tempRect.isValidRect())
			blitFrom(*_backBuffer, Common::Point(tempRect.left, tempRect.top), tempRect);
	}
}

void Screen::flushImage(ImageFrame *frame, const Common::Point &pt,
		int16 *xp, int16 *yp, int16 *width, int16 *height) {
	Common::Point imgPos = pt + frame->_offset;
	Common::Rect newBounds(imgPos.x, imgPos.y, imgPos.x + frame->_frame.w, imgPos.y + frame->_frame.h);
	Common::Rect oldBounds(*xp, *yp, *xp + *width, *yp + *height);

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
	*width = newBounds.width();
	*height = newBounds.height();
}

void Screen::print(const Common::Point &pt, byte color, const char *formatStr, ...) {
	// Create the string to display
	va_list args;
	va_start(args, formatStr);
	Common::String str = Common::String::vformat(formatStr, args);
	va_end(args);

	// Figure out area to draw text in
	Common::Point pos = pt;
	int width = stringWidth(str);
	pos.y--;		// Font is always drawing one line higher
	if (!pos.x)
		// Center text horizontally
		pos.x = (this->w() - width) / 2;

	Common::Rect textBounds(pos.x, pos.y, pos.x + width, pos.y + _fontHeight);
	if (textBounds.right > this->w())
		textBounds.moveTo(this->w() - width, textBounds.top);
	if (textBounds.bottom > this->h())
		textBounds.moveTo(textBounds.left, this->h() - _fontHeight);

	// Write out the string at the given position
	writeString(str, Common::Point(textBounds.left, textBounds.top), color);

	// Copy the affected area to the screen
	slamRect(textBounds);
}

void Screen::gPrint(const Common::Point &pt, byte color, const char *formatStr, ...) {
	// Create the string to display
	va_list args;
	va_start(args, formatStr);
	Common::String str = Common::String::vformat(formatStr, args);
	va_end(args);

	// Print the text
	writeString(str, pt, color);
}

int Screen::stringWidth(const Common::String &str) {
	int width = 0;

	for (const char *c = str.c_str(); *c; ++c)
		width += charWidth(*c);

	return width;
}

int Screen::charWidth(char c) {
	if (c == ' ')
		return 5;
	else if (Common::isPrint(c))
		return (*_font)[c - 33]._frame.w + 1;
	else
		return 0;
}

void Screen::writeString(const Common::String &str, const Common::Point &pt, byte color) {
	Common::Point charPos = pt;

	for (const char *c = str.c_str(); *c; ++c) {
		if (*c == ' ')
			charPos.x += 5;
		else {
			assert(Common::isPrint(*c));
			ImageFrame &frame = (*_font)[*c - 33];
			_backBuffer->transBlitFrom(frame, charPos, false, color);
			charPos.x += frame._frame.w + 1;
		}
	}
}

void Screen::vgaBar(const Common::Rect &r, int color) {
	_backBuffer->fillRect(r, color);
	slamRect(r);
}

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

void Screen::buttonPrint(const Common::Point &pt, byte color, bool slamIt,
		const Common::String &str) {
	int xStart = pt.x - stringWidth(str) / 2;

	if (color == COMMAND_FOREGROUND) {
		// First character needs to be highlighted
		if (slamIt) {
			print(Common::Point(xStart, pt.y + 1), COMMAND_HIGHLIGHTED, "%c", str[0]);
			print(Common::Point(xStart + charWidth(str[0]), pt.y + 1),
				COMMAND_FOREGROUND, "%s", str.c_str() + 1);
		} else {
			gPrint(Common::Point(xStart, pt.y), COMMAND_HIGHLIGHTED, "%c", str[0]);
			gPrint(Common::Point(xStart + charWidth(str[0]), pt.y),
				COMMAND_FOREGROUND, "%s", str.c_str() + 1);
		}
	} else if (slamIt) {
		print(Common::Point(xStart, pt.y + 1), color, "%s", str.c_str());
	} else {
		gPrint(Common::Point(xStart, pt.y), color, "%s", str.c_str());
	}
}

void Screen::makePanel(const Common::Rect &r) {
	_backBuffer->fillRect(r, BUTTON_MIDDLE);
	_backBuffer->hLine(r.left, r.top, r.right - 2, BUTTON_TOP);
	_backBuffer->hLine(r.left + 1, r.top + 1, r.right - 3, BUTTON_TOP);
	_backBuffer->vLine(r.left, r.top, r.bottom - 1, BUTTON_TOP);
	_backBuffer->vLine(r.left + 1, r.top + 1, r.bottom - 2, BUTTON_TOP);

	_backBuffer->vLine(r.right - 1, r.top, r.bottom - 1, BUTTON_BOTTOM);
	_backBuffer->vLine(r.right - 2, r.top + 1, r.bottom - 2, BUTTON_BOTTOM);
	_backBuffer->hLine(r.left, r.bottom - 1, r.right - 1, BUTTON_BOTTOM);
	_backBuffer->hLine(r.left + 1, r.bottom - 2, r.right - 1, BUTTON_BOTTOM);
}

void Screen::makeField(const Common::Rect &r) {
	_backBuffer->fillRect(r, BUTTON_MIDDLE);
	_backBuffer->hLine(r.left, r.top, r.right - 1, BUTTON_BOTTOM);
	_backBuffer->hLine(r.left + 1, r.bottom - 1, r.right - 1, BUTTON_TOP);
	_backBuffer->vLine(r.left, r.top + 1, r.bottom - 1, BUTTON_BOTTOM);
	_backBuffer->vLine(r.right - 1, r.top + 1, r.bottom - 2, BUTTON_TOP);
}

void Screen::setDisplayBounds(const Common::Rect &r) {
	assert(r.left == 0 && r.top == 0);
	_sceneSurface.setPixels(_backBuffer1.getPixels(), r.width(), r.height());

	_backBuffer = &_sceneSurface;
}

void Screen::resetDisplayBounds() {
	_backBuffer = &_backBuffer1;
}

Common::Rect Screen::getDisplayBounds() {
	return (_backBuffer == &_sceneSurface) ? Common::Rect(0, 0, _sceneSurface.w(), _sceneSurface.h()) :
		Common::Rect(0, 0, this->w(), this->h());
}

void Screen::synchronize(Common::Serializer &s) {
	int fontNumb = _fontNumber;
	s.syncAsByte(fontNumb);
	if (s.isLoading())
		setFont(fontNumb);
}

void Screen::initPaletteFade(int bytesToRead) {
	Common::copy(&_cMap[0], &_cMap[PALETTE_SIZE], &_sMap[0]);
	Common::copy(&_cMap[0], &_cMap[PALETTE_SIZE], &_tMap[0]);

	// Set how many bytes need to be read / have been read
	_fadeBytesRead = 0;
	_fadeBytesToRead = bytesToRead;
	_oldFadePercent = 0;
}

int Screen::fadeRead(Common::SeekableReadStream &stream, byte *buf, int totalSize) {
	warning("TODO: fadeRead");
	stream.read(buf, totalSize);
	return totalSize;
}

/**
 * Creates a grey-scale version of the passed palette
 */
void Screen::setupBGArea(const byte cMap[PALETTE_SIZE]) {
	warning("TODO");
}

/**
 * Initializes scroll variables
 */
void Screen::initScrollVars() {
	_scrollSize = 0;
	_currentScroll = 0;
	_targetScroll = 0;
}

} // End of namespace Sherlock
