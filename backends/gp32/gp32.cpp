/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
 * Copyright (C) 2002 ph0x (GP32 port)
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
 */


//#define GPDEBUG //uncomment for debug messages (use DarkFader's emudebug.exe)
#include "gp32.h"

#define MARK printf("%s, %s, %d", __FUNCTION__, __FILE__, __LINE__) //ph0x
#define GP_RGB16(r,g,b) (((((r>>3))&0x1f)<<11)|((((g>>3))&0x1f)<<6)|((((b>>3))&0x1f)<<1))
void GpSetPaletteEntry(u8 i, u8 r, u8 g, u8 b);

#define GAME_SURFACE 0
#define NAMEME_SURFACE 1
#define DEBUG_SURFACE 2

GPDRAWSURFACE gpDraw[3];	// surfaces
int mx=1, my=1;	// wrong if 0?
char currsurface;
FILE *fstderr, *fstdout, *fstdin;

// Set colors of the palette
void OSystem_GP32::set_palette(const byte *colors, uint start, uint num) { 
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

// Set the size of the video bitmap.
// Typically, 320x200
void OSystem_GP32::init_size(uint w, uint h) { 
	// Avoid redundant res changes
	if ((int)w == _screenWidth && (int)h == _screenHeight)
		return;

	_screenWidth = w;
	_screenHeight = h;
	CKSUM_NUM = (_screenWidth*_screenHeight/(8*8));
	if (_dirty_checksums)
		free(_dirty_checksums);
	_dirty_checksums = (uint32*)calloc(CKSUM_NUM*2, sizeof(uint32));

	unload_gfx_mode();
	load_gfx_mode();

}

void OSystem_GP32::add_dirty_rect(int x, int y, int w, int h) {
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
		if (x < 0) { w+=x; x=0; }
		if (y < 0) { h+=y; y=0; }
		if (w > _screenWidth-x) { w = _screenWidth - x; }
		if (h > _screenHeight-y) { h = _screenHeight - y; }
	
		r->x = x;
		r->y = y;
		r->w = w;
		r->h = h;
	}
}

#define ROL(a,n) a = (a<<(n)) | (a>>(32-(n)))
#define DOLINE(x) a ^= ((uint32*)buf)[0+(x)*(_screenWidth/4)]; b ^= ((uint32*)buf)[1+(x)*(_screenWidth/4)]
void OSystem_GP32::mk_checksums(const byte *buf) {
	uint32 *sums = _dirty_checksums;
	uint x,y;
	const uint last_x = (uint)_screenWidth/8;
	const uint last_y = (uint)_screenHeight/8;

	/* the 8x8 blocks in buf are enumerated starting in the top left corner and
	 * reading each line at a time from left to right */
	for(y=0; y != last_y; y++, buf+=_screenWidth*(8-1))
		for(x=0; x != last_x; x++, buf+=8) {
			uint32 a = x;
			uint32 b = y;

			DOLINE(0); ROL(a,13); ROL(b,11);
			DOLINE(2); ROL(a,13); ROL(b,11);
			DOLINE(4); ROL(a,13); ROL(b,11);
			DOLINE(6); ROL(a,13); ROL(b,11);

			a*=0xDEADBEEF;
			b*=0xBAADF00D;

			DOLINE(1); ROL(a,13); ROL(b,11);
			DOLINE(3); ROL(a,13); ROL(b,11);
			DOLINE(5); ROL(a,13); ROL(b,11);
			DOLINE(7); ROL(a,13); ROL(b,11);

			/* output the checksum for this block */
			*sums++=a+b;
		}
}
#undef DOLINE
#undef ROL


