/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
	OSystem_SDL_OpenGL();

	// Set colors of the palette
	void set_palette(const byte *colors, uint start, uint num);

	// Update the dirty areas of the screen
	void update_screen();

	// Set a parameter
	uint32 property(int param, Property *value);

	// Get the next event.
	// Returns true if an event was retrieved.	
//	bool poll_event(Event *event);

protected:
	FB2GL fb2gl;
	int _glFlags;
	int _glScreenStart;
	bool _glBilinearFilter;
	bool _glAspectRatio;
	bool _usingOpenGL;
	SDL_Surface *tmpSurface; // Used for black rectangles blitting 
	SDL_Rect tmpBlackRect;   // Black rectangle at end of the GL screen
	SDL_Rect _glWindow;      // Only uses w and h (for window resizing)
	int _glBottomOfTexture;

	SDL_Surface *_hwscreen;  // hardware screen (=> _usingOpenGL == false)

	ScalerProc *_scaler_proc;
	
	virtual void load_gfx_mode();
	virtual void unload_gfx_mode();
	void hotswap_gfx_mode();
};

OSystem_SDL_Common *OSystem_SDL_Common::create_intern() {
	return new OSystem_SDL_OpenGL();
}

OSystem_SDL_OpenGL::OSystem_SDL_OpenGL()
  : _hwscreen(0), _scaler_proc(0)
{
  _glScreenStart = 0; 
  _glBilinearFilter = true;
  _glAspectRatio = false;
  _usingOpenGL = false; // false => Switch to filters used in the sdl.cpp version
  _glBottomOfTexture = 256; // height is always 256
  // 640x480 resolution
  _glWindow.w = 640;
  _glWindow.h = 480;
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
	_mode_flags = DF_UPDATE_EXPAND_1_PIXEL;

	_tmpscreen = NULL;
	_tmpScreenWidth = (_screenWidth + 3);

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
	case GFX_ADVMAME3X:
		_scaleFactor = 3;
		_scaler_proc = AdvMame3x;
		break;
	case GFX_TV2X:
		_scaleFactor = 2;
		_scaler_proc = TV2x;
		break;
	case GFX_DOTMATRIX:
		_scaleFactor = 2;
		_scaler_proc = DotMatrix;
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
		error("unknown gfx mode %d", _mode);
		_mode = GFX_NORMAL;
		_scaleFactor = 1;
		_scaler_proc = Normal1x;
	}

	if (_mode != GFX_NORMAL)
	  _usingOpenGL = false;
	
	//
	// Create the surface that contains the 8 bit game data
	//
	_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, _screenWidth, _screenHeight, 8, 0, 0, 0, 0);
	if (_screen == NULL)
		error("_screen failed");

	//
	// Create the surface that contains the scaled graphics in 16 bit mode
	//
	if (_usingOpenGL) {
	  
		_glFlags =  FB2GL_320 | FB2GL_RGBA | FB2GL_16BIT;
		if (_full_screen) {
			_glFlags |= (FB2GL_FS);
			_glScreenStart = 0;
		}
		// _glWindow defines the resolution
		fb2gl.init(_glWindow.w, _glWindow.h, 0, _glScreenStart? 15: 70, 
		    _glFlags);
		
	}
	else { // SDL backend
	  
		_hwscreen = SDL_SetVideoMode(_screenWidth * _scaleFactor, _screenHeight * _scaleFactor, 16, 
		  _full_screen ? (SDL_FULLSCREEN|SDL_SWSURFACE) : SDL_SWSURFACE
		);
		if (_hwscreen == NULL)
			error("_hwscreen failed");

		// Distinguish 555 and 565 mode
		if (_hwscreen->format->Rmask == 0x7C00)
			Init_2xSaI(555);
		else
			Init_2xSaI(565);
	}

	SDL_SetGamma(1.25, 1.25, 1.25);
	
	//
	// Create the surface used for the graphics in 16 bit before scaling, and also the overlay
	//

	// Need some extra bytes around when using 2xSaI
	uint16 *tmp_screen = (uint16 *)calloc(_tmpScreenWidth * (_screenHeight + 3),sizeof(uint16));

	if (_usingOpenGL) {
		_tmpscreen = SDL_CreateRGBSurfaceFrom(tmp_screen,
						_tmpScreenWidth, 
						_screenHeight + 3, 
						16, 
						_tmpScreenWidth * 2,
						Rmask,
						Gmask,
						Bmask,
						Amask);
	}
	else { // SDL backend
		_tmpscreen = SDL_CreateRGBSurfaceFrom(tmp_screen,
						_tmpScreenWidth, 
						_screenHeight + 3, 
						16, 
						_tmpScreenWidth * 2,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);
	}

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
  
	SDL_SetGamma(1.0, 1.0, 1.0);

	if (_screen) {
		SDL_FreeSurface(_screen);
		_screen = NULL; 
	}
	if (_hwscreen) {
		SDL_FreeSurface(_hwscreen);
		_hwscreen = NULL; 
	}
	if (_tmpscreen) {
		free((uint16 *)_tmpscreen->pixels);
		SDL_FreeSurface(_tmpscreen);
		_tmpscreen = NULL;
	}
}

