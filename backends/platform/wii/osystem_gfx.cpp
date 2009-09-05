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

#include "common/config-manager.h"
#include "graphics/conversion.h"
#include "backends/fs/wii/wii-fs-factory.h"

#include "osystem.h"
#include "gfx.h"

#define ROUNDUP(x,n) (-(-(x) & -(n)))
#define MAX_FPS 30
#define TLUT_GAME GX_TLUT0
#define TLUT_MOUSE GX_TLUT1

static const OSystem::GraphicsMode _supportedGraphicsModes[] = {
	{
		"default",
		"Default",
		OSystem_Wii::gmStandard
	},
	{
		"defaultbilinear",
		"Default, bilinear filtering",
		OSystem_Wii::gmStandardFiltered
	},
	{
		"ds",
		"Double-strike",
		OSystem_Wii::gmDoubleStrike
	},
	{
		"dsbilinear",
		"Double-strike, bilinear filtering",
		OSystem_Wii::gmDoubleStrikeFiltered
	},
	{ 0, 0, 0 }
};

void OSystem_Wii::initGfx() {
	gfx_video_init(GFX_STANDARD_AUTO, GFX_MODE_DEFAULT);
	gfx_init();
	gfx_set_underscan(ConfMan.getInt("wii_video_default_underscan_x"),
						ConfMan.getInt("wii_video_default_underscan_y"));

	_overlayWidth = gfx_video_get_width();
	_overlayHeight = gfx_video_get_height();

#ifndef GAMECUBE
	if (CONF_GetAspectRatio() && _fullscreen)
		_overlayHeight = 400;
#endif

	_overlaySize = _overlayWidth * _overlayHeight * 2;
	_overlayPixels = (OverlayColor *) memalign(32, _overlaySize);

	memset(&_texMouse, 0, sizeof(gfx_tex_t));
	memset(&_texOverlay, 0, sizeof(gfx_tex_t));
	memset(&_texGame, 0, sizeof(gfx_tex_t));

	_cursorPalette = (u16 *) malloc(256 * 2);
	if (!_cursorPalette) {
		printf("could not alloc palette buffer\n");
		::abort();
	}

	memset(_cursorPalette, 0, 256 * 2);

	if (!gfx_tex_init(&_texOverlay, GFX_TF_RGB5A3, 0,
						_overlayWidth, _overlayHeight)) {
		printf("could not init the overlay texture\n");
		::abort();
	}

	gfx_coords(&_coordsOverlay, &_texOverlay, GFX_COORD_FULLSCREEN);
}

void OSystem_Wii::deinitGfx() {
	gfx_deinit();

	gfx_tex_deinit(&_texMouse);
	gfx_tex_deinit(&_texGame);
	gfx_tex_deinit(&_texOverlay);

	free(_cursorPalette);
	_cursorPalette = NULL;

	free(_gamePixels);
	_gamePixels = NULL;

	free(_overlayPixels);
	_overlayPixels = NULL;
}

void OSystem_Wii::updateScreenResolution() {
	if (_overlayVisible) {
		_currentWidth = _overlayWidth;
		_currentHeight = _overlayHeight;
	} else {
		_currentWidth = _gameWidth;
		_currentHeight = _gameHeight;
	}

	if (_currentWidth > 0)
		_currentXScale = f32(gfx_video_get_width()) / f32(_currentWidth);
	else
		_currentXScale = 1.0;

	if (_currentHeight > 0)
		_currentYScale = f32(gfx_video_get_height()) / f32(_currentHeight);
	else
		_currentYScale = 1.0;

	updateEventScreenResolution();
}

