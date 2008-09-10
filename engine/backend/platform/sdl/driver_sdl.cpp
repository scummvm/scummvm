/* Residual - Virtual machine to run LucasArts' 3D adventure games
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

#include "common/debug.h"

#include "mixer/mixer_intern.h"

#include "engine/backend/platform/sdl/driver_sdl.h"
#include "engine/backend/timer/default/default-timer.h"
#include "engine/backend/saves/default/default-saves.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include "icons/residual.xpm"

#define SAMPLES_PER_SEC 22050

/*
 * Include header files needed for the getFilesystemFactory() method.
 */
#if defined(__amigaos4__)
	#include "engine/backend/fs/amigaos4/amigaos4-fs-factory.h"
#elif defined(UNIX)
	#include "engine/backend/fs/posix/posix-fs-factory.h"
#elif defined(WIN32)
	#include "engine/backend/fs/windows/windows-fs-factory.h"
#endif


// NOTE: This is not a complete driver, it needs to be subclassed
//       to provide rendering functionality.


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

// numupper provides conversion between number keys and their "upper case"
const char numupper[] = {')', '!', '@', '#', '$', '%', '^', '&', '*', '('};

static int mapKey(SDLKey key, SDLMod mod, Uint16 unicode) {
	if (key >= SDLK_F1 && key <= SDLK_F9) {
		return key - SDLK_F1 + Common::ASCII_F1;
	} else if (key >= SDLK_KP0 && key <= SDLK_KP9) {
		return key - SDLK_KP0 + '0';
	} else if (key >= SDLK_UP && key <= SDLK_PAGEDOWN) {
		return key;
	} else if (unicode) {
		return unicode;
	} else if (key >= 'a' && key <= 'z' && mod & KMOD_SHIFT) {
		return key & ~0x20;
	} else if (key >= SDLK_0 && key <= SDLK_9 && mod & KMOD_SHIFT) {
		return numupper[key - SDLK_0];
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

void DriverSDL::fillMouseEvent(Common::Event &event, int x, int y) {
	event.mouse.x = x;
	event.mouse.y = y;

	// Update the "keyboard mouse" coords
	_km.x = x;
	_km.y = y;
}

void DriverSDL::handleKbdMouse() {
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

void DriverSDL::warpMouse(int x, int y) {
	SDL_WarpMouse(x, y);
}

bool DriverSDL::pollEvent(Common::Event &event) {
	SDL_Event ev;
	int axis;
	byte b = 0;

	handleKbdMouse();

	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
		case SDL_KEYDOWN:
			{
				b = event.kbd.flags = SDLModToOSystemKeyFlags(SDL_GetModState());

				// Alt-Return and Alt-Enter toggle full screen mode
				if (b == Common::KBD_ALT && (ev.key.keysym.sym == SDLK_RETURN || ev.key.keysym.sym == SDLK_KP_ENTER)) {
					toggleFullscreenMode();
					break;
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

bool DriverSDL::remapKey(SDL_Event &ev, Common::Event &event) {

	return false;
}

static Uint32 timer_handler(Uint32 interval, void *param) {
	((DefaultTimerManager *)param)->handler();
	return interval;
}

Common::TimerManager *DriverSDL::getTimerManager() {
	assert(_timer);
	return _timer;
}

void DriverSDL::getTimeAndDate(struct tm &t) const {
	time_t curTime = time(0);
	t = *localtime(&curTime);
}

DriverSDL::DriverSDL() {
	_mixer = NULL;
	_timer = NULL;
	_savefile = NULL;
}

DriverSDL::~DriverSDL() {
	SDL_RemoveTimer(_timerID);
	SDL_CloseAudio();

	delete _mixer;
	delete _timer;
	delete _savefile;
}

void DriverSDL::init() {
	_timer = new DefaultTimerManager();
	_timerID = SDL_AddTimer(10, &timer_handler, _timer);

	if (!_savefile) {
		_savefile = new DefaultSaveFileManager();
	}

#if !defined(MACOSX)
	setupIcon();
#endif
}

const char *DriverSDL::getVideoDeviceName() {
	return "SDL Video Device";
}

uint32 DriverSDL::getMillis() {
	return SDL_GetTicks();
}

void DriverSDL::delayMillis(uint msecs) {
	SDL_Delay(msecs);
}

Common::MutexRef DriverSDL::createMutex() {
	return (MutexRef)SDL_CreateMutex();
}

void DriverSDL::lockMutex(MutexRef mutex) {
	SDL_mutexP((SDL_mutex *)mutex);
}

void DriverSDL::unlockMutex(MutexRef mutex) {
	SDL_mutexV((SDL_mutex *)mutex);
}

void DriverSDL::deleteMutex(MutexRef mutex) {
	SDL_DestroyMutex((SDL_mutex *)mutex);
}

void DriverSDL::mixCallback(void *sys, byte *samples, int len) {
	DriverSDL *this_ = (DriverSDL *)sys;
	assert(this_);

	if (this_->_mixer)
		this_->_mixer->mixCallback(samples, len);
}

void DriverSDL::setupMixer() {
	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;

	// Determine the desired output sampling frequency.
	_samplesPerSec = 0;

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
	_mixer = new Audio::MixerImpl();
	assert(_mixer);

	if (SDL_OpenAudio(&desired, &obtained) != 0) {
		warning("Could not open audio device: %s", SDL_GetError());
		_samplesPerSec = 0;
		_mixer->setReady(false);
	} else {
		_samplesPerSec = obtained.freq;
		_mixer->setOutputRate(_samplesPerSec);
		_mixer->setReady(true);
		SDL_PauseAudio(0);
	}
}

Audio::Mixer *DriverSDL::getMixer() {
	assert(_mixer);
	return _mixer;
}

/* This function sends the SDL signal to
 * go ahead and exit the game
 */
void DriverSDL::quit() {
	SDL_Event event;
	
	event.type = SDL_QUIT;
	if (SDL_PushEvent(&event) != 0)
		error("Unable to push exit event!");
}

FilesystemFactory *DriverSDL::getFilesystemFactory() {
	#if defined(__amigaos4__)
		return &AmigaOSFilesystemFactory::instance();	
	#elif defined(UNIX)
		return &POSIXFilesystemFactory::instance();
	#elif defined(WIN32)
		return &WindowsFilesystemFactory::instance();
	#elif defined(__SYMBIAN32__)
		// Do nothing since its handled by the Symbian SDL inheritance
	#else
		#error Unknown and unsupported backend in Driver_SDL::getFilesystemFactory
	#endif
}

Common::SaveFileManager *DriverSDL::getSavefileManager() {
	assert(_savefile);
	return _savefile;
}

void DriverSDL::setupIcon() {
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

#if defined (WIN32)
int __stdcall WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/,  LPSTR /*lpCmdLine*/, int /*iShowCmd*/) {
	SDL_SetModuleHandle(GetModuleHandle(NULL));
	return main(__argc, __argv);
}
#endif

int main(int argc, char *argv[]) {
	int res = residual_main(argc, argv);

	return res;
}