void OSystem_GP32::add_dirty_rgn_auto(const byte *buf) {
	assert( ((uint32)buf & 3) == 0);
	
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
		
		for(y=0; y!=_screenHeight/8; y++) {
			for(x=0; x!=_screenWidth/8; x++,ck++) {
				if (ck[0] != ck[CKSUM_NUM]) {
					/* found a dirty 8x8 block, now go as far to the right as possible,
						 and at the same time, unmark the dirty status by setting old to new. */
					w=0;
					do {
						ck[w+CKSUM_NUM] = ck[w];
						w++;
					} while (x+w != _screenWidth/8 && ck[w] != ck[w+CKSUM_NUM]);
					
					add_dirty_rect(x*8, y*8, w*8, 8);

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

// Draw a bitmap to screen.
// The screen will not be updated to reflect the new bitmap
void OSystem_GP32::copy_rect(const byte *buf, int pitch, int x, int y, int w, int h) { 
	if (_screen == NULL)
		return;

	if (pitch == _screenWidth && x==0 && y==0 && w==_screenWidth && h==_screenHeight && _mode_flags&DF_WANT_RECT_OPTIM) {
		/* Special, optimized case for full screen updates.
		 * It tries to determine what areas were actually changed,
		 * and just updates those, on the actual display. */
		add_dirty_rgn_auto(buf);
	} else {
		/* Clip the coordinates */
		if (x < 0) { w+=x; buf-=x; x = 0; }
		if (y < 0) { h+=y; buf-=y*pitch; y = 0; }
		if (w > _screenWidth-x) { w = _screenWidth - x; }
		if (h > _screenHeight-y) { h = _screenHeight - y; }
			
		if (w <= 0 || h <= 0)
			return;

		cksum_valid = false;
		add_dirty_rect(x, y, w, h);
	}

	/* FIXME: undraw mouse only if the draw rect intersects with the mouse rect */
	if (_mouseDrawn)
		undraw_mouse();

	///if (SDL_LockSurface(_screen) == -1)
	///	error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	byte *dst = (byte *)_screen->pixels + y * _screenWidth + x;
	do {
		memcpy(dst, buf, w);
		dst += _screenWidth;
		buf += pitch;
	} while (--h);

	///SDL_UnlockSurface(_screen);
}


SDL_Surface *SDL_CreateRGBSurface
			(Uint32 flags, int width, int height, int depth, 
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) {

	SDL_Surface *surf = (SDL_Surface*)malloc(sizeof(SDL_Surface));
	surf->format = (SDL_PixelFormat*)malloc(sizeof(SDL_PixelFormat));

	if ((flags & SDL_HWSURFACE) == SDL_HWSURFACE) {
		error(">HW surface (w=%d, h=%d)", width, height);		
	} else
	if ((flags & SDL_SWSURFACE) == SDL_SWSURFACE) {
		int size=width*height*(depth/8);
		printf(">SW surface (w=%d, h=%d, size=%d, depth=%d)", width, height, size, depth);
		surf->pixels = malloc(size);
	} else {
		error(">unknown surface", width, height);
		return NULL;
	} 			
	surf->w=width;
	surf->h=height;
	surf->pitch=width*(depth/8);
	surf->format->BitsPerPixel=depth;
	surf->format->BytesPerPixel=depth/8;
	return surf;
}

SDL_Surface *SDL_SetVideoMode
	(int width, int height, int bpp, Uint32 flags) {

	return SDL_CreateRGBSurface(flags, width, height, bpp, 0, 0, 0, 0);
}

void SDL_FreeSurface(SDL_Surface *surface) {
	// implement
}

SDL_Surface *SDL_CreateRGBSurfaceFrom(void *pixels,
			int width, int height, int depth, int pitch,
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) {

// FIXME dont reuse code

	SDL_Surface *surf = (SDL_Surface*)malloc(sizeof(SDL_Surface));
	surf->format = (SDL_PixelFormat*)malloc(sizeof(SDL_PixelFormat));

	surf->w=width;
	surf->h=height;
	surf->pitch=pitch; 	
	surf->pixels=pixels;
	surf->format->BitsPerPixel=depth;
	surf->format->BytesPerPixel=depth/8;
	return surf;
}

int SDL_FillRect
	(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color) {

	// implement
	return 0;
}


void SDL_UpdateRects(SDL_Surface *screen, int numrects, SDL_Rect *rects) {

	// FIXME dont duplicate code :)

	if (screen->format->BitsPerPixel==8) 
	while (numrects--) {
		u8 *s=(u8*)((u8*)screen->pixels+rects->y*320+rects->x);
		u8 *d=(u8*)((u8*)gpDraw[GAME_SURFACE].ptbuffer+rects->x*240+239-rects->y);
		u8 *s2=s, *d2=d;

		for (int x=rects->w; x; x--) {
			for (int y=rects->h; y; y--) {
				*d--=*s;
				s+=320; // FIXME? screen->pitch;
			}
			d2+=240;
			d=d2;
			s2++;
			s=s2;
		}
		rects++;
	}
	else
	if (screen->format->BitsPerPixel==16)
	while (numrects--) {
		u16 *s=(u16*)((u16*)screen->pixels+rects->y*320+rects->x);
		u16 *d=(u16*)((u16*)gpDraw[GAME_SURFACE].ptbuffer+rects->x*240+239-rects->y);
		u16 *s2=s, *d2=d;

		for (int x=rects->w; x; x--) {
			for (int y=rects->h; y; y--) {
				*d--=*s;
				s+=320; // FIXME? screen->pitch;
			}
			d2+=240;
			d=d2;
			s2++;
			s=s2;
		}
		rects++;
	} 
	else error("blitting surface with wrong depth (%d)", screen->format->BitsPerPixel);
	// eh? works also when rects++ is here??
}

//#define SDL_BlitSurface SDL_UpperBlit
int SDL_BlitSurface(SDL_Surface *screen, SDL_Rect *rects,
				SDL_Surface *dst, SDL_Rect *dstrect) {

	// FIXME? role??
	//SDL_UpdateRects(screen, 1, rects); //ph0x! sdl_hwscreen
	return 0;
}

int SDL_SetColors(SDL_Surface *surface, SDL_Color *colors, int firstcolor, int ncolors) {
	if (currsurface == DEBUG_SURFACE) return 1;

	GpPaletteEntryChangeEx(firstcolor, ncolors, (GP_LOGPALENTRY*)colors, 0);
	return 1;
}

// Moves the screen content around by the given amount of pixels
// but only the top height pixel rows, the rest stays untouched
void OSystem_GP32::move_screen(int dx, int dy, int height) { 
	if ((dx == 0) && (dy == 0))
		return;

	if (dx == 0) {
		// vertical movement
		if (dy > 0) {
			// move down
			// copy from bottom to top
			for (int y = height - 1; y >= dy; y--)
				copy_rect((byte *)_screen->pixels + _screenWidth * (y - dy), _screenWidth, 0, y, _screenWidth, 1);
		} else {
			// move up
			// copy from top to bottom
			for (int y = 0; y < height + dx; y++)
				copy_rect((byte *)_screen->pixels + _screenWidth * (y - dy), _screenWidth, 0, y, _screenWidth, 1);
		}
	} else if (dy == 0) {
		// horizontal movement
		if (dx > 0) {
			// move right
			// copy from right to left
			for (int x = _screenWidth - 1; x >= dx; x--)
				copy_rect((byte *)_screen->pixels + x - dx, _screenWidth, x, 0, 1, height);
		} else {
			// move left
			// copy from left to right
			for (int x = 0; x < _screenWidth; x++)
				copy_rect((byte *)_screen->pixels + x - dx, _screenWidth, x, 0, 1, height);
		}
	} else {
		// free movement
		// not neccessary for now
	}
}

void OSystem_GP32::load_gfx_mode() {

	GpRectFill(NULL,&gpDraw[GAME_SURFACE], 0, 0, 320, 240, 0); //black border

	_forceFull = true;
	_mode_flags = DF_WANT_RECT_OPTIM | DF_UPDATE_EXPAND_1_PIXEL;

	sdl_tmpscreen = NULL;
	TMP_SCREEN_WIDTH = (_screenWidth + 3);
	
	switch(_mode) {

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

/*
	// Distinguish 555 and 565 mode
	if (sdl_hwscreen->format->Rmask == 0x7C00)
		Init_2xSaI(555);
	else
		Init_2xSaI(565);
*/
	//Init_2xSaI(555); // ph0x fixme?

 //ph0x fixme - tmpscreen needed?
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

void OSystem_GP32::unload_gfx_mode() {
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


#include "common/util.h"
void OSystem_GP32::draw_mouse() {
	if (!_overlay_visible) {

	if (_mouseDrawn || !_mouseVisible)
		return;

	int x = _mouse_cur_state.x - _mouseHotspotX;
	int y = _mouse_cur_state.y - _mouseHotspotY;
	int w = _mouse_cur_state.w;
	int h = _mouse_cur_state.h;
	byte color;
	byte *src = _mouseData;		// Image representing the mouse
	byte *bak = _mouseBackup;		// Surface used to backup the area obscured by the mouse
	byte *dst;					// Surface we are drawing into

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
	if (w > _screenWidth - x)
		w = _screenWidth - x;
	if (h > _screenHeight - y)
		h = _screenHeight - y;

	// Quick check to see if anything has to be drawn at all
	if (w <= 0 || h <= 0)
		return;

	// Store the bounding box so that undraw mouse can restore the area the
	// mouse currently covers to its original content.
	_mouse_old_state.x = x;
	_mouse_old_state.y = y;
	_mouse_old_state.w = w;
	_mouse_old_state.h = h;

	// Draw the mouse cursor; backup the covered area in "bak"

	///if (SDL_LockSurface(_screen) == -1)
	///	error("SDL_LockSurface failed: %s.\n", SDL_GetError());
	
	// Mark as dirty
	add_dirty_rect(x, y, w, h);

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
		src += _mouse_cur_state.w - w;
		bak += MAX_MOUSE_W - w;
		dst += _screenWidth - w;
		h--;
	}

	///SDL_UnlockSurface(_screen);

	// Finally, set the flag to indicate the mouse has been drawn
	_mouseDrawn = true;
	}

	if (_mouseDrawn || !_mouseVisible)
		return;

	int x = _mouse_cur_state.x - _mouseHotspotX;
	int y = _mouse_cur_state.y - _mouseHotspotY;
	int w = _mouse_cur_state.w;
	int h = _mouse_cur_state.h;
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
		src -= y * _mouse_cur_state.w;
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
	_mouse_old_state.x = x;
	_mouse_old_state.y = y;
	_mouse_old_state.w = w;
	_mouse_old_state.h = h;

	// Draw the mouse cursor; backup the covered area in "bak"

	///if (SDL_LockSurface(sdl_tmpscreen) == -1)
	///	error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	// Mark as dirty
	add_dirty_rect(x, y, w, h);

	dst = (uint16 *)sdl_tmpscreen->pixels + (y+1) * TMP_SCREEN_WIDTH + (x+1);
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
		src += _mouse_cur_state.w - w;
		bak += MAX_MOUSE_W - w;
		dst += TMP_SCREEN_WIDTH - w;
		h--;
	}

	///SDL_UnlockSurface(sdl_tmpscreen);
	
	// Finally, set the flag to indicate the mouse has been drawn
	_mouseDrawn = true; 
}

void OSystem_GP32::undraw_mouse() {	//return; //fixme!
	if (!_overlay_visible) {

	if (!_mouseDrawn)
		return;
	_mouseDrawn = false;

	///if (SDL_LockSurface(_screen) == -1)
	///	error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	byte *dst, *bak = _mouseBackup;
	const int old_mouse_x = _mouse_old_state.x;
	const int old_mouse_y = _mouse_old_state.y;
	const int old_mouse_w = _mouse_old_state.w;
	const int old_mouse_h = _mouse_old_state.h;
	int x, y;

	// No need to do clipping here, since draw_mouse() did that already

	dst = (byte *)_screen->pixels + old_mouse_y * _screenWidth + old_mouse_x;
	for (y = 0; y < old_mouse_h; ++y, bak += MAX_MOUSE_W, dst += _screenWidth) {
		for (x = 0; x < old_mouse_w; ++x) {
			dst[x] = bak[x];
		}
	}

	add_dirty_rect(old_mouse_x, old_mouse_y, old_mouse_w, old_mouse_h);

	///SDL_UnlockSurface(_screen);
	}

	if (!_mouseDrawn)
		return;
	_mouseDrawn = false;

	///if (SDL_LockSurface(sdl_tmpscreen) == -1)
	///	error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	uint16 *dst, *bak = (uint16 *)_mouseBackup;
	const int old_mouse_x = _mouse_old_state.x;
	const int old_mouse_y = _mouse_old_state.y;
	const int old_mouse_w = _mouse_old_state.w;
	const int old_mouse_h = _mouse_old_state.h;
	int x, y;

	// No need to do clipping here, since draw_mouse() did that already

	dst = (uint16 *)sdl_tmpscreen->pixels + (old_mouse_y+1) * TMP_SCREEN_WIDTH + (old_mouse_x+1);
	for (y = 0; y < old_mouse_h; ++y, bak += MAX_MOUSE_W, dst += TMP_SCREEN_WIDTH) {
		for (x = 0; x < old_mouse_w; ++x) {
			dst[x] = bak[x];
		}
	}

	add_dirty_rect(old_mouse_x, old_mouse_y, old_mouse_w, old_mouse_h);

	///SDL_UnlockSurface(sdl_tmpscreen); 
}

char * SDL_GetError() {

	// implement
	return NULL;
}

// Update the dirty areas of the screen
void OSystem_GP32::update_screen() { 
	assert(sdl_hwscreen != NULL);

	// If the shake position changed, fill the dirty area with blackness
	if (_currentShakePos != _newShakePos) {
		SDL_Rect blackrect = {0, 0, _screenWidth*_scaleFactor, _newShakePos*_scaleFactor};
		SDL_FillRect(sdl_hwscreen, &blackrect, 0);

		_currentShakePos = _newShakePos;

		_forceFull = true;
	}

	// Make sure the mouse is drawn, if it should be drawn.
	draw_mouse(); //ph0x
	
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
				dst.x++;	// FIXME? Shift rect by one since 2xSai needs to acces the data around
				dst.y++;	// FIXME? any pixel to scale it, and we want to avoid mem access crashes.				
				if (SDL_BlitSurface(_screen, r, sdl_hwscreen, &dst) != 0) //ph0x! sdl_tmpscreen
					error("SDL_BlitSurface failed: %s", SDL_GetError());				
			}
		}
	
		///SDL_LockSurface(sdl_tmpscreen);
		///SDL_LockSurface(sdl_hwscreen);
	
// ph0x! (no scaling) cannot skip intro if commented?

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

				_scaler_proc((byte*)sdl_tmpscreen->pixels + (r->x*2+2) + (r->y+1)*srcPitch, srcPitch, (byte*)sdl_hwscreen->pixels + r->x*2*_scaleFactor + dst_y*dstPitch, dstPitch, r->w, dst_h);			
			}			
			r->x *= _scaleFactor;
			r->y = dst_y;
			r->w *= _scaleFactor;
			r->h = dst_h * _scaleFactor;
		}

		///SDL_UnlockSurface(sdl_tmpscreen);
		///SDL_UnlockSurface(sdl_hwscreen);

		// Readjust the dirty rect list in case we are doing a full update.
		// This is necessary if shaking is active.
		if (_forceFull) {
			_dirty_rect_list[0].y = 0;
			_dirty_rect_list[0].h = _screenHeight * _scaleFactor;
		}

		// Finally, blit all our changes to the screen

		// FIXME (dont use condition)
		if (_overlay_visible)
			SDL_UpdateRects(sdl_hwscreen, _num_dirty_rects, _dirty_rect_list); //ph0x! sdl_hwscreen
		else
			SDL_UpdateRects(_screen, _num_dirty_rects, _dirty_rect_list);
	}

	_num_dirty_rects = 0;
	_forceFull = false;
}

