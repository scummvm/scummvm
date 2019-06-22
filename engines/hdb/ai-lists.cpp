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
		uint32 size;

		for (int i = start;i <= end;i++) {
			if (i < 10)
				snprintf(name, 32, "%s0%d", tileName, i + 1);
			else
				snprintf(name, 32, "%s%d", tileName, i + 1);
			debug("AddAnimateTarget: %s", name);
			size = g_hdb->_fileMan->getLength(name, TYPE_TILE32);
			at->gfxList[i] = g_hdb->_drawMan->getTileGfx(name, size);
		}
	}

	// Insert in the beginning
	_animTargets->insert_at(0, at);
}

/*
	Animate _animTargets
	Called every frame
*/
void AI::animateTargets() {
	AnimTarget *at;
	int mx, my;
	int layer;

	g_hdb->_map->getMapXY(&mx, &my);

	for (Common::Array<AnimTarget *>::iterator it = _animTargets->begin(); it != _animTargets->end(); it++) {

		at = *it;
		// Draw Non-Map stuff every frame
		if (!at->inMap)
			at->gfxList[at->start]->drawMasked(at->x - mx, at->y - my);

		// Frame Timer
		if (at->animFrame-- < 1) {
			at->animFrame = at->animCycle;

			if (at->inMap) {
				// Animate Map Tiles
				layer = 0; // BG layer
				if (!at->start == g_hdb->_map->getMapBGTileIndex(at->x, at->y))
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

				_animTargets->erase(it);
				continue;
			}
		}
	}
}

