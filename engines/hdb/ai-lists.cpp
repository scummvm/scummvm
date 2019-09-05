/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can Redistribute it and/or
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

#include "hdb/hdb.h"
#include "hdb/ai.h"
#include "hdb/ai-player.h"
#include "hdb/file-manager.h"
#include "hdb/gfx.h"
#include "hdb/lua-script.h"
#include "hdb/map.h"
#include "hdb/sound.h"
#include "hdb/window.h"

namespace HDB {

/*
	Adds a tile to an animation list
*/
void AI::addAnimateTarget(int x, int y, int start, int end, AnimSpeed speed, bool killAuto, bool inMap, const char *tileName) {
	AnimTarget *at = new AnimTarget;

	at->x = x;
	at->y = y;
	at->start = start;
	at->end = end;
	at->killAuto = killAuto; // Doesn't actually kill it, deactivates it

	// Set animCycle and animFrame as per speed
	switch (speed) {
	case ANIM_SLOW:
		at->animCycle = 10;
		at->animFrame = 10;
		break;
	case ANIM_NORMAL:
		at->animCycle = 6;
		at->animFrame = 6;
		break;
	case ANIM_FAST:
		at->animCycle = 2;
		at->animFrame = 2;
		break;
	}

	// Set +1/-1 for tile anim direction
	if ((end - start) > 0)
		at->vel = 1;
	else
		at->vel = -1;

	// Set Info if this is not an inMap animation
	at->inMap = inMap;
	if (!inMap) {
		char name[32];
		for (int i = start; i <= end; i++) {
			if (i < 10)
				snprintf(name, 32, "%s0%d", tileName, i + 1);
			else
				snprintf(name, 32, "%s%d", tileName, i + 1);
			uint32 size = g_hdb->_fileMan->getLength(name, TYPE_TILE32);
			at->gfxList[i] = g_hdb->_gfx->getTileGfx(name, size);
		}
	}

	// Insert in the beginning
	_animTargets.insert_at(0, at);
}

/*
	Animate _animTargets
	Called every frame
*/
void AI::animateTargets() {
	int mx, my;
	g_hdb->_map->getMapXY(&mx, &my);

	for (uint i = 0; i < _animTargets.size(); i++) {
		AnimTarget *at = _animTargets[i];
		debug(9, "AnimTarget #%i: at: at->x: %d, at->y: %d, at->start: %d, at->end: %d, at->vel: %d", i, at->x, at->y, at->start, at->end, at->vel);

		// Draw Non-map stuff every frame
		if (!at->inMap)
			// FIXME: Out of bounds reference to gfxList
			at->gfxList[at->start]->drawMasked(at->x - mx, at->y - my);

		// Frame Timer
		if (at->animFrame-- < 1) {
			at->animFrame = at->animCycle;

			if (at->inMap) {
				// Animate Map Tiles
				int layer = 0; // BG layer
				if (!(at->start == g_hdb->_map->getMapBGTileIndex(at->x, at->y)))
					layer = 1;

				// Change Tile Anim
				at->start += at->vel;

				// Set it back in map
				if (!layer)
					g_hdb->_map->setMapBGTileIndex(at->x, at->y, at->start);
				else
					g_hdb->_map->setMapFGTileIndex(at->x, at->y, at->start);
			} else {
				// Animate non-map tiles
				at->start++;
			}

			// Animation Finished ?
			if (at->start == at->end) {

				if (at->killAuto)
					autoDeactivate(at->x, at->y);

				delete _animTargets[i];
				_animTargets.remove_at(i);
				i--;
				continue;
			}
		}
	}
}

void AI::addBridgeExtend(int x, int y, int bridgeType) {
	if (_numBridges >= kMaxBridges)
		return;

	if (bridgeType == _targetBridgeU)
		_bridges[_numBridges].dir = DIR_UP;
	else if (bridgeType == _targetBridgeD)
		_bridges[_numBridges].dir = DIR_DOWN;
	else if (bridgeType == _targetBridgeL)
		_bridges[_numBridges].dir = DIR_LEFT;
	else if (bridgeType == _targetBridgeR)
		_bridges[_numBridges].dir = DIR_RIGHT;

	_bridges[_numBridges].delay = 5;
	_bridges[_numBridges].x = x;
	_bridges[_numBridges].y = y;
	_bridges[_numBridges].anim = 0;

	if (g_hdb->_map->onScreen(_bridges[_numBridges].x, _bridges[_numBridges].y))
		g_hdb->_sound->playSound(SND_BRIDGE_START);

	_numBridges++;
}

void AI::animateBridges() {
	// out quick!
	if (!_numBridges)
		return;

	for (int i = 0; i < _numBridges; i++) {
		if (_bridges[i].delay-- > 0)
			continue;

		_bridges[i].delay = 5;
		bool done = false;
		int xv = 0;
		int yv = 0;

		switch (_bridges[i].dir) {
		case DIR_UP:
			g_hdb->_map->setMapFGTileIndex(_bridges[i].x, _bridges[i].y, _targetBridgeU + _bridges[i].anim);
			_bridges[i].anim++;
			if (_bridges[i].anim > 2) {
				g_hdb->_map->setMapFGTileIndex(_bridges[i].x, _bridges[i].y, _targetBridgeMidUD);
				yv = -1;
				done = true;
			}
			break;
		case DIR_DOWN:
			g_hdb->_map->setMapFGTileIndex(_bridges[i].x, _bridges[i].y, _targetBridgeD + _bridges[i].anim);
			_bridges[i].anim++;
			if (_bridges[i].anim > 2) {
				g_hdb->_map->setMapFGTileIndex(_bridges[i].x, _bridges[i].y, _targetBridgeMidUD);
				yv = 1;
				done = true;
			}
			break;
		case DIR_LEFT:
			g_hdb->_map->setMapFGTileIndex(_bridges[i].x, _bridges[i].y, _targetBridgeL + _bridges[i].anim);
			_bridges[i].anim++;
			if (_bridges[i].anim > 2) {
				g_hdb->_map->setMapFGTileIndex(_bridges[i].x, _bridges[i].y, _targetBridgeMidLR);
				xv = -1;
				done = true;
			}
			break;
		case DIR_RIGHT:
			g_hdb->_map->setMapFGTileIndex(_bridges[i].x, _bridges[i].y, _targetBridgeR + _bridges[i].anim);
			_bridges[i].anim++;
			if (_bridges[i].anim > 2) {
				g_hdb->_map->setMapFGTileIndex(_bridges[i].x, _bridges[i].y, _targetBridgeMidLR);
				xv = 1;
				done = true;
			}
			break;
		case DIR_NONE:
			break;
		}

		// is this bridge done extending one chunk?
		if (done) {
			if (g_hdb->_map->onScreen(_bridges[i].x, _bridges[i].y))
				g_hdb->_sound->playSound(SND_BRIDGE_EXTEND);
			_bridges[i].anim = 0;
			_bridges[i].x += xv;
			_bridges[i].y += yv;
			int tileIndex = g_hdb->_map->getMapFGTileIndex(_bridges[i].x, _bridges[i].y);
			uint32 flags = g_hdb->_map->getMapBGTileFlags(_bridges[i].x, _bridges[i].y);
			if (!flags || (flags & kFlagMetal) || tileIndex >= 0 || (flags & kFlagSolid)) {
				if (g_hdb->_map->onScreen(_bridges[i].x, _bridges[i].y))
					g_hdb->_sound->playSound(SND_BRIDGE_END);
				// TODO: CHECKME - Using i as an index looks very wrong as the for statement uses j.
				// This results in copying multiple times the same data
				for (int j = 0; j < _numBridges - 1; j++)
					memcpy(&_bridges[i], &_bridges[i + 1], sizeof(Bridge));
				_numBridges--;
			}
		}
	}
}

void AI::addToFairystones(int index, int tileX, int tileY, int sourceOrDest) {
	if (!sourceOrDest) {
		_fairystones[index].srcX = tileX;
		_fairystones[index].srcY = tileY;
	} else {
		_fairystones[index].destX = tileX;
		_fairystones[index].destY = tileY;
	}
}

int AI::checkFairystones(int tileX, int tileY) {
	for (int i = 0; i < kMaxFairystones; i++) {
		if (_fairystones[i].destX == tileX && _fairystones[i].destY == tileY)
			return i;
	}
	return -1;
}

// Add an action location to the list of possible actions
// Each action must be paired with another of the same number
void AI::addToActionList(int actionIndex, int x, int y, char *luaFuncInit, char *luaFuncUse) {
	if (!_actions[actionIndex].x1) {
		_actions[actionIndex].x1 = x;
		_actions[actionIndex].y1 = y;
		if (luaFuncInit[0] != '*')
			strcpy(_actions[actionIndex].luaFuncInit, luaFuncInit);
		if (luaFuncUse[0] != '*')
			strcpy(_actions[actionIndex].luaFuncUse, luaFuncUse);

		if (_actions[actionIndex].luaFuncInit[0]) {
			g_hdb->_lua->callFunction(_actions[actionIndex].luaFuncInit, 2);
			strcpy(_actions[actionIndex].entityName, g_hdb->_lua->getStringOffStack());
			strcpy(_actions[actionIndex].entityName, g_hdb->_lua->getStringOffStack());
		}
		return;
	}

	if (!_actions[actionIndex].x2) {
		_actions[actionIndex].x2 = x;
		_actions[actionIndex].y2 = y;
		if (luaFuncInit[0] != '*')
			strcpy(_actions[actionIndex].luaFuncInit, luaFuncInit);
		if (luaFuncUse[0] != '*')
			strcpy(_actions[actionIndex].luaFuncUse, luaFuncUse);

		if (_actions[actionIndex].luaFuncInit[0]) {
			g_hdb->_lua->callFunction(_actions[actionIndex].luaFuncInit, 2);
			strcpy(_actions[actionIndex].entityName, g_hdb->_lua->getStringOffStack());
			strcpy(_actions[actionIndex].entityName, g_hdb->_lua->getStringOffStack());
		}
		return;
	}

	warning("Adding a 3rd action to ACTION-%d is illegal", actionIndex);
}

// Checks if the location passed-in matches an action pair.
// If so, activate it if possible. Returns TRUE for finding pair.
bool AI::checkActionList(AIEntity *e, int x, int y, bool lookAndGrab) {
	for (int i = 0; i < kMaxActions; i++) {
		if ((_actions[i].x1 == x && _actions[i].y1 == y) || (_actions[i].x2 == x && _actions[i].y2 == y)) {
			int targetX = _actions[i].x2;
			int targetY = _actions[i].y2;
			bool success;

			// Choose target co-ordinates
			if (x == targetX && y == targetY) {
				targetX = _actions[i].x1;
				targetY = _actions[i].y1;
			}

			// Is this an actual switch?
			uint32 flags = g_hdb->_map->getMapFGTileFlags(x, y);
			if (!flags)
				flags = g_hdb->_map->getMapBGTileFlags(x, y);
			if (!(flags & kFlagSolid) && (_player->tileX != x && _player->tileY != y))
				return false;
			// Closing on something?
			if (findEntity(targetX, targetY))
				return false;

			success = activateAction(e, x, y, targetX, targetY);

			// If successful, remove action from list
			if (success) {
				_actions[i].x1 = _actions[i].y1 = _actions[i].x2 = _actions[i].y2 = 0;

				// Call Lua Use function if it exists
				if (_actions[i].luaFuncUse[0]) {
					g_hdb->_lua->callFunction(_actions[i].luaFuncUse, 0);
				}

			} else if (e == _player && !checkForTouchplate(x, y))
				addWaypoint(e->tileX, e->tileY, x, y, e->level);

			if (lookAndGrab && e == _player) {
				lookAtXY(x, y);
				animGrabbing();
			}

			return true;
		}
	}

	return false;
}

void AI::addToHereList(const char *entName, int x, int y) {
	HereT *h = new HereT;
	strcpy(h->entName, entName);
	h->x = x;
	h->y = y;
	_hereList->push_back(h);
}

HereT *AI::findHere(int x, int y) {
	for (Common::Array<HereT *>::iterator it = _hereList->begin(); it != _hereList->end(); ++it) {
		if ((*it)->x == x && (*it)->y == y)
			return *it;
	}
	return NULL;
}

void AI::addToAutoList(int x, int y, const char *luaFuncInit, const char *luaFuncUse) {
	for (int i = 0; i < kMaxAutoActions; i++) {
		if (!_autoActions[i].x) {
			_autoActions[i].x = x;
			_autoActions[i].y = y;
			_autoActions[i].activated = false;
			if (luaFuncInit[0] != '*')
				strcpy(&_autoActions[i].luaFuncInit[0], luaFuncInit);
			if (luaFuncUse[0] != '*')
				strcpy(&_autoActions[i].luaFuncUse[0], luaFuncUse);

			if (_autoActions[i].luaFuncInit[0]) {
				g_hdb->_lua->callFunction(_autoActions[i].luaFuncInit, 2);
				const char *get = g_hdb->_lua->getStringOffStack();
				if (!get)
					return;
				strcpy(&_autoActions[i].entityName[0], get);
				get = g_hdb->_lua->getStringOffStack();
				if (!get)
					return;
				strcpy(&_autoActions[i].entityName[0], get);
			}
			return;
		}
	}
}

void AI::autoDeactivate(int x, int y) {
	for (int i = 0; i < kMaxAutoActions; i++) {
		if (_autoActions[i].x == x && _autoActions[i].y == y) {
			_autoActions[i].activated = false;
			return;
		}
	}
}

bool AI::activateAction(AIEntity *e, int x, int y, int targetX, int targetY) {
	bool success = false;
	int tileIndex = g_hdb->_map->getMapFGTileIndex(x, y);

	// If FG tile invisivle or grating, ignore if
	int fgFlags = g_hdb->_map->getMapFGTileFlags(x, y);
	if (fgFlags & (kFlagInvisible | kFlagGrating))
		tileIndex = -1;

	if (tileIndex < 0)
		tileIndex = g_hdb->_map->getMapBGTileIndex(x, y);

	// Check which tile is going to activate
	if (tileIndex == _useSwitchOff || tileIndex == _useSwitchOff + 1)
		success = useSwitch(e, x, y, targetX, targetY, _useSwitchOn);
	else if (tileIndex == _useSwitchOn)
		success = useSwitchOn(e, x, y, targetX, targetY, _useSwitchOff);
	//-------------------------------------------------------------------
	else if (tileIndex == _useHandswitchOff || tileIndex == _useHandswitchOff + 1)
		success = useSwitch(e, x, y, targetX, targetY, _useHandswitchOn);
	else if (tileIndex == _useHandswitchOn)
		success = useSwitchOn(e, x, y, targetX, targetY, _useHandswitchOff);
	//-------------------------------------------------------------------
	else if (tileIndex == _kcHolderWhiteOff || tileIndex == _kcHolderWhiteOff + 1)
		success = useLockedSwitch(e, x, y, targetX, targetY, _kcHolderWhiteOn, ITEM_KEYCARD_WHITE, "I need a White Keycard.");
	else if (tileIndex == _kcHolderWhiteOn)
		success = useLockedSwitchOn(e, x, y, targetX, targetY, _kcHolderWhiteOff, ITEM_KEYCARD_WHITE);
	else if (tileIndex == _kcHolderBlueOff || tileIndex == _kcHolderBlueOff + 1)
		success = useLockedSwitch(e, x, y, targetX, targetY, _kcHolderBlueOn, ITEM_KEYCARD_BLUE, "I need a Blue Keycard.");
	else if (tileIndex == _kcHolderBlueOn)
		success = useLockedSwitchOn(e, x, y, targetX, targetY, _kcHolderBlueOff, ITEM_KEYCARD_BLUE);
	else if (tileIndex == _kcHolderRedOff || tileIndex == _kcHolderRedOff + 1)
		success = useLockedSwitch(e, x, y, targetX, targetY, _kcHolderRedOn, ITEM_KEYCARD_RED, "I need a Red Keycard.");
	else if (tileIndex == _kcHolderRedOn)
		success = useLockedSwitchOn(e, x, y, targetX, targetY, _kcHolderRedOff, ITEM_KEYCARD_RED);
	else if (tileIndex == _kcHolderGreenOff || tileIndex == _kcHolderGreenOff + 1)
		success = useLockedSwitch(e, x, y, targetX, targetY, _kcHolderGreenOn, ITEM_KEYCARD_GREEN, "I need a Green Keycard.");
	else if (tileIndex == _kcHolderGreenOn)
		success = useLockedSwitchOn(e, x, y, targetX, targetY, _kcHolderGreenOff, ITEM_KEYCARD_GREEN);
	else if (tileIndex == _kcHolderPurpleOff || tileIndex == _kcHolderPurpleOff + 1)
		success = useLockedSwitch(e, x, y, targetX, targetY, _kcHolderPurpleOn, ITEM_KEYCARD_PURPLE, "I need a Purple Keycard.");
	else if (tileIndex == _kcHolderPurpleOn)
		success = useLockedSwitchOn(e, x, y, targetX, targetY, _kcHolderPurpleOff, ITEM_KEYCARD_PURPLE);
	else if (tileIndex == _kcHolderBlackOff || tileIndex == _kcHolderBlackOff + 1)
		success = useLockedSwitch(e, x, y, targetX, targetY, _kcHolderBlackOn, ITEM_KEYCARD_BLACK, "I need a Black Keycard.");
	else if (tileIndex == _kcHolderBlackOn)
		success = useLockedSwitchOn(e, x, y, targetX, targetY, _kcHolderBlackOff, ITEM_KEYCARD_BLACK);
	//-------------------------------------------------------------------
	else if (tileIndex == _useSwitch2Off || tileIndex == _useSwitch2Off + 1)
		success = useSwitch2(e, x, y, targetX, targetY);
	else if (tileIndex == _useHolderEmpty || tileIndex == _useHolderEmpty + 1)
		success = useCellHolder(e, x, y, targetX, targetY);
	//-------------------------------------------------------------------
	else if (tileIndex == _targetDoorN || tileIndex == _targetDoorN + 3)
		success = useAutoDoorOpenClose(e, x, y);
	else if (tileIndex == _targetDoorP || tileIndex == _targetDoorP + 3)
		success = useDoorOpenCloseBot(e, x, y);
	else if (tileIndex == _targetDoorS || tileIndex == _targetDoorS + 3)
		success = useDoorOpenCloseBot(e, x, y);
	else if (tileIndex == _targetDoorNv || tileIndex == _targetDoorNv + 3)
		success = useAutoDoorOpenClose(e, x, y);
	else if (tileIndex == _targetDoorPv || tileIndex == _targetDoorPv + 3)
		success = useDoorOpenCloseBot(e, x, y);
	else if (tileIndex == _targetDoorSv || tileIndex == _targetDoorSv + 3)
		success = useDoorOpenCloseBot(e, x, y);
	//-------------------------------------------------------------------
	else if (tileIndex == _targetDoorN || tileIndex == _targetDoor2N + 3)
		success = useAutoDoorOpenClose(e, x, y);
	else if (tileIndex == _targetDoorP || tileIndex == _targetDoor2P + 3)
		success = useDoorOpenCloseBot(e, x, y);
	else if (tileIndex == _targetDoorS || tileIndex == _targetDoor2S + 3)
		success = useDoorOpenCloseBot(e, x, y);
	else if (tileIndex == _targetDoorNv || tileIndex == _targetDoor2Nv + 3)
		success = useAutoDoorOpenClose(e, x, y);
	else if (tileIndex == _targetDoorPv || tileIndex == _targetDoor2Pv + 3)
		success = useDoorOpenCloseBot(e, x, y);
	else if (tileIndex == _targetDoorSv || tileIndex == _targetDoor2Sv + 3)
		success = useDoorOpenCloseBot(e, x, y);
	//-------------------------------------------------------------------
	else if (tileIndex == _touchplateOff)
		success = useTouchplate(e, x, y, targetX, targetY, _touchplateOn);
	else if (tileIndex == _touchplateOn)
		success = useTouchplateOn(e, x, y, targetX, targetY, _touchplateOff);
	else if (tileIndex == _templeTouchpOff)
		success = useTouchplate(e, x, y, targetX, targetY, _templeTouchpOn);
	else if (tileIndex == _templeTouchpOn)
		success = useTouchplateOn(e, x, y, targetX, targetY, _templeTouchpOff);

	return success;
}

bool AI::checkAutoList(AIEntity *e, int x, int y) {
	for (int i = 0; i < kMaxAutoActions; i++) {
		if (_autoActions[i].x == x && _autoActions[i].y == y && !_autoActions[i].activated) {
			debug(1, "Activating action for Entity: %s, x: %d, y: %d", e->entityName, x, y);
			bool success = activateAction(e, x, y, 0, 0);
			_autoActions[i].activated = true;

			if (success && _autoActions[i].luaFuncUse[0])
				g_hdb->_lua->callFunction(_autoActions[i].luaFuncUse, 0);

			if (e == _player) {
				lookAtXY(x, y);
				animGrabbing();
			}

			return true;
		}
	}
	return false;
}

bool AI::autoActive(int x, int y) {
	for (int i = 0; i < kMaxAutoActions; i++) {
		if (_autoActions[i].x == x && _autoActions[i].y == y) {
			if (!_autoActions[i].activated)
				return false;
			return true;
		}
	}
	return false;
}

CallbackDef allCallbacks[] = {
	{NO_FUNCTION, NULL},
	{AI_BARREL_EXPLOSION_END, aiBarrelExplosionEnd},
	{CALLBACK_DOOR_OPEN_CLOSE, callbackDoorOpenClose},
	{CALLBACK_AUTODOOR_OPEN_CLOSE, callbackAutoDoorOpenClose},
	{CALLBACK_END, NULL}
};

void AI::addCallback(CallbackType type, int x, int y, int delay) {
	for (int i = kMaxCallbacks - 1; i >= 0; i--)
		if (_callbacks[i].type == NO_FUNCTION) {
			_callbacks[i].type = type;
			_callbacks[i].x = x;
			_callbacks[i].y = y;
			_callbacks[i].delay = delay;
			return;
		}
}

void AI::processCallbackList() {
	for (int i = 0; i < kMaxCallbacks; i++)
		if (_callbacks[i].type != NO_FUNCTION) {
			if (_callbacks[i].delay) {
				_callbacks[i].delay--;
				return;
			}
			allCallbacks[_callbacks[i].type].function(_callbacks[i].x, _callbacks[i].y);
			_callbacks[i].type = NO_FUNCTION;
			_callbacks[i].x = _callbacks[i].y = 0;
			return;
		}
}

void AI::addToLuaList(int x, int y, int value1, int value2, char *luaFuncInit, char *luaFuncAction, char *luaFuncUse) {
	for (int i = 0; i < kMaxLuaEnts; i++) {
		if (!_luaList[i].luaFuncInit[0] && !_luaList[i].luaFuncAction[0] && !_luaList[i].luaFuncUse[0]) {
			_luaList[i].x = x;
			_luaList[i].y = y;
			_luaList[i].value1 = value1;
			_luaList[i].value2 = value2;

			strcpy(_luaList[i].luaFuncInit, luaFuncInit);
			if (luaFuncInit[0] == '*')
				_luaList[i].luaFuncInit[0] = 0;
			strcpy(_luaList[i].luaFuncAction, luaFuncAction);
			if (luaFuncAction[0] == '*')
				_luaList[i].luaFuncAction[0] = 0;
			strcpy(_luaList[i].luaFuncUse, luaFuncUse);
			if (luaFuncUse[0] == '*')
				_luaList[i].luaFuncUse[0] = 0;

			_numLuaList++;
			if (_luaList[i].luaFuncInit[0])
				g_hdb->_lua->invokeLuaFunction(luaFuncInit, x, y, value1, value2);

			spawnBlocking(x, y, 1);

			return;
		}
	}
}

bool AI::checkLuaList(AIEntity *e, int x, int y) {
	for (int i = 0; i < _numLuaList; i++) {
		if (_luaList[i].x == x && _luaList[i].y == y && _luaList[i].luaFuncUse[0]) {
			if (e == _player) {
				lookAtXY(x, y);
				animGrabbing();
			}

			g_hdb->_lua->invokeLuaFunction(_luaList[i].luaFuncUse, _luaList[i].x, _luaList[i].y, _luaList[i].value1, _luaList[i].value2);

			return true;
		}
	}

	return false;
}

bool AI::luaExistAtXY(int x, int y) {
	for (int i = 0; i < _numLuaList; i++) {
		if (_luaList[i].x == x && _luaList[i].y == y && _luaList[i].luaFuncUse[0]) {
			return true;
		}
	}

	return false;
}

void AI::addToTeleportList(int teleIndex, int x, int y, int dir, int level, int anim, int usable, const char *luaFuncUse) {
	if (!level)
		level = 1;

	if (!_teleporters[teleIndex].x1) {
		_teleporters[teleIndex].x1 = x;
		_teleporters[teleIndex].y1 = y;
		_teleporters[teleIndex].dir1 = (AIDir)dir;
		_teleporters[teleIndex].level1 = level;
		_teleporters[teleIndex].anim1 = anim;
		_teleporters[teleIndex].usable1 = usable;
		strcpy(_teleporters[teleIndex].luaFuncUse1, luaFuncUse);
		if (_teleporters[teleIndex].luaFuncUse1[0] == '*')
			_teleporters[teleIndex].luaFuncUse1[0] = 0;
		_numTeleporters++;
		return;
	}
	if (!_teleporters[teleIndex].x2) {
		_teleporters[teleIndex].x2 = x;
		_teleporters[teleIndex].y2 = y;
		_teleporters[teleIndex].dir2 = (AIDir)dir;
		_teleporters[teleIndex].level2 = level;
		_teleporters[teleIndex].anim2 = anim;
		_teleporters[teleIndex].usable2 = usable;
		strcpy(_teleporters[teleIndex].luaFuncUse2, luaFuncUse);
		if (_teleporters[teleIndex].luaFuncUse2[0] == '*')
			_teleporters[teleIndex].luaFuncUse2[0] = 0;
		_numTeleporters++;
		return;
	}

	warning("addToTeleporterList: Adding a 3rd teleporter is illegal");
}

bool AI::findTeleporterDest(int tileX, int tileY, SingleTele *info) {
	for (int i = 0; i < _numTeleporters; i++) {
		if ((_teleporters[i].x1 == tileX) && (_teleporters[i].y1 == tileY)) {
			info->anim = _teleporters[i].anim2;
			info->x = _teleporters[i].x2;
			info->y = _teleporters[i].y2;
			info->dir = _teleporters[i].dir2;
			info->level = _teleporters[i].level2;
			info->usable = _teleporters[i].usable2;
			return true;
		}
		if ((_teleporters[i].x2 == tileX) && (_teleporters[i].y2 == tileY)) {
			info->anim = _teleporters[i].anim1;
			info->x = _teleporters[i].x1;
			info->y = _teleporters[i].y1;
			info->dir = _teleporters[i].dir1;
			info->level = _teleporters[i].level1;
			info->usable = _teleporters[i].usable1;
			return true;
		}
	}
	return false;
}

bool AI::checkTeleportList(AIEntity *e, int x, int y) {
	for (int i = 0; i < kMaxTeleporters; i++) {
		if ((_teleporters[i].x1 == x && _teleporters[i].y1 == y) || (_teleporters[i].x2 == x && _teleporters[i].y2 == y)) {
			int targetX = _teleporters[i].x1;
			int targetY = _teleporters[i].y1;
			int targetX2 = _teleporters[i].x2;
			int targetY2 = _teleporters[i].y2;
			//AIDir dir1 = _teleporters[i].dir1;
			AIDir dir2 = _teleporters[i].dir2;
			//int level1 = _teleporters[i].level1;
			int level2 = _teleporters[i].level2;
			int usable1 = _teleporters[i].usable1;
			//int usable2 = _teleporters[i].usable2;
			int anim1 = _teleporters[i].anim1;
			int anim2 = _teleporters[i].anim2;
			//const char *luaFuncUse1 = _teleporters[i].luaFuncUse1;
			const char *luaFuncUse2 = _teleporters[i].luaFuncUse2;

			// Choose which set of co-ordinates is the target
			if (x != targetX || y != targetY) {
				targetX = _teleporters[i].x2;
				targetY = _teleporters[i].y2;
				targetX2 = _teleporters[i].x1;
				targetY2 = _teleporters[i].y1;
				//dir1 = _teleporters[i].dir2;
				dir2 = _teleporters[i].dir1;
				//level1 = _teleporters[i].level2;
				level2 = _teleporters[i].level1;
				usable1 = _teleporters[i].usable2;
				//usable2 = _teleporters[i].usable1;
				anim1 = _teleporters[i].anim2;
				anim2 = _teleporters[i].anim1;
				//luaFuncUse1 = _teleporters[i].luaFuncUse2;
				luaFuncUse2 = _teleporters[i].luaFuncUse1;
			}

			// We must be exactly on the teleporter
			if (abs(targetX*kTileWidth - e->x) > 2 || abs(targetY*kTileHeight - e->y) > 2)
				return false;

			// Can this teleporter be used?
			if (usable1)
				return false;

			// Move Entity to new Spot, then walk forward one tile
			e->tileX = targetX2;
			e->tileY = targetY2;
			e->x = targetX2 * kTileWidth;
			e->y = targetY2 * kTileHeight;
			e->xVel = e->yVel = 0;
			e->goalX = e->goalY = 0;
			e->animFrame = 0;
			e->drawXOff = e->drawYOff = 0;
			e->dir = dir2;
			e->level = level2;

			if (luaFuncUse2[0])
				g_hdb->_lua->callFunction(luaFuncUse2, 0);

			e->draw = e->standdownGfx[0];
			if (e == _player) {
				clearWaypoints();
			}

			switch (e->dir) {
			case DIR_UP:
				setEntityGoal(e, e->tileX, e->tileY - 1);
				break;
			case DIR_DOWN:
				setEntityGoal(e, e->tileX, e->tileY + 1);
				break;
			case DIR_LEFT:
				setEntityGoal(e, e->tileX - 1, e->tileY);
				break;
			case DIR_RIGHT:
				setEntityGoal(e, e->tileX + 1, e->tileY);
				break;
			case DIR_NONE:
				break;
			}

			g_hdb->_map->centerMapXY(e->x + 16, e->y + 16);

			// Start up Teleport flash animation only if value1 is set to 1
			if (anim1 == 1 || anim2 == 2) {
				addAnimateTarget(e->x, e->y, 0, 7, ANIM_NORMAL, false, false, "teleporter_flash_sit");
				g_hdb->_sound->playSound(SND_TELEPORT);
			}

			// PANIC ZONE Teleports?
			if (anim2 >= 2)
				g_hdb->_window->startPanicZone();
			else
				g_hdb->_window->stopPanicZone();

			// Is there an attack gem still floating around?
			for (Common::Array<AIEntity *>::iterator it = _ents->begin(); it != _ents->end(); ++it) {
				if ((*it)->type == AI_GEM_ATTACK) {
					int amt = getGemAmount();
					setGemAmount(amt + 1);
					removeEntity(*it);
					break;
				}
			}

			_playerEmerging = true;
			return true;
		}
	}

	return false;
}

void AI::addToPathList(int x, int y, int type, AIDir dir) {
	ArrowPath *arrowPath = new ArrowPath;

	arrowPath->type = type;
	arrowPath->tileX = x;
	arrowPath->tileY = y;
	arrowPath->dir = dir;

	_arrowPaths->push_back(arrowPath);
}

ArrowPath *AI::findArrowPath(int x, int y) {
	for (Common::Array<ArrowPath *>::iterator it = _arrowPaths->begin(); it != _arrowPaths->end(); ++it) {
		if ((*it)->tileX == x && (*it)->tileY == y)
			return *it;
	}
	return NULL;
}

void AI::addToTriggerList(char *luaFuncInit, char *luaFuncUse, int x, int y, int value1, int value2, char *id) {
	Trigger *t = new Trigger;

	_triggerList->push_back(t);

	strcpy(t->id, id);
	t->x = x;
	t->y = y;
	t->value1 = value1;
	t->value2 = value2;
	if (luaFuncInit[0] != '*')
		strcpy(t->luaFuncInit, luaFuncInit);
	if (luaFuncUse[0] != '*')
		strcpy(t->luaFuncUse, luaFuncUse);

	if (!t->luaFuncUse[0])
		g_hdb->_window->openMessageBar("Trigger without USE!", 10);

	if (t->luaFuncInit[0]) {
		g_hdb->_lua->pushFunction(t->luaFuncInit);
		g_hdb->_lua->pushInt(x);
		g_hdb->_lua->pushInt(y);
		g_hdb->_lua->pushInt(value1);
		g_hdb->_lua->pushInt(value2);
		g_hdb->_lua->call(4, 0);
	}
}

bool AI::checkTriggerList(char *entName, int x, int y) {
	for (Common::Array<Trigger *>::iterator it = _triggerList->begin(); it != _triggerList->end(); ++it) {
		Trigger *t = *it;
		if (t->x == x && t->y == y) {
			if (!t->luaFuncUse[0])
				return false;

			g_hdb->_lua->pushFunction(t->luaFuncUse);
			g_hdb->_lua->pushString(entName);
			g_hdb->_lua->pushInt(t->x);
			g_hdb->_lua->pushInt(t->y);
			g_hdb->_lua->pushInt(t->value1);
			g_hdb->_lua->pushInt(t->value2);
			g_hdb->_lua->call(5, 0);
			return true;
		}
	}

	return false;
}

void AI::killTrigger(const char *id) {
	for (uint i = 0; i < _triggerList->size(); i++) {
		if (!scumm_stricmp(id, _triggerList->operator[](i)->id)) {
			_triggerList->remove_at(i);
			i--;
		}
	}
}

} // End of Namespace
