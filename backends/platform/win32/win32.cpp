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

#include <windows.h>
// winnt.h defines ARRAYSIZE, but we want our own one... - this is needed before including util.h
#undef ARRAYSIZE

#include "backends/platform/win32/win32.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/util.h"

#include "backends/saves/default/default-saves.h"
#include "backends/audiocd/sdl/sdl-audiocd.h"
#include "backends/events/sdl/sdl-events.h"
#include "backends/mutex/sdl/sdl-mutex.h"
#include "backends/mixer/sdl/sdl-mixer.h"
#include "backends/timer/sdl/sdl-timer.h"

#include "icons/scummvm.xpm"

#include "backends/fs/windows/windows-fs-factory.h"

#define DEFAULT_CONFIG_FILE "scummvm.ini"

OSystem_Win32::OSystem_Win32() {
	_fsFactory = new WindowsFilesystemFactory();
}

void OSystem_Win32::initBackend() {
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
		_savefileManager = new DefaultSaveFileManager();
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

	// Setup a custom program icon.
	setupIcon();

	// Invoke parent implementation of this method
	OSystem::initBackend();

	_inited = true;
}

Common::String OSystem_Win32::getDefaultConfigFileName() {
	char configFile[MAXPATHLEN];

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

		strcat(configFile, "\\ScummVM");
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

	return configFile;
}
