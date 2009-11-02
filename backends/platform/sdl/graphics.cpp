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

#include "backends/platform/sdl/sdl.h"
#include "common/mutex.h"
#include "common/util.h"
#ifdef USE_RGB_COLOR
#include "common/list.h"
#endif
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/scaler.h"
#include "graphics/surface.h"

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{"1x", "Normal (no scaling)", GFX_NORMAL},
	{"2x", "2x", GFX_DOUBLESIZE},
	{"3x", "3x", GFX_TRIPLESIZE},
	{"2xsai", "2xSAI", GFX_2XSAI},
	{"super2xsai", "Super2xSAI", GFX_SUPER2XSAI},
	{"supereagle", "SuperEagle", GFX_SUPEREAGLE},
	{"advmame2x", "AdvMAME2x", GFX_ADVMAME2X},
	{"advmame3x", "AdvMAME3x", GFX_ADVMAME3X},
#ifndef DISABLE_HQ_SCALERS
	{"hq2x", "HQ2x", GFX_HQ2X},
	{"hq3x", "HQ3x", GFX_HQ3X},
#endif
	{"tv2x", "TV2x", GFX_TV2X},
	{"dotmatrix", "DotMatrix", GFX_DOTMATRIX},
	{0, 0, 0}
};

// Table of relative scalers magnitudes
// [definedScale - 1][scaleFactor - 1]
static ScalerProc *scalersMagn[3][3] = {
#ifndef DISABLE_SCALERS
	{ Normal1x, AdvMame2x, AdvMame3x },
	{ Normal1x, Normal1x, Normal1o5x },
	{ Normal1x, Normal1x, Normal1x }
#else // remove dependencies on other scalers
	{ Normal1x, Normal1x, Normal1x },
	{ Normal1x, Normal1x, Normal1x },
	{ Normal1x, Normal1x, Normal1x }
#endif
};

static const int s_gfxModeSwitchTable[][4] = {
		{ GFX_NORMAL, GFX_DOUBLESIZE, GFX_TRIPLESIZE, -1 },
		{ GFX_NORMAL, GFX_ADVMAME2X, GFX_ADVMAME3X, -1 },
		{ GFX_NORMAL, GFX_HQ2X, GFX_HQ3X, -1 },
		{ GFX_NORMAL, GFX_2XSAI, -1, -1 },
		{ GFX_NORMAL, GFX_SUPER2XSAI, -1, -1 },
		{ GFX_NORMAL, GFX_SUPEREAGLE, -1, -1 },
		{ GFX_NORMAL, GFX_TV2X, -1, -1 },
		{ GFX_NORMAL, GFX_DOTMATRIX, -1, -1 }
	};

#ifndef DISABLE_SCALERS
static int cursorStretch200To240(uint8 *buf, uint32 pitch, int width, int height, int srcX, int srcY, int origSrcY);
#endif

const OSystem::GraphicsMode *OSystem_SDL::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

int OSystem_SDL::getDefaultGraphicsMode() const {
	return GFX_DOUBLESIZE;
}

void OSystem_SDL::beginGFXTransaction() {
	assert(_transactionMode == kTransactionNone);

	_transactionMode = kTransactionActive;

	_transactionDetails.sizeChanged = false;

	_transactionDetails.needHotswap = false;
	_transactionDetails.needUpdatescreen = false;

	_transactionDetails.normal1xScaler = false;
#ifdef USE_RGB_COLOR
	_transactionDetails.formatChanged = false;
#endif

	_oldVideoMode = _videoMode;
}

OSystem::TransactionError OSystem_SDL::endGFXTransaction() {
	int errors = kTransactionSuccess;

	assert(_transactionMode != kTransactionNone);

	if (_transactionMode == kTransactionRollback) {
		if (_videoMode.fullscreen != _oldVideoMode.fullscreen) {
			errors |= kTransactionFullscreenFailed;

			_videoMode.fullscreen = _oldVideoMode.fullscreen;
		} else if (_videoMode.aspectRatioCorrection != _oldVideoMode.aspectRatioCorrection) {
			errors |= kTransactionAspectRatioFailed;

			_videoMode.aspectRatioCorrection = _oldVideoMode.aspectRatioCorrection;
		} else if (_videoMode.mode != _oldVideoMode.mode) {
			errors |= kTransactionModeSwitchFailed;

			_videoMode.mode = _oldVideoMode.mode;
			_videoMode.scaleFactor = _oldVideoMode.scaleFactor;
#ifdef USE_RGB_COLOR
		} else if (_videoMode.format != _oldVideoMode.format) {
			errors |= kTransactionFormatNotSupported;

			_videoMode.format = _oldVideoMode.format;
			_screenFormat = _videoMode.format;
#endif
		} else if (_videoMode.screenWidth != _oldVideoMode.screenWidth || _videoMode.screenHeight != _oldVideoMode.screenHeight) {
			errors |= kTransactionSizeChangeFailed;

			_videoMode.screenWidth = _oldVideoMode.screenWidth;
			_videoMode.screenHeight = _oldVideoMode.screenHeight;
			_videoMode.overlayWidth = _oldVideoMode.overlayWidth;
			_videoMode.overlayHeight = _oldVideoMode.overlayHeight;
		}

		if (_videoMode.fullscreen == _oldVideoMode.fullscreen &&
			_videoMode.aspectRatioCorrection == _oldVideoMode.aspectRatioCorrection &&
			_videoMode.mode == _oldVideoMode.mode &&
			_videoMode.screenWidth == _oldVideoMode.screenWidth &&
		   	_videoMode.screenHeight == _oldVideoMode.screenHeight) {

			// Our new video mode would now be exactly the same as the
			// old one. Since we still can not assume SDL_SetVideoMode
			// to be working fine, we need to invalidate the old video
			// mode, so loadGFXMode would error out properly.
			_oldVideoMode.setup = false;
		}
	}

#ifdef USE_RGB_COLOR
	if (_transactionDetails.sizeChanged || _transactionDetails.formatChanged) {
#else
	if (_transactionDetails.sizeChanged) {
#endif
		unloadGFXMode();
		if (!loadGFXMode()) {
			if (_oldVideoMode.setup) {
				_transactionMode = kTransactionRollback;
				errors |= endGFXTransaction();
			}
		} else {
			setGraphicsModeIntern();
			clearOverlay();

			_videoMode.setup = true;
			_modeChanged = true;
			// OSystem_SDL::pollEvent used to update the screen change count,
			// but actually it gives problems when a video mode was changed
			// but OSystem_SDL::pollEvent was not called. This for example
			// caused a crash under certain circumstances when doing an RTL.
			// To fix this issue we update the screen change count right here.
			_screenChangeCount++;
		}
	} else if (_transactionDetails.needHotswap) {
		setGraphicsModeIntern();
		if (!hotswapGFXMode()) {
			if (_oldVideoMode.setup) {
				_transactionMode = kTransactionRollback;
				errors |= endGFXTransaction();
			}
		} else {
			_videoMode.setup = true;
			_modeChanged = true;
			// OSystem_SDL::pollEvent used to update the screen change count,
			// but actually it gives problems when a video mode was changed
			// but OSystem_SDL::pollEvent was not called. This for example
			// caused a crash under certain circumstances when doing an RTL.
			// To fix this issue we update the screen change count right here.
			_screenChangeCount++;

			if (_transactionDetails.needUpdatescreen)
				internUpdateScreen();
		}
	} else if (_transactionDetails.needUpdatescreen) {
		setGraphicsModeIntern();
		internUpdateScreen();
	} 

	_transactionMode = kTransactionNone;
	return (TransactionError)errors;
}

#ifdef USE_RGB_COLOR
const Graphics::PixelFormat RGBList[] = {
#ifdef ENABLE_32BIT
	// RGBA8888, ARGB8888, RGB888
	Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0),
	Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24),
	Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0),
