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
#include "common/system.h"
#include "graphics/surface.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/screen.h"

namespace Sci {

GfxScreen::GfxScreen(ResourceManager *resMan, int16 width, int16 height, bool upscaledHires) :
	_resMan(resMan), _width(width), _height(height), _upscaledHires(upscaledHires) {

	_pixels = _width * _height;
	_displayWidth = _width;
	_displayHeight = _height;

	if (_upscaledHires) {
		_displayWidth *= 2;
		_displayHeight *= 2;

#ifdef ENABLE_SCI32
		// SCI32 also corrects the aspect ratio when upscaling the resolution.
		// This is especially needed in GK1, as the credits video is 640x480.
		if (getSciVersion() >= SCI_VERSION_2)
			_displayHeight = _displayHeight * 6 / 5;
#endif
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

	if (_resMan->isVGA() || (_resMan->isAmiga32color())) {
		// It's not 100% accurate to set white to be 255 for amiga 32-color games
		//  255 is defined as white in our sci at all times, so it doesnt matter
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

GfxScreen::~GfxScreen() {
	free(_visualScreen);
	free(_priorityScreen);
	free(_controlScreen);
	free(_displayScreen);
}

void GfxScreen::copyToScreen() {
	g_system->copyRectToScreen(_activeScreen, _displayWidth, 0, 0, _displayWidth, _displayHeight);
}

void GfxScreen::copyFromScreen(byte *buffer) {
	Graphics::Surface *screen;
	screen = g_system->lockScreen();
	memcpy(buffer, screen->pixels, _displayWidth * _displayHeight);
	g_system->unlockScreen();
}

void GfxScreen::kernelSyncWithFramebuffer() {
	Graphics::Surface *screen = g_system->lockScreen();

	memcpy(_displayScreen, screen->pixels, _displayPixels);
	g_system->unlockScreen();
}

void GfxScreen::copyRectToScreen(const Common::Rect &rect) {
	if (!_upscaledHires)  {
		g_system->copyRectToScreen(_activeScreen + rect.top * _displayWidth + rect.left, _displayWidth, rect.left, rect.top, rect.width(), rect.height());
	} else {
		g_system->copyRectToScreen(_activeScreen + rect.top * 2 * _displayWidth + rect.left * 2, _displayWidth, rect.left * 2, rect.top * 2, rect.width() * 2, rect.height() * 2);
	}
}

// This copies a rect to screen w/o scaling adjustment and is only meant to be used on hires graphics used in upscaled hires mode
void GfxScreen::copyDisplayRectToScreen(const Common::Rect &rect) {
	if (!_upscaledHires)
		error("copyDisplayRectToScreen: not in upscaled hires mode");
	g_system->copyRectToScreen(_activeScreen + rect.top * _displayWidth + rect.left, _displayWidth, rect.left, rect.top, rect.width(), rect.height());
}

void GfxScreen::copyRectToScreen(const Common::Rect &rect, int16 x, int16 y) {
	if (!_upscaledHires)  {
		g_system->copyRectToScreen(_activeScreen + rect.top * _displayWidth + rect.left, _displayWidth, x, y, rect.width(), rect.height());
	} else {
		g_system->copyRectToScreen(_activeScreen + rect.top * 2 * _displayWidth + rect.left * 2, _displayWidth, x * 2, y * 2, rect.width() * 2, rect.height() * 2);
	}
}

byte GfxScreen::getDrawingMask(byte color, byte prio, byte control) {
	byte flag = 0;
	if (color != 255)
		flag |= SCI_SCREEN_MASK_VISUAL;
	if (prio != 255)
		flag |= SCI_SCREEN_MASK_PRIORITY;
	if (control != 255)
		flag |= SCI_SCREEN_MASK_CONTROL;
	return flag;
}

void GfxScreen::putPixel(int x, int y, byte drawMask, byte color, byte priority, byte control) {
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
void GfxScreen::putPixelOnDisplay(int x, int y, byte color) {
	int offset = y * _displayWidth + x;
	_displayScreen[offset] = color;
}

// Sierra's Bresenham line drawing
// WARNING: Do not just blindly replace this with Graphics::drawLine(), as it seems to create issues with flood fill
void GfxScreen::drawLine(Common::Point startPoint, Common::Point endPoint, byte color, byte priority, byte control) {
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

byte GfxScreen::getVisual(int x, int y) {
	return _visualScreen[y * _width + x];
}

byte GfxScreen::getPriority(int x, int y) {
	return _priorityScreen[y * _width + x];
}

byte GfxScreen::getControl(int x, int y) {
	return _controlScreen[y * _width + x];
}

byte GfxScreen::isFillMatch(int16 x, int16 y, byte screenMask, byte t_color, byte t_pri, byte t_con) {
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

int GfxScreen::bitsGetDataSize(Common::Rect rect, byte mask) {
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

void GfxScreen::bitsSave(Common::Rect rect, byte mask, byte *memoryPtr) {
	memcpy(memoryPtr, (void *)&rect, sizeof(rect)); memoryPtr += sizeof(rect);
	memcpy(memoryPtr, (void *)&mask, sizeof(mask)); memoryPtr += sizeof(mask);

	if (mask & SCI_SCREEN_MASK_VISUAL) {
		bitsSaveScreen(rect, _visualScreen, _width, memoryPtr);
		bitsSaveDisplayScreen(rect, memoryPtr);
	}
	if (mask & SCI_SCREEN_MASK_PRIORITY) {
		bitsSaveScreen(rect, _priorityScreen, _width, memoryPtr);
	}
	if (mask & SCI_SCREEN_MASK_CONTROL) {
		bitsSaveScreen(rect, _controlScreen, _width, memoryPtr);
	}
	if (mask & SCI_SCREEN_MASK_DISPLAY) {
		if (!_upscaledHires)
			error("bitsSave() called w/o being in upscaled hires mode");
		bitsSaveScreen(rect, _displayScreen, _displayWidth, memoryPtr);
	}
}

void GfxScreen::bitsSaveScreen(Common::Rect rect, byte *screen, uint16 screenWidth, byte *&memoryPtr) {
	int width = rect.width();
	int y;

	screen += (rect.top * screenWidth) + rect.left;

	for (y = rect.top; y < rect.bottom; y++) {
		memcpy(memoryPtr, (void*)screen, width); memoryPtr += width;
		screen += screenWidth;
	}
}

void GfxScreen::bitsSaveDisplayScreen(Common::Rect rect, byte *&memoryPtr) {
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

void GfxScreen::bitsGetRect(byte *memoryPtr, Common::Rect *destRect) {
	memcpy((void *)destRect, memoryPtr, sizeof(Common::Rect));
}

void GfxScreen::bitsRestore(byte *memoryPtr) {
	Common::Rect rect;
	byte mask;

	memcpy((void *)&rect, memoryPtr, sizeof(rect)); memoryPtr += sizeof(rect);
	memcpy((void *)&mask, memoryPtr, sizeof(mask)); memoryPtr += sizeof(mask);

	if (mask & SCI_SCREEN_MASK_VISUAL) {
		bitsRestoreScreen(rect, memoryPtr, _visualScreen, _width);
		bitsRestoreDisplayScreen(rect, memoryPtr);
	}
	if (mask & SCI_SCREEN_MASK_PRIORITY) {
		bitsRestoreScreen(rect, memoryPtr, _priorityScreen, _width);
	}
	if (mask & SCI_SCREEN_MASK_CONTROL) {
		bitsRestoreScreen(rect, memoryPtr, _controlScreen, _width);
	}
	if (mask & SCI_SCREEN_MASK_DISPLAY) {
		if (!_upscaledHires)
			error("bitsRestore() called w/o being in upscaled hires mode");
		bitsRestoreScreen(rect, memoryPtr, _displayScreen, _displayWidth);
	}
}

void GfxScreen::bitsRestoreScreen(Common::Rect rect, byte *&memoryPtr, byte *screen, uint16 screenWidth) {
	int width = rect.width();
	int y;

	screen += (rect.top * screenWidth) + rect.left;

	for (y = rect.top; y < rect.bottom; y++) {
		memcpy((void*) screen, memoryPtr, width); memoryPtr += width;
		screen += screenWidth;
	}
}

void GfxScreen::bitsRestoreDisplayScreen(Common::Rect rect, byte *&memoryPtr) {
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

void GfxScreen::setPalette(Palette*pal) {
	// just copy palette to system
	byte bpal[4 * 256];
	// Get current palette, update it and put back
	g_system->grabPalette(bpal, 0, 256);
	for (int16 i = 0; i < 256; i++) {
		if (!pal->colors[i].used)
			continue;
		bpal[i * 4] = CLIP(pal->colors[i].r * pal->intensity[i] / 100, 0, 255);
		bpal[i * 4 + 1] = CLIP(pal->colors[i].g * pal->intensity[i] / 100, 0, 255);
		bpal[i * 4 + 2] = CLIP(pal->colors[i].b * pal->intensity[i] / 100, 0, 255);
		bpal[i * 4 + 3] = 100;
	}
	g_system->setPalette(bpal, 0, 256);
}

void GfxScreen::setVerticalShakePos(uint16 shakePos) {
	if (!_upscaledHires)
		g_system->setShakePos(shakePos);
	else
		g_system->setShakePos(shakePos * 2);
}

void GfxScreen::dither(bool addToFlag) {
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

void GfxScreen::debugUnditherSetState(bool flag) {
	_unditherState = flag;
}

int16 *GfxScreen::unditherGetMemorial() {
	if (_unditherState)
		return (int16 *)&_unditherMemorial;
	else
		return NULL;
}

void GfxScreen::debugShowMap(int mapNo) {
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

void GfxScreen::scale2x(byte *src, byte *dst, int16 srcWidth, int16 srcHeight) {
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

int16 GfxScreen::kernelPicNotValid(int16 newPicNotValid) {
	int16 oldPicNotValid;

	if (getSciVersion() >= SCI_VERSION_1_1) {
		oldPicNotValid = _picNotValidSci11;

		if (newPicNotValid != -1)
			_picNotValidSci11 = newPicNotValid;
	} else {
		oldPicNotValid = _picNotValid;

		if (newPicNotValid != -1)
			_picNotValid = newPicNotValid;
	}

	return oldPicNotValid;
}

} // End of namespace Sci
