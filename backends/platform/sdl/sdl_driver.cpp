/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/util.h"

#include "base/main.h"

#include "backends/platform/sdl/sdl_driver.h"

#ifdef UNIX
	#include "backends/saves/posix/posix-saves.h"
#else
	#include "backends/saves/default/default-saves.h"
#endif
#include "backends/timer/default/default-timer.h"
#include "sound/mixer_intern.h"

#include "backends/timer/default/default-timer.h"
#include "backends/saves/default/default-saves.h"
#include "backends/events/default/default-events.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include "icons/residual.xpm"

#define SAMPLES_PER_SEC 22050

/*
 * Include header files needed for the getFilesystemFactory() method.
 */
#if defined(__amigaos4__)
	#include "backends/fs/amigaos4/amigaos4-fs-factory.h"
#elif defined(UNIX)
	#include "backends/fs/posix/posix-fs-factory.h"
#elif defined(WIN32)
	#include "backends/fs/windows/windows-fs-factory.h"
#endif


#if defined(UNIX)
#ifdef MACOSX
#define DEFAULT_CONFIG_FILE "Library/Preferences/Residual Preferences"
#else
#define DEFAULT_CONFIG_FILE ".residualrc"
#endif
#else
#define DEFAULT_CONFIG_FILE "residual.ini"
#endif

#if defined(MACOSX) || defined(IPHONE)
#include "CoreFoundation/CoreFoundation.h"
#endif

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

void OSystem_SDL::fillMouseEvent(Common::Event &event, int x, int y) {
	event.mouse.x = x;
	event.mouse.y = y;

	// Update the "keyboard mouse" coords
	_km.x = x;
	_km.y = y;
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

			// disable wrap mouse for now, it's really annoying
			//SDL_WarpMouse((Uint16)_km.x, (Uint16)_km.y);
		}
	}
}

bool OSystem_SDL::showMouse(bool visible) {
	return false;
}

void OSystem_SDL::warpMouse(int x, int y) {
	SDL_WarpMouse(x, y);
}

bool OSystem_SDL::pollEvent(Common::Event &event) {
	SDL_Event ev;
	int axis;
	byte b = 0;

	handleKbdMouse();

	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
		case SDL_KEYDOWN:
			{
				b = event.kbd.flags = SDLModToOSystemKeyFlags(SDL_GetModState());

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
				const bool event_complete = remapKey(ev, event);

				if (event_complete)
					return true;

				event.type = Common::EVENT_KEYUP;
				event.kbd.keycode = (Common::KeyCode)ev.key.keysym.sym;
				event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);

				return true;
			}
		case SDL_MOUSEMOTION:
			event.type = Common::EVENT_MOUSEMOVE;
			fillMouseEvent(event, ev.motion.x, ev.motion.y);

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

		case SDL_JOYBUTTONDOWN:
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
				}
			}
			return true;

		case SDL_JOYBUTTONUP:
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
				}
			}
			return true;

		case SDL_JOYAXISMOTION:
			axis = ev.jaxis.value;
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
				_km.x_vel = axis / 2000;
				_km.x_down_count = 0;
#else
				if (axis != 0) {
					_km.x_vel = (axis > 0) ? 1 : -1;
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

		case SDL_VIDEOEXPOSE:
			event.type = Common::EVENT_SCREEN_CHANGED;
			return true;

		case SDL_QUIT:
			event.type = Common::EVENT_QUIT;
			return true;
		}
	}

	return false;
}

