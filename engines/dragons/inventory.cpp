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
#include "inventory.h"

namespace Dragons {

static const Common::Point positionTable[4] = {
		{2,0},
		{0xce,0},
		{2,0x9e},
		{0xce,0x9e}
};

Inventory::Inventory(DragonsEngine *vm) : _vm(vm) {
	_type = 0;
	_sequenceId = 0;
	_screenPositionIndex = 0;
	_old_showing_value = 0;
}

void Inventory::init(ActorManager *actorManager) {
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

} // End of namespace Dragons