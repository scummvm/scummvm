/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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

bool OSystem_SDL::setGraphicsMode(int mode) {
	Common::StackLock lock(_graphicsMutex, this);

	int newScaleFactor = 1;
	ScalerProc *newScalerProc;

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

	if (newScaleFactor != _scaleFactor) {
		hotswap_gfx_mode();
	} else {
		_scaler_proc = newScalerProc;
		_forceFull = true;

		// Blit everything to the screen
		internUpdateScreen();
	
		// Make sure that an EVENT_SCREEN_CHANGED gets sent later
		_modeChanged = true;
	}

	return true;
}

int OSystem_SDL::getGraphicsMode() const {
	return _mode;
}

void OSystem_SDL::initSize(uint w, uint h) {
	// Avoid redundant res changes
	if ((int)w == _screenWidth && (int)h == _screenHeight)
		return;

	_screenWidth = w;
	_screenHeight = h;

	if (h != 200)
		_adjustAspectRatio = false;

	CKSUM_NUM = (_screenWidth * _screenHeight / (8 * 8));

	free(_dirty_checksums);
	_dirty_checksums = (uint32 *)calloc(CKSUM_NUM * 2, sizeof(uint32));

	unload_gfx_mode();
	load_gfx_mode();
}

void OSystem_SDL::load_gfx_mode() {
	_forceFull = true;
	_mode_flags |= DF_UPDATE_EXPAND_1_PIXEL;

	_tmpscreen = NULL;
	_tmpScreenWidth = (_screenWidth + 3);
	
	switch(_mode) {
	case GFX_NORMAL:
		_scaleFactor = 1;
		_scaler_proc = Normal1x;
		break;
	case GFX_DOUBLESIZE:
		_scaleFactor = 2;
		_scaler_proc = Normal2x;
		break;
	case GFX_TRIPLESIZE:
		_scaleFactor = 3;
		_scaler_proc = Normal3x;
		break;

	case GFX_2XSAI:
		_scaleFactor = 2;
		_scaler_proc = _2xSaI;
		break;
	case GFX_SUPER2XSAI:
		_scaleFactor = 2;
		_scaler_proc = Super2xSaI;
		break;
	case GFX_SUPEREAGLE:
		_scaleFactor = 2;
		_scaler_proc = SuperEagle;
		break;
	case GFX_ADVMAME2X:
		_scaleFactor = 2;
		_scaler_proc = AdvMame2x;
		break;
	case GFX_ADVMAME3X:
		_scaleFactor = 3;
		_scaler_proc = AdvMame3x;
		break;
	case GFX_HQ2X:
		_scaleFactor = 2;
		_scaler_proc = HQ2x;
		break;
	case GFX_HQ3X:
		_scaleFactor = 3;
		_scaler_proc = HQ3x;
		break;
	case GFX_TV2X:
		_scaleFactor = 2;
		_scaler_proc = TV2x;
		break;
	case GFX_DOTMATRIX:
		_scaleFactor = 2;
		_scaler_proc = DotMatrix;
		break;

	default:
		error("unknown gfx mode %d", _mode);
	}

	//
	// Create the surface that contains the 8 bit game data
	//
	_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, _screenWidth, _screenHeight, 8, 0, 0, 0, 0);
	if (_screen == NULL)
		error("_screen failed");

	//
	// Create the surface that contains the scaled graphics in 16 bit mode
	//

	_hwscreen = SDL_SetVideoMode(_screenWidth * _scaleFactor, effectiveScreenHeight(), 16, 
		_full_screen ? (SDL_FULLSCREEN|SDL_SWSURFACE) : SDL_SWSURFACE
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
	uint16 *tmp_screen = (uint16 *)calloc(_tmpScreenWidth * (_screenHeight + 3), sizeof(uint16));
	_tmpscreen = SDL_CreateRGBSurfaceFrom(tmp_screen,
						_tmpScreenWidth, _screenHeight + 3, 16, _tmpScreenWidth * 2,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

	if (_tmpscreen == NULL)
		error("_tmpscreen failed");

	// keyboard cursor control, some other better place for it?
	km.x_max = _screenWidth * _scaleFactor - 1;
	km.y_max = effectiveScreenHeight() - 1;
	km.delay_time = 25;
	km.last_time = 0;
}

void OSystem_SDL::unload_gfx_mode() {
	if (_screen) {
		SDL_FreeSurface(_screen);
		_screen = NULL; 
	}

	if (_hwscreen) {
		SDL_FreeSurface(_hwscreen); 
		_hwscreen = NULL;
	}

	if (_tmpscreen) {
		free(_tmpscreen->pixels);
		SDL_FreeSurface(_tmpscreen);
		_tmpscreen = NULL;
	}
}

void OSystem_SDL::hotswap_gfx_mode() {
	if (!_screen)
		return;

	// Keep around the old _screen & _tmpscreen so we can restore the screen data
	// after the mode switch.
	SDL_Surface *old_screen = _screen;
	SDL_Surface *old_tmpscreen = _tmpscreen;

	// Release the HW screen surface
	SDL_FreeSurface(_hwscreen); 

	// Setup the new GFX mode
	load_gfx_mode();

	// reset palette
	SDL_SetColors(_screen, _currentPalette, 0, 256);

	// Restore old screen content
	SDL_BlitSurface(old_screen, NULL, _screen, NULL);
	SDL_BlitSurface(old_tmpscreen, NULL, _tmpscreen, NULL);
	
	// Free the old surfaces
	SDL_FreeSurface(old_screen);
	free(old_tmpscreen->pixels);
	SDL_FreeSurface(old_tmpscreen);

	// Blit everything to the screen
	internUpdateScreen();
	
	// Make sure that an EVENT_SCREEN_CHANGED gets sent later
	_modeChanged = true;
}

void OSystem_SDL::updateScreen() {
	Common::StackLock lock(_graphicsMutex, this);	// Lock the mutex until this function ends

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
	draw_mouse();
	
	// Check whether the palette was changed in the meantime and update the
	// screen surface accordingly. 
	if (_paletteDirtyEnd != 0) {
		SDL_SetColors(_screen, _currentPalette + _paletteDirtyStart, 
			_paletteDirtyStart,
			_paletteDirtyEnd - _paletteDirtyStart);
		
		_paletteDirtyEnd = 0;

		_forceFull = true;
	}

	// Force a full redraw if requested
	if (_forceFull) {
		_num_dirty_rects = 1;

		_dirty_rect_list[0].x = 0;
		_dirty_rect_list[0].y = 0;
		_dirty_rect_list[0].w = _screenWidth;
		_dirty_rect_list[0].h = _screenHeight;
	}

	// Only draw anything if necessary
	if (_num_dirty_rects > 0) {

		SDL_Rect *r; 
		SDL_Rect dst;
		uint32 srcPitch, dstPitch;
		SDL_Rect *last_rect = _dirty_rect_list + _num_dirty_rects;

		if (_scaler_proc == Normal1x && !_adjustAspectRatio) {
			SDL_Surface *target = _overlayVisible ? _tmpscreen : _screen;
			for (r = _dirty_rect_list; r != last_rect; ++r) {
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
				for (r = _dirty_rect_list; r != last_rect; ++r) {
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

			for (r = _dirty_rect_list; r != last_rect; ++r) {
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

					_scaler_proc((byte *)_tmpscreen->pixels + (r->x * 2 + 2) + (r->y + 1) * srcPitch, srcPitch,
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
			_dirty_rect_list[0].y = 0;
			_dirty_rect_list[0].h = effectiveScreenHeight();
		}

		// Finally, blit all our changes to the screen
		SDL_UpdateRects(_hwscreen, _num_dirty_rects, _dirty_rect_list);
	}

	_num_dirty_rects = 0;
	_forceFull = false;
}

bool OSystem_SDL::save_screenshot(const char *filename) {
	assert(_hwscreen != NULL);

	Common::StackLock lock(_graphicsMutex, this);	// Lock the mutex until this function ends
	SDL_SaveBMP(_hwscreen, filename);
	return true;
}

void OSystem_SDL::setFullscreenMode(bool enable) {
	Common::StackLock lock(_graphicsMutex, this);

	if (_full_screen != enable) {
		assert(_hwscreen != 0);
		_full_screen ^= true;

		undraw_mouse();
	
#if defined(MACOSX) && !SDL_VERSION_ATLEAST(1, 2, 6)
		// On OS X, SDL_WM_ToggleFullScreen is currently not implemented. Worse,
		// before SDL 1.2.6 it always returned -1 (which would indicate a
		// successful switch). So we simply don't call it at all and use
		// hotswap_gfx_mode() directly to switch to fullscreen mode.
		hotswap_gfx_mode();
#else
		if (!SDL_WM_ToggleFullScreen(_hwscreen)) {
			// if ToggleFullScreen fails, achieve the same effect with hotswap gfx mode
			hotswap_gfx_mode();
		} else {
			// Make sure that an EVENT_SCREEN_CHANGED gets sent later
			_modeChanged = true;
		}
#endif
	}
}

void OSystem_SDL::copy_rect(const byte *src, int pitch, int x, int y, int w, int h) {
	if (_screen == NULL)
		return;

	Common::StackLock lock(_graphicsMutex, this);	// Lock the mutex until this function ends
	
	if (((long)src & 3) == 0 && pitch == _screenWidth && x==0 && y==0 &&
			w==_screenWidth && h==_screenHeight && _mode_flags&DF_WANT_RECT_OPTIM) {
		/* Special, optimized case for full screen updates.
		 * It tries to determine what areas were actually changed,
		 * and just updates those, on the actual display. */
		add_dirty_rgn_auto(src);
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

		cksum_valid = false;
		add_dirty_rect(x, y, w, h);
	}

	/* FIXME: undraw mouse only if the draw rect intersects with the mouse rect */
	undraw_mouse();

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


void OSystem_SDL::add_dirty_rect(int x, int y, int w, int h) {
	if (_forceFull)
		return;

	if (_num_dirty_rects == NUM_DIRTY_RECT)
		_forceFull = true;
	else {
		SDL_Rect *r = &_dirty_rect_list[_num_dirty_rects++];
		// Extend the dirty region by 1 pixel for scalers
		// that "smear" the screen, e.g. 2xSAI
		if (_mode_flags & DF_UPDATE_EXPAND_1_PIXEL) {
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


void OSystem_SDL::mk_checksums(const byte *buf) {
	uint32 *sums = _dirty_checksums;
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

void OSystem_SDL::add_dirty_rgn_auto(const byte *buf) {
	assert(((long)buf & 3) == 0);

	/* generate a table of the checksums */
	mk_checksums(buf);

	if (!cksum_valid) {
		_forceFull = true;
		cksum_valid = true;
	}

	/* go through the checksum list, compare it with the previous checksums,
		 and add all dirty rectangles to a list. try to combine small rectangles
		 into bigger ones in a simple way */
	if (!_forceFull) {
		int x,y,w;
		uint32 *ck = _dirty_checksums;

		for(y = 0; y!=_screenHeight / 8; y++) {
			for(x = 0; x!=_screenWidth / 8; x++, ck++) {
				if (ck[0] != ck[CKSUM_NUM]) {
					/* found a dirty 8x8 block, now go as far to the right as possible,
						 and at the same time, unmark the dirty status by setting old to new. */
					w=0;
					do {
						ck[w + CKSUM_NUM] = ck[w];
						w++;
					} while (x + w != _screenWidth / 8 && ck[w] != ck[w + CKSUM_NUM]);

					add_dirty_rect(x * 8, y * 8, w * 8, 8);

					if (_forceFull)
						goto get_out;
				}
			}
		}
	} else {
		get_out:;
		/* Copy old checksums to new */
		memcpy(_dirty_checksums + CKSUM_NUM, _dirty_checksums, CKSUM_NUM * sizeof(uint32));
	}
}

int16 OSystem_SDL::get_height() {
	return _screenHeight;
}

int16 OSystem_SDL::get_width() {
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

void OSystem_SDL::move_screen(int dx, int dy, int height) {

	// Short circuit check - do we have to do anything anyway?
	if ((dx == 0 && dy == 0) || height <= 0)
		return;

	Common::StackLock lock(_graphicsMutex, this);	// Lock the mutex until this function ends

	byte *src, *dst;
	int x, y;

	// We'll have to do a full screen redraw anyway, so set the flag.
	_forceFull = true;

	// Hide the mouse
	undraw_mouse();

	// Try to lock the screen surface
	if (SDL_LockSurface(_screen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	// vertical movement
	if (dy > 0) {
		// move down - copy from bottom to top
		dst = (byte *)_screen->pixels + (height - 1) * _screenWidth;
		src = dst - dy * _screenWidth;
		for (y = dy; y < height; y++) {
			memcpy(dst, src, _screenWidth);
			src -= _screenWidth;
			dst -= _screenWidth;
		}
	} else if (dy < 0) {
		// move up - copy from top to bottom
		dst = (byte *)_screen->pixels;
		src = dst - dy * _screenWidth;
		for (y = -dy; y < height; y++) {
			memcpy(dst, src, _screenWidth);
			src += _screenWidth;
			dst += _screenWidth;
		}
	}

	// horizontal movement
	if (dx > 0) {
		// move right - copy from right to left
		dst = (byte *)_screen->pixels + (_screenWidth - 1);
		src = dst - dx;
		for (y = 0; y < height; y++) {
			for (x = dx; x < _screenWidth; x++) {
				*dst-- = *src--;
			}
			src += _screenWidth + (_screenWidth - dx);
			dst += _screenWidth + (_screenWidth - dx);
		}
	} else if (dx < 0)  {
		// move left - copy from left to right
		dst = (byte *)_screen->pixels;
		src = dst - dx;
		for (y = 0; y < height; y++) {
			for (x = -dx; x < _screenWidth; x++) {
				*dst++ = *src++;
			}
			src += _screenWidth - (_screenWidth + dx);
			dst += _screenWidth - (_screenWidth + dx);
		}
	}

	// Unlock the screen surface
	SDL_UnlockSurface(_screen);
}

void OSystem_SDL::set_shake_pos(int shake_pos) {
	_newShakePos = shake_pos;
}


#pragma mark -
#pragma mark --- Overlays ---
#pragma mark -

void OSystem_SDL::show_overlay() {
	// hide the mouse
	undraw_mouse();

	_overlayVisible = true;
	clear_overlay();
}

void OSystem_SDL::hide_overlay() {
	// hide the mouse
	undraw_mouse();

	_overlayVisible = false;
	_forceFull = true;
}

void OSystem_SDL::clear_overlay() {
	if (!_overlayVisible)
		return;
	
	Common::StackLock lock(_graphicsMutex, this);	// Lock the mutex until this function ends
	
	// hide the mouse
	undraw_mouse();

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

void OSystem_SDL::grab_overlay(int16 *buf, int pitch) {
	if (!_overlayVisible)
		return;

	if (_tmpscreen == NULL)
		return;

	// hide the mouse
	undraw_mouse();

	if (SDL_LockSurface(_tmpscreen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	int16 *src = (int16 *)_tmpscreen->pixels + _tmpScreenWidth + 1;
	int h = _screenHeight;
	do {
		memcpy(buf, src, _screenWidth*2);
		src += _tmpScreenWidth;
		buf += pitch;
	} while (--h);

	SDL_UnlockSurface(_tmpscreen);
}

void OSystem_SDL::copy_rect_overlay(const int16 *buf, int pitch, int x, int y, int w, int h) {
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
	cksum_valid = false;
	add_dirty_rect(x, y, w, h);

	/* FIXME: undraw mouse only if the draw rect intersects with the mouse rect */
	undraw_mouse();

	if (SDL_LockSurface(_tmpscreen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	int16 *dst = (int16 *)_tmpscreen->pixels + (y + 1) * _tmpScreenWidth + (x + 1);
	do {
		memcpy(dst, buf, w * 2);
		dst += _tmpScreenWidth;
		buf += pitch;
	} while (--h);

	SDL_UnlockSurface(_tmpscreen);
}

int16 OSystem_SDL::RGBToColor(uint8 r, uint8 g, uint8 b) {
	return SDL_MapRGB(_tmpscreen->format, r, g, b);
}

void OSystem_SDL::colorToRGB(int16 color, uint8 &r, uint8 &g, uint8 &b) {
	SDL_GetRGB(color, _tmpscreen->format, &r, &g, &b);
}


#pragma mark -
#pragma mark --- Mouse ---
#pragma mark -

bool OSystem_SDL::show_mouse(bool visible) {
	if (_mouseVisible == visible)
		return visible;
	
	bool last = _mouseVisible;
	_mouseVisible = visible;

	if (visible)
		draw_mouse();
	else
		undraw_mouse();

	return last;
}

void OSystem_SDL::set_mouse_pos(int x, int y) {
	if (x != _mouseCurState.x || y != _mouseCurState.y) {
		undraw_mouse();
		_mouseCurState.x = x;
		_mouseCurState.y = y;
		updateScreen();
	}
}

void OSystem_SDL::warp_mouse(int x, int y) {
	if (_mouseCurState.x != x || _mouseCurState.y != y) {
		SDL_WarpMouse(x * _scaleFactor, y * _scaleFactor);

		// SDL_WarpMouse() generates a mouse movement event, so
		// set_mouse_pos() would be called eventually. However, the
		// cannon script in CoMI calls this function twice each time
		// the cannon is reloaded. Unless we update the mouse position
		// immediately the second call is ignored, causing the cannon
		// to change its aim.

		set_mouse_pos(x, y);
	}
}
	
void OSystem_SDL::set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y) {

	undraw_mouse();

	assert(w <= MAX_MOUSE_W);
	assert(h <= MAX_MOUSE_H);
	_mouseCurState.w = w;
	_mouseCurState.h = h;

	_mouseHotspotX = hotspot_x;
	_mouseHotspotY = hotspot_y;

	_mouseData = buf;
}

void OSystem_SDL::toggleMouseGrab() {
	if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF)
		SDL_WM_GrabInput(SDL_GRAB_ON);
	else
		SDL_WM_GrabInput(SDL_GRAB_OFF);
}

void OSystem_SDL::draw_mouse() {
	if (_mouseDrawn || !_mouseVisible)
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
	add_dirty_rect(x, y, w, h);

	if (!_overlayVisible) {
		byte *bak = _mouseBackup;		// Surface used to backup the area obscured by the mouse
		byte *dst;					// Surface we are drawing into
	
		dst = (byte *)_screen->pixels + y * _screenWidth + x;
		while (h > 0) {
			int width = w;
			while (width > 0) {
				*bak++ = *dst;
				color = *src++;
				if (color != 0xFF)	// 0xFF = transparent, don't draw
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
		uint16 *dst;					// Surface we are drawing into
	
		dst = (uint16 *)_tmpscreen->pixels + (y + 1) * _tmpScreenWidth + (x + 1);
		while (h > 0) {
			int width = w;
			while (width > 0) {
				*bak++ = *dst;
				color = *src++;
				if (color != 0xFF)	// 0xFF = transparent, don't draw
					*dst = RGBToColor(_currentPalette[color].r, _currentPalette[color].g, _currentPalette[color].b);
				dst++;
				width--;
			}
			src += _mouseCurState.w - w;
			bak += MAX_MOUSE_W - w;
			dst += _tmpScreenWidth - w;
			h--;
		}
	}

	SDL_UnlockSurface(_overlayVisible ? _tmpscreen : _screen);

	// Finally, set the flag to indicate the mouse has been drawn
	_mouseDrawn = true;
}

void OSystem_SDL::undraw_mouse() {
	if (!_mouseDrawn)
		return;
	_mouseDrawn = false;

	if (SDL_LockSurface(_overlayVisible ? _tmpscreen : _screen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

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


	int x, y;
	if (!_overlayVisible) {
		byte *dst, *bak = _mouseBackup;

		// No need to do clipping here, since draw_mouse() did that already
		dst = (byte *)_screen->pixels + old_mouse_y * _screenWidth + old_mouse_x;
		for (y = 0; y < old_mouse_h; ++y, bak += MAX_MOUSE_W, dst += _screenWidth) {
			for (x = 0; x < old_mouse_w; ++x) {
				dst[x] = bak[x];
			}
		}
	
	} else {

		uint16 *dst, *bak = (uint16 *)_mouseBackup;
	
		// No need to do clipping here, since draw_mouse() did that already
		dst = (uint16 *)_tmpscreen->pixels + (old_mouse_y + 1) * _tmpScreenWidth + (old_mouse_x + 1);
		for (y = 0; y < old_mouse_h; ++y, bak += MAX_MOUSE_W, dst += _tmpScreenWidth) {
			for (x = 0; x < old_mouse_w; ++x) {
				dst[x] = bak[x];
			}
		}
	}

	add_dirty_rect(old_mouse_x, old_mouse_y, old_mouse_w, old_mouse_h);

	SDL_UnlockSurface(_overlayVisible ? _tmpscreen : _screen);
}
