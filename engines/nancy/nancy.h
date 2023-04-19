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
 * The Vampire Diaries and Nancy Drew: Secrets can Kill are completable.
 * Every other game is untested but definitely unplayable
 *
 * Games using this engine:
 *	- The Vampire Diaries (1996)
 *	- Almost every mainline Nancy Drew game by HeR Interactive,
 *		beginnning with Nancy Drew: Secrets can Kill (1998)
 *		up to and including Nancy Drew: Sea of Darkness (2015)
 */
namespace Nancy {

static const int kSavegameVersion = 2;

struct NancyGameDescription;

class ResourceManager;
class IFF;
class InputManager;
class SoundManager;
class GraphicsManager;
class CursorManager;
class NancyConsole;

namespace State {
class State;
}

class NancyEngine : public Engine {
public:
	friend class NancyConsole;

	NancyEngine(OSystem *syst, const NancyGameDescription *gd);
	~NancyEngine();

	static NancyEngine *create(GameType type, OSystem *syst, const NancyGameDescription *gd);

	bool hasFeature(EngineFeature f) const override;

	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;
	bool canSaveAutosaveCurrently() override;

	void secondChance();

	const char *getCopyrightString() const;
	uint32 getGameFlags() const;
	const char *getGameId() const;
	GameType getGameType() const;
	Common::Platform getPlatform() const;

	const StaticData &getStaticData() const;

	void setState(NancyState::NancyState state, NancyState::NancyState overridePrevious = NancyState::kNone);
	NancyState::NancyState getState() { return _gameFlow.curState; }
	void setToPreviousState();

	void setMouseEnabled(bool enabled);

	// Managers
	ResourceManager *_resource;
	GraphicsManager *_graphicsManager;
	CursorManager *_cursorManager;
	InputManager *_input;
	SoundManager *_sound;

	Common::RandomSource *_randomSource;

	// BOOT chunks data
	BSUM *_bootSummary;
	VIEW *_viewportData;
	INV *_inventoryData;
	TBOX *_textboxData;
	MAP *_mapData;
	HELP *_helpData;
	CRED *_creditsData;
	HINT *_hintData;
	SPUZ *_sliderPuzzleData;
	CLOK *_clockData;
	SPEC *_specialEffectData;

	Common::HashMap<Common::String, ImageChunk> _imageChunks;

protected:
	Common::Error run() override;
	void pauseEngineIntern(bool pause) override;

private:
	struct GameFlow {
		NancyState::NancyState curState = NancyState::kNone;
		NancyState::NancyState prevState = NancyState::kNone;
		bool changingState = true;
	};

	void bootGameEngine();

	State::State *getStateObject(NancyState::NancyState state) const;
	void destroyState(NancyState::NancyState state) const;

	void preloadCals(const IFF &boot);

	void readDatFile();

	Common::Error synchronize(Common::Serializer &serializer);

	bool isCompressed();

	StaticData _staticData;
	const byte _datFileMajorVersion;
	const byte _datFileMinorVersion;

	GameFlow _gameFlow;
	OSystem *_system;

	const NancyGameDescription *_gameDescription;
};

extern NancyEngine *g_nancy;

} // End of namespace Nancy

#endif // NANCY_H
