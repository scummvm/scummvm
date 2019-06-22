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

void AI::addToAutoList(int x, int y, const char *luaFuncInit, const char *luaFuncUse) {

	const char *get;

	for (int i = 0;i < kMaxAutoActions;i++) {
		if (!_autoActions[i]) {
			_autoActions[i] = new AutoAction;
			_autoActions[i]->x = x;
			_autoActions[i]->y = y;
			_autoActions[i]->activated = false;
			if (luaFuncInit[0] != '*')
				strcpy(&_autoActions[i]->luaFuncInit[0], luaFuncInit);
			if (luaFuncUse[0] != '*')
				strcpy(&_autoActions[i]->luaFuncUse[0], luaFuncUse);

			if (_autoActions[i]->luaFuncInit[0]) {
				g_hdb->_lua->callFunction(_autoActions[i]->luaFuncInit, 2);
				get = g_hdb->_lua->getStringOffStack();
				if (!get)
					return;
				strcpy(&_autoActions[i]->entityName[0], get);
				get = g_hdb->_lua->getStringOffStack();
				if (!get)
					return;
				strcpy(&_autoActions[i]->entityName[0], get);
			}
			return;
		}
	}
}

void AI::autoDeactivate(int x, int y) {
	for (int i = 0; i < kMaxAutoActions;i++) {
		if (_autoActions[i]->x == x && _autoActions[i]->y == y) {
			_autoActions[i]->activated = false;
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
		if (_autoActions[i]->x == x && _autoActions[i]->y == y && !_autoActions[i]->activated) {
			bool success = activateAction(e, x, y, 0, 0);
			_autoActions[i]->activated = true;

			if (success && _autoActions[i]->luaFuncUse[0])
				g_hdb->_lua->callFunction(_autoActions[i]->luaFuncUse, 0);

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
		if (_autoActions[i]->x == x && _autoActions[i]->y == y) {
			if (!_autoActions[i]->activated)
				return false;
			return true;
		}
	}
	return false;
}

} // End of Namespace
