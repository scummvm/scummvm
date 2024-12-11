/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "backends/modular-backend.h"
#include "backends/graphics/graphics.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/tokenizer.h"

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/wincursor.h"

#include "director/director.h"
#include "director/debugger.h"
#include "director/archive.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/picture.h"
#include "director/score.h"
#include "director/sound.h"
#include "director/window.h"
#include "director/debugger/debugtools.h"

/**
 * When detection is compiled dynamically, directory globs end up in detection plugin and
 * engine cannot link to them so duplicate them in the engine in this case
 */
#ifndef DETECTION_STATIC
#include "director/detection_paths.h"
#endif

namespace Director {

DirectorEngine *g_director;

DirectorEngine::DirectorEngine(OSystem *syst, const DirectorGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	g_director = this;
	g_debugger = new Debugger();
	setDebugger(g_debugger);

	// parseOptions depends on the _dirSeparator
	_version = getDescriptionVersion();
	if (getPlatform() == Common::kPlatformWindows && _version >= 400) {
		_dirSeparator = '\\';
	} else {
		_dirSeparator = ':';
	}

	parseOptions();

	// Setup mixer
	syncSoundSettings();
	_defaultVolume = _mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType);

	// Load Palettes
	loadDefaultPalettes();

	// Load Patterns
	loadPatterns();

	// Load key codes
	loadKeyCodes();

	memset(_currentPalette, 0, 768);
	_currentPaletteLength = 0;
	_stage = nullptr;
	_windowList = new Datum;
	_windowList->type = ARRAY;
	_windowList->u.farr = new FArray;
	_currentWindow = nullptr;
	_cursorWindow = nullptr;
	_lingo = nullptr;
	_clipBoard = nullptr;
	_fixStageSize = false;
	_fixStageRect = Common::Rect();
	_wmMode = 0;

	_wmWidth = 1024;
	_wmHeight = 768;

	_fpsLimit = 0;
	_forceDate.tm_sec = -1;
	_forceDate.tm_min = -1;
	_forceDate.tm_hour = -1;
	_forceDate.tm_mday = -1;
	_forceDate.tm_mon = -1;
	_forceDate.tm_year = -1;
	_forceDate.tm_wday = -1;
	_loadSlowdownFactor = 0;
	_loadSlowdownCooldownTime = 0;

	_wm = nullptr;

	_gameDataDir = Common::FSNode(ConfMan.getPath("path"));

	SearchMan.addDirectory(_gameDataDir, 0, 5);

	for (uint i = 0; Director::directoryGlobs[i]; i++) {
		Common::String directoryGlob = directoryGlobs[i];
		SearchMan.addSubDirectoryMatching(_gameDataDir, directoryGlob, 0, 5);
	}

	if (debugChannelSet(-1, kDebug32bpp) || (getGameFlags() & GF_32BPP)) {
#ifdef USE_RGB_COLOR
		_colorDepth = 32;
#else
		warning("32-bpp color dept is not supported, forcing 8-bit");
		_colorDepth = 8;
#endif
	} else {
		_colorDepth = 8;	// 256-color
	}

	switch (getPlatform()) {
	case Common::kPlatformMacintoshII:
		_machineType = 4;
		break;
	case Common::kPlatformPippin:
		_machineType = 71;
		break;
	case Common::kPlatformWindows:
		_machineType = 256;
		break;
	case Common::kPlatformMacintosh:
	default:
		_machineType = 9;	// Macintosh IIci
	}

	_playbackPaused = false;
	_skipFrameAdvance = false;
	_centerStage = true;

	_surface = nullptr;
	_tickBaseline = 0;
}

DirectorEngine::~DirectorEngine() {
	delete _windowList;
	delete _lingo;
	delete _wm;
	delete _surface;

	for (auto &it : _allSeenResFiles) {
		delete it._value;
	}

	for (uint i = 0; i < _winCursor.size(); i++)
		delete _winCursor[i];

	clearPalettes();
}