#endif
	// RGB565, XRGB1555, RGB555, RGBA4444, ARGB4444
	Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0),
	Graphics::PixelFormat(2, 5, 5, 5, 1, 10, 5, 0, 15),
	Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0),
	Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0),
	Graphics::PixelFormat(2, 4, 4, 4, 4, 8, 4, 0, 12)
};
const Graphics::PixelFormat BGRList[] = {
#ifdef ENABLE_32BIT
	// ABGR8888, BGRA8888, BGR888
	Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24),
	Graphics::PixelFormat(4, 8, 8, 8, 8, 8, 16, 24, 0),
	Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0),
#endif
	// BGR565, XBGR1555, BGR555, ABGR4444, BGRA4444
	Graphics::PixelFormat(2, 5, 6, 5, 0, 0, 5, 11, 0),
	Graphics::PixelFormat(2, 5, 5, 5, 1, 0, 5, 10, 15),
	Graphics::PixelFormat(2, 5, 5, 5, 0, 0, 5, 10, 0),
	Graphics::PixelFormat(2, 4, 4, 4, 4, 0, 4, 8, 12),
	Graphics::PixelFormat(2, 4, 4, 4, 4, 4, 8, 12, 0)
};

// TODO: prioritize matching alpha masks
Common::List<Graphics::PixelFormat> OSystem_SDL::getSupportedFormats() {
	static Common::List<Graphics::PixelFormat> list;
	static bool inited = false;

	if (inited)
		return list;

	bool BGR = false;
	int listLength = ARRAYSIZE(RGBList);

	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
	if (_hwscreen) {
		// Get our currently set hardware format
		format = Graphics::PixelFormat(_hwscreen->format->BytesPerPixel, 
			8 - _hwscreen->format->Rloss, 8 - _hwscreen->format->Gloss, 
			8 - _hwscreen->format->Bloss, 8 - _hwscreen->format->Aloss, 
			_hwscreen->format->Rshift, _hwscreen->format->Gshift, 
			_hwscreen->format->Bshift, _hwscreen->format->Ashift);

		// Workaround to MacOSX SDL not providing an accurate Aloss value.
		if (_hwscreen->format->Amask == 0)
			format.aLoss = 8;

		// Push it first, as the prefered format.
		list.push_back(format);

		if (format.bShift > format.rShift)
			BGR = true;

		// Mark that we don't need to do this any more.
		inited = true;
	}

	for (int i = 0; i < listLength; i++) {
		if (inited && (RGBList[i].bytesPerPixel > format.bytesPerPixel))
			continue;
		if (BGR) {
			if (BGRList[i] != format)
				list.push_back(BGRList[i]);
			list.push_back(RGBList[i]);
		} else {
			if (RGBList[i] != format)
				list.push_back(RGBList[i]);
			list.push_back(BGRList[i]);
		}
	}
	list.push_back(Graphics::PixelFormat::createFormatCLUT8());
	return list;
}
#endif

bool OSystem_SDL::setGraphicsMode(int mode) {
	Common::StackLock lock(_graphicsMutex);

	assert(_transactionMode == kTransactionActive);

	if (_oldVideoMode.setup && _oldVideoMode.mode == mode)
		return true;

	int newScaleFactor = 1;

	switch (mode) {
	case GFX_NORMAL:
		newScaleFactor = 1;
		break;
#ifndef DISABLE_SCALERS
	case GFX_DOUBLESIZE:
		newScaleFactor = 2;
		break;
	case GFX_TRIPLESIZE:
		newScaleFactor = 3;
		break;

	case GFX_2XSAI:
		newScaleFactor = 2;
		break;
	case GFX_SUPER2XSAI:
		newScaleFactor = 2;
		break;
	case GFX_SUPEREAGLE:
		newScaleFactor = 2;
		break;
	case GFX_ADVMAME2X:
		newScaleFactor = 2;
		break;
	case GFX_ADVMAME3X:
		newScaleFactor = 3;
		break;
#ifndef DISABLE_HQ_SCALERS
	case GFX_HQ2X:
		newScaleFactor = 2;
		break;
	case GFX_HQ3X:
		newScaleFactor = 3;
		break;
#endif
	case GFX_TV2X:
		newScaleFactor = 2;
		break;
	case GFX_DOTMATRIX:
		newScaleFactor = 2;
		break;
#endif // DISABLE_SCALERS

	default:
		warning("unknown gfx mode %d", mode);
		return false;
	}

	_transactionDetails.normal1xScaler = (mode == GFX_NORMAL);
	if (_oldVideoMode.setup && _oldVideoMode.scaleFactor != newScaleFactor)
		_transactionDetails.needHotswap = true;

	_transactionDetails.needUpdatescreen = true;

	_videoMode.mode = mode;
	_videoMode.scaleFactor = newScaleFactor;

	return true;
}

void OSystem_SDL::setGraphicsModeIntern() {
	Common::StackLock lock(_graphicsMutex);
	ScalerProc *newScalerProc = 0;

	switch (_videoMode.mode) {
	case GFX_NORMAL:
		newScalerProc = Normal1x;
		break;
#ifndef DISABLE_SCALERS
	case GFX_DOUBLESIZE:
		newScalerProc = Normal2x;
		break;
	case GFX_TRIPLESIZE:
		newScalerProc = Normal3x;
		break;

	case GFX_2XSAI:
		newScalerProc = _2xSaI;
		break;
	case GFX_SUPER2XSAI:
		newScalerProc = Super2xSaI;
		break;
	case GFX_SUPEREAGLE:
		newScalerProc = SuperEagle;
		break;
	case GFX_ADVMAME2X:
		newScalerProc = AdvMame2x;
		break;
	case GFX_ADVMAME3X:
		newScalerProc = AdvMame3x;
		break;
#ifndef DISABLE_HQ_SCALERS
	case GFX_HQ2X:
		newScalerProc = HQ2x;
		break;
	case GFX_HQ3X:
		newScalerProc = HQ3x;
		break;
#endif
	case GFX_TV2X:
		newScalerProc = TV2x;
		break;
	case GFX_DOTMATRIX:
		newScalerProc = DotMatrix;
		break;
#endif // DISABLE_SCALERS

	default:
		error("Unknown gfx mode %d", _videoMode.mode);
	}

	_scalerProc = newScalerProc;

	if (_videoMode.mode != GFX_NORMAL) {
		for (int i = 0; i < ARRAYSIZE(s_gfxModeSwitchTable); i++) {
			if (s_gfxModeSwitchTable[i][1] == _videoMode.mode || s_gfxModeSwitchTable[i][2] == _videoMode.mode) {
				_scalerType = i;
				break;
			}
		}
	}

	if (!_screen || !_hwscreen)
		return;

	// Blit everything to the screen
	_forceFull = true;

	// Even if the old and new scale factors are the same, we may have a
	// different scaler for the cursor now.
	blitCursor();
}

int OSystem_SDL::getGraphicsMode() const {
	assert (_transactionMode == kTransactionNone);
	return _videoMode.mode;
}

void OSystem_SDL::initSize(uint w, uint h, const Graphics::PixelFormat *format) {
	assert(_transactionMode == kTransactionActive);

#ifdef USE_RGB_COLOR
	//avoid redundant format changes
	Graphics::PixelFormat newFormat;
	if (!format)
		newFormat = Graphics::PixelFormat::createFormatCLUT8();
	else
		newFormat = *format;

	assert(newFormat.bytesPerPixel > 0);

	if (newFormat != _videoMode.format)
	{
		_videoMode.format = newFormat;
		_transactionDetails.formatChanged = true;
		_screenFormat = newFormat;
	}
#endif

	// Avoid redundant res changes
	if ((int)w == _videoMode.screenWidth && (int)h == _videoMode.screenHeight)
		return;

	_videoMode.screenWidth = w;
	_videoMode.screenHeight = h;

	_cksumNum = (w * h / (8 * 8));

	_transactionDetails.sizeChanged = true;

	free(_dirtyChecksums);
	_dirtyChecksums = (uint32 *)calloc(_cksumNum * 2, sizeof(uint32));
}


