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

#include "common/config-manager.h"
#include "common/random.h"

#include "engines/util.h"

#include "hdb/hdb.h"
#include "hdb/ai.h"
#include "hdb/file-manager.h"
#include "hdb/gfx.h"
#include "hdb/input.h"
#include "hdb/lua-script.h"
#include "hdb/map.h"
#include "hdb/menu.h"
#include "hdb/sound.h"
#include "hdb/mpc.h"
#include "hdb/window.h"

#define CHEAT_PATCHES 0

namespace HDB {

HDBGame* g_hdb;

HDBGame::HDBGame(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	g_hdb = this;
	if (isPPC()) {
		_screenWidth = 240;
		_screenHeight = 320;
		_screenDrawWidth = 240;
		_screenDrawHeight = 320;
		_progressY = 280;
	} else {
		_screenWidth = 640;
		_screenHeight = 480;
		_screenDrawWidth = _screenWidth - 160;
		_screenDrawHeight = 480;
		_progressY = _screenHeight - 64;
	}

	_format = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	_systemInit = false;

	_fileMan = nullptr;
	_gfx = nullptr;
	_lua = nullptr;
	_menu = nullptr;
	_map = nullptr;
	_ai = nullptr;
	_input = nullptr;
	_sound = nullptr;
	_window = nullptr;
	_rnd = new Common::RandomSource("hdb");

	_cheating = false;

	_currentMapname[0] = _currentLuaName[0] = 0;
	_lastMapname[0] = _lastLuaName[0] = 0;
	_inMapName[0] = 0;

	_timePlayed = _timeSlice = _prevTimeSlice = _timeSeconds = _tiempo = 0;

	_currentOutSaveFile = nullptr;
	_currentInSaveFile = nullptr;

	_progressActive = false;

	_monkeystone7 = STARS_MONKEYSTONE_7_FAKE;
	_monkeystone14 = STARS_MONKEYSTONE_14_FAKE;
	_monkeystone21 = STARS_MONKEYSTONE_21_FAKE;

	_gameShutdown = false;
	_progressGfx = nullptr;
	_progressMarkGfx = nullptr;
	_loadingScreenGfx = nullptr;
	_logoGfx = nullptr;
	_progressCurrent = -1;
	_progressXOffset = -1;
	_progressMax = -1;
	_gameState = GAME_TITLE;
	_actionMode = -1;
	_pauseFlag = false;
	_debugFlag = -1;
	_debugLogo = nullptr;
	_dx = 0;
	_dy = 0;
	_changeLevel = false;
	_saveInfo.active = false;
	_saveInfo.slot = 0;
	_loadInfo.active = false;
	_loadInfo.slot = 0;

	syncSoundSettings();
}

HDBGame::~HDBGame() {
	delete _fileMan;
	delete _gfx;
	delete _lua;
	delete _menu;
	delete _map;
	delete _ai;
	delete _input;
	delete _sound;
	delete _window;
	delete _rnd;

	delete _progressGfx;
	_progressGfx = nullptr;
	delete _progressMarkGfx;
	_progressMarkGfx = nullptr;
	delete _loadingScreenGfx;
	_loadingScreenGfx = nullptr;
	if (_logoGfx) {
		delete _logoGfx;
		_logoGfx = nullptr;
	}
	delete _debugLogo;
	_debugLogo = nullptr;
}

bool HDBGame::init() {
	/*
		Game Subsystem Initializations
	*/

	_systemInit = false;
	_fileMan = new FileMan;
	_gfx = new Gfx;
	_lua = new LuaScript;
	_menu = new Menu;
	_map = new Map;
	_ai = new AI;
	_input = new Input;
	_sound = new Sound;
	_window = new Window;

	// Init fileMan

	_fileMan->openMPC(getGameFile());

	_gfx->init();
	_sound->init();
	_ai->init();
	_window->init();
	_input->init();
	_lua->init();
	_menu->init();

	_debugLogo = _gfx->loadIcon("icon_debug_logo");
	_progressGfx = _gfx->loadPic(PIC_LOADBAR);
	_progressMarkGfx = _gfx->loadPic(PIC_LOADSTAR);
	_logoGfx = nullptr;

	_changeLevel = false;
	_changeMapname[0] = 0;
	_loadInfo.active = _saveInfo.active = false;

	_menu->startTitle();

	_gameShutdown = false;
	_pauseFlag = false;
	_systemInit = true;
	if (!g_hdb->isPPC())
		_loadingScreenGfx = _gfx->loadPic(PIC_LOADSCREEN);
	else
		_loadingScreenGfx = nullptr;

	return true;
}

void HDBGame::initializePath(const Common::FSNode &gamePath) {
	Engine::initializePath(gamePath);
	SearchMan.addDirectory("music", gamePath.getChild("music"));
}

void HDBGame::changeGameState() {

	switch (_gameState) {
	case GAME_TITLE:
		_menu->startMenu();
		_gameState = GAME_MENU;
		break;
	case GAME_MENU:
		_menu->freeMenu();
		_sound->stopMusic();
		_sound->clearPersistent();
		_ai->clearPersistent();
		_timePlayed = 0;
		_timeSeconds = 0;

		if (!isDemo()) {
			if (!startMap("CINE_INTRO"))
				error("Can't load CINE_INTRO");
		} else {
			if (!startMap("CINE_INTRO_DEMO"))
				error("Can't load CINE_INTRO_DEMO");
		}
		_gameState = GAME_PLAY;
		break;
	case GAME_PLAY:
		_menu->startMenu();
		_gameState = GAME_MENU;
		break;
	case GAME_LOADING:
		break;
	default:
		break;
	}
}

void HDBGame::start() {
	_gameState = GAME_TITLE;

	_debugFlag = 0;
}

bool HDBGame::restartMap() {
	if (!_currentMapname[0])
		return false;

	debug(0, "Starting map %s", _currentMapname);

	_gfx->emptyGfxCaches();
	_lua->callFunction("level_shutdown", 0);

	_gfx->turnOffSnow();
	_window->restartSystem();
	_ai->restartSystem();
	_lua->init();

#if CHEAT_PATCHES
	if (!strcmp(_currentLuaName, "MAP11.LUA")) {
		// Let enter the labs

		_lua->saveGlobalNumber("map12_complete", 1);

		Common::strlcpy(_lastMapname, "MAP12", 64);
	}

	if (!strcmp(_currentLuaName, "MAP06.LUA")) {
		// Have overtime pay for Hanson

		_ai->setGemAmount(100);
	}

	if (!strcmp(_currentLuaName, "MAP29.LUA")) {
		// Finish MAP29

		_ai->_numGooCups = 8;
	}

#endif

	_lua->loadLua(_currentLuaName);

	_sound->markSoundCacheFreeable();
	_map->restartSystem();

	if (!_map->loadMap(_currentMapname))
		return false;

	_ai->initAnimInfo();

	// Cheat/workarounds
#if CHEAT_PATCHES
	if (!strcmp(_currentLuaName, "MAP00.LUA")) {
		Common::String patch("KillTrigger( \"mannyquest\" )");

		_lua->executeChunk(patch, "MAP00 patch");

		_ai->addItemToInventory(ITEM_CELL, 1, 0, 0, 0);
	}
#endif

	// if there are Secret Stars here, stick the variable in Lua
	if (!_menu->_starWarp && getStarsMonkeystone7() == STARS_MONKEYSTONE_7)
		_lua->setLuaGlobalValue("secretstars", 1);
	if (_menu->_starWarp == 1 && getStarsMonkeystone14() == STARS_MONKEYSTONE_14)
		_lua->setLuaGlobalValue("secretstars", 2);
	if (_menu->_starWarp == 2 && getStarsMonkeystone21() == STARS_MONKEYSTONE_21)
		_lua->setLuaGlobalValue("secretstars", 3);

	_lua->callFunction("level_loaded", 0);
	if (!_ai->cinematicsActive())
		_gfx->turnOffFade();

	// center the player on the screen
	int x, y;

	_ai->getPlayerXY(&x, &y);
	_map->centerMapXY(x + 16, y + 16);

	debug(5, "Action List Info:");
	for (int k = 0; k < 20; k++) {
		debug(5, "Action %d: entityName: %s", k, _ai->_actions[k].entityName);
		debug(5, "Action %d: x1: %d, y1: %d", k, _ai->_actions[k].x1, _ai->_actions[k].y1);
		debug(5, "Action %d: x2: %d, y2: %d", k, _ai->_actions[k].x2, _ai->_actions[k].y2);
		debug(5, "Action %d: luaFuncInit: %s, luaFuncUse: %s", k, _ai->_actions[k].luaFuncInit, _ai->_actions[k].luaFuncUse);
	}

	return true;
}

bool HDBGame::startMap(const char *name) {
	// save last mapname
	Common::strlcpy(_lastMapname, _currentMapname, sizeof(_lastMapname));

	// set current mapname
	Common::strlcpy(_currentMapname, name, sizeof(_currentMapname));
	Common::strlcat(_currentMapname, ".MSM", sizeof(_currentMapname));

	// set current luaname
	Common::strlcpy(_currentLuaName, name, sizeof(_currentLuaName));
	Common::strlcat(_currentLuaName, ".LUA", sizeof(_currentLuaName));

	restartMap();

	//
	// here is where we will be autosaving the start of level
	// don't save cine intro/outro/etc...OR map30 (secret star map)
	//
	if (!scumm_strnicmp(name, "map", 3) && scumm_stricmp(name, "map30")) {
		_menu->fillSavegameSlots();
		saveGameState(0, Common::String::format("Autosave %s", name)); // we ignore the slot parameter in everything else since we just keep saving...
	}
	return true;
}

void HDBGame::paint() {

	_tiempo = g_system->getMillis();

	switch (_gameState) {
	case GAME_TITLE:
		_menu->drawTitle();
		break;
	case GAME_MENU:
		_menu->drawMenu();
		// fall through
	case GAME_PLAY:
		_gfx->drawPointer();
		break;
	case GAME_LOADING:
		{
			// clear video, then draw HDB logo
			drawLoadingScreen();

			// if the graphic has never been loaded, load it now and leave it in memory
			if (!_logoGfx)
				_logoGfx = _gfx->loadPic(TITLELOGO);
			_logoGfx->drawMasked(_screenWidth / 2 - _logoGfx->_width / 2, 10);

			int	x = _screenWidth / 2 - _progressGfx->_width / 2;
			int pixels = _progressGfx->_width - _progressMarkGfx->_width;
			_progressXOffset = (int)(((double)pixels / _progressMax) * (double)_progressCurrent) + x;
		}
		break;
	default:
		break;
	}

	// Draw FPS on Screen in Debug Mode
	if (_debugFlag == 1)
		_gfx->drawDebugInfo(_debugLogo, _frames.size());
	else if (_debugFlag == 2)
		_debugLogo->drawMasked(_screenWidth - 32, 0);

	_gfx->updateVideo();
}

// builds a waypoint list if an entity is not next to player,
//	or gives info on an entity, or actually uses an entity
void HDBGame::setTargetXY(int x, int y) {
	// if ANY button is pressed
	if (_input->getButtons() || _ai->_playerEmerging)
		return;

	// Check if an entity is next to us
	x /= kTileWidth;
	y /= kTileHeight;

	// Don't ever allow going to X-coord 0
	if (!x)
		return;

	AIEntity *e = _ai->findEntity(x, y);
	AIEntity *p = _ai->getPlayer();

	if (!p)
		return;

	int px = p->x / kTileWidth;
	int py = p->y / kTileHeight;

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

	bool oneTileAway = (abs(px - x) + abs(py - y) < 2);

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

void HDBGame::startMoveMap(int x, int y) {
	_dx = x;
	_dy = y;
}

void HDBGame::moveMap(int x, int y) {
	int	ox, oy;
	g_hdb->_map->getMapXY(&ox, &oy);

	ox += (_dx - x) / 8;
	oy += (_dy - y) / 8;

	ox = CLIP(ox, 0, g_hdb->_map->mapPixelWidth() - 240);
	oy = CLIP(oy, 0, g_hdb->_map->mapPixelHeight() - 320);

	g_hdb->_map->setMapXY(ox, oy);
}

// PLAYER is trying to use this entity
void HDBGame::useEntity(AIEntity *e) {
	AIEntity *p = _ai->getPlayer();
	// Check if entity is on same level or if its a stairtop
	if ((p->level != e->level) && !(_map->getMapBGTileFlags(p->tileX, p->tileY) & kFlagStairTop))
		return;

	bool added = false;

	AIEntity temp;
	if (_ai->getTableEnt(e->type)) {
		temp = *e;

		_ai->getItemSound(e->type);

		added = _ai->addToInventory(e);
		if (added) {
			e = &temp;

			if (temp.aiUse)
				temp.aiUse(&temp);

			if (temp.luaFuncUse[0])
				_lua->callFunction(temp.luaFuncUse, 0);
		}
	} else {
		// These should be run over or run through
		if (_ai->walkThroughEnt(e->type) || e->type == AI_NONE)
			return;

		if (e->aiUse)
			e->aiUse(e);

		if (e->luaFuncUse[0])
			_lua->callFunction(e->luaFuncUse, 0);
	}

	// PUSHING
	// If its a pushable object, push it. Unless it's in/on water.
	if (e->type == AI_CRATE || e->type == AI_LIGHTBARREL || e->type == AI_BOOMBARREL || e->type == AI_MAGIC_EGG || e->type == AI_ICE_BLOCK || e->type == AI_FROGSTATUE || e->type == AI_DIVERTER) {
		// if it's floating, don't touch!
		if (e->state >= STATE_FLOATING && e->state <= STATE_FLOATRIGHT) {
			g_hdb->_ai->lookAtEntity(e);
			g_hdb->_ai->animGrabbing();
			g_hdb->_window->openMessageBar("I can't lift that!", 1);
			return;
		}

		int xDir = 0;
		int yDir = 0;
		if (p->tileX > e->tileX)
			xDir = -2;
		else if (p->tileX < e->tileX)
			xDir = 2;

		if (p->tileY > e->tileY)
			yDir = -2;
		else if (p->tileY < e->tileY)
			yDir = 2;

		// no diagonals allowed!
		if (xDir && yDir)
			return;

		int chX = p->tileX + xDir;
		int chY = p->tileY + yDir;
		uint32 flags;
		// are we going to push this over a sliding surface? (ok)
		// are we going to push this into a blocking tile? (not ok)
		if (e->level == 2) {
			int	fg_flags = g_hdb->_map->getMapFGTileFlags(chX, chY);
			if (fg_flags & kFlagSolid) {
				g_hdb->_sound->playSound(SND_GUY_UHUH);
				g_hdb->_ai->lookAtXY(chX, chY);
				g_hdb->_ai->animGrabbing();
				return;
			}

			flags = g_hdb->_map->getMapBGTileFlags(chX, chY);
			if (((flags & kFlagSolid) == kFlagSolid) && !(fg_flags & kFlagGrating)) {
				g_hdb->_sound->playSound(SND_GUY_UHUH);
				g_hdb->_ai->lookAtXY(chX, chY);
				g_hdb->_ai->animGrabbing();
				return;
			}
		} else {
			flags = g_hdb->_map->getMapBGTileFlags(chX, chY);
			if (!(flags & kFlagSlide) && (flags & kFlagSolid)) {
				g_hdb->_sound->playSound(SND_GUY_UHUH);
				g_hdb->_ai->lookAtXY(chX, chY);
				g_hdb->_ai->animGrabbing();
				return;
			}
		}

		// are we going to push this up the stairs? (not ok)
		if (flags & kFlagStairBot) {
			flags = g_hdb->_map->getMapBGTileFlags(e->tileX, e->tileY);
			if (!(flags & kFlagStairTop)) {
				g_hdb->_ai->lookAtEntity(e);
				g_hdb->_ai->animGrabbing();
				g_hdb->_sound->playSound(SND_GUY_UHUH);
				return;
			}
		}

		// is player trying to push across a dangerous floor (where the player would be ON the floor after the push)?
		// don't allow it.
		flags = g_hdb->_map->getMapBGTileFlags(p->tileX + (xDir >> 1), p->tileY + (yDir >> 1));
		if (((flags & kFlagRadFloor) == kFlagRadFloor || (flags & kFlagPlasmaFloor) == kFlagPlasmaFloor) &&
			false == g_hdb->_ai->checkFloating(p->tileX + (xDir >> 1), p->tileY + (yDir >> 1))) {
			g_hdb->_ai->lookAtEntity(e);
			g_hdb->_ai->animGrabbing();
			g_hdb->_sound->playSound(SND_NOPUSH_SIZZLE);
			return;
		}

		// are we going to push this into a gem?
		// if it's a goodfairy, make it move!
		AIEntity *e2 = g_hdb->_ai->findEntityIgnore(chX, chY, &g_hdb->_ai->_dummyLaser);
		if (e2 && e2->type == ITEM_GEM_WHITE) {
			g_hdb->_ai->addAnimateTarget(e2->x, e2->y, 0, 3, ANIM_NORMAL, false, false, GEM_FLASH);
			g_hdb->_ai->removeEntity(e2);
			g_hdb->_sound->playSound(SND_BRIDGE_END);
			g_hdb->_ai->animGrabbing();
			return;
		}

		// if so, is it a MELTED or FLOATING entity?  if so, that's cool...
		if (e2) {
			if (!g_hdb->_ai->checkFloating(e2->tileX, e2->tileY)) {
				g_hdb->_ai->lookAtXY(chX, chY);
				g_hdb->_ai->animGrabbing();
				g_hdb->_sound->playSound(SND_GUY_UHUH);
				return;
			}
		}

		// are we trying to push this through a door? (teleporter!)
		SingleTele info;
		if (true == g_hdb->_ai->findTeleporterDest(chX, chY, &info)) {
			g_hdb->_ai->lookAtXY(chX, chY);
			g_hdb->_ai->animGrabbing();
			g_hdb->_sound->playSound(SND_GUY_UHUH);
			return;
		}

		// everything's clear - time to push!
		// set goal for pushed object
		if (e->type != AI_DIVERTER)
			e->moveSpeed = kPushMoveSpeed;	// push DIVERTERS real fast
		g_hdb->_ai->setEntityGoal(e, chX, chY);

		// Diverters are very special - don't mess with their direction & state!
		if (e->type == AI_DIVERTER) {
			switch (e->dir2) {
			case DIR_DOWN:
				e->state = STATE_DIVERTER_BL;
				break;
			case DIR_UP:
				e->state = STATE_DIVERTER_BR;
				break;
			case DIR_LEFT:
				e->state = STATE_DIVERTER_TL;
				break;
			case DIR_RIGHT:
				e->state = STATE_DIVERTER_TR;
				break;
			case DIR_NONE:
			default:
				break;
			}
		}

		// set goal for player
		if (xDir)
			xDir = xDir >> 1;
		if (yDir)
			yDir = yDir >> 1;
		if (e->type != AI_DIVERTER)			// push DIVERTERS real fast
			p->moveSpeed = kPushMoveSpeed;
		else
			p->moveSpeed = kPlayerMoveSpeed;

		g_hdb->_ai->setEntityGoal(p, p->tileX + xDir, p->tileY + yDir);

		// need to set the state AFTER the SetEntityGoal!
		switch (p->dir) {
		case DIR_UP:
			p->state = STATE_PUSHUP;
			p->drawYOff = -10;
			break;
		case DIR_DOWN:
			p->state = STATE_PUSHDOWN;
			p->drawYOff = 9;
			break;
		case DIR_LEFT:
			p->state = STATE_PUSHLEFT;
			p->drawXOff = -10;
			break;
		case DIR_RIGHT:
			p->state = STATE_PUSHRIGHT;
			p->drawXOff = 10;
			break;
		case DIR_NONE:
		default:
			break;
		}

		// if player is running, keep speed slow since we're pushing
		if (g_hdb->_ai->playerRunning()) {
			p->xVel = p->xVel >> 1;
			p->yVel = p->yVel >> 1;
		}

		switch (e->type) {
		case AI_CRATE:
			g_hdb->_sound->playSound(SND_CRATE_SLIDE);
			break;
		case AI_LIGHTBARREL:
		case AI_FROGSTATUE:
		case AI_ICE_BLOCK:
			g_hdb->_sound->playSound(SND_LIGHT_SLIDE);
			break;
		case AI_HEAVYBARREL:
		case AI_MAGIC_EGG:
		case AI_BOOMBARREL:
			g_hdb->_sound->playSound(SND_HEAVY_SLIDE);
			break;
		case AI_DIVERTER:
			g_hdb->_sound->playSound(SND_PUSH_DIVERTER);
			break;
		default:
			break;
		}

		return;
	}

	// Look at Entity
	if (e->type != AI_RAILRIDER_ON)
		_ai->lookAtEntity(e);

	// Grab animation
	if (added)
		_ai->animGrabbing();

	// Can't push it - make a sound
	if (e->type == AI_HEAVYBARREL)
		g_hdb->_sound->playSound(SND_GUY_UHUH);
}

void HDBGame::setupProgressBar(int maxCount) {
	_progressMax = maxCount;
	_progressCurrent = 0;
	_progressActive = true;
}

void HDBGame::drawProgressBar() {
	if (!_progressActive)
		return;

	GameState temp = _gameState;
	_gameState = GAME_LOADING;
	paint();
	_gameState = temp;
}

void HDBGame::checkProgress() {
	if (!_progressActive)
		return;

	int x = _screenWidth / 2 - _progressGfx->_width / 2;
	_progressGfx->drawMasked(x, g_hdb->_progressY);
	for (int i = x; i < _progressXOffset; i += _progressMarkGfx->_width)
		_progressMarkGfx->drawMasked(i, g_hdb->_progressY);
	_progressMarkGfx->drawMasked(_progressXOffset, g_hdb->_progressY);
}

void HDBGame::drawLoadingScreen() {
	if (g_hdb->isPPC())
		_gfx->fillScreen(0);
	else
		_loadingScreenGfx->draw(0, 0);
}

struct MapName {
	const char *fName, *printName;
} static mapNames[] = {
	{	"MAP00",			"HDS Colby Jack" },
	{	"MAP01",			"Servandrones, Inc." },
	{	"MAP02",			"Pushbot Storage" },
	{	"MAP03",			"Rightbot Problems" },
	{	"MAP04",			"Shockbot Secrets" },
	{	"MAP05",			"The Drain Pain" },
	{	"MAP06",			"Energy Column Tower" },
	{	"MAP07",			"Water Supply Systems" },
	{	"MAP08",			"Food Supply Systems" },
	{	"MAP09",			"Purple Storage Room" },
	{	"MAP10",			"Back On The Jack" },
	{	"MAP11",			"Bridia" },
	{	"MAP12",			"BEAL Offices" },
	{	"MAP13",			"BEAL Labs" },
	{	"MAP14",			"Earthen Plain" },
	{	"MAP15",			"Fatfrog Swamp" },
	{	"MAP16",			"Fatfrog Deeps" },
	{	"MAP17",			"Glacier West" },
	{	"MAP18",			"Glacier East" },
	{	"MAP19",			"Mystery Pizza Factory" },
	{	"MAP20",			"Colby Jack Attack" },
	{	"MAP21",			"Pharitale" },
	{	"MAP22",			"Happy Meadow" },
	{	"MAP23",			"Water Caves" },
	{	"MAP24",			"Rocky Crag" },
	{	"MAP25",			"Dragon Deeps" },
	{	"MAP26",			"Lower Dragon Deeps" },
	{	"MAP27",			"Ice Dragon Valley" },
	{	"MAP28",			"Faerie Glade" },
	{	"MAP29",			"Palace In The Clouds" },
	{	"MAP30",			"Monkeystone Star Zone" },
};

void HDBGame::setInMapName(const char *name) {
	for (uint i = 0; i < ARRAYSIZE(mapNames); i++) {
		if (!scumm_stricmp(name, mapNames[i].fName)) {
			memset(&_inMapName, 0, 32);
			Common::strlcpy(_inMapName, mapNames[i].printName, 32);
			return;
		}
	}

	memset(&_inMapName, 0, 32);
	Common::strlcpy(_inMapName, name, 32);
}

Common::Error HDBGame::run() {

	// Initialize System
	if (!_systemInit)
		init();

	// Initializes Graphics
	initGraphics(_screenWidth, _screenHeight, &_format);

	start();

#if 0
	Common::SeekableReadStream *titleStream = _fileMan->findFirstData("monkeylogoscreen", TYPE_PIC);
	if (titleStream == nullptr) {
		debug("The TitleScreen MPC entry can't be found.");
		delete titleStream;
		return Common::kReadingFailed;
	}

	Picture *titlePic = new Picture;
	titlePic->load(titleStream);
	delete titleStream;

	Common::SeekableReadStream *tileStream = _fileMan->findFirstData("t32_ground1", TYPE_TILE32);
	if (tileStream == nullptr) {
		debug("The t32_shipwindow_lr MPC entry can't be found.");
		delete tileStream;
		return Common::kReadingFailed;
	}

	Tile *tile = new Tile;
	tile->load(tileStream);
	delete tileStream;
#endif

	if (ConfMan.hasKey("boot_param")) {
		int arg = ConfMan.getInt("boot_param");
		int actionMode = MIN(arg / 100, 1);
		int level = MIN(arg % 100, 31);

		setActionMode(actionMode);

		Common::String mapNameString = Common::String::format("MAP%02d", level);

		if (level > 30) {
			mapNameString = "CINE_OUTRO";
		}

		if (isDemo()) {
			mapNameString += "_DEMO";
		}

		debug("Starting level %s in %s Mode", mapNameString.c_str(), getActionMode() ? "Action" : "Puzzle");

		_ai->clearPersistent();
		startMap(mapNameString.c_str());

		_gameState = GAME_PLAY;
	} else {
		if (ConfMan.hasKey("save_slot") && loadGameState(ConfMan.getInt("save_slot")).getCode() == Common::kNoError)
			_gameState = GAME_PLAY;
	}

	//_window->openDialog("Sgt. Filibuster", 0, "You address me as 'sarge' or 'sergeant' or get your snappin' teeth kicked in! Got me?", 0, nullptr);

#if 0
	lua->executeFile("test.lua");
#endif

	uint32 lastTime = g_system->getMillis();
	while (!shouldQuit()) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				_input->updateMouse(event.mouse.x, event.mouse.y);
				break;
			case Common::EVENT_LBUTTONDOWN:
				_input->updateMouseButtons(true);
				break;
			case Common::EVENT_LBUTTONUP:
				_input->updateMouseButtons(false);
				break;
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				_input->updateActions(event, true, true);
				break;
			case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
				_input->updateActions(event, false, true);
				break;
			default:
				break;
			}
		}