void OSystem_Wii::switchVideoMode(int mode) {
	static const struct {
		gfx_video_mode_t mode;
		bool filter;
	} map[] = {
		{ GFX_MODE_DEFAULT, false },
		{ GFX_MODE_DEFAULT, true },
		{ GFX_MODE_DS, false },
		{ GFX_MODE_DS, true }
	};

	if (_gameHeight > 240) {
		if (mode == gmDoubleStrike)
			mode = gmStandard;
		else if (mode == gmDoubleStrikeFiltered)
			mode = gmStandardFiltered;
	}

	printf("switchVideoMode %d\n", mode);

	if (map[_actualGraphicsMode].mode != map[mode].mode) {
		gfx_video_deinit();
		gfx_video_init(GFX_STANDARD_AUTO, map[mode].mode);
		gfx_init();
	}

	_actualGraphicsMode = mode;

	_bilinearFilter = map[mode].filter;
	gfx_tex_set_bilinear_filter(&_texGame, _bilinearFilter);
	gfx_tex_set_bilinear_filter(&_texMouse, _bilinearFilter);

	u16 usx, usy;
	if (map[mode].mode == GFX_MODE_DS) {
		usx = ConfMan.getInt("wii_video_ds_underscan_x",
								Common::ConfigManager::kApplicationDomain);
		usy = ConfMan.getInt("wii_video_ds_underscan_y",
								Common::ConfigManager::kApplicationDomain);
	} else {
		usx = ConfMan.getInt("wii_video_default_underscan_x",
								Common::ConfigManager::kApplicationDomain);
		usy = ConfMan.getInt("wii_video_default_underscan_y",
								Common::ConfigManager::kApplicationDomain);
	}

	gfx_set_underscan(usx, usy);
	gfx_coords(&_coordsOverlay, &_texOverlay, GFX_COORD_FULLSCREEN);
	gfx_coords(&_coordsGame, &_texGame, GFX_COORD_FULLSCREEN);
	updateScreenResolution();
}

const OSystem::GraphicsMode* OSystem_Wii::getSupportedGraphicsModes() const {
	return _supportedGraphicsModes;
}

int OSystem_Wii::getDefaultGraphicsMode() const {
	return gmStandard;
}

bool OSystem_Wii::setGraphicsMode(int mode) {
	_configGraphicsMode = mode;
	return true;
}

int OSystem_Wii::getGraphicsMode() const {
	return _configGraphicsMode;
}

#ifdef USE_RGB_COLOR
Graphics::PixelFormat OSystem_Wii::getScreenFormat() const {
	return _pfGame;
}

Common::List<Graphics::PixelFormat> OSystem_Wii::getSupportedFormats() {
	Common::List<Graphics::PixelFormat> res;
	res.push_back(_pfRGB565);
	res.push_back(Graphics::PixelFormat::createFormatCLUT8());

	return res;
}
#endif

void OSystem_Wii::initSize(uint width, uint height,
							const Graphics::PixelFormat *format) {
	bool update = false;
	gfx_tex_format_t tex_format;

#ifdef USE_RGB_COLOR
	Graphics::PixelFormat newFormat;

	if (format)
		newFormat = *format;
	else
		newFormat = Graphics::PixelFormat::createFormatCLUT8();

	if (newFormat.bytesPerPixel > 2)
		newFormat = Graphics::PixelFormat::createFormatCLUT8();

	if (_pfGame != newFormat) {
		_pfGame = newFormat;
		update = true;
	}
#endif

	if (_gameWidth != width || _gameHeight != height) {
		assert((width <= 640) && (height <= 480));

		_gameWidth = width;
		_gameHeight = height;
		update = true;
	}

	if (_gameRunning) {
		switchVideoMode(_configGraphicsMode);

		if (_arCorrection && (_gameWidth == 320) && (_gameHeight == 200))
			gfx_set_ar(320.0 / 240.0);
		else
			gfx_set_ar(f32(_gameWidth) / f32(_gameHeight));
	}

	if (update) {
		if(_gamePixels)
			free(_gamePixels);

		tex_format = GFX_TF_PALETTE_RGB565;

#ifdef USE_RGB_COLOR
		if (_pfGame.bytesPerPixel > 1) {
			tex_format = GFX_TF_RGB565;
			_pfGameTexture = _pfRGB565;
		}

		printf("initSize %u*%u*%u (%u%u%u -> %u%u%u match: %d)\n",
				_gameWidth, _gameHeight, _pfGame.bytesPerPixel * 8,
				8 - _pfGame.rLoss, 8 - _pfGame.gLoss, 8 - _pfGame.bLoss,
				8 - _pfGameTexture.rLoss, 8 - _pfGameTexture.gLoss,
				8 - _pfGameTexture.bLoss, _pfGame == _pfGameTexture);

		_gamePixels = (u8 *) memalign(32, _gameWidth * _gameHeight *
										_pfGame.bytesPerPixel);
		memset(_gamePixels, 0, _gameWidth * _gameHeight *
				_pfGame.bytesPerPixel);
#else
		printf("initSize %u*%u\n", _gameWidth, _gameHeight);

		_gamePixels = (u8 *) memalign(32, _gameWidth * _gameHeight);
		memset(_gamePixels, 0, _gameWidth * _gameHeight);
#endif

		if (!gfx_tex_init(&_texGame, tex_format, TLUT_GAME,
					_gameWidth, _gameHeight)) {
			printf("could not init the game texture\n");
			::abort();
		}

		gfx_tex_set_bilinear_filter(&_texGame, _bilinearFilter);
		gfx_coords(&_coordsGame, &_texGame, GFX_COORD_FULLSCREEN);

		updateScreenResolution();
	}
}

