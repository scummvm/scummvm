/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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

#include "sdl-common.h"
#include "common/scaler.h"
#include "common/util.h"
#include "common/engine.h"	// Only #included for error() and warning()

class OSystem_SDL_Normal : public OSystem_SDL_Common {
public:
	OSystem_SDL_Normal();

	// Set colors of the palette
	void set_palette(const byte *colors, uint start, uint num);

	// Update the dirty areas of the screen
	void update_screen();

	// Set a parameter
	uint32 property(int param, Property *value);

	// Overlay
	virtual void show_overlay();
	virtual void hide_overlay();
	virtual void clear_overlay();
	virtual void grab_overlay(int16 *buf, int pitch);
	virtual void copy_rect_overlay(const int16 *buf, int pitch, int x, int y, int w, int h);

protected:
	typedef void ScalerProc(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr,
								uint8 *dstPtr, uint32 dstPitch, int width, int height);

	SDL_Surface *sdl_tmpscreen;   // temporary screen (for scalers/overlay)
	SDL_Surface *sdl_hwscreen;    // hardware screen
	bool _overlay_visible;

	ScalerProc *_scaler_proc;

	int TMP_SCREEN_WIDTH;

	virtual void draw_mouse();
	virtual void undraw_mouse();

	virtual void load_gfx_mode();
	virtual void unload_gfx_mode();
	void hotswap_gfx_mode();
};

OSystem_SDL_Common *OSystem_SDL_Common::create() {
	return new OSystem_SDL_Normal();
}

OSystem_SDL_Normal::OSystem_SDL_Normal()
	 : sdl_tmpscreen(0), sdl_hwscreen(0), _overlay_visible(false),
	   _scaler_proc(0), TMP_SCREEN_WIDTH(0)
{
}