		if (_gameState == GAME_PLAY) {
			_gfx->drawSky();

			if (!_pauseFlag) {
				_ai->moveEnts();
				_ai->animateBridges();
				_ai->processCallbackList();
			}

			_map->draw();
			_ai->processCines();
			//_window->drawDialog();

			AIEntity *e = _ai->getPlayer();

			if (e && e->level < 2)
				_ai->drawWayPoints();

			if (!(g_hdb->isDemo() && g_hdb->isPPC()))
				_map->drawEnts();

			_map->drawGratings();

			if (e && e->level == 2)
				_ai->drawWayPoints();

			_ai->drawLevel2Ents();

			_map->drawForegrounds();
			_ai->animateTargets();

			_window->drawDialog();
			_window->drawDialogChoice();
			_window->drawInventory();
			_window->drawMessageBar();
			_window->drawDeliveries();
			_window->drawTryAgain();
			_window->drawPanicZone();
			_window->drawTextOut();
			_window->drawPause();
			_gfx->drawBonusStars();
			_gfx->drawSnow();

			if (_changeLevel == true) {
				_changeLevel = false;
				startMap(_changeMapname);
			}

			//
			// should we save the game at this point?
			//
			if (_saveInfo.active == true) {
				_sound->playSound(SND_VORTEX_SAVE);
				_ai->stopEntity(e);
				_menu->fillSavegameSlots();
				saveGameState(_saveInfo.slot, "FIXME"); // Add here date/level name // TODO
				_saveInfo.active = false;
			}

			// calculate time spent ONLY in the game...
			_timePlayed += g_system->getMillis() - _tiempo;
		}

		// Update Timer that's NOT used for in-game Timing
		_prevTimeSlice = _timeSlice;
		_timeSlice = g_system->getMillis();

		paint();

		if (g_hdb->getDebug()) {
			g_hdb->_frames.push_back(g_system->getMillis());
			while (g_hdb->_frames[0] < g_system->getMillis() - 1000)
				g_hdb->_frames.remove_at(0);
		}
		uint32 curTime = g_system->getMillis();
		uint32 frameTime = curTime - lastTime;

		uint32 frameCap = 1000 / kGameFPS;
		if (frameTime < frameCap)
			g_system->delayMillis(frameCap - frameTime);

		lastTime = g_system->getMillis();
	}

	return Common::kNoError;
}

} // End of namespace HDB
