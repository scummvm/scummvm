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

#if defined(WIN32)
#include <windows.h>
// winnt.h defines ARRAYSIZE, but we want our own one... - this is needed before including util.h
#undef ARRAYSIZE
#endif

#include "backends/platform/sdl/sdl.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/util.h"
#include "common/EventRecorder.h"

#ifdef UNIX
  #include "backends/saves/posix/posix-saves.h"
#else
  #include "backends/saves/default/default-saves.h"
#endif
#include "backends/audiocd/sdl/sdl-audiocd.h"
#include "backends/events/sdl/sdl-events.h"
#include "backends/mutex/sdl/sdl-mutex.h"
#include "backends/mixer/sdl/sdl-mixer.h"
#include "backends/timer/sdl/sdl-timer.h"

#include "icons/scummvm.xpm"

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
#define DEFAULT_CONFIG_FILE "Library/Preferences/ScummVM Preferences"
#elif defined(SAMSUNGTV)
#define DEFAULT_CONFIG_FILE "/dtv/usb/sda1/.scummvmrc"
#else
#define DEFAULT_CONFIG_FILE ".scummvmrc"
#endif
#else
#define DEFAULT_CONFIG_FILE "scummvm.ini"
#endif

#if defined(MACOSX) || defined(IPHONE)
#include "CoreFoundation/CoreFoundation.h"
#endif

#include <time.h>

void OSystem_SDL::initBackend() {
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
#ifdef UNIX
	_savefileManager = new POSIXSaveFileManager();
#else
	_savefileManager = new DefaultSaveFileManager();
#endif
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

#if !defined(MACOSX) && !defined(__SYMBIAN32__)
	// Setup a custom program icon.
	// Don't set icon on OS X, as we use a nicer external icon there.
	// Don't for Symbian: it uses the EScummVM.aif file for the icon.
	setupIcon();
#endif

	// Invoke parent implementation of this method
	OSystem::initBackend();

	_inited = true;
}

OSystem_SDL::OSystem_SDL()
	:
	_inited(false) {
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
	deinit();
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

Common::String OSystem_SDL::getDefaultConfigFileName() {
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
	Common::String cap;
	byte c;

	// The string caption is supposed to be in LATIN-1 encoding.
	// SDL expects UTF-8. So we perform the conversion here.
	while ((c = *(const byte *)caption++)) {
		if (c < 0x80)
			cap += c;
		else {
			cap += 0xC0 | (c >> 6);
			cap += 0x80 | (c & 0x3F);
		}
	}

	SDL_WM_SetCaption(cap.c_str(), cap.c_str());
}

void OSystem_SDL::deinit() {
	SDL_ShowCursor(SDL_ENABLE);

	delete _eventManager;
	_eventManager = 0;
	delete _savefileManager;
	_savefileManager = 0;
	delete _graphicsManager;
	_graphicsManager = 0;
	delete _audiocdManager;
	_audiocdManager = 0;
	delete _mixer;
	_mixer = 0;
	delete _timerManager;
	_timerManager = 0;
	delete _mutexManager;
	_mutexManager = 0;

	SDL_Quit();
}

void OSystem_SDL::quit() {
	deinit();

	exit(0);
}

void OSystem_SDL::setupIcon() {
	int x, y, w, h, ncols, nbytes, i;
	unsigned int rgba[256];
	unsigned int *icon;

	sscanf(scummvm_icon[0], "%d %d %d %d", &w, &h, &ncols, &nbytes);
	if ((w > 512) || (h > 512) || (ncols > 255) || (nbytes > 1)) {
		warning("Could not load the built-in icon (%d %d %d %d)", w, h, ncols, nbytes);
		return;
	}
	icon = (unsigned int*)malloc(w*h*sizeof(unsigned int));
	if (!icon) {
		warning("Could not allocate temp storage for the built-in icon");
		return;
	}

	for (i = 0; i < ncols; i++) {
		unsigned char code;
		char color[32];
		unsigned int col;
		sscanf(scummvm_icon[1 + i], "%c c %s", &code, color);
		if (!strcmp(color, "None"))
			col = 0x00000000;
		else if (!strcmp(color, "black"))
			col = 0xFF000000;
		else if (color[0] == '#') {
			sscanf(color + 1, "%06x", &col);
			col |= 0xFF000000;
		} else {
			warning("Could not load the built-in icon (%d %s - %s) ", code, color, scummvm_icon[1 + i]);
			free(icon);
			return;
		}

		rgba[code] = col;
	}
	for (y = 0; y < h; y++) {
		const char *line = scummvm_icon[1 + ncols + y];
		for (x = 0; x < w; x++) {
			icon[x + w * y] = rgba[(int)line[x]];
		}
	}

	SDL_Surface *sdl_surf = SDL_CreateRGBSurfaceFrom(icon, w, h, 32, w * 4, 0xFF0000, 0x00FF00, 0x0000FF, 0xFF000000);
	if (!sdl_surf) {
		warning("SDL_CreateRGBSurfaceFrom(icon) failed");
	}
	SDL_WM_SetIcon(sdl_surf, NULL);
	SDL_FreeSurface(sdl_surf);
	free(icon);
}

SdlGraphicsManager *OSystem_SDL::getGraphicsManager() {
	assert(_graphicsManager);
	return (SdlGraphicsManager *)_graphicsManager;
}

bool OSystem_SDL::pollEvent(Common::Event &event) {
	assert(_eventManager);
	return ((SdlEventManager *)_eventManager)->pollSdlEvent(event);
}

uint32 OSystem_SDL::getMillis() {
	uint32 millis = SDL_GetTicks();
	g_eventRec.processMillis(millis);
	return millis;
}

void OSystem_SDL::delayMillis(uint msecs) {
	SDL_Delay(msecs);
}

void OSystem_SDL::getTimeAndDate(TimeDate &td) const {
	time_t curTime = time(0);
	struct tm t = *localtime(&curTime);
	td.tm_sec = t.tm_sec;
	td.tm_min = t.tm_min;
	td.tm_hour = t.tm_hour;
	td.tm_mday = t.tm_mday;
	td.tm_mon = t.tm_mon;
	td.tm_year = t.tm_year;
}