Archive *DirectorEngine::getMainArchive() const { return _currentWindow->getMainArchive(); }
Movie *DirectorEngine::getCurrentMovie() const { return _currentWindow->getCurrentMovie(); }
Common::String DirectorEngine::getCurrentPath() const { return _currentWindow->getCurrentPath(); }
Common::String DirectorEngine::getCurrentAbsolutePath() {
	Common::String currentPath = getCurrentPath();
	Common::String result;
	result += (getPlatform() == Common::kPlatformWindows && _version >= 400) ? "C:\\" : "";
	result += convertPath(currentPath);
	return result;
}

static bool buildbotErrorHandler(const char *msg) { return true; }

void DirectorEngine::setCurrentWindow(Window *window) {
	if (_currentWindow == window)
		return;
	if (_currentWindow)
		_currentWindow->decRefCount();
	_currentWindow = window;
	_currentWindow->incRefCount();
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

	memset(_currentPalette, 0, 768);

	//        we run mac-style menus     |   and we will redraw all widgets
	_wmMode = Graphics::kWMModalMenuMode | Graphics::kWMModeManualDrawWidgets;

	if (!debugChannelSet(-1, kDebugDesktop))
		_wmMode |= Graphics::kWMModeFullscreen | Graphics::kWMModeNoDesktop;

#ifdef USE_RGB_COLOR
	if (debugChannelSet(-1, kDebug32bpp) || (getGameFlags() & GF_32BPP))
		_wmMode |= Graphics::kWMMode32bpp;
#endif

	if (getGameFlags() & GF_DESKTOP)
		_wmMode &= ~Graphics::kWMModeNoDesktop;

	if (getGameFlags() & GF_640x480) {
		_wmWidth = 640;
		_wmHeight = 480;
	}

	_wm = new Graphics::MacWindowManager(_wmMode, &_director3QuickDrawPatterns, getLanguage());
	_wm->setEngine(this);

	gameQuirks(_gameDescription->desc.gameId, _gameDescription->desc.platform);

	_wm->setDesktopMode(_wmMode);

	_wm->printWMMode();

	_pixelformat = _wm->_pixelformat;

	debug("Director pixelformat is: %s", _pixelformat.toString().c_str());

	_stage = new Window(_wm->getNextId(), false, false, false, _wm, this, true);
	*_stage->_refCount += 1;

	// Set this as background so it doesn't come to foreground when multiple windows present
	_wm->setBackgroundWindow(_stage);

	if (!desktopEnabled())
		_stage->disableBorder();

	_surface = new Graphics::ManagedSurface(1, 1);
	_wm->setScreen(_surface);
	_wm->addWindowInitialized(_stage);
	_wm->setActiveWindow(_stage->getId());
	setPalette(CastMemberID(kClutSystemMac, -1));

	setCurrentWindow(_stage);

	_lingo = new Lingo(this);
	_lingo->switchStateFromWindow();

	if (getGameGID() == GID_TEST) {
		_currentWindow->runTests();
		return Common::kNoError;
	} else if (getGameGID() == GID_TESTALL) {
		_currentWindow->enqueueAllMovies();
	}

	if (getPlatform() == Common::kPlatformWindows)
		_machineType = 256; // IBM PC-type machine

	Common::Error err = _currentWindow->loadInitialMovie();

	// Exit gracefully when run with buildbot
	if (debugChannelSet(-1, kDebugFewFramesOnly) && err.getCode() == Common::kNoGameDataFoundError)
		return Common::kNoError;

	if (err.getCode() != Common::kNoError)
		return err;

	if (debugChannelSet(-1, kDebugConsole)) {
		g_debugger->attach();
		g_system->updateScreen();
	}

#ifdef USE_IMGUI
	ImGuiCallbacks callbacks;
	bool drawImGui = debugChannelSet(-1, kDebugImGui);
	callbacks.init = DT::onImGuiInit;
	callbacks.render = drawImGui ? DT::onImGuiRender : nullptr;
	callbacks.cleanup = DT::onImGuiCleanup;
	_system->setImGuiCallbacks(callbacks);
#endif

	bool loop = true;

	while (loop) {
		if (_stage->getCurrentMovie())
			processEvents();

		setCurrentWindow(_stage);
		g_lingo->switchStateFromWindow();
		loop = _currentWindow->step();

		if (loop) {
			FArray *windowList = g_lingo->_windowList.u.farr;
			for (uint i = 0; i < windowList->arr.size(); i++) {
				if (windowList->arr[i].type != OBJECT || windowList->arr[i].u.obj->getObjType() != kWindowObj)
					continue;

				setCurrentWindow(static_cast<Window *>(windowList->arr[i].u.obj));
				g_lingo->switchStateFromWindow();
				_currentWindow->step();
			}
		}

		draw();
		g_director->delayMillis(10);
#ifdef USE_IMGUI
		// For performance reasons, disable the renderer callback if the ImGui debug flag isn't set
		if (debugChannelSet(-1, kDebugImGui) != drawImGui) {
			drawImGui = !drawImGui;
			callbacks.render = drawImGui ? DT::onImGuiRender : nullptr;
			_system->setImGuiCallbacks(callbacks);
		}
#endif
	}

#ifdef USE_IMGUI
	_system->setImGuiCallbacks(ImGuiCallbacks());
#endif

	return Common::kNoError;
}

