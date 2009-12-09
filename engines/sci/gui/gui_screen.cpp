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
#include "sci/gui/gui_screen.h"

namespace Sci {

SciGuiScreen::SciGuiScreen(ResourceManager *resMan, int16 width, int16 height, bool upscaledHires) : 
	_resMan(resMan), _width(width), _height(height), _upscaledHires(upscaledHires) {

	_pixels = _width * _height;

	_displayWidth = _width;
	_displayHeight = _height;
	if (_upscaledHires) {
		_displayWidth *= 2;
		_displayHeight *= 2;
	}
	_displayPixels = _displayWidth * _displayHeight;

	_visualScreen = (byte *)calloc(_pixels, 1);
	_priorityScreen = (byte *)calloc(_pixels, 1);
	_controlScreen = (byte *)calloc(_pixels, 1);
	_displayScreen = (byte *)calloc(_displayPixels, 1);

	// Sets display screen to be actually displayed
	_activeScreen = _displayScreen;

	_picNotValid = 0;
	_picNotValidSci11 = 0;
	_unditherState = true;

	if (_resMan->isVGA()) {
		_colorWhite = 255;
		if (getSciVersion() >= SCI_VERSION_1_1)
			_colorDefaultVectorData = 255;
		else
			_colorDefaultVectorData = 0;
	} else {
		_colorWhite = 15;
		_colorDefaultVectorData = 0;
	}

	// Initialize the actual screen
	initGraphics(_displayWidth, _displayHeight, _displayWidth > 320);
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
	if (!_upscaledHires)  {
		g_system->copyRectToScreen(_activeScreen + rect.top * _displayWidth + rect.left, _displayWidth, rect.left, rect.top, rect.width(), rect.height());
	} else {
		g_system->copyRectToScreen(_activeScreen + rect.top * 2 * _displayWidth + rect.left * 2, _displayWidth, rect.left * 2, rect.top * 2, rect.width() * 2, rect.height() * 2);
	}
}

void SciGuiScreen::copyRectToScreen(const Common::Rect &rect, int16 x, int16 y) {
	if (!_upscaledHires)  {
		g_system->copyRectToScreen(_activeScreen + rect.top * _displayWidth + rect.left, _displayWidth, x, y, rect.width(), rect.height());
	} else {
		g_system->copyRectToScreen(_activeScreen + rect.top * 2 * _displayWidth + rect.left * 2, _displayWidth, x * 2, y * 2, rect.width() * 2, rect.height() * 2);
	}
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
		if (!_upscaledHires) {
			_displayScreen[offset] = color;
		} else {
			int displayOffset = y * 2 * _displayWidth + x * 2;
			_displayScreen[displayOffset] = color;
			_displayScreen[displayOffset + 1] = color;
			_displayScreen[displayOffset + _displayWidth] = color;
			_displayScreen[displayOffset + _displayWidth + 1] = color;
		}
	}
	if (drawMask & SCI_SCREEN_MASK_PRIORITY)
		_priorityScreen[offset] = priority;
	if (drawMask & SCI_SCREEN_MASK_CONTROL)
		_controlScreen[offset] = control;
}

