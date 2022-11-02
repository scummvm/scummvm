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

namespace Common {
class RandomSource;
class Serializer;
}

/**
 * This is the namespace of the Nancy engine.
 *
 * Status of this engine:
 * Nancy Drew: Secrets can kill is completable,
 * with a few minor graphical glitches;
 * The Vampire Diaries boots, but crashes almost immediately;
 * every other game is untested but definitely unplayable
 *
 * Games using this engine:
 *  - The Vampire Diaries (1996)
 *  - Almost every mainline Nancy Drew game by HeR Interactive,
 * 	up to and including Nancy Drew: Sea of Darkness (2015)
 */
namespace Nancy {

static const int kSavegameVersion = 1;

struct NancyGameDescription;

class ResourceManager;
class IFF;
class InputManager;
class SoundManager;
class GraphicsManager;
class CursorManager;
class CheatDialog;
class NancyConsole;
struct GameConstants;

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

	const char *getCopyrightString() const;
	uint32 getGameFlags() const;
	const char *getGameId() const;
	GameType getGameType() const;
	Common::Platform getPlatform() const;

	const GameConstants &getConstants() const;

	void setState(NancyState::NancyState state, NancyState::NancyState overridePrevious = NancyState::kNone);
	NancyState::NancyState getState() { return _gameFlow.curState; }
	void setToPreviousState();

	// Chunks found in BOOT get extracted and cached at startup, this function lets other classes access them
	Common::SeekableReadStream *getBootChunkStream(const Common::String &name) const;

	void setMouseEnabled(bool enabled);

	void callCheatMenu(bool eventFlags);

	// Managers
	ResourceManager *_resource;
	GraphicsManager *_graphicsManager;
	CursorManager *_cursorManager;
	InputManager *_input;
	SoundManager *_sound;

	Common::RandomSource *_randomSource;

	// BSUM data
	SceneChangeDescription _firstScene;
	
	uint16 _startTimeHours;
	uint16 _startTimeMinutes;

	bool _overrideMovementTimeDeltas;
	Time _slowMovementTimeDelta;
	Time _fastMovementTimeDelta;
	Time _playerTimeMinuteLength;

	uint _horizontalEdgesSize;
	uint _verticalEdgesSize;

	Common::Rect _textboxScreenPosition;

private:
	struct GameFlow {
		NancyState::NancyState curState = NancyState::kNone;
		NancyState::NancyState prevState = NancyState::kNone;
	};

	Common::Error run() override;

	void bootGameEngine();

	State::State *getStateObject(NancyState::NancyState state) const;

	bool addBootChunk(const Common::String &name, Common::SeekableReadStream *stream);
	void clearBootChunks();

	void preloadCals(const IFF &boot);
	void readChunkList(const IFF &boot, Common::Serializer &ser, const Common::String &prefix);

	void readBootSummary(const IFF &boot);

	Common::Error synchronize(Common::Serializer &serializer);

	bool isCompressed();

	bool _cheatTypeIsEventFlag;

	GameFlow _gameFlow;
	OSystem *_system;

	const NancyGameDescription *_gameDescription;

	Common::HashMap<Common::String, Common::SeekableReadStream *> _bootChunks;
};

extern NancyEngine *g_nancy;

} // End of namespace Nancy

#endif // NANCY_H
