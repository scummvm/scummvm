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

#include "audio/mixer.h"

#include "common/macresman.h"

#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/score.h"
#include "director/sound.h"
#include "director/lingo/lingo.h"
#include "director/util.h"

namespace Director {

DirectorEngine *g_director;

DirectorEngine::DirectorEngine(OSystem *syst, const DirectorGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc),
		_rnd("director") {
	DebugMan.addDebugChannel(kDebugLingoExec, "lingoexec", "Lingo Execution");
	DebugMan.addDebugChannel(kDebugLingoCompile, "lingocompile", "Lingo Compilation");
	DebugMan.addDebugChannel(kDebugLingoParse, "lingoparse", "Lingo code parsing");
	DebugMan.addDebugChannel(kDebugLingoCompileOnly, "compileonly", "Skip Lingo code execution");
	DebugMan.addDebugChannel(kDebugLoading, "loading", "Loading");
	DebugMan.addDebugChannel(kDebugImages, "images", "Image drawing");
	DebugMan.addDebugChannel(kDebugText, "text", "Text rendering");
	DebugMan.addDebugChannel(kDebugEvents, "events", "Event processing");
	DebugMan.addDebugChannel(kDebugSlow, "slow", "Slow playback");
	DebugMan.addDebugChannel(kDebugFast, "fast", "Fast (no delay) playback");
	DebugMan.addDebugChannel(kDebugNoLoop, "noloop", "Do not loop the playback");
	DebugMan.addDebugChannel(kDebugBytecode, "bytecode", "Execute Lscr bytecode");

	g_director = this;

	// Setup mixer
	syncSoundSettings();

	// Load Palettes
	loadPalettes();

	// Load Patterns
	loadPatterns();

	_currentScore = nullptr;
	_soundManager = nullptr;
	_currentPalette = nullptr;
	_currentPaletteLength = 0;
	_lingo = nullptr;

	_sharedScore = nullptr;

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

	_wm = new Graphics::MacWindowManager(Graphics::kWMModalMenuMode);

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
	} else if (getGameID() == GID_TESTALL) {
		enqueueAllMovies();
	}

	// FIXME
	//_mainArchive = new RIFFArchive();
	//_mainArchive->openFile("bookshelf_example.mmm");

	_currentScore = new Score(this);
	_currentPath = getPath(getEXEName(), _currentPath);

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

	loadSharedCastsFrom(_currentPath + _sharedCastFile);

	debug(0, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\nObtaining score name\n");

	if (getGameID() == GID_TESTALL)  {
		_nextMovie = getNextMovieFromQueue();
		loadInitialMovie(_nextMovie.movie);
	} else {
		loadInitialMovie(getEXEName());

		// Let's check if it is a projector file
		// So far tested with Spaceship Warlock, D2
		if (_mainArchive->hasResource(MKTAG('B', 'N', 'D', 'L'), "Projector")) {
			warning("Detected Projector file");

			if (_mainArchive->hasResource(MKTAG('S', 'T', 'R', '#'), 0)) {
				_currentScore->setArchive(_mainArchive);

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

				delete _currentScore;
				_currentScore = nullptr;

				delete name;
			}
		}
	}

	if (_currentScore)
		_currentScore->setArchive(_mainArchive);

	bool loop = true;

	while (loop) {
		loop = false;

		if (_currentScore) {
			debug(0, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
			debug(0, "@@@@   Score name '%s' in '%s'", _currentScore->getMacName().c_str(), _currentPath.c_str());
			debug(0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

			_currentScore->loadArchive();
		}

		// If we came in a loop, then skip as requested
		if (!_nextMovie.frameS.empty()) {
			_currentScore->setStartToLabel(_nextMovie.frameS);
			_nextMovie.frameS.clear();
		}

		if (_nextMovie.frameI != -1) {
			_currentScore->setCurrentFrame(_nextMovie.frameI);
			_nextMovie.frameI = -1;
		}

		if (!debugChannelSet(-1, kDebugLingoCompileOnly) && _currentScore) {
			debugC(1, kDebugEvents, "Starting playback of score '%s'", _currentScore->getMacName().c_str());

			_currentScore->startLoop();

			debugC(1, kDebugEvents, "Finished playback of score '%s'", _currentScore->getMacName().c_str());
		}

		if (getGameID() == GID_TESTALL) {
			_nextMovie = getNextMovieFromQueue();
		}

		// If a loop was requested, do it
		if (!_nextMovie.movie.empty()) {
			_lingo->restartLingo();

			delete _currentScore;
			_currentScore = nullptr;

			_currentPath = getPath(_nextMovie.movie, _currentPath);

			loadSharedCastsFrom(_currentPath + _sharedCastFile);

			Archive *mov = openMainArchive(_currentPath + Common::lastPathComponent(_nextMovie.movie, '/'));

			if (!mov) {
				warning("nextMovie: No score is loaded");

				if (getGameID() == GID_TESTALL) {
					loop = true;
					continue;
				}

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

void DirectorEngine::enqueueAllMovies() {
	Common::FSNode dir(ConfMan.get("path"));
	Common::FSList files;
	dir.getChildren(files, Common::FSNode::kListFilesOnly);

	for (Common::FSList::const_iterator file = files.begin(); file != files.end(); ++file)
		_movieQueue.push_back((*file).getName());

	Common::sort(_movieQueue.begin(), _movieQueue.end());

	debug(1, "=========> Enqueued %d movies", _movieQueue.size());
}

MovieReference DirectorEngine::getNextMovieFromQueue() {
	MovieReference res;

	if (_movieQueue.empty())
		return res;

	res.movie = _movieQueue.front();

	debug(0, "=======================================");
	debug(0, "=========> Next movie is %s", res.movie.c_str());
	debug(0, "=======================================");

	_movieQueue.remove_at(0);

	return res;
}

} // End of namespace Director
