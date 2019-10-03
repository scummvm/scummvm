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

#include "actor.h"
#include "dragons.h"
#include "dragonrms.h"
#include "dragonini.h"
#include "background.h"
#include "inventory.h"
#include "bag.h"
#include "scene.h"
#include "talk.h"

namespace Dragons {


static const Common::Point positionTable[4] = {
		{2,0},
		{0xce,0},
		{2,0x9e},
		{0xce,0x9e}
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
	_type = 0;
	_sequenceId = 0;
	_screenPositionIndex = 0;
	_old_showing_value = 0;
	_bag = NULL;

	inventionBookPrevSceneUpdateFunc = NULL;
	inventionBookPrevSceneId = 0;
	inventionBookPrevFlickerINISceneId = 0;
	inventionBookPrevFlickerINIPosition = Common::Point(0,0);
}

void Inventory::init(ActorManager *actorManager, BackgroundResourceLoader *backgroundResourceLoader, Bag *bag, DragonINIResource *dragonIniResource) {
	_actor = actorManager->loadActor(1, 1); //Load inventory
	_actor->x_pos = 2;
	_actor->y_pos = 0;
	_actor->priorityLayer = 6;
	_actor->flags = 0;
	_actor->scale = 0x100;
	_actor->updateSequence(0);
	_actor->flags |= (Dragons::ACTOR_FLAG_40 | Dragons::ACTOR_FLAG_80 | Dragons::ACTOR_FLAG_100 |
						 Dragons::ACTOR_FLAG_200);
	_sequenceId = 0;
	_type = 0;
	_old_showing_value = 0;
	_bag = bag;

	for(int i = 0; i < DRAGONS_MAX_INVENTORY_ITEMS; i++) {
		actorManager->loadActor(0, i + ACTOR_INVENTORY_OFFSET); // TODO need to share resource between inventory item actors.
	}

	loadInventoryItemsFromSave();
}


void Inventory::loadScene(uint32 sceneId) {
	if (!_type) {
		_sequenceId = _vm->isFlagSet(Dragons::ENGINE_FLAG_400000) ? 1 : 0;
	}

	if (_sequenceId == 0 && _vm->getVar(7) == 1) {
		_actor->updateSequence(5);
	} else {
		_actor->updateSequence(_sequenceId);
	}

	setPositionFromSceneId(sceneId);
}

void Inventory::updateVisibility() {
	_actor->priorityLayer = _vm->isFlagSet(Dragons::ENGINE_FLAG_10) ? (int16)6 : (int16)0;
}

Common::Point Inventory::getPosition() {
	return positionTable[_screenPositionIndex];
}

void Inventory::setActorFlag400() {
	_actor->setFlag(ACTOR_FLAG_400);
}

void Inventory::clearActorFlag400() {
	_actor->clearFlag(ACTOR_FLAG_400);
}

void Inventory::setPriority(uint16 priority) {
	_actor->priorityLayer = priority;
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
	_actor->x_pos = positionTable[_screenPositionIndex].x;
	if ((_sequenceId == 0) || (_sequenceId == 2)) {
		_actor->x_pos = positionTable[_screenPositionIndex].x + 0x32;
	}
	_actor->y_pos = positionTable[_screenPositionIndex].y;
	animateBagIn();

	//TODO 0x800310e0 update cursor position.

	for(int i = 0; i < DRAGONS_MAX_INVENTORY_ITEMS; i++) {
		Actor *item = _vm->_actorManager->getActor(i + ACTOR_INVENTORY_OFFSET);

		item->x_pos = item->target_x_pos = invXPosTable[i] + 0x10;
		item->y_pos = item->target_y_pos = invYPosTable[i] + 0xc;

		if (inventoryItemTbl[i]) {
			item->flags = 0; //clear all flags
			item->scale = 0x100;
			item->priorityLayer = 0;
			item->updateSequence(_vm->getINI(inventoryItemTbl[i] - 1)->field_8 * 2 + 10);
			item->setFlag(ACTOR_FLAG_200);
			item->setFlag(ACTOR_FLAG_100);
			item->setFlag(ACTOR_FLAG_80);
			item->setFlag(ACTOR_FLAG_40);
			item->priorityLayer = 6;
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

	_vm->playSound(0x8001);

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
	_vm->playSound(0x8000);
	Common::Point pos = _bag->getPosition();
	if (pos.y != 0xc8) {
		for (;pos.y != 0xc8; pos.y += 0x19) {
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
	}
	else {
		if (_old_showing_value == 2) {
			_sequenceId = 3;
		}
		else {
			_sequenceId = 1;
		}
	}
	_actor->updateSequence(_sequenceId);
	_actor->x_pos = positionTable[_screenPositionIndex].x;
	if (((_sequenceId == 0) || (_sequenceId == 2)) && ((_screenPositionIndex == 1 || (_screenPositionIndex == 3)))) {
		_actor->x_pos += 0x32;
	}
	_actor->y_pos = positionTable[_screenPositionIndex].y;
	animateBagOut();
}

void Inventory::draw() {
	if(_bag) {
		_bag->draw();
	}
}

uint16 Inventory::getIniAtPosition(int16 x, int16 y) {
	for (int i = 0; i < DRAGONS_MAX_INVENTORY_ITEMS; i++) {
		if (inventoryItemTbl[i]) {
			Actor *item = _vm->_actorManager->getActor(i + ACTOR_INVENTORY_OFFSET);
			if (item->x_pos - 0x10 <= x && x < item->x_pos + 0x10
				&& item->y_pos - 0xc <= y && y < item->y_pos + 0xc) {
				return inventoryItemTbl[i];
			}
		}
	}
	return 0;
}

void Inventory::loadInventoryItemsFromSave() {
	memset(inventoryItemTbl, 0, sizeof(inventoryItemTbl));
	int j = 0;
	for (int i=0; i < _vm->_dragonINIResource->totalRecords() && j < DRAGONS_MAX_INVENTORY_ITEMS; i++ ) {
		DragonINI *ini = _vm->_dragonINIResource->getRecord(i);
		if (ini->sceneId == 1) {
			inventoryItemTbl[j++] = i + 1;
		}
	}
}

void Inventory::openInventionBook() {
	inventionBookPrevSceneUpdateFunc = _vm->getSceneUpdateFunction();
	_vm->setSceneUpdateFunction(NULL);
//	fade_related_calls_with_1f();
	_sequenceId = 2;
	_actor->updateSequence(2);
	inventionBookPrevSceneId = _vm->getCurrentSceneId();
	DragonINI *flicker = _vm->_dragonINIResource->getFlickerRecord();
	if (flicker && flicker->actor) {
		inventionBookPrevFlickerINISceneId = flicker->sceneId;
		inventionBookPrevFlickerINIPosition = Common::Point(flicker->actor->x_pos, flicker->actor->y_pos);
		flicker->sceneId = 0;
	}
	_vm->_scene->setSceneId(2);
	_vm->_scene->loadScene(2, 0);
}

void Inventory::closeInventionBook() {
	uint uVar1;
	uint uVar2;

	// TODO fade_related_calls_with_1f();

	DragonINI *flicker = _vm->_dragonINIResource->getFlickerRecord();
	if (flicker && flicker->actor) {
		flicker->actor->x_pos = inventionBookPrevFlickerINIPosition.x;
		flicker->actor->y_pos = inventionBookPrevFlickerINIPosition.y;
		flicker->sceneId = inventionBookPrevFlickerINISceneId;
	}
	_vm->_scene->setSceneId(inventionBookPrevSceneId);

	_sequenceId = 0;
	setActorSequenceId(0);
	setPositionFromSceneId(inventionBookPrevSceneId);
	uVar2 = _vm->_scene->getSceneId();
	if (((((uVar2 == 0x23) || (uVar2 == 0x2d)) || (uVar2 == 0x2e)) || ((uVar2 == 0x31 || (uVar2 == 0x32)))) || (uVar2 == 0x28)) {
		LAB_80038b9c:
		if ((uint)_vm->_scene->getSceneId() == 0x27) goto LAB_80038bb8;
	}
	else {
		if (uVar2 != 0x27) {
			if (((uVar2 != 0x1c) && (uVar2 != 0x1d)) && (uVar1 = uVar2 | 0x8000, uVar2 != 0x21)) goto LAB_80038be8;
			goto LAB_80038b9c;
		}
		LAB_80038bb8:
		_vm->getINI(0x206)->sceneId = 0;
	}
	uVar1 = (uint)_vm->_scene->getSceneId();
	LAB_80038be8:
	_vm->_scene->loadScene(uVar1,0x1e);
	_vm->setSceneUpdateFunction(inventionBookPrevSceneUpdateFunc);
	return;
}

void Inventory::setPositionFromSceneId(uint32 sceneId) {
	_screenPositionIndex = _vm->_dragonRMS->getInventoryPosition(sceneId);

	_actor->x_pos = positionTable[_screenPositionIndex].x;
	if ((_sequenceId == 0 || _sequenceId == 2) && (_screenPositionIndex == 1 || _screenPositionIndex == 3)) {
		_actor->x_pos += 0x32;
	}
	_actor->y_pos = positionTable[_screenPositionIndex].y;
}

bool Inventory::addItem(uint16 initId) {
	for (int i = 0; i < DRAGONS_MAX_INVENTORY_ITEMS; i++) {
		if (inventoryItemTbl[i] == 0) {
			inventoryItemTbl[i] = initId;
			return true;
		}
	}

	return false;
}

Actor *Inventory::getInventoryItemActor(uint16 iniId) {
	for (int i = 0; i < DRAGONS_MAX_INVENTORY_ITEMS; i++) {
		if (inventoryItemTbl[i] == iniId) {
			return _vm->_actorManager->getActor(i + ACTOR_INVENTORY_OFFSET);
		}
	}
	error("getInventoryItemActor(%d) not found", iniId);
}

void Inventory::replaceItem(uint16 existingIniId, uint16 newIniId) {
	for (int i = 0; i < DRAGONS_MAX_INVENTORY_ITEMS; i++) {
		if (inventoryItemTbl[i] == existingIniId) {
			inventoryItemTbl[i] = newIniId;
			return;
		}
	}
}

bool Inventory::addItemIfPositionIsEmpty(uint16 iniId, uint16 x, uint16 y) {
	for (int i = 0; i < DRAGONS_MAX_INVENTORY_ITEMS; i++) {
		Actor *actor = _vm->_actorManager->getActor(i + ACTOR_INVENTORY_OFFSET);
		if ((((actor->x_pos - 0x10 <= x) &&
			  (x < actor->x_pos + 0x10)) &&
			 (actor->y_pos - 0xc <= y)) &&
			(y < actor->y_pos + 0xc)) {
			inventoryItemTbl[i] = iniId;
			return true;
		}
	}
	return false;
}

bool Inventory::clearItem(uint16 iniId) {
	for (int i = 0; i < DRAGONS_MAX_INVENTORY_ITEMS; i++) {
		if(inventoryItemTbl[i] == iniId) {
			inventoryItemTbl[i] = 0;
		}
	}
	return false;
}

void Inventory::inventoryMissing() {
	bool flag8Set;
	uint32 textIndex;
	static uint16 counter = 0;

	DragonINI *flicker = _vm->_dragonINIResource->getFlickerRecord();
	if (flicker->actor != NULL) {
		flicker->actor->clearFlag(ACTOR_FLAG_10);
		if ((_vm->getCurrentSceneId() != 0x2e) || (flicker->actor->resourceID != 0x91)) {
			flicker->actor->setFlag(ACTOR_FLAG_4);
		}
	}
	flag8Set = _vm->isFlagSet(ENGINE_FLAG_8);
	_vm->clearFlags(ENGINE_FLAG_8);
	if (counter == 0) {
		textIndex = 0x114FA; //Hey!  My bag is missing!
	}
	else {
		textIndex = 0x11538; //The Chancellor snaked my bag!
	}
	counter = counter + 1;
	_vm->_talk->talkFromIni(0, textIndex);
	if (flag8Set) {
		_vm->setFlags(ENGINE_FLAG_8);
	}
}

} // End of namespace Dragons