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
#include "common/util.h"

// FIXME move joystick defines out and replace with confile file options
// we should really allow users to map any key to a joystick button
#define JOY_DEADZONE 3200
#define JOY_ANALOG
// #define JOY_INVERT_Y
#define JOY_XAXIS 0
#define JOY_YAXIS 1
// buttons
#define JOY_BUT_LMOUSE 0
#define JOY_BUT_RMOUSE 2
#define JOY_BUT_ESCAPE 3
#define JOY_BUT_PERIOD 1
#define JOY_BUT_SPACE 4
#define JOY_BUT_F5 5


static const int s_gfxModeSwitchTable[][4] = {
		{ GFX_NORMAL, GFX_DOUBLESIZE, GFX_TRIPLESIZE, -1 },
		{ GFX_NORMAL, GFX_ADVMAME2X, GFX_ADVMAME3X, -1 },
		{ GFX_NORMAL, GFX_HQ2X, GFX_HQ3X, -1 },
		{ GFX_NORMAL, GFX_2XSAI, -1, -1 },
		{ GFX_NORMAL, GFX_SUPER2XSAI, -1, -1 },
		{ GFX_NORMAL, GFX_SUPEREAGLE, -1, -1 },
		{ GFX_NORMAL, GFX_TV2X, -1, -1 },
		{ GFX_NORMAL, GFX_DOTMATRIX, -1, -1 }
	};



static int mapKey(SDLKey key, SDLMod mod, Uint16 unicode)
{
	if (key >= SDLK_F1 && key <= SDLK_F9) {
		return key - SDLK_F1 + 315;
	} else if (key >= SDLK_KP0 && key <= SDLK_KP9) {
		return key - SDLK_KP0 + '0';
	} else if (key >= SDLK_UP && key <= SDLK_PAGEDOWN) {
		return key;
	} else if (unicode) {
		return unicode;
	} else if (key >= 'a' && key <= 'z' && mod & KMOD_SHIFT) {
		return key & ~0x20;
	} else if (key >= SDLK_NUMLOCK && key <= SDLK_EURO) {
		return 0;
	}
	return key;
}

void OSystem_SDL::fillMouseEvent(Event &event, int x, int y) {
	event.mouse.x = x;
	event.mouse.y = y;
	
	// Update the "keyboard mouse" coords
	km.x = x;
	km.y = y;

	// Adjust for the screen scaling
	event.mouse.x /= _scaleFactor;
	event.mouse.y /= _scaleFactor;

	// Optionally perform aspect ratio adjusting
	if (_adjustAspectRatio)
		event.mouse.y = aspect2Real(event.mouse.y);
}

void OSystem_SDL::kbd_mouse() {
	uint32 curTime = get_msecs();
	if (curTime >= km.last_time + km.delay_time) {
		km.last_time = curTime;
		if (km.x_down_count == 1) {
			km.x_down_time = curTime;
			km.x_down_count = 2;
		}
		if (km.y_down_count == 1) {
			km.y_down_time = curTime;
			km.y_down_count = 2;
		}

		if (km.x_vel || km.y_vel) {
			if (km.x_down_count) {
				if (curTime > km.x_down_time + km.delay_time * 12) {
					if (km.x_vel > 0)
						km.x_vel++;
					else
						km.x_vel--;
				} else if (curTime > km.x_down_time + km.delay_time * 8) {
					if (km.x_vel > 0)
						km.x_vel = 5;
					else
						km.x_vel = -5;
				}
			}
			if (km.y_down_count) {
				if (curTime > km.y_down_time + km.delay_time * 12) {
					if (km.y_vel > 0)
						km.y_vel++;
					else
						km.y_vel--;
				} else if (curTime > km.y_down_time + km.delay_time * 8) {
					if (km.y_vel > 0)
						km.y_vel = 5;
					else
						km.y_vel = -5;
				}
			}

			km.x += km.x_vel;
			km.y += km.y_vel;

			if (km.x < 0) {
				km.x = 0;
				km.x_vel = -1;
				km.x_down_count = 1;
			} else if (km.x > km.x_max) {
				km.x = km.x_max;
				km.x_vel = 1;
				km.x_down_count = 1;
			}

			if (km.y < 0) {
				km.y = 0;
				km.y_vel = -1;
				km.y_down_count = 1;
			} else if (km.y > km.y_max) {
				km.y = km.y_max;
				km.y_vel = 1;
				km.y_down_count = 1;
			}

			SDL_WarpMouse(km.x, km.y);
		}
	}
}