bool OSystem_SDL::remapKey(SDL_Event &ev, Common::Event &event) {
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

static Uint32 timer_handler(Uint32 interval, void *param) {
	((DefaultTimerManager *)param)->handler();
	return interval;
}

OSystem_SDL::OSystem_SDL() {
	_mixer = NULL;
	_timer = NULL;
	_savefile = NULL;
	_timerID = 0;
#ifdef MIXER_DOUBLE_BUFFERING
	_soundMutex = NULL;
	_soundCond = NULL;
	_soundThread = NULL;
	_soundThreadIsRunning = false;
	_soundThreadShouldQuit = false;
#endif
	_samplesPerSec = 0;
	_cdrom = 0;
	_overlayVisible = false;
	_overlayscreen = NULL;

	memset(&_km, 0, sizeof(_km));

	#if defined(__amigaos4__)
		_fsFactory = new AmigaOSFilesystemFactory();
	#elif defined(UNIX)
		_fsFactory = new POSIXFilesystemFactory();
	#elif defined(WIN32)
		_fsFactory = new WindowsFilesystemFactory();
	#elif defined(__SYMBIAN32__)
		// Do nothing since its handled by the Symbian SDL inheritance
	#else
		#error Unknown and unsupported FS backend
	#endif
}

OSystem_SDL::~OSystem_SDL() {
	SDL_RemoveTimer(_timerID);
	closeMixer();

	if (_overlayscreen) {
		SDL_FreeSurface(_overlayscreen);
		_overlayscreen = NULL;
#ifdef USE_OPENGL
		if (_overlayNumTex > 0) {
			glDeleteTextures(_overlayNumTex, _overlayTexIds);
			delete[] _overlayTexIds;
			_overlayNumTex = 0;
		}
#endif
	}

	delete _fsFactory;
}

void OSystem_SDL::initBackend() {
	int joystick_num = ConfMan.getInt("joystick_num");
	uint32 sdlFlags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;

	if (ConfMan.hasKey("disable_sdl_parachute"))
		sdlFlags |= SDL_INIT_NOPARACHUTE;

#ifdef _WIN32_WCE
	if (ConfMan.hasKey("use_GDI") && ConfMan.getBool("use_GDI")) {
		SDL_VideoInit("windib", 0);
		sdlFlags ^= SDL_INIT_VIDEO;
	}
#endif

	if (joystick_num > -1)
		sdlFlags |= SDL_INIT_JOYSTICK;

	if (SDL_Init(sdlFlags) == -1) {
		error("Could not initialize SDL: %s", SDL_GetError());
	}

	// disabled for now
	//SDL_ShowCursor(SDL_DISABLE);

	// Enable unicode support if possible
	SDL_EnableUNICODE(1);

#if !defined(MACOSX)
	setupIcon();
#endif

	// Create the savefile manager, if none exists yet (we check for this to
	// allow subclasses to provide their own).
	if (!_savefile) {
#ifdef UNIX
		_savefile = new POSIXSaveFileManager();
#else
		_savefile = new DefaultSaveFileManager();
#endif
	}

	// Create and hook up the mixer, if none exists yet (we check for this to
	// allow subclasses to provide their own).
	if (_mixer == 0) {
		setupMixer();
	}

	// Create and hook up the timer manager, if none exists yet (we check for
	// this to allow subclasses to provide their own).
	if (_timer == NULL) {
		// Note: We could implement a custom SDLTimerManager by using
		// SDL_AddTimer. That might yield better timer resolution, but it would
		// also change the semantics of a timer: Right now, ScummVM timers
		// *never* run in parallel, due to the way they are implemented. If we
		// switched to SDL_AddTimer, each timer might run in a separate thread.
		// However, not all our code is prepared for that, so we can't just
		// switch. Still, it's a potential future change to keep in mind.
		_timer = new DefaultTimerManager();
		_timerID = SDL_AddTimer(10, &timer_handler, _timer);
	}
}

byte *OSystem_SDL::setupScreen(int screenW, int screenH, bool fullscreen, bool accel3d) {
	uint32 sdlflags;
	int bpp;

#ifdef USE_OPENGL
	_opengl = accel3d;
#endif
	_fullscreen = fullscreen;

#ifdef USE_OPENGL
	if (_opengl) {
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		sdlflags = SDL_OPENGL;
		bpp = 24;
	} else
#endif
	{
		bpp = 16;
		sdlflags = SDL_HWSURFACE;
	}

	if (_fullscreen)
		sdlflags |= SDL_FULLSCREEN;

	_screen = SDL_SetVideoMode(screenW, screenH, bpp, sdlflags);
	if (!_screen)
		error("Could not initialize video");

#ifdef USE_OPENGL
	if (_opengl) {
		int glflag;

		// apply atribute again for sure based on SDL docs
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &glflag);
		warning("INFO: GL RED bits: %d", glflag);
		SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &glflag);
		warning("INFO: GL GREEN bits: %d", glflag);
		SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &glflag);
		warning("INFO: GL BLUE bits: %d", glflag);
		SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &glflag);
		warning("INFO: GL APLHA bits: %d", glflag);
		SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &glflag);
		warning("INFO: GL Z buffer depth bits: %d", glflag);
		SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &glflag);
		warning("INFO: GL Double Buffer: %d", glflag);
		SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &glflag);
		warning("INFO: GL Stencil buffer bits: %d", glflag);
	}
#endif

	_overlayWidth = screenW;
	_overlayHeight = screenH;

	Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0x00001f00;
	gmask = 0x000007e0;
	bmask = 0x000000f8;
	amask = 0x00000000;
