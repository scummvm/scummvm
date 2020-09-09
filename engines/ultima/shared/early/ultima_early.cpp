/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include "ultima/shared/engine/ultima.h"
#include "ultima/shared/engine/debugger.h"
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
		UltimaEngine(syst, gameDesc), _game(nullptr) {
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
	if (!UltimaEngine::initialize())
		return false;

	// Set up the resources datafile
	Resources *res = new Resources();
	if (!res->open()) {
		GUIErrorMessage(_("Could not find correct ultima.dat datafile"));
		return false;
	}
	SearchMan.add("ultima", res);

	setDebugger(new Debugger());
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

void UltimaEarlyEngine::deinitialize() {
	UltimaEngine::deinitialize();
}

Common::Error UltimaEarlyEngine::run() {
	// Initialize the engine and play the game
	if (initialize())
		playGame();

	// Deinitialize and free the engine
	deinitialize();
	return Common::kNoError;
}

void UltimaEarlyEngine::playGame() {
	while (!shouldQuit()) {
		_events->pollEventsAndWait();
	}
}

Graphics::Screen *UltimaEarlyEngine::getScreen() const {
	return _screen;
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

bool UltimaEarlyEngine::canLoadGameStateCurrently(bool isAutosave) {
	return _game->canLoadGameStateCurrently();
}

bool UltimaEarlyEngine::canSaveGameStateCurrently(bool isAutosave) {
	return _game->canSaveGameStateCurrently();
}

bool UltimaEarlyEngine::isDataRequired(Common::String &folder, int &majorVersion, int &minorVersion) {
	folder = "ultima1";
	majorVersion = 1;
	minorVersion = 0;
	return true;
}

} // End of namespace Shared
} // End of namespace Ultima
