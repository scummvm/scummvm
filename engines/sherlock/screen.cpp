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
#include "sherlock/scalpel/scalpel_screen.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/palette.h"

namespace Sherlock {

Screen *Screen::init(SherlockEngine *vm) {
	if (vm->getGameID() == GType_SerratedScalpel)
		return new Scalpel::ScalpelScreen(vm);
	else
		return new Screen(vm);
}

Screen::Screen(SherlockEngine *vm) : Surface(g_system->getWidth(), g_system->getHeight()), _vm(vm),
		_backBuffer1(g_system->getWidth(), g_system->getHeight()),
		_backBuffer2(g_system->getWidth(), g_system->getHeight()),
		_backBuffer(&_backBuffer1) {
	_transitionSeed = 1;
	_fadeStyle = false;
	Common::fill(&_cMap[0], &_cMap[PALETTE_SIZE], 0);
	Common::fill(&_sMap[0], &_sMap[PALETTE_SIZE], 0);
	Common::fill(&_tMap[0], &_tMap[PALETTE_SIZE], 0);
	
	// Set up the initial font
	setFont(IS_SERRATED_SCALPEL ? 1 : 4);

	// Rose Tattoo specific fields
	_fadeBytesRead = _fadeBytesToRead = 0;
	_oldFadePercent = 0;
	_flushScreen = false;
}

Screen::~Screen() {
	Fonts::free();
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

void Screen::makeAllDirty() {
	addDirtyRect(Common::Rect(0, 0, this->w(), this->h()));
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
	assert(IS_SERRATED_SCALPEL);

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

void Screen::fadeIntoScreen3DO(int speed) {
	Events &events = *_vm->_events;
	uint16 *currentScreenBasePtr = (uint16 *)getPixels();
	uint16 *targetScreenBasePtr = (uint16 *)_backBuffer->getPixels();
	uint16  currentScreenPixel = 0;
	uint16  targetScreenPixel = 0;

	uint16  currentScreenPixelRed = 0;
	uint16  currentScreenPixelGreen = 0;
	uint16  currentScreenPixelBlue = 0;

	uint16  targetScreenPixelRed = 0;
	uint16  targetScreenPixelGreen = 0;
	uint16  targetScreenPixelBlue = 0;

	uint16  screenWidth = this->w();
	uint16  screenHeight = this->h();
	uint16  screenX = 0;
	uint16  screenY = 0;
	uint16  pixelsChanged = 0;

	_dirtyRects.clear();

	do {
		pixelsChanged = 0;
		uint16 *currentScreenPtr = currentScreenBasePtr;
		uint16 *targetScreenPtr = targetScreenBasePtr;

		for (screenY = 0; screenY < screenHeight; screenY++) {
			for (screenX = 0; screenX < screenWidth; screenX++) {
				currentScreenPixel = *currentScreenPtr;
				targetScreenPixel  = *targetScreenPtr;

				if (currentScreenPixel != targetScreenPixel) {
					// pixel doesn't match, adjust accordingly
					currentScreenPixelRed   = currentScreenPixel & 0xF800;
					currentScreenPixelGreen = currentScreenPixel & 0x07E0;
					currentScreenPixelBlue  = currentScreenPixel & 0x001F;
					targetScreenPixelRed    = targetScreenPixel & 0xF800;
					targetScreenPixelGreen  = targetScreenPixel & 0x07E0;
					targetScreenPixelBlue   = targetScreenPixel & 0x001F;

					if (currentScreenPixelRed != targetScreenPixelRed) {
						if (currentScreenPixelRed < targetScreenPixelRed) {
							currentScreenPixelRed += 0x0800;
						} else {
							currentScreenPixelRed -= 0x0800;
						}
					}
					if (currentScreenPixelGreen != targetScreenPixelGreen) {
						// Adjust +2/-2 because we are running RGB555 at RGB565
						if (currentScreenPixelGreen < targetScreenPixelGreen) {
							currentScreenPixelGreen += 0x0040;
						} else {
							currentScreenPixelGreen -= 0x0040;
						}
					}
					if (currentScreenPixelBlue != targetScreenPixelBlue) {
						if (currentScreenPixelBlue < targetScreenPixelBlue) {
							currentScreenPixelBlue += 0x0001;
						} else {
							currentScreenPixelBlue -= 0x0001;
						}
					}
					*currentScreenPtr = currentScreenPixelRed | currentScreenPixelGreen | currentScreenPixelBlue;
					pixelsChanged++;
				}

				currentScreenPtr++;
				targetScreenPtr++;
			}
		}

		// Too much considered dirty at the moment
		addDirtyRect(Common::Rect(0, 0, screenWidth, screenHeight));

		events.pollEvents();
		events.delay(10 * speed);
	} while ((pixelsChanged) && (!_vm->shouldQuit()));
}

void Screen::blitFrom3DOcolorLimit(uint16 limitColor) {
	uint16 *currentScreenPtr = (uint16 *)getPixels();
	uint16 *targetScreenPtr = (uint16 *)_backBuffer->getPixels();
	uint16  currentScreenPixel = 0;

	uint16  screenWidth = this->w();
	uint16  screenHeight = this->h();
	uint16  screenX = 0;
	uint16  screenY = 0;

	uint16  currentScreenPixelRed = 0;
	uint16  currentScreenPixelGreen = 0;
	uint16  currentScreenPixelBlue = 0;

	uint16  limitPixelRed = limitColor & 0xF800;
	uint16  limitPixelGreen = limitColor & 0x07E0;
	uint16  limitPixelBlue = limitColor & 0x001F;

	for (screenY = 0; screenY < screenHeight; screenY++) {
		for (screenX = 0; screenX < screenWidth; screenX++) {
			currentScreenPixel = *targetScreenPtr;

			currentScreenPixelRed   = currentScreenPixel & 0xF800;
			currentScreenPixelGreen = currentScreenPixel & 0x07E0;
			currentScreenPixelBlue  = currentScreenPixel & 0x001F;

			if (currentScreenPixelRed < limitPixelRed)
				currentScreenPixelRed = limitPixelRed;
			if (currentScreenPixelGreen < limitPixelGreen)
				currentScreenPixelGreen = limitPixelGreen;
			if (currentScreenPixelBlue < limitPixelBlue)
				currentScreenPixelBlue = limitPixelBlue;

			*currentScreenPtr = currentScreenPixelRed | currentScreenPixelGreen | currentScreenPixelBlue;
			currentScreenPtr++;
			targetScreenPtr++;
		}
	}

	// Too much considered dirty at the moment
	addDirtyRect(Common::Rect(0, 0, screenWidth, screenHeight));
}

void Screen::restoreBackground(const Common::Rect &r) {
	if (r.width() > 0 && r.height() > 0)
		_backBuffer1.blitFrom(_backBuffer2, Common::Point(r.left, r.top), r);
}

void Screen::slamArea(int16 xp, int16 yp, int16 width, int16 height) {
	slamRect(Common::Rect(xp, yp, xp + width, yp + height));
}

void Screen::slamRect(const Common::Rect &r) {
	if (r.width() && r.height() > 0) {
		Common::Rect srcRect = r, destRect = r;

		destRect.translate(-_currentScroll.x, -_currentScroll.y);

		if (destRect.left < 0) {
			srcRect.left += -destRect.left;
			destRect.left = 0;
		}
		if (destRect.top < 0) {
			srcRect.top += -destRect.top;
			destRect.top = 0;
		}
		if (destRect.right > SHERLOCK_SCREEN_WIDTH) {
			srcRect.right -= (destRect.left - SHERLOCK_SCREEN_WIDTH);
			destRect.right = SHERLOCK_SCREEN_WIDTH;
		}
		if (destRect.bottom > SHERLOCK_SCREEN_HEIGHT) {
			srcRect.bottom -= (destRect.bottom - SHERLOCK_SCREEN_HEIGHT);
			destRect.bottom = SHERLOCK_SCREEN_HEIGHT;
		}

		if (srcRect.isValidRect())
			blitFrom(*_backBuffer, Common::Point(destRect.left, destRect.top), srcRect);
	}
}


void Screen::flushImage(ImageFrame *frame, const Common::Point &pt, int16 *xp, int16 *yp, 
		int16 *width, int16 *height) {
	Common::Point imgPos = pt + frame->_offset;
	Common::Rect newBounds(imgPos.x, imgPos.y, imgPos.x + frame->_frame.w, imgPos.y + frame->_frame.h);
	Common::Rect oldBounds(*xp, *yp, *xp + *width, *yp + *height);

	if (!_flushScreen) {
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
	}

	*xp = newBounds.left;
	*yp = newBounds.top;
	*width = newBounds.width();
	*height = newBounds.height();
}

void Screen::flushScaleImage(ImageFrame *frame, const Common::Point &pt, int16 *xp, int16 *yp,
		int16 *width, int16 *height, int scaleVal) {
	Common::Point imgPos(pt.x + frame->sDrawXOffset(scaleVal), pt.y + frame->sDrawYOffset(scaleVal));
	Common::Rect newBounds(imgPos.x, imgPos.y, imgPos.x + frame->sDrawXSize(scaleVal), 
		imgPos.y + frame->sDrawYSize(scaleVal));
	Common::Rect oldBounds(*xp, *yp, *xp + *width, *yp + *height);

	if (!_flushScreen) {
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
	}

	*xp = newBounds.left;
	*yp = newBounds.top;
	*width = newBounds.width();
	*height = newBounds.height();
}

void Screen::flushImage(ImageFrame *frame, const Common::Point &pt, Common::Rect &newBounds, int scaleVal) {
	Common::Point newPos(newBounds.left, newBounds.top);
	Common::Point newSize(newBounds.width(), newBounds.height());

	if (scaleVal == SCALE_THRESHOLD)
		flushImage(frame, pt, &newPos.x, &newPos.y, &newSize.x, &newSize.y);
	else
		flushScaleImage(frame, pt, &newPos.x, &newPos.y, &newSize.x, &newSize.y, scaleVal);

	// Transfer the pos and size amounts into a single bounds rect
	newBounds = Common::Rect(newPos.x, newPos.y, newPos.x + newSize.x, newPos.y + newSize.y);
}

void Screen::blockMove(const Common::Rect &r) {
	Common::Rect bounds = r;
	slamRect(bounds);
}

void Screen::blockMove() {
	blockMove(Common::Rect(0, 0, w(), h()));
}

void Screen::print(const Common::Point &pt, uint color, const char *formatStr, ...) {
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

void Screen::gPrint(const Common::Point &pt, uint color, const char *formatStr, ...) {
	// Create the string to display
	va_list args;
	va_start(args, formatStr);
	Common::String str = Common::String::vformat(formatStr, args);
	va_end(args);

	// Print the text
	writeString(str, pt, color);
}

void Screen::writeString(const Common::String &str, const Common::Point &pt, uint overrideColor) {
	Fonts::writeString(_backBuffer, str, pt, overrideColor);
}

void Screen::vgaBar(const Common::Rect &r, int color) {
	_backBuffer->fillRect(r, color);
	slamRect(r);
}

void Screen::setDisplayBounds(const Common::Rect &r) {
	_sceneSurface.setPixels(_backBuffer1.getBasePtr(r.left, r.top), r.width(), r.height(), _backBuffer1.getPixelFormat());

	_backBuffer = &_sceneSurface;
}

void Screen::resetDisplayBounds() {
	_backBuffer = &_backBuffer1;
}

Common::Rect Screen::getDisplayBounds() {
	return (_backBuffer == &_sceneSurface) ? Common::Rect(0, 0, _sceneSurface.w(), _sceneSurface.h()) :
		Common::Rect(0, 0, this->w(), this->h());
}

void Screen::synchronize(Serializer &s) {
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

void Screen::translatePalette(byte palette[PALETTE_SIZE]) {
	for (int idx = 0; idx < PALETTE_SIZE; ++idx)
		palette[idx] = VGA_COLOR_TRANS(palette[idx]);
}

} // End of namespace Sherlock
