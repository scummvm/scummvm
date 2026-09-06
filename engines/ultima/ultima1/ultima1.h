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

#ifndef ULTIMA1_H
#define ULTIMA1_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/ptr.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/palette.h"
#include "graphics/screen.h"
#include "ultima/detection.h"
#include "ultima/shared/engine/events.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/data/map_dungeon.h"
#include "ultima/ultima1/data/savegame.h"
#include "ultima/ultima1/data/tiles.h"
#include "ultima/ultima1/logic/logic.h"

namespace Ultima {
namespace Ultima1 {

struct Ultima1GameDescription;

class Ultima1Engine : public Engine, public Shared::Events {
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

public:
	Data::Map _map;
	Data::MapDungeon _dungeon;
	Data::Savegame _savegame;
	Common::SharedPtr<Logic::Logic> _logic;
	int _overworldEntityCount = 0;
	int _creaturesCount = 0;
	uint32 _moveCtr = 0;

public:
	Ultima1Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc);
	~Ultima1Engine() override;

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

	/**
	 * Returns true if any savegames exist
	 */
	bool savegamesExist() const;

	/**
	 * Play a sound effect
	 */
	void playFX(int num);
};

extern Ultima1Engine *g_engine;
#define _G(X) (Ultima1::g_engine->_##X)

} // namespace Ultima1
} // namespace Ultima

#endif
