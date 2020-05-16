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

#include "dragons/actor.h"
#include "dragons/dragons.h"
#include "dragons/dragonini.h"
#include "dragons/background.h"
#include "dragons/inventory.h"
#include "dragons/bag.h"
#include "dragons/scene.h"
#include "dragons/talk.h"
#include "dragons/screen.h"

namespace Dragons {


static const struct {
	int x, y;
} positionTable[4] = {
	{   2,   0 },
	{ 206,   0 },
	{   2, 158 },
	{ 206, 158 }
};

static const int16 bagBounceTable[4] = {
	-5, -0xa, -5, 0
};

static const int16 invXPosTable[41] = {
	0x0080, 0x00a0, 0x00c0, 0x0060, 0x0080, 0x00a0, 0x00c0, 0x00e0,
	0x0100, 0x0020, 0x0040, 0x0060, 0x0080, 0x00a0, 0x00c0, 0x00e0,
	0x0100, 0x0020, 0x0040, 0x0060, 0x0080, 0x00a0, 0x00c0, 0x00e0,
	0x0100, 0x0020, 0x0040, 0x0060, 0x0080, 0x00a0, 0x00c0, 0x00e0,
	0x0100, 0x0020, 0x0040, 0x0060, 0x0080, 0x00a0, 0x00c0, 0x00e0,
	0x0100
};

static const int16 invYPosTable[41] = {
	0x0028, 0x0028, 0x0028, 0x0040, 0x0040, 0x0040, 0x0040, 0x0040,
	0x0040, 0x0058, 0x0058, 0x0058, 0x0058, 0x0058, 0x0058, 0x0058,
	0x0058, 0x0070, 0x0070, 0x0070, 0x0070, 0x0070, 0x0070, 0x0070,
	0x0070, 0x0088, 0x0088, 0x0088, 0x0088, 0x0088, 0x0088, 0x0088,
	0x0088, 0x00a0, 0x00a0, 0x00a0, 0x00a0, 0x00a0, 0x00a0, 0x00a0,
	0x00a0
};

Inventory::Inventory(DragonsEngine *vm) : _vm(vm) {
	_state = Closed;
	_sequenceId = 0;
	_screenPositionIndex = 0;
	_previousState = Closed;
	_bag = nullptr;
	_actor = nullptr;

	_inventionBookPrevSceneUpdateFunc = nullptr;
	_inventionBookPrevSceneId = 0;
	_inventionBookPrevFlickerINISceneId = 0;
	_inventionBookPrevFlickerINIPosition = Common::Point(0, 0);
}

void Inventory::init(ActorManager *actorManager, BackgroundResourceLoader *backgroundResourceLoader, Bag *bag, DragonINIResource *dragonIniResource) {
	_actor = actorManager->loadActor(1, 1); //Load inventory
	_actor->_x_pos = 2;
	_actor->_y_pos = 0;
	_actor->_priorityLayer = 6;
	_actor->_flags = 0;
	_actor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
	_actor->updateSequence(0);
	_actor->_flags |= (ACTOR_FLAG_40 | Dragons::ACTOR_FLAG_80 | Dragons::ACTOR_FLAG_100 |
					   ACTOR_FLAG_200);
	_sequenceId = 0;
	_state = Closed;
	_previousState = Closed;
	_bag = bag;

	for (int i = 0; i < DRAGONS_MAX_INVENTORY_ITEMS; i++) {
		actorManager->loadActor(0, i + ACTOR_INVENTORY_OFFSET); // TODO need to share resource between inventory item actors.
	}

	loadInventoryItemsFromSave();
}


void Inventory::loadScene(uint32 sceneId) {
	if (_state == Closed) {
		_sequenceId = _vm->isFlagSet(ENGINE_FLAG_400000) ? 1 : 0;
	}

	if (_sequenceId == 0 && _vm->getVar(7) == 1) {
		_actor->updateSequence(5);
	} else {
		_actor->updateSequence(_sequenceId);
	}

	setPositionFromSceneId(sceneId);
}

void Inventory::updateVisibility() {
	_actor->_priorityLayer = _vm->isFlagSet(ENGINE_FLAG_10) ? (int16)6 : (int16)0;
}

Common::Point Inventory::getPosition() {
	return Common::Point(positionTable[_screenPositionIndex].x, positionTable[_screenPositionIndex].y);
}

void Inventory::setActorFlag400() {
	_actor->setFlag(ACTOR_FLAG_400);
}

void Inventory::clearActorFlag400() {
	_actor->clearFlag(ACTOR_FLAG_400);
}

void Inventory::setPriority(uint16 priority) {
	_actor->_priorityLayer = priority;
}

void Inventory::setActorSequenceId(int32 sequenceId) {
	if (isActorSet()) {
		_actor->_sequenceID = sequenceId;
	}
}

void Inventory::updateActorSequenceId(int32 sequenceId) {
	if (isActorSet()) {
		_actor->updateSequence(sequenceId);
	}
}

void Inventory::resetSequenceId() {
	_actor->updateSequence(_sequenceId);
}

void Inventory::openInventory() {
	//TODO 0x80030e8c
	_sequenceId = 4;

	if (!_vm->isFlagSet(ENGINE_FLAG_400000)) {
		_sequenceId = 2;
	}
	_actor->updateSequence(_sequenceId);
	_screenPositionIndex = 1;
	_actor->_x_pos = positionTable[_screenPositionIndex].x;
	if ((_sequenceId == 0) || (_sequenceId == 2)) {
		_actor->_x_pos = positionTable[_screenPositionIndex].x + 0x32;
	}
	_actor->_y_pos = positionTable[_screenPositionIndex].y;
	animateBagIn();

	//TODO 0x800310e0 update cursor position.

	for (int i = 0; i < DRAGONS_MAX_INVENTORY_ITEMS; i++) {
		Actor *item = _vm->_actorManager->getActor(i + ACTOR_INVENTORY_OFFSET);

		item->_x_pos = item->_walkDestX = invXPosTable[i] + 0x10;
		item->_y_pos = item->_walkDestY = invYPosTable[i] + 0xc;

		if (_inventoryItemTbl[i]) {
			item->_flags = 0; //clear all flags
			item->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
			item->_priorityLayer = 0;
			item->updateSequence(_vm->getINI(_inventoryItemTbl[i] - 1)->inventorySequenceId * 2 + 10);
			item->setFlag(ACTOR_FLAG_200);
			item->setFlag(ACTOR_FLAG_100);
			item->setFlag(ACTOR_FLAG_80);
			item->setFlag(ACTOR_FLAG_40);
			item->_priorityLayer = 6;
		}
	}
}

void Inventory::animateBagIn() {
	_vm->clearFlags(ENGINE_FLAG_8);
	_vm->setFlags(ENGINE_FLAG_80);

	Common::Point pos = _bag->getPosition();
	pos.y = -228;
	int16 accel = 8;

	// Drop bag down into position.
	while (pos.y < 0) {
		pos.y += accel;
		_bag->updatePosition(pos);
		_vm->waitForFrames(1);
		accel += 2;
	}

	_vm->playOrStopSound(0x8001);

	// Shake bag at the end.
	for (int i = 0; i < 4; i++) {
		pos.y = bagBounceTable[i];
		_bag->updatePosition(pos);
		_vm->waitForFrames(2);
	}

	_vm->setFlags(ENGINE_FLAG_8);
	_vm->setFlags(ENGINE_FLAG_10);
}

void Inventory::animateBagOut() {
	_vm->playOrStopSound(0x8000);
	Common::Point pos = _bag->getPosition();
	if (pos.y != 0xc8) {
		for (; pos.y != 0xc8; pos.y += 0x19) {
			_bag->updatePosition(pos);
			_vm->waitForFrames(1);
		}
	}
	_vm->clearFlags(ENGINE_FLAG_80);
}

void Inventory::closeInventory() {
	_vm->_actorManager->clearActorFlags(ACTOR_INVENTORY_OFFSET);
	_screenPositionIndex = _vm->_dragonRMS->getInventoryPosition(_vm->getCurrentSceneId());

	if (!_vm->isFlagSet(ENGINE_FLAG_400000)) {
		_sequenceId = 0;
	} else {
		if (_previousState == InventionBookOpen) {
			_sequenceId = 3;
		} else {
			_sequenceId = 1;
		}
	}
	_actor->updateSequence(_sequenceId);
	_actor->_x_pos = positionTable[_screenPositionIndex].x;
	if (((_sequenceId == 0) || (_sequenceId == 2)) && ((_screenPositionIndex == 1 || (_screenPositionIndex == 3)))) {
		_actor->_x_pos += 0x32;
	}
	_actor->_y_pos = positionTable[_screenPositionIndex].y;
	animateBagOut();
}

void Inventory::draw() {
	if (_bag) {
		_bag->draw();
	}
}

uint16 Inventory::getIniAtPosition(int16 x, int16 y) {
	for (int i = 0; i < DRAGONS_MAX_INVENTORY_ITEMS; i++) {
		if (_inventoryItemTbl[i]) {
			Actor *item = _vm->_actorManager->getActor(i + ACTOR_INVENTORY_OFFSET);
			if (item->_x_pos - 0x10 <= x && x < item->_x_pos + 0x10
					&& item->_y_pos - 0xc <= y && y < item->_y_pos + 0xc) {
				return _inventoryItemTbl[i];
			}
		}
	}
	return 0;
}

void Inventory::loadInventoryItemsFromSave() {
	memset(_inventoryItemTbl, 0, sizeof(_inventoryItemTbl));
	int j = 0;
	for (int i = 0; i < _vm->_dragonINIResource->totalRecords() && j < DRAGONS_MAX_INVENTORY_ITEMS; i++) {
		DragonINI *ini = _vm->_dragonINIResource->getRecord(i);
		if (ini->sceneId == 1) {
			_inventoryItemTbl[j++] = i + 1;
		}
	}
}

void Inventory::openInventionBook() {
	_inventionBookPrevSceneUpdateFunc = _vm->getSceneUpdateFunction();
	_vm->clearSceneUpdateFunction();
	_vm->fadeToBlack();
	_sequenceId = 2;
	_actor->updateSequence(2);
	_inventionBookPrevSceneId = _vm->getCurrentSceneId();
	DragonINI *flicker = _vm->_dragonINIResource->getFlickerRecord();
	if (flicker && flicker->actor) {
		_inventionBookPrevFlickerINISceneId = flicker->sceneId;
		_inventionBookPrevFlickerINIPosition = Common::Point(flicker->actor->_x_pos, flicker->actor->_y_pos);
		flicker->sceneId = 0;
	}
	_vm->_scene->setSceneId(2);
	_vm->_scene->loadScene(2, 0);
}

void Inventory::closeInventionBook() {
	uint sceneId;

	_vm->fadeToBlack();

	DragonINI *flicker = _vm->_dragonINIResource->getFlickerRecord();
	if (flicker) {
		flicker->x = _inventionBookPrevFlickerINIPosition.x;
		flicker->y = _inventionBookPrevFlickerINIPosition.y;
		flicker->sceneId = _inventionBookPrevFlickerINISceneId;
	}
	_vm->_scene->setSceneId(_inventionBookPrevSceneId);

	_sequenceId = 0;
	setActorSequenceId(0);
	setPositionFromSceneId(_inventionBookPrevSceneId);
	sceneId = _vm->_scene->getSceneId();
	if (((((sceneId == 0x23) || (sceneId == 0x2d)) || (sceneId == 0x2e)) || ((sceneId == 0x31 || (sceneId == 0x32)))) || (sceneId == 0x28)) {
		if ((uint)_vm->_scene->getSceneId() == 0x27) {
			_vm->getINI(0x206)->sceneId = 0;
		}
	} else {
		if (sceneId != 0x27) {
			if (sceneId != 0x1c && sceneId != 0x1d && sceneId != 0x21) {
				_vm->_scene->loadScene(sceneId | 0x8000u, 0x1e);
				_vm->setSceneUpdateFunction(_inventionBookPrevSceneUpdateFunc);
				return;
			}
			if ((uint)_vm->_scene->getSceneId() == 0x27) {
				_vm->getINI(0x206)->sceneId = 0;
			}
		} else {
			_vm->getINI(0x206)->sceneId = 0;
		}
	}
	_vm->_scene->loadScene(_vm->_scene->getSceneId(), 0x1e);
	_vm->setSceneUpdateFunction(_inventionBookPrevSceneUpdateFunc);
}

void Inventory::setPositionFromSceneId(uint32 sceneId) {
	_screenPositionIndex = _vm->_dragonRMS->getInventoryPosition(sceneId);

	_actor->_x_pos = positionTable[_screenPositionIndex].x;
	if ((_sequenceId == 0 || _sequenceId == 2) && (_screenPositionIndex == 1 || _screenPositionIndex == 3)) {
		_actor->_x_pos += 0x32;
	}
	_actor->_y_pos = positionTable[_screenPositionIndex].y;
}

bool Inventory::addItem(uint16 initId) {
	for (int i = 0; i < DRAGONS_MAX_INVENTORY_ITEMS; i++) {
		if (_inventoryItemTbl[i] == 0) {
			_inventoryItemTbl[i] = initId;
			return true;
		}
	}

	return false;
}

Actor *Inventory::getInventoryItemActor(uint16 iniId) {
	for (int i = 0; i < DRAGONS_MAX_INVENTORY_ITEMS; i++) {
		if (_inventoryItemTbl[i] == iniId) {
			return _vm->_actorManager->getActor(i + ACTOR_INVENTORY_OFFSET);
		}
	}
	error("getInventoryItemActor(%d) not found", iniId);
}

void Inventory::replaceItem(uint16 existingIniId, uint16 newIniId) {
	for (int i = 0; i < DRAGONS_MAX_INVENTORY_ITEMS; i++) {
		if (_inventoryItemTbl[i] == existingIniId) {
			_inventoryItemTbl[i] = newIniId;
			return;
		}
	}
}

bool Inventory::addItemIfPositionIsEmpty(uint16 iniId, uint16 x, uint16 y) {
	for (int i = 0; i < DRAGONS_MAX_INVENTORY_ITEMS; i++) {
		Actor *actor = _vm->_actorManager->getActor(i + ACTOR_INVENTORY_OFFSET);
		if ((((actor->_x_pos - 0x10 <= x) &&
				(x < actor->_x_pos + 0x10)) &&
				(actor->_y_pos - 0xc <= y)) &&
				(y < actor->_y_pos + 0xc)) {
			_inventoryItemTbl[i] = iniId;
			return true;
		}
	}
	return false;
}

bool Inventory::clearItem(uint16 iniId) {
	for (int i = 0; i < DRAGONS_MAX_INVENTORY_ITEMS; i++) {
		if (_inventoryItemTbl[i] == iniId) {
			_inventoryItemTbl[i] = 0;
			return true;
		}
	}
	return false;
}

void Inventory::inventoryMissing() {
	bool flag8Set;
	uint32 textIndex;
	static uint16 counter = 0;

	DragonINI *flicker = _vm->_dragonINIResource->getFlickerRecord();
	if (flicker->actor != nullptr) {
		flicker->actor->clearFlag(ACTOR_FLAG_10);
		if ((_vm->getCurrentSceneId() != 0x2e) || (flicker->actor->_resourceID != 0x91)) {
			flicker->actor->setFlag(ACTOR_FLAG_4);
		}
	}
	flag8Set = _vm->isFlagSet(ENGINE_FLAG_8);
	_vm->clearFlags(ENGINE_FLAG_8);
	if (counter == 0) {
		textIndex = 0x114FA; //Hey!  My bag is missing!
	} else {
		textIndex = 0x11538; //The Chancellor snaked my bag!
	}
	counter = counter + 1;
	_vm->_talk->talkFromIni(0, textIndex);
	if (flag8Set) {
		_vm->setFlags(ENGINE_FLAG_8);
	}
}

void Inventory::setPreviousState() {
	InventoryState tmpState = _state;
	setState(_previousState);
	_previousState = tmpState;
}

bool Inventory::hasItem(uint16 iniId) {
	for (int i = 0; i < DRAGONS_MAX_INVENTORY_ITEMS; i++) {
		if (_inventoryItemTbl[i] == iniId) {
			return true;
		}
	}
	return false;
}

} // End of namespace Dragons
