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

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{"1x", "Normal (no scaling)", GFX_NORMAL},
	{"2x", "2x", GFX_DOUBLESIZE},
	{"3x", "3x", GFX_TRIPLESIZE},
	{"2xsai", "2xSAI", GFX_2XSAI},
	{"super2xsai", "Super2xSAI", GFX_SUPER2XSAI},
	{"supereagle", "SuperEagle", GFX_SUPEREAGLE},
	{"advmame2x", "AdvMAME2x", GFX_ADVMAME2X},
	{"advmame3x", "AdvMAME3x", GFX_ADVMAME3X},
	{"hq2x", "HQ2x", GFX_HQ2X},
	{"hq3x", "HQ3x", GFX_HQ3X},
	{"tv2x", "TV2x", GFX_TV2X},
	{"dotmatrix", "DotMatrix", GFX_DOTMATRIX},
	{0, 0, 0}
};

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
	_transactionDetails.wChanged = false;
	_transactionDetails.hChanged = false;
	_transactionDetails.arChanged = false;
	_transactionDetails.fsChanged = false;
}

void OSystem_SDL::endGFXTransaction(void) {
	assert (_transactionMode == kTransactionActive);

	_transactionMode = kTransactionCommit;
	if (_transactionDetails.modeChanged) {
		setGraphicsMode(_transactionDetails.mode);
	}
	if (_transactionDetails.wChanged || _transactionDetails.hChanged) {
		initSize(_transactionDetails.w, _transactionDetails.h);
	}
	if (_transactionDetails.arChanged) {
		setAspectRatioCorrection(_transactionDetails.ar);
	}
	if (_transactionDetails.fsChanged) {
		setFullscreenMode(_transactionDetails.fs);
	}
	_transactionMode = kTransactionNone;
}