bool OSystem_SDL::poll_event(Event *event) {
	SDL_Event ev;
	int axis;
	byte b = 0;
	
	kbd_mouse();
	
	// If the screen mode changed, send an EVENT_SCREEN_CHANGED
	if (_modeChanged) {
		_modeChanged = false;
		event->event_code = EVENT_SCREEN_CHANGED;
		return true;
	}

	while(SDL_PollEvent(&ev)) {
		switch(ev.type) {
		case SDL_KEYDOWN:
#ifdef LINUPY
			// Yopy has no ALT key, steal the SHIFT key 
			// (which isn't used much anyway)
			if (ev.key.keysym.mod & KMOD_SHIFT)
				b |= KBD_ALT;
			if (ev.key.keysym.mod & KMOD_CTRL)
				b |= KBD_CTRL;
#else
			if (ev.key.keysym.mod & KMOD_SHIFT)
				b |= KBD_SHIFT;
			if (ev.key.keysym.mod & KMOD_CTRL)
				b |= KBD_CTRL;
			if (ev.key.keysym.mod & KMOD_ALT)
				b |= KBD_ALT;
#endif
			event->kbd.flags = b;

			// Alt-Return toggles full screen mode				
			if (b == KBD_ALT && ev.key.keysym.sym == SDLK_RETURN) {
				setFeatureState(kFeatureFullscreenMode, !_full_screen);
				break;
			}

			// Alt-S: Create a screenshot
			if (b == KBD_ALT && ev.key.keysym.sym == 's') {
				char filename[20];

				for (int n = 0;; n++) {
					SDL_RWops *file;

					sprintf(filename, "scummvm%05d.bmp", n);
					file = SDL_RWFromFile(filename, "r");
					if (!file)
						break;
					SDL_RWclose(file);
				}
				if (save_screenshot(filename))
					printf("Saved '%s'\n", filename);
				else
					printf("Could not save screenshot!\n");
				break;
			}

			// Ctrl-m toggles mouse capture
			if (b == KBD_CTRL && ev.key.keysym.sym == 'm') {
				toggleMouseGrab();
				break;
			}

#ifdef MACOSX
			// On Macintosh', Cmd-Q quits
			if ((ev.key.keysym.mod & KMOD_META) && ev.key.keysym.sym == 'q') {
				event->event_code = EVENT_QUIT;
				return true;
			}
#else
			// Ctrl-z and Alt-X quit
			if ((b == KBD_CTRL && ev.key.keysym.sym == 'z') || (b == KBD_ALT && ev.key.keysym.sym == 'x')) {
				event->event_code = EVENT_QUIT;
				return true;
			}
#endif

			// Ctrl-Alt-<key> will change the GFX mode
			if ((b & (KBD_CTRL|KBD_ALT)) == (KBD_CTRL|KBD_ALT)) {
				// FIXME EVIL HACK: This shouldn't be a static int, rather it
				// should be a member variable. Furthermore, it shouldn't be
				// set in this code, rather it should be set by load_gfx_mode().
				// But for now this quick&dirty hack works.
				static int _scalerType = 0;
				if (_mode != GFX_NORMAL) {
					// Try to figure out which gfx mode "group" we are in
					// This is just a temporary hack until the proper solution
					// (i.e. code in load_gfx_mode()) is in effect.
					for (int i = 0; i < ARRAYSIZE(s_gfxModeSwitchTable); i++) {
						if (s_gfxModeSwitchTable[i][1] == _mode || s_gfxModeSwitchTable[i][2] == _mode) {
							_scalerType = i;
							break;
						}
					}
				}
				
				int factor = _scaleFactor - 1;

				// Ctrl-Alt-a toggles aspect ratio correction
				if (ev.key.keysym.sym == 'a') {
					setFeatureState(kFeatureAspectRatioCorrection, !_adjustAspectRatio);
					break;
				}

				// Increase/decrease the scale factor
				// TODO: Shall we 'wrap around' here?
				if (ev.key.keysym.sym == '=' || ev.key.keysym.sym == '+' || ev.key.keysym.sym == '-') {
					factor += (ev.key.keysym.sym == '-' ? -1 : +1);
					if (0 <= factor && factor < 4 && s_gfxModeSwitchTable[_scalerType][factor] >= 0) {
						setGraphicsMode(s_gfxModeSwitchTable[_scalerType][factor]);
					}
					break;
				}
				
				if ('1' <= ev.key.keysym.sym && ev.key.keysym.sym <= '9') {
					_scalerType = ev.key.keysym.sym - '1';
					if (_scalerType >= ARRAYSIZE(s_gfxModeSwitchTable))
						break;
					
					while (s_gfxModeSwitchTable[_scalerType][factor] < 0) {
						assert(factor > 0);
						factor--;
					}
					setGraphicsMode(s_gfxModeSwitchTable[_scalerType][factor]);
					break;
				}
			}

#ifdef LINUPY
			// On Yopy map the End button to quit
			if ((ev.key.keysym.sym==293)) {
				event->event_code = EVENT_QUIT;
				return true;
			}
			// Map menu key to f5 (scumm menu)
			if (ev.key.keysym.sym==306) {
				event->event_code = EVENT_KEYDOWN;
				event->kbd.keycode = SDLK_F5;
				event->kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
				return true;
			}
			// Map action key to action
			if (ev.key.keysym.sym==291) {
				event->event_code = EVENT_KEYDOWN;
				event->kbd.keycode = SDLK_TAB;
				event->kbd.ascii = mapKey(SDLK_TAB, ev.key.keysym.mod, 0);
				return true;
			}
			// Map OK key to skip cinematic
			if (ev.key.keysym.sym==292) {
				event->event_code = EVENT_KEYDOWN;
				event->kbd.keycode = SDLK_ESCAPE;
				event->kbd.ascii = mapKey(SDLK_ESCAPE, ev.key.keysym.mod, 0);
				return true;
			}
#endif

#ifdef QTOPIA
			// quit on fn+backspace on zaurus
			if (ev.key.keysym.sym == 127) {
				event->event_code = EVENT_QUIT;
				return true;
			}

			// map menu key (f11) to f5 (scumm menu)
			if (ev.key.keysym.sym == SDLK_F11) {
				event->event_code = EVENT_KEYDOWN;
				event->kbd.keycode = SDLK_F5;
				event->kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
			}
			// map center (space) to tab (default action )
			// I wanted to map the calendar button but the calendar comes up
			//
			else if (ev.key.keysym.sym == SDLK_SPACE) {
				event->event_code = EVENT_KEYDOWN;
				event->kbd.keycode = SDLK_TAB;
				event->kbd.ascii = mapKey(SDLK_TAB, ev.key.keysym.mod, 0);
			}
			// since we stole space (pause) above we'll rebind it to the tab key on the keyboard
			else if (ev.key.keysym.sym == SDLK_TAB) {
				event->event_code = EVENT_KEYDOWN;
				event->kbd.keycode = SDLK_SPACE;
				event->kbd.ascii = mapKey(SDLK_SPACE, ev.key.keysym.mod, 0);
			} else {
			// let the events fall through if we didn't change them, this may not be the best way to
			// set it up, but i'm not sure how sdl would like it if we let if fall through then redid it though.
			// and yes i have an huge terminal size so i dont wrap soon enough.
				event->event_code = EVENT_KEYDOWN;
				event->kbd.keycode = ev.key.keysym.sym;
				event->kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);
			}
#else
			event->event_code = EVENT_KEYDOWN;
			event->kbd.keycode = ev.key.keysym.sym;
			event->kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);
#endif
			
			switch(ev.key.keysym.sym) {
			case SDLK_LEFT:
				km.x_vel = -1;
				km.x_down_count = 1;
				break;
			case SDLK_RIGHT:
				km.x_vel =  1;
				km.x_down_count = 1;
				break;
			case SDLK_UP:
				km.y_vel = -1;
				km.y_down_count = 1;
				break;
			case SDLK_DOWN:
				km.y_vel =  1;
				km.y_down_count = 1;
				break;
			default:
				break;
			}

			return true;
	
		case SDL_KEYUP:
			event->event_code = EVENT_KEYUP;
			event->kbd.keycode = ev.key.keysym.sym;
			event->kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);

			switch(ev.key.keysym.sym) {
			case SDLK_LEFT:
				if (km.x_vel < 0) {
					km.x_vel = 0;
					km.x_down_count = 0;
				}
				break;
			case SDLK_RIGHT:
				if (km.x_vel > 0) {
					km.x_vel = 0;
					km.x_down_count = 0;
				}
				break;
			case SDLK_UP:
				if (km.y_vel < 0) {
					km.y_vel = 0;
					km.y_down_count = 0;
				}
				break;
			case SDLK_DOWN:
				if (km.y_vel > 0) {
					km.y_vel = 0;
					km.y_down_count = 0;
				}
				break;
			default:
				break;
			}
			return true;

		case SDL_MOUSEMOTION:
			event->event_code = EVENT_MOUSEMOVE;
			fillMouseEvent(*event, ev.motion.x, ev.motion.y);
			
			set_mouse_pos(event->mouse.x, event->mouse.y);
			return true;

		case SDL_MOUSEBUTTONDOWN:
			if (ev.button.button == SDL_BUTTON_LEFT)
				event->event_code = EVENT_LBUTTONDOWN;
			else if (ev.button.button == SDL_BUTTON_RIGHT)
				event->event_code = EVENT_RBUTTONDOWN;
