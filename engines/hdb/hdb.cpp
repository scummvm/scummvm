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
#include "hdb/menu.h"

#define CHEAT_PATCHES 0

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
	_menu = new Menu;
	_map = new Map;
	_ai = new AI;
	_input = new Input;
	_sound = new Sound;
	_window = new Window;
	_rnd = new Common::RandomSource("hdb");

	_currentMapname[0] = _currentLuaName[0] = 0;
	_lastMapname[0] = _lastLuaName[0] = 0;
	_inMapName[0] = 0;

	_monkeystone7 = STARS_MONKEYSTONE_7_FAKE;
	_monkeystone14 = STARS_MONKEYSTONE_14_FAKE;
	_monkeystone21 = STARS_MONKEYSTONE_21_FAKE;

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
	delete _sound;
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
	if (!_sound->init()) {
		error("Window::init: Couldn't initialize Sound");
	}
	if (!_ai->init()) {
		error("AI::init: Couldn't initialize AI");
	}
	if (!_window->init()) {
		error("Window::init: Couldn't initialize Window");
	}
	if (!_input->init()) {
		error("Input::init: Couldn't initialize Input");
	}
	if (!_lua->init()) {
		error("LuaScript::init: Couldn't load the GLOBAL.LUA code.");
	}

	_menu->init();

	_changeLevel = false;
	_changeMapname[0] = 0;
	_loadInfo.active = _saveInfo.active = false;

	_menu->startTitle();

	_gameShutdown = false;
	_pauseFlag = 0;
	_systemInit = true;

	return true;
}

void HDBGame::save(Common::OutSaveFile *out) {
	out->write(_currentMapname, 64);
	out->write(_lastMapname, 64);
	out->write(_currentLuaName, 64);
	out->writeSint32LE(_actionMode);
	out->writeByte(_changeLevel);
	out->write(_changeMapname, 64);
	out->write(_inMapName, 32);
}