bool OSystem_SDL::setGraphicsMode(int mode) {
	Common::StackLock lock(_graphicsMutex);

	int newScaleFactor = 1;
	ScalerProc *newScalerProc;

	switch (_transactionMode) {
	case kTransactionActive:
		_transactionDetails.mode = mode;
		_transactionDetails.modeChanged = true;
		return true;
		break;
	case kTransactionCommit:
		break;
	default:
		break;
	} 

	switch(mode) {
	case GFX_NORMAL:
		newScaleFactor = 1;
		newScalerProc = Normal1x;
		break;
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
	case GFX_HQ2X:
		newScaleFactor = 2;
		newScalerProc = HQ2x;
		break;
	case GFX_HQ3X:
		newScaleFactor = 3;
		newScalerProc = HQ3x;
		break;
	case GFX_TV2X:
		newScaleFactor = 2;
		newScalerProc = TV2x;
		break;
	case GFX_DOTMATRIX:
		newScaleFactor = 2;
		newScalerProc = DotMatrix;
		break;

	default:
		warning("unknown gfx mode %d", mode);
		return false;
	}

	_mode = mode;
	_scalerProc = newScalerProc;
	if (newScaleFactor != _scaleFactor) {
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
	internUpdateScreen();
	
	// Make sure that an EVENT_SCREEN_CHANGED gets sent later
	_modeChanged = true;

	return true;
}

int OSystem_SDL::getGraphicsMode() const {
	assert (_transactionMode == kTransactionNone);
	return _mode;
}

void OSystem_SDL::initSize(uint w, uint h) {
	switch (_transactionMode) {
	case kTransactionActive:
		_transactionDetails.w = w;
		_transactionDetails.wChanged = true;
		_transactionDetails.h = h;
		_transactionDetails.hChanged = true;
		return;
		break;
	case kTransactionCommit:
		break;
	default:
		break;
	} 

	// Avoid redundant res changes
	if ((int)w == _screenWidth && (int)h == _screenHeight)
		return;

	_screenWidth = w;
	_screenHeight = h;

	if (h != 200)
		_adjustAspectRatio = false;

	_cksumNum = (_screenWidth * _screenHeight / (8 * 8));

	free(_dirtyChecksums);
	_dirtyChecksums = (uint32 *)calloc(_cksumNum * 2, sizeof(uint32));

	_mouseData = NULL;

	unloadGFXMode();
	loadGFXMode();
}

void OSystem_SDL::loadGFXMode() {
	_forceFull = true;
	_modeFlags |= DF_UPDATE_EXPAND_1_PIXEL;

	_tmpscreen = NULL;
	
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
	_tmpscreen = SDL_CreateRGBSurface(SDL_SWSURFACE,
						_screenWidth + 3,
						_screenHeight + 3,
						16,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

	if (_tmpscreen == NULL)
		error("allocating _tmpscreen failed");
	
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

	// Keep around the old _screen & _tmpscreen so we can restore the screen data
	// after the mode switch.
	SDL_Surface *old_screen = _screen;
	SDL_Surface *old_tmpscreen = _tmpscreen;

	// Release the HW screen surface
	SDL_FreeSurface(_hwscreen); 

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
	SDL_BlitSurface(old_tmpscreen, NULL, _tmpscreen, NULL);
	
	// Free the old surfaces
	SDL_FreeSurface(old_screen);
	SDL_FreeSurface(old_tmpscreen);

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
	assert(_hwscreen != NULL);

	// If the shake position changed, fill the dirty area with blackness
	if (_currentShakePos != _newShakePos) {
		SDL_Rect blackrect = {0, 0, _screenWidth * _scaleFactor, _newShakePos * _scaleFactor};

		if (_adjustAspectRatio)
			blackrect.h = real2Aspect(blackrect.h - 1) + 1;

		SDL_FillRect(_hwscreen, &blackrect, 0);

		_currentShakePos = _newShakePos;

		_forceFull = true;
	}

	// Make sure the mouse is drawn, if it should be drawn.
	drawMouse();
	
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

	// Force a full redraw if requested
	if (_forceFull) {
		_numDirtyRects = 1;
		_dirtyRectList[0].x = 0;
		_dirtyRectList[0].y = 0;
		_dirtyRectList[0].w = _screenWidth;
		_dirtyRectList[0].h = _screenHeight;
	}

	// Only draw anything if necessary
	if (_numDirtyRects > 0) {

		SDL_Rect *r; 
		SDL_Rect dst;
		uint32 srcPitch, dstPitch;
		SDL_Rect *lastRect = _dirtyRectList + _numDirtyRects;

		if (_scalerProc == Normal1x && !_adjustAspectRatio) {
			SDL_Surface *target = _overlayVisible ? _tmpscreen : _screen;
			for (r = _dirtyRectList; r != lastRect; ++r) {
				dst = *r;
				
				if (_overlayVisible) {
					// FIXME: I don't understand why this is necessary...
					dst.x--;
					dst.y--;
				}
				dst.y += _currentShakePos;
				if (SDL_BlitSurface(target, r, _hwscreen, &dst) != 0)
					error("SDL_BlitSurface failed: %s", SDL_GetError());
			}
		} else {
			if (!_overlayVisible) {
				for (r = _dirtyRectList; r != lastRect; ++r) {
					dst = *r;
					dst.x++;	// Shift rect by one since 2xSai needs to acces the data around
					dst.y++;	// any pixel to scale it, and we want to avoid mem access crashes.
					if (SDL_BlitSurface(_screen, r, _tmpscreen, &dst) != 0)
						error("SDL_BlitSurface failed: %s", SDL_GetError());
				}
			}

			SDL_LockSurface(_tmpscreen);
			SDL_LockSurface(_hwscreen);

			srcPitch = _tmpscreen->pitch;
			dstPitch = _hwscreen->pitch;

			for (r = _dirtyRectList; r != lastRect; ++r) {
				register int dst_y = r->y + _currentShakePos;
				register int dst_h = 0;
				register int orig_dst_y = 0;

				if (dst_y < _screenHeight) {
					dst_h = r->h;
					if (dst_h > _screenHeight - dst_y)
						dst_h = _screenHeight - dst_y;

					dst_y *= _scaleFactor;

					if (_adjustAspectRatio) {
						orig_dst_y = dst_y;
						dst_y = real2Aspect(dst_y);
					}

					_scalerProc((byte *)_tmpscreen->pixels + (r->x * 2 + 2) + (r->y + 1) * srcPitch, srcPitch,
						(byte *)_hwscreen->pixels + r->x * 2 * _scaleFactor + dst_y * dstPitch, dstPitch, r->w, dst_h);
				}

				r->x *= _scaleFactor;
				r->y = dst_y;
				r->w *= _scaleFactor;
				r->h = dst_h * _scaleFactor;

				if (_adjustAspectRatio && orig_dst_y / _scaleFactor < _screenHeight)
					r->h = stretch200To240((uint8 *) _hwscreen->pixels, dstPitch, r->w, r->h, r->x, r->y, orig_dst_y);
			}
			SDL_UnlockSurface(_tmpscreen);
			SDL_UnlockSurface(_hwscreen);
		}

		// Readjust the dirty rect list in case we are doing a full update.
		// This is necessary if shaking is active.
		if (_forceFull) {
			_dirtyRectList[0].y = 0;
			_dirtyRectList[0].h = effectiveScreenHeight();
		}

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
}

bool OSystem_SDL::saveScreenshot(const char *filename) {
	assert(_hwscreen != NULL);

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends
	return SDL_SaveBMP(_hwscreen, filename) == 0;
}

void OSystem_SDL::setFullscreenMode(bool enable) {
	switch (_transactionMode) {
	case kTransactionActive:
		_transactionDetails.fs = enable;
		_transactionDetails.fsChanged = true;
		return;
		break;
	case kTransactionCommit:
		break;
	default:
		break;
	} 

	Common::StackLock lock(_graphicsMutex);

	if (_fullscreen != enable) {
		assert(_hwscreen != 0);
		_fullscreen ^= true;
		
		undrawMouse();

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
	switch (_transactionMode) {
	case kTransactionActive:
		_transactionDetails.ar = enable;
		_transactionDetails.arChanged = true;
		return;
		break;
	case kTransactionCommit:
		break;
	default:
		break;
	} 

	if (_screenHeight == 200 && _adjustAspectRatio != enable) {
		Common::StackLock lock(_graphicsMutex);

		//assert(_hwscreen != 0);
		_adjustAspectRatio ^= true;
		hotswapGFXMode();
			
		// Blit everything to the screen
		internUpdateScreen();
			
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

	/* FIXME: undraw mouse only if the draw rect intersects with the mouse rect */
	undrawMouse();

	// Try to lock the screen surface
	if (SDL_LockSurface(_screen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *dst = (byte *)_screen->pixels + y * _screenWidth + x;

	if (_screenWidth==pitch && pitch == w) {
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


void OSystem_SDL::addDirtyRect(int x, int y, int w, int h) {
	if (_forceFull)
		return;

	if (_numDirtyRects == NUM_DIRTY_RECT)
		_forceFull = true;
	else {
		SDL_Rect *r = &_dirtyRectList[_numDirtyRects++];
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
			w += x; x = 0;
		}

		if (y < 0) {
			h += y;
			y=0;
		}

		if (w > _screenWidth - x) {
			w = _screenWidth - x;
		}

		if (h > _screenHeight - y) {
			h = _screenHeight - y;
		}

		if (_adjustAspectRatio)
			makeRectStretchable(x, y, w, h);
	
		r->x = x;
		r->y = y;
		r->w = w;
		r->h = h;
	}
}


void OSystem_SDL::makeChecksums(const byte *buf) {
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

	// hide the mouse
	undrawMouse();

	_overlayVisible = true;
	clearOverlay();
}

void OSystem_SDL::hideOverlay() {
	assert (_transactionMode == kTransactionNone);

	// hide the mouse
	undrawMouse();

	_overlayVisible = false;
	_forceFull = true;
}

void OSystem_SDL::clearOverlay() {
	assert (_transactionMode == kTransactionNone);

	if (!_overlayVisible)
		return;
	
	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends
	
	// hide the mouse
	undrawMouse();

	// Clear the overlay by making the game screen "look through" everywhere.
	SDL_Rect src, dst;
	src.x = src.y = 0;
	dst.x = dst.y = 1;
	src.w = dst.w = _screenWidth;
	src.h = dst.h = _screenHeight;
	if (SDL_BlitSurface(_screen, &src, _tmpscreen, &dst) != 0)
		error("SDL_BlitSurface failed: %s", SDL_GetError());

	_forceFull = true;
}

void OSystem_SDL::grabOverlay(OverlayColor *buf, int pitch) {
	assert (_transactionMode == kTransactionNone);

	if (!_overlayVisible)
		return;

	if (_tmpscreen == NULL)
		return;

	// hide the mouse
	undrawMouse();

	if (SDL_LockSurface(_tmpscreen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *src = (byte *)_tmpscreen->pixels + _tmpscreen->pitch + 2;	// Offset by one row, one column
	int h = _screenHeight;
	do {
		memcpy(buf, src, _screenWidth*2);
		src += _tmpscreen->pitch;
		buf += pitch;
	} while (--h);

	SDL_UnlockSurface(_tmpscreen);
}

void OSystem_SDL::copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {
	assert (_transactionMode == kTransactionNone);

	if (!_overlayVisible)
		return;

	if (_tmpscreen == NULL)
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

	if (w > _screenWidth - x) {
		w = _screenWidth - x;
	}

	if (h > _screenHeight-y) {
		h = _screenHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;

	// Mark the modified region as dirty
	_cksumValid = false;
	addDirtyRect(x, y, w, h);

	/* FIXME: undraw mouse only if the draw rect intersects with the mouse rect */
	undrawMouse();

	if (SDL_LockSurface(_tmpscreen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *dst = (byte *)_tmpscreen->pixels + (y + 1) * _tmpscreen->pitch + (x + 1) * 2;
	do {
		memcpy(dst, buf, w * 2);
		dst += _tmpscreen->pitch;
		buf += pitch;
	} while (--h);

	SDL_UnlockSurface(_tmpscreen);
}

OverlayColor OSystem_SDL::RGBToColor(uint8 r, uint8 g, uint8 b) {
	return SDL_MapRGB(_tmpscreen->format, r, g, b);
}

void OSystem_SDL::colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b) {
	SDL_GetRGB(color, _tmpscreen->format, &r, &g, &b);
}


#pragma mark -
#pragma mark --- Mouse ---
#pragma mark -

bool OSystem_SDL::showMouse(bool visible) {
	if (_mouseVisible == visible)
		return visible;
	
	bool last = _mouseVisible;
	_mouseVisible = visible;

	if (visible)
		drawMouse();
	else
		undrawMouse();

	return last;
}

void OSystem_SDL::setMousePos(int x, int y) {
	if (x != _mouseCurState.x || y != _mouseCurState.y) {
		undrawMouse();
		_mouseCurState.x = x;
		_mouseCurState.y = y;
		updateScreen();
	}
}

void OSystem_SDL::warpMouse(int x, int y) {
	if (_mouseCurState.x != x || _mouseCurState.y != y) {
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
	
void OSystem_SDL::setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, byte keycolor) {

	undrawMouse();

	assert(w <= MAX_MOUSE_W);
	assert(h <= MAX_MOUSE_H);
	_mouseCurState.w = w;
	_mouseCurState.h = h;

	_mouseHotspotX = hotspot_x;
	_mouseHotspotY = hotspot_y;

	_mouseKeyColor = keycolor;

	free(_mouseData);

	_mouseData = (byte *)malloc(w * h);
	memcpy(_mouseData, buf, w * h);
}

void OSystem_SDL::toggleMouseGrab() {
	if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF)
		SDL_WM_GrabInput(SDL_GRAB_ON);
	else
		SDL_WM_GrabInput(SDL_GRAB_OFF);
}

void OSystem_SDL::drawMouse() {
	if (_mouseDrawn || !_mouseVisible || !_mouseData)
		return;

	int x = _mouseCurState.x - _mouseHotspotX;
	int y = _mouseCurState.y - _mouseHotspotY;
	int w = _mouseCurState.w;
	int h = _mouseCurState.h;
	byte color;
	const byte *src = _mouseData;		// Image representing the mouse
	
	// clip the mouse rect, and addjust the src pointer accordingly
	if (x < 0) {
		w += x;
		src -= x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		src -= y * _mouseCurState.w;
		y = 0;
	}

	if (w > _screenWidth - x)
		w = _screenWidth - x;
	if (h > _screenHeight - y)
		h = _screenHeight - y;

	// Quick check to see if anything has to be drawn at all
	if (w <= 0 || h <= 0)
		return;

	// Draw the mouse cursor; backup the covered area in "bak"
	if (SDL_LockSurface(_overlayVisible ? _tmpscreen : _screen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	// Mark as dirty
	addDirtyRect(x, y, w, h);

	if (!_overlayVisible) {
		byte *bak = _mouseBackup;		// Surface used to backup the area obscured by the mouse
		byte *dst;					// Surface we are drawing into
	
		dst = (byte *)_screen->pixels + y * _screenWidth + x;
		while (h > 0) {
			int width = w;
			while (width > 0) {
				*bak++ = *dst;
				color = *src++;
				if (color != _mouseKeyColor)	// transparent, don't draw
					*dst = color;
				dst++;
				width--;
			}
			src += _mouseCurState.w - w;
			bak += MAX_MOUSE_W - w;
			dst += _screenWidth - w;
			h--;
		}
	
	} else {
		uint16 *bak = (uint16 *)_mouseBackup;	// Surface used to backup the area obscured by the mouse
		byte *dst;					// Surface we are drawing into
	
		dst = (byte *)_tmpscreen->pixels + (y + 1) * _tmpscreen->pitch + (x + 1) * 2;
		while (h > 0) {
			int width = w;
			while (width > 0) {
				*bak++ = *(uint16 *)dst;
				color = *src++;
				if (color != 0xFF)	// 0xFF = transparent, don't draw
					*(uint16 *)dst = RGBToColor(_currentPalette[color].r, _currentPalette[color].g, _currentPalette[color].b);
				dst += 2;
				width--;
			}
			src += _mouseCurState.w - w;
			bak += MAX_MOUSE_W - w;
			dst += _tmpscreen->pitch - w * 2;
			h--;
		}
	}

	SDL_UnlockSurface(_overlayVisible ? _tmpscreen : _screen);

	// Finally, set the flag to indicate the mouse has been drawn
	_mouseDrawn = true;
}

void OSystem_SDL::undrawMouse() {
	assert (_transactionMode == kTransactionNone || _transactionMode == kTransactionCommit);

	if (!_mouseDrawn)
		return;
	_mouseDrawn = false;

	int old_mouse_x = _mouseCurState.x - _mouseHotspotX;
	int old_mouse_y = _mouseCurState.y - _mouseHotspotY;
	int old_mouse_w = _mouseCurState.w;
	int old_mouse_h = _mouseCurState.h;

	// clip the mouse rect, and addjust the src pointer accordingly
	if (old_mouse_x < 0) {
		old_mouse_w += old_mouse_x;
		old_mouse_x = 0;
	}
	if (old_mouse_y < 0) {
		old_mouse_h += old_mouse_y;
		old_mouse_y = 0;
	}

	if (old_mouse_w > _screenWidth - old_mouse_x)
		old_mouse_w = _screenWidth - old_mouse_x;
	if (old_mouse_h > _screenHeight - old_mouse_y)
		old_mouse_h = _screenHeight - old_mouse_y;

	// Quick check to see if anything has to be drawn at all
	if (old_mouse_w <= 0 || old_mouse_h <= 0)
		return;

	if (SDL_LockSurface(_overlayVisible ? _tmpscreen : _screen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	int x, y;
	if (!_overlayVisible) {
		byte *dst, *bak = _mouseBackup;

		// No need to do clipping here, since drawMouse() did that already
		dst = (byte *)_screen->pixels + old_mouse_y * _screenWidth + old_mouse_x;
		for (y = 0; y < old_mouse_h; ++y, bak += MAX_MOUSE_W, dst += _screenWidth) {
			for (x = 0; x < old_mouse_w; ++x) {
				dst[x] = bak[x];
			}
		}
	
	} else {

		byte *dst;
		uint16 *bak = (uint16 *)_mouseBackup;
	
		// No need to do clipping here, since drawMouse() did that already
		dst = (byte *)_tmpscreen->pixels + (old_mouse_y + 1) * _tmpscreen->pitch + (old_mouse_x + 1) * 2;
		for (y = 0; y < old_mouse_h; ++y, bak += MAX_MOUSE_W, dst += _tmpscreen->pitch) {
			for (x = 0; x < old_mouse_w; ++x) {
				*((uint16 *)dst + x) = bak[x];
			}
		}
	}

	addDirtyRect(old_mouse_x, old_mouse_y, old_mouse_w, old_mouse_h);

	SDL_UnlockSurface(_overlayVisible ? _tmpscreen : _screen);
}


#pragma mark -
#pragma mark --- Mouse ---
#pragma mark -

#ifdef USE_OSD
void OSystem_SDL::displayMessageOnOSD(const char *msg) {
	assert (_transactionMode == kTransactionNone);

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
//	const Graphics::Font *font = &Graphics::g_sysfont;
	const Graphics::Font *font = &Graphics::g_scummfont;
	
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
