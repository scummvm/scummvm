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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_H
#define NANCY_H

#include "common/file.h"
#include "common/str.h"
#include "common/ptr.h"

#include "engines/engine.h"

#include "nancy/detection.h"
#include "nancy/time.h"
#include "nancy/commontypes.h"
#include "nancy/enginedata.h"

namespace Common {
class RandomSource;
class Serializer;
}

/**
 * This is the namespace of the Nancy engine.
 *
 * Status of this engine:
 * The Vampire Diaries and all Nancy Drew games up to and including
 * Nancy Drew: Danger on Deception Island are fully completable.
 * Every other game is untested but definitely unplayable.
 *
 * Games using this engine:
 *	- The Vampire Diaries (1996)
 *	- Almost every mainline Nancy Drew game by HeR Interactive,
 *		beginning with Nancy Drew: Secrets can Kill (1998)
 *		up to and including Nancy Drew: Sea of Darkness (2015)
 */
namespace Nancy {

// Save game version history:
// - 1: Initial version
// - 2: Conditional dialogue and hints moved to nancy.dat
// - 3: Puzzle data stored as lazily initialized PuzzleData objects
// - 4: Journal entries sync their scene ID (Nancy9+)
// - 5: Nancy10 taskbar notification badges persisted
// - 6: Nancy12 timers reworked
// - 7: Nancy10 unnamed notebook task event flags added
// - 8: Nancy12 DrivingPuzzle fuel state persisted
// - 9: RippedLetterPuzzle stores its scene ID and tried flag
static const int kSavegameVersion = 9;

struct NancyGameDescription;

class ResourceManager;
class IFF;
class InputManager;
class SoundManager;
class GraphicsManager;
class CursorManager;
class NancyConsole;
class DeferredLoader;
class MoviePlayer;

namespace State {
class State;
}

class NancyEngine : public Engine {
public:
	NancyEngine(OSystem *syst, const NancyGameDescription *gd);
	~NancyEngine();

	static NancyEngine *create(GameType type, OSystem *syst, const NancyGameDescription *gd);

	void errorString(const char *buf_input, char *buf_output, int buf_output_size) override;
	bool hasFeature(EngineFeature f) const override;

	Common::Error loadGameState(int slot) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;

	void secondChance();

	const char *getCopyrightString() const;
	uint32 getGameFlags() const;
	const char *getGameId() const;
	GameType getGameType() const;
	Common::Language getGameLanguage() const;
	Common::Platform getPlatform() const;

	const StaticData &getStaticData() const;
	const EngineData *getEngineData(const Common::String &name) const;
	const Common::String getEventFlagName(uint flagID) const;

	// Nancy15+ lets the player alternate between several protagonists, each of whom
	// carries their own copy of the popup UI. Swaps the engine data describing it to
	// the given PCUI character's, and returns whether anything actually changed.
	// Scene::reloadPlayerCharacterUI() rebuilds the widgets themselves.
	bool setPlayerCharacter(uint characterIndex);
	uint getPlayerCharacter() const { return _playerCharacter; }

	// A character's "design" is the look their UI wears; it names the CIF tree
	// everything is loaded from. Nancy's can be changed on the Design Select
	// screen, and defaults to the character's PCUI entry.
	Common::String getPlayerCharacterDesign(uint characterIndex) const;
	void setPlayerCharacterDesign(uint characterIndex, const Common::String &designName);

	// Whether setPlayerCharacter() would actually have to load anything
	bool playerCharacterNeedsReload(uint characterIndex) const;

	void setState(NancyState::NancyState state, NancyState::NancyState overridePrevious = NancyState::kNone);
	NancyState::NancyState getState() { return _gameFlow.curState; }
	NancyState::NancyState getPreviousState() const { return _gameFlow.prevState; }
	void setToPreviousState();

	void setMouseEnabled(bool enabled);

	void addDeferredLoader(Common::SharedPtr<DeferredLoader> &loaderPtr);

	// The first few games used 1/2 for false/true in
	// inventory, logic conditions, and event flags
	const byte _true;
	const byte _false;

	// Managers
	ResourceManager *_resource;
	GraphicsManager *_graphics;
	CursorManager *_cursor;
	InputManager *_input;
	SoundManager *_sound;

	Common::RandomSource *_randomSource;

	// All loaded movies, for MoviePlayer::findLoadedMovie().
	Common::Array<MoviePlayer *> _loadedMovies;

	// Used to check whether we need to show the SaveDialog
	bool _hasJustSaved;

protected:
	Common::Error run() override;
	void pauseEngineIntern(bool pause) override;

private:
	struct GameFlow {
		NancyState::NancyState curState = NancyState::kNone;
		NancyState::NancyState prevState = NancyState::kNone;
		NancyState::NancyState nextState = NancyState::kNone;
		bool changingState = true;
	};

	void bootGameEngine();

	State::State *getStateObject(NancyState::NancyState state) const;
	void destroyState(NancyState::NancyState state) const;

	void preloadCals();
	void readDatFile();
	// Nancy12 onwards no longer ship their static data in nancy.dat; the values
	// the engine still needs are provided here instead (see also the EVNT chunk).
	void populateStaticData();

	Common::Error synchronize(Common::Serializer &serializer);

	bool isCompressed();

	StaticData _staticData;
	Common::HashMap<Common::String, EngineData *> _engineData;

	// Nancy15+ active player character, the CIF tree their UI came from, and
	// each character's chosen design (empty = their PCUI default)
	uint _playerCharacter = 0;
	Common::String _playerCharacterTree;
	Common::String _playerCharacterDesigns[kMaxPlayerCharacters];

	const byte _datFileMajorVersion;
	const byte _datFileMinorVersion;

	GameFlow _gameFlow;
	OSystem *_system;

	const NancyGameDescription *_gameDescription;

	Common::Array<Common::WeakPtr<DeferredLoader>> _deferredLoaderObjects;
};

extern NancyEngine *g_nancy;
#define GetEngineData(s) (const s*)g_nancy->getEngineData(#s);

} // End of namespace Nancy

#endif // NANCY_H
