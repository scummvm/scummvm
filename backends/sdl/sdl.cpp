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
#include "scaler.h"
#include "scumm.h"	// Only #included for error() and warning()


class OSystem_SDL_Normal : public OSystem_SDL_Common {
public:
	// Set colors of the palette
	void set_palette(const byte *colors, uint start, uint num);

	// Update the dirty areas of the screen
	void update_screen();

	// Set a parameter
	uint32 property(int param, Property *value);

protected:
	SDL_Surface *sdl_hwscreen;    // hardware screen

	void load_gfx_mode();
	void unload_gfx_mode();
	void hotswap_gfx_mode();
};

OSystem_SDL_Common *OSystem_SDL_Common::create() {
	return new OSystem_SDL_Normal();
}

void OSystem_SDL_Normal::set_palette(const byte *colors, uint start, uint num) {
	const byte *b = colors;
	uint i;
	SDL_Color *base = _cur_pal + start;
	for(i=0;i!=num;i++) {
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

void OSystem_SDL_Normal::load_gfx_mode() {
	force_full = true;
	scaling = 1;
	_mode_flags = 0;

	_sai_func = NULL;
	sdl_tmpscreen = NULL;
	
	switch(_mode) {
	case GFX_2XSAI:
		scaling = 2;
		_sai_func = _2xSaI;
		break;
	case GFX_SUPER2XSAI:
		scaling = 2;
		_sai_func = Super2xSaI;
		break;
	case GFX_SUPEREAGLE:
		scaling = 2;
		_sai_func = SuperEagle;
		break;
	case GFX_ADVMAME2X:
		scaling = 2;
		_sai_func = AdvMame2x;
		break;

	case GFX_DOUBLESIZE:
		scaling = 2;
		break;

	case GFX_TRIPLESIZE:
		if (_full_screen) {
			warning("full screen in useless in triplesize mode, reverting to normal mode");
			goto normal_mode;
		}
		scaling = 3;
		break;

	case GFX_NORMAL:
normal_mode:;
		scaling = 1;
		break;
	}

	sdl_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, 8, 0, 0, 0, 0);
	if (sdl_screen == NULL)
		error("sdl_screen failed failed");

	if (_sai_func) {
		uint16 *tmp_screen = (uint16*)calloc((SCREEN_WIDTH+3)*(SCREEN_HEIGHT+3),sizeof(uint16));
		_mode_flags = DF_WANT_RECT_OPTIM | DF_SEPARATE_TEMPSCREEN | DF_UPDATE_EXPAND_1_PIXEL;

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
							SCREEN_WIDTH + 3, SCREEN_HEIGHT + 3, 16, (SCREEN_WIDTH + 3)*2,
							sdl_hwscreen->format->Rmask,
							sdl_hwscreen->format->Gmask,
							sdl_hwscreen->format->Bmask,
							sdl_hwscreen->format->Amask);

		if (sdl_tmpscreen == NULL)
			error("sdl_tmpscreen failed");
		
	} else {
		switch(scaling) {
		case 3:
			_sai_func = Normal3x;
			break;
		case 2:
			_sai_func = Normal2x;
			break;
		case 1:
			_sai_func = Normal1x;
			break;
		}

		_mode_flags = DF_WANT_RECT_OPTIM | DF_REAL_8BIT;

		sdl_hwscreen = SDL_SetVideoMode(SCREEN_WIDTH * scaling, SCREEN_HEIGHT * scaling, 8, 
			_full_screen ? (SDL_FULLSCREEN|SDL_SWSURFACE) : SDL_SWSURFACE
		);
		if (sdl_hwscreen == NULL)
			error("sdl_hwscreen failed");
		
		sdl_tmpscreen = sdl_screen;
	}

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
	
	if (_mode_flags & DF_SEPARATE_TEMPSCREEN) {
		free((uint16*)sdl_tmpscreen->pixels);
		SDL_FreeSurface(sdl_tmpscreen);
	}
	sdl_tmpscreen = NULL;
}