static void fixupResolutionForAspectRatio(AspectRatio desiredAspectRatio, int &width, int &height) {
	assert(&width != &height);

	if (desiredAspectRatio.isAuto())
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

bool OSystem_SDL::loadGFXMode() {
	assert(_inited);
	_forceFull = true;

#if !defined(__MAEMO__) && !defined(GP2XWIZ) && !defined(LINUXMOTO)
	_videoMode.overlayWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
	_videoMode.overlayHeight = _videoMode.screenHeight * _videoMode.scaleFactor;

	if (_videoMode.screenHeight != 200 && _videoMode.screenHeight != 400)
		_videoMode.aspectRatioCorrection = false;

	if (_videoMode.aspectRatioCorrection)
		_videoMode.overlayHeight = real2Aspect(_videoMode.overlayHeight);

	_videoMode.hardwareWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
	_videoMode.hardwareHeight = effectiveScreenHeight();
#else
	_videoMode.hardwareWidth = _videoMode.overlayWidth;
	_videoMode.hardwareHeight = _videoMode.overlayHeight;
#endif

	//
	// Create the surface that contains the 8 bit game data
	//
#ifdef USE_RGB_COLOR
	_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.screenWidth, _videoMode.screenHeight, 
						_screenFormat.bytesPerPixel << 3, 
						((1 << _screenFormat.rBits()) - 1) << _screenFormat.rShift ,
						((1 << _screenFormat.gBits()) - 1) << _screenFormat.gShift ,
						((1 << _screenFormat.bBits()) - 1) << _screenFormat.bShift ,
						((1 << _screenFormat.aBits()) - 1) << _screenFormat.aShift );
	if (_screen == NULL)
		error("allocating _screen failed");

#else
	_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.screenWidth, _videoMode.screenHeight, 8, 0, 0, 0, 0);
	if (_screen == NULL)
		error("allocating _screen failed");
#endif

	//
	// Create the surface that contains the scaled graphics in 16 bit mode
	//

	if (_videoMode.fullscreen) {
		fixupResolutionForAspectRatio(_videoMode.desiredAspectRatio, _videoMode.hardwareWidth, _videoMode.hardwareHeight);
	}

	_hwscreen = SDL_SetVideoMode(_videoMode.hardwareWidth, _videoMode.hardwareHeight, 16,
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
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

	if (_tmpscreen == NULL)
		error("allocating _tmpscreen failed");

	_overlayscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.overlayWidth, _videoMode.overlayHeight,
						16,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

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
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

	if (_tmpscreen2 == NULL)
		error("allocating _tmpscreen2 failed");

#ifdef USE_OSD
	_osdSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA,
						_hwscreen->w,
						_hwscreen->h,
						16,
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
	if (_hwscreen->format->Rmask == 0x7C00)
		InitScalers(555);
	else
		InitScalers(565);

	return true;
}

void OSystem_SDL::unloadGFXMode() {
	if (_screen) {
		SDL_FreeSurface(_screen);
		_screen = NULL;
	}

	if (_hwscreen) {
		SDL_FreeSurface(_hwscreen);
		_hwscreen = NULL;
	}

	if (_tmpscreen) {
		SDL_FreeSurface(_tmpscreen);
		_tmpscreen = NULL;
	}

	if (_tmpscreen2) {
		SDL_FreeSurface(_tmpscreen2);
		_tmpscreen2 = NULL;
	}

	if (_overlayscreen) {
		SDL_FreeSurface(_overlayscreen);
		_overlayscreen = NULL;
	}

#ifdef USE_OSD
	if (_osdSurface) {
		SDL_FreeSurface(_osdSurface);
		_osdSurface = NULL;
	}
#endif
	DestroyScalers();
}

bool OSystem_SDL::hotswapGFXMode() {
	if (!_screen)
		return false;

	// Keep around the old _screen & _overlayscreen so we can restore the screen data
	// after the mode switch.
	SDL_Surface *old_screen = _screen;
	_screen = NULL;
	SDL_Surface *old_overlayscreen = _overlayscreen;
	_overlayscreen = NULL;

	// Release the HW screen surface
	SDL_FreeSurface(_hwscreen); _hwscreen = NULL;

	SDL_FreeSurface(_tmpscreen); _tmpscreen = NULL;
	SDL_FreeSurface(_tmpscreen2); _tmpscreen2 = NULL;

#ifdef USE_OSD
	// Release the OSD surface
	SDL_FreeSurface(_osdSurface); _osdSurface = NULL;
#endif

	// Setup the new GFX mode
	if (!loadGFXMode()) {
		unloadGFXMode();

		_screen = old_screen;
		_overlayscreen = old_overlayscreen;

		return false;
	}

	// reset palette
	SDL_SetColors(_screen, _currentPalette, 0, 256);

	// Restore old screen content
	SDL_BlitSurface(old_screen, NULL, _screen, NULL);
	SDL_BlitSurface(old_overlayscreen, NULL, _overlayscreen, NULL);

	// Free the old surfaces
	SDL_FreeSurface(old_screen);
	SDL_FreeSurface(old_overlayscreen);

	// Update cursor to new scale
	blitCursor();

	// Blit everything to the screen
	internUpdateScreen();

	return true;
}

void OSystem_SDL::updateScreen() {
	assert (_transactionMode == kTransactionNone);

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	internUpdateScreen();
}

