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
#include "common/substream.h"

#include "common/macresman.h"

#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sound.h"
#include "director/stage.h"
#include "director/util.h"
#include "director/lingo/lingo.h"

namespace Director {

const uint32 wmMode = Graphics::kWMModalMenuMode | Graphics::kWMModeNoDesktop
	| Graphics::kWMModeManualDrawWidgets | Graphics::kWMModeFullscreen;

DirectorEngine *g_director;

DirectorEngine::DirectorEngine(OSystem *syst, const DirectorGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc),
		_rnd("director") {
	DebugMan.addDebugChannel(kDebugLingoExec, "lingoexec", "Lingo Execution");
	DebugMan.addDebugChannel(kDebugCompile, "compile", "Lingo Compilation");
	DebugMan.addDebugChannel(kDebugParse, "parse", "Lingo code parsing");
	DebugMan.addDebugChannel(kDebugCompileOnly, "compileonly", "Skip Lingo code execution");
	DebugMan.addDebugChannel(kDebugLoading, "loading", "Loading");
	DebugMan.addDebugChannel(kDebugImages, "images", "Image drawing");
	DebugMan.addDebugChannel(kDebugText, "text", "Text rendering");
	DebugMan.addDebugChannel(kDebugEvents, "events", "Event processing");
	DebugMan.addDebugChannel(kDebugSlow, "slow", "Slow playback");
	DebugMan.addDebugChannel(kDebugFast, "fast", "Fast (no delay) playback");
	DebugMan.addDebugChannel(kDebugNoLoop, "noloop", "Do not loop the playback");
	DebugMan.addDebugChannel(kDebugBytecode, "bytecode", "Execute Lscr bytecode");
	DebugMan.addDebugChannel(kDebugFewFramesOnly, "fewframesonly", "Only run the first 10 frames");
	DebugMan.addDebugChannel(kDebugPreprocess, "preprocess", "Lingo preprocessing");
	DebugMan.addDebugChannel(kDebugScreenshot, "screenshot", "screenshot each frame");

	g_director = this;

	// Setup mixer
	syncSoundSettings();

	// Load Palettes
	loadPalettes();

	// Load Patterns
	loadPatterns();

	// Load key codes
	loadKeyCodes();

	_currentMovie = nullptr;
	_soundManager = nullptr;
	_currentPalette = nullptr;
	_currentPaletteLength = 0;
	_lingo = nullptr;

	_mainArchive = nullptr;
	_macBinary = nullptr;

	_movies = nullptr;

	_nextMovie.frameI = -1;

	_wm = nullptr;

	const Common::FSNode gameDataDir(ConfMan.get("path"));

	// Meet Mediaband could have up to 5 levels of directories
	SearchMan.addDirectory(gameDataDir.getPath(), gameDataDir, 0, 5);

	SearchMan.addSubDirectoryMatching(gameDataDir, "data");
	SearchMan.addSubDirectoryMatching(gameDataDir, "install");
	SearchMan.addSubDirectoryMatching(gameDataDir, "main");		// Meet Mediaband

	_colorDepth = 8;	// 256-color
	_key = 0;
	_keyCode = 0;
	_machineType = 9; // Macintosh IIci
	_playbackPaused = false;
	_skipFrameAdvance = false;

	_draggingSprite = false;
	_draggingSpriteId = 0;

	_newMovieStarted = true;
}

DirectorEngine::~DirectorEngine() {
	delete _currentMovie;

	_wm->removeWindow(_currentStage);

	if (_macBinary) {
		delete _macBinary;
		_macBinary = nullptr;
	}

	delete _soundManager;
	delete _lingo;
	delete _wm;
}

Common::Error DirectorEngine::run() {
	debug("Starting v%d Director game", getVersion());

	if (!_mixer->isReady()) {
		return Common::kAudioDeviceInitFailed;
	}

	_currentPalette = nullptr;

	_macBinary = nullptr;
	_soundManager = nullptr;

	_wm = new Graphics::MacWindowManager(wmMode, &_director3QuickDrawPatterns);
	_wm->setEngine(this);

	_currentStage = new Stage(_wm->getNextId(), false, false, false, _wm);
	_currentStage->disableBorder();
	_wm->addWindowInitialized(_currentStage);
	_wm->setScreen(_currentStage->getSurface());

	_lingo = new Lingo(this);
	_soundManager = new DirectorSound(this);

	if (getGameGID() == GID_TEST) {
		runTests();
		return Common::kNoError;
	} else if (getGameGID() == GID_TESTALL) {
		enqueueAllMovies();
	}

	// FIXME
	//_mainArchive = new RIFFArchive();
	//_mainArchive->openFile("bookshelf_example.mmm");

	if (getPlatform() == Common::kPlatformWindows)
		_machineType = 256; // IBM PC-type machine

	if (getVersion() < 4) {
		if (getPlatform() == Common::kPlatformWindows) {
			_sharedCastFile = "SHARDCST.MMM";
		} else {
			_sharedCastFile = "Shared Cast";
		}
	} else if (getVersion() == 5) {
		if (getPlatform() == Common::kPlatformWindows) {
			_sharedCastFile = "SHARED.Cxt";
		}
	} else {
		_sharedCastFile = "Shared.dir";
	}

	debug(0, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\nObtaining score name\n");

	if (getGameGID() == GID_TESTALL)  {
		_nextMovie = getNextMovieFromQueue();
		loadInitialMovie(_nextMovie.movie);
	} else {
		loadInitialMovie(getEXEName());

		if (!_mainArchive) {
			warning("Cannot open main movie");
			return Common::kNoGameDataFoundError;
		}

		// Let's check if it is a projector file
		// So far tested with Spaceship Warlock, D2
		if (_mainArchive->hasResource(MKTAG('B', 'N', 'D', 'L'), "Projector")) {
			warning("Detected Projector file");

			if (_mainArchive->hasResource(MKTAG('X', 'C', 'O', 'D'), -1)) {
				Common::Array<uint16> xcod = _mainArchive->getResourceIDList(MKTAG('X', 'C', 'O', 'D'));
				for (Common::Array<uint16>::iterator iterator = xcod.begin(); iterator != xcod.end(); ++iterator) {
					Resource res = _mainArchive->getResourceDetail(MKTAG('X', 'C', 'O', 'D'), *iterator);
					debug(0, "Detected XObject '%s'", res.name.c_str());
					g_lingo->openXLib(res.name, kXObj);
				}
			}

			if (_mainArchive->hasResource(MKTAG('S', 'T', 'R', '#'), 0)) {
				_currentMovie->setArchive(_mainArchive);

				Common::SeekableSubReadStreamEndian *name = _mainArchive->getResource(MKTAG('S', 'T', 'R', '#'), 0);
				int num = name->readUint16();
				if (num != 1) {
					warning("Incorrect number of strings in Projector file");
				}

				if (num == 0)
					error("No strings in Projector file");

				Common::String sname = name->readPascalString();

				_nextMovie.movie = pathMakeRelative(sname);
				warning("Replaced score name with: %s (from %s)", _nextMovie.movie.c_str(), sname.c_str());

				delete _currentMovie;
				_currentMovie = nullptr;

				delete name;
			}
		}
	}

	if (_currentMovie)
		_currentMovie->setArchive(_mainArchive);

	bool loop = true;

	while (loop) {
		loop = false;

		if (_currentMovie) {
			debug(0, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
			debug(0, "@@@@   Movie name '%s' in '%s'", _currentMovie->getMacName().c_str(), _currentPath.c_str());
			debug(0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

			bool goodMovie = _currentMovie->loadArchive();

			// If we came in a loop, then skip as requested
			if (!_nextMovie.frameS.empty()) {
				_currentMovie->getScore()->setStartToLabel(_nextMovie.frameS);
				_nextMovie.frameS.clear();
			}

			if (_nextMovie.frameI != -1) {
				_currentMovie->getScore()->setCurrentFrame(_nextMovie.frameI);
				_nextMovie.frameI = -1;
			}

			if (!debugChannelSet(-1, kDebugCompileOnly) && goodMovie) {
				debugC(1, kDebugEvents, "Starting playback of movie '%s'", _currentMovie->getMacName().c_str());

				_currentMovie->getScore()->startLoop();

				debugC(1, kDebugEvents, "Finished playback of movie '%s'", _currentMovie->getMacName().c_str());
			}
		}

		if (getGameGID() == GID_TESTALL) {
			_nextMovie = getNextMovieFromQueue();
		}

		// If a loop was requested, do it
		if (!_nextMovie.movie.empty()) {
			_newMovieStarted = true;

			_currentPath = getPath(_nextMovie.movie, _currentPath);

			Cast *sharedCast = nullptr;
			if (_currentMovie) {
				sharedCast = _currentMovie->getSharedCast();
				_currentMovie->_sharedCast = nullptr;
			}

			delete _currentMovie;
			_currentMovie = nullptr;

			Archive *mov = openMainArchive(_currentPath + Common::lastPathComponent(_nextMovie.movie, '/'));

			if (!mov) {
				warning("nextMovie: No movie is loaded");

				if (getGameGID() == GID_TESTALL) {
					loop = true;
					continue;
				}

				return Common::kNoError;
			}

			_currentMovie = new Movie(this);
			_currentMovie->setArchive(mov);
			debug(0, "Switching to movie '%s'", _currentMovie->getMacName().c_str());

			_lingo->resetLingo();
			if (sharedCast && sharedCast->_castArchive
					&& sharedCast->_castArchive->getFileName().equalsIgnoreCase(_currentPath + _sharedCastFile)) {
				_currentMovie->_sharedCast = sharedCast;
			} else {
				delete sharedCast;
				_currentMovie->loadSharedCastsFrom(_currentPath + _sharedCastFile);
			}

			_nextMovie.movie.clear();
			loop = true;
		}
	}

	return Common::kNoError;
}

} // End of namespace Director
