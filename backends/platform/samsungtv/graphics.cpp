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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/platform/sdl/graphics.cpp $
 * $Id: graphics.cpp 44495 2009-09-30 16:16:53Z fingolfin $
 *
 */

#include "backends/platform/samsungtv/samsungtv.h"
#include "common/mutex.h"
#include "common/util.h"
#include "common/list.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/scaler.h"
#include "graphics/surface.h"

#if defined(SAMSUNGTV)

Common::List<Graphics::PixelFormat> OSystem_SDL_SamsungTV::getSupportedFormats() {
	static Common::List<Graphics::PixelFormat>list;
	SDL_Surface *bak = _hwscreen;
	_hwscreen = _prehwscreen;
	list = OSystem_SDL::getSupportedFormats();
	_hwscreen = bak;
	return list;
}

static void fixupResolutionForAspectRatio(AspectRatio desiredAspectRatio, int &width, int &height) {
	assert(&width != &height);

	// FIXME: TV SDL return empty list. However TV accept any resolution
	// Consider list fixed list or make calculation. For now it's disabled.
//	if (desiredAspectRatio.isAuto())
		return;
	
	int kw = desiredAspectRatio.kw();
	int kh = desiredAspectRatio.kh();

	const int w = width;
	const int h = height;

	SDL_Rect const* const*availableModes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_SWSURFACE); //TODO : Maybe specify a pixel format
	assert(availableModes);

	const SDL_Rect *bestMode = NULL;
	uint bestMetric = (uint)-1; // Metric is wasted space
	while (const SDL_Rect *mode = *availableModes++) {
		if (mode->w < w)
			continue;
		if (mode->h < h)
			continue;
		if (mode->h * kw != mode->w * kh)
			continue;
		//printf("%d %d\n", mode->w, mode->h);

		uint metric = mode->w * mode->h - w * h;
		if (metric > bestMetric)
			continue;

		bestMetric = metric;
		bestMode = mode;
	}

	if (!bestMode) {
		warning("Unable to enforce the desired aspect ratio!");
		return;
	}
	//printf("%d %d\n", bestMode->w, bestMode->h);
	width = bestMode->w;
	height = bestMode->h;
}

bool OSystem_SDL_SamsungTV::loadGFXMode() {
	assert(_inited);
	_forceFull = true;

	_videoMode.overlayWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
	_videoMode.overlayHeight = _videoMode.screenHeight * _videoMode.scaleFactor;

	if (_videoMode.screenHeight != 200 && _videoMode.screenHeight != 400)
		_videoMode.aspectRatioCorrection = false;

	if (_videoMode.aspectRatioCorrection)
		_videoMode.overlayHeight = real2Aspect(_videoMode.overlayHeight);

	_videoMode.hardwareWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
	_videoMode.hardwareHeight = effectiveScreenHeight();

	_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.screenWidth, _videoMode.screenHeight, 
						_screenFormat.bytesPerPixel << 3, 
						((1 << _screenFormat.rBits()) - 1) << _screenFormat.rShift ,
						((1 << _screenFormat.gBits()) - 1) << _screenFormat.gShift ,
						((1 << _screenFormat.bBits()) - 1) << _screenFormat.bShift ,
						((1 << _screenFormat.aBits()) - 1) << _screenFormat.aShift );
	if (_screen == NULL)
		error("allocating _screen failed");

	//
	// Create the surface that contains the scaled graphics in 16 bit mode
	//

	if (_videoMode.fullscreen) {
		fixupResolutionForAspectRatio(_videoMode.desiredAspectRatio, _videoMode.hardwareWidth, _videoMode.hardwareHeight);
	}

	_prehwscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.hardwareWidth, _videoMode.hardwareHeight,
						16, 0, 0, 0, 0);
	if (_prehwscreen == NULL)
		error("allocating _prehwscreen failed");

	_hwscreen = SDL_SetVideoMode(_videoMode.hardwareWidth, _videoMode.hardwareHeight, 32,
		_videoMode.fullscreen ? (SDL_FULLSCREEN|SDL_SWSURFACE) : SDL_SWSURFACE
	);

	if (_hwscreen == NULL) {
		// DON'T use error(), as this tries to bring up the debug
		// console, which WON'T WORK now that _hwscreen is hosed.

		if (!_oldVideoMode.setup) {
			warning("SDL_SetVideoMode says we can't switch to that mode (%s)", SDL_GetError());
			quit();
		} else {
			return false;
		}
	}

	//
	// Create the surface used for the graphics in 16 bit before scaling, and also the overlay
	//

	// Need some extra bytes around when using 2xSaI
	_tmpscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.screenWidth + 3, _videoMode.screenHeight + 3,
						16,
						_prehwscreen->format->Rmask,
						_prehwscreen->format->Gmask,
						_prehwscreen->format->Bmask,
						_prehwscreen->format->Amask);

	if (_tmpscreen == NULL)
		error("allocating _tmpscreen failed");

	_overlayscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.overlayWidth, _videoMode.overlayHeight,
						16,
						_prehwscreen->format->Rmask,
						_prehwscreen->format->Gmask,
						_prehwscreen->format->Bmask,
						_prehwscreen->format->Amask);

	if (_overlayscreen == NULL)
		error("allocating _overlayscreen failed");

	_overlayFormat.bytesPerPixel = _overlayscreen->format->BytesPerPixel;

	_overlayFormat.rLoss = _overlayscreen->format->Rloss;
	_overlayFormat.gLoss = _overlayscreen->format->Gloss;
	_overlayFormat.bLoss = _overlayscreen->format->Bloss;
	_overlayFormat.aLoss = _overlayscreen->format->Aloss;

	_overlayFormat.rShift = _overlayscreen->format->Rshift;
	_overlayFormat.gShift = _overlayscreen->format->Gshift;
	_overlayFormat.bShift = _overlayscreen->format->Bshift;
	_overlayFormat.aShift = _overlayscreen->format->Ashift;

	_tmpscreen2 = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.overlayWidth + 3, _videoMode.overlayHeight + 3,
						16,
						_prehwscreen->format->Rmask,
						_prehwscreen->format->Gmask,
						_prehwscreen->format->Bmask,
						_prehwscreen->format->Amask);

	if (_tmpscreen2 == NULL)
		error("allocating _tmpscreen2 failed");

