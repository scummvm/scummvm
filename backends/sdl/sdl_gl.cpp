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

#ifdef WIN32
int glColorTable(int, int, int, int, int, void *) { return 0; }
int glGetColorTable(int, int, int, void *) { return 0; }
#endif

#include "fb2opengl.h"

class OSystem_SDL_OpenGL : public OSystem_SDL_Common {
public:
	OSystem_SDL_OpenGL() { _glScreenStart = 0; _glBilinearFilter = true; }

	// Set colors of the palette
	void set_palette(const byte *colors, uint start, uint num);

	// Update the dirty areas of the screen
	void update_screen();

	// Set a parameter
	uint32 property(int param, Property *value);

protected:
	FB2GL fb2gl;
	int gl_flags;
	int _glScreenStart;
	bool _glBilinearFilter;
	SDL_Surface *tmpSurface; // Used for black rectangles blitting 
	SDL_Rect tmpBlackRect; // Black rectangle at end of the GL screen

	virtual void load_gfx_mode();
	virtual void unload_gfx_mode();
	void hotswap_gfx_mode();
};

OSystem_SDL_Common *OSystem_SDL_Common::create() {
	return new OSystem_SDL_OpenGL();
}

void OSystem_SDL_OpenGL::set_palette(const byte *colors, uint start, uint num) {
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

void OSystem_SDL_OpenGL::load_gfx_mode() {
	uint32 Rmask, Gmask, Bmask, Amask;
	// I have to force 16 bit color depth with 565 ordering
	// SDL_SetVideoMode sometimes doesn't accept your color depth definition
	Rmask = 0xF800; // 5
	Gmask = 0x07E0; // 6
	Bmask = 0x001F; // 5
	Amask = 0;
	
	_forceFull = true;
	_mode_flags = DF_WANT_RECT_OPTIM | DF_UPDATE_EXPAND_1_PIXEL;
	_scaleFactor = 2;

	_tmpscreen = NULL;
	_tmpScreenWidth = (_screenWidth + 3);
	
	//
	// Create the surface that contains the 8 bit game data
	//
	_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, _screenWidth, _screenHeight, 8, 0, 0, 0, 0);
	if (_screen == NULL)
		error("_screen failed");


	//
	// Create the surface that contains the scaled graphics in 16 bit mode
	//

	gl_flags =  FB2GL_320 | FB2GL_RGBA | FB2GL_16BIT;
        if (_full_screen) {
	  gl_flags |= (FB2GL_FS);
	  _glScreenStart = 0;
	}
	// 640x480 screen resolution
	fb2gl.init(640,480,0,_glScreenStart? 15: 70,gl_flags);

	SDL_SetGamma(1.25,1.25,1.25);

	
	//
	// Create the surface used for the graphics in 16 bit before scaling, and also the overlay
	//

	// Need some extra bytes around when using 2xSaI
	uint16 *tmp_screen = (uint16*)calloc(_tmpScreenWidth*(_screenHeight+3),sizeof(uint16));
	_tmpscreen = SDL_CreateRGBSurfaceFrom(tmp_screen,
						_tmpScreenWidth, _screenHeight + 3, 16, _tmpScreenWidth*2,
						Rmask,
						Gmask,
						Bmask,
						Amask);

	tmpSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, _screenWidth, 
						// 320x256 texture (black end)
						256-_screenHeight-_glScreenStart,
						16,
						Rmask,
						Gmask,
						Bmask,
						Amask);

	tmpBlackRect.x = 0;
	tmpBlackRect.y = 0;
	tmpBlackRect.w = _screenWidth;
	tmpBlackRect.h = 256-_screenHeight-_glScreenStart;
	
	if (_tmpscreen == NULL)
		error("_tmpscreen failed");
	
	// keyboard cursor control, some other better place for it?
	km.x_max = _screenWidth * _scaleFactor - 1;
	km.y_max = _screenHeight * _scaleFactor - 1;
	km.delay_time = 25;
	km.last_time = 0;

}

void OSystem_SDL_OpenGL::unload_gfx_mode() {
	if (_screen) {
		SDL_FreeSurface(_screen);
		_screen = NULL; 
	}

	if (_tmpscreen) {
		free((uint16*)_tmpscreen->pixels);
		SDL_FreeSurface(_tmpscreen);
		_tmpscreen = NULL;
	}
}