// Either show or hide the mouse cursor
bool OSystem_GP32::show_mouse(bool visible) { 
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
	
// Set the position of the mouse cursor
void OSystem_GP32::set_mouse_pos(int x, int y) { 
	if (x != _mouse_cur_state.x || y != _mouse_cur_state.y) {
		_mouse_cur_state.x = x;
		_mouse_cur_state.y = y;

		mx=x;	//ph0x fixme
		my=y; //ph0x fixme
		undraw_mouse();
	}
}

void OSystem_GP32::warp_mouse(int x, int y) {
	set_mouse_pos(x, y);	
}

// Set the bitmap that's used when drawing the cursor.
void OSystem_GP32::set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y) { 
	_mouse_cur_state.w = w;
	_mouse_cur_state.h = h;

	_mouseHotspotX = hotspot_x;
	_mouseHotspotY = hotspot_y;

	_mouseData = (byte*)buf;

	undraw_mouse();
}
	
// Shaking is used in SCUMM. Set current shake position.
void OSystem_GP32::set_shake_pos(int shake_pos) { 
	_newShakePos = shake_pos;
}
		
// Get the number of milliseconds since the program was started.
uint32 OSystem_GP32::get_msecs() { 
	return GpTickCountGet(); 
}
	
// Delay for a specified amount of milliseconds
void OSystem_GP32::delay_msecs(uint msecs) { 
	int n = GpTickCountGet();
	while ( ( GpTickCountGet() - n ) < msecs) ;
}
	