void OSystem_SDL_Normal::update_screen() {
	
	if (sdl_hwscreen == NULL)
		return;	// Can this really happen?

	/* First make sure the mouse is drawn, if it should be drawn. */
	draw_mouse();
	
	/* Palette update in case we are NOT in "real" 8 bit color mode.
	 * Must take place before updating the screen, since the palette must
	 * be set up for conversion from 8bit to 16bit.
	 */ 
	if (((_mode_flags & DF_REAL_8BIT) == 0) && _palette_changed_last != 0) {
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

	/* force a full redraw if requested */
	if (force_full) {
		num_dirty_rects = 1;

		dirty_rect_list[0].x = 0;
		dirty_rect_list[0].y = 0;
		dirty_rect_list[0].w = SCREEN_WIDTH;
		dirty_rect_list[0].h = SCREEN_HEIGHT;
	}

	/* Only draw anything if necessary */
	if (num_dirty_rects > 0) {
	
		SDL_Rect *r; 
		uint32 srcPitch, dstPitch;
		SDL_Rect *last_rect = dirty_rect_list + num_dirty_rects;
	
		/* Convert appropriate parts of the image into 16bpp */
		if ((_mode_flags & DF_REAL_8BIT) == 0) {
			SDL_Rect dst;
			for(r=dirty_rect_list; r!=last_rect; ++r) {
				dst = *r;
				dst.x++;
				dst.y++;
				if (SDL_BlitSurface(sdl_screen, r, sdl_tmpscreen, &dst) != 0)
					error("SDL_BlitSurface failed: %s", SDL_GetError());
			}
		}
	
		SDL_LockSurface(sdl_tmpscreen);
		SDL_LockSurface(sdl_hwscreen);
	
		srcPitch = sdl_tmpscreen->pitch;
		dstPitch = sdl_hwscreen->pitch;
	
		if ((_mode_flags & DF_REAL_8BIT) == 0) {
			for(r=dirty_rect_list; r!=last_rect; ++r) {
				register int dst_y = r->y + _current_shake_pos;
				register int dst_h = 0;
				if (dst_y < SCREEN_HEIGHT) {
					dst_h = r->h;
					if (dst_h > SCREEN_HEIGHT - dst_y)
						dst_h = SCREEN_HEIGHT - dst_y;
					
					r->x <<= 1;
					dst_y <<= 1;
					
					_sai_func((byte*)sdl_tmpscreen->pixels + (r->x+2) + (r->y+1)*srcPitch, srcPitch, NULL, 
						(byte*)sdl_hwscreen->pixels + r->x*scaling + dst_y*dstPitch, dstPitch, r->w, dst_h);
				}
				
				r->y = dst_y;
				r->w <<= 1;
				r->h = dst_h << 1;
			}
		} else {
			for(r=dirty_rect_list; r!=last_rect; ++r) {
				register int dst_y = r->y + _current_shake_pos;
				register int dst_h = 0;
				if (dst_y < SCREEN_HEIGHT) {
					dst_h = r->h;
					if (dst_h > SCREEN_HEIGHT - dst_y)
						dst_h = SCREEN_HEIGHT - dst_y;
					
					dst_y *= scaling;
					
					_sai_func((byte*)sdl_tmpscreen->pixels + r->x + r->y*srcPitch, srcPitch, NULL, 
						(byte*)sdl_hwscreen->pixels + r->x*scaling + dst_y*dstPitch, dstPitch, r->w, dst_h);
				}
				
				r->x *= scaling;
				r->y = dst_y;
				r->w *= scaling;
				r->h = dst_h * scaling;
			}
		}
	
		if (force_full) {
			dirty_rect_list[0].y = 0;
			dirty_rect_list[0].h = SCREEN_HEIGHT * scaling;
		}
		
		SDL_UnlockSurface(sdl_tmpscreen);
		SDL_UnlockSurface(sdl_hwscreen);
	}
	
	/* Palette update in case we are in "real" 8 bit color mode.
	 * Must take place after the screen data was updated, since with
	 * "real" 8bit mode, palatte changes may be visible immediatly,
	 * and we want to avoid any ugly effects.
	 */
	if (_mode_flags & DF_REAL_8BIT && _palette_changed_last != 0) {
		SDL_SetColors(sdl_hwscreen, _cur_pal + _palette_changed_first, 
			_palette_changed_first,
			_palette_changed_last - _palette_changed_first);
		
		_palette_changed_last = 0;
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

	/* reset palette */
	if (_mode_flags & DF_REAL_8BIT)
		SDL_SetColors(sdl_hwscreen, _cur_pal, 0, 256);
	else
		SDL_SetColors(sdl_screen, _cur_pal, 0, 256);

	/* blit image */
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
