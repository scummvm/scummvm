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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/platform/sdl/events.cpp $
 * $Id: events.cpp 43636 2009-08-22 12:35:49Z sev $
 *
 */

#include "backends/platform/samsungtv/sdl.h"
#include "common/util.h"
#include "common/events.h"

#if defined(SAMSUNGTV)

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

void OSystem_SDL_SamsungTV::fillMouseEvent(Common::Event &event, int x, int y) {
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

void OSystem_SDL_SamsungTV::handleKbdMouse() {
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

			setMousePos(_km.x, _km.y);
		}
	}
}

static byte SDLModToOSystemKeyFlags(SDLMod mod) {
	byte b = 0;

	if (mod & KMOD_SHIFT)
		b |= Common::KBD_SHIFT;
	if (mod & KMOD_ALT)
		b |= Common::KBD_ALT;
	if (mod & KMOD_CTRL)
		b |= Common::KBD_CTRL;

	return b;
}

bool OSystem_SDL_SamsungTV::pollEvent(Common::Event &event) {
	SDL_Event ev;
	byte b = 0;

	handleKbdMouse();

	// If the screen mode changed, send an Common::EVENT_SCREEN_CHANGED
	if (_modeChanged) {
		_modeChanged = false;
		event.type = Common::EVENT_SCREEN_CHANGED;
		return true;
	}

	while (SDL_PollEvent(&ev)) {
		preprocessEvents(&ev);

		switch (ev.type) {
		case SDL_KEYDOWN:{
			if (ev.key.keysym.sym == SDLK_UP) {
				_km.y_vel = -1;
				_km.y_down_count = 1;
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);
				return true;
			} else if (ev.key.keysym.sym == SDLK_DOWN) {
				_km.y_vel = 1;
				_km.y_down_count = 1;
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);
				return true;
			} else if (ev.key.keysym.sym == SDLK_LEFT) {
				_km.x_vel = -1;
				_km.x_down_count = 1;
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);
				return true;
			} else if (ev.key.keysym.sym == SDLK_RIGHT) {
				_km.x_vel = 1;
				_km.x_down_count = 1;
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);
				return true;
			} else if (ev.key.keysym.sym == SDLK_z) {
				event.type = Common::EVENT_LBUTTONDOWN;
				fillMouseEvent(event, _km.x, _km.y);
				return true;
			} else if (ev.key.keysym.sym == SDLK_HOME) {
				event.type = Common::EVENT_RBUTTONDOWN;
				fillMouseEvent(event, _km.x, _km.y);
				return true;
			} else if (ev.key.keysym.sym == SDLK_F4) {
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.keycode = Common::KEYCODE_SPACE;
				event.kbd.ascii = ' ';
				return true;
			}

			b = event.kbd.flags = SDLModToOSystemKeyFlags(SDL_GetModState());

			// Alt-S: Create a screenshot
			if (b == Common::KBD_ALT && ev.key.keysym.sym == 's') {
				char filename[20];

				for (int n = 0;; n++) {
					SDL_RWops *file;

					sprintf(filename, "scummvm%05d.bmp", n);
					file = SDL_RWFromFile(filename, "r");
					if (!file)
						break;
					SDL_RWclose(file);
				}
				if (saveScreenshot(filename))
					printf("Saved '%s'\n", filename);
				else
					printf("Could not save screenshot!\n");
				break;
			}

			// On other unices, Control-Q quits
			if ((ev.key.keysym.mod & KMOD_CTRL) && ev.key.keysym.sym == 'q') {
				event.type = Common::EVENT_QUIT;
				return true;
			}

			if ((ev.key.keysym.mod & KMOD_CTRL) && ev.key.keysym.sym == 'u') {
				event.type = Common::EVENT_MUTE;
				return true;
			}

			// Ctrl-Alt-<key> will change the GFX mode
			if ((b & (Common::KBD_CTRL|Common::KBD_ALT)) == (Common::KBD_CTRL|Common::KBD_ALT)) {

				handleScalerHotkeys(ev.key);
				break;
			}
			const bool event_complete = remapKey(ev, event);

			if (event_complete)
				return true;

			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = (Common::KeyCode)ev.key.keysym.sym;
			event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);

			return true;
			}
		case SDL_KEYUP:
			{
			if (ev.key.keysym.sym == SDLK_UP || ev.key.keysym.sym == SDLK_DOWN || ev.key.keysym.sym == SDLK_LEFT || ev.key.keysym.sym == SDLK_RIGHT) {
				_km.x_vel = 0;
				_km.x_down_count = 0;
				_km.y_vel = 0;
				_km.y_down_count = 0;
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);
				return true;
			} else if (ev.key.keysym.sym == SDLK_z) {
				event.type = Common::EVENT_LBUTTONUP;
				fillMouseEvent(event, _km.x, _km.y);
				return true;
			} else if (ev.key.keysym.sym == SDLK_HOME) {
				event.type = Common::EVENT_RBUTTONUP;
				fillMouseEvent(event, _km.x, _km.y);
				return true;
			} else if (ev.key.keysym.sym == SDLK_F4) {
				event.type = Common::EVENT_KEYUP;
				event.kbd.keycode = Common::KEYCODE_SPACE;
				event.kbd.ascii = ' ';
				return true;
			}

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
				break;
			}

			return true;
			}
		case SDL_MOUSEMOTION:
			event.type = Common::EVENT_MOUSEMOVE;
			fillMouseEvent(event, ev.motion.x, ev.motion.y);

			setMousePos(event.mouse.x, event.mouse.y);
			return true;

		case SDL_MOUSEBUTTONDOWN:
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
				break;

			fillMouseEvent(event, ev.button.x, ev.button.y);

			return true;

		case SDL_MOUSEBUTTONUP:
			if (ev.button.button == SDL_BUTTON_LEFT)
				event.type = Common::EVENT_LBUTTONUP;
			else if (ev.button.button == SDL_BUTTON_RIGHT)
				event.type = Common::EVENT_RBUTTONUP;
#if defined(SDL_BUTTON_MIDDLE)
			else if (ev.button.button == SDL_BUTTON_MIDDLE)
				event.type = Common::EVENT_MBUTTONUP;
#endif
			else
				break;
			fillMouseEvent(event, ev.button.x, ev.button.y);

			return true;

		case SDL_VIDEOEXPOSE:
			_forceFull = true;
			break;

		case SDL_QUIT:
			event.type = Common::EVENT_QUIT;
			return true;
		}
	}
	return false;
}

bool OSystem_SDL_SamsungTV::remapKey(const SDL_Event &ev, Common::Event &event) {
	return false;
}

#endif
