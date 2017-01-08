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
#include "director/sound.h"
#include "director/lingo/lingo.h"

namespace Director {

DirectorEngine *g_director;

DirectorEngine::DirectorEngine(OSystem *syst, const DirectorGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc),
		_rnd("director") {
	DebugMan.addDebugChannel(kDebugLingoExec, "lingoexec", "Lingo Execution");
	DebugMan.addDebugChannel(kDebugLingoCompile, "lingocompile", "Lingo Compilation");
	DebugMan.addDebugChannel(kDebugLoading, "loading", "Loading");
	DebugMan.addDebugChannel(kDebugImages, "images", "Image drawing");
	DebugMan.addDebugChannel(kDebugText, "text", "Text rendering");

	g_director = this;

	if (!_mixer->isReady())
		error("Sound initialization failed");

	// Setup mixer
	syncSoundSettings();

	_sharedCasts = nullptr;

	_currentScore = nullptr;
	_soundManager = nullptr;
	_currentPalette = nullptr;
	_currentPaletteLength = 0;
	_lingo = nullptr;

	_sharedCasts = nullptr;
	_sharedSound = nullptr;
	_sharedBMP = nullptr;
	_sharedSTXT = nullptr;
	_sharedDIB = nullptr;

	_mainArchive = nullptr;
	_macBinary = nullptr;

	_movies = nullptr;

	_wm = nullptr;

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "data");
	SearchMan.addSubDirectoryMatching(gameDataDir, "install");

	_colorDepth = 8;	// 256-color
	_key = 0;
	_keyCode = 0;
	_machineType = 9; // Macintosh IIci
}

DirectorEngine::~DirectorEngine() {
	delete _sharedSound;
	delete _sharedBMP;
	delete _sharedSTXT;
	delete _sharedDIB;

	delete _currentScore;

	cleanupMainArchive();

	delete _soundManager;
	delete _lingo;
}

Common::Error DirectorEngine::run() {
	debug("Starting v%d Director game", getVersion());

	_currentPalette = nullptr;

	_macBinary = nullptr;
	_soundManager = nullptr;

	_wm = new Graphics::MacWindowManager;

	_lingo = new Lingo(this);
	_soundManager = new DirectorSound();

	if (getGameID() == GID_TEST) {
		_mainArchive = nullptr;
		_currentScore = nullptr;

		_lingo->runTests();

		return Common::kNoError;
	}

	//FIXME
	//_mainArchive = new RIFFArchive();
	//_mainArchive->openFile("bookshelf_example.mmm");

	//testFont();

	if (getPlatform() == Common::kPlatformWindows)
		_sharedCastFile = "SHARDCST.MMM";
	else
		_sharedCastFile = "Shared Cast*";

	loadSharedCastsFrom(_sharedCastFile);

	loadMainArchive();

	_currentScore = new Score(this, _mainArchive);
	debug(0, "Score name %s", _currentScore->getMacName().c_str());

	_currentScore->loadArchive();
	_currentScore->startLoop();

	return Common::kNoError;
}

Common::HashMap<Common::String, Score *> *DirectorEngine::scanMovies(const Common::String &folder) {
	Common::FSNode directory(folder);
	Common::FSList movies;
	const char *sharedMMMname;

	if (getPlatform() == Common::kPlatformWindows)
		sharedMMMname = "SHARDCST.MMM";
	else
		sharedMMMname = "Shared Cast*";


	Common::HashMap<Common::String, Score *> *nameMap = new Common::HashMap<Common::String, Score *>();
	if (!directory.getChildren(movies, Common::FSNode::kListFilesOnly))
		return nameMap;

	if (!movies.empty()) {
		for (Common::FSList::const_iterator i = movies.begin(); i != movies.end(); ++i) {
			debugC(2, kDebugLoading, "File: %s", i->getName().c_str());

			if (Common::matchString(i->getName().c_str(), sharedMMMname, true)) {
				_sharedCastFile = i->getName();
				continue;
			}

			Archive *arc = createArchive();

			warning("name: %s", i->getName().c_str());
			arc->openFile(i->getName());
			Score *sc = new Score(this, arc);
			nameMap->setVal(sc->getMacName(), sc);

			debugC(2, kDebugLoading, "Movie name: \"%s\"", sc->getMacName().c_str());
		}
	}

	return nameMap;
}

Common::String DirectorEngine::readPascalString(Common::SeekableReadStream &stream) {
	byte length = stream.readByte();
	Common::String x;

	while (length--)
		x += (char)stream.readByte();

	return x;
}

void DirectorEngine::setPalette(byte *palette, uint16 count) {
	_currentPalette = palette;
	_currentPaletteLength = count;
}

} // End of namespace Director