// This will just change a pixel directly on displayscreen. Its supposed to get only used on upscaled-Hires games where
//  hires content needs to get drawn ONTO the upscaled display screen (like japanese fonts, hires portraits, etc.)
void SciGuiScreen::putPixelOnDisplay(int x, int y, byte color) {
	int offset = y * _width + x;
	_displayScreen[offset] = color;
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

int SciGuiScreen::bitsGetDataSize(Common::Rect rect, byte mask) {
	int byteCount = sizeof(rect) + sizeof(mask);
	int pixels = rect.width() * rect.height();
	if (mask & SCI_SCREEN_MASK_VISUAL) {
		byteCount += pixels; // _visualScreen
		if (!_upscaledHires) {
			byteCount += pixels; // _displayScreen
		} else {
			byteCount += pixels * 4; // _displayScreen (upscaled hires)
		}
	}
	if (mask & SCI_SCREEN_MASK_PRIORITY) {
		byteCount += pixels; // _priorityScreen
	}
	if (mask & SCI_SCREEN_MASK_CONTROL) {
		byteCount += pixels; // _controlScreen
	}
	if (mask & SCI_SCREEN_MASK_DISPLAY) {
		if (!_upscaledHires)
			error("bitsGetDataSize() called w/o being in upscaled hires mode");
		byteCount += pixels; // _displayScreen (coordinates actually are given to us for hires displayScreen)
	}

	return byteCount;
}

void SciGuiScreen::bitsSave(Common::Rect rect, byte mask, byte *memoryPtr) {
	memcpy(memoryPtr, (void *)&rect, sizeof(rect)); memoryPtr += sizeof(rect);
	memcpy(memoryPtr, (void *)&mask, sizeof(mask)); memoryPtr += sizeof(mask);

	if (mask & SCI_SCREEN_MASK_VISUAL) {
		bitsSaveScreen(rect, _visualScreen, memoryPtr);
		bitsSaveDisplayScreen(rect, memoryPtr);
	}
	if (mask & SCI_SCREEN_MASK_PRIORITY) {
		bitsSaveScreen(rect, _priorityScreen, memoryPtr);
	}
	if (mask & SCI_SCREEN_MASK_CONTROL) {
		bitsSaveScreen(rect, _controlScreen, memoryPtr);
	}
	if (mask & SCI_SCREEN_MASK_DISPLAY) {
		if (!_upscaledHires)
			error("bitsSave() called w/o being in upscaled hires mode");
		bitsSaveScreen(rect, _displayScreen, memoryPtr);
	}
}

void SciGuiScreen::bitsSaveScreen(Common::Rect rect, byte *screen, byte *&memoryPtr) {
	int width = rect.width();
	int y;

	screen += (rect.top * _width) + rect.left;

	for (y = rect.top; y < rect.bottom; y++) {
		memcpy(memoryPtr, (void*)screen, width); memoryPtr += width;
		screen += _width;
	}
}

void SciGuiScreen::bitsSaveDisplayScreen(Common::Rect rect, byte *&memoryPtr) {
	byte *screen = _displayScreen;
	int width = rect.width();
	int y;

	if (!_upscaledHires) {
		screen += (rect.top * _displayWidth) + rect.left;
	} else {
		screen += (rect.top * 2 * _displayWidth) + rect.left * 2;
		width *= 2;
		rect.top *= 2; rect.bottom *= 2;
	}

	for (y = rect.top; y < rect.bottom; y++) {
		memcpy(memoryPtr, (void*)screen, width); memoryPtr += width;
		screen += _displayWidth;
	}
}

void SciGuiScreen::bitsGetRect(byte *memoryPtr, Common::Rect *destRect) {
	memcpy((void *)destRect, memoryPtr, sizeof(Common::Rect));
}

void SciGuiScreen::bitsRestore(byte *memoryPtr) {
	Common::Rect rect;
	byte mask;

	memcpy((void *)&rect, memoryPtr, sizeof(rect)); memoryPtr += sizeof(rect);
	memcpy((void *)&mask, memoryPtr, sizeof(mask)); memoryPtr += sizeof(mask);

	if (mask & SCI_SCREEN_MASK_VISUAL) {
		bitsRestoreScreen(rect, memoryPtr, _visualScreen);
		bitsRestoreDisplayScreen(rect, memoryPtr);
	}
	if (mask & SCI_SCREEN_MASK_PRIORITY) {
		bitsRestoreScreen(rect, memoryPtr, _priorityScreen);
	}
	if (mask & SCI_SCREEN_MASK_CONTROL) {
		bitsRestoreScreen(rect, memoryPtr, _controlScreen);
	}
	if (mask & SCI_SCREEN_MASK_DISPLAY) {
		if (!_upscaledHires)
			error("bitsRestore() called w/o being in upscaled hires mode");
		bitsRestoreScreen(rect, memoryPtr, _displayScreen);
	}
}

void SciGuiScreen::bitsRestoreScreen(Common::Rect rect, byte *&memoryPtr, byte *screen) {
	int width = rect.width();
	int y;

	screen += (rect.top * _width) + rect.left;

	for (y = rect.top; y < rect.bottom; y++) {
		memcpy((void*) screen, memoryPtr, width); memoryPtr += width;
		screen += _width;
	}
}

void SciGuiScreen::bitsRestoreDisplayScreen(Common::Rect rect, byte *&memoryPtr) {
	byte *screen = _displayScreen;
	int width = rect.width();
	int y;

	if (!_upscaledHires) {
		screen += (rect.top * _displayWidth) + rect.left;
	} else {
		screen += (rect.top * 2 * _displayWidth) + rect.left * 2;
		width *= 2;
		rect.top *= 2; rect.bottom *= 2;
	}

	for (y = rect.top; y < rect.bottom; y++) {
		memcpy((void*) screen, memoryPtr, width); memoryPtr += width;
		screen += _displayWidth;
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
	if (!_upscaledHires)
		g_system->setShakePos(shakePos);
	else
		g_system->setShakePos(shakePos * 2);
}

void SciGuiScreen::dither(bool addToFlag) {
	int y, x;
	byte color, ditheredColor;
	byte *visualPtr = _visualScreen;
	byte *displayPtr = _displayScreen;

	if (!_unditherState) {
		// Do dithering on visual and display-screen
		for (y = 0; y < _height; y++) {
			for (x = 0; x < _width; x++) {
				color = *visualPtr;
				if (color & 0xF0) {
					color ^= color << 4;
					color = ((x^y) & 1) ? color >> 4 : color & 0x0F;
					*displayPtr = color;
					if (_upscaledHires) {
						*(displayPtr + 1) = color;
						*(displayPtr + _displayWidth) = color;
						*(displayPtr + _displayWidth + 1) = color;
					}
					*visualPtr = color;
				}
				visualPtr++; displayPtr++;
				if (_upscaledHires)
					displayPtr++;
			}
			if (_upscaledHires)
				displayPtr += _displayWidth;
		}
	} else {
		if (!addToFlag)
			memset(&_unditherMemorial, 0, sizeof(_unditherMemorial));
		// Do dithering on visual screen and put decoded but undithered byte onto display-screen
		for (y = 0; y < _height; y++) {
			for (x = 0; x < _width; x++) {
				color = *visualPtr;
				if (color & 0xF0) {
					color ^= color << 4;
					// remember dither combination for cel-undithering
					_unditherMemorial[color]++;
					// if decoded color wants do dither with black on left side, we turn it around
					//  otherwise the normal ega color would get used for display
					if (color & 0xF0) {
						ditheredColor = color;
					}	else {
						ditheredColor = color << 4;
					}
					*displayPtr = ditheredColor;
					if (_upscaledHires) {
						*(displayPtr + 1) = ditheredColor;
						*(displayPtr + _displayWidth) = ditheredColor;
						*(displayPtr + _displayWidth + 1) = ditheredColor;
					}
					color = ((x^y) & 1) ? color >> 4 : color & 0x0F;
					*visualPtr = color;
				}
				visualPtr++; displayPtr++;
				if (_upscaledHires)
					displayPtr++;
			}
			if (_upscaledHires)
				displayPtr += _displayWidth;
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
	// We cannot really support changing maps when in upscaledHires mode
	if (_upscaledHires)
		return;

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
	copyToScreen();
}

void SciGuiScreen::scale2x(byte *src, byte *dst, int16 srcWidth, int16 srcHeight) {
	int newWidth = srcWidth * 2;
	byte *srcPtr = src;

	for (int y = 0; y < srcHeight; y++) {
		for (int x = 0; x < srcWidth; x++) {
			int destOffset = y * 2 * newWidth + x * 2;
			dst[destOffset] = *srcPtr;
			dst[destOffset + 1] = *srcPtr;
			dst[destOffset + newWidth] = *srcPtr;
			dst[destOffset + newWidth + 1] = *srcPtr;
			srcPtr++;
		}
	}
}

} // End of namespace Sci
