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
#include "cursor.h"
#include "actor.h"
#include "dragons.h"
#include "dragonini.h"
#include "inventory.h"

namespace Dragons {

Cursor::Cursor(DragonsEngine *vm): _vm(vm), _actor(0), _x(0), _y(0) {
	_sequenceID = 0;
}

void Cursor::init(ActorManager *actorManager, DragonINIResource *dragonINIResource) {
	_sequenceID = 0;
	_actor = actorManager->loadActor(0, 0); //Load cursor
	_actor->x_pos = _x = 160;
	_actor->y_pos = _y = 100;
	_actor->priorityLayer = 6;
	_actor->flags = 0;
	_actor->field_e = 0x100;
	_actor->updateSequence(_sequenceID);
	_actor->flags |= (Dragons::ACTOR_FLAG_40 | Dragons::ACTOR_FLAG_80 | Dragons::ACTOR_FLAG_100 |
					  Dragons::ACTOR_FLAG_200);

	dragonINIResource->getFlickerRecord()->actor = _actor; //TODO is this correct?
	dragonINIResource->getFlickerRecord()->field_1a_flags_maybe |= Dragons::INI_FLAG_1;
}

void Cursor::update() {
	if (!_vm->isFlagSet(Dragons::ENGINE_FLAG_8) || _vm->isFlagSet(Dragons::ENGINE_FLAG_100)) {
		return;
	}
	// TODO update cursor from inputs here.

	// 0x800280b8
	if (_sequenceID == 0 && (_vm->_inventory->getType() == 1 || _vm->_inventory->getType() == 2)) {
		_sequenceID = 1;
	}

	_actor->x_pos = _x;
	_actor->y_pos = _y;

	// 0x80028104
}

void Cursor::updateVisibility() {
	if (_vm->isFlagSet(Dragons::ENGINE_FLAG_8) && !_vm->isUnkFlagSet(Dragons::ENGINE_UNK1_FLAG_10)) {
		_actor->priorityLayer = 9;
	} else {
		_actor->priorityLayer = 0;
	}
}

void Cursor::updatePosition(int16 x, int16 y) {
	_x = x;
	_y = y;
}

} // End of namespace Dragons
