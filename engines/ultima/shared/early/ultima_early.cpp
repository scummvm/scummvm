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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/system.h"
#include "common/translation.h"
#include "graphics/thumbnail.h"
#include "graphics/scaler.h"
#include "gui/saveload.h"
#include "ultima/shared/early/ultima_early.h"
#include "ultima/shared/early/game.h"
#include "ultima/shared/engine/events.h"
#include "ultima/shared/engine/resources.h"
#include "ultima/shared/core/mouse_cursor.h"
#include "ultima/shared/gfx/screen.h"

#ifndef RELEASE_BUILD
#include "ultima/ultima1/game.h"
#endif

namespace Ultima {

Shared::UltimaEarlyEngine *g_vm;

namespace Shared {

UltimaEarlyEngine::UltimaEarlyEngine(OSystem *syst, const UltimaGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc), _game(nullptr), _randomSource("Ultima") {
	g_vm = this;
	_mouseCursor = nullptr;
	_screen = nullptr;
}

UltimaEarlyEngine::~UltimaEarlyEngine() {
	delete _events;
	delete _game;
	delete _mouseCursor;
	delete _screen;
}

bool UltimaEarlyEngine::initialize() {
	// Call syncSoundSettings to get default volumes set
	syncSoundSettings();

	// Set up the resources datafile
	Resources *res = new Resources();
	if (!res->open()) {
		GUIErrorMessage(_("Could not find correct ultima.dat datafile"));
		return false;
	}
	SearchMan.add("ultima", res);

	_events = new EventsManager(this);
	_screen = new Gfx::Screen();

	// Create the game, and signal to it that the game is starting
	_game = createGame();
	_events->addTarget(_game);
	_game->starting(false);

	// Load cursors
	_mouseCursor = new MouseCursor();

	// If requested, load a savegame instead of showing the intro
	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= 999)
			loadGameState(saveSlot);
	}

	return true;
}

Common::Error UltimaEarlyEngine::run() {
	// Initialize the engine and play the game
	if (initialize())
		playGame();

	// Deinitialize and free the engine
	deinitialize();
	return Common::kNoError;
}

bool UltimaEarlyEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsChangingOptionsDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

void UltimaEarlyEngine::playGame() {
	while (!shouldQuit()) {
		_events->pollEventsAndWait();
	}
}

Graphics::Screen *UltimaEarlyEngine::getScreen() const {
	return _screen;
}

GameId UltimaEarlyEngine::getGameId() const {
	return _gameDescription->gameId;
}

bool UltimaEarlyEngine::isEnhanced() const {
	return _gameDescription->features & GF_VGA_ENHANCED;
}

Game *UltimaEarlyEngine::createGame() const {
	switch (getGameId()) {
#ifndef RELEASE_BUILD
	case GAME_ULTIMA1:
		return new Ultima1::Ultima1Game();
#endif
	default:
		error("Unknown game");
	}
}

Common::Error UltimaEarlyEngine::loadGameStream(Common::SeekableReadStream *stream) {
	// Read in the game's data
	Common::Serializer s(stream, nullptr);
	_game->synchronize(s);
	return Common::kNoError;
}

Common::Error UltimaEarlyEngine::saveGameStream(Common::WriteStream *stream, bool) {
	// Write out the game's data
	Common::Serializer s(nullptr, stream);
	_game->synchronize(s);
	return Common::kNoError;
}

bool UltimaEarlyEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return _game->canLoadGameStateCurrently(msg);
}

bool UltimaEarlyEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return _game->canSaveGameStateCurrently(msg);
}

} // End of namespace Shared
} // End of namespace Ultima
