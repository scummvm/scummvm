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

#ifndef ULTIMA2_H
#define ULTIMA2_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/ptr.h"
#include "common/serializer.h"
#include "common/util.h"
#include "engines/engine.h"
#include "ultima/detection.h"
#include "ultima/shared/engine/events.h"
#include "ultima/ultima2/data/map.h"
#include "ultima/ultima2/data/map_dungeon.h"
#include "ultima/ultima2/data/savegame.h"
#include "ultima/ultima2/logic/logic.h"

namespace Audio {
class PCSpeaker;
}

namespace Ultima {
namespace Ultima2 {

struct Ultima2GameDescription;

class Ultima2Engine : public Engine, public Shared::Events {
private:
	const Ultima::UltimaGameDescription *_gameDescription;

protected:
	// Engine APIs
	Common::Error run() override;

	/**
	 * Returns true if the game should quit
	 */
	bool shouldQuit() const override {
		return Engine::shouldQuit();
	}

private:
	// 6-byte lagged-Fibonacci-style state for randByte(), matching the
	// original's rand_byte generator
	byte _rngState[6] = {};

	Audio::PCSpeaker *_pcSpeaker = nullptr;
	bool _pcSpeakerReady = false;

	/**
	 * Queues a single square-wave note. divisor is the original's PIT
	 * timer divisor (frequency = ~1193182/divisor), matching how each
	 * original sound is specified
	 */
	void queueTone(int divisor, uint32 lengthMs);

	/**
	 * Queues a moment of silence
	 */
	void queueSilence(uint32 lengthMs);

public:
	Data::Savegame _savegame;
	Data::Map _map;
	Data::MapDungeon _dungeon;
	Common::SharedPtr<Logic::Logic> _logic;

	// Debug flag toggled by the "intangible" console command, allowing
	// movement through normally impassable terrain
	bool _intangible = false;

public:
	Ultima2Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc);
	~Ultima2Engine() override;

	uint32 getFeatures() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	bool hasFeature(EngineFeature f) const override {
		return
			(f == kSupportsLoadingDuringRuntime) ||
			(f == kSupportsSavingDuringRuntime) ||
			(f == kSupportsReturnToLauncher);
	};

	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;

	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	}

	/**
	 * Returns true if any savegames exist
	 */
	bool savegamesExist() const;

	/**
	 * Returns the next byte from the game's random number generator
	 */
	byte randByte();

	/**
	 * Plays a short PC speaker sound effect. Ultima II has no numbered
	 * sound-effect table of its own - each original sound is its own
	 * hardcoded tone sweep, reproduced here as a short note sequence
	 */
	void playFX(Data::SoundEffect fx);
};

extern Ultima2Engine *g_engine;
#define _G(X) (Ultima2::g_engine->_##X)

} // namespace Ultima2
} // namespace Ultima

#endif
