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
#include "common/stream.h"
#include "common/memstream.h"

#include "hdb/hdb.h"
#include "hdb/console.h"

namespace HDB {

HDBGame* g_hdb;

HDBGame::HDBGame(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_console = nullptr;
	_format = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	_systemInit = false;
	g_hdb = this;
	_fileMan = new FileMan;
	_drawMan = new DrawMan;
	_lua = new LuaScript;
	_map = new Map;
	_ai = new AI;
	_window = new Window;
	_rnd = new Common::RandomSource("hdb");

	DebugMan.addDebugChannel(kDebugExample1, "Example1", "This is just an example to test");
	DebugMan.addDebugChannel(kDebugExample2, "Example2", "This is also an example");
}

HDBGame::~HDBGame() {
	delete _console;
	delete _fileMan;
	delete _drawMan;
	delete _lua;
	delete _map;
	delete _ai;
	delete _window;
	delete _rnd;

	DebugMan.clearAllDebugChannels();
}

bool HDBGame::init() {
	/*
		Game Subsystem Initializations
	*/

	// Init fileMan

	if (!_fileMan->openMPC(getGameFile())) {
		error("FileMan::openMPC: Cannot find the hyperspace.mpc data file.");
	}
	if (!_drawMan->init()) {
		error("DrawMan::init: Couldn't initialize DrawMan");
	}
	if (!_ai->init()) {
		error("AI::init: Couldn't initialize AI");
	}
	if (!_lua->init()) {
		error("LuaScript::init: Couldn't load the GLOBAL_LUA code.");
	}
	if (!_window->init()) {
		error("Window::init: Couldn't initialize Window");
	}

	// REMOVE: Putting this here since Menu hasn't been implemented yet.
	// Defaults the game into Action Mode
	setActionMode(1);

	_gameShutdown = false;
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

void HDBGame::paint() {
	switch (_gameState) {
	case GAME_TITLE:
		debug(9, "STUB: MENU::DrawTitle required");
		break;
	case GAME_MENU:
		warning("STUB: MENU::DrawMenu required");
		break;
	case GAME_PLAY:
		warning("STUB: DrawMan::DrawPointer required");
		break;
	case GAME_LOADING:
		warning("STUB: DrawMan::DrawLoadingScreen required");
		break;
	}

	_drawMan->updateVideo();
}

// PLAYER is trying to use this entity
void HDBGame::useEntity(AIEntity *e) {
	warning("STUB: HDBGame::useEntity incomplete");

	AIEntity *p, temp;
	bool added;

	p = _ai->getPlayer();
	// Check if entity is on same level or if its a stairtop
	if ((p->level != e->level) && !(_map->getMapBGTileFlags(p->tileX, p->tileY) & kFlagStairTop)) {
		return;
	}

	added = false;

	if (_ai->getTableEnt(e->type)) {
		memcpy(&temp, e, sizeof(AIEntity));

		_ai->getItemSound(e->type);

		added = _ai->addToInventory(e);
		if (added) {
			e = &temp;

			if (temp.aiUse) {
				temp.aiUse(&temp);
			}

			if (temp.luaFuncUse[0]) {
				_lua->callFunction(temp.luaFuncUse, 0);
			}
		}

	} else {
		// These should be run over or run through
		if (_ai->walkThroughEnt(e->type) || e->type == AI_NONE) {
			return;
		}

		if (e->aiUse) {
			e->aiUse(e);
		}

		if (e->luaFuncUse[0]) {
			_lua->callFunction(e->luaFuncUse, 0);
		}
	}

	/*
		PUSHING
		If its a pushable object, push it. Unless it's in/on water.
	*/
	if (e->type == AI_CRATE || e->type == AI_LIGHTBARREL || e->type == AI_BOOMBARREL || e->type == AI_MAGIC_EGG || e->type == AI_ICE_BLOCK || e->type == AI_FROGSTATUE || e->type == AI_DIVERTER) {
		warning("STUB: HDBGame::useEntity PUSHING required");
	}

	// Look at Entity
	if (e->type != AI_RAILRIDER_ON) {
		_ai->lookAtEntity(e);
	}

	// Grab animation
	if (added) {
		warning("STUB: HDBGame::useEntity show Grab Animation");
	}

	// Can't push it - make a sound
	if (e->type == AI_HEAVYBARREL) {
		warning("STUB: HDBGame::useEntity Play HEAVYBARREL sound");
	}
}

Common::Error HDBGame::run() {

	// Initialize System
	if (!_systemInit) {
		init();
	}

	// Initializes Graphics
	initGraphics(kScreenWidth, kScreenHeight, &_format);
	_console = new Console();


	Common::SeekableReadStream *titleStream = _fileMan->findFirstData("monkeylogoscreen", TYPE_PIC);
	if (titleStream == NULL) {
		debug("The TitleScreen MPC entry can't be found.");
		return Common::kReadingFailed;
	}

	Picture *titlePic = new Picture;
	titlePic->load(titleStream);

	Common::SeekableReadStream *tileStream = _fileMan->findFirstData("t32_ground1", TYPE_TILE32);
	if (tileStream == NULL) {
		debug("The t32_shipwindow_lr MPC entry can't be found.");
		return Common::kReadingFailed;
	}

	Tile *tile = new Tile;
	tile->load(tileStream);

	Common::SeekableReadStream *luaStream = _fileMan->findFirstData("MAP00_LUA", TYPE_BINARY);
	int32 luaLength = _fileMan->getLength("MAP00_LUA", TYPE_BINARY);
	if (luaStream == NULL) {
		debug("The MAP00_LUA MPC entry can't be found.");
		return Common::kReadingFailed;
	}

	_lua->initScript(luaStream, "MAP00_LUA", luaLength);

	_lua->callFunction("level_loaded", 0);

	Common::SeekableReadStream *mapStream = _fileMan->findFirstData("MAP00_MSM", TYPE_BINARY);
	if (mapStream == NULL) {
		debug("The MAP00_MSM MPC entry can't be found.");
		return Common::kReadingFailed;
	}

	_map->load(mapStream);
	_ai->initAnimInfo();

#if 0
	lua->executeFile("test.lua");
#endif

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

		_drawMan->drawSky();
		_map->draw();
		_ai->processCines();
		_map->drawEnts();
		_map->drawGratings();
		_map->drawForegrounds();
		_ai->animateTargets();

		_window->drawDialog();

		// Update Timer that's NOT used for in-game Timing
		_prevTimeSlice = _timeSlice;
		_timeSlice = g_system->getMillis();

		paint();

		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	return Common::kNoError;
}

} // End of namespace HDB
