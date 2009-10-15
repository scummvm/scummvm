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
 * $URL$
 * $Id$
 *
 */

#include "common/timer.h"
#include "common/util.h"
#include "graphics/surface.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/tools.h"
#include "sci/gui/gui_screen.h"

namespace Sci {

SciGuiScreen::SciGuiScreen(int16 width, int16 height, int16 scaleFactor) : 
	_width(width), _height(height) {

	_pixels = _width * _height;

	// if you want to do scaling, adjust putPixel() accordingly
	_displayWidth = _width * scaleFactor;
	_displayHeight = _height * scaleFactor;
	_displayPixels = _displayWidth * _displayHeight;

	_visualScreen = (byte *)calloc(_pixels, 1);
	_priorityScreen = (byte *)calloc(_pixels, 1);
	_controlScreen = (byte *)calloc(_pixels, 1);
	_displayScreen = (byte *)calloc(_displayPixels, 1);

	// Sets display screen to be actually displayed
	_activeScreen = _displayScreen;

	_picNotValid = false;
	_unditherState = true;
}

SciGuiScreen::~SciGuiScreen() {
	free(_visualScreen);
	free(_priorityScreen);
	free(_controlScreen);
	free(_displayScreen);
}

void SciGuiScreen::copyToScreen() {
	g_system->copyRectToScreen(_activeScreen, _displayWidth, 0, 0, _displayWidth, _displayHeight);
}

void SciGuiScreen::copyFromScreen(byte *buffer) {
	Graphics::Surface *screen;
	screen = g_system->lockScreen();
	memcpy(buffer, screen->pixels, _displayWidth * _displayHeight);
	g_system->unlockScreen();
}

void SciGuiScreen::copyRectToScreen(const Common::Rect &rect) {
	g_system->copyRectToScreen(_activeScreen + rect.top * _displayWidth + rect.left, _displayWidth, rect.left, rect.top, rect.width(), rect.height());
}

void SciGuiScreen::copyRectToScreen(const Common::Rect &rect, int16 x, int16 y) {
	g_system->copyRectToScreen(_activeScreen + rect.top * _displayWidth + rect.left, _displayWidth, x, y, rect.width(), rect.height());
}

byte SciGuiScreen::getDrawingMask(byte color, byte prio, byte control) {
	byte flag = 0;
	if (color != 255)
		flag |= SCI_SCREEN_MASK_VISUAL;
	if (prio != 255)
		flag |= SCI_SCREEN_MASK_PRIORITY;
	if (control != 255)
		flag |= SCI_SCREEN_MASK_CONTROL;
	return flag;
}

void SciGuiScreen::putPixel(int x, int y, byte drawMask, byte color, byte priority, byte control) {
	int offset = y * _width + x;

	if (drawMask & SCI_SCREEN_MASK_VISUAL) {
		_visualScreen[offset] = color;
		_displayScreen[offset] = color;
	}
	if (drawMask & SCI_SCREEN_MASK_PRIORITY)
		_priorityScreen[offset] = priority;
	if (drawMask & SCI_SCREEN_MASK_CONTROL)
		_controlScreen[offset] = control;
}

// Sierra's Bresenham line drawing
// WARNING: Do not just blindly replace this with Graphics::drawLine(), as it seems to create issues with flood fill
void SciGuiScreen::drawLine(Common::Point startPoint, Common::Point endPoint, byte color, byte priority, byte control) {
	int16 left = startPoint.x;
	int16 top = startPoint.y;
	int16 right = endPoint.x;
	int16 bottom = endPoint.y;

	//set_drawing_flag
	byte drawMask = getDrawingMask(color, priority, control);

	// horizontal line
	if (top == bottom) {
		if (right < left)
			SWAP(right, left);
		for (int i = left; i <= right; i++)
			putPixel(i, top, drawMask, color, priority, control);
		return;
	}
	// vertical line
	if (left == right) {
		if (top > bottom)
			SWAP(top, bottom);
		for (int i = top; i <= bottom; i++)
			putPixel(left, i, drawMask, color, priority, control);
		return;
	}
	// sloped line - draw with Bresenham algorithm
	int dy = bottom - top;
	int dx = right - left;
	int stepy = dy < 0 ? -1 : 1;
	int stepx = dx < 0 ? -1 : 1;
	dy = ABS(dy) << 1;
	dx = ABS(dx) << 1;

	// setting the 1st and last pixel
	putPixel(left, top, drawMask, color, priority, control);
	putPixel(right, bottom, drawMask, color, priority, control);
	// drawing the line
	if (dx > dy) { // going horizontal
		int fraction = dy - (dx >> 1);
		while (left != right) {
			if (fraction >= 0) {
				top += stepy;
				fraction -= dx;
			}
			left += stepx;
			fraction += dy;
			putPixel(left, top, drawMask, color, priority, control);
		}
	} else { // going vertical
		int fraction = dx - (dy >> 1);
		while (top != bottom) {
			if (fraction >= 0) {
				left += stepx;
				fraction -= dy;
			}
			top += stepy;
			fraction += dx;
			putPixel(left, top, drawMask, color, priority, control);
		}
	}
}

byte SciGuiScreen::getVisual(int x, int y) {
	return _visualScreen[y * _width + x];
}

byte SciGuiScreen::getPriority(int x, int y) {
	return _priorityScreen[y * _width + x];
}

byte SciGuiScreen::getControl(int x, int y) {
	return _controlScreen[y * _width + x];
}

byte SciGuiScreen::isFillMatch(int16 x, int16 y, byte screenMask, byte t_color, byte t_pri, byte t_con) {
	int offset = y * _width + x;
	byte match = 0;

	if (screenMask & SCI_SCREEN_MASK_VISUAL && *(_visualScreen + offset) == t_color)
		match |= SCI_SCREEN_MASK_VISUAL;
	if (screenMask & SCI_SCREEN_MASK_PRIORITY && *(_priorityScreen + offset) == t_pri)
		match |= SCI_SCREEN_MASK_PRIORITY;
	if (screenMask & SCI_SCREEN_MASK_CONTROL && *(_controlScreen + offset) == t_con)
		match |= SCI_SCREEN_MASK_CONTROL;
	return match;
}

int SciGuiScreen::getBitsDataSize(Common::Rect rect, byte mask) {
	int byteCount = sizeof(rect) + sizeof(mask);
	int pixels = rect.width() * rect.height();
	if (mask & SCI_SCREEN_MASK_VISUAL) {
		byteCount += pixels; // _visualScreen
		byteCount += pixels; // _displayScreen
	}
	if (mask & SCI_SCREEN_MASK_PRIORITY) {
		byteCount += pixels; // _priorityScreen
	}
	if (mask & SCI_SCREEN_MASK_CONTROL) {
		byteCount += pixels; // _controlScreen
	}

	return byteCount;
}

void SciGuiScreen::saveBits(Common::Rect rect, byte mask, byte *memoryPtr) {
	memcpy(memoryPtr, (void *)&rect, sizeof(rect)); memoryPtr += sizeof(rect);
	memcpy(memoryPtr, (void *)&mask, sizeof(mask)); memoryPtr += sizeof(mask);

	if (mask & SCI_SCREEN_MASK_VISUAL) {
		saveBitsScreen(rect, _visualScreen, memoryPtr);
		saveBitsScreen(rect, _displayScreen, memoryPtr);
	}
	if (mask & SCI_SCREEN_MASK_PRIORITY) {
		saveBitsScreen(rect, _priorityScreen, memoryPtr);
	}
	if (mask & SCI_SCREEN_MASK_CONTROL) {
		saveBitsScreen(rect, _controlScreen, memoryPtr);
	}
}

void SciGuiScreen::saveBitsScreen(Common::Rect rect, byte *screen, byte *&memoryPtr) {
	int width = rect.width();
	int y;

	screen += (rect.top * _width) + rect.left;

	for (y = rect.top; y < rect.bottom; y++) {
		memcpy(memoryPtr, (void*)screen, width); memoryPtr += width;
		screen += _width;
	}
}

void SciGuiScreen::restoreBits(byte *memoryPtr) {
	Common::Rect rect;
	byte mask;

	memcpy((void *)&rect, memoryPtr, sizeof(rect)); memoryPtr += sizeof(rect);
	memcpy((void *)&mask, memoryPtr, sizeof(mask)); memoryPtr += sizeof(mask);

	if (mask & SCI_SCREEN_MASK_VISUAL) {
		restoreBitsScreen(rect, memoryPtr, _visualScreen);
		restoreBitsScreen(rect, memoryPtr, _displayScreen);
	}
	if (mask & SCI_SCREEN_MASK_PRIORITY) {
		restoreBitsScreen(rect, memoryPtr, _priorityScreen);
	}
	if (mask & SCI_SCREEN_MASK_CONTROL) {
		restoreBitsScreen(rect, memoryPtr, _controlScreen);
	}
}

void SciGuiScreen::restoreBitsScreen(Common::Rect rect, byte *&memoryPtr, byte *screen) {
	int width = rect.width();
	int y;

	screen += (rect.top * _width) + rect.left;

	for (y = rect.top; y < rect.bottom; y++) {
		memcpy((void*) screen, memoryPtr, width); memoryPtr += width;
		screen += _width;
	}
}

void SciGuiScreen::setPalette(GuiPalette*pal) {
	// just copy palette to system
	byte bpal[4 * 256];
	// Get current palette, update it and put back
	g_system->grabPalette(bpal, 0, 256);
	for (int16 i = 0; i < 256; i++) {
		if (!pal->colors[i].used)
			continue;
		bpal[i * 4] = pal->colors[i].r * pal->intensity[i] / 100;
		bpal[i * 4 + 1] = pal->colors[i].g * pal->intensity[i] / 100;
		bpal[i * 4 + 2] = pal->colors[i].b * pal->intensity[i] / 100;
		bpal[i * 4 + 3] = 100;
	}
	g_system->setPalette(bpal, 0, 256);
}

void SciGuiScreen::setVerticalShakePos(uint16 shakePos) {
	g_system->setShakePos(shakePos);
}

// Currently not really done, its supposed to be possible to only dither _visualScreen
void SciGuiScreen::dither(bool addToFlag) {
	int y, x;
	byte color;
	byte *screenPtr = _visualScreen;
	byte *displayPtr = _displayScreen;

	if (!_unditherState) {
		// Do dithering on visual and display-screen
		for (y = 0; y < _height; y++) {
			for (x = 0; x < _width; x++) {
				color = *screenPtr;
				if (color & 0xF0) {
					color ^= color << 4;
					color = ((x^y) & 1) ? color >> 4 : color & 0x0F;
					*screenPtr = color; *displayPtr = color;
				}
				screenPtr++; displayPtr++;
			}
		}
	} else {
		if (!addToFlag)
			memset(&_unditherMemorial, 0, sizeof(_unditherMemorial));
		// Do dithering on visual screen and put decoded but undithered byte onto display-screen
		for (y = 0; y < _height; y++) {
			for (x = 0; x < _width; x++) {
				color = *screenPtr;
				if (color & 0xF0) {
					color ^= color << 4;
					// remember dither combination for cel-undithering
					_unditherMemorial[color]++;
					// if decoded color wants do dither with black on left side, we turn it around
					//  otherwise the normal ega color would get used for display
					if (color & 0xF0) {
						*displayPtr = color;
					}	else {
						*displayPtr = color << 4;
					}
					color = ((x^y) & 1) ? color >> 4 : color & 0x0F;
					*screenPtr = color;
				}
				screenPtr++; displayPtr++;
			}
		}
	}
}

void SciGuiScreen::unditherSetState(bool flag) {
	_unditherState = flag;
}

int16 *SciGuiScreen::unditherGetMemorial() {
	if (_unditherState)
		return (int16 *)&_unditherMemorial;
	else
		return NULL;
}

void SciGuiScreen::debugShowMap(int mapNo) {
	switch (mapNo) {
	case 0:
		_activeScreen = _visualScreen;
		break;
	case 1:
		_activeScreen = _priorityScreen;
		break;
	case 2:
		_activeScreen = _controlScreen;
		break;
	case 3:
		_activeScreen = _displayScreen;
		break;
	}
}

} // End of namespace Sci
