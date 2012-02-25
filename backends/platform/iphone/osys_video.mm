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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "osys_main.h"

#include "iphone_video.h"

void OSystem_IPHONE::initVideoContext() {
	_videoContext = [g_iPhoneViewInstance getVideoContext];
}

const OSystem::GraphicsMode *OSystem_IPHONE::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

int OSystem_IPHONE::getDefaultGraphicsMode() const {
	return kGraphicsModeLinear;
}

bool OSystem_IPHONE::setGraphicsMode(int mode) {
	switch (mode) {
	case kGraphicsModeNone:
	case kGraphicsModeLinear:
		_videoContext->graphicsMode = (GraphicsModes)mode;
		return true;

	default:
		return false;
	}
}

int OSystem_IPHONE::getGraphicsMode() const {
	return _videoContext->graphicsMode;
}

void OSystem_IPHONE::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	//printf("initSize(%i, %i)\n", width, height);

	_videoContext->screenWidth = width;
	_videoContext->screenHeight = height;
	_videoContext->shakeOffsetY = 0;

	free(_gameScreenRaw);

	_gameScreenRaw = (byte *)malloc(width * height);
	bzero(_gameScreenRaw, width * height);

	_fullScreenIsDirty = false;
	dirtyFullScreen();
	_mouseCursorPaletteEnabled = false;
}

void OSystem_IPHONE::beginGFXTransaction() {
}

OSystem::TransactionError OSystem_IPHONE::endGFXTransaction() {
	_screenChangeCount++;
	updateOutputSurface();
	[g_iPhoneViewInstance performSelectorOnMainThread:@selector(setGraphicsMode) withObject:nil waitUntilDone: YES];

	// TODO: Can we return better error codes?
	return kTransactionSuccess;
}

void OSystem_IPHONE::updateOutputSurface() {
	[g_iPhoneViewInstance performSelectorOnMainThread:@selector(initSurface) withObject:nil waitUntilDone: YES];
}

int16 OSystem_IPHONE::getHeight() {
	return _videoContext->screenHeight;
}

int16 OSystem_IPHONE::getWidth() {
	return _videoContext->screenWidth;
}

void OSystem_IPHONE::setPalette(const byte *colors, uint start, uint num) {
	assert(start + num <= 256);
	const byte *b = colors;

	for (uint i = start; i < start + num; ++i) {
		_gamePalette[i] = Graphics::RGBToColor<Graphics::ColorMasks<565> >(b[0], b[1], b[2]);
		_gamePaletteRGBA5551[i] = Graphics::RGBToColor<Graphics::ColorMasks<5551> >(b[0], b[1], b[2]);
		b += 3;
	}

	dirtyFullScreen();
}