void OSystem_SDL_Normal::set_palette(const byte *colors, uint start, uint num) {
	const byte *b = colors;
	uint i;
	SDL_Color *base = _currentPalette + start;
	for(i = 0; i < num; i++) {
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

void OSystem_SDL_Normal::draw_mouse() {
	if (!_overlay_visible) {
		OSystem_SDL_Common::draw_mouse();
	}

	if (_mouseDrawn || !_mouseVisible)
		return;

	int x = _mouseCurState.x - _mouseHotspotX;
	int y = _mouseCurState.y - _mouseHotspotY;
	int w = _mouseCurState.w;
	int h = _mouseCurState.h;
	byte color;
	byte *src = _mouseData;		// Image representing the mouse
	uint16 *bak = (uint16*)_mouseBackup;	// Surface used to backup the area obscured by the mouse
	uint16 *dst;					// Surface we are drawing into

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

	// Quick check to see if anything has to be drawn at all
	if (w <= 0 || h <= 0)
		return;

	if (w > _screenWidth - x)
		w = _screenWidth - x;
	if (h > _screenHeight - y)
		h = _screenHeight - y;

	// Store the bounding box so that undraw mouse can restore the area the
	// mouse currently covers to its original content.
	_mouseOldState.x = x;
	_mouseOldState.y = y;
	_mouseOldState.w = w;
	_mouseOldState.h = h;

	// Draw the mouse cursor; backup the covered area in "bak"

	if (SDL_LockSurface(sdl_tmpscreen) == -1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	// Mark as dirty
	add_dirty_rect(x, y, w, h);

	dst = (uint16 *)sdl_tmpscreen->pixels + (y+1) * TMP_SCREEN_WIDTH + (x+1);
	while (h > 0) {
		int width = w;
		while (width > 0) {
			*bak++ = *dst;
			color = *src++;
			if (color != 0xFF)	// 0xFF = transparent, don't draw
				*dst = RGB_TO_16(_currentPalette[color].r, _currentPalette[color].g, _currentPalette[color].b);
			dst++;
			width--;
		}
		src += _mouseCurState.w - w;
		bak += MAX_MOUSE_W - w;
		dst += TMP_SCREEN_WIDTH - w;
		h--;
	}

	SDL_UnlockSurface(sdl_tmpscreen);
	
	// Finally, set the flag to indicate the mouse has been drawn
	_mouseDrawn = true;
}

void OSystem_SDL_Normal::undraw_mouse() {
	if (!_overlay_visible) {
		OSystem_SDL_Common::undraw_mouse();
	}

	if (!_mouseDrawn)
		return;
	_mouseDrawn = false;

	if (SDL_LockSurface(sdl_tmpscreen) == -1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	uint16 *dst, *bak = (uint16 *)_mouseBackup;
	const int old_mouse_x = _mouseOldState.x;
	const int old_mouse_y = _mouseOldState.y;
	const int old_mouse_w = _mouseOldState.w;
	const int old_mouse_h = _mouseOldState.h;
	int x, y;

	// No need to do clipping here, since draw_mouse() did that already

	dst = (uint16 *)sdl_tmpscreen->pixels + (old_mouse_y+1) * TMP_SCREEN_WIDTH + (old_mouse_x+1);
	for (y = 0; y < old_mouse_h; ++y, bak += MAX_MOUSE_W, dst += TMP_SCREEN_WIDTH) {
		for (x = 0; x < old_mouse_w; ++x) {
			dst[x] = bak[x];
		}
	}

	add_dirty_rect(old_mouse_x, old_mouse_y, old_mouse_w, old_mouse_h);

	SDL_UnlockSurface(sdl_tmpscreen);
}

void OSystem_SDL_Normal::load_gfx_mode() {
	_forceFull = true;
	_mode_flags = DF_WANT_RECT_OPTIM | DF_UPDATE_EXPAND_1_PIXEL;

	sdl_tmpscreen = NULL;
	TMP_SCREEN_WIDTH = (_screenWidth + 3);
	
	switch(_mode) {
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

	case GFX_DOUBLESIZE:
		_scaleFactor = 2;
		_scaler_proc = Normal2x;
		break;

	case GFX_TRIPLESIZE:
		if (_full_screen) {
			warning("full screen in useless in triplesize mode, reverting to normal mode");
			goto normal_mode;
		}
		_scaleFactor = 3;
		_scaler_proc = Normal3x;
		break;

	case GFX_NORMAL:
normal_mode:;
		_scaleFactor = 1;
		_scaler_proc = Normal1x;
		break;
	default:
		error("unknown gfx mode");
		_scaleFactor = 1;
		_scaler_proc = NULL;
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
	sdl_hwscreen = SDL_SetVideoMode(_screenWidth * _scaleFactor, _screenHeight * _scaleFactor, 16, 
		_full_screen ? (SDL_FULLSCREEN|SDL_SWSURFACE) : SDL_SWSURFACE
	);
	if (sdl_hwscreen == NULL)
		error("sdl_hwscreen failed");
	
	//
	// Create the surface used for the graphics in 16 bit before scaling, and also the overlay
	//

	// Distinguish 555 and 565 mode
	if (sdl_hwscreen->format->Rmask == 0x7C00)
		Init_2xSaI(555);
	else
		Init_2xSaI(565);
	
	// Need some extra bytes around when using 2xSaI
	uint16 *tmp_screen = (uint16*)calloc(TMP_SCREEN_WIDTH*(_screenHeight+3),sizeof(uint16));
	sdl_tmpscreen = SDL_CreateRGBSurfaceFrom(tmp_screen,
						TMP_SCREEN_WIDTH, _screenHeight + 3, 16, TMP_SCREEN_WIDTH*2,
						sdl_hwscreen->format->Rmask,
						sdl_hwscreen->format->Gmask,
						sdl_hwscreen->format->Bmask,
						sdl_hwscreen->format->Amask);

	if (sdl_tmpscreen == NULL)
		error("sdl_tmpscreen failed");
	
	// keyboard cursor control, some other better place for it?
	km.x_max = _screenWidth * _scaleFactor - 1;
	km.y_max = _screenHeight * _scaleFactor - 1;
	km.delay_time = 25;
	km.last_time = 0;

}

void OSystem_SDL_Normal::unload_gfx_mode() {
	if (_screen) {
		SDL_FreeSurface(_screen);
		_screen = NULL; 
	}

	if (sdl_hwscreen) {
		SDL_FreeSurface(sdl_hwscreen); 
		sdl_hwscreen = NULL;
	}
	
	if (sdl_tmpscreen) {
		free((uint16*)sdl_tmpscreen->pixels);
		SDL_FreeSurface(sdl_tmpscreen);
		sdl_tmpscreen = NULL;
	}
}

void OSystem_SDL_Normal::update_screen() {
	
	assert(sdl_hwscreen != NULL);

	// If the shake position changed, fill the dirty area with blackness
	if (_currentShakePos != _newShakePos) {
		SDL_Rect blackrect = {0, 0, _screenWidth*_scaleFactor, _newShakePos*_scaleFactor};
		SDL_FillRect(sdl_hwscreen, &blackrect, 0);

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
		uint32 srcPitch, dstPitch;
		SDL_Rect *last_rect = _dirty_rect_list + _num_dirty_rects;
	
		// Convert appropriate parts of the 8bpp image into 16bpp
		if (!_overlay_visible) {
			SDL_Rect dst;
			for(r = _dirty_rect_list; r != last_rect; ++r) {
				dst = *r;
				dst.x++;	// Shift rect by one since 2xSai needs to acces the data around
				dst.y++;	// any pixel to scale it, and we want to avoid mem access crashes.
				if (SDL_BlitSurface(_screen, r, sdl_tmpscreen, &dst) != 0)
					error("SDL_BlitSurface failed: %s", SDL_GetError());
			}
		}
	
		SDL_LockSurface(sdl_tmpscreen);
		SDL_LockSurface(sdl_hwscreen);
	
		srcPitch = sdl_tmpscreen->pitch;
		dstPitch = sdl_hwscreen->pitch;
	
		for(r = _dirty_rect_list; r != last_rect; ++r) {
			register int dst_y = r->y + _currentShakePos;
			register int dst_h = 0;
			if (dst_y < _screenHeight) {
				dst_h = r->h;
				if (dst_h > _screenHeight - dst_y)
					dst_h = _screenHeight - dst_y;
				
				dst_y *= _scaleFactor;
				
				_scaler_proc((byte*)sdl_tmpscreen->pixels + (r->x*2+2) + (r->y+1)*srcPitch, srcPitch, NULL, 
					(byte*)sdl_hwscreen->pixels + r->x*2*_scaleFactor + dst_y*dstPitch, dstPitch, r->w, dst_h);
			}
			
			r->x *= _scaleFactor;
			r->y = dst_y;
			r->w *= _scaleFactor;
			r->h = dst_h * _scaleFactor;
		}
		
		SDL_UnlockSurface(sdl_tmpscreen);
		SDL_UnlockSurface(sdl_hwscreen);

		// Readjust the dirty rect list in case we are doing a full update.
		// This is necessary if shaking is active.
		if (_forceFull) {
			_dirty_rect_list[0].y = 0;
			_dirty_rect_list[0].h = _screenHeight * _scaleFactor;
		}

		// Finally, blit all our changes to the screen
		SDL_UpdateRects(sdl_hwscreen, _num_dirty_rects, _dirty_rect_list);
	}

	_num_dirty_rects = 0;
	_forceFull = false;
}

void OSystem_SDL_Normal::hotswap_gfx_mode() {
	/* We allocate a screen sized bitmap which contains a "backup"
	 * of the screen data during the change. Then we draw that to
	 * the new screen right after it's setup.
	 */
	
	byte *bak_mem = (byte*)malloc(_screenWidth*_screenHeight);

	get_screen_image(bak_mem);

	unload_gfx_mode();
	load_gfx_mode();

	// reset palette
	SDL_SetColors(_screen, _currentPalette, 0, 256);

	// blit image
	copy_rect(bak_mem, _screenWidth, 0, 0, _screenWidth, _screenHeight);
	free(bak_mem);

	update_screen();
}

uint32 OSystem_SDL_Normal::property(int param, Property *value) {

	if (param == PROP_TOGGLE_FULLSCREEN) {
		assert(sdl_hwscreen != 0);
		_full_screen ^= true;

		if (!SDL_WM_ToggleFullScreen(sdl_hwscreen)) {
			// if ToggleFullScreen fails, achieve the same effect with hotswap gfx mode
			hotswap_gfx_mode();
		}
		return 1;
	} else if (param == PROP_OVERLAY_IS_565) {
		assert(sdl_tmpscreen != 0);
		return (sdl_tmpscreen->format->Rmask != 0x7C00);
	}
	
	return OSystem_SDL_Common::property(param, value);
}


void OSystem_SDL_Normal::show_overlay()
{
	// hide the mouse
	undraw_mouse();

	_overlay_visible = true;
	clear_overlay();
}

void OSystem_SDL_Normal::hide_overlay()
{
	// hide the mouse
	undraw_mouse();

	_overlay_visible = false;
	_forceFull = true;
}

void OSystem_SDL_Normal::clear_overlay()
{
	if (!_overlay_visible)
		return;
	
	// hide the mouse
	undraw_mouse();

	// Clear the overlay by making the game screen "look through" everywhere.
	SDL_Rect src, dst;
	src.x = src.y = 0;
	dst.x = dst.y = 1;
	src.w = dst.w = _screenWidth;
	src.h = dst.h = _screenHeight;
	if (SDL_BlitSurface(_screen, &src, sdl_tmpscreen, &dst) != 0)
		error("SDL_BlitSurface failed: %s", SDL_GetError());

	_forceFull = true;
}

void OSystem_SDL_Normal::grab_overlay(int16 *buf, int pitch)
{
	if (!_overlay_visible)
		return;

	if (sdl_tmpscreen == NULL)
		return;

	// hide the mouse
	undraw_mouse();

	if (SDL_LockSurface(sdl_tmpscreen) == -1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	int16 *src = (int16 *)sdl_tmpscreen->pixels + TMP_SCREEN_WIDTH + 1;
	int h = _screenHeight;
	do {
		memcpy(buf, src, _screenWidth*2);
		src += TMP_SCREEN_WIDTH;
		buf += pitch;
	} while (--h);

	SDL_UnlockSurface(sdl_tmpscreen);
}

void OSystem_SDL_Normal::copy_rect_overlay(const int16 *buf, int pitch, int x, int y, int w, int h)
{
	if (!_overlay_visible)
		return;

	if (sdl_tmpscreen == NULL)
		return;

	// Clip the coordinates
	if (x < 0) { w+=x; buf-=x; x = 0; }
	if (y < 0) { h+=y; buf-=y*pitch; y = 0; }
	if (w > _screenWidth-x) { w = _screenWidth - x; }
	if (h > _screenHeight-y) { h = _screenHeight - y; }
	if (w <= 0 || h <= 0)
		return;
	
	// Mark the modified region as dirty
	cksum_valid = false;
	add_dirty_rect(x, y, w, h);

	/* FIXME: undraw mouse only if the draw rect intersects with the mouse rect */
	undraw_mouse();

	if (SDL_LockSurface(sdl_tmpscreen) == -1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	int16 *dst = (int16 *)sdl_tmpscreen->pixels + (y+1) * TMP_SCREEN_WIDTH + (x+1);
	do {
		memcpy(dst, buf, w*2);
		dst += TMP_SCREEN_WIDTH;
		buf += pitch;
	} while (--h);

	SDL_UnlockSurface(sdl_tmpscreen);
}