void OSystem_SDL_OpenGL::update_screen() {
	
	// If the shake position changed, fill the dirty area with blackness
	if (_currentShakePos != _newShakePos) {
		SDL_Rect blackrect = {0, _glScreenStart, _screenWidth, _newShakePos+_glScreenStart};
		
		SDL_FillRect(tmpSurface, &blackrect, 0);
		fb2gl.blit16(tmpSurface,1,&blackrect,0,0);

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
		SDL_Rect *last_rect = _dirty_rect_list + _num_dirty_rects;
	
		// Convert appropriate parts of the 8bpp image into 16bpp
		if (!_overlayVisible) {
			SDL_Rect dst;
			for(r = _dirty_rect_list; r != last_rect; ++r) {
				dst = *r;
//				dst.x++;	// Shift rect by one since 2xSai needs to acces the data around
//				dst.y++;	// any pixel to scale it, and we want to avoid mem access crashes.
				if (SDL_BlitSurface(_screen, r, _tmpscreen, &dst) != 0)
					error("SDL_BlitSurface failed: %s", SDL_GetError());
			}
		}

		// Almost the same thing as SDL_UpdateRects
		fb2gl.blit16(_tmpscreen,_num_dirty_rects,_dirty_rect_list,0,
		    _currentShakePos+_glScreenStart);

		tmpBlackRect.h = 256-_screenHeight-_glScreenStart-_currentShakePos;
		
		SDL_FillRect(tmpSurface, &tmpBlackRect, 0);
		fb2gl.blit16(tmpSurface,1,&tmpBlackRect,0,_screenHeight+_glScreenStart+_currentShakePos);

		fb2gl.display();
	}

	_num_dirty_rects = 0;
	_forceFull = false;
}

void OSystem_SDL_OpenGL::hotswap_gfx_mode() {
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

uint32 OSystem_SDL_OpenGL::property(int param, Property *value) {

	if (param == PROP_TOGGLE_FULLSCREEN) {
		_full_screen ^= true;
	
		SDL_WM_ToggleFullScreen(fb2gl.screen);
		return 1;
	}
	else if (param == PROP_SET_GFX_MODE) {
		SDL_Rect full = {0,0,_screenWidth,_screenHeight};
		glPopMatrix();

		switch(value->gfx_mode) {
		  case 0: // Bilinear Filtering (on/off)
		    _glBilinearFilter ^= true;
		    for (int i=0; i<2; i++) {
		      glBindTexture(GL_TEXTURE_2D,i);
		      if (_glBilinearFilter) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
			    GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
			    GL_LINEAR);
		      }
		      else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
			    GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
			    GL_NEAREST);
		      }
		    }
		    break;
		  case 1: // Don't fit the whole screen
		    fb2gl.init(0,0,0,15,gl_flags);
		    _glScreenStart = 20;
		    SDL_FillRect(tmpSurface,&tmpBlackRect,0);
		    fb2gl.blit16(tmpSurface,1,&tmpBlackRect,0,0);
		    break;
		  case 2: // Fit the whole screen
		    fb2gl.init(0,0,0,70,gl_flags);
		    _glScreenStart = 0;
		    break;
		  default: // Zooming
		    glPushMatrix();
/*		    SDL_FillRect(tmpSurface, &full, 0);
		    fb2gl.blit16(tmpSurface,1,&full,0,_glScreenStart);
		    fb2gl.display();
		    double x = (double)((_mouseCurState.x) 
			- (_screenWidth/2)) / (_screenWidth/2);
		    double y = (double)((_mouseCurState.y) 
			- (_screenHeight/2)) / (_screenHeight/2);
		    glTranslatef(-x,y,0);
*/
		    glScalef(1.0+(double)(value->gfx_mode-1)/10,
		      1.0+(double)(value->gfx_mode-1)/10,
		      0);
		};
		fb2gl.blit16(_tmpscreen,1,&full,0,_glScreenStart);
		fb2gl.display();
		
		return 1;
	}
	
	
	return OSystem_SDL_Common::property(param, value);
}