int16 OSystem_Wii::getWidth() {
	return _gameWidth;
}

int16 OSystem_Wii::getHeight() {
	return _gameHeight;
}

void OSystem_Wii::setPalette(const byte *colors, uint start, uint num) {
#ifdef USE_RGB_COLOR
	assert(_pfGame.bytesPerPixel == 1);
#endif

	const byte *s = colors;
	u16 *d = _texGame.palette;

	for (uint i = 0; i < num; ++i) {
		d[start + i] = Graphics::RGBToColor<Graphics::ColorMasks<565> >(s[0], s[1], s[2]);
		s += 4;
	}

	gfx_tex_flush_palette(&_texGame);

	if (_cursorPaletteDisabled) {
		assert(_texMouse.palette);

		s = colors;
		d = _texMouse.palette;

		for (uint i = 0; i < num; ++i) {
			d[start + i] = Graphics::ARGBToColor<Graphics::ColorMasks<3444> >(0xff, s[0], s[1], s[2]);
			s += 4;
		}

		_cursorPaletteDirty = true;
	}
}

void OSystem_Wii::grabPalette(byte *colors, uint start, uint num) {
#ifdef USE_RGB_COLOR
	assert(_pfGame.bytesPerPixel == 1);
#endif

	u16 *s = _texGame.palette;
	byte *d = colors;

	u8 r, g, b;
	for (uint i = 0; i < num; ++i) {
		Graphics::colorToRGB<Graphics::ColorMasks<565> >(s[start + i], r, g, b);
		d[0] = r;
		d[1] = g;
		d[2] = b;
		d[3] = 0xff;
		d += 4;
	}
}

void OSystem_Wii::setCursorPalette(const byte *colors, uint start, uint num) {
	if (!_texMouse.palette) {
		printf("switching to palette based cursor\n");

		if (!gfx_tex_init(&_texMouse, GFX_TF_PALETTE_RGB5A3, TLUT_MOUSE,
							16, 16)) {
			printf("could not init the mouse texture\n");
			::abort();
		}
	
		gfx_tex_set_bilinear_filter(&_texMouse, _bilinearFilter);
	}

	if (_cursorPaletteDisabled) {
		memcpy(_cursorPalette, _texMouse.palette, 256 * 2);
		_cursorPaletteDisabled = false;
	}

	const byte *s = colors;
	u16 *d = _texMouse.palette;

	for (uint i = 0; i < num; ++i) {
		d[start + i] = Graphics::ARGBToColor<Graphics::ColorMasks<3444> >(0xff, s[0], s[1], s[2]);
		s += 4;
	}

	_cursorPaletteDirty = true;
}

