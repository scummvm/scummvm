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

	if (start < _palette_changed_first)
		_palette_changed_first = start;

	if (start + num > _palette_changed_last)
		_palette_changed_last = start + num;
}

void OSystem_SDL_GL::load_gfx_mode() {
	int gl_flags =  FB2GL_320 | FB2GL_PITCH; 
	force_full = true;
	scaling = 1;
	_mode_flags = 0;

	_sai_func = NULL;
	sdl_tmpscreen = NULL;

	/* It's easier to work with 8 bit (256 colors) */
	_mode_flags |= DF_REAL_8BIT;
	
	sdl_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, 8, 0, 0, 0, 0);
	if (sdl_screen == NULL)
		error("sdl_screen failed failed");

	_sai_func = Normal1x;

	_mode_flags = DF_WANT_RECT_OPTIM | DF_REAL_8BIT;

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
	  
	sdl_tmpscreen = sdl_screen;
}

void OSystem_SDL_GL::unload_gfx_mode() {
	if (sdl_screen) {
		SDL_FreeSurface(sdl_screen);
		sdl_screen = NULL; 
	}

	if (_mode_flags & DF_SEPARATE_TEMPSCREEN) {
		free((uint16*)sdl_tmpscreen->pixels);
		SDL_FreeSurface(sdl_tmpscreen);
	}
	sdl_tmpscreen = NULL;
}

void OSystem_SDL_GL::update_screen() {

	/* First make sure the mouse is drawn, if it should be drawn. */
	draw_mouse();
	
	/* If the shake position changed, fill the dirty area with blackness */
	if (_current_shake_pos != _new_shake_pos) {

		_current_shake_pos = _new_shake_pos;

	}

	/* Palette update in case we are in "real" 8 bit color mode.
	 * Must take place after the screen data was updated, since with
	 * "real" 8bit mode, palatte changes may be visible immediatly,
	 * and we want to avoid any ugly effects.
	 */
	if (_palette_changed_last != 0) {
                fb2gl.setPalette(_palette_changed_first, 
		    _palette_changed_last - _palette_changed_first);
		
		_palette_changed_last = 0;
	}

	fb2gl.update(sdl_tmpscreen->pixels,320,200,320,0,_current_shake_pos);

}

void OSystem_SDL_GL::hotswap_gfx_mode() {
	/* hmm, need to allocate a 320x200 bitmap
	 * which will contain the "backup" of the screen during the change.
	 * then draw that to the new screen right after it's setup.
	 */
	
	byte *bak_mem = (byte*)malloc(SCREEN_WIDTH*SCREEN_HEIGHT);

	get_320x200_image(bak_mem);

	unload_gfx_mode();
	load_gfx_mode();

	fb2gl.setPalette(0,256);
	fb2gl.update(sdl_tmpscreen->pixels,320,200,320,0,_current_shake_pos);

	/* blit image */
	copy_rect(bak_mem, SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
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
