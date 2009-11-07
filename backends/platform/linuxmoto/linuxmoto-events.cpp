/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */


#include "backends/platform/linuxmoto/linuxmoto-sdl.h"
#include "backends/platform/sdl/sdl.h"

static int mapKey(SDLKey key, SDLMod mod, Uint16 unicode) {
	if (key >= SDLK_F1 && key <= SDLK_F9) {
		return key - SDLK_F1 + Common::ASCII_F1;
	} else if (key >= SDLK_KP0 && key <= SDLK_KP9) {
		return key - SDLK_KP0 + '0';
	} else if (key >= SDLK_UP && key <= SDLK_PAGEDOWN) {
		return key;
	} else if (unicode) {
		return unicode;
	} else if (key >= 'a' && key <= 'z' && (mod & KMOD_SHIFT)) {
		return key & ~0x20;
	} else if (key >= SDLK_NUMLOCK && key <= SDLK_EURO) {
		return 0;
	}
	return key;
}

void OSystem_LINUXMOTO::fillMouseEvent(Common::Event &event, int x, int y) {
	if (_videoMode.mode == GFX_HALF && !_overlayVisible) {
		event.mouse.x = x*2;
		event.mouse.y = y*2;
	} else {
		event.mouse.x = x;
		event.mouse.y = y;
	}

	// Update the "keyboard mouse" coords
	_km.x = x;
	_km.y = y;

	// Adjust for the screen scaling
	if (!_overlayVisible) {
		event.mouse.x /= _videoMode.scaleFactor;
		event.mouse.y /= _videoMode.scaleFactor;
		if (_videoMode.aspectRatioCorrection)
			event.mouse.y = aspect2Real(event.mouse.y);
	}
}

bool OSystem_LINUXMOTO::remapKey(SDL_Event &ev, Common::Event &event) {
	//  Motorol A1200/E6/A1600 remapkey by Lubomyr
#ifdef MOTOEZX
	// Quit on MOD+Camera Key on A1200
	if (ev.key.keysym.sym == SDLK_e) {
		event.type = Common::EVENT_QUIT;
		return true;
	}
	// '1' Bypass security protection - MOD+Call key
	if (ev.key.keysym.sym == SDLK_f) {
		ev.key.keysym.sym = SDLK_1;
	}
	// F5 Game Menu - Call key
	else if (ev.key.keysym.sym == SDLK_SPACE) {
		ev.key.keysym.sym = SDLK_F5;
	}
	// VirtualKeyboard - Camera key
	else if (ev.key.keysym.sym == SDLK_PAUSE) {
		ev.key.keysym.sym = SDLK_F7;
	}
	// Enter - mod+fire key
	else if (ev.key.keysym.sym == SDLK_b) {
		ev.key.keysym.sym = SDLK_RETURN;
	}
	// '3' - mod+up key
	else if (ev.key.keysym.sym == SDLK_j) {
		ev.key.keysym.sym = SDLK_3;
	}
	// '6' - mod+up key
	else if (ev.key.keysym.sym == SDLK_i) {
		ev.key.keysym.sym = SDLK_6;
	}
	// 'y' - mod+right key
	else if (ev.key.keysym.sym == SDLK_g) {
		ev.key.keysym.sym = SDLK_y;
	}
	// 'n' - mod+right key
	else if (ev.key.keysym.sym == SDLK_h) {
		ev.key.keysym.sym = SDLK_n;
	}
	//  mod+vol'+' -> volume'+'
	else if (ev.key.keysym.sym == SDLK_c) {
		ev.key.keysym.sym = SDLK_RIGHTBRACKET;
	}
	//  mod+vol'-' -> volume'-'
	else if (ev.key.keysym.sym == SDLK_d) {	
		ev.key.keysym.sym = SDLK_LEFTBRACKET;
	}
#endif

#ifdef MOTOMAGX
	// Quit on cancel
	if (ev.key.keysym.sym == SDLK_F12) {
		event.type = Common::EVENT_QUIT;
		return true;
	}
#endif

// Joystick to Mouse
	else if (ev.key.keysym.sym == SDLK_LEFT) {
		if (ev.type == SDL_KEYDOWN) {
			_km.x_vel = -1;
			_km.x_down_count = 1;
		} else {
			_km.x_vel = 0;
			_km.x_down_count = 0;
		}

		event.type = Common::EVENT_MOUSEMOVE;
		fillMouseEvent(event, _km.x, _km.y);
		return true;
	} else if (ev.key.keysym.sym == SDLK_RIGHT) {
		if (ev.type == SDL_KEYDOWN) {
			_km.x_vel = 1;
			_km.x_down_count = 1;
		} else {
			_km.x_vel = 0;
			_km.x_down_count = 0;
		}

		event.type = Common::EVENT_MOUSEMOVE;
		fillMouseEvent(event, _km.x, _km.y);

		return true;
	} else if (ev.key.keysym.sym == SDLK_DOWN) {
		if (ev.type == SDL_KEYDOWN) {
			_km.y_vel = 1;
			_km.y_down_count = 1;
		} else {
			_km.y_vel = 0;
			_km.y_down_count = 0;
		}

		event.type = Common::EVENT_MOUSEMOVE;
		fillMouseEvent(event, _km.x, _km.y);

		return true;
	} else if (ev.key.keysym.sym == SDLK_UP) {
		if (ev.type == SDL_KEYDOWN) {
			_km.y_vel = -1;
			_km.y_down_count = 1;
		} else {
			_km.y_vel = 0;
			_km.y_down_count = 0;
		}

		event.type = Common::EVENT_MOUSEMOVE;
		fillMouseEvent(event, _km.x, _km.y);

		return true;
	} else if (ev.key.keysym.sym == SDLK_RETURN) { 
		// Joystick center to pressing Left Mouse	
		if (ev.key.type == SDL_KEYDOWN) {
			event.type = Common::EVENT_LBUTTONDOWN;
		} else {
			event.type = Common::EVENT_LBUTTONUP;			
		}

		fillMouseEvent(event, _km.x, _km.y);

		return true;
	} else if (ev.key.keysym.sym == SDLK_PLUS) { 
		// Volume Up to pressing Right Mouse
		if (ev.key.type == SDL_KEYDOWN ) {
			event.type = Common::EVENT_RBUTTONDOWN;
		} else {
			event.type = Common::EVENT_RBUTTONUP;			
		}
		fillMouseEvent(event, _km.x, _km.y);

		return true;
	} else if (ev.key.keysym.sym == SDLK_MINUS) { 
		// Volume Down to pressing Left Mouse	
		if (ev.key.type == SDL_KEYDOWN) {
			event.type = Common::EVENT_LBUTTONDOWN;
		} else {
			event.type = Common::EVENT_LBUTTONUP;		
		}

		fillMouseEvent(event, _km.x, _km.y);

		return true;
	} else {
		// Let the events fall through if we didn't change them, this may not be the best way to
		// set it up, but i'm not sure how sdl would like it if we let if fall through then redid it though.
		// and yes i have an huge terminal size so i dont wrap soon enough.
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = (Common::KeyCode)ev.key.keysym.sym;
		event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);
	} 

	return false;
}
