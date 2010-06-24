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
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/util.h"
#include "common/EventRecorder.h"

#include "backends/saves/default/default-saves.h"
#include "backends/audiocd/sdl/sdl-audiocd.h"
#include "backends/events/sdl/sdl-events.h"
#include "backends/mutex/sdl/sdl-mutex.h"
#include "backends/timer/sdl/sdl-timer.h"

#include "icons/scummvm.xpm"

#define DEFAULT_CONFIG_FILE "scummvm.ini"

#include <time.h>

OSystem_SDL::OSystem_SDL()
	:
	_inited(false),
	_initedSDL(false),
	_mixerManager(0) {

}

OSystem_SDL::~OSystem_SDL() {
	deinit();
}

void OSystem_SDL::init() {
	// Initialize SDL
	initSDL();

	// Creates the early needed managers, if they don't exist yet
	// (we check for this to allow subclasses to provide their own).
	if (_mutexManager == 0)
		_mutexManager = new SdlMutexManager();

	if (_timerManager == 0)
		_timerManager = new SdlTimerManager();
}

void OSystem_SDL::initBackend() {
	// Check if backend has not been initialized
	assert(!_inited);

	// Creates the backend managers, if they don't exist yet (we check
	// for this to allow subclasses to provide their own).
	if (_eventManager == 0)
		_eventManager = new SdlEventManager(this);

	if (_savefileManager == 0)
		_savefileManager = new DefaultSaveFileManager();

	if (_mixerManager == 0) {
		_mixerManager = new SdlMixerManager();

		// Setup and start mixer
		_mixerManager->init();
	}

	if (_graphicsManager == 0)
		_graphicsManager = new SdlGraphicsManager();

	if (_audiocdManager == 0)
		_audiocdManager = new SdlAudioCDManager();

	// Setup a custom program icon.
	setupIcon();

	_inited = true;
}

void OSystem_SDL::initSDL() {
	// Check if SDL has not been initialized
	if (!_initedSDL) {
		uint32 sdlFlags = 0;
		if (ConfMan.hasKey("disable_sdl_parachute"))
			sdlFlags |= SDL_INIT_NOPARACHUTE;

		// Initialize SDL (SDL Subsystems are initiliazed in the corresponding sdl managers)
		if (SDL_Init(sdlFlags) == -1)
			error("Could not initialize SDL: %s", SDL_GetError());

		// Enable unicode support if possible
		SDL_EnableUNICODE(1);

		_initedSDL = true;
	}
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

}

Common::String OSystem_SDL::getDefaultConfigFileName() {
	char configFile[MAXPATHLEN];
	strcpy(configFile, DEFAULT_CONFIG_FILE);
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
	delete _mixerManager;
	_mixerManager = 0;
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

Audio::Mixer *OSystem_SDL::getMixer() {
	assert(_mixerManager);
	return _mixerManager->getMixer();
}