#ifdef USE_OSD
	_osdSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA,
						_hwscreen->w,
						_hwscreen->h,
						32,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);
	if (_osdSurface == NULL)
		error("allocating _osdSurface failed");
	SDL_SetColorKey(_osdSurface, SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA, kOSDColorKey);
#endif

	// keyboard cursor control, some other better place for it?
	_km.x_max = _videoMode.screenWidth * _videoMode.scaleFactor - 1;
	_km.y_max = effectiveScreenHeight() - 1;
	_km.delay_time = 25;
	_km.last_time = 0;

	// Distinguish 555 and 565 mode
	if (_screen->format->Rmask == 0x7C00)
		InitScalers(555);
	else
		InitScalers(565);

	return true;
}

void OSystem_SDL_SamsungTV::unloadGFXMode() {
	if (_prehwscreen) {
		SDL_FreeSurface(_prehwscreen);
		_prehwscreen = NULL;
	}

	OSystem_SDL::unloadGFXMode();
}

bool OSystem_SDL_SamsungTV::hotswapGFXMode() {
	if (!_screen)
		return false;

	SDL_FreeSurface(_prehwscreen); _prehwscreen = NULL;

	return OSystem_SDL::hotswapGFXMode();
}

void OSystem_SDL_SamsungTV::internUpdateScreen() {
	// HACK: Use _prehwscreen instead of _hwscreen for this one.
	// On the long run, it would be cleaner to use _prehwscreen as _hwscreen,
	// and keep what is now called _hwscreen in a new variable _realhwscreen.
	// This way we wouldn't have to overload drawMouse(), too.
	SDL_Surface *bak = _hwscreen;
	_hwscreen = _prehwscreen;
	OSystem_SDL::internUpdateScreen();
	_hwscreen = bak;

	SDL_BlitSurface(_prehwscreen, 0, _hwscreen, 0);
	SDL_UpdateRect(_hwscreen, 0, 0, 0, 0);
}

void OSystem_SDL_SamsungTV::warpMouse(int x, int y) {
	int y1 = y;

	if (_videoMode.aspectRatioCorrection && !_overlayVisible)
		y1 = real2Aspect(y);

	if (_mouseCurState.x != x || _mouseCurState.y != y) {
		if (!_overlayVisible)
			generateMouseMoveEvent(x * _videoMode.scaleFactor, y1 * _videoMode.scaleFactor);
		else
			generateMouseMoveEvent(x, y1);

		setMousePos(x, y);
	}
}

void OSystem_SDL_SamsungTV::setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, uint32 keycolor, int cursorTargetScale, const Graphics::PixelFormat *format) {
	SDL_Surface *bak = _hwscreen;
	_hwscreen = _prehwscreen;
	OSystem_SDL::setMouseCursor(buf, w, h, hotspot_x, hotspot_y, keycolor, cursorTargetScale, format);
	_hwscreen = bak;
}

void OSystem_SDL_SamsungTV::blitCursor() {
	SDL_Surface *bak = _hwscreen;
	_hwscreen = _prehwscreen;
	OSystem_SDL::blitCursor();
	_hwscreen = bak;
}

void OSystem_SDL_SamsungTV::drawMouse() {
	SDL_Surface *bak = _hwscreen;
	_hwscreen = _prehwscreen;
	OSystem_SDL::drawMouse();
	_hwscreen = bak;
}

#endif
