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
	_gfx = new Gfx;
	_lua = new LuaScript;
	_map = new Map;
	_ai = new AI;
	_input = new Input;
	_window = new Window;
	_rnd = new Common::RandomSource("hdb");

	DebugMan.addDebugChannel(kDebugExample1, "Example1", "This is just an example to test");
	DebugMan.addDebugChannel(kDebugExample2, "Example2", "This is also an example");
}

HDBGame::~HDBGame() {
	delete _console;
	delete _fileMan;
	delete _gfx;
	delete _lua;
	delete _map;
	delete _ai;
	delete _input;
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
	if (!_gfx->init()) {
		error("Gfx::init: Couldn't initialize Gfx");
	}
	if (!_input->init()) {
		error("Input::init: Couldn't initialize Input");
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

	start();
	_gameShutdown = false;
	_pauseFlag = 0;
	_systemInit = true;

	return true;
}

void HDBGame::start() {
	warning("REMOVE: _gameState initialized to GAME_PLAY");
	_gameState = GAME_PLAY;
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
		_gfx->drawPointer();
		break;
	case GAME_LOADING:
		warning("STUB: Gfx::DrawLoadingScreen required");
		break;
	}

	_gfx->updateVideo();
}

// builds a waypoint list if an entity is not next to player,
//	or gives info on an entity, or actually uses an entity
void HDBGame::setTargetXY(int x, int y) {
	AIEntity *e, *p;
	int px, py;
	bool oneTileAway;

	// if ANY button is pressed
	if (_input->getButtons() || _ai->_playerEmerging)
		return;

	// Check if an entity is next to us
	x /= kTileWidth;
	y /= kTileHeight;

	// Don't ever allow going to X-coord 0
	if (!x)
		return;

	e = _ai->findEntity(x, y);
	p = _ai->getPlayer();

	if (!p)
		return;

	px = p->x / kTileWidth;
	py = p->y / kTileHeight;

	// Are we on a touchplate and trying to move within the waiting period?
	if (p->touchpWait)
		return;

	// If we're attacking...don't do anything else
	AIState stateList[] = {
		STATE_ATK_CLUB_UP,	STATE_ATK_CLUB_DOWN, STATE_ATK_CLUB_LEFT, STATE_ATK_CLUB_RIGHT,
		STATE_ATK_STUN_UP,	STATE_ATK_STUN_DOWN, STATE_ATK_STUN_LEFT, STATE_ATK_STUN_RIGHT,
		STATE_ATK_SLUG_UP,	STATE_ATK_SLUG_DOWN, STATE_ATK_SLUG_LEFT, STATE_ATK_SLUG_RIGHT,
		STATE_PUSHUP,		STATE_PUSHDOWN,		 STATE_PUSHLEFT,	  STATE_PUSHRIGHT};

	for (int i = 0; i < 16; i++) {
		if (p->state == stateList[i])
			return;
	}

	oneTileAway = (abs(px - x) + abs(py - y) < 2);

	// If any entity has been targeted
	if (e && !_ai->waypointsLeft()) {
		// Clicking on a gettable item?
		// First check if an iterm is on top of a BLOCKER entity.
		// If so, try to find another entity there
		if (e->type == AI_NONE) {
			AIEntity *temp = g_hdb->_ai->findEntityIgnore(x, y, e);
			if (temp)
				e = temp;
		}

		if ((p->level == e->level) && _ai->getTableEnt(e->type)) {
			if (g_hdb->_ai->tileDistance(e, p) < 2) {
				useEntity(e);
				return;
			}
		}

		// Clicking on a Walkthrough Item?
		if ((p->level == e->level) && _ai->walkThroughEnt(e->type)) {
			_ai->addWaypoint(px, py, x, y, p->level);
			return;
		}

		// Is this an invisible blocker? If so, it probably has a LUA entity under it
		if (e->type == AI_NONE && _ai->luaExistAtXY(x, y)) {
			// Did player click on a LUA tile?
			if (oneTileAway && _ai->checkLuaList(_ai->getPlayer(), x, y))
				return;
		}

		// On the same Level? (Allow pushing on stairs, down only)
		if ((p->level != e->level && !(_map->getMapBGTileFlags(e->tileX, e->tileY) & kFlagStairBot)) || (p->level == e->level && _ai->walkThroughEnt(e->type))) {
			_ai->addWaypoint(px, py, x, y, p->level);
			return;
		}

		int chx = abs(px - x);
		int chy = abs(py - y);

		// And its a unit away and the Player's GOALS are done...
		if (chx <= 1 && chy <= 1 && !p->goalX) {
			// At a horizontal or vertical direction?
			if (chx + chy > 1) {
				AIEntity *e1, *e2;
				uint32 flag1, flag2;

				e1 = _ai->findEntity(px, y);
				e2 = _ai->findEntity(x, py);
				flag1 = _map->getMapBGTileFlags(px, y) & kFlagSolid;
				flag2 = _map->getMapBGTileFlags(x, py) & kFlagSolid;
				if ((e1 || flag1) && (e2 || flag2))
					return;
			}

			// Check for items that should NOT be picked up or talked to
			switch (e->type) {
				// USEing a floating crate or barrel?  Just go there.
				// Unless it's not floating, in which case you wanna push it.
			case AI_CRATE:
			case AI_LIGHTBARREL:
				// USEing a heavy barrel ONLY means walking on it if it's floating
				// *** cannot push a heavy barrel
			case AI_HEAVYBARREL:
				if (e->state == STATE_FLOATING || e->state == STATE_MELTED)
					_ai->addWaypoint(px, py, x, y, p->level);
				else
					useEntity(e);
				return;
			default:
				useEntity(e);
				return;
			}
		} else {
			_ai->addWaypoint(px, py, x, y, p->level);
			return;
		}
	}

	// Are we trying to "activate" a touchplate?
	// Set a waypoint on it
	if (_ai->checkForTouchplate(x, y)) {
		_ai->addWaypoint(px, py, x, y, p->level);
		return;
	}

	// Did the player click on an action tile?
	if (oneTileAway && _ai->checkActionList(_ai->getPlayer(), x, y, true))
		return;

	// Did the player click on an auto-action tile?
	if (oneTileAway && _ai->checkAutoList(_ai->getPlayer(), x, y))
		return;

	// we need to add this point to the waypoint list!
	// the list is tile coord-based
	//
	// if the player is not PUSHING anything and has no GOALS,
	// it's ok to set up a waypoint
	switch (p->state) {
	case STATE_PUSHDOWN:
	case STATE_PUSHUP:
	case STATE_PUSHLEFT:
	case STATE_PUSHRIGHT:
	case STATE_NONE:
		break;
	default:
		_ai->addWaypoint(px, py, x, y, p->level);
		break;
	}
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
		_ai->animGrabbing();
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

#if 0
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
#endif

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

	//_window->openDialog("Sgt. Filibuster", 0, "You address me as 'sarge' or 'sergeant' or get your snappin' teeth kicked in! Got me?", 0, NULL);

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
			case Common::EVENT_MOUSEMOVE:
				_input->updateMouse(event.mouse.x, event.mouse.y);
				break;
			case Common::EVENT_LBUTTONDOWN:
				_input->updateMouseButtons(1, 0, 0);
				break;
			case Common::EVENT_LBUTTONUP:
				_input->updateMouseButtons(-1, 0, 0);
				break;
			case Common::EVENT_MBUTTONDOWN:
				_input->updateMouseButtons(0, 1, 0);
				break;
			case Common::EVENT_MBUTTONUP:
				_input->updateMouseButtons(0, -1, 0);
				break;
			case Common::EVENT_RBUTTONDOWN:
				_input->updateMouseButtons(0, 0, 1);
				break;
			case Common::EVENT_RBUTTONUP:
				_input->updateMouseButtons(0, 0, -1);
				break;
			case Common::EVENT_KEYDOWN:
				_input->updateKeys(event, true);
				break;
			case Common::EVENT_KEYUP:
				_input->updateKeys(event, false);
				break;
			default:
				break;
			}
		}

		if (_gameState == GAME_PLAY) {
			_gfx->drawSky();

			if (!_pauseFlag) {
				_ai->moveEnts();
				_ai->processCallbackList();
			}

			_map->draw();
			_ai->processCines();
			//_window->drawDialog();

			AIEntity *e = _ai->getPlayer();

			if (e && e->level < 2)
				_ai->drawWayPoints();

			_map->drawEnts();
			_map->drawGratings();

			if (e && e->level == 2)
				_ai->drawWayPoints();

			_ai->drawLevel2Ents();

			_map->drawForegrounds();
			_ai->animateTargets();

			_window->drawDialogChoice();
			_window->drawDialog();
			_window->drawMessageBar();
			_window->drawInventory();
			_window->drawDeliveries();
			_window->drawTextOut();
			_window->drawPause();
		}

		// Update Timer that's NOT used for in-game Timing
		_prevTimeSlice = _timeSlice;
		_timeSlice = g_system->getMillis();

		paint();

		g_system->updateScreen();
		g_system->delayMillis(1000/60);
	}

	return Common::kNoError;
}

} // End of namespace HDB
