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
#include "common/system.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/file.h"
#include "common/error.h"
#include "graphics/surface.h"
#include "common/stream.h"
#include "common/memstream.h"

#include "hdb/hdb.h"
#include "hdb/console.h"

namespace HDB {

HDBGame::HDBGame(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_console = nullptr;
	_systemInit = false;
	g_hdb = this;
	fileMan = new FileMan;
	lua = new LuaScript;

	DebugMan.addDebugChannel(kDebugExample1, "Example1", "This is just an example to test");
	DebugMan.addDebugChannel(kDebugExample2, "Example2", "This is also an example");
}

HDBGame::~HDBGame() {
	delete _console;
	delete g_hdb;
	delete fileMan;
	delete lua;
	DebugMan.clearAllDebugChannels();
}

bool HDBGame::init() {
	/*
		Game Subsystem Initializations
	*/

	// Init fileMan

	if (!fileMan->openMPC(getGameFile())) {
		error("FileMan::openMPC: Cannot find the hyperspace.mpc data file.");
		return false;
	}

	gameShutdown = false;
	_systemInit = true;

	return true;
}

void HDBGame::start() {
	_gameState = GAME_TITLE;
}

/*
	Changes the current GameState to the next one.
	Game State Transitions are deterministic: each state can
	only a particular state. The next state is held in gameState.

	TODO: All the functionality hasn't been implemented yet since
	their subsystems are incomplete. This section needs to be periodically
	updated as soon as the subsytems are improved.
*/
void HDBGame::changeGameState() {

	switch (_gameState) {
	case GAME_TITLE:
		_gameState = GAME_MENU;
		break;
	case GAME_MENU:
		_gameState = GAME_PLAY;
		break;
	case GAME_PLAY:
		_gameState = GAME_MENU;
		break;
	case GAME_LOADING:
		break;
	}
}

Common::Error HDBGame::run() {

	// Initialize System
	if (!_systemInit) {
		init();
	}

	// Initializes Graphics
	Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	initGraphics(640, 480, &format);
	_console = new Console();
	
	
	Common::SeekableReadStream *titleStream = fileMan->findFirstData("monkeylogoscreen", TYPE_PIC);
	if (titleStream == NULL) {
		debug("The TitleScreen MPC entry can't be found.");
		return Common::kReadingFailed;
	}

	Picture *titlePic = new Picture;
	Graphics::Surface surf = titlePic->load(titleStream);

	Common::SeekableReadStream *tileStream = fileMan->findFirstData("t32_ground1", TYPE_TILE32);
	if (tileStream == NULL) {
		debug("The t32_shipwindow_lr MPC entry can't be found.");
		return Common::kReadingFailed;
	}

	Tile *tile = new Tile;
	Graphics::Surface surf2 = tile->load(tileStream);

	Common::SeekableReadStream *luaStream = fileMan->findFirstData("CINE_INTRO_DEMO_LUA", TYPE_BINARY);
	int32 luaLength = fileMan->getLength("CINE_INTRO_DEMO_LUA", TYPE_BINARY);
	if (luaStream == NULL) {
		debug("The CINE_INTRO_DEMO_LUA MPC entry can't be found.");
		return Common::kReadingFailed;
	}

	lua->initScript(luaStream, "CINE_INTRO_DEMO_LUA", luaLength);
	//lua->executeMPC(luaStream, "CINE_INTRO_DEMO_LUA", luaLength);
	
	while (!shouldQuit()) {

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RTL:
				break;
			case Common::EVENT_KEYDOWN:
				debug("Key was pressed.");
				break;
			default:
				break;
			}
		}

		g_system->copyRectToScreen(surf.getBasePtr(0, 0), surf.pitch, 0, 0, surf.w, surf.h);
		g_system->copyRectToScreen(surf2.getBasePtr(0, 0), surf2.pitch, 0, 0, surf2.w, surf2.h);

		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	return Common::kNoError;
}

} // End of namespace HDB