#else
	rmask = 0x0000001f;
	gmask = 0x000007e0;
	bmask = 0x0000f800;
	amask = 0x00000000;
#endif
	_overlayscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _overlayWidth, _overlayHeight, 16,
						rmask, gmask, bmask, amask);

	if (!_overlayscreen)
		error("allocating _overlayscreen failed");

	_overlayFormat.bytesPerPixel = _overlayscreen->format->BytesPerPixel;

	_overlayFormat.rLoss = _overlayscreen->format->Rloss;
	_overlayFormat.gLoss = _overlayscreen->format->Gloss;
	_overlayFormat.bLoss = _overlayscreen->format->Bloss;
	_overlayFormat.aLoss = _overlayscreen->format->Aloss;

	_overlayFormat.rShift = _overlayscreen->format->Rshift;
	_overlayFormat.gShift = _overlayscreen->format->Gshift;
	_overlayFormat.bShift = _overlayscreen->format->Bshift;
	_overlayFormat.aShift = _overlayscreen->format->Ashift;

	return (byte *)_screen->pixels;
}

#define BITMAP_TEXTURE_SIZE 256

void OSystem_SDL::updateScreen() {
#ifdef USE_OPENGL
	if (_opengl) {
		if (_overlayVisible) {
			if (_overlayDirty) {
				// remove if already exist
				if (_overlayNumTex > 0) {
					glDeleteTextures(_overlayNumTex, _overlayTexIds);
					delete[] _overlayTexIds;
					_overlayNumTex = 0;
				}

				_overlayNumTex = ((_overlayWidth + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE) *
								((_overlayHeight + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE);
				_overlayTexIds = new GLuint[_overlayNumTex];
				glGenTextures(_overlayNumTex, _overlayTexIds);
				for (int i = 0; i < _overlayNumTex; i++) {
					glBindTexture(GL_TEXTURE_2D, _overlayTexIds[i]);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BITMAP_TEXTURE_SIZE, BITMAP_TEXTURE_SIZE, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
				}

				glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
				glPixelStorei(GL_UNPACK_ROW_LENGTH, _overlayWidth);

				int curTexIdx = 0;
				for (int y = 0; y < _overlayHeight; y += BITMAP_TEXTURE_SIZE) {
					for (int x = 0; x < _overlayWidth; x += BITMAP_TEXTURE_SIZE) {
						int t_width = (x + BITMAP_TEXTURE_SIZE >= _overlayWidth) ? (_overlayWidth - x) : BITMAP_TEXTURE_SIZE;
						int t_height = (y + BITMAP_TEXTURE_SIZE >= _overlayHeight) ? (_overlayHeight - y) : BITMAP_TEXTURE_SIZE;
						glBindTexture(GL_TEXTURE_2D, _overlayTexIds[curTexIdx]);
						glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, t_width, t_height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (byte *)_overlayscreen->pixels + (y * 2 * _overlayWidth) + (2 * x));
						curTexIdx++;
					}
				}
				glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
				glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			}

			// prepare view
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, _overlayWidth, _overlayHeight, 0, 0, 1);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glDisable(GL_LIGHTING);
			glEnable(GL_TEXTURE_2D);
			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);
			glEnable(GL_SCISSOR_TEST);

			glScissor(0, 0, _overlayWidth, _overlayHeight);

			int curTexIdx = 0;
			for (int y = 0; y < _overlayHeight; y += BITMAP_TEXTURE_SIZE) {
				for (int x = 0; x < _overlayWidth; x += BITMAP_TEXTURE_SIZE) {
					glBindTexture(GL_TEXTURE_2D, _overlayTexIds[curTexIdx]);
					glBegin(GL_QUADS);
					glTexCoord2f(0, 0);
					glVertex2i(x, y);
					glTexCoord2f(1.0, 0.0);
					glVertex2i(x + BITMAP_TEXTURE_SIZE, y);
					glTexCoord2f(1.0, 1.0);
					glVertex2i(x + BITMAP_TEXTURE_SIZE, y + BITMAP_TEXTURE_SIZE);
					glTexCoord2f(0.0, 1.0);
					glVertex2i(x, y + BITMAP_TEXTURE_SIZE);
					glEnd();
					curTexIdx++;
				}
			}

			glDisable(GL_SCISSOR_TEST);
			glDisable(GL_TEXTURE_2D);
			glDepthMask(GL_TRUE);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_LIGHTING);
		}
		SDL_GL_SwapBuffers();
	} else
