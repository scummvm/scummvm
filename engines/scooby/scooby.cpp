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

#include "scooby/scooby.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/framelimiter.h"
#include "graphics/paletteman.h"
#include "graphics/tile_scene.h"
#include "raylib_host.h"
#include "scooby/console.h"
#include "scooby/detection.h"
#include "startup/application_loop.h"

namespace Scooby {

ScoobyEngine *g_engine;

ScoobyEngine::ScoobyEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Scooby") {
	g_engine = this;
}

ScoobyEngine::~ScoobyEngine() {
}

uint32 ScoobyEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String ScoobyEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error ScoobyEngine::run() {
	Common::String loadError;
	Common::ScopedPtr<Scooby::ScoobyDooRom> rom =
		Scooby::ScoobyDooRom::load("scooby.md", loadError);
	if (!rom) {
		error("%s\n", loadError.c_str());
		return Common::kUnknownError;
	}

	initGraphics(320, 240);

	Common::ScopedPtr<Scooby::RaylibHost> host = Scooby::RaylibHost::create();
	if (!host) {
		error("Could not open the raylib host window.\n");
		return Common::kUnknownError;
	}

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	Graphics::Screen screen;

	Scooby::TileScene scene;
	Scooby::IndexedFrame frame(screen);

	Scooby::ApplicationLoop(*rom, scene, frame, *host).run();

	return Common::kNoError;
}

Common::Error ScoobyEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace Scooby
