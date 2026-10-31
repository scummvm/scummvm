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

#ifndef SCOOBY_H
#define SCOOBY_H

#include "common/error.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/framelimiter.h"

#include "scooby/detection.h"
#include "startup/application_loop.h"

namespace Scooby {

struct ScoobyGameDescription;
class GemsDriver;
class ControllerInput;
class MouseInput;

enum ScoobyAction {
	kScoobyActionNone,
	kScoobyActionSelect,
	kScoobyActionToggleInventory,
	kScoobyActionCancel,
	kScoobyActionStart,
	kScoobyActionQuit,
	kScoobyActionSkipCutscene,

	kScoobyActionUp,
	kScoobyActionDown,
	kScoobyActionLeft,
	kScoobyActionRight
};

class ScoobyEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;

	Graphics::FrameLimiter *_limiter;
	ApplicationLoop *_applicationLoop;
	GemsDriver *_gemsDriver;
	ControllerInput *_input;

	uint32 _nextUpdatetime;
	bool _walkAreaShown;

protected:
	// Engine APIs
	Common::Error run() override;
public:
	ScoobyEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~ScoobyEngine() override;

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
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;

	void syncSoundSettings() override;

	// The GEMS sound driver, or nullptr while the engine is not running.
	GemsDriver *getGemsDriver() const { return _gemsDriver; }

	// Managed-only mouse support: the shared mouse owner for loops without a ControllerInput reference.
	MouseInput &getMouseInput();

	// Managed-only debug overlay: whether rooms show the walk area (Console "showWalkArea").
	bool isWalkAreaShown() const { return _walkAreaShown; }
	void setWalkAreaShown(bool shown) { _walkAreaShown = shown; }

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
};

extern ScoobyEngine *g_engine;
#define SHOULD_QUIT ::Scooby::g_engine->shouldQuit()

} // End of namespace Scooby

#endif // SCOOBY_H
