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

#ifndef GOT_H
#define GOT_H

#include "common/error.h"
#include "common/random.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/util.h"
#include "engines/engine.h"
#include "got/events.h"
#include "got/vars.h"

namespace Got {

struct GotGameDescription;

class GotEngine : public Engine, public Events {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;

	void savegameLoaded();

protected:
	// Engine APIs
	Common::Error run() override;

	/**
     * Returns true if the game should quit
     */
	bool shouldQuit() const override {
		return Engine::shouldQuit();
	}

public:
	Vars _vars;

public:
	GotEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~GotEngine() override;

	uint32 getFeatures() const;
	bool isDemo() const override;

	/**
     * Returns the game Id
     */
	Common::String getGameId() const;

	/**
     * Gets a random number
     */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return (f == kSupportsLoadingDuringRuntime) ||
			   (f == kSupportsSavingDuringRuntime) ||
			   (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;

	/**
     * Uses a serializer to allow implementing savegame
     * loading and saving using a single method
     */
	Common::Error syncGame(Common::Serializer &s);

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;

	void syncSoundSettings() override;
	void pauseEngineIntern(bool pause) override;

	Common::String getHighScoresSaveName() const;
};

extern GotEngine *g_engine;
#define SHOULD_QUIT ::Got::g_engine->shouldQuit()

} // End of namespace Got

#endif // GOT_H