void OSystem_IPHONE::grabPalette(byte *colors, uint start, uint num) {
	assert(start + num <= 256);
	byte *b = colors;

	for (uint i = start; i < start + num; ++i) {
		Graphics::colorToRGB<Graphics::ColorMasks<565> >(_gamePalette[i], b[0], b[1], b[2]);
		b += 3;
	}
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

	if (w > (int)_videoContext->screenWidth - x) {
		w = _videoContext->screenWidth - x;
	}

	if (h > (int)_videoContext->screenHeight - y) {
		h = _videoContext->screenHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;

	if (!_fullScreenIsDirty) {
		_dirtyRects.push_back(Common::Rect(x, y, x + w, y + h));
	}


	byte *dst = _gameScreenRaw + y * _videoContext->screenWidth + x;
	if ((int)_videoContext->screenWidth == pitch && pitch == w)
		memcpy(dst, buf, h * w);
	else {
		do {
			memcpy(dst, buf, w);
			buf += pitch;
			dst += _videoContext->screenWidth;
		} while (--h);
	}
}

void OSystem_IPHONE::updateScreen() {
	//printf("updateScreen(): %i dirty rects.\n", _dirtyRects.size());

	if (_dirtyRects.size() == 0 && _dirtyOverlayRects.size() == 0 && !_mouseDirty)
		return;

	internUpdateScreen();
	_mouseDirty = false;
	_fullScreenIsDirty = false;
	_fullScreenOverlayIsDirty = false;

	iPhone_updateScreen();
}

void OSystem_IPHONE::internUpdateScreen() {
	if (_mouseNeedTextureUpdate) {
		updateMouseTexture();
		_mouseNeedTextureUpdate = false;
	}

	while (_dirtyRects.size()) {
		Common::Rect dirtyRect = _dirtyRects.remove_at(_dirtyRects.size() - 1);

		//printf("Drawing: (%i, %i) -> (%i, %i)\n", dirtyRect.left, dirtyRect.top, dirtyRect.right, dirtyRect.bottom);
		drawDirtyRect(dirtyRect);
		// TODO: Implement dirty rect code
		//updateHardwareSurfaceForRect(dirtyRect);
	}

	if (_videoContext->overlayVisible) {
		// TODO: Implement dirty rect code
		_dirtyOverlayRects.clear();
		/*while (_dirtyOverlayRects.size()) {
			Common::Rect dirtyRect = _dirtyOverlayRects.remove_at(_dirtyOverlayRects.size() - 1);

			//printf("Drawing: (%i, %i) -> (%i, %i)\n", dirtyRect.left, dirtyRect.top, dirtyRect.right, dirtyRect.bottom);
			drawDirtyOverlayRect(dirtyRect);
		}*/
	}
}

void OSystem_IPHONE::drawDirtyRect(const Common::Rect &dirtyRect) {
	int h = dirtyRect.bottom - dirtyRect.top;
	int w = dirtyRect.right - dirtyRect.left;

	byte *src = &_gameScreenRaw[dirtyRect.top * _videoContext->screenWidth + dirtyRect.left];
	byte *dstRaw = (byte *)_videoContext->screenTexture.getBasePtr(dirtyRect.left, dirtyRect.top);
	for (int y = h; y > 0; y--) {
		uint16 *dst = (uint16 *)dstRaw;
		for (int x = w; x > 0; x--)
			*dst++ = _gamePalette[*src++];

		dstRaw += _videoContext->screenTexture.pitch;
		src += _videoContext->screenWidth - w;
	}
}

Graphics::Surface *OSystem_IPHONE::lockScreen() {
	//printf("lockScreen()\n");

	_framebuffer.pixels = _gameScreenRaw;
	_framebuffer.w = _videoContext->screenWidth;
	_framebuffer.h = _videoContext->screenHeight;
	_framebuffer.pitch = _videoContext->screenWidth;
	_framebuffer.format = Graphics::PixelFormat::createFormatCLUT8();

	return &_framebuffer;
}

void OSystem_IPHONE::unlockScreen() {
	//printf("unlockScreen()\n");
	dirtyFullScreen();
}

void OSystem_IPHONE::setShakePos(int shakeOffset) {
	//printf("setShakePos(%i)\n", shakeOffset);
	_videoContext->shakeOffsetY = shakeOffset;
	[g_iPhoneViewInstance performSelectorOnMainThread:@selector(setViewTransformation) withObject:nil waitUntilDone: YES];
	// HACK: We use this to force a redraw.
	_mouseDirty = true;
}

void OSystem_IPHONE::showOverlay() {
	//printf("showOverlay()\n");
	_videoContext->overlayVisible = true;
	dirtyFullOverlayScreen();
	updateScreen();
	[g_iPhoneViewInstance performSelectorOnMainThread:@selector(updateMouseCursorScaling) withObject:nil waitUntilDone: YES];
	[g_iPhoneViewInstance performSelectorOnMainThread:@selector(clearColorBuffer) withObject:nil waitUntilDone: YES];
}

void OSystem_IPHONE::hideOverlay() {
	//printf("hideOverlay()\n");
	_videoContext->overlayVisible = false;
	_dirtyOverlayRects.clear();
	dirtyFullScreen();
	[g_iPhoneViewInstance performSelectorOnMainThread:@selector(updateMouseCursorScaling) withObject:nil waitUntilDone: YES];
	[g_iPhoneViewInstance performSelectorOnMainThread:@selector(clearColorBuffer) withObject:nil waitUntilDone: YES];
}

void OSystem_IPHONE::clearOverlay() {
	//printf("clearOverlay()\n");
	bzero(_videoContext->overlayTexture.getBasePtr(0, 0), _videoContext->overlayTexture.h * _videoContext->overlayTexture.pitch);
	dirtyFullOverlayScreen();
}

void OSystem_IPHONE::grabOverlay(OverlayColor *buf, int pitch) {
	//printf("grabOverlay()\n");
	int h = _videoContext->overlayHeight;

	const byte *src = (const byte *)_videoContext->overlayTexture.getBasePtr(0, 0);
	do {
		memcpy(buf, src, _videoContext->overlayWidth * sizeof(OverlayColor));
		src += _videoContext->overlayTexture.pitch;
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

	if (w > (int)_videoContext->overlayWidth - x)
		w = _videoContext->overlayWidth - x;

	if (h > (int)_videoContext->overlayHeight - y)
		h = _videoContext->overlayHeight - y;

	if (w <= 0 || h <= 0)
		return;

	if (!_fullScreenOverlayIsDirty) {
		_dirtyOverlayRects.push_back(Common::Rect(x, y, x + w, y + h));
	}

	byte *dst = (byte *)_videoContext->overlayTexture.getBasePtr(x, y);
	do { 
		memcpy(dst, buf, w * sizeof(OverlayColor));
		buf += pitch;
		dst += _videoContext->overlayTexture.pitch;
	} while (--h);
}

int16 OSystem_IPHONE::getOverlayHeight() {
	return _videoContext->overlayHeight;
}

int16 OSystem_IPHONE::getOverlayWidth() {
	return _videoContext->overlayWidth;
}

bool OSystem_IPHONE::showMouse(bool visible) {
	bool last = _videoContext->mouseIsVisible;
	_videoContext->mouseIsVisible = visible;
	_mouseDirty = true;

	return last;
}

void OSystem_IPHONE::warpMouse(int x, int y) {
	//printf("warpMouse()\n");
	_videoContext->mouseX = x;
	_videoContext->mouseY = y;
	[g_iPhoneViewInstance performSelectorOnMainThread:@selector(notifyMouseMove) withObject:nil waitUntilDone: YES];
	_mouseDirty = true;
}

void OSystem_IPHONE::dirtyFullScreen() {
	if (!_fullScreenIsDirty) {
		_dirtyRects.clear();
		_dirtyRects.push_back(Common::Rect(0, 0, _videoContext->screenWidth, _videoContext->screenHeight));
		_fullScreenIsDirty = true;
	}
}

void OSystem_IPHONE::dirtyFullOverlayScreen() {
	if (!_fullScreenOverlayIsDirty) {
		_dirtyOverlayRects.clear();
		_dirtyOverlayRects.push_back(Common::Rect(0, 0, _videoContext->overlayWidth, _videoContext->overlayHeight));
		_fullScreenOverlayIsDirty = true;
	}
}

void OSystem_IPHONE::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, int cursorTargetScale, const Graphics::PixelFormat *format) {
	//printf("setMouseCursor(%i, %i, scale %u)\n", hotspotX, hotspotY, cursorTargetScale);

	if (_mouseBuf != NULL && (_videoContext->mouseWidth != w || _videoContext->mouseHeight != h)) {
		free(_mouseBuf);
		_mouseBuf = NULL;
	}

	if (_mouseBuf == NULL)
		_mouseBuf = (byte *)malloc(w * h);

	_videoContext->mouseWidth = w;
	_videoContext->mouseHeight = h;

	_videoContext->mouseHotspotX = hotspotX;
	_videoContext->mouseHotspotY = hotspotY;

	_mouseKeyColor = (byte)keycolor;

	memcpy(_mouseBuf, buf, w * h);

	_mouseDirty = true;
	_mouseNeedTextureUpdate = true;
}

void OSystem_IPHONE::setCursorPalette(const byte *colors, uint start, uint num) {
	assert(start + num <= 256);

	for (uint i = start; i < start + num; ++i, colors += 3)
		_mouseCursorPalette[i] = Graphics::RGBToColor<Graphics::ColorMasks<5551> >(colors[0], colors[1], colors[2]);
	
	// FIXME: This is just stupid, our client code seems to assume that this
	// automatically enables the cursor palette.
	_mouseCursorPaletteEnabled = true;

	if (_mouseCursorPaletteEnabled)
		_mouseDirty = _mouseNeedTextureUpdate = true;
}

void OSystem_IPHONE::updateMouseTexture() {
	uint texWidth = getSizeNextPOT(_videoContext->mouseWidth);
	uint texHeight = getSizeNextPOT(_videoContext->mouseHeight);

	Graphics::Surface &mouseTexture = _videoContext->mouseTexture;
	if (mouseTexture.w != texWidth || mouseTexture.h != texHeight)
		mouseTexture.create(texWidth, texHeight, Graphics::createPixelFormat<5551>());

	const uint16 *palette;
	if (_mouseCursorPaletteEnabled)
		palette = _mouseCursorPalette;
	else
		palette = _gamePaletteRGBA5551;

	uint16 *mouseBuf = (uint16 *)mouseTexture.getBasePtr(0, 0);
	for (uint x = 0; x < _videoContext->mouseWidth; ++x) {
		for (uint y = 0; y < _videoContext->mouseHeight; ++y) {
			const byte color = _mouseBuf[y * _videoContext->mouseWidth + x];
			if (color != _mouseKeyColor)
				mouseBuf[y * texWidth + x] = palette[color] | 0x1;
			else
				mouseBuf[y * texWidth + x] = 0x0;
		}
	}

	[g_iPhoneViewInstance performSelectorOnMainThread:@selector(updateMouseCursor) withObject:nil waitUntilDone: YES];
}
