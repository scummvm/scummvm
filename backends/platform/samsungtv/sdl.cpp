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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/platform/sdl/sdl.cpp $
 * $Id: sdl.cpp 44793 2009-10-08 19:41:38Z fingolfin $
 *
 */

#include "backends/platform/samsungtv/sdl.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/EventRecorder.h"
#include "common/util.h"

#include "backends/saves/posix/posix-saves.h"
#include "backends/timer/default/default-timer.h"
#include "sound/mixer_intern.h"

#include <time.h>	// for getTimeAndDate()

/*
 * Include header files needed for the getFilesystemFactory() method.
 */
#include "backends/fs/posix/posix-fs-factory.h"

#if defined(SAMSUNGTV)

#define DEFAULT_CONFIG_FILE "/dtv/usb/sda1/.scummvmrc"

static Uint32 timer_handler(Uint32 interval, void *param) {
	((DefaultTimerManager *)param)->handler();
	return interval;
}

static const size_t AR_COUNT = 4;
static const char*       desiredAspectRatioAsStrings[AR_COUNT] = {            "auto",            "4/3",            "16/9",            "16/10" };
static const AspectRatio desiredAspectRatios[AR_COUNT]         = { AspectRatio(0, 0), AspectRatio(4,3), AspectRatio(16,9), AspectRatio(16,10) };
static AspectRatio getDesiredAspectRatio() {
	//TODO : We could parse an arbitrary string, if we code enough proper validation
	Common::String desiredAspectRatio = ConfMan.get("desired_screen_aspect_ratio");

	for (size_t i = 0; i < AR_COUNT; i++) {
		assert(desiredAspectRatioAsStrings[i] != NULL);

		if (!scumm_stricmp(desiredAspectRatio.c_str(), desiredAspectRatioAsStrings[i])) {
			return desiredAspectRatios[i];
		}
	}
	// TODO : Report a warning
	return AspectRatio(0, 0);
}

void OSystem_SDL_SamsungTV::initBackend() {
	assert(!_inited);

	uint32 sdlFlags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;

	if (ConfMan.hasKey("disable_sdl_parachute"))
		sdlFlags |= SDL_INIT_NOPARACHUTE;

	if (SDL_Init(sdlFlags) == -1) {
		error("Could not initialize SDL: %s", SDL_GetError());
	}

	_graphicsMutex = createMutex();

	// Enable unicode support if possible
	SDL_EnableUNICODE(1);

	memset(&_oldVideoMode, 0, sizeof(_oldVideoMode));
	memset(&_videoMode, 0, sizeof(_videoMode));
	memset(&_transactionDetails, 0, sizeof(_transactionDetails));

	_cksumValid = false;
	_videoMode.mode = GFX_2XSAI;
	_videoMode.scaleFactor = 2;
	_videoMode.aspectRatioCorrection = ConfMan.getBool("aspect_ratio");
	_videoMode.desiredAspectRatio = getDesiredAspectRatio();
	_scalerProc = _2xSaI;
	_scalerType = 0;
	_modeFlags = 0;
	_videoMode.fullscreen = true;


	// Create the savefile manager, if none exists yet (we check for this to
	// allow subclasses to provide their own).
	if (_savefile == NULL) {
	    _savefile = new POSIXSaveFileManager();
	}

	// Create and hook up the mixer, if none exists yet (we check for this to
	// allow subclasses to provide their own).
	if (_mixer == NULL) {
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

	// Invoke parent implementation of this method
	OSystem::initBackend();

	_inited = true;
}

OSystem_SDL_SamsungTV::OSystem_SDL_SamsungTV() : OSystem_SDL(),
	_prehwscreen(0) {
}

void OSystem_SDL_SamsungTV::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	// Add the global DATA_PATH to the directory search list
	// FIXME: We use depth = 4 for now, to match the old code. May want to change that
	Common::FSNode dataNode(".");
	if (dataNode.exists() && dataNode.isDirectory()) {
		s.add(".", new Common::FSDirectory(dataNode, 4), priority);
	}
}


static Common::String getDefaultConfigFileName() {
	char configFile[MAXPATHLEN];
	strcpy(configFile, DEFAULT_CONFIG_FILE);
	return configFile;
}

Common::SeekableReadStream *OSystem_SDL_SamsungTV::createConfigReadStream() {
	Common::FSNode file(getDefaultConfigFileName());
	return file.createReadStream();
}

Common::WriteStream *OSystem_SDL_SamsungTV::createConfigWriteStream() {
	Common::FSNode file(getDefaultConfigFileName());
	return file.createWriteStream();
}

bool OSystem_SDL_SamsungTV::hasFeature(Feature f) {
	return
		(f == kFeatureAutoComputeDirtyRects) ||
		(f == kFeatureCursorHasPalette);
}

void OSystem_SDL_SamsungTV::setFeatureState(Feature f, bool enable) {
	switch (f) {
	case kFeatureAspectRatioCorrection:
		setAspectRatioCorrection(enable);
		break;
	case kFeatureAutoComputeDirtyRects:
		if (enable)
			_modeFlags |= DF_WANT_RECT_OPTIM;
		else
			_modeFlags &= ~DF_WANT_RECT_OPTIM;
		break;
	default:
		break;
	}
}

bool OSystem_SDL_SamsungTV::getFeatureState(Feature f) {
	assert (_transactionMode == kTransactionNone);

	switch (f) {
	case kFeatureAspectRatioCorrection:
		return _videoMode.aspectRatioCorrection;
	case kFeatureAutoComputeDirtyRects:
		return _modeFlags & DF_WANT_RECT_OPTIM;
	default:
		return false;
	}
}

void OSystem_SDL_SamsungTV::quit() {
	unloadGFXMode();
	deleteMutex(_graphicsMutex);

	SDL_RemoveTimer(_timerID);
	closeMixer();

	free(_dirtyChecksums);
	free(_currentPalette);
	free(_cursorPalette);
	free(_mouseData);

	delete _timer;

	SDL_Quit();

	// Even Manager requires save manager for storing
	// recorded events
	delete getEventManager();
	delete _savefile;
}

#endif
