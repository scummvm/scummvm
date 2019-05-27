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
}

void Inventory::init(ActorManager *actorManager, BackgroundResourceLoader *backgroundResourceLoader, Bag *bag, DragonINIResource *dragonIniResource) {
	_actor = actorManager->loadActor(1, 1); //Load inventory
	_actor->x_pos = 2;
	_actor->y_pos = 0;
	_actor->priorityLayer = 6;
	_actor->flags = 0;
	_actor->field_e = 0x100;
	_actor->updateSequence(0);
	_actor->flags |= (Dragons::ACTOR_FLAG_40 | Dragons::ACTOR_FLAG_80 | Dragons::ACTOR_FLAG_100 |
						 Dragons::ACTOR_FLAG_200);
	_sequenceId = 0;
	_type = 0;
	_old_showing_value = 0;
	_bag = bag;

	for(int i = 0x17; i < 0x29; i++) {
		actorManager->loadActor(0, i); // TODO need to share resource between inventory item actors.
	}

	loadInventoryItemsFromSave();
}


void Inventory::loadScene(uint32 sceneId) {
	if (!_type) {
		_sequenceId = _vm->isFlagSet(Dragons::ENGINE_FLAG_400000) ? 1 : 0;
	}
	_screenPositionIndex = _vm->_dragonRMS->getInventoryPosition(sceneId);

	if (_sequenceId == 0 && _vm->getVar(7) == 1) {
		_actor->updateSequence(5);
	} else {
		_actor->updateSequence(_sequenceId);
	}

	_actor->x_pos = positionTable[_screenPositionIndex].x;
	if ((_sequenceId == 0 || _sequenceId == 2) && (_screenPositionIndex == 1 || _screenPositionIndex == 3)) {
		_actor->x_pos += 0x32;
	}
	_actor->y_pos = positionTable[_screenPositionIndex].y;
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

void Inventory::setPriority(uint16 priority) {
	_actor->priorityLayer = priority;
}

void Inventory::setActorSequenceId(int32 sequenceId) {
	if (isActorSet()) {
		_actor->_sequenceID = sequenceId;
	}
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

	for(int i = 0; i < 0x29; i++) {
		Actor *item = _vm->_actorManager->getActor(i + 0x17);

		item->x_pos = item->target_x_pos = invXPosTable[i] + 0x10;
		item->y_pos = item->target_y_pos = invYPosTable[i] + 0xc;

		if (_vm->unkArray_uint16[i]) {
			item->flags = 0; //clear all flags
			item->field_e = 0x100;
			item->priorityLayer = 0;
			item->updateSequence(_vm->getINI(_vm->unkArray_uint16[i] - 1)->field_8 * 2 + 10);
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
	_vm->_actorManager->clearActorFlags(0x17);
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
	for (int i = 0; i < 0x29; i++) {
		if (_vm->unkArray_uint16[i]) {
			Actor *item = _vm->_actorManager->getActor(i + 0x17);
			if (item->x_pos - 0x10 <= x && x < item->x_pos + 0x10
				&& item->y_pos - 0xc <= y && y < item->y_pos + 0xc) {
				return _vm->unkArray_uint16[i];
			}
		}
	}
	return 0;
}

void Inventory::loadInventoryItemsFromSave() {
	int j = 0;
	for (int i=0; i < _vm->_dragonINIResource->totalRecords() && j < 0x29; i++ ) {
		DragonINI *ini = _vm->_dragonINIResource->getRecord(i);
		if (ini->sceneId == 1) {
			_vm->unkArray_uint16[j++] = i + 1;
		}
	}
}

} // End of namespace Dragons