#endif
	{
		if (_overlayVisible) {
			SDL_LockSurface(_screen);
			SDL_LockSurface(_overlayscreen);
			byte *src = (byte *)_overlayscreen->pixels;
			byte *buf = (byte *)_screen->pixels;
			int h = _overlayHeight;
			do {
				memcpy(buf, src, _overlayWidth * _overlayscreen->format->BytesPerPixel);
				src += _overlayscreen->pitch;
				buf += _screen->pitch;
			} while (--h);
			SDL_UnlockSurface(_screen);
			SDL_UnlockSurface(_overlayscreen);
		}
		SDL_Flip(_screen);
	}
}

void OSystem_SDL::showOverlay() {
	if (_overlayVisible)
		return;

	_overlayVisible = true;

	clearOverlay();
}

void OSystem_SDL::hideOverlay() {
	if (!_overlayVisible)
		return;

	_overlayVisible = false;

	clearOverlay();
}

void OSystem_SDL::clearOverlay() {
	if (!_overlayVisible)
		return;

#ifdef USE_OPENGL
	if (_opengl) {
		SDL_LockSurface(_overlayscreen);
		glReadPixels(0, 0, _overlayWidth, _overlayWidth, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, _overlayscreen->pixels);
		SDL_UnlockSurface(_overlayscreen);
	} else
#endif
	{
		SDL_LockSurface(_screen);
		SDL_LockSurface(_overlayscreen);
		byte *src = (byte *)_screen->pixels;
		byte *buf = (byte *)_overlayscreen->pixels;
		int h = _overlayHeight;
		do {
			memcpy(buf, src, _overlayWidth * _overlayscreen->format->BytesPerPixel);
			src += _screen->pitch;
			buf += _overlayscreen->pitch;
		} while (--h);
		SDL_UnlockSurface(_screen);
		SDL_UnlockSurface(_overlayscreen);
	}
	_overlayDirty = true;
}

