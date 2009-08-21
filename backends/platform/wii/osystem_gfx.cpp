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
 */

#include <malloc.h>

#include "osystem.h"
#include "gx_supp.h"

#define MAX_FPS 30

enum GraphicModeID {
	GM_DEFAULT = 0,
	GM_OVERSCAN1,
	GM_OVERSCAN2,
	GM_OVERSCAN3,
	GM_OVERSCAN4,
	GM_OVERSCAN5
};

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{ "standard", "Standard", GM_DEFAULT },
	{ "overscan1", "Overscan 1", GM_OVERSCAN1 },
	{ "overscan2", "Overscan 2", GM_OVERSCAN2 },
	{ "overscan3", "Overscan 3", GM_OVERSCAN3 },
	{ "overscan4", "Overscan 4", GM_OVERSCAN4 },
	{ "overscan5", "Overscan 5", GM_OVERSCAN5 },
	{ 0, 0, 0 }
};

void OSystem_Wii::initGfx() {
	_surface.w = 0;
	_surface.h = 0;
	_surface.pitch = 0;
	_surface.pixels = NULL;
	_surface.bytesPerPixel = 0;

	GX_InitVideo();

	_overlayWidth = 640;
	_overlayHeight = 480;

#ifndef GAMECUBE
	if (CONF_GetAspectRatio() && _fullscreen)
		_overlayHeight = 400;
#endif

	_overlaySize = _overlayWidth * _overlayHeight * 2;
	_overlayPixels = (OverlayColor *) memalign(32, _overlaySize);

	_palette = (u16 *) memalign(32, 256 * 2);
	memset(_palette, 0, 256 * 2);

	_cursorPalette = (u16 *) memalign(32, 256 * 2);
	memset(_cursorPalette, 0, 256 * 2);

	_texture = (u16 *) memalign(32, 640 * 480 * 2);

	setGraphicsMode(_activeGraphicsMode);
}

void OSystem_Wii::deinitGfx() {
	GX_AbortFrame();

	if (_gamePixels) {
		free(_gamePixels);
		_gamePixels = NULL;
	}

	if (_palette) {
		free(_palette);
		_palette = NULL;
	}

	if (_overlayPixels) {
		free(_overlayPixels);
		_overlayPixels = NULL;
	}

	if (_mouseCursor) {
		free(_mouseCursor);
		_mouseCursor = NULL;
	}

	if (_cursorPalette) {
		free(_cursorPalette);
		_cursorPalette = NULL;
	}

	if (_texture) {
		free(_texture);
		_texture = NULL;
	}
}

const OSystem::GraphicsMode* OSystem_Wii::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

int OSystem_Wii::getDefaultGraphicsMode() const {
	return GM_DEFAULT;
}

bool OSystem_Wii::setGraphicsMode(int mode) {
	s16 xar, yar;

	printf("setGraphicsMode %d\n", mode);

	xar = vmode->viWidth / 2;
	yar = vmode->xfbHeight / 2;

#ifndef GAMECUBE
	if (CONF_GetAspectRatio() && !_fullscreen)
		xar = s16(float(xar) / 1.33f);
#endif

	GX_SetCamPosZ(400 - mode * 10);
	GX_Start(640, 480, xar, yar);

	_activeGraphicsMode = mode;

	return true;
}

int OSystem_Wii::getGraphicsMode() const {
	return _activeGraphicsMode;
}

void OSystem_Wii::initSize(uint width, uint height,
							const Graphics::PixelFormat *format) {
	if (_gameWidth != width || _gameHeight != height) {
		printf("initSize %u %u\n", width, height);

		assert((width <= 640) && (height <= 480));

		_gameWidth = width;
		_gameHeight = height;

		if(_gamePixels)
			free(_gamePixels);

		_gamePixels = (u8 *) memalign(32, _gameWidth * _gameHeight);
		memset(_gamePixels, 0, _gameWidth * _gameHeight);

		if (!_overlayVisible) {
			_currentWidth = _gameWidth;
			_currentHeight = _gameHeight;
			updateEventScreenResolution();
		}

		setGraphicsMode(_activeGraphicsMode);
	}
}

