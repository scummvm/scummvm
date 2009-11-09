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

#include "backends/platform/sdl/sdl.h"
#include "common/util.h"
#include "common/events.h"

// FIXME move joystick defines out and replace with confile file options
// we should really allow users to map any key to a joystick button
#define JOY_DEADZONE 3200

#ifndef __SYMBIAN32__ // Symbian wants dialog joystick i.e cursor for movement/selection
	#define JOY_ANALOG
#endif

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

void OSystem_SDL::fillMouseEvent(Common::Event &event, int x, int y) {
	event.mouse.x = x;
	event.mouse.y = y;

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

void OSystem_SDL::handleKbdMouse() {
	uint32 curTime = getMillis();
	if (curTime >= _km.last_time + _km.delay_time) {
		_km.last_time = curTime;
		if (_km.x_down_count == 1) {
			_km.x_down_time = curTime;
			_km.x_down_count = 2;
		}
		if (_km.y_down_count == 1) {
			_km.y_down_time = curTime;
			_km.y_down_count = 2;
		}

		if (_km.x_vel || _km.y_vel) {
			if (_km.x_down_count) {
				if (curTime > _km.x_down_time + _km.delay_time * 12) {
					if (_km.x_vel > 0)
						_km.x_vel++;
					else
						_km.x_vel--;
				} else if (curTime > _km.x_down_time + _km.delay_time * 8) {
					if (_km.x_vel > 0)
						_km.x_vel = 5;
					else
						_km.x_vel = -5;
				}
			}
			if (_km.y_down_count) {
				if (curTime > _km.y_down_time + _km.delay_time * 12) {
					if (_km.y_vel > 0)
						_km.y_vel++;
					else
						_km.y_vel--;
				} else if (curTime > _km.y_down_time + _km.delay_time * 8) {
					if (_km.y_vel > 0)
						_km.y_vel = 5;
					else
						_km.y_vel = -5;
				}
			}

			_km.x += _km.x_vel;
			_km.y += _km.y_vel;

			if (_km.x < 0) {
				_km.x = 0;
				_km.x_vel = -1;
				_km.x_down_count = 1;
			} else if (_km.x > _km.x_max) {
				_km.x = _km.x_max;
				_km.x_vel = 1;
				_km.x_down_count = 1;
			}

			if (_km.y < 0) {
				_km.y = 0;
				_km.y_vel = -1;
				_km.y_down_count = 1;
			} else if (_km.y > _km.y_max) {
				_km.y = _km.y_max;
				_km.y_vel = 1;
				_km.y_down_count = 1;
			}

			SDL_WarpMouse((Uint16)_km.x, (Uint16)_km.y);
		}
	}
}

static byte SDLModToOSystemKeyFlags(SDLMod mod) {
	byte b = 0;
#ifdef LINUPY
	// Yopy has no ALT key, steal the SHIFT key
	// (which isn't used much anyway)
	if (mod & KMOD_SHIFT)
		b |= Common::KBD_ALT;
#else
	if (mod & KMOD_SHIFT)
		b |= Common::KBD_SHIFT;
	if (mod & KMOD_ALT)
		b |= Common::KBD_ALT;
#endif
	if (mod & KMOD_CTRL)
		b |= Common::KBD_CTRL;

	return b;
}

bool OSystem_SDL::pollEvent(Common::Event &event) {
	SDL_Event ev;

	handleKbdMouse();

	// If the screen mode changed, send an Common::EVENT_SCREEN_CHANGED
	if (_modeChanged) {
		_modeChanged = false;
		event.type = Common::EVENT_SCREEN_CHANGED;
		return true;
	}

	while (SDL_PollEvent(&ev)) {
		preprocessEvents(&ev);
		if (dispatchSDLEvent(ev, event))
			return true;
	}
	return false;
}

bool OSystem_SDL::dispatchSDLEvent(const SDL_Event &ev, Common::Event &event) {
	switch (ev.type) {
	case SDL_KEYDOWN:
		return handleKeyDown(ev, event);
	case SDL_KEYUP:
		return handleKeyUp(ev, event);
	case SDL_MOUSEMOTION:
		return handleMouseMotion(ev, event);
	case SDL_MOUSEBUTTONDOWN:
		return handleMouseButtonDown(ev, event);
	case SDL_MOUSEBUTTONUP:
		return handleMouseButtonUp(ev, event);
	case SDL_JOYBUTTONDOWN:
		return handleJoyButtonDown(ev, event);
	case SDL_JOYBUTTONUP:
		return handleJoyButtonUp(ev, event);
	case SDL_JOYAXISMOTION:
		return handleJoyAxisMotion(ev, event);

	case SDL_VIDEOEXPOSE:
		_forceFull = true;
		break;

	case SDL_QUIT:
		event.type = Common::EVENT_QUIT;
		return true;

	}
	
	return false;
}


bool OSystem_SDL::handleKeyDown(const SDL_Event &ev, Common::Event &event) {
	byte b = 0;
	b = event.kbd.flags = SDLModToOSystemKeyFlags(SDL_GetModState());

	// Alt-Return and Alt-Enter toggle full screen mode
	if (b == Common::KBD_ALT && (ev.key.keysym.sym == SDLK_RETURN
					  || ev.key.keysym.sym == SDLK_KP_ENTER)) {
		beginGFXTransaction();
			setFullscreenMode(!_videoMode.fullscreen);
		endGFXTransaction();
#ifdef USE_OSD
		if (_videoMode.fullscreen)
			displayMessageOnOSD("Fullscreen mode");
		else
			displayMessageOnOSD("Windowed mode");
#endif

		return false;
	}

	// Alt-S: Create a screenshot
	if (b == Common::KBD_ALT && ev.key.keysym.sym == 's') {
		char filename[20];

		for (int n = 0;; n++) {
			SDL_RWops *file;

			sprintf(filename, "scummvm%05d.bmp", n);
			file = SDL_RWFromFile(filename, "r");
			if (!file)
				return false;
			SDL_RWclose(file);
		}
		if (saveScreenshot(filename))
			printf("Saved '%s'\n", filename);
		else
			printf("Could not save screenshot!\n");
		return false;
	}

	// Ctrl-m toggles mouse capture
	if (b == Common::KBD_CTRL && ev.key.keysym.sym == 'm') {
		toggleMouseGrab();
		return false;
	}

#if defined(MACOSX)
	// On Macintosh', Cmd-Q quits
	if ((ev.key.keysym.mod & KMOD_META) && ev.key.keysym.sym == 'q') {
		event.type = Common::EVENT_QUIT;
		return true;
	}
#elif defined(UNIX)
	// On other unices, Control-Q quits
	if ((ev.key.keysym.mod & KMOD_CTRL) && ev.key.keysym.sym == 'q') {
		event.type = Common::EVENT_QUIT;
		return true;
	}
#else
	// Ctrl-z and Alt-X quit
	if ((b == Common::KBD_CTRL && ev.key.keysym.sym == 'z') || (b == Common::KBD_ALT && ev.key.keysym.sym == 'x')) {
		event.type = Common::EVENT_QUIT;
		return true;
	}
#endif

	if ((ev.key.keysym.mod & KMOD_CTRL) && ev.key.keysym.sym == 'u') {
		event.type = Common::EVENT_MUTE;
		return true;
	}

	// Ctrl-Alt-<key> will change the GFX mode
	if ((b & (Common::KBD_CTRL|Common::KBD_ALT)) == (Common::KBD_CTRL|Common::KBD_ALT)) {

		handleScalerHotkeys(ev.key);
		return false;
	}
	const bool event_complete = remapKey(ev, event);

	if (event_complete)
		return true;

	event.type = Common::EVENT_KEYDOWN;
	event.kbd.keycode = (Common::KeyCode)ev.key.keysym.sym;
	event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);

	return true;
}

