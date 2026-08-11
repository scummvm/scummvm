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

#ifndef ULTIMA_SHARED_EARLY_ULTIMA_EARLY_H
#define ULTIMA_SHARED_EARLY_ULTIMA_EARLY_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/endian.h"
#include "common/hash-str.h"
#include "common/serializer.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/util.h"
#include "graphics/surface.h"
#include "engines/engine.h"
#include "ultima/detection.h"

#include "ultima/shared/engine/events.h"

namespace Ultima {

struct UltimaGameDescription;

namespace Shared {

struct UltimaSavegameHeader {
	uint8 _version;
	uint8 _gameId;
	uint8 _language;
	uint8 _videoMode;
	Common::String _saveName;
	Graphics::Surface *_thumbnail;
	int _year, _month, _day;
	int _hour, _minute;
	int _totalFrames;
};

class Events;
class Game;
class GameBase;
class MouseCursor;
class Resources;

namespace Gfx {
class Screen;
}

class UltimaEarlyEngine : public Engine, public EventsCallback {
private:
	/**
	 * Initialize the engine
	 */
	virtual bool initialize();

	/**
	 * Deinitialize the engine
	 */
	virtual void deinitialize() {}

private:
	Common::RandomSource _randomSource;
protected:
	const UltimaGameDescription *_gameDescription;
public:
	GameBase *_game;
	MouseCursor *_mouseCursor;
	Gfx::Screen *_screen;
	EventsManager *_events;
public:
	UltimaEarlyEngine(OSystem *syst, const UltimaGameDescription *gameDesc);
	~UltimaEarlyEngine() override;

	/**
	 * Main method for running the game
	 */
	Common::Error run() override;

	/**
	 * Returns supported engine features
	 */
	bool hasFeature(EngineFeature f) const override;

	/**
	 * Play the game
	 */
	void playGame();

	/**
	 * Get a random number
	 */
	uint getRandomNumber(uint maxVal) { return _randomSource.getRandomNumber(maxVal); }

	/**
	 * Gets a random number
	 */
	uint getRandomNumber(uint min, uint max) {
		return min + _randomSource.getRandomNumber(max - min);
	}

	/**
	 * Get the screen
	 */
	Graphics::Screen *getScreen() const override;

	/**
	 * Indicates whether a game state can be loaded.
	 * @param isAutosave	Flags whether it's an autosave check
	 */
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;

	/**
	 * Indicates whether a game state can be saved.
	 * @param isAutosave	Flags whether it's an autosave check
	 */
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;

	/**
	 * Load a savegame
	 */
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;

	/**
	 * Save a game state.
	 */
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;

	/**
	 * Returns the game type being played
	 */
	GameId getGameId() const;

	/**
	 * Returns true if the game is running an enhanced version
	 * as compared to the original game
	 */
	bool isEnhanced() const;

	/*
	 * Creates a new hierarchy for the game, that contains all the logic for playing that particular game.
	 */
	Game *createGame() const;
};

} // End of namespace Shared

extern Shared::UltimaEarlyEngine *g_vm;

} // End of namespace Ultima

#endif
