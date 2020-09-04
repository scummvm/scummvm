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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"

#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sound.h"
#include "director/window.h"
#include "director/lingo/lingo.h"

namespace Director {

const uint32 wmModeDesktop = Graphics::kWMModalMenuMode | Graphics::kWMModeManualDrawWidgets;
const uint32 wmModeFullscreen = Graphics::kWMModalMenuMode | Graphics::kWMModeNoDesktop
	| Graphics::kWMModeManualDrawWidgets | Graphics::kWMModeFullscreen;
uint32 wmMode = 0;

DirectorEngine *g_director;

DirectorEngine::DirectorEngine(OSystem *syst, const DirectorGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	DebugMan.addDebugChannel(kDebug32bpp, "32bpp", "Work in 32bpp mode");
	DebugMan.addDebugChannel(kDebugCompile, "compile", "Lingo Compilation");
	DebugMan.addDebugChannel(kDebugCompileOnly, "compileonly", "Skip Lingo code execution");
	DebugMan.addDebugChannel(kDebugDesktop, "desktop", "Show the Classic Mac desktop");
	DebugMan.addDebugChannel(kDebugEndVideo, "endvideo", "Fake that the end of video is reached setting");
	DebugMan.addDebugChannel(kDebugEvents, "events", "Event processing");
	DebugMan.addDebugChannel(kDebugFast, "fast", "Fast (no delay) playback");
	DebugMan.addDebugChannel(kDebugFewFramesOnly, "fewframesonly", "Only run the first 10 frames");
	DebugMan.addDebugChannel(kDebugImages, "images", "Image drawing");
	DebugMan.addDebugChannel(kDebugLingoExec, "lingoexec", "Lingo Execution");
	DebugMan.addDebugChannel(kDebugLoading, "loading", "Loading");
	DebugMan.addDebugChannel(kDebugNoBytecode, "nobytecode", "Do not execute Lscr bytecode");
	DebugMan.addDebugChannel(kDebugNoLoop, "noloop", "Do not loop the playback");
	DebugMan.addDebugChannel(kDebugParse, "parse", "Lingo code parsing");
	DebugMan.addDebugChannel(kDebugPreprocess, "preprocess", "Lingo preprocessing");
	DebugMan.addDebugChannel(kDebugScreenshot, "screenshot", "screenshot each frame");
	DebugMan.addDebugChannel(kDebugSlow, "slow", "Slow playback");
	DebugMan.addDebugChannel(kDebugText, "text", "Text rendering");

	g_director = this;

	// Setup mixer
	syncSoundSettings();

	// Load Palettes
	loadDefaultPalettes();

	// Load Patterns
	loadPatterns();

	// Load key codes
	loadKeyCodes();

	_soundManager = nullptr;
	_currentPalette = nullptr;
	_currentPaletteLength = 0;
	_stage = nullptr;
	_windowList = new Datum;
	_windowList->type = ARRAY;
	_windowList->u.farr = new DatumArray;
	_currentWindow = nullptr;
	_lingo = nullptr;
	_version = getDescriptionVersion();

	_wm = nullptr;

	_gameDataDir = Common::FSNode(ConfMan.get("path"));

	// Meet Mediaband could have up to 5 levels of directories
	SearchMan.addDirectory(_gameDataDir.getPath(), _gameDataDir, 0, 5);

	SearchMan.addSubDirectoryMatching(_gameDataDir, "data");
	SearchMan.addSubDirectoryMatching(_gameDataDir, "install");
	SearchMan.addSubDirectoryMatching(_gameDataDir, "main");		// Meet Mediaband
	SearchMan.addSubDirectoryMatching(_gameDataDir, "l_zone");
	SearchMan.addSubDirectoryMatching(_gameDataDir, "win_data", 0, 2);	// L-ZONE

	_colorDepth = 8;	// 256-color
	_machineType = 9; // Macintosh IIci
	_playbackPaused = false;
	_skipFrameAdvance = false;
	_centerStage = true;
}

DirectorEngine::~DirectorEngine() {
	delete _windowList;
	delete _soundManager;
	delete _lingo;
	delete _wm;

	for (Common::HashMap<Common::String, Archive *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::iterator it = _openResFiles.begin(); it != _openResFiles.end(); ++it) {
		delete it->_value;
	}

	clearPalettes();
}

Archive *DirectorEngine::getMainArchive() const { return _currentWindow->getMainArchive(); }
Movie *DirectorEngine::getCurrentMovie() const { return _currentWindow->getCurrentMovie(); }
Common::String DirectorEngine::getCurrentPath() const { return _currentWindow->getCurrentPath(); }

static void buildbotErrorHandler(const char *msg) { }

void DirectorEngine::setCurrentMovie(Movie *movie) {
	_currentWindow = movie->getWindow();
}

void DirectorEngine::setVersion(uint16 version) {
	if (version == _version)
		return;

	debug("Switching to Director v%d", version);
	_version = version;
	_lingo->reloadBuiltIns();
}

Common::Error DirectorEngine::run() {
	debug("Starting v%d Director game", getVersion());

	// We want to avoid GUI errors for buildbot, because they hang it
	if (debugChannelSet(-1, kDebugFewFramesOnly))
		Common::setErrorHandler(buildbotErrorHandler);

	if (!_mixer->isReady()) {
		return Common::kAudioDeviceInitFailed;
	}

	_currentPalette = nullptr;

	_soundManager = nullptr;

	wmMode = debugChannelSet(-1, kDebugDesktop) ? wmModeDesktop : wmModeFullscreen;

	if (debugChannelSet(-1, kDebug32bpp))
		wmMode |= Graphics::kWMMode32bpp;

	_wm = new Graphics::MacWindowManager(wmMode, &_director3QuickDrawPatterns);
	_wm->setEngine(this);

	_pixelformat = _wm->_pixelformat;

	_stage = new Window(_wm->getNextId(), false, false, false, _wm, this, true);
	*_stage->_refCount += 1;

	if (!debugChannelSet(-1, kDebugDesktop))
		_stage->disableBorder();

	_wm->setScreen(1, 1);
	_wm->addWindowInitialized(_stage);
	_wm->setActiveWindow(_stage->getId());
	setPalette(-1);

	_currentWindow = _stage;

	_lingo = new Lingo(this);
	_soundManager = new DirectorSound(this);

	if (getGameGID() == GID_TEST) {
		_currentWindow->runTests();
		return Common::kNoError;
	} else if (getGameGID() == GID_TESTALL) {
		_currentWindow->enqueueAllMovies();
	}

	if (getPlatform() == Common::kPlatformWindows)
		_machineType = 256; // IBM PC-type machine

	if (getVersion() < 400) {
		if (getPlatform() == Common::kPlatformWindows) {
			_sharedCastFile = "SHARDCST.MMM";
		} else {
			_sharedCastFile = "Shared Cast";
		}
	} else if (getVersion() == 500) {
		if (getPlatform() == Common::kPlatformWindows) {
			_sharedCastFile = "SHARED.Cxt";
		}
	} else {
		_sharedCastFile = "Shared.dir";
	}

	Common::Error err = _currentWindow->loadInitialMovie();
	if (err.getCode() != Common::kNoError)
		return err;

	bool loop = true;

	while (loop) {
		if (_stage->getCurrentMovie())
			processEvents();

		_currentWindow = _stage;
		loop = _currentWindow->step();

		if (loop) {
			DatumArray *windowList = g_lingo->_windowList.u.farr;
			for (uint i = 0; i < windowList->size(); i++) {
				if ((*windowList)[i].type != OBJECT || (*windowList)[i].u.obj->getObjType() != kWindowObj)
					continue;

				_currentWindow = static_cast<Window *>((*windowList)[i].u.obj);
				_currentWindow->step();
			}
		}

		draw();
	}

	return Common::kNoError;
}

} // End of namespace Director
