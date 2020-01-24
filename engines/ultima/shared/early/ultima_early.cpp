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
#include "ultima/shared/early/ultima_early.h"
#include "ultima/shared/engine/debugger.h"
#include "ultima/shared/engine/events.h"
#include "ultima/shared/engine/resources.h"
#include "ultima/shared/core/mouse_cursor.h"
#include "ultima/shared/gfx/screen.h"
#include "ultima/ultima1/game.h"

namespace Ultima {

Shared::UltimaEarlyEngine *g_vm;

namespace Shared {

UltimaEarlyEngine::UltimaEarlyEngine(OSystem *syst, const UltimaGameDescription *gameDesc) :
		UltimaEngine(syst, gameDesc) {
	g_vm = this;
	_mouseCursor = nullptr;
	_screen = nullptr;
}

UltimaEarlyEngine::~UltimaEarlyEngine() {
	delete _debugger;
	delete _events;
	delete _game;
	delete _mouseCursor;
	delete _screen;
}

GameId UltimaEarlyEngine::getGameID() const {
	return _gameDescription->gameId;
}

bool UltimaEarlyEngine::initialize() {
	if (!UltimaEngine::initialize())
		return false;

	Resources *res = new Shared::Resources();
	if (!res->setup()) {
		delete res;
		return false;
	}

	_debugger = new Debugger();
	_events = new EventsManager(this);
	_screen = new Gfx::Screen();
	_mouseCursor = new MouseCursor();

	// Create the game, and signal to it that the game is starting
	_game = createGame();
	_events->addTarget(_game);
	_game->starting();
	return true;
}

void UltimaEarlyEngine::deinitialize() {
	UltimaEngine::deinitialize();
}

Common::Error UltimaEarlyEngine::run() {
	if (initialize()) {
		playGame();
	}

	deinitialize();
	return Common::kNoError;
}

void UltimaEarlyEngine::playGame() {
	while (!shouldQuit()) {
		_events->pollEventsAndWait();
	}
}

Game *UltimaEarlyEngine::createGame() const {
	switch (getGameID()) {
	case GAME_ULTIMA1:
		return new Ultima1::Ultima1Game();
	default:
		error("Unknown game");
	}
}

bool UltimaEarlyEngine::isDataRequired(Common::String &folder, int &majorVersion, int &minorVersion) {
	folder = "ultima1";
	majorVersion = 1;
	minorVersion = 0;
	return true;
}

Graphics::Screen *UltimaEarlyEngine::getScreen() const {
	return _screen;
}

} // End of namespace Shared
} // End of namespace Ultima