void OSystem_SDL_OpenGL::hotswap_gfx_mode() {
	if (!_screen)
		return;

	// Keep around the old _screen & _tmpscreen so we can restore the screen data
	// after the mode switch.
	SDL_Surface *old_screen = _screen;
	SDL_Surface *old_tmpscreen = _tmpscreen;

	// Release the HW screen surface
	if (fb2gl.getScreen()) { // _usingOpenGL was true
	  SDL_FreeSurface(fb2gl.getScreen());
	  fb2gl.setScreen(NULL);
	}
	if (_hwscreen) {
	  SDL_FreeSurface(_hwscreen);
	  _hwscreen = NULL;
	}

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

	// Finally, blit everything to the screen
	update_screen();
}

void OSystem_SDL_OpenGL::update_screen() {
	// If the shake position changed, fill the dirty area with blackness
	if (_currentShakePos != _newShakePos) {
		if (_usingOpenGL) {
			SDL_Rect blackrect = {
			  0, 
			  _glScreenStart, 
			  _screenWidth, 
			  _newShakePos + _glScreenStart
			};
		
			SDL_FillRect(tmpSurface, &blackrect, 0);
			fb2gl.blit16(tmpSurface, 1, &blackrect, 0, 0);
		}
		else { // SDL backend
			SDL_Rect blackrect = {0, 0, _screenWidth * _scaleFactor, _newShakePos * _scaleFactor};
			SDL_FillRect(_hwscreen, &blackrect, 0);
		}

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
		SDL_Rect dst;
		if (!_overlayVisible) {
			for(r = _dirty_rect_list; r != last_rect; ++r) {
				dst = *r;
				dst.x++;	// Shift rect by one since 2xSai needs to acces the data around
				dst.y++;	// any pixel to scale it, and we want to avoid mem access crashes.
				if (_scaler_proc == Normal1x) {
				    if (_usingOpenGL) {
					if (SDL_BlitSurface(_screen, r, _tmpscreen, &dst) != 0)
						error("SDL_BlitSurface failed: %s", SDL_GetError());
				    }
				    else { // SDL backend
					if (SDL_BlitSurface(_screen, r, _hwscreen, &dst) != 0)
						error("SDL_BlitSurface failed: %s", SDL_GetError());
				    }
				} else { // _scaler_proc != Normal1x
					if (SDL_BlitSurface(_screen, r, _tmpscreen, &dst) != 0)
						error("SDL_BlitSurface failed: %s", SDL_GetError());
				}
			}
		} else {
		    if (!_usingOpenGL) {
			for(r = _dirty_rect_list; r != last_rect; ++r) {
				dst = *r;
				if (SDL_BlitSurface(_tmpscreen, r, _hwscreen, &dst) != 0)
					error("SDL_BlitSurface failed: %s", SDL_GetError());
			}
		    }
		}

		
		if (_usingOpenGL) {
			// Almost the same thing as SDL_UpdateRects
			fb2gl.blit16(
			    _tmpscreen, 
			    _num_dirty_rects, 
			    _dirty_rect_list, 
			    0,
			    _currentShakePos + _glScreenStart
			);

			int _glBottomOfGameScreen = _screenHeight + 
			  _glScreenStart + _currentShakePos; 

			// Bottom black border height
			tmpBlackRect.h = _glBottomOfTexture - _glBottomOfGameScreen;
			if (!(_full_screen) && (tmpBlackRect.h > 0)) {
				SDL_FillRect(tmpSurface, &tmpBlackRect, 0);
				fb2gl.blit16(tmpSurface, 1, &tmpBlackRect, 0,
				  _glBottomOfGameScreen);
			}

			fb2gl.display();
		}
		else { // SDL backend
		
		  if (_scaler_proc != Normal1x) {
			SDL_LockSurface(_tmpscreen);
			SDL_LockSurface(_hwscreen);

			srcPitch = _tmpscreen->pitch;
			dstPitch = _hwscreen->pitch;

			for(r = _dirty_rect_list; r != last_rect; ++r) {
				register int dst_y = r->y + _currentShakePos;
				register int dst_h = 0;
				if (dst_y < _screenHeight) {
					dst_h = r->h;
					if (dst_h > _screenHeight - dst_y)
						dst_h = _screenHeight - dst_y;

						dst_y *= _scaleFactor;

						_scaler_proc((byte *)_tmpscreen->pixels + (r->x * 2 + 2) + (r->y + 1) * srcPitch, srcPitch,
						(byte *)_hwscreen->pixels + r->x * 2 * _scaleFactor + dst_y * dstPitch, dstPitch, r->w, dst_h);
				}
			
				r->x *= _scaleFactor;
				r->y = dst_y;
				r->w *= _scaleFactor;
				r->h = dst_h * _scaleFactor;
			}

			SDL_UnlockSurface(_tmpscreen);
			SDL_UnlockSurface(_hwscreen);
		  }

		  // Readjust the dirty rect list in case we are doing a full update.
		  // This is necessary if shaking is active.
		  if (_forceFull) {
			_dirty_rect_list[0].y = 0;
			_dirty_rect_list[0].h = _screenHeight * _scaleFactor;
		  }

		  // Finally, blit all our changes to the screen
		  SDL_UpdateRects(_hwscreen, _num_dirty_rects, _dirty_rect_list);
		} // END OF "SDL backend"
	} // if (num_dirty_rects > 0) ...

	_num_dirty_rects = 0;
	_forceFull = false;
}

