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
#include "common/engine.h"	// Only #included for error() and warning()

#ifdef WIN32
int glColorTable(int, int, int, int, int, void *) { return 0; }
int glGetColorTable(int, int, int, void *) { return 0; }
/* Use OpenGL 1.1 */
bool OGL_1_1 = true;
#else
bool OGL_1_1 = false;
#endif

#include "fb2opengl.h"


class OSystem_SDL_GL : public OSystem_SDL_Common {
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
	FB2GL fb2gl;

	void load_gfx_mode();
	void unload_gfx_mode();
	void hotswap_gfx_mode();
};

OSystem_SDL_Common *OSystem_SDL_Common::create() {
	return new OSystem_SDL_GL();
}

void OSystem_SDL_GL::set_palette(const byte *colors, uint start, uint num) {
	const byte *b = colors;
	uint i;

	for(i=0;i!=num;i++) {
	    fb2gl.palette(i+start,b[0],b[1],b[2]);
	    b += 4;
	}

	if (start < _paletteDirtyStart)
		_paletteDirtyStart = start;

	if (start + num > _paletteDirtyEnd)
		_paletteDirtyEnd = start + num;
}

void OSystem_SDL_GL::load_gfx_mode() {
	int gl_flags =  FB2GL_320 | FB2GL_PITCH; 
	_forceFull = true;
	_scaleFactor = 2;
	_mode_flags = 0;
	
	_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, _screenWidth, _screenHeight, 8, 0, 0, 0, 0);
	if (_screen == NULL)
		error("_screen failed failed");

	_mode_flags = DF_WANT_RECT_OPTIM;

	if (_full_screen) gl_flags |= (FB2GL_FS);
	
	if (OGL_1_1) { // OpenGL 1.1
	  gl_flags |= (FB2GL_RGBA | FB2GL_EXPAND);
	  fb2gl.init(640,480,0,70,gl_flags );
	}
	else { // OpenGL 1.2
	  if (!fb2gl.init(640,480,0,70,gl_flags)) { // Try to use 8bpp textures
	    gl_flags |= (FB2GL_RGBA | FB2GL_EXPAND); // using RGBA textures
	    fb2gl.init(640,480,0,70,gl_flags);	
	  }
	}

	SDL_SetGamma(1.25,1.25,1.25);
}

void OSystem_SDL_GL::unload_gfx_mode() {
	if (_screen) {
		SDL_FreeSurface(_screen);
		_screen = NULL; 
	}
}

void OSystem_SDL_GL::update_screen() {

	/* First make sure the mouse is drawn, if it should be drawn. */
	draw_mouse();
	
	/* If the shake position changed, fill the dirty area with blackness */
	if (_currentShakePos != _newShakePos) {

		_currentShakePos = _newShakePos;

	}

	/* Palette update in case we are in "real" 8 bit color mode.
	 * Must take place after the screen data was updated, since with
	 * "real" 8bit mode, palatte changes may be visible immediatly,
	 * and we want to avoid any ugly effects.
	 */
	if (_paletteDirtyEnd != 0) {
                fb2gl.setPalette(_paletteDirtyStart, 
		    _paletteDirtyEnd - _paletteDirtyStart);
		
		_paletteDirtyEnd = 0;
	}

	// FIXME - this seems to be tied to 320x200 - what about Zak256 which needs 320x240 ?
	fb2gl.update(_screen->pixels,320,200,320,0,_currentShakePos);

}

void OSystem_SDL_GL::hotswap_gfx_mode() {
	/* hmm, need to allocate a 320x200 bitmap
	 * which will contain the "backup" of the screen during the change.
	 * then draw that to the new screen right after it's setup.
	 */
	
	byte *bak_mem = (byte*)malloc(_screenWidth*_screenHeight);

	get_screen_image(bak_mem);

	unload_gfx_mode();
	load_gfx_mode();

	fb2gl.setPalette(0,256);
	// FIXME - this seems to be tied to 320x200 - what about Zak256 which needs 320x240 ?
	fb2gl.update(_screen->pixels,320,200,320,0,_currentShakePos);

	/* blit image */
	copy_rect(bak_mem, _screenWidth, 0, 0, _screenWidth, _screenHeight);
	free(bak_mem);

	update_screen();
}

uint32 OSystem_SDL_GL::property(int param, Property *value) {

	if (param == PROP_TOGGLE_FULLSCREEN) {
		_full_screen ^= true;
		SDL_WM_ToggleFullScreen(fb2gl.screen);
		return 1;
	}
	
	return OSystem_SDL_Common::property(param, value);
}