int16 OSystem_Wii::getWidth() {
	return _gameWidth;
}

int16 OSystem_Wii::getHeight() {
	return _gameHeight;
}

void OSystem_Wii::setPalette(const byte *colors, uint start, uint num) {
	const byte *p = colors;
	for (uint i = 0; i < num; ++i) {
		_palette[start + i] = Graphics::RGBToColor<Graphics::ColorMasks<565> >(p[0], p[1], p[2]);
		p += 4;
	}
}

void OSystem_Wii::grabPalette(byte *colors, uint start, uint num) {
	byte *p = colors;
	u8 r, g, b;
	for (uint i = 0; i < num; ++i) {
		Graphics::colorToRGB<Graphics::ColorMasks<565> >(_palette[start + i], r, g, b);
		p[0] = r;
		p[1] = g;
		p[2] = b;
		p[3] = 0xff;
		p += 4;
	}
}

void OSystem_Wii::setCursorPalette(const byte *colors, uint start, uint num) {
	const byte *p = colors;
	for (uint i = 0; i < num; ++i) {
		_cursorPalette[start + i] = Graphics::RGBToColor<Graphics::ColorMasks<565> >(p[0], p[1], p[2]);
		p += 4;
	}

	_cursorPaletteDisabled = false;
}

void OSystem_Wii::disableCursorPalette(bool disable) {
	_cursorPaletteDisabled = disable;
}

void OSystem_Wii::copyRectToScreen(const byte *buf, int pitch, int x, int y,
									int w, int h) {
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

	if (w > _gameWidth - x)
		w = _gameWidth - x;

	if (h > _gameHeight - y)
		h = _gameHeight - y;

	if (w <= 0 || h <= 0)
		return;

	byte *dst = _gamePixels + y * _gameWidth + x;
	if (_gameWidth == pitch && pitch == w) {
		memcpy(dst, buf, h * w);
	} else {
		do {
			memcpy(dst, buf, w);
			buf += pitch;
			dst += _gameWidth;
		} while (--h);
	}
}

void OSystem_Wii::updateScreen() {
	static u32 x, y, h, skip;
	static s16 msx, msy, mox, moy, mskip;
	static u16 mpx, mpy;
	static u8 *s;
	static u16 *d, *p;

	u32 now = getMillis();
	if (now - _lastScreenUpdate < 1000 / MAX_FPS)
		return;

#ifdef DEBUG_WII_MEMSTATS
	wii_memstats();
#endif

	_lastScreenUpdate = now;

	h = 0;
	if (_overlayVisible) {
		memcpy(_texture, _overlayPixels, _overlaySize);
	} else {
		for (y = 0; y < _gameHeight; ++y) {
			for (x = 0; x < _gameWidth; ++x)
				_texture[h + x] = _palette[_gamePixels[h + x]];

			h += _gameWidth;
		}
	}

	if (_mouseVisible) {
		msx = _mouseX - _mouseHotspotX;
		msy = _mouseY - _mouseHotspotY;
		mox = 0;
		moy = 0;
		mpx = _mouseWidth;
		mpy = _mouseHeight;

		if (msx < 0) {
			mox = -msx;
			mpx -= mox;
			msx = 0;
		} else
			if (msx + mpx > _currentWidth - 1)
				mpx = _currentWidth - msx - 1;

		if (msy < 0) {
			moy = -msy;
			mpy -= moy;
			msy = 0;
		} else
			if (msy + mpy + 1 > _currentHeight - 1)
				mpy = _currentHeight - msy - 1;


		if (_cursorPaletteDisabled)
			p = _palette;
		else
			p = _cursorPalette;

		skip = _currentWidth - mpx;
		mskip = _mouseWidth - mpx;

		s = _mouseCursor + moy * _mouseWidth + mox;
		d = _texture + (msy * _currentWidth + msx);

		for (y = 0; y < mpy; ++y) {
			for (x = 0; x < mpx; ++x) {
				if (*s == _mouseKeyColor) {
					s++;
					d++;

					continue;
				}

				*d++ = p[*s];
				s++;
			}

			d += skip;
			s += mskip;
		}
	}

	GX_Render(_currentWidth, _currentHeight, (u8 *) _texture,
				_currentWidth * 2);
}

