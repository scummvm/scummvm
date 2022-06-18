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

#ifndef IMMORTAL_IMMORTAL_H
#define IMMORTAL_IMMORTAL_H

#include "audio/mixer.h"

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "common/platform.h"

#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"

#include "immortal/detection.h"

namespace Immortal {

struct ImmortalGameDescription;
class ProDosDisk;

class ImmortalEngine : public Engine {
private:
	Common::RandomSource _randomSource;

protected:
	// Engine APIs
	Common::Error run() override;
public:
	const ADGameDescription *_gameDescription;

public:
	ImmortalEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~ImmortalEngine() override;

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

	bool canLoadGameStateCurrently() override {
		return true;
	}
	bool canSaveGameStateCurrently() override {
		return true;
	}

	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	/* Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	} */

	/* Common::Error loadGameStream(Common::SeekableReadStream *stream) {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	} */
};

extern ImmortalEngine *g_engine;
#define SHOULD_QUIT ::Immortal::g_engine->shouldQuit()

} // namespace Immortal

#endif
