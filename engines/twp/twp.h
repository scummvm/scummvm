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

#ifndef TWP_H
#define TWP_H

#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"
#include "twp/detection.h"
#include "twp/vm.h"
#include "twp/resmanager.h"
#include "twp/room.h"
#include "twp/ggpack.h"
#include "twp/squirrel/squirrel.h"

namespace Twp {

class Thread;
struct TwpGameDescription;

class TwpEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;

protected:
	// Engine APIs
	Common::Error run() override;

public:
	Graphics::Screen *_screen = nullptr;
	Common::Array<Object*> _objects;
	Common::Array<Thread*> _threads;
	GGPackDecoder _pack;
	ResManager _resManager;
	Common::Array<Room> _rooms;
	Room* _room = nullptr;

public:
	TwpEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~TwpEngine() override;

	uint32 getFeatures() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	/**
	 * Gets the random source
	 */
	Common::RandomSource& getRandomSource() { return _randomSource; }

	HSQUIRRELVM getVm() { return _vm.get(); }
	inline Gfx& getGfx() { return _gfx; }

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

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	}

	Math::Vector2d roomToScreen(Math::Vector2d pos);
	Math::Vector2d screenToRoom(Math::Vector2d pos);

private:
	Gfx _gfx;
	Vm _vm;
};

extern TwpEngine *g_engine;
#define SHOULD_QUIT ::Twp::g_engine->shouldQuit()

} // End of namespace Twp

#endif // TWP_H
