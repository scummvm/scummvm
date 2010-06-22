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

#include "backends/platform/posix/posix.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/util.h"
#include "common/EventRecorder.h"

#include "backends/saves/posix/posix-saves.h"
  
#include "backends/audiocd/sdl/sdl-audiocd.h"
#include "backends/events/sdl/sdl-events.h"
#include "backends/mutex/sdl/sdl-mutex.h"
#include "backends/mixer/sdl/sdl-mixer.h"
#include "backends/timer/sdl/sdl-timer.h"

#include "backends/fs/posix/posix-fs-factory.h"

#define DEFAULT_CONFIG_FILE ".scummvmrc"

OSystem_POSIX::OSystem_POSIX() {
	_fsFactory = new POSIXFilesystemFactory();
}

OSystem_POSIX::~OSystem_POSIX() {
	deinit();
}

void OSystem_POSIX::initBackend() {
	assert(!_inited);

	uint32 sdlFlags = 0;

	if (ConfMan.hasKey("disable_sdl_parachute"))
		sdlFlags |= SDL_INIT_NOPARACHUTE;

	if (SDL_Init(sdlFlags) == -1) {
		error("Could not initialize SDL: %s", SDL_GetError());
	}

	// Enable unicode support if possible
	SDL_EnableUNICODE(1);

	// Create and hook up the mutex manager, if none exists yet (we check for
	// this to allow subclasses to provide their own).
	if (_mutexManager == 0) {
		_mutexManager = new SdlMutexManager();
	}

	// Create and hook up the event manager, if none exists yet (we check for
	// this to allow subclasses to provide their own).
	if (_eventManager == 0) {
		_eventManager = new SdlEventManager(this);
	}

	// Create the savefile manager, if none exists yet (we check for this to
	// allow subclasses to provide their own).
	if (_savefileManager == 0) {
		_savefileManager = new POSIXSaveFileManager();
	}

	// Create and hook up the mixer, if none exists yet (we check for this to
	// allow subclasses to provide their own).
	if (_mixer == 0) {
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
			error("Could not initialize SDL: %s", SDL_GetError());
		}

		_mixer = new SdlMixerImpl(this);
	}

	// Create and hook up the timer manager, if none exists yet (we check for
	// this to allow subclasses to provide their own).
	if (_timerManager == 0) {
		_timerManager = new SdlTimerManager();
	}

	// Create and hook up the graphics manager, if none exists yet (we check for
	// this to allow subclasses to provide their own).
	if (_graphicsManager == 0) {
		_graphicsManager = new SdlGraphicsManager();
	}

	if (_audiocdManager == 0) {
		_audiocdManager = (AudioCDManager *)new SdlAudioCDManager();
	}

	// Invoke parent implementation of this method
	OSystem::initBackend();

	_inited = true;
}

Common::String OSystem_POSIX::getDefaultConfigFileName() {
	char configFile[MAXPATHLEN];

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

	return configFile;
}