// Create a thread
void OSystem_GP32::create_thread(ThreadProc *proc, void *param) { }
	
// Get the next event.
// Returns true if an event was retrieved.	

bool OSystem_GP32::poll_event(Event *event) { 	// fixme: make more user-friendly :)

	#define EVENT_COUNT	2 // >=1
	#define MOUSE_MIPS	1 // bg updates wrong if >1 ??
	
	static int oldkey, eventcount=EVENT_COUNT, lastevent=0;
	int key;	

	key=GpKeyGet();	
		if (key == GPC_VK_NONE) {
			if (lastevent==EVENT_LBUTTONDOWN) {
				lastevent=0;
				event->event_code = EVENT_LBUTTONUP;
				return true;
			}
			return false;
		}	

		if (key == oldkey) {
			eventcount--;
			if (eventcount) return false;
		}
		oldkey=key;
		eventcount=EVENT_COUNT;

		event->event_code = EVENT_KEYDOWN;

		if (key & GPC_VK_FL && key & GPC_VK_FR) { // L+R = save state
			printf("Saving game, please wait...");

			//extern void autosave(void * engine); 
			//autosave(NULL); //FIXME?
			do key=GpKeyGet(); while (key != GPC_VK_NONE) ;
			return false;
		} else

		if(key & GPC_VK_FL) { // L = debug console
			//GpGraphicModeSet(8, NULL); //FIXME: if 16bit?
			currsurface=DEBUG_SURFACE;
			GpSurfaceFlip(&gpDraw[currsurface]);
			GpSetPaletteEntry ( 0, 0,0,0 );
			GpSetPaletteEntry ( 1, 255,0,0 );
			GpSetPaletteEntry ( 2, 255,255,255 );
			return false;
		} else

		if (key & GPC_VK_FR) {  // R = game screen
			//if (_overlay_visible) GpGraphicModeSet(16, NULL); 
			//	else GpGraphicModeSet(8, NULL);
			currsurface=GAME_SURFACE;
			GpSurfaceFlip(&gpDraw[currsurface]);

			_paletteDirtyStart=0;
			_paletteDirtyEnd=255; //fixme?			
			return false;
		}

		if(key & GPC_VK_START) { // START = menu
			event->kbd.keycode = 319;
			event->kbd.ascii = 319;
			return true;			
		}

		if(key & GPC_VK_SELECT) { // SELECT == escape/skip
			if (_overlay_visible) 
				do key=GpKeyGet(); while (key != GPC_VK_NONE) ; // prevent 2xESC
			event->kbd.keycode = 27;
			event->kbd.ascii = 27;		
			return true;
		}		

		if (key & GPC_VK_FA) {
			lastevent=EVENT_LBUTTONDOWN;
			event->event_code = EVENT_LBUTTONDOWN;
			return true;
		}
		if (key & GPC_VK_FB) {
			lastevent=EVENT_RBUTTONDOWN;
			event->event_code = EVENT_RBUTTONDOWN;
			return true;
		}

		event->event_code = EVENT_MOUSEMOVE;

		if(key & GPC_VK_LEFT) {
			mx-=MOUSE_MIPS;
			if (mx<1) mx=1; // wrong if 0?
		}

		if(key & GPC_VK_RIGHT) {
			mx+=MOUSE_MIPS;
			if (mx>319) mx=319;
		}

		if(key & GPC_VK_UP) {
			my-=MOUSE_MIPS;
			if (my<1) my=1; // wrong if 0?
		}

		if(key & GPC_VK_DOWN) {
			my+=MOUSE_MIPS;
			if (my>199) my=199;
		}

		event->event_code = EVENT_MOUSEMOVE;
		km.x = event->mouse.x = mx;
		km.y = event->mouse.y = my;

		event->mouse.x /= _scaleFactor;
		event->mouse.y /= _scaleFactor;	
}