void OSystem_SDL::internUpdateScreen() {
	SDL_Surface *srcSurf, *origSurf;
	int height, width;
	ScalerProc *scalerProc;
	int scale1;

#if defined (DEBUG) && ! defined(_WIN32_WCE) // definitions not available for non-DEBUG here. (needed this to compile in SYMBIAN32 & linux?)
	assert(_hwscreen != NULL);
	assert(_hwscreen->map->sw_data != NULL);
#endif

	// If the shake position changed, fill the dirty area with blackness
	if (_currentShakePos != _newShakePos) {
		SDL_Rect blackrect = {0, 0, _videoMode.screenWidth * _videoMode.scaleFactor, _newShakePos * _videoMode.scaleFactor};

		if (_videoMode.aspectRatioCorrection && !_overlayVisible)
			blackrect.h = real2Aspect(blackrect.h - 1) + 1;

		SDL_FillRect(_hwscreen, &blackrect, 0);

		_currentShakePos = _newShakePos;

		_forceFull = true;
	}

	// Check whether the palette was changed in the meantime and update the
	// screen surface accordingly.
	if (_screen && _paletteDirtyEnd != 0) {
		SDL_SetColors(_screen, _currentPalette + _paletteDirtyStart,
			_paletteDirtyStart,
			_paletteDirtyEnd - _paletteDirtyStart);

		_paletteDirtyEnd = 0;

		_forceFull = true;
	}

#ifdef USE_OSD
	// OSD visible (i.e. non-transparent)?
	if (_osdAlpha != SDL_ALPHA_TRANSPARENT) {
		// Updated alpha value
		const int diff = SDL_GetTicks() - _osdFadeStartTime;
		if (diff > 0) {
			if (diff >= kOSDFadeOutDuration) {
				// Back to full transparency
				_osdAlpha = SDL_ALPHA_TRANSPARENT;
			} else {
				// Do a linear fade out...
				const int startAlpha = SDL_ALPHA_TRANSPARENT + kOSDInitialAlpha * (SDL_ALPHA_OPAQUE - SDL_ALPHA_TRANSPARENT) / 100;
				_osdAlpha = startAlpha + diff * (SDL_ALPHA_TRANSPARENT - startAlpha) / kOSDFadeOutDuration;
			}
			SDL_SetAlpha(_osdSurface, SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA, _osdAlpha);
			_forceFull = true;
		}
	}
#endif

	if (!_overlayVisible) {
		origSurf = _screen;
		srcSurf = _tmpscreen;
		width = _videoMode.screenWidth;
		height = _videoMode.screenHeight;
		scalerProc = _scalerProc;
		scale1 = _videoMode.scaleFactor;
	} else {
		origSurf = _overlayscreen;
		srcSurf = _tmpscreen2;
		width = _videoMode.overlayWidth;
		height = _videoMode.overlayHeight;
		scalerProc = Normal1x;

		scale1 = 1;
	}

	// Add the area covered by the mouse cursor to the list of dirty rects if
	// we have to redraw the mouse.
	if (_mouseNeedsRedraw)
		undrawMouse();

	// Force a full redraw if requested
	if (_forceFull) {
		_numDirtyRects = 1;
		_dirtyRectList[0].x = 0;
		_dirtyRectList[0].y = 0;
		_dirtyRectList[0].w = width;
		_dirtyRectList[0].h = height;
	}

	// Only draw anything if necessary
	if (_numDirtyRects > 0 || _mouseNeedsRedraw) {
		SDL_Rect *r;
		SDL_Rect dst;
		uint32 srcPitch, dstPitch;
		SDL_Rect *lastRect = _dirtyRectList + _numDirtyRects;

		for (r = _dirtyRectList; r != lastRect; ++r) {
			dst = *r;
			dst.x++;	// Shift rect by one since 2xSai needs to access the data around
			dst.y++;	// any pixel to scale it, and we want to avoid mem access crashes.

			if (SDL_BlitSurface(origSurf, r, srcSurf, &dst) != 0)
				error("SDL_BlitSurface failed: %s", SDL_GetError());
		}

		SDL_LockSurface(srcSurf);
		SDL_LockSurface(_hwscreen);

		srcPitch = srcSurf->pitch;
		dstPitch = _hwscreen->pitch;

		for (r = _dirtyRectList; r != lastRect; ++r) {
			register int dst_y = r->y + _currentShakePos;
			register int dst_h = 0;
			register int orig_dst_y = 0;
			register int rx1 = r->x * scale1;

			if (dst_y < height) {
				dst_h = r->h;
				if (dst_h > height - dst_y)
					dst_h = height - dst_y;

				orig_dst_y = dst_y;
				dst_y = dst_y * scale1;

				if (_videoMode.aspectRatioCorrection && !_overlayVisible)
					dst_y = real2Aspect(dst_y);

				assert(scalerProc != NULL);
				scalerProc((byte *)srcSurf->pixels + (r->x * 2 + 2) + (r->y + 1) * srcPitch, srcPitch,
						   (byte *)_hwscreen->pixels + rx1 * 2 + dst_y * dstPitch, dstPitch, r->w, dst_h);
			}

			r->x = rx1;
			r->y = dst_y;
			r->w = r->w * scale1;
			r->h = dst_h * scale1;

#ifndef DISABLE_SCALERS
			if (_videoMode.aspectRatioCorrection && orig_dst_y < height && !_overlayVisible)
				r->h = stretch200To240((uint8 *) _hwscreen->pixels, dstPitch, r->w, r->h, r->x, r->y, orig_dst_y * scale1);
#endif
		}
		SDL_UnlockSurface(srcSurf);
		SDL_UnlockSurface(_hwscreen);

		// Readjust the dirty rect list in case we are doing a full update.
		// This is necessary if shaking is active.
		if (_forceFull) {
			_dirtyRectList[0].y = 0;
			_dirtyRectList[0].h = effectiveScreenHeight();
		}

		drawMouse();

#ifdef USE_OSD
		if (_osdAlpha != SDL_ALPHA_TRANSPARENT) {
			SDL_BlitSurface(_osdSurface, 0, _hwscreen, 0);
		}
#endif
		// Finally, blit all our changes to the screen
		SDL_UpdateRects(_hwscreen, _numDirtyRects, _dirtyRectList);
	}

	_numDirtyRects = 0;
	_forceFull = false;
	_mouseNeedsRedraw = false;
}

bool OSystem_SDL::saveScreenshot(const char *filename) {
	assert(_hwscreen != NULL);

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends
	return SDL_SaveBMP(_hwscreen, filename) == 0;
}

void OSystem_SDL::setFullscreenMode(bool enable) {
	Common::StackLock lock(_graphicsMutex);

	if (_oldVideoMode.setup && _oldVideoMode.fullscreen == enable)
		return;

	if (_transactionMode == kTransactionActive) {
		_videoMode.fullscreen = enable;
		_transactionDetails.needHotswap = true;
	}
}

void OSystem_SDL::setAspectRatioCorrection(bool enable) {
	Common::StackLock lock(_graphicsMutex);

	if (_oldVideoMode.setup && _oldVideoMode.aspectRatioCorrection == enable)
		return;

	if (_transactionMode == kTransactionActive) {
		_videoMode.aspectRatioCorrection = enable;
		_transactionDetails.needHotswap = true;
	}
}

