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

	virtual void draw_mouse();
	virtual void undraw_mouse();

	virtual void load_gfx_mode();
	virtual void unload_gfx_mode();
	void hotswap_gfx_mode();
	
	int TMP_SCREEN_WIDTH;
};

OSystem_SDL_Common *OSystem_SDL_Common::create() {
	return new OSystem_SDL_Normal();
}

void OSystem_SDL_Normal::set_palette(const byte *colors, uint start, uint num) {
	const byte *b = colors;
	uint i;
	SDL_Color *base = _cur_pal + start;
	for(i = 0; i < num; i++) {
		base[i].r = b[0];
		base[i].g = b[1];
		base[i].b = b[2];
		b += 4;
	}

	if (start < _palette_changed_first)
		_palette_changed_first = start;

	if (start + num > _palette_changed_last)
		_palette_changed_last = start + num;
}

void OSystem_SDL_Normal::draw_mouse() {
	if (!_overlay_visible) {
		OSystem_SDL_Common::draw_mouse();
	}


	if (_mouse_drawn || !_mouse_visible)
		return;

	int x = _mouse_cur_state.x - _mouse_hotspot_x;
	int y = _mouse_cur_state.y - _mouse_hotspot_y;
	int w = _mouse_cur_state.w;
	int h = _mouse_cur_state.h;
	byte color;
	byte *src = _mouse_data;		// Image representing the mouse
	uint16 *bak = (uint16*)_mouse_backup;	// Surface used to backup the area obscured by the mouse
	uint16 *dst;					// Surface we are drawing into

	// clip the mouse rect, and addjust the src pointer accordingly
	if (x < 0) {
		w += x;
		src -= x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		src -= y * _mouse_cur_state.w;
		y = 0;
	}
	if (w > SCREEN_WIDTH - x)
		w = SCREEN_WIDTH - x;
	if (h > SCREEN_HEIGHT - y)
		h = SCREEN_HEIGHT - y;

	// Adjust for tmp_screen offset
	x++;
	y++;

	// Store the bounding box so that undraw mouse can restore the area the
	// mouse currently covers to its original content.
	_mouse_old_state.x = x;
	_mouse_old_state.y = y;
	_mouse_old_state.w = w;
	_mouse_old_state.h = h;

	// Quick check to see if anything has to be drawn at all
	if (w <= 0 || h <= 0)
		return;

	// Draw the mouse cursor; backup the covered area in "bak"

	if (SDL_LockSurface(sdl_tmpscreen) == -1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	dst = (uint16 *)sdl_tmpscreen->pixels + y * TMP_SCREEN_WIDTH + x;
	while (h > 0) {
		int width = w;
		while (width > 0) {
			*bak++ = *dst;
			color = *src++;
			if (color != 0xFF)	// 0xFF = transparent, don't draw
				*dst = RGB_TO_16(_cur_pal[color].r, _cur_pal[color].g, _cur_pal[color].b);
			dst++;
			width--;
		}
		src += _mouse_cur_state.w - w;
		bak += MAX_MOUSE_W - w;
		dst += TMP_SCREEN_WIDTH - w;
		h--;
	}

	SDL_UnlockSurface(sdl_tmpscreen);
	
	// Mark as dirty
	add_dirty_rect(x-1, y-1, w, h);

	// Finally, set the flag to indicate the mouse has been drawn
	_mouse_drawn = true;
}

void OSystem_SDL_Normal::undraw_mouse() {
	if (!_overlay_visible) {
		OSystem_SDL_Common::undraw_mouse();
	}


	if (!_mouse_drawn)
		return;
	_mouse_drawn = false;

	uint16 *dst, *bak = (uint16 *)_mouse_backup;
	const int old_mouse_x = _mouse_old_state.x;
	const int old_mouse_y = _mouse_old_state.y;
	const int old_mouse_w = _mouse_old_state.w;
	const int old_mouse_h = _mouse_old_state.h;
	int x,y;

	if (SDL_LockSurface(sdl_tmpscreen) == -1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	// No need to do clipping here, since draw_mouse() did that already

	dst = (uint16 *)sdl_tmpscreen->pixels + old_mouse_y * TMP_SCREEN_WIDTH + old_mouse_x;
	for (y = 0; y < old_mouse_h; ++y, bak += MAX_MOUSE_W, dst += TMP_SCREEN_WIDTH) {
		for (x = 0; x < old_mouse_w; ++x) {
			dst[x] = bak[x];
		}
	}

	add_dirty_rect(old_mouse_x-1, old_mouse_y-1, old_mouse_w, old_mouse_h);

	SDL_UnlockSurface(sdl_tmpscreen);
}

void OSystem_SDL_Normal::load_gfx_mode() {
	force_full = true;
	scaling = 1;
	_mode_flags = 0;
	_overlay_visible = false;

	_scaler_proc = NULL;
	sdl_tmpscreen = NULL;
	TMP_SCREEN_WIDTH = (SCREEN_WIDTH + 3);
	
	switch(_mode) {
	case GFX_2XSAI:
		scaling = 2;
		_scaler_proc = _2xSaI;
		break;
	case GFX_SUPER2XSAI:
		scaling = 2;
		_scaler_proc = Super2xSaI;
		break;
	case GFX_SUPEREAGLE:
		scaling = 2;
		_scaler_proc = SuperEagle;
		break;
	case GFX_ADVMAME2X:
		scaling = 2;
		_scaler_proc = AdvMame2x;
		break;

	case GFX_DOUBLESIZE:
		scaling = 2;
		_scaler_proc = Normal2x;
		break;

	case GFX_TRIPLESIZE:
		if (_full_screen) {
			warning("full screen in useless in triplesize mode, reverting to normal mode");
			goto normal_mode;
		}
		scaling = 3;
		_scaler_proc = Normal3x;
		break;

	case GFX_NORMAL:
normal_mode:;
		scaling = 1;
		_scaler_proc = Normal1x;
		break;
	}

	sdl_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, 8, 0, 0, 0, 0);
	if (sdl_screen == NULL)
		error("sdl_screen failed failed");

	uint16 *tmp_screen = (uint16*)calloc(TMP_SCREEN_WIDTH*(SCREEN_HEIGHT+3),sizeof(uint16));
	_mode_flags = DF_WANT_RECT_OPTIM | DF_UPDATE_EXPAND_1_PIXEL;

	sdl_hwscreen = SDL_SetVideoMode(SCREEN_WIDTH * scaling, SCREEN_HEIGHT * scaling, 16, 
		_full_screen ? (SDL_FULLSCREEN|SDL_SWSURFACE) : SDL_SWSURFACE
	);
	if (sdl_hwscreen == NULL)
		error("sdl_hwscreen failed");
	
	/* Need some extra bytes around when using 2XSAI */
	if (sdl_hwscreen->format->Rmask == 0x7C00)	// HACK HACK HACK
		Init_2xSaI(555);
	else
		Init_2xSaI(565);
	sdl_tmpscreen = SDL_CreateRGBSurfaceFrom(tmp_screen,
						TMP_SCREEN_WIDTH, SCREEN_HEIGHT + 3, 16, TMP_SCREEN_WIDTH*2,
						sdl_hwscreen->format->Rmask,
						sdl_hwscreen->format->Gmask,
						sdl_hwscreen->format->Bmask,
						sdl_hwscreen->format->Amask);

	if (sdl_tmpscreen == NULL)
		error("sdl_tmpscreen failed");
	
	// keyboard cursor control, some other better place for it?
	km.x_max = SCREEN_WIDTH * scaling - 1;
	km.y_max = SCREEN_HEIGHT * scaling - 1;
	km.delay_time = 25;
	km.last_time = 0;

}

void OSystem_SDL_Normal::unload_gfx_mode() {
	if (sdl_screen) {
		SDL_FreeSurface(sdl_screen);
		sdl_screen = NULL; 
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
	
	if (sdl_hwscreen == NULL)
		return;	// Can this really happen?

	// First make sure the mouse is drawn, if it should be drawn.
	draw_mouse();
	
	// Check whether the palette was changed in the meantime and update the
	// screen surface accordingly. 
	if (_palette_changed_last != 0) {
		SDL_SetColors(sdl_screen, _cur_pal + _palette_changed_first, 
			_palette_changed_first,
			_palette_changed_last - _palette_changed_first);
		
		_palette_changed_last = 0;

		force_full = true;
	}

	
	/* If the shake position changed, fill the dirty area with blackness */
	if (_current_shake_pos != _new_shake_pos) {
		SDL_Rect blackrect = {0, 0, SCREEN_WIDTH*scaling, _new_shake_pos*scaling};
		SDL_FillRect(sdl_hwscreen, &blackrect, 0);

		_current_shake_pos = _new_shake_pos;

		force_full = true;
	}

	// Force a full redraw if requested
	if (force_full) {
		num_dirty_rects = 1;

		dirty_rect_list[0].x = 0;
		dirty_rect_list[0].y = 0;
		dirty_rect_list[0].w = SCREEN_WIDTH;
		dirty_rect_list[0].h = SCREEN_HEIGHT;
	}

	// Only draw anything if necessary
	if (num_dirty_rects > 0) {
	
		SDL_Rect *r; 
		uint32 srcPitch, dstPitch;
		SDL_Rect *last_rect = dirty_rect_list + num_dirty_rects;
	
		// Convert appropriate parts of the 8bpp image into 16bpp
		if (!_overlay_visible) {
			SDL_Rect dst;
			for(r=dirty_rect_list; r!=last_rect; ++r) {
				dst = *r;
				dst.x++;	// Shift rect by one since 2xSai needs to acces the data around
				dst.y++;	// any pixel to scale it, and we want to avoid mem access crashes.
				if (SDL_BlitSurface(sdl_screen, r, sdl_tmpscreen, &dst) != 0)
					error("SDL_BlitSurface failed: %s", SDL_GetError());
			}
		}
	
		SDL_LockSurface(sdl_tmpscreen);
		SDL_LockSurface(sdl_hwscreen);
	
		srcPitch = sdl_tmpscreen->pitch;
		dstPitch = sdl_hwscreen->pitch;
	
		for(r=dirty_rect_list; r!=last_rect; ++r) {
			register int dst_y = r->y + _current_shake_pos;
			register int dst_h = 0;
			if (dst_y < SCREEN_HEIGHT) {
				dst_h = r->h;
				if (dst_h > SCREEN_HEIGHT - dst_y)
					dst_h = SCREEN_HEIGHT - dst_y;
				
				dst_y *= scaling;
				
				_scaler_proc((byte*)sdl_tmpscreen->pixels + (r->x*2+2) + (r->y+1)*srcPitch, srcPitch, NULL, 
					(byte*)sdl_hwscreen->pixels + r->x*2*scaling + dst_y*dstPitch, dstPitch, r->w, dst_h);
			}
			
			r->x *= scaling;
			r->y = dst_y;
			r->w *= scaling;
			r->h = dst_h * scaling;
		}
	
		if (force_full) {
			dirty_rect_list[0].y = 0;
			dirty_rect_list[0].h = SCREEN_HEIGHT * scaling;
		}
		
		SDL_UnlockSurface(sdl_tmpscreen);
		SDL_UnlockSurface(sdl_hwscreen);
	}
	
	if (num_dirty_rects > 0) {
		/* Finally, blit all our changes to the screen */
		SDL_UpdateRects(sdl_hwscreen, num_dirty_rects, dirty_rect_list);
	}

	num_dirty_rects = 0;
	force_full = false;
}

void OSystem_SDL_Normal::hotswap_gfx_mode() {
	/* hmm, need to allocate a 320x200 bitmap
	 * which will contain the "backup" of the screen during the change.
	 * then draw that to the new screen right after it's setup.
	 */
	
	byte *bak_mem = (byte*)malloc(SCREEN_WIDTH*SCREEN_HEIGHT);

	get_320x200_image(bak_mem);

	unload_gfx_mode();
	load_gfx_mode();

	force_full = true;

	// reset palette
	SDL_SetColors(sdl_screen, _cur_pal, 0, 256);

	// blit image
	OSystem_SDL_Normal::copy_rect(bak_mem, SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	free(bak_mem);

	OSystem_SDL_Normal::update_screen();
}

uint32 OSystem_SDL_Normal::property(int param, Property *value) {

	if (param == PROP_TOGGLE_FULLSCREEN) {
		_full_screen ^= true;

		if (!SDL_WM_ToggleFullScreen(sdl_hwscreen)) {
			/* if ToggleFullScreen fails, achieve the same effect with hotswap gfx mode */
			hotswap_gfx_mode();
		}
		return 1;
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
	_overlay_visible = false;
	force_full = true;
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
	src.w = dst.w = SCREEN_WIDTH;
	src.h = dst.h = SCREEN_HEIGHT;
	if (SDL_BlitSurface(sdl_screen, &src, sdl_tmpscreen, &dst) != 0)
		error("SDL_BlitSurface failed: %s", SDL_GetError());

	force_full = true;
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
	int h = SCREEN_HEIGHT;
	do {
		memcpy(buf, src, SCREEN_WIDTH*2);
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
	if (w > SCREEN_WIDTH-x) { w = SCREEN_WIDTH - x; }
	if (h > SCREEN_HEIGHT-y) { h = SCREEN_HEIGHT - y; }
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


