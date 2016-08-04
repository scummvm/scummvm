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
 */

#ifndef DIRECTOR_DIRECTOR_H
#define DIRECTOR_DIRECTOR_H

#include "common/scummsys.h"
#include "common/random.h"
#include "common/substream.h"

#include "common/str.h"
#include "common/hashmap.h"
#include "engines/engine.h"
#include "engines/director/sound.h"

namespace Common {
class MacResManager;
}

namespace Graphics {
class MacWindowManager;
}

namespace Director {

enum DirectorGameID {
	GID_GENERIC,
	GID_TEST
};

class Archive;
struct DirectorGameDescription;
class Lingo;
class Score;
struct Cast;

class DirectorEngine : public ::Engine {
public:
	DirectorEngine(OSystem *syst, const DirectorGameDescription *gameDesc);
	~DirectorEngine();

	// Detection related functions

	DirectorGameID getGameID() const;
	uint16 getVersion() const;
	Common::Platform getPlatform() const;
	Common::Language getLanguage() const;
	Common::String getEXEName() const;
	DirectorSound *getSoundManager() const { return _soundManager; }
	Archive *getMainArchive() const { return _mainArchive; }
	Lingo *getLingo() const { return _lingo; }
	Score *getCurrentScore() const { return _currentScore; }
	void setPalette(byte *palette, uint16 count);
	bool hasFeature(EngineFeature f) const;
	const byte *getPalette() const { return _currentPalette; }
	uint16 getPaletteColorCount() const { return _currentPaletteLength; }
	void loadSharedCastsFrom(Common::String filename);
	Common::HashMap<int, Common::SeekableSubReadStreamEndian *> *getSharedDIB() const { return _sharedDIB; }
	Common::HashMap<int, Common::SeekableSubReadStreamEndian *> *getSharedBMP() const { return _sharedBMP; }
	Common::HashMap<int, Common::SeekableSubReadStreamEndian *> *getSharedSTXT() const { return _sharedSTXT; }
	Common::HashMap<int, Cast *> *getSharedCasts() const { return _sharedCasts; }

	Common::HashMap<Common::String, Score *> *_movies;
	Score *_currentScore;

	Common::RandomSource _rnd;
	Graphics::MacWindowManager *_wm;

protected:
	virtual Common::Error run();

private:
	const DirectorGameDescription *_gameDescription;

	Common::HashMap<Common::String, Score *> loadMMMNames(Common::String folder);
	void loadEXE();
	void loadEXEv3(Common::SeekableReadStream *stream);
	void loadEXEv4(Common::SeekableReadStream *stream);
	void loadEXEv5(Common::SeekableReadStream *stream);
	void loadEXEv7(Common::SeekableReadStream *stream);
	void loadEXERIFX(Common::SeekableReadStream *stream, uint32 offset);
	void loadMac();

	Common::String readPascalString(Common::SeekableReadStream &stream);

	Common::String _sharedMMM;
	Common::HashMap<int, Cast *> *_sharedCasts;
	Common::HashMap<int, Common::SeekableSubReadStreamEndian *> *_sharedDIB;
	Common::HashMap<int, Common::SeekableSubReadStreamEndian *> *_sharedSTXT;
	Common::HashMap<int, Common::SeekableSubReadStreamEndian *> *_sharedSound;
	Common::HashMap<int, Common::SeekableSubReadStreamEndian *> *_sharedBMP;

	Archive *_mainArchive;
	Common::MacResManager *_macBinary;
	DirectorSound *_soundManager;
	byte *_currentPalette;
	uint16 _currentPaletteLength;
	Lingo *_lingo;
};

} // End of namespace Director

#endif