Common::CodePage DirectorEngine::getPlatformEncoding() {
	// Returns the default encoding for the platform we're pretending to be.
	// (English Mac OS, Japanese Mac OS, English Windows, etc.)
	return getEncoding(getPlatform(), getLanguage());
}

Common::String DirectorEngine::getRawEXEName() const {
	if (!_gameDescription->desc.filesDescriptions[0].fileName)
		return Common::String();

	// Returns raw executable name (without getting overloaded from --start-movie option)
	return Common::Path(_gameDescription->desc.filesDescriptions[0].fileName).toString(g_director->_dirSeparator);
}

Common::String DirectorEngine::getEXEName() const {
	StartMovie startMovie = getStartMovie();
	if (startMovie.startMovie.size() > 0)
		return startMovie.startMovie;

	return getRawEXEName();
}

void DirectorEngine::parseOptions() {
	_options.startMovie.startFrame = -1;

	if (!ConfMan.hasKey("start_movie"))
		return;

	Common::StringTokenizer tok(ConfMan.get("start_movie"), ",");

	while (!tok.empty()) {
		Common::String part = tok.nextToken();

		int eqPos = part.findLastOf("=");
		Common::String key;
		Common::String value;

		if ((uint)eqPos != Common::String::npos) {
			key = part.substr(0, eqPos);
			value = part.substr(eqPos + 1, part.size());
		} else {
			value = part;
		}

		if (key == "movie" || key.empty()) { // Format is movie[@startFrame]
			if (!_options.startMovie.startMovie.empty()) {
				warning("parseOptions(): Duplicate startup movie: %s", value.c_str());
			}

			int atPos = value.findLastOf("@");

			if ((uint)atPos == Common::String::npos) {
				_options.startMovie.startMovie = value;
			} else {
				_options.startMovie.startMovie = value.substr(0, atPos);
				Common::String tail = value.substr(atPos + 1, value.size());
				if (tail.size() > 0)
					_options.startMovie.startFrame = atoi(tail.c_str());
			}

			_options.startMovie.startMovie = Common::Path(_options.startMovie.startMovie).punycodeDecode().toString(_dirSeparator);

			debug(2, "parseOptions(): Movie is: %s, frame is: %d", _options.startMovie.startMovie.c_str(), _options.startMovie.startFrame);
		} else if (key == "startup") {
			_options.startupPath = value;

			debug(2, "parseOptions(): Startup is: %s", value.c_str());
		} else {
			warning("parseOptions(): unknown option %s", part.c_str());
		}

	}
}

StartMovie DirectorEngine::getStartMovie() const {
	return _options.startMovie;
}

Common::Path DirectorEngine::getStartupPath() const {
	return Common::Path(_options.startupPath, g_director->_dirSeparator);
}

bool DirectorEngine::desktopEnabled() {
	return !(_wmMode & Graphics::kWMModeNoDesktop);
}

PatternTile::~PatternTile() {
	delete img;
}

} // End of namespace Director