bool OSystem_SDL::handleKeyUp(const SDL_Event &ev, Common::Event &event) {
	byte b = 0;
	const bool event_complete = remapKey(ev,event);

	if (event_complete)
		return true;

	event.type = Common::EVENT_KEYUP;
	event.kbd.keycode = (Common::KeyCode)ev.key.keysym.sym;
	event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);
	b = event.kbd.flags = SDLModToOSystemKeyFlags(SDL_GetModState());

	// Ctrl-Alt-<key> will change the GFX mode
	if ((b & (Common::KBD_CTRL|Common::KBD_ALT)) == (Common::KBD_CTRL|Common::KBD_ALT)) {
		// Swallow these key up events
		return false;
	}

	return true;
}

bool OSystem_SDL::handleMouseMotion(const SDL_Event &ev, Common::Event &event) {
	event.type = Common::EVENT_MOUSEMOVE;
	fillMouseEvent(event, ev.motion.x, ev.motion.y);

	setMousePos(event.mouse.x, event.mouse.y);
	return true;
}

bool OSystem_SDL::handleMouseButtonDown(const SDL_Event &ev, Common::Event &event) {
	if (ev.button.button == SDL_BUTTON_LEFT)
		event.type = Common::EVENT_LBUTTONDOWN;
	else if (ev.button.button == SDL_BUTTON_RIGHT)
		event.type = Common::EVENT_RBUTTONDOWN;
#if defined(SDL_BUTTON_WHEELUP) && defined(SDL_BUTTON_WHEELDOWN)
	else if (ev.button.button == SDL_BUTTON_WHEELUP)
		event.type = Common::EVENT_WHEELUP;
	else if (ev.button.button == SDL_BUTTON_WHEELDOWN)
		event.type = Common::EVENT_WHEELDOWN;
#endif
#if defined(SDL_BUTTON_MIDDLE)
	else if (ev.button.button == SDL_BUTTON_MIDDLE)
		event.type = Common::EVENT_MBUTTONDOWN;
#endif
	else
		return false;

	fillMouseEvent(event, ev.button.x, ev.button.y);

	return true;
}

