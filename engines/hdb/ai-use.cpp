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
 * This program is distributed in the hope that it will be AI::useful,
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

// Black Door Switch
bool AI::useSwitch(AIEntity *e, int x, int y, int targetX, int targetY, int onTile) {
	warning("STUB: Define useSwitch");
	return false;
}

bool AI::useSwitchOn(AIEntity *e, int x, int y, int targetX, int targetY, int offTile) {
	warning("STUB: Define useSwitchOn");
	return false;
}

bool AI::useSwitch2(AIEntity *e, int x, int y, int targetX, int targetY) {
	warning("STUB: Define useSwitch2");
	return false;
}

// Colored Keycard Switch
bool AI::useLockedSwitch(AIEntity *e, int x, int y, int targetX, int targetY, int onTile, AIType item, const char *keyerror) {
	warning("STUB: Define useLockedSwitch");
	return false;
}

bool AI::useLockedSwitchOn(AIEntity *e, int x, int y, int targetX, int targetY, int offTile, AIType item) {
	warning("STUB: Define useLockedSwitchOn");
	return false;
}

// Purple Cell Holder Switch
bool AI::useCellHolder(AIEntity *e, int x, int y, int targetX, int targetY) {
	warning("STUB: Define useCellHolder");
	return false;
}

// Touchplate
bool AI::useTouchplate(AIEntity *e, int x, int y, int targetX, int targetY, int type) {
	warning("STUB: Define useTouchplate");
	return false;
}
bool AI::useTouchplateOn(AIEntity *e, int x, int y, int targetX, int targetY, int type) {
	warning("STUB: Define useTouchplateOn");
	return false;
}

void callbackDoorOpenClose(int x, int y) {
	int	tileIndex = g_hdb->_map->getMapBGTileIndex(x, y);

	// is the door gonna close on something?  if so, wait again
	if (!g_hdb->_ai->findEntity(x, y)) {
		g_hdb->_ai->addCallback(CALLBACK_DOOR_OPEN_CLOSE, x, y, kDelay5Seconds);
		return;
	}

	g_hdb->_ai->addAnimateTarget(x, y, tileIndex, tileIndex + 3, ANIM_SLOW, false, true, NULL);
	if (g_hdb->_map->onScreen(x, y))
		warning("STUB: Play SND_DOOR_OPEN_CLOSE");
	return;
}

// Normal Door
bool AI::useDoorOpenClose(AIEntity *e, int x, int y) {
	int tileIndex = g_hdb->_map->getMapBGTileIndex(x, y);

	addAnimateTarget(x, y, tileIndex, tileIndex - 3, ANIM_SLOW, false, true, NULL);
	addCallback(CALLBACK_DOOR_OPEN_CLOSE, x, y, kDelay5Seconds);
	if (g_hdb->_map->onScreen(x, y))
		warning("Play SND_DOOR_OPEN_CLOSE");
	return true;
}

void callbackAutoDoorOpenClose(int x, int y) {
	int tileIndex = g_hdb->_map->getMapBGTileIndex(x, y);

	// Is the door going to close on something?
	if (g_hdb->_ai->findEntity(x, y)) {
		g_hdb->_ai->addCallback(CALLBACK_AUTODOOR_OPEN_CLOSE, x, y, kDelay5Seconds);
		return;
	}

	g_hdb->_ai->addAnimateTarget(x, y, tileIndex, tileIndex + 3, ANIM_SLOW, true, true, NULL);
	if (g_hdb->_map->onScreen(x, y))
		warning("STUB: callbackAutoDoorOpenClose: Play SND_DOOR_OPEN_CLOSE");
	return;
}

bool AI::useAutoDoorOpenClose(AIEntity *e, int x, int y) {
	int tileIndex = g_hdb->_map->getMapBGTileIndex(x, y);

	if (autoActive(x, y))
		return false;

	addAnimateTarget(x, y, tileIndex, tileIndex - 3, ANIM_SLOW, false, true, NULL);
	addCallback(CALLBACK_AUTODOOR_OPEN_CLOSE, x, y, kDelay5Seconds);
	if (g_hdb->_map->onScreen(x, y))
		warning("useAutoDoorOpenClose: Play SND_DOOR_OPEN_CLOSE");
	return false;
}

// Any Type Door
bool AI::useDoorOpenCloseBot(AIEntity *e, int x, int y) {
	warning("STUB: Define useDoorOpenCloseBot");
	return false;
}

} // End of Namespace