/*
bool OSystem_SDL_OpenGL::poll_event(Event *event) {
	SDL_Event ev;
	ev.type = 0;

	SDL_PeepEvents(&ev, 1, SDL_GETEVENT, SDL_VIDEORESIZEMASK);

	if (ev.type == SDL_VIDEORESIZE) {
	      int w = ev.resize.w; 
	      int h = ev.resize.h;
	      glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	      glMatrixMode(GL_PROJECTION);
	      glLoadIdentity();
	      glOrtho(-1.0,1.0,-1.0,1.0,-1.0,1.0);
	      _glWindow.w = w;
	      _glWindow.h = h;
	}

	return OSystem_SDL_Common::poll_event(event);
}
*/

uint32 OSystem_SDL_OpenGL::property(int param, Property *value) {
	int i;

	if (param == PROP_TOGGLE_FULLSCREEN) {
		if (!_usingOpenGL)
			assert(_hwscreen != 0);
		_full_screen ^= true;
#ifdef MACOSX
		// On OS X, SDL_WM_ToggleFullScreen is currently not implemented. Worse,
		// it still always returns -1. So we simply don't call it at all and
		// use hotswap_gfx_mode() directly to switch to fullscreen mode.
		hotswap_gfx_mode();
#else
		SDL_Surface *_tmpScreen;
		if (_usingOpenGL) {
			_tmpScreen = fb2gl.getScreen();
		} 
		else { // SDL backend
			_tmpScreen = _hwscreen;
		}

		if (!SDL_WM_ToggleFullScreen(_tmpScreen)) {
			// if ToggleFullScreen fails, achieve the same effect with hotswap gfx mode
			hotswap_gfx_mode();
		}
#endif

		return 1;
	}
	else if (param == PROP_SET_GFX_MODE) {
		SDL_Rect full = {0, 0, _screenWidth, _screenHeight};

		if (value->gfx_mode > 10) { // OpenGL modes
			if (!_usingOpenGL) {
				_usingOpenGL = true;
				_mode = GFX_NORMAL;
				hotswap_gfx_mode();
			}
		}
	  
		switch(value->gfx_mode) {
			case GFX_BILINEAR: // Bilinear Filtering (on/off)
				_glBilinearFilter ^= true;
				for (i = 0; i < 2; i++) {
					glBindTexture(GL_TEXTURE_2D, i);
					if (_glBilinearFilter) {
						glTexParameteri(
						    GL_TEXTURE_2D, 
						    GL_TEXTURE_MAG_FILTER, 
						    GL_LINEAR
						);
						glTexParameteri(
						    GL_TEXTURE_2D, 
						    GL_TEXTURE_MIN_FILTER, 
						    GL_LINEAR
						);
					} else {
						glTexParameteri(
						    GL_TEXTURE_2D, 
						    GL_TEXTURE_MAG_FILTER, 
						    GL_NEAREST
						);
						glTexParameteri(
						    GL_TEXTURE_2D, 
						    GL_TEXTURE_MIN_FILTER, 
						    GL_NEAREST
						);
					}
				}
				break;
			case GFX_ASPECTRATIO: 
				_glAspectRatio ^= true;
				if (_glAspectRatio) {
					// Don't use the whole screen (black borders)
					fb2gl.init(0, 0, 0, 15, _glFlags);
					_glScreenStart = 20;
					SDL_FillRect(tmpSurface, &tmpBlackRect, 0);
					fb2gl.blit16(tmpSurface, 1, &tmpBlackRect, 0, 0);
				} else {
					// Use the whole screen
					fb2gl.init(0, 0, 0, 70, _glFlags);
					_glScreenStart = 0;
				}
				break;
			default: // SDL backend
				if (value->gfx_mode >= 10)
				  return 0;

				_mode = value->gfx_mode;

				if (_usingOpenGL)
					_usingOpenGL = false;
				
				hotswap_gfx_mode();
		};

		if (_usingOpenGL) {
			fb2gl.blit16(_tmpscreen, 1, &full, 0, _glScreenStart);
			fb2gl.display();
		}

		return 1;
	}

	return OSystem_SDL_Common::property(param, value);
}