bool OSystem_SDL::handleMouseButtonUp(const SDL_Event &ev, Common::Event &event) {
	if (ev.button.button == SDL_BUTTON_LEFT)
		event.type = Common::EVENT_LBUTTONUP;
	else if (ev.button.button == SDL_BUTTON_RIGHT)
		event.type = Common::EVENT_RBUTTONUP;
#if defined(SDL_BUTTON_MIDDLE)
	else if (ev.button.button == SDL_BUTTON_MIDDLE)
		event.type = Common::EVENT_MBUTTONUP;
#endif
	else
		return false;
	fillMouseEvent(event, ev.button.x, ev.button.y);

	return true;
}

bool OSystem_SDL::handleJoyButtonDown(const SDL_Event &ev, Common::Event &event) {
	if (ev.jbutton.button == JOY_BUT_LMOUSE) {
		event.type = Common::EVENT_LBUTTONDOWN;
		fillMouseEvent(event, _km.x, _km.y);
	} else if (ev.jbutton.button == JOY_BUT_RMOUSE) {
		event.type = Common::EVENT_RBUTTONDOWN;
		fillMouseEvent(event, _km.x, _km.y);
	} else {
		event.type = Common::EVENT_KEYDOWN;
		switch (ev.jbutton.button) {
		case JOY_BUT_ESCAPE:
			event.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = mapKey(SDLK_ESCAPE, ev.key.keysym.mod, 0);
			break;
		case JOY_BUT_PERIOD:
			event.kbd.keycode = Common::KEYCODE_PERIOD;
			event.kbd.ascii = mapKey(SDLK_PERIOD, ev.key.keysym.mod, 0);
			break;
		case JOY_BUT_SPACE:
			event.kbd.keycode = Common::KEYCODE_SPACE;
			event.kbd.ascii = mapKey(SDLK_SPACE, ev.key.keysym.mod, 0);
			break;
		case JOY_BUT_F5:
			event.kbd.keycode = Common::KEYCODE_F5;
			event.kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
			break;
		}
	}
	return true;
}

bool OSystem_SDL::handleJoyButtonUp(const SDL_Event &ev, Common::Event &event) {
	if (ev.jbutton.button == JOY_BUT_LMOUSE) {
		event.type = Common::EVENT_LBUTTONUP;
		fillMouseEvent(event, _km.x, _km.y);
	} else if (ev.jbutton.button == JOY_BUT_RMOUSE) {
		event.type = Common::EVENT_RBUTTONUP;
		fillMouseEvent(event, _km.x, _km.y);
	} else {
		event.type = Common::EVENT_KEYUP;
		switch (ev.jbutton.button) {
		case JOY_BUT_ESCAPE:
			event.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = mapKey(SDLK_ESCAPE, ev.key.keysym.mod, 0);
			break;
		case JOY_BUT_PERIOD:
			event.kbd.keycode = Common::KEYCODE_PERIOD;
			event.kbd.ascii = mapKey(SDLK_PERIOD, ev.key.keysym.mod, 0);
			break;
		case JOY_BUT_SPACE:
			event.kbd.keycode = Common::KEYCODE_SPACE;
			event.kbd.ascii = mapKey(SDLK_SPACE, ev.key.keysym.mod, 0);
			break;
		case JOY_BUT_F5:
			event.kbd.keycode = Common::KEYCODE_F5;
			event.kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
			break;
		}
	}
	return true;
}

