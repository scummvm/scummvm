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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef DRAGONS_INVENTORY_H
#define DRAGONS_INVENTORY_H

#include "common/system.h"

namespace Dragons {

class Actor;
class ActorManager;
class BackgroundResourceLoader;
class Background;
class Bag;
class DragonINIResource;

#define DRAGONS_MAX_INVENTORY_ITEMS 0x29

#define ACTOR_INVENTORY_OFFSET 0x17

enum InventoryState {
	Closed = 0,
	InventoryOpen = 1,
	InventionBookOpen = 2
};

class Inventory {
public:
	InventoryState _previousState;
private:
	DragonsEngine *_vm;
	int32 _sequenceId;
	int16 _screenPositionIndex;
	Actor *_actor;
	InventoryState _state;
	Bag *_bag;

	void (*_inventionBookPrevSceneUpdateFunc)();
	uint16 _inventionBookPrevSceneId;
	uint16 _inventionBookPrevFlickerINISceneId;
	Common::Point _inventionBookPrevFlickerINIPosition;

	uint16 _inventoryItemTbl[DRAGONS_MAX_INVENTORY_ITEMS];

public:
	Inventory(DragonsEngine *vm);

	void init(ActorManager *actorManager, BackgroundResourceLoader *backgroundResourceLoader, Bag *bag, DragonINIResource *dragonIniResource);
	void loadScene(uint32 sceneId);
	bool isActorSet() { return true; }

	int32 getSequenceId() {
		return _sequenceId;
	}
	void setActorSequenceId(int32 sequenceId);
	void updateActorSequenceId(int32 sequenceId);
	void resetSequenceId();

	InventoryState getState() { return _state; }
	void setState(InventoryState newState) { _state = newState; }
	void setPreviousState();

	int16 getPositionIndex() { return _screenPositionIndex; }
	Common::Point getPosition();

	bool isOpen() {
		return _state != Closed;
	}

	void close() { _state = Closed; }

	void updateVisibility();
	void setActorFlag400();
	void clearActorFlag400();
	void setPriority(uint16 priority);

	void openInventory();
	void closeInventory();

	void draw();
	uint16 getIniAtPosition(int16 x, int16 y);
	void loadInventoryItemsFromSave();

	void openInventionBook();
	void closeInventionBook();

	bool addItem(uint16 iniId);
	bool addItemIfPositionIsEmpty(uint16 iniId, uint16 x, uint16 y);
	void replaceItem(uint16 existingIniId, uint16 newIniId);
	bool clearItem(uint16 iniId);
	bool hasItem(uint16 iniId);

	Actor *getInventoryItemActor(uint16 iniId);
	void inventoryMissing();

private:
	void setPositionFromSceneId(uint32 sceneId);
	void animateBagIn();
	void animateBagOut();
};

} // End of namespace Dragons

#endif //DRAGONS_INVENTORY_H