void HDBGame::load(Common::InSaveFile *in) {
	in->read(_currentMapname, 64);
	in->read(_lastMapname, 64);
	in->read(_currentLuaName, 64);
	_actionMode = in->readSint32LE();
	_changeLevel = in->readByte();
	in->read(_changeMapname, 64);
	in->read(_inMapName, 32);
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

bool HDBGame::restartMap() {
	if (!_currentMapname[0])
		return false;

	_gfx->emptyGfxCaches();
	_lua->callFunction("level_shutdown", 0);

	_gfx->turnOffSnow();
	_window->restartSystem();
	_ai->restartSystem();
	_lua->init();
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

	return true;
}

bool HDBGame::startMap(const char *name) {
	// save last mapname
	strcpy(_lastMapname, _currentMapname);

	// set current mapname
	strcpy(_currentMapname, name);
	strcat(_currentMapname, ".MSM");

	// set current luaname
	strcpy(_currentLuaName, name );
	strcat(_currentLuaName, ".LUA");

	restartMap();

	//
	// here is where we will be autosaving the start of level
	// don't save cine intro/outro/etc...OR map30 (secret star map)
	//
	if (!scumm_strnicmp(name, "map", 3) && scumm_stricmp(name, "map30")) {
		_menu->fillSavegameSlots();
		saveSlot(0);          // we ignore the slot parameter in everything else since we just keep saving...
	}
	return true;
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

bool HDBGame::saveSlot(int slot) {

	// If no map is loaded, don't try to save
	if (!g_hdb->_map->isLoaded())
		return false;

	Common::OutSaveFile *out;

	Common::String saveFileName = Common::String::format("%s.%03d", _targetName.c_str(), slot);
	if (!(out = _saveFileMan->openForSaving(saveFileName)))
		error("Unable to open save file");

	warning("STUB: Save MetaData");
	Graphics::saveThumbnail(*out);

	// Actual Save Data
	saveGame(out);
	_lua->save(out, _targetName.c_str(), slot);

	out->finalize();
	if (out->err())
		warning("Can't write file '%s'. (Disk full?)", saveFileName.c_str());

	delete out;

	return true;
}

bool HDBGame::loadSlot(int slot) {
	Common::InSaveFile *in;

	Common::String saveFileName = Common::String::format("%s.%03d", _targetName.c_str(), slot);
	if (!(in = _saveFileMan->openForLoading(saveFileName))) {
		warning("missing savegame file %s", saveFileName.c_str());
		if (g_hdb->_map->isLoaded())
			g_hdb->setGameState(GAME_PLAY);
		return false;
	}

	warning("STUB: Load MetaData");
	Graphics::skipThumbnail(*in);

	// Actual Save Data
	loadGame(in);

	delete in;

	return true;
}

void HDBGame::saveGame(Common::OutSaveFile *out) {

	// Save Map Name
	out->write(_inMapName, 32);

	// Save Map Object Data
	_map->save(out);

	// Save Window Object Data
	_window->save(out);

	// Save Gfx Object Data
	_gfx->save(out);

	// Save Sound Object Data
	_sound->save(out);

	// Save Game Object Data
	save(out);

	// Save AI Object Data

	_ai->save(out);

}

void HDBGame::loadGame(Common::InSaveFile *in) {
	warning("STUB: Add loadGame()");
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

	// PUSHING
	// If its a pushable object, push it. Unless it's in/on water.
	if (e->type == AI_CRATE || e->type == AI_LIGHTBARREL || e->type == AI_BOOMBARREL || e->type == AI_MAGIC_EGG || e->type == AI_ICE_BLOCK || e->type == AI_FROGSTATUE || e->type == AI_DIVERTER) {
		int	xDir, yDir, chX, chY;
		uint32 flags;
		AIEntity *e2;

		// if it's floating, don't touch!
		if (e->state >= STATE_FLOATING && e->state <= STATE_FLOATRIGHT) {
			g_hdb->_ai->lookAtEntity(e);
			g_hdb->_ai->animGrabbing();
			g_hdb->_window->openMessageBar("I can't lift that!", 1);
			return;
		}

		xDir = yDir = 0;
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

		chX = p->tileX + xDir;
		chY = p->tileY + yDir;

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
		e2 = g_hdb->_ai->findEntityIgnore(chX, chY, &g_hdb->_ai->_dummyLaser);
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
			case DIR_DOWN: e->state = STATE_DIVERTER_BL; break;
			case DIR_UP: e->state = STATE_DIVERTER_BR; break;
			case DIR_LEFT: e->state = STATE_DIVERTER_TL; break;
			case DIR_RIGHT: e->state = STATE_DIVERTER_TR; break;
			case DIR_NONE: break;
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
		case DIR_UP:	p->state = STATE_PUSHUP;	p->drawYOff = -10; break;
		case DIR_DOWN:	p->state = STATE_PUSHDOWN;	p->drawYOff = 9; break;
		case DIR_LEFT:	p->state = STATE_PUSHLEFT;	p->drawXOff = -10; break;
		case DIR_RIGHT:	p->state = STATE_PUSHRIGHT; p->drawXOff = 10; break;
		case DIR_NONE: break;
		}

		// if player is running, keep speed slow since we're pushing
		if (g_hdb->_ai->playerRunning()) {
			p->xVel = p->xVel >> 1;
			p->yVel = p->yVel >> 1;
		}

		switch (e->type) {
		case AI_CRATE:
			g_hdb->_sound->playSound(SND_CRATE_SLIDE); break;
		case AI_LIGHTBARREL:
		case AI_FROGSTATUE:
		case AI_ICE_BLOCK:
			g_hdb->_sound->playSound(SND_LIGHT_SLIDE); break;
		case AI_HEAVYBARREL:
		case AI_MAGIC_EGG:
		case AI_BOOMBARREL:
			g_hdb->_sound->playSound(SND_HEAVY_SLIDE); break;
		case AI_DIVERTER:
			g_hdb->_sound->playSound(SND_PUSH_DIVERTER); break;
		default:
			break;
		}

		return;
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
		g_hdb->_sound->playSound(SND_GUY_UHUH);
	}
}

struct {
	const char *fName, *printName;
} mapNames[] = {
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
	int i = 0;

	while (mapNames[i].fName) {
		if (!scumm_stricmp(name, mapNames[i].fName)) {
			memset(&_inMapName, 0, 32);
			strcpy(_inMapName, mapNames[i].printName);
			return;
		}
		i++;
	}

	memset(&_inMapName, 0, 32);
	strcpy(_inMapName, name);
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

	if (ConfMan.hasKey("boot_param")) {
		char mapname[10];
		int level = ConfMan.getInt("boot_param");

		if (level > 30 || level < 0)
			level = 1;

		snprintf(mapname, 10, "MAP%02d", level);

		startMap(mapname);
	} else {
		startMap("MAP00");
	}

	//_window->openDialog("Sgt. Filibuster", 0, "You address me as 'sarge' or 'sergeant' or get your snappin' teeth kicked in! Got me?", 0, NULL);

#if 0
	lua->executeFile("test.lua");
#endif

	AIEntity *e;

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

			e = _ai->getPlayer();

			if (e && e->level < 2)
				_ai->drawWayPoints();

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
			_window->drawTextOut();
			_window->drawPause();

			//_gfx->drawBonusStars();
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
				saveSlot(_saveInfo.slot);
				_saveInfo.active = false;
			}
		}

		// Update Timer that's NOT used for in-game Timing
		_prevTimeSlice = _timeSlice;
		_timeSlice = g_system->getMillis();

		paint();

		g_system->updateScreen();
		g_system->delayMillis(1000 / kGameFPS);
	}

	return Common::kNoError;
}

} // End of namespace HDB
