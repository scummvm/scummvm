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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/punycode.h"
#include "common/tokenizer.h"

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/wincursor.h"

#include "director/director.h"
#include "director/debugger.h"
#include "director/archive.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sound.h"
#include "director/window.h"
#include "director/lingo/lingo.h"
#include "director/detection.h"

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

	_wm = nullptr;

	_gameDataDir = Common::FSNode(ConfMan.get("path"));

	SearchMan.addDirectory(_gameDataDir.getPath(), _gameDataDir, 0, 5);

	for (uint i = 0; Director::directoryGlobs[i]; i++) {
		Common::String directoryGlob = directoryGlobs[i];
		SearchMan.addSubDirectoryMatching(_gameDataDir, directoryGlob, 0, 5);
	}

	if (debugChannelSet(-1, kDebug32bpp))
		_colorDepth = 32;
	else
		_colorDepth = 8;	// 256-color

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

if ( _machineType == 4 )         // setting Width , Height , colorDepth for Macintosh II
{
    _wmWidth = 512;
    _wmHeight = 384;
    _colorDepth = 8;
} 
else if ( _machineType == 71 )    // setting Width , Height , colorDepth for Pippin
{
    _wmWidth = 640;
    _wmHeight = 480;
    _colorDepth = 16;
}
else if ( _machineType == 256 )    // setting Width , Height , colorDepth for Windows
{
    _wmWidth = 640;
    _wmHeight = 480;
    _colorDepth = 8;
} 
else {                           // setting Width , Height , colorDepth for OTHERS
    _wmWidth = 1024;
    _wmHeight = 768;
    _colorDepth = 8;
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

	for (Common::HashMap<Common::String, Archive *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::iterator it = _allOpenResFiles.begin(); it != _allOpenResFiles.end(); ++it) {
		delete it->_value;
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
	result += (getPlatform() == Common::kPlatformWindows) ? "C:\\" : "";
	result += convertPath(currentPath);
	return result;
}

static bool buildbotErrorHandler(const char *msg) { return true; }

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

	memset(_currentPalette, 0, 768);

	//        we run mac-style menus     |   and we will redraw all widgets
	_wmMode = Graphics::kWMModalMenuMode | Graphics::kWMModeManualDrawWidgets;

	if (!debugChannelSet(-1, kDebugDesktop))
		_wmMode |= Graphics::kWMModeFullscreen | Graphics::kWMModeNoDesktop;

	if (debugChannelSet(-1, kDebug32bpp))
		_wmMode |= Graphics::kWMMode32bpp;

	_wm = new Graphics::MacWindowManager(_wmMode, &_director3QuickDrawPatterns, getLanguage());
	_wm->setEngine(this);

	gameQuirks(_gameDescription->desc.gameId, _gameDescription->desc.platform);

	_wm->setDesktopMode(_wmMode);

	_wm->printWMMode();

	_pixelformat = _wm->_pixelformat;

	debug("Director pixelformat is: %s", _pixelformat.toString().c_str());

	_stage = new Window(_wm->getNextId(), false, false, false, _wm, this, true);
	*_stage->_refCount += 1;

	if (!desktopEnabled())
		_stage->disableBorder();

	_surface = new Graphics::ManagedSurface(1, 1);
	_wm->setScreen(_surface);
	_wm->addWindowInitialized(_stage);
	_wm->setActiveWindow(_stage->getId());
	setPalette(-1);

	_currentWindow = _stage;

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

	bool loop = true;

	while (loop) {
		if (_stage->getCurrentMovie())
			processEvents();

		_currentWindow = _stage;
		g_lingo->switchStateFromWindow();
		loop = _currentWindow->step();

		if (loop) {
			FArray *windowList = g_lingo->_windowList.u.farr;
			for (uint i = 0; i < windowList->arr.size(); i++) {
				if (windowList->arr[i].type != OBJECT || windowList->arr[i].u.obj->getObjType() != kWindowObj)
					continue;

				_currentWindow = static_cast<Window *>(windowList->arr[i].u.obj);
				g_lingo->switchStateFromWindow();
				_currentWindow->step();
			}
		}

		draw();
		g_director->delayMillis(10);
	}

	return Common::kNoError;
}

Common::CodePage DirectorEngine::getPlatformEncoding() {
	// Returns the default encoding for the platform we're pretending to be.
	// (English Mac OS, Japanese Mac OS, English Windows, etc.)
	return getEncoding(getPlatform(), getLanguage());
}

Common::String DirectorEngine::getEXEName() const {
	StartMovie startMovie = getStartMovie();
	if (startMovie.startMovie.size() > 0)
		return startMovie.startMovie;

	return Common::punycode_decodefilename(Common::lastPathComponent(_gameDescription->desc.filesDescriptions[0].fileName, '/'));
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

Common::String DirectorEngine::getStartupPath() const {
	return _options.startupPath;
}

bool DirectorEngine::desktopEnabled() {
	return !(_wmMode & Graphics::kWMModeNoDesktop);
}

} // End of namespace Director