// Set the function to be invoked whenever samples need to be generated
// Format is the sample type format.
// Only 16-bit signed mode is needed for simon & scumm
bool OSystem_GP32::set_sound_proc(SoundProc *proc, void *param, SoundFormat format) { 
	return false; 
}

void OSystem_GP32::clear_sound_proc() {
	//_sound_proc = NULL;
	//_sound_proc_param = NULL;
}

void OSystem_GP32::get_screen_image(byte *buf) {
	/* make sure the mouse is gone */
	undraw_mouse();
	
	///if (SDL_LockSurface(_screen) == -1)
	///	error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	memcpy(buf, _screen->pixels, _screenWidth*_screenHeight);

	///SDL_UnlockSurface(_screen);
}

void OSystem_GP32::hotswap_gfx_mode() {
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
	
// Get or set a property
uint32 OSystem_GP32::property(int param, Property *value) {
	switch(param) {

	case PROP_GET_FULLSCREEN:
		return _full_screen;

	case PROP_OPEN_CD: //fixme?
		/*if (SDL_InitSubSystem(SDL_INIT_CDROM) == -1)
			_cdrom = NULL;
		else {
			_cdrom = SDL_CDOpen(value->cd_num);
			// Did if open? Check if _cdrom is NULL 
			if (!_cdrom) {
				warning("Couldn't open drive: %s\n", SDL_GetError());
			}
		}*/ 
		break;

	case PROP_SET_GFX_MODE:
		if (value->gfx_mode >= 7)
			return 0;
		_mode = value->gfx_mode;
		hotswap_gfx_mode();

		return 1;

	case PROP_SHOW_DEFAULT_CURSOR:
		///SDL_ShowCursor(value->show_cursor ? SDL_ENABLE : SDL_DISABLE);	//fixme?
		break;

	case PROP_GET_SAMPLE_RATE:
		///return SAMPLES_PER_SEC; //ph0x fixme
		return 22050;
	}

	return 0;
}
		
// Poll cdrom status
// Returns true if cd audio is playing
bool OSystem_GP32::poll_cdrom() { return false; }

// Play cdrom audio track
void OSystem_GP32::play_cdrom(int track, int num_loops, int start_frame, int end_frame) { }

// Stop cdrom audio track
void OSystem_GP32::stop_cdrom() { }

// Update cdrom audio status
void OSystem_GP32::update_cdrom() { }

// Add a new callback timer
void OSystem_GP32::set_timer(int timer, int (*callback)(int)) { }

// Mutex handling
OSystem::MutexRef OSystem_GP32::create_mutex() {
	return NULL;
}
void OSystem_GP32::lock_mutex(MutexRef mutex) { }
void OSystem_GP32::unlock_mutex(MutexRef mutex) { }
void OSystem_GP32::delete_mutex(MutexRef mutex) { }

// Quit
void gphalt(int);
void OSystem_GP32::quit() { 
	exit(0);
}
	
// Overlay
void OSystem_GP32::show_overlay() { 
	// hide the mouse

	undraw_mouse();

u8* s=(u8*)_screen->pixels;
u16* d=(u16*)sdl_tmpscreen->pixels;
u8 c;
// convert to 16 bit
for (int y=0; y<200; y++) {
	for (int x=0; x<320; x++) {
		c=*s;
		*d++ = (u16)GP_RGB16(_currentPalette[c].r, _currentPalette[c].g, _currentPalette[c].b);
		s++;
	}
	d+=3; // tmpscreen width is screen+3
}	
GpGraphicModeSet(16, NULL); //ph0x
//GpRectFill(NULL,&gpDraw[GAME_SURFACE], 0, 0, 320, 240*2, 0); //black border

	_overlay_visible = true;
	clear_overlay();
}

void OSystem_GP32::hide_overlay() { 
	// hide the mouse
	undraw_mouse();

GpGraphicModeSet(8, NULL); //ph0x
GpRectFill(NULL,&gpDraw[GAME_SURFACE], 0, 200, 320, 40, 0); //black border

	_overlay_visible = false;
	_forceFull = true;
}

void OSystem_GP32::clear_overlay() { 
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
	if (SDL_BlitSurface(_screen, &src, sdl_tmpscreen, &dst) != 0) //FIXME
		error("SDL_BlitSurface failed: %s", SDL_GetError());

	_forceFull = true;
}

void OSystem_GP32::grab_overlay(int16 *buf, int pitch) { 
	if (!_overlay_visible)
		return;

	if (sdl_tmpscreen == NULL)
		return;

	// hide the mouse
	undraw_mouse();

	///if (SDL_LockSurface(sdl_tmpscreen) == -1)
	///	error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	int16 *src = (int16 *)sdl_tmpscreen->pixels + TMP_SCREEN_WIDTH + 1;
	int h = _screenHeight;
	do {
		memcpy(buf, src, _screenWidth*2);
		src += TMP_SCREEN_WIDTH;
		buf += pitch;
	} while (--h);

	///SDL_UnlockSurface(sdl_tmpscreen);
}

void OSystem_GP32::copy_rect_overlay(const int16 *buf, int pitch, int x, int y, int w, int h) { 
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

	///if (SDL_LockSurface(sdl_tmpscreen) == -1)
	///	error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	int16 *dst = (int16 *)sdl_tmpscreen->pixels + (y+1) * TMP_SCREEN_WIDTH + (x+1);
	do {
		memcpy(dst, buf, w*2);
		dst += TMP_SCREEN_WIDTH;
		buf += pitch;
	} while (--h);

	///SDL_UnlockSurface(sdl_tmpscreen);
}

OSystem *OSystem_GP32::create(int gfx_mode, bool full_screen) {
	OSystem_GP32 *syst = new OSystem_GP32();
	
	syst->_mode = gfx_mode;
	syst->_full_screen = full_screen;
	
	// allocate palette storage
	syst->_currentPalette = (SDL_Color*)calloc(sizeof(SDL_Color), 256);

	// allocate the dirty rect storage
	syst->_mouseBackup = (byte*)malloc(MAX_MOUSE_W * MAX_MOUSE_H * MAX_SCALING * 2);
	return syst;
}

OSystem *OSystem_GP32_create(int gfx_mode, bool full_screen) { 
	return OSystem_GP32::create(gfx_mode, full_screen); // fullscreen	
}

//////////////////////////////////////////////////
// GP32 stuff
//////////////////////////////////////////////////

extern "C" int write(int fd, void *p, size_t n);
int write(int fd, void *p, size_t n) { return 0; } //ph0x hack!

// Converts 8bit rgb values to a GP32 palette value 
void GpSetPaletteEntry(u8 i, u8 r, u8 g, u8 b) {
  GP_PALETTEENTRY entry = GP_RGB16(r,g,b);
  GpPaletteEntryChange ( i, 1, &entry, 0 );
}

void switchsurf(int surf) {
	GPLCDINFO lcd;
	GpLcdInfoGet(&lcd);

	if (surf == DEBUG_SURFACE) {
		if (lcd.lcd_global.U8_lcd.bpp == 16)
			GpGraphicModeSet(8, NULL);

		currsurface = DEBUG_SURFACE;
		GpSurfaceFlip(&gpDraw[currsurface]);
		GpSetPaletteEntry(0, 0, 0, 0);
		GpSetPaletteEntry(1, 255, 0, 0);
		GpSetPaletteEntry(2, 255, 255, 255);
	} else {
		if (surf == GAME_SURFACE) {
			currsurface = GAME_SURFACE;
			GpSurfaceFlip(&gpDraw[currsurface]);
		}
	}
			
}

int gpprintf(const char *fmt, ...) { //return 0; //fixme
	static int y;
	char s[1024]; // ?
	int r;
	va_list marker;
	
	// combine
	va_start(marker, fmt);
	r = vsprintf(s, fmt, marker);
	va_end(marker);	
	// print to console

#ifdef GPDEBUG	
//dprintf("mem: %d  ", gm_availablesize());
	dprintf(s);
	if (s[strlen(s)-1] != '\n') dprintf("\n");			
	//if (s[0]!='>') return r;
#endif		
	
	// print to lcd
	GpTextOut(NULL, &gpDraw[DEBUG_SURFACE], 0, y, s, 1); 
	y+= (ENGFONT_H-FONT_LINEGAP);
	if (y>(240/(ENGFONT_H-FONT_LINEGAP)) * (ENGFONT_H-FONT_LINEGAP)) {
		y=0;
		GpRectFill(NULL,&gpDraw[DEBUG_SURFACE], 0, 0, 320, 240, 2);
	}
	return r;
}

int gpfprintf(FILE *stream, const char *fmt, ...) {
	//printf(fmt, "fixme");
}

FILE *gpfopen(const char *filename, const char *mode) { 
	//FIXME:
	// - allocation ?
	// - mode
	// - malloc -> new
	ulong m;
	FILE *f = (FILE*)malloc(sizeof(FILE) + sizeof(ulong));

	//printf(">open %s as %s", filename, mode); 

	// FIXME add binary/text support
	if (tolower(mode[0])=='r') { 
		m=OPEN_R;
		GpFileGetSize(filename, (ulong*)(f+1)); // hack (size in handle :)
	} else
	if (tolower(mode[0])=='w') {
		//printf("open if as W");

		*(ulong*)(f+1)=0; // FIXME? new file has no size?
		m=OPEN_W;
		GpFileCreate(filename, ALWAYS_CREATE, f);
	}
	else error("wrong file mode");

	if (!f) error("%s: cannot crate F_HANDLE", __FUNCTION__);
	ERR_CODE err = GpFileOpen(filename, m, f);
	if (err) {
		//if (strcmp(filename, "tentacle.000")==0 || strcmp(filename, "TENTACLE.000")==0) error(">bingo!");
		//if (blah>1) error("(%s) %s", filename, __FUNCTION__);	else 
		return NULL; 
	}	else return f;
}



int gpfclose(FILE *stream) {	
	if (*(u32*)((char*)stream-sizeof(u32)) == 0x4321) {
		debug(0, "double closing", __FUNCTION__); 
		return 1;
	} // return 1 ??
	ERR_CODE err = GpFileClose(*stream);
	free(stream);
	return err;
}

int gpfseek(FILE *stream, long offset, int whence) {
	ulong dummy;

	switch (whence) {
		case SEEK_SET : whence = FROM_BEGIN; break;
		case SEEK_CUR : whence = FROM_CURRENT; break;
		case SEEK_END : whence = FROM_END; break;
	}
	return GpFileSeek(*stream, whence, offset, (long*)&dummy);
}

long gpftell(FILE *stream) { // fixme? use standard func
	ulong pos=0;
	ERR_CODE err = GpFileSeek(*stream, FROM_CURRENT, 0, (long*)&pos);
	return pos;
}


size_t gpfread(void *ptr, size_t size, size_t n, FILE *stream) {
	ulong readcount=0;
	ERR_CODE err = GpFileRead(*stream, ptr, size*n, &readcount); //fixme? size*n
	return readcount/size; //FIXME?
}

size_t gpfwrite(const void *ptr, size_t size, size_t n, FILE *stream) {
	ERR_CODE err=GpFileWrite(*stream, ptr, size*n); //fixme size*n?
	//printf("writing to file");
	return err;
}

void gpclearerr(FILE *stream) {
	//error("fixme: %s", __FUNCTION__);
}

int gpfeof(FILE *stream) { //fixme!
	return ftell(stream) >= *(ulong*)(stream+1);
}

char *gpfgets(char *s, int n, FILE *f) {
	int err, i=0;

	while (!feof(f) && i<n) {
		fread(&s[i], 1, 1, f);
		if (s[i]=='\n') {
			s[i+1]=0;
			return s;
		}
		i++;
	}
	if (feof(f))
		return NULL;
	else return s;
}

int gpfflush(FILE *stream) { return 0;}

void *gpmalloc(size_t size) {
	void *p = gm_malloc(size+sizeof(u32)); // gm_zi_malloc(size+sizeof(u32));

	//memset((char*)((char*)p+sizeof(u32)), 0, size);
	//printf("callocing");	
	if (p) {
		*(u32*)p = 0x1234;	
		return ((char*)p+sizeof(u32));
	} else return NULL;
}

void *gpcalloc(size_t nitems, size_t size) { 
	void *p = gpmalloc(nitems*size); //gpcalloc doesnt clear?
	
	memset(p, 0, nitems*size);	
	if (*(u8*)p) warning("%s: calloc doesn't clear!", __FUNCTION__);	//fixme: was error
	//printf("callocing");	
	return p; 	
}

void gpfree(void *block) {
	if (!block) {debug(0, "freeing null pointer"); return;}
	if (*(u32*)((char*)block-sizeof(u32)) == 0x4321) error("%s: double deallocation!", __FUNCTION__);
	if (*(u32*)((char*)block-sizeof(u32)) != 0x1234) error("%s: corrupt block!", __FUNCTION__);
	*(u32*)((char*)block-sizeof(u32)) = 0x4321;
	gm_free((char*)block-sizeof(u32));
}

void gphalt(int code=0) {
	GpGraphicModeSet(8, NULL);
	currsurface=DEBUG_SURFACE;
	GpSurfaceFlip(&gpDraw[currsurface]);
	GpSetPaletteEntry ( 0, 0,0,0 );
	GpSetPaletteEntry ( 1, 255,0,0 );
	GpSetPaletteEntry ( 2, 255,255,255 );
	printf("HALT!");
	while (1); 
}

char *gpstrdup(const char *strSource) {

	char *buffer;
	buffer = (char *)malloc(strlen(strSource) + 1);
	if (buffer)
			strcpy(buffer, strSource);
	return buffer;
}

time_t gptime(time_t *timer) {

	time_t t = GpTickCountGet() / 1000;
	if (timer)
		*timer = t;

	return t;
}

void gpdeinit() {
	fclose(fstdin);
	fclose(fstdout);
	fclose(fstderr);
}

void gpexit(int code) {

	switchsurf(DEBUG_SURFACE);

	printf("Your GP32 will now restart...");
	gpdeinit();
	GpAppExit();
}

//#include <string.h>
#include "common/gamedetector.h"
VersionSettings* menu() {
	const VersionSettings *v = version_settings;
	VersionSettings* games[30];
	int n=0;

	/*GpSetPaletteEntry ( 0, 0,0,0 );
	GpSetPaletteEntry ( 1, 255,0,0 );
	GpSetPaletteEntry ( 2, 255,255,255 );*/

	currsurface=GAME_SURFACE;
	GpSurfaceFlip(&gpDraw[currsurface]);
	printf("menu");
	
	char s[256];
	while (v->filename && v->gamename) {
		sprintf(s, "%s.000", v->filename); //fixme? (extension ok?)
		FILE* f = fopen(s, "r");	
		if (f) {			
			(const VersionSettings*)games[n++]=v;
			fclose(f);			
		}
		v++;
	}

	int i, key, fg, bg, choice=0, y=0;	

	//GpRectFill(NULL,&gpDraw[currsurface], 0, 0, 320, 200, 2);
	GpTextOut(NULL, &gpDraw[currsurface], 0, y, "ScummVM (GP32 port by ph0x)", 255); y+=ENGFONT_H;

	if (!n) {		
		GpTextOut(NULL, &gpDraw[currsurface], 0, y, "No games found! put game data in gp:\\", 255); y+=ENGFONT_H;
		while (1);
	}
	

	if (n==1) return games[choice]; //fixme?
	GpTextOut(NULL, &gpDraw[currsurface], 0, y, "select game:", 255); y+=ENGFONT_H;
	do {		
		for (i=0; i<n; i++) {
			if (i==choice) {fg=100; bg=255;} else {fg=123; bg=0;}
			GpRectFill(NULL,&gpDraw[currsurface], 0, y+i*ENGFONT_H, 320, ENGFONT_H, bg);
			GpTextOut(NULL, &gpDraw[currsurface], 0, y+i*ENGFONT_H, (char*)games[i]->gamename, fg);
		}	
		
		do key=GpKeyGet(); while (key  == GPC_VK_NONE) ;
		if (key  & GPC_VK_DOWN)
			{if (choice<n-1) choice++;} else
		if (key  & GPC_VK_UP)
			{if (choice>0) choice--;} else
		if (key  & GPC_VK_START || key  & GPC_VK_FA) return games[choice]; //fixme?
		do key=GpKeyGet(); while (key != GPC_VK_NONE) ;
	
	} while (1);
}

int gpinit() {	

	ERR_CODE err;
	
	//GpKeyInit();
	GpFatInit();
	GpRelativePathSet("gp:\\gpmm");

	// Initialize graphics 
	GpGraphicModeSet(8, NULL);
	GpLcdSurfaceGet(&gpDraw[DEBUG_SURFACE], DEBUG_SURFACE);
	GpLcdSurfaceGet(&gpDraw[NAMEME_SURFACE], NAMEME_SURFACE);		
	GpLcdSurfaceGet(&gpDraw[GAME_SURFACE], GAME_SURFACE);	
	
	GpSetPaletteEntry(0, 0, 0, 0);
	GpSetPaletteEntry(1, 255, 0, 0);
	GpSetPaletteEntry(2, 255, 255, 255);

	// fixme - use get function
	currsurface=DEBUG_SURFACE;
	GpSurfaceSet(&gpDraw[currsurface]);

	GpLcdEnable();
	
#ifdef GPDEBUG
	printf(">waiting for debugger...");
	InitDebug();
#endif
	printf(">Running ScummVM");

}	

extern "C" void GpMain (void * arg); // hack
void GpMain (void * arg) {
	gpinit();
	
	// fixme - use get function
	currsurface=GAME_SURFACE;
	GpSurfaceFlip(&gpDraw[currsurface]);

	char *argv[] = { "scummvm", /*(char*)menu()->filename*/ NULL };
	int argc = 1;

	extern int main(int argc, char *argv[]);
	main(argc, argv);
	
	error("returned from main ?!");	
}