void OSystem_SDL::copyRectToScreen(const byte *src, int pitch, int x, int y, int w, int h) {
	assert (_transactionMode == kTransactionNone);
	assert(src);

	if (_screen == NULL) {
		warning("OSystem_SDL::copyRectToScreen: _screen == NULL");
		return;
	}

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	assert(x >= 0 && x < _videoMode.screenWidth);
	assert(y >= 0 && y < _videoMode.screenHeight);
	assert(h > 0 && y + h <= _videoMode.screenHeight);
	assert(w > 0 && x + w <= _videoMode.screenWidth);

	if (IS_ALIGNED(src, 4) && pitch == _videoMode.screenWidth && x == 0 && y == 0 &&
			w == _videoMode.screenWidth && h == _videoMode.screenHeight && _modeFlags & DF_WANT_RECT_OPTIM) {
		/* Special, optimized case for full screen updates.
		 * It tries to determine what areas were actually changed,
		 * and just updates those, on the actual display. */
		addDirtyRgnAuto(src);
	} else {
		_cksumValid = false;
		addDirtyRect(x, y, w, h);
	}

	// Try to lock the screen surface
	if (SDL_LockSurface(_screen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

#ifdef USE_RGB_COLOR
	byte *dst = (byte *)_screen->pixels + y * _videoMode.screenWidth * _screenFormat.bytesPerPixel + x * _screenFormat.bytesPerPixel;
	if (_videoMode.screenWidth == w && pitch == w * _screenFormat.bytesPerPixel) {
		memcpy(dst, src, h*w*_screenFormat.bytesPerPixel);
	} else {
		do {
			memcpy(dst, src, w * _screenFormat.bytesPerPixel);
			src += pitch;
			dst += _videoMode.screenWidth * _screenFormat.bytesPerPixel;
		} while (--h);
	}
#else
	byte *dst = (byte *)_screen->pixels + y * _videoMode.screenWidth + x;
	if (_videoMode.screenWidth == pitch && pitch == w) {
		memcpy(dst, src, h*w);
	} else {
		do {
			memcpy(dst, src, w);
			src += pitch;
			dst += _videoMode.screenWidth;
		} while (--h);
	}
#endif

	// Unlock the screen surface
	SDL_UnlockSurface(_screen);
}

Graphics::Surface *OSystem_SDL::lockScreen() {
	assert (_transactionMode == kTransactionNone);

	// Lock the graphics mutex
	lockMutex(_graphicsMutex);

	// paranoia check
	assert(!_screenIsLocked);
	_screenIsLocked = true;

	// Try to lock the screen surface
	if (SDL_LockSurface(_screen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	_framebuffer.pixels = _screen->pixels;
	_framebuffer.w = _screen->w;
	_framebuffer.h = _screen->h;
	_framebuffer.pitch = _screen->pitch;
#ifdef USE_RGB_COLOR
	_framebuffer.bytesPerPixel = _screenFormat.bytesPerPixel;
#else
	_framebuffer.bytesPerPixel = 1;
#endif

	return &_framebuffer;
}

void OSystem_SDL::unlockScreen() {
	assert (_transactionMode == kTransactionNone);

	// paranoia check
	assert(_screenIsLocked);
	_screenIsLocked = false;

	// Unlock the screen surface
	SDL_UnlockSurface(_screen);

	// Trigger a full screen update
	_forceFull = true;

	// Finally unlock the graphics mutex
	unlockMutex(_graphicsMutex);
}

void OSystem_SDL::addDirtyRect(int x, int y, int w, int h, bool realCoordinates) {
	if (_forceFull)
		return;

	if (_numDirtyRects == NUM_DIRTY_RECT) {
		_forceFull = true;
		return;
	}

	int height, width;

	if (!_overlayVisible && !realCoordinates) {
		width = _videoMode.screenWidth;
		height = _videoMode.screenHeight;
	} else {
		width = _videoMode.overlayWidth;
		height = _videoMode.overlayHeight;
	}

	// Extend the dirty region by 1 pixel for scalers
	// that "smear" the screen, e.g. 2xSAI
	if (!realCoordinates) {
		x--;
		y--;
		w+=2;
		h+=2;
	}

	// clip
	if (x < 0) {
		w += x;
		x = 0;
	}

	if (y < 0) {
		h += y;
		y=0;
	}

	if (w > width - x) {
		w = width - x;
	}

	if (h > height - y) {
		h = height - y;
	}

#ifndef DISABLE_SCALERS
	if (_videoMode.aspectRatioCorrection && !_overlayVisible && !realCoordinates) {
		makeRectStretchable(x, y, w, h);
	}
#endif

	if (w == width && h == height) {
		_forceFull = true;
		return;
	}

	if (w > 0 && h > 0) {
		SDL_Rect *r = &_dirtyRectList[_numDirtyRects++];

		r->x = x;
		r->y = y;
		r->w = w;
		r->h = h;
	}
}


void OSystem_SDL::makeChecksums(const byte *buf) {
	assert(buf);
	uint32 *sums = _dirtyChecksums;
	uint x,y;
	const uint last_x = (uint)_videoMode.screenWidth / 8;
	const uint last_y = (uint)_videoMode.screenHeight / 8;

	const uint BASE = 65521; /* largest prime smaller than 65536 */

	/* the 8x8 blocks in buf are enumerated starting in the top left corner and
	 * reading each line at a time from left to right */
	for (y = 0; y != last_y; y++, buf += _videoMode.screenWidth * (8 - 1))
		for (x = 0; x != last_x; x++, buf += 8) {
			// Adler32 checksum algorithm (from RFC1950, used by gzip and zlib).
			// This computes the Adler32 checksum of a 8x8 pixel block. Note
			// that we can do the modulo operation (which is the slowest part)
			// of the algorithm) at the end, instead of doing each iteration,
			// since we only have 64 iterations in total - and thus s1 and
			// s2 can't overflow anyway.
			uint32 s1 = 1;
			uint32 s2 = 0;
			const byte *ptr = buf;
			for (int subY = 0; subY < 8; subY++) {
				for (int subX = 0; subX < 8; subX++) {
					s1 += ptr[subX];
					s2 += s1;
				}
				ptr += _videoMode.screenWidth;
			}

			s1 %= BASE;
			s2 %= BASE;

			/* output the checksum for this block */
			*sums++ =  (s2 << 16) + s1;
	}
}

void OSystem_SDL::addDirtyRgnAuto(const byte *buf) {
	assert(buf);
	assert(IS_ALIGNED(buf, 4));

	/* generate a table of the checksums */
	makeChecksums(buf);

	if (!_cksumValid) {
		_forceFull = true;
		_cksumValid = true;
	}

	/* go through the checksum list, compare it with the previous checksums,
		 and add all dirty rectangles to a list. try to combine small rectangles
		 into bigger ones in a simple way */
	if (!_forceFull) {
		int x, y, w;
		uint32 *ck = _dirtyChecksums;

		for (y = 0; y != _videoMode.screenHeight / 8; y++) {
			for (x = 0; x != _videoMode.screenWidth / 8; x++, ck++) {
				if (ck[0] != ck[_cksumNum]) {
					/* found a dirty 8x8 block, now go as far to the right as possible,
						 and at the same time, unmark the dirty status by setting old to new. */
					w=0;
					do {
						ck[w + _cksumNum] = ck[w];
						w++;
					} while (x + w != _videoMode.screenWidth / 8 && ck[w] != ck[w + _cksumNum]);

					addDirtyRect(x * 8, y * 8, w * 8, 8);

					if (_forceFull)
						goto get_out;
				}
			}
		}
	} else {
		get_out:;
		/* Copy old checksums to new */
		memcpy(_dirtyChecksums + _cksumNum, _dirtyChecksums, _cksumNum * sizeof(uint32));
	}
}

int16 OSystem_SDL::getHeight() {
	return _videoMode.screenHeight;
}

int16 OSystem_SDL::getWidth() {
	return _videoMode.screenWidth;
}

void OSystem_SDL::setPalette(const byte *colors, uint start, uint num) {
	assert(colors);

#ifdef USE_RGB_COLOR
	if (_screenFormat.bytesPerPixel > 1)
		return; //not using a paletted pixel format
#endif

	// Setting the palette before _screen is created is allowed - for now -
	// since we don't actually set the palette until the screen is updated.
	// But it could indicate a programming error, so let's warn about it.

	if (!_screen)
		warning("OSystem_SDL::setPalette: _screen == NULL");

	const byte *b = colors;
	uint i;
	SDL_Color *base = _currentPalette + start;
	for (i = 0; i < num; i++) {
		base[i].r = b[0];
		base[i].g = b[1];
		base[i].b = b[2];
		b += 4;
	}

	if (start < _paletteDirtyStart)
		_paletteDirtyStart = start;

	if (start + num > _paletteDirtyEnd)
		_paletteDirtyEnd = start + num;

	// Some games blink cursors with palette
	if (_cursorPaletteDisabled)
		blitCursor();
}

void OSystem_SDL::grabPalette(byte *colors, uint start, uint num) {
	assert(colors);
	const SDL_Color *base = _currentPalette + start;

	for (uint i = 0; i < num; ++i) {
		colors[i * 4] = base[i].r;
		colors[i * 4 + 1] = base[i].g;
		colors[i * 4 + 2] = base[i].b;
		colors[i * 4 + 3] = 0xFF;
	}
}

void OSystem_SDL::setCursorPalette(const byte *colors, uint start, uint num) {
	assert(colors);
	const byte *b = colors;
	uint i;
	SDL_Color *base = _cursorPalette + start;
	for (i = 0; i < num; i++) {
		base[i].r = b[0];
		base[i].g = b[1];
		base[i].b = b[2];
		b += 4;
	}

	_cursorPaletteDisabled = false;
	blitCursor();
}


void OSystem_SDL::setShakePos(int shake_pos) {
	assert (_transactionMode == kTransactionNone);

	_newShakePos = shake_pos;
}


#pragma mark -
#pragma mark --- Overlays ---
#pragma mark -

void OSystem_SDL::showOverlay() {
	assert (_transactionMode == kTransactionNone);

	int x, y;

	if (_overlayVisible)
		return;

	_overlayVisible = true;

	// Since resolution could change, put mouse to adjusted position
	// Fixes bug #1349059
	x = _mouseCurState.x * _videoMode.scaleFactor;
	if (_videoMode.aspectRatioCorrection)
		y = real2Aspect(_mouseCurState.y) * _videoMode.scaleFactor;
	else
		y = _mouseCurState.y * _videoMode.scaleFactor;

	warpMouse(x, y);

	clearOverlay();
}

void OSystem_SDL::hideOverlay() {
	assert (_transactionMode == kTransactionNone);

	if (!_overlayVisible)
		return;

	int x, y;

	_overlayVisible = false;

	// Since resolution could change, put mouse to adjusted position
	// Fixes bug #1349059
	x = _mouseCurState.x / _videoMode.scaleFactor;
	y = _mouseCurState.y / _videoMode.scaleFactor;
	if (_videoMode.aspectRatioCorrection)
		y = aspect2Real(y);

	warpMouse(x, y);

	clearOverlay();

	_forceFull = true;
}

void OSystem_SDL::clearOverlay() {
	//assert (_transactionMode == kTransactionNone);

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	if (!_overlayVisible)
		return;

	// Clear the overlay by making the game screen "look through" everywhere.
	SDL_Rect src, dst;
	src.x = src.y = 0;
	dst.x = dst.y = 1;
	src.w = dst.w = _videoMode.screenWidth;
	src.h = dst.h = _videoMode.screenHeight;
	if (SDL_BlitSurface(_screen, &src, _tmpscreen, &dst) != 0)
		error("SDL_BlitSurface failed: %s", SDL_GetError());

	SDL_LockSurface(_tmpscreen);
	SDL_LockSurface(_overlayscreen);
	_scalerProc((byte *)(_tmpscreen->pixels) + _tmpscreen->pitch + 2, _tmpscreen->pitch,
	(byte *)_overlayscreen->pixels, _overlayscreen->pitch, _videoMode.screenWidth, _videoMode.screenHeight);

#ifndef DISABLE_SCALERS
	if (_videoMode.aspectRatioCorrection)
		stretch200To240((uint8 *)_overlayscreen->pixels, _overlayscreen->pitch,
						_videoMode.overlayWidth, _videoMode.screenHeight * _videoMode.scaleFactor, 0, 0, 0);
#endif
	SDL_UnlockSurface(_tmpscreen);
	SDL_UnlockSurface(_overlayscreen);

	_forceFull = true;
}

void OSystem_SDL::grabOverlay(OverlayColor *buf, int pitch) {
	assert (_transactionMode == kTransactionNone);

	if (_overlayscreen == NULL)
		return;

	if (SDL_LockSurface(_overlayscreen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *src = (byte *)_overlayscreen->pixels;
	int h = _videoMode.overlayHeight;
	do {
		memcpy(buf, src, _videoMode.overlayWidth * 2);
		src += _overlayscreen->pitch;
		buf += pitch;
	} while (--h);

	SDL_UnlockSurface(_overlayscreen);
}

void OSystem_SDL::copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {
	assert (_transactionMode == kTransactionNone);

	if (_overlayscreen == NULL)
		return;

	// Clip the coordinates
	if (x < 0) {
		w += x;
		buf -= x;
		x = 0;
	}

	if (y < 0) {
		h += y; buf -= y * pitch;
		y = 0;
	}

	if (w > _videoMode.overlayWidth - x) {
		w = _videoMode.overlayWidth - x;
	}

	if (h > _videoMode.overlayHeight - y) {
		h = _videoMode.overlayHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;

	// Mark the modified region as dirty
	_cksumValid = false;
	addDirtyRect(x, y, w, h);

	if (SDL_LockSurface(_overlayscreen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *dst = (byte *)_overlayscreen->pixels + y * _overlayscreen->pitch + x * 2;
	do {
		memcpy(dst, buf, w * 2);
		dst += _overlayscreen->pitch;
		buf += pitch;
	} while (--h);

	SDL_UnlockSurface(_overlayscreen);
}


#pragma mark -
#pragma mark --- Mouse ---
#pragma mark -

bool OSystem_SDL::showMouse(bool visible) {
	if (_mouseVisible == visible)
		return visible;

	bool last = _mouseVisible;
	_mouseVisible = visible;
	_mouseNeedsRedraw = true;

	return last;
}

void OSystem_SDL::setMousePos(int x, int y) {
	if (x != _mouseCurState.x || y != _mouseCurState.y) {
		_mouseNeedsRedraw = true;
		_mouseCurState.x = x;
		_mouseCurState.y = y;
	}
}

void OSystem_SDL::warpMouse(int x, int y) {
	int y1 = y;

	if (_videoMode.aspectRatioCorrection && !_overlayVisible)
		y1 = real2Aspect(y);

	if (_mouseCurState.x != x || _mouseCurState.y != y) {
		if (!_overlayVisible)
			SDL_WarpMouse(x * _videoMode.scaleFactor, y1 * _videoMode.scaleFactor);
		else
			SDL_WarpMouse(x, y1);

		// SDL_WarpMouse() generates a mouse movement event, so
		// setMousePos() would be called eventually. However, the
		// cannon script in CoMI calls this function twice each time
		// the cannon is reloaded. Unless we update the mouse position
		// immediately the second call is ignored, causing the cannon
		// to change its aim.

		setMousePos(x, y);
	}
}

void OSystem_SDL::setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, uint32 keycolor, int cursorTargetScale, const Graphics::PixelFormat *format) {
#ifdef USE_RGB_COLOR
	if (!format)
		_cursorFormat = Graphics::PixelFormat::createFormatCLUT8();
	else if (format->bytesPerPixel <= _screenFormat.bytesPerPixel)
		_cursorFormat = *format;
	keycolor &= (1 << (_cursorFormat.bytesPerPixel << 3)) - 1;
#else
	keycolor &= 0xFF;
#endif

	if (w == 0 || h == 0)
		return;

	_mouseCurState.hotX = hotspot_x;
	_mouseCurState.hotY = hotspot_y;

	_mouseKeyColor = keycolor;

	_cursorTargetScale = cursorTargetScale;

	if (_mouseCurState.w != (int)w || _mouseCurState.h != (int)h) {
		_mouseCurState.w = w;
		_mouseCurState.h = h;

		if (_mouseOrigSurface)
			SDL_FreeSurface(_mouseOrigSurface);

		// Allocate bigger surface because AdvMame2x adds black pixel at [0,0]
		_mouseOrigSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA,
						_mouseCurState.w + 2,
						_mouseCurState.h + 2,
						16,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

		if (_mouseOrigSurface == NULL)
			error("allocating _mouseOrigSurface failed");
		SDL_SetColorKey(_mouseOrigSurface, SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA, kMouseColorKey);
	}

	free(_mouseData);
#ifdef USE_RGB_COLOR
	_mouseData = (byte *)malloc(w * h * _cursorFormat.bytesPerPixel);
	memcpy(_mouseData, buf, w * h * _cursorFormat.bytesPerPixel);
#else
	_mouseData = (byte *)malloc(w * h);
	memcpy(_mouseData, buf, w * h);
#endif

	blitCursor();
}

void OSystem_SDL::blitCursor() {
	byte *dstPtr;
	const byte *srcPtr = _mouseData;
#ifdef USE_RGB_COLOR
	uint32 color;
#else
	byte color;
#endif
	int w, h, i, j;

	if (!_mouseOrigSurface || !_mouseData)
		return;

	_mouseNeedsRedraw = true;

	w = _mouseCurState.w;
	h = _mouseCurState.h;

	SDL_LockSurface(_mouseOrigSurface);

	// Make whole surface transparent
	for (i = 0; i < h + 2; i++) {
		dstPtr = (byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch * i;
		for (j = 0; j < w + 2; j++) {
			*(uint16 *)dstPtr = kMouseColorKey;
			dstPtr += 2;
		}
	}

	// Draw from [1,1] since AdvMame2x adds artefact at 0,0
	dstPtr = (byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch + 2;

	SDL_Color *palette;

	if (_cursorPaletteDisabled)
		palette = _currentPalette;
	else
		palette = _cursorPalette;

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
#ifdef USE_RGB_COLOR
			if (_cursorFormat.bytesPerPixel > 1) {
				if (_cursorFormat.bytesPerPixel == 2)
					color = *(uint16 *)srcPtr;
				else
					color = *(uint32 *)srcPtr;
				if (color != _mouseKeyColor) {	// transparent, don't draw
					uint8 r, g, b;
					_cursorFormat.colorToRGB(color, r, g, b);
					*(uint16 *)dstPtr = SDL_MapRGB(_mouseOrigSurface->format,
						r, g, b);
				}
				dstPtr += 2;
				srcPtr += _cursorFormat.bytesPerPixel;
			} else {
#endif
				color = *srcPtr;
				if (color != _mouseKeyColor) {	// transparent, don't draw
					*(uint16 *)dstPtr = SDL_MapRGB(_mouseOrigSurface->format,
						palette[color].r, palette[color].g, palette[color].b);
				}
				dstPtr += 2;
				srcPtr++;
#ifdef USE_RGB_COLOR
			}
#endif
		}
		dstPtr += _mouseOrigSurface->pitch - w * 2;
	}

	int rW, rH;

	if (_cursorTargetScale >= _videoMode.scaleFactor) {
		// The cursor target scale is greater or equal to the scale at
		// which the rest of the screen is drawn. We do not downscale
		// the cursor image, we draw it at its original size. It will
		// appear too large on screen.

		rW = w;
		rH = h;
		_mouseCurState.rHotX = _mouseCurState.hotX;
		_mouseCurState.rHotY = _mouseCurState.hotY;

		// The virtual dimensions may be larger than the original.

		_mouseCurState.vW = w * _cursorTargetScale / _videoMode.scaleFactor;
		_mouseCurState.vH = h * _cursorTargetScale / _videoMode.scaleFactor;
		_mouseCurState.vHotX = _mouseCurState.hotX * _cursorTargetScale /
			_videoMode.scaleFactor;
		_mouseCurState.vHotY = _mouseCurState.hotY * _cursorTargetScale /
			_videoMode.scaleFactor;
	} else {
		// The cursor target scale is smaller than the scale at which
		// the rest of the screen is drawn. We scale up the cursor
		// image to make it appear correct.

		rW = w * _videoMode.scaleFactor / _cursorTargetScale;
		rH = h * _videoMode.scaleFactor / _cursorTargetScale;
		_mouseCurState.rHotX = _mouseCurState.hotX * _videoMode.scaleFactor /
			_cursorTargetScale;
		_mouseCurState.rHotY = _mouseCurState.hotY * _videoMode.scaleFactor /
			_cursorTargetScale;

		// The virtual dimensions will be the same as the original.

		_mouseCurState.vW = w;
		_mouseCurState.vH = h;
		_mouseCurState.vHotX = _mouseCurState.hotX;
		_mouseCurState.vHotY = _mouseCurState.hotY;
	}

#ifndef DISABLE_SCALERS
	int rH1 = rH; // store original to pass to aspect-correction function later
#endif

	if (_videoMode.aspectRatioCorrection && _cursorTargetScale == 1) {
		rH = real2Aspect(rH - 1) + 1;
		_mouseCurState.rHotY = real2Aspect(_mouseCurState.rHotY);
	}

	if (_mouseCurState.rW != rW || _mouseCurState.rH != rH) {
		_mouseCurState.rW = rW;
		_mouseCurState.rH = rH;

		if (_mouseSurface)
			SDL_FreeSurface(_mouseSurface);

		_mouseSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA,
						_mouseCurState.rW,
						_mouseCurState.rH,
						16,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

		if (_mouseSurface == NULL)
			error("allocating _mouseSurface failed");

		SDL_SetColorKey(_mouseSurface, SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA, kMouseColorKey);
	}

	SDL_LockSurface(_mouseSurface);

	ScalerProc *scalerProc;

	// If possible, use the same scaler for the cursor as for the rest of
	// the game. This only works well with the non-blurring scalers so we
	// actually only use the 1x, 1.5x, 2x and AdvMame scalers.

	if (_cursorTargetScale == 1 && (_videoMode.mode == GFX_DOUBLESIZE || _videoMode.mode == GFX_TRIPLESIZE))
		scalerProc = _scalerProc;
	else
		scalerProc = scalersMagn[_cursorTargetScale - 1][_videoMode.scaleFactor - 1];

	scalerProc((byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch + 2,
		_mouseOrigSurface->pitch, (byte *)_mouseSurface->pixels, _mouseSurface->pitch,
		_mouseCurState.w, _mouseCurState.h);

#ifndef DISABLE_SCALERS
	if (_videoMode.aspectRatioCorrection && _cursorTargetScale == 1)
		cursorStretch200To240((uint8 *)_mouseSurface->pixels, _mouseSurface->pitch, rW, rH1, 0, 0, 0);
#endif

	SDL_UnlockSurface(_mouseSurface);
	SDL_UnlockSurface(_mouseOrigSurface);
}

#ifndef DISABLE_SCALERS
// Basically it is kVeryFastAndUglyAspectMode of stretch200To240 from
// common/scale/aspect.cpp
static int cursorStretch200To240(uint8 *buf, uint32 pitch, int width, int height, int srcX, int srcY, int origSrcY) {
	int maxDstY = real2Aspect(origSrcY + height - 1);
	int y;
	const uint8 *startSrcPtr = buf + srcX * 2 + (srcY - origSrcY) * pitch;
	uint8 *dstPtr = buf + srcX * 2 + maxDstY * pitch;

	for (y = maxDstY; y >= srcY; y--) {
		const uint8 *srcPtr = startSrcPtr + aspect2Real(y) * pitch;

		if (srcPtr == dstPtr)
			break;
		memcpy(dstPtr, srcPtr, width * 2);
		dstPtr -= pitch;
	}

	return 1 + maxDstY - srcY;
}
#endif

void OSystem_SDL::toggleMouseGrab() {
	if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF)
		SDL_WM_GrabInput(SDL_GRAB_ON);
	else
		SDL_WM_GrabInput(SDL_GRAB_OFF);
}

void OSystem_SDL::undrawMouse() {
	const int x = _mouseBackup.x;
	const int y = _mouseBackup.y;

	// When we switch bigger overlay off mouse jumps. Argh!
	// This is intended to prevent undrawing offscreen mouse
	if (!_overlayVisible && (x >= _videoMode.screenWidth || y >= _videoMode.screenHeight))
		return;

	if (_mouseBackup.w != 0 && _mouseBackup.h != 0)
		addDirtyRect(x, y, _mouseBackup.w, _mouseBackup.h);
}

void OSystem_SDL::drawMouse() {
	if (!_mouseVisible || !_mouseSurface) {
		_mouseBackup.x = _mouseBackup.y = _mouseBackup.w = _mouseBackup.h = 0;
		return;
	}

	SDL_Rect dst;
	int scale;
	int width, height;
	int hotX, hotY;

	dst.x = _mouseCurState.x;
	dst.y = _mouseCurState.y;

	if (!_overlayVisible) {
		scale = _videoMode.scaleFactor;
		width = _videoMode.screenWidth;
		height = _videoMode.screenHeight;
		dst.w = _mouseCurState.vW;
		dst.h = _mouseCurState.vH;
		hotX = _mouseCurState.vHotX;
		hotY = _mouseCurState.vHotY;
	} else {
		scale = 1;
		width = _videoMode.overlayWidth;
		height = _videoMode.overlayHeight;
		dst.w = _mouseCurState.rW;
		dst.h = _mouseCurState.rH;
		hotX = _mouseCurState.rHotX;
		hotY = _mouseCurState.rHotY;
	}

	// The mouse is undrawn using virtual coordinates, i.e. they may be
	// scaled and aspect-ratio corrected.

	_mouseBackup.x = dst.x - hotX;
	_mouseBackup.y = dst.y - hotY;
	_mouseBackup.w = dst.w;
	_mouseBackup.h = dst.h;

	// We draw the pre-scaled cursor image, so now we need to adjust for
	// scaling, shake position and aspect ratio correction manually.

	if (!_overlayVisible) {
		dst.y += _currentShakePos;
	}

	if (_videoMode.aspectRatioCorrection && !_overlayVisible)
		dst.y = real2Aspect(dst.y);

	dst.x = scale * dst.x - _mouseCurState.rHotX;
	dst.y = scale * dst.y - _mouseCurState.rHotY;
	dst.w = _mouseCurState.rW;
	dst.h = _mouseCurState.rH;

	// Note that SDL_BlitSurface() and addDirtyRect() will both perform any
	// clipping necessary

	if (SDL_BlitSurface(_mouseSurface, NULL, _hwscreen, &dst) != 0)
		error("SDL_BlitSurface failed: %s", SDL_GetError());

	// The screen will be updated using real surface coordinates, i.e.
	// they will not be scaled or aspect-ratio corrected.

	addDirtyRect(dst.x, dst.y, dst.w, dst.h, true);
}

#pragma mark -
#pragma mark --- On Screen Display ---
#pragma mark -

#ifdef USE_OSD
void OSystem_SDL::displayMessageOnOSD(const char *msg) {
	assert (_transactionMode == kTransactionNone);
	assert(msg);

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	uint i;

	// Lock the OSD surface for drawing
	if (SDL_LockSurface(_osdSurface))
		error("displayMessageOnOSD: SDL_LockSurface failed: %s", SDL_GetError());

	Graphics::Surface dst;
	dst.pixels = _osdSurface->pixels;
	dst.w = _osdSurface->w;
	dst.h = _osdSurface->h;
	dst.pitch = _osdSurface->pitch;
	dst.bytesPerPixel = _osdSurface->format->BytesPerPixel;

	// The font we are going to use:
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kOSDFont);

	// Clear everything with the "transparent" color, i.e. the colorkey
	SDL_FillRect(_osdSurface, 0, kOSDColorKey);

	// Split the message into separate lines.
	Common::StringList lines;
	const char *ptr;
	for (ptr = msg; *ptr; ++ptr) {
		if (*ptr == '\n') {
			lines.push_back(Common::String(msg, ptr - msg));
			msg = ptr + 1;
		}
	}
	lines.push_back(Common::String(msg, ptr - msg));

	// Determine a rect which would contain the message string (clipped to the
	// screen dimensions).
	const int vOffset = 6;
	const int lineSpacing = 1;
	const int lineHeight = font->getFontHeight() + 2 * lineSpacing;
	int width = 0;
	int height = lineHeight * lines.size() + 2 * vOffset;
	for (i = 0; i < lines.size(); i++) {
		width = MAX(width, font->getStringWidth(lines[i]) + 14);
	}

	// Clip the rect
	if (width > dst.w)
		width = dst.w;
	if (height > dst.h)
		height = dst.h;

	// Draw a dark gray rect
	// TODO: Rounded corners ? Border?
	SDL_Rect osdRect;
	osdRect.x = (dst.w - width) / 2;
	osdRect.y = (dst.h - height) / 2;
	osdRect.w = width;
	osdRect.h = height;
	SDL_FillRect(_osdSurface, &osdRect, SDL_MapRGB(_osdSurface->format, 64, 64, 64));

	// Render the message, centered, and in white
	for (i = 0; i < lines.size(); i++) {
		font->drawString(&dst, lines[i],
							osdRect.x, osdRect.y + i * lineHeight + vOffset + lineSpacing, osdRect.w,
							SDL_MapRGB(_osdSurface->format, 255, 255, 255),
							Graphics::kTextAlignCenter);
	}

	// Finished drawing, so unlock the OSD surface again
	SDL_UnlockSurface(_osdSurface);

	// Init the OSD display parameters, and the fade out
	_osdAlpha = SDL_ALPHA_TRANSPARENT +  kOSDInitialAlpha * (SDL_ALPHA_OPAQUE - SDL_ALPHA_TRANSPARENT) / 100;
	_osdFadeStartTime = SDL_GetTicks() + kOSDFadeOutDelay;
	SDL_SetAlpha(_osdSurface, SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA, _osdAlpha);

	// Ensure a full redraw takes place next time the screen is updated
	_forceFull = true;
}
#endif


#pragma mark -
#pragma mark --- Misc ---
#pragma mark -

void OSystem_SDL::handleScalerHotkeys(const SDL_KeyboardEvent &key) {
	// Ctrl-Alt-a toggles aspect ratio correction
	if (key.keysym.sym == 'a') {
		beginGFXTransaction();
			setFeatureState(kFeatureAspectRatioCorrection, !_videoMode.aspectRatioCorrection);
		endGFXTransaction();
#ifdef USE_OSD
		char buffer[128];
		if (_videoMode.aspectRatioCorrection)
			sprintf(buffer, "Enabled aspect ratio correction\n%d x %d -> %d x %d",
				_videoMode.screenWidth, _videoMode.screenHeight,
				_hwscreen->w, _hwscreen->h
				);
		else
			sprintf(buffer, "Disabled aspect ratio correction\n%d x %d -> %d x %d",
				_videoMode.screenWidth, _videoMode.screenHeight,
				_hwscreen->w, _hwscreen->h
				);
		displayMessageOnOSD(buffer);
#endif
		internUpdateScreen();
		return;
	}

	int newMode = -1;
	int factor = _videoMode.scaleFactor - 1;

	// Increase/decrease the scale factor
	if (key.keysym.sym == SDLK_EQUALS || key.keysym.sym == SDLK_PLUS || key.keysym.sym == SDLK_MINUS ||
		key.keysym.sym == SDLK_KP_PLUS || key.keysym.sym == SDLK_KP_MINUS) {
		factor += (key.keysym.sym == SDLK_MINUS || key.keysym.sym == SDLK_KP_MINUS) ? -1 : +1;
		if (0 <= factor && factor <= 3) {
			newMode = s_gfxModeSwitchTable[_scalerType][factor];
		}
	}

	const bool isNormalNumber = (SDLK_1 <= key.keysym.sym && key.keysym.sym <= SDLK_9);
	const bool isKeypadNumber = (SDLK_KP1 <= key.keysym.sym && key.keysym.sym <= SDLK_KP9);
	if (isNormalNumber || isKeypadNumber) {
		_scalerType = key.keysym.sym - (isNormalNumber ? SDLK_1 : SDLK_KP1);
		if (_scalerType >= ARRAYSIZE(s_gfxModeSwitchTable))
			return;

		while (s_gfxModeSwitchTable[_scalerType][factor] < 0) {
			assert(factor > 0);
			factor--;
		}
		newMode = s_gfxModeSwitchTable[_scalerType][factor];
	}

	if (newMode >= 0) {
		beginGFXTransaction();
			setGraphicsMode(newMode);
		endGFXTransaction();
#ifdef USE_OSD
		if (_osdSurface) {
			const char *newScalerName = 0;
			const GraphicsMode *g = getSupportedGraphicsModes();
			while (g->name) {
				if (g->id == _videoMode.mode) {
					newScalerName = g->description;
					break;
				}
				g++;
			}
			if (newScalerName) {
				char buffer[128];
				sprintf(buffer, "Active graphics filter: %s\n%d x %d -> %d x %d",
					newScalerName,
					_videoMode.screenWidth, _videoMode.screenHeight,
					_hwscreen->w, _hwscreen->h
					);
				displayMessageOnOSD(buffer);
			}
		}
#endif
		internUpdateScreen();
	}

}
