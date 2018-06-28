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

#include "common/macresman.h"
#include "graphics/fonts/macfont.h"

#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/sound.h"
#include "director/lingo/lingo.h"

namespace Director {

DirectorEngine *g_director;

DirectorEngine::DirectorEngine(OSystem *syst, const DirectorGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc),
		_rnd("director") {
	DebugMan.addDebugChannel(kDebugLingoExec, "lingoexec", "Lingo Execution");
	DebugMan.addDebugChannel(kDebugLingoCompile, "lingocompile", "Lingo Compilation");
	DebugMan.addDebugChannel(kDebugLingoParse, "lingoparse", "Lingo code parsing");
	DebugMan.addDebugChannel(kDebugLoading, "loading", "Loading");
	DebugMan.addDebugChannel(kDebugImages, "images", "Image drawing");
	DebugMan.addDebugChannel(kDebugText, "text", "Text rendering");
	DebugMan.addDebugChannel(kDebugEvents, "events", "Event processing");

	g_director = this;

	// Setup mixer
	syncSoundSettings();

	// Load Patterns
	loadPatterns();

	_sharedScore = nullptr;

	_currentScore = nullptr;
	_soundManager = nullptr;
	_currentPalette = nullptr;
	_currentPaletteLength = 0;
	_lingo = nullptr;

	_sharedScore = nullptr;
	_sharedSound = nullptr;
	_sharedBMP = nullptr;
	_sharedSTXT = nullptr;
	_sharedDIB = nullptr;

	_mainArchive = nullptr;
	_macBinary = nullptr;

	_movies = nullptr;

	_nextMovie.frameI = -1;

	_wm = nullptr;

	const Common::FSNode gameDataDir(ConfMan.get("path"));
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
}

DirectorEngine::~DirectorEngine() {
	delete _sharedSound;
	delete _sharedBMP;
	delete _sharedSTXT;
	delete _sharedDIB;
	delete _sharedScore;

	delete _currentScore;

	cleanupMainArchive();

	delete _soundManager;
	delete _lingo;
}

Common::Error DirectorEngine::run() {
	debug("Starting v%d Director game", getVersion());

	if (!_mixer->isReady()) {
		return Common::kAudioDeviceInitFailed;
	}

	_currentPalette = nullptr;

	_macBinary = nullptr;
	_soundManager = nullptr;

	_wm = new Graphics::MacWindowManager;

	_lingo = new Lingo(this);
	_soundManager = new DirectorSound();

	if (getGameID() == GID_TEST) {
		_mainArchive = nullptr;
		_currentScore = nullptr;

		if (debugChannelSet(-1, kDebugText)) {
			testFontScaling();
			testFonts();
		}

		_lingo->runTests();

		return Common::kNoError;
	}

	// FIXME
	//_mainArchive = new RIFFArchive();
	//_mainArchive->openFile("bookshelf_example.mmm");

	_currentScore = new Score(this);

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

	loadSharedCastsFrom(_sharedCastFile);

	loadInitialMovie(getEXEName());

	_currentScore->setArchive(_mainArchive);
	debug(0, "Score name %s", _currentScore->getMacName().c_str());

	bool loop = true;

	while (loop) {
		loop = false;

		_currentScore->loadArchive();

		// If we came in a loop, then skip as requested
		if (!_nextMovie.frameS.empty()) {
			_currentScore->setStartToLabel(_nextMovie.frameS);
			_nextMovie.frameS.clear();
		}

		if (_nextMovie.frameI != -1) {
			_currentScore->setCurrentFrame(_nextMovie.frameI);
			_nextMovie.frameI = -1;
		}

		debugC(1, kDebugEvents, "Starting playback of score '%s'", _currentScore->getMacName().c_str());

		_currentScore->startLoop();

		debugC(1, kDebugEvents, "Finished playback of score '%s'", _currentScore->getMacName().c_str());

		// If a loop was requested, do it
		if (!_nextMovie.movie.empty()) {
			_lingo->restartLingo();

			delete _currentScore;

			Archive *mov = openMainArchive(_nextMovie.movie);

			if (!mov) {
				warning("nextMovie: No score is loaded");

				return Common::kNoError;
			}

			_currentScore = new Score(this);
			_currentScore->setArchive(mov);
			debug(0, "Switching to score '%s'", _currentScore->getMacName().c_str());

			_nextMovie.movie.clear();
			loop = true;
		}
	}

	return Common::kNoError;
}

Common::HashMap<Common::String, Score *> *DirectorEngine::scanMovies(const Common::String &folder) {
	Common::FSNode directory(folder);
	Common::FSList movies;
	const char *sharedMMMname;

	if (getPlatform() == Common::kPlatformWindows)
		sharedMMMname = "SHARDCST.MMM";
	else
		sharedMMMname = "Shared Cast";


	Common::HashMap<Common::String, Score *> *nameMap = new Common::HashMap<Common::String, Score *>();
	if (!directory.getChildren(movies, Common::FSNode::kListFilesOnly))
		return nameMap;

	if (!movies.empty()) {
		for (Common::FSList::const_iterator i = movies.begin(); i != movies.end(); ++i) {
			debugC(2, kDebugLoading, "File: %s", i->getName().c_str());

			if (Common::matchString(i->getName().c_str(), sharedMMMname, true)) {
				_sharedCastFile = i->getName();

				debugC(2, kDebugLoading, "Shared cast detected: %s", i->getName().c_str());
				continue;
			}

			Archive *arc = createArchive();

			warning("name: %s", i->getName().c_str());
			arc->openFile(i->getName());
			Score *sc = new Score(this);
			sc->setArchive(arc);
			nameMap->setVal(sc->getMacName(), sc);

			debugC(2, kDebugLoading, "Movie name: \"%s\"", sc->getMacName().c_str());
		}
	}

	return nameMap;
}

Common::HashMap<int, CastType> *DirectorEngine::getSharedCastTypes() {
	if (_sharedScore)
		return &_sharedScore->_castTypes;

	return &_dummyCastType;
}

} // End of namespace Director