Graphics::Surface *OSystem_Wii::lockScreen() {
	_surface.pixels = _gamePixels;
	_surface.w = _gameWidth;
	_surface.h = _gameHeight;
	_surface.pitch = _gameWidth;
	_surface.bytesPerPixel = 1;

	return &_surface;
}

void OSystem_Wii::unlockScreen() {
}

void OSystem_Wii::setShakePos(int shakeOffset) {
	GX_SetTexTrans(0, (float) -shakeOffset * ((float) vmode->efbHeight /
												(float) _currentHeight));
}

void OSystem_Wii::showOverlay() {
	_mouseX = _overlayWidth / 2;
	_mouseY = _overlayHeight / 2;
	_overlayVisible = true;
	_currentWidth = _overlayWidth;
	_currentHeight = _overlayHeight;

	updateEventScreenResolution();
}

void OSystem_Wii::hideOverlay() {
	_mouseX = _gameWidth / 2;
	_mouseY = _gameHeight / 2;
	_overlayVisible = false;
	_currentWidth = _gameWidth;
	_currentHeight = _gameHeight;

	updateEventScreenResolution();
}

void OSystem_Wii::clearOverlay() {
	memset(_overlayPixels, 0, _overlaySize);
}

void OSystem_Wii::grabOverlay(OverlayColor *buf, int pitch) {
	int h = _overlayHeight;
	OverlayColor *src = _overlayPixels;

	do {
		memcpy(buf, src, _overlayWidth * sizeof(OverlayColor));
		src += _overlayWidth;
		buf += pitch;
	} while (--h);
}

void OSystem_Wii::copyRectToOverlay(const OverlayColor *buf, int pitch, int x,
									int y, int w, int h) {
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

	if (w > _overlayWidth - x)
		w = _overlayWidth - x;

	if (h > _overlayHeight - y)
		h = _overlayHeight - y;

	if (w <= 0 || h <= 0)
		return;

	OverlayColor *dst = _overlayPixels + (y * _overlayWidth + x);
	if (_overlayWidth == pitch && pitch == w) {
		memcpy(dst, buf, h * w * sizeof(OverlayColor));
	} else {
		do {
			memcpy(dst, buf, w * sizeof(OverlayColor));
			buf += pitch;
			dst += _overlayWidth;
		} while (--h);
	}
}

int16 OSystem_Wii::getOverlayWidth() {
	return _overlayWidth;
}

int16 OSystem_Wii::getOverlayHeight() {
	return _overlayHeight;
}

Graphics::PixelFormat OSystem_Wii::getOverlayFormat() const {
	return Graphics::createPixelFormat<565>();
}

bool OSystem_Wii::showMouse(bool visible) {
	bool last = _mouseVisible;
	_mouseVisible = visible;

	return last;
}

void OSystem_Wii::warpMouse(int x, int y) {
	_mouseX = x;
	_mouseY = y;
}

void OSystem_Wii::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX,
									int hotspotY, uint32 keycolor,
									int cursorTargetScale,
									const Graphics::PixelFormat *format) {
	(void) cursorTargetScale; // TODO

	_mouseWidth = w;
	_mouseHeight = h;
	_mouseHotspotX = hotspotX;
	_mouseHotspotY = hotspotY;
	_mouseKeyColor = keycolor & 0xff;

	if (_mouseCursor)
		free(_mouseCursor);

	_mouseCursor = (u8 *) memalign(32, w * h);
	memcpy(_mouseCursor, buf, w * h);
}

