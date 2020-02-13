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
#include "hdb/ai.h"
#include "hdb/map.h"
#include "hdb/sound.h"
#include "hdb/window.h"

namespace HDB {

bool AI::isClosedDoor(int x, int y) {
	int tileIndex = g_hdb->_map->getMapBGTileIndex(x, y);

	if ((tileIndex == _targetDoorN + 3) ||		// locked SILVER door?
		(tileIndex == _targetDoorNv + 3) ||
		(tileIndex == _targetDoorP + 3) ||		// locked BLUE door?
		(tileIndex == _targetDoorPv + 3) ||
		(tileIndex == _targetDoorS + 3) ||		// locked RED door?
		(tileIndex == _targetDoorSv + 3) ||
		(tileIndex == _targetDoor2N + 3) ||		// locked SILVER door?
		(tileIndex == _targetDoor2Nv + 3) ||
		(tileIndex == _targetDoor2P + 3) ||		// locked BLUE door?
		(tileIndex == _targetDoor2Pv + 3) ||
		(tileIndex == _targetDoor2S + 3) ||		// locked RED door?
		(tileIndex == _targetDoor2Sv + 3) ||
		(tileIndex == _target2DoorN + 3) ||		// locked SILVER door?
		(tileIndex == _target2DoorNv + 3) ||
		(tileIndex == _target2DoorP + 3) ||		// locked BLUE door?
		(tileIndex == _target2DoorPv + 3) ||
		(tileIndex == _target2DoorS + 3) ||		// locked RED door?
		(tileIndex == _target2DoorSv + 3) ||
		(tileIndex == _target3DoorN + 3) ||		// locked SILVER door?
		(tileIndex == _target3DoorNv + 3) ||
		(tileIndex == _target3DoorP + 3) ||		// locked BLUE door?
		(tileIndex == _target3DoorPv + 3) ||
		(tileIndex == _target3DoorS + 3) ||		// locked RED door?
		(tileIndex == _target3DoorSv + 3) ||
		(tileIndex == _blockpole + 3))			// blockpole UP?
		return true;
	return false;
}

bool AI::isOpenDoor(int x, int y) {
	int tileIndex = g_hdb->_map->getMapBGTileIndex(x, y);

	if ((tileIndex == _targetDoorN) ||		// open SILVER door?
		(tileIndex == _targetDoorNv) ||
		(tileIndex == _targetDoorP) ||		// open BLUE door?
		(tileIndex == _targetDoorPv) ||
		(tileIndex == _targetDoorS) ||		// open RED door?
		(tileIndex == _targetDoorSv) ||
		(tileIndex == _targetDoor2N) ||		// open SILVER door?
		(tileIndex == _targetDoor2Nv) ||
		(tileIndex == _targetDoor2P) ||		// open BLUE door?
		(tileIndex == _targetDoor2Pv) ||
		(tileIndex == _targetDoor2S) ||		// open RED door?
		(tileIndex == _targetDoor2Sv) ||
		(tileIndex == _target2DoorN) ||		// open SILVER door?
		(tileIndex == _target2DoorNv) ||
		(tileIndex == _target2DoorP) ||		// open BLUE door?
		(tileIndex == _target2DoorPv) ||
		(tileIndex == _target2DoorS) ||		// open RED door?
		(tileIndex == _target2DoorSv) ||
		(tileIndex == _target3DoorN) ||		// open SILVER door?
		(tileIndex == _target3DoorNv) ||
		(tileIndex == _target3DoorP) ||		// open BLUE door?
		(tileIndex == _target3DoorPv) ||
		(tileIndex == _target3DoorS) ||		// open RED door?
		(tileIndex == _target3DoorSv) ||
		(tileIndex == _blockpole))			// blockpole DOWN?
		return true;
	return false;
}

bool AI::useTarget(int x, int y, int targetX, int targetY, int newTile, int *worked) {
	// open a locked door?
	if (isClosedDoor(targetX, targetY)) {
		int tileIndex = g_hdb->_map->getMapBGTileIndex(targetX, targetY);

		addAnimateTarget(targetX, targetY, tileIndex, tileIndex - 3, ANIM_SLOW, false, true, NULL);
		g_hdb->_map->setMapBGTileIndex(x, y, newTile);
		if (g_hdb->_map->onScreen(x, y))
			g_hdb->_sound->playSound(SND_DOOR_OPEN_CLOSE);
		*worked = 1;
		return false;						// return FALSE because we need to be able to do it some more
	}

	// close an open door?
	if (isOpenDoor(targetX, targetY)) {
		int tileIndex = g_hdb->_map->getMapBGTileIndex(targetX, targetY);

		addAnimateTarget(targetX, targetY, tileIndex, tileIndex + 3, ANIM_SLOW, false, true, NULL);
		g_hdb->_map->setMapBGTileIndex(x, y, newTile);
		if (g_hdb->_map->onScreen(x, y))
			g_hdb->_sound->playSound(SND_DOOR_OPEN_CLOSE);
		*worked = 1;
		return false;						// return FALSE because we need to be able to do it some more
	}

	// open up a bridge?
	int tileIndex = g_hdb->_map->getMapFGTileIndex(targetX, targetY);
	if (tileIndex == _targetBridgeU ||
		tileIndex == _targetBridgeD ||
		tileIndex == _targetBridgeL ||
		tileIndex == _targetBridgeR) {
		addBridgeExtend(targetX, targetY, tileIndex);
		g_hdb->_map->setMapBGTileIndex(x, y, newTile);
		*worked = 1;
		return true;						// return TRUE because we can't open it again
	}

	*worked = 0;
	return false;
}

// Black Door Switch
bool AI::useSwitch(AIEntity *e, int x, int y, int targetX, int targetY, int onTile) {
	int worked;
	if (g_hdb->_map->onScreen(x, y))
		g_hdb->_sound->playSound(SND_SWITCH_USE);
	return useTarget(x, y, targetX, targetY, onTile, &worked);
}

bool AI::useSwitchOn(AIEntity *e, int x, int y, int targetX, int targetY, int offTile) {
	int worked;
	if (g_hdb->_map->onScreen(x, y))
		g_hdb->_sound->playSound(SND_SWITCH_USE);
	return useTarget(x, y, targetX, targetY, offTile, &worked);
}

bool AI::useSwitch2(AIEntity *e, int x, int y, int targetX, int targetY) {
	// int i = 10; // unused
	return true;
}

// Colored Keycard Switch
bool AI::useLockedSwitch(AIEntity *e, int x, int y, int targetX, int targetY, int onTile, AIType item, const char *keyerror) {
	// is the PLAYER next to this thing?  No other entities are allowed to unlock anything!
	if (abs(x - _player->tileX) > 1 || abs(y - _player->tileY) > 1)
		return false;

	int	amount = queryInventoryType(item);
	if (amount) {
		int	worked;
		bool rtn = useTarget(x, y, targetX, targetY, onTile, &worked);
		if (worked) {
			removeInvItemType(item, 1);
			if (g_hdb->_map->onScreen(x, y))
				g_hdb->_sound->playSound(SND_SWITCH_USE);
		}
		return rtn;
	} else {
		if (g_hdb->_map->onScreen(x, y))
			g_hdb->_sound->playSound(SND_CELLHOLDER_USE_REJECT);
		g_hdb->_window->openMessageBar(keyerror, 3);
	}
	return false;
}

bool AI::useLockedSwitchOn(AIEntity *e, int x, int y, int targetX, int targetY, int offTile, AIType item) {
	// is the PLAYER next to this thing?  No other entities are allowed to unlock anything!
	if (abs(x - _player->tileX) > 1 || abs(y - _player->tileY) > 1)
		return false;

	if (getInvAmount() == 10)
		return false;

	int	worked;
	bool rtn = useTarget(x, y, targetX, targetY, offTile, &worked);
	if (worked) {
		addItemToInventory(item, 1, NULL, NULL, NULL);
		if (g_hdb->_map->onScreen(x, y))
			g_hdb->_sound->playSound(SND_SWITCH_USE);
	}
	return rtn;
}

// Purple Cell Holder Switch
bool AI::useCellHolder(AIEntity *e, int x, int y, int targetX, int targetY) {
	// is the PLAYER next to this thing?  No other entities are allowed to unlock anything!
	if (abs(x - _player->tileX) > 1 || abs(y - _player->tileY) > 1)
		return false;

	int	amount = queryInventoryType(ITEM_CELL);

	if (amount) {
		int	worked;
		bool rtn = useTarget(x, y, targetX, targetY, _useHolderFull, &worked);
		if (worked) {
			removeInvItemType(ITEM_CELL, 1);
			if (g_hdb->_map->onScreen(x, y))
				g_hdb->_sound->playSound(SND_SWITCH_USE);
		}
		return rtn;
	} else {
		if (g_hdb->_map->onScreen(x, y))
			g_hdb->_sound->playSound(SND_CELLHOLDER_USE_REJECT);
	}

	g_hdb->_window->openDialog("Locked!", -1, "I can't use that unless I have an Energy Cell.", 0, NULL);
	g_hdb->_sound->playVoice(GUY_ENERGY_CELL, 0);
	return false;
}

// Touchplate
bool AI::useTouchplate(AIEntity *e, int x, int y, int targetX, int targetY, int type) {
	int worked;
	g_hdb->_sound->playSound(SND_TOUCHPLATE_CLICK);
	return useTarget(x, y, targetX, targetY, type, &worked);
}
bool AI::useTouchplateOn(AIEntity *e, int x, int y, int targetX, int targetY, int type) {
	int worked;
	g_hdb->_sound->playSound(SND_TOUCHPLATE_CLICK);
	return useTarget(x, y, targetX, targetY, type, &worked);
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
		g_hdb->_sound->playSound(SND_DOOR_OPEN_CLOSE);
	return;
}

// Normal Door
bool AI::useDoorOpenClose(AIEntity *e, int x, int y) {
	int tileIndex = g_hdb->_map->getMapBGTileIndex(x, y);

	addAnimateTarget(x, y, tileIndex, tileIndex - 3, ANIM_SLOW, false, true, NULL);
	addCallback(CALLBACK_DOOR_OPEN_CLOSE, x, y, kDelay5Seconds);
	if (g_hdb->_map->onScreen(x, y))
		g_hdb->_sound->playSound(SND_DOOR_OPEN_CLOSE);
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
		g_hdb->_sound->playSound(SND_DOOR_OPEN_CLOSE);
	return;
}

bool AI::useAutoDoorOpenClose(AIEntity *e, int x, int y) {
	int tileIndex = g_hdb->_map->getMapBGTileIndex(x, y);

	if (autoActive(x, y))
		return false;

	addAnimateTarget(x, y, tileIndex, tileIndex - 3, ANIM_SLOW, false, true, NULL);
	addCallback(CALLBACK_AUTODOOR_OPEN_CLOSE, x, y, kDelay5Seconds);
	if (g_hdb->_map->onScreen(x, y))
		g_hdb->_sound->playSound(SND_DOOR_OPEN_CLOSE);
	return false;
}

// Any Type Door
bool AI::useDoorOpenCloseBot(AIEntity *e, int x, int y) {
	int	tileIndex = g_hdb->_map->getMapBGTileIndex(x, y);

	if (e == _player || e->type == AI_SLUG_ATTACK || e->type == AI_GEM_ATTACK) {
		if (isClosedDoor(x, y))
			g_hdb->_sound->playSound(SND_GUY_UHUH);
		return false;
	}

	addAnimateTarget(x, y, tileIndex, tileIndex - 3, ANIM_SLOW, false, true, NULL);
	//	AddCallback( CALLBACK_DOOR_OPEN_CLOSE, x, y, DELAY_5SECONDS / fs );
	if (g_hdb->_map->onScreen(x, y))
		g_hdb->_sound->playSound(SND_DOOR_OPEN_CLOSE);
	return true;
}

} // End of Namespace
