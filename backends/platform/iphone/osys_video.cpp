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

#include "osys_main.h"

const OSystem::GraphicsMode* OSystem_IPHONE::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}


int OSystem_IPHONE::getDefaultGraphicsMode() const {
	return -1;
}

bool OSystem_IPHONE::setGraphicsMode(const char *mode) {
	return true;
}

bool OSystem_IPHONE::setGraphicsMode(int mode) {
	return true;
}

int OSystem_IPHONE::getGraphicsMode() const {
	return -1;
}

void OSystem_IPHONE::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	//printf("initSize(%i, %i)\n", width, height);

	_screenWidth = width;
	_screenHeight = height;

	free(_offscreen);

	_offscreen = (byte *)malloc(width * height);
	bzero(_offscreen, width * height);

	free(_overlayBuffer);

	int fullSize = _screenWidth * _screenHeight * sizeof(OverlayColor);
	_overlayBuffer = (OverlayColor *)malloc(fullSize);
	clearOverlay();

	free(_fullscreen);

	_fullscreen = (uint16 *)malloc(fullSize);
	bzero(_fullscreen, fullSize);

	iPhone_initSurface(width, height);

	_fullScreenIsDirty = false;
	dirtyFullScreen();
	_mouseVisible = false;
	_screenChangeCount++;
	updateScreen();
}

int16 OSystem_IPHONE::getHeight() {
	return _screenHeight;
}

int16 OSystem_IPHONE::getWidth() {
	return _screenWidth;
}

void OSystem_IPHONE::setPalette(const byte *colors, uint start, uint num) {
	//printf("setPalette()\n");
	const byte *b = colors;

	for (uint i = start; i < start + num; ++i) {
		_palette[i] = Graphics::RGBToColor<Graphics::ColorMasks<565> >(b[0], b[1], b[2]);
		b += 4;
	}

	dirtyFullScreen();
}

void OSystem_IPHONE::grabPalette(byte *colors, uint start, uint num) {
	//printf("grabPalette()\n");
}