bool OSystem_SDL::handleJoyAxisMotion(const SDL_Event &ev, Common::Event &event) {
	int axis = ev.jaxis.value;
	if ( axis > JOY_DEADZONE) {
		axis -= JOY_DEADZONE;
		event.type = Common::EVENT_MOUSEMOVE;
	} else if ( axis < -JOY_DEADZONE ) {
		axis += JOY_DEADZONE;
		event.type = Common::EVENT_MOUSEMOVE;
	} else
		axis = 0;

	if ( ev.jaxis.axis == JOY_XAXIS) {
#ifdef JOY_ANALOG
		_km.x_vel = axis/2000;
		_km.x_down_count = 0;
#else
		if (axis != 0) {
			_km.x_vel = (axis > 0) ? 1:-1;
			_km.x_down_count = 1;
		} else {
			_km.x_vel = 0;
			_km.x_down_count = 0;
		}
#endif

	} else if (ev.jaxis.axis == JOY_YAXIS) {
#ifndef JOY_INVERT_Y
		axis = -axis;
#endif
#ifdef JOY_ANALOG
		_km.y_vel = -axis / 2000;
		_km.y_down_count = 0;
#else
		if (axis != 0) {
			_km.y_vel = (-axis > 0) ? 1: -1;
			_km.y_down_count = 1;
		} else {
			_km.y_vel = 0;
			_km.y_down_count = 0;
		}
#endif
	}

	fillMouseEvent(event, _km.x, _km.y);

	return true;
}

bool OSystem_SDL::remapKey(const SDL_Event &ev, Common::Event &event) {
#ifdef LINUPY
	// On Yopy map the End button to quit
	if ((ev.key.keysym.sym == 293)) {
		event.type = Common::EVENT_QUIT;
		return true;
	}
	// Map menu key to f5 (scumm menu)
	if (ev.key.keysym.sym == 306) {
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_F5;
		event.kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
		return true;
	}
	// Map action key to action
	if (ev.key.keysym.sym == 291) {
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_TAB;
		event.kbd.ascii = mapKey(SDLK_TAB, ev.key.keysym.mod, 0);
		return true;
	}
	// Map OK key to skip cinematic
	if (ev.key.keysym.sym == 292) {
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_ESCAPE;
		event.kbd.ascii = mapKey(SDLK_ESCAPE, ev.key.keysym.mod, 0);
		return true;
	}
#endif

#ifdef QTOPIA
	// Quit on fn+backspace on zaurus
	if (ev.key.keysym.sym == 127) {
		event.type = Common::EVENT_QUIT;
		return true;
	}

	// Map menu key (f11) to f5 (scumm menu)
	if (ev.key.keysym.sym == SDLK_F11) {
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_F5;
		event.kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
	}
	// Nap center (space) to tab (default action )
	// I wanted to map the calendar button but the calendar comes up
	//
	else if (ev.key.keysym.sym == SDLK_SPACE) {
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_TAB;
		event.kbd.ascii = mapKey(SDLK_TAB, ev.key.keysym.mod, 0);
	}
	// Since we stole space (pause) above we'll rebind it to the tab key on the keyboard
	else if (ev.key.keysym.sym == SDLK_TAB) {
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_SPACE;
		event.kbd.ascii = mapKey(SDLK_SPACE, ev.key.keysym.mod, 0);
	} else {
	// Let the events fall through if we didn't change them, this may not be the best way to
	// set it up, but i'm not sure how sdl would like it if we let if fall through then redid it though.
	// and yes i have an huge terminal size so i dont wrap soon enough.
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = ev.key.keysym.sym;
		event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);
	}
#endif
	return false;
}