void OSystem_Wii::disableCursorPalette(bool disable) {
	_cursorPaletteDisabled = disable;

	if (_texMouse.palette && disable) {
		memcpy(_texMouse.palette, _cursorPalette, 256 * 2);
		_cursorPaletteDirty = true;
	}
}

void OSystem_Wii::copyRectToScreen(const byte *buf, int pitch, int x, int y,
									int w, int h) {
	assert(x >= 0 && x < _gameWidth);
	assert(y >= 0 && y < _gameHeight);
	assert(w > 0 && x + w <= _gameWidth);
	assert(h > 0 && y + h <= _gameHeight);

#ifdef USE_RGB_COLOR
	if (_pfGame.bytesPerPixel > 1) {
		if (!Graphics::crossBlit(_gamePixels +
									y * _gameWidth * _pfGame.bytesPerPixel +
									x * _pfGame.bytesPerPixel,
									buf, _gameWidth * _pfGame.bytesPerPixel,
									pitch, w, h, _pfGameTexture, _pfGame)) {
			printf("crossBlit failed\n");
			::abort();
		}
	} else {
#endif
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
#ifdef USE_RGB_COLOR
	}
#endif

	_gameDirty = true;
}

void OSystem_Wii::updateScreen() {
	static gfx_coords_t cc;
	static int cs;

	u32 now = getMillis();
	if (now - _lastScreenUpdate < 1000 / MAX_FPS)
		return;

#ifdef DEBUG_WII_MEMSTATS
	wii_memstats();
#endif

	cs = _cursorScale;
	_lastScreenUpdate = now;

	gfx_frame_start();


	if (_gameRunning) {
		if (_gameDirty) {
			gfx_tex_convert(&_texGame, _gamePixels);
			_gameDirty = false;
		}

		gfx_draw_tex(&_texGame, &_coordsGame);
	}

	if (_overlayVisible) {
		// ugly, but the modern theme sets a factor of 3, only god knows why
		if (cs > 2)
			cs = 1;
		else
			cs *= 2;

		if (_overlayDirty) {
			gfx_tex_convert(&_texOverlay, _overlayPixels);
			_overlayDirty = false;
		}

		gfx_draw_tex(&_texOverlay, &_coordsOverlay);
	}

	if (_mouseVisible) {
		cc.x = f32(_mouseX - cs * _mouseHotspotX) * _currentXScale;
		cc.y = f32(_mouseY - cs * _mouseHotspotY) * _currentYScale;
		cc.w = f32(_texMouse.width) * _currentXScale * cs;
		cc.h = f32(_texMouse.height) * _currentYScale * cs;

		if (_texMouse.palette && _cursorPaletteDirty) {
			_texMouse.palette[_mouseKeyColor] = 0;
			gfx_tex_flush_palette(&_texMouse);
			_cursorPaletteDirty = false;
		}

		gfx_draw_tex(&_texMouse, &cc);
	}

	gfx_frame_end();
}

Graphics::Surface *OSystem_Wii::lockScreen() {
	_surface.pixels = _gamePixels;
	_surface.w = _gameWidth;
	_surface.h = _gameHeight;
	_surface.pitch = _gameWidth;
#ifdef USE_RGB_COLOR
	_surface.bytesPerPixel = _pfGame.bytesPerPixel;
#else
	_surface.bytesPerPixel = 1;
#endif

	return &_surface;
}

void OSystem_Wii::unlockScreen() {
	_gameDirty = true;
}

void OSystem_Wii::setShakePos(int shakeOffset) {
	gfx_coords(&_coordsGame, &_texGame, GFX_COORD_FULLSCREEN);
	_coordsGame.y -= f32(shakeOffset) * _currentYScale;
}

void OSystem_Wii::showOverlay() {
	_mouseX = _overlayWidth / 2;
	_mouseY = _overlayHeight / 2;
	_overlayVisible = true;
	updateScreenResolution();
	gfx_tex_set_bilinear_filter(&_texMouse, true);
}