// Add an action location to the list of possible actions
// Each action must be paired with another of the same number
void AI::addToActionList(int actionIndex, int x, int y, char *luaFuncInt, char *luaFuncUse) {

	if (!_actions[actionIndex].x1) {
		_actions[actionIndex].x1 = x;
		_actions[actionIndex].y1 = y;
		if (luaFuncInt[0] != '*')
			strcpy(_actions[actionIndex].luaFuncInit, luaFuncInt);
		if (luaFuncInt[0] != '*')
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
		if (luaFuncInt[0] != '*')
			strcpy(_actions[actionIndex].luaFuncInit, luaFuncInt);
		if (luaFuncInt[0] != '*')
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
	for (int i = 0;i < kMaxActions;i++) {
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
				if (_actions[i].luaFuncUse[0])
					g_hdb->_lua->callFunction(_actions[i].luaFuncUse, 0);
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

void AI::addToAutoList(int x, int y, const char *luaFuncInit, const char *luaFuncUse) {

	const char *get;

	for (int i = 0;i < kMaxAutoActions;i++) {
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
				get = g_hdb->_lua->getStringOffStack();
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
	for (int i = 0; i < kMaxAutoActions;i++) {
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
	if ( tileIndex == _useSwitchOff || tileIndex == _useSwitchOff + 1 )
		success = useSwitch( e, x, y, targetX, targetY, _useSwitchOn );
	else
	if ( tileIndex == _useSwitchOn )
		success = useSwitchOn( e, x, y, targetX, targetY, _useSwitchOff );
	else
	//-------------------------------------------------------------------
	if ( tileIndex == _useHandswitchOff || tileIndex == _useHandswitchOff + 1 )
		success = useSwitch( e, x, y, targetX, targetY, _useHandswitchOn );
	else
	if ( tileIndex == _useHandswitchOn )
		success = useSwitchOn( e, x, y, targetX, targetY, _useHandswitchOff );
	else
	//-------------------------------------------------------------------
	if ( tileIndex == _kcHolderWhiteOff || tileIndex == _kcHolderWhiteOff + 1 )
		success = useLockedSwitch( e, x, y, targetX, targetY, _kcHolderWhiteOn, ITEM_KEYCARD_WHITE, "I need a White Keycard." );
	else
	if ( tileIndex == _kcHolderWhiteOn )
		success = useLockedSwitchOn( e, x, y, targetX, targetY, _kcHolderWhiteOff, ITEM_KEYCARD_WHITE );
	else
	if ( tileIndex == _kcHolderBlueOff || tileIndex == _kcHolderBlueOff + 1 )
		success = useLockedSwitch( e, x, y, targetX, targetY, _kcHolderBlueOn, ITEM_KEYCARD_BLUE, "I need a Blue Keycard." );
	else
	if ( tileIndex == _kcHolderBlueOn )
		success = useLockedSwitchOn( e, x, y, targetX, targetY, _kcHolderBlueOff, ITEM_KEYCARD_BLUE );
	else
	if ( tileIndex == _kcHolderRedOff || tileIndex == _kcHolderRedOff + 1 )
		success = useLockedSwitch( e, x, y, targetX, targetY, _kcHolderRedOn, ITEM_KEYCARD_RED, "I need a Red Keycard." );
	else
	if ( tileIndex == _kcHolderRedOn )
		success = useLockedSwitchOn( e, x, y, targetX, targetY, _kcHolderRedOff, ITEM_KEYCARD_RED );
	else
	if ( tileIndex == _kcHolderGreenOff || tileIndex == _kcHolderGreenOff + 1 )
		success = useLockedSwitch( e, x, y, targetX, targetY, _kcHolderGreenOn, ITEM_KEYCARD_GREEN, "I need a Green Keycard." );
	else
	if ( tileIndex == _kcHolderGreenOn )
		success = useLockedSwitchOn( e, x, y, targetX, targetY, _kcHolderGreenOff, ITEM_KEYCARD_GREEN );
	else
	if ( tileIndex == _kcHolderPurpleOff || tileIndex == _kcHolderPurpleOff + 1 )
		success = useLockedSwitch( e, x, y, targetX, targetY, _kcHolderPurpleOn, ITEM_KEYCARD_PURPLE, "I need a Purple Keycard." );
	else
	if ( tileIndex == _kcHolderPurpleOn )
		success = useLockedSwitchOn( e, x, y, targetX, targetY, _kcHolderPurpleOff, ITEM_KEYCARD_PURPLE );
	else
	if ( tileIndex == _kcHolderBlackOff || tileIndex == _kcHolderBlackOff + 1 )
		success = useLockedSwitch( e, x, y, targetX, targetY, _kcHolderBlackOn, ITEM_KEYCARD_BLACK, "I need a Black Keycard." );
	else
	if ( tileIndex == _kcHolderBlackOn )
		success = useLockedSwitchOn( e, x, y, targetX, targetY, _kcHolderBlackOff, ITEM_KEYCARD_BLACK );
	else
	//-------------------------------------------------------------------
	if ( tileIndex == _useSwitch2Off || tileIndex == _useSwitch2Off + 1 )
		success = useSwitch2( e, x, y, targetX, targetY );
	else
	if ( tileIndex == _useHolderEmpty || tileIndex == _useHolderEmpty + 1 )
		success = useCellHolder( e, x, y, targetX, targetY );
	else
	//-------------------------------------------------------------------
	if ( tileIndex == _targetDoorN || tileIndex == _targetDoorN + 3 )
		success = useAutoDoorOpenClose( e, x, y );
	else
	if ( tileIndex == _targetDoorP || tileIndex == _targetDoorP + 3 )
		success = useDoorOpenCloseBot( e, x, y );
	else
	if ( tileIndex == _targetDoorS || tileIndex == _targetDoorS + 3 )
		success = useDoorOpenCloseBot( e, x, y );
	else
	if ( tileIndex == _targetDoorNv || tileIndex == _targetDoorNv + 3 )
		success = useAutoDoorOpenClose( e, x, y );
	else
	if ( tileIndex == _targetDoorPv || tileIndex == _targetDoorPv + 3 )
		success = useDoorOpenCloseBot( e, x, y );
	else
	if ( tileIndex == _targetDoorSv || tileIndex == _targetDoorSv + 3 )
		success = useDoorOpenCloseBot( e, x, y );
	else
	//-------------------------------------------------------------------
	if ( tileIndex == _targetDoorN || tileIndex == _targetDoor2N + 3 )
		success = useAutoDoorOpenClose( e, x, y );
	else
	if ( tileIndex == _targetDoorP || tileIndex == _targetDoor2P + 3 )
		success = useDoorOpenCloseBot( e, x, y );
	else
	if ( tileIndex == _targetDoorS || tileIndex == _targetDoor2S + 3 )
		success = useDoorOpenCloseBot( e, x, y );
	else
	if ( tileIndex == _targetDoorNv || tileIndex == _targetDoor2Nv + 3 )
		success = useAutoDoorOpenClose( e, x, y );
	else
	if ( tileIndex == _targetDoorPv || tileIndex == _targetDoor2Pv + 3 )
		success = useDoorOpenCloseBot( e, x, y );
	else
	if ( tileIndex == _targetDoorSv || tileIndex == _targetDoor2Sv + 3 )
		success = useDoorOpenCloseBot( e, x, y );
	else
	//-------------------------------------------------------------------
	if ( tileIndex == _touchplateOff )
		success = useTouchplate( e, x, y, targetX, targetY, _touchplateOn );
	else
	if ( tileIndex == _touchplateOn )
		success = useTouchplateOn( e, x, y, targetX, targetY, _touchplateOff );
	else
	if ( tileIndex == _templeTouchpOff )
		success = useTouchplate( e, x, y, targetX, targetY, _templeTouchpOn );
	else
	if ( tileIndex == _templeTouchpOn )
		success = useTouchplateOn( e, x, y, targetX, targetY, _templeTouchpOff );

		return success;
}

bool AI::checkAutoList(AIEntity *e, int x, int y) {
	for (int i = 0;i < kMaxAutoActions;i++) {
		if (_autoActions[i].x == x && _autoActions[i].y == y && !_autoActions[i].activated) {
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
	for (int i = 0;i < kMaxAutoActions;i++) {
		if (_autoActions[i].x == x && _autoActions[i].y == y) {
			if (!_autoActions[i].activated)
				return false;
			return true;
		}
	}
	return false;
}

} // End of Namespace
