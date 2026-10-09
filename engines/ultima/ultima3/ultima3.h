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

#ifndef ULTIMA3_H
#define ULTIMA3_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/ptr.h"
#include "common/serializer.h"
#include "common/util.h"
#include "engines/engine.h"
#include "ultima/detection.h"
#include "ultima/shared/engine/events.h"
#include "ultima/ultima3/data/map.h"
#include "ultima/ultima3/data/savegame.h"
#include "ultima/ultima3/gfx/message_log.h"
#include "ultima/ultima3/gfx/screen_effects.h"
#include "ultima/ultima3/gfx/shapes.h"

namespace Audio {
class PCSpeaker;
}

namespace Ultima {
namespace Ultima3 {

struct Ultima3GameDescription;

class Ultima3Engine : public Engine, public Shared::Events {
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
	Audio::PCSpeaker *_pcSpeaker = nullptr;
	bool _pcSpeakerReady = false;

public:
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

	/**
	 * Plays the short low buzz used to reject invalid input
	 */
	void playErrorBeep();

	/**
	 * Plays one of the game's sound effects, identified by its original number
	 */
	void playSoundEffect(byte effect);

	Data::Savegame _savegame;
	Data::Map _map;
	Data::Map _worldMap;
	Gfx::Shapes _shapes;
	Gfx::MessageLog _messages;
	Gfx::ScreenEffects _effects;
	byte _windDirection = 0;

	// Toggled in the game by the Volume command
	bool _soundEnabled = true;

	// Set when a loaded game should continue in the world rather than start it
	bool _resumeGame = false;

	// Debug flag toggled by the "intangible" console command, allowing
	// movement through normally impassable terrain
	bool _intangible = false;

	Ultima3Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc);
	~Ultima3Engine() override;

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
	 * Returns a font object that GfxSurface instances can use
	 */
	Graphics::Font *createFont() const override;
};

extern Ultima3Engine *g_engine;
#define _G(X) (Ultima3::g_engine->_##X)

} // namespace Ultima3
} // namespace Ultima

#endif