void OSystem_Wii::hideOverlay() {
	_mouseX = _gameWidth / 2;
	_mouseY = _gameHeight / 2;
	_overlayVisible = false;
	updateScreenResolution();
	gfx_tex_set_bilinear_filter(&_texMouse, _bilinearFilter);
}

void OSystem_Wii::clearOverlay() {
	memset(_overlayPixels, 0, _overlaySize);
	_overlayDirty = true;
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

	_overlayDirty = true;
}

int16 OSystem_Wii::getOverlayWidth() {
	return _overlayWidth;
}

int16 OSystem_Wii::getOverlayHeight() {
	return _overlayHeight;
}

Graphics::PixelFormat OSystem_Wii::getOverlayFormat() const {
	return Graphics::createPixelFormat<3444>();
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
	gfx_tex_format_t tex_format = GFX_TF_PALETTE_RGB5A3;
	uint tw, th;
	bool tmpBuf = false;
	uint32 oldKeycolor = _mouseKeyColor;

#ifdef USE_RGB_COLOR
	if (!format)
		_pfCursor = Graphics::PixelFormat::createFormatCLUT8();
	else
		_pfCursor = *format;

	if (_pfCursor.bytesPerPixel > 1) {
		tex_format = GFX_TF_RGB5A3;
		_mouseKeyColor = keycolor & 0xffff;
		tw = ROUNDUP(w, 4);
		th = ROUNDUP(h, 4);

		if (_pfCursor != _pfRGB3444)
			tmpBuf = true;
	} else {
#endif
		_mouseKeyColor = keycolor & 0xff;
		tw = ROUNDUP(w, 8);
		th = ROUNDUP(h, 4);
#ifdef USE_RGB_COLOR
	}
#endif

	if (!gfx_tex_init(&_texMouse, tex_format, TLUT_MOUSE, tw, th)) {
		printf("could not init the mouse texture\n");
		::abort();
	}

	gfx_tex_set_bilinear_filter(&_texMouse, _bilinearFilter);

	if ((tw != w) || (th != h))
		tmpBuf = true;

	if (!tmpBuf) {
		gfx_tex_convert(&_texMouse, buf);
	} else {
		u8 bpp = _texMouse.bpp;
		byte *tmp = (byte *) malloc(tw * th * bpp);

		if (!tmp) {
			printf("could not alloc temp cursor buffer\n");
			::abort();
		}

		if (bpp > 1)
			memset(tmp, 0, tw * th * bpp);
		else
			memset(tmp, _mouseKeyColor, tw * th);

#ifdef USE_RGB_COLOR
		if (bpp > 1) {
			if (!Graphics::crossBlit(tmp, buf,
										tw * _pfRGB3444.bytesPerPixel,
										w * _pfCursor.bytesPerPixel,
										tw, th, _pfRGB3444, _pfCursor)) {
				printf("crossBlit failed\n");
				::abort();
			}

			// nasty, shouldn't the frontend set the alpha channel?
			u16 *s = (u16 *) buf;
			u16 *d = (u16 *) tmp;
			for (u16 y = 0; y < h; ++y) {
				for (u16 x = 0; x < w; ++x) {
					if (*s++ != _mouseKeyColor)
						*d++ |= 7 << 12;
					else
						d++;
				}

				d += tw - w;
			}
		} else {
#endif
			byte *dst = tmp;

			do {
				memcpy(dst, buf, w * bpp);
				buf += w * bpp;
				dst += tw * bpp;
			} while (--h);
#ifdef USE_RGB_COLOR
		}
#endif

		gfx_tex_convert(&_texMouse, tmp);
		free(tmp);
	}

	_mouseHotspotX = hotspotX;
	_mouseHotspotY = hotspotY;
	_cursorScale = cursorTargetScale;

	if ((_texMouse.palette) && (oldKeycolor != _mouseKeyColor))
		_cursorPaletteDirty = true;
}