void OSystem_SDL::grabOverlay(OverlayColor *buf, int pitch) {
	if (_overlayscreen == NULL)
		return;

	if (SDL_LockSurface(_overlayscreen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *src = (byte *)_overlayscreen->pixels;
	int h = _overlayHeight;
	do {
		memcpy(buf, src, _overlayWidth * _overlayscreen->format->BytesPerPixel);
		src += _overlayscreen->pitch;
		buf += pitch;
	} while (--h);

	SDL_UnlockSurface(_overlayscreen);
}

void OSystem_SDL::copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {
	if (!_overlayscreen)
		return;

	// Clip the coordinates
	if (x < 0) {
		w += x;
		buf -= x;
		x = 0;
	}

	if (y < 0) {
		h += y;
		buf -= y * pitch;
		y = 0;
	}

	if (w > _overlayWidth - x) {
		w = _overlayWidth - x;
	}

	if (h > _overlayHeight - y) {
		h = _overlayHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;

	if (SDL_LockSurface(_overlayscreen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *dst = (byte *)_overlayscreen->pixels + y * _overlayscreen->pitch + x * _overlayscreen->format->BytesPerPixel;
	do {
		memcpy(dst, buf, w * _overlayscreen->format->BytesPerPixel);
		dst += _overlayscreen->pitch;
		buf += pitch;
	} while (--h);

	SDL_UnlockSurface(_overlayscreen);
}

int16 OSystem_SDL::getHeight() {
	return _screen->h;
}

int16 OSystem_SDL::getWidth() {
	return _screen->w;
}

uint32 OSystem_SDL::getMillis() {
	uint32 millis = SDL_GetTicks();
	getEventManager()->processMillis(millis);
	return millis;
}

void OSystem_SDL::delayMillis(uint msecs) {
	SDL_Delay(msecs);
}

void OSystem_SDL::getTimeAndDate(struct tm &t) const {
	time_t curTime = time(0);
	t = *localtime(&curTime);
}

Common::TimerManager *OSystem_SDL::getTimerManager() {
	assert(_timer);
	return _timer;
}

Common::SaveFileManager *OSystem_SDL::getSavefileManager() {
	assert(_savefile);
	return _savefile;
}

FilesystemFactory *OSystem_SDL::getFilesystemFactory() {
	assert(_fsFactory);
	return _fsFactory;
}

void OSystem_SDL::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {

#ifdef DATA_PATH
	// Add the global DATA_PATH to the directory search list
	// FIXME: We use depth = 4 for now, to match the old code. May want to change that
	Common::FSNode dataNode(DATA_PATH);
	if (dataNode.exists() && dataNode.isDirectory()) {
		s.add(DATA_PATH, new Common::FSDirectory(dataNode, 4), priority);
	}
#endif

#ifdef MACOSX
	// Get URL of the Resource directory of the .app bundle
	CFURLRef fileUrl = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
	if (fileUrl) {
		// Try to convert the URL to an absolute path
		UInt8 buf[MAXPATHLEN];
		if (CFURLGetFileSystemRepresentation(fileUrl, true, buf, sizeof(buf))) {
			// Success: Add it to the search path
			Common::String bundlePath((const char *)buf);
			s.add("__OSX_BUNDLE__", new Common::FSDirectory(bundlePath), priority);
		}
		CFRelease(fileUrl);
	}

#endif

}

static Common::String getDefaultConfigFileName() {
	char configFile[MAXPATHLEN];
#if defined (WIN32) && !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
	OSVERSIONINFO win32OsVersion;
	ZeroMemory(&win32OsVersion, sizeof(OSVERSIONINFO));
	win32OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&win32OsVersion);
	// Check for non-9X version of Windows.
	if (win32OsVersion.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS) {
		// Use the Application Data directory of the user profile.
		if (win32OsVersion.dwMajorVersion >= 5) {
			if (!GetEnvironmentVariable("APPDATA", configFile, sizeof(configFile)))
				error("Unable to access application data directory");
		} else {
			if (!GetEnvironmentVariable("USERPROFILE", configFile, sizeof(configFile)))
				error("Unable to access user profile directory");

			strcat(configFile, "\\Application Data");
			CreateDirectory(configFile, NULL);
		}

		strcat(configFile, "\\Residual");
		CreateDirectory(configFile, NULL);
		strcat(configFile, "\\" DEFAULT_CONFIG_FILE);

		FILE *tmp = NULL;
		if ((tmp = fopen(configFile, "r")) == NULL) {
			// Check windows directory
			char oldConfigFile[MAXPATHLEN];
			GetWindowsDirectory(oldConfigFile, MAXPATHLEN);
			strcat(oldConfigFile, "\\" DEFAULT_CONFIG_FILE);
			if ((tmp = fopen(oldConfigFile, "r"))) {
				strcpy(configFile, oldConfigFile);

				fclose(tmp);
			}
		} else {
			fclose(tmp);
		}
	} else {
		// Check windows directory
		GetWindowsDirectory(configFile, MAXPATHLEN);
		strcat(configFile, "\\" DEFAULT_CONFIG_FILE);
	}
#elif defined(UNIX)
	// On UNIX type systems, by default we store the config file inside
	// to the HOME directory of the user.
	//
	// GP2X is Linux based but Home dir can be read only so do not use
	// it and put the config in the executable dir.
	//
	// On the iPhone, the home dir of the user when you launch the app
	// from the Springboard, is /. Which we don't want.
	const char *home = getenv("HOME");
	if (home != NULL && strlen(home) < MAXPATHLEN)
		snprintf(configFile, MAXPATHLEN, "%s/%s", home, DEFAULT_CONFIG_FILE);
	else
		strcpy(configFile, DEFAULT_CONFIG_FILE);
#else
	strcpy(configFile, DEFAULT_CONFIG_FILE);
#endif

	return configFile;
}

Common::SeekableReadStream *OSystem_SDL::createConfigReadStream() {
	Common::FSNode file(getDefaultConfigFileName());
	return file.createReadStream();
}

Common::WriteStream *OSystem_SDL::createConfigWriteStream() {
	Common::FSNode file(getDefaultConfigFileName());
	return file.createWriteStream();
}

void OSystem_SDL::setWindowCaption(const char *caption) {
	Common::String cap(caption);

	// Filter out any non-ASCII characters, replacing them by question marks.
	// At some point, we may wish to allow LATIN 1 or UTF-8.
	for (uint i = 0; i < cap.size(); ++i)
		if ((byte)cap[i] > 0x7F)
			cap.setChar('?', i);
	SDL_WM_SetCaption(caption, caption);
}

bool OSystem_SDL::hasFeature(Feature f) {
	return
		(f == kFeatureFullscreenMode) ||
#ifdef USE_OPENGL
		(f == kFeatureOpenGL) ||
#endif
		(f == kFeatureIconifyWindow);
}

void OSystem_SDL::setFeatureState(Feature f, bool enable) {
	switch (f) {
	case kFeatureIconifyWindow:
		if (enable)
			SDL_WM_IconifyWindow();
		break;
	default:
		break;
	}
}

bool OSystem_SDL::getFeatureState(Feature f) {
	switch (f) {
	case kFeatureFullscreenMode:
		return _fullscreen;
	default:
		return false;
	}
}

void OSystem_SDL::quit() {
	SDL_ShowCursor(SDL_ENABLE);

	SDL_RemoveTimer(_timerID);
	closeMixer();

	delete _savefile;
	delete _timer;

	SDL_Quit();

	delete getEventManager();
}

void OSystem_SDL::setupIcon() {
	int w, h, ncols, nbytes, i;
	unsigned int rgba[256], icon[32 * 32];
	unsigned char mask[32][4];

	sscanf(residual_icon[0], "%d %d %d %d", &w, &h, &ncols, &nbytes);
	if ((w != 32) || (h != 32) || (ncols > 255) || (nbytes > 1)) {
		warning("Could not load the icon (%d %d %d %d)", w, h, ncols, nbytes);
		return;
	}
	for (i = 0; i < ncols; i++) {
		unsigned char code;
		char color[32];
		unsigned int col;

		sscanf(residual_icon[1 + i], "%c c %s", &code, color);
		if (!strcmp(color, "None"))
			col = 0x00000000;
		else if (!strcmp(color, "black"))
			col = 0xFF000000;
		else if (color[0] == '#') {
			sscanf(color + 1, "%06x", &col);
			col |= 0xFF000000;
		} else {
			warning("Could not load the icon (%d %s - %s) ", code, color, residual_icon[1 + i]);
			return;
		}

		rgba[code] = col;
	}
	memset(mask, 0, sizeof(mask));
	for (h = 0; h < 32; h++) {
		const char *line = residual_icon[1 + ncols + h];
		for (w = 0; w < 32; w++) {
			icon[w + 32 * h] = rgba[(int)line[w]];
			if (rgba[(int)line[w]] & 0xFF000000) {
				mask[h][w >> 3] |= 1 << (7 - (w & 0x07));
			}
		}
	}

	SDL_Surface *sdl_surf = SDL_CreateRGBSurfaceFrom(icon, 32, 32, 32, 32 * 4, 0xFF0000, 0x00FF00, 0x0000FF, 0xFF000000);
	SDL_WM_SetIcon(sdl_surf, (unsigned char *)mask);
	SDL_FreeSurface(sdl_surf);
}

OSystem::MutexRef OSystem_SDL::createMutex() {
	return (MutexRef)SDL_CreateMutex();
}

void OSystem_SDL::lockMutex(MutexRef mutex) {
	SDL_mutexP((SDL_mutex *)mutex);
}

void OSystem_SDL::unlockMutex(MutexRef mutex) {
	SDL_mutexV((SDL_mutex *)mutex);
}

void OSystem_SDL::deleteMutex(MutexRef mutex) {
	SDL_DestroyMutex((SDL_mutex *)mutex);
}


#pragma mark -
#pragma mark --- Audio ---
#pragma mark -

#ifdef MIXER_DOUBLE_BUFFERING

void OSystem_SDL::mixerProducerThread() {
	byte nextSoundBuffer;

	SDL_LockMutex(_soundMutex);
	while (true) {
		// Wait till we are allowed to produce data
		SDL_CondWait(_soundCond, _soundMutex);

		if (_soundThreadShouldQuit)
			break;

		// Generate samples and put them into the next buffer
		nextSoundBuffer = _activeSoundBuf ^ 1;
		_mixer->mixCallback(_soundBuffers[nextSoundBuffer], _soundBufSize);

		// Swap buffers
		_activeSoundBuf = nextSoundBuffer;
	}
	SDL_UnlockMutex(_soundMutex);
}

int SDLCALL OSystem_SDL::mixerProducerThreadEntry(void *arg) {
	OSystem_SDL *this_ = (OSystem_SDL *)arg;
	assert(this_);
	this_->mixerProducerThread();
	return 0;
}

void OSystem_SDL::initThreadedMixer(Audio::MixerImpl *mixer, uint bufSize) {
	_soundThreadIsRunning = false;
	_soundThreadShouldQuit = false;

	// Create mutex and condition variable
	_soundMutex = SDL_CreateMutex();
	_soundCond = SDL_CreateCond();

	// Create two sound buffers
	_activeSoundBuf = 0;
	_soundBufSize = bufSize;
	_soundBuffers[0] = (byte *)calloc(1, bufSize);
	_soundBuffers[1] = (byte *)calloc(1, bufSize);

	_soundThreadIsRunning = true;

	// Finally start the thread
	_soundThread = SDL_CreateThread(mixerProducerThreadEntry, this);
}

void OSystem_SDL::deinitThreadedMixer() {
	// Kill thread?? _soundThread

	if (_soundThreadIsRunning) {
		// Signal the producer thread to end, and wait for it to actually finish.
		_soundThreadShouldQuit = true;
		SDL_CondBroadcast(_soundCond);
		SDL_WaitThread(_soundThread, NULL);

		// Kill the mutex & cond variables.
		// Attention: AT this point, the mixer callback must not be running
		// anymore, else we will crash!
		SDL_DestroyMutex(_soundMutex);
		SDL_DestroyCond(_soundCond);

		_soundThreadIsRunning = false;

		free(_soundBuffers[0]);
		free(_soundBuffers[1]);
	}
}


void OSystem_SDL::mixCallback(void *arg, byte *samples, int len) {
	OSystem_SDL *this_ = (OSystem_SDL *)arg;
	assert(this_);
	assert(this_->_mixer);

	assert((int)this_->_soundBufSize == len);

	// Lock mutex, to ensure our data is not overwritten by the producer thread
	SDL_LockMutex(this_->_soundMutex);

	// Copy data from the current sound buffer
	memcpy(samples, this_->_soundBuffers[this_->_activeSoundBuf], len);

	// Unlock mutex and wake up the produced thread
	SDL_UnlockMutex(this_->_soundMutex);
	SDL_CondSignal(this_->_soundCond);
}

#else

void OSystem_SDL::mixCallback(void *sys, byte *samples, int len) {
	OSystem_SDL *this_ = (OSystem_SDL *)sys;
	assert(this_);

	if (this_->_mixer)
		this_->_mixer->mixCallback(samples, len);
}

#endif

void OSystem_SDL::setupMixer() {
	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;

	// Determine the desired output sampling frequency.
	_samplesPerSec = 0;

	if (ConfMan.hasKey("output_rate"))
		_samplesPerSec = ConfMan.getInt("output_rate");
	if (_samplesPerSec <= 0)
		_samplesPerSec = SAMPLES_PER_SEC;

	// Determine the sample buffer size. We want it to store enough data for
	// about 1/16th of a second. Note that it must be a power of two.
	// So e.g. at 22050 Hz, we request a sample buffer size of 2048.
	int samples = 8192;
	while (16 * samples >= _samplesPerSec) {
		samples >>= 1;
	}

	memset(&desired, 0, sizeof(desired));
	desired.freq = _samplesPerSec;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = (uint16)samples;
	desired.callback = mixCallback;
	desired.userdata = this;

	// Create the mixer instance
	assert(!_mixer);
	_mixer = new Audio::MixerImpl(this);
	assert(_mixer);

	if (SDL_OpenAudio(&desired, &obtained) != 0) {
		warning("Could not open audio device: %s", SDL_GetError());
		_samplesPerSec = 0;
		_mixer->setReady(false);
	} else {
		// Note: This should be the obtained output rate, but it seems that at
		// least on some platforms SDL will lie and claim it did get the rate
		// even if it didn't. Probably only happens for "weird" rates, though.
		_samplesPerSec = obtained.freq;
		debug(1, "Output sample rate: %d Hz", _samplesPerSec);

		// Tell the mixer that we are ready and start the sound processing
		_mixer->setOutputRate(_samplesPerSec);
		_mixer->setReady(true);

#ifdef MIXER_DOUBLE_BUFFERING
		initThreadedMixer(_mixer, obtained.samples * 4);
#endif

		// start the sound system
		SDL_PauseAudio(0);
	}
}

void OSystem_SDL::closeMixer() {
	if (_mixer)
		_mixer->setReady(false);

	SDL_CloseAudio();

	delete _mixer;
	_mixer = 0;

#ifdef MIXER_DOUBLE_BUFFERING
	deinitThreadedMixer();
#endif

}

Audio::Mixer *OSystem_SDL::getMixer() {
	assert(_mixer);
	return _mixer;
}

bool OSystem_SDL::openCD(int drive) {
	if (SDL_InitSubSystem(SDL_INIT_CDROM) == -1)
		_cdrom = NULL;
	else {
		_cdrom = SDL_CDOpen(drive);
		// Did it open? Check if _cdrom is NULL
		if (!_cdrom) {
			warning("Couldn't open drive: %s", SDL_GetError());
		} else {
			_cdNumLoops = 0;
			_cdStopTime = 0;
			_cdEndTime = 0;
		}
	}

	return (_cdrom != NULL);
}

void OSystem_SDL::stopCD() {	/* Stop CD Audio in 1/10th of a second */
	_cdStopTime = SDL_GetTicks() + 100;
	_cdNumLoops = 0;
}

void OSystem_SDL::playCD(int track, int num_loops, int start_frame, int duration) {
	if (!num_loops && !start_frame)
		return;

	if (!_cdrom)
		return;

	if (duration > 0)
		duration += 5;

	_cdTrack = track;
	_cdNumLoops = num_loops;
	_cdStartFrame = start_frame;

	SDL_CDStatus(_cdrom);
	if (start_frame == 0 && duration == 0)
		SDL_CDPlayTracks(_cdrom, track, 0, 1, 0);
	else
		SDL_CDPlayTracks(_cdrom, track, start_frame, 0, duration);
	_cdDuration = duration;
	_cdStopTime = 0;
	_cdEndTime = SDL_GetTicks() + _cdrom->track[track].length * 1000 / CD_FPS;
}

bool OSystem_SDL::pollCD() {
	if (!_cdrom)
		return false;

	return (_cdNumLoops != 0 && (SDL_GetTicks() < _cdEndTime || SDL_CDStatus(_cdrom) == CD_PLAYING));
}

void OSystem_SDL::updateCD() {
	if (!_cdrom)
		return;

	if (_cdStopTime != 0 && SDL_GetTicks() >= _cdStopTime) {
		SDL_CDStop(_cdrom);
		_cdNumLoops = 0;
		_cdStopTime = 0;
		return;
	}

	if (_cdNumLoops == 0 || SDL_GetTicks() < _cdEndTime)
		return;

	if (_cdNumLoops != 1 && SDL_CDStatus(_cdrom) != CD_STOPPED) {
		// Wait another second for it to be done
		_cdEndTime += 1000;
		return;
	}

	if (_cdNumLoops > 0)
		_cdNumLoops--;

	if (_cdNumLoops != 0) {
		if (_cdStartFrame == 0 && _cdDuration == 0)
			SDL_CDPlayTracks(_cdrom, _cdTrack, 0, 1, 0);
		else
			SDL_CDPlayTracks(_cdrom, _cdTrack, _cdStartFrame, 0, _cdDuration);
		_cdEndTime = SDL_GetTicks() + _cdrom->track[_cdTrack].length * 1000 / CD_FPS;
	}
}

#if defined(__SYMBIAN32__)
#include "SymbianOs.h"
#endif

#if !defined(__MAEMO__) && !defined(_WIN32_WCE)

#if defined (WIN32)
int __stdcall WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/,  LPSTR /*lpCmdLine*/, int /*iShowCmd*/) {
	SDL_SetModuleHandle(GetModuleHandle(NULL));
	return main(__argc, __argv);
}
#endif

int main(int argc, char *argv[]) {

#if defined(__SYMBIAN32__)
	//
	// Set up redirects for stdout/stderr under Windows and Symbian.
	// Code copied from SDL_main.
	//

	// Symbian does not like any output to the console through any *print* function
	char STDOUT_FILE[256], STDERR_FILE[256]; // shhh, don't tell anybody :)
	strcpy(STDOUT_FILE, Symbian::GetExecutablePath());
	strcpy(STDERR_FILE, Symbian::GetExecutablePath());
	strcat(STDOUT_FILE, "residual.stdout.txt");
	strcat(STDERR_FILE, "residual.stderr.txt");

	/* Flush the output in case anything is queued */
	fclose(stdout);
	fclose(stderr);

	/* Redirect standard input and standard output */
	FILE *newfp = freopen(STDOUT_FILE, "w", stdout);
	if (newfp == NULL) {	/* This happens on NT */
#if !defined(stdout)
		stdout = fopen(STDOUT_FILE, "w");
#else
		newfp = fopen(STDOUT_FILE, "w");
		if (newfp) {
			*stdout = *newfp;
		}
#endif
	}
	newfp = freopen(STDERR_FILE, "w", stderr);
	if (newfp == NULL) {	/* This happens on NT */
#if !defined(stderr)
		stderr = fopen(STDERR_FILE, "w");
#else
		newfp = fopen(STDERR_FILE, "w");
		if (newfp) {
			*stderr = *newfp;
		}
#endif
	}
	setbuf(stderr, NULL);			/* No buffering */

#endif // defined(__SYMBIAN32__)

	// Create our OSystem instance
#if defined(__SYMBIAN32__)
	g_system = new OSystem_SDL_Symbian();
#else
	g_system = new OSystem_SDL();
#endif

	assert(g_system);

	int res = residual_main(argc, argv);

	return res;
}

#endif