#if defined(SDL_BUTTON_WHEELUP) && defined(SDL_BUTTON_WHEELDOWN)
			else if (ev.button.button == SDL_BUTTON_WHEELUP)
				event->event_code = EVENT_WHEELUP;
			else if (ev.button.button == SDL_BUTTON_WHEELDOWN)
				event->event_code = EVENT_WHEELDOWN;
#endif
			else
				break;

			fillMouseEvent(*event, ev.button.x, ev.button.y);

			return true;

		case SDL_MOUSEBUTTONUP:
			if (ev.button.button == SDL_BUTTON_LEFT)
				event->event_code = EVENT_LBUTTONUP;
			else if (ev.button.button == SDL_BUTTON_RIGHT)
				event->event_code = EVENT_RBUTTONUP;
			else
				break;
			fillMouseEvent(*event, ev.button.x, ev.button.y);

			return true;

		case SDL_JOYBUTTONDOWN:
			if (ev.jbutton.button == JOY_BUT_LMOUSE) {
				event->event_code = EVENT_LBUTTONDOWN;
			} else if (ev.jbutton.button == JOY_BUT_RMOUSE) {
				event->event_code = EVENT_RBUTTONDOWN;
			} else {
				event->event_code = EVENT_KEYDOWN;
				switch (ev.jbutton.button) {
					case JOY_BUT_ESCAPE:
						event->kbd.keycode = SDLK_ESCAPE;
						event->kbd.ascii = mapKey(SDLK_ESCAPE, ev.key.keysym.mod, 0);
						break;
					case JOY_BUT_PERIOD:
						event->kbd.keycode = SDLK_PERIOD;
						event->kbd.ascii = mapKey(SDLK_PERIOD, ev.key.keysym.mod, 0);
						break;
					case JOY_BUT_SPACE:
						event->kbd.keycode = SDLK_SPACE;
						event->kbd.ascii = mapKey(SDLK_SPACE, ev.key.keysym.mod, 0);
						break;
					case JOY_BUT_F5:
						event->kbd.keycode = SDLK_F5;
						event->kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
						break; 
				}
			}
			return true;

		case SDL_JOYBUTTONUP:
			if (ev.jbutton.button == JOY_BUT_LMOUSE) {
				event->event_code = EVENT_LBUTTONUP;
			} else if (ev.jbutton.button == JOY_BUT_RMOUSE) {
				event->event_code = EVENT_RBUTTONUP;
			} else {
				event->event_code = EVENT_KEYUP;
				switch (ev.jbutton.button) {
					case JOY_BUT_ESCAPE:
						event->kbd.keycode = SDLK_ESCAPE;
						event->kbd.ascii = mapKey(SDLK_ESCAPE, ev.key.keysym.mod, 0);
						break;
					case JOY_BUT_PERIOD:
						event->kbd.keycode = SDLK_PERIOD;
						event->kbd.ascii = mapKey(SDLK_PERIOD, ev.key.keysym.mod, 0);
						break;
					case JOY_BUT_SPACE:
						event->kbd.keycode = SDLK_SPACE;
						event->kbd.ascii = mapKey(SDLK_SPACE, ev.key.keysym.mod, 0);
						break;
					case JOY_BUT_F5:
						event->kbd.keycode = SDLK_F5;
						event->kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
						break;
				} 
			}
			return true;

		case SDL_JOYAXISMOTION:
			axis = ev.jaxis.value;
			if ( axis > JOY_DEADZONE) {
				axis -= JOY_DEADZONE;
				event->event_code = EVENT_MOUSEMOVE;
			} else if ( axis < -JOY_DEADZONE ) {
				axis += JOY_DEADZONE;
				event->event_code = EVENT_MOUSEMOVE;
			} else
				axis = 0;

			if ( ev.jaxis.axis == JOY_XAXIS) { 
#ifdef JOY_ANALOG
				km.x_vel = axis/2000;
				km.x_down_count = 0;
#else
				if (axis != 0) {
					km.x_vel = (axis > 0) ? 1:-1;
					km.x_down_count = 1;
				} else {
					km.x_vel = 0;
					km.x_down_count = 0;
				}
#endif

			} else if (ev.jaxis.axis == JOY_YAXIS) { 
#ifndef JOY_INVERT_Y
				axis = -axis;
#endif
#ifdef JOY_ANALOG
				km.y_vel = -axis / 2000;
				km.y_down_count = 0;
#else
				if (axis != 0) {
					km.y_vel = (-axis > 0) ? 1: -1;
					km.y_down_count = 1;
				} else {
					km.y_vel = 0;
					km.y_down_count = 0;
				}
#endif
			}
			
			fillMouseEvent(*event, km.x, km.y);

			return true;

		case SDL_VIDEOEXPOSE:
			_forceFull = true;
			break;

		case SDL_QUIT:
			event->event_code = EVENT_QUIT;
			return true;
		}
	}
	return false;
}


