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

#ifndef DIRECTOR_DIRECTOR_H
#define DIRECTOR_DIRECTOR_H

#include "common/random.h"
#include "common/rect.h"
#include "common/str-array.h"

#include "common/hashmap.h"
#include "engines/engine.h"
#include "director/types.h"

namespace Common {
class MacResManager;
class SeekableReadStream;
class SeekableSubReadStreamEndian;
}

namespace Graphics {
class MacWindowManager;
typedef Common::Array<byte *> MacPatterns;
}

namespace Director {

enum DirectorGameID {
	GID_GENERIC,
	GID_TEST,
	GID_TESTALL
};

class Archive;
struct DirectorGameDescription;
class DirectorSound;
class Lingo;
class Score;
class Cast;

enum {
	kDebugLingoExec			= 1 << 0,
	kDebugLingoCompile		= 1 << 1,
	kDebugLoading			= 1 << 2,
	kDebugImages			= 1 << 3,
	kDebugText				= 1 << 4,
	kDebugEvents			= 1 << 5,
	kDebugLingoParse		= 1 << 6,
	kDebugLingoCompileOnly	= 1 << 7,
	kDebugSlow				= 1 << 8,
	kDebugFast				= 1 << 9,
	kDebugNoLoop			= 1 << 10,
	kDebugBytecode			= 1 << 11
};

struct MovieReference {
	Common::String movie;
	Common::String frameS;
	int frameI;

	MovieReference() { frameI = -1; }
};

struct PaletteV4 {
	int id;
	byte *palette;
	int length;
};

class DirectorEngine : public ::Engine {
public:
	DirectorEngine(OSystem *syst, const DirectorGameDescription *gameDesc);
	~DirectorEngine() override;

	// Detection related functions

	DirectorGameID getGameID() const;
	uint16 getVersion() const;
	Common::Platform getPlatform() const;
	Common::Language getLanguage() const;
	Common::String getEXEName() const;
	DirectorSound *getSoundManager() const { return _soundManager; }
	Graphics::MacWindowManager *getMacWindowManager() const { return _wm; }
	Archive *getMainArchive() const { return _mainArchive; }
	Lingo *getLingo() const { return _lingo; }
	Score *getCurrentScore() const { return _currentScore; }
	Score *getSharedScore() const { return _sharedScore; }
	Common::String getCurrentPath() const { return _currentPath; }
	void setPalette(int id);
	void setPalette(byte *palette, uint16 count);
	bool hasFeature(EngineFeature f) const override;
	void loadPalettes();
	const byte *getPalette() const { return _currentPalette; }
	uint16 getPaletteColorCount() const { return _currentPaletteLength; }
	void loadSharedCastsFrom(Common::String filename);
	void clearSharedCast();
	void loadPatterns();
	byte transformColor(byte color);
	Graphics::MacPatterns &getPatterns();

	void loadInitialMovie(const Common::String movie);
	Archive *openMainArchive(const Common::String movie);
	Archive *createArchive();
	void cleanupMainArchive();

	void processEvents(); // events.cpp
	void setDraggedSprite(uint16 id); // events.cpp

	Common::HashMap<Common::String, Score *> *_movies;

	Common::RandomSource _rnd;
	Graphics::MacWindowManager *_wm;

public:
	int _colorDepth;
	unsigned char _key;
	int _keyCode;
	int _machineType;
	bool _playbackPaused;
	bool _skipFrameAdvance;

	MovieReference _nextMovie;
	Common::List<MovieReference> _movieStack;

protected:
	Common::Error run() override;

private:
	const DirectorGameDescription *_gameDescription;

	Common::HashMap<Common::String, Score *> *scanMovies(const Common::String &folder);
	void loadEXE(const Common::String movie);
	void loadEXEv3(Common::SeekableReadStream *stream);
	void loadEXEv4(Common::SeekableReadStream *stream);
	void loadEXEv5(Common::SeekableReadStream *stream);
	void loadEXEv7(Common::SeekableReadStream *stream);
	void loadEXERIFX(Common::SeekableReadStream *stream, uint32 offset);
	void loadMac(const Common::String movie);

	Score *_sharedScore;

	Archive *_mainArchive;
	Common::MacResManager *_macBinary;
	DirectorSound *_soundManager;
	byte *_currentPalette;
	uint16 _currentPaletteLength;
	Lingo *_lingo;

	Score *_currentScore;
	Common::String _currentPath;

	Graphics::MacPatterns _director3Patterns;
	Graphics::MacPatterns _director3QuickDrawPatterns;

	Common::HashMap<int, PaletteV4 *> _director4Palettes;

	Common::String _sharedCastFile;

	bool _draggingSprite;
	uint16 _draggingSpriteId;
	Common::Point _draggingSpritePos;

	Common::StringArray _movieQueue;

private:
	void testFontScaling();
	void testFonts();

	void enqueueAllMovies();
	MovieReference getNextMovieFromQueue();
};

extern DirectorEngine *g_director;

} // End of namespace Director

#endif
