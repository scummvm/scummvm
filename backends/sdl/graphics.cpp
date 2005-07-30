/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "backends/sdl/sdl-common.h"
#include "common/scaler.h"
#include "common/util.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
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
// [definedScale - 1][_scaleFactor - 1]
static ScalerProc *scalersMagn[3][3] = {
#ifndef __SYMBIAN32__
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

static int cursorStretch200To240(uint8 *buf, uint32 pitch, int width, int height, int srcX, int srcY, int origSrcY);

const OSystem::GraphicsMode *OSystem_SDL::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

int OSystem_SDL::getDefaultGraphicsMode() const {
	return GFX_DOUBLESIZE;
}

void OSystem_SDL::beginGFXTransaction(void) {
	assert (_transactionMode == kTransactionNone);

	_transactionMode = kTransactionActive;

	_transactionDetails.modeChanged = false;
	_transactionDetails.sizeChanged = false;
	_transactionDetails.arChanged = false;
	_transactionDetails.fsChanged = false;

	_transactionDetails.needHotswap = false;
	_transactionDetails.needUpdatescreen = false;
	_transactionDetails.needUnload = false;

	_transactionDetails.normal1xScaler = false;
}

void OSystem_SDL::endGFXTransaction(void) {
	// for each engine we run initCommonGFX() as first thing in the transaction
	// and initSize() is called later. If user runs launcher at 320x200 with
	// 2x overlay, setting to Nomral1x sclaler in that case will be suppressed
	// and backend is forced to 2x
	//
	// This leads to bad results such as 1280x960 window for 640x480 engines.
	// To prevent that we rerun setGraphicsMode() if there was 1x scaler request
	if (_transactionDetails.normal1xScaler)
		setGraphicsMode(GFX_NORMAL);

	assert (_transactionMode == kTransactionActive);

	_transactionMode = kTransactionCommit;
	if (_transactionDetails.modeChanged)
		setGraphicsMode(_transactionDetails.mode);

	if (_transactionDetails.sizeChanged)
		initSize(_transactionDetails.w, _transactionDetails.h,
				 _transactionDetails.overlayScale);

	if (_transactionDetails.arChanged)
		setAspectRatioCorrection(_transactionDetails.ar);

	if (_transactionDetails.needUnload) {
		unloadGFXMode();
		loadGFXMode();
		clearOverlay();
	} else {
		if (!_transactionDetails.fsChanged) {
			if (_transactionDetails.needHotswap)
				hotswapGFXMode();
			else if (_transactionDetails.needUpdatescreen)
				internUpdateScreen();
		}
	}

	if (_transactionDetails.fsChanged)
		setFullscreenMode(_transactionDetails.fs);

	_transactionMode = kTransactionNone;
}

bool OSystem_SDL::setGraphicsMode(int mode) {
	Common::StackLock lock(_graphicsMutex);

	int newScaleFactor = 1;
	ScalerProc *newScalerProc;

	switch(mode) {
	case GFX_NORMAL:
		newScaleFactor = 1;
		newScalerProc = Normal1x;
		break;
#ifndef __SYMBIAN32__ // remove dependencies on other scalers
	case GFX_DOUBLESIZE:
		newScaleFactor = 2;
		newScalerProc = Normal2x;
		break;
	case GFX_TRIPLESIZE:
		newScaleFactor = 3;
		newScalerProc = Normal3x;
		break;

	case GFX_2XSAI:
		newScaleFactor = 2;
		newScalerProc = _2xSaI;
		break;
	case GFX_SUPER2XSAI:
		newScaleFactor = 2;
		newScalerProc = Super2xSaI;
		break;
	case GFX_SUPEREAGLE:
		newScaleFactor = 2;
		newScalerProc = SuperEagle;
		break;
	case GFX_ADVMAME2X:
		newScaleFactor = 2;
		newScalerProc = AdvMame2x;
		break;
	case GFX_ADVMAME3X:
		newScaleFactor = 3;
		newScalerProc = AdvMame3x;
		break;
#ifndef DISABLE_HQ_SCALERS
	case GFX_HQ2X:
		newScaleFactor = 2;
		newScalerProc = HQ2x;
		break;
	case GFX_HQ3X:
		newScaleFactor = 3;
		newScalerProc = HQ3x;
		break;
#endif
	case GFX_TV2X:
		newScaleFactor = 2;
		newScalerProc = TV2x;
		break;
	case GFX_DOTMATRIX:
		newScaleFactor = 2;
		newScalerProc = DotMatrix;
		break;
#endif // __SYMBIAN32__

	default:
		warning("unknown gfx mode %d", mode);
		return false;
	}

	_transactionDetails.normal1xScaler = (mode == GFX_NORMAL);

#ifndef __SYMBIAN32__ // this code introduces a dependency on Normal2x()
	// Do not let switch to lesser than overlay size resolutions
	if (_screenWidth * newScaleFactor < _overlayWidth) {
		if (_scaleFactor == 1) { // Force 2x mode
			mode = GFX_DOUBLESIZE;
			newScaleFactor = 2;
			newScalerProc = Normal2x;
			_scaleFactor = 2;
		} else
			return false;
	}
#endif

	_mode = mode;
	_scalerProc = newScalerProc;

	if (_transactionMode == kTransactionActive) {
		_transactionDetails.mode = mode;
		_transactionDetails.modeChanged = true;

		if (newScaleFactor != _scaleFactor) {
			_transactionDetails.needHotswap = true;
			_scaleFactor = newScaleFactor;
		}

		_transactionDetails.needUpdatescreen = true;

		return true;
	}

	// NOTE: This should not be executed at transaction commit
	//   Otherwise there is some unsolicited setGraphicsMode() call
	//   which should be properly removed
	if (newScaleFactor != _scaleFactor) {
		assert(_transactionMode != kTransactionCommit);

		_scaleFactor = newScaleFactor;
		hotswapGFXMode();
	}

	// Determine the "scaler type", i.e. essentially an index into the
	// s_gfxModeSwitchTable array defined in events.cpp.
	if (_mode != GFX_NORMAL) {
		for (int i = 0; i < ARRAYSIZE(s_gfxModeSwitchTable); i++) {
			if (s_gfxModeSwitchTable[i][1] == _mode || s_gfxModeSwitchTable[i][2] == _mode) {
				_scalerType = i;
				break;
			}
		}
	}

	if (!_screen)
		return true;

	// Blit everything to the screen
	_forceFull = true;

	// Even if the old and new scale factors are the same, we may have a
	// different scaler for the cursor now.
	blitCursor();

	if (_transactionMode != kTransactionCommit)
		internUpdateScreen();

	// Make sure that an EVENT_SCREEN_CHANGED gets sent later
	_modeChanged = true;

	return true;
}

int OSystem_SDL::getGraphicsMode() const {
	assert (_transactionMode == kTransactionNone);
	return _mode;
}

void OSystem_SDL::initSize(uint w, uint h, int overlayScale) {
	// Avoid redundant res changes
	if ((int)w == _screenWidth && (int)h == _screenHeight &&
		 (int)overlayScale == _overlayScale &&
		_transactionMode != kTransactionCommit)
		return;

	_screenWidth = w;
	_screenHeight = h;

	if (h != 200)
		_adjustAspectRatio = false;

	if (overlayScale != -1) {
		_overlayScale = overlayScale;
		if (w != 320 && w != 256) // 256 is for MM NES
			_overlayScale = 1;

		_overlayWidth = w * _overlayScale;
		_overlayHeight = h * _overlayScale;
	}

	_cksumNum = (_screenWidth * _screenHeight / (8 * 8));

	if (_transactionMode == kTransactionActive) {
		_transactionDetails.w = w;
		_transactionDetails.h = h;
		_transactionDetails.overlayScale = _overlayScale;
		_transactionDetails.sizeChanged = true;

		_transactionDetails.needUnload = true;

		return;
	}

	free(_dirtyChecksums);
	_dirtyChecksums = (uint32 *)calloc(_cksumNum * 2, sizeof(uint32));

	if (_transactionMode != kTransactionCommit) {
		unloadGFXMode();
		loadGFXMode();

		// if initSize() gets called in the middle, overlay is not transparent
		clearOverlay();
	}
}

void OSystem_SDL::loadGFXMode() {
	assert(_inited);
	_forceFull = true;
	_modeFlags |= DF_UPDATE_EXPAND_1_PIXEL;

	//
	// Create the surface that contains the 8 bit game data
	//
	_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, _screenWidth, _screenHeight, 8, 0, 0, 0, 0);
	if (_screen == NULL)
		error("allocating _screen failed");

	//
	// Create the surface that contains the scaled graphics in 16 bit mode
	//

	_hwscreen = SDL_SetVideoMode(_screenWidth * _scaleFactor, effectiveScreenHeight(), 16,
		_fullscreen ? (SDL_FULLSCREEN|SDL_SWSURFACE) : SDL_SWSURFACE
	);
	if (_hwscreen == NULL) {
		// DON'T use error(), as this tries to bring up the debug
		// console, which WON'T WORK now that _hwscreen is hosed.

		// FIXME: We should be able to continue the game without
		// shutting down or bringing up the debug console, but at
		// this point we've already screwed up all our member vars.
		// We need to find a way to call SDL_SetVideoMode *before*
		// that happens and revert to all the old settings if we
		// can't pull off the switch to the new settings.
		//
		// Fingolfin says: the "easy" way to do that is not to modify
		// the member vars before we are sure everything is fine. Think
		// of "transactions, commit, rollback" style... we use local vars
		// in place of the member vars, do everything etc. etc.. In case
		// of a failure, rollback is trivial. Only if everything worked fine
		// do we "commit" the changed values to the member vars.
		warning("SDL_SetVideoMode says we can't switch to that mode");
		quit();
	}

	//
	// Create the surface used for the graphics in 16 bit before scaling, and also the overlay
	//

	// Distinguish 555 and 565 mode
	if (_hwscreen->format->Rmask == 0x7C00)
		InitScalers(555);
	else
		InitScalers(565);

	// Need some extra bytes around when using 2xSaI
	_tmpscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _screenWidth + 3, _screenHeight + 3,
						16,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

	if (_tmpscreen == NULL)
		error("allocating _tmpscreen failed");

	_overlayscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _overlayWidth, _overlayHeight,
						16,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

	if (_overlayscreen == NULL)
		error("allocating _overlayscreen failed");

	_tmpscreen2 = SDL_CreateRGBSurface(SDL_SWSURFACE, _overlayWidth + 3, _overlayHeight + 3,
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
	_km.x_max = _screenWidth * _scaleFactor - 1;
	_km.y_max = effectiveScreenHeight() - 1;
	_km.delay_time = 25;
	_km.last_time = 0;
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
}

void OSystem_SDL::hotswapGFXMode() {
	if (!_screen)
		return;

	// Keep around the old _screen & _overlayscreen so we can restore the screen data
	// after the mode switch.
	SDL_Surface *old_screen = _screen;
	SDL_Surface *old_overlayscreen = _overlayscreen;

	// Release the HW screen surface
	SDL_FreeSurface(_hwscreen);

	SDL_FreeSurface(_tmpscreen);
	SDL_FreeSurface(_tmpscreen2);

#ifdef USE_OSD
	// Release the OSD surface
	SDL_FreeSurface(_osdSurface);
#endif

	// Setup the new GFX mode
	loadGFXMode();

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

	// Make sure that an EVENT_SCREEN_CHANGED gets sent later
	_modeChanged = true;
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
	int scale1, scale2;

#if defined (DEBUG) && ! defined(_WIN32_WCE) // definitions not available for non-DEBUG here. (needed this to compile in SYMBIAN32 & linux?)
	assert(_hwscreen != NULL);
	assert(_hwscreen->map->sw_data != NULL);
#endif

	// If the shake position changed, fill the dirty area with blackness
	if (_currentShakePos != _newShakePos) {
		SDL_Rect blackrect = {0, 0, _screenWidth * _scaleFactor, _newShakePos * _scaleFactor};

		if (_adjustAspectRatio)
			blackrect.h = real2Aspect(blackrect.h - 1) + 1;

		SDL_FillRect(_hwscreen, &blackrect, 0);

		_currentShakePos = _newShakePos;

		_forceFull = true;
	}

	// Check whether the palette was changed in the meantime and update the
	// screen surface accordingly.
	if (_paletteDirtyEnd != 0) {
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
		width = _screenWidth;
		height = _screenHeight;
		scalerProc = _scalerProc;
		scale1 = _scaleFactor;
		scale2 = 1;
	} else {
		origSurf = _overlayscreen;
		srcSurf = _tmpscreen2;
		width = _overlayWidth;
		height = _overlayHeight;
		scalerProc = scalersMagn[_overlayScale - 1][_scaleFactor - 1];

		scale1 = _scaleFactor;
		scale2 = _overlayScale;
	}

	// Force a full redraw if requested
	if (_forceFull) {
		_numDirtyRects = 1;
		_dirtyRectList[0].x = 0;
		_dirtyRectList[0].y = 0;
		_dirtyRectList[0].w = width;
		_dirtyRectList[0].h = height;
	} else
		undrawMouse();

	// Only draw anything if necessary
	if (_numDirtyRects > 0) {

		SDL_Rect *r;
		SDL_Rect dst;
		uint32 srcPitch, dstPitch;
		SDL_Rect *lastRect = _dirtyRectList + _numDirtyRects;

		if (scalerProc == Normal1x && !_adjustAspectRatio && 0) {
			for (r = _dirtyRectList; r != lastRect; ++r) {
				dst = *r;

				dst.y += _currentShakePos;
				if (SDL_BlitSurface(origSurf, r, _hwscreen, &dst) != 0)
					error("SDL_BlitSurface failed: %s", SDL_GetError());
			}
		} else {
			for (r = _dirtyRectList; r != lastRect; ++r) {
				dst = *r;
				dst.x++;	// Shift rect by one since 2xSai needs to acces the data around
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
				register int rx1 = r->x * scale1 / scale2;

				if (dst_y < height) {
					dst_h = r->h;
					if (dst_h > height - dst_y)
						dst_h = height - dst_y;

					orig_dst_y = dst_y;
					dst_y = dst_y * scale1 / scale2;

					if (_adjustAspectRatio)
						dst_y = real2Aspect(dst_y);

					if (scale1 == 3 && scale2 == 2 && _overlayVisible) {
						if (r->y % 2)
							r->y--;
						dst_y -= dst_y % 3;
					}

					assert(scalerProc != NULL);
					scalerProc((byte *)srcSurf->pixels + (r->x * 2 + 2) + (r->y + 1) * srcPitch, srcPitch,
							   (byte *)_hwscreen->pixels + rx1 * 2 + dst_y * dstPitch, dstPitch, r->w, dst_h);
				}

				r->x = rx1;
				r->y = dst_y;
				r->w = r->w * scale1 / scale2;
				r->h = dst_h * scale1 / scale2;

#ifndef __SYMBIAN32__ // don't want to introduce dep on aspect.cpp
				if (_adjustAspectRatio && orig_dst_y < height)
					r->h = stretch200To240((uint8 *) _hwscreen->pixels, dstPitch, r->w, r->h, r->x, r->y, orig_dst_y * scale1 / scale2);
#endif
			}
			SDL_UnlockSurface(srcSurf);
			SDL_UnlockSurface(_hwscreen);
		}

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
	} else {
		drawMouse();
		if (_numDirtyRects)
			SDL_UpdateRects(_hwscreen, _numDirtyRects, _dirtyRectList);
	}

	_numDirtyRects = 0;
	_forceFull = false;
}

bool OSystem_SDL::saveScreenshot(const char *filename) {
	assert(_hwscreen != NULL);

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends
	return SDL_SaveBMP(_hwscreen, filename) == 0;
}

void OSystem_SDL::setFullscreenMode(bool enable) {
	Common::StackLock lock(_graphicsMutex);

	if (_fullscreen != enable || _transactionMode == kTransactionCommit) {
		assert(_hwscreen != 0);
		_fullscreen = enable;

		if (_transactionMode == kTransactionActive) {
			_transactionDetails.fs = enable;
			_transactionDetails.fsChanged = true;

			_transactionDetails.needHotswap = true;

			return;
		}

#if defined(MACOSX) && !SDL_VERSION_ATLEAST(1, 2, 6)
		// On OS X, SDL_WM_ToggleFullScreen is currently not implemented. Worse,
		// before SDL 1.2.6 it always returned -1 (which would indicate a
		// successful switch). So we simply don't call it at all and use
		// hotswapGFXMode() directly to switch to fullscreen mode.
		hotswapGFXMode();
#else
		if (!SDL_WM_ToggleFullScreen(_hwscreen)) {
			// if ToggleFullScreen fails, achieve the same effect with hotswap gfx mode
			hotswapGFXMode();
		} else {
			// Blit everything to the screen
			internUpdateScreen();

			// Make sure that an EVENT_SCREEN_CHANGED gets sent later
			_modeChanged = true;
		}
#endif
	}
}

void OSystem_SDL::setAspectRatioCorrection(bool enable) {
	if ((_screenHeight == 200 && _adjustAspectRatio != enable) ||
		_transactionMode == kTransactionCommit) {
		Common::StackLock lock(_graphicsMutex);

		//assert(_hwscreen != 0);
		_adjustAspectRatio = enable;

		if (_transactionMode == kTransactionActive) {
			_transactionDetails.ar = enable;
			_transactionDetails.arChanged = true;

			_transactionDetails.needHotswap = true;

			return;
		} else {
			if (_transactionMode != kTransactionCommit)
				hotswapGFXMode();
		}

		// Make sure that an EVENT_SCREEN_CHANGED gets sent later
		_modeChanged = true;
	}
}

void OSystem_SDL::clearScreen() {
	assert (_transactionMode == kTransactionNone);

	// Try to lock the screen surface
	if (SDL_LockSurface(_screen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *dst = (byte *)_screen->pixels;

	// Clear the screen
	memset(dst, 0, _screenWidth * _screenHeight);

	// Unlock the screen surface
	SDL_UnlockSurface(_screen);
}

void OSystem_SDL::copyRectToScreen(const byte *src, int pitch, int x, int y, int w, int h) {
	assert (_transactionMode == kTransactionNone);
	assert(src);

	if (_screen == NULL)
		return;

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	if (((long)src & 3) == 0 && pitch == _screenWidth && x == 0 && y == 0 &&
			w == _screenWidth && h == _screenHeight && _modeFlags & DF_WANT_RECT_OPTIM) {
		/* Special, optimized case for full screen updates.
		 * It tries to determine what areas were actually changed,
		 * and just updates those, on the actual display. */
		addDirtyRgnAuto(src);
	} else {
		/* Clip the coordinates */
		if (x < 0) {
			w += x;
			src -= x;
			x = 0;
		}

		if (y < 0) {
			h += y;
			src -= y * pitch;
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

		_cksumValid = false;
		addDirtyRect(x, y, w, h);
	}

	// Try to lock the screen surface
	if (SDL_LockSurface(_screen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *dst = (byte *)_screen->pixels + y * _screenWidth + x;

	if (_screenWidth == pitch && pitch == w) {
		memcpy(dst, src, h*w);
	} else {
		do {
			memcpy(dst, src, w);
			src += pitch;
			dst += _screenWidth;
		} while (--h);
	}

	// Unlock the screen surface
	SDL_UnlockSurface(_screen);
}

bool OSystem_SDL::grabRawScreen(Graphics::Surface *surf) {
	assert(_screen);
	assert(surf);

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	surf->create(_screenWidth, _screenHeight, _screen->format->BytesPerPixel);

	// Try to lock the screen surface
	if (SDL_LockSurface(_screen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	memcpy(surf->pixels, _screen->pixels, _screenWidth * _screenHeight * _screen->format->BytesPerPixel);

	// Unlock the screen surface
	SDL_UnlockSurface(_screen);

	return true;
}

void OSystem_SDL::addDirtyRect(int x, int y, int w, int h, bool mouseRect) {
	if (_forceFull)
		return;

	if (_numDirtyRects == NUM_DIRTY_RECT) {
		_forceFull = true;
		return;
	}

	SDL_Rect *r = &_dirtyRectList[_numDirtyRects++];

	if (mouseRect) {
		r->x = x;
		r->y = y;
		r->w = w;
		r->h = h;
		return;
	}

	int height, width;

	if (!_overlayVisible) {
		width = _screenWidth;
		height = _screenHeight;
	} else {
		width = _overlayWidth;
		height = _overlayHeight;
	}

	// Extend the dirty region by 1 pixel for scalers
	// that "smear" the screen, e.g. 2xSAI
	if (_modeFlags & DF_UPDATE_EXPAND_1_PIXEL) {
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

#ifndef __SYMBIAN32__  // don't want to introduce dep on aspect.cpp
	if (_adjustAspectRatio) {
		makeRectStretchable(x, y, w, h);
		if (_scaleFactor == 3 && _overlayScale == 2 && _overlayVisible) {
			if (y % 2)
				y++;
		}
	}
#endif

	r->x = x;
	r->y = y;
	r->w = w;
	r->h = h;
}


void OSystem_SDL::makeChecksums(const byte *buf) {
	assert(buf);
	uint32 *sums = _dirtyChecksums;
	uint x,y;
	const uint last_x = (uint)_screenWidth / 8;
	const uint last_y = (uint)_screenHeight / 8;

	const uint BASE = 65521; /* largest prime smaller than 65536 */

	/* the 8x8 blocks in buf are enumerated starting in the top left corner and
	 * reading each line at a time from left to right */
	for(y = 0; y != last_y; y++, buf += _screenWidth * (8 - 1))
		for(x = 0; x != last_x; x++, buf += 8) {
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
				ptr += _screenWidth;
			}

			s1 %= BASE;
			s2 %= BASE;

			/* output the checksum for this block */
			*sums++ =  (s2 << 16) + s1;
	}
}

void OSystem_SDL::addDirtyRgnAuto(const byte *buf) {
	assert(buf);
	assert(((long)buf & 3) == 0);

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

		for(y = 0; y != _screenHeight / 8; y++) {
			for(x = 0; x != _screenWidth / 8; x++, ck++) {
				if (ck[0] != ck[_cksumNum]) {
					/* found a dirty 8x8 block, now go as far to the right as possible,
						 and at the same time, unmark the dirty status by setting old to new. */
					w=0;
					do {
						ck[w + _cksumNum] = ck[w];
						w++;
					} while (x + w != _screenWidth / 8 && ck[w] != ck[w + _cksumNum]);

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
	return _screenHeight;
}

int16 OSystem_SDL::getWidth() {
	return _screenWidth;
}

void OSystem_SDL::setPalette(const byte *colors, uint start, uint num) {
	assert(colors);
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
	if (!_overlayVisible && (!_cursorHasOwnPalette || _cursorPaletteDisabled))
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

	_cursorHasOwnPalette = true;
	_cursorPaletteDisabled = false;

	if (!_overlayVisible)
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

	_overlayVisible = true;
	clearOverlay();
}

void OSystem_SDL::hideOverlay() {
	assert (_transactionMode == kTransactionNone);

	_overlayVisible = false;
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
	src.w = dst.w = _screenWidth;
	src.h = dst.h = _screenHeight;
	if (SDL_BlitSurface(_screen, &src, _tmpscreen, &dst) != 0)
		error("SDL_BlitSurface failed: %s", SDL_GetError());

	SDL_LockSurface(_tmpscreen);
	SDL_LockSurface(_overlayscreen);
	if (_overlayScale == _scaleFactor) {
		_scalerProc((byte *)(_tmpscreen->pixels) + _tmpscreen->pitch + 2,
					_tmpscreen->pitch, (byte *)_overlayscreen->pixels, _overlayscreen->pitch, _screenWidth, _screenHeight);
	} else {
		// Quality is degraded here. It is possible to run one-less scaler here, but is it
		// really needed? Quality will anyway be degraded because of 1.5x scaler.
		(scalersMagn[0][_overlayScale - 1])((byte *)(_tmpscreen->pixels) + _tmpscreen->pitch + 2,
					  _tmpscreen->pitch, (byte *)_overlayscreen->pixels, _overlayscreen->pitch, _screenWidth, _screenHeight);
	}
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
	int h = _overlayHeight;
	do {
		memcpy(buf, src, _overlayWidth * 2);
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

	if (w > _overlayWidth - x) {
		w = _overlayWidth - x;
	}

	if (h > _overlayHeight - y) {
		h = _overlayHeight - y;
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

OverlayColor OSystem_SDL::RGBToColor(uint8 r, uint8 g, uint8 b) {
	return SDL_MapRGB(_overlayscreen->format, r, g, b);
}

void OSystem_SDL::colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b) {
	SDL_GetRGB(color, _overlayscreen->format, &r, &g, &b);
}


#pragma mark -
#pragma mark --- Mouse ---
#pragma mark -

bool OSystem_SDL::showMouse(bool visible) {
	if (_mouseVisible == visible)
		return visible;

	bool last = _mouseVisible;
	_mouseVisible = visible;

	updateScreen();

	return last;
}

void OSystem_SDL::setMousePos(int x, int y) {
	if (x != _mouseCurState.x || y != _mouseCurState.y) {
		_mouseCurState.x = x;
		_mouseCurState.y = y;
		updateScreen();
	}
}

void OSystem_SDL::warpMouse(int x, int y) {
	if (_mouseCurState.x != x || _mouseCurState.y != y) {
		if (_overlayVisible)
			SDL_WarpMouse(x * _scaleFactor / _overlayScale, y * _scaleFactor / _overlayScale);
		else
			SDL_WarpMouse(x * _scaleFactor, y * _scaleFactor);

		// SDL_WarpMouse() generates a mouse movement event, so
		// setMousePos() would be called eventually. However, the
		// cannon script in CoMI calls this function twice each time
		// the cannon is reloaded. Unless we update the mouse position
		// immediately the second call is ignored, causing the cannon
		// to change its aim.

		setMousePos(x, y);
	}
}

void OSystem_SDL::setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, byte keycolor, int cursorTargetScale) {
	if (w == 0 || h == 0)
		return;

	_mouseHotspotX = hotspot_x;
	_mouseHotspotY = hotspot_y;

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

	_mouseData = (byte *)malloc(w * h);
	memcpy(_mouseData, buf, w * h);
	blitCursor();
}

void OSystem_SDL::blitCursor() {
	byte *dstPtr;
	const byte *srcPtr = _mouseData;
	byte color;
	int w, h, i, j;

	if (!_mouseOrigSurface || !_mouseData)
 		return;

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

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			color = *srcPtr;
			if (color != _mouseKeyColor) {	// transparent, don't draw
				if (_cursorHasOwnPalette && !_overlayVisible && !_cursorPaletteDisabled)
					*(uint16 *)dstPtr = SDL_MapRGB(_mouseOrigSurface->format,
								   _cursorPalette[color].r, _cursorPalette[color].g,
															   _cursorPalette[color].b);
				else
					*(uint16 *)dstPtr = SDL_MapRGB(_mouseOrigSurface->format,
								   _currentPalette[color].r, _currentPalette[color].g,
												   				_currentPalette[color].b);
			}
			dstPtr += 2;
			srcPtr++;
		}
		dstPtr += _mouseOrigSurface->pitch - w * 2;
  	}

	int hW, hH, hH1;

	if (_cursorTargetScale >= _scaleFactor) {
		hW = w;
		hH = hH1 = h;
	} else {
		hW = w * _scaleFactor / _cursorTargetScale;
		hH = hH1 = h * _scaleFactor / _cursorTargetScale;
  	}

	if (_adjustAspectRatio) {
		hH = real2Aspect(hH - 1) + 1;
	}

	if (_mouseCurState.hW != hW || _mouseCurState.hH != hH) {
		_mouseCurState.hW = hW;
		_mouseCurState.hH = hH;

		if (_mouseSurface)
			SDL_FreeSurface(_mouseSurface);

		_mouseSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA,
						_mouseCurState.hW,
						_mouseCurState.hH,
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

	if (_cursorTargetScale == 1 && (_mode == GFX_DOUBLESIZE || _mode == GFX_TRIPLESIZE))
		scalerProc = _scalerProc;
	else
		scalerProc = scalersMagn[_cursorTargetScale - 1][_scaleFactor - 1];

	scalerProc((byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch + 2,
		_mouseOrigSurface->pitch, (byte *)_mouseSurface->pixels, _mouseSurface->pitch,
		_mouseCurState.w, _mouseCurState.h);

	if (_adjustAspectRatio)
		cursorStretch200To240((uint8 *)_mouseSurface->pixels, _mouseSurface->pitch, hW, hH1, 0, 0, 0);

	SDL_UnlockSurface(_mouseSurface);
	SDL_UnlockSurface(_mouseOrigSurface);
}

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

void OSystem_SDL::toggleMouseGrab() {
	if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF)
		SDL_WM_GrabInput(SDL_GRAB_ON);
	else
		SDL_WM_GrabInput(SDL_GRAB_OFF);
}

void OSystem_SDL::undrawMouse() {
	const int x = _mouseBackup.x;
	const int y = _adjustAspectRatio ? aspect2Real(_mouseBackup.y) : _mouseBackup.y;

	// When we switch bigger overlay off mouse jumps. Argh!
	// This is intended to prevent undrawing offscreen mouse
	if (!_overlayVisible && (x >= _screenWidth || y >= _screenHeight)) {
		return;
	}

	if (_mouseBackup.w != 0 && _mouseBackup.h != 0) {
		addDirtyRect(x, y, _mouseBackup.w, _mouseBackup.h);
	}
}

void OSystem_SDL::drawMouse() {
	if (!_mouseVisible) {
		_mouseBackup.x = _mouseBackup.y = _mouseBackup.w = _mouseBackup.h = 0;
  		return;
	}

	SDL_Rect src, dst;
	int scale1, scale2;
	int width, height;

	if (!_overlayVisible) {
		scale1 = _scaleFactor;
		scale2 = 1;
		width = _screenWidth;
		height = _screenHeight;
	} else {
		scale1 = _scaleFactor;
		scale2 = _overlayScale;
		width = _overlayWidth;
		height = _overlayHeight;
	}

	// Note: _mouseCurState is in *virtual* coordinates. That means if initSize
	// was used to set a 320x200 game screen, then _mouseCurState is bounded
	// by that; if the overlay is visible, then it is bounded by the overlay
	// size (typically 640x400).
	//
	// _mouseBackup is stored in "virtual" coordinates, too.
	//
	// On the other hand, _mouseSurface was scaled to screen coordinates in
	// OSystem_SDL::blitCursor(), including aspect ratio corection. Its
	// scaling is determined by _scaleFactor, but *not* by whatever value
	// _overlayScale has.
	// Atop of that, it was scaled by _cursorTargetScale.
	// That means, we have to scale it by "scale2".
	//
	// TODO/FIXME: Clean up and fix this code, see also bug #1185275.

	const bool useScaling = (_scaleFactor > _cursorTargetScale);

	dst.x = _mouseCurState.x - _mouseHotspotX * scale2 / _cursorTargetScale;
	dst.y = _mouseCurState.y - _mouseHotspotY * scale2 / _cursorTargetScale;

	dst.w = _mouseCurState.hW;
	dst.h = _mouseCurState.hH;
	src.x = src.y = 0;

	// clip the mouse rect, and adjust the src pointer accordingly
	if (dst.x < 0) {
		const int dx = useScaling ? dst.x * scale1 / scale2 / _cursorTargetScale : dst.x;
		dst.w += dx;
		src.x -= dx;
		dst.x = 0;
	}
	if (dst.y < 0) {
		const int dy = useScaling ? dst.y * scale1 / scale2 / _cursorTargetScale : dst.y;
		dst.h += dy;
		src.y -= dy;
		dst.y = 0;
	}

  	// Quick check to see if anything has to be drawn at all
	if (dst.w <= 0 || dst.h <= 0 || dst.x >= width || dst.y >= height)
  		return;

	src.w = dst.w;
	src.h = dst.h;

	if (_adjustAspectRatio)
		dst.y = real2Aspect(dst.y);

	// special case for 1o5x scaler to prevent backgound shaking
	if (scale1 == 3 && scale2 == 2) {
		if (dst.x % 2)
			dst.x--;
		if (dst.y % 2)
			dst.y--;
	}

	_mouseBackup = dst;

	dst.x = dst.x * scale1 / scale2;
	dst.y = (dst.y + _currentShakePos) * scale1 / scale2;

	if (SDL_BlitSurface(_mouseSurface, &src, _hwscreen, &dst) != 0)
		error("SDL_BlitSurface failed: %s", SDL_GetError());

	addDirtyRect(dst.x, dst.y, dst.w, dst.h, true);
}

#pragma mark -
#pragma mark --- On Screen Display ---
#pragma mark -

#ifdef USE_OSD
void OSystem_SDL::displayMessageOnOSD(const char *msg) {
	assert (_transactionMode == kTransactionNone);
	assert(msg);

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
		setFeatureState(kFeatureAspectRatioCorrection, !_adjustAspectRatio);
#ifdef USE_OSD
		char buffer[128];
		if (_adjustAspectRatio)
			sprintf(buffer, "Enabled aspect ratio correction\n%d x %d -> %d x %d",
				_screenWidth, _screenHeight,
				_hwscreen->w, _hwscreen->h
				);
		else
			sprintf(buffer, "Disabled aspect ratio correction\n%d x %d -> %d x %d",
				_screenWidth, _screenHeight,
				_hwscreen->w, _hwscreen->h
				);
		displayMessageOnOSD(buffer);
#endif

		return;
	}

	int newMode = -1;
	int factor = _scaleFactor - 1;

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
		setGraphicsMode(newMode);
#ifdef USE_OSD
		if (_osdSurface) {
			const char *newScalerName = 0;
			const GraphicsMode *g = getSupportedGraphicsModes();
			while (g->name) {
				if (g->id == _mode) {
					newScalerName = g->description;
					break;
				}
				g++;
			}
			if (newScalerName) {
				char buffer[128];
				sprintf(buffer, "Active graphics filter: %s\n%d x %d -> %d x %d",
					newScalerName,
					_screenWidth, _screenHeight,
					_hwscreen->w, _hwscreen->h
					);
				displayMessageOnOSD(buffer);
			}
		}
#endif

	}

}
