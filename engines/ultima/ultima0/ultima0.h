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

#ifndef ULTIMA0_H
#define ULTIMA0_H

#include "common/random.h"
#include "common/serializer.h"
#include "graphics/palette.h"
#include "engines/engine.h"
#include "ultima/detection.h"
#include "ultima/ultima0/data/data.h"
#include "ultima/ultima0/events.h"
#include "ultima/ultima0/music.h"

namespace Ultima {
namespace Ultima0 {

class Ultima0Engine : public Engine, public Events {
private:
	Common::RandomSource _randomSource;
	//const UltimaGameDescription *_gameDescription;

	void syncSavegame(Common::Serializer &s);

protected:
	// Engine APIs
	Common::Error run() override;

public:
	Graphics::Palette _palette;
	PlayerInfo _player;
	WorldMapInfo _worldMap;
	DungeonMapInfo _dungeon;
	bool _showMinimap = false;
	MusicPlayer *_music = nullptr;

	Ultima0Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc);
	~Ultima0Engine() override;

	/**
	 * Returns true if the game should quit
	 */
	bool shouldQuit() const override {
		return Engine::shouldQuit();
	}

	/**
	 * Returns supported engine features
	 */
	bool hasFeature(EngineFeature f) const override;

	/**
	 * Sets the random number seed
	 */
	void setRandomSeed(uint seed) {
		_randomSource.setSeed(seed);
	}

	/**
	 * Get a random number
	 */
	uint getRandomNumber(uint maxVal = RND_MAX) { return _randomSource.getRandomNumber(maxVal); }
	uint getRandomNumber(uint minVal, uint maxVal) {
		return _randomSource.getRandomNumber(maxVal - minVal) + minVal;
	}

	/**
	 * Returns true if enhancements are turned on
	 */
	bool isEnhanced() const {
		return true;
	}

	/**
	 * Returns true if a savegame can be loaded
	 */
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}

	/**
	 * Returns true if the game can be saved
	 */
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;

	/**
	 * Load a game state.
	 * @param stream	the stream to load the savestate from
	 * @return returns kNoError on success, else an error code.
	 */
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;

	/**
	 * Save a game state.
	 * @param stream	The write stream to save the savegame data to
	 * @param isAutosave	Expected to be true if an autosave is being created
	 * @return returns kNoError on success, else an error code.
	 */
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;

	/**
	 * Returns true if any savegames exist
	 */
	bool savegamesExist() const;

	void playMidi(const char *name);
	void stopMidi();
	bool isMidiPlaying() const {
		return _music != nullptr;
	}
};

extern Ultima0Engine *g_engine;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
