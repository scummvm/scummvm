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

#ifndef TESORO_H
#define TESORO_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"

#include "tesoro/detection.h"
#include "tesoro/core/archive.h"
#include "tesoro/core/animbank.h"
#include "tesoro/core/camera.h"
#include "tesoro/core/clock.h"
#include "tesoro/core/display.h"
#include "tesoro/core/object.h"

namespace Tesoro {

struct TesoroGameDescription;

class Level;

class TesoroEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	GameClock _clock;
	Common::RandomSource _randomSource;
	Archive _archive;
	Camera _camera;
	Common::Array<AnimPlayer> _testPlayers;	// the "level" of the test object
	Level *_level = nullptr;
	Display _display;
	int32 _scaleX = 0x10000, _scaleY = 0x10000;	// 0x10000 is normal scale
	AnimPlayer _font;

protected:
	// Engine APIs
	Common::Error run() override;
public:
	Graphics::Screen *_screen = nullptr;
public:
	TesoroEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~TesoroEngine() override;

	int32 gameTime() { return _clock.update(_system->getMillis()); }

	uint32 getFeatures() const;

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
		return
		    (f == kSupportsLoadingDuringRuntime) ||
		    (f == kSupportsSavingDuringRuntime) ||
		    (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}

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

	// temp functions
	void setScale(int32 x, int32 y) { _scaleX = x; _scaleY = y; }
	void fadeOut(int steps, int delayMs) { _display.fadeOut(steps, delayMs, _screen); }
	/** The camera of the level being shown, else the test object's. */
	Camera &camera();
	/** Loads a level from the archive and shows it. */
	bool level(const char *name);
};

extern TesoroEngine *g_engine;
#define SHOULD_QUIT ::Tesoro::g_engine->shouldQuit()

} // End of namespace Tesoro

#endif // TESORO_H