void OSystem_IPHONE::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) {
	//printf("copyRectToScreen(%i, %i, %i, %i)\n", x, y, w, h);
	//Clip the coordinates
	if (x < 0) {
		w += x;
		buf -= x;
		x = 0;
	}

	if (y < 0) {
		h += y;
		buf -= y * pitch;
		y = 0;
	}

	if (w > _screenWidth - x) {
		w = _screenWidth - x;
	}

	if (h > _screenHeight - y) {
		h = _screenHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;

	if (!_fullScreenIsDirty) {
		_dirtyRects.push_back(Common::Rect(x, y, x + w, y + h));
	}


	byte *dst = _offscreen + y * _screenWidth + x;
	if (_screenWidth == pitch && pitch == w)
		memcpy(dst, buf, h * w);
	else {
		do {
			memcpy(dst, buf, w);
			buf += pitch;
			dst += _screenWidth;
		} while (--h);
	}
}

void OSystem_IPHONE::clipRectToScreen(int16 &x, int16 &y, int16 &w, int16 &h) {
	if (x < 0) {
		w += x;
		x = 0;
	}

	if (y < 0) {
		h += y;
		y = 0;
	}

	if (w > _screenWidth - x)
		w = _screenWidth - x;

	if (h > _screenHeight - y)
		h = _screenHeight - y;

	if (w < 0) {
		w = 0;
	}

	if (h < 0) {
		h = 0;
	}
}

void OSystem_IPHONE::updateScreen() {
	//printf("updateScreen(): %i dirty rects.\n", _dirtyRects.size());

	if (_dirtyRects.size() == 0 && _dirtyOverlayRects.size() == 0 && !_mouseDirty)
		return;

	internUpdateScreen();
	_fullScreenIsDirty = false;
	_fullScreenOverlayIsDirty = false;

	iPhone_updateScreen();
}

void OSystem_IPHONE::internUpdateScreen() {
	int16 mouseX = _mouseX - _mouseHotspotX;
	int16 mouseY = _mouseY - _mouseHotspotY;
	int16 mouseWidth = _mouseWidth;
	int16 mouseHeight = _mouseHeight;

	clipRectToScreen(mouseX, mouseY, mouseWidth, mouseHeight);

	Common::Rect mouseRect(mouseX, mouseY, mouseX + mouseWidth, mouseY + mouseHeight);

	if (_mouseDirty) {
		if (!_fullScreenIsDirty) {
			_dirtyRects.push_back(_lastDrawnMouseRect);
			_dirtyRects.push_back(mouseRect);
		}
		if (!_fullScreenOverlayIsDirty && _overlayVisible) {
			_dirtyOverlayRects.push_back(_lastDrawnMouseRect);
			_dirtyOverlayRects.push_back(mouseRect);
		}
		_mouseDirty = false;
		_lastDrawnMouseRect = mouseRect;
	}

	while (_dirtyRects.size()) {
		Common::Rect dirtyRect = _dirtyRects.remove_at(_dirtyRects.size() - 1);

		//printf("Drawing: (%i, %i) -> (%i, %i)\n", dirtyRect.left, dirtyRect.top, dirtyRect.right, dirtyRect.bottom);

		drawDirtyRect(dirtyRect);

		if (_overlayVisible)
			drawDirtyOverlayRect(dirtyRect);

		drawMouseCursorOnRectUpdate(dirtyRect, mouseRect);
		updateHardwareSurfaceForRect(dirtyRect);
	}

	if (_overlayVisible) {
		while (_dirtyOverlayRects.size()) {
			Common::Rect dirtyRect = _dirtyOverlayRects.remove_at(_dirtyOverlayRects.size() - 1);

			//printf("Drawing: (%i, %i) -> (%i, %i)\n", dirtyRect.left, dirtyRect.top, dirtyRect.right, dirtyRect.bottom);

			drawDirtyOverlayRect(dirtyRect);
			drawMouseCursorOnRectUpdate(dirtyRect, mouseRect);
			updateHardwareSurfaceForRect(dirtyRect);
		}
	}
}

void OSystem_IPHONE::drawDirtyRect(const Common::Rect& dirtyRect) {
	int h = dirtyRect.bottom - dirtyRect.top;
	int w = dirtyRect.right - dirtyRect.left;

	byte  *src = &_offscreen[dirtyRect.top * _screenWidth + dirtyRect.left];
	uint16 *dst = &_fullscreen[dirtyRect.top * _screenWidth + dirtyRect.left];
	for (int y = h; y > 0; y--) {
		for (int x = w; x > 0; x--)
			*dst++ = _palette[*src++];

		dst += _screenWidth - w;
		src += _screenWidth - w;
	}
}

void OSystem_IPHONE::drawDirtyOverlayRect(const Common::Rect& dirtyRect) {
	int h = dirtyRect.bottom - dirtyRect.top;

	uint16 *src = (uint16 *)&_overlayBuffer[dirtyRect.top * _screenWidth + dirtyRect.left];
	uint16 *dst = &_fullscreen[dirtyRect.top * _screenWidth + dirtyRect.left];
	int x = (dirtyRect.right - dirtyRect.left) * 2;
	for (int y = h; y > 0; y--) {
		memcpy(dst, src, x);
		src += _screenWidth;
		dst += _screenWidth;
	}
}

void OSystem_IPHONE::drawMouseCursorOnRectUpdate(const Common::Rect& updatedRect, const Common::Rect& mouseRect) {
	//draw mouse on top
	if (_mouseVisible && (updatedRect.intersects(mouseRect))) {
		int srcX = 0;
		int srcY = 0;
		int left = _mouseX - _mouseHotspotX;
		if (left < 0) {
			srcX -= left;
			left = 0;
		}
		int top = _mouseY - _mouseHotspotY;
		if (top < 0) {
			srcY -= top;
			top = 0;
		}
			//int right = left + _mouseWidth;
		int bottom = top + _mouseHeight;
		if (bottom > _screenWidth)
			bottom = _screenWidth;
			int displayWidth = _mouseWidth;
		if (_mouseWidth + left > _screenWidth)
			displayWidth = _screenWidth - left;
			int displayHeight = _mouseHeight;
		if (_mouseHeight + top > _screenHeight)
			displayHeight = _screenHeight - top;
		byte *src = &_mouseBuf[srcY * _mouseWidth + srcX];
		uint16 *dst = &_fullscreen[top * _screenWidth + left];
		for (int y = displayHeight; y > srcY; y--) {
			for (int x = displayWidth; x > srcX; x--) {
				if (*src != _mouseKeyColour)
					*dst = _palette[*src];
				dst++;
				src++;
			}
			dst += _screenWidth - displayWidth + srcX;
			src += _mouseWidth - displayWidth + srcX;
		}
	}
}

void OSystem_IPHONE::updateHardwareSurfaceForRect(const Common::Rect& updatedRect) {
	iPhone_updateScreenRect(_fullscreen, updatedRect.left, updatedRect.top, updatedRect.right, updatedRect.bottom );
}

Graphics::Surface *OSystem_IPHONE::lockScreen() {
	//printf("lockScreen()\n");

	_framebuffer.pixels = _offscreen;
	_framebuffer.w = _screenWidth;
	_framebuffer.h = _screenHeight;
	_framebuffer.pitch = _screenWidth;
	_framebuffer.bytesPerPixel = 1;

	return &_framebuffer;
}

void OSystem_IPHONE::unlockScreen() {
	//printf("unlockScreen()\n");
	dirtyFullScreen();
}

void OSystem_IPHONE::setShakePos(int shakeOffset) {
	//printf("setShakePos(%i)\n", shakeOffset);
}

void OSystem_IPHONE::showOverlay() {
	//printf("showOverlay()\n");
	_overlayVisible = true;
	dirtyFullOverlayScreen();
}

void OSystem_IPHONE::hideOverlay() {
	//printf("hideOverlay()\n");
	_overlayVisible = false;
	_dirtyOverlayRects.clear();
	dirtyFullScreen();
}

void OSystem_IPHONE::clearOverlay() {
	//printf("clearOverlay()\n");
	bzero(_overlayBuffer, _screenWidth * _screenHeight * sizeof(OverlayColor));
	dirtyFullOverlayScreen();
}

void OSystem_IPHONE::grabOverlay(OverlayColor *buf, int pitch) {
	//printf("grabOverlay()\n");
	int h = _screenHeight;
	OverlayColor *src = _overlayBuffer;

	do {
		memcpy(buf, src, _screenWidth * sizeof(OverlayColor));
		src += _screenWidth;
		buf += pitch;
	} while (--h);
}

void OSystem_IPHONE::copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {
	//printf("copyRectToOverlay(buf, pitch=%i, x=%i, y=%i, w=%i, h=%i)\n", pitch, x, y, w, h);

	//Clip the coordinates
	if (x < 0) {
		w += x;
		buf -= x;
		x = 0;
	}

	if (y < 0) {
		h += y;
		buf -= y * pitch;
		y = 0;
	}

	if (w > _screenWidth - x)
		w = _screenWidth - x;

	if (h > _screenHeight - y)
		h = _screenHeight - y;

	if (w <= 0 || h <= 0)
		return;

	if (!_fullScreenOverlayIsDirty) {
		_dirtyOverlayRects.push_back(Common::Rect(x, y, x + w, y + h));
	}

	OverlayColor *dst = _overlayBuffer + (y * _screenWidth + x);
	if (_screenWidth == pitch && pitch == w)
		memcpy(dst, buf, h * w * sizeof(OverlayColor));
	else {
		do {
			memcpy(dst, buf, w * sizeof(OverlayColor));
			buf += pitch;
			dst += _screenWidth;
		} while (--h);
	}
}

int16 OSystem_IPHONE::getOverlayHeight() {
	return _screenHeight;
}

int16 OSystem_IPHONE::getOverlayWidth() {
	return _screenWidth;
}

bool OSystem_IPHONE::showMouse(bool visible) {
	bool last = _mouseVisible;
	_mouseVisible = visible;
	_mouseDirty = true;

	return last;
}

void OSystem_IPHONE::warpMouse(int x, int y) {
	//printf("warpMouse()\n");

	_mouseX = x;
	_mouseY = y;
	_mouseDirty = true;
}

void OSystem_IPHONE::dirtyFullScreen() {
	if (!_fullScreenIsDirty) {
		_dirtyRects.clear();
		_dirtyRects.push_back(Common::Rect(0, 0, _screenWidth, _screenHeight));
		_fullScreenIsDirty = true;
	}
}

void OSystem_IPHONE::dirtyFullOverlayScreen() {
	if (!_fullScreenOverlayIsDirty) {
		_dirtyOverlayRects.clear();
		_dirtyOverlayRects.push_back(Common::Rect(0, 0, _screenWidth, _screenHeight));
		_fullScreenOverlayIsDirty = true;
	}
}

void OSystem_IPHONE::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor, int cursorTargetScale, const Graphics::PixelFormat *format) {
	//printf("setMouseCursor(%i, %i)\n", hotspotX, hotspotY);

	if (_mouseBuf != NULL && (_mouseWidth != w || _mouseHeight != h)) {
		free(_mouseBuf);
		_mouseBuf = NULL;
	}

	if (_mouseBuf == NULL)
		_mouseBuf = (byte *)malloc(w * h);

	_mouseWidth = w;
	_mouseHeight = h;

	_mouseHotspotX = hotspotX;
	_mouseHotspotY = hotspotY;

	_mouseKeyColour = keycolor;

	memcpy(_mouseBuf, buf, w * h);

	_mouseDirty = true;
}
