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

#ifndef SDL_COMMON_H
#define SDL_COMMON_H

#if defined(__SYMBIAN32__)
#include <esdl\SDL.h>
#else
#include <SDL.h>
#endif

#include "backends/modular-backend.h"

#include "backends/mutex/sdl/sdl-mutex.h"
#include "backends/graphics/sdl/sdl-graphics.h"
#include "backends/audiocd/sdl/sdl-audiocd.h"

#include "graphics/scaler.h"


namespace Audio {
	class MixerImpl;
}

#if !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
// Uncomment this to enable the 'on screen display' code.
#define USE_OSD	1
#endif

#if defined(MACOSX)
// On Mac OS X, we need to double buffer the audio buffer, else anything
// which produces sampled data with high latency (like the MT-32 emulator)
// will sound terribly.
// This could be enabled for more / most ports in the future, but needs some
// testing.
#define MIXER_DOUBLE_BUFFERING 1
#endif

class OSystem_SDL : public ModularBackend {
public:
	OSystem_SDL();
	virtual ~OSystem_SDL();

	virtual void initBackend();

	// Get the number of milliseconds since the program was started.
	uint32 getMillis();

	// Delay for a specified amount of milliseconds
	void delayMillis(uint msecs);

	virtual void getTimeAndDate(TimeDate &t) const;

	// Get the next event.
	// Returns true if an event was retrieved.
	virtual bool pollEvent(Common::Event &event); // overloaded by CE backend

	// Define all hardware keys for keymapper
	virtual Common::HardwareKeySet *getHardwareKeySet();

	virtual void preprocessEvents(SDL_Event *event) {}

	// Set function that generates samples
	virtual void setupMixer();
	static void mixCallback(void *s, byte *samples, int len);
	virtual void closeMixer();

	// Quit
	virtual void quit(); // overloaded by CE backend

	void deinit();

	virtual void setWindowCaption(const char *caption);

	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0);
	virtual Common::SeekableReadStream *createConfigReadStream();
	virtual Common::WriteStream *createConfigWriteStream();

protected:
	bool _inited;
	SDL_AudioSpec _obtainedRate;

	// Keyboard mouse emulation.  Disabled by fingolfin 2004-12-18.
	// I am keeping the rest of the code in for now, since the joystick
	// code (or rather, "hack") uses it, too.
	struct KbdMouse {
		int16 x, y, x_vel, y_vel, x_max, y_max, x_down_count, y_down_count;
		uint32 last_time, delay_time, x_down_time, y_down_time;
	};

	KbdMouse _km;

	// Scroll lock state - since SDL doesn't track it
	bool _scrollLock;
	
	// joystick
	SDL_Joystick *_joystick;

	virtual bool dispatchSDLEvent(SDL_Event &ev, Common::Event &event);

	// Handlers for specific SDL events, called by pollEvent.
	// This way, if a backend inherits fromt the SDL backend, it can
	// change the behavior of only a single event, without having to override all
	// of pollEvent.
	virtual bool handleKeyDown(SDL_Event &ev, Common::Event &event);
	virtual bool handleKeyUp(SDL_Event &ev, Common::Event &event);
	virtual bool handleMouseMotion(SDL_Event &ev, Common::Event &event);
	virtual bool handleMouseButtonDown(SDL_Event &ev, Common::Event &event);
	virtual bool handleMouseButtonUp(SDL_Event &ev, Common::Event &event);
	virtual bool handleJoyButtonDown(SDL_Event &ev, Common::Event &event);
	virtual bool handleJoyButtonUp(SDL_Event &ev, Common::Event &event);
	virtual bool handleJoyAxisMotion(SDL_Event &ev, Common::Event &event);

#ifdef MIXER_DOUBLE_BUFFERING
	SDL_mutex *_soundMutex;
	SDL_cond *_soundCond;
	SDL_Thread *_soundThread;
	bool _soundThreadIsRunning;
	bool _soundThreadShouldQuit;

	byte _activeSoundBuf;
	uint _soundBufSize;
	byte *_soundBuffers[2];

	void mixerProducerThread();
	static int SDLCALL mixerProducerThreadEntry(void *arg);
	void initThreadedMixer(Audio::Mixer *mixer, uint bufSize);
	void deinitThreadedMixer();
#endif

	SDL_TimerID _timerID;

	virtual void fillMouseEvent(Common::Event &event, int x, int y); // overloaded by CE backend
	void toggleMouseGrab();

	void handleKbdMouse();

	void setupIcon();

	virtual bool remapKey(SDL_Event &ev, Common::Event &event);
};

#endif
